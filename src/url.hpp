#pragma once

#include <string>

namespace x
{
	template<class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem>> class url_impl : public std::basic_string<_Elem, _Traits, _Alloc>
	{
	public:
		using char_type = _Elem;
		using string_type = std::basic_string<_Elem, _Traits, _Alloc>;
		using string_view_type = std::basic_string_view<_Elem, _Traits>;
		using iterator = string_type::iterator;
		using const_iterator = string_type::const_iterator;

	private:
		template<typename T> struct constexpr_flags;
		template<> struct constexpr_flags<char>
		{
			static constexpr const char * scheme_flag = "://";
			static constexpr const char * username_flag = "@";
			static constexpr const char * password_flag = ":";
			static constexpr const char * host_flag = "/";
			static constexpr const char * port_flag = ":";
			static constexpr const char * path_flag = "?";
			static constexpr const char * fragment_flag = "#";
			static constexpr const char query_flag = '=';
			static constexpr const char query_pair_flag = '&';
		};
		template<> struct constexpr_flags<wchar_t>
		{
			static constexpr const wchar_t * scheme_flag = L"://";
			static constexpr const wchar_t * username_flag = L"@";
			static constexpr const wchar_t * password_flag = L":";
			static constexpr const wchar_t * host_flag = L"/";
			static constexpr const wchar_t * port_flag = L":";
			static constexpr const wchar_t * path_flag = L"?";
			static constexpr const wchar_t * fragment_flag = L"#";
			static constexpr const wchar_t query_flag = L'=';
			static constexpr const wchar_t query_pair_flag = L'&';
		};
		template<> struct constexpr_flags<char8_t>
		{
			static constexpr const char8_t * scheme_flag = u8"://";
			static constexpr const char8_t * username_flag = u8"@";
			static constexpr const char8_t * password_flag = u8":";
			static constexpr const char8_t * host_flag = u8"/";
			static constexpr const char8_t * port_flag = u8":";
			static constexpr const char8_t * path_flag = u8"?";
			static constexpr const char8_t * fragment_flag = u8"#";
			static constexpr const char8_t query_flag = u8'=';
			static constexpr const char8_t query_pair_flag = u8'&';
		};
		template<> struct constexpr_flags<char16_t>
		{
			static constexpr const char16_t * scheme_flag = u"://";
			static constexpr const char16_t * username_flag = u"@";
			static constexpr const char16_t * password_flag = u":";
			static constexpr const char16_t * host_flag = u"/";
			static constexpr const char16_t * port_flag = u":";
			static constexpr const char16_t * path_flag = u"?";
			static constexpr const char16_t * fragment_flag = u"#";
			static constexpr const char16_t query_flag = u'=';
			static constexpr const char16_t query_pair_flag = u'&';
		};
		template<> struct constexpr_flags<char32_t>
		{
			static constexpr const char32_t * scheme_flag = U"://";
			static constexpr const char32_t * username_flag = U"@";
			static constexpr const char32_t * password_flag = U":";
			static constexpr const char32_t * host_flag = U"/";
			static constexpr const char32_t * port_flag = U":";
			static constexpr const char32_t * path_flag = U"?";
			static constexpr const char32_t * fragment_flag = U"#";
			static constexpr const char32_t query_flag = U'=';
			static constexpr const char32_t query_pair_flag = U'&';
		};
		using constexpr_flags_type = constexpr_flags<char_type>;

	public:
		url_impl() = default;
		url_impl( url_impl && val )
			: string_type( val )
		{
			parse();
		}
		url_impl( const url_impl && val )
			: string_type( val )
		{
			parse();
		}
		url_impl( string_type && val )
			: string_type( val )
		{
			parse();
		}
		url_impl( const string_type & val )
			: string_type( val )
		{
			parse();
		}

	public:
		url_impl & operator=( url_impl && val )
		{
			string_type::operator=( val );

			parse();

			return *this;
		}
		url_impl & operator=( const url_impl & val )
		{
			string_type::operator=( val );

			parse();

			return *this;
		}
		url_impl & operator=( string_type && val )
		{
			string_type::operator=( val );

			parse();

			return *this;
		}
		url_impl & operator=( const string_type & val )
		{
			string_type::operator=( val );

			parse();

			return *this;
		}

	public:
		string_view_type string_view() const
		{
			return { string_type::begin(), string_type::end() };
		}

	public:
		string_view_type scheme() const
		{
			return _scheme;
		}
		string_view_type username() const
		{
			return _username;
		}
		string_view_type password() const
		{
			return _password;
		}
		string_view_type host() const
		{
			return _host;
		}
		string_view_type port() const
		{
			return _port;
		}
		string_view_type path() const
		{
			return _path;
		}
		string_view_type querys() const
		{
			return _query;
		}
		string_view_type fragment() const
		{
			return _fragment;
		}

	public:
		std::size_t query_count() const
		{
			return std::count( _query.begin(), _query.end(), constexpr_flags_type::query_flag );
		}
		string_view_type query_at( std::size_t idx ) const
		{
			auto beg = _query.begin();
			auto end = std::find( beg, _query.end(), constexpr_flags_type::query_pair_flag );

			auto count = query_count();
			for ( std::size_t i = 0; i < idx && i < count; i++ )
			{
				beg = end + 1;
				end = std::find( beg, _query.end(), constexpr_flags_type::query_pair_flag );
			}

			return { beg, end };
		}

	private:
		const_iterator find_it( string_view_type substr, const_iterator offset ) const
		{
			auto idx = string_type::find( substr, std::distance( string_type::begin(), offset ) );
			if ( idx == string_type::npos )
				return string_type::end();
			return string_type::begin() + idx;
		}
		const_iterator find_it( string_view_type substr, const_iterator beg, const_iterator end ) const
		{
			string_view_type tmp{ beg, end };

			auto idx = tmp.find( substr );
			if ( idx == string_type::npos )
				return end;

			return beg + idx;
		}

	private:
		void parse()
		{
			// {scheme}://{username}:{password}@{host}:{port}/{path}?{query}#{fragment}

			const_iterator ibeg = string_type::begin();
			const_iterator iend = string_type::end();

			iend = find_it( constexpr_flags_type::scheme_flag, ibeg );
			if ( iend != string_type::end() )
			{
				_scheme = { ibeg, iend };
				iend += 3;
			}
			else
			{
				_scheme = { ibeg, iend };
				return;
			}

			ibeg = iend;
			iend = find_it( constexpr_flags_type::host_flag, ibeg );
			if ( iend != string_type::end() )
			{
				auto im_end = find_it( constexpr_flags_type::username_flag, ibeg, iend );
				if ( im_end != iend )
				{
					auto imm_end = im_end;

					imm_end = find_it( constexpr_flags_type::port_flag, ibeg, im_end );
					if ( imm_end != im_end )
					{
						_username = { ibeg, imm_end };
						_password = { imm_end + 1, im_end };
					}
					else
					{
						_username = { ibeg, imm_end };
					}

					ibeg = im_end + 1;
				}

				im_end = find_it( constexpr_flags_type::port_flag, ibeg, iend );
				if ( im_end != iend )
				{
					_host = { ibeg, im_end };
					_port = { im_end + 1, iend };
				}
				else
				{
					_host = { ibeg, iend };
				}

				iend += 1;
			}
			else
			{
				_host = { ibeg, iend };
				return;
			}

			ibeg = iend;
			iend = find_it( constexpr_flags_type::path_flag, ibeg );
			if ( iend != string_type::end() )
			{
				_path = { ibeg, iend };
				iend += 1;
			}
			else
			{
				_path = { ibeg, iend };
				return;
			}

			ibeg = iend;
			iend = find_it( constexpr_flags_type::fragment_flag, ibeg );
			if ( iend != string_type::end() )
			{
				_query = { ibeg, iend };
				iend += 1;
			}
			else
			{
				_query = { ibeg, iend };
				return;
			}

			ibeg = iend;
			_fragment = { ibeg, string_type::end() };
		}

	private:
		string_view_type _scheme;
		string_view_type _username;
		string_view_type _password;
		string_view_type _host;
		string_view_type _port;
		string_view_type _path;
		string_view_type _query;
		string_view_type _fragment;
	};

	using url = url_impl<char>;
	using wurl = url_impl<wchar_t>;
	using u8url = url_impl<char8_t>;
	using u16url = url_impl<char16_t>;
	using u32url = url_impl<char32_t>;
}
