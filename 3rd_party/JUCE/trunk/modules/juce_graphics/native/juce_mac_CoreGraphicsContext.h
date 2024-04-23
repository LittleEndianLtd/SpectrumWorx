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

#ifndef JUCE_MAC_COREGRAPHICSCONTEXT_H_INCLUDED
#define JUCE_MAC_COREGRAPHICSCONTEXT_H_INCLUDED

//==============================================================================
class CoreGraphicsContext   : public LowLevelGraphicsContext
{
public:
    CoreGraphicsContext (CGContextRef context, const float flipHeight, const float targetScale);
    ~CoreGraphicsContext();

    //==============================================================================
    bool isVectorDevice() const JUCE_ORIGINAL( override )         { return false; }

    void setOrigin (int x, int y) LE_PATCH( noexcept ) override;
    void addTransform (const AffineTransform&) LE_PATCH( noexcept ) override;
    float getPhysicalPixelScaleFactor() LE_PATCH( noexcept ) override;
    bool clipToRectangle (const Rectangle<int>&) LE_PATCH( noexcept ) override;
    bool clipToRectangleList (const RectangleList<int>&) override;
    void excludeClipRectangle (const Rectangle<int>&) LE_PATCH( noexcept ) override;
    void clipToPath (const Path&, const AffineTransform&) override;
    void clipToImageAlpha (const Image&, const AffineTransform&) override;
    bool clipRegionIntersects (const Rectangle<int>&) LE_PATCH( noexcept ) override;
    Rectangle<int> getClipBounds() const LE_PATCH( noexcept ) override;
    bool isClipEmpty() const LE_PATCH( noexcept ) override;

    //==============================================================================
    void saveState() override;
    void restoreState() LE_PATCH( noexcept ) override;
    void beginTransparencyLayer (float opacity) override;
    void endTransparencyLayer() override;

    //==============================================================================
    void setFill (const FillType&) LE_PATCH( noexcept ) override;
    void setOpacity (float) LE_PATCH( noexcept ) override;
    void setInterpolationQuality (Graphics::ResamplingQuality) LE_PATCH( noexcept ) override;

    //==============================================================================
    void fillRect (const Rectangle<int>&, bool replaceExistingContents) override;
    void fillRectList (const RectangleList<float>&) override;
    void fillPath (const Path&, const AffineTransform&) override;
    void drawImage (const Image& sourceImage, const AffineTransform&) override;

    //==============================================================================
    void drawLine (const Line<float>&) override;
    void drawVerticalLine (const int x, float top, float bottom) override;
    void drawHorizontalLine (const int y, float left, float right) override;
    void setFont (const Font&) LE_PATCH( noexcept ) override;
    const Font& getFont() LE_PATCH( noexcept ) override;
    void drawGlyph (int glyphNumber, const AffineTransform&) override;
    bool drawTextLayout (const AttributedString&, const Rectangle<float>&) override;

private:
    CGContextRef context;
    const CGFloat flipHeight;
    float targetScale;
    CGColorSpaceRef rgbColourSpace, greyColourSpace;
    CGFunctionCallbacks gradientCallbacks;
    mutable Rectangle<int> lastClipRect;
    mutable bool lastClipRectIsValid;

    struct SavedState
    {
        SavedState();
        SavedState (const SavedState& other);
        ~SavedState();

        void setFill (const FillType& newFill);
        CGShadingRef getShading (CoreGraphicsContext& owner);

        static void gradientCallback (void* info, const CGFloat* inData, CGFloat* outData);

        FillType fillType;
        Font font;
        CGFontRef fontRef;
        CGAffineTransform fontTransform;

    private:
        CGShadingRef shading;
        HeapBlock <PixelARGB> gradientLookupTable;
        int numGradientLookupEntries;
    };

    ScopedPointer <SavedState> state;
    OwnedArray <SavedState> stateStack;

    void drawGradient();
    void createPath (const Path&) const;
    void createPath (const Path&, const AffineTransform&) const;
    void flip() const;
    void applyTransform (const AffineTransform&) const;
    void drawImage (const Image&, const AffineTransform&, bool fillEntireClipAsTiles);
    bool clipToRectangleListWithoutTest (const RectangleList<int>&);
    void fillCGRect (const CGRect&, bool replaceExistingContents);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsContext)
};

#endif   // JUCE_MAC_COREGRAPHICSCONTEXT_H_INCLUDED
