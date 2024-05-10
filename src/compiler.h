#pragma once

#include <fstream>
#include <filesystem>

#include "type.h"

namespace x
{
	class compiler
	{
		struct private_p;

	public:
		compiler();
		~compiler();

	public:
		void add_search_path( const std::filesystem::path & path );
		x::context_ptr compile( const std::filesystem::path & file );

	private:
		bool load_source_file( std::filesystem::path file );
		x::context_ptr generate() const;

	private:
		private_p * _p = nullptr;
	};
}