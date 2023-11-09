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

#include "platform/platform.h"
#include "gfx/gl/gfxGLOcclusionQuery.h"
#include "gfx/gl/ggl/ggl.h"

GFXGLOcclusionQuery::GFXGLOcclusionQuery(GFXDevice* device)
   : GFXOcclusionQuery(device)
   , mQuery(0)
   , mActive(false)
   , mResultPending(false)
   , mLastResult(1)
{
   glGenQueries(1, &mQuery);
}

GFXGLOcclusionQuery::~GFXGLOcclusionQuery()
{
   glDeleteQueries(1, &mQuery);
}

bool GFXGLOcclusionQuery::begin()
{
   if (mActive)
      return false;

   glBeginQuery(GL_SAMPLES_PASSED, mQuery);
   mActive = true;
   return true;
}

void GFXGLOcclusionQuery::end()
{
   if (!mActive)
      return;

   glEndQuery(GL_SAMPLES_PASSED);
   mActive = false;
   mResultPending = true;
}

GFXOcclusionQuery::OcclusionQueryStatus GFXGLOcclusionQuery::getStatus(bool block, U32* data)
{
   // If this ever shows up near the top of a profile 
   // then your system is GPU bound.
   PROFILE_SCOPE(GFXGLOcclusionQuery_getStatus);

   AssertFatal(!mActive, "Cannot get status of active query.");

   if (!mResultPending)
      return mLastResult > 0 ? NotOccluded : Occluded;

   GLint queryDone = false;
   
   if (block)
      queryDone = true;
   else
      glGetQueryObjectiv(mQuery, GL_QUERY_RESULT_AVAILABLE, &queryDone);
   
   if (queryDone)
      glGetQueryObjectiv(mQuery, GL_QUERY_RESULT, &mLastResult);
   else
      return Waiting;

   mResultPending = false;

   if (data)
      *data = mLastResult;
   
   return mLastResult > 0 ? NotOccluded : Occluded;
}

void GFXGLOcclusionQuery::zombify()
{
   glDeleteQueries(1, &mQuery);
   mActive = false;
   mResultPending = false;
   mQuery = 0;
}

void GFXGLOcclusionQuery::resurrect()
{
   glGenQueries(1, &mQuery);
}

const String GFXGLOcclusionQuery::describeSelf() const
{
   // We've got nothing
   return String();
}
