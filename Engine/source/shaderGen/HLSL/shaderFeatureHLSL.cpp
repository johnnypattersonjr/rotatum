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
#include "shaderGen/HLSL/shaderFeatureHLSL.h"

#include "shaderGen/langElement.h"
#include "shaderGen/shaderOp.h"
#include "shaderGen/shaderGenVars.h"
#include "gfx/gfxDevice.h"
#include "materials/matInstance.h"
#include "materials/processedMaterial.h"
#include "materials/materialFeatureTypes.h"
#include "core/util/autoPtr.h"

#include "lighting/advanced/advancedLightBinManager.h"

ShaderFeatureHLSL::ShaderFeatureHLSL()
{
   output = NULL;
}

Var* ShaderFeatureHLSL::getInColor( const char *name,
                                    const char *type,
                                    Vector<ShaderComponent*> &componentList )
{
   Var *inColor = (Var*)LangElement::find( name );
   if ( !inColor )
   {
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector*>( componentList[C_CONNECTOR] );
      inColor = connectComp->getElement( RT_COLOR );
      inColor->setName( name );
      inColor->setStructName( "IN" );
      inColor->setType( type );
   }

   AssertFatal( dStrcmp( type, (const char*)inColor->type ) == 0, 
      "ShaderFeatureHLSL::getInColor - Type mismatch!" );

   return inColor;
}

Var* ShaderFeatureHLSL::getModelView(  Vector<ShaderComponent*> &componentList,                                       
                                       bool useInstancing,
                                       MultiLine *meta )
{
   Var *modelview = (Var*)LangElement::find( "modelview" );
   if ( modelview )
      return modelview;

   if ( useInstancing )
   {
      Var *objTrans = sHelper->getObjTrans( componentList, useInstancing, mInstancingFormat, meta );

      Var *viewProj = (Var*)LangElement::find( "viewProj" );
      if ( !viewProj )
      {
         viewProj = new Var;
         viewProj->setType( "float4x4" );
         viewProj->setName( "viewProj" );
         viewProj->uniform = true;
         viewProj->constSortPos = cspPass;        
      }

      modelview = new Var;
      modelview->setType( "float4x4" );
      modelview->setName( "modelview" );
      meta->addStatement( new GenOp( "   @ = mul( @, @ ); // Instancing!\r\n", new DecOp( modelview ), viewProj, objTrans ) );
   }
   else
   {
      modelview = new Var;
      modelview->setType( "float4x4" );
      modelview->setName( "modelview" );
      modelview->uniform = true;
      modelview->constSortPos = cspPrimitive;   
   }

   return modelview;
}

void ShaderFeatureHLSL::getWsPosition( Vector<ShaderComponent*> &componentList,                                       
                                       bool useInstancing,
                                       MultiLine *meta,
                                       LangElement *wsPosition )
{
   Var *inPosition = (Var*)LangElement::find( "wsPosition" );
   if ( inPosition )
   {
      meta->addStatement( new GenOp( "   @ = @.xyz;\r\n", 
         wsPosition, inPosition ) );
      return;
   }

   // Get the input position.
   inPosition = (Var*)LangElement::find( "inPosition" );
   if ( !inPosition )
      inPosition = (Var*)LangElement::find( "position" );

   AssertFatal( inPosition, "ShaderFeatureHLSL::getWsPosition - The vertex position was not found!" );

   Var *objTrans = sHelper->getObjTrans( componentList, useInstancing, mInstancingFormat, meta );

   meta->addStatement( new GenOp( "   @ = mul( @, float4( @.xyz, 1 ) ).xyz;\r\n", 
      wsPosition, objTrans, inPosition ) );
}

Var* ShaderFeatureHLSL::addOutWsPosition( Vector<ShaderComponent*> &componentList,                                       
                                          bool useInstancing,
                                          MultiLine *meta )
{
   Var *outWsPosition = (Var*)LangElement::find( "outWsPosition" );
   if ( !outWsPosition )
   {
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      outWsPosition = connectComp->getElement( RT_TEXCOORD );
      outWsPosition->setName( "outWsPosition" );
      outWsPosition->setStructName( "OUT" );
      outWsPosition->setType( "float3" );
      outWsPosition->mapsToSampler = false;

      getWsPosition( componentList, useInstancing, meta, outWsPosition );
   }

   return outWsPosition;
}

Var* ShaderFeatureHLSL::getInWsPosition( Vector<ShaderComponent*> &componentList )
{
   Var *wsPosition = (Var*)LangElement::find( "wsPosition" );
   if ( !wsPosition )
   {
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      wsPosition = connectComp->getElement( RT_TEXCOORD );
      wsPosition->setName( "wsPosition" );
      wsPosition->setStructName( "IN" );
      wsPosition->setType( "float3" );
   }

   return wsPosition;
}

Var* ShaderFeatureHLSL::getWsView( Var *wsPosition, MultiLine *meta )
{
   Var *wsView = (Var*)LangElement::find( "wsView" );
   if ( !wsView )
   {
      wsView = new Var( "wsView", "float3" );

      Var *eyePos = (Var*)LangElement::find( "eyePosWorld" );
      if ( !eyePos )
      {
         eyePos = new Var;
         eyePos->setType( "float3" );
         eyePos->setName( "eyePosWorld" );
         eyePos->uniform = true;
         eyePos->constSortPos = cspPass;
      }

      meta->addStatement( new GenOp( "   @ = normalize( @ - @ );\r\n", 
         new DecOp( wsView ), eyePos, wsPosition ) );
   }

   return wsView;
}

//****************************************************************************
// Base Texture
//****************************************************************************

void DiffuseMapFeatHLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;
   sHelper->getOutTexCoord( "texCoord", "float2", true, fd.features[MFT_TexAnim], meta, componentList );
   output = meta;
}

void DiffuseMapFeatHLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   // grab connector texcoord register
   Var *inTex = sHelper->getInTexCoord( "texCoord", "float2", true, componentList );

   // create texture var
   Var *diffuseMap = new Var;
   diffuseMap->setType( "sampler2D" );
   diffuseMap->setName( "diffuseMap" );
   diffuseMap->uniform = true;
   diffuseMap->sampler = true;
   diffuseMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   if (  fd.features[MFT_CubeMap] )
   {
      MultiLine * meta = new MultiLine;
      
      // create sample color
      Var *diffColor = new Var;
      diffColor->setType( "float4" );
      diffColor->setName( "diffuseColor" );
      LangElement *colorDecl = new DecOp( diffColor );
   
      meta->addStatement(  new GenOp( "   @ = tex2D(@, @);\r\n", 
                           colorDecl, 
                           diffuseMap, 
                           inTex ) );
      
      meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( diffColor, Material::Mul ) ) );
      output = meta;
   }
   else if(fd.features[MFT_DiffuseMapAtlas])
   {   
      // Handle atlased textures
      // http://www.infinity-universe.com/Infinity/index.php?option=com_content&task=view&id=65&Itemid=47
      MultiLine * meta = new MultiLine;
      output = meta;

      Var *atlasedTex = new Var;
      atlasedTex->setName("atlasedTexCoord");
      atlasedTex->setType("float2");
      LangElement *atDecl = new DecOp(atlasedTex);

      // Parameters of the texture atlas
      Var *atParams  = new Var;
      atParams->setType("float4");
      atParams->setName("diffuseAtlasParams");
      atParams->uniform = true;
      atParams->constSortPos = cspPotentialPrimitive;

      // Parameters of the texture (tile) this object is using in the atlas
      Var *tileParams  = new Var;
      tileParams->setType("float4");
      tileParams->setName("diffuseAtlasTileParams");
      tileParams->uniform = true;
      tileParams->constSortPos = cspPotentialPrimitive;

      const bool is_sm3 = (GFX->getPixelShaderVersion() > 2.0f);
      if(is_sm3)
      {
         // Figure out the mip level
         meta->addStatement(new GenOp("   float2 _dx = ddx(@ * @.z);\r\n", inTex, atParams));
         meta->addStatement(new GenOp("   float2 _dy = ddy(@ * @.z);\r\n", inTex, atParams));
         meta->addStatement(new GenOp("   float mipLod = 0.5 * log2(max(dot(_dx, _dx), dot(_dy, _dy)));\r\n"));
         meta->addStatement(new GenOp("   mipLod = clamp(mipLod, 0.0, @.w);\r\n", atParams));

         // And the size of the mip level
         meta->addStatement(new GenOp("   float mipPixSz = pow(2.0, @.w - mipLod);\r\n", atParams));
         meta->addStatement(new GenOp("   float2 mipSz = mipPixSz / @.xy;\r\n", atParams));
      }
      else
      {
         meta->addStatement(new GenOp("   float2 mipSz = float2(1.0, 1.0);\r\n"));
      }

      // Tiling mode
      // TODO: Select wrap or clamp somehow
      if( true ) // Wrap
         meta->addStatement(new GenOp("   @ = frac(@);\r\n", atDecl, inTex));
      else       // Clamp
         meta->addStatement(new GenOp("   @ = saturate(@);\r\n", atDecl, inTex));

      // Finally scale/offset, and correct for filtering
      meta->addStatement(new GenOp("   @ = @ * ((mipSz * @.xy - 1.0) / mipSz) + 0.5 / mipSz + @.xy * @.xy;\r\n", 
         atlasedTex, atlasedTex, atParams, atParams, tileParams));

      // Add a newline
      meta->addStatement(new GenOp( "\r\n"));

      // For the rest of the feature...
      inTex = atlasedTex;

      // create sample color var
      Var *diffColor = new Var;
      diffColor->setType("float4");
      diffColor->setName("diffuseColor");

      // To dump out UV coords...
//#define DEBUG_ATLASED_UV_COORDS
#ifdef DEBUG_ATLASED_UV_COORDS
      if(!fd.features[MFT_PrePassConditioner])
      {
         meta->addStatement(new GenOp("   @ = float4(@.xy, mipLod / @.w, 1.0);\r\n", new DecOp(diffColor), inTex, atParams));
         meta->addStatement(new GenOp("   @; return OUT;\r\n", sHelper->assignColor(diffColor, Material::Mul)));
         return;
      }
#endif

      if(is_sm3)
      {
         meta->addStatement(new GenOp( "   @ = tex2Dlod(@, float4(@, 0.0, mipLod));\r\n", 
            new DecOp(diffColor), diffuseMap, inTex));
      }
      else
      {
         meta->addStatement(new GenOp( "   @ = tex2D(@, @);\r\n", 
            new DecOp(diffColor), diffuseMap, inTex));
      }

      meta->addStatement(new GenOp( "   @;\r\n", sHelper->assignColor(diffColor, Material::Mul)));
   }
   else
   {
      LangElement *statement = new GenOp( "tex2D(@, @)", diffuseMap, inTex );
      output = new GenOp( "   @;\r\n", sHelper->assignColor( statement, Material::Mul ) );
   }
   
}

ShaderFeature::Resources DiffuseMapFeatHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 
   res.numTex = 1;
   res.numTexReg = 1;

   return res;
}

void DiffuseMapFeatHLSL::setTexData(   Material::StageData &stageDat,
                                       const MaterialFeatureData &fd,
                                       RenderPassData &passData,
                                       U32 &texIndex )
{
   GFXTextureObject *tex = stageDat.getTex( MFT_DiffuseMap );
   if ( tex )
   {
      TexBind& bind = passData.mTexBind[ texIndex++ ];
      bind.samplerName = "$diffuseMap";
      bind.object = tex;
   }
}


//****************************************************************************
// Overlay Texture
//****************************************************************************

void OverlayTexFeatHLSL::processVert(  Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   Var *inTex = sHelper->getVertTexCoord( "vert_texCoord2" );
   AssertFatal( inTex, "OverlayTexFeatHLSL::processVert() - The second UV set was not found!" );

   // grab connector texcoord register
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *outTex = connectComp->getElement( RT_TEXCOORD );
   outTex->setName( "texCoord2" );
   outTex->setStructName( "OUT" );
   outTex->setType( "float2" );
   outTex->mapsToSampler = true;

   if( fd.features[MFT_TexAnim] )
   {
      inTex->setType( "float4" );

      // Find or create the texture matrix.
      Var *texMat = (Var*)LangElement::find( "texMat" );
      if ( !texMat )
      {
         texMat = new Var;
         texMat->setType( "float4x4" );
         texMat->setName( "texMat" );
         texMat->uniform = true;
         texMat->constSortPos = cspPass;   
      }
     
      output = new GenOp( "   @ = mul(@, @);\r\n", outTex, texMat, inTex );
      return;
   }
   
   // setup language elements to output incoming tex coords to output
   output = new GenOp( "   @ = @;\r\n", outTex, inTex );
}

void OverlayTexFeatHLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{

   // grab connector texcoord register
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *inTex = connectComp->getElement( RT_TEXCOORD );
   inTex->setName( "texCoord2" );
   inTex->setStructName( "IN" );
   inTex->setType( "float2" );
   inTex->mapsToSampler = true;

   // create texture var
   Var *diffuseMap = new Var;
   diffuseMap->setType( "sampler2D" );
   diffuseMap->setName( "overlayMap" );
   diffuseMap->uniform = true;
   diffuseMap->sampler = true;
   diffuseMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   LangElement *statement = new GenOp( "tex2D(@, @)", diffuseMap, inTex );
   output = new GenOp( "   @;\r\n", sHelper->assignColor( statement, Material::LerpAlpha ) );
}

ShaderFeature::Resources OverlayTexFeatHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 
   res.numTex = 1;
   res.numTexReg = 1;
   return res;
}

void OverlayTexFeatHLSL::setTexData(   Material::StageData &stageDat,
                                       const MaterialFeatureData &fd,
                                       RenderPassData &passData,
                                       U32 &texIndex )
{
   GFXTextureObject *tex = stageDat.getTex( MFT_OverlayMap );
   if ( tex )
   {
      TexBind& bind = passData.mTexBind[ texIndex++ ];
      bind.samplerName = "$overlayMap";
      bind.object = tex;
   }
}


//****************************************************************************
// Diffuse color
//****************************************************************************

void DiffuseFeatureHLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   Var *diffuseMaterialColor  = new Var;
   diffuseMaterialColor->setType( "float4" );
   diffuseMaterialColor->setName( "diffuseMaterialColor" );
   diffuseMaterialColor->uniform = true;
   diffuseMaterialColor->constSortPos = cspPotentialPrimitive;

   MultiLine * meta = new MultiLine;
   meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( diffuseMaterialColor, Material::Mul ) ) );
   output = meta;
}


//****************************************************************************
// Diffuse vertex color
//****************************************************************************

void DiffuseVertColorFeatureHLSL::processVert(  Vector< ShaderComponent* >& componentList, 
                                                const MaterialFeatureData& fd )
{
   // Create vertex color connector if it doesn't exist.
   
   Var* outColor = dynamic_cast< Var* >( LangElement::find( "vertColor" ) );
   if( !outColor )
   {
      // Search for vert color.
      
      Var* inColor = dynamic_cast< Var* >( LangElement::find( "diffuse" ) );
      if( !inColor )
      {
         output = NULL;
         return;
      }
      
      // Create connector.

      ShaderConnector* connectComp = dynamic_cast< ShaderConnector* >( componentList[ C_CONNECTOR ] );
      AssertFatal( connectComp, "DiffuseVertColorFeatureGLSL::processVert - C_CONNECTOR is not a ShaderConnector" );
      Var* outColor = connectComp->getElement( RT_COLOR );
      outColor->setName( "vertColor" );
      outColor->setStructName( "OUT" );
      outColor->setType( "float4" );

      output = new GenOp( "   @ = @;\r\n", outColor, inColor );
   }
   else
      output = NULL; // Nothing we need to do.
}

void DiffuseVertColorFeatureHLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                                const MaterialFeatureData &fd )
{
   Var* vertColor = dynamic_cast< Var* >( LangElement::find( "vertColor" ) );
   if( !vertColor )
   {
      ShaderConnector* connectComp = dynamic_cast< ShaderConnector* >( componentList[ C_CONNECTOR ] );
      AssertFatal( connectComp, "DiffuseVertColorFeatureGLSL::processVert - C_CONNECTOR is not a ShaderConnector" );
      vertColor = connectComp->getElement( RT_COLOR );
      vertColor->setName( "vertColor" );
      vertColor->setStructName( "IN" );
      vertColor->setType( "float4" );
   }
   
   MultiLine* meta = new MultiLine;
   meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( vertColor, Material::Mul ) ) );
   output = meta;
}


//****************************************************************************
// Lightmap
//****************************************************************************

void LightmapFeatHLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{
   // grab tex register from incoming vert
   Var *inTex = sHelper->getVertTexCoord( "vert_texCoord2" );

   // grab connector texcoord register
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *outTex = connectComp->getElement( RT_TEXCOORD );
   outTex->setName( "texCoord2" );
   outTex->setStructName( "OUT" );
   outTex->setType( "float2" );
   outTex->mapsToSampler = true;

   // setup language elements to output incoming tex coords to output
   output = new GenOp( "   @ = @;\r\n", outTex, inTex );
}

void LightmapFeatHLSL::processPix(  Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{
   // grab connector texcoord register
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *inTex = connectComp->getElement( RT_TEXCOORD );
   inTex->setName( "texCoord2" );
   inTex->setStructName( "IN" );
   inTex->setType( "float2" );
   inTex->mapsToSampler = true;

   // create texture var
   Var *lightMap = new Var;
   lightMap->setType( "sampler2D" );
   lightMap->setName( "lightMap" );
   lightMap->uniform = true;
   lightMap->sampler = true;
   lightMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   
   // argh, pixel specular should prob use this too
   if( fd.features[MFT_NormalMap] )
   {
      Var *lmColor = new Var;
      lmColor->setName( "lmColor" );
      lmColor->setType( "float4" );
      LangElement *lmColorDecl = new DecOp( lmColor );
      
      output = new GenOp( "   @ = tex2D(@, @);\r\n", lmColorDecl, lightMap, inTex );
      return;
   }

   // Add realtime lighting, if it is available
   LangElement *statement = NULL;
   if( fd.features[MFT_RTLighting] )
   {
      // Advanced lighting is the only dynamic lighting supported right now
      Var *inColor = (Var*) LangElement::find( "d_lightcolor" );
      if(inColor != NULL)
      {
         // Find out if RTLighting should be added or substituted
         bool bPreProcessedLighting = false;
         AdvancedLightBinManager *lightBin;
         if ( Sim::findObject( "AL_LightBinMgr", lightBin ) )
            bPreProcessedLighting = lightBin->MRTLightmapsDuringPrePass();

         // Lightmap has already been included in the advanced light bin, so
         // no need to do any sampling or anything
         if(bPreProcessedLighting)
            statement = new GenOp( "float4(@, 1.0)", inColor );
         else
            statement = new GenOp( "tex2D(@, @) + float4(@.rgb, 0.0)", lightMap, inTex, inColor );
      }
   }
   
   // If we still don't have it... then just sample the lightmap.   
   if ( !statement )
      statement = new GenOp( "tex2D(@, @)", lightMap, inTex );

   // Assign to proper render target
   MultiLine *meta = new MultiLine;
   if( fd.features[MFT_LightbufferMRT] )
   {
      meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( statement, Material::None, NULL, RenderTarget1 ) ) );
      meta->addStatement( new GenOp( "   @.a = 0.0001;\r\n", LangElement::find( sHelper->getOutputTargetVarName(RenderTarget1) ) ) );
   }
   else
      meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( statement, Material::Mul ) ) );

   output = meta;
}

ShaderFeature::Resources LightmapFeatHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 
   res.numTex = 1;
   res.numTexReg = 1;

   return res;
}

void LightmapFeatHLSL::setTexData(  Material::StageData &stageDat,
                                    const MaterialFeatureData &fd,
                                    RenderPassData &passData,
                                    U32 &texIndex )
{
   GFXTextureObject *tex = stageDat.getTex( MFT_LightMap );
   TexBind& bind = passData.mTexBind[ texIndex++ ];
   bind.samplerName = "$lightMap";
   if ( tex )
      bind.object = tex;
   else
      bind.type = Material::Lightmap;
}

U32 LightmapFeatHLSL::getOutputTargets( const MaterialFeatureData &fd ) const
{
   return fd.features[MFT_LightbufferMRT] ? RenderTarget1 : DefaultTarget;
}

//****************************************************************************
// Tonemap
//****************************************************************************

void TonemapFeatHLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{
   // Grab the connector
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

   // Set up the second set of texCoords
   Var *inTex2 = sHelper->getVertTexCoord( "vert_texCoord2" );

   if ( inTex2 )
   {
      Var *outTex2 = connectComp->getElement( RT_TEXCOORD );
      outTex2->setName( "texCoord2" );
      outTex2->setStructName( "OUT" );
      outTex2->setType( "float2" );
      outTex2->mapsToSampler = true;

      output = new GenOp( "   @ = @;\r\n", outTex2, inTex2 );
   }
}

void TonemapFeatHLSL::processPix(  Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{
   // Grab connector
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

   Var *inTex2 = connectComp->getElement( RT_TEXCOORD );
   inTex2->setName( "texCoord2" );
   inTex2->setStructName( "IN" );
   inTex2->setType( "float2" );
   inTex2->mapsToSampler = true;

   // create texture var
   Var *toneMap = new Var;
   toneMap->setType( "sampler2D" );
   toneMap->setName( "toneMap" );
   toneMap->uniform = true;
   toneMap->sampler = true;
   toneMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   MultiLine * meta = new MultiLine;

   // First get the toneMap color
   Var *toneMapColor = new Var;
   toneMapColor->setType( "float4" );
   toneMapColor->setName( "toneMapColor" );
   LangElement *toneMapColorDecl = new DecOp( toneMapColor );

   meta->addStatement( new GenOp( "   @ = tex2D(@, @);\r\n", toneMapColorDecl, toneMap, inTex2 ) );

   // We do a different calculation if there is a diffuse map or not
   Material::BlendOp blendOp = Material::Mul;
   if ( fd.features[MFT_DiffuseMap] )
   {
      // Reverse the tonemap
      meta->addStatement( new GenOp( "   @ = -1.0f * log(1.0f - @);\r\n", toneMapColor, toneMapColor ) );

      // Re-tonemap with the current color factored in
      blendOp = Material::ToneMap;
   }

   // Find out if RTLighting should be added
   bool bPreProcessedLighting = false;
   AdvancedLightBinManager *lightBin;
   if ( Sim::findObject( "AL_LightBinMgr", lightBin ) )
      bPreProcessedLighting = lightBin->MRTLightmapsDuringPrePass();

   // Add in the realtime lighting contribution
   if ( fd.features[MFT_RTLighting] )
   {
      // Right now, only Advanced Lighting is supported
      Var *inColor = (Var*) LangElement::find( "d_lightcolor" );
      if(inColor != NULL)
      {
         // Assign value in d_lightcolor to toneMapColor if it exists. This is
         // the dynamic light buffer, and it already has the tonemap included
         if(bPreProcessedLighting)
            meta->addStatement( new GenOp( "   @.rgb = @;\r\n", toneMapColor, inColor ) );
         else
            meta->addStatement( new GenOp( "   @.rgb += @.rgb;\r\n", toneMapColor, inColor ) );
      }
   }

   // Assign to proper render target
   if( fd.features[MFT_LightbufferMRT] )
   {
      meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( toneMapColor, Material::None, NULL, RenderTarget1 ) ) );
      meta->addStatement( new GenOp( "   @.a = 0.0001;\r\n", LangElement::find( sHelper->getOutputTargetVarName(RenderTarget1) ) ) );
   }
   else
      meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( toneMapColor, blendOp ) ) );
  
   output = meta;
}

ShaderFeature::Resources TonemapFeatHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 
   res.numTex = 1;
   res.numTexReg = 1;

   return res;
}

void TonemapFeatHLSL::setTexData(  Material::StageData &stageDat,
                                    const MaterialFeatureData &fd,
                                    RenderPassData &passData,
                                    U32 &texIndex )
{
   GFXTextureObject *tex = stageDat.getTex( MFT_ToneMap );
   if ( tex )
   {
      TexBind& bind = passData.mTexBind[ texIndex++ ];
      bind.samplerName = "$toneMap";
      bind.type = Material::ToneMapTex;
      bind.object = tex;
   }
}

U32 TonemapFeatHLSL::getOutputTargets( const MaterialFeatureData &fd ) const
{
   return fd.features[MFT_LightbufferMRT] ? RenderTarget1 : DefaultTarget;
}

//****************************************************************************
// pureLIGHT Lighting
//****************************************************************************

void VertLitHLSL::processVert(   Vector<ShaderComponent*> &componentList, 
                                 const MaterialFeatureData &fd )
{
   // If we have a lightMap or toneMap then our lighting will be
   // handled by the MFT_LightMap or MFT_ToneNamp feature instead
   if ( fd.features[MFT_LightMap] || fd.features[MFT_ToneMap] )
   {
      output = NULL;
      return;
   }

   // Create vertex color connector if it doesn't exist.
   
   Var* outColor = dynamic_cast< Var* >( LangElement::find( "vertColor" ) );
   if( !outColor )
   {
      // Search for vert color
      Var *inColor = (Var*) LangElement::find( "diffuse" );   

      // If there isn't a vertex color then we can't do anything
      if( !inColor )
      {
         output = NULL;
         return;
      }

      // Grab the connector color
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      Var *outColor = connectComp->getElement( RT_COLOR );
      outColor->setName( "vertColor" );
      outColor->setStructName( "OUT" );
      outColor->setType( "float4" );

      output = new GenOp( "   @ = @;\r\n", outColor, inColor );
   }
   else
      output = NULL; // Nothing we need to do.
}

void VertLitHLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   // If we have a lightMap or toneMap then our lighting will be
   // handled by the MFT_LightMap or MFT_ToneNamp feature instead
   if ( fd.features[MFT_LightMap] || fd.features[MFT_ToneMap] )
   {
      output = NULL;
      return;
   }
   
   // Grab the connector color register
   Var* vertColor = dynamic_cast< Var* >( LangElement::find( "vertColor" ) );
   if( !vertColor )
   {
      ShaderConnector* connectComp = dynamic_cast< ShaderConnector* >( componentList[ C_CONNECTOR ] );
      AssertFatal( connectComp, "VertLitGLSL::processVert - C_CONNECTOR is not a ShaderConnector" );
      vertColor = connectComp->getElement( RT_COLOR );
      vertColor->setName( "vertColor" );
      vertColor->setStructName( "IN" );
      vertColor->setType( "float4" );
   }

   MultiLine * meta = new MultiLine;

   // Defaults (no diffuse map)
   Material::BlendOp blendOp = Material::Mul;
   LangElement *outColor = vertColor;

   // We do a different calculation if there is a diffuse map or not
   if ( fd.features[MFT_DiffuseMap] || fd.features[MFT_VertLitTone] )
   {
      Var * finalVertColor = new Var;
      finalVertColor->setName( "finalVertColor" );
      finalVertColor->setType( "float4" );
      LangElement *finalVertColorDecl = new DecOp( finalVertColor );
      
      // Reverse the tonemap
      meta->addStatement( new GenOp( "   @ = -1.0f * log(1.0f - @);\r\n", finalVertColorDecl, vertColor ) );

      // Set the blend op to tonemap
      blendOp = Material::ToneMap;
      outColor = finalVertColor;
   }

   // Add in the realtime lighting contribution, if applicable
   if ( fd.features[MFT_RTLighting] )
   {
      Var *rtLightingColor = (Var*) LangElement::find( "d_lightcolor" );
      if(rtLightingColor != NULL)
      {
         bool bPreProcessedLighting = false;
         AdvancedLightBinManager *lightBin;
         if ( Sim::findObject( "AL_LightBinMgr", lightBin ) )
            bPreProcessedLighting = lightBin->MRTLightmapsDuringPrePass();

         // Assign value in d_lightcolor to toneMapColor if it exists. This is
         // the dynamic light buffer, and it already has the baked-vertex-color 
         // included in it
         if(bPreProcessedLighting)
            outColor = new GenOp( "float4(@.rgb, 1.0)", rtLightingColor );
         else
            outColor = new GenOp( "float4(@.rgb + @.rgb, 1.0)", rtLightingColor, outColor );
      }
   }

   // Output the color
   if ( fd.features[MFT_LightbufferMRT] )
   {
      meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( outColor, Material::None, NULL, RenderTarget1 ) ) );
      meta->addStatement( new GenOp( "   @.a = 0.0001;\r\n", LangElement::find( sHelper->getOutputTargetVarName(RenderTarget1) ) ) );
   }
   else
      meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( outColor, blendOp ) ) );

   output = meta;
}

U32 VertLitHLSL::getOutputTargets( const MaterialFeatureData &fd ) const
{
   return fd.features[MFT_LightbufferMRT] ? RenderTarget1 : DefaultTarget;
}

//****************************************************************************
// Detail map
//****************************************************************************

void DetailFeatHLSL::processVert(   Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;
   sHelper->addOutDetailTexCoord( componentList, meta, fd.features[MFT_TexAnim] );
   output = meta;
}

void DetailFeatHLSL::processPix( Vector<ShaderComponent*> &componentList, 
                                 const MaterialFeatureData &fd )
{
   // Get the detail texture coord.
   Var *inTex = sHelper->getInTexCoord( "detCoord", "float2", true, componentList );

   // create texture var
   Var *detailMap = new Var;
   detailMap->setType( "sampler2D" );
   detailMap->setName( "detailMap" );
   detailMap->uniform = true;
   detailMap->sampler = true;
   detailMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   // We're doing the standard greyscale detail map
   // technique which can darken and lighten the 
   // diffuse texture.

   // TODO: We could add a feature to toggle between this
   // and a simple multiplication with the detail map.

   LangElement *statement = new GenOp( "( tex2D(@, @) * 2.0 ) - 1.0", detailMap, inTex );
   output = new GenOp( "   @;\r\n", sHelper->assignColor( statement, Material::Add ) );
}

ShaderFeature::Resources DetailFeatHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 
   res.numTex = 1;
   res.numTexReg = 1;

   return res;
}

void DetailFeatHLSL::setTexData( Material::StageData &stageDat,
                                 const MaterialFeatureData &fd,
                                 RenderPassData &passData,
                                 U32 &texIndex )
{
   GFXTextureObject *tex = stageDat.getTex( MFT_DetailMap );
   if ( tex )
   {
      TexBind& bind = passData.mTexBind[ texIndex++ ];
      bind.samplerName = "$detailMap";
      bind.object = tex;
   }
}


//****************************************************************************
// Vertex position
//****************************************************************************

void VertPositionHLSL::determineFeature(  Material *material,
                                          const GFXVertexFormat *vertexFormat,
                                          U32 stageNum,
                                          const FeatureType &type,
                                          const FeatureSet &features,
                                          MaterialFeatureData *outFeatureData )
{
   // This feature is always on!
   outFeatureData->features.addFeature( type );
}

void VertPositionHLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                    const MaterialFeatureData &fd )
{
   // First check for an input position from a previous feature
   // then look for the default vertex position.
   Var *inPosition = (Var*)LangElement::find( "inPosition" );
   if ( !inPosition )
      inPosition = (Var*)LangElement::find( "position" );

   // grab connector position
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *outPosition = connectComp->getElement( RT_POSITION );
   outPosition->setName( "hpos" );
   outPosition->setStructName( "OUT" );

   MultiLine *meta = new MultiLine;

   Var *modelview = getModelView( componentList, fd.features[MFT_UseInstancing], meta ); 

   meta->addStatement( new GenOp( "   @ = mul(@, float4(@.xyz,1));\r\n", 
      outPosition, modelview, inPosition ) );

   output = meta;
}


//****************************************************************************
// Reflect Cubemap
//****************************************************************************

void ReflectCubeFeatHLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   // search for vert normal
   Var *inNormal = (Var*) LangElement::find( "normal" );
   if ( !inNormal )
      return;

   MultiLine * meta = new MultiLine;

   // If a base or bump tex is present in the material, but not in the
   // current pass - we need to add one to the current pass to use
   // its alpha channel as a gloss map.  Here we just need the tex coords.
   if( !fd.features[MFT_DiffuseMap] &&
       !fd.features[MFT_NormalMap] )
   {
      if( fd.materialFeatures[MFT_DiffuseMap] ||
          fd.materialFeatures[MFT_NormalMap] )
      {
         // find incoming texture var
         Var *inTex = sHelper->getVertTexCoord( "vert_texCoord" );

         // grab connector texcoord register
         ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
         Var *outTex = connectComp->getElement( RT_TEXCOORD );
         outTex->setName( "texCoord" );
         outTex->setStructName( "OUT" );
         outTex->setType( "float2" );
         outTex->mapsToSampler = true;

         // setup language elements to output incoming tex coords to output
         meta->addStatement( new GenOp( "   @ = @;\r\n", outTex, inTex ) );
      }
   }

   // create cubeTrans
   Var *cubeTrans = new Var;
   cubeTrans->setType( "float3x3" );
   cubeTrans->setName( "cubeTrans" );
   cubeTrans->uniform = true;
   cubeTrans->constSortPos = cspPrimitive;   

   // create cubeEye position
   Var *cubeEyePos = new Var;
   cubeEyePos->setType( "float3" );
   cubeEyePos->setName( "cubeEyePos" );
   cubeEyePos->uniform = true;
   cubeEyePos->constSortPos = cspPrimitive;   

   // cube vert position
   Var * cubeVertPos = new Var;
   cubeVertPos->setName( "cubeVertPos" );
   cubeVertPos->setType( "float3" );
   LangElement *cubeVertPosDecl = new DecOp( cubeVertPos );

   meta->addStatement( new GenOp( "   @ = mul(@, @).xyz;\r\n", 
                       cubeVertPosDecl, cubeTrans, LangElement::find( "position" ) ) );

   // cube normal
   Var * cubeNormal = new Var;
   cubeNormal->setName( "cubeNormal" );
   cubeNormal->setType( "float3" );
   LangElement *cubeNormDecl = new DecOp( cubeNormal );

   meta->addStatement( new GenOp( "   @ = normalize( mul(@, normalize(@)).xyz );\r\n", 
                       cubeNormDecl, cubeTrans, inNormal ) );

   // eye to vert
   Var * eyeToVert = new Var;
   eyeToVert->setName( "eyeToVert" );
   eyeToVert->setType( "float3" );
   LangElement *e2vDecl = new DecOp( eyeToVert );

   meta->addStatement( new GenOp( "   @ = @ - @;\r\n", 
                       e2vDecl, cubeVertPos, cubeEyePos ) );

   // grab connector texcoord register
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *reflectVec = connectComp->getElement( RT_TEXCOORD );
   reflectVec->setName( "reflectVec" );
   reflectVec->setStructName( "OUT" );
   reflectVec->setType( "float3" );
   reflectVec->mapsToSampler = true;

   meta->addStatement( new GenOp( "   @ = reflect(@, @);\r\n", reflectVec, eyeToVert, cubeNormal ) );

   output = meta;
}

void ReflectCubeFeatHLSL::processPix(  Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   MultiLine * meta = new MultiLine;
   Var *glossColor = NULL;
   
   // If a base or bump tex is present in the material, but not in the
   // current pass - we need to add one to the current pass to use
   // its alpha channel as a gloss map.
   if( !fd.features[MFT_DiffuseMap] &&
       !fd.features[MFT_NormalMap] )
   {
      if( fd.materialFeatures[MFT_DiffuseMap] ||
          fd.materialFeatures[MFT_NormalMap] )
      {
         // grab connector texcoord register
         Var *inTex = sHelper->getInTexCoord( "texCoord", "float2", true, componentList );
      
         // create texture var
         Var *newMap = new Var;
         newMap->setType( "sampler2D" );
         newMap->setName( "glossMap" );
         newMap->uniform = true;
         newMap->sampler = true;
         newMap->constNum = Var::getTexUnitNum();     // used as texture unit num here
      
         // create sample color
         Var *color = new Var;
         color->setType( "float4" );
         color->setName( "diffuseColor" );
         LangElement *colorDecl = new DecOp( color );

         glossColor = color;
         
         meta->addStatement( new GenOp( "   @ = tex2D( @, @ );\r\n", colorDecl, newMap, inTex ) );
      }
   }
   else
   {
      glossColor = (Var*) LangElement::find( "diffuseColor" );
      if( !glossColor )
         glossColor = (Var*) LangElement::find( "bumpNormal" );
   }

   // grab connector texcoord register
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *reflectVec = connectComp->getElement( RT_TEXCOORD );
   reflectVec->setName( "reflectVec" );
   reflectVec->setStructName( "IN" );
   reflectVec->setType( "float3" );
   reflectVec->mapsToSampler = true;

   // create cubemap var
   Var *cubeMap = new Var;
   cubeMap->setType( "samplerCUBE" );
   cubeMap->setName( "cubeMap" );
   cubeMap->uniform = true;
   cubeMap->sampler = true;
   cubeMap->constNum = Var::getTexUnitNum();     // used as texture unit num here

   // TODO: Restore the lighting attenuation here!
   Var *attn = NULL;
   //if ( fd.materialFeatures[MFT_DynamicLight] )
	   //attn = (Var*)LangElement::find("attn");
   //else 
      if ( fd.materialFeatures[MFT_RTLighting] )
      attn =(Var*)LangElement::find("d_NL_Att");

   LangElement *texCube = new GenOp( "texCUBE( @, @ )", cubeMap, reflectVec );
   LangElement *lerpVal = NULL;
   Material::BlendOp blendOp = Material::LerpAlpha;

   // Note that the lerpVal needs to be a float4 so that
   // it will work with the LerpAlpha blend.

   if ( glossColor )
   {
      if ( attn )
         lerpVal = new GenOp( "@ * saturate( @ )", glossColor, attn );
      else
         lerpVal = glossColor;
   }
   else
   {
      if ( attn )
         lerpVal = new GenOp( "saturate( @ ).xxxx", attn );
      else
         blendOp = Material::Mul;
   }

   meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( texCube, blendOp, lerpVal ) ) );         
   output = meta;
}

ShaderFeature::Resources ReflectCubeFeatHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 

   if( fd.features[MFT_DiffuseMap] ||
       fd.features[MFT_NormalMap] )
   {
      res.numTex = 1;
      res.numTexReg = 1;
   }
   else
   {
      res.numTex = 2;
      res.numTexReg = 2;
   }

   return res;
}

void ReflectCubeFeatHLSL::setTexData(  Material::StageData &stageDat,
                                       const MaterialFeatureData &stageFeatures,
                                       RenderPassData &passData,
                                       U32 &texIndex )
{
   // set up a gloss map if one is not present in the current pass
   // but is present in the current material stage
   if( !passData.mFeatureData.features[MFT_DiffuseMap] &&
       !passData.mFeatureData.features[MFT_NormalMap] )
   {
      GFXTextureObject *tex = stageDat.getTex( MFT_DetailMap );
      if (  tex &&
            stageFeatures.features[MFT_DiffuseMap] )
      {
         TexBind& bind = passData.mTexBind[ texIndex++ ];
         bind.samplerName = "$glossMap";
         bind.object = tex;
      }
      else
      {
         tex = stageDat.getTex( MFT_NormalMap );

         if (  tex &&
               stageFeatures.features[ MFT_NormalMap ] )
         {
            TexBind& bind = passData.mTexBind[ texIndex++ ];
            bind.samplerName = "$glossMap";
            bind.object = tex;
         }
      }
   }
   
   if( stageDat.getCubemap() )
   {
      TexBind& bind = passData.mTexBind[ texIndex++ ];
      bind.samplerName = "$cubeMap";
      bind.type = Material::Cube;
      passData.mCubeMap = stageDat.getCubemap();
   }
   else
   {
      if( stageFeatures.features[MFT_CubeMap] )
      {
         // assuming here that it is a scenegraph cubemap
         TexBind& bind = passData.mTexBind[ texIndex++ ];
         bind.samplerName = "$cubeMap";
         bind.type = Material::SGCube;
      }
   }
}


//****************************************************************************
// RTLighting
//****************************************************************************

RTLightingFeatHLSL::RTLightingFeatHLSL()
   : mDep( "shaders/common/lighting.hlsl" )
{
   addDependency( &mDep );
}

void RTLightingFeatHLSL::processVert(  Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;   

   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

   // Special case for lighting imposters. We dont have a vert normal and may not
   // have a normal map. Generate and pass the normal data the pixel shader needs.
   if ( fd.features[MFT_ImposterVert] )
   {
      if ( !fd.features[MFT_NormalMap] )
      {
         Var *eyePos = (Var*)LangElement::find( "eyePosWorld" );
         if ( !eyePos )
         {
            eyePos = new Var( "eyePosWorld", "float3" );
            eyePos->uniform = true;
            eyePos->constSortPos = cspPass;
         }
          
         Var *inPosition = (Var*)LangElement::find( "position" );

         Var *outNormal = connectComp->getElement( RT_TEXCOORD );
         outNormal->setName( "wsNormal" );
         outNormal->setStructName( "OUT" );
         outNormal->setType( "float3" );
         outNormal->mapsToSampler = false;

         // Transform the normal to world space.
         meta->addStatement( new GenOp( "   @ = normalize( @ - @.xyz );\r\n", outNormal, eyePos, inPosition ) );
      }

      addOutWsPosition( componentList, fd.features[MFT_UseInstancing], meta );

      output = meta;

      return;
   }

   // Find the incoming vertex normal.
   Var *inNormal = (Var*)LangElement::find( "normal" );   

   // Skip out on realtime lighting if we don't have a normal
   // or we're doing some sort of baked lighting.
   if (  !inNormal || 
         fd.features[MFT_LightMap] || 
         fd.features[MFT_ToneMap] || 
         fd.features[MFT_VertLit] )
      return;   

   // If there isn't a normal map then we need to pass
   // the world space normal to the pixel shader ourselves.
   if ( !fd.features[MFT_NormalMap] )
   {
      Var *outNormal = connectComp->getElement( RT_TEXCOORD );
      outNormal->setName( "wsNormal" );
      outNormal->setStructName( "OUT" );
      outNormal->setType( "float3" );
      outNormal->mapsToSampler = false;

      // Get the transform to world space.
      Var *objTrans = sHelper->getObjTrans( componentList, fd.features[MFT_UseInstancing], mInstancingFormat, meta );

      // Transform the normal to world space.
      meta->addStatement( new GenOp( "   @ = mul( @, float4( normalize( @ ), 0.0 ) ).xyz;\r\n", outNormal, objTrans, inNormal ) );
   }

   addOutWsPosition( componentList, fd.features[MFT_UseInstancing], meta );

   output = meta;
}

void RTLightingFeatHLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   // Skip out on realtime lighting if we don't have a normal
   // or we're doing some sort of baked lighting.
   //
   // TODO: We can totally detect for this in the material
   // feature setup... we should move it out of here!
   //
   if ( fd.features[MFT_LightMap] || fd.features[MFT_ToneMap] || fd.features[MFT_VertLit] )
      return;
  
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );

   MultiLine *meta = new MultiLine;

   // Look for a wsNormal or grab it from the connector.
   Var *wsNormal = (Var*)LangElement::find( "wsNormal" );
   if ( !wsNormal )
   {
      wsNormal = connectComp->getElement( RT_TEXCOORD );
      wsNormal->setName( "wsNormal" );
      wsNormal->setStructName( "IN" );
      wsNormal->setType( "float3" );

      // If we loaded the normal its our responsibility
      // to normalize it... the interpolators won't.
      //
      // Note we cast to half here to get partial precision
      // optimized code which is an acceptable loss of
      // precision for normals and performs much better
      // on older Geforce cards.
      //
      meta->addStatement( new GenOp( "   @ = normalize( half3( @ ) );\r\n", wsNormal, wsNormal ) );
   }

   // Now the wsPosition and wsView.
   Var *wsPosition = getInWsPosition( componentList );
   Var *wsView = getWsView( wsPosition, meta );

   // Create temporaries to hold results of lighting.
   Var *rtShading = new Var( "rtShading", "float4" );
   Var *specular = new Var( "specular", "float4" );
   meta->addStatement( new GenOp( "   @; @;\r\n", 
      new DecOp( rtShading ), new DecOp( specular ) ) );   

   // Look for a light mask generated from a previous
   // feature (this is done for BL terrain lightmaps).
   LangElement *lightMask = LangElement::find( "lightMask" );
   if ( !lightMask )
      lightMask = new GenOp( "float4( 1, 1, 1, 1 )" );

   // Get all the light constants.
   Var *inLightPos  = new Var( "inLightPos", "float4" );
   inLightPos->uniform = true;
   inLightPos->arraySize = 3;
   inLightPos->constSortPos = cspPotentialPrimitive;

   Var *inLightInvRadiusSq  = new Var( "inLightInvRadiusSq", "float4" );
   inLightInvRadiusSq->uniform = true;
   inLightInvRadiusSq->constSortPos = cspPotentialPrimitive;

   Var *inLightColor  = new Var( "inLightColor", "float4" );
   inLightColor->uniform = true;
   inLightColor->arraySize = 4;
   inLightColor->constSortPos = cspPotentialPrimitive;

   Var *inLightSpotDir  = new Var( "inLightSpotDir", "float4" );
   inLightSpotDir->uniform = true;
   inLightSpotDir->arraySize = 3;
   inLightSpotDir->constSortPos = cspPotentialPrimitive;

   Var *inLightSpotAngle  = new Var( "inLightSpotAngle", "float4" );
   inLightSpotAngle->uniform = true;
   inLightSpotAngle->constSortPos = cspPotentialPrimitive;

   Var *lightSpotFalloff  = new Var( "inLightSpotFalloff", "float4" );
   lightSpotFalloff->uniform = true;
   lightSpotFalloff->constSortPos = cspPotentialPrimitive;

   Var *specularPower  = new Var( "specularPower", "float" );
   specularPower->uniform = true;
   specularPower->constSortPos = cspPotentialPrimitive;

   Var *specularColor = (Var*)LangElement::find( "specularColor" );
   if ( !specularColor )
   {
      specularColor  = new Var( "specularColor", "float4" );
      specularColor->uniform = true;
      specularColor->constSortPos = cspPotentialPrimitive;
   }

   Var *ambient  = new Var( "ambient", "float4" );
   ambient->uniform = true;
   ambient->constSortPos = cspPass;

   // Calculate the diffuse shading and specular powers.
   meta->addStatement( new GenOp( "   compute4Lights( @, @, @, @,\r\n"
                                  "      @, @, @, @, @, @, @, @,\r\n"
                                  "      @, @ );\r\n", 
      wsView, wsPosition, wsNormal, lightMask,
      inLightPos, inLightInvRadiusSq, inLightColor, inLightSpotDir, inLightSpotAngle, lightSpotFalloff, specularPower, specularColor,
      rtShading, specular ) );

   // Apply the lighting to the diffuse color.
   LangElement *lighting = new GenOp( "float4( @.rgb + @.rgb, 1 )", rtShading, ambient );
   meta->addStatement( new GenOp( "   @;\r\n", sHelper->assignColor( lighting, Material::Mul ) ) );
   output = meta;  
}

ShaderFeature::Resources RTLightingFeatHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;

   // These features disable realtime lighting.
   if (  !fd.features[MFT_LightMap] && 
         !fd.features[MFT_ToneMap] &&
         !fd.features[MFT_VertLit] )
   {
      // If enabled we pass the position.
      res.numTexReg = 1;

      // If there isn't a bump map then we pass the
      // world space normal as well.
      if ( !fd.features[MFT_NormalMap] )
         res.numTexReg++;
   }

   return res;
}


//****************************************************************************
// Fog
//****************************************************************************

FogFeatHLSL::FogFeatHLSL()
   : mFogDep( "shaders/common/torque.hlsl" )
{
   addDependency( &mFogDep );
}

void FogFeatHLSL::processVert(   Vector<ShaderComponent*> &componentList, 
                                 const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;

   const bool vertexFog = Con::getBoolVariable( "$useVertexFog", false );
   if ( vertexFog || GFX->getPixelShaderVersion() < 3.0 )
   {
      // Grab the eye position.
      Var *eyePos = (Var*)LangElement::find( "eyePosWorld" );
      if ( !eyePos )
      {
         eyePos = new Var( "eyePosWorld", "float3" );
         eyePos->uniform = true;
         eyePos->constSortPos = cspPass;
      }

      Var *fogData = new Var( "fogData", "float3" );
      fogData->uniform = true;
      fogData->constSortPos = cspPass;   

      Var *wsPosition = new Var( "fogPos", "float3" );
      getWsPosition( componentList, 
                     fd.features[MFT_UseInstancing], 
                     meta,
                     new DecOp( wsPosition ) );

      // We pass the fog amount to the pixel shader.
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      Var *fogAmount = connectComp->getElement( RT_TEXCOORD );
      fogAmount->setName( "fogAmount" );
      fogAmount->setStructName( "OUT" );
      fogAmount->setType( "float" );
      fogAmount->mapsToSampler = false;

      meta->addStatement( new GenOp( "   @ = saturate( computeSceneFog( @, @, @.r, @.g, @.b ) );\r\n", 
         fogAmount, eyePos, wsPosition, fogData, fogData, fogData ) );
   }
   else
   {
      // We fog in world space... make sure the world space
      // position is passed to the pixel shader.  This is
      // often already passed for lighting, so it takes up
      // no extra output registers.
      addOutWsPosition( componentList, fd.features[MFT_UseInstancing], meta );
   }

   output = meta;
}

void FogFeatHLSL::processPix( Vector<ShaderComponent*> &componentList, 
                              const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;

   Var *fogColor = new Var;
   fogColor->setType( "float4" );
   fogColor->setName( "fogColor" );
   fogColor->uniform = true;
   fogColor->constSortPos = cspPass;

   // Get the out color.
   Var *color = (Var*) LangElement::find( "col" );
   if ( !color )
   {
      color = new Var;
      color->setType( "fragout" );
      color->setName( "col" );
      color->setStructName( "OUT" );
   }

   Var *fogAmount;

   const bool vertexFog = Con::getBoolVariable( "$useVertexFog", false );
   if ( vertexFog || GFX->getPixelShaderVersion() < 3.0 )
   {
      // Per-vertex.... just get the fog amount.
      ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      fogAmount = connectComp->getElement( RT_TEXCOORD );
      fogAmount->setName( "fogAmount" );
      fogAmount->setStructName( "IN" );
      fogAmount->setType( "float" );
   }
   else
   {
      Var *wsPosition = getInWsPosition( componentList );

      // grab the eye position
      Var *eyePos = (Var*)LangElement::find( "eyePosWorld" );
      if ( !eyePos )
      {
         eyePos = new Var( "eyePosWorld", "float3" );
         eyePos->uniform = true;
         eyePos->constSortPos = cspPass;
      }

      Var *fogData = new Var( "fogData", "float3" );
      fogData->uniform = true;
      fogData->constSortPos = cspPass;   

      /// Get the fog amount.
      fogAmount = new Var( "fogAmount", "float" );
      meta->addStatement( new GenOp( "   @ = saturate( computeSceneFog( @, @, @.r, @.g, @.b ) );\r\n", 
         new DecOp( fogAmount ), eyePos, wsPosition, fogData, fogData, fogData ) );
   }

   // Lerp between the fog color and diffuse color.
   LangElement *fogLerp = new GenOp( "lerp( @.rgb, @.rgb, @ )", fogColor, color, fogAmount );
   meta->addStatement( new GenOp( "   @.rgb = @;\r\n", color, fogLerp ) );

   output = meta;
}

ShaderFeature::Resources FogFeatHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res;
   res.numTexReg = 1;
   return res;
}


//****************************************************************************
// Visibility
//****************************************************************************

VisibilityFeatHLSL::VisibilityFeatHLSL()
   : mTorqueDep( "shaders/common/torque.hlsl" )
{
   addDependency( &mTorqueDep );
}

void VisibilityFeatHLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                      const MaterialFeatureData &fd )
{
   MultiLine *meta = new MultiLine;
   output = meta;

   if ( fd.features[ MFT_UseInstancing ] )
   {
      // We pass the visibility to the pixel shader via
      // another output register.
      //
      // TODO: We should see if we can share this register
      // with some other common instanced data.
      //
      ShaderConnector *conn = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
      Var *outVisibility = conn->getElement( RT_TEXCOORD );
      outVisibility->setStructName( "OUT" );
      outVisibility->setName( "visibility" );
      outVisibility->setType( "float" );

      ShaderConnector *vertStruct = dynamic_cast<ShaderConnector *>( componentList[C_VERT_STRUCT] );
      Var *instVisibility = vertStruct->getElement( RT_TEXCOORD, 1 );
      instVisibility->setStructName( "IN" );
      instVisibility->setName( "inst_visibility" );
      instVisibility->setType( "float" );
      mInstancingFormat->addElement( "visibility", GFXDeclType_Float, instVisibility->constNum );

      meta->addStatement( new GenOp( "   @ = @; // Instancing!\r\n", outVisibility, instVisibility ) );
   }

   if ( fd.features[ MFT_IsTranslucent ] )
      return;

   sHelper->addOutVpos( meta, componentList );
}

void VisibilityFeatHLSL::processPix(   Vector<ShaderComponent*> &componentList, 
                                       const MaterialFeatureData &fd )
{
   // Get the visibility constant.
   Var *visibility = NULL;
   if ( fd.features[ MFT_UseInstancing ] )
      visibility = sHelper->getInTexCoord( "visibility", "float", false, componentList );
   else
   {
      visibility = (Var*)LangElement::find( "visibility" );

      if ( !visibility )
      {
         visibility = new Var();
         visibility->setType( "float" );
         visibility->setName( "visibility" );
         visibility->uniform = true;
         visibility->constSortPos = cspPotentialPrimitive;  
      }
   }

   MultiLine *meta = new MultiLine;
   output = meta;

   // Translucent objects do a simple alpha fade.
   if ( fd.features[ MFT_IsTranslucent ] )
   {
      Var *color = (Var*)LangElement::find( "col" );      
      meta->addStatement( new GenOp( "   @.a *= @;\r\n", color, visibility ) );
      return;
   }

   // Everything else does a fizzle.
   Var *vPos = sHelper->getInVpos( meta, componentList );
   meta->addStatement( new GenOp( "   fizzle( @, @ );\r\n", vPos, visibility ) );
}

ShaderFeature::Resources VisibilityFeatHLSL::getResources( const MaterialFeatureData &fd )
{
   Resources res; 

   // TODO: Fix for instancing.
   
   if ( !fd.features[ MFT_IsTranslucent ] )
      res.numTexReg = 1;

   return res;
}

//****************************************************************************
// AlphaTest
//****************************************************************************

void AlphaTestHLSL::processPix(  Vector<ShaderComponent*> &componentList,
                                 const MaterialFeatureData &fd )
{
   // If we're below SM3 and don't have a depth output
   // feature then don't waste an instruction here.
   if ( GFX->getPixelShaderVersion() < 3.0 &&
        !fd.features[ MFT_EyeSpaceDepthOut ]  &&
        !fd.features[ MFT_DepthOut ] )
   {
      output = NULL;
      return;
   }

   // If we don't have a color var then we cannot do an alpha test.
   Var *color = (Var*)LangElement::find( "col" );
   if ( !color )
   {
      output = NULL;
      return;
   }

   // Now grab the alpha test value.
   Var *alphaTestVal  = new Var;
   alphaTestVal->setType( "float" );
   alphaTestVal->setName( "alphaTestValue" );
   alphaTestVal->uniform = true;
   alphaTestVal->constSortPos = cspPotentialPrimitive;

   // Do the clip.
   output = new GenOp( "   clip( @.a - @ );\r\n", color, alphaTestVal );
}


//****************************************************************************
// GlowMask
//****************************************************************************

void GlowMaskHLSL::processPix(   Vector<ShaderComponent*> &componentList,
                                 const MaterialFeatureData &fd )
{
   output = NULL;

   // Get the output color... and make it black to mask out 
   // glow passes rendered before us.
   //
   // The shader compiler will optimize out all the other
   // code above that doesn't contribute to the alpha mask.
   Var *color = (Var*)LangElement::find( "col" );
   if ( color )
      output = new GenOp( "   @.rgb = 0;\r\n", color );
}


//****************************************************************************
// RenderTargetZero
//****************************************************************************

void RenderTargetZeroHLSL::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // Do not actually assign zero, but instead a number so close to zero it may as well be zero.
   // This will prevent a divide by zero causing an FP special on float render targets
   output = new GenOp( "   @;\r\n", sHelper->assignColor( new GenOp( "0.00001" ), Material::None, NULL, mOutputTargetMask ) );
}


//****************************************************************************
// HDR Output
//****************************************************************************

HDROutHLSL::HDROutHLSL()
   : mTorqueDep( "shaders/common/torque.hlsl" )
{
   addDependency( &mTorqueDep );
}

void HDROutHLSL::processPix(  Vector<ShaderComponent*> &componentList,
                              const MaterialFeatureData &fd )
{
   // Let the helper function do the work.
   Var *color = (Var*)LangElement::find( "col" );
   if ( color )
      output = new GenOp( "   @ = hdrEncode( @ );\r\n", color, color );
}

//****************************************************************************
// FoliageFeatureHLSL
//****************************************************************************

#include "T3D/fx/groundCover.h"

FoliageFeatureHLSL::FoliageFeatureHLSL()
: mDep( "shaders/common/foliage.hlsl" )
{
   addDependency( &mDep );
}

void FoliageFeatureHLSL::processVert( Vector<ShaderComponent*> &componentList, 
                                      const MaterialFeatureData &fd )
{      
   // Get the input variables we need.

   Var *inPosition = (Var*)LangElement::find( "inPosition" );
   if ( !inPosition )
      inPosition = (Var*)LangElement::find( "position" );
      
   Var *inColor = (Var*)LangElement::find( "diffuse" );   
   
   Var *inParams = (Var*)LangElement::find( "texCoord" );   

   MultiLine *meta = new MultiLine;

   // Declare the normal and tangent variables since they do not exist
   // in this vert type, but we do need to set them up for others.

   Var *normal = (Var*)LangElement::find( "normal" );   
   AssertFatal( normal, "FoliageFeatureHLSL requires vert normal!" );   

   Var *tangent = new Var;
   tangent->setType( "float3" );
   tangent->setName( "T" );
   LangElement *tangentDec = new DecOp( tangent );
   meta->addStatement( new GenOp( "   @;\n", tangentDec ) );         

   // We add a float foliageFade to the OUT structure.
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *fade = connectComp->getElement( RT_TEXCOORD );
   fade->setName( "foliageFade" );
   fade->setStructName( "OUT" );
   fade->setType( "float" );

   // grab the eye position
   Var *eyePos = (Var*)LangElement::find( "eyePosWorld" );
   if ( !eyePos )
   {
      eyePos = new Var( "eyePosWorld", "float3" );
      eyePos->uniform = true;
      eyePos->constSortPos = cspPass;
   }

   // All actual work is offloaded to this method.
   meta->addStatement( new GenOp( "   foliageProcessVert( @, @, @, @, @, @ );\r\n", inPosition, inColor, inParams, normal, tangent, eyePos ) );   

   // Assign to foliageFade. InColor.a was set to the correct value inside foliageProcessVert.
   meta->addStatement( new GenOp( "   @ = @.a;\r\n", fade, inColor ) );

   output = meta;
}

void FoliageFeatureHLSL::processPix( Vector<ShaderComponent*> &componentList, 
                                     const MaterialFeatureData &fd )
{
   // Find / create IN.foliageFade
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *fade = connectComp->getElement( RT_TEXCOORD );
   fade->setName( "foliageFade" );
   fade->setStructName( "IN" );
   fade->setType( "float" );
      
   // Find / create visibility
   Var *visibility = (Var*) LangElement::find( "visibility" );
   if ( !visibility )
   {
      visibility = new Var();
      visibility->setType( "float" );
      visibility->setName( "visibility" );
      visibility->uniform = true;
      visibility->constSortPos = cspPotentialPrimitive;  
   }      

   MultiLine *meta = new MultiLine;

   // Multiply foliageFade into visibility.
   meta->addStatement( new GenOp( "   @ *= @;\r\n", visibility, fade ) );

   output = meta;
}

void FoliageFeatureHLSL::determineFeature( Material *material, const GFXVertexFormat *vertexFormat, U32 stageNum, const FeatureType &type, const FeatureSet &features, MaterialFeatureData *outFeatureData )
{      
   // This isn't really necessary since the outFeatureData will be filtered after
   // this call.
   if ( features.hasFeature( MFT_Foliage  ) )
      outFeatureData->features.addFeature( type );
}


ShaderFeatureConstHandles* FoliageFeatureHLSL::createConstHandles( GFXShader *shader, SimObject *userObject )
{
   GroundCover *gcover = dynamic_cast< GroundCover* >( userObject );
   AssertFatal( gcover != NULL, "FoliageFeatureHLSL::createConstHandles - userObject was not valid!" );

   GroundCoverShaderConstHandles *handles = new GroundCoverShaderConstHandles();
   handles->mGroundCover = gcover;

   handles->init( shader );
   
   return handles;
}


void ParticleNormalFeatureHLSL::processVert(Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd)
{
   MultiLine *meta = new MultiLine;
   output = meta;

   // Calculate normal and tangent values since we want to keep particle verts
   // as light-weight as possible

   Var *normal = (Var*) LangElement::find("normal");
   if(normal == NULL)
   {
      normal = new Var;
      normal->setType( "float3" );
      normal->setName( "normal" );

      // These values are not accidental. It is slightly adjusted from facing straight into the
      // screen because there is a discontinuity at (0, 1, 0) for gbuffer encoding. Do not
      // cause this value to be (0, -1, 0) or interlaced normals will be discontinuous.
      // [11/23/2009 Pat]
      meta->addStatement(new GenOp("   @ = float3(0.0, -0.97, 0.14);\r\n", new DecOp(normal)));
   }

   Var *T = (Var*) LangElement::find( "T" );
   if(T == NULL)
   {
      T = new Var;
      T->setType( "float3" );
      T->setName( "T" );
      meta->addStatement(new GenOp("   @ = float3(0.0, 0.0, -1.0);\r\n", new DecOp(T)));
   }
}

//****************************************************************************
// ImposterVertFeatureHLSL
//****************************************************************************

ImposterVertFeatureHLSL::ImposterVertFeatureHLSL()
   :  mDep( "shaders/common/imposter.hlsl" )
{
   addDependency( &mDep );
}

void ImposterVertFeatureHLSL::processVert(   Vector<ShaderComponent*> &componentList, 
                                             const MaterialFeatureData &fd )
{      
   MultiLine *meta = new MultiLine;
   output = meta;

   // Get the input vertex variables.   
   Var *inPosition = (Var*)LangElement::find( "position" );
   Var *inMiscParams = (Var*)LangElement::find( "tcImposterParams" );   
   Var *inUpVec = (Var*)LangElement::find( "tcImposterUpVec" );   
   Var *inRightVec = (Var*)LangElement::find( "tcImposterRightVec" );

   // Get the input shader constants.
   Var *imposterLimits  = new Var;
   imposterLimits->setType( "float4" );
   imposterLimits->setName( "imposterLimits" );
   imposterLimits->uniform = true;
   imposterLimits->constSortPos = cspPotentialPrimitive;

   Var *imposterUVs  = new Var;
   imposterUVs->setType( "float4" );
   imposterUVs->setName( "imposterUVs" );
   imposterUVs->arraySize = 64; // See imposter.hlsl
   imposterUVs->uniform = true;
   imposterUVs->constSortPos = cspPotentialPrimitive;

   Var *eyePos = (Var*)LangElement::find( "eyePosWorld" );
   if ( !eyePos )
   {
      eyePos = new Var( "eyePosWorld", "float3" );
      eyePos->uniform = true;
      eyePos->constSortPos = cspPass;
   }

   // Declare the outputs from this feature.
   Var *outInPosition = new Var;
   outInPosition->setType( "float3" );
   outInPosition->setName( "inPosition" );
   meta->addStatement( new GenOp( "   @;\r\n", new DecOp( outInPosition ) ) );         

   Var *outTexCoord = new Var;
   outTexCoord->setType( "float2" );
   outTexCoord->setName( "texCoord" );
   meta->addStatement( new GenOp( "   @;\r\n", new DecOp( outTexCoord ) ) );         

   Var *outWorldToTangent = new Var;
   outWorldToTangent->setType( "float3x3" );
   outWorldToTangent->setName( "worldToTangent" );
   meta->addStatement( new GenOp( "   @;\r\n", new DecOp( outWorldToTangent ) ) );         

   // Add imposterFade to the OUT structure.
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *outFade = connectComp->getElement( RT_TEXCOORD );
   outFade->setName( "imposterFade" );
   outFade->setStructName( "OUT" );
   outFade->setType( "float" ); 

   // Assign OUT.imposterFade
   meta->addStatement( new GenOp( "   @ = @.y;\r\n", outFade, inMiscParams ) );

   // All actual work is done in this method.
   meta->addStatement( new GenOp( "   imposter_v( @.xyz, @.w, @.x * length(@), normalize(@), normalize(@), @.y, @.x, @.z, @.w, @, @, @, @, @ );\r\n",

                        inPosition,
                        inPosition,

                        inMiscParams,
                        inRightVec,

                        inUpVec,
                        inRightVec,

                        imposterLimits,
                        imposterLimits,
                        imposterLimits,
                        imposterLimits,

                        eyePos,
                        imposterUVs,

                        outInPosition, 
                        outTexCoord,
                        outWorldToTangent ) );

   // Copy the position to wsPosition for use in shaders 
   // down stream instead of looking for objTrans.
   Var *wsPosition = new Var;
   wsPosition->setType( "float3" );
   wsPosition->setName( "wsPosition" );
   meta->addStatement( new GenOp( "   @ = @.xyz;\r\n", new DecOp( wsPosition ), outInPosition ) ); 

   // If we new viewToTangent... its the same as the
   // world to tangent for an imposter.
   Var *viewToTangent = new Var;
   viewToTangent->setType( "float3x3" );
   viewToTangent->setName( "viewToTangent" );
   meta->addStatement( new GenOp( "   @ = @;\r\n", new DecOp( viewToTangent ), outWorldToTangent ) );       
}

void ImposterVertFeatureHLSL::processPix( Vector<ShaderComponent*> &componentList,
                                          const MaterialFeatureData &fd )
{
   // Find / create IN.imposterFade
   ShaderConnector *connectComp = dynamic_cast<ShaderConnector *>( componentList[C_CONNECTOR] );
   Var *fade = connectComp->getElement( RT_TEXCOORD );
   fade->setName( "imposterFade" );
   fade->setStructName( "IN" );
   fade->setType( "float" );
      
   // Find / create visibility
   Var *visibility = (Var*) LangElement::find( "visibility" );
   if ( !visibility )
   {
      visibility = new Var();
      visibility->setType( "float" );
      visibility->setName( "visibility" );
      visibility->uniform = true;
      visibility->constSortPos = cspPotentialPrimitive;  
   }      

   MultiLine *meta = new MultiLine;

   // Multiply foliageFade into visibility.
   meta->addStatement( new GenOp( "   @ *= @;\r\n", visibility, fade ) );

   output = meta;
}

void ImposterVertFeatureHLSL::determineFeature( Material *material, 
                                                const GFXVertexFormat *vertexFormat, 
                                                U32 stageNum, 
                                                const FeatureType &type, 
                                                const FeatureSet &features, 
                                                MaterialFeatureData *outFeatureData )
{      
   if ( features.hasFeature( MFT_ImposterVert ) )
      outFeatureData->features.addFeature( MFT_ImposterVert );
}

