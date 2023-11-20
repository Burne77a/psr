/* includes */

#include "vxWorks.h"
#include "PSRTestStarter.h"


void start(const int id) 
{
  (void)StartPSRTest(id); //Do not return when starting successfully.
}

void startOneBackupStorage(const int id, const unsigned int period, const unsigned int bytes) 
{
  SetPeriodAndBytes(period,bytes);
  SetOneBackupStorage();
  (void)StartPSRTest(id);
}

void startPartibleStorage(const int id, const unsigned int period, const unsigned int bytes) 
{
  SetPeriodAndBytes(period,bytes);
  (void)StartPSRTest(id);
}

