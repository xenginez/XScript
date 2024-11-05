#include "optimize.h"

#include "symbols.h"

#define REG_OPTIMIZER( TYPE ) static x::code_optimize_visitor::optimizer::register_optimizer<x::##TYPE> __reg_##TYPE = {}

namespace
{
	REG_OPTIMIZER( translate_closure_optimizer );
}

std::vector<x::code_optimize_visitor::optimizer *> & x::code_optimize_visitor::optimizer::optimizers()
{
	static std::vector<x::code_optimize_visitor::optimizer *> _optimizers = {};
	return _optimizers;
}

void x::code_optimize_visitor::optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast, x::uint32 level )
{
	_level = level;
	scope_scan_visitor::scan( logger, symbols, ast );
}

void x::code_optimize_visitor::visit( x::closure_expr_ast * val )
{
	for ( auto it : optimizer::optimizers() )
	{
		auto types = it->ast_types();
		if ( std::find( types.begin(), types.end(), val->type() ) != types.end() )
		{
			if ( !it->optimize( logger(), symbols(), val->shared_from_this() ) )
				return;
		}
	}
}

x::uint32 x::translate_closure_optimizer::level() const
{
	return x::code_optimize_visitor::optimizer::MIN_LEVEL;
}

std::vector<x::ast_t> x::translate_closure_optimizer::ast_types() const
{
	return { x::ast_t::CLOSURE_EXP };
}

bool x::translate_closure_optimizer::optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast )
{
	if ( auto closure = std::static_pointer_cast<x::closure_expr_ast>( ast ) )
	{
		auto closure_class = std::make_shared<x::class_decl_ast>();
		{
			closure_class->set_location( closure->get_location() );
			closure_class->set_name( closure->get_name() );

			for ( const auto & it : closure->get_captures() )
			{
				auto variable = std::make_shared<x::variable_decl_ast>();
				variable->set_location( closure->get_location() );
				variable->set_access( x::access_t::PRIVATE );

				if ( it->get_ident() == "this" || it->get_ident() == "base" )
				{
					variable->set_name( "_" + it->get_ident());

					variable->set_is_local( false );
					variable->set_is_static( false );
					variable->set_is_thread( false );

					if ( auto cls = std::static_pointer_cast<x::class_decl_ast>( symbols->find_ast( x::symbol_t::CLASS ) ) )
					{
						auto type = std::make_shared<x::type_ast>();
						type->set_name( cls->get_name() );
						variable->set_valuetype( type );
					}
					else if ( auto temp = std::static_pointer_cast<x::template_decl_ast>( symbols->find_ast( x::symbol_t::TEMPLATE ) ) )
					{
						auto type = std::make_shared<x::temp_type_ast>();
						type->set_name( temp->get_name() );
						for ( const auto & it : temp->get_elements() )
						{
							auto ident = std::make_shared<x::identifier_expr_ast>();
							ident->set_ident( it->get_name() );
							type->insert_element( ident );
						}
						variable->set_valuetype( type );
					}
				}
				else
				{
					variable->set_name( it->get_ident() );
					if ( auto local = std::static_pointer_cast<x::local_stat_ast>( symbols->find_ast( it->get_ident() ) ) )
					{
						variable->set_is_local( local->get_is_local() );
						variable->set_is_static( local->get_is_static() );
						variable->set_is_thread( local->get_is_thread() );
						variable->set_valuetype( local->get_valuetype() );
					}
				}

				variable->set_init( std::make_shared<x::initializer_expr_ast>() );

				closure_class->insert_variable( variable );
			}

			auto oper_body = std::make_shared<x::operator_decl_ast>();
			oper_body->set_location( closure->get_location() );
			oper_body->set_name( "()" );

			for ( const auto & it : closure->get_parameters() )
				oper_body->insert_parameter( it );
			for ( const auto & it : closure->get_results() )
				oper_body->insert_result( it );

			oper_body->set_body( closure->get_body() );

			closure_class->insert_operator( oper_body );
		}

		symbols->push_scope( symbols->find_symbol( x::symbol_t::NAMESPACE ) );
		{
			std::static_pointer_cast<x::namespace_decl_ast>( symbols->find_ast( symbols->current_scope() ) )->insert_member( closure_class );

			x::symbol_scan_visitor symbol;
			symbol.scan( logger, symbols, closure_class );
		}
		symbols->pop_scope();

		auto new_exp = std::make_shared<x::new_expr_ast>();
		{
			new_exp->set_location( closure->get_location() );

			auto new_type = std::make_shared<x::type_ast>();
			new_type->set_location( closure->get_location() );
			new_type->set_name( closure->get_name() );
			new_exp->set_type( new_type );

			auto new_init = std::make_shared<x::initializer_expr_ast>();
			new_init->set_location( closure->get_location() );
			for ( const auto & it : closure->get_captures() )
			{
				auto ident = std::make_shared<x::identifier_expr_ast>();
				ident->set_location( closure->get_location() );
				ident->set_ident( it->get_ident() );
				new_init->insert_arg( ident );
			}
			new_exp->set_init_stat( new_init );
		}

		return ast->get_parent()->reset_child( ast, new_exp );
	}

	return false;
}
