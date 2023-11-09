#include "FirstSimpleTestApp.h"
#include "Logger.h"
#include <errnoLib.h>
FirstSimpleTestApp::FirstSimpleTestApp(unsigned int appId,std::string_view primaryIpAddr, std::string_view backupIpAddr, ARF& arf,unsigned int periodInMs) : 
m_primaryIpAddr{primaryIpAddr},m_backupIpAddr{backupIpAddr},m_appId{appId}, m_arf{arf}, m_periodInMs{periodInMs}
{
 
}

FirstSimpleTestApp::~FirstSimpleTestApp()
{
 
}

void FirstSimpleTestApp::Start(const bool asPrimary) 
{
  static const int TaskPrio = 40;   
  static const std::string TaskNameBase{"tSApp"};
  const std::string TaskName{TaskNameBase+std::to_string(m_appId)};
  
  m_isPrimary = asPrimary;
  
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)FirstSimpleTestApp::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
   LogMsg(LogPrioError, "ERROR FirstSimpleTestApp::Start Failed to spawn FirstSimpleTestApp task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
}

void FirstSimpleTestApp::RunStateMachine()
{
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
  LogMsg(LogPrioInfo, "AFD::FailureDetectionTaskMethod - AFD for AppId %u running ",m_appId);
  
  if(m_arf.RegisterApp(m_appId, m_primaryIpAddr, m_backupIpAddr, m_periodInMs))
  {
    m_isRunning = true;
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
  LogMsg(LogPrioInfo, "FirstSimpleTestApp: id %u period: %u state: %s ",m_appId,m_periodInMs,m_isPrimary ? "Primary" : "Backup");
}
