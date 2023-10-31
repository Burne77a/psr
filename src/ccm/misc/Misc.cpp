#include "Misc.h"
#include "Logger.h"
#include "NwAid.h"
#include <errnoLib.h>

bool Misc::CreateISendersFromMembers(const int port, GMM &gmm, std::vector<std::unique_ptr<ISender>> &senders)
{
  bool isAllCreatedOk = true;
  const int myId = gmm.GetMyId();
  gmm.ForEachMember([myId,&isAllCreatedOk,&senders,&gmm,port](int id, Member& member)
    {
     if(myId != member.GetID())
     {
       std::unique_ptr<ISender> pSender = NwAid::CreateUniCastSender(member.GetIP(), port);
       if(pSender)
       {
         senders.push_back(std::move(pSender));
       }
       else
       {
         LogMsg(LogPrioCritical, "ERROR Misc::CreateISendersFromMembers failed to create sender. %s %d Errno: 0x%x (%s)",member.GetIP().c_str(),myId,errnoGet(),strerror(errnoGet()));
         isAllCreatedOk = false;
       }
     }
    });
  return isAllCreatedOk;
}   

void Misc::SendToAllMembers(const ISerializable & objToSend, const GMM &gmm, const std::vector<std::unique_ptr<ISender>> &senders)
{
  for(auto &pSender : senders)
  {
    if(pSender)
    {
      if(!pSender->Send(objToSend))
      {
        LogMsg(LogPrioCritical, "ERROR Misc::SendToAllMembers failed to send to %s  Errno: 0x%x (%s)",pSender->GetIpAddr().c_str() ,errnoGet(),strerror(errnoGet()));
      }
    }
  }
}

bool Misc::SendToIp(const ISerializable & objToSend, std::string_view ipAddr, const std::vector<std::unique_ptr<ISender>> &senders)
{
  bool isSuccessfullySent = false;;
  bool isIpFound = false;
  for(auto &pSender : senders)
  {
    if(pSender)
    {
      if(pSender->GetIpAddr().compare(ipAddr) == 0)
      {
        isIpFound = true;
        isSuccessfullySent = pSender->Send(objToSend);
        
      }
    }
  }
  if(!isSuccessfullySent)
  {
    LogMsg(LogPrioCritical, "ERROR Misc::SendToIp failed to send to %s  Errno: 0x%x (%s) %s ",ipAddr.data() ,errnoGet(),strerror(errnoGet()), isIpFound ? "IP found" : "IP not found");
  }
  return isSuccessfullySent;
}

