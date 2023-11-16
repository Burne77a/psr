#ifndef ARF_ASR_STATESTORER_H
#define ARF_ASR_STATESTORER_H


#include "TaskAbstraction.h"
#include "ISender.h"
#include "StateDataMsg.h"
#include <memory>
#include <string>


class StateStorer
{
  public:
    
    static std::unique_ptr<StateStorer> CreateStateStorer(const unsigned int appId, const unsigned int port, std::string_view storageIp);
    StateStorer(const unsigned int appId,std::unique_ptr<ISender> &pSender);
    ~StateStorer() = default;
    bool SendAppDataToStorage(const ISerializable & appStateData);    
    void Print() const;
 
  private:        
    std::unique_ptr<ISender> m_pSender; 
    const unsigned int m_appId;
};

#endif //ARF_ASR_STATESTORER_H
