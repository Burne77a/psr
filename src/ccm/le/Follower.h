#ifndef CCM_FOLLOWER_H
#define CCM_FOLLOWER_H

#include "StateBaseLE.h"


class Follower : public StateBaseLE
{
public:
  std::string GetStateName() const override
  {
    return "Follower";
  }

  StateValue GetValue() const override
  {
    return StateValue::Follower;
  }

  void HandleActivity(StateBaseLE::StateValue &nextState) override;
};


#endif //CCM_FOLLOWER_H
