#pragma once

#include "meta.h"

namespace x
{
	class context : public std::enable_shared_from_this<context>
	{
	public:
		friend class compiler;

	public:
		context();
		~context();

	public:
		int version() const;
		x::meta_ptr find_meta( x::uint64 hashcode ) const;
		x::meta_ptr find_meta( std::string_view fullname ) const;

	public:
		void load( std::istream & in );
		void save( std::ostream & out ) const;

	private:
		int _version = 0;
		std::string _strpool;
		std::map<x::uint64, meta_ptr> _metas;
	};
}
