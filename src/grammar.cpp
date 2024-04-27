#include "grammar.h"

#include <map>
#include <regex>

namespace
{
    static std::map<std::string, x::token_t> token_map =
    {
        { (const char *)u8";", x::token_t::TK_SEMICOLON },
        { (const char *)u8",", x::token_t::TK_COMMA },
        { (const char *)u8"++", x::token_t::TK_INC },
        { (const char *)u8"--", x::token_t::TK_DEC },
        { (const char *)u8"+", x::token_t::TK_ADD },
        { (const char *)u8"-", x::token_t::TK_SUB },
        { (const char *)u8"*", x::token_t::TK_MUL },
        { (const char *)u8"/", x::token_t::TK_DIV },
        { (const char *)u8"%", x::token_t::TK_MOD },
        { (const char *)u8"&", x::token_t::TK_AND },
        { (const char *)u8"|", x::token_t::TK_OR },
        { (const char *)u8"^", x::token_t::TK_XOR },
        { (const char *)u8"<<", x::token_t::TK_LEFT_SHIFT },
        { (const char *)u8">>", x::token_t::TK_RIGHT_SHIFT },
        { (const char *)u8"&&", x::token_t::TK_LAND },
        { (const char *)u8"||", x::token_t::TK_LOR },
        { (const char *)u8"!", x::token_t::TK_LNOT },
        { (const char *)u8"~", x::token_t::TK_NOT },
        { (const char *)u8"=", x::token_t::TK_ASSIGN },
        { (const char *)u8"+=", x::token_t::TK_ADD_ASSIGN },
        { (const char *)u8"-=", x::token_t::TK_SUB_ASSIGN },
        { (const char *)u8"*=", x::token_t::TK_MUL_ASSIGN },
        { (const char *)u8"/=", x::token_t::TK_DIV_ASSIGN },
        { (const char *)u8"%=", x::token_t::TK_MOD_ASSIGN },
        { (const char *)u8"&=", x::token_t::TK_AND_ASSIGN },
        { (const char *)u8"|=", x::token_t::TK_OR_ASSIGN },
        { (const char *)u8"^=", x::token_t::TK_XOR_ASSIGN },
        { (const char *)u8"<<=", x::token_t::TK_LSHIFT_EQUAL },
        { (const char *)u8">>=", x::token_t::TK_RSHIFT_EQUAL },
        { (const char *)u8"==", x::token_t::TK_EQUAL },
        { (const char *)u8"!=", x::token_t::TK_NOT_EQUAL },
        { (const char *)u8"<", x::token_t::TK_LESS },
        { (const char *)u8">", x::token_t::TK_LARG },
        { (const char *)u8"<=", x::token_t::TK_LESS_OR_EQUAL },
        { (const char *)u8">=", x::token_t::TK_LARG_OR_EQUAL },
        { (const char *)u8":", x::token_t::TK_TYPECAST },
        { (const char *)u8".", x::token_t::TK_MEMBER_POINT },
        { (const char *)u8"?", x::token_t::TK_QUESTION },
        { (const char *)u8"...", x::token_t::TK_VARIADIC_SIGN },
        { (const char *)u8"[", x::token_t::TK_LEFT_INDEX },
        { (const char *)u8"]", x::token_t::TK_RIGHT_INDEX },
        { (const char *)u8"->", x::token_t::TK_FUNCTION_RESULT },
        { (const char *)u8"(", x::token_t::TK_LEFT_BRACKETS },
        { (const char *)u8")", x::token_t::TK_RIGHT_BRACKETS },
        { (const char *)u8"{", x::token_t::TK_LEFT_CURLY_BRACES },
        { (const char *)u8"}", x::token_t::TK_RIGHT_CURLY_BRACES },
        { (const char *)u8"void", x::token_t::TK_VOID },
        { (const char *)u8"byte", x::token_t::TK_BYTE },
        { (const char *)u8"bool", x::token_t::TK_BOOL },
        { (const char *)u8"any", x::token_t::TK_ANY },
        { (const char *)u8"int8", x::token_t::TK_INT8 },
        { (const char *)u8"int16", x::token_t::TK_INT16 },
        { (const char *)u8"int32", x::token_t::TK_INT32 },
        { (const char *)u8"int64", x::token_t::TK_INT64 },
        { (const char *)u8"uint8", x::token_t::TK_UINT8 },
        { (const char *)u8"uint16", x::token_t::TK_UINT16 },
        { (const char *)u8"uint32", x::token_t::TK_UINT32 },
        { (const char *)u8"uint64", x::token_t::TK_UINT64 },
        { (const char *)u8"float16", x::token_t::TK_FLOAT16 },
        { (const char *)u8"float32", x::token_t::TK_FLOAT32 },
        { (const char *)u8"float64", x::token_t::TK_FLOAT64 },
        { (const char *)u8"string", x::token_t::TK_STRING },
        { (const char *)u8"import", x::token_t::TK_IMPORT },
        { (const char *)u8"template", x::token_t::TK_TEMPLATE },
        { (const char *)u8"namespace", x::token_t::TK_NAMESPACE },
        { (const char *)u8"using", x::token_t::TK_USING },
        { (const char *)u8"enum", x::token_t::TK_ENUM },
        { (const char *)u8"class", x::token_t::TK_CLASS },
        { (const char *)u8"var", x::token_t::TK_VARIABLE },
        { (const char *)u8"func", x::token_t::TK_FUNCTION },
        { (const char *)u8"ref", x::token_t::TK_REF },
        { (const char *)u8"private", x::token_t::TK_PRIVATE },
        { (const char *)u8"public", x::token_t::TK_PUBLIC },
        { (const char *)u8"protected", x::token_t::TK_PROTECTED },
        { (const char *)u8"const", x::token_t::TK_CONST },
        { (const char *)u8"static", x::token_t::TK_STATIC },
        { (const char *)u8"extern", x::token_t::TK_EXTERN },
        { (const char *)u8"native", x::token_t::TK_NATIVE },
        { (const char *)u8"thread_local", x::token_t::TK_THREAD },
        { (const char *)u8"while", x::token_t::TK_WHILE },
        { (const char *)u8"if", x::token_t::TK_IF },
        { (const char *)u8"else", x::token_t::TK_ELSE },
        { (const char *)u8"for", x::token_t::TK_FOR },
        { (const char *)u8"foreach", x::token_t::TK_FOREACH },
        { (const char *)u8"case", x::token_t::TK_CASE },
        { (const char *)u8"default", x::token_t::TK_DEFAULT },
        { (const char *)u8"try", x::token_t::TK_TRY },
        { (const char *)u8"catch", x::token_t::TK_CATCH },
        { (const char *)u8"throw", x::token_t::TK_THROW },
        { (const char *)u8"await", x::token_t::TK_AWAIT },
        { (const char *)u8"yield", x::token_t::TK_YIELD },
        { (const char *)u8"break", x::token_t::TK_BREAK },
        { (const char *)u8"return", x::token_t::TK_RETURN },
        { (const char *)u8"continue", x::token_t::TK_CONTINUE },
        { (const char *)u8"null", x::token_t::TK_NULL },
        { (const char *)u8"true", x::token_t::TK_TRUE },
        { (const char *)u8"flase", x::token_t::TK_FALSE },
        { (const char *)u8"as", x::token_t::TK_AS },
        { (const char *)u8"is", x::token_t::TK_IS },
        { (const char *)u8"sizeof", x::token_t::TK_SIZEOF },
    };
}

x::grammar::grammar( std::istream & stream, std::string_view name )
    : _stream( stream.rdbuf() )
{
    _location.file = name;
}

x::grammar::~grammar()
{
}

x::unit_ast_ptr x::grammar::unit()
{
    auto ast = std::make_shared<unit_ast>();
    ast->location = _location;

    auto tk = lookup();
    do
    {
        switch ( tk.type )
        {
        case token_t::TK_IMPORT:
            ast->imports.emplace_back( import() );
            break;
        case token_t::TK_NAMESPACE:
            ast->namespaces.emplace_back( namespace_decl() );
            break;
        }
    } while ( tk.type != token_t::TK_EOF );

    return ast;
}

x::type_ast_ptr x::grammar::type()
{
    auto ast = std::make_shared<x::type_ast>();
    ast->location = _location;

    ast->is_ref = verify( token_t::TK_REF );
    ast->is_const = verify( token_t::TK_CONST );
    ast->name = type_name();
    ast->is_array = ( verify( token_t::TK_LEFT_INDEX ) && verify( token_t::TK_RIGHT_INDEX ) );

    return ast;
}

x::import_ast_ptr x::grammar::import()
{
    validity( token_t::TK_IMPORT );

    auto ast = std::make_shared<import_ast>();
    ast->location = _location;

    ast->path = validity( token_t::TK_LITERAL_STRING ).str;

    return ast;
}

x::enum_decl_ast_ptr x::grammar::enum_decl()
{
    validity( token_t::TK_ENUM );

    auto ast = std::make_shared<enum_decl_ast>();
    ast->location = _location;

    ast->name = validity( token_t::TK_IDENTIFIER ).str;

    validity( token_t::TK_LEFT_CURLY_BRACES );
    do
    {
        auto element = std::make_shared<enum_element_ast>();
        element->location = _location;

        element->name = validity( token_t::TK_IDENTIFIER ).str;

        if ( verify( token_t::TK_ASSIGN ) )
        {
            element->value = int_const_exp();
        }
        else
        {
            element->value = std::make_shared<int_const_exp_ast>();
            element->value->location = _location;

            if ( ast->elements.empty() )
                element->value->value = 0;
            else
                element->value->value = ast->elements.back()->value->value + 1;
        }

        ast->elements.emplace_back( element );

        verify( token_t::TK_COMMA );

    } while ( !verify( token_t::TK_RIGHT_CURLY_BRACES ) );

    return ast;
}

x::class_decl_ast_ptr x::grammar::class_decl()
{
    validity( token_t::TK_CLASS );

    auto ast = std::make_shared<class_decl_ast>();
    ast->location = _location;

    ast->name = validity( token_t::TK_IDENTIFIER ).str;

    if ( verify( token_t::TK_TYPECAST ) )
        ast->base = type();

    validity( token_t::TK_LEFT_CURLY_BRACES );
    do
    {
        auto acc = access();
        auto mod = modify();

        switch ( lookup().type )
        {
        case token_t::TK_USING:
        {
            auto usi = using_decl();
            usi->access = acc;
            usi->modify = mod;
            ast->usings.emplace_back( usi );
        }
        break;
        case token_t::TK_VARIABLE:
        {
            auto var = variable_decl();
            var->access = acc;
            var->modify = mod;
            ast->variables.emplace_back( var );
        }
        break;
        case token_t::TK_FUNCTION:
        {
            auto fun = function_decl();
            fun->access = acc;
            fun->modify = mod;
            ast->functions.emplace_back( fun );
        }
        break;
        case token_t::TK_SEMICOLON:
            next();
            break;
        default:
            ASSERT( true, "" );
            break;
        }

    } while ( !verify( token_t::TK_RIGHT_CURLY_BRACES ) );

    return ast;
}

x::using_decl_ast_ptr x::grammar::using_decl()
{
    validity( token_t::TK_USING );

    auto ast = std::make_shared<using_decl_ast>();
    ast->location = _location;

    ast->name = validity( token_t::TK_IDENTIFIER ).str;
    ast->retype = type();

    return ast;
}

x::template_decl_ast_ptr x::grammar::template_decl()
{
    validity( token_t::TK_TEMPLATE );

    /// TODO: template

    return x::template_decl_ast_ptr();
}

x::variable_decl_ast_ptr x::grammar::variable_decl()
{
    validity( token_t::TK_VARIABLE );

    auto ast = std::make_shared<variable_decl_ast>();
    ast->location = _location;

    ast->name = validity( token_t::TK_IDENTIFIER ).str;

    if ( verify( token_t::TK_TYPECAST ) )
        ast->value_type = type();
    else
        ast->value_type = type( "any" );

    if ( verify( token_t::TK_ASSIGN ) ) ast->init = initializers_exp();

    return ast;
}

x::function_decl_ast_ptr x::grammar::function_decl()
{
    validity( token_t::TK_FUNCTION );

    auto ast = std::make_shared<function_decl_ast>();
    ast->location = _location;

    ast->name = validity( token_t::TK_IDENTIFIER ).str;

    validity( token_t::TK_LEFT_BRACKETS );
    {
        do
        {
            ast->parameters.emplace_back( parameter_decl() );
        } while ( verify( token_t::TK_COMMA ) );
    }
    validity( token_t::TK_RIGHT_BRACKETS );

    if ( verify( token_t::TK_FUNCTION_RESULT ) )
        ast->result = type();
    else
        ast->result = type( "void" );

    if ( !verify( token_t::TK_SEMICOLON ) )
        ast->stat = compound_stat();

    return ast;
}

x::parameter_decl_ast_ptr x::grammar::parameter_decl()
{
    auto parameter = std::make_shared<parameter_decl_ast>();
    parameter->location = _location;

    parameter->name = validity( token_t::TK_IDENTIFIER ).str;
    validity( token_t::TK_TYPECAST );
    parameter->value_type = type();

    return parameter;
}

x::namespace_decl_ast_ptr x::grammar::namespace_decl()
{
    validity( token_t::TK_NAMESPACE );

    auto ast = std::make_shared<namespace_decl_ast>();
    ast->location = _location;

    validity( token_t::TK_LEFT_CURLY_BRACES );
    do
    {
        decl_ast_ptr decl = nullptr;

        auto acc = access();
        auto mod = modify();

        switch ( lookup().type )
        {
        case token_t::TK_ENUM:
            decl = enum_decl();
            break;
        case token_t::TK_CLASS:
            decl = class_decl();
            break;
        case token_t::TK_USING:
            decl = using_decl();
            break;
        case token_t::TK_TEMPLATE:
            decl = template_decl();
            break;
        case token_t::TK_VARIABLE:
            decl = variable_decl();
            mod = mod | x::modify_flag::STATIC;
            break;
        case token_t::TK_FUNCTION:
            decl = function_decl();
            mod = mod | x::modify_flag::STATIC;
            break;
        case token_t::TK_SEMICOLON:
            next();
            break;
        default:
            ASSERT( true, "" );
            break;
        }

        if ( decl )
        {
            decl->access = acc;
            decl->modify = mod;
            ast->members.emplace_back( decl );
        }

    } while ( !verify( token_t::TK_RIGHT_CURLY_BRACES ) );

    return ast;
}

x::stat_ast_ptr x::grammar::stat()
{
    switch ( lookup().type )
    {
    case x::token_t::TK_SEMICOLON:
        return std::make_shared<empty_stat_ast>();
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

x::compound_stat_ast_ptr x::grammar::compound_stat()
{
    validity( token_t::TK_LEFT_CURLY_BRACES );

    auto ast = std::make_shared<x::compound_stat_ast>();
    ast->location = _location;

    while ( verify( token_t::TK_RIGHT_CURLY_BRACES ) )
    {
        ast->stats.push_back( stat() );
    }

    validity( token_t::TK_RIGHT_CURLY_BRACES );

    return ast;
}

x::await_stat_ast_ptr x::grammar::await_stat()
{
    validity( token_t::TK_AWAIT );

    auto ast = std::make_shared<await_stat_ast>();
    ast->location = _location;

    ast->exp = exp_stat();

    return ast;
}

x::yield_stat_ast_ptr x::grammar::yield_stat()
{
    validity( token_t::TK_YIELD );

    auto ast = std::make_shared<yield_stat_ast>();
    ast->location = _location;

    ast->is_break = !verify( token_t::TK_RETURN );

    if ( !verify( token_t::TK_SEMICOLON ) ) ast->exp = exp_stat();

    return ast;
}

x::try_stat_ast_ptr x::grammar::try_stat()
{
    validity( token_t::TK_TRY );

    auto ast = std::make_shared<try_stat_ast>();
    ast->location = _location;

    ast->body = compound_stat();

    while ( verify( token_t::TK_CATCH ) )
    {
        ast->catchs.push_back( catch_stat() );
    }

    return ast;
}

x::catch_stat_ast_ptr x::grammar::catch_stat()
{
    validity( token_t::TK_CATCH );

    auto ast = std::make_shared<catch_stat_ast>();
    ast->location = _location;

    validity( token_t::TK_LEFT_BRACKETS );
    ast->param = parameter_decl();
    validity( token_t::TK_RIGHT_BRACKETS );
    ast->body = compound_stat();

    return ast;
}

x::throw_stat_ast_ptr x::grammar::throw_stat()
{
    validity( token_t::TK_THROW );

    auto ast = std::make_shared<throw_stat_ast>();
    ast->location = _location;

    ast->stat = stat();

    return ast;
}

x::if_stat_ast_ptr x::grammar::if_stat()
{
    validity( token_t::TK_IF );

    auto ast = std::make_shared<if_stat_ast>();
    ast->location = _location;

    validity( token_t::TK_LEFT_BRACKETS );
    ast->exp = exp_stat();
    validity( token_t::TK_RIGHT_BRACKETS );

    ast->then_stat = stat();

    if ( verify( token_t::TK_ELSE ) )
        ast->else_stat = stat();

    return ast;
}

x::while_stat_ast_ptr x::grammar::while_stat()
{
    validity( token_t::TK_WHILE );

    auto ast = std::make_shared<while_stat_ast>();
    ast->location = _location;

    validity( token_t::TK_LEFT_BRACKETS );
    ast->cond = exp_stat();
    validity( token_t::TK_RIGHT_BRACKETS );

    ast->stat = stat();

    return ast;
}

x::for_stat_ast_ptr x::grammar::for_stat()
{
    validity( token_t::TK_FOR );

    auto ast = std::make_shared<for_stat_ast>();
    ast->location = _location;

    validity( token_t::TK_LEFT_BRACKETS );
    ast->init = stat();
    validity( token_t::TK_SEMICOLON );
    ast->cond = exp_stat();
    validity( token_t::TK_SEMICOLON );
    ast->step = exp_stat();
    validity( token_t::TK_RIGHT_BRACKETS );

    ast->stat = stat();

    return ast;
}

x::foreach_stat_ast_ptr x::grammar::foreach_stat()
{
    validity( token_t::TK_FOREACH );

    auto ast = std::make_shared<foreach_stat_ast>();
    ast->location = _location;

    validity( token_t::TK_LEFT_BRACKETS );
    ast->init = stat();
    validity( token_t::TK_TYPECAST );
    ast->cond = exp_stat();
    validity( token_t::TK_RIGHT_BRACKETS );

    ast->stat = stat();

    return ast;
}

x::break_stat_ast_ptr x::grammar::break_stat()
{
    validity( token_t::TK_BREAK );

    auto ast = std::make_shared<break_stat_ast>();
    ast->location = _location;

    return ast;
}

x::return_stat_ast_ptr x::grammar::return_stat()
{
    validity( token_t::TK_RETURN );

    auto ast = std::make_shared<return_stat_ast>();
    ast->location = _location;

    ast->exp = exp_stat();

    return ast;
}

x::continue_stat_ast_ptr x::grammar::continue_stat()
{
    validity( token_t::TK_CONTINUE );

    auto ast = std::make_shared<continue_stat_ast>();
    ast->location = _location;

    return ast;
}

x::local_stat_ast_ptr x::grammar::local_stat()
{
    validity( token_t::TK_VARIABLE );

    auto ast = std::make_shared<local_stat_ast>();
    ast->location = _location;

    ast->name = validity( token_t::TK_IDENTIFIER ).str;

    if ( verify( token_t::TK_TYPECAST ) )
        ast->value_type = type();
    else
        ast->value_type = type( "any" );

    if ( verify( token_t::TK_ASSIGN ) ) ast->init = initializers_exp();

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
            auto ast = std::make_shared<assignment_exp_ast>();
            ast->location = _location;

            ast->tk_type = next().type;
            ast->left = left;
            ast->right = assignment_exp();

            return ast;
        }
        else
        {
            return left;
        }
    }

    return conditional_exp();
}

x::exp_stat_ast_ptr x::grammar::conditional_exp()
{
    x::exp_stat_ast_ptr ast = logical_or_exp();

    while ( verify( token_t::TK_QUESTION ) )
    {
        auto exp = std::make_shared<conditional_exp_ast>();
        exp->location = _location;

        exp->tk_type = token_t::TK_QUESTION;
        exp->cond = ast;
        exp->then_exp = exp_stat();
        validity( token_t::TK_TYPECAST );
        exp->else_exp = conditional_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::logical_or_exp()
{
    x::exp_stat_ast_ptr ast = logical_and_exp();

    while ( verify( token_t::TK_LOR ) )
    {
        auto exp = std::make_shared<logical_or_exp_ast>();
        exp->location = _location;

        exp->tk_type = token_t::TK_LOR;
        exp->left = ast;
        exp->right = logical_and_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::logical_and_exp()
{
    x::exp_stat_ast_ptr ast = or_exp();

    while ( verify( token_t::TK_LAND ) )
    {
        auto exp = std::make_shared<logical_and_exp_ast>();
        exp->location = _location;

        exp->tk_type = token_t::TK_LAND;
        exp->left = ast;
        exp->right = or_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::or_exp()
{
    x::exp_stat_ast_ptr ast = xor_exp();

    while ( verify( token_t::TK_OR ) )
    {
        auto exp = std::make_shared<or_exp_ast>();
        exp->location = _location;

        exp->tk_type = token_t::TK_OR;
        exp->left = ast;
        exp->right = xor_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::xor_exp()
{
    x::exp_stat_ast_ptr ast = and_exp();

    while ( verify( token_t::TK_XOR ) )
    {
        auto exp = std::make_shared<xor_exp_ast>();
        exp->location = _location;

        exp->tk_type = token_t::TK_XOR;
        exp->left = ast;
        exp->right = and_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::and_exp()
{
    x::exp_stat_ast_ptr ast = compare_exp();

    while ( verify( token_t::TK_AND ) )
    {
        auto exp = std::make_shared<and_exp_ast>();
        exp->location = _location;

        exp->tk_type = token_t::TK_AND;
        exp->left = ast;
        exp->right = compare_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::compare_exp()
{
    x::exp_stat_ast_ptr ast = shift_exp();

    while ( lookup().type >= token_t::TK_EQUAL && lookup().type <= token_t::TK_LARG_OR_EQUAL )
    {
        auto exp = std::make_shared<compare_exp_ast>();
        exp->location = _location;

        exp->tk_type = next().type;
        exp->left = ast;
        exp->right = shift_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::shift_exp()
{
    x::exp_stat_ast_ptr ast = add_exp();

    while ( lookup().type == token_t::TK_LEFT_SHIFT || lookup().type == token_t::TK_RIGHT_SHIFT )
    {
        auto exp = std::make_shared<shift_exp_ast>();
        exp->location = _location;

        exp->tk_type = next().type;
        exp->left = ast;
        exp->right = add_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::add_exp()
{
    x::exp_stat_ast_ptr ast = mul_exp();

    while ( lookup().type == token_t::TK_ADD || lookup().type == token_t::TK_SUB )
    {
        auto exp = std::make_shared<add_exp_ast>();
        exp->location = _location;

        exp->tk_type = next().type;
        exp->left = ast;
        exp->right = mul_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::mul_exp()
{
    x::exp_stat_ast_ptr ast = as_exp();

    while ( lookup().type == token_t::TK_MUL || lookup().type == token_t::TK_DIV || lookup().type == token_t::TK_MOD )
    {
        auto exp = std::make_shared<mul_exp_ast>();
        exp->location = _location;

        exp->tk_type = next().type;
        exp->left = ast;
        exp->right = as_exp();

        ast = exp;
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::as_exp()
{
    x::exp_stat_ast_ptr ast = is_exp();

    while ( verify( token_t::TK_AS ) )
    {
        auto exp = std::make_shared<as_exp_ast>();
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

    while ( verify( token_t::TK_IS ) )
    {
        auto exp = std::make_shared<is_exp_ast>();
        exp->location = _location;

        exp->value = ast;
        exp->cast_type = type();

        ast = exp;
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
        auto exp = std::make_shared<unary_exp_ast>();
        exp->location = _location;

        exp->tk_type = next().type;
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
        auto exp = std::make_shared<postfix_exp_ast>();
        exp->location = _location;

        exp->tk_type = next().type;
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

    while ( verify( token_t::TK_LEFT_INDEX ) )
    {
        auto exp = std::make_shared<index_exp_ast>();
        exp->location = _location;

        exp->left = ast;
        exp->right = exp_stat();

        ast = exp;

        validity( token_t::TK_RIGHT_INDEX );
    }

    return ast;
}

x::exp_stat_ast_ptr x::grammar::invoke_exp()
{
    x::exp_stat_ast_ptr ast = member_exp();

    if ( lookup().type == token_t::TK_LEFT_BRACKETS )
    {
        auto exp = std::make_shared<invoke_exp_ast>();
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

    while ( verify( token_t::TK_MEMBER_POINT ) )
    {
        auto exp = std::make_shared<member_exp_ast>();
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
    case token_t::TK_FUNCTION:
        ast = closure_exp();
        break;
    case token_t::TK_IDENTIFIER:
        ast = identifier_exp();
        break;
    case token_t::TK_LEFT_BRACKETS:
        ast = arguments_exp();
        break;
    case token_t::TK_LEFT_CURLY_BRACES:
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
    validity( token_t::TK_FUNCTION );

    auto ast = std::make_shared<closure_exp_ast>();
    ast->location = _location;

    ast->name = location_to_name( ast->location, "closure_" );

    if ( verify( token_t::TK_LEFT_INDEX ) )
    {
        do
        {
            ast->captures.push_back( identifier_exp() );
        } while ( verify( token_t::TK_COMMA ) );

        verify( token_t::TK_RIGHT_INDEX );
    }

    ast->location = _location;
    ast->access = access_t::PUBLIC;

    if ( ast->captures.empty() )
        ast->modify = ast->modify | x::modify_flag::STATIC;

    validity( token_t::TK_LEFT_BRACKETS );
    {
        do
        {
            ast->parameters.emplace_back( parameter_decl() );
        } while ( verify( token_t::TK_COMMA ) );
    }
    validity( token_t::TK_RIGHT_BRACKETS );

    if ( verify( token_t::TK_ASYNC ) )
        ast->modify = ast->modify | x::modify_flag::ASYNC;

    if ( verify( token_t::TK_FUNCTION_RESULT ) )
        ast->result = type();
    else
        ast->result = type( "void" );

    ast->stat = stat();

    return ast;
}

x::arguments_exp_ast_ptr x::grammar::arguments_exp()
{
    validity( token_t::TK_LEFT_BRACKETS );

    auto ast = std::make_shared<arguments_exp_ast>();
    ast->location = _location;
    if ( !verify( token_t::TK_RIGHT_BRACKETS ) )
    {
        do
        {
            ast->args.push_back( assignment_exp() );
        } while ( verify( token_t::TK_COMMA ) );
    }
    validity( token_t::TK_RIGHT_BRACKETS );

    return ast;
}

x::identifier_exp_ast_ptr x::grammar::identifier_exp()
{
    auto ast = std::make_shared<identifier_exp_ast>();
    ast->location = _location;

    ast->ident = validity( token_t::TK_IDENTIFIER ).str;

    return ast;
}

x::initializers_exp_ast_ptr x::grammar::initializers_exp()
{
    validity( token_t::TK_LEFT_CURLY_BRACES );

    auto ast = std::make_shared<initializers_exp_ast>();
    ast->location = _location;
    if ( !verify( token_t::TK_RIGHT_CURLY_BRACES ) )
    {
        do
        {
            ast->args.push_back( assignment_exp() );
        } while ( verify( token_t::TK_COMMA ) );
    }
    validity( token_t::TK_RIGHT_CURLY_BRACES );

    return ast;
}

x::const_exp_ast_ptr x::grammar::const_exp()
{
    x::const_exp_ast_ptr ast = nullptr;

    switch ( lookup().type )
    {
    case token_t::TK_NULL:
        ast = null_const_exp();
        break;
    case token_t::TK_TRUE:
        ast = true_const_exp();
        break;
    case token_t::TK_FALSE:
        ast = false_const_exp();
        break;
    case token_t::TK_LITERAL_INT:
        ast = int_const_exp();
        break;
    case token_t::TK_LITERAL_FLOAT:
        ast = float_const_exp();
        break;
    case token_t::TK_LITERAL_STRING:
        ast = string_const_exp();
        break;
    }

    return ast;
}

x::null_const_exp_ast_ptr x::grammar::null_const_exp()
{
    validity( token_t::TK_NULL );

    auto ast = std::make_shared<null_const_exp_ast>();
    ast->location = _location;

    return ast;
}

x::bool_const_exp_ast_ptr x::grammar::true_const_exp()
{
    validity( token_t::TK_TRUE );

    auto ast = std::make_shared<bool_const_exp_ast>();
    ast->location = _location;

    ast->value = true;

    return ast;
}

x::bool_const_exp_ast_ptr x::grammar::false_const_exp()
{
    validity( token_t::TK_FALSE );

    auto ast = std::make_shared<bool_const_exp_ast>();
    ast->location = _location;

    ast->value = false;

    return ast;
}

x::int_const_exp_ast_ptr x::grammar::int_const_exp()
{
    auto ast = std::make_shared<int_const_exp_ast>();
    ast->location = _location;

    ast->value = std::stoll( validity( token_t::TK_LITERAL_INT ).str );

    return ast;
}

x::float_const_exp_ast_ptr x::grammar::float_const_exp()
{
    auto ast = std::make_shared<float_const_exp_ast>();
    ast->location = _location;

    ast->value = std::stod( validity( token_t::TK_LITERAL_FLOAT ).str );

    return ast;
}

x::string_const_exp_ast_ptr x::grammar::string_const_exp()
{
    auto ast = std::make_shared<string_const_exp_ast>();
    ast->location = _location;

    ast->value = validity( token_t::TK_LITERAL_STRING ).str;

    return ast;
}

std::string x::grammar::type_name()
{
    std::string name;

    do
    {
        if ( !name.empty() ) name += ".";
        name += validity( token_t::TK_IDENTIFIER ).str;

    } while ( verify( token_t::TK_COMMA ) );

    return name;
}

x::type_ast_ptr x::grammar::type( std::string_view name, bool is_ref, bool is_const, bool is_array )
{
    auto ast = std::make_shared<x::type_ast>();
    ast->location = _location;
    ast->name = name;
    ast->is_ref = is_ref;
    ast->is_const = is_const;
    ast->is_array = is_array;
    return ast;
}

std::string x::grammar::location_to_name( const x::source_location & location, std::string_view suffix )
{
    std::regex reg( ":|/|\\\\" );

    std::string name( suffix.begin(), suffix.end() );
    name.append( location.file.begin(), location.file.end() );

    name = std::regex_replace( name, reg, "_" );
    name += std::to_string( location.line ) + "_" + std::to_string( location.column );

    return name;
}

x::modify_flag x::grammar::modify()
{
    int result = 0;

    while ( 1 )
    {
        if ( verify( token_t::TK_ASYNC ) ) result |= x::modify_flag::ASYNC;
        else if ( verify( token_t::TK_CONST ) ) result |= x::modify_flag::CONST;
        else if ( verify( token_t::TK_STATIC ) ) result |= x::modify_flag::STATIC;
        else if ( verify( token_t::TK_THREAD ) ) result |= x::modify_flag::THREAD;
        else if ( verify( token_t::TK_NATIVE ) ) result |= x::modify_flag::NATIVE;
        else if ( verify( token_t::TK_EXTERN ) ) result |= x::modify_flag::EXTERN;
        else break;
    }

    return (modify_flag)result;
}

x::access_t x::grammar::access()
{
    x::access_t result = access_t::PRIVATE;

    while ( 1 )
    {
        if ( verify( token_t::TK_PUBLIC ) ) result = access_t::PUBLIC;
        else if ( verify( token_t::TK_PRIVATE ) ) result = access_t::PRIVATE;
        else if ( verify( token_t::TK_PROTECTED ) ) result = access_t::PROTECTED;
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

    token tk;
    tk.location = _location;
    tk.type = token_t::TK_EOF;

    while ( !_stream.eof() )
    {
        int c = get();

        if ( std::isspace( c ) ) // space \r \n \t \0 ' '
        {
            continue;
        }
        else if ( std::isdigit( c ) ) // number 1 233 0x123456 0b1101001
        {
            if ( c == '0' && std::tolower( peek() ) == 'x' )
            {
                push( tk.str, c );
                push( tk.str, get() );

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

            tk.type = ( tk.str.find( '.' ) == std::string::npos ) ? token_t::TK_LITERAL_INT : token_t::TK_LITERAL_FLOAT;

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

            tk.type = token_t::TK_LITERAL_STRING;

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

            tk.type = token_t::TK_LITERAL_STRING;

            break;
        }
        else if ( std::isalpha( c ) || c == '_' || c > 127 ) // identifier
        {
            push( tk.str, c );

            while ( std::isalnum( peek() ) || c > 127 ) push( tk.str, get() );

            auto it = token_map.find( tk.str );
            tk.type = it != token_map.end() ? it->second : token_t::TK_IDENTIFIER;

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

            auto it = token_map.find( tk.str );
            ASSERT( it == token_map.end(), "" );
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

bool x::grammar::verify( token_t k )
{
    if ( lookup().type == k )
    {
        next();
        return true;
    }

    return false;
}

x::token x::grammar::validity( token_t k )
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
