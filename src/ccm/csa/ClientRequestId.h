#ifndef CCM_CLIENTREQUESTID_H
#define CCM_CLIENTREQUESTID_H
#include <string>
class ClientRequestId
{
  private:
    static constexpr unsigned int Invalid = 0U;
  public:
    ClientRequestId() :  m_reqSeq{Invalid}, m_clientId{Invalid} {}
    ClientRequestId(const unsigned int reqSeq, const unsigned int clientId) : m_reqSeq{reqSeq}, m_clientId{clientId} {}
    bool IsValid() const {return ((m_reqSeq != Invalid) && (m_clientId != Invalid));}
    std::string GetIdAsStr() const {return std::string{std::to_string(m_clientId) + ":" + std::to_string(m_reqSeq)};}
    static constexpr unsigned int LowestValidReqId = Invalid + 1U;
  private: 
    unsigned int m_reqSeq;
    unsigned int m_clientId;
   
};

#endif //CCM_CLIENTREQUESTID_H
