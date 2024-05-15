#pragma once

#include <filesystem>
#include <functional>

#include "type.h"

namespace x
{
	class compiler : public std::enable_shared_from_this<compiler>
	{
		struct private_p;

	public:
		using log_callback_t = std::function<void( std::string_view )>;

	public:
		compiler(const log_callback_t & callback = nullptr );
		~compiler();

	public:
		void add_search_path( const std::filesystem::path & path );
		x::module_ptr compile( const std::filesystem::path & file );

	private:
		void symbols();
		void instant();
		void checker();
		void gen_unit_module();
		x::module_ptr merge_all_module();
		void load_source_file( std::filesystem::path file );

	private:
		private_p * _p = nullptr;
	};
}