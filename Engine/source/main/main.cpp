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
#include "app/mainLoop.h"
#include "T3D/gameFunctions.h"

// Entry point for your game.
//
// This is build by default using the "StandardMainLoop" toolkit. Feel free
// to bring code over directly as you need to modify or extend things. You
// will need to merge against future changes to the SML code if you do this.
S32 TorqueMain(S32 argc, const char **argv)
{
   // Some handy debugging code:
   //   if (argc == 1) {
   //      static const char* argvFake[] = { "dtest.exe", "-jload", "test.jrn" };
   //      argc = 3;
   //      argv = argvFake;
   //   }

   //   Memory::enableLogging("testMem.log");
   //   Memory::setBreakAlloc(104717);

   // Initialize the subsystems.
   StandardMainLoop::init();

   // Handle any command line args.
   if(!StandardMainLoop::handleCommandLine(argc, argv))
   {
      Platform::AlertOK("Error", "Failed to initialize game, shutting down.");

      return 1;
   }

   // Main loop
   while(StandardMainLoop::doMainLoop());

   // Clean everything up.
   StandardMainLoop::shutdown();

   // Do we need to restart?
   if( StandardMainLoop::requiresRestart() )
      Platform::restartInstance();

   // Return.
   return 0;
}
