#ifndef CCM_GMM_H
#define CCM_GMM_H

#include "Member.h"
#include <unordered_map>
#include <mutex>

class GMM {
  public:
    void AddMember(const int id, const std::string& name, const std::string& ip);
    Member* GetMember(const int id);
    void EstablishConnection(const int id1, const int id2);
    bool IsQuorumConnected(const int id);
    void UpdateMemberHeartbeat(const int id);
    std::chrono::system_clock::time_point GetMemberLastHeartbeat(const int id);
    bool HasMemberHeartbeatExceeded(const int id, const std::chrono::milliseconds& duration);
    void ForEachMember(const std::function<void(const int, Member&)>& func);

  private:
    std::unordered_map<int, Member> m_members;
    mutable std::mutex m_mutex;
};

#endif // CCM_GMM_H
