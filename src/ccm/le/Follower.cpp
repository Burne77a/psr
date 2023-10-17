#include "Follower.h"
#include "Logger.h"
void Follower::HandleActivity(StateBaseLE::StateValue &nextState) 
{
  // Implement follower-specific behavior here.
  LogMsg(LogPrioInfo, "Handling activity in Follower state.");
}
