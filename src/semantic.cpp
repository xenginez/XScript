#include "semantic.h"

#include "symbols.h"

void x::semantics_analyzer_visitor::analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast )
{
}

bool x::semantics_analyzer_visitor::is_constant( x::ast * ast )
{
	return false;
}

void x::expression_analyzer::analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast )
{
    if ( ast->type() == x::ast_t::UNARY_EXP )
    {

    }
    else if ( ast->type() == x::ast_t::BINRARY_EXP )
    {

    }

//    // �����۵�
//    if ( ast->is_binary_expression() )
//    {
//        auto left = ast->left();
//        auto right = ast->right();
//        if ( is_constant( left ) && is_constant( right ) )
//        {
//            // ִ�г����۵�
//            ast->set_value( evaluate_constant_expression( left, right, ast->op() ) );
//        }
//    }
//
//    // ���������
//    if ( ast->is_binary_expression() && ast->op() == x::token_type::divide )
//    {
//        auto right = ast->right();
//        if ( is_constant( right ) && evaluate_constant( right ) == 0 )
//        {
//            logger->error( ast->location(), "�������" );
//        }
//    }
}
