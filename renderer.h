/*
  Copyright (C) 2022 Jerry R. VanAken

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/
//---------------------------------------------------------------------
//
// renderer.h:
//   Header file for rendering code. This header defines the
//   interfaces for renderers, paint generators, and image readers.
//
//---------------------------------------------------------------------

#ifndef RENDERER_H
  #define RENDERER_H

#include "shapegen.h"

typedef unsigned int COLOR;

// Macro definitions 
#define RGBX(r,g,b)    (COLOR)(((r)&255)|(((g)&255)<<8)|(((b)&255)<<16)|0xff000000)
#define RGBA(r,g,b,a)  (COLOR)(((r)&255)|(((g)&255)<<8)|(((b)&255)<<16)|((a)<<24))
#define ARRAY_LEN(a)  (sizeof(a)/sizeof((a)[0]))

// Maximum number of color stops for gradient fill
const int STOPARRAY_MAXLEN = 32;

// Gradient color-stop array element in renderer
struct COLOR_STOP
{
    float offset;
    COLOR color;
};

// Gradient spread type
enum SPREAD_METHOD
{
    SPREAD_UNKNOWN,
    SPREAD_PAD,
    SPREAD_REFLECT,
    SPREAD_REPEAT,
};

// Flags for pattern fills and gradient fills
const int FLAG_EXTEND_START = 1;
const int FLAG_EXTEND_END = 2;
const int FLAG_IMAGE_BOTTOMUP = 4;
const int FLAG_IMAGE_BGRA32 = 8;
const int FLAG_SWAP_REDBLUE = 16;
const int FLAG_PREMULTALPHA = 32;

//---------------------------------------------------------------------
//
// Class ImageReader: Reads 32-bit pixel data from a .bmp file or
// other source. Users can pass an object of this type to an
// EnhancedRenderer::SetPattern function, which then calls the
// ImageReader::ReadPixels function to load a 2-D image to use for
// pattern fills. The ReadPixels function's 'count' parameter
// specifies the number of pixels to copy from the source (for
// example, a bitmap file) to the specified 'buffer' array. The
// return value is the number of pixels the function has copied,
// which can be less than 'count' if the source supply of pixels is
// low, or zero if the source is empty.  
//
//---------------------------------------------------------------------

class ImageReader
{
public:
    virtual int ReadPixels(COLOR *buffer, int count) = 0;      
};

//---------------------------------------------------------------------
//
// SimpleRenderer class: User interface for a simple renderer, which
// is derived from the Renderer base class defined in shapegen.h. This
// renderer fills shapes with solid colors.
//
//---------------------------------------------------------------------

class SimpleRenderer : public Renderer
{
public:
    virtual void SetColor(COLOR color) = 0;
};

//---------------------------------------------------------------------
//
// An enhanced renderer derived from the SimpleRenderer base class.
// This type of renderer works exclusively with full-color displays,
// and does antialiasing, alpha blending, solid-color fills, pattern
// fills, linear gradient fills, and radial gradient fills.
//
//---------------------------------------------------------------------

class EnhancedRenderer : public SimpleRenderer
{
public:
    virtual void SetColor(COLOR color) = 0;
    virtual void SetLinearGradient(float x0, float y0, float x1, float y1,
                                   SPREAD_METHOD spread, int flags) = 0;
    virtual void SetRadialGradient(float x0, float y0, float r0,
                                   float x1, float y1, float r1,
                                   SPREAD_METHOD spread, int flags) = 0;
    virtual void SetPattern(const COLOR *pattern, float u0, float v0, 
                            int w, int h, int stride, int flags) = 0;
    virtual void SetPattern(ImageReader *imgrdr, float u0, float v0, 
                            int w, int h, int flags) = 0;
    virtual void AddColorStop(float offset, COLOR color) = 0;
    virtual void ResetColorStops() = 0;
    virtual void SetTransform(const float xform[] = 0) = 0;
    virtual void SetConstantAlpha(COLOR alpha) = 0;
};

//-----------------------------------------------------------------------
//
// PaintGen class: Paint generator for exclusive use by renderers. The
// FillSpan function generates a span (horizontal row of pixels) painted
// according to some specified function -- some examples are pattern
// fills and gradient fills. The span starts at pixel coordinates
// (xs,ys) and extends (to the right) for 'len' pixels. The inAlpha
// array supplies the input alpha values to be pre-mixed with the paint,
// and the resulting painted pixels are written to the outBuf array.
// Both arrays are of length 'len'. However, an inAlpha value of zero (a
// null pointer) has the same effect as an array of alpha = 255 (fully
// opaque). The SetScrollPosition function enables a pattern or gradient
// to scroll in unison with a filled shape.
//
//-----------------------------------------------------------------------

class PaintGen
{
public:
    PaintGen()
    {
    }
    virtual ~PaintGen()
    {
    }
    virtual void FillSpan(int xs, int ys, int len, 
                          COLOR outBuf[], const COLOR inAlpha[] = 0) = 0;
    virtual bool SetScrollPosition(int x, int y) = 0;
};

// Paint generator for tiled pattern fills
//
class TiledPattern : public PaintGen
{
public:
    TiledPattern() 
    { 
    }
    TiledPattern(const COLOR *pattern, float u0, float v0, int w, int h, 
                 int stride, int flags, const float xform[]);
    TiledPattern(ImageReader *imgrdr, float u0, float v0, int w, int h, 
                 int flags, const float xform[]);
    virtual ~TiledPattern() 
    { 
    }
    virtual void FillSpan(int xs, int ys, int len, COLOR outBuf[], const COLOR inAlpha[]) = 0;
    virtual bool SetScrollPosition(int x, int y) = 0;
};

extern TiledPattern* CreateTiledPattern(const COLOR *pattern, float u0, float v0, 
                                        int w, int h, int stride, int flags, 
                                        const float xform[] = 0);

extern TiledPattern* CreateTiledPattern(ImageReader *imgrdr, float u0, float v0, 
                                        int w, int h, int flags, 
                                        const float xform[] = 0);

// Paint generator for linear gradient fills
//
class LinearGradient : public PaintGen
{
public:
    LinearGradient()
    {
    }
    LinearGradient(float x0, float y0, float x1, float y1,
                   SPREAD_METHOD spread, int flags, const float xform[] = 0);
    virtual ~LinearGradient()
    {
    }
    virtual void FillSpan(int xs, int ys, int len, COLOR outBuf[], const COLOR inAlpha[]) = 0;
    virtual bool SetScrollPosition(int x, int y) = 0;
    virtual bool AddColorStop(float offset, COLOR color) = 0;
};

extern LinearGradient* CreateLinearGradient(float x0, float y0, float x1, float y1,
                                            SPREAD_METHOD spread, int flags,
                                            const float xform[] = 0);

// Paint generator for radial gradient fills
//
class RadialGradient : public PaintGen
{
public:
    RadialGradient()
    {
    }
    RadialGradient(float x0, float y0, float r0, float x1, float y1, float r1,
                   SPREAD_METHOD spread, int flags, const float xform[] = 0);
    virtual ~RadialGradient()
    {
    }
    virtual void FillSpan(int xs, int ys, int len, COLOR outBuf[], const COLOR inAlpha[]) = 0;
    virtual bool SetScrollPosition(int x, int y) = 0;
    virtual bool AddColorStop(float offset, COLOR color) = 0;
};

extern RadialGradient* CreateRadialGradient(float x0, float y0, float r0,
                                            float x1, float y1, float r1,
                                            SPREAD_METHOD spread, int flags,
                                            const float xform[] = 0);

#endif // RENDERER_H