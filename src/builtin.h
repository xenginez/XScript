#pragma once

#include "visitor.h"

namespace x
{
	class builtin : public x::visitor
	{
	public:
		virtual x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const = 0;
	};

	// uint64 builtin_sizeof( type | var )
	class builtin_sizeof : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// meta_type builtin_typeof( type | var )
	class builtin_typeof : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// type builtin_list_type( list_type, index )
	class builtin_list_type : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// uint64 builtin_list_sizeof( list_type )
	class builtin_list_sizeof : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// meta_type builtin_list_typeof( list_type, index )
	class builtin_list_typeof : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_same( left_type, right_type )
	class builtin_is_same : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_void( type )
	class builtin_is_void : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_enum( type )
	class builtin_is_enum : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_class( type )
	class builtin_is_class : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_base_of( type, base_type )
	class builtin_is_base_of : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_member_of( type, member_ident )
	class builtin_is_member_of : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// bool builtin_is_template_of( type, template_type )
	class builtin_is_template_of : public x::builtin
	{
	public:
		x::ast_ptr translate( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
}
