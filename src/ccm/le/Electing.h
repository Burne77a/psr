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

  void HandleActivity(StateBaseLE::StateValue &nextState, GMM &gmm)  override;
  
  void Print() const override;
};

#endif //CCM_ELECTING_H
