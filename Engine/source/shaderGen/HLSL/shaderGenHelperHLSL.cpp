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
#include "materials/materialFeatureTypes.h"

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

LangElement* ShaderGenHelperHLSL::assignColor(LangElement* elem, Material::BlendOp blend, LangElement* lerpElem,
                                              OutputTarget outputTarget)
{
	// search for color var
	Var* color = (Var*)LangElement::find(getOutputTargetVarName(outputTarget));

	if ( !color )
	{
		// create color var
		color = new Var;
		color->setType("fragout");
		color->setName(getOutputTargetVarName(outputTarget));
		color->setStructName("OUT");

		return new GenOp("@ = @", color, elem);
	}

	LangElement* assign;

	switch (blend)
	{
	case Material::Add:
		assign = new GenOp("@ += @", color, elem);
		break;

	case Material::Sub:
		assign = new GenOp("@ -= @", color, elem);
		break;

	case Material::Mul:
		assign = new GenOp("@ *= @", color, elem);
		break;

	case Material::AddAlpha:
		assign = new GenOp("@ += @ * @.a", color, elem, elem);
		break;

	case Material::LerpAlpha:
		if (!lerpElem)
			lerpElem = elem;
		assign = new GenOp("@.rgb = lerp(@.rgb, (@).rgb, (@).a)", color, color, elem, lerpElem);
		break;

	case Material::ToneMap:
		assign = new GenOp("@ = 1.0 - exp(-1.0 * @ * @)", color, color, elem);
		break;

	default:
		AssertFatal(false, "Unrecognized color blendOp");
		// Fallthru

	case Material::None:
		assign = new GenOp("@ = @", color, elem);
		break;
	}

	return assign;
}

LangElement* ShaderGenHelperHLSL::expandNormalMap(LangElement* sampleNormalOp, LangElement* normalDecl, LangElement* normalVar,
                                                  const MaterialFeatureData& fd, S32 processIndex)
{
	MultiLine* meta = new MultiLine;

	if (fd.features.hasFeature(MFT_IsDXTnm, processIndex))
	{
		if (fd.features[MFT_ImposterVert])
		{
			// The imposter system uses object space normals and
			// encodes them with the z axis in the alpha component.
			meta->addStatement(new GenOp("   @ = float4(normalize( @.xyw * 2.0 - 1.0), 0.0); // Obj DXTnm\r\n", normalDecl, sampleNormalOp));
		}
		else
		{
			// DXT Swizzle trick
			meta->addStatement(new GenOp("   @ = float4(@.ag * 2.0 - 1.0, 0.0, 0.0); // DXTnm\r\n", normalDecl, sampleNormalOp));
			meta->addStatement(new GenOp("   @.z = sqrt(1.0 - dot(@.xy, @.xy)); // DXTnm\r\n", normalVar, normalVar, normalVar));
		}
	}
	else
	{
		meta->addStatement(new GenOp("   @ = @;\r\n", normalDecl, sampleNormalOp));
		meta->addStatement(new GenOp("   @.xyz = @.xyz * 2.0 - 1.0;\r\n", normalVar, normalVar));
	}

	return meta;
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

Var* ShaderGenHelperHLSL::getVertTexCoord(const String &name)
{
	Var* inTex = NULL;

	for (U32 i = 0; i < LangElement::elementList.size(); i++)
	{
		if (!dStrcmp((char*)LangElement::elementList[i]->name, name.c_str()))
		{
			inTex = dynamic_cast<Var*>(LangElement::elementList[i]);
			if (inTex)
			{
				// NOTE: This used to do this check...
				//
				// dStrcmp((char*)inTex->structName, "IN")
				//
				// ... to ensure that the var was from the input
				// vertex structure, but this kept some features
				// ( ie. imposter vert ) from decoding their own
				// coords for other features to use.
				//
				// If we run into issues with collisions between
				// IN vars and local vars we may need to revise.

				break;
			}
		}
	}

	return inTex;
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

LangElement* ShaderGenHelperHLSL::setupTexSpaceMat(Vector<ShaderComponent*>& componentList, Var** texSpaceMat)
{
	Var* N = (Var*)LangElement::find("normal");
	Var* B = (Var*)LangElement::find("B");
	Var* T = (Var*)LangElement::find("T");

	Var* tangentW = (Var*)LangElement::find("tangentW");

	// setup matrix var
	*texSpaceMat = new Var;
	(*texSpaceMat)->setType("float3x3");
	(*texSpaceMat)->setName("objToTangentSpace");

	MultiLine* meta = new MultiLine;
	meta->addStatement(new GenOp("   @;\r\n", new DecOp(*texSpaceMat)));

	// Protect against missing normal and tangent.
	if (!N || !T)
	{
		meta->addStatement(new GenOp("   @[0] = float3(1, 0, 0); @[1] = float3(0, 1, 0); @[2] = float3(0, 0, 1);\r\n", *texSpaceMat, *texSpaceMat, *texSpaceMat));
		return meta;
	}

	meta->addStatement(new GenOp("   @[0] = @;\r\n", *texSpaceMat, T));
	if (B)
	{
		meta->addStatement(new GenOp("   @[1] = @;\r\n", *texSpaceMat, B));
	}
	else
	{
		if (dStricmp((char*)T->type, "float4") == 0)
			meta->addStatement(new GenOp("   @[1] = cross(@, normalize(@)) * @.w;\r\n", *texSpaceMat, T, N, T));
		else if(tangentW)
			meta->addStatement(new GenOp("   @[1] = cross(@, normalize(@)) * @;\r\n", *texSpaceMat, T, N, tangentW));
		else
			meta->addStatement(new GenOp("   @[1] = cross(@, normalize(@));\r\n", *texSpaceMat, T, N));
	}
	meta->addStatement(new GenOp("   @[2] = normalize(@);\r\n", *texSpaceMat, N));

	return meta;
}
