#pragma once

#ifndef __AFXWIN_H__
	#error "include 'PCH.hpp' before including this file for PCH"
#endif  // !__AFXWIN_H__


extern
class DemoApp : public CWinApp
{
public:
	DemoApp();

protected:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
}
theApp;
