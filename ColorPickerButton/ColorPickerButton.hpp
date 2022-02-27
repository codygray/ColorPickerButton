// A color-picker button (ColorPickerButton) for MFC applications.
//
// Although thoroughly redesigned and rewritten by Cody Gray (cody@codygray.com),
// this implementation is based on the original CColorButton class by:
//    Chris Maunder           (chrismaunder@codeguru.com),
//    Alexander Bischofberger (bischofb@informatik.tu-muenchen.de),
//    and James White.
// http://www.codetools.com/miscctrl/colorbutton.asp
// http://www.codetools.com/miscctrl/colour_picker.asp
// Original copyright (c) 1998 by Chris Maunder and Alexander Bischofberger.
// Copyright (c) 2000-2002 by Descartes Systems Sciences, Inc.
// The code was permissively licensed, for use "in compiled form in any way
// that you desire." Redistribution of the file was more restrictive:
// "This file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and providing
// that this notice and the authors name is included."
//
// To utilize this ColorPickerButton class in a dialog-based MFC application:
//  1) Copy the following files into your application directory, and
//     ensure that they have all been included in your project:
//        - ColorPickerButton.hpp
//        - ColorPickerButton.cpp
//        - ThemeHelper.hpp
//        - ThemeHelper.cpp
//     or build them as a static library, which you then link in to your application
//     and reference only the header files.
//  2) If you experience difficulty compiling (e.g., undefined symbol errors),
//     ensure that the target Windows version is set to Vista (or later).
//  3) Add a button to a dialog using the resource editor (aside from adjusting
//     its position and size, no additional manipulations are necessary).
//  4) Edit the class definition for the parent dialog box as follows:
//     a) Ensure that `#include "ColorPickerButton.hpp"` appears at the top of the file.
//     b) Add a new member variable to the dialog, of type ColorPickerButton,
//        to represent the color button control. It can be named anything you want.
//     c) To the DoDataExchange() member function, add the following line:
//           DDX_Control(pDX, IDC_NAMEOFYOURCOLORBUTTONCTRL, m_clrNameOfYourMemberVar);
//  5) If you wish to customize any of the default settings for the control, call
//     the appropriate member functions in the OnInitDialog() member function.
//     For example, you will probably want to at least set the default color:
//        m_clrNameOfYourMemberVar.SetDefaultColor(RGB(255, 0, 0));
//  6) If you wish to handle any of the notifications sent by the control,
//     you must edit the class definition for the parent dialog box as follows:
//     a) Add a member function to handle the message; call it anything you like.
//        For example:
//           afx_msg void YourHandlerMemberFxn(NMHDR* pNMHDR, LRESULT* pResult);
//     b) Add a new item to handle notification messages to the message map.
//        For example, to handle the color changed notification (CPN_SELCHANGED), add:
//           ON_NOTIFY(CPN_SELCHANGED, IDC_NAMEOFYOURCOLORBUTTONCTRL, &YourHandlerMemberFxn)
//     c) Add the necessary code to the body of the handler member function
//        (in the implementation file). Since the result is ignored, you can simply
//        set pResult to 0 at the end of the function body; e.g.,
//           *pResult = 0;
//     Alternatively, you could derive a custom class from ColorPickerButton and
//     handle the notifications directly in there using the MFC infrastructure.
//     If you want to customize any of the behavior of ColorPickerButton, such as
//     which custom color-picker dialog box it displays, you will need to derive
//     a custom class from it (to override the DisplayCustomColorPicker function),
//     which you use in the same way as described above.

#pragma once

#include <vector>
#include <utility>
#include <optional>

class ThemeHelper;


enum ColorPickerButtonNotification : UINT
{
   CPN_SELCHANGED   = 0x8000,  // selection changed
   CPN_DROPDOWN     = 0x8001,  // drop down
   CPN_CLOSEUP      = 0x8002,  // close up
   CPN_SELENDOK     = 0x8003,  // okayed
   CPN_SELENDCANCEL = 0x8004,  // cancelled
};

struct NMCOLORPICKERBUTTON
{
   NMHDR    hdr;
   COLORREF clrCurrent;   // the new or current color
   COLORREF clrPrevious;  // the previous color (if applicable; otherwise, same as the current color)
};

class ColorPickerButton : public CButton
{
public:

   ColorPickerButton();

   virtual ~ColorPickerButton() = default;


   /// Gets the currently-selected color.
   COLORREF GetColor() const;

   /// Sets the currently-selected color.
   void SetColor(COLORREF clr);


   static constexpr auto kclrDefaultColorDefault = RGB(0, 0, 0);

   /// Gets the default (automatic) color.
   COLORREF GetDefaultColor() const;

   /// Sets the default (automatic) color.
   void SetDefaultColor(COLORREF clr);


   static constexpr const TCHAR* const kpszDefaultTextDefault = TEXT("&Automatic");

   /// Gets whether the color picker pop-up window
   /// displays the default/automatic color option.
   bool GetShowDefault() const;

   /// Sets whether the color picker pop-up window
   /// displays the default/automatic color option.
   void SetShowDefault(bool show);

   /// Gets the caption text for the default/automatic color option
   /// when displayed in the color picker pop-up window.
   const CString& GetDefaultText() const;

   /// Sets the caption text for the default/automatic color option
   /// when displayed in the color picker pop-up window.
   void SetDefaultText(CString strText, bool show = true);


   static constexpr const TCHAR* const kpszCustomTextDefault = TEXT("&More Colors…");

   /// Gets whether the color picker pop-up window
   /// displays the custom color option.
   bool GetShowCustom() const;

   /// Sets whether the color picker pop-up window
   /// displays the custom color option.
   void SetShowCustom(bool show);

   /// Gets the caption text for the custom color option
   /// when displayed in the color picker pop-up window.
   const CString& GetCustomText() const;

   /// Sets the caption text for the custom color option
   /// when displayed in the color picker pop-up window.
   void SetCustomText(CString strText, bool show = true);


   /// Gets whether the color picker pop-up window displays tooltips
   /// when the user hovers over color swatches.
   bool GetShowTooltips() const;

   /// Sets whether or not the color picker pop-up window displays tooltips.
   /// (The default is true.)
   void SetShowTooltips(bool displayTooltips);


   /// Gets whether the color picker pop-up window automatically tracks the user's selection
   /// when hovering over color swatches.
   bool GetTrackSelection() const;

   /// Sets whether the color picker pop-up window automatically tracks the user's selection
   /// when hovering over color swatches.
   void SetTrackSelection(bool trackSelection);


   static constexpr size_t                       kcColorTableMax            = std::numeric_limits<decltype(LOGPALETTE().palNumEntries)>::max();
   static constexpr size_t                       kcColorTableDefault        = 48;
   static const     std::pair<COLORREF, CString> kColorTableDefault[kcColorTableDefault];
   static constexpr size_t                       kcColorTableColumnsDefault = 8;

   /// Gets the table of color swatches displayed in the color picker pop-up window.
   const std::vector<std::pair<COLORREF, CString>>& GetColorTable() const;

   /// Gets the number of rows and columns of color swatches
   /// displayed in the color picker pop-up window.
   CSize GetColorTableGrid() const;

   /// Sets the table of color swatches displayed in the color picker pop-up window.
   void SetColorTable(std::vector<std::pair<COLORREF, CString>> colorTable,
                      size_t                                    cColumns = kcColorTableColumnsDefault);

   /// Sets the table of color swatches displayed in the color picker pop-up window.
   void SetColorTable(const std::pair<COLORREF, CString>* parrColorTable,
                      size_t                              cColors,
                      size_t                              cColumns = kcColorTableColumnsDefault);

   /// Sets the table of color swatches displayed in the color picker pop-up window,
   /// setting the tooltip for all colors to an empty string (thus preventing any
   /// tooltips from being displayed, regardless of the tooltip setting).
   void SetColorTable(const COLORREF* parrColorTable,
                      size_t          cColors,
                      size_t          cColumns = kcColorTableColumnsDefault);


private:

   /// Gets the color palette for the color picker pop-up window.
   const CPalette& GetPalette() const;

   /// Gets the color palette for the color picker pop-up window.
   CPalette& GetPalette();

   /// Sets the color palette for the color picker pop-up window,
   /// based on the current color table.
   void SetPaletteFromColorTable();

private:

   /// Sends a notification message to the parent dialog.
   void SendParentNotification(ColorPickerButtonNotification notificationCode,
                               COLORREF                      clrCurrent,
                               COLORREF                      clrPrevious);

protected:

   virtual std::optional<COLORREF> DisplayCustomColorPicker(COLORREF clrCurrent);

   virtual void PreSubclassWindow() override;
   virtual void DrawItem(LPDRAWITEMSTRUCT pDIS) override;

   DECLARE_MESSAGE_MAP()
   afx_msg void OnMouseMove(UINT nFlags, CPoint point);
   afx_msg void OnMouseLeave();
   afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
   afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
   afx_msg void OnBnClicked();


private:

   COLORREF                                  m_clrCurrent;         // current color
   COLORREF                                  m_clrDefault;         // default/automatic color
   size_t                                    m_cColumns;
   std::vector<std::pair<COLORREF, CString>> m_colorTable;
   CPalette                                  m_palette;
   CString                                   m_strDefaultText;     // default/automatic text
   CString                                   m_strCustomText;      // custom color text
   bool                                      m_showDefault;        // true if showing default/automatic option
   bool                                      m_showCustom;         // true if showing custom option
   bool                                      m_showTooltips;       // true if showing tooltips
   bool                                      m_trackSelection;     // true if tracking selection
   bool                                      m_isPopupActive;      // true if popup active
   bool                                      m_isMouseOver;        // true if the mouse is over

   // ---------------------------
   // ColorPickerPopup class
   // ---------------------------

private:

   class ColorPickerPopup : public CWnd
   {
   public:

      explicit ColorPickerPopup(ColorPickerButton& colorPickerButton);

      virtual ~ColorPickerPopup();

   public:

      /// Displays the color picker pop-up window and begins the selection process.
      /// Returns true if a new color was selected, or false if the user canceled the picker.
      bool Open();

   private:

      /// Registers the window class for the color picker pop-up window.
      static bool Register();

      /// Unregisters the window class for the color picker pop-up window.
      static bool Unregister();


      /// Closes the color picker pop-up window, ends the selection process,
      /// and updates the currently-selected color.
      void Close();

      /// Closes the color picker pop-up window, ends the selection process,
      /// and restores the previously-selected color.
      void Cancel();


      /// Do a hit-test, converting the specified point inside of the color picker pop-up window
      /// into the index of a color cell or the invalid color index.
      int HitTest(const POINT& pt) const;


      /// Maps the specified index to a color value.
      COLORREF ColorFromIndex(int index) const;


      /// Set the currently-selected color in the color picker pop-up window to the specified index.
      void ChangeSelection(int index);

      /// Set the currently-selected color in the color picker pop-up window to the specified color.
      void ChangeSelectionToColor(COLORREF clr);

      void ChangeSelectionByOffset(int offset);


      /// Retrieve the dimensions of the specified cell in the color picker pop-up window,
      /// if the specified index is valid.
      std::optional<RECT> GetSwatchRect(int index) const;

      struct PaintSwatchInfo
      {
         bool           hot;
         bool           selected;
         CRect          rc;
         CSize          szMargin;
         CSize          szHiBorder;
         const CString* pstrText;
         COLORREF       clr;
      };
      std::optional<PaintSwatchInfo> GetPaintSwatchInfo(int index) const;

      /// Draw the specified cell from a paint event handler function, which means that the
      /// DC has already been prepared and the required colors have already been retrieved.
      void PaintSwatchUnthemed(int      index,
                               CDC&     dc,
                               COLORREF clrText,
                               COLORREF clrBackground,
                               COLORREF clrHighlightBorder,
                               COLORREF clrHighlight,
                               COLORREF clrHighlightText,
                               COLORREF clrLowlight);

      /// Draw the specified cell from a paint event handler function, which means that the DC
      /// has already been prepared and the required drawing metrics have already been retrieved.
      void PaintSwatchThemed(int                index,
                             CDC&               dc,
                             const ThemeHelper& theme,
                             const MARGINS&     marginsBorder);

      void PaintContent(CDC& dc);

   protected:
      DECLARE_MESSAGE_MAP()
      afx_msg void    OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
      afx_msg void    OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
      afx_msg void    OnLButtonDown(UINT nFlags, CPoint point);
      afx_msg void    OnMouseMove(UINT nFlags, CPoint point);
      afx_msg void    OnPaint();
      afx_msg LRESULT OnPrintClient(WPARAM wParam, LPARAM lParam);
      afx_msg BOOL    OnQueryNewPalette();
      afx_msg void    OnPaletteChanged(CWnd* pFocusWnd);

   private:
      ColorPickerButton& m_wndColorPickerBtn;
      const CSize        m_szMargins;      // margins for the color picker window
      CRect              m_rcDefaultText;  // rectangle for the default/automatic text
      CRect              m_rcCustomText;   // rectangle for the custom text
      CRect              m_rcSwatches;     // rectangle for the color swatches
      COLORREF           m_clrOriginal;    // the originally-selected color when the picker window is opened
      int                m_iCurrentColor;  // index of the current selection in the picker window
      int                m_iChosenColor;   // index of the user's original/final selection in the picker window
      bool               m_okayed;         // true if the picker was OKed; false if it was canceled
   };
};
