#include "pass.h"

#include "context.h"
#include "symbols.h"

void x::pass::set_ctx( x::context * ctx )
{
	_ctx = ctx;
}

x::context * x::pass::context() const
{
	return _ctx;
}

x::symbols * x::pass::symbols() const
{
	return _ctx->symbols().get();
}

void x::scanner_pass::visit( x::unit_ast * val )
{
	symbols()->push_unit( val->location );
	ast_visitor::visit( val );
	symbols()->pop_unit();
}

void x::scanner_pass::visit( x::enum_decl_ast * val )
{
	symbols()->push_scope( val->location );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scanner_pass::visit( x::class_decl_ast * val )
{
	symbols()->push_scope( val->location );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scanner_pass::visit( x::namespace_decl_ast * val )
{
	symbols()->push_scope( val->location );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scanner_pass::visit( x::compound_stat_ast * val )
{
	symbols()->push_scope( val->location );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::scanner_pass::visit( x::closure_exp_ast * val )
{
	symbols()->push_scope( val->location );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::type_scanner_pass::visit( x::enum_decl_ast * val )
{
	auto symbol = std::make_shared<x::symbols::enum_symbol>();

	symbol->name = val->name;
	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}

void x::type_scanner_pass::visit( x::class_decl_ast * val )
{
	auto symbol = std::make_shared<x::symbols::class_symbol>();

	symbol->name = val->name;
	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}

void x::type_scanner_pass::visit( x::using_decl_ast * val )
{
	auto symbol = std::make_shared<x::symbols::alias_symbol>();

	symbol->name = val->name;
	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}

void x::type_scanner_pass::visit( x::template_decl_ast * val )
{
	auto symbol = std::make_shared<x::symbols::template_symbol>();

	symbol->name = val->name;
	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}

void x::type_scanner_pass::visit( x::namespace_decl_ast * val )
{
	auto symbol = std::make_shared<x::symbols::namespace_symbol>();

	symbol->name = val->name;
	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}

void x::function_scanner_pass::visit( x::function_decl_ast * val )
{
	auto symbol = std::make_shared<x::symbols::function_symbol>();

	symbol->name = val->name;
	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}

void x::variable_scanner_pass::visit( x::enum_element_ast * val )
{
	auto symbol = std::make_shared<x::symbols::variable_symbol>();

	symbol->name = val->name;
	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}

void x::variable_scanner_pass::visit( x::variable_decl_ast * val )
{
	auto symbol = std::make_shared<x::symbols::variable_symbol>();

	symbol->name = val->name;
	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}

void x::variable_scanner_pass::visit( x::parameter_decl_ast * val )
{
	auto symbol = std::make_shared<x::symbols::variable_symbol>();

	symbol->name = val->name;
	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}

void x::variable_scanner_pass::visit( x::local_stat_ast * val )
{
	auto symbol = std::make_shared<x::symbols::variable_symbol>();

	symbol->name = val->name;
	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}

void x::scope_scanner_pass::visit( x::compound_stat_ast * val )
{
	auto symbol = std::make_shared<x::symbols::block_symbol>();

	symbol->location = val->location;

	symbols()->add_symbol( symbol );

	scanner_pass::visit( val );
}
