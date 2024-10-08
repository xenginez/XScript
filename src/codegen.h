#pragma once

#include "visitor.h"

namespace x
{
	class module_generater : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		void generate( const x::module_ptr & module, const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast );

	private:
		x::module_ptr _module;
	};

	class llvm_module_generater
	{
	public:
		void generate( const llvm::module_ptr & llvm_module, const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::module_ptr & module );

	private:
		llvm::module_ptr _module;
	};

	class spirv_module_generater
	{
	public:
		void generate( const spirv::module_ptr & spirv_module, const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::module_ptr & module );

	private:
		spirv::module_ptr _module;
	};
}