#pragma once

#include "type.h"

namespace x
{
	class context : public std::enable_shared_from_this<context>
	{
		struct private_p;

	public:
		context();
		~context();

	public:
		int version() const;
		bool load( const x::module_ptr & val );

	public:
		const x::meta * find_meta( x::uint64 hashcode ) const;
		const x::meta * find_meta( std::string_view fullname ) const;

	public:
		void * dlsym( std::string_view dllname, std::string_view funcname );
		void dlclose();

	private:
		private_p * _p;
	};
}
