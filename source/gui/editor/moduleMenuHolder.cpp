////////////////////////////////////////////////////////////////////////////////
///
/// moduleMenuHolder.cpp
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "moduleMenuHolder.hpp"

#include "le/spectrumworx/effects/configuration/effectIndexToGroupMapping.hpp"
#include "le/spectrumworx/effects/configuration/effectNames.hpp"
#include "le/spectrumworx/effects/configuration/includedEffects.hpp"
#include "le/utility/staticForEach.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace GUI
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Private implementation details for this module.
// -----------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

namespace
{
    using Menu  = ModuleMenuHolder::Menu ;
    using Menus = ModuleMenuHolder::Menus;


    void addModuleToMenuEntry( Menu & menu, std::uint8_t const moduleIndex )
    {
        unsigned int const menuEntryID  (                           moduleIndex   );
        char const * const moduleName   ( Effects::effectName     ( moduleIndex ) );
        bool         const moduleEnabled( Effects::includedEffects[ moduleIndex ] );
    #ifdef LE_SW_FULL
        LE_ASSUME( moduleEnabled == true );
    #endif // LE_SW_FULL
        menu.addItem( menuEntryID, moduleName, juce::Image::null, moduleEnabled );
    }


    template <unsigned int moduleIndex, unsigned int subMenuIndex>
    void addModulesToMenu( Menus &      , boost::mpl::true_  /*end reached*/     ) {}

    template <unsigned int moduleIndex, unsigned int subMenuIndex>
    LE_FORCEINLINE
    void addModulesToMenu( Menus & menus, boost::mpl::false_ /*end not reached*/ )
    {
        using namespace boost;

        std::uint8_t const menuIndex( 1 + subMenuIndex );
        addModuleToMenuEntry
        (
            menus[ menuIndex ],
            moduleIndex
        );

        typedef mpl::bool_<( moduleIndex + 1 ) == Effects::Constants::numberOfEffects> LastModule;
        unsigned int const nextModuleIndex( moduleIndex + !LastModule::value );

        typedef typename Effects::Group<moduleIndex    >::type CurrentModuleGroup;
        typedef typename Effects::Group<nextModuleIndex>::type NextModuleGroup   ;
        unsigned int const nextSubMenuIndex( subMenuIndex + !is_same<CurrentModuleGroup, NextModuleGroup>::value );

        addModulesToMenu
        <
            nextModuleIndex,
            nextSubMenuIndex
        >( menus, LastModule() );
    }


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class TopMenusAdder
    ///
    /// \brief A helper functor for adding menus from a ModuleMenuHolder to a
    /// parent menu.
    ///
    ////////////////////////////////////////////////////////////////////////////

    class TopMenusAdder
    {
    public:
        TopMenusAdder( Menus & menus ) : pCurrentMenu_( &menus[ 1 ] ), parentMenu_( menus.front() ) {}

        template <class Group>
        void operator()() { parentMenu_.addSubMenu( *pCurrentMenu_++, Group::name() ); }

    private:
        void operator=( TopMenusAdder const & );

        Menu * pCurrentMenu_;
        Menu & parentMenu_  ;
    };


    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.

    struct FlatMenuAdder
    {
        typedef void result_type;
        template <typename ModuleMenuIndex>
        void operator()() const
        {
            addModuleToMenuEntry
            (
                menus[ 0 ],
                ModuleMenuIndex::value
            );
        }
        Menus & menus;
    };

    #pragma warning( pop )


    void fillMenu( Menus & menus, boost::mpl::true_ /*has sub menus*/ )
    {
        // Implementation note:
        //   Our own implementation of the boost::mpl::detail::execute() helper
        // template function that passes along all intermediate results for
        // vastly improved compilation times (on GCC atleast).
        //                                    (23.09.2010.) (Domagoj Saric)
        addModulesToMenu<0, 0>( menus, boost::mpl::false_() );

        TopMenusAdder topMenusAdder( menus );
        Utility::forEach<Effects::Groups>( topMenusAdder );
    }

    void fillMenu( Menus & menus, boost::mpl::false_ /*does not have sub menus*/ )
    {
        FlatMenuAdder const adder = { menus };
        Utility::forEach<Effects::ValidIndices>( adder );
    }
} // namespace anonymous


////////////////////////////////////////////////////////////////////////////////
//
// ModuleMenuHolder::ModuleMenuHolder()
// ------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \throws std::bad_alloc Out of memory.
///
////////////////////////////////////////////////////////////////////////////////

ModuleMenuHolder::ModuleMenuHolder()
{
    fillMenu( menus_, boost::mpl::bool_<hasSubMenus>() );
}


bool ModuleMenuHolder::isOwnerOfEntry( unsigned int const menuEntryID ) const
{
    unsigned int const totalNumberOfMenuEntries( Effects::Constants::numberOfEffects );
    return menuEntryID < totalNumberOfMenuEntries;
}


std::uint8_t ModuleMenuHolder::effectIndexForEntry( unsigned int const menuEntryID ) const
{
    BOOST_ASSERT( isOwnerOfEntry( menuEntryID ) );
    return static_cast<std::uint8_t>( menuEntryID );
}


Menu & ModuleMenuHolder::subMenu( std::uint8_t const index )
{
    return menus_[ 1 + index ];
}

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
