#ifndef ARF_MAIN_H
#define ARF_MAIN_H
#include "afd/AFD.h"
#include "asr/ASR.h"
#include "../psrm/PSRM.h"
#include "TaskAbstraction.h"
#include <string>
#include <memory>
#include <unordered_map>

class ARF
{
  public:
    static std::unique_ptr<ARF> CreateARF(PSRM &psrm, const unsigned int backupId);
    ARF(PSRM &psrm,const unsigned int backupId);
    bool RegisterAppForFD(const unsigned int appId, std::string_view primaryIp, std::string_view backupIp, const unsigned int hbTmoTimeInMs,PrimaryHbTimeoutCb hbTmoCb);
    bool RegisterAppForStateStorage(const unsigned int appId, const unsigned int primaryNodeId, const unsigned int bytes, const unsigned int periodInMs);
    bool SetAsPrimary(const unsigned int appId);
    bool SetAsBackup(const unsigned int appId);
    bool Kickwatchdog(const unsigned int appId);
    void Print() const;  
    
  private:
    
    //Called when there is an update of app state storage pairing
    void StateStorageChangeCallback(const AppStateStoragePair &affectedPair, bool isRemoved);
    void CreateAndInstallASR(const unsigned int appId, const unsigned int primaryNodeId, const unsigned int storageNodeId, const unsigned int thisNodeId ,std::string_view backupIp,std::string_view storageIp);
    std::unordered_map<unsigned int, std::unique_ptr<AFD>> m_appFailuredetectors{};
    std::unordered_map<unsigned int, std::unique_ptr<ASR>> m_appStateReplicators{};
    PSRM & m_psrm;
    const unsigned int m_backupId;
};

#endif //ARF_MAIN_H
