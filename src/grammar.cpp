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
    _location.line = 1;
    _location.col = 1;

    auto result = unit();

    _stream = nullptr;
    _tokenmap = nullptr;
    _location.file = {};
    _location.line = 1;
    _location.col = 1;

    return result;
}

x::unit_ast_ptr x::grammar::unit()
{
    auto ast = std::make_shared<x::unit_ast>();
    ast->set_location( _location );

    x::attribute_ast_ptr attr = nullptr;

    auto tk = lookup();
    while( tk.type != x::token_t::TK_EOF )
    {
        switch ( tk.type )
        {
        case x::token_t::TK_IMPORT:
            ast->insert_import( import() );
            break;
        case x::token_t::TK_ATTRIBUTE:
            attr = attribute();
            break;
        case x::token_t::TK_NAMESPACE:
            ast->insert_namespace( namespace_decl() );
            if ( attr )
                ast->get_namespaces().back()->set_attribute( attr );
            break;
        case x::token_t::TK_SEMICOLON:
            break;
        }

        attr = nullptr;

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
        case x::token_t::TK_REF: is_ref = true; break;
        case x::token_t::TK_CONST: is_const = true; break;
    END_VERIFYS()

    std::string name;

    if ( verify( x::token_t::TK_LEFT_BRACKETS ) )
    {
        auto func_type = std::make_shared<x::func_type_ast>();

        do { func_type->insert_parameter( type() ); } while ( verify( x::token_t::TK_COMMA ) );

        validity( x::token_t::TK_RIGHT_BRACKETS );

        if ( verify( x::token_t::TK_FUNCTION_RESULT ) )
        {
            do
            {
                func_type->insert_result( type() );
            } while ( verify( x::token_t::TK_COMMA ) );
        }

        ast = func_type;
    }
    else
    {
        name = type_name();

        if ( verify( x::token_t::TK_LESS ) )
        {
            auto temp_type = std::make_shared<x::temp_type_ast>();

            do
            {
                temp_type->insert_element( express() );
            } while ( verify( x::token_t::TK_COMMA ) );

            rsl_2_2larg = true;
            verify( x::token_t::TK_LARG );
            rsl_2_2larg = false;

            ast = temp_type;
        }
        else if ( verify( x::token_t::TK_VARIADIC_SIGN ) )
        {
            ast = std::make_shared<x::list_type_ast>();
        }
        else
        {
            ast = std::make_shared<x::type_ast>();
        }
    }

    ast->set_location( location );
    ast->set_is_const( is_const );
    ast->set_is_ref( is_ref );
    ast->set_name( name );

    return ast;
}

x::import_ast_ptr x::grammar::import()
{
    validity( x::token_t::TK_IMPORT );

    auto ast = std::make_shared<x::import_ast>();
    ast->set_location( _location );

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->set_modulename( validity( x::token_t::TK_CONSTEXPR_STRING ).str );
    validity( x::token_t::TK_RIGHT_BRACKETS );

    return ast;
}

x::attribute_ast_ptr x::grammar::attribute()
{
    validity( x::token_t::TK_ATTRIBUTE );

    auto ast = std::make_shared<x::attribute_ast>();
    ast->set_location( _location );

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]()
    {
        std::pair<std::string, std::string> value;

        value.first = validity( x::token_t::TK_IDENTIFIER ).str;
        
        validity( x::token_t::TK_ASSIGN );

        while ( lookup().type != x::token_t::TK_COMMA && lookup().type != x::token_t::TK_RIGHT_BRACKETS )
        {
            if ( !value.second.empty() ) value.second.append( " " );

            value.second.append( next().str );
        }
        ast->insert_attribute( value );
    } );

    return ast;
}

x::parameter_ast_ptr x::grammar::parameter()
{
    verify( x::token_t::TK_VARIABLE );

    auto ast = std::make_shared<x::parameter_ast>();
    ast->set_location( _location );

    ast->set_name( validity( x::token_t::TK_IDENTIFIER ).str );

    if ( verify( x::token_t::TK_VARIADIC_SIGN ) )
        ast->set_valuetype( anylisttype() );
    else if ( verify( x::token_t::TK_TYPECAST ) )
        ast->set_valuetype( type() );
    else
        ast->set_valuetype( anytype() );

    if ( verify( x::token_t::TK_ASSIGN ) )
    {
        ast->set_default( express() );
    }

    return ast;
}

x::enum_decl_ast_ptr x::grammar::enum_decl()
{
    validity( x::token_t::TK_ENUM );

    auto ast = std::make_shared<x::enum_decl_ast>();
    ast->set_location( _location );

    ast->set_name( validity( x::token_t::TK_IDENTIFIER ).str );

    verify_list( x::token_t::TK_LEFT_CURLY_BRACES, x::token_t::TK_RIGHT_CURLY_BRACES, x::token_t::TK_COMMA, [&]()
    {
        if ( lookup().type == x::token_t::TK_RIGHT_CURLY_BRACES )
            return;

        x::enum_decl_ast::element_pair element;
        element.first = validity( x::token_t::TK_IDENTIFIER ).str;

        if ( verify( x::token_t::TK_ASSIGN ) )
            element.second = express();

        ast->insert_element( element );
    } );

    return ast;
}

x::class_decl_ast_ptr x::grammar::class_decl()
{
    validity( x::token_t::TK_CLASS );

    auto ast = std::make_shared<x::class_decl_ast>();
    ast->set_location( _location );

    ast->set_name( validity( x::token_t::TK_IDENTIFIER ).str );

    if ( verify( x::token_t::TK_TYPECAST ) )
        ast->set_base( type() );

    while ( verify( x::token_t::TK_COMMA ) )
    {
        ast->insert_interface( type() );
    }

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
            if ( attr )
                decl->set_attribute( attr );
            decl->set_access( acce );
            ast->insert_using( decl );
        }
        break;
        case x::token_t::TK_FRIEND:
        {
            ast->insert_friend( type() );
        }
        break;
        case x::token_t::TK_VARIABLE:
        {
            auto decl = variable_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->insert_variable( decl );
        }
        break;
        case x::token_t::TK_FUNCTION:
        {
            auto decl = function_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->insert_function( decl );
        }
        break;
        case x::token_t::TK_OPERATOR:
        {
            auto decl = operator_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->insert_operator( decl );
        }
        break;
        case x::token_t::TK_CONSTRUCT:
        {
            auto decl = construct_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->set_construct( decl );
        }
        break;
        case x::token_t::TK_FINALIZE:
        {
            auto decl = finalize_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->set_finalize( decl );
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
    ast->set_location( _location );

    ast->set_name( validity( x::token_t::TK_IDENTIFIER ).str );

    validity( x::token_t::TK_ASSIGN );

    ast->set_retype( type() );

    return ast;
}

x::template_decl_ast_ptr x::grammar::template_decl()
{
    validity( x::token_t::TK_TEMPLATE );

    auto ast = std::make_shared<x::template_decl_ast>();
    ast->set_location( _location );

    ast->set_name( validity( x::token_t::TK_IDENTIFIER ).str );

    rsl_2_2larg = true;
    verify_list( x::token_t::TK_LESS, x::token_t::TK_LARG, x::token_t::TK_COMMA, [&]() -> bool
    {
        ast->insert_element( parameter() );

        return ast->get_elements().back()->type() == x::ast_t::LIST_TYPE;
    } );
    rsl_2_2larg = false;

    if ( verify( x::token_t::TK_TYPECAST ) )
        ast->set_base( type() );

    while ( verify( x::token_t::TK_COMMA ) )
    {
        ast->insert_interface( type() );
    }

    if ( verify( x::token_t::TK_WHERE ) )
        ast->set_where( express() );

    verify_list( x::token_t::TK_LEFT_CURLY_BRACES, x::token_t::TK_RIGHT_CURLY_BRACES, x::token_t::TK_COMMA, [&]()
    {
        auto attr = attribute();
        auto acce = access();

        switch ( lookup().type )
        {
        case x::token_t::TK_USING:
        {
            auto decl = using_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->insert_using( decl );
        }
        break;
        case x::token_t::TK_FRIEND:
        {
            ast->insert_friend( type() );
        }
        break;
        case x::token_t::TK_VARIABLE:
        {
            auto decl = variable_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->insert_variable( decl );
        }
        break;
        case x::token_t::TK_FUNCTION:
        {
            auto decl = function_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->insert_function( decl );
        }
        break;
        case x::token_t::TK_OPERATOR:
        {
            auto decl = operator_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->insert_operator( decl );
        }
        break;
        case x::token_t::TK_CONSTRUCT:
        {
            auto decl = construct_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->set_construct( decl );
        }
        break;
        case x::token_t::TK_FINALIZE:
        {
            auto decl = finalize_decl();
            if ( attr )
            decl->set_attribute( attr );
            decl->set_access( acce );
            ast->set_finalize( decl );
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
    ast->set_location( _location );

    switch ( verify( { x::token_t::TK_LOCAL, x::token_t::TK_STATIC, x::token_t::TK_THREAD } ).type )
    {
    case x::token_t::TK_LOCAL: ast->set_is_local( true ); break;
    case x::token_t::TK_STATIC: ast->set_is_static( true ); break;
    case x::token_t::TK_THREAD: ast->set_is_thread( true ); break;
    }

    ast->set_name( validity( x::token_t::TK_IDENTIFIER ).str );

    if ( verify( x::token_t::TK_TYPECAST ) )
        ast->set_valuetype( type() );
    else
        ast->set_valuetype( anytype() );

    if ( verify( x::token_t::TK_ASSIGN ) )
    {
        x::initializer_expr_ast_ptr init;

        if ( lookup().type == x::token_t::TK_LEFT_CURLY_BRACES )
        {
            init = initializer_exp();
        }
        else
        {
            init = std::make_shared<x::initializer_expr_ast>();
            init->set_location( _location );
            init->insert_arg( express() );
        }

        ast->set_init( init );
    }

    return ast;
}

x::function_decl_ast_ptr x::grammar::function_decl( bool interface_func )
{
    validity( x::token_t::TK_FUNCTION );

    auto ast = std::make_shared<x::function_decl_ast>();
    ast->set_location( _location );

    if ( !interface_func )
    {
        switch ( verify( { x::token_t::TK_FINAL, x::token_t::TK_STATIC, x::token_t::TK_VIRTUAL, x::token_t::TK_OVERRIDE } ).type )
        {
        case x::token_t::TK_FINAL: ast->set_is_final( true ); break;
        case x::token_t::TK_STATIC: ast->set_is_static( true ); break;
        case x::token_t::TK_VIRTUAL: ast->set_is_virtual( true ); break;
        case x::token_t::TK_OVERRIDE: ast->set_is_override( true ); break;
        }
    }
    else
    {
        validity( x::token_t::TK_VIRTUAL );
        ast->set_is_virtual( true );
    }

    if ( verify( x::token_t::TK_CONST ) )
    {
        ast->set_is_const( true );
    }

    ast->set_name( validity( x::token_t::TK_IDENTIFIER ).str );

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]() -> bool
    {
        auto param = parameter();
        ast->insert_parameter( param );

        return ( param->get_valuetype()->type() == x::ast_t::LIST_TYPE );
    } );

    if ( verify( x::token_t::TK_FUNCTION_RESULT ) )
    {
        do
        {
            ast->insert_result( type() );
        } while ( verify( x::token_t::TK_COMMA ) );
    }

    if ( !interface_func )
    {
        if ( verify( x::token_t::TK_ASSIGN ) )
            ast->set_body( extern_stat() );
        else              
            ast->set_body( compound_stat() );
    }
    else
    {
        verify( x::token_t::TK_SEMICOLON );
    }

    return ast;
}

x::operator_decl_ast_ptr x::grammar::operator_decl()
{
    validity( x::token_t::TK_OPERATOR );

    auto ast = std::make_shared<x::operator_decl_ast>();
    ast->set_location( _location );

    switch ( lookup().type )
    {
    case x::token_t::TK_ADD:
    case x::token_t::TK_SUB:
    case x::token_t::TK_MUL:
    case x::token_t::TK_DIV:
    case x::token_t::TK_MOD:
    case x::token_t::TK_AND:
    case x::token_t::TK_OR:
    case x::token_t::TK_XOR:
    case x::token_t::TK_LEFT_SHIFT:
    case x::token_t::TK_RIGHT_SHIFT:
    case x::token_t::TK_LAND:
    case x::token_t::TK_LOR:
    case x::token_t::TK_NOT:
    case x::token_t::TK_REV:
    case x::token_t::TK_EQUAL:
    case x::token_t::TK_COMPARE:
        ast->set_name( next().str );
        break;
    case x::token_t::TK_LEFT_INDEX:
        next(); validity( x::token_t::TK_RIGHT_INDEX ); ast->set_name( "[]" );
        break;
    case x::token_t::TK_LEFT_BRACKETS:
        next(); validity( x::token_t::TK_RIGHT_BRACKETS ); ast->set_name( "()" );
        break;
    default:
        break;
    }

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]() -> bool
    {
        auto param = parameter();
        ast->insert_parameter( param );

        return ( param->get_valuetype()->type() == x::ast_t::LIST_TYPE );
    } );

    if ( verify( x::token_t::TK_FUNCTION_RESULT ) )
    {
        do
        {
            ast->insert_result( type() );
        } while ( verify( x::token_t::TK_COMMA ) );
    }

    ast->set_body( compound_stat() );

    return ast;
}

x::function_decl_ast_ptr x::grammar::finalize_decl()
{
    validity( x::token_t::TK_FINALIZE );
    validity( x::token_t::TK_LEFT_BRACKETS );
    validity( x::token_t::TK_RIGHT_BRACKETS );

    auto ast = std::make_shared<x::function_decl_ast>();
    ast->set_location( _location );

    ast->set_name( "finalize" );

    verify( x::token_t::TK_LEFT_BRACKETS );
    verify( x::token_t::TK_RIGHT_BRACKETS );

    ast->set_body( compound_stat() );

    return ast;
}

x::function_decl_ast_ptr x::grammar::construct_decl()
{
    validity( x::token_t::TK_CONSTRUCT );

    auto ast = std::make_shared<x::function_decl_ast>();
    ast->set_location( _location );

    ast->set_name( "construct" );

    verify( x::token_t::TK_LEFT_BRACKETS );
    verify( x::token_t::TK_RIGHT_BRACKETS );

    ast->set_body( compound_stat() );

    return ast;
}

x::interface_decl_ast_ptr x::grammar::interface_decl()
{
    validity( x::token_t::TK_INTERFACE );

    auto ast = std::make_shared<x::interface_decl_ast>();
    ast->set_location( _location );

    ast->set_name( validity( x::token_t::TK_IDENTIFIER ).str );

    x::access_t acce = x::access_t::PRIVATE;
    x::attribute_ast_ptr attr = nullptr;
    validity( x::token_t::TK_LEFT_CURLY_BRACES );
    {
        if ( lookup().type == x::token_t::TK_ATTRIBUTE )
            attr = attribute();
        else
            attr = nullptr;

        acce = access();

        auto decl = function_decl( true );
        if ( attr )
        decl->set_attribute( attr );
        decl->set_access( acce );
        decl->set_is_virtual( true );
        ast->insert_function( decl );
    }
    validity( x::token_t::TK_RIGHT_CURLY_BRACES );

    return ast;
}

x::namespace_decl_ast_ptr x::grammar::namespace_decl()
{
    validity( x::token_t::TK_NAMESPACE );

    auto ast = std::make_shared<x::namespace_decl_ast>();
    ast->set_location( _location );

    ast->set_name( validity( x::token_t::TK_IDENTIFIER ).str );

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

        auto tk = lookup();
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
        case x::token_t::TK_VARIABLE:
            decl = variable_decl();
            break;
        case x::token_t::TK_FUNCTION:
            decl = function_decl();
            break;
        case x::token_t::TK_INTERFACE:
            decl = interface_decl();
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
            if ( attr )
                decl->set_attribute( attr );
            decl->set_access( acce );
            ast->insert_member( std::move( decl ) );

            acce = x::access_t::PRIVATE;
        }
    } );

    return ast;
}

x::stat_ast_ptr x::grammar::statement()
{
    switch ( lookup().type )
    {
    case x::token_t::TK_SEMICOLON:
        return nullptr;
    case x::token_t::TK_EXTERN:
        return extern_stat();
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
    case x::token_t::TK_AWAIT:
        return await_stat();
    case x::token_t::TK_YIELD:
        return yield_stat();
    case x::token_t::TK_BREAK:
        return break_stat();
    case x::token_t::TK_RETURN:
        return return_stat();
    case x::token_t::TK_TRY:
        return try_stat();
    case x::token_t::TK_THROW:
        return throw_stat();
    case x::token_t::TK_CONTINUE:
        return continue_stat();
    case x::token_t::TK_VARIABLE:
        return local_stat();
    default:
        return express();
    }
}

x::extern_stat_ast_ptr x::grammar::extern_stat()
{
    validity( x::token_t::TK_EXTERN );

    auto ast = std::make_shared<x::extern_stat_ast>();
    ast->set_location( _location );

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]()
    {
        ast->set_libname( validity( x::token_t::TK_CONSTEXPR_STRING ).str );
        validity( x::token_t::TK_COMMA );
        ast->set_funcname( validity( x::token_t::TK_CONSTEXPR_STRING ).str );
        if ( verify( x::token_t::TK_COMMA ) )
        {
            auto type = validity( x::token_t::TK_CONSTEXPR_STRING ).str;

            if ( type == "stdcall" )
                ast->set_call( x::callmode_t::MODE_STD );
            else if ( type == "fastcall" )
                ast->set_call( x::callmode_t::MODE_FAST );
            else if ( type == "thiscall" )
                ast->set_call( x::callmode_t::MODE_THIS );
            else
                ast->set_call( x::callmode_t::MODE_C );
        }
        else
        {
            ast->set_call( x::callmode_t::MODE_C );
        }
    } );

    validity( x::token_t::TK_SEMICOLON );

    return ast;
}

x::compound_stat_ast_ptr x::grammar::compound_stat()
{
    validity( x::token_t::TK_LEFT_CURLY_BRACES );

    auto ast = std::make_shared<x::compound_stat_ast>();
    ast->set_location( _location );

    while ( !verify( x::token_t::TK_RIGHT_CURLY_BRACES ) )
    {
        auto s = statement();

        if ( s && s->type() != x::ast_t::EMPTY_STAT )
        {
            ast->insert_stat( s );
        }

        if ( lookup().type == x::token_t::TK_SEMICOLON )
            next();
    }

    return ast;
}

x::await_stat_ast_ptr x::grammar::await_stat()
{
    validity( x::token_t::TK_AWAIT );

    auto ast = std::make_shared<x::await_stat_ast>();
    ast->set_location( _location );

    ast->set_exp( express() );

    return ast;
}

x::yield_stat_ast_ptr x::grammar::yield_stat()
{
    validity( x::token_t::TK_YIELD );

    auto ast = std::make_shared<x::yield_stat_ast>();
    ast->set_location( _location );

    if ( !verify( x::token_t::TK_SEMICOLON ) )
        ast->set_exp( express() );

    verify( x::token_t::TK_SEMICOLON );

    return ast;
}

x::if_stat_ast_ptr x::grammar::if_stat()
{
    validity( x::token_t::TK_IF );

    auto ast = std::make_shared<x::if_stat_ast>();
    ast->set_location( _location );

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->set_cond( express() );
    validity( x::token_t::TK_RIGHT_BRACKETS );

    ast->set_then_stat( statement() );

    if ( verify( x::token_t::TK_ELSE ) )
        ast->set_else_stat( statement() );

    return ast;
}

x::while_stat_ast_ptr x::grammar::while_stat()
{
    validity( x::token_t::TK_WHILE );

    auto ast = std::make_shared<x::while_stat_ast>();
    ast->set_location( _location );

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->set_cond( express() );
    validity( x::token_t::TK_RIGHT_BRACKETS );

    ast->set_body( statement() );

    return ast;
}

x::for_stat_ast_ptr x::grammar::for_stat()
{
    validity( x::token_t::TK_FOR );

    auto ast = std::make_shared<x::for_stat_ast>();
    ast->set_location( _location );

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->set_init( statement() );
    validity( x::token_t::TK_SEMICOLON );
    ast->set_cond( express() );
    validity( x::token_t::TK_SEMICOLON );
    ast->set_step( express() );
    validity( x::token_t::TK_RIGHT_BRACKETS );

    ast->set_body( statement() );

    return ast;
}

x::foreach_stat_ast_ptr x::grammar::foreach_stat()
{
    validity( x::token_t::TK_FOREACH );

    auto ast = std::make_shared<x::foreach_stat_ast>();
    ast->set_location( _location );

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->set_item( statement() );
    validity( x::token_t::TK_TYPECAST );
    ast->set_collection( express() );
    validity( x::token_t::TK_RIGHT_BRACKETS );

    ast->set_body( statement() );

    return ast;
}

x::switch_stat_ast_ptr x::grammar::switch_stat()
{
    validity( x::token_t::TK_SWITCH );

    auto ast = std::make_shared<x::switch_stat_ast>();
    ast->set_location( _location );

    validity( x::token_t::TK_LEFT_BRACKETS );
    ast->set_exp( express() );
    validity( x::token_t::TK_RIGHT_BRACKETS );

    validity( x::token_t::TK_LEFT_CURLY_BRACES );
    {
        while ( verify( x::token_t::TK_CASE ) )
        {
            x::switch_stat_ast::case_pair value;

            value.first = constant_exp();

            validity( x::token_t::TK_TYPECAST );
            
            value.second = compound_stat();

            ast->insert_case( value );
        }

        if ( verify( x::token_t::TK_DEFAULT ) )
        {
            validity( x::token_t::TK_TYPECAST );

            ast->set_defult( compound_stat() );
        }
    }
    validity( x::token_t::TK_RIGHT_CURLY_BRACES );

    return ast;
}

x::break_stat_ast_ptr x::grammar::break_stat()
{
    validity( x::token_t::TK_BREAK );

    auto ast = std::make_shared<x::break_stat_ast>();
    ast->set_location( _location );

    return ast;
}

x::return_stat_ast_ptr x::grammar::return_stat()
{
    validity( x::token_t::TK_RETURN );

    auto ast = std::make_shared<x::return_stat_ast>();
    ast->set_location( _location );

    if ( !verify( x::token_t::TK_SEMICOLON ) )
    {
        do
        {
            ast->insert_exp( express() );
        } while ( verify( x::token_t::TK_COMMA ) );
    }

    return ast;
}

x::try_stat_ast_ptr x::grammar::try_stat()
{
    validity( x::token_t::TK_TRY );

    auto ast = std::make_shared<x::try_stat_ast>();
    ast->set_location( _location );

    ast->set_try_stat( compound_stat() );

    while ( verify( x::token_t::TK_CATCH ) )
    {
        x::try_stat_ast::catch_pair cat;

        validity( x::token_t::TK_LEFT_BRACKETS );
        cat.first = parameter();
        validity( x::token_t::TK_RIGHT_BRACKETS );
        cat.second = compound_stat();

        ast->insert_catch_stat( cat );
    }

    if ( verify( x::token_t::TK_FINAL ) )
    {
        ast->set_final_stat( compound_stat() );
    }

    return ast;
}

x::throw_stat_ast_ptr x::grammar::throw_stat()
{
    validity( x::token_t::TK_THROW );

    auto ast = std::make_shared<x::throw_stat_ast>();
    ast->set_location( _location );

    ast->set_exception( express() );

    return ast;
}

x::continue_stat_ast_ptr x::grammar::continue_stat()
{
    validity( x::token_t::TK_CONTINUE );

    auto ast = std::make_shared<x::continue_stat_ast>();
    ast->set_location( _location );

    return ast;
}

x::stat_ast_ptr x::grammar::local_stat()
{
    validity( x::token_t::TK_VARIABLE );

    if ( !verify( x::token_t::TK_LEFT_INDEX ) )
    {
        auto ast = std::make_shared<x::local_stat_ast>();
        ast->set_location( _location );

        switch ( verify( { x::token_t::TK_LOCAL, x::token_t::TK_STATIC, x::token_t::TK_THREAD } ).type )
        {
        case x::token_t::TK_LOCAL: ast->set_is_local( true ); break;
        case x::token_t::TK_STATIC: ast->set_is_static( true ); break;
        case x::token_t::TK_THREAD: ast->set_is_thread( true ); break;
        }

        ast->set_name( validity( x::token_t::TK_IDENTIFIER ).str );

        if ( verify( x::token_t::TK_TYPECAST ) )
            ast->set_valuetype( type() );
        else
            ast->set_valuetype( anytype() );

        if ( verify( x::token_t::TK_ASSIGN ) )
        {
            if ( lookup().type == x::token_t::TK_LEFT_CURLY_BRACES )
            {
                ast->set_init( initializer_exp() );
            }
            else
            {
                auto init = std::make_shared<x::initializer_expr_ast>();
                init->set_location( _location );

                init->insert_arg( express() );

                ast->set_init( init );
            }
        }

        return ast;
    }
    else
    {
        auto ast = std::make_shared<x::mulocal_stat_ast>();
        ast->set_location( _location );

        do
        {
            auto local = std::make_shared<x::local_stat_ast>();
            local->set_location( _location );

            local->set_name( validity( x::token_t::TK_IDENTIFIER ).str );
            local->set_valuetype( autotype() );

            ast->insert_local( local );
        } while ( verify( x::token_t::TK_COMMA ) );

        validity( x::token_t::TK_RIGHT_INDEX );

        validity( x::token_t::TK_ASSIGN );

        ast->set_init( initializer_exp() );

        return ast;
    }
}

x::expr_stat_ast_ptr x::grammar::express()
{
    return assignment_exp();
}

x::expr_stat_ast_ptr x::grammar::assignment_exp()
{
    x::expr_stat_ast_ptr ast = logical_or_exp();

    auto binary_ast = [this]( auto left, auto op )
    {
        auto ast = std::make_shared<x::binary_expr_ast>();
        ast->set_location( _location );

        ast->set_op( op );
        ast->set_left( left );
        ast->set_right( assignment_exp() );

        return ast;
    };

    for ( bool is_break = false; !is_break; )
    {
        switch ( lookup().type )
        {
        case x::token_t::TK_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::ASSIGN ); break;
        case x::token_t::TK_ADD_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::ADD_ASSIGN ); break;
        case x::token_t::TK_SUB_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::SUB_ASSIGN ); break;
        case x::token_t::TK_MUL_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::MUL_ASSIGN ); break;
        case x::token_t::TK_DIV_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::DIV_ASSIGN ); break;
        case x::token_t::TK_MOD_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::MOD_ASSIGN ); break;
        case x::token_t::TK_AND_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::ADD_ASSIGN ); break;
        case x::token_t::TK_OR_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::OR_ASSIGN ); break;
        case x::token_t::TK_XOR_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::XOR_ASSIGN ); break;
        case x::token_t::TK_LSHIFT_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::LSHIFT_ASSIGN ); break;
        case x::token_t::TK_RSHIFT_ASSIGN: next(); ast = binary_ast( ast, x::operator_t::RSHIFT_ASSIGN ); break;
        default: is_break = true; break;
        }
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::logical_or_exp()
{
    x::expr_stat_ast_ptr ast = logical_and_exp();

    while ( verify( x::token_t::TK_LOR ) )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::LOR );
        exp->set_left( ast );
        exp->set_right( logical_and_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::logical_and_exp()
{
    x::expr_stat_ast_ptr ast = or_exp();

    while ( verify( x::token_t::TK_LAND ) )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::LAND );
        exp->set_left( ast );
        exp->set_right( or_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::or_exp()
{
    x::expr_stat_ast_ptr ast = xor_exp();

    while ( verify( x::token_t::TK_OR ) )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::OR );
        exp->set_left( ast );
        exp->set_right( xor_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::xor_exp()
{
    x::expr_stat_ast_ptr ast = and_exp();

    while ( verify( x::token_t::TK_XOR ) )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::XOR );
        exp->set_left( ast );
        exp->set_right( and_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::and_exp()
{
    x::expr_stat_ast_ptr ast = compare_exp();

    while ( verify( x::token_t::TK_AND ) )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::AND );
        exp->set_left( ast );
        exp->set_right( compare_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::compare_exp()
{
    x::expr_stat_ast_ptr ast = shift_exp();

    auto binary_ast = [this]( auto left, auto op )
    {
        auto ast = std::make_shared<x::binary_expr_ast>();
        ast->set_location( _location );

        ast->set_op( op );
        ast->set_left( left );
        ast->set_right( shift_exp() );

        return ast;
    };

    for ( bool is_break = false; !is_break; )
    {
        switch ( lookup().type )
        {
        case x::token_t::TK_EQUAL: next(); ast = binary_ast( ast, x::operator_t::EQUAL ); break;
        case x::token_t::TK_NOT_EQUAL: next(); ast = binary_ast( ast, x::operator_t::NOT_EQUAL ); break;
        case x::token_t::TK_LESS: next(); ast = binary_ast( ast, x::operator_t::LESS ); break;
        case x::token_t::TK_LARG: next(); ast = binary_ast( ast, x::operator_t::LARG ); break;
        case x::token_t::TK_LESS_EQUAL: next(); ast = binary_ast( ast, x::operator_t::LESS_EQUAL ); break;
        case x::token_t::TK_LARG_EQUAL: next(); ast = binary_ast( ast, x::operator_t::LARG_EQUAL ); break;
        case x::token_t::TK_COMPARE: next(); ast = binary_ast( ast, x::operator_t::COMPARE ); break;
        default: is_break = true; break;
        }
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::shift_exp()
{
    x::expr_stat_ast_ptr ast = add_exp();

    while ( lookup().type == x::token_t::TK_LEFT_SHIFT || lookup().type == x::token_t::TK_RIGHT_SHIFT )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( ( next().type == x::token_t::TK_LEFT_SHIFT ) ? x::operator_t::LEFT_SHIFT : x::operator_t::RIGHT_SHIFT );
        exp->set_left( ast );
        exp->set_right( add_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::add_exp()
{
    x::expr_stat_ast_ptr ast = mul_exp();

    while ( lookup().type == x::token_t::TK_ADD || lookup().type == x::token_t::TK_SUB )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( ( next().type == x::token_t::TK_ADD ) ? x::operator_t::ADD : x::operator_t::SUB );
        exp->set_left( ast );
        exp->set_right( mul_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::mul_exp()
{
    x::expr_stat_ast_ptr ast = as_exp();

    while ( lookup().type == x::token_t::TK_MUL || lookup().type == x::token_t::TK_DIV || lookup().type == x::token_t::TK_MOD )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        switch ( next().type )
        {
        case x::token_t::TK_MUL: exp->set_op( x::operator_t::MUL ); break;
        case x::token_t::TK_DIV: exp->set_op( x::operator_t::DIV ); break;
        case x::token_t::TK_MOD: exp->set_op( x::operator_t::MOD ); break;
        }
        exp->set_left( ast );
        exp->set_right( as_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::as_exp()
{
    x::expr_stat_ast_ptr ast = is_exp();

    while ( verify( x::token_t::TK_AS ) )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::AS );
        exp->set_left( ast );
        exp->set_right( is_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::is_exp()
{
    x::expr_stat_ast_ptr ast = sizeof_exp();

    while ( verify( x::token_t::TK_IS ) )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::IS );
        exp->set_left( ast );
        exp->set_right( sizeof_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::sizeof_exp()
{
    if ( verify( x::token_t::TK_SIZEOF ) )
    {
        verify( x::token_t::TK_LEFT_BRACKETS );

        auto exp = std::make_shared<x::unary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::SIZEOF );
        exp->set_exp( typeof_exp() );

        validity( x::token_t::TK_RIGHT_BRACKETS );

        return exp;
    }

    return typeof_exp();
}

x::expr_stat_ast_ptr x::grammar::typeof_exp()
{
    if ( verify( x::token_t::TK_TYPEOF ) )
    {
        verify( x::token_t::TK_LEFT_BRACKETS );

        auto exp = std::make_shared<x::unary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::TYPEOF );
        exp->set_exp( invoke_exp() );

        validity( x::token_t::TK_RIGHT_BRACKETS );

        return exp;
    }

    return invoke_exp();
}

x::expr_stat_ast_ptr x::grammar::invoke_exp()
{
    x::expr_stat_ast_ptr ast = member_exp();

    while ( lookup().type == x::token_t::TK_LEFT_BRACKETS )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::INVOKE );
        exp->set_left( ast );
        exp->set_right( arguments_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::member_exp()
{
    x::expr_stat_ast_ptr ast = unary_exp();

    while ( verify( x::token_t::TK_POINT ) )
    {
        auto exp = std::make_shared<x::binary_expr_ast>();
        exp->set_location( _location );

        exp->set_op( x::operator_t::MEMBER );
        exp->set_left( ast );
        exp->set_right( unary_exp() );

        ast = exp;
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::unary_exp()
{
    auto unary_ast = [this]( auto op )
    {
        auto ast = std::make_shared<x::unary_expr_ast>();
        ast->set_location( _location );
        ast->set_op( op );
        ast->set_exp( postfix_exp() );
        return ast;
    };

    switch ( lookup().type )
    {
    case x::token_t::TK_INC: next(); return unary_ast( x::operator_t::INC );
    case x::token_t::TK_DEC: next(); return unary_ast( x::operator_t::DEC );
    case x::token_t::TK_REV: next(); return unary_ast( x::operator_t::REV );
    case x::token_t::TK_NOT: next(); return unary_ast( x::operator_t::NOT );
    case x::token_t::TK_ADD: next(); return unary_ast( x::operator_t::PLUS );
    case x::token_t::TK_SUB: next(); return unary_ast( x::operator_t::MINUS );
    }

    return postfix_exp();
}

x::expr_stat_ast_ptr x::grammar::postfix_exp()
{
    auto unary_ast = [this]( auto exp, auto op )
    {
        auto ast = std::make_shared<x::unary_expr_ast>();
        ast->set_location( _location );
        ast->set_op( op );
        ast->set_exp( exp );
        return ast;
    };

    x::expr_stat_ast_ptr ast = primary_exp();

    for ( bool is_break = false; !is_break; )
    {
        switch ( lookup().type )
        {
        case x::token_t::TK_INC: next(); ast = unary_ast( ast, x::operator_t::POSTINC ); break;
        case x::token_t::TK_DEC: next(); ast = unary_ast( ast, x::operator_t::POSTDEC ); break;
        default: is_break = true; break;
        }
    }

    return ast;
}

x::expr_stat_ast_ptr x::grammar::primary_exp()
{
    x::expr_stat_ast_ptr ast = nullptr;

    switch ( lookup().type )
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
    case x::token_t::TK_THIS:
    case x::token_t::TK_BASE:
    case x::token_t::TK_IDENTIFIER:
        ast = identifier_exp();
        break;
    case x::token_t::TK_NEW:
        ast = new_exp();
        break;
    case x::token_t::TK_FUNCTION:
        ast = closure_exp();
        break;
    case x::token_t::TK_LEFT_INDEX:
        ast = elements_exp();
        break;
    case x::token_t::TK_LEFT_BRACKETS:
        ast = arguments_exp();
        break;
    case x::token_t::TK_LEFT_CURLY_BRACES:
        ast = initializer_exp();
        break;
    default:
        ast = constant_exp();
        break;
    }

    return ast;
}

x::new_expr_ast_ptr x::grammar::new_exp()
{
    validity( x::token_t::TK_NEW );

    auto ast = std::make_shared<x::new_expr_ast>();
    ast->set_location( _location );

    ast->set_type( type() );
    ast->set_init_stat( initializer_exp() );

    return ast;
}

x::bracket_expr_ast_ptr x::grammar::bracket_exp()
{
    auto ast = std::make_shared<x::bracket_expr_ast>();
    ast->set_location( _location );
    
    validity( x::token_t::TK_LEFT_BRACKETS );

    ast->set_exp( express() );

    validity( x::token_t::TK_RIGHT_BRACKETS );

    return ast;
}

x::closure_expr_ast_ptr x::grammar::closure_exp()
{
    validity( x::token_t::TK_FUNCTION );

    auto ast = std::make_shared<x::closure_expr_ast>();
    ast->set_location( _location );

    ast->set_name( std::format( "closure_{}_{}_{}", _location.line, _location.col, x::time_2_stamp( std::chrono::system_clock::now() ) ) );

    verify_list( x::token_t::TK_LEFT_INDEX, x::token_t::TK_RIGHT_INDEX, x::token_t::TK_COMMA, [&]()
    {
        ast->insert_capture( identifier_exp() );
    } );

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]()
    {
        ast->insert_parameter( parameter() );
    } );

    if ( verify( x::token_t::TK_FUNCTION_RESULT ) )
    {
        while ( 1 )
        {
            ast->insert_result( type() );

            if ( !verify( x::token_t::TK_COMMA ) )
                break;
        }
    }

    ast->set_body( compound_stat() );

    return ast;
}

x::elements_expr_ast_ptr x::grammar::elements_exp()
{
    auto ast = std::make_shared<x::elements_expr_ast>();
    ast->set_location( _location );

    verify_list( x::token_t::TK_LEFT_INDEX, x::token_t::TK_RIGHT_INDEX, x::token_t::TK_COMMA, [&]()
    {
        ast->insert_element( arguments_exp() );
    } );

    return ast;
}

x::arguments_expr_ast_ptr x::grammar::arguments_exp()
{
    auto ast = std::make_shared<x::arguments_expr_ast>();
    ast->set_location( _location );

    verify_list( x::token_t::TK_LEFT_BRACKETS, x::token_t::TK_RIGHT_BRACKETS, x::token_t::TK_COMMA, [&]()
    {
        ast->insert_arg( assignment_exp() );
    } );

    return ast;
}

x::identifier_expr_ast_ptr x::grammar::identifier_exp()
{
    auto ast = std::make_shared<x::identifier_expr_ast>();
    ast->set_location( _location );

    switch( lookup().type )
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
	case x::token_t::TK_THIS:
	case x::token_t::TK_BASE:
        ast->set_ident( next().str );
        break;
	case x::token_t::TK_IDENTIFIER:
        ast->set_ident( next().str );
        break;
    default:
        XTHROW( x::syntax_exception, true, "", _location );
        break;
    }
    
    return ast;
}

x::initializer_expr_ast_ptr x::grammar::initializer_exp()
{
    auto ast = std::make_shared<x::initializer_expr_ast>();
    ast->set_location( _location );

    verify_list( x::token_t::TK_LEFT_CURLY_BRACES, x::token_t::TK_RIGHT_CURLY_BRACES, x::token_t::TK_COMMA, [&]()
    {
        ast->insert_arg( express() );
    } );

    return ast;
}

x::constant_expr_ast_ptr x::grammar::constant_exp()
{
    x::constant_expr_ast_ptr ast = nullptr;

    switch ( lookup().type )
    {
    case x::token_t::TK_NULL:
        ast = null_constant_exp();
        break;
    case x::token_t::TK_TRUE:
        ast = true_constant_exp();
        break;
    case x::token_t::TK_FALSE:
        ast = false_constant_exp();
        break;
    case x::token_t::TK_CONSTEXPR_INT:
        ast = int_constant_exp();
        break;
    case x::token_t::TK_CONSTEXPR_FLOAT:
        ast = float_constant_exp();
        break;
    case x::token_t::TK_CONSTEXPR_STRING:
        ast = string_constant_exp();
        break;
    }

    return ast;
}

x::constant_expr_ast_ptr x::grammar::null_constant_exp()
{
    validity( x::token_t::TK_NULL );

    auto ast = std::make_shared<x::null_constant_expr_ast>();
    ast->set_location( _location );

    return ast;
}

x::constant_expr_ast_ptr x::grammar::true_constant_exp()
{
    validity( x::token_t::TK_TRUE );

    auto ast = std::make_shared<x::bool_constant_expr_ast>();
    ast->set_location( _location );

    ast->set_value( true );

    return ast;
}

x::constant_expr_ast_ptr x::grammar::false_constant_exp()
{
    validity( x::token_t::TK_FALSE );

    auto ast = std::make_shared<x::bool_constant_expr_ast>();
    ast->set_location( _location );

    ast->set_value( false );

    return ast;
}

x::constant_expr_ast_ptr x::grammar::int_constant_exp()
{
    x::constant_expr_ast_ptr ast;

    auto str = validity( x::token_t::TK_CONSTEXPR_INT ).str;

    if ( str[0] == '-' )
    {
        x::int64 i64 = 0;
        std::from_chars( str.c_str(), str.c_str() + str.size(), i64 );

        if ( std::abs( i64 ) < std::numeric_limits<x::int32>::max() )
        {
            auto i_ast = std::make_shared<x::int32_constant_expr_ast>();
            i_ast->set_location( _location );

            i_ast->set_value( static_cast<x::int32>( i64 ) );

            ast = i_ast;
        }
        else
        {
            auto i_ast = std::make_shared<x::int64_constant_expr_ast>();
            i_ast->set_location( _location );

            i_ast->set_value( i64 );

            ast = i_ast;
        }
    }
    else
    {
        x::uint64 u64 = 0;
        std::from_chars( str.c_str(), str.c_str() + str.size(), u64 );

        if ( u64 < std::numeric_limits<x::int32>::max() )
        {
            auto i_ast = std::make_shared<x::int32_constant_expr_ast>();
            i_ast->set_location( _location );

            i_ast->set_value( static_cast<x::int32>( u64 ) );

            ast = i_ast;
        }
        else if ( u64 < (x::uint64)std::numeric_limits<x::int64>::max() )
        {
            auto i_ast = std::make_shared<x::int64_constant_expr_ast>();
            i_ast->set_location( _location );

            i_ast->set_value( static_cast<x::int64>( u64 ) );

            ast = i_ast;
        }
        else if ( u64 < std::numeric_limits<x::uint32>::max() )
        {
            auto i_ast = std::make_shared<x::uint32_constant_expr_ast>();
            i_ast->set_location( _location );

            i_ast->set_value( static_cast<x::uint32>( u64 ) );

            ast = i_ast;
        }
        else
        {
            auto i_ast = std::make_shared<x::uint64_constant_expr_ast>();
            i_ast->set_location( _location );

            i_ast->set_value( u64 );

            ast = i_ast;
        }
    }

    return ast;
}

x::constant_expr_ast_ptr x::grammar::float_constant_exp()
{
    x::float_constant_expr_ast_ptr ast;

    auto str = validity( x::token_t::TK_CONSTEXPR_FLOAT ).str;

    auto beg = str.c_str(); if ( *beg == '-' ) ++beg;
    auto dom = str.c_str() + str.find( '.' );
    auto end = str.c_str() + str.size();

    x::uint64 mantissa = 0, exponent = 0;
    std::from_chars( beg, dom, mantissa );
    std::from_chars( dom + 1, end, exponent );

    int man_cnt = std::numeric_limits<x::uint64>::digits - std::countl_zero( mantissa );
    if ( man_cnt < std::numeric_limits<x::float32>::digits && exponent < std::numeric_limits<x::float32>::max_exponent )
    {
        x::float32 value;
        std::from_chars( str.c_str(), str.c_str() + str.size(), value );

        auto f_ast = std::make_shared<x::float32_constant_expr_ast>();
        f_ast->set_location( _location );

        f_ast->set_value( value );

        ast = f_ast;
    }
    else
    {
        x::float64 value;
        std::from_chars( str.c_str(), str.c_str() + str.size(), value );

        auto f_ast = std::make_shared<x::float64_constant_expr_ast>();
        f_ast->set_location( _location );

        f_ast->set_value( value );

        ast = f_ast;
    }

    return ast;
}

x::constant_expr_ast_ptr x::grammar::string_constant_exp()
{
    auto ast = std::make_shared<x::string_constant_expr_ast>();
    ast->set_location( _location );

    ast->set_value( validity( x::token_t::TK_CONSTEXPR_STRING ).str );

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
        case x::token_t::TK_IDENTIFIER:
            name += next().str;
            break;
        default:
            XTHROW( x::syntax_exception, true, "", _location );
            break;
        }
    } while ( verify( x::token_t::TK_POINT ) );

    return name;
}

x::type_ast_ptr x::grammar::anytype()
{
    auto ast = std::make_shared<x::type_ast>();
    ast->set_location( _location );
    ast->set_name( "any" );
    return ast;
}

x::type_ast_ptr x::grammar::autotype()
{
    auto ast = std::make_shared<x::type_ast>();
    ast->set_location( _location );
    ast->set_name( "auto" );
    return ast;
}

x::type_ast_ptr x::grammar::anylisttype()
{
    auto ast = std::make_shared<x::list_type_ast>();
    ast->set_location( _location );
    ast->set_name( "any" );
    return ast;
}

x::token x::grammar::next()
{
    x::token tk;
    tk.location = _location;
    tk.type = x::token_t::TK_EOF;

    while ( !_stream->eof() )
    {
        char32_t c = get();

        if ( c == char32_t( -1 ) )
            continue;

        // space \r \n \t \0 ' '
        if ( std::isspace( c ) )
        {
            continue;
        }
        // number 0 1 -1 233 0x123456 0b1101001 123.456
        else if ( std::isdigit( c ) || ( c == '-' && std::isdigit( peek() ) ) )
        {
            if ( c == '-' )
            {
                push( tk.str, c );
                c = get();
            }

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

                tk.str = std::to_string( std::stoll( tk.str, nullptr, 16 ) );
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
                for ( int64_t i = (int64_t)tk.str.size() - 1, j = 0; i >= 0; i--, j++ )
                {
                    if ( tk.str[i] == '1' )
                        n |= ( int64_t( 1 ) << ( j ) );
                }

                if ( tk.str.front() == '-' )
                    n = -n;

                tk.str = std::to_string( n );
            }
            else
            {
                push( tk.str, c );

                c = peek();
                while ( std::isdigit( c ) )
                {
                    push( tk.str, get() );

                    c = peek();
                }

                if ( c == '.' )
                {
                    push( tk.str, get() );

                    c = peek();
                    while ( std::isdigit( c ) )
                    {
                        push( tk.str, get() );

                        c = peek();
                    }
                }
            }

            tk.type = ( tk.str.find( '.' ) == std::string::npos ) ? x::token_t::TK_CONSTEXPR_INT : x::token_t::TK_CONSTEXPR_FLOAT;

            break;
        }
        // string "..."
        else if ( c == '\"' )
        {
            while ( peek() != '\"' )
            {
                c = get();

                if ( c == '\\' )
                {
                    c = get();

                    switch ( c )
                    {
                    case '\'':
                        c = '\'';
                        break;
                    case '\"':
                        c = '\"';
                        break;
                    case '\\':
                        c = '\\';
                        break;
                    case 'n':
                        c = '\n';
                        break;
                    case 'r':
                        c = '\r';
                        break;
                    case 't':
                        c = '\t';
                        break;
                    default:
                        push( tk.str, '\\' );
                        break;
                    }
                }

                push( tk.str, c );
            }

            get();

            tk.type = x::token_t::TK_CONSTEXPR_STRING;

            break;
        }
        // raw string R"(...)"
        else if ( c == 'R' && peek() == '\"' )
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

            tk.type = x::token_t::TK_CONSTEXPR_STRING;

            break;
        }
        // identifier
        else if ( std::isalpha( c ) || c == '_' || c > 127 )
        {
            push( tk.str, c );

            while ( std::isalnum( peek() ) || peek() == '_' || peek() > 127 ) push( tk.str, get() );

            auto it = _tokenmap->find( tk.str );
            tk.type = it != _tokenmap->end() ? it->second : x::token_t::TK_IDENTIFIER;

            break;
        }
        // note
        else if ( c == '/' && ( peek() == '/' || peek() == '*' ) )
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
        // operator
        else if ( std::ispunct( c ) )
        {
            push( tk.str, c );

            XTHROW( x::syntax_exception, _tokenmap->find( tk.str ) == _tokenmap->end(), "", _location );

            tk.type = _tokenmap->find( tk.str )->second;

            if ( rsl_2_2larg && tk.type == x::token_t::TK_LARG && peek() == '>' )
                break;

            while ( std::ispunct( peek() ) )
            {
                auto s = tk.str; push( s, peek() );

                if ( _tokenmap->find( s ) == _tokenmap->end() )
                    break;

                push( tk.str, get() );
                tk.type = _tokenmap->find( tk.str )->second;
            }

            break;
        }
        // exception
        else
        {
            XTHROW( x::lexical_exception, true, "", _location );
        }
    }

    return tk;
}

x::token x::grammar::lookup()
{
    auto line = _location.line;
    auto col = _location.col;
    auto pos = _stream->tellg();

    auto tk = next();

    _stream->seekg( pos );
    _location.col = col;
    _location.line = line;

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

x::token x::grammar::validity( x::token_t k )
{
    auto tk = lookup();
    XTHROW( x::lexical_exception, tk.type != k, "", _location );
    return next();
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

char32_t x::grammar::get()
{
    char32_t c = _stream->get();

    if ( _stream->eof() )
        return -1;

    if ( ( c & 0b10000000 ) != 0 )
    {
        if ( ( c & 0b11111100 ) == 0b11111100 )
        {
            int shift = 5 * 6;
            c = ( c & 0b00000001 ) << shift; shift -= 2;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
        }
        else if ( ( c & 0b11111000 ) == 0b11111000 )
        {
            int shift = 4 * 6;
            c = ( c & 0b00000011 ) << shift; shift -= 3;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
        }
        else if ( ( c & 0b11110000 ) == 0b11110000 )
        {
            int shift = 3 * 6;
            c = ( c & 0b00000111 ) << shift; shift -= 4;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
        }
        else if ( ( c & 0b11100000 ) == 0b11100000 )
        {
            int shift = 2 * 6;
            c = ( c & 0b00001111 ) << shift; shift -= 5;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
        }
        else if ( ( c & 0b11000000 ) == 0b11000000 )
        {
            int shift = 1 * 6;
            c = ( c & 0b00011111 ) << shift; shift -= 6;
            c |= ( _stream->get() & 0x3F ) << shift; shift -= 6;
        }
    }

    if ( c == '\n' )
    {
        _location.line++;
        _location.col = 1;
    }
    else
    {
        _location.col++;
    }

    return c;
}

char32_t x::grammar::peek()
{
    auto line = _location.line;
    auto col = _location.col;
    auto pos = _stream->tellg();

    auto c = get();

    _stream->seekg( pos );
    _location.col = col;
    _location.line = line;

    return c;
}

void x::grammar::push( std::string & str, char32_t c ) const
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
        str.push_back( (char)c );
    }
}
