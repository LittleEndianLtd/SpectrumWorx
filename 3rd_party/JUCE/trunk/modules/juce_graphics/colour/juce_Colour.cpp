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

namespace ColourHelpers
{
    static uint8 floatToUInt8 (const float n) noexcept
    {
    #ifdef LE_PATCHED_JUCE
        jassert( n >= 0.0f && n <= 1.0f );
        return (uint8) roundToInt( n * 255 );
    #else
        return n <= 0.0f ? 0 : (n >= 1.0f ? 255 : (uint8) (n * 255.0f));
    #endif // LE_PATCHED_JUCE
    }

    // This is an adjusted brightness value, based on the way the human
    // eye responds to different colour channels..
    static float getPerceivedBrightness (Colour c) noexcept
    {
        const float r = c.getFloatRed();
        const float g = c.getFloatGreen();
        const float b = c.getFloatBlue();

        return std::sqrt (r * r * 0.241f
                           + g * g * 0.691f
                           + b * b * 0.068f);
    }

    //==============================================================================
    struct HSB
    {
        HSB (Colour col) noexcept
        {
            const int r = col.getRed();
            const int g = col.getGreen();
            const int b = col.getBlue();

            const int hi = jmax (r, g, b);
            const int lo = jmin (r, g, b);

            if (hi != 0)
            {
                saturation = (hi - lo) / (float) hi;

                if (saturation > 0)
                {
                    const float invDiff = 1.0f / (hi - lo);

                    const float red   = (hi - r) * invDiff;
                    const float green = (hi - g) * invDiff;
                    const float blue  = (hi - b) * invDiff;

                    if (r == hi)
                        hue = blue - green;
                    else if (g == hi)
                        hue = 2.0f + red - blue;
                    else
                        hue = 4.0f + green - red;

                    hue *= 1.0f / 6.0f;

                    if (hue < 0)
                        ++hue;
                }
                else
                {
                    hue = 0;
                }
            }
            else
            {
                saturation = hue = 0;
            }

            brightness = hi / 255.0f;
        }

        Colour toColour (Colour original) const noexcept
        {
            return Colour (hue, saturation, brightness, original.getAlpha());
        }

        static PixelARGB toRGB (float h, float s, float v, const uint8 alpha) noexcept
        {
            v = jlimit (0.0f, 255.0f, v * 255.0f);
            const uint8 intV = (uint8) roundToInt (v);

            if (s <= 0)
                return PixelARGB (alpha, intV, intV, intV);

            s = jmin (1.0f, s);
            h = (h - std::floor (h)) * 6.0f + 0.00001f; // need a small adjustment to compensate for rounding errors
            const float f = h - std::floor (h);
            const uint8 x = (uint8) roundToInt (v * (1.0f - s));

            if (h < 1.0f)   return PixelARGB (alpha, intV,    (uint8) roundToInt (v * (1.0f - (s * (1.0f - f)))), x);
            if (h < 2.0f)   return PixelARGB (alpha,          (uint8) roundToInt (v * (1.0f - s * f)), intV, x);
            if (h < 3.0f)   return PixelARGB (alpha, x, intV, (uint8) roundToInt (v * (1.0f - (s * (1.0f - f)))));
            if (h < 4.0f)   return PixelARGB (alpha, x,       (uint8) roundToInt (v * (1.0f - s * f)), intV);
            if (h < 5.0f)   return PixelARGB (alpha,          (uint8) roundToInt (v * (1.0f - (s * (1.0f - f)))), x, intV);
                            return PixelARGB (alpha, intV, x, (uint8) roundToInt (v * (1.0f - s * f)));
        }

        float hue, saturation, brightness;
    };

    //==============================================================================
    struct YIQ
    {
        YIQ (Colour c) noexcept
        {
            const float r = c.getFloatRed();
            const float g = c.getFloatGreen();
            const float b = c.getFloatBlue();

            y = 0.2999f * r + 0.5870f * g + 0.1140f * b;
            i = 0.5957f * r - 0.2744f * g - 0.3212f * b;
            q = 0.2114f * r - 0.5225f * g - 0.3113f * b;
            alpha = c.getFloatAlpha();
        }

        Colour toColour() const noexcept
        {
            return Colour::fromFloatRGBA (y + 0.9563f * i + 0.6210f * q,
                                          y - 0.2721f * i - 0.6474f * q,
                                          y - 1.1070f * i + 1.7046f * q,
                                          alpha);
        }

        float y, i, q, alpha;
    };
}

//==============================================================================
Colour::Colour() noexcept
    : argb (0)
{
}

Colour::Colour (const Colour& other) noexcept
    : argb (other.argb)
{
}

Colour& Colour::operator= (const Colour& other) noexcept
{
    argb = other.argb;
    return *this;
}

bool Colour::operator== (const Colour& other) const noexcept    { return argb.getARGB() == other.argb.getARGB(); }
bool Colour::operator!= (const Colour& other) const noexcept    { return argb.getARGB() != other.argb.getARGB(); }

//==============================================================================
Colour::Colour (const uint32 col) noexcept  : argb (col)
{
}

Colour::Colour (const uint8 red, const uint8 green, const uint8 blue) noexcept
{
    argb.setARGB (0xff, red, green, blue);
}

Colour Colour::fromRGB (const uint8 red, const uint8 green, const uint8 blue) noexcept
{
    return Colour (red, green, blue);
}

Colour::Colour (const uint8 red, const uint8 green, const uint8 blue, const uint8 alpha) noexcept
{
    argb.setARGB (alpha, red, green, blue);
}

Colour Colour::fromRGBA (const uint8 red, const uint8 green, const uint8 blue, const uint8 alpha) noexcept
{
    return Colour (red, green, blue, alpha);
}

Colour::Colour (const uint8 red, const uint8 green, const uint8 blue, const float alpha) noexcept
{
    argb.setARGB (ColourHelpers::floatToUInt8 (alpha), red, green, blue);
}

Colour Colour::fromFloatRGBA (const float red, const float green, const float blue, const float alpha) noexcept
{
    return Colour (ColourHelpers::floatToUInt8 (red),
                   ColourHelpers::floatToUInt8 (green),
                   ColourHelpers::floatToUInt8 (blue), alpha);
}

Colour::Colour (const float hue, const float saturation, const float brightness, const float alpha) noexcept
    : argb (ColourHelpers::HSB::toRGB (hue, saturation, brightness, ColourHelpers::floatToUInt8 (alpha)))
{
}

Colour Colour::fromHSV (const float hue, const float saturation, const float brightness, const float alpha) noexcept
{
    return Colour (hue, saturation, brightness, alpha);
}

Colour::Colour (const float hue, const float saturation, const float brightness, const uint8 alpha) noexcept
    : argb (ColourHelpers::HSB::toRGB (hue, saturation, brightness, alpha))
{
}

JUCE_ORIGINAL( Colour::~Colour() noexcept { } )

//==============================================================================
const PixelARGB Colour::getPixelARGB() const noexcept
{
    PixelARGB p (argb);
    p.premultiply();
    return p;
}

uint32 Colour::getARGB() const noexcept
{
    return argb.getARGB();
}

//==============================================================================
bool Colour::isTransparent() const noexcept
{
    return getAlpha() == 0;
}

bool Colour::isOpaque() const noexcept
{
    return getAlpha() == 0xff;
}

Colour Colour::withAlpha (const uint8 newAlpha) const noexcept
{
    PixelARGB newCol (argb);
    newCol.setAlpha (newAlpha);
    return Colour (newCol.getARGB());
}

Colour Colour::withAlpha (const float newAlpha) const noexcept
{
    jassert (newAlpha >= 0 && newAlpha <= 1.0f);

    PixelARGB newCol (argb);
    newCol.setAlpha (ColourHelpers::floatToUInt8 (newAlpha));
    return Colour (newCol.getARGB());
}

Colour Colour::withMultipliedAlpha (const float alphaMultiplier) const noexcept
{
    jassert (alphaMultiplier >= 0);

    PixelARGB newCol (argb);
    newCol.setAlpha ((uint8) jmin (0xff, roundToInt (alphaMultiplier * newCol.getAlpha())));
    return Colour (newCol.getARGB());
}

//==============================================================================
Colour Colour::overlaidWith (Colour src) const noexcept
{
    const int destAlpha = getAlpha();

    if (destAlpha <= 0)
        return src;

    const int invA = 0xff - (int) src.getAlpha();
    const int resA = 0xff - (((0xff - destAlpha) * invA) >> 8);

    if (resA <= 0)
        return *this;

    const int da = (invA * destAlpha) / resA;

    return Colour ((uint8) (src.getRed()   + ((((int) getRed()   - src.getRed())   * da) >> 8)),
                   (uint8) (src.getGreen() + ((((int) getGreen() - src.getGreen()) * da) >> 8)),
                   (uint8) (src.getBlue()  + ((((int) getBlue()  - src.getBlue())  * da) >> 8)),
                   (uint8) resA);
}

Colour Colour::interpolatedWith (Colour other, float proportionOfOther) const noexcept
{
    if (proportionOfOther <= 0)
        return *this;

    if (proportionOfOther >= 1.0f)
        return other;

    PixelARGB c1 (getPixelARGB());
    const PixelARGB c2 (other.getPixelARGB());
    c1.tween (c2, (uint32) roundToInt (proportionOfOther * 255.0f));
    c1.unpremultiply();

    return Colour (c1.getARGB());
}

//==============================================================================
float Colour::getFloatRed() const noexcept      { return getRed()   / 255.0f; }
float Colour::getFloatGreen() const noexcept    { return getGreen() / 255.0f; }
float Colour::getFloatBlue() const noexcept     { return getBlue()  / 255.0f; }
float Colour::getFloatAlpha() const noexcept    { return getAlpha() / 255.0f; }

//==============================================================================
void Colour::getHSB (float& h, float& s, float& v) const noexcept
{
    const ColourHelpers::HSB hsb (*this);
    h = hsb.hue;
    s = hsb.saturation;
    v = hsb.brightness;
}

float Colour::getHue() const noexcept           { return ColourHelpers::HSB (*this).hue; }
float Colour::getSaturation() const noexcept    { return ColourHelpers::HSB (*this).saturation; }
float Colour::getBrightness() const noexcept    { return ColourHelpers::HSB (*this).brightness; }

Colour Colour::withHue (float h) const noexcept          { ColourHelpers::HSB hsb (*this); hsb.hue = h;        return hsb.toColour (*this); }
Colour Colour::withSaturation (float s) const noexcept   { ColourHelpers::HSB hsb (*this); hsb.saturation = s; return hsb.toColour (*this); }
Colour Colour::withBrightness (float v) const noexcept   { ColourHelpers::HSB hsb (*this); hsb.brightness = v; return hsb.toColour (*this); }

//==============================================================================
Colour Colour::withRotatedHue (const float amountToRotate) const noexcept
{
    ColourHelpers::HSB hsb (*this);
    hsb.hue += amountToRotate;
    return hsb.toColour (*this);
}

Colour Colour::withMultipliedSaturation (const float amount) const noexcept
{
    ColourHelpers::HSB hsb (*this);
    hsb.saturation = jmin (1.0f, hsb.saturation * amount);
    return hsb.toColour (*this);
}

Colour Colour::withMultipliedBrightness (const float amount) const noexcept
{
    ColourHelpers::HSB hsb (*this);
    hsb.brightness = jmin (1.0f, hsb.brightness * amount);
    return hsb.toColour (*this);
}

//==============================================================================
Colour Colour::brighter (float amount) const noexcept
{
    amount = 1.0f / (1.0f + amount);

    return Colour ((uint8) (255 - (amount * (255 - getRed()))),
                   (uint8) (255 - (amount * (255 - getGreen()))),
                   (uint8) (255 - (amount * (255 - getBlue()))),
                   getAlpha());
}

Colour Colour::darker (float amount) const noexcept
{
    amount = 1.0f / (1.0f + amount);

    return Colour ((uint8) (amount * getRed()),
                   (uint8) (amount * getGreen()),
                   (uint8) (amount * getBlue()),
                   getAlpha());
}

//==============================================================================
Colour Colour::greyLevel (const float brightness) noexcept
{
    const uint8 level = ColourHelpers::floatToUInt8 (brightness);
    return Colour (level, level, level);
}

//==============================================================================
Colour Colour::contrasting (const float amount) const noexcept
{
   return overlaidWith ((ColourHelpers::getPerceivedBrightness (*this) >= 0.5f
                            ? Colours::black
                            : Colours::white).withAlpha (amount));
}

Colour Colour::contrasting (Colour target, float minContrast) const noexcept
{
    const ColourHelpers::YIQ bg (*this);
    ColourHelpers::YIQ fg (target);

    if (std::abs (bg.y - fg.y) >= minContrast)
        return target;

    const float y1 = jmax (0.0f, bg.y - minContrast);
    const float y2 = jmin (1.0f, bg.y + minContrast);
    fg.y = (std::abs (y1 - bg.y) > std::abs (y2 - bg.y)) ? y1 : y2;

    return fg.toColour();
}

Colour Colour::contrasting (Colour colour1,
                            Colour colour2) noexcept
{
    const float b1 = ColourHelpers::getPerceivedBrightness (colour1);
    const float b2 = ColourHelpers::getPerceivedBrightness (colour2);
    float best = 0.0f;
    float bestDist = 0.0f;

    for (float i = 0.0f; i < 1.0f; i += 0.02f)
    {
        const float d1 = std::abs (i - b1);
        const float d2 = std::abs (i - b2);
        const float dist = jmin (d1, d2, 1.0f - d1, 1.0f - d2);

        if (dist > bestDist)
        {
            best = i;
            bestDist = dist;
        }
    }

    return colour1.overlaidWith (colour2.withMultipliedAlpha (0.5f))
                  .withBrightness (best);
}

//==============================================================================
#ifndef LE_PATCHED_JUCE
String Colour::toString() const
{
    return String::toHexString ((int) argb.getARGB());
}

Colour Colour::fromString (const String& encodedColourString)
{
    return Colour ((uint32) encodedColourString.getHexValue32());
}

String Colour::toDisplayString (const bool includeAlphaValue) const
{
    return String::toHexString ((int) (argb.getARGB() & (includeAlphaValue ? 0xffffffff : 0xffffff)))
                  .paddedLeft ('0', includeAlphaValue ? 8 : 6)
                  .toUpperCase();
}
#endif // LE_PATCHED_JUCE
