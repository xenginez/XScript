#pragma once

#include "ast.h"

namespace x
{
	class pass : public ast_visitor
	{
		friend class context;

	public:
		using ast_visitor::visit;

	public:
		void visit( x::unit_ast * val ) override;
		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;
		void visit( x::compound_stat_ast * val ) override;
		void visit( x::closure_exp_ast * val ) override;

	private:
		void set_ctx( x::context * ctx );

	protected:
		context * context() const;
		symbols * symbols() const;

	private:
		x::context * _ctx;
	};

	class type_scanner_pass : public pass
	{
	public:
		using pass::visit;

	public:
		type_scanner_pass() = default;

	public:
		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::using_decl_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;
	};

	class function_scanner_pass : public pass
	{
	public:
		using pass::visit;

	public:
		void visit( x::function_decl_ast * val ) override;
	};

	class variable_scanner_pass : public pass
	{
	public:
		using pass::visit;

	public:
		void visit( x::enum_element_ast * val ) override;
		void visit( x::variable_decl_ast * val ) override;
		void visit( x::parameter_decl_ast * val ) override;
		void visit( x::local_stat_ast * val ) override;
	};

	class scope_scanner_pass : public pass
	{
	public:
		using pass::visit;

	public:
		void visit( x::compound_stat_ast * val ) override;
	};

	class reference_solver_pass : public pass
	{
	public:
		using pass::visit;

	public:

	};

	class type_checker_pass : public pass
	{
	public:
		using pass::visit;

	public:

	};

	class semantic_validator_pass : public pass
	{
	public:
		using pass::visit;

	public:

	};
}
