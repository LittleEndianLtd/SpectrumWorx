////////////////////////////////////////////////////////////////////////////////
///
/// \file pimplPrivate.hpp
/// ----------------------
///
/// http://www.gotw.ca/gotw/024.htm
/// http://www.gotw.ca/publications/mill05.htm The Joy of Pimpls
/// http://www.gotw.ca/publications/mill06.htm Uses and Abuses of Inheritance
/// http://c2.com/cgi/wiki?PimplIdiom
/// http://boost.2283326.n4.nabble.com/Review-Request-Generalization-of-the-Pimpl-idiom-td2644770.html
/// http://marcmutz.wordpress.com/translated-articles/pimp-my-pimpl-—-reloaded
/// http://probablydance.com/2013/10/05/type-safe-pimpl-implementation-without-overhead
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pimplPrivate_hpp__A80FC85F_A159_4794_8313_908334D97E92
#define pimplPrivate_hpp__A80FC85F_A159_4794_8313_908334D97E92
#pragma once
//------------------------------------------------------------------------------
#include "pimpl.hpp"

#include <boost/assert.hpp>

#include <new>
#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

template <typename T1, typename T2>
std::ptrdiff_t ptrdiff( T1 const * const p1, T2 const * const p2 )
{
    return reinterpret_cast<char const *>( p1 ) - reinterpret_cast<char const *>( p2 );
}

template <class Interface, class BaseInterface>
typename Implementation<Interface>::type & impl( PImpl<Interface, BaseInterface> & interfaceBase )
{
    static_assert( std::is_empty<BaseInterface                   >::value, "" );
  //static_assert( std::is_empty<Interface                       >::value, "" );
  //static_assert( std::is_empty<PImpl<Interface, BaseInterface> >::value, "" );

    using BaseImplementation = typename Implementation<BaseInterface>::type;
    using Implementation     = typename Implementation<Interface    >::type;

    Interface      * const pInterface     ( static_cast<Interface      *>( &interfaceBase )                    );
    Implementation * const pImplementation( static_cast<Implementation *>( static_cast<void *>( pInterface ) ) );
    BOOST_ASSERT_MSG
    (
        ptrdiff( pImplementation, static_cast<BaseImplementation *>( pImplementation ) )
            ==
        ptrdiff( pInterface     , static_cast<BaseInterface      *>( pInterface      ) ),
        "Interface and implementation with multiple inheritance at different offsets!"
    );
    LE_ASSUME( pImplementation );
    return *pImplementation;
}

template <class Interface, class BaseInterface> typename Implementation<Interface>::type       && impl( PImpl<Interface, BaseInterface>       && anInterface ) { return std::move( impl( anInterface ) ); }
template <class Interface, class BaseInterface> typename Implementation<Interface>::type const  & impl( PImpl<Interface, BaseInterface> const  & anInterface ) { return impl( const_cast<PImpl<Interface, BaseInterface> &>( anInterface ) ); }


template <class Interface>
typename Implementation<Interface>::type & impl( PImpl<Interface, Detail::PImplTerminator> & interfaceBase )
{
    using Implementation = typename Implementation<Interface>::type;

    Interface      * const pInterface     ( static_cast<Interface      *>( &interfaceBase )                     );
    Implementation * const pImplementation( static_cast<Implementation *>( static_cast<void * >( pInterface ) ) );
    LE_ASSUME( pImplementation );
    return *pImplementation;
}

template <class Interface> typename Implementation<Interface>::type       && impl( PImpl<Interface, Detail::PImplTerminator>       && interfaceBase ) { return std::move( impl( interfaceBase ) ); }
template <class Interface> typename Implementation<Interface>::type const  & impl( PImpl<Interface, Detail::PImplTerminator> const  & interfaceBase ) { return impl( const_cast<PImpl<Interface, Detail::PImplTerminator> &>( interfaceBase ) ); }


template <class Interface, class BaseInterface> typename Implementation<Interface>::type        & impl( HeapPImpl<Interface, BaseInterface>        & anInterface ) { return impl( static_cast<PImpl<Interface, BaseInterface> &>( anInterface ) ); }
template <class Interface, class BaseInterface> typename Implementation<Interface>::type       && impl( HeapPImpl<Interface, BaseInterface>       && anInterface ) { return std::move( impl( anInterface ) ); }
template <class Interface, class BaseInterface> typename Implementation<Interface>::type const  & impl( HeapPImpl<Interface, BaseInterface> const  & anInterface ) { return impl( const_cast<HeapPImpl<Interface, BaseInterface> &>( anInterface ) ); }

//...mrmlj...inheritance still does not work correctly on its own: it causes
//...mrmlj..."double destruction" of the base class (quick&dirty workaround for
//...mrmlj...'abstract'/never-instantiated-by-itself bases is to create explicit
//...mrmlj...empty PImpl::~PImpl() specializations for those cases)
//...mrmlj...the pimpl framework should be improved to handle this cleaner...
template <class Interface, class BaseInterface>
LE_NOTHROWNOALIAS LE_FORCEINLINE
PImpl<Interface, BaseInterface>::~PImpl()
{
    using Implementation = typename Implementation<Interface>::type;
    impl( *this ).Implementation::~Implementation();
}

//...mrmlj...simplify/clean this up...
template <class Interface, std::size_t storageSize, std::size_t alignment, class BaseInterface>
typename Implementation<Interface>::type       && impl( StackPImpl<Interface, storageSize, alignment, BaseInterface>       && implInterface ) { return static_cast<typename Implementation<Interface>::type       &&>( impl( static_cast<typename Detail::StackPImplBase<Interface, BaseInterface>::type       &&>( implInterface ) ) ); }
template <class Interface, std::size_t storageSize, std::size_t alignment, class BaseInterface>
typename Implementation<Interface>::type        & impl( StackPImpl<Interface, storageSize, alignment, BaseInterface>        & implInterface ) { return static_cast<typename Implementation<Interface>::type        &>( impl( static_cast<typename Detail::StackPImplBase<Interface, BaseInterface>::type        &>( implInterface ) ) ); }
template <class Interface, std::size_t storageSize, std::size_t alignment, class BaseInterface>
typename Implementation<Interface>::type const  & impl( StackPImpl<Interface, storageSize, alignment, BaseInterface> const  & implInterface ) { return static_cast<typename Implementation<Interface>::type const  &>( impl( static_cast<typename Detail::StackPImplBase<Interface, BaseInterface>::type const  &>( implInterface ) ) ); }

namespace Detail
{
    template <std::size_t PlaceholderSize, std::size_t ImplementationSize>
    struct AssertStorageSize      { static_assert( PlaceholderSize      >= ImplementationSize     , "Stack PImpl: insufficient preallocated storage"           ); };

    template <std::size_t PlaceholderAlignment, std::size_t ImplementationAlignment>
    struct AssertStorageAlignment { static_assert( PlaceholderAlignment >= ImplementationAlignment, "Stack PImpl: insufficient preallocated storage alignment" ); };
} // namespace Detail

template <class Interface, std::size_t storageSize, std::size_t alignment, class BaseInterface>
LE_NOTHROWNOALIAS LE_FORCEINLINE
StackPImpl<Interface, storageSize, alignment, BaseInterface>::StackPImpl()
{
    using Implementation = typename Implementation<Interface>::type;

    Detail::AssertStorageAlignment<std::alignment_of<Placeholder>::value, std::alignment_of<Implementation>::value>();
    Detail::AssertStorageSize     <sizeof( *this )                      , sizeof( Implementation )                >();

    LE_ASSUME( this  );
    Implementation * LE_RESTRICT const pImpl( new ( this ) Implementation );
    LE_ASSUME( pImpl );
}

template <class Interface, std::size_t storageSize, std::size_t alignment, class BaseInterface>
LE_NOTHROWNOALIAS LE_FORCEINLINE
StackPImpl<Interface, storageSize, alignment, BaseInterface>::StackPImpl( StackPImpl && other )
{
    using Implementation = typename Implementation<Interface>::type;

    Detail::AssertStorageAlignment<std::alignment_of<Placeholder>::value, std::alignment_of<Implementation>::value>();
    Detail::AssertStorageSize     <sizeof( *this )                      , sizeof( Implementation )                >();

    LE_ASSUME( this  );
    Implementation * LE_RESTRICT const pImpl( new ( this ) Implementation( std::move( impl( std::move( other ) ) ) ) );
    LE_ASSUME( pImpl );
}

template <class Interface, std::size_t storageSize, std::size_t alignment, class BaseInterface>
LE_NOTHROWNOALIAS LE_FORCEINLINE
StackPImpl<Interface, storageSize, alignment, BaseInterface>::StackPImpl( Interface && other )
{
    using Implementation = typename Implementation<Interface>::type;

    Detail::AssertStorageAlignment<std::alignment_of<Placeholder>::value, std::alignment_of<Implementation>::value>();
    Detail::AssertStorageSize     <sizeof( *this )                      , sizeof( Implementation )                >();

    LE_ASSUME( this  );
    Implementation && source( std::move( impl( std::move( other ) ) ) );
    Implementation * LE_RESTRICT const pImpl( new ( this ) Implementation( std::move( source ) ) );
    LE_ASSUME( pImpl );
}

template <class Interface, std::size_t storageSize, std::size_t alignment, class BaseInterface>
template <typename T>
LE_NOTHROWNOALIAS LE_FORCEINLINE
StackPImpl<Interface, storageSize, alignment, BaseInterface>::StackPImpl( T && constructionArgument )
{
    using Implementation = typename Implementation<Interface>::type;

    static_assert( sizeof( *this )                       >= sizeof( Implementation )                , "Stack PImpl: insufficient preallocated storage"           );
    static_assert( std::alignment_of<Placeholder>::value >= std::alignment_of<Implementation>::value, "Stack PImpl: insufficient preallocated storage alignment" );
    LE_ASSUME( this  );
    Implementation * LE_RESTRICT const pImpl( new ( this ) Implementation( std::forward<T>( constructionArgument ) ) );
    LE_ASSUME( pImpl );
}


template <class Interface, class BaseInterface>
LE_NOTHROWNOALIAS
void LE_FASTCALL_ABI HeapPImpl<Interface, BaseInterface>::operator delete( void * const pInterface )
{
    ::operator delete( pInterface );
}

template <class Interface, class BaseInterface>
LE_NOTHROWNOALIAS
boost::intrusive_ptr<Interface> LE_FASTCALL_ABI HeapPImpl<Interface, BaseInterface>::create()
{
    using Implementation = typename Implementation<Interface>::type;
    Implementation * LE_RESTRICT const pImplementation( new (std::nothrow) Implementation                );
    Interface      *             const pInterface     ( reinterpret_cast<Interface *>( pImplementation ) );
    // https://llvm.org/bugs/show_bug.cgi?id=20521
#if !defined( __clang__ )
    LE_ASSUME( &Utility::impl( *pInterface ) == pImplementation );
#endif // !__clang__
    return boost::intrusive_ptr<Interface>( pInterface );
}

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // pimplPrivate_hpp
