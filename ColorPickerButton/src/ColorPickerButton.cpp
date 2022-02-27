#include "PCH.hpp"
#include "ColorPickerButton.hpp"
#include "ThemeHelper.hpp"
#include <memory>                 // for unique_ptr


namespace {

//////////////////////////////////////////////////
// Environment Helper Functions
//////////////////////////////////////////////////

bool AreFlatMenusEnabled()
{
   if (ThemeHelper::IsWinXPOrLater())
   {
      BOOL       flatMenus;
      const auto succeeded = ::SystemParametersInfo(SPI_GETFLATMENU, 0, &flatMenus, 0);
      _ASSERTE(succeeded);
      return (succeeded && (flatMenus != FALSE));
   }
   else
   {
      return false;
   }
}

bool AreDropShadowsEnabled()
{
   if (ThemeHelper::IsWinXPOrLater())
   {
      BOOL       dropShadows;
      const auto succeeded = ::SystemParametersInfo(SPI_GETDROPSHADOW, 0, &dropShadows, 0);
      _ASSERTE(succeeded);
      return (succeeded && (dropShadows != FALSE));
   }
   else
   {
      return false;
   }
}

bool AreComboBoxAnimationsEnabled()
{
   BOOL       animation;
   const auto succeeded = ::SystemParametersInfo(SPI_GETCOMBOBOXANIMATION, 0, &animation, 0);
   _ASSERTE(succeeded);
   return (succeeded && (animation != FALSE));
}

bool AreKeyboardAcceleratorsHidden(HWND hWnd)
{
   return ((::SendMessage(hWnd, WM_QUERYUISTATE, 0, 0) & UISF_HIDEACCEL) == UISF_HIDEACCEL);
}

CRect GetScreenRect(HWND hWnd)
{
   const auto hmodUser32 = ::GetModuleHandle(TEXT("user32.dll"));
   _ASSERTE(hmodUser32 != NULL);  // should already be loaded!

   typedef HMONITOR (WINAPI * fnMonitorFromWindow)(HWND, DWORD);
   typedef BOOL     (WINAPI * fnGetMonitorInfo)   (HMONITOR, LPMONITORINFO);
   const auto pfnMonitorFromWindow = reinterpret_cast<fnMonitorFromWindow>(::GetProcAddress(hmodUser32, _CRT_STRINGIZE(MonitorFromWindow)));
   const auto pfnGetMonitorInfo    = reinterpret_cast<fnGetMonitorInfo>   (::GetProcAddress(hmodUser32, _CRT_STRINGIZE(GetMonitorInfo)));
   if (pfnMonitorFromWindow && pfnGetMonitorInfo)
   {
      const auto hMonitor = pfnMonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

      MONITORINFO mi;
      mi.cbSize = sizeof(mi);
      VERIFY(pfnGetMonitorInfo(hMonitor, &mi));
      return mi.rcWork;
   }
   else
   {
      return CRect(CPoint(0, 0),
                   CSize (::GetSystemMetrics(SM_CXSCREEN),
                          ::GetSystemMetrics(SM_CYSCREEN)));
   }
}

//////////////////////////////////////////////////
// Drawing Helper Functions
//////////////////////////////////////////////////

void FillSolidRect(HDC hDC, const RECT& rc, COLORREF clr)
{
   ::SetBkColor(hDC, clr);
   VERIFY(::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr));
}

void DrawArrow(HDC hDC, const RECT& rc, COLORREF clrArrow)
{
   _ASSERTE(hDC);
   const POINT ptArrow[3] = { { rc.left ,                 rc.top    },
                              { rc.right,                 rc.top    },
                              { (rc.left + rc.right) / 2, rc.bottom }
                            };
   CBrush      brush(clrArrow);
   CPen        pen  (PS_SOLID, 0, clrArrow);
   const auto  hbrushOriginal = ::SelectObject(hDC, brush.m_hObject);
   const auto  hpenOriginal   = ::SelectObject(hDC, pen  .m_hObject);
   _ASSERTE(hbrushOriginal);
   _ASSERTE(hpenOriginal);
   VERIFY(::Polygon(hDC, ptArrow, ARRAYSIZE(ptArrow)));
   VERIFY(::SelectObject(hDC, hpenOriginal  ) == pen  .m_hObject);
   VERIFY(::SelectObject(hDC, hbrushOriginal) == brush.m_hObject);
}

//////////////////////////////////////////////////
// Other Helper Functions
//////////////////////////////////////////////////

std::optional<TCHAR> GetAcceleratorCharacterFromString(const CString& str)
{
   const auto cchStr = str.GetLength();
   const auto iAccel = str.Find(TEXT('&'));
   if ((iAccel != -1) && (iAccel < (cchStr - 1)))
   {
      return str[iAccel + 1];
   }
   else
   {
      return std::nullopt;
   }
}

}  // anonymous namespace

//////////////////////////////////////////////////
// ColorPickerButton
//////////////////////////////////////////////////

// ------------------------------
// Default Color Table
// ------------------------------

const std::pair<COLORREF, CString> ColorPickerButton::kColorTableDefault[kcColorTableDefault] =
{
   { RGB(0x00, 0x00, 0x00), TEXT("Black") },
   { RGB(0x80, 0x40, 0x00), TEXT("Brown") },
   { RGB(0x33, 0x33, 0x00), TEXT("Olive Green") },
   { RGB(0x00, 0x33, 0x00), TEXT("Dark Green") },
   { RGB(0x00, 0x33, 0x66), TEXT("Dark Teal") },
   { RGB(0x00, 0x00, 0x80), TEXT("Dark Blue") },
   { RGB(0x33, 0x33, 0x99), TEXT("Indigo") },
   { RGB(0x33, 0x33, 0x33), TEXT("80% Gray") },

   { RGB(0x80, 0x00, 0x00), TEXT("Dark Red") },
   { RGB(0xFF, 0x66, 0x00), TEXT("Orange") },
   { RGB(0x80, 0x80, 0x00), TEXT("Dark Yellow") },
   { RGB(0x00, 0x80, 0x00), TEXT("Green") },
   { RGB(0x00, 0x80, 0x80), TEXT("Teal") },
   { RGB(0x00, 0x00, 0xFF), TEXT("Blue") },
   { RGB(0x66, 0x66, 0x99), TEXT("Blue-Gray") },
   { RGB(0x5B, 0x5B, 0x5B), TEXT("65% Gray") },

   { RGB(0xFF, 0x00, 0x00), TEXT("Red") },
   { RGB(0xFF, 0x99, 0x00), TEXT("Light Orange") },
   { RGB(0x99, 0xCC, 0x00), TEXT("Lime") },
   { RGB(0x33, 0x99, 0x66), TEXT("Sea Green") },
   { RGB(0x33, 0xCC, 0xCC), TEXT("Aqua") },
   { RGB(0x33, 0x66, 0xFF), TEXT("Light Blue") },
   { RGB(0x80, 0x00, 0x80), TEXT("Violet") },
   { RGB(0x80, 0x80, 0x80), TEXT("50% Gray") },

   { RGB(0xFF, 0x00, 0xFF), TEXT("Pink") },
   { RGB(0xFF, 0xCC, 0x00), TEXT("Gold") },
   { RGB(0xFF, 0xFF, 0x00), TEXT("Yellow") },
   { RGB(0x00, 0xFF, 0x00), TEXT("Bright Green") },
   { RGB(0x00, 0xFF, 0xFF), TEXT("Turquoise") },
   { RGB(0x00, 0xCC, 0xFF), TEXT("Sky Blue") },
   { RGB(0x99, 0x33, 0x66), TEXT("Plum") },
   { RGB(0xC0, 0xC0, 0xC0), TEXT("25% Gray") },

   { RGB(0xFF, 0x99, 0xCC), TEXT("Rose") },
   { RGB(0xFF, 0xCC, 0x99), TEXT("Tan") },
   { RGB(0xFF, 0xFF, 0x99), TEXT("Light Yellow") },
   { RGB(0xCC, 0xFF, 0xCC), TEXT("Light Green ") },
   { RGB(0xCC, 0xFF, 0xFF), TEXT("Light Turquoise") },
   { RGB(0x99, 0xCC, 0xFF), TEXT("Pale Blue") },
   { RGB(0xCC, 0x99, 0xFF), TEXT("Lavender") },
   { RGB(0xDF, 0xDF, 0xDF), TEXT("12.5% Gray") },

   { RGB(0xFF, 0xCC, 0xFF), TEXT("Light Pink") },
   { RGB(0xFF, 0xEE, 0xCC), TEXT("Cantaloupe") },
   { RGB(0xFF, 0xFF, 0xCC), TEXT("Banana") },
   { RGB(0xEE, 0xFF, 0xEE), TEXT("Honeydew") },
   { RGB(0xEE, 0xFF, 0xFF), TEXT("Ice") },
   { RGB(0xCC, 0xEE, 0xFF), TEXT("Mist") },
   { RGB(0xEE, 0xCC, 0xFF), TEXT("Thistle") },
   { RGB(0xFF, 0xFF, 0xFF), TEXT("White") },
};
static_assert(ARRAYSIZE(ColorPickerButton::kColorTableDefault) == ColorPickerButton::kcColorTableDefault,
              "The default color table contains the wrong number of items.");
static_assert(ARRAYSIZE(ColorPickerButton::kColorTableDefault) <= ColorPickerButton::kcColorTableMax,
              "The default color table contains too many items.");

// ------------------------------
// Constructors
// ------------------------------

ColorPickerButton::ColorPickerButton()
   : m_clrCurrent    (CLR_DEFAULT)
   , m_clrDefault    (kclrDefaultColorDefault)
   , m_cColumns      ()  // \ these members are initialized
   , m_colorTable    ()  // |   below, by a function called
   , m_palette       ()  // /   in the constructor's body
   , m_strDefaultText(kpszDefaultTextDefault)
   , m_strCustomText (kpszCustomTextDefault)
   , m_showDefault   (true)
   , m_showCustom    (true)
   , m_showTooltips  (true)
   , m_trackSelection(false)
   , m_isPopupActive (false)
   , m_isMouseOver   (false)
{
   this->SetColorTable(kColorTableDefault, kcColorTableDefault);
}

// ------------------------------
// Properties
// ------------------------------

COLORREF ColorPickerButton::GetColor() const
{
   return (m_clrCurrent != CLR_DEFAULT) ? m_clrCurrent : m_clrDefault;
}

void ColorPickerButton::SetColor(COLORREF clr)
{
   if (m_clrCurrent != clr)
   {
      const auto clrPrevious = m_clrCurrent;
      m_clrCurrent           = clr;
      this->Invalidate(TRUE);
      this->SendParentNotification(CPN_SELCHANGED, clr, clrPrevious);
   }
}


COLORREF ColorPickerButton::GetDefaultColor() const
{
   return m_clrDefault;
}

void ColorPickerButton::SetDefaultColor(COLORREF clr)
{
   if (m_clrDefault != clr)
   {
      m_clrDefault = clr;
      this->Invalidate(TRUE);
   }
}


bool ColorPickerButton::GetShowDefault() const
{
   return m_showDefault;
}

void ColorPickerButton::SetShowDefault(bool show)
{
   m_showDefault = show;
}

const CString& ColorPickerButton::GetDefaultText() const
{
   return m_strDefaultText;
}

void ColorPickerButton::SetDefaultText(CString strText, bool show /* = true */)
{
   m_strDefaultText = std::move(strText);
   this->SetShowDefault(show);
}


bool ColorPickerButton::GetShowCustom() const
{
   return m_showCustom;
}

void ColorPickerButton::SetShowCustom(bool show)
{
   m_showCustom = show;
}

const CString& ColorPickerButton::GetCustomText() const
{
   return m_strCustomText;
}

void ColorPickerButton::SetCustomText(CString strText, bool show /* = true */)
{
   m_strCustomText = std::move(strText);
   this->SetShowCustom(show);
}


bool ColorPickerButton::GetShowTooltips() const
{
   return m_showTooltips;
}

void ColorPickerButton::SetShowTooltips(bool show)
{
   m_showTooltips = show;
}


bool ColorPickerButton::GetTrackSelection() const
{
   return m_trackSelection;
}

void ColorPickerButton::SetTrackSelection(bool trackSelection)
{
   m_trackSelection = trackSelection;
}


const std::vector<std::pair<COLORREF, CString>>& ColorPickerButton::GetColorTable() const
{
   return m_colorTable;
}

CSize ColorPickerButton::GetColorTableGrid() const
{
   const auto cColors  = this->GetColorTable().size();
   const auto cColumns = m_cColumns;
   const auto cRows    = ((cColors / cColumns) + ((cColors % cColumns) != 0));
   return CSize(cRows, cColumns);
}

void ColorPickerButton::SetColorTable(std::vector<std::pair<COLORREF, CString>> colorTable,
                                      size_t                                    cColumns /* = kcColorTableColumnsDefault */)
{
   if (colorTable.size() <= kcColorTableMax)
   {
      m_cColumns   = cColumns;
      m_colorTable = std::move(colorTable);

      this->SetPaletteFromColorTable();
   }
   else
   {
      _ASSERT_EXPR(false,
                   TEXT("Too many items in color table; color table will be left unmodified."));
   }
}

void ColorPickerButton::SetColorTable(const std::pair<COLORREF, CString>* parrColorTable,
                                      size_t                              cColors,
                                      size_t                              cColumns /* = kcColorTableColumnsDefault */)
{
   m_cColumns = cColumns;

   m_colorTable.resize(cColors);
   for (size_t iColor = 0; iColor < cColors; ++iColor)
   {
      m_colorTable[iColor].first  = parrColorTable[iColor].first;
      m_colorTable[iColor].second = parrColorTable[iColor].second;
   }

   this->SetPaletteFromColorTable();
}

void ColorPickerButton::SetColorTable(const COLORREF* parrColorTable,
                                      size_t          cColors,
                                      size_t          cColumns /* = kcColorTableColumnsDefault */)
{
   m_cColumns = cColumns;

   m_colorTable.resize(cColors);
   for (size_t iColor = 0; iColor < cColors; ++iColor)
   {
      m_colorTable[iColor].first  = parrColorTable[iColor];
      m_colorTable[iColor].second = TEXT("");
   }

   this->SetPaletteFromColorTable();
}


const CPalette& ColorPickerButton::GetPalette() const
{
   return m_palette;
}

CPalette& ColorPickerButton::GetPalette()
{
   return m_palette;
}

void ColorPickerButton::SetPaletteFromColorTable()
{
   // Delete the existing CPalette object, if any.
   if (m_palette.GetSafeHandle())
   {
      VERIFY(m_palette.DeleteObject());
   }
   _ASSERTE(!m_palette.GetSafeHandle());

   const auto& colors  = this->GetColorTable();
   const auto  cColors = colors.size();
   _ASSERTE(cColors <= kcColorTableMax);
   if (cColors > 0)
   {
      auto lp            = std::make_unique<char[]>(sizeof(LOGPALETTE) +
                                                    (sizeof(PALETTEENTRY) * cColors));
      auto plp           = reinterpret_cast<LOGPALETTE*>(&lp[0]);
      plp->palVersion    = 0x300;
      plp->palNumEntries = static_cast<decltype(plp->palNumEntries)>(cColors);
      for (size_t iColor = 0; iColor < cColors; ++iColor)
      {
         plp->palPalEntry[iColor].peRed   = GetRValue(colors[iColor].first);
         plp->palPalEntry[iColor].peGreen = GetGValue(colors[iColor].first);
         plp->palPalEntry[iColor].peBlue  = GetBValue(colors[iColor].first);
         plp->palPalEntry[iColor].peFlags = 0;
      }

      // Create a new CPalette object.
      VERIFY(m_palette.CreatePalette(plp));
      _ASSERTE(m_palette.GetSafeHandle());
   }
}

// ------------------------------
// Helper Methods
// ------------------------------

void ColorPickerButton::SendParentNotification(ColorPickerButtonNotification notificationCode,
                                               COLORREF                clrCurrent,
                                               COLORREF                clrPrevious)
{
   const CWnd* const pwndParent = this->GetParent();
   if (pwndParent)
   {
      NMCOLORPICKERBUTTON nmcpb;
      nmcpb.hdr.code     = notificationCode;
      nmcpb.hdr.hwndFrom = this->m_hWnd;
      nmcpb.hdr.idFrom   = this->GetDlgCtrlID();
      nmcpb.clrCurrent   = clrCurrent;
      nmcpb.clrPrevious  = clrPrevious;

      pwndParent->SendMessage(WM_NOTIFY,
                              static_cast<WPARAM>(nmcpb.hdr.idFrom),
                              reinterpret_cast<LPARAM>(&nmcpb));
   }
   else
   {
      TRACE(TEXT("ColorPickerButton has no parent; cannot send it a notification.\n"));
   }
}

// ------------------------------
// Message Handlers
// ------------------------------

void ColorPickerButton::PreSubclassWindow()
{
   CButton::PreSubclassWindow();

   this->ModifyStyle(0, BS_OWNERDRAW);
}

void ColorPickerButton::DrawItem(LPDRAWITEMSTRUCT pDIS)
{
   const CSize szBorder(::GetSystemMetrics(SM_CXBORDER),
                        ::GetSystemMetrics(SM_CYBORDER));
   const CSize szEdge  (::GetSystemMetrics(SM_CXEDGE),
                        ::GetSystemMetrics(SM_CYEDGE));
   CRect       rcDraw(pDIS->rcItem);

   // Determine if we are themed.
   ThemeHelper theme(this->m_hWnd, VSCLASS_BUTTON);
   if (theme.IsThemed())
   {
      // Draw the background, which includes the outer edge.
      const auto iPartId  = BP_PUSHBUTTON;
      auto       iStateId = 0;
      if (((pDIS->itemState & ODS_SELECTED) != 0) || m_isPopupActive)
      {
         iStateId |= PBS_PRESSED;
      }
      if ((pDIS->itemState & ODS_DISABLED) != 0)
      {
         iStateId |= PBS_DISABLED;
      }
      if (((pDIS->itemState & ODS_HOTLIGHT) != 0) || m_isMouseOver)
      {
         iStateId |= PBS_HOT;
      }
      if ((pDIS->itemState & ODS_DEFAULT) != 0)
      {
         iStateId |= PBS_DEFAULTED;
      }
      theme.DrawThemeBackground(pDIS->hDC, iPartId, iStateId, rcDraw);
      rcDraw = theme.GetThemeBackgroundContentRect(pDIS->hDC, iPartId, iStateId, rcDraw);
   }
   else
   {
      // Draw the outer edge.
      auto nState = DFCS_BUTTONPUSH | DFCS_ADJUSTRECT;
      if (((pDIS->itemState & ODS_SELECTED) != 0) || (m_isPopupActive))
      {
         nState |= DFCS_PUSHED;
      }
      if ((pDIS->itemState & ODS_DISABLED) != 0)
      {
         nState |= DFCS_INACTIVE;
      }
      if (((pDIS->itemState & ODS_HOTLIGHT) != 0) || m_isMouseOver)
      {
         nState |= DFCS_HOT;
      }
      VERIFY(::DrawFrameControl(pDIS->hDC, &rcDraw, DFC_BUTTON, nState));

      // Apply a little visual fix-up.
      rcDraw.bottom -= 1;

      // If the button is depressed, adjust the position to give a 3D "pushed-in" appearance.
      if ((nState & DFCS_PUSHED) != 0)
      {
         rcDraw.OffsetRect(1, 1);
      }
   }

   // Create the inner margin.
   rcDraw.DeflateRect(szEdge);

   // Draw the arrow.
   if (theme.IsThemed())
   {
      ThemeHelper themeCBX(m_hWnd, VSCLASS_COMBOBOX);
      _ASSERTE(themeCBX.IsThemed());  // we're already themed as a button...

      // The size of the arrow was empirically determined to match the arrow that is drawn
      // by the Theme APIs, since they do not expose its actual size as a property.
      CRect rcArrow;
      rcArrow.left   = rcDraw.right - 9;
      rcArrow.top    = rcDraw.top   + (rcDraw.bottom / 2) - 14;
      rcArrow.right  = rcDraw.right;
      rcArrow.bottom = rcArrow.top + 14;

      const auto iPartId   = CP_DROPDOWNBUTTONRIGHT;
      const auto iStateId  = ((pDIS->itemState & ODS_DISABLED) == 0) ? 0 : CBXSR_DISABLED;
      rcArrow = themeCBX.GetThemeBackgroundContentRect(pDIS->hDC, iPartId, iStateId, rcArrow);
      themeCBX.DrawThemeBackground(pDIS->hDC, iPartId, iStateId, rcArrow);

      rcDraw.right = (rcArrow.left - (szEdge.cx / 2));
   }
   else
   {
      // The size of the arrow was empirically determined to match what the classic Windows theme
      // uses for the arrow drawn on comboboxes.
      CRect rcArrow;
      rcArrow.top    = rcDraw.top   + (rcDraw.bottom / 2) - 4;
      rcArrow.right  = rcDraw.right - szEdge.cx;
      rcArrow.left   = rcArrow.right - 6;
      rcArrow.bottom = rcArrow.top   + 3;
      if ((pDIS->itemState & ODS_DISABLED) == 0)
      {
         DrawArrow(pDIS->hDC, rcArrow, ::GetSysColor(COLOR_BTNTEXT));
      }
      else
      {
         // Draw an "etched"-looking triangle, like Windows does for disabled comboboxes.
         // (We could probably also do this using ::DrawState(), but that's harder.)
         rcArrow.OffsetRect(1, 1);
         DrawArrow(pDIS->hDC, rcArrow, ::GetSysColor(COLOR_BTNHIGHLIGHT));
         rcArrow.OffsetRect(-1, -1);
         DrawArrow(pDIS->hDC, rcArrow, ::GetSysColor(COLOR_BTNSHADOW));
      }

      rcDraw.right = rcArrow.left - szEdge.cx;
   }

   rcDraw.right -= szBorder.cx;

   // Fill the interior of the color swatch.
   // If the button is in a normal state (i.e., not disabled), then fill with the selected color.
   // Otherwise, if the button is disabled, instead of filling it with the selected color, fill
   // with a dark shadow color to make the disabled state more obvious. (Of course, the actual
   // dark shadow color is different depending on whether or not we're themed.)
   const auto clr = ((pDIS->itemState & ODS_DISABLED) == 0)
                     ? this->GetColor()
                     : theme.IsThemed() ? theme.GetThemeColor(BP_PUSHBUTTON, 0, TMT_EDGESHADOWCOLOR)
                                        : ::GetSysColor(COLOR_BTNSHADOW);
   FillSolidRect(pDIS->hDC, rcDraw, clr);

   // Draw the border around the color swatch.
   if ((pDIS->itemState & ODS_DISABLED) == 0)
   {
      const auto uEdge  = BDR_RAISEDOUTER;
      const auto nFlags = BF_RECT | BF_MONO;
      if (theme.IsThemed())
      {
         theme.DrawThemeEdge(pDIS->hDC, BP_PUSHBUTTON, 0, rcDraw, uEdge, nFlags);
      }
      else
      {
         VERIFY(::DrawEdge(pDIS->hDC, &rcDraw, uEdge, nFlags));
      }
   }

   // Draw the focus rectangle.
   if ((((pDIS->itemState & ODS_FOCUS) != 0) || m_isPopupActive) &&
       !((pDIS->itemState & ODS_NOFOCUSRECT) == ODS_NOFOCUSRECT))
   {
      // Reset the DC's color attributes.
      ::SetBkColor  (pDIS->hDC, RGB(255, 255, 255));
      ::SetTextColor(pDIS->hDC, RGB(0, 0, 0));

      rcDraw.InflateRect(std::max(1, (::GetSystemMetrics(SM_CXFOCUSBORDER) / 2)),
                         std::max(1, (::GetSystemMetrics(SM_CYFOCUSBORDER) / 2)));
      VERIFY(::DrawFocusRect(pDIS->hDC, &rcDraw));
   }
}

BEGIN_MESSAGE_MAP(ColorPickerButton, CButton)
   ON_WM_MOUSEMOVE()
   ON_WM_MOUSELEAVE()
   ON_WM_KEYDOWN()
   ON_WM_SYSKEYDOWN()
   ON_CONTROL_REFLECT(BN_CLICKED, &ColorPickerButton::OnBnClicked)
END_MESSAGE_MAP()

void ColorPickerButton::OnMouseMove(UINT nFlags, CPoint point)
{
   CButton::OnMouseMove(nFlags, point);

   if (!m_isMouseOver)
   {
      m_isMouseOver = true;

      TRACKMOUSEEVENT tme;
      tme.cbSize    = sizeof(tme);
      tme.dwFlags   = TME_LEAVE;
      tme.hwndTrack = m_hWnd;
      VERIFY(::_TrackMouseEvent(&tme));

      this->Invalidate(TRUE);
   }
}

void ColorPickerButton::OnMouseLeave()
{
   CButton::OnMouseLeave();

   if (m_isMouseOver)
   {
      m_isMouseOver = false;
      this->Invalidate(TRUE);
   }
}

void ColorPickerButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   if (nChar == VK_F4)
   {
      // Like a standard combobox, the F4 key should drop down the color picker.
      this->SendMessage(BM_CLICK, 0, 0);
   }
   else
   {
      CButton::OnKeyDown(nChar, nRepCnt, nFlags);
   }
}

void ColorPickerButton::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   const auto altKeyDown = ((nFlags & (1U << 13U)) == (1U << 13U));  // check the 13th bit
   if (((nChar == VK_DOWN) || (nChar == VK_UP)) && (altKeyDown))
   {
      // Like a standard combobox, pressing Alt+Down Arrow or Alt+Up Arrow
      // should drop down the picker.
      this->SendMessage(BM_CLICK, 0, 0);
   }
   else
   {
      CButton::OnSysKeyDown(nChar, nRepCnt, nFlags);
   }
}

void ColorPickerButton::OnBnClicked()
{
   // Mark the button as active.
   m_isPopupActive = true;

   // Save the current color for future reference.
   const auto clrOriginal = m_clrCurrent;

   // Send the drop-down notification to the parent.
   this->SendParentNotification(CPN_DROPDOWN, m_clrCurrent, clrOriginal);

   this->Invalidate(TRUE);

   // Create and display the color picker pop-up window.
   ColorPickerPopup picker(*this);
   const auto okayed = picker.Open();

   this->Invalidate(TRUE);

   // Cancel the pop-up window.
   m_isPopupActive = false;

   // Check to see if the picker was cancelled without a selection.
   if (!okayed)
   {
      // If we are tracking, restore the old selection.
      if (m_trackSelection)
      {
         this->SetColor(clrOriginal);
      }
   }
   else
   {
      if (m_clrCurrent != clrOriginal)
      {
         this->SendParentNotification(CPN_SELCHANGED, m_clrCurrent, clrOriginal);
      }
   }
   this->SendParentNotification(CPN_CLOSEUP,  m_clrCurrent, m_clrCurrent);
   this->SendParentNotification(okayed ? CPN_SELENDOK
                                       : CPN_SELENDCANCEL,
                                m_clrCurrent,
                                m_clrCurrent);
}

// ------------------------------
// Client-Customizable Functions
// ------------------------------

std::optional<COLORREF> ColorPickerButton::DisplayCustomColorPicker(COLORREF clrCurrent)
{
#if 0
   CColorDialog dlg(clrCurrent, CC_FULLOPEN | CC_ANYCOLOR, this);
   if (dlg.DoModal() == IDOK)
   {
      return dlg.GetColor();
   }
#else
   CMFCColorDialog dlg(clrCurrent, 0, this, NULL);
   if (dlg.DoModal() == IDOK)
   {
      return dlg.GetColor();
   }
#endif  // 0

   return std::nullopt;
}

//////////////////////////////////////////////////
// CColorPickerPopup class
//////////////////////////////////////////////////

namespace {

constexpr const TCHAR* const kpszClassName = TEXT("ColorPickerPopup");

constexpr int kDefaultColorIndex = -3;
constexpr int kCustomColorIndex  = -2;
constexpr int kInvalidColorIndex = -1;

// The exact same sizing rules apply to all elements in the color picker pop-up window.
// Each element is defined by 3 features: its core size, the size of its highlight border,
// and the size of its margin. For text, the core size is just the extent of the string
// to be drawn. For the color swatches (cells/boxes), it is the kszBoxCore value
// (note that this size does not necessarily have to be square).
//
// Also of note: each area has a well-defined rectangle (rcDefaultText, rcCustomText, and
// rcSwatches). Even if the default/automatic and/or custom selection option is not being
// displayed, the corresponding rectangle(s) is still well-defined, it just has no height.
// The rectangle's upper-left corner is still at the proper position, and the rectangle
// still has a valid width. rcSwatches is the bounding rectangle that contains all of the
// color swatches. These rules make drawing and hit-testing significantly easier.
constexpr SIZE kszTextHiBorder  { 3,  3};
constexpr SIZE kszTextMargin    { 2,  2};
constexpr SIZE kszSwatchHiBorder{ 2,  2};  // X and Y must be the same
constexpr SIZE kszSwatchMargin  { 0,  0};  // X and Y must be the same
constexpr SIZE kszSwatchCore    {14, 14};
constexpr SIZE kszSwatch        {kszSwatchCore.cx + (kszSwatchHiBorder.cx + kszSwatchMargin.cx) * 2,
                                 kszSwatchCore.cy + (kszSwatchHiBorder.cy + kszSwatchMargin.cy) * 2};

}  // anonymous namespace

ColorPickerButton::ColorPickerPopup::ColorPickerPopup(ColorPickerButton& colorPickerButton)
   : m_wndColorPickerBtn(colorPickerButton)
   , m_szMargins        (::GetSystemMetrics(SM_CXEDGE),
                         ::GetSystemMetrics(SM_CYEDGE))
   , m_rcDefaultText    ()
   , m_rcCustomText     ()
   , m_rcSwatches       ()
   , m_clrOriginal      ()  /* must be set later, when the picker is opened */
   , m_iCurrentColor    (kInvalidColorIndex)
   , m_iChosenColor     (kInvalidColorIndex)
   , m_okayed           (false)
{
   // Register the window class.
   VERIFY(this->Register());

   // Create the window.
   VERIFY(this->CreateEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
                         kpszClassName,
                         TEXT(""),
                         WS_POPUP,
                         0, 0, 0, 0,
                         colorPickerButton.m_hWnd,
                         NULL,
                         NULL));
}

/* virtual */ ColorPickerButton::ColorPickerPopup::~ColorPickerPopup()
{
   // If there is a window that was created in the constructor, destroy it.
   if (this->m_hWnd)
   {
      this->DestroyWindow();
   }

   // Unregister the window class.
   VERIFY(this->Unregister());
}


bool ColorPickerButton::ColorPickerPopup::Register()
{
   WNDCLASSEX wcex;
   wcex.cbSize        = sizeof(wcex);
   wcex.style         = CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW |
                        (AreDropShadowsEnabled() ? CS_DROPSHADOW : 0);
   wcex.lpfnWndProc   = ::DefWindowProc;
   wcex.cbClsExtra    = 0;
   wcex.cbWndExtra    = 0;
   wcex.hInstance     = ::AfxGetInstanceHandle();
   wcex.hIcon         = NULL;
   wcex.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground = NULL;
   wcex.lpszMenuName  = NULL;
   wcex.lpszClassName = kpszClassName;
   wcex.hIconSm       = NULL;
   return (::RegisterClassEx(&wcex) != FALSE);
}

bool ColorPickerButton::ColorPickerPopup::Unregister()
{
   return (::UnregisterClass(kpszClassName, ::AfxGetInstanceHandle()) != FALSE);
}


bool ColorPickerButton::ColorPickerPopup::Open()
{
   // Initialize our state.
   m_clrOriginal   = m_wndColorPickerBtn.GetColor();
   m_iCurrentColor = kInvalidColorIndex;
   m_iChosenColor  = kInvalidColorIndex;

   // Set the window size.
   auto dropDown = true;  // true if dropping down; false if dropping up
   {
      CSize szText(0, 0);

      // If we are showing a default/automatic or custom text area, get the font and text size.
      const auto defaultText = m_wndColorPickerBtn.GetShowDefault();
      const auto customText  = m_wndColorPickerBtn.GetShowCustom();
      if (defaultText || customText)
      {
         CClientDC  dc(this);
         const auto pFont         = m_wndColorPickerBtn.GetFont();
         const auto pfontOriginal = (pFont ? dc.SelectObject(pFont) : nullptr);

         // Get the size of the custom text (if there is custom text).
         if (customText)
         {
            szText = dc.GetTextExtent(m_wndColorPickerBtn.m_strCustomText);
         }

         // Get the size of the default text (if there is default text).
         if (defaultText)
         {
            auto szDefault = dc.GetTextExtent(m_wndColorPickerBtn.m_strDefaultText);
            if (szDefault.cx > szText.cx)
            {
               szText.cx = szDefault.cx;
            }
            if (szDefault.cy > szText.cy)
            {
               szText.cy = szDefault.cy;
            }
         }

         // Compute the final size.
         szText.cx += (kszTextMargin.cx + kszTextHiBorder.cx) * 2;
         szText.cy += (kszTextMargin.cy + kszTextHiBorder.cy) * 2;

         if (pfontOriginal)
         {
            VERIFY(dc.SelectObject(pfontOriginal) == pFont);
         }
      }

      const auto colorTableGrid = m_wndColorPickerBtn.GetColorTableGrid();

      // Compute the minimum width.
      const auto cxTotalBoxWidth = colorTableGrid.cy * kszSwatch.cx;
      auto       cxMinWidth      = cxTotalBoxWidth;
      if (cxMinWidth < szText.cx)
      {
         cxMinWidth = szText.cx;
      }

      // Create the rectangle for the default text.
      m_rcDefaultText.SetRect(0, 0, cxMinWidth, defaultText ? szText.cy : 0);

      // Initialize the color box rectangle.
      m_rcSwatches = CRect(CPoint((cxMinWidth - cxTotalBoxWidth) / 2, m_rcDefaultText.bottom),
                           CSize(cxTotalBoxWidth, colorTableGrid.cx * kszSwatch.cy));

      // Create the rectangle for the custom text.
      m_rcCustomText = CRect(CPoint(0, m_rcSwatches.bottom),
                             CSize(cxMinWidth, customText ? szText.cy : 0));

      // Determine the window's position and size, based on the parent button.
      CRect rcWindow(m_rcDefaultText.TopLeft(),
                     m_rcCustomText.BottomRight());
      CRect rcButton;
      m_wndColorPickerBtn.GetWindowRect(&rcButton);
      rcWindow.OffsetRect(rcButton.left, rcButton.bottom);

      // Adjust the rectangles for the border.
      rcWindow.right  += (m_szMargins.cx * 2);
      rcWindow.bottom += (m_szMargins.cy * 2);
      m_rcDefaultText.OffsetRect(m_szMargins);
      m_rcSwatches   .OffsetRect(m_szMargins);
      m_rcCustomText .OffsetRect(m_szMargins);

      // Make sure that the window will fit on the screen.
      const auto rcScreen = GetScreenRect(this->m_hWnd);
      if (rcWindow.right > rcScreen.right)
      {
         // It falls off the right of the screen, so move it to the left.
         rcWindow.OffsetRect(rcScreen.right - rcWindow.right, 0);
      }
      if (rcWindow.left < rcScreen.left)
      {
         // It falls off the left of the screen, so move it to the right.
         rcWindow.OffsetRect(rcScreen.left - rcWindow.left, 0);
      }
      if (rcWindow.bottom > rcScreen.bottom)
      {
         // It falls off the bottom of the screen, so move the whole window up above the button
         // so that it pops up instead of down.
         rcWindow.OffsetRect(0, -((rcButton.bottom - rcButton.top) + (rcWindow.bottom - rcWindow.top)));
         dropDown = false;
      }

      // Set the window size and position.
      this->SetWindowPos(nullptr,
                         rcWindow.left,
                         rcWindow.top,
                         rcWindow.Width(),
                         rcWindow.Height(),
                         SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
   }

   // Create the tooltip control.
   // (The tooltip control is always created, even if the client has not requested that
   // they be displayed. The request to not display the tooltips is honored by not
   // relaying mouse event messages to the tooltip control. This way, the client can
   // change their mind any time about displaying tooltips, even while the color picker
   // pop-up window is visible.)
   CToolTipCtrl toolTip;
   if (toolTip.Create(this))
   {
      // Add a tip for each color swatch.
      // (The ID of a tooltip cannot be 0 if a rectangle is specified, but our color indices
      // start at 0, so we add a 1 to the color index to ensure that it is non-zero.)
      const auto& colorTable = m_wndColorPickerBtn.GetColorTable();
      const auto  cColors    = colorTable.size();
      for (size_t i = 0; i < cColors; ++i)
      {
         const auto orcSwatch = this->GetSwatchRect(i);
         if (orcSwatch)
         {
            toolTip.AddTool(this,
                            static_cast<LPCTSTR>(colorTable[i].second),
                            &(*orcSwatch),
                            (i + 1));
         }
      }
   }

   // Select the swatch, if any, that corresponds to the initial color.
   this->ChangeSelectionToColor(m_wndColorPickerBtn.m_clrCurrent);

   // Show the window.
   if (AreComboBoxAnimationsEnabled())
   {
      // The leaked Windows 2000 source code (see <../ntos/w32/ntuser/client/combo.c> for the
      // implementation of the combobox) uses a constant named CMS_QANIMATION for the time of
      // ::AnimateWindow(). This is #defined in the file <../ntos/w32/ntuser/inc/user.h>.
      const auto CMS_QANIMATION = 165;
      this->AnimateWindow(CMS_QANIMATION, AW_SLIDE | (dropDown ? AW_VER_POSITIVE : AW_VER_NEGATIVE));
   }
   else
   {
      this->ShowWindow(SW_SHOWNA);
   }

   // Purge the message queue of paint messages.
   MSG msg;
   while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
   {
      if (!::GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
      {
         return false;
      }
      ::DispatchMessage(&msg);
   }

   // Set capture to the window.
   this->SetCapture();
   _ASSERTE(::GetCapture() == this->m_hWnd);

   // Pump messages until capture is lost or the pop-up window is dismissed.
   while (::GetCapture() == this->m_hWnd)
   {
      if (!::GetMessage(&msg, NULL, 0, 0))
      {
         ::PostQuitMessage(msg.wParam);
         break;
      }

      if (m_wndColorPickerBtn.GetShowTooltips())
      {
         toolTip.RelayEvent(&msg);
      }

      switch (msg.message)
      {
         case WM_LBUTTONUP:
         {
            this->OnLButtonUp(msg.wParam,
                              CPoint(GET_X_LPARAM(msg.lParam),
                                     GET_Y_LPARAM(msg.lParam)));
            break;
         }
         case WM_MOUSEMOVE:
         {
            this->OnMouseMove(msg.wParam,
                              CPoint(GET_X_LPARAM(msg.lParam),
                                     GET_Y_LPARAM(msg.lParam)));
            break;
         }
         case WM_KEYUP:
         {
            break;
         }
         case WM_SYSKEYDOWN:
         {
            this->OnSysKeyDown(msg.wParam,
                               LOWORD(msg.lParam),
                               HIWORD(msg.lParam));
            break;
         }
         case WM_KEYDOWN:
         {
            this->OnKeyDown(msg.wParam,
                            LOWORD(msg.lParam),
                            HIWORD(msg.lParam));
            break;
         }
         default:
         {
            // Just dispatch the rest of the messages that we don't need special handling for.
            ::DispatchMessage(&msg);
            break;
         }
      }
   }
   VERIFY(::ReleaseCapture());
   VERIFY(this->DestroyWindow());

   // If needed, show the custom color picker.
   // Note that we do not assume the custom color picker will know how to map CLR_DEFAULT
   // to the default/automatic color, so we do the mapping ourselves first, always passing
   // it a legitimate COLORREF value.
   if (m_okayed)
   {
      if (m_iCurrentColor == kCustomColorIndex)
      {
         const auto clrCurrent   = (m_iChosenColor == kDefaultColorIndex)
                                   ? m_wndColorPickerBtn.GetDefaultColor()
                                   : this->ColorFromIndex(m_iCurrentColor);
         const auto oclrSelected = m_wndColorPickerBtn.DisplayCustomColorPicker(clrCurrent);
         if (oclrSelected)
         {
            m_wndColorPickerBtn.SetColor(*oclrSelected);
         }
         else
         {
            m_okayed = false;
         }
      }
      else
      {
         m_wndColorPickerBtn.SetColor(this->ColorFromIndex(m_iCurrentColor));
      }
   }
   return m_okayed;
}

void ColorPickerButton::ColorPickerPopup::Close()
{
   if (m_iCurrentColor == kInvalidColorIndex)
   {
      this->Cancel();
   }
   else
   {
      VERIFY(::ReleaseCapture());
      m_okayed = true;
   }
}

void ColorPickerButton::ColorPickerPopup::Cancel()
{
   VERIFY(::ReleaseCapture());
   m_okayed = false;
}

int ColorPickerButton::ColorPickerPopup::HitTest(const POINT& pt) const
{
   // If in the custom text rectangle, return that index.
   if (m_rcCustomText.PtInRect(pt))
   {
      return kCustomColorIndex;
   }

   // If in the default/automatic text rectangle, return that index.
   if (m_rcDefaultText.PtInRect(pt))
   {
      return kDefaultColorIndex;
   }

   // If the point isn't in the rectangle containing the color swatches, return an invalid color.
   if (!m_rcSwatches.PtInRect(pt))
   {
      return kInvalidColorIndex;
   }

   // Convert the point to a specific color index.
   const auto cColors        = static_cast<int>(m_wndColorPickerBtn.GetColorTable().size());
   const auto colorTableGrid = m_wndColorPickerBtn.GetColorTableGrid();
   const auto row            = (pt.y - m_rcSwatches.top)  / kszSwatch.cy;
   const auto col            = (pt.x - m_rcSwatches.left) / kszSwatch.cx;
   if ((row < 0) || (row >= colorTableGrid.cx) ||
       (col < 0) || (col >= colorTableGrid.cy))
   {
      return kInvalidColorIndex;
   }
   else
   {
      const auto index = row * colorTableGrid.cy + col;
      return (index < cColors) ? index : kInvalidColorIndex;
   }
}

COLORREF ColorPickerButton::ColorPickerPopup::ColorFromIndex(int index) const
{
   switch (index)
   {
      case kDefaultColorIndex:
      {
         return CLR_DEFAULT;
      }
      case kCustomColorIndex:
      {
         // If the chosen color can be mapped to an actual color, then return that color.
         // Otherwise, the chosen color is a custom color, so return that one.
         return (m_iChosenColor != kCustomColorIndex) ? this->ColorFromIndex(m_iChosenColor)
                                                      : m_clrOriginal;
      }
      case kInvalidColorIndex:
      {
         return m_clrOriginal;
      }
      default:
      {
         return m_wndColorPickerBtn.GetColorTable()[index].first;
      }
   }
}

void ColorPickerButton::ColorPickerPopup::ChangeSelection(int index)
{
   // Ensure that the specified index is in range.
   const auto cColors = m_wndColorPickerBtn.GetColorTable().size();
   _ASSERTE(index < static_cast<int>(cColors));

   // Set the current selection.
   m_iCurrentColor = index;

   // If the parent button control is tracking the selection, and we have a valid selection,
   // set its color to reflect this latest update.
   if (m_wndColorPickerBtn.GetTrackSelection())
   {
      const auto clr = this->ColorFromIndex((m_iCurrentColor != kInvalidColorIndex) ? m_iCurrentColor
                                                                                    : m_iChosenColor);
      m_wndColorPickerBtn.SetColor(clr);
   }

   // Repaint in order to ensure that the old swatch is deselected and the new swatch is selected.
   this->Invalidate(TRUE);
}

void ColorPickerButton::ColorPickerPopup::ChangeSelectionToColor(COLORREF clr)
{
   if ((clr == CLR_DEFAULT) && (m_wndColorPickerBtn.GetShowDefault()))
   {
      m_iChosenColor = kDefaultColorIndex;
   }
   else
   {
      const auto& colors  = m_wndColorPickerBtn.GetColorTable();
      const auto  cColors = colors.size();
      for (size_t i = 0; i < cColors; ++i)
      {
         if (colors[i].first == clr)
         {
            m_iChosenColor = i;
            return;
         }
      }
      m_iChosenColor = (m_wndColorPickerBtn.GetShowCustom()) ? kCustomColorIndex
                                                             : kInvalidColorIndex;
   }
}

void ColorPickerButton::ColorPickerPopup::ChangeSelectionByOffset(int offset)
{
   _ASSERTE(offset != 0);

   const auto cColors = static_cast<int>(m_wndColorPickerBtn.GetColorTable().size());

   // Based on our current position, compute a new position.
   int iNewSelection;
   if (m_iCurrentColor == kInvalidColorIndex)
   {
      iNewSelection = m_iChosenColor;
   }
   else if (m_iCurrentColor == kDefaultColorIndex)
   {
      iNewSelection = (offset > 0) ? 0
                                   : kCustomColorIndex;
   }
   else if (m_iCurrentColor == kCustomColorIndex)
   {
      iNewSelection = (offset > 0) ? kDefaultColorIndex
                                   : (cColors - 1);
   }
   else
   {
      iNewSelection = m_iCurrentColor + offset;
      if (iNewSelection < 0)
      {
         iNewSelection = kDefaultColorIndex;
      }
      else if (iNewSelection >= cColors)
      {
         iNewSelection = kCustomColorIndex;
      }
   }

   // For simplicity, the previous code blindly set default/custom indexes without caring
   // if we really have those boxes. Now, the following code ensures that we actually map
   // those values into their proper locations. This loop will run *at most* twice.
   for (;;)
   {
      if ((iNewSelection == kDefaultColorIndex) && !(m_wndColorPickerBtn.GetShowDefault()))
      {
         iNewSelection = (offset > 0) ? 0
                                      : kCustomColorIndex;
      }
      else if ((iNewSelection == kCustomColorIndex) && !(m_wndColorPickerBtn.GetShowCustom()))
      {
         iNewSelection = (offset > 0) ? kDefaultColorIndex
                                      : (cColors - 1);
      }
      else
      {
         break;
      }
   }

   // Set the new location.
   this->ChangeSelection(iNewSelection);
}

std::optional<RECT> ColorPickerButton::ColorPickerPopup::GetSwatchRect(int index) const
{
   if (index == kCustomColorIndex)
   {
      return m_rcCustomText;
   }
   else if (index == kDefaultColorIndex)
   {
      return m_rcDefaultText;
   }
   else
   {
      // The specified index should correspond to one of the color swatches;
      // validate the range.
      const auto cColors  = static_cast<LONG>(m_wndColorPickerBtn.GetColorTable().size());
      const auto cColumns = m_wndColorPickerBtn.GetColorTableGrid().cy;
      if ((index >= 0) && (index < cColors))
      {
         CRect rcSwatch;
         rcSwatch.left   = m_rcSwatches.left + (kszSwatch.cx * (index % cColumns));
         rcSwatch.top    = m_rcSwatches.top  + (kszSwatch.cy * (index / cColumns));
         rcSwatch.right  = rcSwatch.left + kszSwatch.cx;
         rcSwatch.bottom = rcSwatch.top  + kszSwatch.cy;
         return rcSwatch;
      }
      else
      {
         return std::nullopt;
      }
   }
}

std::optional<ColorPickerButton::ColorPickerPopup::PaintSwatchInfo>
   ColorPickerButton::ColorPickerPopup::GetPaintSwatchInfo(int index) const
{
   const auto orcSwatch = this->GetSwatchRect(index);
   if (orcSwatch)
   {
      PaintSwatchInfo info;
      info.hot      = (index == m_iCurrentColor);
      info.selected = (index == m_iChosenColor);
      info.rc       = *orcSwatch;
      switch (index)
      {
         case kCustomColorIndex:
         {
            info.szMargin   = kszTextMargin;
            info.szHiBorder = kszTextHiBorder;
            info.pstrText   = &(m_wndColorPickerBtn.GetCustomText());
            break;
         }
         case kDefaultColorIndex:
         {
            info.szMargin   = kszTextMargin;
            info.szHiBorder = kszTextHiBorder;
            info.pstrText   = &(m_wndColorPickerBtn.GetDefaultText());
            break;
         }
         default:
         {
            info.szMargin   = kszSwatchMargin;
            info.szHiBorder = kszSwatchHiBorder;
            info.pstrText   = nullptr;
            info.clr        = m_wndColorPickerBtn.GetColorTable()[index].first;
            break;
         }
      }
      return info;
   }
   else
   {
      return std::nullopt;
   }
}

void ColorPickerButton::ColorPickerPopup::PaintSwatchUnthemed(int      index,
                                                              CDC&     dc,
                                                              COLORREF clrText,
                                                              COLORREF clrBackground,
                                                              COLORREF clrHighlightBorder,
                                                              COLORREF clrHighlight,
                                                              COLORREF clrHighlightText,
                                                              COLORREF clrLowlight)
{
   auto oSwatch = this->GetPaintSwatchInfo(index);
   if (oSwatch)
   {
      // Draw the outline of the swatch cell.
      if (oSwatch->hot || oSwatch->selected)
      {
         // The swatch is selected.

         // Draw the background margin (if there is one).
         if ((oSwatch->szMargin.cx > 0) || (oSwatch->szMargin.cy > 0))
         {
            FillSolidRect(dc.m_hDC, oSwatch->rc, clrBackground);
            oSwatch->rc.InflateRect(-oSwatch->szMargin.cx,
                                    -oSwatch->szMargin.cy);
         }

         // Draw the selection rectangle.
         FillSolidRect(dc.m_hDC, oSwatch->rc, clrHighlightBorder);
         oSwatch->rc.InflateRect(-1, -1);

         // Draw the inner coloring.
         FillSolidRect(dc.m_hDC, oSwatch->rc, (oSwatch->hot ? clrHighlight
                                                            : clrLowlight));
         oSwatch->rc.InflateRect(-(oSwatch->szHiBorder.cx - 1),
                                 -(oSwatch->szHiBorder.cy - 1));
      }
      else
      {
         // Otherwise, the swatch is not selected, so just fill the background.
         FillSolidRect(dc.m_hDC, oSwatch->rc, clrBackground);
         oSwatch->rc.InflateRect(-(oSwatch->szMargin.cx + oSwatch->szHiBorder.cx),
                                 -(oSwatch->szMargin.cy + oSwatch->szHiBorder.cy));
      }

      // Draw the contents of the swatch cell.
      if (oSwatch->pstrText)
      {
         // Draw the text.
         dc.SetTextColor(oSwatch->hot ? clrHighlightText
                                        : clrText);
         dc.SetBkMode(TRANSPARENT);
         dc.DrawText(static_cast<LPCTSTR>(*(oSwatch->pstrText)),
                     &(oSwatch->rc),
                     DT_CENTER | DT_VCENTER | DT_SINGLELINE
                      | (AreKeyboardAcceleratorsHidden(m_wndColorPickerBtn) ? DT_HIDEPREFIX : 0));
      }
      else
      {
         // Draw the color.
         FillSolidRect(dc.m_hDC, oSwatch->rc, ::GetSysColor(COLOR_3DSHADOW));
         oSwatch->rc.InflateRect(-1, -1);
         FillSolidRect(dc.m_hDC, oSwatch->rc, m_wndColorPickerBtn.GetColorTable()[index].first);
      }
   }
}

void ColorPickerButton::ColorPickerPopup::PaintSwatchThemed(int                index,
                                                            CDC&               dc,
                                                            const ThemeHelper& theme,
                                                            const MARGINS&     marginsBorder)
{
   auto oSwatch = this->GetPaintSwatchInfo(index);
   if (oSwatch)
   {
      // Draw the outline of the swatch cell.
      if (oSwatch->selected)
      {
         oSwatch->rc.InflateRect(-(oSwatch->szMargin.cx - 1),
                                 -(oSwatch->szMargin.cy - 1),
                                 -(oSwatch->szMargin.cx - 1),
                                 -(oSwatch->szMargin.cy));  // to match the hot outline
         theme.DrawThemeBackground(dc.m_hDC, MENU_BARITEM, MBI_PUSHED, oSwatch->rc);
         oSwatch->rc.InflateRect(-1, -1, -1, 0);
      }
      if (oSwatch->hot)
      {
         if (oSwatch->selected)
         {
            // Draw the hotlight rectangle *inside* of the pushed-down area.
            oSwatch->rc.InflateRect(-marginsBorder.cxLeftWidth,
                                    -marginsBorder.cyTopHeight,
                                    -marginsBorder.cxRightWidth,
                                    -marginsBorder.cyBottomHeight);
         }
         else
         {
            // Otherwise, draw the hotlight rectangle normally around the content area.
            oSwatch->rc.InflateRect(-oSwatch->szMargin.cx,
                                    -oSwatch->szMargin.cy);
         }
         theme.DrawThemeBackground(dc.m_hDC, MENU_POPUPITEM, MPI_HOT, oSwatch->rc);
      }

      // Draw the contents of the swatch cell.
      if (oSwatch->pstrText)
      {
         // Draw the text.
         theme.DrawThemeText(dc.m_hDC,
                             MENU_POPUPITEM,
                             (oSwatch->selected) ? MPI_HOT : MPI_NORMAL,
                             static_cast<LPCTSTR>(*(oSwatch->pstrText)),
                             DT_CENTER | DT_VCENTER | DT_SINGLELINE
                              | (AreKeyboardAcceleratorsHidden(m_wndColorPickerBtn) ? DT_HIDEPREFIX : 0),
                             0,
                             oSwatch->rc);
      }
      else
      {
         // Draw the color.
         oSwatch->rc.InflateRect(-(oSwatch->szMargin.cx + oSwatch->szHiBorder.cx),
                                 -(oSwatch->szMargin.cy + oSwatch->szHiBorder.cy));
         theme.DrawThemeBackground(dc.m_hDC, MENU_POPUPBORDERS, 0, oSwatch->rc);
         oSwatch->rc.InflateRect(-marginsBorder.cxLeftWidth,
                                 -marginsBorder.cyTopHeight,
                                 -marginsBorder.cxRightWidth,
                                 -marginsBorder.cyBottomHeight);
         FillSolidRect(dc.m_hDC, oSwatch->rc, m_wndColorPickerBtn.GetColorTable()[index].first);
      }
   }
}

void ColorPickerButton::ColorPickerPopup::PaintContent(CDC& dc)
{
   const auto cColors = m_wndColorPickerBtn.GetColorTable().size();

   // Save the DC state.
   const auto iDCSaved = dc.SaveDC();
   _ASSERTE(iDCSaved > 0);

   // If we have a palette, select and realize it.
   auto& palette = m_wndColorPickerBtn.GetPalette();
   if (palette.m_hObject &&
       ((dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE) == RC_PALETTE))
   {
      dc.SelectPalette(&palette, FALSE);
      VERIFY(dc.RealizePalette() != GDI_ERROR);
   }

   // Select the font.
   const auto pFont = m_wndColorPickerBtn.GetFont();
   if (pFont)
   {
      dc.SelectObject(pFont);
   }

   // Get the client rectangle.
   CRect rc;
   this->GetClientRect(&rc);

   // Determine if we are themed.
   ThemeHelper theme(m_hWnd, VSCLASS_MENU);
   if (theme.IsThemed())
   {
      // Get the themed drawing metrics.
      const auto border = theme.GetThemeMargins(dc.m_hDC,
                                                MENU_POPUPBORDERS,
                                                0,
                                                TMT_SIZINGMARGINS,
                                                rc);

      // Draw the pop-up window's border.
      theme.DrawThemeBackground(dc.m_hDC, MENU_POPUPBORDERS, 0, rc);
      rc.DeflateRect(border.cxLeftWidth,
                     border.cyTopHeight,
                     border.cxRightWidth,
                     border.cyBottomHeight);

      // Then, draw the pop-up window's background inside of that border.
      theme.DrawThemeBackground(dc.m_hDC, MENU_POPUPBACKGROUND, 0, rc);

      // Draw the default/automatic color area.
      if (m_wndColorPickerBtn.GetShowDefault())
      {
         this->PaintSwatchThemed(kDefaultColorIndex, dc, theme, border);
      }

      // Draw the color swatches.
      for (size_t i = 0; i < cColors; ++i)
      {
         this->PaintSwatchThemed(i, dc, theme, border);
      }

      // Draw the custom color area.
      if (m_wndColorPickerBtn.GetShowCustom())
      {
         this->PaintSwatchThemed(kCustomColorIndex, dc, theme, border);
      }
   }
   else
   {
      // Get the old-school drawing metrics.
      const auto kAlpha             = 48;   // no idea why the original author chose this value
      const auto flatMenus          = AreFlatMenusEnabled();
      const auto clrText            = ::GetSysColor(COLOR_MENUTEXT);
      const auto clrBackground      = ::GetSysColor(COLOR_MENU);
      const auto clrHighlightBorder = ::GetSysColor(COLOR_HIGHLIGHT);
      const auto clrHighlight       = (flatMenus) ? ::GetSysColor(COLOR_MENUHILIGHT) : clrHighlightBorder;
      const auto clrHighlightText   = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
      const auto clrLowlight        = RGB((GetRValue(clrBackground) * (255 - kAlpha) + GetRValue(clrHighlightBorder) * kAlpha) >> 8,
                                          (GetGValue(clrBackground) * (255 - kAlpha) + GetGValue(clrHighlightBorder) * kAlpha) >> 8,
                                          (GetBValue(clrBackground) * (255 - kAlpha) + GetBValue(clrHighlightBorder) * kAlpha) >> 8);

      // Draw the pop-up window's border.
      if (flatMenus)
      {
         VERIFY(::FrameRect(dc.m_hDC, &rc, ::GetSysColorBrush(COLOR_BTNSHADOW)));
      }
      else
      {
         VERIFY(dc.DrawEdge(&rc, EDGE_RAISED, BF_RECT));
      }

      // Draw the default/automatic color area.
      if (m_wndColorPickerBtn.GetShowDefault())
      {
         this->PaintSwatchUnthemed(kDefaultColorIndex,
                                   dc,
                                   clrText,
                                   clrBackground,
                                   clrHighlightBorder,
                                   clrHighlight,
                                   clrHighlightText,
                                   clrLowlight);
      }

      // Draw the color swatches.
      for (size_t i = 0; i < cColors; ++i)
      {
         this->PaintSwatchUnthemed(i,
                                   dc,
                                   clrText,
                                   clrBackground,
                                   clrHighlightBorder,
                                   clrHighlight,
                                   clrHighlightText,
                                   clrLowlight);
      }

      // Draw the custom color area.
      if (m_wndColorPickerBtn.GetShowCustom())
      {
         this->PaintSwatchUnthemed(kCustomColorIndex,
                                   dc,
                                   clrText,
                                   clrBackground,
                                   clrHighlightBorder,
                                   clrHighlight,
                                   clrHighlightText,
                                   clrLowlight);
      }
   }

   // Restore the DC state.
   VERIFY(dc.RestoreDC(iDCSaved));
}

BEGIN_MESSAGE_MAP(ColorPickerButton::ColorPickerPopup, CWnd)
   ON_WM_SYSKEYDOWN()
   ON_WM_KEYDOWN()
   ON_WM_LBUTTONDOWN()
   ON_WM_MOUSEMOVE()
   ON_WM_PAINT()
   ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
   ON_WM_QUERYNEWPALETTE()
   ON_WM_PALETTECHANGED()
END_MESSAGE_MAP()

void ColorPickerButton::ColorPickerPopup::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   const auto altKeyDown = ((nFlags & (1U << 13U)) == (1U << 13U));  // check the 13th bit
   if (nChar == VK_MENU)
   {
      // If the Alt key is pressed, then show keyboard accelerators (in case they are not already).
      m_wndColorPickerBtn.SendMessage(WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL), 0);
      this->Invalidate(TRUE);
   }
   else if (((nChar == VK_DOWN) || (nChar == VK_UP)) && (altKeyDown))
   {
      // Alt+Down Arrow and Alt+Up Arrow should close the picker, just as they opened it.
      this->Close();
   }
   else
   {
      CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
   }
}

void ColorPickerButton::ColorPickerPopup::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   switch (nChar)
   {
      case VK_ESCAPE:
      {
         this->Cancel();
         return;
      }
      case VK_RETURN:
      case VK_SPACE:
      case VK_F4:
      {
         this->Close();
         return;
      }
      case VK_LEFT:  // left arrow
      {
         this->ChangeSelectionByOffset(-1);
         return;
      }
      case VK_RIGHT:  // right arrow
      {
         this->ChangeSelectionByOffset(1);
         return;
      }
      case VK_UP:     // up arrow
      case VK_PRIOR:  // page up
      {
         this->ChangeSelectionByOffset(-m_wndColorPickerBtn.GetColorTableGrid().cy);
         return;
      }
      case VK_DOWN:  // down arrow
      case VK_NEXT:  // page down
      {
         this->ChangeSelectionByOffset(m_wndColorPickerBtn.GetColorTableGrid().cy);
         return;
      }
      default:
      {
         // Handle accelerators, if any.
         if (m_wndColorPickerBtn.GetShowDefault())
         {
            const auto ochAccel = GetAcceleratorCharacterFromString(m_wndColorPickerBtn.GetDefaultText());
            if (ochAccel && (nChar == *ochAccel))
            {
               this->ChangeSelection(kDefaultColorIndex);
               this->Close();  // finalize the new selection
               return;
            }
         }
         if (m_wndColorPickerBtn.GetShowCustom())
         {
            const auto ochAccel = GetAcceleratorCharacterFromString(m_wndColorPickerBtn.GetCustomText());
            if (ochAccel && (nChar == *ochAccel))
            {
               this->ChangeSelection(kCustomColorIndex);
               this->Close();  // finalize the new selection
               return;
            }
         }
         break;
      }
   }

   CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void ColorPickerButton::ColorPickerPopup::OnLButtonDown(UINT /* nFlags */, CPoint point)
{
   // Perform a hit-test to see what the pointer was on when the button was released.
   // If it is a valid location that corresponds to a different color
   // than the currently-selected color, then change the selection.
   const auto iNewSelection = this->HitTest(point);
   if (iNewSelection != m_iCurrentColor)
   {
      this->ChangeSelection(iNewSelection);
   }

   this->Close();
}

void ColorPickerButton::ColorPickerPopup::OnMouseMove(UINT /* nFlags */, CPoint point)
{
   // Perform a hit-test to get the row and column of the currently-hovered color
   // (or one of the special IDs for the default/automatic color or a custom color).
   // If it is a valid location that corresponds to a different color than the
   // currently-selected color, then change the selection.
   const auto iNewSelection = this->HitTest(point);
   if (iNewSelection != m_iCurrentColor)
   {
      this->ChangeSelection(iNewSelection);
   }
}

void ColorPickerButton::ColorPickerPopup::OnPaint()
{
   CPaintDC dc(this);
   this->PaintContent(dc);
}

LRESULT ColorPickerButton::ColorPickerPopup::OnPrintClient(WPARAM wParam, LPARAM /* lParam */)
{
   // WM_PRINTCLIENT must be handled so that ::AnimateWindow() will work correctly.
   // However, the implementation is simple, since it is exactly the same as WM_PAINT.
   CDC* const pDC = CDC::FromHandle(reinterpret_cast<HDC>(wParam));
   _ASSERTE(pDC);
   if (pDC)
   {
      this->PaintContent(*pDC);
   }
   return 0;
}

BOOL ColorPickerButton::ColorPickerPopup::OnQueryNewPalette()
{
   const auto result = CWnd::OnQueryNewPalette();
   this->Invalidate(TRUE);
   return result;
}

void ColorPickerButton::ColorPickerPopup::OnPaletteChanged(CWnd* pFocusWnd)
{
   CWnd::OnPaletteChanged(pFocusWnd);
   if (pFocusWnd && (pFocusWnd->m_hWnd != this->m_hWnd))
   {
      this->Invalidate(TRUE);
   }
}
