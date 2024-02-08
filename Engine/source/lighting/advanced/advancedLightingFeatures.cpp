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
#include "lighting/advanced/advancedLightingFeatures.h"

#include "core/util/safeDelete.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "lighting/advanced/advancedLightingFeatures.h"
#include "lighting/advanced/gBufferConditioner.h"
#include "lighting/advanced/glsl/advancedLightingFeaturesGLSL.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialParameters.h"
#include "materials/matTextureTarget.h"
#include "materials/processedMaterial.h"
#include "shaderGen/featureMgr.h"

#ifdef TORQUE_OS_WIN32
#include "lighting/advanced/hlsl/advancedLightingFeaturesHLSL.h"
#endif

bool AdvancedLightingFeatures::smFeaturesRegistered = false;

void AdvancedLightingFeatures::registerFeatures( const GFXFormat &prepassTargetFormat, const GFXFormat &lightInfoTargetFormat )
{
   AssertFatal( !smFeaturesRegistered, "AdvancedLightingFeatures::registerFeatures() - Features already registered. Bad!" );

   // If we ever need this...
   TORQUE_UNUSED(lightInfoTargetFormat);

   ConditionerFeature *cond = new GBufferConditioner( prepassTargetFormat, GBufferConditioner::ViewSpace );
   FEATUREMGR->registerFeature(MFT_PrePassConditioner, cond);

   if (GFX->getAdapterType() == OpenGL)
   {
      FEATUREMGR->registerFeature(MFT_RTLighting, new DeferredRTLightingFeatGLSL());
      FEATUREMGR->registerFeature(MFT_NormalMap, new DeferredBumpFeat());
      FEATUREMGR->registerFeature(MFT_PixSpecular, new DeferredPixelSpecularGLSL());
      FEATUREMGR->registerFeature(MFT_MinnaertShading, new DeferredMinnaertGLSL());
      FEATUREMGR->registerFeature(MFT_SubSurface, new DeferredSubSurfaceGLSL());
   }
   else
   {
#ifdef TORQUE_OS_WIN32
      FEATUREMGR->registerFeature(MFT_RTLighting, new DeferredRTLightingFeatHLSL());
      FEATUREMGR->registerFeature(MFT_NormalMap, new DeferredBumpFeat());
      FEATUREMGR->registerFeature(MFT_PixSpecular, new DeferredPixelSpecularHLSL());
      FEATUREMGR->registerFeature(MFT_MinnaertShading, new DeferredMinnaertHLSL());
      FEATUREMGR->registerFeature(MFT_SubSurface, new DeferredSubSurfaceHLSL());
#endif
   }

   NamedTexTarget *target = NamedTexTarget::find( "prepass" );
   if ( target )
      target->setConditioner( cond );

   smFeaturesRegistered = true;
}

void AdvancedLightingFeatures::unregisterFeatures()
{
   NamedTexTarget *target = NamedTexTarget::find( "prepass" );
   if ( target )
      target->setConditioner( NULL );

   FEATUREMGR->unregisterFeature(MFT_PrePassConditioner);
   FEATUREMGR->unregisterFeature(MFT_RTLighting);
   FEATUREMGR->unregisterFeature(MFT_NormalMap);
   FEATUREMGR->unregisterFeature(MFT_PixSpecular);
   FEATUREMGR->unregisterFeature(MFT_MinnaertShading);
   FEATUREMGR->unregisterFeature(MFT_SubSurface);

   smFeaturesRegistered = false;
}

void DeferredBumpFeat::processVert( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   if( fd.features[MFT_PrePassConditioner] )
   {
      // There is an output conditioner active, so we need to supply a transform
      // to the pixel shader.
      MultiLine *meta = new MultiLine;

      // We need the view to tangent space transform in the pixel shader.
      sHelper->getOutViewToTangent( componentList, mInstancingFormat, meta, fd );

      // Make sure there are texcoords
      if( !fd.features[MFT_Parallax] && !fd.features[MFT_DiffuseMap] )
      {
         const bool useTexAnim = fd.features[MFT_TexAnim];

         sHelper->getOutTexCoord( "texCoord", "float2", true, useTexAnim, meta, componentList );

         if ( fd.features.hasFeature( MFT_DetailNormalMap ) )
            sHelper->addOutDetailTexCoord( componentList, meta, useTexAnim );
      }

      output = meta;
   }
   else if (   fd.materialFeatures[MFT_NormalsOut] ||
               fd.features[MFT_ForwardShading] ||
               !fd.features[MFT_RTLighting] )
   {
      Parent::processVert( componentList, fd );
      return;
   }
   else
   {
      output = NULL;
   }
}

void DeferredBumpFeat::processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
{
   // NULL output in case nothing gets handled
   output = NULL;

   if( fd.features[MFT_PrePassConditioner] )
   {
      MultiLine *meta = new MultiLine;

      Var *viewToTangent = sHelper->getInViewToTangent( componentList );

      // create texture var
      Var *bumpMap = sHelper->getNormalMapTex();
      Var *texCoord = sHelper->getInTexCoord( "texCoord", "float2", true, componentList );
      LangElement *texOp = new GenOp( "tex2D(@, @)", bumpMap, texCoord );

      // create bump normal
      Var *bumpNorm = new Var;
      bumpNorm->setName( "bumpNormal" );
      bumpNorm->setType( "float4" );

      LangElement *bumpNormDecl = new DecOp( bumpNorm );
      meta->addStatement( sHelper->expandNormalMap( texOp, bumpNormDecl, bumpNorm, fd, getProcessIndex() ) );

      // If we have a detail normal map we add the xy coords of
      // it to the base normal map.  This gives us the effect we
      // want with few instructions and minial artifacts.
      if ( fd.features.hasFeature( MFT_DetailNormalMap ) )
      {
         bumpMap = new Var;
         bumpMap->setType( "sampler2D" );
         bumpMap->setName( "detailBumpMap" );
         bumpMap->uniform = true;
         bumpMap->sampler = true;
         bumpMap->constNum = Var::getTexUnitNum();

         texCoord = sHelper->getInTexCoord( "detCoord", "float2", true, componentList );
         texOp = new GenOp( "tex2D(@, @)", bumpMap, texCoord );

         Var *detailBump = new Var;
         detailBump->setName( "detailBump" );
         detailBump->setType( "float4" );
         meta->addStatement( sHelper->expandNormalMap( texOp, new DecOp( detailBump ), detailBump, fd, getProcessIndex() ) );

         Var *detailBumpScale = new Var;
         detailBumpScale->setType( "float" );
         detailBumpScale->setName( "detailBumpStrength" );
         detailBumpScale->uniform = true;
         detailBumpScale->constSortPos = cspPass;
         meta->addStatement( new GenOp( "   @.xy += @.xy * @;\r\n", bumpNorm, detailBump, detailBumpScale ) );
      }

      // This var is read from GBufferConditioner and
      // used in the prepass output.
      //
      // By using the 'half' type here we get a bunch of partial
      // precision optimized code on further operations on the normal
      // which helps alot on older Geforce cards.
      //
      Var *gbNormal = new Var;
      gbNormal->setName( "gbNormal" );
      if ( GFX->getAdapterType() == Direct3D9 )
         gbNormal->setType( "half3" );
      else
         gbNormal->setType( "float3" );
      LangElement *gbNormalDecl = new DecOp( gbNormal );

      // Normalize is done later...
      // Note: The reverse mul order is intentional. Affine matrix.
      if ( GFX->getAdapterType() == Direct3D9 )
         meta->addStatement( new GenOp( "   @ = (half3)mul( @.xyz, @ );\r\n", gbNormalDecl, bumpNorm, viewToTangent ) );
      else
         meta->addStatement( new GenOp( "   @ = @ * @.xyz;\r\n", gbNormalDecl, viewToTangent, bumpNorm ) );

      output = meta;
      return;
   }
   else if (   fd.materialFeatures[MFT_NormalsOut] ||
               fd.features[MFT_ForwardShading] ||
               !fd.features[MFT_RTLighting] )
   {
      Parent::processPix( componentList, fd );
      return;
   }
   else if ( fd.features[MFT_PixSpecular] && !fd.features[MFT_SpecularMap] )
   {
      Var *bumpSample = (Var *)LangElement::find( "bumpSample" );
      if( bumpSample == NULL )
      {
         Var *texCoord = sHelper->getInTexCoord( "texCoord", "float2", true, componentList );

         Var *bumpMap = sHelper->getNormalMapTex();

         bumpSample = new Var;
         bumpSample->setType( "float4" );
         bumpSample->setName( "bumpSample" );
         LangElement *bumpSampleDecl = new DecOp( bumpSample );

         output = new GenOp( "   @ = tex2D(@, @);\r\n", bumpSampleDecl, bumpMap, texCoord );
         return;
      }
   }

   output = NULL;
}

ShaderFeature::Resources DeferredBumpFeat::getResources( const MaterialFeatureData &fd )
{
   if (  fd.materialFeatures[MFT_NormalsOut] ||
         fd.features[MFT_ForwardShading] ||
         fd.features[MFT_Parallax] ||
         !fd.features[MFT_RTLighting] )
      return Parent::getResources( fd );

   Resources res;
   if(!fd.features[MFT_SpecularMap])
   {
      res.numTex = 1;
      res.numTexReg = 1;

      if (  fd.features[MFT_PrePassConditioner] &&
            fd.features.hasFeature( MFT_DetailNormalMap ) )
      {
         res.numTex += 1;
         if ( !fd.features.hasFeature( MFT_DetailMap ) )
            res.numTexReg += 1;
      }
   }

   return res;
}

void DeferredBumpFeat::setTexData( Material::StageData &stageDat, const MaterialFeatureData &fd, RenderPassData &passData, U32 &texIndex )
{
   if (  fd.materialFeatures[MFT_NormalsOut] ||
         fd.features[MFT_ForwardShading] ||
         !fd.features[MFT_RTLighting] )
   {
      Parent::setTexData( stageDat, fd, passData, texIndex );
      return;
   }

   if (  !fd.features[MFT_Parallax] && !fd.features[MFT_SpecularMap] &&
         ( fd.features[MFT_PrePassConditioner] ||
           fd.features[MFT_PixSpecular] ) )
   {
      TexBind& bind = passData.mTexBind[ texIndex++ ];
      bind.samplerName = "$bumpMap";
      bind.type = Material::Bump;
      bind.object = stageDat.getTex( MFT_NormalMap );

      if (  fd.features[MFT_PrePassConditioner] &&
            fd.features.hasFeature( MFT_DetailNormalMap ) )
      {
         TexBind& bind = passData.mTexBind[ texIndex++ ];
         bind.samplerName = "$detailBumpMap";
         bind.type = Material::DetailBump;
         bind.object = stageDat.getTex( MFT_DetailNormalMap );
      }
   }
}
