#ifndef ARF_ASR_H
#define ARF_ASR_H

#include "StateKeeper.h"
#include "StateRetriever.h"
#include "StateStorer.h"

#include <memory>
#include <string>


class ASR
{
  public:
    
    static std::unique_ptr<ASR> CreateASR(const unsigned int appId, const unsigned int primaryNodeId, const unsigned int storageNodeId, const unsigned int thisNodeId ,std::string_view backupIp,std::string_view storageIp);
    ASR(const unsigned int appId,  std::unique_ptr<StateKeeper> & pStateKeeper,std::unique_ptr<StateRetriever> & pStateRetriever,std::unique_ptr<StateStorer> &pStateStorer);
    ~ASR() = default;
    
    bool PrimaryAppSendStateToStorage(const ISerializable & objToSend);
    
    bool BackupAppGetStateFromStorage(ISerializable & objToRcvTo);
    
    OSAStatusCode Start();
    void Stop();
    void Print() const;
 
  private:
        
    const unsigned int m_appId;
    std::unique_ptr<StateKeeper> m_pStateKeeper{nullptr}; 
    std::unique_ptr<StateRetriever> m_pStateRetriever{nullptr};
    std::unique_ptr<StateStorer> m_pStateStorer{nullptr};
    
};

#endif //ARF_ASR_H
