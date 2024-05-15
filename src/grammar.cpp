#include "grammar.h"

#include <bit>
#include <map>
#include <regex>

x::grammar::grammar( std::istream & stream, std::string_view name, const std::map<std::string, x::token_t> tokens )
    : _stream( stream.rdbuf() ), _tokenmap( tokens )
{
    _location.file = name;
}

x::grammar::~grammar()
{
}

x::unit_ast_ptr x::grammar::unit()
{
    auto ast = std::make_shared<x::unit_ast>();
    ast->location = _location;

    auto tk = lookup();
    do
    {
        switch ( tk.type )
        {
        case x::token_t::TK_IMPORT:
            ast->imports.emplace_back( import() );
            break;
        case x::token_t::TK_NAMESPACE:
            ast->namespaces.emplace_back( namespace_decl() );
            break;
        }
    } while ( tk.type != x::token_t::TK_EOF );

    return ast;
}

x::type_ast_ptr x::grammar::type()
{
    x::type_ast_ptr ast;

    auto location = _location;

    auto is_const = false;
    auto is_ref = false;
    switch ( verify( { x::token_t::TK_REF, x::token_t::TK_CONST } ).type )
    {
    case x::token_t::TK_REF:
        is_ref = true;
        break;
    case x::token_t::TK_CONST:
        is_const = true;
        break;
    }
    switch ( verify( { x::token_t::TK_REF, x::token_t::TK_CONST } ).type )
    {
    case x::token_t::TK_REF:
        is_ref = true;
        break;
    case x::token_t::TK_CONST:
        is_const = true;
        break;
    }

    auto name = type_name();

    if ( verify( x::token_t::TK_LESS ) )
    {
        auto temp_type = std::make_shared<x::temp_type_ast>();

        while ( !verify( x::token_t::TK_LARG ) )
        {
            temp_type->elements.push_back( type() );

            if ( !verify( x::token_t::TK_COMMA ) )
                break;
        }

        ast = temp_type;
    }
    else if ( verify( x::token_t::TK_LEFT_BRACKETS ) )
    {
        auto func_type = std::make_shared<x::func_type_ast>();

        while ( !verify( x::token_t::TK_RIGHT_BRACKETS ) )
        {
            func_type->parameters.push_back( type() );

            if ( !verify( x::token_t::TK_COMMA ) )
                break;
        }

        ast = func_type;
    }
    else if ( verify( x::token_t::TK_LEFT_INDEX ) )
    {
        auto array_type = std::make_shared<x::array_type_ast>();

        while ( !verify( x::token_t::TK_RIGHT_INDEX ) )
        {
            ++array_type->array_count;

            if ( !verify( x::token_t::TK_COMMA ) )
                break;
        }

        ast = array_type;
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

    ast->path = validity( x::token_t::TK_LITERAL_STRING ).str;

    return ast;
}

x::attribute_ast_ptr x::grammar::attribute()
{
    validity( x::token_t::TK_ATTRIBUTE );

    auto ast = std::make_shared<x::attribute_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_BRACKETS );
    while ( 1 )
    {
        auto key = validity( x::token_t::TK_IDENTIFIER ).str;
        validity( x::token_t::TK_ASSIGN );
        auto val = validity( x::token_t::TK_LITERAL_STRING ).str;
        ast->_attributes.insert( { key, val } );

        if ( !verify( x::token_t::TK_COMMA ) ) break;
    }
    validity( x::token_t::TK_RIGHT_BRACKETS );

    return ast;
}

x::enum_decl_ast_ptr x::grammar::enum_decl()
{
    validity( x::token_t::TK_ENUM );

    auto ast = std::make_shared<x::enum_decl_ast>();
    ast->location = _location;

    ast->name = validity( x::token_t::TK_IDENTIFIER ).str;

    validity( x::token_t::TK_LEFT_CURLY_BRACES );
    while ( !verify( x::token_t::TK_RIGHT_CURLY_BRACES ) )
    {
        auto element = std::make_shared<x::element_decl_ast>();
        element->location = _location;

        element->access = x::access_t::PUBLIC;
        element->attr = attribute();
        element->name = validity( x::token_t::TK_IDENTIFIER ).str;
        element->value = exp_stat();

        ast->elements.emplace_back( element );

        if ( !verify( x::token_t::TK_COMMA ) )
        {
            validity( x::token_t::TK_RIGHT_CURLY_BRACES );
            break;
        }
    }

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

    validity( x::token_t::TK_LEFT_CURLY_BRACES );
    do
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
            ASSERT( true, "" );
            break;
        }

    } while ( !verify( x::token_t::TK_RIGHT_CURLY_BRACES ) );

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

    validity( x::token_t::TK_LESS );
    while ( !verify( x::token_t::TK_LARG ) )
    {
        ast->elements.push_back( type() );

        if ( !verify( x::token_t::TK_COMMA ) )
            break;
    }

    if ( verify( x::token_t::TK_WHERE ) )
        ast->where = exp_stat();

    if ( verify( x::token_t::TK_TYPECAST ) )
        ast->base = type();

    validity( x::token_t::TK_LEFT_CURLY_BRACES );
    do
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
            ASSERT( true, "" );
            break;
        }

    } while ( !verify( x::token_t::TK_RIGHT_CURLY_BRACES ) );

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
        ast->value_type = type( "object" );

    if ( verify( x::token_t::TK_ASSIGN ) ) ast->init = initializers_exp();

    return ast;
}

x::function_decl_ast_ptr x::grammar::function_decl()
{
    validity( x::token_t::TK_FUNCTION );

    auto ast = std::make_shared<x::function_decl_ast>();
    ast->location = _location;

    auto tk = x::token_t::TK_EOF;

    do
    {
        tk = verify( { x::token_t::TK_CONST, x::token_t::TK_ASYNC } ).type;
        switch ( tk )
        {
        case x::token_t::TK_CONST:
            ast->is_const = true;
            break;
        case x::token_t::TK_ASYNC:
            ast->is_async = true;
            break;
        }
    } while ( tk != x::token_t::TK_EOF );
    switch ( verify( { x::token_t::TK_STATIC, x::token_t::TK_VIRTUAL, x::token_t::TK_OVERRIDE } ).type )
    {
    case x::token_t::TK_STATIC: ast->is_static = true; break;
    case x::token_t::TK_VIRTUAL: ast->is_virtual = true; break;
    case x::token_t::TK_OVERRIDE: ast->is_virtual = true; break;
    }
    do
    {
        tk = verify( { x::token_t::TK_CONST, x::token_t::TK_ASYNC } ).type;
        switch ( tk )
        {
        case x::token_t::TK_CONST:
            ast->is_const = true;
            break;
        case x::token_t::TK_ASYNC:
            ast->is_async = true;
            break;
        }
    } while ( tk != x::token_t::TK_EOF );

    ast->name = validity( x::token_t::TK_IDENTIFIER ).str;

    validity( x::token_t::TK_LEFT_BRACKETS );
    {
        do
        {
            ast->parameters.emplace_back( parameter_decl() );
        } while ( verify( x::token_t::TK_COMMA ) );
    }
    validity( x::token_t::TK_RIGHT_BRACKETS );

    if ( verify( x::token_t::TK_FUNCTION_RESULT ) )
        ast->result = type();
    else
        ast->result = type( "void" );

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
    else
        parameter->value_type = type( "object" );

    return parameter;
}

x::namespace_decl_ast_ptr x::grammar::namespace_decl()
{
    validity( x::token_t::TK_NAMESPACE );

    auto ast = std::make_shared<x::namespace_decl_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_CURLY_BRACES );
    do
    {
        x::decl_ast_ptr decl = nullptr;
        x::attribute_ast_ptr attr = nullptr;
        x::access_t acce = x::access_t::PRIVATE;

        switch ( lookup().type )
        {
        case x::token_t::TK_ENUM:
            attr = attribute();
            acce = access();
            decl = enum_decl();
            break;
        case x::token_t::TK_CLASS:
            attr = attribute();
            acce = access();
            decl = class_decl();
            break;
        case x::token_t::TK_USING:
            attr = attribute();
            acce = access();
            decl = using_decl();
            break;
        case x::token_t::TK_TEMPLATE:
            attr = attribute();
            acce = access();
            decl = template_decl();
            break;
        case x::token_t::TK_NAMESPACE:
            attr = attribute();
            acce = x::access_t::PUBLIC;
            decl = namespace_decl();
            break;
        case x::token_t::TK_SEMICOLON:
            next();
            break;
        default:
            ASSERT( true, "" );
            break;
        }

        if ( decl )
        {
            decl->attr = attr;
            decl->access = acce;
            ast->members.emplace_back( decl );
        }

    } while ( !verify( x::token_t::TK_RIGHT_CURLY_BRACES ) );

    return ast;
}

x::stat_ast_ptr x::grammar::stat()
{
    switch ( lookup().type )
    {
    case x::token_t::TK_SEMICOLON:
        return std::make_shared<x::empty_stat_ast>();
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
    case x::token_t::TK_NEW:
        return new_stat();
    case x::token_t::TK_TRY:
        return try_stat();
    case x::token_t::TK_THROW:
        return throw_stat();
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
        return exp_stat();
    }
}

x::extern_stat_ast_ptr x::grammar::extern_stat()
{
    validity( x::token_t::TK_EXTERN );

    auto ast = std::make_shared<x::extern_stat_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_BRACKETS );
    {
        ast->libname = validity( x::token_t::TK_LITERAL_STRING ).str;
        validity( x::token_t::TK_COMMA );
        ast->funcname = validity( x::token_t::TK_LITERAL_STRING ).str;
    }
    validity( x::token_t::TK_RIGHT_BRACKETS );

    validity( x::token_t::TK_SEMICOLON );

    return ast;
}

x::compound_stat_ast_ptr x::grammar::compound_stat()
{
    validity( x::token_t::TK_LEFT_CURLY_BRACES );

    auto ast = std::make_shared<x::compound_stat_ast>();
    ast->location = _location;

    while ( verify( x::token_t::TK_RIGHT_CURLY_BRACES ) )
    {
        ast->stats.push_back( stat() );
    }

    validity( x::token_t::TK_RIGHT_CURLY_BRACES );

    return ast;
}

x::await_stat_ast_ptr x::grammar::await_stat()
{
    validity( x::token_t::TK_AWAIT );

    auto ast = std::make_shared<x::await_stat_ast>();
    ast->location = _location;

    ast->exp = exp_stat();

    return ast;
}

x::yield_stat_ast_ptr x::grammar::yield_stat()
{
    validity( x::token_t::TK_YIELD );

    auto ast = std::make_shared<x::yield_stat_ast>();
    ast->location = _location;

    if ( !verify( x::token_t::TK_SEMICOLON ) )
        ast->exp = exp_stat();

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

x::try_stat_ast_ptr x::grammar::try_stat()
{
    validity( x::token_t::TK_TRY );

    auto ast = std::make_shared<x::try_stat_ast>();
    ast->location = _location;

    ast->body = compound_stat();

    while ( verify( x::token_t::TK_CATCH ) )
    {
        ast->catchs.push_back( catch_stat() );
    }

    return ast;
}

x::catch_stat_ast_ptr x::grammar::catch_stat()
{
    validity( x::token_t::TK_CATCH );

    auto ast = std::make_shared<x::catch_stat_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->param = parameter_decl();
    validity( x::token_t::TK_RIGHT_BRACKETS );
    ast->body = compound_stat();

    return ast;
}

x::throw_stat_ast_ptr x::grammar::throw_stat()
{
    validity( x::token_t::TK_THROW );

    auto ast = std::make_shared<x::throw_stat_ast>();
    ast->location = _location;

    ast->stat = stat();

    return ast;
}

x::if_stat_ast_ptr x::grammar::if_stat()
{
    validity( x::token_t::TK_IF );

    auto ast = std::make_shared<x::if_stat_ast>();
    ast->location = _location;

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->cond = exp_stat();
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
    ast->cond = exp_stat();
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
    ast->cond = exp_stat();
    validity( x::token_t::TK_SEMICOLON );
    ast->step = exp_stat();
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
    ast->collection = exp_stat();
    validity( x::token_t::TK_RIGHT_BRACKETS );

    ast->stat = stat();

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

    ast->exp = exp_stat();

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
        ast->value_type = type( "object" );

    if ( verify( x::token_t::TK_ASSIGN ) ) ast->init = initializers_exp();

    return ast;
}

x::exp_stat_ast_ptr x::grammar::exp_stat()
{
    return assignment_exp();
}

x::exp_stat_ast_ptr x::grammar::assignment_exp()
{
    x::exp_stat_ast_ptr left = unary_exp();

    if ( left != nullptr )
    {
        if ( lookup().type >= x::token_t::TK_ASSIGN && lookup().type <= x::token_t::TK_XOR_ASSIGN )
        {
            auto ast = std::make_shared<x::assignment_exp_ast>();
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

x::exp_stat_ast_ptr x::grammar::logical_or_exp()
{
    x::exp_stat_ast_ptr ast = logical_and_exp();

    while ( verify( x::token_t::TK_LOR ) )
    {
        auto exp = std::make_shared<x::logical_or_exp_ast>();
        exp->location = _location;

        exp->token = x::token_t::TK_LOR;
        exp->left = ast;
        exp->right = logical_and_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::logical_and_exp()
{
    x::exp_stat_ast_ptr ast = or_exp();

    while ( verify( x::token_t::TK_LAND ) )
    {
        auto exp = std::make_shared<x::logical_and_exp_ast>();
        exp->location = _location;

        exp->token = x::token_t::TK_LAND;
        exp->left = ast;
        exp->right = or_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::or_exp()
{
    x::exp_stat_ast_ptr ast = xor_exp();

    while ( verify( x::token_t::TK_OR ) )
    {
        auto exp = std::make_shared<x::or_exp_ast>();
        exp->location = _location;

        exp->token = x::token_t::TK_OR;
        exp->left = ast;
        exp->right = xor_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::xor_exp()
{
    x::exp_stat_ast_ptr ast = and_exp();

    while ( verify( x::token_t::TK_XOR ) )
    {
        auto exp = std::make_shared<x::xor_exp_ast>();
        exp->location = _location;

        exp->token = x::token_t::TK_XOR;
        exp->left = ast;
        exp->right = and_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::and_exp()
{
    x::exp_stat_ast_ptr ast = compare_exp();

    while ( verify( x::token_t::TK_AND ) )
    {
        auto exp = std::make_shared<x::and_exp_ast>();
        exp->location = _location;

        exp->token = x::token_t::TK_AND;
        exp->left = ast;
        exp->right = compare_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::compare_exp()
{
    x::exp_stat_ast_ptr ast = shift_exp();

    while ( lookup().type >= x::token_t::TK_EQUAL && lookup().type <= x::token_t::TK_LARG_OR_EQUAL )
    {
        auto exp = std::make_shared<x::compare_exp_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->left = ast;
        exp->right = shift_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::shift_exp()
{
    x::exp_stat_ast_ptr ast = add_exp();

    while ( lookup().type == x::token_t::TK_LEFT_SHIFT || lookup().type == x::token_t::TK_RIGHT_SHIFT )
    {
        auto exp = std::make_shared<x::shift_exp_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->left = ast;
        exp->right = add_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::add_exp()
{
    x::exp_stat_ast_ptr ast = mul_exp();

    while ( lookup().type == x::token_t::TK_ADD || lookup().type == x::token_t::TK_SUB )
    {
        auto exp = std::make_shared<x::add_exp_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->left = ast;
        exp->right = mul_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::mul_exp()
{
    x::exp_stat_ast_ptr ast = as_exp();

    while ( lookup().type == x::token_t::TK_MUL || lookup().type == x::token_t::TK_DIV || lookup().type == x::token_t::TK_MOD )
    {
        auto exp = std::make_shared<x::mul_exp_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->left = ast;
        exp->right = as_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::as_exp()
{
    x::exp_stat_ast_ptr ast = is_exp();

    while ( verify( x::token_t::TK_AS ) )
    {
        auto exp = std::make_shared<x::as_exp_ast>();
        exp->location = _location;

        exp->value = ast;
        exp->cast_type = type();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::is_exp()
{
    x::exp_stat_ast_ptr ast = unary_exp();

    while ( verify( x::token_t::TK_IS ) )
    {
        auto exp = std::make_shared<x::is_exp_ast>();
        exp->location = _location;

        exp->value = ast;
        exp->cast_type = type();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::sizeof_exp()
{
    x::exp_stat_ast_ptr ast = typeof_exp();

    while ( verify( x::token_t::TK_LEFT_BRACKETS ) )
    {
        auto exp = std::make_shared<x::sizeof_exp_ast>();
        exp->location = _location;

        exp->value = ast;

        ast = exp;

        validity( x::token_t::TK_RIGHT_BRACKETS );
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::typeof_exp()
{
    x::exp_stat_ast_ptr ast = unary_exp();

    while ( verify( x::token_t::TK_LEFT_BRACKETS ) )
    {
        auto exp = std::make_shared<x::typeof_exp_ast>();
        exp->location = _location;

        exp->value = ast;

        ast = exp;

        validity( x::token_t::TK_RIGHT_BRACKETS );
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::unary_exp()
{
    x::exp_stat_ast_ptr ast = nullptr;

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
        auto exp = std::make_shared<x::unary_exp_ast>();
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

x::exp_stat_ast_ptr x::grammar::postfix_exp()
{
    x::exp_stat_ast_ptr ast = primary_exp();

    switch ( lookup().type )
    {
    case x::token_t::TK_INC:
    case x::token_t::TK_DEC:
    {
        auto exp = std::make_shared<x::postfix_exp_ast>();
        exp->location = _location;

        exp->token = next().type;
        exp->exp = ast;

        ast = exp;
    }
    break;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::index_exp()
{
    x::exp_stat_ast_ptr ast = invoke_exp();

    while ( verify( x::token_t::TK_LEFT_INDEX ) )
    {
        auto exp = std::make_shared<x::index_exp_ast>();
        exp->location = _location;

        exp->left = ast;
        exp->right = exp_stat();

        ast = exp;

        validity( x::token_t::TK_RIGHT_INDEX );
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::invoke_exp()
{
    x::exp_stat_ast_ptr ast = member_exp();

    if ( lookup().type == x::token_t::TK_LEFT_BRACKETS )
    {
        auto exp = std::make_shared<x::invoke_exp_ast>();
        exp->location = _location;

        exp->left = ast;
        exp->right = arguments_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::member_exp()
{
    x::exp_stat_ast_ptr ast = primary_exp();

    while ( verify( x::token_t::TK_MEMBER_POINT ) )
    {
        auto exp = std::make_shared<x::member_exp_ast>();
        exp->location = _location;

        exp->left = ast;
        exp->right = identifier_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::primary_exp()
{
    x::exp_stat_ast_ptr ast = nullptr;

    switch ( lookup().type )
    {
    case x::token_t::TK_FUNCTION:
        ast = closure_exp();
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

x::closure_exp_ast_ptr x::grammar::closure_exp()
{
    validity( x::token_t::TK_FUNCTION );

    auto ast = std::make_shared<x::closure_exp_ast>();
    ast->location = _location;

    ast->name = location_to_name( ast->location, "closure_" );

    if ( verify( x::token_t::TK_LEFT_INDEX ) )
    {
        do
        {
            ast->captures.push_back( identifier_exp() );
        } while ( verify( x::token_t::TK_COMMA ) );

        verify( x::token_t::TK_RIGHT_INDEX );
    }

    validity( x::token_t::TK_LEFT_BRACKETS );
    while ( lookup().type != x::token_t::TK_RIGHT_BRACKETS )
    {
        ast->parameters.emplace_back( parameter_decl() );

        if ( !verify( x::token_t::TK_COMMA ) )
            break;
    };
    validity( x::token_t::TK_RIGHT_BRACKETS );

    if ( verify( x::token_t::TK_ASYNC ) )
        ast->is_async = true;

    if ( verify( x::token_t::TK_FUNCTION_RESULT ) )
        ast->result = type();
    else
        ast->result = type( "void" );

    ast->stat = compound_stat();

    return ast;
}

x::arguments_exp_ast_ptr x::grammar::arguments_exp()
{
    validity( x::token_t::TK_LEFT_BRACKETS );

    auto ast = std::make_shared<x::arguments_exp_ast>();
    ast->location = _location;
    if ( !verify( x::token_t::TK_RIGHT_BRACKETS ) )
    {
        do
        {
            ast->args.push_back( assignment_exp() );
        } while ( verify( x::token_t::TK_COMMA ) );
    }
    validity( x::token_t::TK_RIGHT_BRACKETS );

    return ast;
}

x::identifier_exp_ast_ptr x::grammar::identifier_exp()
{
    auto ast = std::make_shared<x::identifier_exp_ast>();
    ast->location = _location;

    switch( lookup().type )
	{
	case x::token_t::TK_THIS:
	case x::token_t::TK_BASE:
	case x::token_t::TK_IDENTIFIER:
        ast->ident = next().str;
        break;
    default:
        ASSERT( false, "" );
        break;
    }
    
    return ast;
}

x::initializers_exp_ast_ptr x::grammar::initializers_exp()
{
    validity( x::token_t::TK_LEFT_CURLY_BRACES );

    auto ast = std::make_shared<x::initializers_exp_ast>();
    ast->location = _location;
    if ( !verify( x::token_t::TK_RIGHT_CURLY_BRACES ) )
    {
        do
        {
            ast->args.push_back( assignment_exp() );
        } while ( verify( x::token_t::TK_COMMA ) );
    }
    validity( x::token_t::TK_RIGHT_CURLY_BRACES );

    return ast;
}

x::const_exp_ast_ptr x::grammar::const_exp()
{
    x::const_exp_ast_ptr ast = nullptr;

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

x::null_const_exp_ast_ptr x::grammar::null_const_exp()
{
    validity( x::token_t::TK_NULL );

    auto ast = std::make_shared<x::null_const_exp_ast>();
    ast->location = _location;

    return ast;
}

x::bool_const_exp_ast_ptr x::grammar::true_const_exp()
{
    validity( x::token_t::TK_TRUE );

    auto ast = std::make_shared<x::bool_const_exp_ast>();
    ast->location = _location;

    ast->value = true;

    return ast;
}

x::bool_const_exp_ast_ptr x::grammar::false_const_exp()
{
    validity( x::token_t::TK_FALSE );

    auto ast = std::make_shared<x::bool_const_exp_ast>();
    ast->location = _location;

    ast->value = false;

    return ast;
}

x::int_const_exp_ast_ptr x::grammar::int_const_exp()
{
    x::int_const_exp_ast_ptr ast;

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
            auto i_ast = std::make_shared<x::int64_const_exp_ast>();
            i_ast->location = _location;
            i_ast->value = i64;
            ast = i_ast;
        }
        else if ( std::abs( i64 ) > std::numeric_limits<x::int16>::max() )
        {
            auto i_ast = std::make_shared<x::int32_const_exp_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::int32>( i64 );
            ast = i_ast;
        }
        else if ( std::abs( i64 ) > std::numeric_limits<x::int8>::max() )
        {
            auto i_ast = std::make_shared<x::int16_const_exp_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::int16>( i64 );
            ast = i_ast;
        }
        else
        {
            auto i_ast = std::make_shared<x::int8_const_exp_ast>();
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
            auto i_ast = std::make_shared<x::uint64_const_exp_ast>();
            i_ast->location = _location;
            i_ast->value = u64;
            ast = i_ast;
        }
        else if ( u64 > std::numeric_limits<x::uint16>::max() )
        {
            auto i_ast = std::make_shared<x::uint32_const_exp_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::uint32>( u64 );
            ast = i_ast;
        }
        else if ( u64 > std::numeric_limits<x::uint8>::max() )
        {
            auto i_ast = std::make_shared<x::uint16_const_exp_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::uint16>( u64 );
            ast = i_ast;
        }
        else
        {
            auto i_ast = std::make_shared<x::uint8_const_exp_ast>();
            i_ast->location = _location;
            i_ast->value = static_cast<x::uint8>( u64 );
            ast = i_ast;
        }
    }

    return ast;
}

x::float_const_exp_ast_ptr x::grammar::float_const_exp()
{
    x::float_const_exp_ast_ptr ast;

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
        auto f_ast = std::make_shared<x::float64_const_exp_ast>();
        f_ast->location = _location;
        std::from_chars( str.c_str(), str.c_str() + str.size(), f_ast->value );
        ast = f_ast;
    }
    else
    {
        auto f_ast = std::make_shared<x::float32_const_exp_ast>();
        f_ast->location = _location;
        std::from_chars( str.c_str(), str.c_str() + str.size(), f_ast->value );
        ast = f_ast;
    }

    return ast;
}

x::string_const_exp_ast_ptr x::grammar::string_const_exp()
{
    auto ast = std::make_shared<x::string_const_exp_ast>();
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
        name += validity( x::token_t::TK_IDENTIFIER ).str;

    } while ( verify( x::token_t::TK_COMMA ) );

    return name;
}

x::type_ast_ptr x::grammar::type( std::string_view name, bool is_const )
{
    auto ast = std::make_shared<x::type_ast>();
    ast->location = _location;
    ast->name = name;
    ast->is_const = is_const;
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

    while ( !_stream.eof() )
    {
        int c = get();

        if ( std::isspace( c ) ) // space \r \n \t \0 ' '
        {
            continue;
        }
        else if ( c == '-' && std::isdigit( peek() ) )
        {
            push( tk.str, c );

            c = peek();
            while ( std::isdigit( c ) || c == '.' )
            {
                ASSERT( tk.str.find( '.' ) != std::string::npos && c == '.', "" );

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
                    ASSERT( tk.str.find( '.' ) != std::string::npos && c == '.', "" );

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

            while ( std::isalnum( peek() ) || c > 127 ) push( tk.str, get() );

            auto it = _tokenmap.find( tk.str );
            tk.type = it != _tokenmap.end() ? it->second : x::token_t::TK_IDENTIFIER;

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

            while ( std::ispunct( peek() ) )
                push( tk.str, get() );

            auto it = _tokenmap.find( tk.str );
            ASSERT( it == _tokenmap.end(), "" );
            tk.type = it->second;

            break;
        }
        else
        {
            ASSERT( true, "" );
        }
    }

    return tk;
}

x::token x::grammar::lookup()
{
    auto pos = _stream.tellg();
    auto tk = next();
    _stream.seekg( pos );

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
    ASSERT( lookup().type != k, "" );
    return next();
}

int x::grammar::get()
{
    int c = _stream.get();

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
    auto pos = _stream.tellg();
    auto c = get();
    _stream.seekg( pos );

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
