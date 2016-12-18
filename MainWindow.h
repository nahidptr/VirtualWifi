#pragma once
#include "basewin.h"
#include "WLanManager.h"
#include "VirtualRouterHost.h"
#include <memory>
#include <Windowsx.h>
class MainWindow : public BaseWindow<MainWindow>
{

	std::shared_ptr< VirtualRouterHost>		m_sp_virtual_router_host;
	HWND									m_passEditBox;
	HWND									m_SSIDEditBox;
	HWND									m_APButton;
	HWND									m_ListBox;
	bool									m_APEanabled = false;

	#define BUTTONENABLEAP		100
	#define EDITBOXSSID			200
	#define EDITBOXPASSWORD		201
	#define LISTBOX1			202

	HMENU OKButtonID = reinterpret_cast<HMENU>(static_cast<DWORD_PTR>(BUTTONENABLEAP));
	HMENU SSIDEditBoxID = reinterpret_cast<HMENU>(static_cast<DWORD_PTR>(EDITBOXSSID));
	HMENU PasswordEditBoxID = reinterpret_cast<HMENU>(static_cast<DWORD_PTR>(EDITBOXPASSWORD));
	HMENU ListBoxID = reinterpret_cast<HMENU>(static_cast<DWORD_PTR>(LISTBOX1));

	void UpdateListBox();

public:

	MainWindow()
	{}

	const WCHAR *ClassName() const override;
	void CreateEditBoxChilds();
	bool ValidateFields(std::string ssid, std::string password) const;
	void OnCreate();
	void OnCommand(WPARAM w_param, LPARAM l_param);
	LRESULT	HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void OnSize(LPARAM lParam);

	void OnDestroy();

	void OnPaint();

	void HostConnectedHandler(PVOID sender, PVOID eventArgs);
	void HostLeavedHandler(PVOID sender, PVOID eventArgs);

	void NetworkStartHandler(PVOID sender, PVOID eventArgs);
	void NetworkStopHandler(PVOID sender, PVOID eventArgs);
	void NetworkAvailableHandler(PVOID sender, PVOID eventArgs);

};
