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

#include "shaderGenHelper.h"

#include "gfx/gfxDevice.h"
#include "materials/materialFeatureTypes.h"

Var* ShaderGenHelper::getInTexCoord(const char* name, const char* type, bool mapsToSampler, Vector<ShaderComponent*>& componentList)
{
	Var* texCoord = (Var*)LangElement::find(name);
	if (!texCoord)
	{
		ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(componentList[C_CONNECTOR]);
		texCoord = connectComp->getElement(RT_TEXCOORD);
		texCoord->setName(name);
		if (GFX->getAdapterType() == Direct3D9)
			texCoord->setStructName("IN");
		texCoord->setType(type);
		texCoord->mapsToSampler = mapsToSampler;
	}

	AssertFatal(dStrcmp(type, (const char*)texCoord->type) == 0, "ShaderGenHelper::getInTexCoord - Type mismatch!");

	return texCoord;
}

Var* ShaderGenHelper::getInViewToTangent(Vector<ShaderComponent*>& componentList)
{
	Var* viewToTangent = (Var*)LangElement::find("viewToTangent");
	if (!viewToTangent)
	{
		ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(componentList[C_CONNECTOR]);
		viewToTangent = connectComp->getElement(RT_TEXCOORD, 1, 3);
		viewToTangent->setName("viewToTangent");
		if (GFX->getAdapterType() == Direct3D9)
			viewToTangent->setStructName("IN");
		viewToTangent->setType("float3x3");
	}

	return viewToTangent;
}

Var* ShaderGenHelper::getInvWorldView(Vector<ShaderComponent*>& componentList, bool useInstancing, GFXVertexFormat* instancingFormat, MultiLine* meta)
{
	Var* viewToObj = (Var*)LangElement::find("viewToObj");
	if (viewToObj)
		return viewToObj;

	if (useInstancing)
	{
		Var* worldView = getWorldView(componentList, useInstancing, instancingFormat, meta);

		viewToObj = new Var;
		viewToObj->setType("float3x3");
		viewToObj->setName("viewToObj");

		// We just use transpose to convert the 3x3 portion 
		// of the world view transform into its inverse.

		meta->addStatement(new GenOp("   @ = transpose((float3x3)@); // Instancing!\r\n", new DecOp(viewToObj), worldView));
	}
	else
	{
		viewToObj = new Var;
		viewToObj->setType("float4x4");
		viewToObj->setName("viewToObj");
		viewToObj->uniform = true;
		viewToObj->constSortPos = cspPrimitive;
	}

	return viewToObj;
}

Var* ShaderGenHelper::getOutObjToTangentSpace(Vector<ShaderComponent*>& componentList, MultiLine* meta, const MaterialFeatureData& fd)
{
	Var* outObjToTangentSpace = (Var*)LangElement::find("objToTangentSpace");
	if (!outObjToTangentSpace)
		meta->addStatement(setupTexSpaceMat(componentList, &outObjToTangentSpace));

	return outObjToTangentSpace;
}

const char* ShaderGenHelper::getOutputTargetVarName(OutputTarget target) const
{
	const char* targName = "col";
	if (target != DefaultTarget)
	{
		targName = "col1";
		AssertFatal(target == RenderTarget1, "");
	}

	return targName;
}

Var* ShaderGenHelper::getOutViewToTangent(Vector<ShaderComponent*>& componentList, GFXVertexFormat* instancingFormat, MultiLine* meta, const MaterialFeatureData& fd)
{
	Var* viewToTangent = (Var*)LangElement::find("viewToTangent");
	if (viewToTangent)
		return viewToTangent;

	Var* texSpaceMat = getOutObjToTangentSpace(componentList, meta, fd);

	// send transform to pixel shader
	ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(componentList[C_CONNECTOR]);

	viewToTangent = connectComp->getElement(RT_TEXCOORD, 1, 3);
	viewToTangent->setName("viewToTangent");
	if (GFX->getAdapterType() == Direct3D9)
		viewToTangent->setStructName("OUT");
	viewToTangent->setType("float3x3");

	if (!fd.features[MFT_ParticleNormal])
	{
		// turn obj->tangent into world->tangent

		// Get the view->obj transform
		Var* viewToObj = getInvWorldView(componentList, fd.features[MFT_UseInstancing], instancingFormat, meta);

		// assign world->tangent transform
		if (GFX->getAdapterType() == Direct3D9)
		{
			meta->addStatement(new GenOp("   @ = mul(@, (float3x3)@);\r\n", viewToTangent, texSpaceMat, viewToObj));
		}
		else
		{
			// assign world->tangent transform
			meta->addStatement(new GenOp("   mat3 mat3ViewToObj;\r\n"));
			meta->addStatement(new GenOp("   mat3ViewToObj[0] = @[0].xyz;\r\n", viewToObj));
			meta->addStatement(new GenOp("   mat3ViewToObj[1] = @[1].xyz;\r\n", viewToObj));
			meta->addStatement(new GenOp("   mat3ViewToObj[2] = @[2].xyz;\r\n", viewToObj));
			meta->addStatement(new GenOp("   @ = @ * mat3ViewToObj;\r\n", new DecOp(viewToTangent), texSpaceMat));
		}
	}
	else
	{
		// Assume particle normal generation has set this up in the proper space
		meta->addStatement(new GenOp("   @ = @;\r\n", new DecOp(viewToTangent), texSpaceMat));
	}

	return viewToTangent;
}
