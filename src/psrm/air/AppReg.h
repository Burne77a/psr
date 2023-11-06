#ifndef AIR_APPREG_H
#define AIR_APPREG_H
#include "AppInfo.h"
#include <memory>
#include <unordered_map>
#include <mutex>
class AppReg
{
  public:
    AppReg();
    ~AppReg() = default;
    bool AddEntry(AppInfo &entryToAdd);
    void RemoveEntry(unsigned int appId);
    void Print() const;
  private:
    std::unordered_map<unsigned int, AppInfo> m_appInfoEntries{};
    mutable std::mutex m_mutex;
};


#endif //AIR_APPREG_H
