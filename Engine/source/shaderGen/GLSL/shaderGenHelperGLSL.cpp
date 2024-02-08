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

#include "shaderGenHelperGLSL.h"

#include "core/strings/stringFunctions.h"
#include "materials/materialFeatureTypes.h"

Var* ShaderGenHelperGLSL::addOutDetailTexCoord(Vector<ShaderComponent*>& componentList, MultiLine* meta, bool useTexAnim)
{
	// Check if its already added.
	Var*outTex = (Var*)LangElement::find("detCoord");
	if (outTex)
		return outTex;

	// Grab incoming texture coords.
	Var* inTex = getVertTexCoord("vert_texCoord");

	// create detail variable
	Var* detScale = new Var;
	detScale->setType("vec2");
	detScale->setName("detailScale");
	detScale->uniform = true;
	detScale->constSortPos = cspPotentialPrimitive;

	// grab connector texcoord register
	ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(componentList[C_CONNECTOR]);
	outTex = connectComp->getElement(RT_TEXCOORD);
	outTex->setName("detCoord");
	outTex->setType("vec2");
	outTex->mapsToSampler = true;

	if (useTexAnim)
	{
		inTex->setType("vec4");

		// Find or create the texture matrix.
		Var* texMat = (Var*)LangElement::find("texMat");
		if (!texMat)
		{
			texMat = new Var;
			texMat->setType("mat4x4");
			texMat->setName("texMat");
			texMat->uniform = true;
			texMat->constSortPos = cspPass;
		}

		meta->addStatement(new GenOp("   @ = (@ * @) * @;\r\n", outTex, texMat, inTex, detScale));
	}
	else
	{
		// setup output to mul texCoord by detail scale
		meta->addStatement(new GenOp("   @ = @ * @;\r\n", outTex, inTex, detScale));
	}

	return outTex;
}

Var* ShaderGenHelperGLSL::addOutVpos(MultiLine* meta, Vector<ShaderComponent*>& componentList)
{
	Var* ssPos = (Var*)LangElement::find("screenspacePos");
	if (ssPos)
		return ssPos;

	ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(componentList[C_CONNECTOR]);

	ssPos = connectComp->getElement(RT_TEXCOORD);
	ssPos->setName("screenspacePos");
	ssPos->setType("vec4");

	meta->addStatement(new GenOp("   @ = gl_Position;\r\n", ssPos));

	return ssPos;
}

LangElement* ShaderGenHelperGLSL::assignColor(LangElement* elem, Material::BlendOp blend, LangElement* lerpElem,
                                              OutputTarget outputTarget)
{
	// search for color var
	Var *color = (Var*)LangElement::find(getOutputTargetVarName(outputTarget));

	if (!color)
	{
		// create color var
		color = new Var;
		color->setName(getOutputTargetVarName(outputTarget));
		color->setType("vec4");

		return new GenOp("@ = @", new DecOp(color), elem);
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
		assign = new GenOp("@.rgb = mix(@.rgb, (@).rgb, (@).a)", color, elem, color, lerpElem);
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

LangElement* ShaderGenHelperGLSL::expandNormalMap(LangElement* sampleNormalOp, LangElement* normalDecl, LangElement* normalVar,
                                                  const MaterialFeatureData& fd, S32 processIndex)
{
	MultiLine* meta = new MultiLine;

	if (fd.features.hasFeature(MFT_IsDXTnm, processIndex))
	{
		// DXT Swizzle trick
		meta->addStatement(new GenOp("   @ = vec4(@.ag * 2.0 - 1.0, 0.0, 0.0);  // DXTnm\r\n", normalDecl, sampleNormalOp));
		meta->addStatement(new GenOp("   @.z = sqrt(1.0 - dot(@.xy, @.xy));  // DXTnm\r\n", normalVar, normalVar, normalVar));
	}
	else
	{
		meta->addStatement(new GenOp( "   @ = @;\r\n", normalDecl, sampleNormalOp));
		meta->addStatement(new GenOp( "   @.xyz = @.xyz * 2.0 - 1.0;\r\n", normalVar, normalVar));
	}

	return meta;
}

Var* ShaderGenHelperGLSL::getInVpos(MultiLine* meta, Vector<ShaderComponent*>& componentList)
{
	Var* ssPos = (Var*)LangElement::find("screenspacePos");
	if (ssPos)
		return ssPos;

	ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(componentList[C_CONNECTOR]);

	ssPos = connectComp->getElement(RT_TEXCOORD);
	ssPos->setName("screenspacePos");
	ssPos->setType("vec4");

	return ssPos;
}

Var* ShaderGenHelperGLSL::getObjTrans(Vector<ShaderComponent*>& componentList, bool useInstancing,
                                      GFXVertexFormat* instancingFormat, MultiLine* meta)
{
	Var* objTrans = (Var*)LangElement::find("objTrans");
	if (objTrans)
		return objTrans;

	objTrans = new Var;
	objTrans->setType("mat4x4");
	objTrans->setName("objTrans");
	objTrans->uniform = true;
	objTrans->constSortPos = cspPrimitive;

	return objTrans;
}

Var* ShaderGenHelperGLSL::getOutTexCoord(const char* name, const char* type, bool mapsToSampler, bool useTexAnim, MultiLine* meta,
                                         Vector<ShaderComponent*>& componentList)
{
	Var* texCoord = (Var*)LangElement::find(name);
	if (!texCoord)
	{
		String vertTexCoordName = String::ToString("vert_%s", name);
		Var* inTex = getVertTexCoord(vertTexCoordName);
		AssertFatal(inTex, "ShaderGenHelperGLSL::getOutTexCoord - Unknown vertex input coord!");

		ShaderConnector* connectComp = dynamic_cast<ShaderConnector*>(componentList[C_CONNECTOR]);

		texCoord = connectComp->getElement(RT_TEXCOORD);
		texCoord->setName(name);
		texCoord->setType(type);
		texCoord->mapsToSampler = mapsToSampler;

		if (useTexAnim)
		{
			inTex->setType("vec4");

			// create texture mat var
			Var *texMat = new Var;
			texMat->setType("mat4");
			texMat->setName("texMat");
			texMat->uniform = true;
			texMat->constSortPos = cspPass;

			// Statement allows for casting of different types which
			// eliminates vector truncation problems.
			String statement = String::ToString("   @ = %s(@ * @);\r\n", type);
			meta->addStatement(new GenOp(statement, texCoord, texMat, inTex));
		}
		else
		{
			// Statement allows for casting of different types which
			// eliminates vector truncation problems.
			String statement = String::ToString("   @ = %s(@);\r\n", type);
			meta->addStatement(new GenOp(statement, texCoord, inTex));
		}
	}

	AssertFatal(dStrcmp(type, (const char*)texCoord->type) == 0, "ShaderGenHelperGLSL::getOutTexCoord - Type mismatch!");

	return texCoord;
}

Var* ShaderGenHelperGLSL::getVertTexCoord(const String &name)
{
	Var* inTex = NULL;

	for (U32 i = 0; i < LangElement::elementList.size(); i++)
	{
		if (!dStrcmp((char*)LangElement::elementList[i]->name, name.c_str()))
		{
			inTex = dynamic_cast<Var*>(LangElement::elementList[i]);
			if (inTex)
			{
				break;
			}
		}
	}

	return inTex;
}

Var* ShaderGenHelperGLSL::getWorldView(Vector<ShaderComponent*>& componentList, bool useInstancing,
                                       GFXVertexFormat* instancingFormat, MultiLine* meta)
{
	Var* worldView = (Var*)LangElement::find("worldViewOnly");
	if (worldView)
		return worldView;

	worldView = new Var;
	worldView->setType("mat4x4");
	worldView->setName("worldViewOnly");
	worldView->uniform = true;
	worldView->constSortPos = cspPrimitive;

	return worldView;
}

LangElement* ShaderGenHelperGLSL::setupTexSpaceMat(Vector<ShaderComponent*>& componentList, Var** texSpaceMat)
{
	Var* N = (Var*)LangElement::find("normal");
	Var* B = (Var*)LangElement::find("B");
	Var* T = (Var*)LangElement::find("T");

	// setup matrix var
	*texSpaceMat = new Var;
	(*texSpaceMat)->setType("mat3");
	(*texSpaceMat)->setName("objToTangentSpace");

	MultiLine* meta = new MultiLine;

	// Recreate the binormal if we don't have one.
	if (!B)
	{
		B = new Var;
		B->setType("vec3");
		B->setName("B");
		meta->addStatement(new GenOp("   @ = cross(@, normalize(@));\r\n", new DecOp(B), T, N));
	}

	meta->addStatement(new GenOp("   @;\r\n", new DecOp(*texSpaceMat)));
	// meta->addStatement(new GenOp( "   @[0] = vec3(@.x, @.x, normalize(@).x);\r\n", *texSpaceMat, T, B, N));
	// meta->addStatement(new GenOp( "   @[1] = vec3(@.y, @.y, normalize(@).y);\r\n", *texSpaceMat, T, B, N));
	// meta->addStatement(new GenOp( "   @[2] = vec3(@.z, @.z, normalize(@).z);\r\n", *texSpaceMat, T, B, N));
	meta->addStatement(new GenOp( "   @[0] = @;\r\n", *texSpaceMat, T));
	meta->addStatement(new GenOp( "   @[1] = @ * tcTANGENTW;\r\n", *texSpaceMat, B));
	meta->addStatement(new GenOp( "   @[2] = normalize(@);\r\n", *texSpaceMat, N));

	return meta;
}
