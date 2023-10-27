#ifndef TESTCLIENT_H_
#define TESTCLIENT_H_
#include "src/ccm/csa/CSA.h"
#include "src/ccm/csa/UpcallCallback.h"
class TestClient
{
  public:
    TestClient(CSA & csa);
    ~TestClient() = default;
    void IssueRequest();
  private:
    void UpcallMethod(const ClientMessage& commitedMsg);
    void RegisterForUpcall();
    static const unsigned int TestClientServiceId{100U};
    CSA & m_csa; 
};

#endif //TESTCLIENT_H_
