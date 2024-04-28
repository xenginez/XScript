#pragma once

#include <memory>
#include <vector>
#include <string>
#include <format>
#include <cassert>
#include <exception>

#include "float16.h"
#include "static_string_view.hpp"

#ifdef _DEBUG
#define ASSERT( a, s ) assert( ( a ) && s );
#else
#define ASSERT( a, s ) if ( ( a ) ) throw std::exception( s );
#endif // _DEBUG

namespace x
{
    static constexpr const int magic_num = 'xsl\0';
    static constexpr const int version_num = '0001';

    using int8 = std::int8_t;
    using int16 = std::int16_t;
    using int32 = std::int32_t;
    using int64 = std::int64_t;
    using uint8 = std::uint8_t;
    using uint16 = std::uint16_t;
    using uint32 = std::uint32_t;
    using uint64 = std::uint64_t;
    using float32 = float;
    using float64 = double;

    enum class ast_t
    {
        UNIT,
        TYPE,
        IMPORT,

        ENUM_DECL,
        FLAG_DECL,
        CLASS_DECL,
        USING_DECL,
        ENUM_ELEMENT,
        FLAG_ELEMENT,
        TEMPLATE_DECL,
        VARIABLE_DECL,
        FUNCTION_DECL,
        PARAMETER_DECL,
        NAMESPACE_DECL,

        EMPTY_STAT,
        EXTERN_STAT,
        COMPOUND_STAT,
        AWAIT_STAT,
        YIELD_STAT,
        TRY_STAT,
        CATCH_STAT,
        THROW_STAT,
        IF_STAT,
        WHILE_STAT,
        FOR_STAT,
        FOREACH_STAT,
        BREAK_STAT,
        RETURN_STAT,
        CONTINUE_STAT,
        LOCAL_STAT,

        ASSIGNMENT_EXP,
        CONDITIONAL_EXP,
        LOGICAL_OR_EXP,
        LOGICAL_AND_EXP,
        OR_EXP,
        XOR_EXP,
        AND_EXP,
        COMPARE_EXP,
        SHIFT_EXP,
        ADD_EXP,
        MUL_EXP,
        AS_EXP,
        IS_EXP,
        UNARY_EXP,
        POSTFIX_EXP,
        INDEX_EXP,
        INVOKE_EXP,
        MEMBER_EXP,
        IDENTIFIER_EXP,
        CLOSURE_EXP,
        ARGUMENTS_EXP,
        INITIALIZERS_EXP,
        CONST_EXP,
        NULL_CONST_EXP,
        BOOL_CONST_EXP,
        INT8_CONST_EXP,
        INT16_CONST_EXP,
        INT32_CONST_EXP,
        INT64_CONST_EXP,
        UINT8_CONST_EXP,
        UINT16_CONST_EXP,
        UINT32_CONST_EXP,
        UINT64_CONST_EXP,
        FLOAT16_CONST_EXP,
        FLOAT32_CONST_EXP,
        FLOAT64_CONST_EXP,
        STRING_CONST_EXP,
    };

    enum class meta_t
    {
        ENUM,
        FLAG,
        CLASS,
        VARIABLE,
        FUNCTION,
        NAMESPACE,
    };

    enum class token_t
    {
        TK_EOF = 0,
        TK_IDENTIFIER,           // identifier
        TK_LITERAL_INT,          // 1 -233 0x123456 0b1101001
        TK_LITERAL_FLOAT,        // 10.234 -432.01
        TK_LITERAL_STRING,       // "" "helloworld" R"(println("hello");)"
        TK_SEMICOLON,            // ;
        TK_COMMA,                // ,
        TK_INC,                  // ++
        TK_DEC,                  // --
        TK_ADD,                  // +
        TK_SUB,                  // - 
        TK_MUL,                  // * 
        TK_DIV,                  // / 
        TK_MOD,                  // %
        TK_AND,                  // &
        TK_OR,                   // |
        TK_XOR,                  // ^
        TK_LEFT_SHIFT,           // <<
        TK_RIGHT_SHIFT,          // >> 
        TK_LAND,                 // &&
        TK_LOR,                  // ||
        TK_LNOT,                 // !
        TK_NOT,                  // ~
        TK_ASSIGN,               // =
        TK_ADD_ASSIGN,           // +=
        TK_SUB_ASSIGN,           // -= 
        TK_MUL_ASSIGN,           // *=
        TK_DIV_ASSIGN,           // /= 
        TK_MOD_ASSIGN,           // %= 
        TK_AND_ASSIGN,           // &= 
        TK_OR_ASSIGN,            // |= 
        TK_XOR_ASSIGN,           // ^= 
        TK_LSHIFT_EQUAL,         // <<=
        TK_RSHIFT_EQUAL,         // >>=
        TK_EQUAL,                // ==
        TK_NOT_EQUAL,            // !=
        TK_LESS,                 // <
        TK_LARG,                 // >
        TK_LESS_OR_EQUAL,        // <=
        TK_LARG_OR_EQUAL,        // >=
        TK_TYPECAST,             // :
        TK_MEMBER_POINT,         // .
        TK_QUESTION,             // ?
        TK_VARIADIC_SIGN,        // ...
        TK_LEFT_INDEX,           // [
        TK_RIGHT_INDEX,          // ]
        TK_FUNCTION_RESULT,      // ->
        TK_LEFT_BRACKETS,        // (
        TK_RIGHT_BRACKETS,       // )
        TK_LEFT_CURLY_BRACES,    // {
        TK_RIGHT_CURLY_BRACES,   // }
        TK_VOID,                 // void
        TK_BYTE,                 // byte
        TK_BOOL,                 // bool
        TK_ANY,                  // any
        TK_INT8,                 // int8
        TK_INT16,                // int16
        TK_INT32,                // int32
        TK_INT64,                // int64
        TK_UINT8,                // uint8
        TK_UINT16,               // uint16
        TK_UINT32,               // uint32
        TK_UINT64,               // uint64
        TK_FLOAT16,              // float16
        TK_FLOAT32,              // float32
        TK_FLOAT64,              // float64
        TK_STRING,               // string
        TK_IMPORT,               // import
        TK_TEMPLATE,             // template
        TK_NAMESPACE,            // namespace
        TK_USING,                // using
        TK_ENUM,                 // enum
        TK_FLAG,                 // flag
        TK_CLASS,                // class
        TK_VARIABLE,             // var
        TK_FUNCTION,             // func
        TK_REF,                  // ref
        TK_PRIVATE,              // private
        TK_PUBLIC,               // public
        TK_PROTECTED,            // protected
        TK_CONST,                // const
        TK_STATIC,               // static
        TK_EXTERN,               // extern
        TK_NATIVE,               // native
        TK_THREAD,               // thread_local
        TK_WHILE,                // while
        TK_IF,                   // if
        TK_ELSE,                 // else
        TK_FOR,                  // for
        TK_FOREACH,              // foreach
        TK_CASE,                 // case
        TK_DEFAULT,              // default
        TK_TRY,                  // try
        TK_CATCH,                // catch
        TK_THROW,                // throw
        TK_ASYNC,                // async
        TK_AWAIT,                // await
        TK_YIELD,                // yield
        TK_BREAK,                // break
        TK_RETURN,               // return
        TK_CONTINUE,             // continue
        TK_NULL,                 // null
        TK_TRUE,                 // true
        TK_FALSE,                // flase
        TK_AS,                   // as
        TK_IS,                   // is
        TK_SIZEOF,               // sizeof
    };

    enum class value_t
    {
        INVALID,

        NIL,
        BYTE,
        BOOL,
        INT8,
        INT16,
        INT32,
        INT64,
        UINT8,
        UINT16,
        UINT32,
        UINT64,
        FLOAT16,
        FLOAT32,
        FLOAT64,
        STRING,
        OBJECT,
        CLOSURE,
    };

    enum class symbol_t
    {
        ENUM,
        LOOP,
        BLOCK,
        ALIAS,
        PARAM,
        LOCAL,
        CLASS,
        ELEMENT,
        TEMPLATE,
        VARIABLE,
        FUNCTION,
        NAMESPACE,
    };

    enum class access_t
    {
        PUBLIC,
        PRIVATE,
        PROTECTED,
    };

    enum class section_t
    {
        TYPE,
        DEPEND,
        GLOBAL,
        FUNCTION,
        VARIABLE,
        CODEDATA,
        STRINGDATA,
        CUSTOMDATA,
    };

    enum class bytecode_t
    {
        BC_NOP = 0,       // nop(TKPLS)                                                     1 byte
        BC_MOV,           // mov(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        BC_ADDI,          // add(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        BC_SUBI,          // sub
        BC_MULI,          // mul
        BC_DIVI,          // div
        BC_MODI,          // mod
        BC_ADDR,          // add(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        BC_SUBR,          // sub
        BC_MULR,          // mul
        BC_DIVR,          // div
        BC_MODR,          // mod
        BC_ADDH,          // add(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        BC_SUBH,          // sub
        BC_ADDS,          // add(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        BC_PSH,           // psh(dr_0)          REGID(1BYTE)/DIFF(4BYTE)                    2-5 byte
        BC_POP,           // pop(dr_STORED?)    REGID(1BYTE)/DIFF(4BYTE)/COUNT(2BYTE)       2-5 byte
        BC_SIDARR,        // sidarr(dr)         REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  REGID
        BC_SIDSTRUCT,     // sidstruct(dr)      REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  REGID
        BC_LDS,           // lds(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF   
        BC_STS,           // sts(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  
        BC_EQUB,          // equb(dr)           REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        BC_NEQUB,         // nequb
        BC_LTI,           // lt
        BC_GTI,           // gt
        BC_ELTI,          // elt
        BC_EGTI,          // egt
        BC_LAND,          // land             
        BC_LOR,           // lor
        BC_SIDMAP,        // sidmap(dr)         REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  REGID
        BC_LTX,           // lt
        BC_GTX,           // gt
        BC_ELTX,          // elt
        BC_EGTX,          // egt
        BC_LTR,           // lt
        BC_GTR,           // gt
        BC_ELTR,          // elt
        BC_EGTR,          // egt
        BC_CALL,          // call(dr_0)         REGID(1BYTE)/DIFF(4BYTE) 
        BC_CALLN,         // calln(0_ISNATIVE)  VM_IR_DIFF(4BYTE)/NATIVE_FUNC(8BYTE)
        BC_RET,           // ret(dr_0)          POP_SIZE(2 BYTE if dr)
        BC_JT,            // jt                 DIFF(4BYTE)
        BC_JF,            // jf                 DIFF(4BYTE)
        BC_JMP,           // jmp                DIFF(4BYTE)
        BC_MOVCAST,       // movcast(dr)        REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF TYPE  4-10 byte
        BC_MKCLOS,        // mkclos(00)         FUNC(8BYTE) CAPTURE_ARG_COUNT(2BYTE) 11 byte
        BC_TYPEAS,        // typeas(dr_0)       REGID(1BYTE)/DIFF(4BYTE) TYPE             3-6 byte
        BC_MKSTRUCT,      // mkstruct(dr_0)     REGID(1BYTE)/DIFF(4BYTE) SZ(2BYTE)               4-7 byte
        BC_ABRT,          // abrt(0_1/0)        (0xcc 0xcd can use it to abort)     
        BC_IDARR,         // idarr(dr)          REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF [Used for array]
        BC_IDDICT,        // iddict(dr)         REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF [Used for dict]
        BC_MKARR,         // mkarr(dr_0)        REGID(1BYTE)/DIFF(4BYTE) SZ(2BYTE)
        BC_MKMAP,         // mkmap(dr_0)        REGID(1BYTE)/DIFF(4BYTE) SZ(2BYTE)
        BC_IDSTR,         // idstr(dr)          REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF [Used for string]
        BC_EQUR,          // equr(dr)           REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  
        BC_NEQUR,         // nequr
        BC_EQUS,          // equs
        BC_NEQUS,         // nequs
        BC_SIDDICT,       // siddict(dr)        REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  REGID 
        BC_JNEQUB,        // jnequb(dr_0)       REGID(1BYTE)/DIFF(4BYTE) PLACE(4BYTE)            6-9 byte
        BC_IDSTRUCT,      // idstruct(dr_0)     REGID(1BYTE)/DIFF(4BYTE) OFFSET(2BYTE)   4-7 byte
    };

    enum modify_flag
    {
        NONE = 0,
        ASYNC = 1 << 0,
        CONST = 1 << 1,
        STATIC = 1 << 2,
        THREAD = 1 << 3,
    };

    class ast; using ast_ptr = std::shared_ptr<ast>;
    class unit_ast; using unit_ast_ptr = std::shared_ptr<unit_ast>;
    class type_ast; using type_ast_ptr = std::shared_ptr< type_ast >;
    class import_ast; using import_ast_ptr = std::shared_ptr< import_ast >;
    class decl_ast; using decl_ast_ptr = std::shared_ptr< decl_ast >;
    class enum_decl_ast; using enum_decl_ast_ptr = std::shared_ptr< enum_decl_ast >;
    class flag_decl_ast; using flag_decl_ast_ptr = std::shared_ptr< flag_decl_ast >;
    class class_decl_ast; using class_decl_ast_ptr = std::shared_ptr< class_decl_ast >;
    class using_decl_ast; using using_decl_ast_ptr = std::shared_ptr< using_decl_ast >;
    class enum_element_ast; using enum_element_ast_ptr = std::shared_ptr< enum_element_ast >;
    class flag_element_ast; using flag_element_ast_ptr = std::shared_ptr< flag_element_ast >;
    class template_decl_ast; using template_decl_ast_ptr = std::shared_ptr< template_decl_ast >;
    class variable_decl_ast; using variable_decl_ast_ptr = std::shared_ptr< variable_decl_ast >;
    class function_decl_ast; using function_decl_ast_ptr = std::shared_ptr< function_decl_ast >;
    class parameter_decl_ast; using parameter_decl_ast_ptr = std::shared_ptr< parameter_decl_ast >;
    class namespace_decl_ast; using namespace_decl_ast_ptr = std::shared_ptr< namespace_decl_ast >;
    class stat_ast; using stat_ast_ptr = std::shared_ptr< stat_ast >;
    class empty_stat_ast; using empty_stat_ast_ptr = std::shared_ptr< empty_stat_ast >;
    class extern_stat_ast; using extern_stat_ast_ptr = std::shared_ptr< extern_stat_ast >;
    class compound_stat_ast; using compound_stat_ast_ptr = std::shared_ptr< compound_stat_ast >;
    class await_stat_ast; using await_stat_ast_ptr = std::shared_ptr< await_stat_ast >;
    class yield_stat_ast; using yield_stat_ast_ptr = std::shared_ptr< yield_stat_ast >;
    class try_stat_ast; using try_stat_ast_ptr = std::shared_ptr< try_stat_ast >;
    class catch_stat_ast; using catch_stat_ast_ptr = std::shared_ptr< catch_stat_ast >;
    class throw_stat_ast; using throw_stat_ast_ptr = std::shared_ptr< throw_stat_ast >;
    class if_stat_ast; using if_stat_ast_ptr = std::shared_ptr< if_stat_ast >;
    class while_stat_ast; using while_stat_ast_ptr = std::shared_ptr< while_stat_ast >;
    class for_stat_ast; using for_stat_ast_ptr = std::shared_ptr< for_stat_ast >;
    class foreach_stat_ast; using foreach_stat_ast_ptr = std::shared_ptr< foreach_stat_ast >;
    class break_stat_ast; using break_stat_ast_ptr = std::shared_ptr< break_stat_ast >;
    class return_stat_ast; using return_stat_ast_ptr = std::shared_ptr< return_stat_ast >;
    class continue_stat_ast; using continue_stat_ast_ptr = std::shared_ptr< continue_stat_ast >;
    class local_stat_ast; using local_stat_ast_ptr = std::shared_ptr< local_stat_ast >;
    class exp_stat_ast; using exp_stat_ast_ptr = std::shared_ptr< exp_stat_ast >;
    class binary_exp_ast; using binary_exp_ast_ptr = std::shared_ptr< binary_exp_ast >;
    class assignment_exp_ast; using assignment_exp_ast_ptr = std::shared_ptr< assignment_exp_ast >;
    class conditional_exp_ast; using conditional_exp_ast_ptr = std::shared_ptr< conditional_exp_ast >;
    class logical_or_exp_ast; using logical_or_exp_ast_ptr = std::shared_ptr< logical_or_exp_ast >;
    class logical_and_exp_ast; using logical_and_exp_ast_ptr = std::shared_ptr< logical_and_exp_ast >;
    class or_exp_ast; using or_exp_ast_ptr = std::shared_ptr< or_exp_ast >;
    class xor_exp_ast; using xor_exp_ast_ptr = std::shared_ptr< xor_exp_ast >;
    class and_exp_ast; using and_exp_ast_ptr = std::shared_ptr< and_exp_ast >;
    class compare_exp_ast; using compare_exp_ast_ptr = std::shared_ptr< compare_exp_ast >;
    class shift_exp_ast; using shift_exp_ast_ptr = std::shared_ptr< shift_exp_ast >;
    class add_exp_ast; using add_exp_ast_ptr = std::shared_ptr< add_exp_ast >;
    class mul_exp_ast; using mul_exp_ast_ptr = std::shared_ptr< mul_exp_ast >;
    class as_exp_ast; using as_exp_ast_ptr = std::shared_ptr< as_exp_ast >;
    class is_exp_ast; using is_exp_ast_ptr = std::shared_ptr< is_exp_ast >;
    class unary_exp_ast; using unary_exp_ast_ptr = std::shared_ptr< unary_exp_ast >;
    class postfix_exp_ast; using postfix_exp_ast_ptr = std::shared_ptr< postfix_exp_ast >;
    class index_exp_ast; using index_exp_ast_ptr = std::shared_ptr< index_exp_ast >;
    class invoke_exp_ast; using invoke_exp_ast_ptr = std::shared_ptr< invoke_exp_ast >;
    class member_exp_ast; using member_exp_ast_ptr = std::shared_ptr< member_exp_ast >;
    class identifier_exp_ast; using identifier_exp_ast_ptr = std::shared_ptr< identifier_exp_ast >;
    class closure_exp_ast; using closure_exp_ast_ptr = std::shared_ptr< closure_exp_ast >;
    class arguments_exp_ast; using arguments_exp_ast_ptr = std::shared_ptr< arguments_exp_ast >;
    class initializers_exp_ast; using initializers_exp_ast_ptr = std::shared_ptr< initializers_exp_ast >;
    class const_exp_ast; using const_exp_ast_ptr = std::shared_ptr< const_exp_ast >;
    class null_const_exp_ast; using null_const_exp_ast_ptr = std::shared_ptr< null_const_exp_ast >;
    class bool_const_exp_ast; using bool_const_exp_ast_ptr = std::shared_ptr< bool_const_exp_ast >;
    class int_const_exp_ast; using int_const_exp_ast_ptr = std::shared_ptr< int_const_exp_ast >;
    class int8_const_exp_ast; using int8_const_exp_ast_ptr = std::shared_ptr< int8_const_exp_ast >;
    class int16_const_exp_ast; using int16_const_exp_ast_ptr = std::shared_ptr< int16_const_exp_ast >;
    class int32_const_exp_ast; using int32_const_exp_ast_ptr = std::shared_ptr< int32_const_exp_ast >;
    class int64_const_exp_ast; using int64_const_exp_ast_ptr = std::shared_ptr< int64_const_exp_ast >;
    class uint8_const_exp_ast; using uint8_const_exp_ast_ptr = std::shared_ptr< uint8_const_exp_ast >;
    class uint16_const_exp_ast; using uint16_const_exp_ast_ptr = std::shared_ptr< uint16_const_exp_ast >;
    class uint32_const_exp_ast; using uint32_const_exp_ast_ptr = std::shared_ptr< uint32_const_exp_ast >;
    class uint64_const_exp_ast; using uint64_const_exp_ast_ptr = std::shared_ptr< uint64_const_exp_ast >;
    class float_const_exp_ast; using float_const_exp_ast_ptr = std::shared_ptr< float_const_exp_ast >;
    class float16_const_exp_ast; using float16_const_exp_ast_ptr = std::shared_ptr< float16_const_exp_ast >;
    class float32_const_exp_ast; using float32_const_exp_ast_ptr = std::shared_ptr< float32_const_exp_ast >;
    class float64_const_exp_ast; using float64_const_exp_ast_ptr = std::shared_ptr< float64_const_exp_ast >;
    class string_const_exp_ast; using string_const_exp_ast_ptr = std::shared_ptr< string_const_exp_ast >;
    class ast_visitor; using ast_visitor_ptr = std::shared_ptr<ast_visitor>;

    class symbol; using symbol_ptr = std::shared_ptr<symbol>;
    class type_symbol; using type_symbol_ptr = std::shared_ptr<type_symbol>;
    class enum_symbol; using enum_symbol_ptr = std::shared_ptr<enum_symbol>;
    class block_symbol; using block_symbol_ptr = std::shared_ptr<block_symbol>;
    class alias_symbol; using alias_symbol_ptr = std::shared_ptr<alias_symbol>;
    class class_symbol; using class_symbol_ptr = std::shared_ptr<class_symbol>;
    class function_symbol; using function_symbol_ptr = std::shared_ptr<function_symbol>;
    class variable_symbol; using variable_symbol_ptr = std::shared_ptr<variable_symbol>;
    class template_symbol; using template_symbol_ptr = std::shared_ptr<template_symbol>;
    class namespace_symbol; using namespace_symbol_ptr = std::shared_ptr<namespace_symbol>;
    class type_element_symbol; using type_element_symbol_ptr = std::shared_ptr<type_element_symbol>;
    class enum_element_symbol; using enum_element_symbol_ptr = std::shared_ptr<enum_element_symbol>;

    class grammar; using grammar_ptr = std::shared_ptr<grammar>;
    class symbols; using symbols_ptr = std::shared_ptr<symbols>;
    class section; using section_ptr = std::shared_ptr<section>;
    class context; using context_ptr = std::shared_ptr<context>;
    class runtime; using runtime_ptr = std::shared_ptr<runtime>;
    class compiler; using compiler_ptr = std::shared_ptr<compiler>;

    class meta; using meta_ptr = std::shared_ptr<meta>;
    class meta_type; using meta_type_ptr = std::shared_ptr<meta_type>;
    class meta_enum; using meta_enum_ptr = std::shared_ptr<meta_enum>;
    class meta_flag; using meta_flag_ptr = std::shared_ptr<meta_flag>;
    class meta_class; using meta_class_ptr = std::shared_ptr<meta_class>;
    class meta_variable; using meta_variable_ptr = std::shared_ptr<meta_variable>;
    class meta_function; using meta_function_ptr = std::shared_ptr<meta_function>;
    class meta_namespace; using meta_namespace_ptr = std::shared_ptr<meta_namespace>;

    class value;
    class object;

    struct source_location
    {
        uint32_t line = 1;
        uint32_t column = 1;
        std::string_view file;
    };

    struct template_type
    {
        bool is_ref = false;
        bool is_const = false;
        bool is_array = false;
        std::string type;
        std::string name;
    };

    struct type_desc
    {
        bool is_ref = false;
        bool is_const = false;
        bool is_array = false;
        std::uint64_t hashcode = 0;
    };

    struct token
    {
        token_t type;
        std::string str;
        source_location location;
    };

    inline x::modify_flag operator|( x::modify_flag left, x::modify_flag right )
    {
        return (x::modify_flag)( (int)left | (int)right );
    }

    inline constexpr std::size_t hash( const char * str, std::size_t size = std::numeric_limits<size_t>::max(), std::size_t value = 14695981039346656037ULL )
    {
        while ( *str++ != '\0' && size-- != 0 )
        {
            value ^= static_cast<std::size_t>( *str );
            value *= 1099511628211ULL;
        }

        return value;
    }
    inline constexpr std::size_t hash( x::static_string_view str )
    {
        return hash( str.data(), str.size() );
    }
    inline constexpr std::size_t hash( const std::string & str )
    {
        return hash( str.data(), str.size() );
    }
    inline constexpr std::size_t hash( std::string_view str )
    {
        return hash( str.data(), str.size() );
    }

    template<typename ... Ts> struct overload : Ts ... { using Ts::operator() ...; }; template<class... Ts> overload( Ts... ) -> overload<Ts...>;
}
