#ifndef CCM_MISC_H
#define CCM_MISC_H
#include "../gmm/GMM.h"
#include "ISender.h"
#include <memory>
#include <vector>


class Misc
{
  public:
    static bool CreateISendersFromMembers(const int port, GMM &gmm, std::vector<std::unique_ptr<ISender>> &senders);
    static void SendToAllMembers(const ISerializable & objToSend, const GMM &gmm, const std::vector<std::unique_ptr<ISender>> &senders);
};


#endif //CCM_MISC_H
