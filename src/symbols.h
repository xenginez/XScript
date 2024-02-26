#pragma once

#include <map>

#include "type.h"

namespace x
{
	class symbols : public std::enable_shared_from_this<symbols>
	{
	public:
		symbols();
		~symbols();

	public:
		void beg_unit();
		void end_unit();

	public:
		void push_scope( x::symbol_t type, std::string_view name, x::ast * ast );
		void add_symbol( x::symbol_t type, std::string_view name, x::ast * ast );
		void pop_scope();

	public:
		x::scope * global_scope() const;
		bool is_scope( x::symbol * sym ) const;
		x::scope * find_scope( x::symbol_t type ) const;
		x::symbol * find_symbol( std::string_view name, x::scope * scope = nullptr ) const;

	public:
		x::symbol * find_reference( x::ast * ast ) const;
		void add_reference( x::ast * ast, x::symbol * sym );

	private:
		x::symbol * find_symbol( x::scope * scope, std::string_view name ) const;

	private:
		x::scope * _cur;
		x::scope * _global;
		std::vector<x::scope> _scopes;
		std::map<x::ast *, x::symbol *> _references;
	};
}