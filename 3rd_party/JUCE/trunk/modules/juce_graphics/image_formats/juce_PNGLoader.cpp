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

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4390 4611 4365 4267)
 #ifdef __INTEL_COMPILER
  #pragma warning (disable: 2544 2545)
 #endif
#endif

namespace zlibNamespace
{
#if JUCE_INCLUDE_ZLIB_CODE
  #undef OS_CODE
  #undef fdopen
  #ifdef LE_PATCHED_JUCE
    #define UNALIGNED_OK
    #define NO_GZIP
    #define USE_COREGRAPHICS_RENDERING 1
  #endif // LE_PATCHED_JUCE
  #include "../../juce_core/zip/zlib/zlib.h"
  #undef OS_CODE
#else
  #include JUCE_ZLIB_INCLUDE_PATH
#endif
}

namespace pnglibNamespace
{
  using namespace zlibNamespace;

#if JUCE_INCLUDE_PNGLIB_CODE || ! defined (JUCE_INCLUDE_PNGLIB_CODE)

  #if _MSC_VER != 1310
   using std::calloc; // (causes conflict in VS.NET 2003)
   using std::malloc;
   using std::free;
  #endif

  #if JUCE_CLANG
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wsign-conversion"
  #endif

  using std::abs;
  #define NO_DUMMY_DECL
  #define PNGLCONF_H 1

 #if JUCE_ANDROID
  #define PNG_ARM_NEON_SUPPORTED
 #endif

#ifndef LE_PATCHED_JUCE
  #define PNG_16BIT_SUPPORTED
#endif // LE_PATCHED_JUCE
  #define PNG_ALIGNED_MEMORY_SUPPORTED
#ifndef LE_PATCHED_JUCE
  #define PNG_BENIGN_ERRORS_SUPPORTED
  #define PNG_BENIGN_READ_ERRORS_SUPPORTED
  #define PNG_BUILD_GRAYSCALE_PALETTE_SUPPORTED
  #define PNG_CHECK_FOR_INVALID_INDEX_SUPPORTED
  #define PNG_COLORSPACE_SUPPORTED
  #define PNG_CONSOLE_IO_SUPPORTED
  #define PNG_EASY_ACCESS_SUPPORTED
  #define PNG_FIXED_POINT_SUPPORTED
#endif // LE_PATCHED_JUCE
  #define PNG_FLOATING_ARITHMETIC_SUPPORTED
#ifndef LE_PATCHED_JUCE
  #define PNG_FLOATING_POINT_SUPPORTED
  #define PNG_FORMAT_AFIRST_SUPPORTED
  #define PNG_FORMAT_BGR_SUPPORTED
  #define PNG_GAMMA_SUPPORTED
  #define PNG_GET_PALETTE_MAX_SUPPORTED
  #define PNG_HANDLE_AS_UNKNOWN_SUPPORTED
  #define PNG_INCH_CONVERSIONS_SUPPORTED
  #define PNG_INFO_IMAGE_SUPPORTED
  #define PNG_IO_STATE_SUPPORTED
  #define PNG_MNG_FEATURES_SUPPORTED
#endif // LE_PATCHED_JUCE
  #define PNG_POINTER_INDEXING_SUPPORTED
#ifndef LE_PATCHED_JUCE
  #define PNG_PROGRESSIVE_READ_SUPPORTED
  #define PNG_READ_16BIT_SUPPORTED
  #define PNG_READ_ALPHA_MODE_SUPPORTED
  #define PNG_READ_ANCILLARY_CHUNKS_SUPPORTED
  #define PNG_READ_BACKGROUND_SUPPORTED
  #define PNG_READ_BGR_SUPPORTED
  #define PNG_READ_CHECK_FOR_INVALID_INDEX_SUPPORTED
  #define PNG_READ_COMPOSITE_NODIV_SUPPORTED
  #define PNG_READ_COMPRESSED_TEXT_SUPPORTED
  #define PNG_READ_EXPAND_16_SUPPORTED
  #define PNG_READ_EXPAND_SUPPORTED
#endif // LE_PATCHED_JUCE
  #define PNG_READ_FILLER_SUPPORTED
#ifndef LE_PATCHED_JUCE
  #define PNG_READ_GAMMA_SUPPORTED
  #define PNG_READ_GET_PALETTE_MAX_SUPPORTED
  #define PNG_READ_GRAY_TO_RGB_SUPPORTED
  #define PNG_READ_INTERLACING_SUPPORTED
  #define PNG_READ_INT_FUNCTIONS_SUPPORTED
  #define PNG_READ_INVERT_ALPHA_SUPPORTED
  #define PNG_READ_INVERT_SUPPORTED
  #define PNG_READ_OPT_PLTE_SUPPORTED
  #define PNG_READ_PACKSWAP_SUPPORTED
  #define PNG_READ_PACK_SUPPORTED
  #define PNG_READ_QUANTIZE_SUPPORTED
  #define PNG_READ_RGB_TO_GRAY_SUPPORTED
  #define PNG_READ_SCALE_16_TO_8_SUPPORTED
  #define PNG_READ_SHIFT_SUPPORTED
#endif // LE_PATCHED_JUCE
  #define PNG_READ_STRIP_16_TO_8_SUPPORTED
#ifndef LE_PATCHED_JUCE
  #define PNG_READ_STRIP_ALPHA_SUPPORTED
#endif // LE_PATCHED_JUCE
  #define PNG_READ_SUPPORTED
#ifndef LE_PATCHED_JUCE
  #define PNG_READ_SWAP_ALPHA_SUPPORTED
  #define PNG_READ_SWAP_SUPPORTED
  #define PNG_READ_TEXT_SUPPORTED
#endif // LE_PATCHED_JUCE
  #define PNG_READ_TRANSFORMS_SUPPORTED
#ifndef LE_PATCHED_JUCE
  #define PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_READ_USER_CHUNKS_SUPPORTED
  #define PNG_READ_USER_TRANSFORM_SUPPORTED
  #define PNG_READ_bKGD_SUPPORTED
  #define PNG_READ_cHRM_SUPPORTED
  #define PNG_READ_gAMA_SUPPORTED
  #define PNG_READ_hIST_SUPPORTED
  #define PNG_READ_iCCP_SUPPORTED
  #define PNG_READ_iTXt_SUPPORTED
  #define PNG_READ_oFFs_SUPPORTED
  #define PNG_READ_pCAL_SUPPORTED
  #define PNG_READ_pHYs_SUPPORTED
  #define PNG_READ_sBIT_SUPPORTED
  #define PNG_READ_sCAL_SUPPORTED
  #define PNG_READ_sPLT_SUPPORTED
  #define PNG_READ_sRGB_SUPPORTED
  #define PNG_READ_tEXt_SUPPORTED
  #define PNG_READ_tIME_SUPPORTED
#endif // LE_PATCHED_JUCE
  #define PNG_READ_tRNS_SUPPORTED
#ifndef LE_PATCHED_JUCE
  #define PNG_READ_zTXt_SUPPORTED
  #define PNG_SAVE_INT_32_SUPPORTED
  #define PNG_SAVE_UNKNOWN_CHUNKS_SUPPORTED
#endif // LE_PATCHED_JUCE
  #define PNG_SEQUENTIAL_READ_SUPPORTED
#ifndef LE_PATCHED_JUCE
  #define PNG_SET_CHUNK_CACHE_LIMIT_SUPPORTED
  #define PNG_SET_CHUNK_MALLOC_LIMIT_SUPPORTED
  #define PNG_SET_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_SET_USER_LIMITS_SUPPORTED
  #define PNG_SIMPLIFIED_READ_AFIRST_SUPPORTED
  #define PNG_SIMPLIFIED_READ_BGR_SUPPORTED
  #define PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED
  #define PNG_SIMPLIFIED_WRITE_BGR_SUPPORTED
  #define PNG_STDIO_SUPPORTED
  #define PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_TEXT_SUPPORTED
  #define PNG_TIME_RFC1123_SUPPORTED
  #define PNG_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_USER_CHUNKS_SUPPORTED
  #define PNG_USER_LIMITS_SUPPORTED
  #define PNG_USER_TRANSFORM_INFO_SUPPORTED
  #define PNG_USER_TRANSFORM_PTR_SUPPORTED
  #define PNG_WARNINGS_SUPPORTED
  #define PNG_WRITE_16BIT_SUPPORTED
  #define PNG_WRITE_ANCILLARY_CHUNKS_SUPPORTED
  #define PNG_WRITE_BGR_SUPPORTED
  #define PNG_WRITE_CHECK_FOR_INVALID_INDEX_SUPPORTED
  #define PNG_WRITE_COMPRESSED_TEXT_SUPPORTED
  #define PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
  #define PNG_WRITE_FILLER_SUPPORTED
  #define PNG_WRITE_FILTER_SUPPORTED
  #define PNG_WRITE_FLUSH_SUPPORTED
  #define PNG_WRITE_GET_PALETTE_MAX_SUPPORTED
  #define PNG_WRITE_INTERLACING_SUPPORTED
  #define PNG_WRITE_INT_FUNCTIONS_SUPPORTED
  #define PNG_WRITE_INVERT_ALPHA_SUPPORTED
  #define PNG_WRITE_INVERT_SUPPORTED
  #define PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
  #define PNG_WRITE_PACKSWAP_SUPPORTED
  #define PNG_WRITE_PACK_SUPPORTED
  #define PNG_WRITE_SHIFT_SUPPORTED
  #define PNG_WRITE_SUPPORTED
  #define PNG_WRITE_SWAP_ALPHA_SUPPORTED
  #define PNG_WRITE_SWAP_SUPPORTED
  #define PNG_WRITE_TEXT_SUPPORTED
  #define PNG_WRITE_TRANSFORMS_SUPPORTED
  #define PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED
  #define PNG_WRITE_USER_TRANSFORM_SUPPORTED
  #define PNG_WRITE_WEIGHTED_FILTER_SUPPORTED
  #define PNG_WRITE_bKGD_SUPPORTED
  #define PNG_WRITE_cHRM_SUPPORTED
  #define PNG_WRITE_gAMA_SUPPORTED
  #define PNG_WRITE_hIST_SUPPORTED
  #define PNG_WRITE_iCCP_SUPPORTED
  #define PNG_WRITE_iTXt_SUPPORTED
  #define PNG_WRITE_oFFs_SUPPORTED
  #define PNG_WRITE_pCAL_SUPPORTED
  #define PNG_WRITE_pHYs_SUPPORTED
  #define PNG_WRITE_sBIT_SUPPORTED
  #define PNG_WRITE_sCAL_SUPPORTED
  #define PNG_WRITE_sPLT_SUPPORTED
  #define PNG_WRITE_sRGB_SUPPORTED
  #define PNG_WRITE_tEXt_SUPPORTED
  #define PNG_WRITE_tIME_SUPPORTED
  #define PNG_WRITE_tRNS_SUPPORTED
  #define PNG_WRITE_zTXt_SUPPORTED
  #define PNG_bKGD_SUPPORTED
  #define PNG_cHRM_SUPPORTED
  #define PNG_gAMA_SUPPORTED
  #define PNG_hIST_SUPPORTED
  #define PNG_iCCP_SUPPORTED
  #define PNG_iTXt_SUPPORTED
  #define PNG_oFFs_SUPPORTED
  #define PNG_pCAL_SUPPORTED
  #define PNG_pHYs_SUPPORTED
  #define PNG_sBIT_SUPPORTED
  #define PNG_sCAL_SUPPORTED
  #define PNG_sPLT_SUPPORTED
  #define PNG_sRGB_SUPPORTED
  #define PNG_tEXt_SUPPORTED
  #define PNG_tIME_SUPPORTED
#endif // LE_PATCHED_JUCE
  #define PNG_tRNS_SUPPORTED
#ifndef LE_PATCHED_JUCE
  #define PNG_zTXt_SUPPORTED
#endif // LE_PATCHED_JUCE

#ifdef LE_PATCHED_JUCE
    #define PNG_DEBUG 0
    #define PNG_NO_WARNINGS 1
    #define PNG_NO_ERROR_TEXT 1
    #define PNG_NO_ERROR_NUMBERS 1
    #define PNG_NO_USER_MEM 1
    #define PNG_NO_READ_iCCP 1
    #define PNG_NO_READ_UNKNOWN_CHUNKS 1
    #define PNG_NO_READ_USER_CHUNKS 1
    #define PNG_NO_READ_iTXt 1
    #define PNG_NO_READ_sCAL 1
    #define PNG_NO_READ_sPLT 1

    #define PNG_NO_CONSOLE_IO 1
    #define PNG_NO_WARNINGS 1
    #define PNG_NO_STDIO 1

    #define PNG_NO_CHECK_cHRM
    #define PNG_NO_MNG_FEATURES

    //#define PNG_NO_READ_TRANSFORMS
    #define PNG_NO_PROGRESSIVE_READ

    #define PNG_NO_READ_EXPAND
    #define PNG_NO_READ_SHIFT
    #define PNG_NO_READ_PACK
    #define PNG_NO_READ_BGR
    #define PNG_NO_READ_SWAP
    #define PNG_NO_READ_PACKSWAP
    #define PNG_NO_READ_INVERT
    #define PNG_NO_READ_DITHER
    #define PNG_NO_READ_BACKGROUND
    #define PNG_NO_READ_16_TO_8
    #define PNG_NO_READ_GRAY_TO_RGB
    #define PNG_NO_READ_SWAP_ALPHA
    #define PNG_NO_READ_INVERT_ALPHA
    #define PNG_NO_READ_STRIP_ALPHA
    #define PNG_NO_READ_USER_TRANSFORM
    #define PNG_NO_READ_RGB_TO_GRAY
    #define PNG_NO_READ_GAMMA
    #define PNG_NO_READ_BACKGROUND
    #define PNG_NO_READ_DITHER
    #define PNG_NO_READ_INVERT
    #define PNG_NO_READ_SHIFT
    #define PNG_NO_READ_PACK
    #define PNG_NO_READ_PACKSWAP
    //#define PNG_NO_READ_FILLER
    #define PNG_NO_READ_SWAP
    #define PNG_NO_READ_SWAP_ALPHA
    #define PNG_NO_READ_INVERT_ALPHA
    #define PNG_NO_READ_RGB_TO_GRAY
    #define PNG_NO_READ_USER_TRANSFORM
    #define PNG_NO_READ_bKGD
    #define PNG_NO_READ_cHRM
    #define PNG_NO_READ_gAMA
    #define PNG_NO_READ_hIST
    #define PNG_NO_READ_pCAL
    #define PNG_NO_READ_pHYs
    #define PNG_NO_READ_sBIT
    #define PNG_NO_READ_sRGB
    #define PNG_NO_READ_TEXT
    #define PNG_NO_READ_tIME
    #define PNG_NO_READ_EMPTY_PLTE
    #define PNG_NO_READ_OPT_PLTE
    #define PNG_NO_READ_STRIP_ALPHA
    #define PNG_NO_READ_oFFs

    #ifndef PNG_NO_WRITE_SUPPORTED
        #define PNG_NO_WRITE_SUPPORTED
    #endif // PNG_NO_WRITE_SUPPORTED

    #define PNG_NO_INFO_IMAGE
    #define PNG_NO_FIXED_POINT_SUPPORTED
    #define PNG_NO_MNG_FEATURES
    #define PNG_NO_USER_TRANSFORM_PTR
    #define PNG_NO_HANDLE_AS_UNKNOWN
    #define PNG_NO_ZALLOC_ZERO
    #define PNG_NO_EASY_ACCESS
    #define PNG_NO_PROGRESSIVE_READ
    #define PNG_NO_USER_LIMITS
    #define PNG_NO_SET_USER_LIMITS
#endif // LE_PATCHED_JUCE

  #define PNG_STRING_COPYRIGHT "";
  #define PNG_STRING_NEWLINE "\n"
  #define PNG_LITERAL_SHARP 0x23
  #define PNG_LITERAL_LEFT_SQUARE_BRACKET 0x5b
  #define PNG_LITERAL_RIGHT_SQUARE_BRACKET 0x5d

  #define PNG_API_RULE 0
  #define PNG_CALLOC_SUPPORTED
  #define PNG_COST_SHIFT 3
  #define PNG_DEFAULT_READ_MACROS 1
  #define PNG_GAMMA_THRESHOLD_FIXED 5000
  #define PNG_IDAT_READ_SIZE PNG_ZBUF_SIZE
  #define PNG_INFLATE_BUF_SIZE 1024
  #define PNG_MAX_GAMMA_8 11
  #define PNG_QUANTIZE_BLUE_BITS 5
  #define PNG_QUANTIZE_GREEN_BITS 5
  #define PNG_QUANTIZE_RED_BITS 5
  #define PNG_TEXT_Z_DEFAULT_COMPRESSION (-1)
  #define PNG_TEXT_Z_DEFAULT_STRATEGY 0
  #define PNG_WEIGHT_SHIFT 8
  #define PNG_ZBUF_SIZE 8192
  #define PNG_Z_DEFAULT_COMPRESSION (-1)
  #define PNG_Z_DEFAULT_NOFILTER_STRATEGY 0
  #define PNG_Z_DEFAULT_STRATEGY 1
  #define PNG_sCAL_PRECISION 5
  #define PNG_sRGB_PROFILE_CHECKS 2

  #define png_debug(a, b)
  #define png_debug1(a, b, c)
  #define png_debug2(a, b, c, d)

  #include "pnglib/png.h"
  #include "pnglib/pngconf.h"

  #define PNG_NO_EXTERN
  #include "pnglib/png.c"
  #include "pnglib/pngerror.c"
  #include "pnglib/pngget.c"
  #include "pnglib/pngmem.c"
  #include "pnglib/pngread.c"
  #include "pnglib/pngpread.c"
  #include "pnglib/pngrio.c"
  #include "pnglib/pngrtran.c"
  #include "pnglib/pngrutil.c"
  #include "pnglib/pngset.c"
  #include "pnglib/pngtrans.c"
  #include "pnglib/pngwio.c"
  #include "pnglib/pngwrite.c"
  #include "pnglib/pngwtran.c"
  #include "pnglib/pngwutil.c"

  #if JUCE_CLANG
   #pragma clang diagnostic pop
  #endif
#else
  extern "C"
  {
#if defined( LE_PATCHED_JUCE ) && defined( PNG_NO_WRITE_SUPPORTED ) && defined( __APPLE__ ) && JUCE_USING_COREIMAGE_LOADER
    #define PNGLCONF_H 1
    #include "pnglib/png.h"
    #include "pnglib/pngconf.h"
#else
    #include <png.h>
    #include <pngconf.h>
#endif // LE_PATCHED_JUCE
  }
#endif
}

#undef max
#undef min
#undef fdopen

#if JUCE_MSVC
 #pragma warning (pop)
#endif

//==============================================================================
namespace PNGHelpers
{
    using namespace pnglibNamespace;

    static void JUCE_CDECL writeDataCallback (png_structp png, png_bytep data, png_size_t length)
    {
        static_cast<OutputStream*> (png_get_io_ptr (png))->write (data, length);
    }

   #if ! JUCE_USING_COREIMAGE_LOADER
    static void JUCE_CDECL readCallback (png_structp png, png_bytep data, png_size_t length)
    {
        static_cast<InputStream*> (png_get_io_ptr (png))->read (data, (int) length);
    }

    struct PNGErrorStruct {};

    static void JUCE_CDECL errorCallback (png_structp, png_const_charp)
    {
        throw PNGErrorStruct();
    }

    static void JUCE_CDECL warningCallback (png_structp, png_const_charp) {}
   #endif
}

//==============================================================================
PNGImageFormat::PNGImageFormat() LE_PATCH( noexcept )    {}
PNGImageFormat::~PNGImageFormat() LE_PATCH( noexcept )  {}

String PNGImageFormat::getFormatName()                   { return "PNG"; }
bool PNGImageFormat::usesFileExtension (const File& f)   { return f.hasFileExtension ("png"); }

bool PNGImageFormat::canUnderstand (InputStream& in)
{
    const int bytesNeeded = 4;
    char header [bytesNeeded];

    return in.read (header, bytesNeeded) == bytesNeeded
            && header[1] == 'P'
            && header[2] == 'N'
            && header[3] == 'G';
}

#if JUCE_USING_COREIMAGE_LOADER
 Image juce_loadWithCoreImage (InputStream& input);
#endif

Image PNGImageFormat::decodeImage (InputStream& in)
{
#if JUCE_USING_COREIMAGE_LOADER
    return juce_loadWithCoreImage (in);
#else
    using namespace pnglibNamespace;
    JUCE_ORIGINAL( Image image; )

    if (png_structp pngReadStruct = png_create_read_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0))
    {
        try
        {
            LE_PATCH_ASSUME( pngReadStruct != 0 );
            png_infop pngInfoStruct = png_create_info_struct (pngReadStruct);

            if (pngInfoStruct == nullptr)
            {
                png_destroy_read_struct (&pngReadStruct, 0, 0);
                return Image::null;
            }
            LE_PATCH_ASSUME( pngInfoStruct != 0 );

            png_set_error_fn (pngReadStruct, 0, PNGHelpers::errorCallback, PNGHelpers::warningCallback);

            // read the header..
            png_set_read_fn (pngReadStruct, &in, PNGHelpers::readCallback);

            png_uint_32 width = 0, height = 0;
            int bitDepth = 0, colorType = 0, interlaceType;

            png_read_info (pngReadStruct, pngInfoStruct);

            png_get_IHDR (pngReadStruct, pngInfoStruct,
                          &width, &height,
                          &bitDepth, &colorType,
                          &interlaceType, 0, 0);

        #ifdef LE_PATCHED_JUCE
            jassert(!(bitDepth == 16));
            jassert(!(colorType == PNG_COLOR_TYPE_PALETTE));
            jassert(!(bitDepth < 8));
            jassert(!png_get_valid (pngReadStruct, pngInfoStruct, PNG_INFO_tRNS));
        #else
            if (bitDepth == 16)
                png_set_strip_16 (pngReadStruct);

            if (colorType == PNG_COLOR_TYPE_PALETTE)
                png_set_expand (pngReadStruct);

            if (bitDepth < 8)
                png_set_expand (pngReadStruct);

            if (png_get_valid (pngReadStruct, pngInfoStruct, PNG_INFO_tRNS))
                png_set_expand (pngReadStruct);

            if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
                png_set_gray_to_rgb (pngReadStruct);
        #endif // LE_PATCHED_JUCE

            png_set_add_alpha (pngReadStruct, 0xff, PNG_FILLER_AFTER);

            bool hasAlphaChan = (colorType & PNG_COLOR_MASK_ALPHA) != 0
                                  || pngInfoStruct->num_trans > 0;

            // Load the image into a temp buffer in the pnglib format..
            const size_t lineStride = width * 4;
            HeapBlock <uint8> tempBuffer (height * lineStride);

            HeapBlock <png_bytep> rows (height);
            for (int y = (int) height; --y >= 0;)
                rows[y] = (png_bytep) (tempBuffer + lineStride * y);

            JUCE_ORIGINAL( try )
            {
                png_read_image (pngReadStruct, rows);
                JUCE_ORIGINAL( png_read_end (pngReadStruct, pngInfoStruct); )
            }
            JUCE_ORIGINAL( catch (PNGHelpers::PNGErrorStruct&) )
            {}

            png_destroy_read_struct (&pngReadStruct, &pngInfoStruct, 0);

            // now convert the data to a juce image format..
            LE_PATCH( Image ) image JUCE_ORIGINAL( = Image ) (hasAlphaChan ? Image::ARGB : Image::RGB,
                           (int) width, (int) height, hasAlphaChan);
        #ifdef LE_PATCHED_JUCE
            jassert( hasAlphaChan == image.hasAlphaChannel() );
        #else
            image.getProperties()->set ("originalImageHadAlpha", image.hasAlphaChannel());
            hasAlphaChan = image.hasAlphaChannel(); // (the native image creator may not give back what we expect)
        #endif

            const Image::BitmapData destData (image, Image::BitmapData::writeOnly);

            for (int y = 0; y < (int) height; ++y)
            {
                const uint8* src = rows[y];
                uint8* dest = destData.getLinePointer (y);

                if (hasAlphaChan)
                {
                    for (int i = (int) width; --i >= 0;)
                    {
                        ((PixelARGB*) dest)->setARGB (src[3], src[0], src[1], src[2]);
                        ((PixelARGB*) dest)->premultiply();
                        dest += destData.pixelStride;
                        src += 4;
                    }
                }
                else
                {
                    for (int i = (int) width; --i >= 0;)
                    {
                        ((PixelRGB*) dest)->setARGB (0, src[0], src[1], src[2]);
                        dest += destData.pixelStride;
                        src += 4;
                    }
                }
            }
            LE_PATCH( return image; )
        }
        catch (JUCE_ORIGINAL(PNGHelpers::PNGErrorStruct&) LE_PATCH(...))
        {}
    }
    LE_PATCH( return Image::null; )
    JUCE_ORIGINAL( return image; )
#endif
}

#if defined( PNG_NO_WRITE_SUPPORTED ) // LE_PATCHED_JUCE
bool PNGImageFormat::writeImageToStream (const Image&, OutputStream& ) { jassert( !"Should not get called." ); return false; }
#else
bool PNGImageFormat::writeImageToStream (const Image& image, OutputStream& out)
{
    using namespace pnglibNamespace;
    const int width = image.getWidth();
    const int height = image.getHeight();

    png_structp pngWriteStruct = png_create_write_struct (PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (pngWriteStruct == nullptr)
        return false;

    png_infop pngInfoStruct = png_create_info_struct (pngWriteStruct);

    if (pngInfoStruct == nullptr)
    {
        png_destroy_write_struct (&pngWriteStruct, (png_infopp) nullptr);
        return false;
    }

    png_set_write_fn (pngWriteStruct, &out, PNGHelpers::writeDataCallback, 0);

    png_set_IHDR (pngWriteStruct, pngInfoStruct, (png_uint_32) width, (png_uint_32) height, 8,
                  image.hasAlphaChannel() ? PNG_COLOR_TYPE_RGB_ALPHA
                                          : PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_BASE,
                  PNG_FILTER_TYPE_BASE);

    HeapBlock <uint8> rowData ((size_t) width * 4);

    png_color_8 sig_bit;
    sig_bit.red = 8;
    sig_bit.green = 8;
    sig_bit.blue = 8;
    sig_bit.alpha = 8;
    png_set_sBIT (pngWriteStruct, pngInfoStruct, &sig_bit);

    png_write_info (pngWriteStruct, pngInfoStruct);

    png_set_shift (pngWriteStruct, &sig_bit);
    png_set_packing (pngWriteStruct);

    const Image::BitmapData srcData (image, Image::BitmapData::readOnly);

    for (int y = 0; y < height; ++y)
    {
        uint8* dst = rowData;
        const uint8* src = srcData.getLinePointer (y);

        if (image.hasAlphaChannel())
        {
            for (int i = width; --i >= 0;)
            {
                PixelARGB p (*(const PixelARGB*) src);
                p.unpremultiply();

                *dst++ = p.getRed();
                *dst++ = p.getGreen();
                *dst++ = p.getBlue();
                *dst++ = p.getAlpha();
                src += srcData.pixelStride;
            }
        }
        else
        {
            for (int i = width; --i >= 0;)
            {
                *dst++ = ((const PixelRGB*) src)->getRed();
                *dst++ = ((const PixelRGB*) src)->getGreen();
                *dst++ = ((const PixelRGB*) src)->getBlue();
                src += srcData.pixelStride;
            }
        }

        png_bytep rowPtr = rowData;
        png_write_rows (pngWriteStruct, &rowPtr, 1);
    }

    png_write_end (pngWriteStruct, pngInfoStruct);
    png_destroy_write_struct (&pngWriteStruct, &pngInfoStruct);

    return true;
}
#endif // LE_PATCHED_JUCE
