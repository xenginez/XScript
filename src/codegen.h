#pragma once

#include "visitor.h"

namespace x
{
	class module_scanner_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		void scanner( const x::module_ptr & module, const x::symbols_ptr & symbols, x::ast_ptr ast );

	private:
		x::module_ptr _module;
	};

	class module_generater_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		void generate( const x::module_ptr & module, const x::symbols_ptr & symbols, x::ast_ptr ast );

	private:
		x::module_ptr _module;
	};

	class llvm_scanner_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		void scanner( const llvm::module_ptr & module, const x::symbols_ptr & symbols, x::ast_ptr ast );

	private:
		llvm::module_ptr _module;
	};

	class llvm_generater_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		void generate( const llvm::module_ptr & module, const x::symbols_ptr & symbols, x::ast_ptr ast );

	private:
		llvm::module_ptr _module;
	};

	class spirv_scanner_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		void scanner( const spirv::module_ptr & module, const x::symbols_ptr & symbols, x::ast_ptr ast );

	private:
		spirv::module_ptr _module;
	};

	class spirv_generater_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		void generate( const spirv::module_ptr & module, const x::symbols_ptr & symbols, x::ast_ptr ast );

	private:
		spirv::module_ptr _module;
	};
}