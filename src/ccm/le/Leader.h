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

  void HandleActivity(StateBaseLE::StateValue &nextState, GMM &gmm) override;
  
  void Print() const override;
  
};

#endif //CCM_LEADER_H
