#ifndef CCM_SERVICEUPCALLDISPATCHER_H
#define CCM_SERVICEUPCALLDISPATCHER_H

#include "ClientMessage.h"
#include "UpcallCallback.h"
#include <unordered_map>
#include <mutex>

class ServiceUpcallDispatcher
{
  public:
    ServiceUpcallDispatcher();
    ~ServiceUpcallDispatcher() = default;
    bool RegisterService(const unsigned int serviceId,UpcallCallbackType upcallCb);
    void MakeUpcall(std::shared_ptr<ClientMessage> pCmsg);
    void Print() const;
  private:
    std::unordered_map<int, UpcallCallbackType> m_upcallCbs;
    mutable std::mutex m_mutex{};
};


#endif //CCM_SERVICEUPCALLDISPATCHER_H
