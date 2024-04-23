////////////////////////////////////////////////////////////////////////////////
///
/// \file moduleChainImpl.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef moduleChainImpl_hpp__E639274C_2DA1_42EC_9B43_AF42EBB9C987
#define moduleChainImpl_hpp__E639274C_2DA1_42EC_9B43_AF42EBB9C987
#pragma once
//------------------------------------------------------------------------------
#include "moduleNode.hpp"

#include "le/parameters/lfoImpl.hpp" //...mrmlj...only for the LFOImpl::Timer nested type...
#include "le/utility/platformSpecifics.hpp"

#include "boost/config.hpp"
#include "boost/get_pointer.hpp"
#if defined( __clang__ ) && !defined( LE_EXCEPTION_ON )
#define throw LE_UNREACHABLE_CODE()
#endif // LE_EXCEPTION_ON
#include "boost/intrusive/circular_list_algorithms.hpp"
#if defined( __clang__ ) && !defined( LE_EXCEPTION_ON )
#undef throw
#endif // LE_EXCEPTION_ON
#include "boost/smart_ptr/intrusive_ptr.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Engine { struct StorageFactors; }
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

class ModuleDSP;
class Setup;

////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleChainImpl
///
/// \note Using an intrusive container (with reference counted objects) because
/// of the implicit thread safety (as well as minor efficiency improvements).
///                                           (25.02.2014.) (Domagoj Saric)
///
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    struct module_node_traits
    {
        using       node     = ModuleNode        ;
        using       node_ptr = node             *;
        using const_node_ptr = node       const *;

        static node_ptr get_next    ( const_node_ptr n                );
        static node_ptr get_previous( const_node_ptr n                );
        static void     set_next    ( node_ptr       n, node_ptr next );
        static void     set_previous( node_ptr       n, node_ptr prev );
    }; // struct module_node_traits

    //...mrmlj...
    /// \note Boost.Intrusive containers do not support shared ownership
    /// semantics so we cannot use them yet (only the
    /// boost::intrusive::circular_list_algorithms come in handy).
    /// https://svn.boost.org/trac/boost/ticket/7003
    ///                                       (08.06.2012.) (Domagoj Saric)
    //struct module_node_value_traits
    //{
    //    typedef module_node_traits                     node_traits;
    //    typedef node_traits::node                      node;
    //    typedef node_traits::node_ptr                  node_ptr;
    //    typedef node_traits::const_node_ptr            const_node_ptr;
    //    typedef ModuleDSP                              value_type;
    //    typedef ModulePtr                              pointer;
    //    typedef ModuleCPtr                             const_pointer;
    //    static const boost::intrusive::link_mode_type link_mode = boost::intrusive::safe_link;
    //    static node_ptr       to_node_ptr ( value_type           & );
    //    static const_node_ptr to_node_ptr ( value_type     const & );
    //    static pointer        to_value_ptr( node_ptr       const & );
    //    static const_pointer  to_value_ptr( const_node_ptr const & );
    //};
} // namespace Detail

class ModuleChainBase
    :
    //...mrmlj...
    //: public boost::intrusive::list<ModuleDSP, boost::intrusive::value_traits<Detail::module_node_value_traits>>
    private ModuleNode,
    private boost::noncopyable
{
public: // Typedefs
    using difference_type = std:: int8_t;
    using       size_type = std::uint8_t;


    using Node            = ModuleNode;
    using node_algorithms = boost::intrusive::circular_list_algorithms<Detail::module_node_traits>;

public: // Iterator
    class chain_const_iterator
        :
        public Node::NodeCPtr,
        public std::iterator<std::bidirectional_iterator_tag, Node const, std::int8_t>
    {
    public:
        using Node        = ModuleChainBase::Node const;
        using smart_ptr_t = Node::NodeCPtr;

    public:
    #ifdef __clang__
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wassume"
    #endif // __clang__
        chain_const_iterator( decltype( nullptr ) = nullptr                           ) {}
        chain_const_iterator( value_type           const  * LE_RESTRICT const pointer ) : smart_ptr_t(                            pointer  ) { LE_ASSUME( pointer     ); }
        chain_const_iterator( chain_const_iterator const  &                   other   ) : smart_ptr_t(                            other    ) { LE_ASSUME( this->get() ); }
        chain_const_iterator( chain_const_iterator       &&                   other   ) : smart_ptr_t( std::forward<smart_ptr_t>( other  ) ) { LE_ASSUME( this->get() ); }
        template <typename Source>
        chain_const_iterator( Source                     &&                   source  ) : smart_ptr_t( std::forward<Source     >( source ) ) { LE_ASSUME( this->get() ); }
        chain_const_iterator & operator++( int ) { ++(*this); return reinterpret_cast<chain_const_iterator &>( this->get()->previous_ ); }
        chain_const_iterator & operator++()
        {
            Node * LE_RESTRICT const pThisNode( this->           get() );
            LE_ASSUME( pThisNode );
            Node * LE_RESTRICT const pNextNode( pThisNode->next_.get() );
            LE_ASSUME( pNextNode );
            LE_ASSUME( pThisNode->referenceCount_ >= 1 ); // can be equal to 1 with iterators to a removed module...
            this->reset( pNextNode );
            LE_ASSUME( this->get() == pNextNode );
            return *this;
        }
        chain_const_iterator & operator--()
        {
            Node * LE_RESTRICT const pNode( this->get()->previous_.get() );
            LE_ASSUME( pNode );
            this->reset( pNode );
            return *this;
        }
        reference operator * () const
        {
            LE_ASSUME( this );
            Node * LE_RESTRICT const pNode( this->get() );
            LE_ASSUME( pNode );
            return *pNode;
        }
        pointer operator -> () const { return &this->operator*(); }
    #ifdef __clang__
        #pragma clang diagnostic pop
    #endif // __clang__

        using smart_ptr_t::operator=;
    }; // class chain_const_iterator

    class chain_iterator //...mrmlj...
        :
        public chain_const_iterator
    {
    public:
        using Node = ModuleChainBase::Node;

        using value_type = Node        ;
        using pointer    = value_type *;
        using reference  = value_type &;

        using smart_ptr_t = boost::intrusive_ptr<Node>;

    public:
        chain_iterator( decltype( nullptr ) = nullptr                     ) {}
        chain_iterator( value_type            * LE_RESTRICT const pointer ) : chain_const_iterator(                                     pointer  ) {}
        chain_iterator( chain_iterator const  &                   other   ) : chain_const_iterator(                                     other    ) {}
        chain_iterator( chain_iterator       &&                   other   ) : chain_const_iterator( std::forward<chain_const_iterator>( other  ) ) {}
        template <typename Source>
        chain_iterator( Source               &&                   source  ) : chain_const_iterator( std::forward<Source              >( source ) ) {}

        chain_iterator & operator=( pointer        const   pOther ) { static_cast<smart_ptr_t &>( *this ) =                                   pOther  ; return *this; }
        chain_iterator & operator=( smart_ptr_t    const & pOther ) { static_cast<smart_ptr_t &>( *this ) =                                   pOther  ; return *this; }
		chain_iterator & operator=( chain_iterator const & pOther ) { static_cast<smart_ptr_t &>( *this ) = static_cast<smart_ptr_t const &>( pOther ); return *this; }

        chain_iterator & operator--()      { return static_cast<chain_iterator &>( chain_const_iterator::operator --() ); }
        chain_iterator & operator++(     ) { return static_cast<chain_iterator &>( chain_const_iterator::operator ++() ); }
        chain_iterator & operator++( int ) { ++(*this); return reinterpret_cast<chain_iterator &>( this->get()->previous_ ); }

        reference operator  * () const { return const_cast<reference>( chain_const_iterator::operator *() ); }
        pointer   operator -> () const { return &this->operator*(); }

        operator smart_ptr_t       & ()       { return reinterpret_cast<smart_ptr_t &>( static_cast<chain_const_iterator::smart_ptr_t &>( *this ) ); }
        operator smart_ptr_t const & () const { return const_cast<chain_iterator &>( *this ).operator smart_ptr_t &(); }

        Node * get() const { return const_cast<Node *>( chain_const_iterator::get() ); }
    }; // class chain_iterator

    using       iterator = chain_iterator      ;
    using const_iterator = chain_const_iterator;

public:
    LE_NOTHROWNOALIAS ModuleChainBase();
    LE_NOTHROW        ModuleChainBase( ModuleChainBase && );
    LE_NOTHROW       ~ModuleChainBase();

    LE_NOTHROW ModuleChainBase & operator=( ModuleChainBase && );

    LE_NOTHROWNOALIAS
    std::uint8_t getIndexForModule( Node const & ) const;
    template <class Module>
    std::uint8_t getIndexForModule( Module const & chainedModule ) const { return getIndexForModule( node( chainedModule ) ); }

    LE_NOTHROWNOALIAS iterator       LE_FASTCALL module( std::uint8_t index )      ;
    LE_NOTHROWNOALIAS const_iterator LE_FASTCALL module( std::uint8_t index ) const;

    void LE_FASTCALL moveModule( std::uint8_t sourceIndex, std::uint8_t targetIndex );

    LE_NOTHROWNOALIAS std::uint8_t LE_FASTCALL size() const;

    LE_NOTHROWNOALIAS bool LE_FASTCALL empty() const;

    void LE_FASTCALL clear();

    static LE_NOTHROWNOALIAS       iterator LE_FASTCALL iterator_to( Node       & );
    static LE_NOTHROWNOALIAS const_iterator LE_FASTCALL iterator_to( Node const & );

    LE_NOTHROWNOALIAS       iterator LE_FASTCALL begin()      ;
    LE_NOTHROWNOALIAS const_iterator LE_FASTCALL begin() const;

    LE_NOTHROWNOALIAS       iterator LE_FASTCALL end  ()      ;
    LE_NOTHROWNOALIAS const_iterator LE_FASTCALL end  () const;

    template <class Iterator>
    LE_NOTHROWNOALIAS bool isEnd( Iterator const & pModule ) const { return isEnd( static_cast<Node const *>( boost::get_pointer( pModule ) ) ); }
    LE_NOTHROWNOALIAS bool isEnd( Node     const *         ) const;

    void LE_FASTCALL push_back( Node & );

    void LE_FASTCALL insertAtAndReplace( iterator const & pInsertPosition, Node & moduleToInsert );

    void LE_FASTCALL remove( Node & );

    template <class ActualModule, class Functor>
    void forEach( Functor && f )
    {
        //for ( auto & module : modules )
        //    f( module );

        iterator pModulePtr( this->begin() );
        while ( !isEnd( pModulePtr ) )
        {
            /// \note pModulePtr must remain pointing to the current module for
            /// the entire duration of the call to f in order to maintain its
            /// reference count (to keep it alive).
            ///                               (06.03.2014.) (Domagoj Saric)
            f( actualModule<ActualModule>( *pModulePtr ) );
            ++pModulePtr;
        }
    }

    template <class ActualModule, class Functor>
    void forEach( Functor && f ) const { const_cast<ModuleChainBase &>( *this ).forEach<ActualModule const>( std::forward<Functor>( f ) ); }

    //...mrmlj...quick-hack for sdk...
    Node       & rootNode()       { LE_ASSUME( this ); return *this; }
    Node const & rootNode() const { LE_ASSUME( this ); return *this; }

private:
    void LE_FASTCALL resetRoot();

    void LE_FASTCALL moveAssign( ModuleChainBase && );
}; // class ModuleChainBase


class ModuleChainImpl : public ModuleChainBase
{
public: // Typedefs
    using Module = ModuleDSP;

    using       value_type = Module                                    ;
    using       reference  = value_type                               &;
    using const_reference  = value_type                         const &;
    using       pointer    = boost::intrusive_ptr<Module      >        ;
    using const_pointer    = boost::intrusive_ptr<Module const>        ;

public:
#if defined( _MSC_VER ) && _MSC_VER < 1900
    ModuleChainImpl() {}
    ModuleChainImpl( ModuleChainImpl && other ) : ModuleChainBase( std::forward<ModuleChainBase>( other ) ) {}
#else
    using ModuleChainBase::ModuleChainBase;
#endif // _MSC_VER
    using ModuleChainBase::operator =;

    void LE_NOTHROW LE_FASTCALL preProcessAll( Parameters::LFOImpl::Timer const &, Setup const & );

    void LE_NOTHROW resetAll();

    bool LE_NOTHROW resizeAll
    (
        StorageFactors const & newfactors,
        StorageFactors const & currentFactors
    );
}; // class ModuleChainImpl

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // moduleChainImpl_hpp
