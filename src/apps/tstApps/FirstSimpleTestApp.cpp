#include "FirstSimpleTestApp.h"
#include "DummyStateData.h"
#include "Logger.h"
#include <errnoLib.h>
FirstSimpleTestApp::FirstSimpleTestApp(unsigned int appId,std::string_view primaryIpAddr,
    std::string_view backupIpAddr, ARF& arf,unsigned int periodInMs,unsigned int nodeId,const unsigned int bytes) : 
m_primaryIpAddr{primaryIpAddr},m_backupIpAddr{backupIpAddr},m_appId{appId}, m_arf{arf}, 
m_periodInMs{periodInMs},m_bytesToSync{bytes}, m_nodeId{nodeId}
{
 
}

FirstSimpleTestApp::~FirstSimpleTestApp()
{
 
}

void FirstSimpleTestApp::PrimaryHbTimeoutCb(const unsigned int appId)
{
  if(appId != m_appId)
  {
    LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::PrimaryHbTimeoutCb AppId mismatch is: %u expected: %u", m_appId,appId);
  }
  else
  {
    LogMsg(LogPrioInfo, "FirstSimpleTestApp::PrimaryHbTimeoutCb ARF says that this app %u should become primary",m_appId);
    m_nextIsPrimary = true;
  }
}


void FirstSimpleTestApp::Start(const bool asPrimary) 
{
  static const int TaskPrio = 40;   
  static const std::string TaskNameBase{"tSApp"};
  const std::string TaskName{TaskNameBase+std::to_string(m_appId)};
  
  m_isPrimary = asPrimary;
  m_nextIsPrimary = asPrimary;
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)FirstSimpleTestApp::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
   LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::Start Failed to spawn FirstSimpleTestApp task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
}

void FirstSimpleTestApp::InitiateCountdownToShutdown()
{
  if(m_isCountDownRunning)
  {
    LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::InitiateCountdownToShutdown count down already running %u",m_appId);
  }
  else if(!m_isPrimary)
  {
    LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::InitiateCountdownToShutdown count down cannot be started on backup %u",m_appId);
  }
  else
  {
    LogMsg(LogPrioInfo, "FirstSimpleTestApp::InitiateCountdownToShutdown count down to shutdown started %u", m_appId);
    m_isCountDownStartRequested = true;
  }
}

void FirstSimpleTestApp::RunStateMachine()
{
  if(m_nextIsPrimary != m_isPrimary)
  {
    LogMsg(LogPrioInfo, "FirstSimpleTestApp::RunStateMachine changed role to %s",m_nextIsPrimary? "Primary" : "Backup");
    m_isPrimary = m_nextIsPrimary;
    if(m_isPrimary)
    {
      GetStateData();
    }
  }
  
  if(m_isPrimary)
  {
    if(!m_arf.Kickwatchdog(m_appId))
    {
      LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::RunStateMachine Kickwatchdog failed %u", m_appId);
    }
    m_iterationCount++;
    SendState();
    HandleCountdownToShutdownIfActive();
  }
}

void FirstSimpleTestApp::HandleCountdownToShutdownIfActive()
{
  if(m_isCountDownStartRequested && !m_isCountDownRunning)
  {
    LogMsg(LogPrioInfo, "Count down to shutdown running for app: %u",m_appId);
    m_isCountDownRunning = true;
    m_iterationCount = 0;
  }
  if(m_isCountDownRunning)
  {
    if(m_iterationCount >= m_shutdownLimit)
    {
      m_isRunning = false;
    }
  }
}

void FirstSimpleTestApp::SendState()
{
  DummyStateData dummyDataToSend(m_bytesToSync);
  dummyDataToSend.SetSeqNr(m_iterationCount);
  if(!m_arf.PrimaryAppSendStateToStorage(m_appId, dummyDataToSend))
  {
    LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::SendState failed to send dummy data to storage Id %u", m_appId);
  }
}

void FirstSimpleTestApp::GetStateData()
{
  DummyStateData rcvdData;
  if(m_arf.BackupAppGetStateFromStorage(m_appId, rcvdData))
  {
    if(rcvdData.GetSeqNr() == m_shutdownLimit)
    {
      LogMsg(LogPrioInfo,"Successfully retrieved state from storage when app %u became primary. With expected seqNr",m_appId);
    }
    else
    {
      LogMsg(LogPrioInfo,"WARNING, Successfully retrieved state from storage when app %u became primary but unexpected seqnr %u",m_appId,rcvdData.GetSeqNr());
    }
    rcvdData.Print();
  }
  else
  {
    LogMsg(LogPrioError,"FirstSimpleTestApp::GetStateData failed to get latest state when becoming primary",m_appId);
  }
}

OSAStatusCode FirstSimpleTestApp::AppTaskMethod()
{
  const unsigned int HbTimeout = m_periodInMs * 3;
  LogMsg(LogPrioInfo, "FirstSimpleTestApp::AppTaskMethod - AFD for AppId %u running ",m_appId);
    
  if(m_arf.RegisterAppForFD(m_appId, m_primaryIpAddr, m_backupIpAddr, HbTimeout,std::bind(&FirstSimpleTestApp::PrimaryHbTimeoutCb,this, std::placeholders::_1)))
  {
    m_isRunning = true;
    if(m_isPrimary)
    {
      if(!m_arf.RegisterAppForStateStorage(m_appId,m_nodeId,m_bytesToSync,m_periodInMs))
      {
        LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::AppTaskMethod failed to register state storage need with ARF %u", m_appId,m_nodeId);
       // return OSA_ERROR;
      }
      if(!m_arf.SetAsPrimary(m_appId))
      {
        LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::AppTaskMethod failed to set app as primary with ARF %u", m_appId);
        return OSA_ERROR;
      }
    }
    else
    {
      if(!m_arf.SetAsBackup(m_appId))
      {
        LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::AppTaskMethod failed to set app as backup with ARF %u", m_appId);
        return OSA_ERROR;
      }
    }
    
  }
  else
  {
    LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::AppTaskMethod failed to register app with ARF %u", m_appId);
    m_isRunning = false;
  }
  
  OSATaskSleep(10*1000); //Wait for state storage to be setup.
  while(m_isRunning)
  {
    RunStateMachine();
    OSATaskSleep(m_periodInMs);
  };
  
  LogMsg(LogPrioInfo, "WARNING: AFD::FailureDetectionTaskMethod - AFD for AppId %u terminated ",m_appId);
  return OSA_OK;
}



OSAStatusCode FirstSimpleTestApp::ClassTaskMethod(void * const pInstance)
{
  return ((FirstSimpleTestApp*)(pInstance))->AppTaskMethod();
}

void FirstSimpleTestApp::Print() const 
{
  LogMsg(LogPrioInfo, "FirstSimpleTestApp: AppId %u NodeId: %u period: %u bytes: %u state: %s ",m_appId,m_nodeId,m_periodInMs,m_bytesToSync,m_isPrimary ? "Primary" : "Backup");
}
