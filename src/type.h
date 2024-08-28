#pragma once

#include <map>
#include <span>
#include <array>
#include <memory>
#include <vector>
#include <string>
#include <format>
#include <chrono>
#include <cassert>

#include "flags.hpp"
#include "float16.h"

#undef ERROR

#define KB ( 1024 )
#define MB ( 1024 * 1024 )
#define WSIZE( size ) ( ( size + sizeof( std::uintptr_t ) - 1 ) / sizeof( std::uintptr_t ) )
#define ALIGN( size, align ) ( ( ( size ) + ( align ) - 1 ) & ~( (align) - 1 ) )
#define PALIGN( size ) ( ( ( size ) + ( sizeof( std::uintptr_t ) ) - 1 ) & ~( ( sizeof( std::uintptr_t ) ) - 1 ) )

#ifdef _DEBUG
#define _WSTR_(s) L ## s
#define _WSTR(s) _WSTR_( s )
#define XTHROW( EX, COND, ... ) if( ( COND ) ){ printf("%s", EX( __VA_ARGS__ ).what() ); _wassert( _WSTR( #COND ), _WSTR( __FILE__ ), __LINE__ ); }
#else
#define XTHROW( EX, COND, ... ) if( ( COND ) ) throw EX( __VA_ARGS__ );
#endif // _DEBUG

#define PTR( TYPE ) class TYPE; using TYPE##_ptr = std::shared_ptr<TYPE>; using TYPE##_wptr = std::weak_ptr<TYPE>; using TYPE##_uptr = std::unique_ptr<TYPE>

namespace x
{
    using byte = std::byte;
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
    using string = const char *;
    using intptr = void *;
    
    enum class ast_t : x::uint8
    {
        UNIT,
        IMPORT,
        ATTRIBUTE,

        TYPE,
        FUNC_TYPE,
        TEMP_TYPE,
        LIST_TYPE,
        ARRAY_TYPE,

        ENUM_ELEMENT,
        TEMPLATE_ELEMENT,
        PARAMETER_ELEMENT,

        ENUM_DECL,
        CLASS_DECL,
        USING_DECL,
        TEMPLATE_DECL,
        VARIABLE_DECL,
        FUNCTION_DECL,
        NAMESPACE_DECL,

        EMPTY_STAT,
        EXTERN_STAT,
        COMPOUND_STAT,
        AWAIT_STAT,
        YIELD_STAT,
        NEW_STAT,
        IF_STAT,
        WHILE_STAT,
        FOR_STAT,
        FOREACH_STAT,
        SWITCH_STAT,
        BREAK_STAT,
        RETURN_STAT,
        CONTINUE_STAT,
        LOCAL_STAT,

        BINRARY_EXP,
        UNARY_EXP,
        BRACKET_EXP,
        CLOSURE_EXP,
        ARGUMENTS_EXP,
        IDENTIFIER_EXP,
        INITIALIZER_EXP,
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
    enum class meta_t : x::uint8
    {
        ENUM,
        FLAG,
        CLASS,
        VARIABLE,
        FUNCTION,
        NAMESPACE,
        ENUM_ELEMENT,
        FLAG_ELEMENT,
        PARAM_ELEMENT,
    };
    enum class token_t : x::uint8
    {
        TK_EOF = 0,
        TK_IDENTIFIER,           // identifier
        TK_CONSTEXPR_INT,        // 1 -233 0x123456 0b1101001
        TK_CONSTEXPR_FLOAT,      // 10.234 -432.01
        TK_CONSTEXPR_STRING,     // "" "helloworld" R"(println("hello");)"
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
        TK_NOT,                  // !
        TK_REV,                  // ~
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
        TK_COMPARE,              // <=>
        TK_TYPECAST,             // :
        TK_POINT,                // .
        TK_QUESTION,             // ?
        TK_VARIADIC_SIGN,        // ...
        TK_LEFT_INDEX,           // [
        TK_RIGHT_INDEX,          // ]
        TK_FUNCTION_RESULT,      // ->
        TK_LEFT_BRACKETS,        // (
        TK_RIGHT_BRACKETS,       // )
        TK_LEFT_CURLY_BRACES,    // {
        TK_RIGHT_CURLY_BRACES,   // }
        TK_ANY,                  // any
        TK_VOID,                 // void
        TK_BYTE,                 // byte
        TK_BOOL,                 // bool
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
        TK_INTPTR,               // intptr
        TK_OBJECT,               // object
        TK_ARRAY,                // array
        TK_COROUTINE,            // coroutine
        TK_IMPORT,               // import
        TK_ASSERT,               // assert
        TK_TEMPLATE,             // template
        TK_NAMESPACE,            // namespace
        TK_ATTRIBUTE,            // attribute
        TK_USING,                // using
        TK_ENUM,                 // enum
        TK_CLASS,                // class
        TK_VARIABLE,             // var
        TK_FUNCTION,             // func
        TK_WHERE,                // where
        TK_PRIVATE,              // private
        TK_PUBLIC,               // public
        TK_PROTECTED,            // protected
        TK_REF,                  // ref
        TK_LOCAL,                // local
        TK_CONST,                // const
        TK_FINAL,                // final
        TK_STATIC,               // static
        TK_EXTERN,               // extern
        TK_VIRTUAL,              // virtual
        TK_OVERRIDE,             // override
        TK_THREAD,               // thread
        TK_NEW,                  // new
        TK_WHILE,                // while
        TK_IF,                   // if
        TK_ELSE,                 // else
        TK_FOR,                  // for
        TK_FOREACH,              // foreach
        TK_SWITCH,               // switch
        TK_CASE,                 // case
        TK_DEFAULT,              // default
        TK_ASYNC,                // async
        TK_AWAIT,                // await
        TK_YIELD,                // yield
        TK_BREAK,                // break
        TK_RETURN,               // return
        TK_CONTINUE,             // continue
        TK_AS,                   // as
        TK_IS,                   // is
        TK_SIZEOF,               // sizeof
        TK_TYPEOF,               // typeof
        TK_NULL,                 // null
        TK_TRUE,                 // true
        TK_FALSE,                // false
        TK_THIS,                 // this
        TK_BASE,                 // base
    };
    enum class value_t : x::int32
    {
        INVALID             = 0,

        NIL                 = 1 << 0,
        BOOL                = 1 << 1,
        INT8                = 1 << 2,
        INT16               = 1 << 3,
        INT32               = 1 << 4,
        INT64               = 1 << 5,
        UINT8               = 1 << 6,
        UINT16              = 1 << 7,
        UINT32              = 1 << 8,
        UINT64              = 1 << 9,
        FLOAT16             = 1 << 10,
        FLOAT32             = 1 << 11,
        FLOAT64             = 1 << 12,
        STRING              = 1 << 13,
        INTPTR              = 1 << 14,
        OBJECT              = 1 << 15,
        REFERENCE           = 1 << 16,

        SIGNED_MASK         = 0x3C,
        UNSIGNED_MASK       = 0x3C0,
        FLOATING_MASK       = 0x1C00,
        TYPE_MASK           = 0xFFFF,
    };
    enum class access_t : x::uint8
    {
        PUBLIC,
        PRIVATE,
        PROTECTED,
    };
    enum class report_t
    {
        INFO,
        WARN,
        ERROR,
    };
    enum class symbol_t : x::uint8
    {
        UNIT,
        ENUM,
        ALIAS,
        CLASS,
        BLOCK,
        CYCLE,
        LOCAL,
        FUNCTION,
        VARIABLE,
        TEMPLATE,
        NAMESPACE,
        FOUNDATION,
        NATIVEFUNC,
        BUILTINFUNC,
        ENUM_ELEMENT,
        TEMPLATE_ELEMENT,
        PARAMATER_ELEMENT,
    };
    enum class opcode_t : x::uint8
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
    enum class valloc_t
    {
        READ                = 1 << 0,
        WRITE               = 1 << 1,
        EXECUTE             = 1 << 2,
    };
    enum class section_t : x::uint8
    {
        TYPE,
        TEMP,
        DESC,
        DEPEND,
        GLOBAL,
        FUNCTION,
        VARIABLE,
        ATTRIBUTE,
        OPCODEDATA,
        STRINGDATA,
        CUSTOMDATA,
    };
    enum class gcstage_t
    {
        NONE,
        MARKING,
        TRACKING,
        CLEARING,
        COLLECT,
    };
    enum class gcstatus_t
    {
        WHITE,
        BLACK,
        GRAY
    };
    enum class pagekind_t
    {
        SMALL,
        MEDIUM,
        LARGE,
        HUGE,
    };
    enum class corostatus_t
    {
        EMPTY,
        SUSPEND,
        READY,
        EXCEPT,
    };
    enum class http_header_t
    {
        Accept,
        Accept_CH,
        Accept_Charset,
        Accept_Encoding,
        Accept_Language,
        Accept_Patch,
        Accept_Post,
        Accept_Ranges,
        Access_Control_Allow_Credentials,
        Access_Control_Allow_Headers,
        Access_Control_Allow_Methods,
        Access_Control_Allow_Origin,
        Access_Control_Expose_Headers,
        Access_Control_Max_Age,
        Access_Control_Request_Headers,
        Access_Control_Request_Method,
        Age,
        Allow,
        Alt_Svc,
        Alt_Used,
        Authorization,
        Cache_Control,
        Clear_Site_Data,
        Connection,
        Content_Disposition,
        Content_Encoding,
        Content_Language,
        Content_Length,
        Content_Location,
        Content_Range,
        Content_Security_Policy,
        Content_Security_Policy_Report_Only,
        Content_Type,
        Cookie,
        Cross_Origin_Embedder_Policy,
        Cross_Origin_Opener_Policy,
        Cross_Origin_Resource_Policy,
        Date,
        Device_Memory,
        ETag,
        Expect,
        Expect_CT,
        Expires,
        Forwarded,
        From,
        Host,
        If_Match,
        If_Modified_Since,
        If_None_Match,
        If_Range,
        If_Unmodified_Since,
        Keep_Alive,
        Last_Modified,
        Link,
        Location,
        Max_Forwards,
        Origin,
        Permissions_Policy,
        Priority,
        Proxy_Authenticate,
        Proxy_Authorization,
        Range,
        Referer,
        Referrer_Policy,
        Reporting_Endpoints,
        Retry_After,
        Sec_Fetch_Dest,
        Sec_Fetch_Mode,
        Sec_Fetch_Site,
        Sec_Fetch_User,
        Sec_Purpose,
        Sec_WebSocket_Accept,
        Server,
        Server_Timing,
        Service_Worker_Navigation_Preload,
        Set_Cookie,
        SourceMap,
        Strict_Transport_Security,
        TE,
        Timing_Allow_Origin,
        Trailer,
        Transfer_Encoding,
        Upgrade,
        Upgrade_Insecure_Requests,
        User_Agent,
        Vary,
        Via,
        WWW_Authenticate,
        X_Content_Type_Options,
        X_Frame_Options,
    };
    enum class http_version_t
    {
        HTTP_1_0,
        HTTP_1_1,
        HTTP_2_0,
    };

    using value_flags = flags<x::value_t>;
    using valloc_flags = flags<x::valloc_t>;

    class value;
    class object;
    class scheduler;

    class meta;
    class meta_type;
    class meta_enum;
    class meta_class;
    class meta_element;
    class meta_variable;
    class meta_function;
    class meta_parameter;
    class meta_namespace;
    class meta_attribute;

    PTR( logger );
    PTR( module );
    PTR( grammar );
    PTR( visitor );
    PTR( builtin );
    PTR( symbols );
    PTR( context );
    PTR( runtime );
    PTR( interpreter );
    PTR( virtual_machine );

    PTR( symbol );
    PTR( unit_symbol );
    PTR( enum_symbol );
    PTR( alias_symbol );
    PTR( class_symbol );
    PTR( block_symbol );
    PTR( cycle_symbol );
    PTR( local_symbol );
    PTR( function_symbol );
    PTR( variable_symbol );
    PTR( template_symbol );
    PTR( namespace_symbol );
    PTR( foundation_symbol );
    PTR( nativefunc_symbol );
    PTR( builtinfunc_symbol );
    PTR( enum_element_symbol );
    PTR( template_element_symbol );
    PTR( paramater_element_symbol );

    PTR( ast );
    PTR( unit_ast );
    PTR( import_ast );
    PTR( attribute_ast );
    PTR( type_ast );
    PTR( func_type_ast );
    PTR( temp_type_ast );
    PTR( list_type_ast );
    PTR( array_type_ast );
    PTR( enum_element_ast );
    PTR( template_element_ast );
    PTR( parameter_element_ast );
    PTR( decl_ast );
    PTR( enum_decl_ast );
    PTR( class_decl_ast );
    PTR( using_decl_ast );
    PTR( template_decl_ast );
    PTR( variable_decl_ast );
    PTR( function_decl_ast );
    PTR( namespace_decl_ast );
    PTR( stat_ast );
    PTR( empty_stat_ast );
    PTR( extern_stat_ast );
    PTR( compound_stat_ast );
    PTR( await_stat_ast );
    PTR( yield_stat_ast );
    PTR( new_stat_ast );
    PTR( if_stat_ast );
    PTR( cycle_stat_ast );
    PTR( while_stat_ast );
    PTR( for_stat_ast );
    PTR( foreach_stat_ast );
    PTR( switch_stat_ast );
    PTR( break_stat_ast );
    PTR( return_stat_ast );
    PTR( continue_stat_ast );
    PTR( local_stat_ast );
    PTR( expr_stat_ast );
    PTR( binary_expr_ast );
    PTR( unary_expr_ast );
    PTR( bracket_expr_ast );
    PTR( closure_expr_ast );
    PTR( arguments_expr_ast );
    PTR( identifier_expr_ast );
    PTR( initializer_expr_ast );
    PTR( const_expr_ast );
    PTR( null_const_expr_ast );
    PTR( bool_const_expr_ast );
    PTR( int_const_expr_ast );
    PTR( int8_const_expr_ast );
    PTR( int16_const_expr_ast );
    PTR( int32_const_expr_ast );
    PTR( int64_const_expr_ast );
    PTR( uint8_const_expr_ast );
    PTR( uint16_const_expr_ast );
    PTR( uint32_const_expr_ast );
    PTR( uint64_const_expr_ast );
    PTR( float_const_expr_ast );
    PTR( float16_const_expr_ast );
    PTR( float32_const_expr_ast );
    PTR( float64_const_expr_ast );
    PTR( string_const_expr_ast );

    PTR( http_client );
    PTR( http_server );
    PTR( http_request );
    PTR( http_respone );

    struct location
    {
        x::uint32 col = 1;
        x::uint32 line = 1;
        std::string file;
    };
    struct range
    {
        x::uint32 beg = 0;
        x::uint32 end = 0;
    };
    struct token
    {
        token_t type = token_t::TK_EOF;
        std::string str;
        location location;
    };
    
    inline constexpr x::uint64 hash( const char * str, x::uint64 size = std::numeric_limits<x::uint64>::max(), x::uint64 value = 14695981039346656037ULL )
    {
        while ( *str++ != '\0' && size-- != 0 )
        {
            value ^= static_cast<x::uint64>( *str );
            value *= 1099511628211ULL;
        }

        return value;
    }
    inline constexpr x::uint64 hash( std::string_view str )
    {
        return hash( str.data(), str.size() );
    }
    inline constexpr x::uint64 hash( const std::string & str )
    {
        return hash( str.data(), str.size() );
    }

    template<typename Clock> inline constexpr std::chrono::time_point<Clock> stamp_2_time( x::int64 stamp )
    {
        return std::chrono::time_point<Clock>() + std::chrono::milliseconds( stamp );
    }
    template<typename Clock, typename Duration = typename Clock::duration> inline constexpr x::int64 time_2_stamp( std::chrono::time_point<Clock, Duration> time )
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::time_point_cast<std::chrono::milliseconds>( time ).time_since_epoch() ).count();
    }

    template<typename ... Ts> struct overload : Ts ... { using Ts::operator() ...; }; template<class... Ts> overload( Ts... ) -> overload<Ts...>;
    template<template<typename...> typename Target, typename T> struct is_template_of : public std::false_type {};
    template<template<typename...> typename Target, typename...Args> struct is_template_of< Target, Target<Args...> > : public std::true_type {};
    template<template<typename...> typename Target, typename T> static constexpr const bool is_template_of_v = is_template_of<Target, T>::value;

    static constexpr const uint32 magic_num = 'xsl\0';
    static constexpr const uint32 version_num = '0001';
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
        { (const char *)u8"!", x::token_t::TK_NOT },
        { (const char *)u8"~", x::token_t::TK_REV },
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
        { (const char *)u8"<=>", x::token_t::TK_COMPARE },
        { (const char *)u8":", x::token_t::TK_TYPECAST },
        { (const char *)u8".", x::token_t::TK_POINT },
        { (const char *)u8"?", x::token_t::TK_QUESTION },
        { (const char *)u8"..", x::token_t::TK_VARIADIC_SIGN },
        { (const char *)u8"...", x::token_t::TK_VARIADIC_SIGN },
        { (const char *)u8"[", x::token_t::TK_LEFT_INDEX },
        { (const char *)u8"]", x::token_t::TK_RIGHT_INDEX },
        { (const char *)u8"->", x::token_t::TK_FUNCTION_RESULT },
        { (const char *)u8"(", x::token_t::TK_LEFT_BRACKETS },
        { (const char *)u8")", x::token_t::TK_RIGHT_BRACKETS },
        { (const char *)u8"{", x::token_t::TK_LEFT_CURLY_BRACES },
        { (const char *)u8"}", x::token_t::TK_RIGHT_CURLY_BRACES },
        { (const char *)u8"any", x::token_t::TK_ANY },
        { (const char *)u8"void", x::token_t::TK_VOID },
        { (const char *)u8"byte", x::token_t::TK_BYTE },
        { (const char *)u8"bool", x::token_t::TK_BOOL },
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
        { (const char *)u8"intptr", x::token_t::TK_INTPTR },
        { (const char *)u8"object", x::token_t::TK_OBJECT },
        { (const char *)u8"array", x::token_t::TK_ARRAY },
        { (const char *)u8"coroutine", x::token_t::TK_COROUTINE },
        { (const char *)u8"import", x::token_t::TK_IMPORT },
        { (const char *)u8"assert", x::token_t::TK_ASSERT },
        { (const char *)u8"template", x::token_t::TK_TEMPLATE },
        { (const char *)u8"namespace", x::token_t::TK_NAMESPACE },
        { (const char *)u8"attribute", x::token_t::TK_ATTRIBUTE },
        { (const char *)u8"using", x::token_t::TK_USING },
        { (const char *)u8"enum", x::token_t::TK_ENUM },
        { (const char *)u8"class", x::token_t::TK_CLASS },
        { (const char *)u8"var", x::token_t::TK_VARIABLE },
        { (const char *)u8"func", x::token_t::TK_FUNCTION },
        { (const char *)u8"where", x::token_t::TK_WHERE },
        { (const char *)u8"private", x::token_t::TK_PRIVATE },
        { (const char *)u8"public", x::token_t::TK_PUBLIC },
        { (const char *)u8"protected", x::token_t::TK_PROTECTED },
        { (const char *)u8"ref", x::token_t::TK_REF },
        { (const char *)u8"local", x::token_t::TK_LOCAL },
        { (const char *)u8"const", x::token_t::TK_CONST },
        { (const char *)u8"final", x::token_t::TK_FINAL },
        { (const char *)u8"static", x::token_t::TK_STATIC },
        { (const char *)u8"extern", x::token_t::TK_EXTERN },
        { (const char *)u8"virtual", x::token_t::TK_VIRTUAL },
        { (const char *)u8"override", x::token_t::TK_OVERRIDE },
        { (const char *)u8"thread", x::token_t::TK_THREAD },
        { (const char *)u8"new", x::token_t::TK_NEW },
        { (const char *)u8"while", x::token_t::TK_WHILE },
        { (const char *)u8"if", x::token_t::TK_IF },
        { (const char *)u8"else", x::token_t::TK_ELSE },
        { (const char *)u8"for", x::token_t::TK_FOR },
        { (const char *)u8"foreach", x::token_t::TK_FOREACH },
        { (const char *)u8"switch", x::token_t::TK_SWITCH },
        { (const char *)u8"case", x::token_t::TK_CASE },
        { (const char *)u8"default", x::token_t::TK_DEFAULT },
        { (const char *)u8"async", x::token_t::TK_ASYNC },
        { (const char *)u8"await", x::token_t::TK_AWAIT },
        { (const char *)u8"yield", x::token_t::TK_YIELD },
        { (const char *)u8"break", x::token_t::TK_BREAK },
        { (const char *)u8"return", x::token_t::TK_RETURN },
        { (const char *)u8"continue", x::token_t::TK_CONTINUE },
        { (const char *)u8"as", x::token_t::TK_AS },
        { (const char *)u8"is", x::token_t::TK_IS },
        { (const char *)u8"sizeof", x::token_t::TK_SIZEOF },
        { (const char *)u8"typeof", x::token_t::TK_TYPEOF },
        { (const char *)u8"null", x::token_t::TK_NULL },
        { (const char *)u8"true", x::token_t::TK_TRUE },
        { (const char *)u8"false", x::token_t::TK_FALSE },
        { (const char *)u8"this", x::token_t::TK_THIS },
        { (const char *)u8"base", x::token_t::TK_BASE },
    };
}

namespace llvm
{
    class Module;
    class LLVMContext;

    using module = llvm::Module;
    using context = llvm::LLVMContext;

    using module_ptr = std::shared_ptr<llvm::module>;
    using context_ptr = std::shared_ptr<llvm::context>;
}

namespace spirv
{
    class module;

    using module_ptr = std::shared_ptr<spirv::module>;
}
