#pragma once

#include <span>
#include <deque>
#include <filesystem>
#include <functional>

#include "type.h"

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
		compiler( const log_callback_t & callback = nullptr );
		virtual ~compiler();

	public:
		bool compile( const std::filesystem::path & file );

	public:
		x::symbols_ptr symbols() const;
		std::span<const x::compiler::object_ptr> objects() const;

	public:
		void add_search_path( const std::filesystem::path & path );

	private:
		void scanner();
		void instant();
		void checker();
		void genunit();
		void linking();
		void load_source_file( std::filesystem::path file );

	protected:
		virtual void genmodule() = 0;
		virtual void linkmodule() = 0;
		virtual x::compiler::object_ptr make_object() = 0;

	private:
		log_callback_t _logger;
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
		module_compiler( const log_callback_t & callback = nullptr );
		~module_compiler() override;

	public:
		x::module_ptr module() const;

	protected:
		void genmodule() override;
		void linkmodule() override;
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
		llvm_compiler( const log_callback_t & callback = nullptr );
		~llvm_compiler() override;

	public:
		llvm::module_ptr module() const;

	protected:
		void genmodule() override;
		void linkmodule() override;
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
		spirv_compiler( const log_callback_t & callback = nullptr );
		~spirv_compiler() override;

	public:
		spirv::module_ptr module() const;

	protected:
		void genmodule() override;
		void linkmodule() override;
		x::compiler::object_ptr make_object() override;

	private:
		spirv::module_ptr _module = nullptr;
	};
}
