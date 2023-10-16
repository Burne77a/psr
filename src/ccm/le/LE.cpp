#include "LE.h"    
#include "Logger.h"
#include <errnoLib.h>

LE::LE(GMM &gmm) : m_gmm(gmm), m_currentState(std::make_unique<Follower>())
{
  
}

LE::~LE()
{
  
}


OSAStatusCode LE::Start()
{
  static const int TaskPrio = 30;   
  static const std::string TaskName("tLeCcm");
  
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)LE::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
    LogMsg(LogPrioError, "ERROR LE::Start Failed to spawn CCM LE task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  return OSA_OK;
  
}

void LE::Stop()
{
  LogMsg(LogPrioInfo, "LE::Stop() - Commanding LE task to end. ");
  m_isRunning = false;
}

void LE::HandleActivity()
{
  m_currentState->HandleActivity();
}

OSAStatusCode LE::LeaderElectionTaskMethod()
{
  m_isRunning = true;
  do
  {
    HandleActivity();
    
    OSATaskSleep(m_periodInMs);
  }while(m_isRunning);
  return OSA_OK;
}

OSAStatusCode LE::ClassTaskMethod(void * const pInstance)
{
  return ((LE*)(pInstance))->LeaderElectionTaskMethod();
}


void LE::SetState(std::unique_ptr<StateBaseLE> newState)
{
  if (m_currentState->GetValue() != newState->GetValue())
  {
    LogStateChange(m_currentState->GetStateName(), newState->GetStateName());
    m_currentState = std::move(newState);
  }
}

std::string LE::GetCurrentStateName() const
{
  return m_currentState->GetStateName();
}

StateBaseLE::StateValue LE::GetCurrentStateValue() const
{
  return m_currentState->GetValue();
}

void LE::LogStateChange(const std::string& from, const std::string& to)
{
  LogMsg(LogPrioInfo, "LE state change: %s --> %s", from.c_str(), to.c_str());
}


