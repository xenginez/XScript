#pragma once

#include "type.h"
#include "url.hpp"

namespace x
{
	class json;

	class language_server
	{
	private:
		struct private_p;

	public:
		language_server();
		~language_server();

	public:
		void cancelRequest( const x::json & params );
		void progress( const x::json & params );

	public:


	private:
		private_p * _p;
	};
}