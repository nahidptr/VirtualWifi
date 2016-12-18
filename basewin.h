/*
*  Copyright © 2015 Nahid.
*  Last Updated : 23 April 15
*/

#pragma once

#include <Windows.h>
template <typename DERIVED_TYPE>
class BaseWindow
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_TYPE *pThis = nullptr;

		if (uMsg == WM_NCCREATE)
		{
			auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			pThis = static_cast<DERIVED_TYPE*>(pCreate->lpCreateParams);
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

			pThis->m_hwnd = hwnd;
		}
		else
		{
			pThis = reinterpret_cast<DERIVED_TYPE*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}
		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	BaseWindow() : m_hwnd(nullptr) { }

	BOOL Create(
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = WS_EX_ACCEPTFILES,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		HWND hWndParent = nullptr,
		HMENU hMenu = nullptr
		)
	{
		WNDCLASS wc = { 0 };

		wc.lpfnWndProc = DERIVED_TYPE::WindowProc;
		wc.hInstance = ::GetModuleHandle(nullptr);
		wc.lpszClassName = ClassName();
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpszMenuName = nullptr;

		::RegisterClass(&wc);

		m_hwnd = ::CreateWindowEx(
			dwExStyle, ClassName(), lpWindowName, dwStyle, x, y,
			nWidth, nHeight, hWndParent, hMenu, ::GetModuleHandle(nullptr), this
			);

		return (m_hwnd ? TRUE : FALSE);
	}

	HWND Window() const { return m_hwnd; }

protected:
	virtual ~BaseWindow()
	{
	}

	virtual PCWSTR  ClassName() const = 0;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	HWND m_hwnd;
};
