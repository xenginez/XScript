#pragma once

#include <deque>

#include "ast.h"

namespace x
{
	class pass : public ast_visitor
	{
		friend class context;

	public:
		using ast_visitor::visit;

	private:
		void set_ctx( x::context * ctx );

	protected:
		context * context() const;
		symbols * symbols() const;

	protected:
		void push( std::string_view scope_name );
		void pop();
		std::string_view current_scope_name() const;

	private:
		x::context * _ctx;
		std::deque<std::string> _scopes;
	};

	class type_scanner_pass : public pass
	{
	public:
		using pass::visit;

	public:

	};

	class function_scanner_pass : public pass
	{
	public:
		using pass::visit;

	public:

	};

	class variable_scanner_pass : public pass
	{
	public:
		using pass::visit;

	public:

	};

	class scope_scanner_pass : public pass
	{
	public:
		using pass::visit;

	public:

	};

	class reference_solver_pass : public pass
	{
	public:
		using pass::visit;

	public:

	};

	class type_checker_pass : public pass
	{
	public:
		using pass::visit;

	public:

	};

	class semantic_validator_pass : public pass
	{
	public:
		using pass::visit;

	public:

	};

}
