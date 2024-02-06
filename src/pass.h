#pragma once

#include "ast.h"

namespace x
{
	class pass : public ast_visitor
	{
	public:
		context * context() const;
		symbols * symbols() const;
		void set_ctx( x::context * ctx );

	protected:
		std::string exp_type_name( x::exp_stat_ast * ast ) const;

	private:
		x::context * _ctx;
	};

	class scope_scanner_pass : public pass
	{
	public:
		void visit( x::unit_ast * val ) override;
		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::using_decl_ast * val ) override;
		void visit( x::enum_element_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;
		void visit( x::variable_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::parameter_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::compound_stat_ast * val ) override;
		void visit( x::try_stat_ast * val ) override;
		void visit( x::catch_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;
		void visit( x::local_stat_ast * val ) override;

		void visit( x::closure_exp_ast * val ) override;
	};

	class reference_resolver_pass : public pass
	{
	public:
		void visit( x::unit_ast * val ) override;
		void visit( x::type_ast * val ) override;

		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::compound_stat_ast * val ) override;
		void visit( x::try_stat_ast * val ) override;
		void visit( x::catch_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;

		void visit( x::break_stat_ast * val ) override;
		void visit( x::return_stat_ast * val ) override;
		void visit( x::continue_stat_ast * val ) override;

		void visit( x::member_exp_ast * val ) override;
		void visit( x::closure_exp_ast * val ) override;
	};

	class type_checker_pass
	{

	};

	class semantic_validator_pass
	{

	};

	class ir_generator_pass
	{

	};
}
