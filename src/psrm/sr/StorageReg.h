#ifndef AIR_STORAGEREG_H
#define AIR_STORAGEREG_H
#include "StorageInfo.h"
#include <memory>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <vector>
class StorageReg
{
  public:
    StorageReg();
    ~StorageReg() = default;
    bool AddEntry(StorageInfo &entryToAdd);
    void RemoveEntry(unsigned int storageId);
    std::optional<StorageInfo> GetStorageInfo(const unsigned int storageId) const;
    void GetAllStorageIds(std::vector<unsigned int> &storageIds) const;
    void Print() const;
  private:
    std::unordered_map<unsigned int, StorageInfo> m_storageInfoEntries{};
    mutable std::mutex m_mutex;
};


#endif //AIR_STORAGEREG_H
