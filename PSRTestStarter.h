#ifndef PSRTESTSTARTER_H_
#define PSRTESTSTARTER_H_
#include "ErrorCodes.h"


#ifdef __cplusplus
extern "C" {
#endif

OSAStatusCode StartPSRTest(const int id);

void Print();
void AddApp(unsigned int id);
void DelApp(unsigned int id);

void AddStorage(unsigned int id);
void DelStorage(unsigned int id);

void TriggerClientReq();
void ForceUpcalls();

#ifdef __cplusplus
}
#endif 

#endif //PSRTESTSTARTER_H_
