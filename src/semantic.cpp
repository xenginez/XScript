#include "semantic.h"

#include "logger.h"
#include "symbols.h"
#include "exception.h"

#ifdef _MSC_VER
#pragma  warning( push ) 
#pragma  warning( disable: 4018 )
#endif

#define XERROR(COND, MSG, ...)  if ( COND ) logger()->error( std::format( MSG, __VA_ARGS__ ), ast->get_location() );
#define XWARNING(COND, MSG, ...)  if ( COND ) logger()->warning( std::format( MSG, __VA_ARGS__ ), ast->get_location() );
#define REG_ANALYZER( TYPE ) static x::semantics_analysis_visitor::analyzer::register_analyzer<x::##TYPE> __reg_##TYPE = {}

namespace
{
	//REG_ANALYZER( assert_analyzer );

	template<typename T1, typename T2> x::int32 x_lor( T1 left, T2 right )
	{
		return left || right;
	}
	template<typename T1, typename T2> x::int32 x_compare( T1 left, T2 right )
	{
		if constexpr ( std::is_same_v<T1, bool> || std::is_same_v<T2, bool> )
		{
			return ( (bool)left == (bool)right ) ? 0 : -1;
		}
		else
		{
			if ( left < right )
				return -1;
			else if ( left == right )
				return 0;

			return 1;
		}
	}
}

std::vector<x::semantics_analysis_visitor::analyzer *> & x::semantics_analysis_visitor::analyzer::analyzers()
{
	static std::vector<x::semantics_analysis_visitor::analyzer *> _analyzers = {};
	return _analyzers;
}

void x::semantics_analysis_visitor::analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast, x::uint32 level )
{
	scope_scan_visitor::scan( logger, symbols, ast );
}

bool x::semantics_analysis_visitor::is_constant( const x::expr_stat_ast * expr ) const
{
	switch ( expr->type() )
	{
	case x::ast_t::UNARY_EXP:
		return is_constant( static_cast<const x::unary_expr_ast *>( expr )->get_exp().get() );
	case x::ast_t::BINARY_EXP:
		return is_constant( static_cast<const x::binary_expr_ast *>( expr )->get_left().get() ) && is_constant( static_cast<const x::binary_expr_ast *>( expr )->get_right().get() );
	case x::ast_t::BRACKET_EXP:
		return is_constant( static_cast<const x::bracket_expr_ast *>( expr )->get_exp().get() );
	case x::ast_t::NULL_CONSTANT_EXP:
	case x::ast_t::BOOL_CONSTANT_EXP:
	case x::ast_t::INT32_CONSTANT_EXP:
	case x::ast_t::INT64_CONSTANT_EXP:
	case x::ast_t::UINT32_CONSTANT_EXP:
	case x::ast_t::UINT64_CONSTANT_EXP:
	case x::ast_t::FLOAT32_CONSTANT_EXP:
	case x::ast_t::FLOAT64_CONSTANT_EXP:
	case x::ast_t::STRING_CONSTANT_EXP:
		return true;
	}
	return false;
}

x::constant_expr_ast_ptr x::semantics_analysis_visitor::calc_constant( const x::expr_stat_ast * ast ) const
{
	x::constant_expr_ast_ptr result;

	switch ( ast->type() )
	{
	case x::ast_t::UNARY_EXP:
		result = calc_unary_constant( static_cast<const x::unary_expr_ast *>( ast ) );
		break;
	case x::ast_t::BINARY_EXP:
		result = calc_binary_constant( static_cast<const x::binary_expr_ast *>( ast ) );
		break;
	case x::ast_t::BRACKET_EXP:
		result = calc_constant( static_cast<const x::bracket_expr_ast *>( ast )->get_exp().get() );
		break;
	case x::ast_t::NULL_CONSTANT_EXP:
	case x::ast_t::BOOL_CONSTANT_EXP:
	case x::ast_t::INT32_CONSTANT_EXP:
	case x::ast_t::INT64_CONSTANT_EXP:
	case x::ast_t::UINT32_CONSTANT_EXP:
	case x::ast_t::UINT64_CONSTANT_EXP:
	case x::ast_t::FLOAT32_CONSTANT_EXP:
	case x::ast_t::FLOAT64_CONSTANT_EXP:
		result = std::static_pointer_cast<x::constant_expr_ast>( const_cast<x::expr_stat_ast *>( ast )->shared_from_this() );
	}

	return result;
}

x::constant_expr_ast_ptr x::semantics_analysis_visitor::calc_unary_constant( const x::unary_expr_ast * ast ) const
{
#define UNARY_OP( TYPE, OP1, ... ) std::static_pointer_cast<x::##TYPE>( value )->set_value( OP1 ( std::static_pointer_cast<x::##TYPE>( value )->get_value() ) __VA_ARGS__ )

	x::constant_expr_ast_ptr value = calc_constant( ast->get_exp().get() );

	switch ( ast->get_op() )
	{
	case x::operator_t::INC:
		switch ( value->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP: UNARY_OP( int32_constant_expr_ast, , +1 ); break;
		case x::ast_t::UINT32_CONSTANT_EXP: UNARY_OP( uint32_constant_expr_ast, , +1 ); break;
		case x::ast_t::INT64_CONSTANT_EXP: UNARY_OP( int64_constant_expr_ast, , +1 ); break;
		case x::ast_t::UINT64_CONSTANT_EXP: UNARY_OP( uint64_constant_expr_ast, , +1 ); break;
		case x::ast_t::FLOAT32_CONSTANT_EXP: UNARY_OP( float32_constant_expr_ast, , +1 ); break;
		case x::ast_t::FLOAT64_CONSTANT_EXP: UNARY_OP( float64_constant_expr_ast, , +1 ); break;
		}
		break;
	case x::operator_t::DEC:
		switch ( value->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP: UNARY_OP( int32_constant_expr_ast, , -1 ); break;
		case x::ast_t::UINT32_CONSTANT_EXP: UNARY_OP( uint32_constant_expr_ast, , -1 ); break;
		case x::ast_t::INT64_CONSTANT_EXP: UNARY_OP( int64_constant_expr_ast, , -1 ); break;
		case x::ast_t::UINT64_CONSTANT_EXP: UNARY_OP( uint64_constant_expr_ast, , -1 ); break;
		case x::ast_t::FLOAT32_CONSTANT_EXP: UNARY_OP( float32_constant_expr_ast, , -1 ); break;
		case x::ast_t::FLOAT64_CONSTANT_EXP: UNARY_OP( float64_constant_expr_ast, , -1 ); break;
		}
		break;
	case x::operator_t::REV:
		switch ( value->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP: UNARY_OP( int32_constant_expr_ast, ~); break;
		case x::ast_t::UINT32_CONSTANT_EXP: UNARY_OP( uint32_constant_expr_ast, ~); break;
		case x::ast_t::INT64_CONSTANT_EXP: UNARY_OP( int64_constant_expr_ast, ~); break;
		case x::ast_t::UINT64_CONSTANT_EXP: UNARY_OP( uint64_constant_expr_ast, ~); break;
		}
		break;
	case x::operator_t::NOT:
		switch ( value->type() )
		{
		case x::ast_t::BOOL_CONSTANT_EXP: UNARY_OP( bool_constant_expr_ast, !); break;
		case x::ast_t::INT32_CONSTANT_EXP: UNARY_OP( int32_constant_expr_ast, !); break;
		case x::ast_t::UINT32_CONSTANT_EXP: UNARY_OP( uint32_constant_expr_ast, !); break;
		case x::ast_t::INT64_CONSTANT_EXP: UNARY_OP( int64_constant_expr_ast, !); break;
		case x::ast_t::UINT64_CONSTANT_EXP: UNARY_OP( uint64_constant_expr_ast, !); break;
		case x::ast_t::FLOAT32_CONSTANT_EXP: UNARY_OP( float32_constant_expr_ast, !); break;
		case x::ast_t::FLOAT64_CONSTANT_EXP: UNARY_OP( float64_constant_expr_ast, !); break;
		}
		break;
	case x::operator_t::PLUS:
		switch ( value->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP: UNARY_OP( int32_constant_expr_ast, +); break;
		case x::ast_t::UINT32_CONSTANT_EXP: UNARY_OP( uint32_constant_expr_ast, +); break;
		case x::ast_t::INT64_CONSTANT_EXP: UNARY_OP( int64_constant_expr_ast, +); break;
		case x::ast_t::UINT64_CONSTANT_EXP: UNARY_OP( uint64_constant_expr_ast, +); break;
		case x::ast_t::FLOAT32_CONSTANT_EXP: UNARY_OP( float32_constant_expr_ast, +); break;
		case x::ast_t::FLOAT64_CONSTANT_EXP: UNARY_OP( float64_constant_expr_ast, +); break;
		}
		break;
	case x::operator_t::MINUS:
		switch ( value->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP: UNARY_OP( int32_constant_expr_ast, -); break;
		case x::ast_t::INT64_CONSTANT_EXP: UNARY_OP( int64_constant_expr_ast, -); break;
		case x::ast_t::FLOAT32_CONSTANT_EXP: UNARY_OP( float32_constant_expr_ast, -); break;
		case x::ast_t::FLOAT64_CONSTANT_EXP: UNARY_OP( float64_constant_expr_ast, -); break;
		}
		break;
	case x::operator_t::SIZEOF:
		switch ( value->type() )
		{
		case x::ast_t::BOOL_CONSTANT_EXP: UNARY_OP( bool_constant_expr_ast, sizeof ); break;
		case x::ast_t::INT32_CONSTANT_EXP: UNARY_OP( int32_constant_expr_ast, sizeof ); break;
		case x::ast_t::UINT32_CONSTANT_EXP: UNARY_OP( uint32_constant_expr_ast, sizeof ); break;
		case x::ast_t::INT64_CONSTANT_EXP: UNARY_OP( int64_constant_expr_ast, sizeof ); break;
		case x::ast_t::UINT64_CONSTANT_EXP: UNARY_OP( uint64_constant_expr_ast, sizeof ); break;
		case x::ast_t::FLOAT32_CONSTANT_EXP: UNARY_OP( float32_constant_expr_ast, sizeof ); break;
		case x::ast_t::FLOAT64_CONSTANT_EXP: UNARY_OP( float64_constant_expr_ast, sizeof ); break;
		}
		break;
	}

	return value;

#undef UNARY_OP
}

x::constant_expr_ast_ptr x::semantics_analysis_visitor::calc_binary_constant( const x::binary_expr_ast * ast ) const
{
	x::constant_expr_ast_ptr value;

	auto left = calc_constant( ast->get_left().get() );
	auto right = calc_constant( ast->get_right().get() );

	switch ( ast->get_op() )
	{
	case x::operator_t::LOR:
	{
		switch ( left->type() )
		{
		case x::ast_t::BOOL_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() || right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::LAND:
	{
		switch ( left->type() )
		{
		case x::ast_t::BOOL_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() && right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::NOT_EQUAL:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() != right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::EQUAL:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() == right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::COMPARE:
	{
		switch ( left->type() )
		{
		case x::ast_t::BOOL_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( x_compare( left_expr->get_value(), right_expr->get_value() ) );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::LARG_EQUAL:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::LESS_EQUAL:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() <= right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::LARG:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() > right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::LESS:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::bool_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() < right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}

	case x::operator_t::OR:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() | right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::XOR:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() ^ right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::AND:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() & right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::RIGHT_SHIFT:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() >> right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::LEFT_SHIFT:
	{
		switch ( left->type() )
		{
		case x::ast_t::BOOL_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() << right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::SUB:
	{
		switch ( left->type() )
		{
		case x::ast_t::BOOL_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() - right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::ADD:
	{
		switch ( left->type() )
		{
		case x::ast_t::BOOL_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() + right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::MOD:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() % right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::DIV:
	{
		switch ( left->type() )
		{
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() / right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	case x::operator_t::MUL:
	{
		switch ( left->type() )
		{
		case x::ast_t::BOOL_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::INT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::int64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::UINT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::uint64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT32_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float32_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		case x::ast_t::FLOAT64_CONSTANT_EXP:
		{
			auto left_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( left );

			switch ( right->type() )
			{
			case x::ast_t::BOOL_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::bool_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::INT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::int64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::UINT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::uint64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT32_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float32_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			case x::ast_t::FLOAT64_CONSTANT_EXP:
			{
				auto right_expr = std::static_pointer_cast<x::float64_constant_expr_ast>( right );

				auto expr = std::make_shared<x::float64_constant_expr_ast>();
				expr->set_location( ast->get_location() );
				expr->set_value( left_expr->get_value() * right_expr->get_value() );
				value = expr;

				break;
			}
			}

			break;
		}
		break;
		}

		break;
	}
	}

	return value;
}

bool x::semantics_analysis_visitor::is_virtual_function( const x::decl_ast * owner, const x::function_decl_ast * ast ) const
{
	static constexpr auto find_func = []( const auto & cls, std::string_view funcname ) -> x::function_decl_ast_ptr
	{
		auto it = std::find_if( cls->get_functions().begin(), cls->get_functions().end(), [&]( const auto & val )
		{
			return funcname == val->get_name();
		} );
		return it != cls->get_functions().end() ? *it : nullptr;
	};

	if ( owner->type() == x::ast_t::CLASS_DECL )
	{
		auto cls = static_cast<const x::class_decl_ast *>( owner );

		if ( cls != ast->get_parent().get() )
		{
			if ( auto func = find_func( cls, ast->get_name() ) )
			{
				return func->get_is_virtual() || func->get_is_override();
			}
		}

		for ( const auto & it : cls->get_interfaces() )
		{
			if ( is_virtual_function( static_cast<const x::decl_ast *>( symbols()->find_ast( it ).get() ), ast ) )
			{
				return true;
			}
		}

		if ( is_virtual_function( static_cast<const x::decl_ast *>( symbols()->find_ast( cls->get_base() ).get() ), ast ) )
		{
			return true;
		}
	}
	else if ( owner->type() == x::ast_t::TEMPLATE_DECL )
	{
		auto cls = static_cast<const x::template_decl_ast *>( owner );

		if ( cls != ast->get_parent().get() )
		{
			if ( auto func = find_func( cls, ast->get_name() ) )
			{
				return func->get_is_virtual() || func->get_is_override();
			}
		}

		for ( const auto & it : cls->get_interfaces() )
		{
			if ( is_virtual_function( static_cast<const x::decl_ast *>( symbols()->find_ast( it ).get() ), ast ) )
			{
				return true;
			}
		}

		if ( is_virtual_function( static_cast<const x::decl_ast *>( symbols()->find_ast( cls->get_base() ).get() ), ast ) )
		{
			return true;
		}
	}
	else if ( owner->type() == x::ast_t::INTERFACE_DECL )
	{
		auto interface = static_cast<const x::interface_decl_ast *>( owner );
		if ( auto func = find_func( interface, ast->get_name() ) )
		{
			return func->get_is_virtual() || func->get_is_override();
		}
	}

	return false;
}

#ifdef _MSC_VER
#pragma warning(  pop  ) 
#endif
