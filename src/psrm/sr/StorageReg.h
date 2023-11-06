#ifndef AIR_STORAGEREG_H
#define AIR_STORAGEREG_H
#include "StorageInfo.h"
#include <memory>
#include <unordered_map>
#include <mutex>
class StorageReg
{
  public:
    StorageReg();
    ~StorageReg() = default;
    bool AddEntry(StorageInfo &entryToAdd);
    void RemoveEntry(unsigned int storageId);
    void Print() const;
  private:
    std::unordered_map<unsigned int, StorageInfo> m_storageInfoEntries{};
    mutable std::mutex m_mutex;
};


#endif //AIR_STORAGEREG_H
