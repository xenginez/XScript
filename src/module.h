#pragma once

#include "ast.h"

namespace x
{
    struct type_section
    {
        enum flag_t
        {
            ENUM,
            FLAG,
            CLASS,
        };

        struct item
        {
            flag_t flag;
            x::uint64 size;
            x::static_string_view name;
        };

        std::vector<item> items;
    };

    struct desc_section
    {
        struct item
        {
            x::uint64 type; // type section index
            int array = 0;
            bool is_const = false;
        };

        std::vector<item> items;
    };

    struct temp_section
    {
        std::vector<std::string> items;
    };

    struct depend_section
    {
    public:
        std::vector<x::static_string_view> items;
    };

    struct global_section
    {
        struct item
        {
            x::uint64 type; // desc section index
            bool is_thread = false;
            x::static_string_view name;
            union
            {
                x::int64 enum_init;
                x::uint64 flag_init;
                x::uint64 struct_init; // codedata section index
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
            x::uint64 code; // codedata section index
            x::uint64 owner; // type section index
            x::uint64 result; // desc section index
            std::vector<x::uint64> parameters; // desc section index
            x::static_string_view name;
        };

        std::vector<item> items;
    };

    struct variable_section
    {
        struct item
        {
            x::uint64 owner; // type section index
            x::uint64 value; // desc section index
            x::static_string_view name;
        };

        std::vector<item> items;
    };

    struct codedata_section
    {
        std::vector<x::byte> datas;
    };

    struct stringdata_section
    {
        std::vector<std::string> datas;
    };

    struct customdata_section
    {
        std::vector<std::string> datas;
    };

	class module : public ast_visitor
	{
	public:
		module();

	public:
		void generate( const x::symbols_ptr & symbols, const x::unit_ast_ptr & unit );

    private:
		void visit( x::unit_ast * val ) override;
		void visit( x::import_ast * val ) override;
		void visit( x::attribute_ast * val ) override;

        void visit( x::type_ast * val ) override;
        void visit( x::temp_type_ast * val ) override;
        void visit( x::func_type_ast * val ) override;
        void visit( x::array_type_ast * val ) override;

		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::using_decl_ast * val ) override;
		void visit( x::element_decl_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;
		void visit( x::variable_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::parameter_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::empty_stat_ast * val ) override;
		void visit( x::extern_stat_ast * val ) override;
		void visit( x::compound_stat_ast * val ) override;
		void visit( x::await_stat_ast * val ) override;
		void visit( x::yield_stat_ast * val ) override;
		void visit( x::try_stat_ast * val ) override;
		void visit( x::catch_stat_ast * val ) override;
		void visit( x::throw_stat_ast * val ) override;
		void visit( x::if_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;
		void visit( x::break_stat_ast * val ) override;
		void visit( x::return_stat_ast * val ) override;
		void visit( x::continue_stat_ast * val ) override;
		void visit( x::local_stat_ast * val ) override;

		void visit( x::assignment_exp_ast * val ) override;
		void visit( x::logical_or_exp_ast * val ) override;
		void visit( x::logical_and_exp_ast * val ) override;
		void visit( x::or_exp_ast * val ) override;
		void visit( x::xor_exp_ast * val ) override;
		void visit( x::and_exp_ast * val ) override;
		void visit( x::compare_exp_ast * val ) override;
		void visit( x::shift_exp_ast * val ) override;
		void visit( x::add_exp_ast * val ) override;
		void visit( x::mul_exp_ast * val ) override;
		void visit( x::as_exp_ast * val ) override;
		void visit( x::is_exp_ast * val ) override;
		void visit( x::sizeof_exp_ast * val ) override;
		void visit( x::typeof_exp_ast * val ) override;
		void visit( x::unary_exp_ast * val ) override;
		void visit( x::postfix_exp_ast * val ) override;
		void visit( x::index_exp_ast * val ) override;
		void visit( x::invoke_exp_ast * val ) override;
		void visit( x::member_exp_ast * val ) override;
		void visit( x::identifier_exp_ast * val ) override;
		void visit( x::closure_exp_ast * val ) override;
		void visit( x::arguments_exp_ast * val ) override;
		void visit( x::initializers_exp_ast * val ) override;
		void visit( x::null_const_exp_ast * val ) override;
		void visit( x::bool_const_exp_ast * val ) override;
		void visit( x::int_const_exp_ast * val ) override;
		void visit( x::float_const_exp_ast * val ) override;
		void visit( x::string_const_exp_ast * val ) override;

	public:
		type_section type;
		desc_section desc;
		temp_section temp;
		depend_section depend;
		global_section global;
		function_section function;
		variable_section variable;
		codedata_section codedata;
		stringdata_section stringdata;
		customdata_section customdata;

	private:
		x::symbols * _symbols = nullptr;
	};
}
