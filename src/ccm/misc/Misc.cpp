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