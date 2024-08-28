#pragma once

#include "type.h"

namespace x
{
	class logger : public std::enable_shared_from_this<logger>
	{
	public:
		virtual void info( std::string_view msg, std::string_view file = {}, int line = 0, int col = 0 );
		virtual void error( std::string_view msg, std::string_view file = {}, int line = 0, int col = 0 );
		virtual void warning( std::string_view msg, std::string_view file = {}, int line = 0, int col = 0 );
	};
}