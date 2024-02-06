#pragma once

#include <filesystem>

#include "meta.h"

namespace x
{
	class context : public std::enable_shared_from_this<context>
	{
		struct private_p;

	public:
		template<typename O, typename T> class native_class;
		template<typename O, typename T> class native_enum;
		template<typename T> class native_namespace;

		template<typename O, typename T> class native_class
		{
			using owner_type = O;
			using class_type = T;
			using meta_class_type = meta_native_class<class_type>;

		public:
			native_class( context * ctx, owner_type owner, const std::shared_ptr<meta_class_type> & ptr )
				: _ctx( ctx ), _owner( owner ), _meta( ptr )
			{
			}

		public:
			template<typename V> native_class & variable( std::string_view name, V * val )
			{
				auto meta = std::make_shared<meta_native_static_variable<std::remove_const_t<V>>>();
				meta->_fullname = _ctx->trans_string_view( _meta->_fullname + "." + name );
				meta->_name = _ctx->trans_string_view( name );
				meta->_var = const_cast<std::remove_const_t<V>>( val );
				_meta->_variables.push_back( meta );
				_ctx->register_meta( meta );

				return *this;
			}
			template<typename V> native_class & variable( std::string_view name, V class_type::* val )
			{
				auto meta = std::make_shared<meta_native_variable<class_type, std::remove_const_t<V>>>();
				meta->_fullname = _ctx->trans_string_view( _meta->_fullname + "." + name );
				meta->_name = _ctx->trans_string_view( name );
				meta->_var = const_cast<std::remove_const_t<V>>( val );
				_meta->_variables.push_back( meta );
				_ctx->register_meta( meta );

				return *this;
			}
			template<typename R, typename ... As> native_class & function( std::string_view name, R( *func )( As... ) )
			{
				auto meta = std::make_shared<meta_native_static_function<R, As...>>();
				meta->_fullname = _ctx->trans_string_view( _meta->_fullname + "." + name );
				meta->_name = _ctx->trans_string_view( name );
				meta->_func = func;
				_meta->_functions.push_back( meta );
				_ctx->register_meta( meta );

				return *this;
			}
			template<typename R, typename ... As> native_class & function( std::string_view name, R( class_type:: * func )( As... ) )
			{
				auto meta = std::make_shared<meta_native_function<class_type, R, As...>>();
				meta->_fullname = _ctx->trans_string_view( _meta->_fullname + "." + name );
				meta->_name = _ctx->trans_string_view( name );
				meta->_func = func;
				_meta->_functions.push_back( meta );
				_ctx->register_meta( meta );

				return *this;
			}
			template<typename R, typename ... As> native_class & function( std::string_view name, R( class_type:: * func )( As... ) const )
			{
				auto meta = std::make_shared<meta_native_const_function<class_type, R, As...>>();
				meta->_fullname = _ctx->trans_string_view( _meta->_fullname + "." + name );
				meta->_name = _ctx->trans_string_view( name );
				meta->_func = func;
				_meta->_functions.push_back( meta );
				_ctx->register_meta( meta );

				return *this;
			}

		public:
			owner_type end_class()
			{
				return _owner;
			}

		private:
			context * _ctx;
			owner_type _owner;
			std::shared_ptr<meta_class_type> _meta;
		};
		template<typename O, typename T> class native_enum
		{
			using owner_type = O;
			using enum_type = T;
			using meta_enum_type = meta_native_enum<enum_type>;

		public:
			native_enum( context * ctx, owner_type owner, const std::shared_ptr<meta_enum_type> & ptr )
				: _ctx( ctx ), _owner( owner ), _meta( ptr )
			{
			}

		public:
			native_enum & element( std::string_view name, enum_type val )
			{
				_meta->_elements.push_back( { _ctx->trans_string_view( name ), (int64_t)val } );
				return *this;
			}

			owner_type end_enum()
			{
				return _owner;
			}

		private:
			context * _ctx;
			owner_type _owner;
			std::shared_ptr<meta_enum_type> _meta;
		};
		template<typename T> class native_namespace
		{
		public:
			using owner_type = T;
			using this_type = native_namespace<owner_type>;

		public:
			native_namespace( context * ctx, owner_type owner, const meta_namespace_ptr & ptr )
				: _ctx( ctx ), _owner( _owner ), _meta( ptr )
			{
			}

		public:
			template<typename T> native_enum<this_type &, T> beg_enum( std::string_view name )
			{
				auto meta = std::make_shared<meta_native_enum<T>>();
				meta->_fullname = _ctx->trans_string_view( _meta->_fullname + "." + name );
				meta->_name = _ctx->trans_string_view( name );
				_meta->_members.push_back( meta );
				_ctx->register_meta( meta );

				return { _ctx, *this, meta };
			}
			template<typename T> native_class<this_type &, T> beg_class( std::string_view name )
			{
				auto meta = std::make_shared<meta_native_class<T>>();
				meta->_fullname = _ctx->trans_string_view( _meta->_fullname + "." + name );
				meta->_name = _ctx->trans_string_view( name );
				_meta->_members.push_back( meta );
				_ctx->register_meta( meta );

				return { _ctx, *this, meta };
			}
			template<typename V> native_namespace & variable( std::string_view name, V * val )
			{
				auto meta = std::make_shared<meta_native_static_variable<std::remove_const_t<V>>>();
				meta->_name = _ctx->trans_string_view( name );
				meta->_var = const_cast<std::remove_const_t<V>>( val );
				_meta->_members.push_back( meta );
				_ctx->register_meta( meta );

				return *this;
			}
			template<typename R, typename ... As> native_namespace & function( std::string_view name, R( *func )( As... ) )
			{
				auto meta = std::make_shared<meta_native_static_function<R, As...>>();
				meta->_name = _ctx->trans_string_view( name );
				meta->_func = func;
				_meta->_members.push_back( meta );
				_ctx->register_meta( meta );

				return *this;
			}
			native_namespace<native_namespace<owner_type> &> beg_namespace( std::string_view name )
			{
				auto meta = std::make_shared<meta_namespace>();
				meta->_fullname = _ctx->trans_string_view( name );
				meta->_name = meta->_fullname;
				_meta->_members.push_back( meta );
				_ctx->register_meta( meta );

				return { _ctx, *this, meta };
			}

		public:
			owner_type end_namespace()
			{
				return _owner;
			}

		private:
			context * _ctx;
			owner_type _owner;
			meta_namespace_ptr _meta;
		};

	public:
		context();
		~context();

	public:
		native_namespace<context_ptr> beg_namespace( std::string_view name )
		{
			auto meta = std::make_shared<meta_namespace>();
			meta->_fullname = trans_string_view( name );
			meta->_name = meta->_fullname;
			register_meta( meta );

			return { this, shared_from_this(), meta };
		}

	public:
		int version() const;
		const x::symbols_ptr & symbols() const;

	public:
		x::meta_ptr find_meta( uint64_t hashcode ) const;

	public:
		void load_stdlib();
		bool load_file( const std::filesystem::path & file );
		bool load_stream( std::istream & stream, std::string_view name );

	public:
		void add_search_path( const std::filesystem::path & path );
		std::filesystem::path search_path( const std::filesystem::path & path ) const;

	private:
		void register_meta( const meta_ptr & val );
		x::static_string_view trans_string_view( std::string_view str );

	private:
		bool recursion_import( std::istream & stream, std::filesystem::path path, std::vector<x::unit_ast_ptr> & units );

	private:
		private_p * _p = nullptr;
	};
}
