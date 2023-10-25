#ifndef CCM_LR_LOGENTRY_H
#define CCM_LR_LOGENTRY_H
#include "LogReplicationMsg.h"
#include <memory>

class LogEntry 
{
  public:
    static std::unique_ptr<LogEntry> CreateEntry(const LogReplicationMsg msg);
    LogEntry(const unsigned int opNumber, const unsigned int viewNumber, const unsigned int rid);
    ~LogEntry() = default;
    bool IsValidNextOpAndViewNumber(const unsigned int opNumber, const unsigned int viewNumber);
    unsigned int GetOpNumber() const {return m_opNumber;}
    unsigned int GetViewNumber() const {return m_viewNumber;}
    unsigned int GetReqId() const{return m_rid;}
    void SetCommitted() {m_isCommited = true;}
    void SetUpCallDone() {m_isUpcallDone = true;}
    bool IsCommited() const {return m_isCommited;}
    bool IsUpcallDone() const {return m_isUpcallDone;}
  
  void Print() const;
  private:
    const unsigned int m_opNumber;
    const unsigned int m_viewNumber;
    const unsigned int m_rid;
    bool m_isCommited{false};
    bool m_isUpcallDone{false};
    
};

#endif //CCM_LR_LOGENTRY_H
