#pragma once

#include "visitor.h"

namespace x
{
	class builtin : public x::visitor
	{
	public:
		virtual x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const = 0;
	};

	// uint64 builtin_sizeof( protocol | var )
	class builtin_sizeof : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// meta_type builtin_typeof( protocol | var )
	class builtin_typeof : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// protocol builtin_list_sizeof( list_type )
	class builtin_list_sizeof : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// protocol builtin_list_typeof( list_type, index )
	class builtin_list_typeof : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_same( left_type, right_type )
	class builtin_is_same : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_enum( protocol )
	class builtin_is_enum : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_class( protocol )
	class builtin_is_class : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_base_of( protocol, base_type )
	class builtin_is_base_of : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// protocol builtin_conditional( cond, then_type, else_type )
	class builtin_conditional : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_template_of( instant_template_type, template_type_name )
	class builtin_is_template_of : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
}
