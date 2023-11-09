#ifndef ARF_MAIN_H
#define ARF_MAIN_H
#include "afd/AFD.h"
#include "TaskAbstraction.h"
#include <string>
#include <memory>
#include <unordered_map>

class ARF
{
  public:
    static std::unique_ptr<ARF> CreateARF();
    ARF();
    bool RegisterApp(const unsigned int appId, std::string_view primaryIp, std::string_view backupIp, const unsigned int hbTmoTimeInMs);
    bool SetAsPrimary(const unsigned int appId);
    bool Kickwatchdog(const unsigned int appId);
    void Print() const;  
    
  private:
    std::unordered_map<unsigned int, std::unique_ptr<AFD>> m_appFailuredetectors{};
};

#endif //ARF_MAIN_H
