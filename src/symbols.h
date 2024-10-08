#pragma once

#include <deque>

#include "type.h"

namespace x
{
	class symbol : public std::enable_shared_from_this<x::symbol>
	{
	public:
		symbol() = default;
		virtual ~symbol() = default;

	private:
		symbol( symbol && ) = delete;
		symbol( const symbol & ) = delete;
		symbol & operator=( symbol && ) = delete;
		symbol & operator=( const symbol & ) = delete;

	public:
		virtual bool is_type() const;
		virtual bool is_scope() const;
		virtual bool is_value() const;
		virtual x::ast_ptr ast() const;
		virtual x::uint64 size() const;
		virtual void add_child( const x::symbol_ptr & val );
		virtual x::symbol_ptr find_child( std::string_view name ) const;

	public:
		x::symbol_t type = x::symbol_t::UNIT;
		std::string name;
		std::string fullname;
		x::symbol_wptr parent;
		x::symbols_wptr symbols;
		x::access_t access = x::access_t::PUBLIC;
	};
	class unit_symbol : public x::symbol
	{
	public:
		unit_symbol();
		~unit_symbol() override;

	public:
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		x::unit_ast_ptr unit_ast;
		std::vector<x::symbol_ptr> children;
	};
	class enum_symbol : public x::symbol
	{
	public:
		enum_symbol();
		~enum_symbol() override;

	public:
		bool is_type() const override;
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		x::uint64 size() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		x::enum_decl_ast_ptr enum_ast;
		std::vector<x::enum_element_symbol_ptr> elements;
	};
	class using_symbol : public x::symbol
	{
	public:
		using_symbol();
		~using_symbol() override;

	public:
		bool is_type() const override;
		x::ast_ptr ast() const override;
		x::uint64 size() const override;

	public:
		x::symbol_wptr retype;
		x::using_decl_ast_ptr using_ast;
	};
	class class_symbol : public x::symbol
	{
	public:
		class_symbol();
		~class_symbol() override;

	public:
		bool is_type() const override;
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		x::uint64 size() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		x::class_symbol_wptr base;
		x::class_decl_ast_ptr class_ast;
		std::vector<x::using_symbol_ptr> usings;
		std::vector<x::function_symbol_ptr> functions;
		std::vector<x::variable_symbol_ptr> variables;
	};
	class block_symbol : public x::symbol
	{
	public:
		block_symbol();
		~block_symbol() override;

	public:
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		x::compound_stat_ast_ptr compound_ast;
		std::vector<x::local_symbol_ptr> locals;
		std::vector<x::block_symbol_ptr> blocks;
	};
	class cycle_symbol : public x::symbol
	{
	public:
		cycle_symbol();
		~cycle_symbol() override;

	public:
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		x::cycle_stat_ast_ptr cycle_ast;
		std::vector<x::local_symbol_ptr> locals;
		std::vector<x::block_symbol_ptr> blocks;
	};
	class local_symbol : public x::symbol
	{
	public:
		local_symbol();
		~local_symbol() override;

	public:
		bool is_value() const override;
		x::ast_ptr ast() const override;

	public:
		x::symbol_wptr valuetype;
		x::local_stat_ast_ptr local_ast;
	};
	class variable_symbol : public x::symbol
	{
	public:
		variable_symbol();
		~variable_symbol() override;

	public:
		bool is_value() const override;
		x::ast_ptr ast() const override;

	public:
		x::symbol_wptr value;
		x::variable_decl_ast_ptr variable_ast;
	};
	class function_symbol : public x::symbol
	{
	public:
		function_symbol();
		~function_symbol() override;

	public:
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		x::block_symbol_ptr block = nullptr;
		x::function_decl_ast_ptr function_ast;
		std::vector<x::symbol_wptr> results;
		std::vector<x::paramater_symbol_ptr> parameters;
	};
	class template_symbol : public x::symbol
	{
	public:
		template_symbol();
		~template_symbol() override;

	public:
		bool is_type() const override;
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		x::uint64 size() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		x::class_symbol_wptr base;
		x::template_decl_ast_ptr template_ast;
		std::vector<x::symbol_wptr> elements;
		std::vector<x::using_symbol_ptr> usings;
		std::vector<x::function_symbol_ptr> functions;
		std::vector<x::variable_symbol_ptr> variables;
		std::vector<x::class_symbol_wptr> specializeds;
	};
	class paramater_symbol : public x::symbol
	{
	public:
		paramater_symbol();
		~paramater_symbol() override;

	public:
		bool is_value() const override;
		x::ast_ptr ast() const override;

	public:
		x::symbol_wptr valuetype;
		x::parameter_ast_ptr parameter_ast;
	};
	class interface_symbol : public x::symbol
	{
	public:
		interface_symbol();
		~interface_symbol() override;

	public:
		bool is_type() const override;
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		x::interface_decl_ast_ptr interface_ast;
		std::vector<x::function_symbol_ptr> functions;
	};
	class namespace_symbol : public x::symbol
	{
	public:
		namespace_symbol();
		~namespace_symbol() override;

	public:
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		std::vector<x::symbol_ptr> children;
		x::namespace_decl_ast_ptr namespace_ast;
	};
	class foundation_symbol : public x::symbol
	{
	public:
		foundation_symbol();
		~foundation_symbol() override;

	public:
		bool is_type() const override;
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		x::uint64 size() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		x::uint64 sz = 0;
		std::vector<x::function_symbol_ptr> functions;
		std::vector<x::variable_symbol_ptr> variables;
	};
	class nativefunc_symbol : public x::symbol
	{
	public:
		nativefunc_symbol();
		~nativefunc_symbol() override;

	public:
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		bool is_await = false;
		void * callback = nullptr;
		x::symbol_wptr result;
		std::vector<x::paramater_symbol_ptr> parameters;
	};
	class builtinfunc_symbol : public x::symbol
	{
	public:
		builtinfunc_symbol();
		~builtinfunc_symbol() override;

	public:
		bool is_scope() const override;
		x::ast_ptr ast() const override;
		void add_child( const x::symbol_ptr & val ) override;
		x::symbol_ptr find_child( std::string_view name ) const override;

	public:
		x::builtin_ptr built = nullptr;
		x::symbol_wptr result;
		std::vector<x::paramater_symbol_ptr> parameters;
	};
	class enum_element_symbol : public x::symbol
	{
	public:
		enum_element_symbol();
		~enum_element_symbol() override;

	public:
		bool is_value() const override;
		x::ast_ptr ast() const override;

	public:
		x::expr_stat_ast_ptr element_ast;
	};

	class symbols : public std::enable_shared_from_this<symbols>
	{
	public:
		symbols();
		~symbols();

	public:
		void add_module( x::module * val );

	public:
		x::unit_symbol_ptr add_unit( x::unit_ast * val );
		x::enum_symbol_ptr add_enum( x::enum_decl_ast * val );
		x::using_symbol_ptr add_using( x::using_decl_ast * val );
		x::class_symbol_ptr add_class( x::class_decl_ast * val );
		x::block_symbol_ptr add_block( x::compound_stat_ast * val );
		x::cycle_symbol_ptr add_cycle( x::cycle_stat_ast * val );
		x::local_symbol_ptr add_local( x::local_stat_ast * val );
		x::function_symbol_ptr add_function( x::function_decl_ast * val );
		x::variable_symbol_ptr add_variable( x::variable_decl_ast * val );
		x::template_symbol_ptr add_template( x::template_decl_ast * val );
		x::paramater_symbol_ptr add_paramater( x::parameter_ast * val );
		x::interface_symbol_ptr add_interface( x::interface_decl_ast * val );
		x::namespace_symbol_ptr add_namespace( x::namespace_decl_ast * val );
		x::foundation_symbol_ptr add_foundation( std::string_view name, x::uint64 size );
		x::nativefunc_symbol_ptr add_nativefunc( std::string_view name, void * callback );
		x::builtinfunc_symbol_ptr add_builtinfunc( std::string_view name, x::builtin_ptr builtin );
		x::enum_element_symbol_ptr add_enum_element( std::string_view name, x::expr_stat_ast * val );

	public:
		void push_scope( std::string_view name );
		void push_scope( const x::symbol_ptr & symbol );
		x::symbol_ptr current_scope() const;
		void pop_scope();

	public:
		x::symbol_ptr find_symbol( std::string_view name, x::symbol_ptr scope = nullptr ) const;
		x::symbol_ptr find_symbol_from_type( std::string_view name, x::symbol_t type, x::symbol_ptr scope = nullptr ) const;

	public:
		x::namespace_symbol_ptr global_namespace() const;
		std::string calc_fullname( std::string_view name ) const;
		x::symbol_ptr find_symbol_from_ast( const x::ast_ptr & ast ) const;
		x::symbol_ptr find_symbol_from_fullname( std::string_view fullname ) const;
		std::vector<x::template_symbol_ptr> find_template_symbols( std::string_view fullname ) const;

	public:
		void add_reference( x::ast * ast, std::string_view name, const x::symbol_ptr & val );
		x::symbol_ptr find_reference( x::ast * ast, std::string_view name ) const;

	private:
		void add_symbol( const x::symbol_ptr & val );
		x::symbol_ptr up_find_symbol( std::string_view name, const x::symbol_ptr & scope ) const;
		x::symbol_ptr down_find_symbol( std::string_view name, const x::symbol_ptr & scope ) const;

	private:
		std::deque<x::symbol_ptr> _scope;
		std::map<x::ast_ptr, x::symbol_ptr> _astmap;
		std::map<std::string, x::symbol_ptr> _symbolmap;
		std::multimap<std::string, x::template_symbol_ptr> _templatemap;
		std::map<x::ast *, std::map<std::string, x::symbol_ptr>> _referencemap;
	};
}
