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

#include "core/util/tVector.h"
#include "gfx/gfxVertexFormat.h"
#include "materials/materialDefinition.h"
#include "materials/materialFeatureData.h"
#include "shaderGen/langElement.h"
#include "shaderGen/shaderComp.h"
#include "shaderGen/shaderOp.h"

enum OutputTarget
{
	DefaultTarget =   1 << 0,
	RenderTarget1 =   1 << 1,
	RenderTarget2 =   1 << 2,
	RenderTarget3 =   1 << 3,
};

class ShaderGenHelper
{
public:
	ShaderGenHelper() {}
	virtual ~ShaderGenHelper() {}

	/// Returns an input texture coord by name adding it
	/// to the input connector if it doesn't exist.
	Var* getInTexCoord(const char* name, const char* type, bool mapsToSampler, Vector<ShaderComponent*> &componentList);

	/// Returns the input "worldToTanget" space transform
	/// adding it to the input connector if it doesn't exist.
	Var* getInWorldToTangent(Vector<ShaderComponent*>& componentList);

	/// Returns the input normal map texture.
	Var* getNormalMapTex();

	/// Returns the "objToTangentSpace" transform or creates one if this
	/// is the first feature to need it.
	Var* getOutObjToTangentSpace(Vector<ShaderComponent*>& componentList, MultiLine* meta, const MaterialFeatureData& fd);

	/// Returns the existing output "outWorldToTangent" transform or
	/// creates one if this is the first feature to need it.
	Var* getOutWorldToTangent(Vector<ShaderComponent*>& componentList, GFXVertexFormat* instancingFormat, MultiLine* meta, const MaterialFeatureData& fd);

	/// Returns the input "viewToTangent" space transform
	/// adding it to the input connector if it doesn't exist.
	Var* getInViewToTangent(Vector<ShaderComponent*>& componentList);

	Var* getInvWorldView(Vector<ShaderComponent*>& componentList, bool useInstancing, GFXVertexFormat* instancingFormat, MultiLine* meta);

	/// Returns the name of output targer var.
	const char* getOutputTargetVarName(OutputTarget target = DefaultTarget) const;

	/// Returns the existing output "viewToTangent" transform or
	/// creates one if this is the first feature to need it.
	Var* getOutViewToTangent(Vector<ShaderComponent*>& componentList, GFXVertexFormat* instancingFormat, MultiLine* meta, const MaterialFeatureData& fd);

	virtual Var* addOutDetailTexCoord(Vector<ShaderComponent*>& componentList, MultiLine* meta, bool useTexAnim) = 0;

	virtual Var* addOutVpos(MultiLine* meta, Vector<ShaderComponent*>& componentList) = 0;

	/// Helper function for applying the color to shader output.
	///
	/// @param elem         The rbg or rgba color to assign.
	///
	/// @param blend        The type of blending to perform.
	///
	/// @param lerpElem     The optional lerp parameter when doing a LerpAlpha blend,
	///                     if not set then the elem is used.
	///
	virtual LangElement* assignColor(LangElement* elem, Material::BlendOp blend, LangElement* lerpElem = NULL, OutputTarget outputTarget = DefaultTarget) = 0;

	/// Expand and assign a normal map. This takes care of compressed normal maps as well.
	virtual LangElement* expandNormalMap(LangElement* sampleNormalOp, LangElement* normalDecl, LangElement* normalVar, const MaterialFeatureData& fd, S32 processIndex) = 0;

	virtual Var* getInVpos(MultiLine* meta, Vector<ShaderComponent*>& componentList) = 0;

	virtual Var* getObjTrans(Vector<ShaderComponent*>& componentList, bool useInstancing, GFXVertexFormat* instancingFormat, MultiLine* meta) = 0;

	virtual Var* getOutTexCoord(const char* name, const char* type, bool mapsToSampler, bool useTexAnim, MultiLine* meta, Vector<ShaderComponent*>& componentList) = 0;

	/// Get the incoming base texture coords - useful for bumpmap and detail maps
	virtual Var* getVertTexCoord(const String &name) = 0;

	virtual Var* getWorldView(Vector<ShaderComponent*>& componentList, bool useInstancing, GFXVertexFormat* instancingFormat, MultiLine* meta) = 0;

	/// Set up a texture space matrix - to pass into pixel shader
	virtual LangElement* setupTexSpaceMat(Vector<ShaderComponent*>& componentList, Var** texSpaceMat) = 0;
};
