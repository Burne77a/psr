#include "Leader.h"
#include "Logger.h"

void Leader::HandleActivity(StateBaseLE::StateValue &nextState) 
{
  // Implement electing-specific behavior here.
  LogMsg(LogPrioInfo, "Handling activity in Leader state.");
}
