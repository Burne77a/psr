#ifndef CCM_ELECTING_H
#define CCM_ELECTING_H

#include "StateBaseLE.h"

class Electing : public StateBaseLE
{
public:
  std::string GetStateName() const override
  {
    return "Electing";
  }

  StateValue GetValue() const override
  {
    return StateValue::Electing;
  }

  void HandleActivity() override
  {
    // Implement electing-specific behavior here.
    LogMsg(LogPrioInfo, "Handling activity in Electing state.");
  }
};

#endif //CCM_ELECTING_H
