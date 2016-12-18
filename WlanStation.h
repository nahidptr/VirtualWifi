#pragma once
#include <wlanapi.h>
#include <string>
#include <sstream>

class WlanStation
{
	template< typename T =  unsigned char >
	std::string int_to_hex(T i)
	{
		std::stringstream stream;
		
		stream << std::hex << ( (i & 0xF0) >> 4);
		stream << std::hex << (i & 0x0F);

		return stream.str();
	}

	std::string ConvertToString(DOT11_MAC_ADDRESS mac)
	{
		std::stringstream ss;

		for (auto it = 0u; it != 6; ++it)
		{
			ss << int_to_hex(mac[it]);
			if (it == 5) continue;
			ss << ":";
		}
		
		return ss.str();
	}

public:

	WlanStation() {}

	explicit WlanStation(WLAN_HOSTED_NETWORK_PEER_STATE state)
	{
		this->State = state;
	}

	WLAN_HOSTED_NETWORK_PEER_STATE State;


	std::string MacAddress()
	{
		return ConvertToString(this->State.PeerMacAddress);
	}
};