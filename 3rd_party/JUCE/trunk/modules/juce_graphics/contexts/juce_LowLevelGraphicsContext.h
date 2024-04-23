/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_LOWLEVELGRAPHICSCONTEXT_H_INCLUDED
#define JUCE_LOWLEVELGRAPHICSCONTEXT_H_INCLUDED


//==============================================================================
/**
    Interface class for graphics context objects, used internally by the Graphics class.

    Users are not supposed to create instances of this class directly - do your drawing
    via the Graphics object instead.

    It's a base class for different types of graphics context, that may perform software-based
    or OS-accelerated rendering.

    E.g. the LowLevelGraphicsSoftwareRenderer renders onto an image in memory, but other
    subclasses could render directly to a windows HDC, a Quartz context, or an OpenGL
    context.
*/
class JUCE_API LE_PATCH_NOVTABLE LowLevelGraphicsContext
{
protected:
    //==============================================================================
    LowLevelGraphicsContext() LE_PATCH( {} );

public:
    virtual ~LowLevelGraphicsContext() LE_PATCH( noexcept {} );

    /** Returns true if this device is vector-based, e.g. a printer. */
    JUCE_ORIGINAL( virtual ) bool isVectorDevice() const JUCE_ORIGINAL( = 0; ) LE_PATCH( { return false; } )

    //==============================================================================
    /** Moves the origin to a new position.

        The coordinates are relative to the current origin, and indicate the new position
        of (0, 0).
    */
    virtual void setOrigin (int x, int y) LE_PATCH( noexcept ) = 0;
    virtual void addTransform (const AffineTransform&) LE_PATCH( noexcept ) = 0;
    virtual float getPhysicalPixelScaleFactor() LE_PATCH( noexcept ) = 0;

    virtual bool clipToRectangle (const Rectangle<int>&) LE_PATCH( noexcept ) = 0;
#if defined( LE_PATCHED_JUCE ) && defined( _WIN32 )
    virtual bool clipToRectangleList (const RectangleList<int>&) LE_PATCH( noexcept ) = 0;
#else
    virtual bool clipToRectangleList (const RectangleList<int>&) = 0;
#endif // LE_PATCHED_JUCE
    virtual void excludeClipRectangle (const Rectangle<int>&) LE_PATCH( noexcept ) = 0;
    virtual void clipToPath (const Path&, const AffineTransform&) = 0;
    virtual void clipToImageAlpha (const Image&, const AffineTransform&) = 0;

    virtual bool clipRegionIntersects (const Rectangle<int>&) LE_PATCH( noexcept ) = 0;
    virtual Rectangle<int> getClipBounds() const LE_PATCH( noexcept ) = 0;
    virtual bool isClipEmpty() const LE_PATCH( noexcept ) = 0;

    virtual void saveState() = 0;
    virtual void restoreState() LE_PATCH( noexcept ) = 0;

    virtual void beginTransparencyLayer (float opacity) = 0;
    virtual void endTransparencyLayer() = 0;

    //==============================================================================
    virtual void setFill (const FillType&) LE_PATCH( noexcept ) = 0;
    virtual void setOpacity (float newOpacity) LE_PATCH( noexcept ) = 0;
    virtual void setInterpolationQuality (Graphics::ResamplingQuality) LE_PATCH( noexcept ) = 0;

    //==============================================================================
    virtual void fillRect (const Rectangle<int>&, bool replaceExistingContents) = 0;
    virtual void fillRectList (const RectangleList<float>&) = 0;

    virtual void fillPath (const Path&, const AffineTransform&) = 0;

    virtual void drawImage (const Image&, const AffineTransform&) = 0;

    virtual void drawLine (const Line <float>&) = 0;
    virtual void drawVerticalLine (int x, float top, float bottom) = 0;
    virtual void drawHorizontalLine (int y, float left, float right) = 0;

    virtual void setFont (const Font&) LE_PATCH( noexcept ) = 0;
    virtual const Font& getFont() LE_PATCH( noexcept ) = 0;
    virtual void drawGlyph (int glyphNumber, const AffineTransform&) = 0;
    virtual bool drawTextLayout (const AttributedString&, const Rectangle<float>&)  { return false; }
};


#endif   // JUCE_LOWLEVELGRAPHICSCONTEXT_H_INCLUDED
