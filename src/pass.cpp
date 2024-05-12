#include "pass.h"

#include "symbols.h"

x::pass::pass( const x::symbols_ptr & val )
	: _symbols( val )
{
}

const x::symbols_ptr & x::pass::symbols() const
{
	return _symbols;
}


x::symbol_scanner_pass::symbol_scanner_pass( const x::symbols_ptr & val )
	: pass( val )
{
}

x::reference_solver_pass::reference_solver_pass( const x::symbols_ptr & val )
	: pass( val )
{
}

x::type_checker_pass::type_checker_pass( const x::symbols_ptr & val )
	: pass( val )
{
}

x::semantic_checker_pass::semantic_checker_pass( const x::symbols_ptr & val )
	: pass( val )
{
}
