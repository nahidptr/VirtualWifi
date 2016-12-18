﻿/*
*  Copyright © 2015 Nahid.
*  Last Updated : 23 April 15
*/

#pragma once
#include <exception>

inline void HR(int retCode) 
{
	if (retCode != 0)
	{
		throw std::exception();
	}
}

enum WLAN_NOTIFICATION_SOURCE : unsigned
{
	None = 0,
	// All notifications, including those generated by the 802.1X module.
	All = 0X0000FFFF,
	// Notifications generated by the auto configuration module.
	ACM = 0X00000008,
	// Notifications generated by MSM.
	MSM = 0X00000010,
	// Notifications generated by the security module.
	Security = 0X00000020,
	// Notifications generated by independent hardware vendors (IHV).
	IHV = 0X00000040
};