/*
  Copyright (C) 2019 Jerry R. VanAken

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
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
// 2-D Polygonal Shape Generator
//
//---------------------------------------------------------------------

#ifndef SHAPEGEN_H
#define SHAPEGEN_H

//----------------------------------------------------------------------
//
// Define types and constants
//
//----------------------------------------------------------------------

// 32-bit fixed-point value with 16-bit fraction 
typedef int FIX16;

// Coordinates, points, rectangles, and trapezoids in user coordinates
typedef int SGCoord;
struct SGPoint { 
    SGCoord x; 
    SGCoord y; 
};
struct SGRect { 
    SGCoord x; 
    SGCoord y; 
    SGCoord w; 
    SGCoord h; 
};
struct SGTpzd {  // renderer only
    short ytop; 
    short height; 
    FIX16 xL; 
    FIX16 dxL; 
    FIX16 xR; 
    FIX16 dxR; 
};

// Default line width attribute for stroked paths 
const float LINEWIDTH_DEFAULT = 4.0;

// Miter limit parameters 
const float MITERLIMIT_DEFAULT = 10.0;   // default miter limit
const float MITERLIMIT_MINIMUM = 1.0;    // minimum miter limit
const float MITERLIMIT_MAXIMUM = 100.0;  // minimum miter limit

// Flatness threshold -- required arc- or curve-to-chord tolerance 
const float FLATNESS_DEFAULT = 0.6;       // default flatness setting
const float FLATNESS_MINIMUM = 1.0/16.0;  // minimum flatness setting
const float FLATNESS_MAXIMUM = 16.0;      // maximum flatness setting

// Fixed-point fraction length -- bits to right of binary point 
const int FIXBITS_DEFAULT = 0;  // default = integer (no fixed point)

// Fill rule attribute values for filled paths 
enum FILLRULE {
    FILLRULE_EVENODD,    // even-odd (aka "parity") fill rule
    FILLRULE_WINDING,    // nonzero winding number fill rule
};

// Join attribute values for stroked paths 
enum LINEJOIN {
    LINEJOIN_BEVEL,   // beveled join between two line segments
    LINEJOIN_ROUND,  // rounded join between two line segments
    LINEJOIN_MITER   // mitered join between two line segments
};
const LINEJOIN LINEJOIN_DEFAULT = LINEJOIN_BEVEL;  // default line join

// Line end (cap) attribute values for stroked paths 
enum LINEEND {
    LINEEND_FLAT,   // flat line end (butt line cap)
    LINEEND_ROUND,  // rounded line end (round cap)
    LINEEND_SQUARE  // squared line end (projecting cap)
};
const LINEEND LINEEND_DEFAULT = LINEEND_FLAT;  // default line end

//----------------------------------------------------------------------
//
// Shape feeder: Breaks a shape into smaller pieces to feed to a
// renderer. The ShapeGen object loads a shape into a shape feeder and
// then hands off the feeder to the renderer. The renderer iteratively
// calls either function below to receive the shape in pieces (either
// as rectangles or trapezoids) that are ready to be drawn to the
// display as filled shapes.
//
//----------------------------------------------------------------------

class ShapeFeeder
{
public:
    virtual bool GetNextSDLRect(SGRect *rect) = 0;
    virtual bool GetNextGDIRect(SGRect *rect) = 0;
    virtual bool GetNextTrapezoid(SGTpzd *tpzd) = 0;
};

//----------------------------------------------------------------------
//
// Renderer: Handles requests from the ShapeGen object to draw filled
// shapes on the display
//
//----------------------------------------------------------------------

class Renderer
{
public:
    virtual void RenderShape(ShapeFeeder *feeder) = 0;
};

//----------------------------------------------------------------------
//
// 2-D Polygonal Shape Generator: Constructs paths that describe
// polygonal shapes, and then maps these shapes onto the pixel grid.
// The ShapeGen object relies on the Renderer object to write to the
// pixels in the display. Consequently, all device dependencies are
// consigned to the renderer. The ShapeGen code contains no device
// dependencies and is highly portable. 
//
//----------------------------------------------------------------------

class ShapeGen
{
public:
    ShapeGen()
    {
    }
    virtual ~ShapeGen()
    {
    }

    // Renderer object
    virtual void SetRenderer(Renderer *renderer) = 0;

    // Basic path construction
    virtual void BeginPath() = 0;
    virtual void CloseFigure() = 0;
    virtual void EndFigure() = 0;
    virtual void Move(SGCoord x, SGCoord y) = 0;
    virtual bool Line(SGCoord x, SGCoord y) = 0;
    virtual bool PolyLine(int npts, const SGPoint xy[]) = 0;
    virtual void Rectangle(const SGRect& rect) = 0;
    virtual bool FillPath(FILLRULE fillrule) = 0;
    virtual void SetScrollPosition(int x, int y) = 0;
    virtual bool GetCurrentPoint(SGPoint *cpoint) = 0;
    virtual bool GetFirstPoint(SGPoint *fpoint) = 0;
    virtual bool GetBoundingBox(SGRect *bbox) = 0;

    // Clipping and masking
    virtual void InitClipRegion(int width, int height) = 0;
    virtual void ResetClipRegion() = 0;
    virtual bool SetClipRegion(FILLRULE fillrule) = 0;
    virtual bool SetMaskRegion(FILLRULE fillrule) = 0;
    virtual bool SaveClipRegion() = 0;
    virtual bool SwapClipRegion() = 0;

    // Basic path attributes
    virtual float SetFlatness(float tol) = 0;
    virtual int SetFixedBits(int nbits) = 0;

    // Stroked path construction
    virtual bool StrokePath() = 0;
    
    // Stroked path attributes
    virtual float SetLineWidth(float width) = 0;
    virtual float SetMiterLimit(float mlim) = 0;
    virtual LINEEND SetLineEnd(LINEEND capstyle) = 0;
    virtual LINEJOIN SetLineJoin(LINEJOIN joinstyle) = 0;
    virtual void SetLineDash(char *dash, int offset, float mult) = 0;
    
    // Ellipses and elliptic arcs
    virtual void Ellipse(const SGPoint& v0, const SGPoint& v1, const SGPoint& v2) = 0;
    virtual void EllipticArc(const SGPoint& v0, const SGPoint& v1, const SGPoint& v2, float aStart, float aSweep) = 0;
    virtual bool EllipticSpline(const SGPoint& v1, const SGPoint& v2) = 0;
    virtual bool PolyEllipticSpline(int npts, const SGPoint xy[]) = 0;
    virtual void RoundedRectangle(const SGRect& rect, const SGPoint& round) = 0;
    
    // Quadratic and cubic Bezier splines
    virtual bool Bezier2(const SGPoint& v1, const SGPoint& v2) = 0;
    virtual bool PolyBezier2(int npts, const SGPoint xy[]) = 0;
    virtual bool Bezier3(const SGPoint& v1, const SGPoint& v2, const SGPoint& v3) = 0;
    virtual bool PolyBezier3(int npts, const SGPoint xy[]) = 0;
};

//---------------------------------------------------------------------
//
// ShapeGen smart pointer. Creates a ShapeGen object that is
// automatically deleted when the SGPtr object goes out of scope.
// Encapsulates the ShapeGen internal implementation details. The ->
// operator is overloaded to enable an SGPtr object to be used as a
// ShapeGen object pointer. The * operator is overloaded to enable a
// ShapeGen object pointer to be passed as a function parameter.
//
//--------------------------------------------------------------------- 
class SGPtr 
{ 
   ShapeGen *sg;

   ShapeGen* CreateShapeGen(Renderer *renderer, const SGRect& cliprect);

public: 
   SGPtr(Renderer *renderer, const SGRect& cliprect) 
   { 
       sg = CreateShapeGen(renderer, cliprect); 
   }
   ~SGPtr() 
   { 
       delete(sg); 
   }
   ShapeGen& operator*() 
   {  
       return *sg; 
   }
   ShapeGen* operator->() 
   { 
       return sg; 
   } 
};

#endif  // SHAPEGEN_H
