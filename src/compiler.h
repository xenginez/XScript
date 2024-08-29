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
		bool compile( const std::filesystem::path & file );

	public:
		x::logger_ptr logger() const;
		x::symbols_ptr symbols() const;
		std::span<const x::compiler::object_ptr> objects() const;

	public:
		void add_search_path( const std::filesystem::path & path );

	private:
		virtual void scanner();
		virtual void analyzer();
		virtual void translate();
		virtual void genmodule() = 0;
		virtual void linkmodule() = 0;
		void loading( const x::url & url );
		void loading( const std::filesystem::path & file );

	protected:
		virtual x::symbols_ptr make_symbols() = 0;
		virtual x::compiler::object_ptr make_object() = 0;

	private:
		x::logger_ptr _logger;
		x::grammar_ptr _grammar;
		x::symbols_ptr _symbols;
		std::vector<object_ptr> _objects;
		std::deque<std::filesystem::path> _relative_paths;
		std::vector<std::filesystem::path> _absolute_paths;
	};

	class module_compiler : public compiler
	{
	public:
		struct object : public x::compiler::object
		{
			x::module_ptr module;
		};

	public:
		module_compiler( const x::logger_ptr & logger = nullptr );
		~module_compiler() override;

	public:
		x::module_ptr module() const;

	protected:
		void genmodule() override;
		void linkmodule() override;
		x::symbols_ptr make_symbols() override;
		x::compiler::object_ptr make_object() override;

	private:
		x::module_ptr _module;
	};

	class llvm_compiler : public compiler
	{
	public:
		struct object : public x::compiler::object
		{
			llvm::module_ptr module;
		};

	public:
		llvm_compiler( const x::logger_ptr & logger = nullptr );
		~llvm_compiler() override;

	public:
		llvm::module_ptr module() const;

	protected:
		void genmodule() override;
		void linkmodule() override;
		x::symbols_ptr make_symbols() override;
		x::compiler::object_ptr make_object() override;

	private:
		llvm::module_ptr _module;
		llvm::context_ptr _context;
	};

	class spirv_compiler : public compiler
	{
	public:
		struct object : public x::compiler::object
		{
			spirv::module_ptr module;
		};

	public:
		spirv_compiler( const x::logger_ptr & logger = nullptr );
		~spirv_compiler() override;

	public:
		spirv::module_ptr module() const;

	protected:
		void genmodule() override;
		void linkmodule() override;
		x::symbols_ptr make_symbols() override;
		x::compiler::object_ptr make_object() override;

	private:
		spirv::module_ptr _module = nullptr;
	};
}
