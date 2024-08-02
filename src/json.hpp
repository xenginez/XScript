#pragma once

#include <map>
#include <array>
#include <vector>
#include <string>
#include <memory>
#include <variant>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <charconv>
#include <unordered_map>

namespace x
{
	template<class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem>> class json_impl
	{
	public:
		using char_type = _Elem;
		using string_type = std::basic_string<_Elem, _Traits, _Alloc>;
		using string_view_type = std::basic_string_view<_Elem, _Traits>;
		using istream_type = std::basic_istream<_Elem, _Traits>;
		using ostream_type = std::basic_ostream<_Elem, _Traits>;
		using isstream_type = std::basic_istringstream<_Elem, _Traits, _Alloc>;
		using osstream_type = std::basic_ostringstream<_Elem, _Traits, _Alloc>;
		using array_type = std::vector<json_impl<_Elem, _Traits, _Alloc>>;
		using object_type = std::vector<std::pair<string_type, json_impl<_Elem, _Traits, _Alloc>>>;

	private:
		using shared_array_type = std::shared_ptr<array_type>;
		using shared_object_type = std::shared_ptr<object_type>;
		using variant_type = std::variant<std::monostate, std::nullptr_t, bool, std::int64_t, std::uint64_t, double, string_type, shared_array_type, shared_object_type>;

	private:
		template<typename T> struct constexpr_flags;
		template<> struct constexpr_flags<char>
		{
			static constexpr const char rev_flag = '-';
			static constexpr const char null_flag = 'n';
			static constexpr const char true_flag = 't';
			static constexpr const char false_flag = 'f';
			static constexpr const char float_flag = '.';
			static constexpr const char pair_flag = ':';
			static constexpr const char comma_flag = ',';
			static constexpr const char space_flag = ' ';
			static constexpr const char string_flag = '\"';
			static constexpr const char array_beg_flag = '[';
			static constexpr const char array_end_flag = ']';
			static constexpr const char object_beg_flag = '{';
			static constexpr const char object_end_flag = '}';
		};
		template<> struct constexpr_flags<wchar_t>
		{
			static constexpr const wchar_t rev_flag = L'-';
			static constexpr const wchar_t null_flag = L'n';
			static constexpr const wchar_t true_flag = L't';
			static constexpr const wchar_t false_flag = L'f';
			static constexpr const wchar_t float_flag = L'.';
			static constexpr const wchar_t pair_flag = L':';
			static constexpr const wchar_t comma_flag = L',';
			static constexpr const wchar_t space_flag = L' ';
			static constexpr const wchar_t string_flag = L'\"';
			static constexpr const wchar_t array_beg_flag = L'[';
			static constexpr const wchar_t array_end_flag = L']';
			static constexpr const wchar_t object_beg_flag = L'{';
			static constexpr const wchar_t object_end_flag = L'}';
		};
		template<> struct constexpr_flags<char8_t>
		{
			static constexpr const char8_t rev_flag = u8'-';
			static constexpr const char8_t null_flag = u8'n';
			static constexpr const char8_t true_flag = u8't';
			static constexpr const char8_t false_flag = u8'f';
			static constexpr const char8_t float_flag = u8'.';
			static constexpr const char8_t pair_flag = u8':';
			static constexpr const char8_t comma_flag = u8',';
			static constexpr const char8_t space_flag = u8' ';
			static constexpr const char8_t string_flag = u8'\"';
			static constexpr const char8_t array_beg_flag = u8'[';
			static constexpr const char8_t array_end_flag = u8']';
			static constexpr const char8_t object_beg_flag = u8'{';
			static constexpr const char8_t object_end_flag = u8'}';
		};
		template<> struct constexpr_flags<char16_t>
		{
			static constexpr const char16_t rev_flag = u'-';
			static constexpr const char16_t null_flag = u'n';
			static constexpr const char16_t true_flag = u't';
			static constexpr const char16_t false_flag = u'f';
			static constexpr const char16_t float_flag = u'.';
			static constexpr const char16_t pair_flag = u':';
			static constexpr const char16_t comma_flag = u',';
			static constexpr const char16_t space_flag = u' ';
			static constexpr const char16_t string_flag = u'\"';
			static constexpr const char16_t array_beg_flag = u'[';
			static constexpr const char16_t array_end_flag = u']';
			static constexpr const char16_t object_beg_flag = u'{';
			static constexpr const char16_t object_end_flag = u'}';
		};
		template<> struct constexpr_flags<char32_t>
		{
			static constexpr const char32_t rev_flag = U'-';
			static constexpr const char32_t null_flag = U'n';
			static constexpr const char32_t true_flag = U't';
			static constexpr const char32_t false_flag = U'f';
			static constexpr const char32_t float_flag = U'.';
			static constexpr const char32_t pair_flag = U':';
			static constexpr const char32_t comma_flag = U',';
			static constexpr const char32_t space_flag = U' ';
			static constexpr const char32_t string_flag = U'\"';
			static constexpr const char32_t array_beg_flag = U'[';
			static constexpr const char32_t array_end_flag = U']';
			static constexpr const char32_t object_beg_flag = U'{';
			static constexpr const char32_t object_end_flag = U'}';
		};
		using constexpr_flags_type = constexpr_flags<char_type>;

	public:
		json_impl() = default;
		json_impl( json_impl && val )
		{
			std::swap( _var, val._var );
		}
		json_impl( const json_impl & val )
			: _var( val._var )
		{

		}
		json_impl( std::nullptr_t )
			: _var( nullptr )
		{
		}
		json_impl( bool val )
			: _var( val )
		{
		}
		json_impl( std::int8_t val )
			: _var( (std::int64_t)val )
		{
		}
		json_impl( std::int16_t val )
			: _var( (std::int64_t)val )
		{
		}
		json_impl( std::int32_t val )
			: _var( (std::int64_t)val )
		{
		}
		json_impl( std::int64_t val )
			: _var( (std::int64_t)val )
		{
		}
		json_impl( std::uint8_t val )
			: _var( (std::uint64_t)val )
		{
		}
		json_impl( std::uint16_t val )
			: _var( (std::uint64_t)val )
		{
		}
		json_impl( std::uint32_t val )
			: _var( (std::uint64_t)val )
		{
		}
		json_impl( std::uint64_t val )
			: _var( (std::uint64_t)val )
		{
		}
		json_impl( double val )
			: _var( (double)val )
		{
		}
		json_impl( const char * const val )
			: _var( val )
		{
		}
		json_impl( string_view_type val )
			: _var( string_type( val.begin(), val.end() ) )
		{
		}
		json_impl( const string_type & val )
			: _var( val )
		{
		}
		template<typename A> json_impl( const std::vector<json_impl<_Elem, _Traits, _Alloc>, A> & val )
			: _var( std::make_shared<array_type>( val.begin(), val.end() ) )
		{
		}
		template<std::size_t N> json_impl( const std::array<json_impl<_Elem, _Traits, _Alloc>, N> & val )
			: _var( std::make_shared<array_type>( val.begin(), val.end() ) )
		{
		}
		template <class K, class C, class A> json_impl( const std::map<K, json_impl<_Elem, _Traits, _Alloc>, C, A> & val )
			: _var( std::make_shared<object_type>() )
		{
			for ( const auto & it : val )
			{
				if constexpr ( std::is_pointer_v<K> )
					std::get<shared_object_type>( _var )->push_back( { it.first, it.second } );
				else
					std::get<shared_object_type>( _var )->push_back( { { it.first.begin(), it.first.end() }, it.second } );
			}
		}
		template <class K, class C, class A> json_impl( const std::unordered_map<K, json_impl<_Elem, _Traits, _Alloc>, C, A> & val )
			: _var( std::make_shared<object_type>() )
		{
			for ( const auto & it : val )
			{
				if constexpr ( std::is_pointer_v<K> )
					std::get<shared_object_type>( _var )->push_back( { it.first, it.second } );
				else
					std::get<shared_object_type>( _var )->push_back( { { it.first.begin(), it.first.end() }, it.second } );
			}
		}
		template <class A> json_impl( const std::vector<std::pair<string_type, json_impl<_Elem, _Traits, _Alloc>>, A> & val )
			: _var( std::make_shared<object_type>() )
		{
			for ( const auto & it : val )
			{
				std::get<shared_object_type>( _var ).push_back( it.first, it.second );
			}

			return *this;
		}
		~json_impl() = default;

	public:
		json_impl & operator=( json_impl && val )
		{
			std::swap( _var, val._var );
			return *this;
		}
		json_impl & operator=( const json_impl & val )
		{
			_var = val._var;
			return *this;
		}
		json_impl & operator=( std::nullptr_t )
		{
			_var = nullptr;
			return *this;
		}
		json_impl & operator=( bool val )
		{
			_var = val;
			return *this;
		}
		json_impl & operator=( std::int8_t val )
		{
			_var = (std::int64_t)val;
			return *this;
		}
		json_impl & operator=( std::int16_t val )
		{
			_var = (std::int64_t)val;
			return *this;
		}
		json_impl & operator=( std::int32_t val )
		{
			_var = (std::int64_t)val;
			return *this;
		}
		json_impl & operator=( std::int64_t val )
		{
			_var = (std::int64_t)val;
			return *this;
		}
		json_impl & operator=( std::uint8_t val )
		{
			_var = (std::uint64_t)val;
			return *this;
		}
		json_impl & operator=( std::uint16_t val )
		{
			_var = (std::uint64_t)val;
			return *this;
		}
		json_impl & operator=( std::uint32_t val )
		{
			_var = (std::uint64_t)val;
			return *this;
		}
		json_impl & operator=( std::uint64_t val )
		{
			_var = (std::uint64_t)val;
			return *this;
		}
		json_impl & operator=( double val )
		{
			_var = (double)val;
			return *this;
		}
		json_impl & operator=( const char * const val )
		{
			_var = val;
			return *this;
		}
		json_impl & operator=( string_view_type val )
		{
			_var = string_type( val.begin(), val.end() );
			return *this;
		}
		json_impl & operator=( const string_type & val )
		{
			_var = val;
			return *this;
		}
		template<typename A> json_impl & operator=( const std::vector<json_impl<_Elem, _Traits, _Alloc>, A> & val )
		{
			_var = std::make_shared<array_type>( val.begin(), val.end() );
			return *this;
		}
		template<std::size_t N> json_impl & operator=( const std::array<json_impl<_Elem, _Traits, _Alloc>, N> & val )
		{
			_var = std::make_shared<array_type>( val.begin(), val.end() );
			return *this;
		}
		template <class K, class C, class A> json_impl & operator=( const std::map<K, json_impl<_Elem, _Traits, _Alloc>, C, A> & val )
		{
			_var = std::make_shared<object_type>();

			for ( const auto & it : val )
			{
				std::get<shared_object_type>( _var ).push_back( it.first, it.second );
			}

			return *this;
		}
		template <class K, class C, class A> json_impl & operator=( const std::unordered_map<K, json_impl<_Elem, _Traits, _Alloc>, C, A> & val )
		{
			_var = std::make_shared<object_type>();

			for ( const auto & it : val )
			{
				std::get<shared_object_type>( _var ).push_back( it.first, it.second );
			}

			return *this;
		}
		template <class A> json_impl & operator=( const std::vector<std::pair<string_type, json_impl<_Elem, _Traits, _Alloc>>, A> & val )
		{
			_var = std::make_shared<object_type>( val.begin(), val.end() );

			return *this;
		}

	private:
		json_impl( const variant_type & val )
			: _var( val )
		{
		}

	public:
		bool empty() const
		{
			return _var.index() == 0;
		}
		bool is_null() const
		{
			return _var.index() == 1;
		}
		bool is_bool() const
		{
			return _var.index() == 2;
		}
		bool is_int() const
		{
			return _var.index() == 3;
		}
		bool is_uint() const
		{
			return _var.index() == 4;
		}
		bool is_float() const
		{
			return _var.index() == 5;
		}
		bool is_array() const
		{
			return _var.index() == 7;
		}
		bool is_object() const
		{
			return _var.index() == 8;
		}
		bool is_string() const
		{
			return _var.index() == 6;
		}
		bool contains( string_view_type key ) const
		{
			if ( is_object() )
			{
				const auto & obj = to_object();
				return std::find_if( obj.begin(), obj.end(), [&key]( const auto & val ) { return val.first == key; } ) != obj.end();
			}

			return false;
		}

	public:
		bool to_bool() const
		{
			return std::get<bool>( _var );
		}
		std::int64_t to_int() const
		{
			return std::get<std::int64_t>( _var );
		}
		std::uint64_t to_uint() const
		{
			return std::get<std::uint64_t>( _var );
		}
		double to_float() const
		{
			return std::get<double>( _var );
		}
		const string_type & to_string() const
		{
			return std::get<string_type>( _var );
		}
		const array_type & to_array() const
		{
			return *std::get<shared_array_type>( _var );
		}
		const object_type & to_object() const
		{
			return *std::get<shared_object_type>( _var );
		}
		template<typename U> U value( const U & defult = {} ) const
		{
			if constexpr ( std::is_same_v<U, bool> )
			{
				if ( auto v = std::get_if<bool>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, std::int8_t> )
			{
				if ( auto v = std::get_if<std::int64_t>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, std::int16_t> )
			{
				if ( auto v = std::get_if<std::int64_t>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, std::int32_t> )
			{
				if ( auto v = std::get_if<std::int64_t>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, std::int64_t> )
			{
				if ( auto v = std::get_if<std::int64_t>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, std::uint8_t> )
			{
				if ( auto v = std::get_if<std::uint64_t>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, std::uint16_t> )
			{
				if ( auto v = std::get_if<std::uint64_t>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, std::uint32_t> )
			{
				if ( auto v = std::get_if<std::uint64_t>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, std::uint64_t> )
			{
				if ( auto v = std::get_if<std::uint64_t>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, float> )
			{
				if ( auto v = std::get_if<double>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, double> )
			{
				if ( auto v = std::get_if<double>( &_var ) != nullptr )
					return *v;
			}
			else if constexpr ( std::is_same_v<U, string_type> )
			{
				if ( auto v = std::get_if<string_type>( &_var ) != nullptr )
					return *v;
			}

			return defult;
		}

	public:
		bool operator ==( std::nullptr_t ) const
		{
			return std::get_if<std::nullptr_t>( &_var ) != nullptr;
		}
		bool operator !=( std::nullptr_t ) const
		{
			return std::get_if<std::nullptr_t>( &_var ) == nullptr;
		}
		operator bool() const
		{
			return std::get<bool>( _var );
		}
		operator std::int8_t() const
		{
			return static_cast<std::int8_t>( std::get<std::int64_t>( _var ) );
		}
		operator std::int16_t() const
		{
			return static_cast<std::int16_t>( std::get<std::int64_t>( _var ) );
		}
		operator std::int32_t() const
		{
			return static_cast<std::int32_t>( std::get<std::int64_t>( _var ) );
		}
		operator std::int64_t() const
		{
			return static_cast<std::int64_t>( std::get<std::int64_t>( _var ) );
		}
		operator std::uint8_t() const
		{
			return static_cast<std::uint8_t>( std::get<std::uint64_t>( _var ) );
		}
		operator std::uint16_t() const
		{
			return static_cast<std::uint16_t>( std::get<std::uint64_t>( _var ) );
		}
		operator std::uint32_t() const
		{
			return static_cast<std::uint32_t>( std::get<std::uint64_t>( _var ) );
		}
		operator std::uint64_t() const
		{
			return static_cast<std::uint64_t>( std::get<std::uint64_t>( _var ) );
		}
		operator float() const
		{
			return static_cast<float>( std::get<double>( _var ) );
		}
		operator double() const
		{
			return static_cast<double>( std::get<double>( _var ) );
		}
		operator string_type() const
		{
			return std::get<string_type>( _var );
		}
		template<typename U, std::enable_if_t<std::is_integral_v<U>, int> = 0> json_impl operator[]( U idx ) const
		{
			if ( empty() )
				_var = std::make_shared<array_type>();

			return to_array()[idx];
		}
		template<typename U, std::enable_if_t<!std::is_integral_v<U>, int> = 0> json_impl & operator[]( U key )
		{
			if ( empty() )
				_var = std::make_shared<object_type>();

			auto obj = std::get<shared_object_type>( _var );

			auto it = std::find_if( obj->begin(), obj->end(), [&key]( const auto & val ) { return val.first == key; } );
			if ( it != obj->end() )
				return it->second;


			obj->push_back( { key, {} } );
			return obj->back().second;
		}
		template<typename U, std::enable_if_t<!std::is_integral_v<U>, int> = 0> json_impl operator[]( U key ) const
		{
			auto obj = std::get<shared_object_type>( _var );

			auto it = std::find_if( obj->begin(), obj->end(), [&key]( const auto & val ) { return val.first == key; } );
			if ( it != obj->end() )
				return it->second;
			return {};
		}

	public:
		void clear()
		{
			_var = std::monostate();
		}
		void push_back( json_impl<_Elem, _Traits, _Alloc> val )
		{
			if ( empty() )
				_var = std::make_shared<array_type>();

			std::get<shared_array_type>( _var )->push_back( val );
		}
		void insert( string_view_type key, json_impl<_Elem, _Traits, _Alloc> val )
		{
			if ( empty() )
				_var = std::make_shared<object_type>();

			std::get<shared_object_type>( _var )->push_back( { string_type( key.begin(), key.end() ), val } );
		}

	public:
		static json_impl load( istream_type & is )
		{
			auto beg = is.tellg();
			is.seekg( 0, std::ios::end );
			auto end = is.tellg();
			is.seekg( beg );

			auto data = string_type( end - beg, 0 );
			is.read( data.data(), end - beg );

			variant_type var;
			unserialization( data.begin(), data.end(), var );
			return var;
		}
		static json_impl load( string_view_type str )
		{
			auto data = string_type( str.begin(), str.end() );

			variant_type var;
			unserialization( data.begin(), data.end(), var );
			return var;
		}
		string_type save( bool compact = false ) const
		{
			osstream_type os;

			serialization( os, _var, 1, compact );

			return os.str();
		}
		ostream_type & save( ostream_type & os, bool compact = false ) const
		{
			serialization( os, _var, 1, compact );

			return os;
		}

	private:
		struct __tab
		{
			friend ostream_type & operator<<( ostream_type & _Ostr, const __tab & val )
			{
				if ( !val.compact )
				{
					for ( size_t i = 0; i < val.layer; i++ )
					{
						_Ostr << "    ";
					}
				}

				return _Ostr;
			}

			int layer;
			bool compact;
		};
		struct __endl
		{
			friend ostream_type & operator<<( ostream_type & _Ostr, const __endl & val )
			{
				_Ostr << ( val.compact ? "" : "\n" );

				return _Ostr;
			}

			bool compact;
		};
		struct __save
		{
			friend ostream_type & operator<<( ostream_type & _Ostr, const __save & val )
			{
				json_impl::serialization( _Ostr, val.var, val.layer, val.compact );

				return _Ostr;
			}

			int layer;
			bool compact;
			const variant_type & var;
		};
		struct __saveop
		{
			__saveop( ostream_type & _os, int layer, bool compact )
				: os( _os ), layer( layer ), compact( compact )
			{
			}

			void operator()( std::monostate )
			{

			}
			void operator()( std::nullptr_t )
			{
				os << "null";
			}
			void operator()( bool val )
			{
				os << ( val ? "true" : "false" );
			}
			void operator()( std::int64_t val )
			{
				if constexpr ( std::is_same_v<char_type, wchar_t> )
					os << std::to_wstring( val );
				else
					os << std::to_string( val );
			}
			void operator()( std::uint64_t val )
			{
				if constexpr ( std::is_same_v<char_type, wchar_t> )
					os << std::to_wstring( val );
				else
					os << std::to_string( val );
			}
			void operator()( double val )
			{
				os << std::to_string( val );
			}
			void operator()( json_impl::string_type val )
			{
				os << constexpr_flags_type::string_flag << val << constexpr_flags_type::string_flag;
			}
			void operator()( const json_impl::shared_array_type & val )
			{
				os << constexpr_flags_type::array_beg_flag << endl();

				for ( size_t i = 0; i < val->size(); i++ )
				{
					os << tab() << save( val->at( i ) );

					if ( i < val->size() - 1 ) os << constexpr_flags_type::comma_flag;

					os << endl();
				}

				os << tab(-1) << constexpr_flags_type::array_end_flag;
			}
			void operator()( const json_impl::shared_object_type & val )
			{
				os << constexpr_flags_type::object_beg_flag << endl();

				for ( auto it = val->begin(); it != val->end(); )
				{
					os << tab() << constexpr_flags_type::string_flag << it->first << constexpr_flags_type::string_flag;
					
					os << constexpr_flags_type::pair_flag << constexpr_flags_type::space_flag;
					
					os << save( it->second );

					if ( ++it != val->end() ) os << constexpr_flags_type::comma_flag;

					os << endl();
				}

				os << tab( -1 ) << constexpr_flags_type::object_end_flag;
			}

			__tab tab(int off = 0)
			{
				return { layer + off, compact };
			}
			__endl endl()
			{
				return { compact };
			}
			__save save( const json_impl & val )
			{
				return { layer + 1, compact, val._var };
			}

			int layer;
			bool compact;
			ostream_type & os;
		};
		struct __loadop
		{
			static string_type::const_iterator space( string_type::const_iterator it )
			{
				while ( std::isspace( *it ) ) ++it;
				return it;
			}
			static bool skip( string_type::const_iterator it, char_type c )
			{
				it = space( it );

				if ( *it == c )
				{
					++it;
					return true;
				}

				return false;
			}
			static bool compare( string_type::const_iterator it, char_type c )
			{
				return ( *space( it ) == c );
			}
			static string_type::const_iterator confirm( string_type::const_iterator it, char_type c )
			{
				it = space( it );

				if ( *it == c )
					return ++it;

				throw "";
			}
		};

	public:
		static string_type::const_iterator unserialization( string_type::const_iterator it, string_type::const_iterator end, variant_type & var )
		{
			if ( it == end )
				return end;

			it = __loadop::space( it );

			if ( *it == constexpr_flags_type::null_flag )
			{
				it += 4;
				var = nullptr;
			}
			else if ( *it == constexpr_flags_type::true_flag )
			{
				it += 4;
				var = true;
			}
			else if ( *it == constexpr_flags_type::false_flag )
			{
				it += 5;
				var = false;
			}
			else if ( *it == constexpr_flags_type::rev_flag )
			{
				auto beg = it++;
				while ( std::isdigit( *it ) || *it == constexpr_flags_type::float_flag ) ++it;
				auto end = it;

				if ( std::find( beg, end, constexpr_flags_type::float_flag ) != end )
				{
					double val;
					std::from_chars( beg.operator->(), end.operator->(), val );
					var = val;
				}
				else
				{
					std::int64_t val;
					std::from_chars( beg.operator->(), end.operator->(), val );
					var = val;
				}
			}
			else if ( std::isdigit( *it ) )
			{
				auto beg = it;
				while ( std::isdigit( *it ) || *it == constexpr_flags_type::float_flag ) ++it;
				auto end = it;

				if ( std::find( beg, end, constexpr_flags_type::float_flag ) != end )
				{
					double val;
					std::from_chars( beg.operator->(), end.operator->(), val );
					var = val;
				}
				else
				{
					std::uint64_t val;
					std::from_chars( beg.operator->(), end.operator->(), val );
					var = val;
				}
			}
			else if ( *it == constexpr_flags_type::string_flag )
			{
				it = __loadop::confirm( it, constexpr_flags_type::string_flag );

				auto beg = it;
				while ( 1 )
				{
					if ( *it == constexpr_flags_type::string_flag )
						break;
					else if ( *it == '\\' )
						++it;

					++it;
				}
				auto end = it;

				it = __loadop::confirm( it, constexpr_flags_type::string_flag );

				var = string_type( beg, end );
			}
			else if ( *it == constexpr_flags_type::array_beg_flag )
			{
				it = __loadop::confirm( it, constexpr_flags_type::array_beg_flag );

				auto array = std::make_shared<array_type>();

				do
				{
					variant_type element;
					it = unserialization( it, end, element );
					array->push_back( element );

					if ( __loadop::skip( it, constexpr_flags_type::comma_flag ) )
						it = __loadop::confirm( it, constexpr_flags_type::comma_flag );
					else
						break;

				} while ( !__loadop::compare( it, constexpr_flags_type::array_end_flag ) );

				it = __loadop::confirm( it, constexpr_flags_type::array_end_flag );

				var = array;
			}
			else if ( *it == constexpr_flags_type::object_beg_flag )
			{
				it = __loadop::confirm( it, constexpr_flags_type::object_beg_flag );

				auto object = std::make_shared<object_type>();

				do
				{
					variant_type key, value;

					it = unserialization( it, end, key );

					it = __loadop::confirm( it, constexpr_flags_type::pair_flag );

					it = unserialization( it, end, value );

					object->push_back( { std::get<string_type>( key ), x::json_impl( value ) } );

					if ( __loadop::skip( it, constexpr_flags_type::comma_flag ) )
						it = __loadop::confirm( it, constexpr_flags_type::comma_flag );
					else
						break;

				} while ( !__loadop::compare( it, constexpr_flags_type::object_end_flag ) );

				it = __loadop::confirm( it, constexpr_flags_type::object_end_flag );

				var = object;
			}

			return it;
		}
		static ostream_type & serialization( ostream_type & os, const variant_type & val, int layer, bool compact )
		{
			std::visit( __saveop( os, layer, compact ), val );
			return os;
		}
		
	private:
		variant_type _var;
	};

	using json = json_impl<char>;
	using wjson = json_impl<wchar_t>;
	using u8json = json_impl<char8_t>;
	using u16json = json_impl<char16_t>;
	using u32json = json_impl<char32_t>;
}
