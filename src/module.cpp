#include "module.h"

#include <iostream>

#include "endian.hpp"
#include "symbols.h"
#include "visitor.h"

namespace
{
    template<typename T> inline void read( std::istream & in, T & val );
    template<typename T> inline void write( std::ostream & out, const T & val );

    template<typename T, typename = void> struct ioable;
    template<typename T> struct ioable<T, std::enable_if_t<std::is_enum_v<T>>>
    {
        static inline void iread( std::istream & in, T & val )
        {
            if constexpr ( sizeof( T ) == 1 )
                read( in, (x::uint8 &)val );
            else if constexpr ( sizeof( T ) == 2 )
                read( in, (x::uint16 &)val );
            else if constexpr ( sizeof( T ) == 4 )
                read( in, (x::uint32 &)val );
            else if constexpr ( sizeof( T ) == 8 )
                read( in, (x::uint64 &)val );
        }
        static inline void owrite( std::ostream & out, const T & val )
        {
            if constexpr ( sizeof( T ) == 1 )
                write( out, (x::uint8)val );
            else if constexpr ( sizeof( T ) == 2 )
                write( out, (x::uint16)val );
            else if constexpr ( sizeof( T ) == 4 )
                write( out, (x::uint32)val );
            else if constexpr ( sizeof( T ) == 8 )
                write( out, (x::uint64)val );
        }
    };
    template<typename T> struct ioable<T, std::enable_if_t<std::is_integral_v<T>>>
    {
        static inline void iread( std::istream & in, T & val )
        {
            T tmp = val;
            in.read( reinterpret_cast<char *>( &tmp ), sizeof( T ) );
            val = x::endian::native_to_little( tmp );
        }
        static inline void owrite( std::ostream & out, const T & val )
        {
            T tmp = x::endian::native_to_little( val );
            out.write( reinterpret_cast<const char *>( &tmp ), sizeof( T ) );
        }
    };
    template<typename T> struct ioable<T, std::enable_if_t<std::is_same_v<x::range, T>>>
    {
        static inline void iread( std::istream & in, T & val )
        {
            read( in, val.beg );
            read( in, val.end );
        }
        static inline void owrite( std::ostream & out, const T & val )
        {
            write( out, val.beg );
            write( out, val.end );
        }
    };
    template<typename T> struct ioable<T, std::enable_if_t<x::is_template_of_v<std::vector, T>>>
    {
        static inline void iread( std::istream & in, T & val )
        {
            typename T::size_type size = 0;
            read( in, size );
            val.resize( size );
            for ( auto & it : val )
            {
                read( in, it );
            }
        }
        static inline void owrite( std::ostream & out, const T & val )
        {
            write( out, val.size() );
            for ( auto it : val )
            {
                write( out, it );
            }
        }
    };
    template<typename T> struct ioable<T, std::enable_if_t<x::is_template_of_v<std::basic_string, T>>>
    {
        static inline void iread( std::istream & in, T & val )
        {
            typename T::size_type size = 0;
            read( in, size );
            val.resize( size );
            in.read( val.data(), val.size() );
        }
        static inline void owrite( std::ostream & out, const T & val )
        {
            write( out, val.size() );
            out.write( val.c_str(), val.size() );
        }
    };

    template<typename T> inline void read( std::istream & in, T & val )
    {
        ioable<T>::iread( in, val );
    }
    template<typename T> inline void write( std::ostream & out, const T & val )
    {
        ioable<T>::owrite( out, val );
    }
}

x::module::module()
    : version( version_num )
{
}

bool x::module::merge( const x::module_ptr & other )
{
    return false;
}

void x::module::load( std::istream & in )
{
    x::uint32 magic;
    x::section_t type;
    std::size_t size = 0;

    read( in, magic );

    read( in, version );
    read( in, lasttime );

    read( in, name );
    read( in, author );
    read( in, origin );

    while ( !in.eof() )
    {
        read( in, type );

        switch ( type )
        {
        case x::section_t::TYPE:
            read( in, size );
            types.items.resize( size );
            for ( auto & it : types.items )
            {
                read( in, it.flag );
                read( in, it.size );
                read( in, it.name );
            }
            break;
        case x::section_t::TEMP:
            read( in, size );
            temps.items.resize( size );
            for ( auto & it : temps.items )
            {
                read( in, it.name );
                read( in, it.data );
            }
            break;
        case x::section_t::DESC:
            read( in, size );
            descs.items.resize( size );
            for ( auto & it : descs.items )
            {
                read( in, it.flag );
                read( in, it.is_const );

                switch ( it.flag )
                {
                case x::desc_section::flag_t::TYPE:
                    read( in, it.type.type );
                    break;
                case x::desc_section::flag_t::REFTYPE:
                    read( in, it.reftype.type );
                    break;
                case x::desc_section::flag_t::TEMPTYPE:
                    read( in, it.temptype.temp );
                    break;
                case x::desc_section::flag_t::FUNCTYPE:
                    read( in, it.functype.result );
                    break;
                default:
                    break;
                }
            }
            break;
        case x::section_t::DEPEND:
            read( in, size );
            depends.items.resize( size );
            for ( auto & it : depends.items )
            {
                read( in, it.flag );
                read( in, it.name );
            }
            break;
        case x::section_t::GLOBAL:
            read( in, size );
            globals.items.resize( size );
            for ( auto & it : globals.items )
            {
                read( in, it.flag );
                read( in, it.type );
                read( in, it.name );
                read( in, it.class_init );
            }
            break;
        case x::section_t::FUNCTION:
            read( in, size );
            functions.items.resize( size );
            for ( auto & it : functions.items )
            {
                read( in, it.is_const );
                read( in, it.is_async );
                read( in, it.is_static );
                read( in, it.is_virtual );
                read( in, it.name );
                read( in, it.code );
                read( in, it.owner );
                read( in, it.result );
                read( in, it.parameters );
            }
            break;
        case x::section_t::VARIABLE:
            read( in, size );
            variables.items.resize( size );
            for ( auto & it : variables.items )
            {
                read( in, it.flag );
                read( in, it.idx );
                read( in, it.name );
                read( in, it.owner );
                read( in, it.value );
            }
            break;
        case x::section_t::ATTRIBUTE:
            read( in, size );
            variables.items.resize( size );
            for ( auto & it : attributes.items )
            {
                read( in, it.type );
                read( in, it.key );
                read( in, it.value );
            }
            break;
        case x::section_t::OPCODEDATA:
            read( in, opcodedatas.datas );
            break;
        case x::section_t::STRINGDATA:
            read( in, stringdatas.datas );
            break;
        case x::section_t::CUSTOMDATA:
            read( in, customdatas.datas );
            break;
        default:
            break;
        }
    }
}

void x::module::save( std::ostream & out ) const
{
    write( out, magic_num );

    write( out, version );
    write( out, lasttime );

    write( out, name );
    write( out, author );
    write( out, origin );

    write( out, x::section_t::TYPE );
    write( out, types.items.size() );
    for ( const auto & it : types.items )
    {
        write( out, it.flag );
        write( out, it.size );
        write( out, it.name );
    }

    write( out, x::section_t::TEMP );
    write( out, temps.items.size() );
    for ( const auto & it : temps.items )
    {
        write( out, it.name );
        write( out, it.data );
    }

    write( out, x::section_t::DESC );
    write( out, descs.items.size() );
    for ( const auto & it : descs.items )
    {
        write( out, it.flag );
        write( out, it.is_const );

        switch ( it.flag )
        {
        case x::desc_section::flag_t::TYPE:
            write( out, it.type.type );
            break;
        case x::desc_section::flag_t::REFTYPE:
            write( out, it.reftype.type );
            break;
        case x::desc_section::flag_t::TEMPTYPE:
            write( out, it.temptype.temp );
            break;
        case x::desc_section::flag_t::FUNCTYPE:
            write( out, it.functype.result );
            break;
        default:
            break;
        }
    }

    write( out, x::section_t::DEPEND );
    write( out, depends.items.size() );
    for ( const auto & it : depends.items )
    {
        write( out, it.flag );
        write( out, it.name );
    }

    write( out, x::section_t::GLOBAL );
    write( out, globals.items.size() );
    for ( const auto & it : globals.items )
    {
        write( out, it.flag );
        write( out, it.type );
        write( out, it.name );
        write( out, it.class_init );
    }

    write( out, x::section_t::FUNCTION );
    write( out, functions.items.size() );
    for ( const auto & it : functions.items )
    {
        write( out, it.is_const );
        write( out, it.is_async );
        write( out, it.is_static );
        write( out, it.is_virtual );
        write( out, it.name );
        write( out, it.code );
        write( out, it.owner );
        write( out, it.result );
        write( out, it.parameters );
    }

    write( out, x::section_t::VARIABLE );
    write( out, variables.items.size() );
    for ( const auto & it : variables.items )
    {
        write( out, it.flag );
        write( out, it.idx );
        write( out, it.name );
        write( out, it.owner );
        write( out, it.value );
    }

    write( out, x::section_t::ATTRIBUTE );
    write( out, attributes.items.size() );
    for ( const auto & it : attributes.items )
    {
        write( out, it.type );
        write( out, it.key );
        write( out, it.value );
    }

    write( out, x::section_t::OPCODEDATA );
    write( out, opcodedatas.datas );

    write( out, x::section_t::STRINGDATA );
    write( out, stringdatas.datas );

    write( out, x::section_t::CUSTOMDATA );
    write( out, customdatas.datas );
}
