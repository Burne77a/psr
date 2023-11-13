#include "AFD.h"
#include "HeartbeatAFD.h"
#include "Logger.h"
#include "NwAid.h"
#include <errnoLib.h>

static const int APP_FD_PORT_BASE = 9000;

std::unique_ptr<AFD> AFD::CreateAFD(const unsigned int appId, std::string_view primaryIp, std::string_view backupIp, 
    const unsigned int hbTmoTimeInMs,PrimaryHbTimeoutCb hbTmoCb)
{
  const int appFdPort = APP_FD_PORT_BASE + appId;
  
  std::unique_ptr<AFD> pAfd{nullptr};
  
  
  std::unique_ptr<IReceiver> pRcv = NwAid::CreateUniCastReceiver(appFdPort);
  if(!pRcv)
  {
    LogMsg(LogPrioCritical, "ERROR: AFD::CreateAFD failed to create IReceiver (port %d). Errno: 0x%x (%s)",appFdPort,errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<ISender> pSnd = NwAid::CreateUniCastSender(backupIp,appFdPort);
  if(!pSnd)
  {
    LogMsg(LogPrioCritical, "ERROR: AFD::CreateAFD failed to create ISender (port %d). Errno: 0x%x (%s)",appFdPort,errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  
  pAfd = std::make_unique<AFD>(appId,pSnd,pRcv,hbTmoTimeInMs,hbTmoCb);
  if(!pAfd)
  {
    LogMsg(LogPrioCritical, "ERROR: AFD::CreateAFD failed to create AFD. appId: %u  Errno: 0x%x (%s)",appId,errnoGet(),strerror(errnoGet()));
  }
  
  return pAfd;
}

AFD::AFD(const unsigned int appId, std::unique_ptr<ISender> &pSender, std::unique_ptr<IReceiver> &pReceiver,const unsigned int hbTmoTimeInMs,PrimaryHbTimeoutCb hbTmoCb) 
: m_appId{appId}, m_pSender{std::move(pSender)},m_pReceiver{std::move(pReceiver)},
  m_hbTmoTimeInMs{hbTmoTimeInMs},m_hbTmoCb{hbTmoCb}
{
  
}

void AFD::Kickwatchdog()
{
  if(!SendHeartbeat())
  {
    LogMsg(LogPrioCritical, "ERROR: AFD::Kickwatchdog Failed to kick watchdog (send heartbeat) for app: %u. Errno: 0x%x (%s)",m_appId,errnoGet(),strerror(errnoGet()));
  }
}


OSAStatusCode AFD::Start()
{
  static const int TaskPrio = 30;   
  static const std::string TaskNameBase{"tAFD"};
  const std::string TaskName{TaskNameBase+std::to_string(m_appId)};
  
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)AFD::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
    LogMsg(LogPrioError, "ERROR AFD::Start Failed to spawn FD task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  return OSA_OK;
}

void AFD::Stop()
{
  LogMsg(LogPrioInfo, "AFD::Stop() - Commanding AFD task to end. ");
  m_isRunning = false;
}

void AFD::WaitingStateHandling()
{
  const EventValue pendEvent = GetPendingEvent();
  if(pendEvent == EventValue::BecomePrimary)
  {
    SetNewState(StateValue::Primary,pendEvent);
    SetPendingEvent(EventValue::None);
  }
  else if(pendEvent == EventValue::BecomeBackup)
  {
    SetNewState(StateValue::BackupWaiting,pendEvent);
    SetPendingEvent(EventValue::None);
  }
}

void AFD::BackupWaitingStateHandling()
{
  if(IsHeartbeatRcvd())
  {
    m_lastHbRcvd = std::chrono::system_clock::now();
    Flush();
    LogMsg(LogPrioInfo, "AFD::BackupWaitingStateHandling() - Heartbeat observed - becoming Backup ");
    SetNewState(StateValue::Backup,EventValue::BecomeBackup);
    SetPendingEvent(EventValue::None);
  }

}

void AFD::BackupStateHandling()
{
  if(IsHeartbeatRcvd())
  {
    m_lastHbRcvd = std::chrono::system_clock::now();
    Flush();
  }
  else
  {
    if(HasHeartbeatTmo())
    {
      LogMsg(LogPrioInfo, "AFD::BackupStateHandling() - Heartbeat timeout - becoming Primary ");
      SetNewState(StateValue::Primary,EventValue::BecomePrimary);
      SetPendingEvent(EventValue::None);
      if(m_hbTmoCb)
      {
        LogMsg(LogPrioInfo, "AFD::BackupStateHandling() - Heartbeat timeout - informing application by calling callback");
      }
    }
  } 
}

void AFD::PrimaryStateHandling()
{
  
}

void AFD::RunStateMachine()
{
  switch(m_currentState)
  {
    case StateValue::Waiting:
    {
      WaitingStateHandling();
      break;
    }
    case StateValue::BackupWaiting:
    {
      BackupWaitingStateHandling();
      break;
    }
    case StateValue::Backup:
    {
      BackupStateHandling();
      break;
    }
    case StateValue::Primary:
    {
      PrimaryStateHandling();
      break;
    }
    default:
    {
      LogMsg(LogPrioCritical, "AFD::RunStateMachine - AFD for AppId %u invalid state %d ",m_appId,m_currentState);
    }
  };
}

bool AFD::HasHeartbeatTmo()
{
  static const std::chrono::milliseconds HbTmoInMsDuration (m_hbTmoTimeInMs);
  auto now = std::chrono::system_clock::now();
  auto elapsed = now - m_lastHbRcvd;
  return (elapsed > HbTmoInMsDuration);
}


bool AFD::SendHeartbeat()
{
  HeartbeatAFD outgoingHeartbeat;
  outgoingHeartbeat.SetSenderAppId(m_appId);
  return m_pSender->Send(outgoingHeartbeat);
}

bool AFD::IsHeartbeatRcvd()
{
  return RcvMyHb();
}

bool AFD::RcvMyHb()
{
  HeartbeatAFD incomingHeartbeat;
  while(m_pReceiver->Rcv(incomingHeartbeat))
  {
    if(incomingHeartbeat.GetSenderAppId() == m_appId)
    {
      return true;
    }
    else
    {
      LogMsg(LogPrioCritical, "AFD::RcvMyHb received heartbeat destined to another app. Rcvd: %u My: %u",incomingHeartbeat.GetSenderAppId(), m_appId);
    }
  }
  return false;
}

void AFD::Flush()
{
  HeartbeatAFD incomingHeartbeat;
  while(m_pReceiver->Rcv(incomingHeartbeat));
}

void AFD::SetPendingEvent(const EventValue event)
{
  std::lock_guard<std::mutex> lock(m_eventMutex);
  if(event != m_pendingEvent)
  {
    LogMsg(LogPrioInfo, "AFD::SetPendingEvent: %s --> %s", GetEventAsString(m_pendingEvent).c_str(), GetEventAsString(event).c_str());
    m_pendingEvent = event;
  }
  
}
 
AFD::EventValue AFD::GetPendingEvent()
{
  std::lock_guard<std::mutex> lock(m_eventMutex);
  return m_pendingEvent;
  
}

void AFD::SetNewState(const StateValue newState, const EventValue event )
{
  if(newState != m_currentState)
  {
    LogMsg(LogPrioInfo, "AFD::SetNewState: %s: %s --> %s",GetEventAsString(event).c_str(),GetStateAsString(m_currentState).c_str(),GetStateAsString(newState).c_str() );
    m_currentState = newState;
  }
}

OSAStatusCode AFD::FailureDetectionTaskMethod()
{
  m_isRunning = true;
  LogMsg(LogPrioInfo, "AFD::FailureDetectionTaskMethod - AFD for AppId %u running ",m_appId);
  do
  {
    RunStateMachine();
    OSATaskSleep(m_periodInMs);
  }while(m_isRunning);
  
  LogMsg(LogPrioInfo, "WARNING: AFD::FailureDetectionTaskMethod - AFD for AppId %u terminated ",m_appId);
  return OSA_OK;
}


OSAStatusCode AFD::ClassTaskMethod(void * const pInstance)
{
  return ((AFD*)(pInstance))->FailureDetectionTaskMethod();
}


  
void AFD::Print() const
{
  LogMsg(LogPrioInfo,"--- >AFD< ---");
  LogMsg(LogPrioInfo,"State: %s (%d) pending event: %s (%d) Tmo: %u",GetStateAsString(m_currentState).c_str(),m_currentState,GetEventAsString(m_pendingEvent).c_str(),m_pendingEvent,m_hbTmoTimeInMs);
  LogMsg(LogPrioInfo,"--- <AFD> ---");
}

static const char * const stateStr[4] =  {"Waiting","BackupWaiting","Backup","Primary"};
const std::string AFD::GetStateAsString(const StateValue state)
{
  return std::string(stateStr[(int)state]);
}

static const char * const eventStr[3] =  {"None","BecomePrimary","BecomeBackup"};
const std::string AFD::GetEventAsString(const EventValue event)
{
  return std::string(eventStr[(int)event]);
}

