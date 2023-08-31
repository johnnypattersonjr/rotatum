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

#define NO_MINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "windowManager/win32/win32Window.h"
#include "windowManager/win32/win32WindowMgr.h"

#include "platform/menus/popupMenu.h"
#include "platform/platformInput.h"

// for winState structure
#include "platformWin32/platformWin32.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#ifndef IDI_ICON1 
#define IDI_ICON1 107
#endif

static bool isScreenSaverRunning()
{
#ifndef SPI_GETSCREENSAVERRUNNING
#define SPI_GETSCREENSAVERRUNNING 114
#endif
	// Windows 2K, and higher. It might be better to hook into
	// the broadcast WM_SETTINGCHANGE message instead of polling for
	// the screen saver status.
	BOOL sreensaver = false;
	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING,0,&sreensaver,0);
	return sreensaver;
}

Win32Window::Win32Window()
	: mAccelHandle(NULL)
	, mMenuHandle(NULL)
{
}

Win32Window::~Win32Window()
{
	if(mAccelHandle)
	{
		DestroyAcceleratorTable(mAccelHandle);
		mAccelHandle = NULL;
	}
}

HWND Win32Window::getHWND() const
{
	return glfwGetWin32Window(mWindowHandle);
}

void Win32Window::setMenuHandle(HMENU menuHandle)
{
	mMenuHandle = menuHandle;
	if (!mFullscreen)
		SetMenu(getHWND(), mMenuHandle);
}

void Win32Window::setVideoMode( const GFXVideoMode &mode )
{
	HWND window = getHWND();

	if (mode.fullScreen)
	{
		// Clear the menu bar from the window for full screen
		HMENU menu = GetMenu(window);
		if (menu)
		{
			SetMenu(window, NULL);
		}
	}

	GLFWWindow::setVideoMode(mode);

	if (!mode.fullScreen)
	{
		// Put back the menu bar, if any
		if (mMenuHandle)
		{
			SetMenu(window, mMenuHandle);
		}
	}
}

bool Win32Window::isVisible() const
{
	return GLFWWindow::isVisible() && !isScreenSaverRunning();
}

LRESULT PASCAL Win32Window::WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	GLFWwindow* glfwWindow = (GLFWwindow*)GetPropW(hWnd, L"GLFW");
	Win32Window* window = (Win32Window*)(GLFWWindow*)glfwGetWindowUserPointer(glfwWindow);

	switch (message)
	{
	case WM_MENUSELECT:
		window->setRenderOnRefresh(true);
		break;

		// Menus
	case WM_COMMAND:
		{
			window->setRenderOnRefresh(false);

			if( window == NULL )
				break;

			// [tom, 8/21/2006] Pass off to the relevant PopupMenu if it's a menu
			// or accelerator command. PopupMenu will in turn hand off to script.
			//
			// Note: PopupMenu::handleSelect() will not do anything if the menu
			// item is disabled, so we don't need to deal with that here.

			S32 numItems = GetMenuItemCount(window->getMenuHandle());
			for(S32 i = 0;i < numItems;i++)
			{
				MENUITEMINFOA mi;
				mi.cbSize = sizeof(mi);
				mi.fMask = MIIM_DATA;
				if(GetMenuItemInfoA(window->getMenuHandle(), i, TRUE, &mi))
				{
					if(mi.fMask & MIIM_DATA && mi.dwItemData != 0)
					{
						PopupMenu *mnu = (PopupMenu *)mi.dwItemData;

						PopupMenu::smSelectionEventHandled = false;
						PopupMenu::smPopupMenuEvent.trigger(mnu->getPopupGUID(), LOWORD(wParam));
						if (PopupMenu::smSelectionEventHandled)
							return 0;
					}
				}
			}
		}
		break;

	case WM_INITMENUPOPUP:
		{
			HMENU menu = (HMENU)wParam;
			MENUINFO mi;
			mi.cbSize = sizeof(mi);
			mi.fMask = MIM_MENUDATA;
			if(GetMenuInfo(menu, &mi) && mi.dwMenuData != 0)
			{
				PopupMenu *pm = (PopupMenu *)mi.dwMenuData;
				if(pm != NULL)
					pm->onMenuSelect();
			}
		}
		break;
	}

	return window->mGLFWWindowProc(hWnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Accelerators
//-----------------------------------------------------------------------------

void Win32Window::addAccelerator(Accelerator &accel)
{
	ACCEL winAccel;
	winAccel.fVirt = FVIRTKEY;
	winAccel.cmd = accel.mID;

	if(accel.mDescriptor.flags & SI_SHIFT)
		winAccel.fVirt |= FSHIFT;
	if(accel.mDescriptor.flags & SI_CTRL)
		winAccel.fVirt |= FCONTROL;
	if(accel.mDescriptor.flags & SI_ALT)
		winAccel.fVirt |= FALT;

	winAccel.key = TranslateKeyCodeToOS(accel.mDescriptor.eventCode);

	for(WinAccelList::iterator i = mWinAccelList.begin();i != mWinAccelList.end();++i)
	{
		if(i->cmd == winAccel.cmd)
		{
			// Already in list, just update it
			i->fVirt = winAccel.fVirt;
			i->key = winAccel.key;
			return;
		}

		if(i->fVirt == winAccel.fVirt && i->key == winAccel.key)
		{
			// Existing accelerator in list, don't add this one
			return;
		}
	}

	mWinAccelList.push_back(winAccel);
}

void Win32Window::removeAccelerator(Accelerator &accel)
{
	for(WinAccelList::iterator i = mWinAccelList.begin();i != mWinAccelList.end();++i)
	{
		if(i->cmd == accel.mID)
		{
			mWinAccelList.erase(i);
			return;
		}
	}
}

//-----------------------------------------------------------------------------

static bool isMenuItemIDEnabled(HMENU menu, U32 id)
{
	S32 numItems = GetMenuItemCount(menu);
	for(S32 i = 0;i < numItems;i++)
	{
		MENUITEMINFOA mi;
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_ID|MIIM_STATE|MIIM_SUBMENU|MIIM_DATA;
		if(GetMenuItemInfoA(menu, i, TRUE, &mi))
		{
			if(mi.fMask & MIIM_ID && mi.wID == id)
			{
				// This is an item on this menu
				return (mi.fMask & MIIM_STATE) && ! (mi.fState & MFS_DISABLED);
			}

			if((mi.fMask & MIIM_SUBMENU) && mi.hSubMenu != 0 && (mi.fMask & MIIM_DATA) && mi.dwItemData != 0)
			{
				// This is a submenu, if it can handle this ID then recurse to find correct state
				PopupMenu *mnu = (PopupMenu *)mi.dwItemData;
				if(mnu->canHandleID(id))
					return isMenuItemIDEnabled(mi.hSubMenu, id);
			}
		}
	}

	return false;
}

bool Win32Window::isAccelerator(const InputEventInfo &info)
{
	U32 virt;
	virt = FVIRTKEY;
	if(info.modifier & SI_SHIFT)
		virt |= FSHIFT;
	if(info.modifier & SI_CTRL)
		virt |= FCONTROL;
	if(info.modifier & SI_ALT)
		virt |= FALT;

	U8 keyCode = TranslateKeyCodeToOS(info.objInst);

	for(S32 i = 0;i < mWinAccelList.size();++i)
	{
		const ACCEL &accel = mWinAccelList[i];
		if(accel.key == keyCode && accel.fVirt == virt && isMenuItemIDEnabled(getMenuHandle(), accel.cmd))
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------

void Win32Window::addAccelerators(AcceleratorList &list)
{
	if(mAccelHandle)
	{
		DestroyAcceleratorTable(mAccelHandle);
		mAccelHandle = NULL;
	}

	for(AcceleratorList::iterator i = list.begin();i != list.end();++i)
	{
		addAccelerator(*i);
	}

	if(mWinAccelList.size() > 0)
		mAccelHandle = CreateAcceleratorTable(&mWinAccelList[0], mWinAccelList.size());
}

void Win32Window::removeAccelerators(AcceleratorList &list)
{
	if(mAccelHandle)
	{
		DestroyAcceleratorTable(mAccelHandle);
		mAccelHandle = NULL;
	}

	for(AcceleratorList::iterator i = list.begin();i != list.end();++i)
	{
		removeAccelerator(*i);
	}

	if(mWinAccelList.size() > 0)
		mAccelHandle = CreateAcceleratorTable(mWinAccelList.address(), mWinAccelList.size());
}

bool Win32Window::translateMessage(MSG &msg)
{
	if(mAccelHandle == NULL || mWindowHandle == NULL || !mEnableAccelerators)
		return false;

	int ret = TranslateAccelerator(getHWND(), mAccelHandle, &msg);
	return ret != 0;
}
