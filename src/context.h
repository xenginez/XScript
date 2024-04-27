#pragma once

#include <fstream>
#include <filesystem>

#include "meta.h"

namespace x
{
	class context : public std::enable_shared_from_this<context>
	{
		struct private_p;

	public:
		context();
		~context();

	public:
		int version() const;
		const x::symbols_ptr & symbols() const;

	public:
		x::meta_ptr find_meta( std::uint64_t hashcode ) const;

	public:
		bool load_script_file( const std::filesystem::path & file );
		bool load_script_stream( std::istream & stream, std::string_view name );
		bool load_library_file( const std::filesystem::path & file );
		bool load_library_stream( std::istream & stream, std::string_view name );

	public:
		void add_search_path( const std::filesystem::path & path );
		std::filesystem::path search_path( const std::filesystem::path & path ) const;

	private:
		void register_meta( const meta_ptr & val );
		x::static_string_view trans_string_view( std::string_view str );
		bool recursion_import( std::istream & stream, std::filesystem::path path, std::vector<x::unit_ast_ptr> & units );

	private:
		private_p * _p = nullptr;
	};
}
