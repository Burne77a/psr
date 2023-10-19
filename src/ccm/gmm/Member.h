#ifndef CCM_MEMBER_H
#define CCM_MEMBER_H

#include <string>
#include <bitset>
#include <chrono>

const size_t MAX_MEMBERS = 64; 
const int INVALID_LEADER_ID = 0U;

class Member {
  public:
    Member(const int id, const std::string& name, const std::string& ip);
    //Connection related
    void AddConnection(const int memberId);
    void RemoveConnection(const int memberId);
    int ConnectionCount() const;
    std::bitset<MAX_MEMBERS> GetConnections() const;
    void SetConnectionPerception(const std::bitset<MAX_MEMBERS> &connectionPerception);
    //Leader related
    int GetLeaderId() const {return m_leaderId;}
    void ClearLeaderId(){m_leaderId = INVALID_LEADER_ID;}
    void IncrementVoteCnt() {m_leaderVoteCnt++;}
    void ResetVoteCnt(){m_leaderVoteCnt = 0U;}
    unsigned int GetLeaderVoteCnt(){return m_leaderVoteCnt;}
    void SetViewNumber(const unsigned int viewNumber){m_viewNumber = viewNumber;}
    unsigned int GetViewNumber() const {return m_viewNumber;}
    //Member related
    int GetID() const;
    const std::string& GetName() const;
    const std::string& GetIP() const;
    
    void UpdateHeartbeat(const int leaderId);
    std::chrono::system_clock::time_point GetLastHeartbeat() const;
    bool HasHeartbeatExceeded(const std::chrono::milliseconds& duration) const;
    void Print() const;

private:
    int m_id;
    std::string m_name;
    std::string m_ip;
    std::bitset<MAX_MEMBERS> m_connections;
    std::chrono::system_clock::time_point m_lastHeartbeat;
    int m_leaderId{INVALID_LEADER_ID};
    unsigned int m_viewNumber{0U};
    unsigned int m_leaderVoteCnt{0U};
};

#endif // CCM_MEMBER_H
