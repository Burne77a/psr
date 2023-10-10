#ifndef CCM_GMM_H
#define CCM_GMM_H

#include "Member.h"
#include <unordered_map>

class GMM {
  public:
    void AddMember(const int id, const std::string& name, const std::string& ip);
    Member* GetMember(const int id);
    void EstablishConnection(const int id1, const int id2);
    bool IsQuorumConnected(const int id);

private:
  std::unordered_map<int, Member> m_members;
};

#endif // CCM_GMM_H
