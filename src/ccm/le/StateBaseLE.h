#ifndef CCM_STATEBASELE_H
#define CCM_STATEBASELE_H
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
    
    // Introducing activity handler in state base.
    virtual void HandleActivity(StateBaseLE::StateValue &nextState) = 0;
};
#endif //CCM_STATEBASELE_H
