////////////////////////////////////////////////////////////////////////////////
///
/// \file moduleNode.hpp
/// --------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef moduleNode_hpp__926B45C3_7354_4519_AE90_42C1965E9F77
#define moduleNode_hpp__926B45C3_7354_4519_AE90_42C1965E9F77
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/parentFromMember.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/referenceCounter.hpp"

#if !defined( BOOST_NO_RTTI ) && ( !defined( BOOST_NO_EXCEPTIONS ) || defined( _MSC_VER ) )
    #include "boost/polymorphic_cast.hpp"
#endif // LE_SW_SDK_BUILD
#include "boost/smart_ptr/intrusive_ptr.hpp"

#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

class LE_NOVTABLE ModuleNode
{
public:
    using NodePtr  = boost::intrusive_ptr<ModuleNode      >;
    using NodeCPtr = boost::intrusive_ptr<ModuleNode const>;

    static_assert( sizeof( NodePtr ) == sizeof( void * ), "" );

    mutable Utility::ReferenceCount referenceCount_;

    mutable NodePtr next_    ;
    mutable NodePtr previous_;

protected:
    ModuleNode() = default;

#if LE_SW_SEPARATED_DSP_GUI
    /// \note Create a virtual destructor in order to be able to reuse the same
    /// (Automated)ModuleChain class (and release/deleter functions) for both
    /// GUI and DSP modules in separated-DSP-and-GUI builds.
    ///                                       (18.03.2015.) (Domagoj Saric)
public:
    LE_NOTHROW virtual ~ModuleNode() {}/* = 0*/;
#elif !defined( NDEBUG )
protected:
    ~ModuleNode() = default;
    virtual void rtti_enforcer() {}
#endif // NDEBUG

    ModuleNode( ModuleNode const &  ) = delete;
    ModuleNode( ModuleNode       && ) = delete;
}; // class ModuleNode

LE_NOTHROWNOALIAS void LE_FASTCALL intrusive_ptr_add_ref        ( ModuleNode const * );
#if !LE_SW_SEPARATED_DSP_GUI
LE_NOTHROW        void LE_FASTCALL intrusive_ptr_release_deleter( ModuleNode const * );
#endif // LE_SW_SEPARATED_DSP_GUI

inline LE_NOTHROW
void LE_FASTCALL intrusive_ptr_release( ModuleNode const * LE_RESTRICT const pModuleNode )
{
    LE_ASSUME( pModuleNode );
    if ( BOOST_UNLIKELY( !--pModuleNode->referenceCount_ ) )
    {
    #if LE_SW_SEPARATED_DSP_GUI
        static_assert
        (
            __has_virtual_destructor( ModuleNode ),
            "Direct delete requires a virtual destructor."
        );
        delete pModuleNode;
    #else
        intrusive_ptr_release_deleter( pModuleNode );
    #endif // LE_SW_SEPARATED_DSP_GUI
    }
}


template <class ActualModule>
ActualModule & actualModule( ModuleNode & node )
{
    auto * LE_RESTRICT const pModule
    (
    #if !defined( BOOST_NO_RTTI ) && ( !defined( BOOST_NO_EXCEPTIONS ) || defined( _MSC_VER ) )
        boost::polymorphic_downcast<ActualModule *>( &node )
    #else
               static_cast         <ActualModule *>( &node )
    #endif // BOOST_NO_RTTI
    );
    LE_ASSUME( pModule );
    return *pModule;
}
template <class ActualModule>
ActualModule const & actualModule( ModuleNode const & node ) { return actualModule<ActualModule>( const_cast<ModuleNode &>( node ) ); }

template <class ActualModule>
ModuleNode & node( ActualModule & module )
{
    auto * LE_RESTRICT const pNode
    (
    #if !defined( BOOST_NO_RTTI ) && ( !defined( BOOST_NO_EXCEPTIONS ) || defined( _MSC_VER ) )
        boost::polymorphic_downcast<ModuleNode *>( &module )
    #else
                static_cast        <ModuleNode *>( &module )
    #endif // BOOST_NO_RTTI
    );
    LE_ASSUME( pNode );
    return *pNode;
}
template <class ActualModule>
ModuleNode const & node( ActualModule const & chainedModule ) { return node( const_cast<ActualModule &>( chainedModule ) ); }

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
template <class ActualModule>
BOOST_FORCEINLINE void intrusive_ptr_add_ref( ActualModule const * const pModule ) { intrusive_ptr_add_ref( &Engine::node( *pModule ) ); }
template <class ActualModule>
BOOST_FORCEINLINE void intrusive_ptr_release( ActualModule const * const pModule ) { intrusive_ptr_release( &Engine::node( *pModule ) ); }
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // moduleNode_hpp
