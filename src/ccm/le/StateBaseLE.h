#ifndef CCM_STATEBASELE_H
#define CCM_STATEBASELE_H
#include "../gmm/GMM.h"
#include <string>



class StateBaseLE
{
  public:
    enum class StateValue
    {
      Follower = 1,
      Electing,
      Leader
    };
  
    virtual ~StateBaseLE() = default;
    virtual std::string GetStateName() const = 0;
    virtual StateValue GetValue() const = 0;
    
    virtual void Print() const = 0;
    
    // Introducing activity handler in state base.
    virtual void HandleActivity(StateBaseLE::StateValue &nextState, GMM &gmm) = 0;
};
#endif //CCM_STATEBASELE_H
