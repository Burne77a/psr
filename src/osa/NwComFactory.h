#ifndef OSFUNCTIONS_NW_COM_FACTORY_H_
#define OSFUNCTIONS_NW_COM_FACTORY_H_

#include "IReceiver.h"
#include "ISender.h"
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
	static std::unique_ptr<ISender> CreateReceiver(const Type type);
};

#endif //OSFUNCTIONS_NW_COM_FACTORY_H_
