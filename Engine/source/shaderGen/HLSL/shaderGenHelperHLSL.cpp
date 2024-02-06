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

#include "shaderGenHelperHLSL.h"

#include "gfx/gfxDevice.h"

Var* ShaderGenHelperHLSL::addOutVpos(MultiLine* meta, Vector<ShaderComponent*>& componentList)
{
	// Nothing to do if we're on SM 3.0... we use the real vpos.
	if (GFX->getPixelShaderVersion() >= 3.0f)
		return NULL;

	// For SM 2.x we need to generate the vpos in the vertex shader
	// and pass it as a texture coord to the pixel shader.

	Var* outVpos = (Var*)LangElement::find("outVpos");
	if (!outVpos)
	{
		ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(componentList[C_CONNECTOR]);

		outVpos = connectComp->getElement(RT_TEXCOORD);
		outVpos->setName("outVpos");
		outVpos->setStructName("OUT");
		outVpos->setType("float4");
		outVpos->mapsToSampler = false;

		Var* outPosition = (Var*)LangElement::find("hpos");
		AssertFatal(outPosition, "ShaderGenHelperHLSL::addOutVpos - Didn't find the output position.");

		meta->addStatement(new GenOp("   @ = @;\r\n", outVpos, outPosition));
	}

	return outVpos;
}

Var* ShaderGenHelperHLSL::getInVpos(MultiLine* meta, Vector<ShaderComponent*>& componentList)
{
	Var* inVpos = (Var*)LangElement::find("vpos");
	if (inVpos)
		return inVpos;

	ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(componentList[C_CONNECTOR]);

	if (GFX->getPixelShaderVersion() >= 3.0f)
	{
		inVpos = connectComp->getElement(RT_VPOS);
		inVpos->setName("vpos");
		inVpos->setStructName("IN");
		inVpos->setType("float2");
		return inVpos;
	}

	inVpos = connectComp->getElement(RT_TEXCOORD);
	inVpos->setName("inVpos");
	inVpos->setStructName("IN");
	inVpos->setType("float4");

	Var* vpos = new Var("vpos", "float2");
	meta->addStatement(new GenOp("   @ = @.xy / @.w;\r\n", new DecOp(vpos), inVpos, inVpos));

	return vpos;
}

Var* ShaderGenHelperHLSL::getObjTrans(Vector<ShaderComponent*>& componentList, bool useInstancing, GFXVertexFormat* instancingFormat, MultiLine* meta)
{
	Var* objTrans = (Var*)LangElement::find("objTrans");
	if (objTrans)
		return objTrans;

	if (useInstancing)
	{
		ShaderConnector* vertStruct = dynamic_cast<ShaderConnector*>(componentList[C_VERT_STRUCT]);
		Var* instObjTrans = vertStruct->getElement(RT_TEXCOORD, 4, 4);
		instObjTrans->setStructName("IN");
		instObjTrans->setName("inst_objectTrans");

		instancingFormat->addElement("objTrans", GFXDeclType_Float4, instObjTrans->constNum + 0);
		instancingFormat->addElement("objTrans", GFXDeclType_Float4, instObjTrans->constNum + 1);
		instancingFormat->addElement("objTrans", GFXDeclType_Float4, instObjTrans->constNum + 2);
		instancingFormat->addElement("objTrans", GFXDeclType_Float4, instObjTrans->constNum + 3);

		objTrans = new Var;
		objTrans->setType("float4x4");
		objTrans->setName("objTrans");
		meta->addStatement(new GenOp("   @ = { // Instancing!\r\n", new DecOp(objTrans), instObjTrans));
		meta->addStatement(new GenOp("      @[0],\r\n", instObjTrans));
		meta->addStatement(new GenOp("      @[1],\r\n", instObjTrans));
		meta->addStatement(new GenOp("      @[2],\r\n",instObjTrans));
		meta->addStatement(new GenOp("      @[3] };\r\n", instObjTrans));
	}
	else
	{
		objTrans = new Var;
		objTrans->setType("float4x4");
		objTrans->setName("objTrans");
		objTrans->uniform = true;
		objTrans->constSortPos = cspPrimitive;
	}

	return objTrans;
}

Var* ShaderGenHelperHLSL::getWorldView(Vector<ShaderComponent*>& componentList, bool useInstancing, GFXVertexFormat* instancingFormat, MultiLine* meta)
{
	Var* worldView = (Var*)LangElement::find("worldViewOnly");
	if (worldView)
		return worldView;

	if (useInstancing)
	{
		Var* objTrans = getObjTrans(componentList, useInstancing, instancingFormat, meta);

		Var* worldToCamera = (Var*)LangElement::find("worldToCamera");
		if (!worldToCamera)
		{
			worldToCamera = new Var;
			worldToCamera->setType("float4x4");
			worldToCamera->setName("worldToCamera");
			worldToCamera->uniform = true;
			worldToCamera->constSortPos = cspPass;
		}

		worldView = new Var;
		worldView->setType("float4x4");
		worldView->setName("worldViewOnly");

		meta->addStatement(new GenOp("   @ = mul( @, @ ); // Instancing!\r\n", new DecOp(worldView), worldToCamera, objTrans));
	}
	else
	{
		worldView = new Var;
		worldView->setType("float4x4");
		worldView->setName("worldViewOnly");
		worldView->uniform = true;
		worldView->constSortPos = cspPrimitive;
	}

	return worldView;
}
