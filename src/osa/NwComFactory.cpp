#include "NWComFactory.h"
#include "snd/UDPMCastSend.h"
#include "snd/UDPUniCastSend.h"
#include "rcv/UDPMCastRcv.h"
#include "rcv/UDPUniCastRcv.h"



std::unique_ptr<ISender> NwCComFactory::CreateSender(const Type type) 
{
	switch (type) 
	{
		case Type::UDP_MULTICAST:
			return std::make_unique<UDPMulticastSender>();
		case Type::UDP_UNICAST:
			return std::make_unique<UDPUnicastSender>();
		// handle other types as necessary
		default:
			throw std::runtime_error("Invalid type");
	}
}

std::unique_ptr<IReceiver> NwCComFactory::CreateReceiver(const Type type) 
{
	switch (type) 
	{
		case Type::UDP_MULTICAST:
			return std::make_unique<UDPMulticastReceiver>();
		case Type::UDP_UNICAST:
			return std::make_unique<UDPUnicastReceiver>();
		// handle other types as necessary
		default:
			throw std::runtime_error("Invalid type");
	}
}
