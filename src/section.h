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
			FLAG,
			CLASS,
		};

	public:
		struct item
		{
			flag_t flag;
			x::uint64 size;
			x::static_string_view name;
		};

	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::vector<item> items;
	};

	class desc_section : public section
	{
	public:
		struct item
		{
			x::uint64 type; // type section index
			int array = 0;
			bool is_ref = false;
			bool is_const = false;
		};

	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::vector<item> items;
	};

	class temp_section : public section
	{
	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::vector<std::string> items;
	};

	class depend_section : public section
	{
	public:
		x::section_t type() const override;
		void load( std::istream & in ) override;
		void save( std::ostream & out ) const override;

	public:
		std::vector<x::static_string_view> items;
	};

	class global_section : public section
	{
	public:
		struct item
		{
			x::uint64 type; // desc section index
			bool is_thread = false;
			x::static_string_view name;
			union
			{
				struct
				{
					x::int64 enum_init;
				};
				struct
				{
					x::uint64 flag_init;
				};
				struct
				{
					x::uint32 class_init_idx;
					x::uint32 class_init_size;
				};
			};
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
			bool is_const = false;
			bool is_async = false;
			bool is_static = false;
			x::uint64 owner; // type section index
			x::uint32 code_data; // codedata section index
			x::uint32 code_size;
			x::uint64 result; // desc section index
			std::vector<x::uint64> parameters; // desc section index
			x::static_string_view name;
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
			x::uint64 owner; // type section index
			x::uint64 value; // desc section index
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
		std::vector<x::byte> codedatas;
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

	class section_module
	{
		type_section type;
		desc_section desc;
		temp_section temp;
		depend_section depend;
		global_section global;
		function_section function;
		variable_section variable;
		codedata_section codedata;
		stringdata_section stringdata;
		customdata_section customdata;
	};
}
