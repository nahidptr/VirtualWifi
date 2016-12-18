/*
*  Copyright © 2015 Nahid.
*  Last Updated : 23 April 15
*/

#include "MainWindow.h"
#include <CommCtrl.h>

#pragma comment(lib, "Wlanapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

INT __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, INT nCmdShow) {

	const static WCHAR WINDOW_NAME[] = L"VIRUTAL AP";

	MainWindow win;

	if (!win.Create(WINDOW_NAME, WS_OVERLAPPEDWINDOW))
	{
		return EXIT_FAILURE;
	}


	::ShowWindow(win.Window(), nCmdShow);

	MSG msg = {};
	while (::GetMessage(&msg, win.Window(), 0, 0) > 0) {

		if (!::TranslateAccelerator(
			win.Window(),			// handle to receiving window 
			nullptr,				// handle to active accelerator table 
			&msg))					// message data 
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

	}

	return msg.wParam;
}

void MainWindow::UpdateListBox()
{

	::SendMessage(m_ListBox, LB_RESETCONTENT, 0, 0);

	if (m_APEanabled)
	{
		auto hostList = m_sp_virtual_router_host->GetConnectedPeers();

		for (auto it = hostList.begin(); it != hostList.end(); ++it)
		{
			int pos = static_cast<int>(SendMessageA(m_ListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(it->first.c_str())));
			// Set the array index of the player as item data.	
			// This enables us to retrieve the item from the array even after the items are sorted by the list box.
			::SendMessage(m_ListBox, LB_SETITEMDATA, pos, static_cast<LPARAM>(std::distance(hostList.begin(), it)));
		}
	}

}

PCWSTR MainWindow::ClassName() const
{
	return L"WindowClass Bla";
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {

	auto hwnd = m_hwnd;

	switch (uMsg)
	{
	case WM_COMMAND:
	{
		OnCommand(wParam, lParam);
		break;
	}

	case WM_CREATE:
	{
		OnCreate();
		break;
	}
	case WM_DESTROY:
	{
		OnDestroy();
		return 0;
	}
	case WM_PAINT:
	case WM_DISPLAYCHANGE:
	{
		OnPaint();
	}
	return 0;

	case WM_SIZE:
	{
		OnSize(lParam);
		return 0;
	}

	/*case WM_ERASEBKGND:
		return 1;*/


	default:
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void MainWindow::OnSize(LPARAM lParam) {

	auto x = static_cast<int>(LOWORD(lParam));
	auto y = static_cast<int>(HIWORD(lParam));

	::InvalidateRect(m_hwnd, nullptr, FALSE);
}


void MainWindow::OnDestroy() {

	::PostQuitMessage(EXIT_SUCCESS);
}

void MainWindow::OnPaint()
{
	PAINTSTRUCT ps;
	::BeginPaint(m_hwnd, &ps);

	::EndPaint(m_hwnd, &ps);
}

void MainWindow::HostConnectedHandler(PVOID sender, PVOID eventArgs)
{
	this->UpdateListBox();
}

void MainWindow::HostLeavedHandler(PVOID sender, PVOID eventArgs)
{
	this->UpdateListBox();
}

void MainWindow::NetworkStartHandler(PVOID sender, PVOID eventArgs)
{
	Button_SetText(m_APButton, L"STOP Virtual router ");
	m_APEanabled = true;
}

void MainWindow::NetworkStopHandler(PVOID sender, PVOID eventArgs)
{
	m_APEanabled = false;
}

void MainWindow::NetworkAvailableHandler(PVOID sender, PVOID eventArgs)
{
	// do nothing for now
}

void MainWindow::CreateEditBoxChilds()
{

	// Enable visual styles 
	::InitCommonControls();

	m_APButton = ::CreateWindowExW(WS_EX_TOPMOST,
		L"BUTTON",
		L"START Virtual router",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		50,
		450,
		150,
		30,
		this->Window(),
		OKButtonID,
		::GetModuleHandle(nullptr),
		nullptr);

	if (!m_APButton)
	{
		throw std::exception("enable to create control");
	}

	// Create Rotate button.
	m_SSIDEditBox = ::CreateWindowExW(WS_EX_CLIENTEDGE,
		L"EDIT",
		L"nahidRouterAP",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		50,
		150,
		200,
		30,
		this->Window(),
		SSIDEditBoxID,
		::GetModuleHandle(nullptr),
		nullptr);

	if (!m_SSIDEditBox)
	{
		throw std::exception("enable to create control");
	}

	m_passEditBox = ::CreateWindowEx(WS_EX_CLIENTEDGE,
		L"EDIT",
		L"fooPassword123",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		50,
		200,
		200,
		30,
		this->Window(),
		PasswordEditBoxID,
		::GetModuleHandle(nullptr),
		nullptr);

	if (!m_SSIDEditBox)
	{
		throw std::exception("enable to create control");
	}


	m_ListBox = ::CreateWindowEx(WS_EX_CLIENTEDGE,
		TEXT("listbox"),
		L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL,
		350, 40, 150, 220,
		this->Window(),
		ListBoxID,
		nullptr,
		nullptr);

	if (!m_ListBox)
	{
		throw std::exception("enable to create control");
	}

}


bool MainWindow::ValidateFields(std::string ssid, std::string password) const
{
	std::string errorMessage;

	if (ssid.empty())
	{
		errorMessage += "Network Name (SSID) is required.\n";
	}

	if (ssid.length() > 32)
	{
		errorMessage += "Network Name (SSID) can not be longer than 32 characters.\n";
	}

	if (password.length() < 8)
	{
		errorMessage += "Password must be at least 8 characters.\n";
	}

	if (password.length() > 64)
	{
		errorMessage += "Password can not be longer than 64 characters.\n";
	}

	if (!errorMessage.empty())
	{
		MessageBoxA(this->Window(), errorMessage.c_str(), "Error", MB_OK);
		return false;
	}

	return true;
}


void MainWindow::OnCreate()
{
	// TODO: Change fonts later

	try
	{
		CreateEditBoxChilds();
	}
	catch (const std::exception& ex)
	{
		::MessageBoxA(this->Window(), ex.what(), "error", MB_OK);
		::PostQuitMessage(EXIT_FAILURE);
	}
	try
	{
		m_sp_virtual_router_host = std::make_shared<VirtualRouterHost>();

	}
	catch (const std::exception& ex)
	{
		::Button_Enable(m_APButton, false);
		::MessageBoxA(this->Window(), ex.what(), "error", MB_OK);
	}


	//! events 
	m_sp_virtual_router_host->OnStationJoin(boost::bind(&MainWindow::HostConnectedHandler, this, _1, _2), false);
	m_sp_virtual_router_host->OnStationLeave(boost::bind(&MainWindow::HostLeavedHandler, this, _1, _2), false);
	m_sp_virtual_router_host->OnNetworkStarted(boost::bind(&MainWindow::NetworkStartHandler, this, _1, _2), false);
	m_sp_virtual_router_host->OnNetworkStopped(boost::bind(&MainWindow::NetworkStopHandler, this, _1, _2), false);
	m_sp_virtual_router_host->OnNetworkAvailable(boost::bind(&MainWindow::NetworkAvailableHandler, this, _1, _2), false);

}

void MainWindow::OnCommand(WPARAM w_param, LPARAM l_param)
{
	switch (LOWORD(w_param))
	{
	case BUTTONENABLEAP: {

		if (!m_APEanabled)
		{
			WCHAR tempBuff[216];
			auto ssidLength = (WORD)SendMessage(m_SSIDEditBox, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);

			::SendMessage(m_SSIDEditBox, WM_GETTEXT, ssidLength + 1, LPARAM(tempBuff));

			std::wstring ssidTextTemp(tempBuff);
			std::string ssidText(ssidTextTemp.begin(), ssidTextTemp.end());


			auto passwordLength = (WORD)SendMessage(m_passEditBox, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);
			::SendMessage(m_passEditBox, WM_GETTEXT, passwordLength + 1, LPARAM(tempBuff));

			std::wstring passTextTemp(tempBuff);
			std::string passwordText(passTextTemp.begin(), passTextTemp.end());

			if (m_sp_virtual_router_host->IsStarted())
			{
				m_sp_virtual_router_host->Stop();
			}
			else
			{
				if (this->ValidateFields(ssidText, passwordText))
				{

					m_sp_virtual_router_host->SetConnectionSettings(ssidText, 100);
					m_sp_virtual_router_host->SetPassword(passwordText);


					if (!m_sp_virtual_router_host->Start())
					{
						std::string strMessage = m_sp_virtual_router_host->GetLastError();

						MessageBoxA(this->Window(), strMessage.c_str(), "ERROR", MB_OK);
					}
				}
			}
		}
		else
		{
			m_APEanabled = false;
			auto bRes = m_sp_virtual_router_host->Stop();
			if (!bRes)
			{
				MessageBoxA(this->Window(), "Error closing", "ERROR", MB_OK);

			}
			::Button_SetText(m_APButton, L"START Virtual router");
			this->UpdateListBox();
		}

	}
	}
}
