#ifndef CCM_GMM_H
#define CCM_GMM_H

#include "Member.h"
#include "Logger.h"
#include "../fd/HeartbeatCCM.h"
#include <unordered_map>
#include <mutex>

class GMM {
  public:
    GMM(const int myId);
    void AddMember(const int id, const std::string& name, const std::string& ip);
    void EstablishConnection(const int id1, const int id2);
    bool GetLowestQuorumConnectedId(int &id);
    bool IsQuorumConnected(const int id);
    bool IsAnyMemberQuorumConnected();
    void UpdateMemberHeartbeat(const HeartbeatCCM & hb);
    std::chrono::system_clock::time_point GetMemberLastHeartbeat(const int id);
    bool HasMemberHeartbeatExceeded(const int id, const std::chrono::milliseconds& duration);
    void UpdateConnectionStatusForMySelf(const std::chrono::milliseconds& duration);
    void SetMySelfToLeader();
    void RemoveMySelfAsLeader();
    
    
    int GetLeaderId(void);
    std::string GetLeaderIp();
    bool IsLeaderAvailable(void) {return (GetLeaderId() != INVALID_LEADER_ID);}
    void ResetLeaderVoteCount();
    void AddLeaderVote(const int id);
    void SetViewNumber(const int id, const unsigned int viewNumber);
    const unsigned int GetViewNumber(const int id);
    void SetMyViewNumber(const unsigned int viewNumber);
    const unsigned int GetMyViewNumber();
    const unsigned int GetLargestViewNumber();
    bool IsVoteCntForMySelfLargerThanMajority();
    
    void ForEachMember(const std::function<void(const int, Member&)>& func);
    void ForMyMember(const std::function<void(const int, Member&)>& func);
    void ForIdMember(const int id, const std::function<void(const int, Member&)>& func);
    const int GetMyId() const {return m_myId;}
    void Print() const;

  private:
    const int m_myId;
    bool IsQuorumConnectedNoLock(const int id) const;
    Member* GetMember(const int id);
    std::unordered_map<int, Member> m_members;
    mutable std::mutex m_mutex;
};

#endif // CCM_GMM_H
