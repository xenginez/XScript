#pragma once

#include "visitor.h"

namespace x
{
	class code_generater : public x::scope_scan_visitor
	{
	public:
		using scope_scan_visitor::visit;

	public:
		void generate( const x::module_ptr & module, const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast );

	private:
		x::module_ptr _module;
	};
}