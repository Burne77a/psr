#ifndef TESTCLIENT_H_
#define TESTCLIENT_H_
#include "src/ccm/ICCM.h"
#include "ISerializable.h"
#include "src/ccm/csa/UpcallCallback.h"

class TestRequest : public ISerializable
{
  private:
    struct TheData
    {
      char aMessage[33]{"Hello world, please replicate me"};
      unsigned int repCounter{0U};
    };
  public:
    TestRequest();
    ~TestRequest();
    const uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    void Print() const;
  private:
    TheData m_data;
    
    
};

class TestClient
{
  public:
    TestClient(ICCM & ccmIf);
    ~TestClient() = default;
    void IssueRequest();
    void RegisterForUpcall();
  private:
    void UpcallMethod(const ClientMessage& commitedMsg);
    
    static const unsigned int TestClientServiceId{100U};
    ICCM & m_ccmIf; 
};

#endif //TESTCLIENT_H_
