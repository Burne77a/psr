#include "Leader.h"
#include "Logger.h"

void Leader::HandleActivity(StateBaseLE::StateValue &nextState, GMM &gmm)
{
  // Implement electing-specific behavior here.
  LogMsg(LogPrioInfo, "Handling activity in Leader state.");
}

void Leader::Print() const
{
  LogMsg(LogPrioInfo, "State Leader.");
}
