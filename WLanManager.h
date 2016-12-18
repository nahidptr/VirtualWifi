/*
*  Copyright © 2015 Nahid.
*  Last Updated : 23 April 15
*/

#pragma once

#include <Windows.h>
#include <Wlanapi.h>
#include <map>
#include "WlanStation.h"
#include <boost/signals2.hpp>
#include "WlanUtil.h"

class WLanManager
{
	HANDLE											m_WlanHandle;
	DWORD											m_ServerVersion;
	WLAN_NOTIFICATION_CALLBACK						m_notificationCallback;
	WLAN_HOSTED_NETWORK_STATE						m_HostedNetworkState;
	std::map< const std::string, WlanStation>		m_Stations;

	using  notificationCallback = void (WINAPI*)(WLAN_NOTIFICATION_DATA notificationData, PVOID context);

	void Init()
	{
		try
		{
			 HR(WlanOpenHandle(2, nullptr, &m_ServerVersion, &m_WlanHandle));

			 //WLAN_NOTIFICATION_SOURCE notifSource;
			 DWORD notifS;
			 HR(WlanRegisterNotification(m_WlanHandle, WLAN_NOTIFICATION_SOURCE::All, true, m_notificationCallback, this, nullptr, &notifS));

			 WLAN_HOSTED_NETWORK_REASON failReason = this->InitSettings();
			 if (failReason != WLAN_HOSTED_NETWORK_REASON::wlan_hosted_network_reason_success)
			 {
				 throw std::exception("Init Error WlanHostedNetworkInitSettings: " + failReason);
			 }

		 }
		 catch(const std::exception& ex)
		 {
			 WlanCloseHandle(this->m_WlanHandle, nullptr);
		 }
		 catch(...)
		 {
			 WlanCloseHandle(this->m_WlanHandle, nullptr);
		 }
	 }

protected:

	void onHostedNetworkStarted()
	{
		this->m_HostedNetworkState = WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_active;
	
		this->HostedNetworkStarted(this, nullptr);
	}

	void onHostedNetworkStopped()
	{
		this->m_HostedNetworkState = WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_idle;

		//if (this->HostedNetworkStopped != nullptr)
		//{
			//this->HostedNetworkStopped(this, nullptr);
		//}
	}

	void onHostedNetworkAvailable()
	{
		this->m_HostedNetworkState = WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_idle;
		this->HostedNetworkAvailable(this, nullptr);
	}

	void onStationJoin(WLAN_HOSTED_NETWORK_PEER_STATE stationState)
	{
		auto pStation = WlanStation(stationState);

		this->m_Stations[pStation.MacAddress()] = pStation;
		this->StationJoin(this, nullptr);
	}

	void onStationLeave(WLAN_HOSTED_NETWORK_PEER_STATE stationState)
	{
		auto pStation = WlanStation(stationState);
		auto strMacToDel = pStation.MacAddress();
		auto debugVal = this->m_Stations.erase(strMacToDel);

		this->StationLeave(this, nullptr);
	}

	void onStationStateChange(WLAN_HOSTED_NETWORK_PEER_STATE stationState)
	{
		this->StationStateChange(this, nullptr);
	}


	static void CALLBACK OnNotificationStatic(PWLAN_NOTIFICATION_DATA notifData, PVOID context) //WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		WLanManager *pThis = nullptr;

		 pThis = reinterpret_cast<WLanManager*>(context);

		if (pThis)
		{
			return pThis->OnNotification(notifData, context);
		}
	}

	void OnNotification(PWLAN_NOTIFICATION_DATA notifData, PVOID context)
	{
		switch (notifData->NotificationCode)
		{

		case static_cast<int>(WLAN_HOSTED_NETWORK_NOTIFICATION_CODE::wlan_hosted_network_state_change):

			if (notifData->dwDataSize > 0 && notifData->pData != nullptr)
			{
				WLAN_HOSTED_NETWORK_STATE_CHANGE pStateChange =  *(WLAN_HOSTED_NETWORK_STATE_CHANGE *)(notifData->pData);
					//(WLAN_HOSTED_NETWORK_STATE_CHANGE)Marshal.PtrToStructure(notifData.dataPtr, typeof(WLAN_HOSTED_NETWORK_STATE_CHANGE));

				switch (pStateChange.NewState)
				{
				case WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_active:
					this->onHostedNetworkStarted();
					break;

				case WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_idle:
					if (pStateChange.OldState == WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_active)
					{
						this->onHostedNetworkStopped();
					}
					else
					{
						this->onHostedNetworkAvailable();
					}
					break;

				case WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_unavailable:
					if (pStateChange.OldState == WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_active)
					{
						this->onHostedNetworkStopped();
					}
					this->onHostedNetworkAvailable();
					break;
				}
			}

			break;

		case static_cast<int>(WLAN_HOSTED_NETWORK_NOTIFICATION_CODE::wlan_hosted_network_peer_state_change):

			if (notifData->dwDataSize > 0 && notifData->pData != nullptr)
			{
				WLAN_HOSTED_NETWORK_DATA_PEER_STATE_CHANGE pPeerStateChange = *(WLAN_HOSTED_NETWORK_DATA_PEER_STATE_CHANGE*)(notifData->pData);
					//(WLAN_HOSTED_NETWORK_DATA_PEER_STATE_CHANGE)Marshal.PtrToStructure(notifData.dataPtr, typeof(WLAN_HOSTED_NETWORK_DATA_PEER_STATE_CHANGE));

				if (pPeerStateChange.NewState.PeerAuthState == WLAN_HOSTED_NETWORK_PEER_AUTH_STATE::wlan_hosted_network_peer_state_authenticated)
				{
					// Station joined the hosted network
					this->onStationJoin(pPeerStateChange.NewState);
				}
				else if (pPeerStateChange.NewState.PeerAuthState == WLAN_HOSTED_NETWORK_PEER_AUTH_STATE::wlan_hosted_network_peer_state_invalid)
				{
					// Station left the hosted network
					this->onStationLeave(pPeerStateChange.NewState);
				}
				else
				{
					// Authentication state changed
					this->onStationStateChange(pPeerStateChange.NewState);
				}
			}

			break;

		case static_cast<int>(WLAN_HOSTED_NETWORK_NOTIFICATION_CODE::wlan_hosted_network_radio_state_change):
			if (notifData->dwDataSize > 0 && notifData->pData != nullptr)
			{
				// Do nothing for now
			}
			
			break;
		}
	}

	WLAN_HOSTED_NETWORK_REASON InitSettings()
	{
		WLAN_HOSTED_NETWORK_REASON failReason;
		HR(WlanHostedNetworkInitSettings(this->m_WlanHandle, &failReason, nullptr));
		return failReason;
	}


public:

	#if defined(funcPtr)
	EventHandler HostedNetworkStarted;
	EventHandler HostedNetworkStopped;
	EventHandler HostedNetworkAvailable;

	EventHandler StationJoin;
	EventHandler StationLeave;
	EventHandler StationStateChange;
	#endif 

	
	boost::signals2::signal<void(PVOID, PVOID)> HostedNetworkStarted;
	boost::signals2::signal<void(PVOID, PVOID)> HostedNetworkStopped;
	boost::signals2::signal<void(PVOID, PVOID)> HostedNetworkAvailable;

	boost::signals2::signal<void(PVOID, PVOID)> StationJoin;
	boost::signals2::signal<void(PVOID, PVOID)> StationLeave;
	boost::signals2::signal<void(PVOID, PVOID)> StationStateChange;

	
	 WLanManager()
	 {
		 m_notificationCallback = OnNotificationStatic;
		 this->Init();
	 }

	~WLanManager()
	{
		try
		{
			this->ForceStop();
		}
		catch (const std::exception&)
		{}

		if (this->m_WlanHandle != nullptr)
		{
			WlanCloseHandle(this->m_WlanHandle, nullptr);
		}
	}


	 WLAN_HOSTED_NETWORK_REASON ForceStart()
	 {
		 WLAN_HOSTED_NETWORK_REASON failReason;
		 HR(WlanHostedNetworkForceStart(this->m_WlanHandle, &failReason, nullptr));

		 this->m_HostedNetworkState = WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_active;

		 return failReason;
	 }

	 WLAN_HOSTED_NETWORK_REASON ForceStop()
	 {
		 WLAN_HOSTED_NETWORK_REASON failReason;
		 HR(WlanHostedNetworkForceStop(this->m_WlanHandle, &failReason, nullptr));
		 //(WlanHostedNetworkForceStop(this->m_WlanHandle, &failReason, nullptr));

		 this->m_HostedNetworkState = WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_idle;

		 return failReason;
	 }

	 WLAN_HOSTED_NETWORK_REASON StartUsing()
	 {
		 WLAN_HOSTED_NETWORK_REASON failReason;
		 HR(WlanHostedNetworkStartUsing(this->m_WlanHandle, &failReason, nullptr));

		 this->m_HostedNetworkState = WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_active;

		 return failReason;
	 }

	 WLAN_HOSTED_NETWORK_REASON StopUsing()
	 {
		 WLAN_HOSTED_NETWORK_REASON failReason;
		 HR(WlanHostedNetworkStopUsing(this->m_WlanHandle, &failReason, nullptr));

		 this->m_HostedNetworkState = WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_idle;

		 return failReason;
	 }

	
	 WLAN_HOSTED_NETWORK_REASON QuerySecondaryKey(std::string& passKey)
	 {
		
		 unsigned char*						pPasskey;
		 BOOL								isPassPhrase;
		 BOOL								isPersistent;
		 WLAN_HOSTED_NETWORK_REASON			failReason;
		 DWORD								keyLen;
		
		 HR(WlanHostedNetworkQuerySecondaryKey(this->m_WlanHandle, &keyLen, &pPasskey, &isPassPhrase, &isPersistent, &failReason, nullptr));
		 
		 passKey.append(reinterpret_cast<const char*>(pPasskey));
		 WlanFreeMemory(pPasskey);

		 return failReason;
	 }

	 WLAN_HOSTED_NETWORK_REASON SetSecondaryKey(std::string passKey)
	 {
		 WLAN_HOSTED_NETWORK_REASON failReason;
		 DWORD length = passKey.length() + 1;
		 PUCHAR keyData = reinterpret_cast<unsigned char*>(const_cast<char*>(passKey.c_str()));

		 HR(WlanHostedNetworkSetSecondaryKey(this->m_WlanHandle, /*(uint)(passKey.Length + 1)*/
											 length, keyData, true, true, &failReason, nullptr));

		 return failReason;
	 }

	 PWLAN_HOSTED_NETWORK_STATUS QueryStatus()
	 {
		 PWLAN_HOSTED_NETWORK_STATUS status;
		 HR(WlanHostedNetworkQueryStatus(this->m_WlanHandle, &status, nullptr));
		 return status;
	 }

	 DOT11_SSID  ConvertStringToDOT11_SSID(std::string ssid)
	 {

		DOT11_SSID temp;
		memcpy(temp.ucSSID, ssid.c_str(), ssid.length()+1);
		temp.uSSIDLength = ssid.length();
		
		return temp;
	}

	 WLAN_HOSTED_NETWORK_REASON SetConnectionSettings(std::string hostedNetworkSSID, int maxNumberOfPeers)
	 {
		 WLAN_HOSTED_NETWORK_REASON failReason;

		 WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS settings = WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS();
		 settings.hostedNetworkSSID = ConvertStringToDOT11_SSID(hostedNetworkSSID);
		 settings.dwMaxNumberOfPeers = maxNumberOfPeers;

		 HR(WlanHostedNetworkSetProperty(this->m_WlanHandle,
										 WLAN_HOSTED_NETWORK_OPCODE::wlan_hosted_network_opcode_connection_settings,
										 sizeof(settings), &settings, &failReason, nullptr));

		 return failReason;
	 }

	 WLAN_OPCODE_VALUE_TYPE QueryConnectionSettings(std::string& hostedNetworkSSID, int& maxNumberOfPeers)
	 {
		 DWORD dataSize;
		 PVOID dataPtr;
		 WLAN_OPCODE_VALUE_TYPE opcode;

		 HR(WlanHostedNetworkQueryProperty( this->m_WlanHandle,
			 WLAN_HOSTED_NETWORK_OPCODE::wlan_hosted_network_opcode_connection_settings,
			 &dataSize, &dataPtr, &opcode, nullptr)
			 );


		 auto settings = *(WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS*)dataPtr;
		 hostedNetworkSSID = std::string(reinterpret_cast<char*>(settings.hostedNetworkSSID.ucSSID));

		 maxNumberOfPeers = (int)settings.dwMaxNumberOfPeers;

		 return opcode;
	 }

	 void StartHostedNetwork()
	 {
		 try
		 {
			 this->ForceStop();

			 auto failReason = this->StartUsing();
			 if (failReason != WLAN_HOSTED_NETWORK_REASON::wlan_hosted_network_reason_success)
			 {
				 throw std::exception("Could Not Start Hosted Network!\n\n" + failReason);
			 }
		 }
		 catch(const std::exception& exp)
		 {
			 throw;
		 }
	 }

	 void StopHostedNetwork()
	 {
		 this->ForceStop();
	 }

	 auto GetStations()-> decltype(m_Stations) const 
	 {
		 return this->m_Stations;
	 }

	 bool IsHostedNetworkStarted()
	 {
		 return (this->m_HostedNetworkState == WLAN_HOSTED_NETWORK_STATE::wlan_hosted_network_active);
	 }

	 WLAN_HOSTED_NETWORK_STATE GetHostedNetworkState()
	 {
		 return this->m_HostedNetworkState;
	 }


	 GUID GetHostedNetworkInterfaceGuid()
	 {
		 auto status = this->QueryStatus();
		 return status->IPDeviceID;
	 }
};