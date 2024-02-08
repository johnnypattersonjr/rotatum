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

#pragma once

#include "shaderGen/shaderGenHelper.h"

class ShaderGenHelperGLSL : public ShaderGenHelper
{
public:
	virtual Var* addOutDetailTexCoord(Vector<ShaderComponent*>& componentList, MultiLine* meta, bool useTexAnim);
	virtual Var* addOutVpos(MultiLine* meta, Vector<ShaderComponent*>& componentList);
	virtual LangElement* assignColor(LangElement* elem, Material::BlendOp blend, LangElement* lerpElem = NULL, OutputTarget outputTarget = DefaultTarget);
	virtual LangElement* expandNormalMap(LangElement* sampleNormalOp, LangElement* normalDecl, LangElement* normalVar, const MaterialFeatureData& fd, S32 processIndex);
	virtual Var* getInVpos(MultiLine* meta, Vector<ShaderComponent*>& componentList);
	virtual Var* getObjTrans(Vector<ShaderComponent*>& componentList, bool useInstancing, GFXVertexFormat* instancingFormat, MultiLine* meta);
	virtual Var* getOutTexCoord(const char* name, const char* type, bool mapsToSampler, bool useTexAnim, MultiLine* meta, Vector<ShaderComponent*>& componentList);
	virtual Var* getVertTexCoord(const String &name);
	virtual Var* getWorldView(Vector<ShaderComponent*>& componentList, bool useInstancing, GFXVertexFormat* instancingFormat, MultiLine* meta);
	virtual LangElement* setupTexSpaceMat(Vector<ShaderComponent*>& componentList, Var** texSpaceMat);
};
