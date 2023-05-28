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

#include "platformMac/macCarbFont.h"
#include "platformMac/platformMacCarb.h"
#include "math/mRect.h"
#include "console/console.h"
#include "core/strings/unicode.h"
#include "core/stringTable.h"
#include "core/strings/stringFunctions.h"


//------------------------------------------------------------------------------
// New Unicode capable font class.
PlatformFont *createPlatformFont(const char *name, U32 size, U32 charset /* = TGE_ANSI_CHARSET */)
{
    PlatformFont *retFont = new MacCarbFont;

    if(retFont->create(name, size, charset))
        return retFont;

    delete retFont;
    return NULL;
}

//------------------------------------------------------------------------------
MacCarbFont::MacCarbFont()
{
#if MAC_USE_CORE_TEXT
   mFont = NULL;
#else
   mStyle   = NULL;
   mLayout  = NULL;
#endif

   mColorSpace = NULL;
}

MacCarbFont::~MacCarbFont()
{
#if MAC_USE_CORE_TEXT
   CFRelease(mFont);
#else
   // apple docs say we should dispose the layout first.
   ATSUDisposeTextLayout(mLayout);
   ATSUDisposeStyle(mStyle);
#endif

   CGColorSpaceRelease(mColorSpace);
}

//------------------------------------------------------------------------------
bool MacCarbFont::create( const char* name, U32 size, U32 charset)
{
   String nameStr = name;
   nameStr = nameStr.trim();
   
   // create and cache the style and layout.
   // based on apple sample code at http://developer.apple.com/qa/qa2001/qa1027.html

   // note: charset is ignored on mac. -- we don't need it to get the right chars.
   // But do we need it to translate encodings? hmm...

   CFStringRef       cfsName;
   ATSFontMetrics    fontMetrics;

#if !MAC_USE_CORE_TEXT
   ATSFontRef        atsFontRef;
   ATSUFontID        atsuFontID;
   Fixed             atsuSize;
   ATSURGBAlphaColor black;
#endif
   
   bool              isBold = false;
   bool              isItalic = false;
   
   bool haveModifier;
   do
   {
      haveModifier = false;
      if( nameStr.compare( "Bold", 4, String::NoCase | String::Right ) == 0 )
      {
         isBold = true;
         nameStr = nameStr.substr( 0, nameStr.length() - 4 ).trim();
         haveModifier = true;
      }
      if( nameStr.compare( "Italic", 6, String::NoCase | String::Right ) == 0 )
      {
         isItalic = true;
         nameStr = nameStr.substr( 0, nameStr.length() - 6 ).trim();
         haveModifier = true;
      }
   }
   while( haveModifier );
      
   // Look up the font. We need it in 2 differnt formats, for differnt Apple APIs.
   cfsName = CFStringCreateWithCString( kCFAllocatorDefault, nameStr.c_str(), kCFStringEncodingUTF8);
   if(!cfsName)
      Con::errorf("Error: could not make a cfstring out of \"%s\" ",nameStr.c_str());
      
   size = mFloor(((double)size) * 72.0 / 96.0);
   mSize = size;

#if MAC_USE_CORE_TEXT
   CTFontDescriptorRef fontDesc = CTFontDescriptorCreateWithNameAndSize(cfsName, size);
   CTFontDescriptorRef lastFontDesc;

   if (isBold)
   {
      lastFontDesc = fontDesc;
      fontDesc = CTFontDescriptorCreateCopyWithSymbolicTraits(lastFontDesc, kCTFontBoldTrait, kCTFontBoldTrait);
      CFRelease(lastFontDesc);
   }

   if (isItalic)
   {
      lastFontDesc = fontDesc;
      fontDesc = CTFontDescriptorCreateCopyWithSymbolicTraits(lastFontDesc, kCTFontItalicTrait, kCTFontItalicTrait);
      CFRelease(lastFontDesc);
   }

   mFont = CTFontCreateWithFontDescriptor(fontDesc, size, NULL);
   CFRelease(fontDesc);

   fontMetrics.ascent = CTFontGetAscent(mFont) / size;
   fontMetrics.descent = CTFontGetDescent(mFont) / size * -1.0;
#else
   atsFontRef = ATSFontFindFromName( cfsName, kATSOptionFlagsDefault);
   atsuFontID = FMGetFontFromATSFontRef( atsFontRef);

   // make sure we found it. ATSFontFindFromName() appears to return 0 if it cant find anything. Apple docs contain no info on error returns.
   if( !atsFontRef || !atsuFontID )
   {
      Con::errorf("MacCarbFont::create - could not load font -%s-",name);
      return false;
   }

   // Set up the size and color. We send these to ATSUSetAttributes().
   atsuSize = IntToFixed(size);
   black.red = black.green = black.blue = black.alpha = 1.0;

   // Three parrallel arrays for setting up font, size, and color attributes.
   ATSUAttributeTag theTags[] = { kATSUFontTag, kATSUSizeTag, kATSURGBAlphaColorTag};
   ByteCount theSizes[] = { sizeof(ATSUFontID), sizeof(Fixed), sizeof(ATSURGBAlphaColor) };
   ATSUAttributeValuePtr theValues[] = { &atsuFontID, &atsuSize, &black };
   
   // create and configure the style object.
   ATSUCreateStyle(&mStyle);
   ATSUSetAttributes( mStyle, 3, theTags, theSizes, theValues );
   
   if( isBold )
   {
      ATSUAttributeTag tag = kATSUQDBoldfaceTag;
      ByteCount size = sizeof( Boolean );
      Boolean value = true;
      ATSUAttributeValuePtr valuePtr = &value;
      ATSUSetAttributes( mStyle, 1, &tag, &size, &valuePtr );
   }
   
   if( isItalic )
   {
      ATSUAttributeTag tag = kATSUQDItalicTag;
      ByteCount size = sizeof( Boolean );
      Boolean value = true;
      ATSUAttributeValuePtr valuePtr = &value;
      ATSUSetAttributes( mStyle, 1, &tag, &size, &valuePtr );
   }
   
   // create the layout object, 
   ATSUCreateTextLayout(&mLayout);  
   // we'll bind the layout to a bitmap context when we actually draw.
   // ATSUSetTextPointerLocation()  - will set the text buffer
   // ATSUSetLayoutControls()       - will set the cg context.
   
   // get font metrics, save our baseline and height
   ATSFontGetHorizontalMetrics(atsFontRef, kATSOptionFlagsDefault, &fontMetrics);
#endif

   mBaseline = size * fontMetrics.ascent;
   mHeight = size * (fontMetrics.ascent - fontMetrics.descent);

   // cache our grey color space, so we dont have to re create it every time.
   mColorSpace = CGColorSpaceCreateDeviceGray();
   
   // and finally cache the font's name. We use this to cheat some antialiasing options below.
   mName = StringTable->insert(name);
   
   return true;
}    

//------------------------------------------------------------------------------
bool MacCarbFont::isValidChar(const UTF8 *str) const
{
   // since only low order characters are invalid, and since those characters
   // are single codeunits in UTF8, we can safely cast here.
   return isValidChar((UTF16)*str);  
}

bool MacCarbFont::isValidChar( const UTF16 ch) const
{
   // We cut out the ASCII control chars here. Only printable characters are valid.
   // 0x20 == 32 == space
   if( ch < 0x20 )
      return false;

#if MAC_USE_CORE_TEXT
   CGGlyph cgGlyphs[1] = { 0 };
   UniChar chUniChar[1] = { ch };
   if (!CTFontGetGlyphsForCharacters(mFont, chUniChar, cgGlyphs, 1))
      return false;
#endif

   return true;
}

PlatformFont::CharInfo& MacCarbFont::getCharInfo(const UTF8 *str) const
{
   return getCharInfo(oneUTF32toUTF16(oneUTF8toUTF32(str,NULL)));
}

PlatformFont::CharInfo& MacCarbFont::getCharInfo(const UTF16 ch) const
{
   // We use some static data here to avoid re allocating the same variable in a loop.
   // this func is primarily called by GFont::loadCharInfo(),
   Rect         imageRect;
   CGContextRef imageCtx;
   U32          bitmapDataSize;

   // 16 bit character buffer for the ATUSI calls.
   // -- hey... could we cache this at the class level, set style and loc *once*, 
   //    then just write to this buffer and clear the layout cache, to speed up drawing?
   static UniChar chUniChar[1];
   chUniChar[0] = ch;

   // Declare and clear out the CharInfo that will be returned.
   static PlatformFont::CharInfo c;
   dMemset(&c, 0, sizeof(c));

#if MAC_USE_CORE_TEXT
   CGRect cgImageRect;
   CGGlyph cgGlyphs[1] = { 0 };
   if (!CTFontGetGlyphsForCharacters(mFont, chUniChar, cgGlyphs, 1))
      return c;

   CTFontGetBoundingRectsForGlyphs(mFont, kCTFontOrientationDefault, cgGlyphs, &cgImageRect, 1);

   imageRect.left = mFloor(cgImageRect.origin.x);
   imageRect.right = mCeil(cgImageRect.origin.x + cgImageRect.size.width);
   imageRect.top = mCeil(cgImageRect.origin.y); // Things look bettter with ceil instead of floor here for some reason.
   imageRect.bottom = mCeil(cgImageRect.origin.y + cgImageRect.size.height);

   CGSize cgAdvance;
   CTFontGetAdvancesForGlyphs(mFont, kCTFontOrientationDefault, cgGlyphs, &cgAdvance, 1);
   c.xIncrement = mCeil(cgAdvance.width);
#else
   ATSUTextMeasurement tbefore, tafter, tascent, tdescent;
   OSStatus err;

   // put the text in the layout.
   // we've hardcoded a string length of 1 here, but this could work for longer strings... (hint hint)
   // note: ATSUSetTextPointerLocation() also clears the previous cached layout information.
   ATSUSetTextPointerLocation( mLayout, chUniChar, 0, 1, 1);
   ATSUSetRunStyle( mLayout, mStyle, 0,1);
   
   // get the typographic bounds. this tells us how characters are placed relative to other characters.
   ATSUGetUnjustifiedBounds( mLayout, 0, 1, &tbefore, &tafter, &tascent, &tdescent);
   c.xIncrement =  FixedToInt(tafter);
   
   // find out how big of a bitmap we'll need.
   // as a bonus, we also get the origin where we should draw, encoded in the Rect.
   ATSUMeasureTextImage( mLayout, 0, 1, 0, 0, &imageRect);
#endif

   int xFudge = 2;
   int yFudge = 2;
   int xoff = xFudge / 2;
   int yoff = yFudge / 2;

   c.width  = imageRect.right - imageRect.left + xFudge; // add 2 because small fonts don't always have enough room
   c.height = imageRect.bottom - imageRect.top + yFudge;
   c.xOrigin = imageRect.left - xoff; // dist x0 -> center line

#if MAC_USE_CORE_TEXT
   c.yOrigin = imageRect.bottom + yoff; // dist y0 -> base line
#else
   c.yOrigin = -imageRect.top + yoff; // dist y0 -> base line
#endif
   
   // kick out early if the character is undrawable
   if( c.width == xFudge || c.height == yFudge)
      return c;
   
   // allocate a greyscale bitmap and clear it.
   bitmapDataSize = c.width * c.height;
   c.bitmapData = new U8[bitmapDataSize];
   dMemset(c.bitmapData,0x00,bitmapDataSize);
   
   // get a graphics context on the bitmap
   imageCtx = CGBitmapContextCreate( c.bitmapData, c.width, c.height, 8, c.width, mColorSpace, kCGImageAlphaNone);
   if(!imageCtx) {
      Con::errorf("Error: failed to create a graphics context on the CharInfo bitmap! Drawing a blank block.");
      c.xIncrement = c.width;
      dMemset(c.bitmapData,0x0F,bitmapDataSize);
      return c;
   }

   // Turn off antialiasing for monospaced console fonts. yes, this is cheating.
   if(mSize < 12  && ( dStrstr(mName,"Monaco")!=NULL || dStrstr(mName,"Courier")!=NULL ))
      CGContextSetShouldAntialias(imageCtx, false);

   // Set up drawing options for the context.
   // Since we're not going straight to the screen, we need to adjust accordingly
   CGContextSetShouldSmoothFonts(imageCtx, false);
   CGContextSetRenderingIntent(imageCtx, kCGRenderingIntentAbsoluteColorimetric);
   CGContextSetInterpolationQuality( imageCtx, kCGInterpolationNone);
   CGContextSetGrayFillColor( imageCtx, 1.0, 1.0);
   CGContextSetTextDrawingMode( imageCtx,  kCGTextFill);
   
#if MAC_USE_CORE_TEXT
   CGPoint positions[1];
   positions[0].x = -imageRect.left + xoff;
   positions[0].y = -imageRect.top + yoff;
   CTFontDrawGlyphs(mFont, cgGlyphs, positions, 1, imageCtx);
#else
   // tell ATSUI to substitute fonts as needed for missing glyphs
   ATSUSetTransientFontMatching(mLayout, true); 

   // set up three parrallel arrays for setting up attributes. 
   // this is how most options in ATSUI are set, by passing arrays of options.
   ATSUAttributeTag theTags[] = { kATSUCGContextTag };
   ByteCount theSizes[] = { sizeof(CGContextRef) };
   ATSUAttributeValuePtr theValues[] = { &imageCtx };
   
   // bind the layout to the context.
   ATSUSetLayoutControls( mLayout, 1, theTags, theSizes, theValues );

   // Draw the character!
   err = ATSUDrawText( mLayout, 0, 1, IntToFixed(-imageRect.left + xoff), IntToFixed(imageRect.bottom + yoff ) );
   
   if(err != noErr) {
      Con::errorf("Error: could not draw the character! Drawing a blank box.");
      dMemset(c.bitmapData,0x0F,bitmapDataSize);
   }

#if TORQUE_DEBUG
//   Con::printf("Font Metrics: Rect = %2i %2i %2i %2i  Char= %C, 0x%x  Size= %i, Baseline= %i, Height= %i",imageRect.top, imageRect.bottom, imageRect.left, imageRect.right,ch,ch, mSize,mBaseline, mHeight);
//   Con::printf("Font Bounds:  left= %2i right= %2i  Char= %C, 0x%x  Size= %i",FixedToInt(tbefore), FixedToInt(tafter), ch,ch, mSize);
#endif
#endif

   CGContextRelease(imageCtx);

   return c;
}

void PlatformFont::enumeratePlatformFonts( Vector< StringTableEntry >& fonts, UTF16* fontFamily )
{
   if( fontFamily )
   {
#if MAC_USE_CORE_TEXT
      AssertFatal(false, "Not implemented.");
#else
      // Determine the font ID from the family name.

      ATSUFontID fontID;
      if( ATSUFindFontFromName(
            fontFamily,
            dStrlen( fontFamily ) * 2,
            kFontFamilyName,
            kFontMicrosoftPlatform,
            kFontNoScriptCode,
            kFontNoLanguageCode, &fontID ) != kATSUInvalidFontErr )
      {
         // Get the number of fonts in the family.
         
         ItemCount numFonts;
         ATSUCountFontNames( fontID, &numFonts );
         
         // Read out font names.
         
         U32 bufferSize = 512;
         char* buffer = ( char* ) dMalloc( bufferSize );
         
         for( U32 i = 0; i < numFonts; ++ i )
         {
            for( U32 n = 0; n < 2; ++ n )
            {
               ByteCount actualNameLength;
               FontNameCode fontNameCode;
               FontPlatformCode fontPlatformCode;
               FontScriptCode fontScriptCode;
               FontLanguageCode fontLanguageCode;
               
               if( ATSUGetIndFontName(
                     fontID,
                     i,
                     bufferSize - 2,
                     buffer,
                     &actualNameLength,
                     &fontNameCode,
                     &fontPlatformCode,
                     &fontScriptCode,
                     &fontLanguageCode ) == noErr )
               {
                  *( ( UTF16* ) &buffer[ actualNameLength ] ) = '\0';
                  char* utf8 = convertUTF16toUTF8( ( UTF16* ) buffer );
                  fonts.push_back( StringTable->insert( utf8 ) );
                  delete [] utf8;
                  break;
               }
               
               // Allocate larger buffer.
               
               bufferSize = actualNameLength + 2;
               buffer = ( char* ) dRealloc( buffer, bufferSize );
            }
         }
         
         dFree( buffer );
      }
   }
   else
   {
      // Get the number of installed fonts.
      
      ItemCount numFonts;
      ATSUFontCount( &numFonts );
      
      // Get all the font IDs.
      
      ATSUFontID* fontIDs = new ATSUFontID[ numFonts ];
      if( ATSUGetFontIDs( fontIDs, numFonts, &numFonts ) == noErr )
      {
         U32 bufferSize = 512;
         char* buffer = ( char* ) dMalloc( bufferSize );
         
         // Read all family names.
         
         for( U32 i = 0; i < numFonts; ++ i )
         {
            for( U32 n = 0; n < 2; ++ n )
            {
               ByteCount actualNameLength;
               ItemCount fontIndex;
               
               OSStatus result = ATSUFindFontName(
                     fontIDs[ i ],
                     kFontFamilyName,
                     kFontMicrosoftPlatform,
                     kFontNoScriptCode,
                     kFontNoLanguageCode,
                     bufferSize - 2,
                     buffer,
                     &actualNameLength,
                     &fontIndex );
               
               if( result == kATSUNoFontNameErr )
                  break;
               else if( result == noErr )
               {
                  *( ( UTF16* ) &buffer[ actualNameLength ] ) = '\0';
                  char* utf8 = convertUTF16toUTF8( ( UTF16* ) buffer );
                  StringTableEntry name = StringTable->insert( utf8 );
                  delete [] utf8;
                  
                  // Avoid duplicates.
                  
                  bool duplicate = false;
                  for( U32 i = 0, num = fonts.size(); i < num; ++ i )
                     if( fonts[ i ] == name )
                     {
                        duplicate = true;
                        break;
                     }
                     
                  if( !duplicate )
                     fonts.push_back( name );
                     
                  break;
               }
               
               // Allocate larger buffer.
               
               bufferSize = actualNameLength + 2;
               buffer = ( char* ) dRealloc( buffer, bufferSize );
            }
         }
         
         dFree( buffer );
      }
      
      delete [] fontIDs;
#endif
   }
}

//-----------------------------------------------------------------------------
// The following code snippet demonstrates how to get the elusive GlyphIDs, 
// which are needed when you want to do various complex and arcane things
// with ATSUI and CoreGraphics.
//
//  ATSUGlyphInfoArray glyphinfoArr;
//  ATSUGetGlyphInfo( mLayout, kATSUFromTextBeginning, kATSUToTextEnd,sizeof(ATSUGlyphInfoArray), &glyphinfoArr);
//  ATSUGlyphInfo glyphinfo = glyphinfoArr.glyphs[0];
//  Con::printf(" Glyphinfo: screenX= %i, idealX=%f, deltaY=%f", glyphinfo.screenX, glyphinfo.idealX, glyphinfo.deltaY);
