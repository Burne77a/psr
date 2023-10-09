#ifndef LOGGING_LOGGER_H_
#define LOGGING_LOGGER_H_
#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif
extern const int LogPrioEmergency;
extern const int LogPrioAlert;
extern const int LogPrioCritical;
extern const int LogPrioError;
extern const int LogPrioWarning;
extern const int LogPrioInfo;

void LogMsgInit();
void LogMsg(const int priority, const char * message, ...);


#ifdef __cplusplus
}
#endif



#endif /* LOGGING_LOGGER_H_ */
