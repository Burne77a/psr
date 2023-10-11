#ifndef CCM_MEMBER_H
#define CCM_MEMBER_H

#include <string>
#include <bitset>
#include <chrono>

const size_t MAX_MEMBERS = 64; 

class Member {
  public:
    Member(const int id, const std::string& name, const std::string& ip);
    
    void AddConnection(int memberId);
    int ConnectionCount() const;
    std::bitset<MAX_MEMBERS> GetConnections() const;
    
    int GetID() const;
    const std::string& GetName() const;
    const std::string& GetIP() const;
    
    void UpdateHeartbeat();
    std::chrono::system_clock::time_point GetLastHeartbeat() const;
    bool HasHeartbeatExceeded(const std::chrono::milliseconds& duration) const;

private:
    int m_id;
    std::string m_name;
    std::string m_ip;
    std::bitset<MAX_MEMBERS> m_connections;
    std::chrono::system_clock::time_point m_lastHeartbeat;
};

#endif // CCM_MEMBER_H
