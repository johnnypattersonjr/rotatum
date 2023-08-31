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

#include "platform/platform.h"
#include "platform/event.h"
#include "windowManager/platformWindowMgr.h"
#include "gfx/gfxInit.h"
#include "core/util/journal/process.h"
#include "core/util/autoPtr.h"

// This file converts from the windowmanager system to the old platform and
// event interfaces. So it's a bit hackish but hopefully will not be necessary
// for very long. Because of the heavy use of globals in the old platform
// layer, this layer also has a fair number of globals.


// WindowManager
//
// PlatformWindowManager::get() wrapped in Macro WindowManager
PlatformWindowManager *PlatformWindowManager::get() 
{
   static AutoPtr< PlatformWindowManager > smWindowManager;
   if( smWindowManager.isNull() )
      smWindowManager = CreatePlatformWindowManager();
   return smWindowManager.ptr();
}

PlatformWindow *gWindow     = NULL;

//------------------------------------------------------------------------------

void Platform::setWindowSize(U32 newWidth, U32 newHeight, bool fullScreen )
{
   AssertISV(gWindow, "Platform::setWindowSize - no window present!");

   // Grab the curent video settings and diddle them with the new values.
   GFXVideoMode vm = gWindow->getVideoMode();
   vm.resolution.set(newWidth, newHeight);
   vm.fullScreen = fullScreen;
   gWindow->setVideoMode(vm);
}

void Platform::setWindowLocked(bool locked)
{
   PlatformWindow* window = WindowManager->getFirstWindow();
   if( window )
      window->setMouseLocked( locked );
}

void Platform::minimizeWindow()
{
   // requires PlatformWindow API extension...
   if(gWindow)
      gWindow->minimize();
}

void Platform::closeWindow()
{
   // Shutdown all our stuff.
   SAFE_DELETE(gWindow);
}

//------------------------------------------------------------------------------



#ifdef TORQUE_OS_WIN32
// Hack so we can get the HWND of the global window more easily - replacement
// for the HWND that was in the platstate.
#include "windowManager/win32/win32Window.h"

HWND getWin32WindowHandle()
{
   PlatformWindow* window = WindowManager->getFocusedWindow();
   if( !window )
   {
      window = WindowManager->getFirstWindow();
      if( !window )
         return NULL;
   }

   return ( ( Win32Window* ) window )->getHWND();
}

#endif