#pragma once

#if 0
#include <vssym32.h>  // defines the part and state IDs used by the theme APIs
                      // ...but the MFC headers already include this, so we don't need to.
#endif  // 0


class ThemeHelper
{
   ThemeHelper           (const ThemeHelper&) = delete;  // not copyable
   ThemeHelper& operator=(const ThemeHelper&) = delete;  // not assignable

public:

   static bool IsWinXPOrLater();

   // Indicates whether or not the theme APIs are supported and enabled.
   // @return  Returns `true` when all of the following conditions are met:
   //             1) The current operating system supports the theme APIs.
   //             2) The user has enabled theming at the operating system (global) level.
   //             3) Theming is enabled for the current application/process.
   //          Otherwise, it returns `false`.
   // @note
   //   Note that this function does *not* check whether a particular window is themed
   //   and/or whether a particular theme class is available. To test those additional
   //   conditions, create an instance of the ThemeHelper class and then call the
   //   IsThemed() member function.
   //   @n
   //   When using the IsThemed() member function, this function need not also be called.
   //   The IsThemed() function is a superset of this function, checking everything that
   //   this function does and more. This function is provided only for use in more
   //   general cases, when one simply needs to test whether the underlying operating system
   //   supports themes.
   static bool ThemesEnabled();

   // If themes are supported and enabled, this function sets the theme for the specified window
   // (&agrave; la the ::SetWindowTheme() API). Otherwise, this function has no effect.
   //
   // Like the ::SetWindowTheme() API, this function is used to redirect an existing window to use
   // a different section of the current theme information than its class would normally use.
   // The Theme Manager remembers the specified theme associations throughout the lifetime of the
   // window (even if the system themes are subsequently changed), so this function need only be
   // called once, following creation of the window.
   //
   // @note  This function is always safe to call, even on downlevel operating systems where themes
   //        are not supported. No guards or additional checks are necessary at the call site.
   //
   // @param hWnd           A handle to the control or other window whose theme is to be set.
   //                       @n This handle cannot be null; it must correspond to a valid, existing window.
   // @param pszSubAppName  The app (group) name to use in place of the calling app's name.
   //                       @n If this parameter is null, the actual calling app's name will be used.
   // @param pszSubIdList   A semicolon-separated list of class ID names to use in place of the
   //                       default list requested by the specified window's class.
   //                       @n If this parameter is null, the window's default class ID list will be used.
   //                       @n If this parameter is an empty string, then it will not match any class
   //                          section entries. This will cause theming to be turned off for the
   //                          specified window, just as if the SafeDisableWindowTheming() function
   //                          were called.
   // @remark
   //   When this function changes the theme of a window, it follows up by automatically sending
   //   the window a `WM_THEMECHANGED` message so that the new theme can be loaded and applied.
   // @see SafeDisableWindowTheming()
   static void SafeSetWindowTheme(HWND hWnd, LPCTSTR pszSubAppName, LPCTSTR pszSubIdList);

   // If themes are supported and enabled, this function disables theming for the specified window.
   // Otherwise, this function has no effect.
   //
   // The Theme Manager remembers the specified theme associations throughout the lifetime of the
   // window (even if the system themes are subsequently changed), so this function need only be
   // called once, following creation of the window.
   //
   // @note  This function is always safe to call, even on downlevel operating systems where themes
   //        are not supported. No guards or additional checks are necessary at the call site.
   //
   // @param hWnd           A handle to the control or other window whose theme is to be removed.
   //                       @n This handle cannot be null; it must correspond to a valid, existing window.
   // @remark
   //   When this function changes the theme of a window, it follows up by automatically sending
   //   the window a `WM_THEMECHANGED` message so that the new theme can be loaded and applied.
   // @see SafeSetWindowTheme()
   static void SafeDisableWindowTheme(HWND hWnd);

public:

   // Constructs a new ThemeHelper object for the specified window and class names.
   // @param hWnd          A handle to the control or other window to be themed.
   //                      @n This handle can be null.
   // @param pszClassList  A semicolon-separated list of class names to attempt to match when
   //                      opening theme data for the specified window.
   //                      @n  The list may contain only one name. If it contains more than one
   //                          name, the names are tested one at a time for a match.
   //                      @n  If a match is found, a theme handle associated with the matching
   //                          class is opened and made available for drawing. Otherwise, no
   //                          theme data is opened and IsThemed() will return `false`.
   //                      @n  Specifying a priority-ordered list of names allows the best match
   //                          to be made between the class and the current theme. For example,
   //                          a button might request `OkButton;Button`. In this case, if the
   //                          current theme has an entry for the class `OkButton`, that will be
   //                          used; otherwise, it will fall back on the normal `Button` class.
   // @warning
   //   Although it should go without saying, the associated window (@a hWnd) must not be destroyed
   //   before this instance of the ThemeHelper class goes out of scope (i.e., is itself destroyed).
   //   Similarly, the active theme should not be changed, either programmatically or by the user.
   //   @n
   //   As such, the correct way to use this class is to **keep object instances to as narrow a
   //   scope as possible**. Ideally, a ThemeHelper object will be created at the beginning of
   //   a painting function, used throughout the function, and then allowed to go out of scope
   //   (and therefore be automatically destructed) when control passes out of the paint function.
   ThemeHelper(HWND hWnd, LPCTSTR pszClassList);

   // Releases memory and system handles as appropriate,
   // and then destroys this ThemeHelper object.
   ~ThemeHelper();


   // Indicates whether or not the theme APIs should be used for drawing.
   // @return  Returns `true` when all of the following conditions are met:
   //             1) The current operating system supports the theme APIs.
   //             2) The user has enabled theming at the operating system (global) level.
   //             3) Theming is enabled for the current application/process.
   //             4) The requested theme is available and active for the window
   //                associated with this object.
   //          Otherwise, it returns `false`.
   // @warning
   //   If this function returns `false`, the theme-based APIs provided by this class
   //   should *not* be called or used for drawing!
   bool IsThemed() const;


   // Retrieves the specified theme-defined COLOR property.
   // @param iPartId       The ID number of the part.
   //                      (A part ID of 0 refers to the root class.)
   // @param iStateId      The ID number of the state (of the part).
   //                      (A state ID of 0 refers to the root part.)
   // @param iPropertyId   The ID number of the property to retrieve.
   // @return  Returns the COLORREF value defined by the specified COLOR property.
   COLORREF GetThemeColor(int iPartId, int iStateId, int iPropertyId) const;


   // Retrieves the specified theme-defined FONT property.
   // @param hDC           [optional] A handle to the device context to be used for drawing.
   //                      @note  The specified device context is used for DPI scaling.
   // @param iPartId       The ID number of the part.
   //                      (A part ID of 0 refers to the root class.)
   // @param iStateId      The ID number of the state (of the part).
   //                      (A state ID of 0 refers to the root part.)
   // @param iPropertyId   The ID number of the property to retrieve.
   // @return  Returns a LOGFONT structure containing the value of the specified FONT property,
   //          scaled for the current logical screen DPI.
   LOGFONT GetThemeFont(__in_opt HDC hDC, int iPartId, int iStateId, int iPropertyId) const;


   // Retrieves the specified theme-defined RECT property.
   // @param iPartId       The ID number of the part.
   //                      (A part ID of 0 refers to the root class.)
   // @param iStateId      The ID number of the state (of the part).
   //                      (A state ID of 0 refers to the root part.)
   // @param iPropertyId   The ID number of the property to retrieve.
   // @return  Returns a RECT structure containing the value of the specified RECT property.
   RECT GetThemeRect(int iPartId, int iStateId, int iPropertyId) const;


   // Retrieves the specified theme-defined MARGINS property.
   // @param hDC           [optional] A handle to the device context to be used for drawing.
   // @param iPartId       The ID number of the part.
   //                      (A part ID of 0 refers to the root class.)
   // @param iStateId      The ID number of the state (of the part).
   //                      (A state ID of 0 refers to the root part.)
   // @param iPropertyId   The ID number of the property to retrieve.
   // @return  Returns a MARGINS structure containing the value of the specified MARGINS property.
   MARGINS GetThemeMargins(__in_opt HDC hDC, int iPartId, int iStateId, int iPropertyId) const;

   // Retrieves the specified theme-defined MARGINS property.
   // @param hDC           [optional] A handle to the device context to be used for drawing.
   // @param iPartId       The ID number of the part.
   //                      (A part ID of 0 refers to the root class.)
   // @param iStateId      The ID number of the state (of the part).
   //                      (A state ID of 0 refers to the root part.)
   // @param iPropertyId   The ID number of the property to retrieve.
   // @param rc            A rectangle that defines the area to be drawn into.
   // @return  Returns a MARGINS structure containing the value of the specified MARGINS property.
   MARGINS GetThemeMargins(__in_opt HDC hDC, int iPartId, int iStateId, int iPropertyId, const RECT& rc) const;


   // Retrieves a rectangle that defines the size of the content for the theme-defined background.
   // (This is usually the area inside of the borders or margins.)
   // @param hDC           [optional] A handle to the device context to be used for drawing.
   // @param iPartId       The ID number of the part being drawn.
   //                      (A part ID of 0 refers to the root class.)
   // @param iStateId      The ID number of the state (of the part) being drawn.
   //                      (A state ID of 0 refers to the root part.)
   // @param rcOuterBound  The outer bounding rectangle of the item being drawn.
   // @return  Returns a RECT structure that defines the content area.
   RECT GetThemeBackgroundContentRect(__in_opt HDC hDC, int iPartId, int iStateId, const RECT& rcOuterBound) const;


   // Draws the theme-defined border and fill for the specified item.
   // (This could be based on a bitmap file, a border and fill, or another image description.)
   // @param hDC       A handle to the device context to draw into.
   // @param iPartId   The ID number of the part to draw.
   //                  (A part ID of 0 refers to the root class.)
   // @param iStateId  The ID number of the state (of the part) to draw.
   //                  (A state ID of 0 refers to the root part.)
   // @param rc        A rectangle that defines the size and location of the item being drawn.
   void DrawThemeBackground(HDC hDC, int iPartId, int iStateId, const RECT& rc) const;

   // @copydoc DrawThemeBackground()
   // @param rcClip  A rectangle that defines the clipping rectangle
   //                (nothing will be drawn outside of this rectangle).
   void DrawThemeBackground(HDC hDC, int iPartId, int iStateId, const RECT& rc, const RECT& rcClip) const;


   // Draws the theme-defined edge for the specified item.
   // @param hDC       A handle to the device context to draw into.
   // @param iPartId   The ID number of the part to draw.
   //                  (A part ID of 0 refers to the root class.)
   // @param iStateId  The ID number of the state (of the part) to draw.
   //                  (A state ID of 0 refers to the root part.)
   // @param rc        A rectangle that defines the size and location of the edges being drawn.
   // @param uEdge     (same as the ::DrawEdge() API)
   // @param uFlags    (same as the ::DrawEdge() API)
   // @return  Returns the interior (content) rectangle (just as if the `BF_ADJUST` flag was passed).
   //          @note  If the caller has no need for it, this return value can be safely ignored.
   // @remark
   //   This function is similar to the ::DrawEdge() API, but it uses theme-defined part colors
   //   and is high-DPI aware.
   RECT DrawThemeEdge(HDC hDC, int iPartId, int iStateId, const RECT& rc, UINT uEdge, UINT uFlags) const;


   // Draws text using the theme-defined font and color for the specified item.
   // @param hDC           A handle to the device context to draw into.
   // @param iPartId       The ID number of the part to draw.
   //                      (A part ID of 0 refers to the root class.)
   // @param iStateId      The ID number of the state (of the part) to draw.
   //                      (A state ID of 0 refers to the root part.)
   // @param pszText       A pointer to a character buffer that contains the text.
   // @param cchText       The total number of characters to draw from @a pszText,
   //                      or `-1` to assume that @a pszText is NUL-terminated.
   // @param dwTextFlags   (same as the `uFormat` parameter to the ::DrawText() API)
   // @param dwTextFlags2  Specifies additional drawing options.
   // @param rc            A rectangle that defines the size and location of the text being drawn.
   void DrawThemeText(HDC hDC, int iPartId, int iStateId,
                      __in_ecount(cchText) LPCTSTR pszText, int cchText,
                      DWORD dwTextFlags, DWORD dwTextFlags2,
                      const RECT& rc) const;

   // Draws text using the theme-defined font and color for the specified item.
   // @param hDC           A handle to the device context to draw into.
   // @param iPartId       The ID nuber of the part to draw.
   //                      (A part ID of 0 refers to the root class.)
   // @param iStateId      The ID number of the state (of the part) to draw.
   //                      (A state ID of 0 refers to the root part.)
   // @param text          A string that contains the text to draw.
   // @param dwTextFlags   (same as the `uFormat` parameter to the ::DrawText() API)
   // @param dwTextFlags2  Specifies additional drawing options.
   // @param rc            A rectangle that defines the size and location of the text being drawn.
   void DrawThemeText(HDC hDC, int iPartId, int iStateId,
                      const CString& text,
                      DWORD dwTextFlags, DWORD dwTextFlags2,
                      const RECT& rc) const;


private:
   HMODULE m_hModule;
   HTHEME  m_hTheme;
};
