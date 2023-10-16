/* includes */

#include "vxWorks.h"
#include "PSRTestStarter.h"


void start(const int id) 
{
  (void)StartPSRTest(id); //Do not return when starting successfully.
}
