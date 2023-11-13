#include "FirstSimpleTestApp.h"
#include "Logger.h"
#include <errnoLib.h>
FirstSimpleTestApp::FirstSimpleTestApp(unsigned int appId,std::string_view primaryIpAddr, std::string_view backupIpAddr, ARF& arf,unsigned int periodInMs,unsigned int nodeId) : 
m_primaryIpAddr{primaryIpAddr},m_backupIpAddr{backupIpAddr},m_appId{appId}, m_arf{arf}, m_periodInMs{periodInMs}, m_nodeId{nodeId}
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
    LogMsg(LogPrioInfo, "AFD::PrimaryHbTimeoutCb ARF says that this app %u should become primary",m_appId);
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

void FirstSimpleTestApp::RunStateMachine()
{
  if(m_nextIsPrimary != m_isPrimary)
  {
    LogMsg(LogPrioInfo, "AFD::RunStateMachine changed role to %s",m_nextIsPrimary? "Primary" : "Backup");
    m_isPrimary = m_nextIsPrimary;
  }
  
  if(m_isPrimary)
  {
    if(!m_arf.Kickwatchdog(m_appId))
    {
      LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::RunStateMachine Kickwatchdog failed %u", m_appId);
    }
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
      if(!m_arf.RegisterAppForStateStorage(m_appId,m_nodeId,100,100))
      {
        LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::AppTaskMethod failed to register state storage need with ARF %u", m_appId,m_nodeId);
        return OSA_ERROR;
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
  LogMsg(LogPrioInfo, "FirstSimpleTestApp: AppId %u NodeId: %u period: %u state: %s ",m_appId,m_nodeId,m_periodInMs,m_isPrimary ? "Primary" : "Backup");
}
