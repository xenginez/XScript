#include "visitor.h"

#include "symbols.h"
#include "exception.h"

void x::visitor::visit( x::unit_ast * val )
{
	for ( const auto & it : val->imports )
		it->accept( this );

	for ( const auto & it : val->namespaces )
		it->accept( this );
}

void x::visitor::visit( x::import_ast * val )
{
}

void x::visitor::visit( x::attribute_ast * val )
{
}

void x::visitor::visit( x::type_ast * val )
{
}

void x::visitor::visit( x::func_type_ast * val )
{
	for ( const auto & it : val->parameters )
		it->accept( this );
}

void x::visitor::visit( x::temp_type_ast * val )
{
	for ( const auto & it : val->elements )
		it->accept( this );
}

void x::visitor::visit( x::list_type_ast * val )
{

}

void x::visitor::visit( x::array_type_ast * val )
{
}

void x::visitor::visit( x::enum_element_ast * val )
{
	if ( val->value )
		val->value->accept( this );
}

void x::visitor::visit( x::template_element_ast * val )
{

}

void x::visitor::visit( x::parameter_element_ast * val )
{
	val->value_type->accept( this );
}

void x::visitor::visit( x::enum_decl_ast * val )
{
	for ( const auto & it : val->elements )
		it->accept( this );
}

void x::visitor::visit( x::class_decl_ast * val )
{
	if ( val->base )
		val->base->accept( this );

	for ( const auto & it : val->usings )
		it->accept( this );
	for ( const auto & it : val->variables )
		it->accept( this );
	for ( const auto & it : val->functions )
		it->accept( this );
}

void x::visitor::visit( x::using_decl_ast * val )
{
	val->retype->accept( this );
}

void x::visitor::visit( x::template_decl_ast * val )
{
	if ( val->base )
		val->base->accept( this );

	if ( val->where )
		val->where->accept( this );

	for ( const auto & it : val->elements )
		it->accept( this );
	for ( const auto & it : val->usings )
		it->accept( this );
	for ( const auto & it : val->variables )
		it->accept( this );
	for ( const auto & it : val->functions )
		it->accept( this );
}

void x::visitor::visit( x::variable_decl_ast * val )
{
	val->value_type->accept( this );

	if ( val->init )
		val->init->accept( this );
}

void x::visitor::visit( x::function_decl_ast * val )
{
	for ( const auto & it : val->parameters )
		it->accept( this );

	for ( const auto & it : val->results )
		it->accept( this );

	val->stat->accept( this );
}

void x::visitor::visit( x::namespace_decl_ast * val )
{
	for ( const auto & it : val->members )
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
	for ( const auto & it : val->stats )
		it->accept( this );
}

void x::visitor::visit( x::await_stat_ast * val )
{
	val->exp->accept( this );
}

void x::visitor::visit( x::yield_stat_ast * val )
{
	val->exp->accept( this );
}

void x::visitor::visit( x::new_stat_ast * val )
{
	val->newtype->accept( this );
	if ( val->init )
		val->init->accept( this );
}

void x::visitor::visit( x::if_stat_ast * val )
{
	val->cond->accept( this );
	val->then_stat->accept( this );
	if ( val->else_stat )
		val->else_stat->accept( this );
}

void x::visitor::visit( x::while_stat_ast * val )
{
	val->cond->accept( this );
	val->stat->accept( this );
}

void x::visitor::visit( x::for_stat_ast * val )
{
	val->init->accept( this );
	val->cond->accept( this );
	val->stat->accept( this );
	val->step->accept( this );
}

void x::visitor::visit( x::foreach_stat_ast * val )
{
	val->item->accept( this );
	val->collection->accept( this );
	val->stat->accept( this );
}

void x::visitor::visit( x::switch_stat_ast * val )
{
	val->expr->accept( this );
	for ( auto & it : val->cases )
	{
		it.first->accept( this );
		it.second->accept( this );
	}
	if ( val->defult )
		val->defult->accept( this );
}

void x::visitor::visit( x::break_stat_ast * val )
{
}

void x::visitor::visit( x::return_stat_ast * val )
{
	for ( auto & it : val->exprs )
		it->accept( this );
}

void x::visitor::visit( x::continue_stat_ast * val )
{
}

void x::visitor::visit( x::local_stat_ast * val )
{
	val->value_type->accept( this );

	if ( val->init )
		val->init->accept( this );
}

void x::visitor::visit( x::binary_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::unary_expr_ast * val )
{
	val->exp->accept( this );
}

void x::visitor::visit( x::bracket_expr_ast * val )
{
	val->expr->accept( this );
}

void x::visitor::visit( x::closure_expr_ast * val )
{
	for ( const auto & it : val->results )
		it->accept( this );

	for ( const auto & it : val->captures )
		it->accept( this );

	for ( const auto & it : val->parameters )
		it->accept( this );

	val->stat->accept( this );
}

void x::visitor::visit( x::arguments_expr_ast * val )
{
	for ( const auto & it : val->args )
		it->accept( this );
}

void x::visitor::visit( x::identifier_expr_ast * val )
{
}

void x::visitor::visit( x::initializer_expr_ast * val )
{
	for ( const auto & it : val->args )
		it->accept( this );
}

void x::visitor::visit( x::null_const_expr_ast * val )
{
}

void x::visitor::visit( x::bool_const_expr_ast * val )
{
}

void x::visitor::visit( x::int_const_expr_ast * val )
{
}

void x::visitor::visit( x::float_const_expr_ast * val )
{
}

void x::visitor::visit( x::string_const_expr_ast * val )
{
}

void x::scope_scanner_visitor::scanner( const x::symbols_ptr & symbols, const x::ast_ptr & ast )
{
	_symbols = symbols;
	ast->accept( this );
	_symbols = nullptr;
}

void x::scope_scanner_visitor::visit( x::unit_ast * val )
{
	symbols()->push_scope( val->location.file );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::enum_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::class_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::function_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::namespace_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::compound_stat_ast * val )
{
	symbols()->push_scope( std::format( "block_{}_{}_{}", x::hash( val->location.file ), val->location.line, val->location.col ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::while_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", x::hash( val->location.file ), val->location.line, val->location.col ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::for_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", x::hash( val->location.file ), val->location.line, val->location.col ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_scanner_visitor::visit( x::foreach_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", x::hash( val->location.file ), val->location.line, val->location.col ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

const x::symbols_ptr & x::scope_scanner_visitor::symbols() const
{
	return _symbols;
}

void x::symbol_scanner_visitor::scanner( const x::symbols_ptr & symbols, const x::ast_ptr & ast )
{
	scope_scanner_visitor::scanner( symbols, ast );
}

void x::symbol_scanner_visitor::visit( x::unit_ast * val )
{
	symbols()->add_unit( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::enum_element_ast * val )
{
	symbols()->add_enum_elem( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::template_element_ast * val )
{
	symbols()->add_template_elem( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::parameter_element_ast * val )
{
	symbols()->add_paramater_elem( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::enum_decl_ast * val )
{
	symbols()->add_enum( val );

	scope_scanner_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::using_decl_ast * val )
{
	symbols()->add_alias( val );

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
	out( std::format( "//{}", val->location.file ) );

	visitor::visit( val );
}

void x::ast_tree_printer_visitor::visit( x::import_ast * val )
{
	outl( std::format( R"(import( "{}" ))", val->path ) );
}

void x::ast_tree_printer_visitor::visit( x::attribute_ast * val )
{
	outl( "attribute( " );
	for ( size_t i = 0; i < val->attributes.size(); i++ )
	{
		out( std::format( R"({} = {})", val->attributes[i].first, val->attributes[i].second ) );
		if ( i < val->attributes.size() - 1 )
			out( ", " );
	}
	out( " )" );
}

void x::ast_tree_printer_visitor::visit( x::type_ast * val )
{
	out( std::format( R"({}{}{})", val->is_const ? "const " : "", val->is_ref ? "ref " : "", val->name ) );
}

void x::ast_tree_printer_visitor::visit( x::func_type_ast * val )
{
	visit( static_cast<x::type_ast *>( val ) );
	out( val->parameters.empty() ? "(" : "( " );
	for ( size_t i = 0; i < val->parameters.size(); i++ )
	{
		val->parameters[i]->accept( this );
		if ( i < val->parameters.size() - 1 )
			out( ", " );
	}
	out( val->parameters.empty() ? ")" : " )" );
}

void x::ast_tree_printer_visitor::visit( x::temp_type_ast * val )
{
	visit( static_cast<x::type_ast *>( val ) );
	out( val->elements.empty() ? "<" : "< " );
	for ( size_t i = 0; i < val->elements.size(); i++ )
	{
		val->elements[i]->accept( this );
		if ( i < val->elements.size() - 1 )
			out( ", " );
	}
	out( val->elements.empty() ? ">" : " >" );
}

void x::ast_tree_printer_visitor::visit( x::list_type_ast * val )
{
	visit( static_cast<x::type_ast *>( val ) );
	out( "..." );
}

void x::ast_tree_printer_visitor::visit( x::array_type_ast * val )
{
	std::string layer;
	for ( size_t i = 1; i < val->layer; i++ )
		layer.push_back( ',' );

	out( std::format( R"({}{}{}[{}])", val->is_const ? "const " : "", val->is_ref ? "ref " : "", val->name, layer ) );
}

void x::ast_tree_printer_visitor::visit( x::enum_element_ast * val )
{
	if ( val->value )
	{
		outl( std::format( "{} = ", val->name ) );
		val->value->accept( this );
		out( "," );
	}
	else
	{
		outl( std::format( "{},", val->name ) );
	}
}

void x::ast_tree_printer_visitor::visit( x::template_element_ast * val )
{
	out( val->name );
	if ( val->is_multi )
		out( "..." );
}

void x::ast_tree_printer_visitor::visit( x::parameter_element_ast * val )
{
	out( val->name );
	if ( val->value_type )
		out( ": " );

	visitor::visit( val );
}

void x::ast_tree_printer_visitor::visit( x::enum_decl_ast * val )
{
	if ( val->attr ) val->attr->accept( this );

	outl( std::format( "{} enum {}", access( val->access ), val->name ) );
	outl( "{" ); push();
	{
		visitor::visit( val );
	}
	pop(); outl( "};" );
}

void x::ast_tree_printer_visitor::visit( x::class_decl_ast * val )
{
	if ( val->attr ) val->attr->accept( this );

	outl( std::format( "{} class {}", access( val->access ), val->name ) );
	if ( val->base )
	{
		out( " : " );
		val->base->accept( this );
	}
	outl( "{" ); push();
	{
		for ( const auto & it : val->usings )
			it->accept( this );
		for ( const auto & it : val->variables )
			it->accept( this );
		for ( const auto & it : val->functions )
			it->accept( this );
	}
	pop(); outl( "};" );
}

void x::ast_tree_printer_visitor::visit( x::using_decl_ast * val )
{
	outl( std::format( "{} using {} = ", access( val->access ), val->name ) );
	visitor::visit( val );
	out( ";" );
}

void x::ast_tree_printer_visitor::visit( x::template_decl_ast * val )
{
	if ( val->attr ) val->attr->accept( this );

	outl( std::format( "{} template {}", access( val->access ), val->name ) );
	out( val->elements.empty() ? "<" : "< " );
	for ( size_t i = 0; i < val->elements.size(); i++ )
	{
		val->elements[i]->accept( this );
		if ( i < val->elements.size() - 1 )
			out( ", " );
	}
	out( val->elements.empty() ? ">" : " >" );
	if ( val->base )
	{
		out( " : " );
		val->base->accept( this );
	}
	if ( val->where )
	{
		out( " where" );
		val->where->accept( this );
	}

	outl( "{" ); push();
	{
		for ( const auto & it : val->usings )
			it->accept( this );
		for ( const auto & it : val->variables )
			it->accept( this );
		for ( const auto & it : val->functions )
			it->accept( this );
	}
	pop(); outl( "};" );
}

void x::ast_tree_printer_visitor::visit( x::variable_decl_ast * val )
{
	if ( val->attr ) val->attr->accept( this );

	outl( std::format( "{} var {}{}{}{}"
		  , access( val->access )
		  , val->is_local ? "local " : ""
		  , val->is_static ? "static " : ""
		  , val->is_thread ? "thread " : ""
		  , val->name ) );

	if ( val->value_type )
	{
		out( ": " );
		val->value_type->accept( this );
	}

	if ( val->init )
	{
		out( " = " );
		val->init->accept( this );
	}

	out( ";" );
}

void x::ast_tree_printer_visitor::visit( x::function_decl_ast * val )
{
	if ( val->attr ) val->attr->accept( this );

	outl( std::format( "{} func {}{}{}{}{}{}"
		  , access( val->access )
		  , val->is_const ? "const " : ""
		  , val->is_async ? "async " : ""
		  , val->is_final ? "final " : ""
		  , val->is_static ? "static " : ""
		  , val->is_virtual ? "virtual " : ""
		  , val->name ) );

	out( val->parameters.empty() ? "(" : "( " );
	for ( size_t i = 0; i < val->parameters.size(); i++ )
	{
		val->parameters[i]->accept( this );

		if ( i < val->parameters.size() - 1 )
			out( ", " );
	}
	out( val->parameters.empty() ? ")" : " )" );

	if ( !val->results.empty() )
	{
		out( " -> " );
		for ( size_t i = 0; i < val->results.size(); i++ )
		{
			val->results[i]->accept( this );

			if ( i < val->results.size() - 1 )
				out( ", " );
		}
	}

	if ( val->stat )
	{
		if ( val->stat->ast_type() == x::ast_t::EXTERN_STAT )
			out( " = " );
		else
			outl();

		val->stat->accept( this );
	}
}

void x::ast_tree_printer_visitor::visit( x::namespace_decl_ast * val )
{
	if ( val->attr ) val->attr->accept( this );

	outl( std::format( "{} namespace {}", access( val->access ), val->name ) );
	outl( "{" ); push();
	{
		visitor::visit( val );
	}
	pop(); outl( "};" );
}

void x::ast_tree_printer_visitor::visit( x::empty_stat_ast * val )
{
	out( "// " );
}

void x::ast_tree_printer_visitor::visit( x::extern_stat_ast * val )
{
	out( std::format( R"(extern( "{}", "{}" );)", val->libname, val->funcname ) );
}

void x::ast_tree_printer_visitor::visit( x::compound_stat_ast * val )
{
	outl( "{" ); push();
	for ( auto it : val->stats )
	{
		outl();
		it->accept( this );
		out( ";" );
	}
	pop(); outl( "}" );
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

void x::ast_tree_printer_visitor::visit( x::new_stat_ast * val )
{
	out( "new " );
	visitor::visit( val );
}

void x::ast_tree_printer_visitor::visit( x::if_stat_ast * val )
{
	out( "if ( " ); val->cond->accept( this ); out( " )" );

	val->then_stat->accept( this );

	if ( val->else_stat )
	{
		outl( "else" );
		val->else_stat->accept( this );
	}
}

void x::ast_tree_printer_visitor::visit( x::while_stat_ast * val )
{
	out( "while ( " ); val->cond->accept( this ); out( " )" );

	val->stat->accept( this );
}

void x::ast_tree_printer_visitor::visit( x::for_stat_ast * val )
{
	out( "for ( " );
	if ( val->init ) val->init->accept( this );
	out( "; " );
	if ( val->cond ) val->cond->accept( this );
	out( "; " );
	if ( val->step ) val->step->accept( this );
	out( " )" );

	val->stat->accept( this );
}

void x::ast_tree_printer_visitor::visit( x::foreach_stat_ast * val )
{
	out( "foreach ( " );
	val->item->accept( this );
	out( " : " );
	val->collection->accept( this );
	out( " )" );

	val->stat->accept( this );
}

void x::ast_tree_printer_visitor::visit( x::switch_stat_ast * val )
{
	out( "switch ( " );
	val->expr->accept( this );
	out( " )" );
	outl( "{" ); push();
	{
		for ( auto it : val->cases )
		{
			outl( "case " ); it.first->accept( this ); out( ":" );
			it.second->accept( this );
		}

		if ( val->defult )
		{
			outl( "default:" );
			val->defult->accept( this );
		}
	}
	pop(); outl( "}" );
}

void x::ast_tree_printer_visitor::visit( x::break_stat_ast * val )
{
	out( "break" );
}

void x::ast_tree_printer_visitor::visit( x::return_stat_ast * val )
{
	if ( val->exprs.empty() )
		out( "return" );
	else
	{
		out( "return " );
		for ( size_t i = 0; i < val->exprs.size(); i++ )
		{
			val->exprs[i]->accept( this );
			if ( i < val->exprs.size() - 1 )
				out( ", " );
		}
	}
}

void x::ast_tree_printer_visitor::visit( x::continue_stat_ast * val )
{
	out( "continue" );
}

void x::ast_tree_printer_visitor::visit( x::local_stat_ast * val )
{
	out( std::format( "var {}{}{}{}"
		 , val->is_local ? "local " : ""
		 , val->is_static ? "static " : ""
		 , val->is_thread ? "thread " : ""
		 , val->name ) );

	if ( val->value_type )
	{
		out( ": " );
		val->value_type->accept( this );
	}

	if ( val->init )
	{
		out( " = " );
		val->init->accept( this );
	}
}

void x::ast_tree_printer_visitor::visit( x::binary_expr_ast * val )
{
	switch ( val->op )
	{
	case x::binary_expr_ast::op_t::NONE:
		break;
	case x::binary_expr_ast::op_t::ADD: val->left->accept( this ); out( " + " );val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::SUB: val->left->accept( this ); out( " - " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::MUL: val->left->accept( this ); out( " * " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::DIV: val->left->accept( this ); out( " / " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::MOD: val->left->accept( this ); out( " % " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::AND: val->left->accept( this ); out( " & " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::OR: val->left->accept( this ); out( " | " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::XOR: val->left->accept( this ); out( " ^ " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::LEFT_SHIFT: val->left->accept( this ); out( " << " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::RIGHT_SHIFT: val->left->accept( this ); out( " >> " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::LAND: val->left->accept( this ); out( " && " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::LOR: val->left->accept( this ); out( " || " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::ASSIGN: val->left->accept( this ); out( " = " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::ADD_ASSIGN: val->left->accept( this ); out( " += " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::SUB_ASSIGN: val->left->accept( this ); out( " -= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::MUL_ASSIGN: val->left->accept( this ); out( " *= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::DIV_ASSIGN: val->left->accept( this ); out( " /= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::MOD_ASSIGN: val->left->accept( this ); out( " %= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::AND_ASSIGN: val->left->accept( this ); out( " &= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::OR_ASSIGN: val->left->accept( this ); out( " |= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::XOR_ASSIGN: val->left->accept( this ); out( " ^= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::LSHIFT_EQUAL: val->left->accept( this ); out( " <<= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::RSHIFT_EQUAL: val->left->accept( this ); out( " >>= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::EQUAL: val->left->accept( this ); out( " == " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::NOT_EQUAL: val->left->accept( this ); out( " != " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::LESS: val->left->accept( this ); out( " < " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::LARG: val->left->accept( this ); out( " > " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::LESS_EQUAL: val->left->accept( this ); out( " <= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::LARG_EQUAL: val->left->accept( this ); out( " >= " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::COMPARE: val->left->accept( this ); out( " <=> " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::AS: val->left->accept( this ); out( " as " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::IS: val->left->accept( this ); out( " is " ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::INDEX: val->left->accept( this ); out( "[" ); val->right->accept( this ); out( "]" ); break;
	case x::binary_expr_ast::op_t::MEMBER: val->left->accept( this ); out( "." ); val->right->accept( this ); break;
	case x::binary_expr_ast::op_t::INVOKE: val->left->accept( this ); val->right->accept( this ); break;
	default:
		break;
	}
}

void x::ast_tree_printer_visitor::visit( x::unary_expr_ast * val )
{
	switch ( val->op )
	{
	case x::unary_expr_ast::op_t::NONE:
		break;
	case x::unary_expr_ast::op_t::PLUS: out( "+" ); visitor::visit( val ); break;
	case x::unary_expr_ast::op_t::MINUS: out( "-" ); visitor::visit( val ); break;
	case x::unary_expr_ast::op_t::INC: out( "++" ); visitor::visit( val ); break;
	case x::unary_expr_ast::op_t::DEC: out( "--" ); visitor::visit( val ); break;
	case x::unary_expr_ast::op_t::POSTINC: visitor::visit( val ); out( "++" ); break;
	case x::unary_expr_ast::op_t::POSTDEC: visitor::visit( val ); out( "--" ); break;
	case x::unary_expr_ast::op_t::REV: out( "~" ); visitor::visit( val ); break;
	case x::unary_expr_ast::op_t::NOT: out( "!" ); visitor::visit( val ); break;
	case x::unary_expr_ast::op_t::SIZEOF: out( "sizeof( " ); visitor::visit( val ); out( " )" ); break;
	case x::unary_expr_ast::op_t::TYPEOF:out( "typeof( " ); visitor::visit( val ); out( " )" ); break;
	default:
		break;
	}
}

void x::ast_tree_printer_visitor::visit( x::identifier_expr_ast * val )
{
	out( val->ident );
}

void x::ast_tree_printer_visitor::visit( x::closure_expr_ast * val )
{
	visitor::visit( val );
}

void x::ast_tree_printer_visitor::visit( x::bracket_expr_ast * val )
{
	out( "( " );
	val->expr->accept( this );
	out( " )" );
}

void x::ast_tree_printer_visitor::visit( x::arguments_expr_ast * val )
{
	out( "( " );
	for ( size_t i = 0; i < val->args.size(); i++ )
	{
		val->args[i]->accept( this );
		if ( i < val->args.size() - 1 )
			out( ", " );
	}
	out( " )" );
}

void x::ast_tree_printer_visitor::visit( x::initializer_expr_ast * val )
{
	out( val->args.empty() ? "{" : "{ " );
	for ( size_t i = 0; i < val->args.size(); i++ )
	{
		val->args[i]->accept( this );
		if ( i < val->args.size() - 1 )
			out( ", " );
	}
	out( val->args.empty() ? "}" : " }" );
}

void x::ast_tree_printer_visitor::visit( x::null_const_expr_ast * val )
{
	out( "null" );
}

void x::ast_tree_printer_visitor::visit( x::bool_const_expr_ast * val )
{
	out( val->value ? "true" : "false" );
}

void x::ast_tree_printer_visitor::visit( x::int_const_expr_ast * val )
{
	switch ( val->ast_type() )
	{
	case x::ast_t::INT8_CONST_EXP: out( std::to_string( static_cast<x::int8_const_expr_ast *>( val )->value ) ); break;
	case x::ast_t::INT16_CONST_EXP: out( std::to_string( static_cast<x::int16_const_expr_ast *>( val )->value ) ); break;
	case x::ast_t::INT32_CONST_EXP: out( std::to_string( static_cast<x::int32_const_expr_ast *>( val )->value ) ); break;
	case x::ast_t::INT64_CONST_EXP: out( std::to_string( static_cast<x::int64_const_expr_ast *>( val )->value ) ); break;
	case x::ast_t::UINT8_CONST_EXP: out( std::to_string( static_cast<x::uint8_const_expr_ast *>( val )->value ) ); break;
	case x::ast_t::UINT16_CONST_EXP: out( std::to_string( static_cast<x::uint16_const_expr_ast *>( val )->value ) ); break;
	case x::ast_t::UINT32_CONST_EXP: out( std::to_string( static_cast<x::uint32_const_expr_ast *>( val )->value ) ); break;
	case x::ast_t::UINT64_CONST_EXP: out( std::to_string( static_cast<x::uint64_const_expr_ast *>( val )->value ) ); break;
	default:
		break;
	}
}

void x::ast_tree_printer_visitor::visit( x::float_const_expr_ast * val )
{
	switch ( val->ast_type() )
	{
	case x::ast_t::FLOAT16_CONST_EXP:  out( std::to_string( static_cast<x::float16_const_expr_ast *>( val )->value.to_float() ) ); break;
	case x::ast_t::FLOAT32_CONST_EXP:  out( std::to_string( static_cast<x::float32_const_expr_ast *>( val )->value ) ); break;
	case x::ast_t::FLOAT64_CONST_EXP:  out( std::to_string( static_cast<x::float64_const_expr_ast *>( val )->value ) ); break;
	default:
		break;
	}
}

void x::ast_tree_printer_visitor::visit( x::string_const_expr_ast * val )
{
	std::string str;

	for ( auto c : val->value )
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

void x::ast_tree_printer_visitor::outl( std::string_view str )
{
	*_cout << std::endl; _outt(); out( str );
}

void x::ast_tree_printer_visitor::push()
{
	_tab++;
}

void x::ast_tree_printer_visitor::pop()
{
	_tab--;
}

void x::ast_tree_printer_visitor::_outt()
{
	for ( size_t i = 0; i < _tab; i++ )
	{
		*_cout << "    ";
	}
}
