#ifndef CCM_LEADER_H
#define CCM_LEADER_H

#include "StateBaseLE.h"

class Leader : public StateBaseLE
{
public:
  std::string GetStateName() const override
  {
    return "Leader";
  }

  StateValue GetValue() const override
  {
    return StateValue::Leader;;
  }

  void HandleActivity() override
  {
    // Implement electing-specific behavior here.
    LogMsg(LogPrioInfo, "Handling activity in Leader state.");
  }
};

#endif //CCM_LEADER_H
