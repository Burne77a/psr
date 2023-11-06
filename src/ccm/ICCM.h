#ifndef CCM_ICCM_H
#define CCM_ICCM_H 
#include "csa/UpcallCallback.h"
#include "csa/ClientMessage.h"
#include "Logger.h"
#include <memory>
class ICCM
{
  public:
    template<typename T>
    static std::unique_ptr<T> CreateInstance(const ClientMessage & msg)
    {
      unsigned int payloadSize = 0U;
      std::unique_ptr<T> pT{nullptr};
      const uint8_t * pPayloadData = msg.GetDeserializedPayload(payloadSize);
        
      if(pPayloadData != nullptr && payloadSize > 0)
      {
        pT = std::make_unique<T>(pPayloadData,payloadSize);
        if(!pT)
        {
          LogMsg(LogPrioError,"ICCM::CreateInstance failed to create instance. 0x%x %u ",pPayloadData,payloadSize);
        }
      }
      else
      {
        LogMsg(LogPrioError,"ICCM::CreateInstance invalid payload or size. 0x%x %u ",pPayloadData,payloadSize);
      } 
      return pT;
    };
    virtual bool ReplicateRequest(const ClientMessage & msg) = 0;
    virtual bool RegisterService(const unsigned int serviceId,UpcallCallbackType upcallCb) = 0; 
    virtual ClientRequestId CreateUniqueId() = 0;
};

    
#endif
