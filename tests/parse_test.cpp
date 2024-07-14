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
		out( std::format( "//{}", val->location.file ) );

		visitor::visit( val );
	}
	void visit( x::import_ast * val ) override
	{
		outl( std::format( R"(import( "{}" ))", val->path ) );
	}
	void visit( x::attribute_ast * val ) override
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

	void visit( x::type_ast * val ) override
	{
		out( std::format( R"({}{}{})", val->is_const ? "const " : "", val->is_ref ? "ref " : "", val->name ) );
	}
	void visit( x::func_type_ast * val ) override
	{
		visitor::visit( val );
	}
	void visit( x::temp_type_ast * val ) override
	{
		visitor::visit( val );
	}
	void visit( x::list_type_ast * val ) override
	{
		visitor::visit( val );
	}
	void visit( x::array_type_ast * val ) override
	{
		std::string layer;
		for ( size_t i = 1; i < val->layer; i++ )
			layer.push_back( ',' );

		out( std::format( R"({}{}{}[{}])", val->is_const ? "const " : "", val->is_ref ? "ref " : "", val->name, layer ) );
	}

	void visit( x::enum_decl_ast * val ) override
	{
		if ( val->attr ) val->attr->accept( this );

		outl( std::format( "{} enum {}", access( val->access ), val->name ) );
		outl( "{" ); push();
		{
			visitor::visit( val );
		}
		pop(); outl( "};" );
	}
	void visit( x::class_decl_ast * val ) override
	{
		if ( val->attr ) val->attr->accept( this );

		outl( std::format( "{} class {}", access( val->access ), val->name ) );
		outl( "{" ); push();
		{
			visitor::visit( val );
		}
		pop(); outl( "};" );
	}
	void visit( x::using_decl_ast * val ) override
	{
		outl( std::format( "{} using {} = ", access( val->access ), val->name ) );
		visitor::visit( val );
		out( ";" );
	}
	void visit( x::element_decl_ast * val ) override
	{
		if ( val->attr ) val->attr->accept( this );

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
	void visit( x::template_decl_ast * val ) override
	{
		visitor::visit( val );
	}
	void visit( x::variable_decl_ast * val ) override
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
	void visit( x::function_decl_ast * val ) override
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
	void visit( x::parameter_decl_ast * val ) override
	{
		out( val->name );
		if ( val->value_type )
			out( ": " );

		visitor::visit( val );
	}
	void visit( x::namespace_decl_ast * val ) override
	{
		if ( val->attr ) val->attr->accept( this );

		outl( std::format( "namespace {}", val->name ) );
		outl( "{" ); push();
		{
			visitor::visit( val );
		}
		pop(); outl( "};" );
	}

	void visit( x::empty_stat_ast * val ) override
	{
		out( "// " );
	}
	void visit( x::extern_stat_ast * val ) override
	{
		out( std::format( R"(extern( "{}", "{}" );)", val->libname, val->funcname ) );
	}
	void visit( x::compound_stat_ast * val ) override
	{
		out( "{" ); push();
		for ( auto it : val->stats )
		{
			outl();
			it->accept( this );
			out( ";" );
		}
		pop(); outl( "}" );
	}
	void visit( x::await_stat_ast * val ) override
	{
		out( "await " );
		visitor::visit( val );
	}
	void visit( x::yield_stat_ast * val ) override
	{
		out( "yield " );
		visitor::visit( val );
	}
	void visit( x::new_stat_ast * val ) override
	{
		out( "new " );
		visitor::visit( val );
	}
	void visit( x::if_stat_ast * val ) override
	{
		out( "if ( " ); val->cond->accept( this ); outl( " )" );

		val->then_stat->accept( this );

		if ( val->else_stat )
		{
			outl( "else" );
			val->else_stat->accept( this );
		}
	}
	void visit( x::while_stat_ast * val ) override
	{
		out( "while ( " ); val->cond->accept( this ); outl( " )" );

		val->stat->accept( this );
	}
	void visit( x::for_stat_ast * val ) override
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
	void visit( x::foreach_stat_ast * val ) override
	{
		out( "for ( " );
		val->item->accept( this );
		out( " : " );
		val->collection->accept( this );
		out( " )" );

		val->stat->accept( this );
	}
	void visit( x::switch_stat_ast * val ) override
	{
		out( "switch ( " );
		val->expr->accept( this );
		out( " )" );
		outl( "{" ); push();
		{
			for ( auto it : val->cases )
			{
				outl( "case " ); it.first->accept( this ); outl( ":" );
				it.second->accept( this );
			}

			if ( val->defult )
			{
				outl( "default:" );
				val->defult->accept( this );
			}
		}
		outl( "}" ); pop();
	}
	void visit( x::break_stat_ast * val ) override
	{
		out( "break" );
	}
	void visit( x::return_stat_ast * val ) override
	{
		if( val->exprs.empty() )
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
	void visit( x::continue_stat_ast * val ) override
	{
		out( "continue" );
	}
	void visit( x::local_stat_ast * val ) override
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

	void visit( x::assignment_expr_ast * val ) override
	{
		val->left->accept( this );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		val->right->accept( this );
	}
	void visit( x::logical_or_expr_ast * val ) override
	{
		val->left->accept( this );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		val->right->accept( this );
	}
	void visit( x::logical_and_expr_ast * val ) override
	{
		val->left->accept( this );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		val->right->accept( this );
	}
	void visit( x::or_expr_ast * val ) override
	{
		val->left->accept( this );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		val->right->accept( this );
	}
	void visit( x::xor_expr_ast * val ) override
	{
		val->left->accept( this );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		val->right->accept( this );
	}
	void visit( x::and_expr_ast * val ) override
	{
		val->left->accept( this );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		val->right->accept( this );
	}
	void visit( x::compare_expr_ast * val ) override
	{
		val->left->accept( this );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		val->right->accept( this );
	}
	void visit( x::shift_expr_ast * val ) override
	{
		val->left->accept( this );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		val->right->accept( this );
	}
	void visit( x::add_expr_ast * val ) override
	{
		val->left->accept( this );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		val->right->accept( this );
	}
	void visit( x::mul_expr_ast * val ) override
	{
		val->left->accept( this );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		val->right->accept( this );
	}
	void visit( x::as_expr_ast * val ) override
	{
		val->value->accept( this );
		out( " as " );
		val->cast_type->accept( this );
	}
	void visit( x::is_expr_ast * val ) override
	{
		val->value->accept( this );
		out( " is " );
		val->cast_type->accept( this );
	}
	void visit( x::sizeof_expr_ast * val ) override
	{
		out( "sizeof( " );
		val->value->accept( this );
		out( ")" );
	}
	void visit( x::typeof_expr_ast * val ) override
	{
		out( "typeof( " );
		val->value->accept( this );
		out( ")" );
	}
	void visit( x::unary_expr_ast * val ) override
	{
		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}

		visitor::visit( val );
	}
	void visit( x::postfix_expr_ast * val ) override
	{
		visitor::visit( val );

		switch ( val->token )
		{
		case x::token_t::TK_ASSIGN: out( " = " ); break;
		case x::token_t::TK_ADD_ASSIGN: out( " += " ); break;
		case x::token_t::TK_SUB_ASSIGN: out( " -= " ); break;
		case x::token_t::TK_MUL_ASSIGN: out( " *= " ); break;
		case x::token_t::TK_DIV_ASSIGN: out( " /= " ); break;
		case x::token_t::TK_MOD_ASSIGN: out( " %= " ); break;
		case x::token_t::TK_AND_ASSIGN: out( " &= " ); break;
		case x::token_t::TK_OR_ASSIGN: out( " |= " ); break;
		case x::token_t::TK_XOR_ASSIGN: out( " ^= " ); break;
		case x::token_t::TK_LSHIFT_EQUAL: out( " <<= " ); break;
		case x::token_t::TK_RSHIFT_EQUAL: out( " >>= " ); break;
		case x::token_t::TK_EQUAL: out( " == " ); break;
		case x::token_t::TK_NOT_EQUAL: out( " != " ); break;
		case x::token_t::TK_LESS: out( " < " ); break;
		case x::token_t::TK_LARG: out( " > " ); break;
		case x::token_t::TK_LESS_OR_EQUAL: out( " <= " ); break;
		case x::token_t::TK_LARG_OR_EQUAL: out( " >= " ); break;
		case x::token_t::TK_COMPARE: out( " <=> " ); break;
		case x::token_t::TK_INC: out( " ++ " ); break;
		case x::token_t::TK_DEC: out( " -- " ); break;
		case x::token_t::TK_ADD: out( " + " ); break;
		case x::token_t::TK_SUB: out( " - " ); break;
		case x::token_t::TK_MUL: out( " * " ); break;
		case x::token_t::TK_DIV: out( " / " ); break;
		case x::token_t::TK_MOD: out( " % " ); break;
		case x::token_t::TK_AND: out( " & " ); break;
		case x::token_t::TK_OR: out( " | " ); break;
		case x::token_t::TK_XOR: out( " ^ " ); break;
		case x::token_t::TK_LEFT_SHIFT: out( " << " ); break;
		case x::token_t::TK_RIGHT_SHIFT: out( " >> " ); break;
		case x::token_t::TK_LAND: out( " && " ); break;
		case x::token_t::TK_LOR: out( " || " ); break;
		case x::token_t::TK_LNOT: out( " ! " ); break;
		case x::token_t::TK_NOT: out( " ~ " ); break;
		default:
			break;
		}
	}
	void visit( x::invoke_expr_ast * val ) override
	{
		val->left->accept( this );
		val->right->accept( this );
	}
	void visit( x::member_expr_ast * val ) override
	{
		val->left->accept( this );
		out( "." );
		val->right->accept( this );
	}
	void visit( x::identifier_expr_ast * val ) override
	{
		out( val->ident );
	}
	void visit( x::closure_expr_ast * val ) override
	{
		visitor::visit( val );
	}
	void visit( x::arguments_expr_ast * val ) override
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
	void visit( x::initializers_expr_ast * val ) override
	{
		out( "{ " );
		for ( size_t i = 0; i < val->args.size(); i++ )
		{
			val->args[i]->accept( this );
			if ( i < val->args.size() - 1 )
				out( ", " );
		}
		out( " }" );
	}
	void visit( x::null_const_expr_ast * val ) override
	{
		out( "null" );
	}
	void visit( x::bool_const_expr_ast * val ) override
	{
		out( val->value ? "true" : "false" );
	}
	void visit( x::int_const_expr_ast * val ) override
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
	void visit( x::float_const_expr_ast * val ) override
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
	void visit( x::string_const_expr_ast * val ) override
	{
		out( std::format( R"("{}")", val->value ) );
	}

private:
	const char * access( x::access_t val )
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

		return "unikwon";
	}

private:
	void out( std::string_view str )
	{
		std::cout << str;
	}
	void outl( std::string_view str = {} )
	{
		std::cout << std::endl; _outt(); out( str );
	}
	void push()
	{
		_tab++;
	}
	void pop()
	{
		_tab--;
	}

	void _outt()
	{
		for ( size_t i = 0; i < _tab; i++ )
		{
			std::cout << "    ";
		}
	}
private:
	int _tab = 0;
};

int main()
{
	auto path = ( std::filesystem::current_path() / ".." / "script_0.xs" ).lexically_normal();

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