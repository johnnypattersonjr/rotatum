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

#include <Cocoa/Cocoa.h>
#include "platform/platformInput.h"
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#include "console/console.h"
#include "platform/threads/thread.h"

extern S32 TorqueInit(S32 argc, const char **argv);
extern bool TorqueTick();
extern S32 TorqueShutdown(S32 exitCode);

@interface MainLoopTimerHandler : NSObject
{
   U32 argc;
   const char** argv;
   NSTimeInterval interval;
   S32 exitCode;
}
   +(id)startTimerWithintervalMs:(U32)intervalMs argc:(U32)_argc argv:(const char**)_argv;
   -(void)firstFire:(NSTimer*)theTimer;
   -(void)fireTimer:(NSTimer*)theTimer;
@end
@implementation MainLoopTimerHandler
   -(void)firstFire:(NSTimer*)theTimer
   {
      exitCode = TorqueInit(argc, argv);
      if (!exitCode)
         [NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(fireTimer:) userInfo:nil repeats:YES];
      else
         exitCode = TorqueShutdown(exitCode);
   }
   -(void)fireTimer:(NSTimer*)theTimer
   {
      if(!TorqueTick())
      {
         exitCode = TorqueShutdown(exitCode);
         [theTimer invalidate];
         [NSApp setDelegate:nil];
         [NSApp terminate:self];
      }
   }
   +(id)startTimerWithintervalMs:(U32)intervalMs argc:(U32)_argc argv:(const char**)_argv
   {
      MainLoopTimerHandler* handler = [[[MainLoopTimerHandler alloc] init] autorelease];
      handler->argc = _argc;
      handler->argv = _argv;
      handler->interval = intervalMs / 1000.0; // interval in milliseconds
      [NSTimer scheduledTimerWithTimeInterval:handler->interval target:handler selector:@selector(firstFire:) userInfo:nil repeats:NO];
      return handler;
   }
@end

#pragma mark -

//-----------------------------------------------------------------------------
// main() - the real one - this is the actual program entry point.
//-----------------------------------------------------------------------------
S32 main(S32 argc, const char **argv)
{   
#ifdef DAE2DTS_TOOL
   S32 exitCode = TorqueInit(argc, argv);

   if (!exitCode)
      while (TorqueTick());

   return TorqueShutdown(exitCode);
#else
   NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

   // get command line and text file args, filter them
      
   // now, we prepare to hand off execution to torque & macosx.
   U32 appReturn = 0;         
   printf("installing torque main loop timer\n");
   [MainLoopTimerHandler startTimerWithintervalMs:1 argc:argc argv:argv];
   printf("starting NSApplicationMain\n");
   appReturn = NSApplicationMain(argc, argv);
   printf("NSApplicationMain exited\n");
   
   // shut down the engine
   
   [pool release];

   return appReturn;
#endif
}

#pragma mark ---- Init funcs  ----
//------------------------------------------------------------------------------
void Platform::init()
{
   // Set the platform variable for the scripts
   Con::setVariable( "$platform", "macos" );

   Input::init();
}

//------------------------------------------------------------------------------
void Platform::shutdown()
{
   Input::destroy();
}
