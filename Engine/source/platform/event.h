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
/// @file
/// Library-wide input events
///
/// All external events are converted into system events, which are defined
/// in this file.

///
#ifndef _EVENT_H_
#define _EVENT_H_

#include "platform/types.h"
#include "core/util/journal/journaledSignal.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

/// @defgroup input_constants Input system constants
/// @{

/// Input event constants:
enum InputObjectInstances
{
   KEY_NULL          = 0x000,     ///< Invalid KeyCode
   KEY_HELP          = GLFW_KEY_LAST + 1,
   KEY_SEPARATOR,
   KEY_OEM_102,
   KEY_ANYKEY,
   KEY_LAST = KEY_ANYKEY,

   /// Joystick event codes.
   SI_XPOV,
   SI_YPOV,
   SI_UPOV,
   SI_DPOV,
   SI_LPOV,
   SI_RPOV,
   SI_XAXIS,
   SI_YAXIS,
   SI_ZAXIS,
   SI_RXAXIS,
   SI_RYAXIS,
   SI_RZAXIS,
   SI_SLIDER,
   SI_XPOV2,
   SI_YPOV2,
   SI_UPOV2,
   SI_DPOV2,
   SI_LPOV2,
   SI_RPOV2,

   XI_CONNECT,
   XI_THUMBLX,
   XI_THUMBLY,
   XI_THUMBRX,
   XI_THUMBRY,
   XI_LEFT_TRIGGER,
   XI_RIGHT_TRIGGER,

   /*XI_DPAD_UP,
   XI_DPAD_DOWN,
   XI_DPAD_LEFT,
   XI_DPAD_RIGHT,*/
   
   XI_START,
   XI_BACK,
   XI_LEFT_THUMB,
   XI_RIGHT_THUMB,
   XI_LEFT_SHOULDER,
   XI_RIGHT_SHOULDER,

   XI_A,
   XI_B,
   XI_X,
   XI_Y,
};

/// Input device types
enum InputDeviceTypes
{
   UnknownDeviceType,
   MouseDeviceType,
   KeyboardDeviceType,
   JoystickDeviceType,
   GamepadDeviceType,
   XInputDeviceType,

   NUM_INPUT_DEVICE_TYPES
};

/// Device Event Action Types
enum InputActionType
{
   /// An axis moved.
   SI_MOVE    = 0x03,
};

///Device Event Types
enum InputEventType
{
   SI_UNKNOWN = 0x01,
   SI_BUTTON  = 0x02,
   SI_POV     = 0x03,
   SI_AXIS    = 0x04,
   SI_KEY     = 0x0A,
};

/// Wildcard match used by the input system.
#define SI_ANY       0xff

// Modifier Keys
enum InputModifiers
{
   /// shift and ctrl are the same between platforms.
   SI_LSHIFT = BIT(0),
   SI_RSHIFT = BIT(1),
   SI_SHIFT  = (SI_LSHIFT|SI_RSHIFT),
   SI_LCTRL  = BIT(2),
   SI_RCTRL  = BIT(3),
   SI_CTRL   = (SI_LCTRL|SI_RCTRL),

   /// win altkey, mapped to mac cmdkey.
   SI_LALT = BIT(4),
   SI_RALT = BIT(5),
   SI_ALT = (SI_LALT|SI_RALT),

   /// mac optionkey
   SI_MAC_LOPT  = BIT(6),
   SI_MAC_ROPT  = BIT(7),
   SI_MAC_OPT   = (SI_MAC_LOPT|SI_MAC_ROPT),

   /// modifier keys used for common operations
#if defined(TORQUE_OS_MAC)
   SI_COPYPASTE = SI_ALT,
   SI_MULTISELECT = SI_ALT,
   SI_RANGESELECT = SI_SHIFT,
   SI_PRIMARY_ALT = SI_MAC_OPT,  ///< Primary key used for toggling into alternates of commands.
   SI_PRIMARY_CTRL = SI_ALT,     ///< Primary key used for triggering commands.
#else
   SI_COPYPASTE = SI_CTRL,
   SI_MULTISELECT = SI_CTRL,
   SI_RANGESELECT = SI_SHIFT,
   SI_PRIMARY_ALT = SI_ALT,
   SI_PRIMARY_CTRL = SI_CTRL,
#endif
   /// modfier key used in conjunction w/ arrow keys to move cursor to next word
#if defined(TORQUE_OS_MAC)
   SI_WORDJUMP = SI_MAC_OPT,
#else
   SI_WORDJUMP = SI_CTRL,
#endif
   /// modifier key used in conjunction w/ arrow keys to move cursor to beginning / end of line
   SI_LINEJUMP = SI_ALT,

   /// modifier key used in conjunction w/ home & end to jump to the top or bottom of a document
#if defined(TORQUE_OS_MAC)
   SI_DOCJUMP = SI_ANY,
#else
   SI_DOCJUMP = SI_CTRL,
#endif
};

/// @}


/// Generic input event.
struct InputEventInfo
{
   InputEventInfo()
   {
      deviceInst = 0;
      fValue     = 0.f;
      deviceType = (InputDeviceTypes)0;
      objType    = (InputEventType)0;
      ascii      = 0;
      objInst    = 0;
      action     = 0;
      modifier   = 0;
   }

   /// Device instance: joystick0, joystick1, etc
   U32 deviceInst;

   /// Value ranges from -1.0 to 1.0
   F32 fValue;

   U8                   action;
   InputDeviceTypes     deviceType;
   InputEventType       objType;
   S32                  objInst;

   /// ASCII character code if this is a keyboard event.
   U16 ascii;
   
   /// Modifiers to action: SI_LSHIFT, SI_LCTRL, etc.
   U8 modifier;

   inline void postToSignal(InputEvent &ie)
   {
      ie.trigger(deviceInst, fValue, deviceType, objType, ascii, objInst, action, modifier);
   }
};


#endif
