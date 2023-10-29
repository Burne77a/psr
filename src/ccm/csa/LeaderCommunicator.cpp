#include "LeaderCommunicator.h"
#include "NwAid.h"
#include "Logger.h"
#include "TaskAbstraction.h"
#include <errnoLib.h>

std::unique_ptr<LeaderCommunicator> LeaderCommunicator::CreateLeaderCommunicator(const std::string_view leaderIpAddress, const int port)
{
  std::unique_ptr<IReceiver> pRcv = NwAid::CreateUniCastReceiver(port);
  if(!pRcv)
  {
    LogMsg(LogPrioCritical, "ERROR: LeaderCommunicator::CreateLeaderCommunicator failed to create receiver. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<ISender> pSender = NwAid::CreateUniCastSender(leaderIpAddress, port);
  if(!pSender)
  {
    LogMsg(LogPrioCritical, "ERROR: LeaderCommunicator::CreateLeaderCommunicator failed to create sender. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<LeaderCommunicator> pLc = std::make_unique<LeaderCommunicator>(pSender,pRcv);
  if(!pLc)
  {
    LogMsg(LogPrioCritical, "ERROR: LeaderCommunicator::CreateLeaderCommunicator failed to create LeaderCommunicator. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  return pLc;
}

LeaderCommunicator::LeaderCommunicator(std::unique_ptr<ISender>& pSender, std::unique_ptr<IReceiver>&  pReceiver) : 
    m_pSender(std::move(pSender)), m_pReceiver(std::move(pReceiver))
{
  
}

bool LeaderCommunicator::SendToLeaderWithRetries(const ClientMessage &msg, const unsigned int retries, const unsigned int timeBetweenInMs)
{
  bool isSuccessfullySent = false;
  unsigned int retryCnt = 0U;
  do
  {
    isSuccessfullySent = SendToLeader(msg);
    if(!isSuccessfullySent)
    {
      retryCnt++;
      OSATaskSleep(timeBetweenInMs);
    }
    
  }while(!isSuccessfullySent && (retryCnt < retries));
  
  if(!isSuccessfullySent)
  {
    LogMsg(LogPrioError, "ERROR: SendToLeaderWithRetries::SendToLeader failed to send message to leader after %u retries. Errno: 0x%x (%s)",retries,errnoGet(),strerror(errnoGet()));
  }
  
  return isSuccessfullySent;
}

bool LeaderCommunicator::SendToLeader(const ClientMessage &msg)
{
  bool isSentSuccessfully = false;
  isSentSuccessfully = m_pSender->Send(msg);
  if(!isSentSuccessfully)
  {
    LogMsg(LogPrioError, "ERROR: LeaderCommunicator::SendToLeader failed to send message to leader. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
  return isSentSuccessfully;
}
