#ifndef PSRTESTSTARTER_H_
#define PSRTESTSTARTER_H_
#include "ErrorCodes.h"


#ifdef __cplusplus
extern "C" {
#endif

OSAStatusCode StartPSRTest(const int id);

void Print();

#ifdef __cplusplus
}
#endif 

#endif //PSRTESTSTARTER_H_
