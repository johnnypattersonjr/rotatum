//-----------------------------------------------------------------------------
// Copyright (c) Johnny Patterson
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

#ifndef _PLATFORM_PLATFORMWINDOWMGR_H_
#define _PLATFORM_PLATFORMWINDOWMGR_H_

#include "math/mRect.h"
#include "core/util/journal/journaledSignal.h"
#include "windowManager/platformWindow.h"


// Global macro
#define WindowManager PlatformWindowManager::get()

/// Abstract representation of a manager for native OS windows.
///
/// The PlatformWindowManager interface provides a variety of methods for querying 
/// the current desktop configuration, as well as allocating and retrieving
/// existing windows. It may also manage application-level event handling.
class PlatformWindowManager
{
   // Generator for window IDs.
   S32 mIdSource;
   
protected:
   /// Get the next available window Id
   inline S32 getNextId() { return mIdSource++; }
public:

   /// Get Global Singleton
   static PlatformWindowManager *get();
   
   PlatformWindowManager() : mIdSource(0) {};

   virtual ~PlatformWindowManager() 
   {
   }

   /// Retrieve the currently set desktop bit depth
   /// @return The current desktop bit depth, or -1 if an error occurred
   virtual S32 getDesktopBitDepth() = 0;

   /// Retrieve the currently set desktop resolution
   /// @return The current desktop resolution, or Point2I(-1,-1) if an error occurred
   virtual Point2I getDesktopResolution() = 0;

   /// Create a new window, appropriate for the specified device and mode.
   ///
   /// @return Pointer to the new window.
   virtual PlatformWindow *createWindow(GFXDevice *device, const GFXVideoMode &mode) = 0;

   /// Populate a list with references to all the windows created from this manager.
   virtual void getWindows(VectorPtr<PlatformWindow*> &windows) = 0;

   /// Get the window that currently has the input focus or NULL.
   virtual PlatformWindow* getFocusedWindow() = 0;

   /// Get a window from a device ID.
   ///
   /// @return The window associated with the specified ID, or NULL if no
   ///         match was found.
   virtual PlatformWindow *getWindowById(WindowId id)=0;

   /// Get the first window in the window list
   ///
   /// @return The first window in the list, or NULL if no windows found
   virtual PlatformWindow *getFirstWindow()=0;

protected:
   /// Setup a window for the specified device and mode.
   ///
   /// @return Whether the window was setup without errors.
   virtual bool setupWindow(PlatformWindow *window, GFXDevice *device, const GFXVideoMode &mode) = 0;
};

/// Global function to allocate a new platform window manager.
///
/// This returns an instance of the appropriate window manager for the current OS.
PlatformWindowManager *CreatePlatformWindowManager();

#endif