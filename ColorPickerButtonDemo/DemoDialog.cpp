#include "PCH.hpp"
#include "Framework.hpp"
#include "DemoDialog.hpp"
#include "DemoApp.hpp"
#include "Resources.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DemoDialog::DemoDialog(CWnd* pParent /* = nullptr */)
   : CDialogEx(IDD_COLORPICKERBUTTONDEMO_DIALOG, pParent)
   , m_hIcon(::AfxGetApp()->LoadIcon(IDR_MAINFRAME))
{ }

BOOL DemoDialog::OnInitDialog()
{
   CDialogEx::OnInitDialog();

   // Set the icon for this dialog.
   SetIcon(m_hIcon, TRUE);	  // set big icon
   SetIcon(m_hIcon, FALSE);  // set small icon

   // TODO: Add extra initialization here

   return TRUE;  // return TRUE  unless you set the focus to a control
}

void DemoDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialogEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_COLORPICKERBUTTON, m_btnColorPicker);
}

BEGIN_MESSAGE_MAP(DemoDialog, CDialogEx)
   ON_WM_PAINT()
   ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

void DemoDialog::OnPaint()
{
   if (IsIconic())
   {
      CPaintDC dc(this); // device context for painting

      this->SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

      // Draw icon centered in client rectangle.
      const auto cxIcon = ::GetSystemMetrics(SM_CXICON);
      const auto cyIcon = ::GetSystemMetrics(SM_CYICON);
      CRect rc;
      GetClientRect(&rc);
      dc.DrawIcon((rc.Width () - cxIcon + 1) / 2,
                  (rc.Height() - cyIcon + 1) / 2,
                  m_hIcon);
   }
   else
   {
      CDialogEx::OnPaint();
   }
}

HCURSOR DemoDialog::OnQueryDragIcon()
{
   return static_cast<HCURSOR>(m_hIcon);
}
