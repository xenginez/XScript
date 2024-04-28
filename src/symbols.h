#pragma once

#include <map>
#include <span>
#include <deque>

#include "symbol.h"

namespace x
{
	class symbols : public std::enable_shared_from_this<symbols>
	{
	public:
		symbols();
		~symbols();

	public:
		void push_unit( const x::source_location & location );
		void pop_unit();

		void push_scope( const x::source_location & location );
		void add_symbol( const x::symbol_ptr & val );
		bool has_symbol( std::string_view symbol_name ) const;
		x::symbol_ptr current_scope() const;
		void pop_scope();

	public:
		x::namespace_symbol_ptr global_namespace() const;
		x::symbol_ptr find_symbol_from_name( std::string_view symbol_name ) const;
		x::symbol_ptr find_symbol_from_fullname( std::string_view symbol_fullname ) const;

	public:
		void add_reference( const x::source_location & location, const x::symbol_ptr & val );
		x::symbol_ptr find_reference( const x::source_location & location ) const;

	private:
		x::namespace_symbol_ptr _global;
		std::map<std::string, symbols_ptr> _symbolmap;
		std::map<std::string, symbols_ptr> _referencemap;
	};
}