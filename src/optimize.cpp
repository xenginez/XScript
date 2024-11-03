#include "optimize.h"

#define REG_OPTIMIZER( TYPE ) static x::code_optimize_visitor::optimizer::register_optimizer<x::##TYPE> __reg_##TYPE = {}

namespace
{
	REG_OPTIMIZER( translate_lambda_optimizer );
}

std::vector<x::code_optimize_visitor::optimizer *> & x::code_optimize_visitor::optimizer::optimizers()
{
	static std::vector<x::code_optimize_visitor::optimizer *> _optimizers = {};
	return _optimizers;
}

void x::code_optimize_visitor::optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast, x::uint32 level )
{
	scope_scanner_visitor::scanning( logger, symbols, ast );
}
