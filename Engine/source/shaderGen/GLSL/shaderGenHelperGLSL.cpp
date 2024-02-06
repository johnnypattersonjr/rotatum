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

Var* ShaderGenHelperGLSL::getObjTrans(Vector<ShaderComponent*>& componentList, bool useInstancing, GFXVertexFormat* instancingFormat, MultiLine* meta)
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

Var* ShaderGenHelperGLSL::getWorldView(Vector<ShaderComponent*>& componentList, bool useInstancing, GFXVertexFormat* instancingFormat, MultiLine* meta)
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
