////////////////////////////////////////////////////////////////////////////////
///
/// \file effectGroups.hpp
/// ----------------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef effectGroups_hpp__00998F05_C8C3_4C9E_A5B8_5005C5Ff61E5
#define effectGroups_hpp__00998F05_C8C3_4C9E_A5B8_5005C5Ff61E5
#pragma once
//------------------------------------------------------------------------------
#include <cstdint>
#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Definition of the module group hierarchy
// ----------------------------------------
//
//    Because of the large number of modules/effects supported and provided
// by SpectrumWorx we divide them in logical groups to make it easier for the
// user to navigate and choose among them.
//    To improve readability and reduce duplication macros are used to define
// the groups and their interrelationship.
//    Each group is represented as a struct/class with two member types (to
// enable easier metaprogramming), two (identical) member constants and an
// arbitrary number of nested classes (representing subgroups):
//    - "parent_group" typedef: 'points to' the (group's) parent group
//    - "type" typedef: 'contains' a group's own type
//    - "groupLevel" integral constant: indicates a group's place/depth/level
//      in the hierarchy (a lower level means a higher place in the hierarchy),
//      it could have probably been deduced from the parent-child relationship
//      with metafunctions but this was much more simpler while it introduced no
//      manual work as it is handled automatically by helper macros
//    - "value" integral constant: an alias for the "groupLevel" constant
//      (needed for easier metaprogramming), it is not simply declared and
//      defined directly in the group body (by the helper macro) but is rather
//      provided by deriving from std::integral_constant<> which also provides
//      the necessary tag(s) making our group classes/structs being recognized
//      as integral constants by MPL thus making it easier to
//      manipulate/sort/transform lists of groups.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// Private implementation details for this module.
// -----------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    ////////////////////////////////////////////////////////////////////////////
    //
    // MODULE_GROUPS_BEGIN
    // -------------------
    //
    ////////////////////////////////////////////////////////////////////////////

    #define MODULE_GROUPS_BEGIN()                                              \
    struct ModuleGroups : public std::integral_constant<std::uint8_t, 0>::type \
    {                                                                          \
        typedef void         parent_group;                                     \
        typedef ModuleGroups type;                                             \
                                                                               \
        static std::uint8_t const groupLevel = value;                          \


    ////////////////////////////////////////////////////////////////////////////
    //
    // MODULE_GROUPS_END
    // -----------------
    //
    ////////////////////////////////////////////////////////////////////////////

    #define MODULE_GROUPS_END() };


    ////////////////////////////////////////////////////////////////////////////
    //
    // SUB_GROUP_BEGIN
    // ---------------
    //
    ////////////////////////////////////////////////////////////////////////////
    //
    // Implementation note:
    //   Because std::integral_constant<> overrides the "type" member of the
    // parent group we must somehow 'save' the parent group information before
    // deriving from std::integral_constant<>. For this the ParentGroup
    // helper struct is used.
    //                                        (01.06.2009.) (Domagoj Saric)
    //
    ////////////////////////////////////////////////////////////////////////////

    // Helper for storing and fetching a group's parent group.
    template <class ParentGroupParam>
    struct ParentGroup { typedef ParentGroupParam parent_group; };

    #define SUB_GROUP_BEGIN( struct_name, user_struct_name )                \
    struct struct_name                                                      \
        : public Detail::ParentGroup<type>,                                 \
          public std::integral_constant<unsigned int, groupLevel + 1>::type \
        {                                                                   \
            typedef struct_name type;                                       \
                                                                            \
            static std::uint8_t const groupLevel    = value;                \
            static std::uint8_t const absoluteOrder = __LINE__;             \
                                                                            \
            static char const * name() { return user_struct_name; }


    ////////////////////////////////////////////////////////////////////////////
    //
    // SUB_GROUP_END
    // -------------
    //
    ////////////////////////////////////////////////////////////////////////////

    #define SUB_GROUP_END };

} // namespace Detail

MODULE_GROUPS_BEGIN()
/*
    SUB_GROUP_BEGIN( TwoInputs, "Two inputs" )
        SUB_GROUP_BEGIN( Morphers             , "Morphers"                         ) SUB_GROUP_END
        SUB_GROUP_BEGIN( Operators            , "Operators"                        ) SUB_GROUP_END
        SUB_GROUP_BEGIN( Vocoders             , "Vocoders and composition effects" ) SUB_GROUP_END
        SUB_GROUP_BEGIN( SpectralInterpolators, "Spectral interpolation effects"   ) SUB_GROUP_END
    SUB_GROUP_END

    SUB_GROUP_BEGIN( PhaseVocoders, "Phase vocoders" )
        SUB_GROUP_BEGIN( DomainTransforms, "Domain transforms" ) SUB_GROUP_END
        SUB_GROUP_BEGIN( PitchShifters   , "Pitch shifters"    ) SUB_GROUP_END
        SUB_GROUP_BEGIN( DomainEffects   , "Domain effects"    ) SUB_GROUP_END
    SUB_GROUP_END

    SUB_GROUP_BEGIN( Effects, "Effects" )
        SUB_GROUP_BEGIN( PitchShifters  , "Pitch shifters"    ) SUB_GROUP_END
        SUB_GROUP_BEGIN( FiltersAndGates, "Filters and gates" ) SUB_GROUP_END
        SUB_GROUP_BEGIN( Extractors     , "Extractors"        ) SUB_GROUP_END
    SUB_GROUP_END

    SUB_GROUP_BEGIN( Wavelets, "Wavelets"  )
    SUB_GROUP_END
*/
    SUB_GROUP_BEGIN( Pitch   , "Pitch"     )
    SUB_GROUP_END
    SUB_GROUP_BEGIN( PVD     , "PV domain" )
    SUB_GROUP_END
    SUB_GROUP_BEGIN( Timbre  , "Timbre"    )
    SUB_GROUP_END
    SUB_GROUP_BEGIN( Loudness, "Loudness"  )
    SUB_GROUP_END
    SUB_GROUP_BEGIN( Combine , "Combine"   )
    SUB_GROUP_END
    SUB_GROUP_BEGIN( Time    , "Time"      )
    SUB_GROUP_END
    SUB_GROUP_BEGIN( Space   , "Space"     )
    SUB_GROUP_END
    SUB_GROUP_BEGIN( Phase   , "Phase"     )
    SUB_GROUP_END
    SUB_GROUP_BEGIN( Misc    , "Misc"      )
    SUB_GROUP_END

MODULE_GROUPS_END()



////////////////////////////////////////////////////////////////////////////////
//
// Helper macros cleanup
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

#undef MODULE_GROUPS_BEGIN
#undef MODULE_GROUPS_END
#undef SUB_GROUP_BEGIN
#undef SUB_GROUP_END

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // effectGroups_hpp
