#include "CSA.h"
#include "Logger.h"
#include "ErrorCodes.h"
#include <errnoLib.h>


std::unique_ptr<CSA> CSA::CreateCSA(const int myId)
{
  static const int CLIENT_REQ_PORT = 7777;
  std::shared_ptr<CCM> pCmm = CCM::CreateAndInitForTest(myId);
  if(!pCmm)
  {
    LogMsg(LogPrioCritical, "ERROR:  CSA::CreateCSA CreateAndInitForTest failed. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
    
  std::unique_ptr<ServiceUpcallDispatcher> pSrvDispatcher = std::make_unique<ServiceUpcallDispatcher>();
  if(!pSrvDispatcher)
  {
    LogMsg(LogPrioCritical, "ERROR: CSA::CreateCSA failed to create ServiceUpcallDispatcher. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<LeaderCommunicator> pLeaderComm = LeaderCommunicator::CreateLeaderCommunicator(CCM::GetLeaderIp(),CLIENT_REQ_PORT);
  if(!pLeaderComm)
  {
    LogMsg(LogPrioCritical, "ERROR: CSA::CreateCSA failed to create LeaderCommunicator. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<CSA> pCSA = std::make_unique<CSA>(pCmm,pSrvDispatcher,pLeaderComm);
  if(!pCSA)
  {
    LogMsg(LogPrioCritical, "ERROR: CSA::CreateCSA failed to create CSA. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  return pCSA;
}


CSA::CSA(std::shared_ptr<CCM> &pCmm, std::unique_ptr<ServiceUpcallDispatcher> & pSrvDispatcher,std::unique_ptr<LeaderCommunicator>& pLeaderComm) :
    m_pCcm{pCmm}, m_pSrvDispatcher{std::move(pSrvDispatcher)},m_pLeaderComm{std::move(pLeaderComm)}
{
  
}

const OSAStatusCode CSA::Start()
{
  const OSAStatusCode startSts = m_pCcm->Start();
  if(startSts != OSA_OK)
  {
    LogMsg(LogPrioCritical, "ERROR: CSA::Start failed. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  return startSts;
}

bool CSA::ReplicateRequest(const ClientMessage & msg)
{
  if(!msg.GetReqId().IsValid())
  {
    LogMsg(LogPrioError, "ERROR: CSA::ReplicateRequest failed - msg do not contain a valid request ID.");
    return false;
  }
  if(!SendToLeaderAndWaitForReply(msg))
  {
    LogMsg(LogPrioError, "ERROR: CSA::ReplicateRequest failed - failed to send ClientMessage to leader");
    return false;
  }
  //Wait for the response
  
  return true;
}


ClientRequestId CSA::CreateUniqueId()
{
  std::lock_guard<std::mutex> lock(m_mutex); 
  static unsigned int reqNumber = ClientRequestId::LowestValidReqId;
  
  reqNumber++;
  ClientRequestId newReq{reqNumber,static_cast<unsigned int>(m_pCcm->GetMyId())};
  return newReq;
}

bool CSA::SendToLeaderAndWaitForReply(const ClientMessage &msg)
{
  static const unsigned int RETRIES = 10U;
  static const unsigned int WAIT_BETWEEN_RETRIES = 2000U;
  const bool isSuccessfullySent = m_pLeaderComm->SendToLeaderWithRetries(msg, RETRIES, WAIT_BETWEEN_RETRIES);
  
  if(!isSuccessfullySent)
  {
    LogMsg(LogPrioError, "ERROR: CSA::SendToLeaderAndWaitForReply failed - failed to send ClientMessage to leader");
    msg.Print();
    return false;
  }
  
  
  
  
  return true;
}


void CSA::Print() const
{
  m_pCcm->Print();
  m_pSrvDispatcher->Print();
}


