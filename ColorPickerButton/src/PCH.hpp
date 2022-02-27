#pragma once


// Including SDKDDKVer.h defines the highest available Windows platform.
//
// If you wish to build your application for a previous Windows platform,
// include WinSDKVer.h and set the _WIN32_WINNT macro to the platform
// you wish to support before including SDKDDKVer.h.
#include <WinSDKVer.h>
#define _WIN32_WINNT    _WIN32_WINNT_WIN7
#include <SDKDDKVer.h>

#define NOMINMAX                            // do not define "min" and "max" macros in Windows headers
#include <algorithm>
using std::min;                             // \ instead, use the "min" and "max" template functions
using std::max;                             // /   from the C++ standard library as replacements

#include <AfxWin.h>                         // MFC core and standard components
#include <AfxDlgs.h>
#include <AfxColorDialog.h>
