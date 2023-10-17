#include "Electing.h"
#include "Logger.h"

void Electing::HandleActivity(StateBaseLE::StateValue &nextState, GMM &gmm) 
{
  // Implement electing-specific behavior here.
  LogMsg(LogPrioInfo, "Handling activity in Electing state.");
}

void Electing::Print() const
{
  LogMsg(LogPrioInfo, "State Electing");
}
