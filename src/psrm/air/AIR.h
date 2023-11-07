#ifndef AIR_MAIN_H
#define AIR_MAIN_H
#include "AppReg.h"
#include "AppInfoMsg.h"
#include "../../ccm/ICCM.h"
#include <memory>

using AppInfoChangeAppAddedCallbackType = std::function<void(const unsigned int appId)>;
using AppInfoChangeAppRemovedCallbackType = std::function<void(const unsigned int appId)>;

class AIR
{
  public:
    static const unsigned int SERVICE_ID = 10;
    static std::unique_ptr<AIR> CreateAIR(std::shared_ptr<ICCM>& pIccm);
    AIR(std::shared_ptr<ICCM>& pIccm,std::unique_ptr<AppReg>& pAppReg);
    ~AIR() = default;
    bool RegisterWithCCM();
    bool RegisterApplication(const unsigned int appId, const unsigned int primaryInfo, const unsigned int bytes, const unsigned int periodInMs);
    bool DeRegisterApplication(const unsigned int appId);
    void RegisterAppAddedCb(AppInfoChangeAppAddedCallbackType callback);
    void RegisterAppRemovedCb(AppInfoChangeAppRemovedCallbackType callback);
    void Print() const;
  private:
    bool PostRequestToCCM(const std::shared_ptr<ISerializable>& pPayload, const AppInfoMsg::MsgType type);
    void HandleRemoveReq(AppInfoMsg & msg);
    void HandleAddReq(AppInfoMsg & msg);
    void HandleUpcallMessage(AppInfoMsg & msg);
    void UpcallMethod(const ClientMessage& commitedMsg);
    
    std::shared_ptr<ICCM> m_pIccm{nullptr};
    std::unique_ptr<AppReg> m_pAppReg{nullptr};
    AppInfoChangeAppAddedCallbackType m_appAddedCb{nullptr};
    AppInfoChangeAppRemovedCallbackType m_appRemovedCb{nullptr};
    
    
};

#endif //AIR_MAIN_H
