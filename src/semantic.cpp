#include "semantic.h"

#include "symbols.h"

namespace
{
	class analyzer
	{
	public:
		virtual void statement( x::symbols * symbols, x::stat_ast * ast ) const
		{
		}
		virtual void function( x::symbols * symbols, x::function_decl_ast * ast ) const
		{
		}
		virtual void operation( x::symbols * symbols, x::expr_stat_ast * ast ) const
		{
		}
		virtual void condition( x::symbols * symbols, x::function_decl_ast * ast ) const
		{
		}
	};
	class div_zero_checker;
	class type_match_checker;
	class control_flow_checker;
	class const_access_checker;
	class const_express_checker;
	class function_call_checker;
	class variable_uninit_checker;
	class variable_unused_checker;
}