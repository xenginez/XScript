#include "grammar.h"

#include <bit>
#include <regex>

#include "exception.h"

#define BEG_VERIFYS( ... ) for( bool is_sucsess = false; !is_sucsess; ) { switch ( verify( { __VA_ARGS__ } ).type ) {
#define END_VERIFYS() default: is_sucsess = true; break; } }

x::grammar::grammar()
{
}

x::grammar::~grammar()
{
}

x::unit_ast_ptr x::grammar::parse( std::istream & stream, std::string_view name, const std::map<std::string, x::token_t> & tokens )
{
    _stream = &stream;
    _tokenmap = &tokens;
    _location.file = name;
    _location.line = 0;
    _location.column = 0;

    return unit();
}

x::unit_ast_ptr x::grammar::unit()
{
    auto ast = std::make_shared<x::unit_ast>();
    ast->location = _location;

    auto tk = lookup();
    while( tk.type != x::token_t::TK_EOF )
    {
        switch ( tk.type )
        {
        case x::token_t::TK_IMPORT:
            ast->imports.emplace_back( import() );
            break;
        case x::token_t::TK_NAMESPACE:
            ast->namespaces.emplace_back( namespace_decl() );
            break;
        case x::token_t::TK_SEMICOLON:
            break;
        }

        tk = lookup();
    }

    return ast;
}

x::type_ast_ptr x::grammar::type()
{
    x::type_ast_ptr ast;

    auto location = _location;

    bool is_ref = false;
    bool is_const = false;

    BEG_VERIFYS( x::token_t::TK_CONST, x::token_t::TK_REF )
        case x::token_t::TK_CONST: ast->is_const = true; break;
        case x::token_t::TK_REF: ast->is_ref = true; break;
    END_VERIFYS()

    auto name = type_name();

    if ( verify( x::token_t::TK_LEFT_BRACKETS ) )
    {
        auto func_type = std::make_shared<x::func_type_ast>();

        do { func_type->parameters.emplace_back( type() ); } while ( verify( x::token_t::TK_COMMA ) );

        verify( x::token_t::TK_RIGHT_BRACKETS );

        ast = func_type;
    }
    else if ( verify( x::token_t::TK_LEFT_INDEX ) )
    {
        auto array_type = std::make_shared<x::array_type_ast>();

        do { array_type->layer++; } while ( verify( x::token_t::TK_COMMA ) );

        verify( x::token_t::TK_RIGHT_INDEX );

        ast = array_type;
    }
    else if ( verify( x::token_t::TK_LESS ) )
    {
        auto temp_type = std::make_shared<x::temp_type_ast>();

        do { temp_type->elements.emplace_back( type() ); } while ( verify( x::token_t::TK_COMMA ) );

        verify( x::token_t::TK_LARG );

        ast = temp_type;
    }
    else
    {
        ast = std::make_shared<x::type_ast>();
    }

    ast->location = location;
    ast->is_const = is_const;
    ast->is_ref = is_ref;
    ast->name = name;

    return ast;
}

x::import_ast_ptr x::grammar::import()
{
    validity( x::token_t::TK_IMPORT );

    auto ast = std::make_shared<x::import_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->path = validity( x::token_t::TK_LITERAL_STRING ).str;
    validity( x::token_t::TK_RIGHT_BRACKETS );

    return ast;
}

x::attribute_ast_ptr x::grammar::attribute()
{
    validity( x::token_t::TK_ATTRIBUTE );

    auto ast = std::make_shared<x::attribute_ast>();
    ast->location = _location;

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]()
    {
        auto key = validity( x::token_t::TK_IDENTIFIER ).str;
        validity( x::token_t::TK_ASSIGN );
        auto val = validity( x::token_t::TK_LITERAL_STRING ).str;
        ast->attributes.insert( { key, val } );
    } );

    return ast;
}

x::enum_decl_ast_ptr x::grammar::enum_decl()
{
    validity( x::token_t::TK_ENUM );

    auto ast = std::make_shared<x::enum_decl_ast>();
    ast->location = _location;

    ast->name = validity( x::token_t::TK_IDENTIFIER ).str;

    verify_list( x::token_t::TK_LEFT_CURLY_BRACES, x::token_t::TK_RIGHT_CURLY_BRACES, x::token_t::TK_COMMA, [&]()
    {
        auto element = std::make_shared<x::element_decl_ast>();
        element->location = _location;

        element->access = x::access_t::PUBLIC;
        element->attr = attribute();
        element->name = validity( x::token_t::TK_IDENTIFIER ).str;
        element->value = expr_stat();

        ast->elements.emplace_back( element );

    } );

    return ast;
}

x::class_decl_ast_ptr x::grammar::class_decl()
{
    validity( x::token_t::TK_CLASS );

    auto ast = std::make_shared<x::class_decl_ast>();
    ast->location = _location;

    ast->name = validity( x::token_t::TK_IDENTIFIER ).str;

    if ( verify( x::token_t::TK_TYPECAST ) )
        ast->base = type();

    x::access_t acce = x::access_t::PRIVATE;
    x::attribute_ast_ptr attr = nullptr;
    verify_list( x::token_t::TK_LEFT_CURLY_BRACES, x::token_t::TK_RIGHT_CURLY_BRACES, x::token_t::TK_SEMICOLON, [&]()
    {
        if ( lookup().type == x::token_t::TK_ATTRIBUTE )
            attr = attribute();
        else
            attr = nullptr;

        acce = access();

        switch ( lookup().type )
        {
        case x::token_t::TK_USING:
        {
            auto decl = using_decl();
            decl->attr = attr;
            decl->access = acce;
            ast->usings.emplace_back( decl );
        }
        break;
        case x::token_t::TK_VARIABLE:
        {
            auto decl = variable_decl();
            decl->attr = attr;
            decl->access = acce;
            ast->variables.emplace_back( decl );
        }
        break;
        case x::token_t::TK_FUNCTION:
        {
            auto decl = function_decl();
            decl->attr = attr;
            decl->access = acce;
            ast->functions.emplace_back( decl );
        }
        break;
        case x::token_t::TK_SEMICOLON:
            next();
            break;
        default:
            XTHROW( x::syntax_exception, true, "", _location );
            break;
        }
    } );

    return ast;
}

x::using_decl_ast_ptr x::grammar::using_decl()
{
    validity( x::token_t::TK_USING );

    auto ast = std::make_shared<x::using_decl_ast>();
    ast->location = _location;

    ast->name = validity( x::token_t::TK_IDENTIFIER ).str;

    validity( x::token_t::TK_ASSIGN );

    ast->retype = type();

    return ast;
}

x::template_decl_ast_ptr x::grammar::template_decl()
{
    validity( x::token_t::TK_TEMPLATE );

    auto ast = std::make_shared<x::template_decl_ast>();
    ast->location = _location;

    ast->name = validity( x::token_t::TK_IDENTIFIER ).str;

    verify_list( x::token_t::TK_LEFT_INDEX, x::token_t::TK_RIGHT_INDEX, x::token_t::TK_COMMA, [&]() -> bool
    {
        x::type_ast_ptr t = type();

        if ( !verify( x::token_t::TK_VARIADIC_SIGN ) )
            ast->elements.emplace_back( t );
        else
        {
            auto l = std::make_shared<x::list_type_ast>();
            l->name = t->name;
            ast->elements.emplace_back( l );
            return true;
        }

        return false;
    } );

    if ( verify( x::token_t::TK_WHERE ) )
        ast->where = compound_stat();

    if ( verify( x::token_t::TK_TYPECAST ) )
        ast->base = type();

    verify_list( x::token_t::TK_LEFT_CURLY_BRACES, x::token_t::TK_RIGHT_CURLY_BRACES, x::token_t::TK_COMMA, [&]()
    {
        auto attr = attribute();
        auto acce = access();

        switch ( lookup().type )
        {
        case x::token_t::TK_USING:
        {
            auto decl = using_decl();
            decl->attr = attr;
            decl->access = acce;
            ast->usings.emplace_back( decl );
        }
        break;
        case x::token_t::TK_VARIABLE:
        {
            auto decl = variable_decl();
            decl->attr = attr;
            decl->access = acce;
            ast->variables.emplace_back( decl );
        }
        break;
        case x::token_t::TK_FUNCTION:
        {
            auto decl = function_decl();
            decl->attr = attr;
            decl->access = acce;
            ast->functions.emplace_back( decl );
        }
        break;
        case x::token_t::TK_SEMICOLON:
            next();
            break;
        default:
            XTHROW( x::syntax_exception, true, "", _location );
            break;
        }
    } );

    return ast;
}

x::variable_decl_ast_ptr x::grammar::variable_decl()
{
    validity( x::token_t::TK_VARIABLE );

    auto ast = std::make_shared<x::variable_decl_ast>();
    ast->location = _location;

    switch ( verify( { x::token_t::TK_LOCAL, x::token_t::TK_STATIC, x::token_t::TK_THREAD } ).type )
    {
    case x::token_t::TK_LOCAL: ast->is_local = true; break;
    case x::token_t::TK_STATIC: ast->is_static = true; break;
    case x::token_t::TK_THREAD: ast->is_thread = true; break;
    }

    ast->name = validity( x::token_t::TK_IDENTIFIER ).str;

    if ( verify( x::token_t::TK_TYPECAST ) )
        ast->value_type = type();
    else
        ast->value_type = any_type();

    if ( verify( x::token_t::TK_ASSIGN ) )
    {
        if ( lookup().type == x::token_t::TK_LEFT_CURLY_BRACES )
        {
            ast->init = initializers_exp();
        }
        else
        {
            auto init = std::make_shared<x::initializers_expr_ast>();
            init->location = _location;

            init->args.emplace_back( expr_stat() );

            ast->init = init;
        }
    }

    return ast;
}

x::function_decl_ast_ptr x::grammar::function_decl()
{
    validity( x::token_t::TK_FUNCTION );

    auto ast = std::make_shared<x::function_decl_ast>();
    ast->location = _location;

    switch ( verify( { x::token_t::TK_FINAL, x::token_t::TK_STATIC, x::token_t::TK_VIRTUAL, x::token_t::TK_OVERRIDE } ).type )
    {
    case x::token_t::TK_FINAL: ast->is_final = true; break;
    case x::token_t::TK_STATIC: ast->is_static = true; break;
    case x::token_t::TK_VIRTUAL: ast->is_virtual = true; break;
    case x::token_t::TK_OVERRIDE: ast->is_virtual = true; break;
    }

    BEG_VERIFYS( x::token_t::TK_CONST, x::token_t::TK_ASYNC )
        case x::token_t::TK_CONST: ast->is_const = true; break;
        case x::token_t::TK_ASYNC: ast->is_async = true; break;
    END_VERIFYS()

    ast->name = validity( x::token_t::TK_IDENTIFIER ).str;

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]() -> bool
    {
        if ( ast->parameters.emplace_back( parameter_decl() )->value_type->ast_type() == x::ast_t::LIST_TYPE )
            return true;
        return false;
    } );

    if ( verify( x::token_t::TK_FUNCTION_RESULT ) )
    {
        while ( 1 )
        {
            ast->results.emplace_back( type() );

            if ( !verify( x::token_t::TK_COMMA ) )
                break;
        }
    }

    if ( verify( x::token_t::TK_ASSIGN ) )
        ast->stat = extern_stat();
    else
        ast->stat = compound_stat();

    return ast;
}

x::parameter_decl_ast_ptr x::grammar::parameter_decl()
{
    auto parameter = std::make_shared<x::parameter_decl_ast>();
    parameter->location = _location;

    parameter->name = validity( x::token_t::TK_IDENTIFIER ).str;
    
    if ( verify( x::token_t::TK_TYPECAST ) )
        parameter->value_type = type();
    else if ( verify( x::token_t::TK_VARIADIC_SIGN ) )
        parameter->value_type = std::make_shared<x::list_type_ast>();
    else
        parameter->value_type = any_type();

    return parameter;
}

x::namespace_decl_ast_ptr x::grammar::namespace_decl()
{
    validity( x::token_t::TK_NAMESPACE );

    auto ast = std::make_shared<x::namespace_decl_ast>();
    ast->location = _location;

    ast->name = validity( x::token_t::TK_IDENTIFIER ).str;

    x::access_t acce = x::access_t::PRIVATE;
    x::decl_ast_ptr decl = nullptr;
    x::attribute_ast_ptr attr = nullptr;

    verify_list( x::token_t::TK_LEFT_CURLY_BRACES, x::token_t::TK_RIGHT_CURLY_BRACES, x::token_t::TK_SEMICOLON, [&]()
    {
        if( lookup().type == x::token_t::TK_ATTRIBUTE )
            attr = attribute();
        else
            attr = nullptr;

        acce = access();

        switch ( lookup().type )
        {
        case x::token_t::TK_ENUM:
            decl = enum_decl();
            break;
        case x::token_t::TK_CLASS:
            decl = class_decl();
            break;
        case x::token_t::TK_USING:
            decl = using_decl();
            break;
        case x::token_t::TK_TEMPLATE:
            decl = template_decl();
            break;
        case x::token_t::TK_NAMESPACE:
            decl = namespace_decl();
            break;
        case x::token_t::TK_SEMICOLON:
            next();
            break;
        default:
            XTHROW( x::syntax_exception, true, "", _location );
            break;
        }

        if ( decl )
        {
            decl->attr = std::move( attr );
            decl->access = acce;
            ast->members.emplace_back( std::move( decl ) );

            acce = x::access_t::PRIVATE;
        }
    } );

    return ast;
}

x::stat_ast_ptr x::grammar::stat()
{
    switch ( lookup().type )
    {
    case x::token_t::TK_SEMICOLON:
        return nullptr;
    case x::token_t::TK_LEFT_CURLY_BRACES:
        return compound_stat();
    case x::token_t::TK_IF:
        return if_stat();
    case x::token_t::TK_WHILE:
        return while_stat();
    case x::token_t::TK_FOR:
        return for_stat();
    case x::token_t::TK_FOREACH:
        return foreach_stat();
    case x::token_t::TK_SWITCH:
        return switch_stat();
    case x::token_t::TK_NEW:
        return new_stat();
    case x::token_t::TK_AWAIT:
        return await_stat();
    case x::token_t::TK_YIELD:
        return yield_stat();
    case x::token_t::TK_BREAK:
        return break_stat();
    case x::token_t::TK_RETURN:
        return return_stat();
    case x::token_t::TK_CONTINUE:
        return continue_stat();
    case x::token_t::TK_VARIABLE:
        return local_stat();
    default:
        return expr_stat();
    }
}

x::extern_stat_ast_ptr x::grammar::extern_stat()
{
    validity( x::token_t::TK_EXTERN );

    auto ast = std::make_shared<x::extern_stat_ast>();
    ast->location = _location;

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]()
    {
        ast->libname = validity( x::token_t::TK_LITERAL_STRING ).str;
        validity( x::token_t::TK_COMMA );
        ast->funcname = validity( x::token_t::TK_LITERAL_STRING ).str;
    } );

    validity( x::token_t::TK_SEMICOLON );

    return ast;
}

x::compound_stat_ast_ptr x::grammar::compound_stat()
{
    validity( x::token_t::TK_LEFT_CURLY_BRACES );

    auto ast = std::make_shared<x::compound_stat_ast>();
    ast->location = _location;

    while ( !verify( x::token_t::TK_RIGHT_CURLY_BRACES ) )
    {
        auto s = stat();

        if ( s )
            ast->stats.emplace_back( s );

        if ( lookup().type == x::token_t::TK_SEMICOLON )
            next();
    }

    return ast;
}

x::await_stat_ast_ptr x::grammar::await_stat()
{
    validity( x::token_t::TK_AWAIT );

    auto ast = std::make_shared<x::await_stat_ast>();
    ast->location = _location;

    ast->exp = expr_stat();

    return ast;
}

x::yield_stat_ast_ptr x::grammar::yield_stat()
{
    validity( x::token_t::TK_YIELD );

    auto ast = std::make_shared<x::yield_stat_ast>();
    ast->location = _location;

    if ( !verify( x::token_t::TK_SEMICOLON ) )
        ast->exp = expr_stat();

    verify( x::token_t::TK_SEMICOLON );

    return ast;
}

x::new_stat_ast_ptr x::grammar::new_stat()
{
    validity( x::token_t::TK_NEW );

    auto ast = std::make_shared<x::new_stat_ast>();
    ast->location = _location;

    ast->newtype = type();

    if ( lookup().type == x::token_t::TK_LEFT_CURLY_BRACES )
    {
        ast->init = initializers_exp();
    }

    return ast;
}

x::if_stat_ast_ptr x::grammar::if_stat()
{
    validity( x::token_t::TK_IF );

    auto ast = std::make_shared<x::if_stat_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->cond = expr_stat();
    validity( x::token_t::TK_RIGHT_BRACKETS );

    ast->then_stat = stat();

    if ( verify( x::token_t::TK_ELSE ) )
        ast->else_stat = stat();

    return ast;
}

x::while_stat_ast_ptr x::grammar::while_stat()
{
    validity( x::token_t::TK_WHILE );

    auto ast = std::make_shared<x::while_stat_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->cond = expr_stat();
    validity( x::token_t::TK_RIGHT_BRACKETS );

    ast->stat = stat();

    return ast;
}

x::for_stat_ast_ptr x::grammar::for_stat()
{
    validity( x::token_t::TK_FOR );

    auto ast = std::make_shared<x::for_stat_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->init = stat();
    validity( x::token_t::TK_SEMICOLON );
    ast->cond = expr_stat();
    validity( x::token_t::TK_SEMICOLON );
    ast->step = expr_stat();
    validity( x::token_t::TK_RIGHT_BRACKETS );

    ast->stat = stat();

    return ast;
}

x::foreach_stat_ast_ptr x::grammar::foreach_stat()
{
    validity( x::token_t::TK_FOREACH );

    auto ast = std::make_shared<x::foreach_stat_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->item = stat();
    validity( x::token_t::TK_TYPECAST );
    ast->collection = expr_stat();
    validity( x::token_t::TK_RIGHT_BRACKETS );

    ast->stat = stat();

    return ast;
}

x::switch_stat_ast_ptr x::grammar::switch_stat()
{
    validity( x::token_t::TK_SWITCH );

    auto ast = std::make_shared<x::switch_stat_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->expr = expr_stat();
    validity( x::token_t::TK_RIGHT_BRACKETS );

    validity( x::token_t::TK_LEFT_CURLY_BRACES );
    {
        while ( verify( x::token_t::TK_CASE ) )
        {
            auto expr = const_exp();
            validity( x::token_t::TK_TYPECAST );
            auto comb = compound_stat();

            ast->cases.push_back( { expr, comb } );
        }

        if ( verify( x::token_t::TK_DEFAULT ) )
        {
            validity( x::token_t::TK_TYPECAST );

            ast->defult = compound_stat();
        }
    }
    validity( x::token_t::TK_RIGHT_CURLY_BRACES );

    return ast;
}

x::break_stat_ast_ptr x::grammar::break_stat()
{
    validity( x::token_t::TK_BREAK );

    auto ast = std::make_shared<x::break_stat_ast>();
    ast->location = _location;

    return ast;
}

x::return_stat_ast_ptr x::grammar::return_stat()
{
    validity( x::token_t::TK_RETURN );

    auto ast = std::make_shared<x::return_stat_ast>();
    ast->location = _location;

    if ( !verify( x::token_t::TK_SEMICOLON ) )
    {
        do
        {
            ast->exprs.emplace_back( expr_stat() );
        } while ( verify( x::token_t::TK_COMMA ) );
    }

    return ast;
}

x::continue_stat_ast_ptr x::grammar::continue_stat()
{
    validity( x::token_t::TK_CONTINUE );

    auto ast = std::make_shared<x::continue_stat_ast>();
    ast->location = _location;

    return ast;
}

x::local_stat_ast_ptr x::grammar::local_stat()
{
    validity( x::token_t::TK_VARIABLE );

    auto ast = std::make_shared<x::local_stat_ast>();
    ast->location = _location;

    switch ( verify( { x::token_t::TK_LOCAL, x::token_t::TK_STATIC, x::token_t::TK_THREAD } ).type )
    {
    case x::token_t::TK_LOCAL: ast->is_local = true; break;
    case x::token_t::TK_STATIC: ast->is_static = true; break;
    case x::token_t::TK_THREAD: ast->is_thread = true; break;
    }

    ast->name = validity( x::token_t::TK_IDENTIFIER ).str;

    if ( verify( x::token_t::TK_TYPECAST ) )
        ast->value_type = type();
    else
        ast->value_type = any_type();

    if ( verify( x::token_t::TK_ASSIGN ) )
    {
        if ( lookup().type == x::token_t::TK_LEFT_CURLY_BRACES )
        {
            ast->init = initializers_exp();
        }
        else
        {
            auto init = std::make_shared<x::initializers_expr_ast>();
            init->location = _location;

            init->args.emplace_back( expr_stat() );

            ast->init = init;
        }
    }


    return ast;
}

x::expr_stat_ast_ptr x::grammar::expr_stat()
{
    return assignment_exp();
}

x::expr_stat_ast_ptr x::grammar::assignment_exp()
{
    x::expr_stat_ast_ptr left = logical_or_exp();

    if ( left != nullptr )
    {
        if ( lookup().type >= x::token_t::TK_ASSIGN && lookup().type <= x::token_t::TK_XOR_ASSIGN )
        {
            auto ast = std::make_shared<x::assignment_expr_ast>();
            ast->location = _location;

            ast->token = next().type;
            ast->left = left;
            ast->right = assignment_exp();

            return ast;
        }
        else
        {
            return left;
        }
    }

    return logical_or_exp();
}

x::expr_stat_ast_ptr x::grammar::logical_or_exp()
{
    x::expr_stat_ast_ptr ast = logical_and_exp();

    while ( verify( x::token_t::TK_LOR ) )
    {
        auto exp = std::make_shared<x::logical_or_expr_ast>();
        exp->location = _location;

        exp->token = x::token_t::TK_LOR;
        exp->left = ast;
        exp->right = logical_and_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::logical_and_exp()
{
    x::expr_stat_ast_ptr ast = or_exp();

    while ( verify( x::token_t::TK_LAND ) )
    {
        auto exp = std::make_shared<x::logical_and_expr_ast>();
        exp->location = _location;

        exp->token = x::token_t::TK_LAND;
        exp->left = ast;
        exp->right = or_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::or_exp()
{
    x::expr_stat_ast_ptr ast = xor_exp();

    while ( verify( x::token_t::TK_OR ) )
    {
        auto exp = std::make_shared<x::or_expr_ast>();
        exp->location = _location;

        exp->token = x::token_t::TK_OR;
        exp->left = ast;
        exp->right = xor_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::xor_exp()
{
    x::expr_stat_ast_ptr ast = and_exp();

    while ( verify( x::token_t::TK_XOR ) )
    {
        auto exp = std::make_shared<x::xor_expr_ast>();
        exp->location = _location;

        exp->token = x::token_t::TK_XOR;
        exp->left = ast;
        exp->right = and_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::and_exp()
{
    x::expr_stat_ast_ptr ast = compare_exp();

    while ( verify( x::token_t::TK_AND ) )
    {
        auto exp = std::make_shared<x::and_expr_ast>();
        exp->location = _location;

        exp->token = x::token_t::TK_AND;
        exp->left = ast;
        exp->right = compare_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::compare_exp()
{
    x::expr_stat_ast_ptr ast = shift_exp();

    while ( lookup().type >= x::token_t::TK_EQUAL && lookup().type <= x::token_t::TK_LARG_OR_EQUAL )
    {
        auto exp = std::make_shared<x::compare_expr_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->left = ast;
        exp->right = shift_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::shift_exp()
{
    x::expr_stat_ast_ptr ast = add_exp();

    while ( lookup().type == x::token_t::TK_LEFT_SHIFT || lookup().type == x::token_t::TK_RIGHT_SHIFT )
    {
        auto exp = std::make_shared<x::shift_expr_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->left = ast;
        exp->right = add_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::add_exp()
{
    x::expr_stat_ast_ptr ast = mul_exp();

    while ( lookup().type == x::token_t::TK_ADD || lookup().type == x::token_t::TK_SUB )
    {
        auto exp = std::make_shared<x::add_expr_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->left = ast;
        exp->right = mul_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::mul_exp()
{
    x::expr_stat_ast_ptr ast = as_exp();

    while ( lookup().type == x::token_t::TK_MUL || lookup().type == x::token_t::TK_DIV || lookup().type == x::token_t::TK_MOD )
    {
        auto exp = std::make_shared<x::mul_expr_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->left = ast;
        exp->right = as_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::as_exp()
{
    x::expr_stat_ast_ptr ast = is_exp();

    while ( verify( x::token_t::TK_AS ) )
    {
        auto exp = std::make_shared<x::as_expr_ast>();
        exp->location = _location;

        exp->value = ast;
        exp->cast_type = type();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::is_exp()
{
    x::expr_stat_ast_ptr ast = sizeof_exp();

    while ( verify( x::token_t::TK_IS ) )
    {
        auto exp = std::make_shared<x::is_expr_ast>();
        exp->location = _location;

        exp->value = ast;
        exp->cast_type = type();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::sizeof_exp()
{
    x::expr_stat_ast_ptr ast = typeof_exp();

    while ( verify( x::token_t::TK_LEFT_BRACKETS ) )
    {
        auto exp = std::make_shared<x::sizeof_expr_ast>();
        exp->location = _location;

        exp->value = ast;

        ast = exp;

        validity( x::token_t::TK_RIGHT_BRACKETS );
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::typeof_exp()
{
    x::expr_stat_ast_ptr ast = index_exp();

    while ( verify( x::token_t::TK_LEFT_BRACKETS ) )
    {
        auto exp = std::make_shared<x::typeof_expr_ast>();
        exp->location = _location;

        exp->value = ast;

        ast = exp;

        validity( x::token_t::TK_RIGHT_BRACKETS );
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::index_exp()
{
    x::expr_stat_ast_ptr ast = invoke_exp();

    while ( verify( x::token_t::TK_LEFT_INDEX ) )
    {
        auto exp = std::make_shared<x::index_expr_ast>();
        exp->location = _location;

        exp->left = ast;
        exp->right = invoke_exp();

        validity( x::token_t::TK_RIGHT_INDEX );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::invoke_exp()
{
    x::expr_stat_ast_ptr ast = member_exp();

    while ( lookup().type == x::token_t::TK_LEFT_BRACKETS )
    {
        auto exp = std::make_shared<x::invoke_expr_ast>();
        exp->location = _location;

        exp->left = ast;
        exp->right = arguments_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::member_exp()
{
    x::expr_stat_ast_ptr ast = unary_exp();

    while ( verify( x::token_t::TK_MEMBER_POINT ) )
    {
        auto exp = std::make_shared<x::member_expr_ast>();
        exp->location = _location;

        exp->left = ast;
        exp->right = unary_exp();

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::unary_exp()
{
    x::expr_stat_ast_ptr ast = nullptr;

    switch ( lookup().type )
    {
    case x::token_t::TK_INC:
    case x::token_t::TK_DEC:
    case x::token_t::TK_ADD:
    case x::token_t::TK_SUB:
    case x::token_t::TK_NOT:
    case x::token_t::TK_LNOT:
    case x::token_t::TK_SIZEOF:
    {
        auto exp = std::make_shared<x::unary_expr_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->exp = postfix_exp();

        ast = exp;
    }
    break;
    default:
        break;
    }

    if ( ast == nullptr )
        ast = postfix_exp();

    return ast;
}

x::expr_stat_ast_ptr x::grammar::postfix_exp()
{
    x::expr_stat_ast_ptr ast = primary_exp();

    switch ( lookup().type )
    {
    case x::token_t::TK_INC:
    case x::token_t::TK_DEC:
    {
        auto exp = std::make_shared<x::postfix_expr_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->exp = ast;

        ast = exp;
    }
    break;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::primary_exp()
{
    x::expr_stat_ast_ptr ast = nullptr;

    switch ( lookup().type )
    {
    case x::token_t::TK_FUNCTION:
        ast = closure_exp();
        break;
    case x::token_t::TK_TYPECAST:
        ast = typecast_exp();
        break;
    case x::token_t::TK_IDENTIFIER:
        ast = identifier_exp();
        break;
    case x::token_t::TK_LEFT_BRACKETS:
        ast = arguments_exp();
        break;
    case x::token_t::TK_LEFT_CURLY_BRACES:
        ast = initializers_exp();
        break;
    default:
        ast = const_exp();
        break;
    }

    return ast;
}

x::closure_expr_ast_ptr x::grammar::closure_exp()
{
    validity( x::token_t::TK_FUNCTION );

    auto ast = std::make_shared<x::closure_expr_ast>();
    ast->location = _location;

    ast->name = std::format( "{}_{}", "closure", x::time_2_stamp( std::chrono::system_clock::now() ) );

    if ( verify( x::token_t::TK_ASYNC ) ) ast->is_async = true;

    verify_list( x::token_t::TK_LEFT_INDEX, x::token_t::TK_RIGHT_INDEX, x::token_t::TK_COMMA, [&]()
    {
        ast->captures.emplace_back( identifier_exp() );
    } );

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]()
    {
        ast->parameters.emplace_back( parameter_decl() );
    } );

    if ( verify( x::token_t::TK_FUNCTION_RESULT ) )
    {
        while ( 1 )
        {
            ast->results.emplace_back( type() );

            if ( !verify( x::token_t::TK_COMMA ) )
                break;
        }
    }

    ast->stat = compound_stat();

    return ast;
}

x::typecast_expr_ast_ptr x::grammar::typecast_exp()
{
    validity( x::token_t::TK_TYPECAST );

    auto ast = std::make_shared<x::typecast_expr_ast>();
    ast->location = _location;

    ast->type = type();

    return ast;
}

x::arguments_expr_ast_ptr x::grammar::arguments_exp()
{
    auto ast = std::make_shared<x::arguments_expr_ast>();
    ast->location = _location;

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]()
    {
        ast->args.emplace_back( assignment_exp() );
    } );

    return ast;
}

x::identifier_expr_ast_ptr x::grammar::identifier_exp()
{
    auto ast = std::make_shared<x::identifier_expr_ast>();
    ast->location = _location;

    switch( lookup().type )
	{
	case x::token_t::TK_THIS:
	case x::token_t::TK_BASE:
	case x::token_t::TK_IDENTIFIER:
        ast->ident = next().str;
        break;
    default:
        XTHROW( x::syntax_exception, false, "", _location );
        break;
    }
    
    return ast;
}

x::initializers_expr_ast_ptr x::grammar::initializers_exp()
{
    auto ast = std::make_shared<x::initializers_expr_ast>();
    ast->location = _location;

    verify_list( x::token_t::TK_LEFT_CURLY_BRACES, x::token_t::TK_RIGHT_CURLY_BRACES, x::token_t::TK_COMMA, [&]()
    {
        ast->args.emplace_back( expr_stat() );
    } );

    return ast;
}

x::const_expr_ast_ptr x::grammar::const_exp()
{
    x::const_expr_ast_ptr ast = nullptr;

    switch ( lookup().type )
    {
    case x::token_t::TK_NULL:
        ast = null_const_exp();
        break;
    case x::token_t::TK_TRUE:
        ast = true_const_exp();
        break;
    case x::token_t::TK_FALSE:
        ast = false_const_exp();
        break;
    case x::token_t::TK_LITERAL_INT:
        ast = int_const_exp();
        break;
    case x::token_t::TK_LITERAL_FLOAT:
        ast = float_const_exp();
        break;
    case x::token_t::TK_LITERAL_STRING:
        ast = string_const_exp();
        break;
    }

    return ast;
}

x::null_const_expr_ast_ptr x::grammar::null_const_exp()
{
    validity( x::token_t::TK_NULL );

    auto ast = std::make_shared<x::null_const_expr_ast>();
    ast->location = _location;

    return ast;
}

x::bool_const_expr_ast_ptr x::grammar::true_const_exp()
{
    validity( x::token_t::TK_TRUE );

    auto ast = std::make_shared<x::bool_const_expr_ast>();
    ast->location = _location;

    ast->value = true;

    return ast;
}

x::bool_const_expr_ast_ptr x::grammar::false_const_exp()
{
    validity( x::token_t::TK_FALSE );

    auto ast = std::make_shared<x::bool_const_expr_ast>();
    ast->location = _location;

    ast->value = false;

    return ast;
}

x::int_const_expr_ast_ptr x::grammar::int_const_exp()
{
    x::int_const_expr_ast_ptr ast;

    auto str = validity( x::token_t::TK_LITERAL_INT ).str;

    int base = 10;
    x::uint64 u64 = 0;
    
    if ( str.find( "0x" ) <= 1 )
        base = 16;
    else if ( str.find( "0b" ) <= 1 )
        base = 2;
    else
        base = 10;

    if ( str[0] == '-' )
    {
        x::int64 i64 = 0;
        std::from_chars( str.c_str(), str.c_str() + str.size(), i64, base );

        if ( std::abs( i64 ) > std::numeric_limits<x::int32>::max() )
        {
            auto i_ast = std::make_shared<x::int64_const_expr_ast>();
            i_ast->location = _location;
            i_ast->value = i64;
            ast = i_ast;
        }
        else if ( std::abs( i64 ) > std::numeric_limits<x::int16>::max() )
        {
            auto i_ast = std::make_shared<x::int32_const_expr_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::int32>( i64 );
            ast = i_ast;
        }
        else if ( std::abs( i64 ) > std::numeric_limits<x::int8>::max() )
        {
            auto i_ast = std::make_shared<x::int16_const_expr_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::int16>( i64 );
            ast = i_ast;
        }
        else
        {
            auto i_ast = std::make_shared<x::int8_const_expr_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::int8>( i64 );
            ast = i_ast;
        }
    }
    else
    {
        x::uint64 u64 = 0;
        std::from_chars( str.c_str(), str.c_str() + str.size(), u64, base );

        if ( u64 > std::numeric_limits<x::uint32>::max() )
        {
            auto i_ast = std::make_shared<x::uint64_const_expr_ast>();
            i_ast->location = _location;
            i_ast->value = u64;
            ast = i_ast;
        }
        else if ( u64 > std::numeric_limits<x::uint16>::max() )
        {
            auto i_ast = std::make_shared<x::uint32_const_expr_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::uint32>( u64 );
            ast = i_ast;
        }
        else if ( u64 > std::numeric_limits<x::uint8>::max() )
        {
            auto i_ast = std::make_shared<x::uint16_const_expr_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::uint16>( u64 );
            ast = i_ast;
        }
        else
        {
            auto i_ast = std::make_shared<x::uint8_const_expr_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::uint8>( u64 );
            ast = i_ast;
        }
    }

    return ast;
}

x::float_const_expr_ast_ptr x::grammar::float_const_exp()
{
    x::float_const_expr_ast_ptr ast;

    auto str = validity( x::token_t::TK_LITERAL_INT ).str;

    auto beg = str.c_str(); if ( *beg == '-' ) ++beg;
    auto dom = str.c_str() + str.find( '.' );
    auto end = str.c_str() + str.size();

    x::uint64 mantissa = 0, exponent = 0;
    std::from_chars( beg, beg + std::distance( beg, dom ), mantissa );
    std::from_chars( dom + 1, dom + std::distance( dom, end ), exponent );

    int man_cnt = std::numeric_limits<x::uint64>::digits - std::countl_zero( mantissa );
    if ( man_cnt >= std::numeric_limits<x::float32>::digits || exponent >= std::numeric_limits<x::float32>::max_exponent )
    {
        auto f_ast = std::make_shared<x::float64_const_expr_ast>();
        f_ast->location = _location;
        std::from_chars( str.c_str(), str.c_str() + str.size(), f_ast->value );
        ast = f_ast;
    }
    else
    {
        auto f_ast = std::make_shared<x::float32_const_expr_ast>();
        f_ast->location = _location;
        std::from_chars( str.c_str(), str.c_str() + str.size(), f_ast->value );
        ast = f_ast;
    }

    return ast;
}

x::string_const_expr_ast_ptr x::grammar::string_const_exp()
{
    auto ast = std::make_shared<x::string_const_expr_ast>();
    ast->location = _location;

    ast->value = validity( x::token_t::TK_LITERAL_STRING ).str;

    return ast;
}

std::string x::grammar::type_name()
{
    std::string name;

    do
    {
        if ( !name.empty() ) name += ".";

        auto tk = lookup();
        switch ( tk.type )
        {
        case x::token_t::TK_ANY:
        case x::token_t::TK_VOID:
        case x::token_t::TK_BYTE:
        case x::token_t::TK_BOOL:
        case x::token_t::TK_INT8:
        case x::token_t::TK_INT16:
        case x::token_t::TK_INT32:
        case x::token_t::TK_INT64:
        case x::token_t::TK_UINT8:
        case x::token_t::TK_UINT16:
        case x::token_t::TK_UINT32:
        case x::token_t::TK_UINT64:
        case x::token_t::TK_FLOAT16:
        case x::token_t::TK_FLOAT32:
        case x::token_t::TK_FLOAT64:
        case x::token_t::TK_STRING:
        case x::token_t::TK_INTPTR:
        case x::token_t::TK_OBJECT:
        case x::token_t::TK_ARRAY:
        case x::token_t::TK_COROUTINE:
        case x::token_t::TK_IDENTIFIER:
            name += next().str;
            break;
        default:
            XTHROW( x::syntax_exception, true, "", _location );
            break;
        }
    } while ( verify( x::token_t::TK_MEMBER_POINT ) );

    return name;
}

x::type_ast_ptr x::grammar::any_type()
{
    auto ast = std::make_shared<x::type_ast>();
    ast->location = _location;
    ast->name = "any";
    return ast;
}

x::access_t x::grammar::access()
{
    x::access_t result = access_t::PRIVATE;

    while ( 1 )
    {
        if ( verify( x::token_t::TK_PUBLIC ) ) result = access_t::PUBLIC;
        else if ( verify( x::token_t::TK_PRIVATE ) ) result = access_t::PRIVATE;
        else if ( verify( x::token_t::TK_PROTECTED ) ) result = access_t::PROTECTED;
        else break;
    }

    return result;
}

x::token x::grammar::next()
{
    while ( std::isspace( peek() ) )
    {
        get();
    }

    x::token tk;
    tk.location = _location;
    tk.type = x::token_t::TK_EOF;

    while ( !_stream->eof() )
    {
        int c = get();

        if ( std::isspace( c ) ) // space \r \n \t \0 ' '
        {
            continue;
        }
        else if ( c == '-' && std::isdigit( peek() ) ) // number | float
        {
            push( tk.str, c );

            c = peek();
            while ( std::isdigit( c ) || c == '.' )
            {
                XTHROW( x::syntax_exception, tk.str.find( '.' ) != std::string::npos && c == '.', "", _location );

                push( tk.str, get() );

                c = peek();
            }
        }
        else if ( std::isdigit( c ) ) // number 1 233 0x123456 0b1101001
        {
            if ( c == '0' && std::tolower( peek() ) == 'x' )
            {
                // ignore 0x
                get();

                push( tk.str, '0' );
                push( tk.str, 'x' );

                c = peek();
                while ( std::isdigit( c ) || ( std::tolower( c ) >= 'a' && std::tolower( c ) <= 'f' ) )
                {
                    push( tk.str, get() );

                    c = peek();
                }

                tk.str = std::to_string( std::stoll( tk.str ) );
            }
            else if ( c == '0' && std::tolower( peek() ) == 'b' )
            {
                // ignore 0b
                get();

                push( tk.str, '0' );
                push( tk.str, 'b' );

                c = peek();
                while ( c == '0' || c == '1' )
                {
                    push( tk.str, get() );

                    c = peek();
                }

                int64_t n = 0;
                for ( int i = (int)tk.str.size() - 1; i >= 0; i-- )
                {
                    if ( tk.str[i] == 1 )
                        n |= ( int64_t( 1 ) << ( i ) );
                }
                tk.str = std::to_string( n );
            }
            else
            {
                push( tk.str, c );

                c = peek();
                while ( std::isdigit( c ) || c == '.' )
                {
                    XTHROW( x::syntax_exception, tk.str.find( '.' ) != std::string::npos && c == '.', "", _location );

                    push( tk.str, get() );

                    c = peek();
                }
            }

            tk.type = ( tk.str.find( '.' ) == std::string::npos ) ? x::token_t::TK_LITERAL_INT : x::token_t::TK_LITERAL_FLOAT;

            break;
        }
        else if ( c == '\"' ) // string "..."
        {
            while ( peek() != '\"' )
            {
                c = get();

                if ( c == '\\' )
                {
                    c = get();

                    switch ( peek() )
                    {
                    case '\'':
                        get();
                        c = '\'';
                        break;
                    case '\"':
                        get();
                        c = '\"';
                        break;
                    case '\\':
                        get();
                        c = '\\';
                        break;
                    case 'n':
                        get();
                        c = '\n';
                        break;
                    case 'r':
                        get();
                        c = '\r';
                        break;
                    case 't':
                        get();
                        c = '\t';
                        break;
                    default:
                        c = '\\';
                        break;
                    }
                }

                push( tk.str, c );
            }

            get();

            tk.type = x::token_t::TK_LITERAL_STRING;

            break;
        }
        else if ( c == 'R' && peek() == '\"' ) // raw string R"(...)"
        {
            get(); // "
            get(); // (

            while ( 1 )
            {
                auto cc = get();
                if ( cc == ')' && peek() == '\"' )
                    break;
                else
                    push( tk.str, cc );
            }

            get();

            tk.type = x::token_t::TK_LITERAL_STRING;

            break;
        }
        else if ( std::isalpha( c ) || c == '_' || c > 127 ) // identifier
        {
            push( tk.str, c );

            while ( std::isalnum( peek() ) || peek() == '_' || peek() > 127 ) push( tk.str, get() );

            auto it = _tokenmap->find( tk.str );
            tk.type = it != _tokenmap->end() ? it->second : x::token_t::TK_IDENTIFIER;

            break;
        }
        else if ( c == '/' && ( peek() == '/' || peek() == '*' ) ) // notations | div operator
        {
            if ( peek() == '/' ) // notation line
            {
                while ( get() != '\n' );

                continue;
            }
            else // notations
            {
                get(); // *

                while ( !( get() == '*' && peek() == '/' ) );

                get(); // /

                continue;
            }
        }
        else if ( std::ispunct( c ) ) // operator
        {
            push( tk.str, c );

            XTHROW( x::syntax_exception, _tokenmap->find( tk.str ) != _tokenmap->end(), "", _location );

            tk.type = _tokenmap->find( tk.str )->second;

            while ( std::ispunct( peek() ) )
            {
                auto s = tk.str; s.push_back( peek() );

                if ( _tokenmap->find( s ) == _tokenmap->end() )
                    break;

                push( tk.str, get() );
                tk.type = _tokenmap->find( tk.str )->second;
            }

            break;
        }
        else
        {
            XTHROW( x::lexical_exception, true, "", _location );
        }
    }

    return tk;
}

x::token x::grammar::lookup()
{
    auto pos = _stream->tellg();
    auto tk = next();
    _stream->seekg( pos );

    return tk;
}

bool x::grammar::verify( x::token_t k )
{
    if ( lookup().type == k )
    {
        next();
        return true;
    }

    return false;
}

x::token x::grammar::verify( std::initializer_list<x::token_t> list )
{
    auto tk = lookup();

    for ( auto k : list )
    {
        if ( tk.type == k )
        {
            next();
            return tk;
        }
    }

    return { x::token_t::TK_EOF, {}, _location };
}

x::token x::grammar::validity( x::token_t k )
{
    auto tk = lookup();
    XTHROW( x::lexical_exception, tk.type == k, "", _location );
    return next();
}

int x::grammar::get()
{
    int c = _stream->get();

    if ( ( c & 0b10000000 ) != 0 )
    {
             if ( ( c & 0b11111100 ) == 0b11111100 )
        {
            int shift = 5 * 6;
            c = ( c & 0b00000001 ) << shift; shift -= 2;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
        }
        else if ( ( c & 0b11111000 ) == 0b11111000 )
        {
            int shift = 4 * 6;
            c = ( c & 0b00000011 ) << shift; shift -= 3;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
        }
        else if ( ( c & 0b11110000 ) == 0b11110000 )
        {
            int shift = 3 * 6;
            c = ( c & 0b00000111 ) << shift; shift -= 4;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
        }
        else if ( ( c & 0b11100000 ) == 0b11100000 )
        {
            int shift = 2 * 6;
            c = ( c & 0b00001111 ) << shift; shift -= 5;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
        }
        else if ( ( c & 0b11000000 ) == 0b11000000 )
        {
            int shift = 1 * 6;
            c = ( c & 0b00011111 ) << shift; shift -= 6;
            c |= ( get() & 0x3F ) << shift; shift -= 6;
        }
    }

    return c;
}

int x::grammar::peek()
{
    auto pos = _stream->tellg();
    auto c = get();
    _stream->seekg( pos );

    return c;
}

void x::grammar::push( std::string & str, int c ) const
{
    if ( c > 0x03FFFFFF )
    {
        int shift = 5 * 6;
        str.push_back( 0b00000001 | ( ( c >> shift ) & 0x01 ) ); shift -= 2;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
    }
    else if ( c > 0x0010FFFF )
    {
        int shift = 4 * 6;
        str.push_back( 0b00000001 | ( ( c >> shift ) & 0x01 ) ); shift -= 3;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
    }
    else if ( c > 0x0000FFFF )
    {
        int shift = 2 * 6;
        str.push_back( 0b00000001 | ( ( c >> shift ) & 0x01 ) ); shift -= 4;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
    }
    else if ( c > 0x000007FF )
    {
        int shift = 1 * 6;
        str.push_back( 0b00000001 | ( ( c >> shift ) & 0x01 ) ); shift -= 5;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
    }
    else if ( c > 0x0000007F )
    {
        int shift = 1 * 6;
        str.push_back( 0b00000001 | ( ( c >> shift ) & 0x01 ) ); shift -= 6;
        str.push_back( 0b10000000 | ( ( c >> shift ) & 0x3F ) ); shift -= 6;
    }
    else
    {
        str.push_back( c );
    }
}
