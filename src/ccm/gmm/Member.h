#ifndef CCM_MEMBER_H
#define CCM_MEMBER_H

#include <string>
#include <bitset>
#include <chrono>

const size_t MAX_MEMBERS = 64; 
const int INVALID_ID = -1;
const int INVALID_LEADER_ID = 0U;
const unsigned int INVALID_REQ_ID = 0U;

class Member {
  public:
    Member(const int id, const std::string& name, const std::string& ip);
    //Connection related
    void AddConnection(const int memberId);
    void RemoveConnection(const int memberId);
    int ConnectionCount() const;
    std::bitset<MAX_MEMBERS> GetConnections() const;
    void SetConnectionPerception(const std::bitset<MAX_MEMBERS> &connectionPerception);
    void ResetConnectionPerception();
    //Leader related
    int GetLeaderId() const {return m_leaderId;}
    void SetLeaderId(const int id) {m_leaderId = id;}
    void ClearLeaderId(){m_leaderId = INVALID_LEADER_ID;}
    void IncrementVoteCnt() {m_leaderVoteCnt++;}
    void ResetVoteCnt(){m_leaderVoteCnt = 0U;}
    unsigned int GetLeaderVoteCnt(){return m_leaderVoteCnt;}
    //Current view and operation perception
    void SetViewNumber(const unsigned int viewNumber){m_viewNumber = viewNumber;}
    unsigned int GetViewNumber() const {return m_viewNumber;}
    unsigned int GetOpNumber() const {return m_operationNumber;}
    void SetOperationNumber(const unsigned int opNumber) {m_operationNumber = opNumber;}
    unsigned int GetCommittedOpNumber() const {return m_committedOpNumber;}
    void SetCommittedOperationNumber(const unsigned int committedOpNumber) {m_committedOpNumber = committedOpNumber;}
    
    //Pending request
    void ClearPendingPrepare() {m_pendingPrepeareViewNumber = INVALID_REQ_ID; m_pedingPrepareOpNumber = INVALID_REQ_ID; m_isValidPrepareOkRcvd = false;}
    void SetPendingPrepare(const unsigned int viewNumber, const unsigned int opNumber) { m_pendingPrepeareViewNumber = viewNumber; m_pedingPrepareOpNumber = opNumber; m_isValidPrepareOkRcvd = false;}
    bool IsValidPrepareOkRcvd() const {return m_isValidPrepareOkRcvd;}
    void SetPrepareOkRcvdIfMatchPending(const unsigned int viewNumber, const unsigned int opNumber);
      
    
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
    unsigned int m_operationNumber{0U};
    unsigned int m_committedOpNumber{0U};
    
    unsigned int m_pendingPrepeareViewNumber{INVALID_REQ_ID};
    unsigned int m_pedingPrepareOpNumber{INVALID_REQ_ID};
    bool m_isValidPrepareOkRcvd{false};
};

#endif // CCM_MEMBER_H
