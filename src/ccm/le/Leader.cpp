#include "Leader.h"
#include "LeaderElectionMsg.h"
#include "Logger.h"

void Leader::HandleActivity(std::unique_ptr<LeaderElectionMsg> &pMsg, StateBaseLE::StateValue &nextState, GMM &gmm)
{
  // Implement electing-specific behavior here.
  LogMsg(LogPrioInfo, "Handling activity in Leader state.");
}

void Leader::Print() const
{
  LogMsg(LogPrioInfo, "State Leader.");
}
