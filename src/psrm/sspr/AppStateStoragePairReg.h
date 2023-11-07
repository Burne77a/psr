#ifndef SSPR_APPSTATE_STORAGE_REG_H
#define SSPR_APPSTATE_STORAGE_REG_H
#include "AppStateStoragePair.h"
#include <memory>
#include <unordered_map>
#include <mutex>
#include <optional>
class AppStateStoragePairReg
{
  public:
    AppStateStoragePairReg();
    ~AppStateStoragePairReg() = default;
    bool AddEntry(AppStateStoragePair &entryToAdd);
    void RemoveEntry(const unsigned int appId);
    std::optional<AppStateStoragePair> GetEntry(const unsigned int appId) const;
    std::optional<unsigned int> FindLowestUsedStateStorage(unsigned int &usingAppsCount) const;
    void GetAllAppIdThatUseThisStorageId(const unsigned int storageId, std::vector<unsigned int>& appIds) const;
    void Print() const;
  private:
    std::unordered_map<unsigned int, AppStateStoragePair> m_appStatePairEntries{};
    mutable std::mutex m_mutex;
};


#endif //SSPR_APPSTATE_STORAGE_REG_H
