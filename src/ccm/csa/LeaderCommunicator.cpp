#include "LeaderCommunicator.h"
#include "NwAid.h"
#include "Logger.h"
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

bool LeaderCommunicator::SendToLeader(const ClientMessage &msg)
{
  bool isSentSuccessfully = false;
  m_pSender->Send(objToSend)
}
