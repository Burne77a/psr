#include "CCM.h"

CCM::CCM()
{
  
}

CCM::~CCM()
{
  
}


void CCM::InitForTest()
{
  m_gmm.AddMember(1, "Instance 1", "192.168.101.1");
  m_gmm.AddMember(2, "Instance 2", "192.168.101.2");
  m_gmm.AddMember(3, "Instance 3", "192.168.101.3");
}
