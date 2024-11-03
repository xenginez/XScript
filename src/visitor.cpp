#include "visitor.h"

#include "symbols.h"
#include "exception.h"

void x::visitor::visit( x::unit_ast * val )
{
	for ( const auto & it : val->get_imports() )
		it->accept( this );

	for ( const auto & it : val->get_namespaces() )
		it->accept( this );
}

void x::visitor::visit( x::import_ast * val )
{
}

void x::visitor::visit( x::attribute_ast * val )
{
}

void x::visitor::visit( x::parameter_ast * val )
{
	val->get_valuetype()->accept( this );
	if ( val->get_default() ) val->get_default()->accept( this );
}

void x::visitor::visit( x::type_ast * val )
{
}

void x::visitor::visit( x::func_type_ast * val )
{
	for ( const auto & it : val->get_parameters() )
		it->accept( this );
}

void x::visitor::visit( x::temp_type_ast * val )
{
	for ( const auto & it : val->get_elements() )
		it->accept( this );
}

void x::visitor::visit( x::list_type_ast * val )
{

}

void x::visitor::visit( x::array_type_ast * val )
{
}

void x::visitor::visit( x::enum_decl_ast * val )
{

}

void x::visitor::visit( x::class_decl_ast * val )
{
	if ( val->get_base() )
		val->get_base()->accept( this );

	for ( const auto & it : val->get_usings() )
		it->accept( this );
	for ( const auto & it : val->get_friends() )
		it->accept( this );
	for ( const auto & it : val->get_variables() )
		it->accept( this );
	for ( const auto & it : val->get_functions() )
		it->accept( this );
}

void x::visitor::visit( x::using_decl_ast * val )
{
	val->get_retype()->accept( this );
}

void x::visitor::visit( x::template_decl_ast * val )
{
	if ( val->get_base() )
		val->get_base()->accept( this );

	if ( val->get_where() )
		val->get_where()->accept( this );

	for ( const auto & it : val->get_usings() )
		it->accept( this );
	for ( const auto & it : val->get_usings() )
		it->accept( this );
	for ( const auto & it : val->get_elements() )
		it->accept( this );
	for ( const auto & it : val->get_variables() )
		it->accept( this );
	for ( const auto & it : val->get_functions() )
		it->accept( this );
}

void x::visitor::visit( x::variable_decl_ast * val )
{
	val->get_valuetype()->accept( this );

	if ( val->get_init() )
		val->get_init()->accept( this );
}

void x::visitor::visit( x::function_decl_ast * val )
{
	for ( const auto & it : val->get_parameters() )
		it->accept( this );

	for ( const auto & it : val->get_results() )
		it->accept( this );

	val->get_body()->accept( this );
}

void x::visitor::visit( x::interface_decl_ast * val )
{
	for ( const auto & it : val->get_functions() )
		it->accept( this );
}

void x::visitor::visit( x::namespace_decl_ast * val )
{
	for ( const auto & it : val->get_members() )
		it->accept( this );
}

void x::visitor::visit( x::empty_stat_ast * val )
{
}

void x::visitor::visit( x::extern_stat_ast * val )
{
}

void x::visitor::visit( x::compound_stat_ast * val )
{
	for ( const auto & it : val->get_stats() )
		it->accept( this );
}

void x::visitor::visit( x::await_stat_ast * val )
{
	val->get_exp()->accept( this );
}

void x::visitor::visit( x::yield_stat_ast * val )
{
	val->get_exp()->accept( this );
}

void x::visitor::visit( x::if_stat_ast * val )
{
	val->get_cond()->accept( this );
	val->get_then_stat()->accept( this );
	if ( val->get_else_stat() )
		val->get_else_stat()->accept( this );
}

void x::visitor::visit( x::while_stat_ast * val )
{
	val->get_cond()->accept( this );
	val->get_body()->accept( this );
}

void x::visitor::visit( x::for_stat_ast * val )
{
	val->get_init()->accept( this );
	val->get_cond()->accept( this );
	val->get_body()->accept( this );
	val->get_step()->accept( this );
}

void x::visitor::visit( x::foreach_stat_ast * val )
{
	val->get_item()->accept( this );
	val->get_collection()->accept( this );
	val->get_body()->accept( this );
}

void x::visitor::visit( x::switch_stat_ast * val )
{
	val->get_exp()->accept( this );
	for ( auto & it : val->get_cases() )
	{
		it.first->accept( this );
		it.second->accept( this );
	}
	if ( val->get_defult() )
		val->get_defult()->accept( this );
}

void x::visitor::visit( x::break_stat_ast * val )
{
}

void x::visitor::visit( x::return_stat_ast * val )
{
	for ( auto & it : val->get_exps() )
		it->accept( this );
}

void x::visitor::visit( x::new_stat_ast * val )
{
	val->get_type()->accept( this );

	val->get_init_stat()->accept( this );
}

void x::visitor::visit( x::try_stat_ast * val )
{
	val->get_try_stat()->accept( this );

	for ( auto & it : val->get_catch_stats() )
	{
		it.first->accept( this );
		it.second->accept( this );
	}

	if ( val->get_final_stat() )
		val->get_final_stat()->accept( this );
}

void x::visitor::visit( x::throw_stat_ast * val )
{
	val->get_exception()->accept( this );
}

void x::visitor::visit( x::continue_stat_ast * val )
{
}

void x::visitor::visit( x::local_stat_ast * val )
{
	val->get_valuetype()->accept( this );

	if ( val->get_init() )
		val->get_init()->accept( this );
}

void x::visitor::visit( x::mulocal_stat_ast * val )
{
	for ( const auto & it : val->get_locals() )
		it->accept( this );
	val->get_init()->accept( this );
}

void x::visitor::visit( x::binary_expr_ast * val )
{
	val->get_left()->accept( this );
	val->get_right()->accept( this );
}

void x::visitor::visit( x::unary_expr_ast * val )
{
	val->get_exp()->accept( this );
}

void x::visitor::visit( x::bracket_expr_ast * val )
{
	val->get_exp()->accept( this );
}

void x::visitor::visit( x::closure_expr_ast * val )
{
	for ( const auto & it : val->get_results() )
		it->accept( this );

	for ( const auto & it : val->get_captures() )
		it->accept( this );

	for ( const auto & it : val->get_parameters() )
		it->accept( this );

	val->get_body()->accept( this );
}

void x::visitor::visit( x::elements_expr_ast * val )
{
	for ( const auto & it : val->get_elements() )
		it->accept( this );
}

void x::visitor::visit( x::arguments_expr_ast * val )
{
	for ( const auto & it : val->get_args() )
		it->accept( this );
}

void x::visitor::visit( x::identifier_expr_ast * val )
{
}

void x::visitor::visit( x::initializer_expr_ast * val )
{
	for ( const auto & it : val->get_args() )
		it->accept( this );
}

void x::visitor::visit( x::null_constant_expr_ast * val )
{
}

void x::visitor::visit( x::bool_constant_expr_ast * val )
{
}

void x::visitor::visit( x::int_constant_expr_ast * val )
{
}

void x::visitor::visit( x::uint_constant_expr_ast * val )
{

}

void x::visitor::visit( x::float_constant_expr_ast * val )
{
}

void x::visitor::visit( x::string_constant_expr_ast * val )
{
}

void x::scope_scanner_visitor::scanning( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast )
{
	_logger = logger;
	_symbols = symbols;

	ast->accept( this );

	_symbols = nullptr;
	_logger = nullptr;
}

void x::scope_scanner_visitor::visit( x::unit_ast * val )
{
	symbols()->push_scope( val->get_location().file );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::enum_decl_ast * val )
{
	symbols()->push_scope( val->get_name() );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::class_decl_ast * val )
{
	symbols()->push_scope( val->get_name() );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::function_decl_ast * val )
{
	symbols()->push_scope( val->get_name() );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::interface_decl_ast * val )
{
	symbols()->push_scope( val->get_name() );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::namespace_decl_ast * val )
{
	symbols()->push_scope( val->get_name() );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::compound_stat_ast * val )
{
	symbols()->push_scope( std::format( "block_{}_{}_{}", x::hash( val->get_location().file ), val->get_location().line, val->get_location().col ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::while_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", x::hash( val->get_location().file ), val->get_location().line, val->get_location().col ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::for_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", x::hash( val->get_location().file ), val->get_location().line, val->get_location().col ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::foreach_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", x::hash( val->get_location().file ), val->get_location().line, val->get_location().col ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

const x::logger_ptr & x::scope_scanner_visitor::logger() const
{
	return _logger;
}

const x::symbols_ptr & x::scope_scanner_visitor::symbols() const
{
	return _symbols;
}

void x::symbol_scanner_visitor::scanning( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast )
{
	scope_scanner_visitor::scanning( logger, symbols, ast );
}

void x::symbol_scanner_visitor::visit( x::unit_ast * val )
{
	symbols()->add_unit( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::parameter_ast * val )
{
	symbols()->add_paramater( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::enum_decl_ast * val )
{
	symbols()->add_enum( val );
	symbols()->push_scope( val->get_name() );
	for ( const auto & it : val->get_elements() )
	{
		symbols()->add_enum_element( it.first, it.second.get() );
	}
	symbols()->pop_scope();
}

void x::symbol_scanner_visitor::visit( x::using_decl_ast * val )
{
	symbols()->add_using( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::class_decl_ast * val )
{
	symbols()->add_class( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::variable_decl_ast * val )
{
	symbols()->add_variable( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::function_decl_ast * val )
{
	symbols()->add_function( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::template_decl_ast * val )
{
	symbols()->add_template( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::interface_decl_ast * val )
{
	symbols()->add_interface( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::namespace_decl_ast * val )
{
	symbols()->add_namespace( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::compound_stat_ast * val )
{
	symbols()->add_block( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::while_stat_ast * val )
{
	symbols()->add_cycle( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::for_stat_ast * val )
{
	symbols()->add_cycle( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::foreach_stat_ast * val )
{
	symbols()->add_cycle( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::local_stat_ast * val )
{
	symbols()->add_local( val );

	scope_scanner_visitor::visit( val );
}

void x::ast_tree_printer_visitor::print( std::ostream & out, const x::ast_ptr & ast )
{
	_cout = &out;
	ast->accept( this );
	_cout = nullptr;
}

void x::ast_tree_printer_visitor::visit( x::unit_ast * val )
{
	out( std::format( "//{}", val->get_location().file ) );

	visitor::visit( val );
}

void x::ast_tree_printer_visitor::visit( x::import_ast * val )
{
	outline( std::format( R"(import( "{}" ))", val->get_modulename() ) );
}

void x::ast_tree_printer_visitor::visit( x::attribute_ast * val )
{
	outline( "attribute( " );
	for ( size_t i = 0; i < val->get_attributes().size(); i++ )
	{
		out( std::format( R"({} = {})", val->get_attributes()[i].first, val->get_attributes()[i].second ) );
		if ( i < val->get_attributes().size() - 1 )
			out( ", " );
	}
	out( " )" );
}

void x::ast_tree_printer_visitor::visit( x::parameter_ast * val )
{
	out( val->get_name() );
	out( ": " ); val->get_valuetype()->accept( this );
	if ( val->get_default() )
	{
		out( " = " );
		val->get_default()->accept( this );
	}
}

void x::ast_tree_printer_visitor::visit( x::type_ast * val )
{
	out( std::format( R"({}{}{})", val->get_is_const() ? "const " : "", val->get_is_ref() ? "ref " : "", val->get_name() ) );
}

void x::ast_tree_printer_visitor::visit( x::func_type_ast * val )
{
	visit( static_cast<x::type_ast *>( val ) );
	out( val->get_parameters().empty() ? "(" : "( " );
	for ( size_t i = 0; i < val->get_parameters().size(); i++ )
	{
		val->get_parameters()[i]->accept( this );
		if ( i < val->get_parameters().size() - 1 )
			out( ", " );
	}
	out( val->get_parameters().empty() ? ")" : " )" );
}

void x::ast_tree_printer_visitor::visit( x::temp_type_ast * val )
{
	visit( static_cast<x::type_ast *>( val ) );
	out( val->get_elements().empty() ? "<" : "< " );
	for ( size_t i = 0; i < val->get_elements().size(); i++ )
	{
		val->get_elements()[i]->accept( this );
		if ( i < val->get_elements().size() - 1 )
			out( ", " );
	}
	out( val->get_elements().empty() ? ">" : " >" );
}

void x::ast_tree_printer_visitor::visit( x::list_type_ast * val )
{
	visit( static_cast<x::type_ast *>( val ) );
	out( "..." );
}

void x::ast_tree_printer_visitor::visit( x::array_type_ast * val )
{
	std::string layer;
	for ( size_t i = 1; i < val->get_layer(); i++ )
		layer.push_back( ',' );

	out( std::format( R"({}{}{}[{}])", val->get_is_const() ? "const " : "", val->get_is_ref() ? "ref " : "", val->get_name(), layer ) );
}

void x::ast_tree_printer_visitor::visit( x::enum_decl_ast * val )
{
	if ( val->get_attribute() ) val->get_attribute()->accept( this );

	outline( std::format( "{} enum {}", access( val->get_access() ), val->get_name() ) );
	outline( "{" ); push();
	{
		for ( const auto & it : val->get_elements() )
		{
			if ( it.second )
			{
				outline( std::format( "{} = ", it.first ) );
				it.second->accept( this );
				out( "," );
			}
			else
			{
				outline( std::format( "{},", it.first ) );
			}
		}
	}
	pop(); outline( "};" );
}

void x::ast_tree_printer_visitor::visit( x::class_decl_ast * val )
{
	if ( val->get_attribute() ) val->get_attribute()->accept( this );

	outline( std::format( "{} class {}", access( val->get_access() ), val->get_name() ) );
	if ( val->get_base() )
	{
		out( " : " );
		val->get_base()->accept( this );
	}
	outline( "{" ); push();
	{
		for ( const auto & it : val->get_friends() )
		{
			out( "friend " ); it->accept( this );
		}
		for ( const auto & it : val->get_usings() )
		{
			it->accept( this );
		}
		for ( const auto & it : val->get_variables() )
		{
			it->accept( this );
		}
		for ( const auto & it : val->get_functions() )
		{
			it->accept( this );
		}
	}
	pop(); outline( "};" );
}

void x::ast_tree_printer_visitor::visit( x::using_decl_ast * val )
{
	outline( std::format( "{} using {} = ", access( val->get_access() ), val->get_name() ) );
	visitor::visit( val );
	out( ";" );
}

void x::ast_tree_printer_visitor::visit( x::template_decl_ast * val )
{
	if ( val->get_attribute() ) val->get_attribute()->accept( this );

	outline( std::format( "{} template {}", access( val->get_access() ), val->get_name() ) );
	out( val->get_elements().empty() ? "<" : "< " );
	for ( size_t i = 0; i < val->get_elements().size(); i++ )
	{
		val->get_elements()[i]->accept( this );
		if ( i < val->get_elements().size() - 1 )
			out( ", " );
	}
	out( val->get_elements().empty() ? ">" : " >" );
	if ( val->get_base() )
	{
		out( " : " );
		val->get_base()->accept( this );
	}
	if ( val->get_where() )
	{
		out( " where" );
		val->get_where()->accept( this );
	}

	outline( "{" ); push();
	{
		for ( const auto & it : val->get_friends() )
		{
			out( "friend " ); it->accept( this );
		}
		for ( const auto & it : val->get_usings() )
		{
			it->accept( this );
		}
		for ( const auto & it : val->get_variables() )
		{
			it->accept( this );
		}
		for ( const auto & it : val->get_functions() )
		{
			it->accept( this );
		}
	}
	pop(); outline( "};" );
}

void x::ast_tree_printer_visitor::visit( x::variable_decl_ast * val )
{
	if ( val->get_attribute() ) val->get_attribute()->accept( this );

	outline( std::format( "{} var {}{}{}{}"
			 , access( val->get_access() )
			 , val->get_is_local() ? "local " : ""
			 , val->get_is_static() ? "static " : ""
			 , val->get_is_thread() ? "thread " : ""
			 , val->get_name() ) );

	if ( val->get_valuetype() )
	{
		out( ": " );
		val->get_valuetype()->accept( this );
	}

	if ( val->get_init() )
	{
		out( " = " );
		val->get_init()->accept( this );
	}

	out( ";" );
}

void x::ast_tree_printer_visitor::visit( x::function_decl_ast * val )
{
	if ( val->get_attribute() ) val->get_attribute()->accept( this );

	outline( std::format( "{} func {}{}{}{}{}{}"
			 , access( val->get_access() )
			 , val->get_is_const() ? "const " : ""
			 , val->get_is_async() ? "async " : ""
			 , val->get_is_final() ? "final " : ""
			 , val->get_is_static() ? "static " : ""
			 , val->get_is_virtual() ? "virtual " : ""
			 , val->get_name() ) );

	out( val->get_parameters().empty() ? "(" : "( " );
	for ( size_t i = 0; i < val->get_parameters().size(); i++ )
	{
		val->get_parameters()[i]->accept( this );

		if ( i < val->get_parameters().size() - 1 )
			out( ", " );
	}
	out( val->get_parameters().empty() ? ")" : " )" );

	if ( !val->get_results().empty() )
	{
		out( " -> " );
		for ( size_t i = 0; i < val->get_results().size(); i++ )
		{
			val->get_results()[i]->accept( this );

			if ( i < val->get_results().size() - 1 )
				out( ", " );
		}
	}

	if ( val->get_body() )
	{
		if ( val->get_body()->type() == x::ast_t::EXTERN_STAT )
			out( " = " );
		else
			outline();

		val->get_body()->accept( this );
	}
}

void x::ast_tree_printer_visitor::visit( x::interface_decl_ast * val )
{
	if ( val->get_attribute() ) val->get_attribute()->accept( this );

	outline( std::format( "{} interface {}", access( val->get_access() ), val->get_name() ) );
	outline( "{" ); push();
	{
		for ( const auto & it : val->get_functions() )
		{
			it->accept( this ); out( ";" );
		}
	}
	pop(); outline( "};" );
}

void x::ast_tree_printer_visitor::visit( x::namespace_decl_ast * val )
{
	if ( val->get_attribute() ) val->get_attribute()->accept( this );

	outline( std::format( "{} namespace {}", access( val->get_access() ), val->get_name() ) );
	outline( "{" ); push();
	{
		visitor::visit( val );
	}
	pop(); outline( "};" );
}

void x::ast_tree_printer_visitor::visit( x::empty_stat_ast * val )
{
	out( "// " );
}

void x::ast_tree_printer_visitor::visit( x::extern_stat_ast * val )
{
	out( std::format( R"(extern( "{}", "{}" );)", val->get_libname(), val->get_funcname() ) );
}

void x::ast_tree_printer_visitor::visit( x::compound_stat_ast * val )
{
	outline( "{" ); push();
	for ( auto it : val->get_stats() )
	{
		outline();
		it->accept( this );
		out( ";" );
	}
	pop(); outline( "}" );
}

void x::ast_tree_printer_visitor::visit( x::await_stat_ast * val )
{
	out( "await " );
	visitor::visit( val );
}

void x::ast_tree_printer_visitor::visit( x::yield_stat_ast * val )
{
	out( "yield " );
	visitor::visit( val );
}

void x::ast_tree_printer_visitor::visit( x::if_stat_ast * val )
{
	out( "if ( " ); val->get_cond()->accept( this ); out( " )" );

	val->get_then_stat()->accept( this );

	if ( val->get_else_stat() )
	{
		outline( "else" );
		val->get_else_stat()->accept( this );
	}
}

void x::ast_tree_printer_visitor::visit( x::while_stat_ast * val )
{
	out( "while ( " ); val->get_cond()->accept( this ); out( " )" );

	val->get_body()->accept( this );
}

void x::ast_tree_printer_visitor::visit( x::for_stat_ast * val )
{
	out( "for ( " );
	if ( val->get_init() ) val->get_init()->accept( this );
	out( "; " );
	if ( val->get_cond() ) val->get_cond()->accept( this );
	out( "; " );
	if ( val->get_step() ) val->get_step()->accept( this );
	out( " )" );

	val->get_body()->accept( this );
}

void x::ast_tree_printer_visitor::visit( x::foreach_stat_ast * val )
{
	out( "foreach ( " );
	val->get_item()->accept( this );
	out( " : " );
	val->get_collection()->accept( this );
	out( " )" );

	val->get_body()->accept( this );
}

void x::ast_tree_printer_visitor::visit( x::switch_stat_ast * val )
{
	out( "switch ( " );
	val->get_exp()->accept( this );
	out( " )" );
	outline( "{" ); push();
	{
		for ( auto it : val->get_cases() )
		{
			outline( "case " ); it.first->accept( this ); out( ":" );
			it.second->accept( this );
		}

		if ( val->get_defult() )
		{
			outline( "default:" );
			val->get_defult()->accept( this );
		}
	}
	pop(); outline( "}" );
}

void x::ast_tree_printer_visitor::visit( x::break_stat_ast * val )
{
	out( "break" );
}

void x::ast_tree_printer_visitor::visit( x::return_stat_ast * val )
{
	if ( val->get_exps().empty() )
		out( "return" );
	else
	{
		out( "return " );
		for ( size_t i = 0; i < val->get_exps().size(); i++ )
		{
			val->get_exps()[i]->accept( this );
			if ( i < val->get_exps().size() - 1 )
				out( ", " );
		}
	}
}

void x::ast_tree_printer_visitor::visit( x::new_stat_ast * val )
{
	out( "new " );
	val->get_type()->accept( this );
	val->get_init_stat()->accept( this );
}

void x::ast_tree_printer_visitor::visit( x::try_stat_ast * val )
{
	out( "try" );
	val->get_try_stat()->accept( this );

	for ( auto & it : val->get_catch_stats() )
	{
		out( "catch(" );
		it.first->accept( this );
		out( ")" );

		it.second->accept( this );
	}

	if ( val->get_final_stat() )
	{
		out( "final" );
		val->get_final_stat()->accept( this );
	}
}

void x::ast_tree_printer_visitor::visit( x::throw_stat_ast * val )
{
	out( "throw " );
	val->get_exception()->accept( this );
}

void x::ast_tree_printer_visitor::visit( x::continue_stat_ast * val )
{
	out( "continue" );
}

void x::ast_tree_printer_visitor::visit( x::local_stat_ast * val )
{
	out( std::format( "var {}{}{}{}"
		 , val->get_is_local() ? "local " : ""
		 , val->get_is_static() ? "static " : ""
		 , val->get_is_thread() ? "thread " : ""
		 , val->get_name() ) );

	if ( val->get_valuetype() )
	{
		out( ": " );
		val->get_valuetype()->accept( this );
	}

	if ( val->get_init() )
	{
		out( " = " );
		val->get_init()->accept( this );
	}
}

void x::ast_tree_printer_visitor::visit( x::mulocal_stat_ast * val )
{
	out( "var [" );
	for ( size_t i = 0; i < val->get_locals().size(); i++ )
	{
		const auto & it = val->get_locals()[i];

		out( it->get_name() );

		if ( it->get_valuetype() )
		{
			out( ": " ); it->get_valuetype()->accept( this );
		}

		if ( i < val->get_locals().size() - 1 )
			out( ", " );
	}
	out( "]" );
}

void x::ast_tree_printer_visitor::visit( x::binary_expr_ast * val )
{
	switch ( val->get_op() )
	{
	case x::operator_t::NONE:
		break;
	case x::operator_t::ADD: val->get_left()->accept( this ); out( " + " ); val->get_right()->accept( this ); break;
	case x::operator_t::SUB: val->get_left()->accept( this ); out( " - " ); val->get_right()->accept( this ); break;
	case x::operator_t::MUL: val->get_left()->accept( this ); out( " * " ); val->get_right()->accept( this ); break;
	case x::operator_t::DIV: val->get_left()->accept( this ); out( " / " ); val->get_right()->accept( this ); break;
	case x::operator_t::MOD: val->get_left()->accept( this ); out( " % " ); val->get_right()->accept( this ); break;
	case x::operator_t::AND: val->get_left()->accept( this ); out( " & " ); val->get_right()->accept( this ); break;
	case x::operator_t::OR: val->get_left()->accept( this ); out( " | " ); val->get_right()->accept( this ); break;
	case x::operator_t::XOR: val->get_left()->accept( this ); out( " ^ " ); val->get_right()->accept( this ); break;
	case x::operator_t::LEFT_SHIFT: val->get_left()->accept( this ); out( " << " ); val->get_right()->accept( this ); break;
	case x::operator_t::RIGHT_SHIFT: val->get_left()->accept( this ); out( " >> " ); val->get_right()->accept( this ); break;
	case x::operator_t::LAND: val->get_left()->accept( this ); out( " && " ); val->get_right()->accept( this ); break;
	case x::operator_t::LOR: val->get_left()->accept( this ); out( " || " ); val->get_right()->accept( this ); break;
	case x::operator_t::ASSIGN: val->get_left()->accept( this ); out( " = " ); val->get_right()->accept( this ); break;
	case x::operator_t::ADD_ASSIGN: val->get_left()->accept( this ); out( " += " ); val->get_right()->accept( this ); break;
	case x::operator_t::SUB_ASSIGN: val->get_left()->accept( this ); out( " -= " ); val->get_right()->accept( this ); break;
	case x::operator_t::MUL_ASSIGN: val->get_left()->accept( this ); out( " *= " ); val->get_right()->accept( this ); break;
	case x::operator_t::DIV_ASSIGN: val->get_left()->accept( this ); out( " /= " ); val->get_right()->accept( this ); break;
	case x::operator_t::MOD_ASSIGN: val->get_left()->accept( this ); out( " %= " ); val->get_right()->accept( this ); break;
	case x::operator_t::AND_ASSIGN: val->get_left()->accept( this ); out( " &= " ); val->get_right()->accept( this ); break;
	case x::operator_t::OR_ASSIGN: val->get_left()->accept( this ); out( " |= " ); val->get_right()->accept( this ); break;
	case x::operator_t::XOR_ASSIGN: val->get_left()->accept( this ); out( " ^= " ); val->get_right()->accept( this ); break;
	case x::operator_t::LSHIFT_ASSIGN: val->get_left()->accept( this ); out( " <<= " ); val->get_right()->accept( this ); break;
	case x::operator_t::RSHIFT_ASSIGN: val->get_left()->accept( this ); out( " >>= " ); val->get_right()->accept( this ); break;
	case x::operator_t::EQUAL: val->get_left()->accept( this ); out( " == " ); val->get_right()->accept( this ); break;
	case x::operator_t::NOT_EQUAL: val->get_left()->accept( this ); out( " != " ); val->get_right()->accept( this ); break;
	case x::operator_t::LESS: val->get_left()->accept( this ); out( " < " ); val->get_right()->accept( this ); break;
	case x::operator_t::LARG: val->get_left()->accept( this ); out( " > " ); val->get_right()->accept( this ); break;
	case x::operator_t::LESS_EQUAL: val->get_left()->accept( this ); out( " <= " ); val->get_right()->accept( this ); break;
	case x::operator_t::LARG_EQUAL: val->get_left()->accept( this ); out( " >= " ); val->get_right()->accept( this ); break;
	case x::operator_t::COMPARE: val->get_left()->accept( this ); out( " <=> " ); val->get_right()->accept( this ); break;
	case x::operator_t::AS: val->get_left()->accept( this ); out( " as " ); val->get_right()->accept( this ); break;
	case x::operator_t::IS: val->get_left()->accept( this ); out( " is " ); val->get_right()->accept( this ); break;
	case x::operator_t::MEMBER: val->get_left()->accept( this ); out( "." ); val->get_right()->accept( this ); break;
	case x::operator_t::INVOKE: val->get_left()->accept( this ); val->get_right()->accept( this ); break;
	default:
		break;
	}
}

void x::ast_tree_printer_visitor::visit( x::unary_expr_ast * val )
{
	switch ( val->get_op() )
	{
	case x::operator_t::NONE:
		break;
	case x::operator_t::PLUS: out( "+" ); visitor::visit( val ); break;
	case x::operator_t::MINUS: out( "-" ); visitor::visit( val ); break;
	case x::operator_t::INC: out( "++" ); visitor::visit( val ); break;
	case x::operator_t::DEC: out( "--" ); visitor::visit( val ); break;
	case x::operator_t::POSTINC: visitor::visit( val ); out( "++" ); break;
	case x::operator_t::POSTDEC: visitor::visit( val ); out( "--" ); break;
	case x::operator_t::REV: out( "~" ); visitor::visit( val ); break;
	case x::operator_t::NOT: out( "!" ); visitor::visit( val ); break;
	case x::operator_t::SIZEOF: out( "sizeof( " ); visitor::visit( val ); out( " )" ); break;
	case x::operator_t::TYPEOF:out( "typeof( " ); visitor::visit( val ); out( " )" ); break;
	default:
		break;
	}
}

void x::ast_tree_printer_visitor::visit( x::identifier_expr_ast * val )
{
	out( val->get_ident() );
}

void x::ast_tree_printer_visitor::visit( x::closure_expr_ast * val )
{
	visitor::visit( val );
}

void x::ast_tree_printer_visitor::visit( x::bracket_expr_ast * val )
{
	out( "( " );
	val->get_exp()->accept( this );
	out( " )" );
}

void x::ast_tree_printer_visitor::visit( x::elements_expr_ast * val )
{
	out( "[" );
	for ( size_t i = 0; i < val->get_elements().size(); i++ )
	{
		val->get_elements()[i]->accept( this );
		if ( i < val->get_elements().size() - 1 )
			out( ", " );
	}
	out( "]" );
}

void x::ast_tree_printer_visitor::visit( x::arguments_expr_ast * val )
{
	out( "( " );
	for ( size_t i = 0; i < val->get_args().size(); i++ )
	{
		val->get_args()[i]->accept( this );
		if ( i < val->get_args().size() - 1 )
			out( ", " );
	}
	out( " )" );
}

void x::ast_tree_printer_visitor::visit( x::initializer_expr_ast * val )
{
	out( val->get_args().empty() ? "{" : "{ " );
	for ( size_t i = 0; i < val->get_args().size(); i++ )
	{
		val->get_args()[i]->accept( this );
		if ( i < val->get_args().size() - 1 )
			out( ", " );
	}
	out( val->get_args().empty() ? "}" : " }" );
}

void x::ast_tree_printer_visitor::visit( x::null_constant_expr_ast * val )
{
	out( "null" );
}

void x::ast_tree_printer_visitor::visit( x::bool_constant_expr_ast * val )
{
	out( val->get_value() ? "true" : "false" );
}

void x::ast_tree_printer_visitor::visit( x::int_constant_expr_ast * val )
{
	switch ( val->type() )
	{
	case x::ast_t::INT32_CONSTANT_EXP: out( std::to_string( static_cast<x::int32_constant_expr_ast *>( val )->get_value() ) ); break;
	case x::ast_t::INT64_CONSTANT_EXP: out( std::to_string( static_cast<x::int64_constant_expr_ast *>( val )->get_value() ) ); break;
	default:
		break;
	}
}

void x::ast_tree_printer_visitor::visit( x::uint_constant_expr_ast * val )
{
	switch ( val->type() )
	{
	case x::ast_t::UINT32_CONSTANT_EXP: out( std::to_string( static_cast<x::uint32_constant_expr_ast *>( val )->get_value() ) ); break;
	case x::ast_t::UINT64_CONSTANT_EXP: out( std::to_string( static_cast<x::uint64_constant_expr_ast *>( val )->get_value() ) ); break;
	default:
		break;
	}
}

void x::ast_tree_printer_visitor::visit( x::float_constant_expr_ast * val )
{
	switch ( val->type() )
	{
	case x::ast_t::FLOAT32_CONSTANT_EXP:  out( std::to_string( static_cast<x::float32_constant_expr_ast *>( val )->get_value() ) ); break;
	case x::ast_t::FLOAT64_CONSTANT_EXP:  out( std::to_string( static_cast<x::float64_constant_expr_ast *>( val )->get_value() ) ); break;
	default:
		break;
	}
}

void x::ast_tree_printer_visitor::visit( x::string_constant_expr_ast * val )
{
	std::string str;

	for ( auto c : val->get_value() )
	{
		switch ( c )
		{
		case '\'':
			str.append( R"(\')" );
			break;
		case '\"':
			str.append( R"(\")" );
			break;
		case '\\':
			str.append( R"(\)" );
			break;
		case '\n':
			str.append( R"(\n)" );
			break;
		case '\r':
			str.append( R"(\r)" );
			break;
		case '\t':
			str.append( R"(\t)" );
			break;
		default:
			str.push_back( c );
			break;
		}
	}

	out( std::format( R"("{}")", str ) );
}

const char * x::ast_tree_printer_visitor::access( x::access_t val )
{
	switch ( val )
	{
	case x::access_t::PUBLIC:
		return "public";
	case x::access_t::PRIVATE:
		return "private";
	case x::access_t::PROTECTED:
		return "protected";
	}

	return "unknown";
}

void x::ast_tree_printer_visitor::out( std::string_view str )
{
	*_cout << str;
}

void x::ast_tree_printer_visitor::outline( std::string_view str )
{
	*_cout << std::endl; outtab(); out( str );
}

void x::ast_tree_printer_visitor::outtab()
{
	for ( size_t i = 0; i < _tab; i++ )
	{
		*_cout << "    ";
	}
}

void x::ast_tree_printer_visitor::push()
{
	_tab++;
}

void x::ast_tree_printer_visitor::pop()
{
	_tab--;
}
