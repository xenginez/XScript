#pragma once

#include <map>

#include "type.h"

namespace x
{
	class library
	{
	public:
		bool is_open() const;
		bool open( x::static_string_view libname );
		void close();
		void * get_proc_address( x::static_string_view name );

	private:
		void * _lib = nullptr;
		x::static_string_view _libname;
		std::map<x::static_string_view, void * > _processes;
	};
}