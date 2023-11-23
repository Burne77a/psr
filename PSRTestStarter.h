#ifndef PSRTESTSTARTER_H_
#define PSRTESTSTARTER_H_
#include "ErrorCodes.h"


#ifdef __cplusplus
extern "C" {
#endif

void SetOneBackupStorage();
void SetPeriodAndBytes(const unsigned int period, const unsigned int bytes);
OSAStatusCode StartPSRTest(const int id);

void Print();
void StartTestApps();
void TriggerShutdown();

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
