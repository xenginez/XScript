#pragma once

#include "type.h"

namespace x
{
	class section : public std::enable_shared_from_this<section>
	{
	public:
		virtual ~section() = default;

	public:
		virtual x::section_t type() const = 0;
		virtual void load( std::istream & in ) = 0;
		virtual void save( std::ostream & out ) const = 0;
	};

	class type_section : public section
	{
	public:
		enum flag_t
		{
			ENUM,
			CLASS,
			NAMESPACE,
		};

		struct item
		{
			flag_t flag;
			std::size_t initialize;
			x::static_string_view name;
		};

	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::vector<item> items;
	};

	class depend_section : public section
	{
	public:
		enum flag_t
		{
			ENUM,
			CLASS,
			FUNCTION,
			VARIABLE,
			NAMESPACE,
		};

		struct item
		{
			flag_t flag;
			x::static_string_view name;
		};

	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::vector<item> items;
	};

	class global_section : public section
	{
	public:
		struct item
		{
			std::size_t initialize;
			x::static_string_view name;
		};

	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::vector<item> items;
	};

	class function_section : public section
	{
	public:
		struct item
		{
			std::size_t owner; // type section index
			std::size_t codedata; // codedata section index
			x::static_string_view name;
			std::size_t result; // type section index
			std::vector<std::size_t> parameters; // type section index
		};

	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::vector<item> items;
	};

	class variable_section : public section
	{
	public:
		struct item
		{
			std::size_t owner; // type section index
			std::size_t value; // type section index
			x::static_string_view name;
		};

	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::vector<item> items;
	};

	class codedata_section : public section
	{
	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::vector<std::uint8_t> codedatas;
	};

	class stringdata_section : public section
	{
	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::string stringdatas;
	};

	class customdata_section : public section
	{
	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::string customdatas;
	};
}