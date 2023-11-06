#include "CSA.h"
#include "Logger.h"
#include "ErrorCodes.h"
#include <errnoLib.h>


std::unique_ptr<CSA> CSA::CreateCSA(const std::string_view leaderIp, GMM& gmm)
{
  static const int LEADER_PORT = 7777;
  static const int CLIENT_PORT = 7778;
      
  std::unique_ptr<ServiceUpcallDispatcher> pSrvDispatcher = std::make_unique<ServiceUpcallDispatcher>();
  if(!pSrvDispatcher)
  {
    LogMsg(LogPrioCritical, "ERROR: CSA::CreateCSA failed to create ServiceUpcallDispatcher. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<LeaderCommunicator> pLeaderComm = LeaderCommunicator::CreateLeaderCommunicator(leaderIp,LEADER_PORT,gmm,CLIENT_PORT);
  if(!pLeaderComm)
  {
    LogMsg(LogPrioCritical, "ERROR: CSA::CreateCSA failed to create LeaderCommunicator. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<CSA> pCSA = std::make_unique<CSA>(gmm,pSrvDispatcher,pLeaderComm);
  if(!pCSA)
  {
    LogMsg(LogPrioCritical, "ERROR: CSA::CreateCSA failed to create CSA. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  return pCSA;
}


CSA::CSA(GMM& gmm, std::unique_ptr<ServiceUpcallDispatcher> & pSrvDispatcher,std::unique_ptr<LeaderCommunicator>& pLeaderComm) :
    m_gmm{gmm}, m_pSrvDispatcher{std::move(pSrvDispatcher)},m_pLeaderComm{std::move(pLeaderComm)}
{
  
}


bool CSA::GetClientRequestsSentToLeader(ClientMessage & msg)
{
  const bool isReqRcvd = m_pLeaderComm->GetClientRequestsSentToLeader(msg);
  return isReqRcvd;
}

bool CSA::SendReplyToClient(ClientMessage & msg)
{
  return m_pLeaderComm->SendToClient(msg,m_gmm);
}

bool CSA::ReplicateRequest(const ClientMessage & msg)
{
  std::lock_guard<std::mutex> lock(m_mutex); 
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
  
  return true;
}


ClientRequestId CSA::CreateUniqueId()
{
  std::lock_guard<std::mutex> lock(m_mutex); 
  static unsigned int reqNumber = ClientRequestId::LowestValidReqId;
  
  reqNumber++;
  ClientRequestId newReq{reqNumber,static_cast<unsigned int>(m_gmm.GetMyId())};
  return newReq;
}

bool CSA::SendToLeaderAndWaitForReply(const ClientMessage &msg)
{
  static const unsigned int RETRIES = 10U;
  static const unsigned int WAIT_BETWEEN_RETRIES = 2000U;
  static const unsigned int TIME_TO_WAIT_ON_REPLY = 5000U;
  const bool isSuccessfullySent = m_pLeaderComm->SendToLeaderWithRetries(msg, RETRIES, WAIT_BETWEEN_RETRIES);
  
  if(!isSuccessfullySent)
  {
    LogMsg(LogPrioError, "ERROR: CSA::SendToLeaderAndWaitForReply failed - failed to send ClientMessage to leader");
    msg.Print();
    return false;
  }
  const bool isValidReplyRcvd = m_pLeaderComm->WaitForCommitAckToRequest(msg, TIME_TO_WAIT_ON_REPLY);
  if(!isValidReplyRcvd)
  {
    LogMsg(LogPrioError, "ERROR: CSA::SendToLeaderAndWaitForReply failed - no valid reply received within timeout time");
    msg.Print();
  }
    
  return isValidReplyRcvd;
}


void CSA::Print() const
{
  LogMsg(LogPrioInfo, "--- >CSA< ---");
  m_pSrvDispatcher->Print();
  LogMsg(LogPrioInfo, "--- <CSA> ---");
}


