#ifndef CCM_CLIENTREQUESTID_H
#define CCM_CLIENTREQUESTID_H
class ClientRequestId
{
  private:
    static constexpr unsigned int Invalid = 0U;
  public:
    ClientRequestId() :  m_reqSeq{Invalid}, m_clientId{Invalid} {}
    ClientRequestId(const unsigned int reqSeq, const unsigned int clientId) : m_reqSeq{reqSeq}, m_clientId{clientId} {}
    bool IsValid(){return ((m_reqSeq != Invalid) && (m_clientId != Invalid));}
    static constexpr unsigned int LowestValidReqId = Invalid + 1U;
  private: 
    unsigned int m_reqSeq;
    unsigned int m_clientId;
   
};

#endif //CCM_CLIENTREQUESTID_H
