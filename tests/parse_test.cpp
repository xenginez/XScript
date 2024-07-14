#include <fstream>
#include <iostream>
#include <filesystem>

#include <grammar.h>
#include <visitor.h>

class ast_print_visitor : public x::visitor
{
public:
	void visit( x::unit_ast * val ) override
	{
		for ( auto it : val->imports )
		{
			std::cout << std::format( R"(import( "{}" ))", it->path ) << std::endl;
		}
	}
	void visit( x::import_ast * val ) override {}
	void visit( x::attribute_ast * val ) override {}

	void visit( x::type_ast * val ) override {}
	void visit( x::func_type_ast * val ) override {}
	void visit( x::temp_type_ast * val ) override {}
	void visit( x::list_type_ast * val ) override {}
	void visit( x::array_type_ast * val ) override {}

	void visit( x::enum_decl_ast * val ) override {}
	void visit( x::class_decl_ast * val ) override {}
	void visit( x::using_decl_ast * val ) override {}
	void visit( x::element_decl_ast * val ) override {}
	void visit( x::template_decl_ast * val ) override {}
	void visit( x::variable_decl_ast * val ) override {}
	void visit( x::function_decl_ast * val ) override {}
	void visit( x::parameter_decl_ast * val ) override {}
	void visit( x::namespace_decl_ast * val ) override {}

	void visit( x::empty_stat_ast * val ) override {}
	void visit( x::extern_stat_ast * val ) override {}
	void visit( x::compound_stat_ast * val ) override {}
	void visit( x::await_stat_ast * val ) override {}
	void visit( x::yield_stat_ast * val ) override {}
	void visit( x::new_stat_ast * val ) override {}
	void visit( x::if_stat_ast * val ) override {}
	void visit( x::while_stat_ast * val ) override {}
	void visit( x::for_stat_ast * val ) override {}
	void visit( x::foreach_stat_ast * val ) override {}
	void visit( x::switch_stat_ast * val ) override {}
	void visit( x::break_stat_ast * val ) override {}
	void visit( x::return_stat_ast * val ) override {}
	void visit( x::continue_stat_ast * val ) override {}
	void visit( x::local_stat_ast * val ) override {}

	void visit( x::assignment_expr_ast * val ) override {}
	void visit( x::logical_or_expr_ast * val ) override {}
	void visit( x::logical_and_expr_ast * val ) override {}
	void visit( x::or_expr_ast * val ) override {}
	void visit( x::xor_expr_ast * val ) override {}
	void visit( x::and_expr_ast * val ) override {}
	void visit( x::compare_expr_ast * val ) override {}
	void visit( x::shift_expr_ast * val ) override {}
	void visit( x::add_expr_ast * val ) override {}
	void visit( x::mul_expr_ast * val ) override {}
	void visit( x::as_expr_ast * val ) override {}
	void visit( x::is_expr_ast * val ) override {}
	void visit( x::sizeof_expr_ast * val ) override {}
	void visit( x::typeof_expr_ast * val ) override {}
	void visit( x::unary_expr_ast * val ) override {}
	void visit( x::postfix_expr_ast * val ) override {}
	void visit( x::invoke_expr_ast * val ) override {}
	void visit( x::member_expr_ast * val ) override {}
	void visit( x::typecast_expr_ast * val ) override {}
	void visit( x::identifier_expr_ast * val ) override {}
	void visit( x::closure_expr_ast * val ) override {}
	void visit( x::arguments_expr_ast * val ) override {}
	void visit( x::initializers_expr_ast * val ) override {}
	void visit( x::null_const_expr_ast * val ) override {}
	void visit( x::bool_const_expr_ast * val ) override {}
	void visit( x::int_const_expr_ast * val ) override {}
	void visit( x::float_const_expr_ast * val ) override {}
	void visit( x::string_const_expr_ast * val ) override {}
};

int main()
{
	auto path = std::filesystem::current_path() / ".." / "script_0.xs";

	std::ifstream ifs( path );
	if ( ifs.is_open() )
	{
		x::grammar grammar;
		ast_print_visitor visiter;

		auto unit = grammar.parse( ifs, path.string() );

		unit->accept( &visiter );
	}

	return 0;
}