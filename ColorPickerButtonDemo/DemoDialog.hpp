#pragma once

#include "ColorPickerButton.hpp"


class DemoDialog : public CDialogEx
{
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COLORPICKERBUTTONDEMO_DIALOG };
#endif

public:
	DemoDialog(CWnd* pParent = nullptr);

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

private:
	HICON					m_hIcon;
	ColorPickerButton m_btnColorPicker;
};
