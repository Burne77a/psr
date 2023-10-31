#include "TestClient.h"
#include "Logger.h"
#include <functional>


TestRequest::TestRequest()
{
  static unsigned int TestRequestCreationCounter = 0U;
  m_data.repCounter = TestRequestCreationCounter;
  TestRequestCreationCounter++;  
}

TestRequest::~TestRequest()
{
  
}
const uint8_t *  TestRequest::Serialize(uint32_t &size) const
{
  size = sizeof(m_data);
  return (uint8_t*)&m_data;
}
const uint8_t * TestRequest::GetSerializableDataBuffer(uint32_t &size) const
{
  return Serialize(size);
}
bool TestRequest::Deserialize()
{
  return true;
}

void TestRequest::Print() const
{
  LogMsg(LogPrioInfo, "TestRequest::print %s:%u",m_data.aMessage,m_data.repCounter);
}

TestClient::TestClient(ICCM & ccmIf) : m_ccmIf{ccmIf}
{

}




void TestClient::IssueRequest()
{
  std::shared_ptr<TestRequest> pTr = std::make_shared<TestRequest>();
  if(pTr)
  {
    ClientMessage req{pTr,TestClientServiceId,m_ccmIf.CreateUniqueId()};
    const bool isReqSuccessfullyReplicatated = m_ccmIf.ReplicateRequest(req);
    if(isReqSuccessfullyReplicatated)
    {
      LogMsg(LogPrioInfo, "TestClient::IssueRequest successfully replicated request");
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR: TestClient::IssueRequest failed to replicate request");
    }
  }
}
    
void TestClient::RegisterForUpcall()
{
  m_ccmIf.RegisterService(TestClientServiceId, std::bind(&TestClient::UpcallMethod,this, std::placeholders::_1));
}

void TestClient::UpcallMethod(const ClientMessage& commitedMsg)
{
  unsigned int payloadSize = 0U;
  LogMsg(LogPrioInfo,"TestClient::UpcallMethod");
  
  void *pPayloadData = commitedMsg.GetDeserializedPayload(payloadSize);
  
  if(pPayloadData != nullptr && payloadSize == sizeof(TestRequest))
  {
    TestRequest* pTr = (TestRequest*)pPayloadData; 
    pTr->Print();
  }
  else
  {
    LogMsg(LogPrioError,"TestClient::UpcallMethod invalid payload or size. 0x%x %u %u ",pPayloadData,payloadSize,sizeof(TestRequest));
  } 
}
