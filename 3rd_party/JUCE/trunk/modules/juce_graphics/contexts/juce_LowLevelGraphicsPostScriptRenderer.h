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

#ifndef JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_H_INCLUDED
#define JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_H_INCLUDED


//==============================================================================
/**
    An implementation of LowLevelGraphicsContext that turns the drawing operations
    into a PostScript document.

*/
class JUCE_API  LowLevelGraphicsPostScriptRenderer    : public LowLevelGraphicsContext
{
public:
    //==============================================================================
    LowLevelGraphicsPostScriptRenderer (OutputStream& resultingPostScript,
                                        const String& documentTitle,
                                        int totalWidth,
                                        int totalHeight);

    ~LowLevelGraphicsPostScriptRenderer();

    //==============================================================================
    bool isVectorDevice() const override;
    void setOrigin (int x, int y) LE_PATCH( noexcept ) override;
    void addTransform (const AffineTransform&) LE_PATCH( noexcept ) override;
    float getPhysicalPixelScaleFactor() LE_PATCH( noexcept ) override;

    bool clipToRectangle (const Rectangle<int>&) LE_PATCH( noexcept ) override;
    bool clipToRectangleList (const RectangleList<int>&) override;
    void excludeClipRectangle (const Rectangle<int>&) LE_PATCH( noexcept ) override;
    void clipToPath (const Path&, const AffineTransform&) override;
    void clipToImageAlpha (const Image&, const AffineTransform&) override;

    void saveState() override;
    void restoreState() LE_PATCH( noexcept ) override;

    void beginTransparencyLayer (float) override;
    void endTransparencyLayer() override;

    bool clipRegionIntersects (const Rectangle<int>&) LE_PATCH( noexcept ) override;
    Rectangle<int> getClipBounds() const LE_PATCH( noexcept ) override;
    bool isClipEmpty() const LE_PATCH( noexcept ) override;

    //==============================================================================
    void setFill (const FillType&) LE_PATCH( noexcept ) override;
    void setOpacity (float) LE_PATCH( noexcept ) override;
    void setInterpolationQuality (Graphics::ResamplingQuality) LE_PATCH( noexcept ) override;

    //==============================================================================
    void fillRect (const Rectangle<int>&, bool replaceExistingContents) override;
    void fillRectList (const RectangleList<float>&) override;
    void fillPath (const Path&, const AffineTransform&) override;
    void drawImage (const Image&, const AffineTransform&) override;
    void drawLine (const Line <float>&) override;
    void drawVerticalLine (int x, float top, float bottom) override;
    void drawHorizontalLine (int x, float top, float bottom) override;

    //==============================================================================
    const Font& getFont() LE_PATCH( noexcept ) override;
    void setFont (const Font&) LE_PATCH( noexcept ) override;
    void drawGlyph (int glyphNumber, const AffineTransform&) override;

protected:
    //==============================================================================
    OutputStream& out;
    int totalWidth, totalHeight;
    bool needToClip;
    Colour lastColour;

    struct SavedState
    {
        SavedState();
        ~SavedState();

        RectangleList<int> clip;
        int xOffset, yOffset;
        FillType fillType;
        Font font;

    private:
        SavedState& operator= (const SavedState&);
    };

    OwnedArray <SavedState> stateStack;

    void writeClip();
    void writeColour (const Colour& colour);
    void writePath (const Path& path) const;
    void writeXY (float x, float y) const;
    void writeTransform (const AffineTransform& trans) const;
    void writeImage (const Image& im, int sx, int sy, int maxW, int maxH) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowLevelGraphicsPostScriptRenderer)
};



#endif   // JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_H_INCLUDED
