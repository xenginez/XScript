#pragma once

#include <deque>
#include <filesystem>
#include <functional>

#include "type.h"
#include "url.hpp"

namespace x
{
	class compiler : public std::enable_shared_from_this<compiler>
	{
	public:
		struct object : public std::enable_shared_from_this<object>
		{
			bool reset = false;
			x::unit_ast_ptr ast;
			std::filesystem::path path;
			std::filesystem::file_time_type time;
		};
		using object_ptr = std::shared_ptr<object>;
		using log_callback_t = std::function<void( std::string_view )>;

	public:
		compiler( const x::logger_ptr & logger = nullptr );
		virtual ~compiler();

	public:
		void reload( const std::filesystem::path & file );
		bool compile( const std::filesystem::path & path );

	public:
		x::module_ptr module() const;
		x::logger_ptr logger() const;
		x::symbols_ptr symbols() const;

	public:
		x::uint32 get_module_version() const;
		void set_module_version( x::uint32 version );
		const std::string & get_module_name() const;
		void set_module_name( const std::string & name );
		const std::string & get_module_author() const;
		void set_module_author( const std::string & author );
		const std::string & get_module_origin() const;
		void set_module_origin( const std::string & origin );
		void add_link_path( const std::filesystem::path & path );

	private:
		void scanning();
		void analysis();
		void optimize();
		void import();
		void generate();
		void linking();

	private:
		virtual void compile_module();
		virtual x::symbols_ptr make_symbols() const;
		virtual std::filesystem::path make_stdpath() const;

	private:
		void load_module( const std::string & modulename );
		x::module_ptr load_std_module( const std::string & modulename );
		x::module_ptr load_network_module( const std::string & modulename );
		x::module_ptr load_filesystem_module( const std::string & modulename );

	private:
		std::string _name;
		std::string _author;
		std::string _origin;
		x::uint32 _version = 0;
		x::module_ptr _module;
		x::logger_ptr _logger;
		x::grammar_ptr _grammar;
		x::symbols_ptr _symbols;
		std::vector<x::module_ptr> _modules;
		std::vector<x::compiler::object_ptr> _objects;
		std::vector<std::filesystem::path> _linkpaths;
	};

	class llvm_compiler : public x::compiler
	{
	public:
		llvm_compiler( const x::logger_ptr & logger = nullptr );
		~llvm_compiler() override;

	public:
		llvm::module_ptr llvm_module() const;

	protected:
		void compile_module() override;
		x::symbols_ptr make_symbols() const override;
		std::filesystem::path make_stdpath() const override;

	private:
		llvm::module_ptr _module;
	};

	class spirv_compiler : public x::compiler
	{
	public:
		spirv_compiler( const x::logger_ptr & logger = nullptr );
		~spirv_compiler() override;

	public:
		spirv::module_ptr spirv_module() const;

	protected:
		void compile_module() override;
		x::symbols_ptr make_symbols() const override;
		std::filesystem::path make_stdpath() const override;

	private:
		spirv::module_ptr _module;
	};
}
