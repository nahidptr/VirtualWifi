/*
*  Copyright © 2015 Nahid.
*  Last Updated : 23 April 15
*/

#pragma once
#include	"WLanManager.h"
#include	<memory>

struct ConnectionSettings
{
	std::string SSID;
	int MaxPeerCount;
};

struct ConnectedPeer
{
	explicit ConnectedPeer(WlanStation peer)
	{
		this->MacAddress = peer.MacAddress();
	}
	
	std::string MacAddress;
};



class VirtualRouterHost 
{
	std::shared_ptr<WLanManager>	m_sp_wlanManager;
	std::string						m_lastErrorMessage; 

	template <typename T, typename R, typename Event>
	auto GenericEventAttach(T* obj, R T::* mf, Event ev, bool disconnect) -> void
	{
		auto& signal = (obj->*mf);
		if (disconnect)
		{
			signal.disconnect(ev);
		}
		else
		{
			signal.connect(ev);
		}
	}

public:
	
	/*template<typename T>
	auto operator+=(T ) -> VirtualRouterHost
	{
	
	}*/

	template<typename T>
	auto OnStationJoin(const T& ev, bool disconnect) -> void 
	{
		GenericEventAttach(m_sp_wlanManager.get(), &WLanManager::StationJoin, ev, disconnect);
	}

	template<typename T> 
	auto OnStationLeave(const T& ev, bool disconnect) -> void 
	{
		GenericEventAttach(m_sp_wlanManager.get(), &WLanManager::StationLeave, ev, disconnect);
	}

	template<typename T>
	auto OnNetworkStarted(const T& ev, bool disconnect) -> void 
	{
		GenericEventAttach(m_sp_wlanManager.get(), &WLanManager::HostedNetworkStarted, ev, disconnect);
	}

	template<typename T>
	auto OnNetworkStopped(const T& ev, bool disconnect) -> void 
	{
		GenericEventAttach(m_sp_wlanManager.get(), &WLanManager::HostedNetworkStopped, ev, disconnect);
	}

	template<typename T>
	auto OnNetworkAvailable(const T& ev, bool disconnect) -> void 
	{

		/*if (disconnect)
		{
			m_sp_wlanManager->HostedNetworkAvailable.disconnect(ev);
		}
		else
		{
			m_sp_wlanManager->HostedNetworkAvailable.connect(ev);

		}*/

		//GenericEventAttach(m_sp_wlanManager->HostedNetworkAvailable, ev, disconnect);
		GenericEventAttach(m_sp_wlanManager.get(), &WLanManager::HostedNetworkAvailable, ev, disconnect);

	}

	VirtualRouterHost() 
	{
		m_sp_wlanManager = std::make_shared<WLanManager>();
		//this->m_sp_wlanManager = new WLanManager();
	}

	~VirtualRouterHost()
	{
	}

	std::string GetLastError() const
	{
		return this->m_lastErrorMessage;
	}

	bool Start()
	{
		try
		{
			this->Stop();

			this->m_sp_wlanManager->StartHostedNetwork();

			Sleep(100);

			return true;
		}
		catch (const std::exception& ex)
		{
			this->m_lastErrorMessage = ex.what();
			return false;
		}
	}

	bool Stop() 
	{
		try
		{
			this->m_sp_wlanManager->StopHostedNetwork();

			return true;
		}
		catch (const std::exception& ex)
		{
			this->m_lastErrorMessage = ex.what();
			return false;
		}
	}

	bool SetConnectionSettings(std::string ssid, int maxNumberOfPeers)
	{
		try
		{
			this->m_sp_wlanManager->SetConnectionSettings(ssid, maxNumberOfPeers);
			return true;
		}
		catch (const std::exception& ex)
		{
			this->m_lastErrorMessage = ex.what();
			return false;
		}
	}

	ConnectionSettings GetConnectionSettings()
	{
		try
		{
			std::string ssid;
			int maxNumberOfPeers;

			auto r = this->m_sp_wlanManager->QueryConnectionSettings(ssid, maxNumberOfPeers);

			ConnectionSettings csTemp;
			csTemp.SSID = ssid;
			csTemp.MaxPeerCount = maxNumberOfPeers;
			return csTemp;
		}
		catch (const std::exception& ex)
		{
			this->m_lastErrorMessage = ex.what();
			return ConnectionSettings();
		}
	}

	bool SetPassword(std::string password)
	{
		try
		{
			this->m_sp_wlanManager->SetSecondaryKey(password);
			return true;
		}
		catch (const std::exception& ex)
		{
			this->m_lastErrorMessage = ex.what();
			return false;
		}
	}

	std::string GetPassword()
	{
		std::string passKey;

		auto failReason = this->m_sp_wlanManager->QuerySecondaryKey(passKey);
		if (failReason != WLAN_HOSTED_NETWORK_REASON::wlan_hosted_network_reason_success)
		{
			throw std::exception("Getting password error " );
		}
	
		return passKey;
	}

	bool IsStarted() 
	{
		try
		{
			return m_sp_wlanManager->IsHostedNetworkStarted();
		}
		catch (const std::exception& ex)
		{
			this->m_lastErrorMessage = ex.what();
			return false;
		}
	}

	auto GetConnectedPeers() const _NOEXCEPT -> std::map<const std::string, WlanStation>
	{
		auto stations = m_sp_wlanManager->GetStations();
		return stations;
	}


	#if 0
	std::vector<ConnectedPeer> GetConnectedPeers()
	{
		std::vector<ConnectedPeer> vCp;
		auto stations = m_sp_wlanManager->GetStations();
		for (auto it = stations.begin(); it != stations.end(); ++it)
		{
			ConnectedPeer cp(it->second);
			vCp.emplace_back(cp);
		}
		return vCp;
	}
	#endif // 0


};
