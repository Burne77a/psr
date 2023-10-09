
#include <memory>
class NwCComFactory
{
public: 
  enum class Type 
  {
		UDP_MULTICAST,
		UDP_UNICAST
	};
	static std::unique_ptr<ISender> CreateSender(const Type type);
};
