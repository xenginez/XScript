#include "pass.h"

#include "context.h"
#include "symbols.h"

void x::pass::set_ctx( x::context * ctx )
{
	_ctx = ctx;
}

std::string x::pass::exp_type_name( x::exp_stat_ast * ast ) const
{
	return std::string();
}

x::context * x::pass::context() const
{
	return _ctx;
}

x::symbols * x::pass::symbols() const
{
	return _ctx->symbols().get();
}

void x::scope_scanner_pass::visit( x::unit_ast * val )
{
	symbols()->beg_unit();
	ast_visitor::visit( val );
	symbols()->end_unit();
}

void x::scope_scanner_pass::visit( x::enum_decl_ast * val )
{
	symbols()->push_scope( symbol_t::ENUM, val->name, val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scope_scanner_pass::visit( x::class_decl_ast * val )
{
	symbols()->push_scope( symbol_t::CLASS, val->name, val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scope_scanner_pass::visit( x::using_decl_ast * val )
{
	symbols()->add_symbol( symbol_t::ALIAS, val->name, val );
	ast_visitor::visit( val );
}

void x::scope_scanner_pass::visit( x::enum_element_ast * val )
{
	symbols()->add_symbol( symbol_t::ELEMENT, val->name, val );
	ast_visitor::visit( val );
}

void x::scope_scanner_pass::visit( x::template_decl_ast * val )
{
	symbols()->add_symbol( symbol_t::TEMPLATE, val->name, val );
	ast_visitor::visit( val );
}

void x::scope_scanner_pass::visit( x::variable_decl_ast * val )
{
	symbols()->add_symbol( symbol_t::VARIABLE, val->name, val );
	ast_visitor::visit( val );
}

void x::scope_scanner_pass::visit( x::function_decl_ast * val )
{
	symbols()->push_scope( symbol_t::FUNCTION, val->name, val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scope_scanner_pass::visit( x::parameter_decl_ast * val )
{
	symbols()->add_symbol( symbol_t::PARAM, val->name, val );
	ast_visitor::visit( val );
}

void x::scope_scanner_pass::visit( x::namespace_decl_ast * val )
{
	symbols()->push_scope( symbol_t::NAMESPACE, val->name, val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scope_scanner_pass::visit( x::compound_stat_ast * val )
{
	symbols()->push_scope( symbol_t::BLOCK, "block_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scope_scanner_pass::visit( x::try_stat_ast * val )
{
	symbols()->push_scope( symbol_t::BLOCK, "block_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scope_scanner_pass::visit( x::catch_stat_ast * val )
{
	symbols()->push_scope( symbol_t::BLOCK, "block_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scope_scanner_pass::visit( x::while_stat_ast * val )
{
	symbols()->push_scope( symbol_t::LOOP, "loop_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scope_scanner_pass::visit( x::for_stat_ast * val )
{
	symbols()->push_scope( symbol_t::LOOP, "loop_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scope_scanner_pass::visit( x::foreach_stat_ast * val )
{
	symbols()->push_scope( symbol_t::LOOP, "loop_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scope_scanner_pass::visit( x::local_stat_ast * val )
{
	symbols()->add_symbol( symbol_t::LOCAL, val->name, val );
	ast_visitor::visit( val );
}

void x::scope_scanner_pass::visit( x::closure_exp_ast * val )
{
	symbols()->push_scope( symbol_t::CLASS, val->name, val );
	{
		for ( const auto & it : val->captures )
			symbols()->add_symbol( symbol_t::VARIABLE, it->ident, it.get() );

		ast_visitor::visit( val );
	}
	symbols()->pop_scope();
}
