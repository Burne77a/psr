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
    unsigned int GetId() const {return m_clientId;}
    
    bool operator==(ClientRequestId const& rhs) const
    {
      bool isValid = IsValid() && rhs.IsValid();
      bool isEqual = false;
      if(isValid)
      {
        isEqual = (m_reqSeq == rhs.m_reqSeq) && (m_clientId == rhs.m_clientId);
      }
      return isEqual;
    }
    
    std::string GetIdAsStr() const {return std::string{std::to_string(m_clientId) + ":" + std::to_string(m_reqSeq)};}
    static constexpr unsigned int LowestValidReqId = Invalid + 1U;
  private: 
    unsigned int m_reqSeq;
    unsigned int m_clientId;
   
};

#endif //CCM_CLIENTREQUESTID_H
