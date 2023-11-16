#ifndef AIR_STORAGEINFO_H
#define AIR_STORAGEINFO_H
#include "ISerializable.h"
#include <memory>
#include <string>
class StorageInfo : public ISerializable
{
  public:
    static const unsigned int INVALID_VALUE{0U};
    static const unsigned int INVALID_APP_ID{INVALID_VALUE};
  private:
    struct StorageInfoData
    {
     unsigned int nodeId{INVALID_VALUE};
     unsigned int storageId{INVALID_APP_ID};
     char ipAddr[20];
     unsigned int sizeInBytes{INVALID_VALUE};
     unsigned int bandwidth{0U}; 
    };
  public: 
    static std::shared_ptr<StorageInfo> CreateFromRawPtr(const uint8_t * pBuffer, const uint32_t size);
    StorageInfo(const unsigned int storageId, std::string_view ipAddr, const unsigned int sizeInBytes, const unsigned int bandwidth,const unsigned int nodeId);
    StorageInfo(const unsigned int storageId);
    StorageInfo();
    ~StorageInfo() = default;
     unsigned int GetId() const {return m_data.storageId;}
     unsigned int GetNodeId() const {return m_data.nodeId;}
     std::string GetIpAddr() const {return std::string{m_data.ipAddr};}
     unsigned int GetSizeInBytes() const {return m_data.sizeInBytes;}
     unsigned int GetBandwidth() const {return m_data.bandwidth;}
    //ISerializable
     const uint8_t *  Serialize(uint32_t &size) const override;
     const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
     bool Deserialize() override;
     void Print() const; 
  private:
     StorageInfoData m_data;
};

#endif //AIR_STORAGEINFO_H
