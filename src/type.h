#pragma once

#include <memory>
#include <vector>
#include <string>
#include <format>
#include <cassert>
#include <exception>

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

    enum class ast_t
    {
        UNIT,
        TYPE,
        IMPORT,

        ENUM_DECL,
        CLASS_DECL,
        USING_DECL,
        ENUM_ELEMENT,
        TEMPLATE_DECL,
        VARIABLE_DECL,
        FUNCTION_DECL,
        PARAMETER_DECL,
        NAMESPACE_DECL,

        EMPTY_STAT,
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
        INT_CONST_EXP,
        FLOAT_CONST_EXP,
        STRING_CONST_EXP,
    };

    enum class meta_t
    {
        ENUM        = 1 << 0,
        CLASS       = 1 << 1,
        VARIABLE    = 1 << 2,
        FUNCTION    = 1 << 3,
        NAMESPACE   = 1 << 4,

        EXTERN      = 1 << 5,
        NATIVE      = 1 << 6,
        SCRIPT      = 1 << 7,
        
        EXTERN_FUNCTION    = FUNCTION | EXTERN,

        NATIVE_ENUM        = ENUM | NATIVE,
        NATIVE_CLASS       = CLASS | NATIVE,
        NATIVE_VARIABLE    = VARIABLE | NATIVE,
        NATIVE_FUNCTION    = FUNCTION | NATIVE,

        SCRIPT_ENUM        = ENUM | SCRIPT,
        SCRIPT_CLASS       = CLASS | SCRIPT,
        SCRIPT_VARIABLE    = VARIABLE | SCRIPT,
        SCRIPT_FUNCTION    = FUNCTION | SCRIPT,
    };

    enum class code_t
    {
        IR,
        JIT,
        AOT,
    };

    enum class token_t
    {
        TK_EOF = 0,
        TK_IDENTIFIER,           // identifier
        TK_LITERAL_INT,          // 1 233 0x123456 0b1101001
        TK_LITERAL_REAL,         // 0.234
        TK_LITERAL_STRING,       // "" "helloworld" R"println("hello");"
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
        TK_INTEGER,              // int
        TK_FLOATING,             // float
        TK_STRING,               // string
        TK_IMPORT,               // import
        TK_TEMPLATE,             // template
        TK_NAMESPACE,            // namespace
        TK_USING,                // using
        TK_ENUM,                 // enum
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
        BOOLEAN,
        INTEGER,
        FLOATING,

        ANY,
        ARRAY,
        STRING,
        NATIVE,
        SCRIPT,
        CLOSURE,
        OBJECT,

        NATIVE_CALL,
        EXTREN_CALL,
        SCRIPT_CALL,
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

    enum class opcode_t
    {
        OP_NOP = 0,       // nop(TKPLS)                                                     1 byte
        OP_MOV,           // mov(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        OP_ADDI,          // add(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        OP_SUBI,          // sub
        OP_MULI,          // mul
        OP_DIVI,          // div
        OP_MODI,          // mod
        OP_ADDR,          // add(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        OP_SUBR,          // sub
        OP_MULR,          // mul
        OP_DIVR,          // div
        OP_MODR,          // mod
        OP_ADDH,          // add(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        OP_SUBH,          // sub
        OP_ADDS,          // add(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        OP_PSH,           // psh(dr_0)          REGID(1BYTE)/DIFF(4BYTE)                    2-5 byte
        OP_POP,           // pop(dr_STORED?)    REGID(1BYTE)/DIFF(4BYTE)/COUNT(2BYTE)       2-5 byte
        OP_SIDARR,        // sidarr(dr)         REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  REGID
        OP_SIDSTRUCT,     // sidstruct(dr)      REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  REGID
        OP_LDS,           // lds(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF   
        OP_STS,           // sts(dr)            REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  
        OP_EQUB,          // equb(dr)           REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF         3-9 byte
        OP_NEQUB,         // nequb
        OP_LTI,           // lt
        OP_GTI,           // gt
        OP_ELTI,          // elt
        OP_EGTI,          // egt
        OP_LAND,          // land             
        OP_LOR,           // lor
        OP_SIDMAP,        // sidmap(dr)         REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  REGID
        OP_LTX,           // lt
        OP_GTX,           // gt
        OP_ELTX,          // elt
        OP_EGTX,          // egt
        OP_LTR,           // lt
        OP_GTR,           // gt
        OP_ELTR,          // elt
        OP_EGTR,          // egt
        OP_CALL,          // call(dr_0)         REGID(1BYTE)/DIFF(4BYTE) 
        OP_CALLN,         // calln(0_ISNATIVE)  VM_IR_DIFF(4BYTE)/NATIVE_FUNC(8BYTE)
        OP_RET,           // ret(dr_0)          POP_SIZE(2 BYTE if dr)
        OP_JT,            // jt                 DIFF(4BYTE)
        OP_JF,            // jf                 DIFF(4BYTE)
        OP_JMP,           // jmp                DIFF(4BYTE)
        OP_MOVCAST,       // movcast(dr)        REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF TYPE  4-10 byte
        OP_MKCLOS,        // mkclos(00)         FUNC(8BYTE) CAPTURE_ARG_COUNT(2BYTE) 11 byte
        OP_TYPEAS,        // typeas(dr_0)       REGID(1BYTE)/DIFF(4BYTE) TYPE             3-6 byte
        OP_MKSTRUCT,      // mkstruct(dr_0)     REGID(1BYTE)/DIFF(4BYTE) SZ(2BYTE)               4-7 byte
        OP_ABRT,          // abrt(0_1/0)        (0xcc 0xcd can use it to abort)     
        OP_IDARR,         // idarr(dr)          REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF [Used for array]
        OP_IDDICT,        // iddict(dr)         REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF [Used for dict]
        OP_MKARR,         // mkarr(dr_0)        REGID(1BYTE)/DIFF(4BYTE) SZ(2BYTE)
        OP_MKMAP,         // mkmap(dr_0)        REGID(1BYTE)/DIFF(4BYTE) SZ(2BYTE)
        OP_IDSTR,         // idstr(dr)          REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF [Used for string]
        OP_EQUR,          // equr(dr)           REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  
        OP_NEQUR,         // nequr
        OP_EQUS,          // equs
        OP_NEQUS,         // nequs
        OP_SIDDICT,       // siddict(dr)        REGID(1BYTE)/DIFF(4BYTE) REGID/DIFF  REGID 
        OP_JNEQUB,        // jnequb(dr_0)       REGID(1BYTE)/DIFF(4BYTE) PLACE(4BYTE)            6-9 byte
        OP_IDSTRUCT,      // idstruct(dr_0)     REGID(1BYTE)/DIFF(4BYTE) OFFSET(2BYTE)   4-7 byte
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

    enum modify_flag
    {
        NONE = 0,
        ASYNC = 1 << 0,
        CONST = 1 << 1,
        STATIC = 1 << 2,
        THREAD = 1 << 3,
        NATIVE = 1 << 4,
        EXTERN = 1 << 5,
    };

    class value;
    class object;
    
    class ast; using ast_ptr = std::shared_ptr<ast>;
    class unit_ast; using unit_ast_ptr = std::shared_ptr<unit_ast>;
    class ast_visitor; using ast_visitor_ptr = std::shared_ptr<ast_visitor>;

    class grammar; using grammar_ptr = std::shared_ptr<grammar>;
    class symbols; using symbols_ptr = std::shared_ptr<symbols>;
    class section; using section_ptr = std::shared_ptr<section>;
    class context; using context_ptr = std::shared_ptr<context>;
    class runtime; using runtime_ptr = std::shared_ptr<runtime>;
    class compiler; using compiler_ptr = std::shared_ptr<compiler>;

    class meta; using meta_ptr = std::shared_ptr<meta>;
    class meta_enum; using meta_enum_ptr = std::shared_ptr<meta_enum>;
    class meta_class; using meta_class_ptr = std::shared_ptr<meta_class>;
    class meta_variable; using meta_variable_ptr = std::shared_ptr<meta_variable>;
    class meta_function; using meta_function_ptr = std::shared_ptr<meta_function>;
    class meta_namespace; using meta_namespace_ptr = std::shared_ptr<meta_namespace>;

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
        uint64_t hashcode = 0;
    };

    struct token
    {
        token_t type;
        std::string str;
        source_location location;
    };

    struct code
    {
        code_t type;
        uint64_t idx;
    };

    inline x::modify_flag operator|( x::modify_flag left, x::modify_flag right )
    {
        return (x::modify_flag)( (int)left | (int)right );
    }
}
