#include "PCH.hpp"
#include "ThemeHelper.hpp"


namespace {

constexpr const TCHAR* const kpszUxThemeDll = TEXT("UxTheme.dll");


bool Internal_IsThemed(HMODULE hModule)
{
   _ASSERTE(hModule);

   typedef BOOL (WINAPI * pfIsAppThemed)(void);
   const auto pfIAT = reinterpret_cast<pfIsAppThemed>(::GetProcAddress(hModule, _CRT_STRINGIZE(IsAppThemed)));
   _ASSERTE(pfIAT);  // if the UxTheme module is available, this function should be, too
   return (pfIAT && (pfIAT() != FALSE));
}

HTHEME Internal_OpenTheme(HMODULE hModule, HWND hWnd, LPCTSTR pszClassList)
{
   _ASSERTE(hModule);

   typedef HTHEME (WINAPI * pfOpenThemeData)(HWND, LPCTSTR);
   const auto pfOTD = reinterpret_cast<pfOpenThemeData>(::GetProcAddress(hModule, _CRT_STRINGIZE(OpenThemeData)));
   _ASSERTE(pfOTD);  // if the UxTheme module is available, this function should be, too
   return (pfOTD ? pfOTD(hWnd, pszClassList) : NULL);
}

}  // anonymous namespace

/* static */ bool ThemeHelper::IsWinXPOrLater()
{
   OSVERSIONINFO osvi;
   osvi.dwOSVersionInfoSize = sizeof(osvi);
   #pragma warning(suppress: 4996)  // suppress deprecation warning: it's OK if the API lies to us
   const auto succeeded = ::GetVersionEx(&osvi);
   _ASSERTE(succeeded);
   return (succeeded                                    &&
           (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
           ((osvi.dwMajorVersion > 5)
            ||
            ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1))));
}

/* static */ bool ThemeHelper::ThemesEnabled()
{
   if (IsWinXPOrLater())
   {
      const auto hModule = ::LoadLibrary(kpszUxThemeDll);
      if (hModule)
      {
         const auto result = Internal_IsThemed(hModule);
         VERIFY(::FreeLibrary(hModule) != FALSE);
         return result;
      }
   }
   return false;
}

/* static */ void ThemeHelper::SafeSetWindowTheme(HWND hWnd, LPCTSTR pszSubAppName, LPCTSTR pszSubIdList)
{
   _ASSERTE(hWnd && (::IsWindow(hWnd)));
   if (IsWinXPOrLater())
   {
      const auto hModule = ::LoadLibrary(kpszUxThemeDll);
      if (hModule)
      {
         typedef HRESULT (WINAPI * pfSetWindowTheme)(HWND, LPCTSTR, LPCTSTR);
         const auto pfSWT = reinterpret_cast<pfSetWindowTheme>(::GetProcAddress(hModule, _CRT_STRINGIZE(SetWindowTheme)));
         _ASSERTE(pfSWT);  // if the UxTheme module is available, this function should be, too
         if (pfSWT)
         {
            VERIFY(SUCCEEDED(pfSWT(hWnd, pszSubAppName, pszSubIdList)));
         }
         VERIFY(::FreeLibrary(hModule) != FALSE);
      }
   }
}

/* static */ void ThemeHelper::SafeDisableWindowTheme(HWND hWnd)
{
   ThemeHelper::SafeSetWindowTheme(hWnd, NULL, TEXT(""));
}

ThemeHelper::ThemeHelper(HWND hWnd, LPCTSTR pszClassList)
   : m_hModule(IsWinXPOrLater() ? ::LoadLibrary(kpszUxThemeDll)
                                : NULL)
   , m_hTheme((m_hModule && Internal_IsThemed(m_hModule)) ? Internal_OpenTheme(m_hModule, hWnd, pszClassList)
                                                          : NULL)
{
   _ASSERTE(IsWinXPOrLater()    ? (m_hModule != NULL) : (m_hModule == NULL));
   _ASSERTE((m_hModule == NULL) ? (m_hTheme == NULL)  : TRUE);
}

ThemeHelper::~ThemeHelper()
{
   _ASSERTE((m_hModule == NULL) ? (m_hTheme == NULL) : TRUE);
   if (m_hTheme)
   {
      typedef HRESULT (WINAPI * pfCloseThemeData)(HTHEME);
      const auto pfCTD = reinterpret_cast<pfCloseThemeData>(::GetProcAddress(m_hModule, _CRT_STRINGIZE(CloseThemeData)));
      _ASSERTE(pfCTD);  // if the UxTheme module is available, this function should be, too
      if (pfCTD)
      {
         VERIFY(SUCCEEDED(pfCTD(m_hTheme)));
      }
   }
   if (m_hModule)
   {
      VERIFY(::FreeLibrary(m_hModule) != FALSE);
   }
}

bool ThemeHelper::IsThemed() const
{
   _ASSERTE((m_hModule == NULL) ? (m_hTheme == NULL) : TRUE);
   return (m_hTheme != NULL);
}

COLORREF ThemeHelper::GetThemeColor(int iPartId, int iStateId, int iPropertyId) const
{
   _ASSERTE(this->IsThemed());

   typedef HRESULT (WINAPI * pfGetThemeColor)(HTHEME, int, int, int, COLORREF*);
   const auto pfGTC = reinterpret_cast<pfGetThemeColor>(::GetProcAddress(m_hModule, _CRT_STRINGIZE(GetThemeColor)));
   _ASSERTE(pfGTC);  // if the UxTheme module is available, this function should be, too

   COLORREF clr;
   VERIFY(SUCCEEDED(pfGTC(m_hTheme, iPartId, iStateId, iPropertyId, &clr)));
   return clr;
}

LOGFONT ThemeHelper::GetThemeFont(__in_opt HDC hDC, int iPartId, int iStateId, int iPropertyId) const
{
   _ASSERTE(this->IsThemed());

   typedef HRESULT (WINAPI * pfGetThemeFont)(HTHEME, HDC, int, int, int, LOGFONT*);
   const auto pfGTF = reinterpret_cast<pfGetThemeFont>(::GetProcAddress(m_hModule, _CRT_STRINGIZE(GetThemeFont)));
   _ASSERTE(pfGTF);  // if the UxTheme module is available, this function should be, too

   LOGFONT lf;
   VERIFY(SUCCEEDED(pfGTF(m_hTheme, hDC, iPartId, iStateId, iPropertyId, &lf)));
   return lf;
}

RECT ThemeHelper::GetThemeRect(int iPartId, int iStateId, int iPropertyId) const
{
   _ASSERTE(this->IsThemed());

   typedef HRESULT (WINAPI * pfGetThemeRect)(HTHEME, int, int, int, LPRECT);
   const auto pfGTR = reinterpret_cast<pfGetThemeRect>(::GetProcAddress(m_hModule, _CRT_STRINGIZE(GetThemeRect)));
   _ASSERTE(pfGTR);  // if the UxTheme module is available, this function should be, too

   RECT rc;
   VERIFY(SUCCEEDED(pfGTR(m_hTheme, iPartId, iStateId, iPropertyId, &rc)));
   return rc;
}

namespace {

MARGINS Internal_GetThemeMargins(HMODULE hModule, HTHEME hTheme,
                                 __in_opt HDC hDC, int iPartId, int iStateId, int iPropId,
                                 __in_opt LPCRECT prc)
{
   ASSERT(hModule);
   ASSERT(hTheme );

   typedef HRESULT (WINAPI * pfGetThemeMargins)(HTHEME, HDC, int, int, int, LPCRECT, MARGINS*);
   const auto pfGTM = reinterpret_cast<pfGetThemeMargins>(::GetProcAddress(hModule, _CRT_STRINGIZE(GetThemeMargins)));
   _ASSERTE(pfGTM);  // if the UxTheme module is available, this function should be, too

   MARGINS margins;
   VERIFY(SUCCEEDED(pfGTM(hTheme, hDC, iPartId, iStateId, iPropId, prc, &margins)));
   return margins;
}

}  // anonymous namespace

MARGINS ThemeHelper::GetThemeMargins(__in_opt HDC hDC, int iPartId, int iStateId, int iPropertyId) const
{
   _ASSERTE(this->IsThemed());
   return Internal_GetThemeMargins(m_hModule, m_hTheme, hDC, iPartId, iStateId, iPropertyId, NULL);
}

MARGINS ThemeHelper::GetThemeMargins(__in_opt HDC hDC, int iPartId, int iStateId, int iPropertyId, const RECT& rc) const
{
   _ASSERTE(this->IsThemed());
   return Internal_GetThemeMargins(m_hModule, m_hTheme, hDC, iPartId, iStateId, iPropertyId, &rc);
}

RECT ThemeHelper::GetThemeBackgroundContentRect(__in_opt HDC hDC, int iPartId, int iStateId, const RECT& rcOuterBound) const
{
   _ASSERTE(this->IsThemed());

   typedef HRESULT (WINAPI * pfGetThemeBackgroundContentRect)(HTHEME, HDC, int, int, LPCRECT, LPCRECT);
   const auto pfGTBCR = reinterpret_cast<pfGetThemeBackgroundContentRect>(::GetProcAddress(m_hModule, _CRT_STRINGIZE(GetThemeBackgroundContentRect)));
   _ASSERTE(pfGTBCR);  // if the UxTheme module is available, this function should be, too

   RECT rcContent;
   VERIFY(SUCCEEDED(pfGTBCR(m_hTheme, hDC, iPartId, iStateId, &rcOuterBound, &rcContent)));
   return rcContent;
}

namespace {

void Internal_DrawThemeBackground(HMODULE hModule, HTHEME hTheme,
                                  HDC hDC, int iPartId, int iStateId, LPCRECT pRect,
                                  __in_opt LPCRECT pClipRect)
{
   ASSERT(hModule);
   ASSERT(hTheme );

   typedef HRESULT (WINAPI * pfDrawThemeBackground)(HTHEME, HDC, int, int, LPCRECT, LPCRECT);
   const auto pfDTB = reinterpret_cast<pfDrawThemeBackground>(::GetProcAddress(hModule, _CRT_STRINGIZE(DrawThemeBackground)));
   _ASSERTE(pfDTB);  // if the UxTheme module is available, this function should be, too

   VERIFY(SUCCEEDED(pfDTB(hTheme, hDC, iPartId, iStateId, pRect, pClipRect)));
}

}  // anonymous namespace

void ThemeHelper::DrawThemeBackground(HDC hDC, int iPartId, int iStateId, const RECT& rc) const
{
   _ASSERTE(this->IsThemed());
   Internal_DrawThemeBackground(m_hModule, m_hTheme, hDC, iPartId, iStateId, &rc, NULL);
}

void ThemeHelper::DrawThemeBackground(HDC hDC, int iPartId, int iStateId, const RECT& rc, const RECT& rcClip) const
{
   _ASSERTE(this->IsThemed());
   Internal_DrawThemeBackground(m_hModule, m_hTheme, hDC, iPartId, iStateId, &rc, &rcClip);
}

RECT ThemeHelper::DrawThemeEdge(HDC hDC, int iPartId, int iStateId, const RECT& rc, UINT uEdge, UINT uFlags) const
{
   _ASSERTE(this->IsThemed());

   typedef HRESULT (WINAPI * pfDrawThemeEdge)(HTHEME, HDC, int, int, LPCRECT, UINT, UINT, LPRECT);
   const auto pfDTE = reinterpret_cast<pfDrawThemeEdge>(::GetProcAddress(m_hModule, _CRT_STRINGIZE(DrawThemeEdge)));
   _ASSERTE(pfDTE);  // if the UxTheme module is available, this function should be, too

   RECT rcContent;
   VERIFY(SUCCEEDED(pfDTE(m_hTheme, hDC, iPartId, iStateId, &rc, uEdge, uFlags | BF_ADJUST, &rcContent)));
   return rcContent;
}

void ThemeHelper::DrawThemeText(HDC hDC, int iPartId, int iStateId,
                                __in_ecount(cchText) LPCTSTR pszText, int cchText,
                                DWORD dwTextFlags, DWORD dwTextFlags2,
                                const RECT& rc) const
{
   _ASSERTE(this->IsThemed());

   typedef HRESULT (WINAPI * pfDrawThemeText)(HTHEME, HDC, int, int, LPCTSTR, int, DWORD, DWORD, LPCRECT);
   const auto pfDTT = reinterpret_cast<pfDrawThemeText>(::GetProcAddress(m_hModule, _CRT_STRINGIZE(DrawThemeText)));
   _ASSERTE(pfDTT);  // if the UxTheme module is available, this function should be, too
   VERIFY(SUCCEEDED(pfDTT(m_hTheme, hDC, iPartId, iStateId, pszText, cchText, dwTextFlags, dwTextFlags2, &rc)));
}

void ThemeHelper::DrawThemeText(HDC hDC, int iPartId, int iStateId,
                                const CString& text,
                                DWORD dwTextFlags, DWORD dwTextFlags2,
                                const RECT& rc) const
{
   this->DrawThemeText(hDC,
                       iPartId,
                       iStateId,
                       static_cast<LPCTSTR>(text),
                       text.GetLength(),
                       dwTextFlags,
                       dwTextFlags2,
                       rc);
}
