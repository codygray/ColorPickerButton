#pragma once

#include "TargetVer.hpp"

#define NOMINMAX                            // do not define "min" and "max" macros in Windows headers
#include <algorithm>
using std::min;                             // \ instead, use the "min" and "max" template functions
using std::max;                             // /   from the C++ standard library as replacements

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN                        // exclude rarely-used stuff from Windows headers
#endif  // !VC_EXTRALEAN
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit
#define _AFX_ALL_WARNINGS                   // turns off MFC's hiding of some common and often safely ignored warning messages

#include <AfxWin.h>                         // MFC core and standard components
#include <AfxExt.h>                         // MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <AfxDtCtl.h>                       // MFC support for Internet Explorer 4 Common Controls
#endif  // _AFX_NO_OLE_SUPPORT
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <AfxCmn.h>                         // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include "AfxDialogEx.h"

#ifdef _UNICODE
   #if defined _M_IX86
      #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
   #elif defined _M_X64
      #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
   #else
      #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
   #endif
#endif  // _UNICODE
