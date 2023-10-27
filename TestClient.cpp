#include "TestClient.h"
#include "Logger.h"
#include <functional>
TestClient::TestClient(CSA & csa) : m_csa{csa}
{
  
}


void TestClient::IssueRequest()
{
  
}
    
void TestClient::RegisterForUpcall()
{
  m_csa.RegisterService(TestClientServiceId, std::bind(&TestClient::UpcallMethod,this, std::placeholders::_1));
}

void TestClient::UpcallMethod(const ClientMessage& commitedMsg)
{
  
  
}
