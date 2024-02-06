#pragma once

#include "object.h"
#include "type_traits.hpp"

namespace x
{
    class value
    {
    public:
        value();
        value( value && val );
        value( const value & val );
        template<typename T> value( T && val )
        {
            set( std::forward<T>( val ) );
        }

    public:
        value & operator=( value && val );
        value & operator=( const value & val );
        template<typename T> value & operator=( T && val )
        {
            set( std::forward<T>( val ) );

            return *this;
        }

    public:
        void swap( value & val );

    public:
        value_t type() const;

    public:
        template<typename T> T get() const
        {
            using value_type = std::decay_t<T>;

            if constexpr ( std::is_enum_v<value_type> )
            {
                ASSERT( _type != value_t::INTEGER, "" );

                return (value_type)_integer;
            }
            else if constexpr ( x::is_byte_v<value_type> )
            {
                ASSERT( _type != value_t::BYTE, "" );

                return ( value_type )_byte;
            }
            else if constexpr ( x::is_boolean_v<value_type> )
            {
                ASSERT( _type != value_t::BOOLEAN, "" );

                return _boolean;
            }
            else if constexpr ( x::is_integer_v<value_type> )
            {
                ASSERT( _type != value_t::INTEGER, "" );

                return (value_type)_integer;
            }
            else if constexpr ( x::is_floating_v<value_type> )
            {
                ASSERT( _type != value_t::FLOATING, "" );

                return (value_type)_floating;
            }
            else if constexpr ( x::is_array_v<value_type> )
            {
                ASSERT( _type != value_t::ARRAY, "" );

                if constexpr ( x::is_vector_v<value_type> )
                    return _array->to_vector<value_type>();
                else if constexpr ( x::is_span_v<value_type> )
                    return _array->to_span<value_type>();
                else
                    static_assert( true, "" );
            }
            else if constexpr ( x::is_string_v<value_type> )
            {
                ASSERT( _type != value_t::STRING, "" );

                if constexpr ( x::is_basic_string_v<value_type> )
                    return _string->string();
                else if constexpr ( x::is_basic_string_view_v<value_type> )
                    return _string->string_view();
                else
                    return _string->c_str();
            }
            else if constexpr ( std::is_base_of_v<x::object, value_type> && std::is_pointer_v<value_type> )
            {
                ASSERT( _type != value_t::OBJECT, "" );

                return (value_type)_object;
            }
            else if constexpr ( std::is_base_of_v<x::object, value_type> && std::is_reference_v<value_type> )
            {
                ASSERT( _type != value_t::OBJECT, "" );

                return *(value_type *)_object;
            }
            else if constexpr ( std::is_pointer_v<value_type> )
            {
                ASSERT( _object->is_native(), "");

                return (value_type)_object->data();
            }
            else
            {
                ASSERT( _type != value_t::OBJECT, "" );

                return *(value_type *)_object->data();
            }

            return {};
        }
        template<typename T> void set( T && val )
        {
            using value_type = std::decay_t<T>;

            if constexpr ( std::is_enum_v<value_type> )
            {
                _integer = (int64_t)val;
                _type = value_t::INTEGER;
            }
            else if constexpr ( x::is_null_v<value_type> )
            {
                _object = nullptr;
                _type = value_t::NIL;
            }
            else if constexpr ( x::is_byte_v<value_type> )
            {
                _byte = (uint8_t)val;
                _type = value_t::BYTE;
            }
            else if constexpr ( x::is_boolean_v<value_type> )
            {
                _boolean = val;
                _type = value_t::BOOLEAN;
            }
            else if constexpr ( x::is_integer_v<value_type> )
            {
                _integer = (int64_t)val;
                _type = value_t::INTEGER;
            }
            else if constexpr ( x::is_floating_v<value_type> )
            {
                _floating = (double)val;
                _type = value_t::FLOATING;
            }
            else if constexpr ( x::is_array_v<value_type> )
            {
                _array = new x::array_object();
                _array->assign( val );
                _type = value_t::ARRAY;
            }
            else if constexpr ( x::is_string_v<value_type> )
            {
                _string = new x::string_object();
                _string->assign( val );
                _type = value_t::STRING;
            }
            else if constexpr ( std::is_same_v<value_type, std::any> )
            {
                _string = new x::any_object();
                _string->assign( val );
                _type = value_t::ANY;
            }
            else if constexpr ( std::is_base_of_v<x::object, value_type> && std::is_pointer_v<value_type> )
            {
                _object = val;

                if ( val->is_any() )
                    _type = value_t::ANY;
                else if ( val->is_array() )
                    _type = value_t::ARRAY;
                else if ( val->is_native() )
                    _type = value_t::NATIVE;
                else if ( val->is_script() )
                    _type = value_t::SCRIPT;
                else if ( val->is_closure() )
                    _type = value_t::CLOSURE;
            }
            else if constexpr ( std::is_base_of_v<x::object, value_type> && !std::is_pointer_v<value_type> )
            {
                if ( val.is_any() )
                {
                    _object = new x::any_object();
                    _object->assign( &val );
                    _type = value_t::ANY;
                }
                else if ( val.is_array() )
                {
                    _object = new x::array_object();
                    _object->assign( &val );
                    _type = value_t::ARRAY;
                }
                else if ( val.is_native() )
                {
                    _object = new x::script_object( val.size() );
                    _object->assign( &val );
                    _type = value_t::NATIVE;
                }
                else if ( val.is_script() )
                {
                    _object = new x::script_object( val.size() );
                    _object->assign( &val );
                    _type = value_t::SCRIPT;
                }
                else if ( val.is_closure() )
                {
                    _object = new x::closure_object();
                    _object->assign( &val );
                    _type = value_t::CLOSURE;
                }
            }
            else
            {
                _object = new x::native_object< std::remove_cvref_t<value_type> >();
                if constexpr ( std::is_pointer_v<value_type> )
                    _object->assign( val );
                else
                    _object->assign( &val );
                _type = value_t::NATIVE;
            }
        }

    private:
        value_t _type = value_t::INVALID;
        union
        {
            uint8_t _byte;
            bool _boolean;
            int64_t _integer;
            double _floating;
            object * _object;
            any_object * _any;
            array_object * _array;
            string_object * _string;
            script_object * _script;
            closure_object * _closure;
        };
    };
}