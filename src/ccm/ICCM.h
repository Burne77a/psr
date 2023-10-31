#ifndef CCM_ICCM_H
#define CCM_ICCM_H 
#include "csa/UpcallCallback.h"
#include "csa/ClientMessage.h"
class ICCM
{
  public:
    virtual bool ReplicateRequest(const ClientMessage & msg) = 0;
    virtual bool RegisterService(const unsigned int serviceId,UpcallCallbackType upcallCb) = 0; 
    virtual ClientRequestId CreateUniqueId() = 0;
};

    
#endif
