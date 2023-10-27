#ifndef CCM_UPCALLCALLBACK_H
#define CCM_UPCALLCALLBACK_H
#include "ClientMessage.h"

using UpcallCallbackType = std::function<void(const ClientMessage&)>;

#endif //CCM_UPCALLCALLBACK_H
