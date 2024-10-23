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
    static constexpr const char * source_extension = ".xs";
    static constexpr const char * module_extension = ".xmod";

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

        PARAMETER,

        ENUM_DECL,
        CLASS_DECL,
        USING_DECL,
        TEMPLATE_DECL,
        VARIABLE_DECL,
        FUNCTION_DECL,
        INTERFACE_DECL,
        NAMESPACE_DECL,

        EMPTY_STAT,
        EXTERN_STAT,
        COMPOUND_STAT,
        AWAIT_STAT,
        YIELD_STAT,
        IF_STAT,
        WHILE_STAT,
        FOR_STAT,
        FOREACH_STAT,
        SWITCH_STAT,
        BREAK_STAT,
        RETURN_STAT,
        TRY_STAT,
        THROW_STAT,
        CONTINUE_STAT,
        LOCAL_STAT,
        MULOCAL_STAT,

        UNARY_EXP,
        BINARY_EXP,
        BRACKET_EXP,
        CLOSURE_EXP,
        ELEMENTS_EXP,
        ARGUMENTS_EXP,
        IDENTIFIER_EXP,
        INITIALIZER_EXP,
        NULL_CONSTANT_EXP,
        BOOL_CONSTANT_EXP,
        INT32_CONSTANT_EXP,
        INT64_CONSTANT_EXP,
        UINT32_CONSTANT_EXP,
        UINT64_CONSTANT_EXP,
        FLOAT32_CONSTANT_EXP,
        FLOAT64_CONSTANT_EXP,
        STRING_CONSTANT_EXP,
    };
    enum class meta_t : x::uint8
    {
        ENUM,
        FLAG,
        CLASS,
        TEMPLATE,
        VARIABLE,
        FUNCTION,
        INTERFACE,
        NAMESPACE,
    };
    enum class token_t : x::uint8
    {
        TK_EOF = 0,
        TK_IDENTIFIER,          // identifier
        TK_CONSTEXPR_INT,       // 1 -233 0x123456 0b1101001
        TK_CONSTEXPR_FLOAT,     // 10.234 -432.01
        TK_CONSTEXPR_STRING,    // "" "helloworld" R"(println("hello");)"
        TK_SEMICOLON,           // ;
        TK_COMMA,               // ,
        TK_INC,                 // ++
        TK_DEC,                 // --
        TK_ADD,                 // +
        TK_SUB,                 // - 
        TK_MUL,                 // * 
        TK_DIV,                 // / 
        TK_MOD,                 // %
        TK_AND,                 // &
        TK_OR,                  // |
        TK_XOR,                 // ^
        TK_LEFT_SHIFT,          // <<
        TK_RIGHT_SHIFT,         // >> 
        TK_LAND,                // &&
        TK_LOR,                 // ||
        TK_NOT,                 // !
        TK_REV,                 // ~
        TK_ASSIGN,              // =
        TK_ADD_ASSIGN,          // +=
        TK_SUB_ASSIGN,          // -= 
        TK_MUL_ASSIGN,          // *=
        TK_DIV_ASSIGN,          // /= 
        TK_MOD_ASSIGN,          // %= 
        TK_AND_ASSIGN,          // &= 
        TK_OR_ASSIGN,           // |= 
        TK_XOR_ASSIGN,          // ^= 
        TK_LSHIFT_ASSIGN,       // <<=
        TK_RSHIFT_ASSIGN,       // >>=
        TK_EQUAL,               // ==
        TK_NOT_EQUAL,           // !=
        TK_LESS,                // <
        TK_LARG,                // >
        TK_LESS_OR_EQUAL,       // <=
        TK_LARG_OR_EQUAL,       // >=
        TK_COMPARE,             // <=>
        TK_TYPECAST,            // :
        TK_POINT,               // .
        TK_QUESTION,            // ?
        TK_VARIADIC_SIGN,       // ...
        TK_LEFT_INDEX,          // [
        TK_RIGHT_INDEX,         // ]
        TK_FUNCTION_RESULT,     // ->
        TK_LEFT_BRACKETS,       // (
        TK_RIGHT_BRACKETS,      // )
        TK_LEFT_CURLY_BRACES,   // {
        TK_RIGHT_CURLY_BRACES,  // }
        TK_ANY,                 // any
        TK_VOID,                // void
        TK_BYTE,                // byte
        TK_BOOL,                // bool
        TK_INT8,                // int8
        TK_INT16,               // int16
        TK_INT32,               // int32
        TK_INT64,               // int64
        TK_UINT8,               // uint8
        TK_UINT16,              // uint16
        TK_UINT32,              // uint32
        TK_UINT64,              // uint64
        TK_FLOAT16,             // float16
        TK_FLOAT32,             // float32
        TK_FLOAT64,             // float64
        TK_STRING,              // string
        TK_INTPTR,              // intptr
        TK_OBJECT,              // object
        TK_IMPORT,              // import
        TK_ASSERT,              // assert
        TK_TEMPLATE,            // template
        TK_NAMESPACE,           // namespace
        TK_ATTRIBUTE,           // attribute
        TK_USING,               // using
        TK_ENUM,                // enum
        TK_CLASS,               // class
        TK_FRIEND,              // friend
        TK_INTERFACE,           // interface
        TK_VARIABLE,            // var
        TK_FUNCTION,            // func
        TK_WHERE,               // where
        TK_CONSTRUCT,           // construct
        TK_FINALIZE,            // finalize
        TK_PRIVATE,             // private
        TK_PUBLIC,              // public
        TK_PROTECTED,           // protected
        TK_REF,                 // ref
        TK_LOCAL,               // local
        TK_CONST,               // const
        TK_FINAL,               // final
        TK_STATIC,              // static
        TK_EXTERN,              // extern
        TK_VIRTUAL,             // virtual
        TK_OVERRIDE,            // override
        TK_THREAD,              // thread
        TK_WHILE,               // while
        TK_IF,                  // if
        TK_ELSE,                // else
        TK_FOR,                 // for
        TK_FOREACH,             // foreach
        TK_SWITCH,              // switch
        TK_CASE,                // case
        TK_DEFAULT,             // default
        TK_AWAIT,               // await
        TK_YIELD,               // yield
        TK_BREAK,               // break
        TK_RETURN,              // return
        TK_TRY,                 // try
        TK_CATCH,               // catch
        TK_THROW,               // throw
        TK_CONTINUE,            // continue
        TK_AS,                  // as
        TK_IS,                  // is
        TK_SIZEOF,              // sizeof
        TK_TYPEOF,              // typeof
        TK_NULL,                // null
        TK_TRUE,                // true
        TK_FALSE,               // false
        TK_THIS,                // this
        TK_BASE,                // base
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
        USING,
        CLASS,
        BLOCK,
        CYCLE,
        LOCAL,
        FUNCTION,
        VARIABLE,
        TEMPLATE,
        INTERFACE,
        NAMESPACE,
        FOUNDATION,
        NATIVEFUNC,
        BUILTINFUNC,
        ENUM_ELEMENT,
        PARAMATER_ELEMENT,
    };
    
    enum class opcode_t : x::uint16
    {
#define SIGNED( OP ) I8_##OP, I16_##OP, I32_##OP, I64_##OP
#define UNSIGNED( OP ) U8_##OP, U16_##OP, U32_##OP, U64_##OP
#define INTEGRAL( OP ) SIGNED( OP ), UNSIGNED( OP )
#define FLOATING( OP ) F32_##OP, F64_##OP
#define ARITHMETIC( OP ) INTEGRAL( OP ), FLOATING( OP )
#define ALLTYPES( OP ) ARITHMETIC( OP ), OBJ_##OP


        NOP = 0,
        CONST_NULL,	            // 将null推送至栈顶
        CONST_STRING,	        // 将string型从字符串池中推送至栈顶
        SIGNED( CONST_M1 ),	    // 将整数类型-1推送至栈顶
        FLOATING( CONST_M1 ),	// 将浮点类型-1推送至栈顶
        ARITHMETIC( CONST_0 ),  // 将算数类型0推送至栈顶
        ARITHMETIC( CONST_1 ),  // 将算数类型1推送至栈顶
        ARITHMETIC( CONST_2 ),  // 将算数类型2推送至栈顶
        ARITHMETIC( CONST_3 ),  // 将算数类型3推送至栈顶
        ARITHMETIC( CONST_4 ),  // 将算数类型4推送至栈顶
        ARITHMETIC( CONST_5 ),  // 将算数类型5推送至栈顶
        ARITHMETIC( PUSH ),     // 将算数类型的常量值推送至栈顶
        ALLTYPES( LOAD ),       // 将指定的算数类型本地变量推送至栈顶
        ALLTYPES( LOAD_0 ),     // 将第0个算数类型本地变量推送至栈顶
        ALLTYPES( LOAD_1 ),     // 将第1个算数类型本地变量推送至栈顶
        ALLTYPES( LOAD_2 ),     // 将第2个算数类型本地变量推送至栈顶
        ALLTYPES( LOAD_3 ),     // 将第3个算数类型本地变量推送至栈顶
        ALLTYPES( LOAD_4 ),     // 将第4个算数类型本地变量推送至栈顶
        ALLTYPES( STORE ),      // 将栈顶算数类型数值存入指定本地变量
        ALLTYPES( STORE_0 ),    // 将栈顶算数类型数值存入第0个本地变量
        ALLTYPES( STORE_1 ),    // 将栈顶算数类型数值存入第1个本地变量
        ALLTYPES( STORE_2 ),    // 将栈顶算数类型数值存入第2个本地变量
        ALLTYPES( STORE_3 ),    // 将栈顶算数类型数值存入第3个本地变量
        ALLTYPES( STORE_4 ),    // 将栈顶算数类型数值存入第4个本地变量
        POP,	                // 将栈顶数值弹出
        DUP,	                // 复制栈顶数值并将复制值压入栈顶
        DUP_X2,	                // 复制栈顶数值并将2个复制值压入栈顶
        DUP_X3,	                // 复制栈顶数值并将3个复制值压入栈顶
        SWAP,	                // 将栈最顶端的两个数值互换
        ARITHMETIC( ADD ),      // 将栈顶两算数类型数值相加并将结果压入栈顶
        ARITHMETIC( SUB ),      // 将栈顶两算数类型数值相减并将结果压入栈顶
        ARITHMETIC( MUL ),      // 将栈顶两算数类型数值相乘并将结果压入栈顶
        ARITHMETIC( DIV ),      // 将栈顶两算数类型数值相除并将结果压入栈顶
        ARITHMETIC( REM ),      // 将栈顶两算数类型数值作取模运算并将结果压入栈顶
        ARITHMETIC( NEG ),      // 将栈顶两算数类型数值取负并将结果压入栈顶
        INTEGRAL( SHL ),        // 将整数类型数值左移位指定位数并将结果压入栈顶
        INTEGRAL( SHR ),        // 将整数类型数值右移位指定位数并将结果压入栈顶
        INTEGRAL( AND ),        // 将栈顶两整数类型数值作“按位与”并将结果压入栈顶
        INTEGRAL( OR ),         // 将栈顶两整数类型数值作“按位或”并将结果压入栈顶
        INTEGRAL( XOR ),        // 将栈顶两整数类型数值作“按位异或”并将结果压入栈顶
        ARITHMETIC( 2_I8 ),     // 将栈顶运算类型数值强制转换成int8型数值并将结果压入栈顶
        ARITHMETIC( 2_I16 ),    // 将栈顶运算类型数值强制转换成int16型数值并将结果压入栈顶
        ARITHMETIC( 2_I32 ),    // 将栈顶运算类型数值强制转换成int32型数值并将结果压入栈顶
        ARITHMETIC( 2_I64 ),    // 将栈顶运算类型数值强制转换成int64型数值并将结果压入栈顶
        ARITHMETIC( 2_U8 ),     // 将栈顶运算类型数值强制转换成uint8型数值并将结果压入栈顶
        ARITHMETIC( 2_U16 ),    // 将栈顶运算类型数值强制转换成uint16型数值并将结果压入栈顶
        ARITHMETIC( 2_U32 ),    // 将栈顶运算类型数值强制转换成uint32型数值并将结果压入栈顶
        ARITHMETIC( 2_U64 ),    // 将栈顶运算类型数值强制转换成uint64型数值并将结果压入栈顶
        ARITHMETIC( 2_F32 ),    // 将栈顶运算类型数值强制转换成float32型数值并将结果压入栈顶
        ARITHMETIC( 2_F64 ),    // 将栈顶运算类型数值强制转换成float64型数值并将结果压入栈顶
        ARITHMETIC( CMP ),      // 比较栈顶两算数类型数值大小，并将结果int32（-1，0，1）压入栈顶
        IF_EQ,	                // 当栈顶int32型数值等于0时跳转
        IF_NE,	                // 当栈顶int32型数值不等于0时跳转
        IF_LT,	                // 当栈顶int32型数值小于0时跳转
        IF_LE,	                // 当栈顶int32型数值小于等于0时跳转
        IF_GT,	                // 当栈顶int32型数值大于0时跳转
        IF_GE,	                // 当栈顶int32型数值大于等于0时跳转
        IF_NULL,	            // 为null时跳转
        IF_NONNULL,	            // 不为null时跳转
        GOTO,	                // 无条件跳转
        JSR,	                // 跳转至指定int32型数值的位置，并将jsr下一条指令地址压入栈顶
        RET,	                // 返回至本地变量指定的index的指令位置（一般与jsr联合使用）
        TABLE_SWITCH,	        // 用于switch条件跳转，case值连续（可变长度指令）
        LOOKUP_SWITCH,	        // 用于switch条件跳转，case值不连续（可变长度指令）
        RETURN,	                // 从当前方法返回void
        ALLTYPES( RETURN ),     // 从当前方法返回任意类型
        NEW,	                // 创建一个对象，并将其引用值压入栈顶
        GET_STATIC,	            // 获取指定类的静态域，并将其值压入栈顶
        PUT_STATIC,	            // 为指定的类的静态域赋值
        GET_PROPERTY,	        // 获取指定类的属性，并将其值压入栈顶
        PUT_PROPERTY,	        // 为指定的类的属性赋值
        INVOKE_VIRTUAL,	        // 调用实例虚函数
        INVOKE_CONSTRUCT,	    // 调用类构造函数
        INVOKE_DECONSTRUCT,	    // 调用类析构函数
        INVOKE_STATIC,	        // 调用静态方法
        THROW,	                // 将栈顶的异常抛出
        TYPE_OF,	            // 检验对象是否是指定的类的实例，如果是将1压入栈顶，否则将0压入栈顶


#undef SIGNED
#undef UNSIGNED
#undef INTEGRAL
#undef FLOATING
#undef ARITHMETIC
#undef ALLTYPES
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
        THIRDPARTY,
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
    enum class operator_t
    {
        NONE,
        XOR_ASSIGN,			// ^= 
        OR_ASSIGN,			// |= 
        AND_ASSIGN,			// &= 
        RSHIFT_ASSIGN,		// >>=
        LSHIFT_ASSIGN,		// <<=
        SUB_ASSIGN,			// -= 
        ADD_ASSIGN,			// +=
        MOD_ASSIGN,			// %= 
        DIV_ASSIGN,			// /= 
        MUL_ASSIGN,			// *=
        ASSIGN,				// =
        LOR,				// ||
        LAND,				// &&
        OR,					// |
        XOR,				// ^
        AND,				// &
        NOT_EQUAL,			// !=
        EQUAL,				// ==
        COMPARE,			// <=>
        LARG_EQUAL,			// >=
        LESS_EQUAL,			// <=
        LARG,				// >
        LESS,				// <
        RIGHT_SHIFT,		// >> 
        LEFT_SHIFT,			// <<
        SUB,				// - 
        ADD,				// +
        MOD,				// %
        DIV,				// / 
        MUL,				// * 
        MINUS,			    // -i
        PLUS,			    // +i
        NOT,			    // !
        REV,			    // ~
        POSTDEC,		    // i--
        POSTINC,		    // i++
        SIZEOF,			    // sizeof
        TYPEOF,			    // typeof
        AS,					// as
        IS,					// is
        DEC,			    // --i
        INC,			    // ++i
        INVOKE,				// x(y)
        INDEX,              // x[y]
        MEMBER,				// x.y

    };
    enum class callmode_t
    {
        MODE_C,
        MODE_STD,
        MODE_FAST,
        MODE_THIS,
    };
    enum class corostatus_t
    {
        EMPTY,
        SUSPEND,
        READY,
        EXCEPT,
    };

    using value_flags = flags<x::value_t>;
    using valloc_flags = flags<x::valloc_t>;

    class value;
    class object;
    class scheduler;

    PTR( logger );
    PTR( module );
    PTR( grammar );
    PTR( visitor );
    PTR( builtin );
    PTR( symbols );
    PTR( context );
    PTR( runtime );
    PTR( compiler );
    PTR( interpreter );
    PTR( virtual_machine );

    PTR( meta );
    PTR( meta_type );
    PTR( meta_enum );
    PTR( meta_class );
    PTR( meta_template );
    PTR( meta_variable );
    PTR( meta_function );
    PTR( meta_interface );
    PTR( meta_namespace );
    PTR( meta_attribute );

    PTR( symbol );
    PTR( unit_symbol );
    PTR( enum_symbol );
    PTR( using_symbol );
    PTR( class_symbol );
    PTR( block_symbol );
    PTR( cycle_symbol );
    PTR( local_symbol );
    PTR( function_symbol );
    PTR( variable_symbol );
    PTR( template_symbol );
    PTR( paramater_symbol );
    PTR( interface_symbol );
    PTR( namespace_symbol );
    PTR( foundation_symbol );
    PTR( nativefunc_symbol );
    PTR( builtinfunc_symbol );
    PTR( enum_element_symbol );

    PTR( ast );
    PTR( unit_ast );
    PTR( import_ast );
    PTR( attribute_ast );
    PTR( parameter_ast );
    PTR( type_ast );
    PTR( func_type_ast );
    PTR( temp_type_ast );
    PTR( list_type_ast );
    PTR( array_type_ast );
    PTR( decl_ast );
    PTR( enum_decl_ast );
    PTR( class_decl_ast );
    PTR( using_decl_ast );
    PTR( template_decl_ast );
    PTR( variable_decl_ast );
    PTR( function_decl_ast );
    PTR( interface_decl_ast );
    PTR( namespace_decl_ast );
    PTR( stat_ast );
    PTR( empty_stat_ast );
    PTR( extern_stat_ast );
    PTR( compound_stat_ast );
    PTR( await_stat_ast );
    PTR( yield_stat_ast );
    PTR( if_stat_ast );
    PTR( cycle_stat_ast );
    PTR( while_stat_ast );
    PTR( for_stat_ast );
    PTR( foreach_stat_ast );
    PTR( switch_stat_ast );
    PTR( break_stat_ast );
    PTR( return_stat_ast );
    PTR( try_stat_ast );
    PTR( throw_stat_ast );
    PTR( continue_stat_ast );
    PTR( local_stat_ast );
    PTR( mulocal_stat_ast );
    PTR( expr_stat_ast );
    PTR( binary_expr_ast );
    PTR( unary_expr_ast );
    PTR( bracket_expr_ast );
    PTR( closure_expr_ast );
    PTR( elements_expr_ast );
    PTR( arguments_expr_ast );
    PTR( identifier_expr_ast );
    PTR( initializer_expr_ast );
    PTR( constant_expr_ast );
    PTR( null_constant_expr_ast );
    PTR( bool_constant_expr_ast );
    PTR( int_constant_expr_ast );
    PTR( int32_constant_expr_ast );
    PTR( int64_constant_expr_ast );
    PTR( uint_constant_expr_ast );
    PTR( uint32_constant_expr_ast );
    PTR( uint64_constant_expr_ast );
    PTR( float_constant_expr_ast );
    PTR( float32_constant_expr_ast );
    PTR( float64_constant_expr_ast );
    PTR( string_constant_expr_ast );

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
        { ";", x::token_t::TK_SEMICOLON },
        { ",", x::token_t::TK_COMMA },
        { "++", x::token_t::TK_INC },
        { "--", x::token_t::TK_DEC },
        { "+", x::token_t::TK_ADD },
        { "-", x::token_t::TK_SUB },
        { "*", x::token_t::TK_MUL },
        { "/", x::token_t::TK_DIV },
        { "%", x::token_t::TK_MOD },
        { "&", x::token_t::TK_AND },
        { "|", x::token_t::TK_OR },
        { "^", x::token_t::TK_XOR },
        { "<<", x::token_t::TK_LEFT_SHIFT },
        { ">>", x::token_t::TK_RIGHT_SHIFT },
        { "&&", x::token_t::TK_LAND },
        { "||", x::token_t::TK_LOR },
        { "!", x::token_t::TK_NOT },
        { "~", x::token_t::TK_REV },
        { "=", x::token_t::TK_ASSIGN },
        { "+=", x::token_t::TK_ADD_ASSIGN },
        { "-=", x::token_t::TK_SUB_ASSIGN },
        { "*=", x::token_t::TK_MUL_ASSIGN },
        { "/=", x::token_t::TK_DIV_ASSIGN },
        { "%=", x::token_t::TK_MOD_ASSIGN },
        { "&=", x::token_t::TK_AND_ASSIGN },
        { "|=", x::token_t::TK_OR_ASSIGN },
        { "^=", x::token_t::TK_XOR_ASSIGN },
        { "<<=", x::token_t::TK_LSHIFT_ASSIGN },
        { ">>=", x::token_t::TK_RSHIFT_ASSIGN },
        { "==", x::token_t::TK_EQUAL },
        { "!=", x::token_t::TK_NOT_EQUAL },
        { "<", x::token_t::TK_LESS },
        { ">", x::token_t::TK_LARG },
        { "<=", x::token_t::TK_LESS_OR_EQUAL },
        { ">=", x::token_t::TK_LARG_OR_EQUAL },
        { "<=>", x::token_t::TK_COMPARE },
        { ":", x::token_t::TK_TYPECAST },
        { ".", x::token_t::TK_POINT },
        { "?", x::token_t::TK_QUESTION },
        { "..", x::token_t::TK_VARIADIC_SIGN },
        { "...", x::token_t::TK_VARIADIC_SIGN },
        { "[", x::token_t::TK_LEFT_INDEX },
        { "]", x::token_t::TK_RIGHT_INDEX },
        { "->", x::token_t::TK_FUNCTION_RESULT },
        { "(", x::token_t::TK_LEFT_BRACKETS },
        { ")", x::token_t::TK_RIGHT_BRACKETS },
        { "{", x::token_t::TK_LEFT_CURLY_BRACES },
        { "}", x::token_t::TK_RIGHT_CURLY_BRACES },
        { "any", x::token_t::TK_ANY },
        { "void", x::token_t::TK_VOID },
        { "byte", x::token_t::TK_BYTE },
        { "bool", x::token_t::TK_BOOL },
        { "int8", x::token_t::TK_INT8 },
        { "int16", x::token_t::TK_INT16 },
        { "int32", x::token_t::TK_INT32 },
        { "int64", x::token_t::TK_INT64 },
        { "uint8", x::token_t::TK_UINT8 },
        { "uint16", x::token_t::TK_UINT16 },
        { "uint32", x::token_t::TK_UINT32 },
        { "uint64", x::token_t::TK_UINT64 },
        { "float16", x::token_t::TK_FLOAT16 },
        { "float32", x::token_t::TK_FLOAT32 },
        { "float64", x::token_t::TK_FLOAT64 },
        { "string", x::token_t::TK_STRING },
        { "intptr", x::token_t::TK_INTPTR },
        { "object", x::token_t::TK_OBJECT },
        { "import", x::token_t::TK_IMPORT },
        { "assert", x::token_t::TK_ASSERT },
        { "template", x::token_t::TK_TEMPLATE },
        { "namespace", x::token_t::TK_NAMESPACE },
        { "attribute", x::token_t::TK_ATTRIBUTE },
        { "using", x::token_t::TK_USING },
        { "enum", x::token_t::TK_ENUM },
        { "class", x::token_t::TK_CLASS },
        { "friend", x::token_t::TK_FRIEND },
        { "interface", x::token_t::TK_INTERFACE },
        { "var", x::token_t::TK_VARIABLE },
        { "func", x::token_t::TK_FUNCTION },
        { "where", x::token_t::TK_WHERE },
        { "construct", x::token_t::TK_CONSTRUCT },
        { "finalize", x::token_t::TK_FINALIZE },
        { "private", x::token_t::TK_PRIVATE },
        { "public", x::token_t::TK_PUBLIC },
        { "protected", x::token_t::TK_PROTECTED },
        { "ref", x::token_t::TK_REF },
        { "local", x::token_t::TK_LOCAL },
        { "const", x::token_t::TK_CONST },
        { "final", x::token_t::TK_FINAL },
        { "static", x::token_t::TK_STATIC },
        { "extern", x::token_t::TK_EXTERN },
        { "virtual", x::token_t::TK_VIRTUAL },
        { "override", x::token_t::TK_OVERRIDE },
        { "thread", x::token_t::TK_THREAD },
        { "while", x::token_t::TK_WHILE },
        { "if", x::token_t::TK_IF },
        { "else", x::token_t::TK_ELSE },
        { "for", x::token_t::TK_FOR },
        { "foreach", x::token_t::TK_FOREACH },
        { "switch", x::token_t::TK_SWITCH },
        { "case", x::token_t::TK_CASE },
        { "default", x::token_t::TK_DEFAULT },
        { "await", x::token_t::TK_AWAIT },
        { "yield", x::token_t::TK_YIELD },
        { "break", x::token_t::TK_BREAK },
        { "return", x::token_t::TK_RETURN },
        { "try", x::token_t::TK_TRY },
        { "catch", x::token_t::TK_CATCH },
        { "throw", x::token_t::TK_THROW },
        { "continue", x::token_t::TK_CONTINUE },
        { "as", x::token_t::TK_AS },
        { "is", x::token_t::TK_IS },
        { "sizeof", x::token_t::TK_SIZEOF },
        { "typeof", x::token_t::TK_TYPEOF },
        { "null", x::token_t::TK_NULL },
        { "true", x::token_t::TK_TRUE },
        { "false", x::token_t::TK_FALSE },
        { "this", x::token_t::TK_THIS },
        { "base", x::token_t::TK_BASE },
    };
}

namespace llvm
{
    PTR( module );
}

namespace spirv
{
    PTR( module );
}
