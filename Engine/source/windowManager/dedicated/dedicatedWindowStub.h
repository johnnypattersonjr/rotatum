//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifdef TORQUE_DEDICATED

#ifndef _DEDICATED_WINDOW_STUB_H_
#define _DEDICATED_WINDOW_STUB_H_

#include "windowManager/platformWindowMgr.h"

/// The DedicatedWindowMgr is for Dedicated servers, which may not
/// even have graphics hardware.  However, the WindowManager is referenced
/// (indirectly) by scripts, and therefore must exist.
class DedicatedWindowMgr : public PlatformWindowManager
{
public:

   DedicatedWindowMgr() {};

   virtual ~DedicatedWindowMgr()
   {
   }

   /// Retrieve the currently set desktop bit depth.
   /// @return -1 since there is no desktop
   virtual S32 getDesktopBitDepth() { return -1; }

   /// Retrieve the currently set desktop resolution
   /// @return Point2I(-1,-1) since there is no desktop
   virtual Point2I getDesktopResolution() { return Point2I(-1, -1); }

   /// Create a new window, appropriate for the specified device and mode.
   ///
   /// @return NULL - there is no graphics hardware available in dedicated mode
   virtual PlatformWindow *createWindow(GFXDevice *device, const GFXVideoMode &mode) { return NULL; }

   /// Produces an empty list since there are no windows in dedicated mode
   virtual void getWindows(VectorPtr<PlatformWindow*> &windows) { windows.clear(); }

   /// Get the window that currently has the input focus or NULL.
   virtual PlatformWindow* getFocusedWindow() { return NULL; }

   /// Get a window from a device ID.
   ///
   /// @return NULL.
   virtual PlatformWindow *getWindowById(WindowId id) { return NULL; }

   /// Get the first window in the window list
   ///
   /// @return The first window in the list, or NULL if no windows found
   virtual PlatformWindow *getFirstWindow() { return NULL; }
};

#endif // _DEDICATED_WINDOW_STUB_H_

#endif // TORQUE_DEDICATED
