#include "LE.h"    
#include "Logger.h"
#include "NwAid.h"
#include "Follower.h"
#include "Electing.h"
#include "Leader.h"
#include "../misc/Misc.h"
#include <errnoLib.h>

std::unique_ptr<LE> LE::CreateLE(GMM & gmm)
{
  static const int LEADER_ELECTION_UDP_PORT = 5555;
  std::vector<std::unique_ptr<ISender>> senders;
  bool isAllCreationOk = Misc::CreateISendersFromMembers(LEADER_ELECTION_UDP_PORT,gmm,senders);
  std::unique_ptr<IReceiver> pRcv = nullptr;
  std::unique_ptr<LE> pLe = nullptr;
    
  if(!isAllCreationOk)
  {
    LogMsg(LogPrioCritical, "ERROR LE::CreateLE failed to create sender(s). Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  else
  {
    pRcv = NwAid::CreateUniCastReceiver(LEADER_ELECTION_UDP_PORT);
    if(!pRcv)
    {
      LogMsg(LogPrioCritical, "ERROR LE::CreateLE failed to create receiver. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
      isAllCreationOk = false;
    }
  }
  if(isAllCreationOk)
  {
    pLe = std::make_unique<LE>(gmm,senders,pRcv);
    if(!pLe)
    {
      LogMsg(LogPrioCritical, "ERROR LE::CreateLE failed to create LE.");
    }
    else
    {
      LogMsg(LogPrioInfo, "LE::CreateLE Successfully created a LE instance.");
    }
  }
  return pLe;
}

LE::LE(GMM &gmm, std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver) : 
    m_gmm(gmm), m_currentState(std::make_unique<Follower>()),m_senders(),m_pReceiver(std::move(pReceiver))
{
  
}

LE::~LE()
{
  
}

void LE::HandleActivity()
{
  StateBaseLE::StateValue nextState = m_currentState->GetValue();
  m_currentState->HandleActivity(nextState,m_gmm);
  if(nextState != m_currentState->GetValue())
  {
    SetState(CreateState(nextState));
  }
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

std::unique_ptr<StateBaseLE> LE::CreateState(StateBaseLE::StateValue stateValue)
{
  std::unique_ptr<StateBaseLE> pState = nullptr;
  if(stateValue == StateBaseLE::StateValue::Follower)
  {
    pState = std::make_unique<Follower>(); 
  }
  else if(stateValue == StateBaseLE::StateValue::Electing)
  {
    pState = std::make_unique<Electing>(); 
  }
  else if(stateValue == StateBaseLE::StateValue::Leader)
  {
    pState = std::make_unique<Leader>(); 
  }
  else
  {
    LogMsg(LogPrioEmergency, "ERROR: LE::CreateState invalid state value 0x%x",stateValue);
  }
  
  if(!pState)
  {
    LogMsg(LogPrioEmergency, "ERROR: LE::CreateState failed to create state",stateValue);
  }
  
  return pState;
}

void LE::LogStateChange(const std::string& from, const std::string& to)
{
  LogMsg(LogPrioInfo, "LE state change: %s --> %s", from.c_str(), to.c_str());
}

void LE::Print()
{
  LogMsg(LogPrioInfo, "--- LE ---");
  
  m_currentState->Print();
  
  LogMsg(LogPrioInfo, "----------");
}


