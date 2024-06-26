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

DrawableText::DrawableText()
    : colour (Colours::black),
      justification (Justification::centredLeft)
{
    setBoundingBox (RelativeParallelogram (RelativePoint (0.0f, 0.0f),
                                           RelativePoint (50.0f, 0.0f),
                                           RelativePoint (0.0f, 20.0f)));
    setFont (Font (15.0f), true);
}

DrawableText::DrawableText (const DrawableText& other)
    : Drawable (other),
      bounds (other.bounds),
      fontHeight (other.fontHeight),
      fontHScale (other.fontHScale),
      font (other.font),
      text (other.text),
      colour (other.colour),
      justification (other.justification)
{
}

DrawableText::~DrawableText()
{
}

//==============================================================================
void DrawableText::setText (const String& newText)
{
    if (text != newText)
    {
        text = newText;
        refreshBounds();
    }
}

void DrawableText::setColour (Colour newColour)
{
    if (colour != newColour)
    {
        colour = newColour;
        repaint();
    }
}

void DrawableText::setFont (const Font& newFont, bool applySizeAndScale)
{
    if (font != newFont)
    {
        font = newFont;

        if (applySizeAndScale)
        {
            fontHeight = font.getHeight();
            fontHScale = font.getHorizontalScale();
        }

        refreshBounds();
    }
}

void DrawableText::setJustification (Justification newJustification)
{
    justification = newJustification;
    repaint();
}

void DrawableText::setBoundingBox (const RelativeParallelogram& newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;
        refreshBounds();
    }
}

void DrawableText::setFontHeight (const RelativeCoordinate& newHeight)
{
    if (fontHeight != newHeight)
    {
        fontHeight = newHeight;
        refreshBounds();
    }
}

void DrawableText::setFontHorizontalScale (const RelativeCoordinate& newScale)
{
    if (fontHScale != newScale)
    {
        fontHScale = newScale;
        refreshBounds();
    }
}

void DrawableText::refreshBounds()
{
    if (bounds.isDynamic() || fontHeight.isDynamic() || fontHScale.isDynamic())
    {
        Drawable::Positioner<DrawableText>* const p = new Drawable::Positioner<DrawableText> (*this);
        setPositioner (p);
        p->apply();
    }
    else
    {
        setPositioner (0);
        recalculateCoordinates (0);
    }
}

bool DrawableText::registerCoordinates (RelativeCoordinatePositionerBase& pos)
{
    bool ok = pos.addPoint (bounds.topLeft);
    ok = pos.addPoint (bounds.topRight) && ok;
    ok = pos.addPoint (bounds.bottomLeft) && ok;
    ok = pos.addCoordinate (fontHeight) && ok;
    return pos.addCoordinate (fontHScale) && ok;
}

void DrawableText::recalculateCoordinates (Expression::Scope* scope)
{
    bounds.resolveThreePoints (resolvedPoints, scope);

    const float w = Line<float> (resolvedPoints[0], resolvedPoints[1]).getLength();
    const float h = Line<float> (resolvedPoints[0], resolvedPoints[2]).getLength();

    const float height = jlimit (0.01f, jmax (0.01f, h), (float) fontHeight.resolve (scope));
    const float hscale = jlimit (0.01f, jmax (0.01f, w), (float) fontHScale.resolve (scope));

    scaledFont = font;
    scaledFont.setHeight (height);
    scaledFont.setHorizontalScale (hscale);

    setBoundsToEnclose (getDrawableBounds());
    repaint();
}

//==============================================================================
void DrawableText::paint (Graphics& g)
{
    transformContextToCorrectOrigin (g);

    const float w = Line<float> (resolvedPoints[0], resolvedPoints[1]).getLength();
    const float h = Line<float> (resolvedPoints[0], resolvedPoints[2]).getLength();

    g.addTransform (AffineTransform::fromTargetPoints (0, 0, resolvedPoints[0].x, resolvedPoints[0].y,
                                                       w, 0, resolvedPoints[1].x, resolvedPoints[1].y,
                                                       0, h, resolvedPoints[2].x, resolvedPoints[2].y));
    g.setFont (scaledFont);
    g.setColour (colour);

    g.drawFittedText (text, Rectangle<float> (w, h).getSmallestIntegerContainer(), justification, 0x100000);
}

Rectangle<float> DrawableText::getDrawableBounds() const
{
    return RelativeParallelogram::getBoundingBox (resolvedPoints);
}

Drawable* DrawableText::createCopy() const
{
    return new DrawableText (*this);
}

//==============================================================================
LE_PATCH( LE_WEAK_SYMBOL ) const Identifier DrawableText::valueTreeType ("Text");

LE_PATCH( LE_WEAK_SYMBOL ) const Identifier DrawableText::ValueTreeWrapper::text ("text");
LE_PATCH( LE_WEAK_SYMBOL ) const Identifier DrawableText::ValueTreeWrapper::colour ("colour");
LE_PATCH( LE_WEAK_SYMBOL ) const Identifier DrawableText::ValueTreeWrapper::font ("font");
LE_PATCH( LE_WEAK_SYMBOL ) const Identifier DrawableText::ValueTreeWrapper::justification ("justification");
LE_PATCH( LE_WEAK_SYMBOL ) const Identifier DrawableText::ValueTreeWrapper::topLeft ("topLeft");
LE_PATCH( LE_WEAK_SYMBOL ) const Identifier DrawableText::ValueTreeWrapper::topRight ("topRight");
LE_PATCH( LE_WEAK_SYMBOL ) const Identifier DrawableText::ValueTreeWrapper::bottomLeft ("bottomLeft");
LE_PATCH( LE_WEAK_SYMBOL ) const Identifier DrawableText::ValueTreeWrapper::fontHeight ("fontHeight");
LE_PATCH( LE_WEAK_SYMBOL ) const Identifier DrawableText::ValueTreeWrapper::fontHScale ("fontHScale");

//==============================================================================
DrawableText::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : ValueTreeWrapperBase (state_)
{
    jassert (state.hasType (valueTreeType));
}

String DrawableText::ValueTreeWrapper::getText() const
{
    return state [text].toString();
}

void DrawableText::ValueTreeWrapper::setText (const String& newText, UndoManager* undoManager)
{
    state.setProperty (text, newText, undoManager);
}

Value DrawableText::ValueTreeWrapper::getTextValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (text, undoManager);
}

Colour DrawableText::ValueTreeWrapper::getColour() const
{
    return Colour::fromString (state [colour].toString());
}

void DrawableText::ValueTreeWrapper::setColour (Colour newColour, UndoManager* undoManager)
{
    state.setProperty (colour, newColour.toString(), undoManager);
}

Justification DrawableText::ValueTreeWrapper::getJustification() const
{
    return Justification ((int) state [justification]);
}

void DrawableText::ValueTreeWrapper::setJustification (Justification newJustification, UndoManager* undoManager)
{
    state.setProperty (justification, newJustification.getFlags(), undoManager);
}

Font DrawableText::ValueTreeWrapper::getFont() const
{
    return Font::fromString (state [font]);
}

void DrawableText::ValueTreeWrapper::setFont (const Font& newFont, UndoManager* undoManager)
{
    state.setProperty (font, newFont.toString(), undoManager);
}

Value DrawableText::ValueTreeWrapper::getFontValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (font, undoManager);
}

RelativeParallelogram DrawableText::ValueTreeWrapper::getBoundingBox() const
{
    return RelativeParallelogram (state [topLeft].toString(), state [topRight].toString(), state [bottomLeft].toString());
}

void DrawableText::ValueTreeWrapper::setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager)
{
    state.setProperty (topLeft, newBounds.topLeft.toString(), undoManager);
    state.setProperty (topRight, newBounds.topRight.toString(), undoManager);
    state.setProperty (bottomLeft, newBounds.bottomLeft.toString(), undoManager);
}

RelativeCoordinate DrawableText::ValueTreeWrapper::getFontHeight() const
{
    return state [fontHeight].toString();
}

void DrawableText::ValueTreeWrapper::setFontHeight (const RelativeCoordinate& coord, UndoManager* undoManager)
{
    state.setProperty (fontHeight, coord.toString(), undoManager);
}

RelativeCoordinate DrawableText::ValueTreeWrapper::getFontHorizontalScale() const
{
    return state [fontHScale].toString();
}

void DrawableText::ValueTreeWrapper::setFontHorizontalScale (const RelativeCoordinate& coord, UndoManager* undoManager)
{
    state.setProperty (fontHScale, coord.toString(), undoManager);
}

//==============================================================================
void DrawableText::refreshFromValueTree (const ValueTree& tree, ComponentBuilder&)
{
    ValueTreeWrapper v (tree);
    setComponentID (v.getID());

    const RelativeParallelogram newBounds (v.getBoundingBox());
    const RelativeCoordinate newFontHeight (v.getFontHeight());
    const RelativeCoordinate newFontHScale (v.getFontHorizontalScale());
    const Colour newColour (v.getColour());
    const Justification newJustification (v.getJustification());
    const String newText (v.getText());
    const Font newFont (v.getFont());

    if (text != newText || font != newFont || justification != newJustification
         || colour != newColour || bounds != newBounds
         || newFontHeight != fontHeight || newFontHScale != fontHScale)
    {
        setBoundingBox (newBounds);
        setFontHeight (newFontHeight);
        setFontHorizontalScale (newFontHScale);
        setColour (newColour);
        setFont (newFont, false);
        setJustification (newJustification);
        setText (newText);
    }
}

ValueTree DrawableText::createValueTree (ComponentBuilder::ImageProvider*) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getComponentID());
    v.setText (text, nullptr);
    v.setFont (font, nullptr);
    v.setJustification (justification, nullptr);
    v.setColour (colour, nullptr);
    v.setBoundingBox (bounds, nullptr);
    v.setFontHeight (fontHeight, nullptr);
    v.setFontHorizontalScale (fontHScale, nullptr);

    return tree;
}
