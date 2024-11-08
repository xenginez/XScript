#pragma once

#include "ast.h"

namespace x
{
    struct type_section
    {
        enum class flag_t : x::uint8
        {
            ENUM,
            CLASS,
        };

        struct item
        {
            flag_t flag;
            x::range name; // stringdata section range
            x::uint64 size;
        };

        std::vector<item> items;
    };

    struct temp_section
    {
        struct item
        {
            std::vector<x::byte> data;
            x::range name; // stringdata section range
        };

        std::vector<item> items;
    };

    struct desc_section
    {
        enum class flag_t : x::uint8
        {
            TYPE,
            REFTYPE,
            TEMPTYPE,
            FUNCTYPE,
        };

        struct item
        {
            flag_t flag;
            bool is_const;
            union
            {
                struct
                {
                    x::uint64 type; // protocol section index
                } type;

                struct
                {
                    x::uint64 type; // protocol section index
                } reftype;

                struct
                {
                    x::uint64 temp; // temp section index
                    //std::vector<x::uint64> elements; // desc section index
                } temptype;

                struct
                {
                    x::uint64 result; // desc section index
                    //std::vector<x::uint64> params; // desc section index
                } functype;
            };
        };

        std::vector<item> items;
    };

    struct depend_section
    {
        enum class flag_t : x::uint8
        {
            ENUM,
            CLASS,
            VARIABLE,
            FUNCTION,
        };

        struct item
        {
            flag_t flag;
            x::range name; // stringdata section range
        };

        std::vector<item> items;
    };

    struct global_section
    {
        enum class flag_t : x::uint8
        {
            STATIC,
            THREAD,
        };

        struct item
        {
            flag_t flag;
            x::uint64 type; // desc section index
            x::range name; // stringdata section range
            union
            {
                x::int64 enum_init;
                x::uint64 class_init; // opcodedata section index
            };
        };

        std::vector<item> items;
    };

    struct function_section
    {
        struct item
        {
            bool is_const = false;
            bool is_async = false;
            bool is_static = false;
            bool is_virtual = false;
            x::range name; // stringdata section range
            x::uint64 code; // opcodedata section index
            x::uint64 owner; // protocol section index
            x::uint64 result; // desc section index
            std::vector<x::uint64> parameters; // desc section index
        };

        std::vector<item> items;
    };

    struct variable_section
    {
        enum class flag_t : x::uint8
        {
            LOCAL,
            STATIC,
            THREAD,
        };

        struct item
        {
            flag_t flag;
            x::uint64 idx;
            x::range name; // stringdata section range
            x::uint64 owner; // protocol section index
            x::uint64 value; // desc section index
        };

        std::vector<item> items;
    };

    struct attribute_section
    {
        struct item
        {
            x::uint64 type; // protocol section index
            x::range key; // stringdata section range
            x::range value; // stringdata section range
        };

        std::vector<item> items;
    };

    struct opcodedata_section
    {
        std::vector<x::byte> datas;
    };

    struct stringdata_section
    {
        std::string datas;
    };

    struct customdata_section
    {
        std::vector<std::string> datas;
    };

    struct thirdparty_section
    {
        struct item
        {
            std::string name;
            std::string author;
            std::string origin;
            x::uint32   version;
        };

        std::vector<item> items;
    };

	class module : private std::enable_shared_from_this<module>
	{
	public:
		module();

    public:
        void load( std::istream & in );
        void save( std::ostream & out ) const;

	public:
        bool merge( const x::module_ptr & other );
		
	public:
        x::int32 version = 0;
        x::int64 lasttime = 0;
        std::string name;
        std::string author;
        std::string origin;
		type_section types;
		temp_section temps;
        desc_section descs;
		depend_section depends;
		global_section globals;
		function_section functions;
		variable_section variables;
        attribute_section attributes;
        opcodedata_section opcodedatas;
		stringdata_section stringdatas;
		customdata_section customdatas;
	};
}

namespace llvm
{
    class module : private std::enable_shared_from_this<module>
    {

    };
}

namespace spirv
{
    class module : private std::enable_shared_from_this<module>
    {

    };
}