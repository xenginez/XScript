namespace std
{
	public template bool_constant< Value: bool >
	{
		public static const value: bool = Value;
	}
	public using true_type = bool_constant< true >;
	public using false_type = bool_constant< false >;
	
	public template enable_if< Value: bool = true, Type >
	{
		public using type = Type1;
	}
	public template enable_if< Value: bool = false, Type >
	{
	}

	public template conditional< Value: bool = true, Type1, Type2 >
	{
		public using type = Type1;
	}
	public template conditional< Value: bool = false, Type1, Type2 >
	{
		public using type = Type2;
	}
	
	public template is_void< Type > : std.conditional< builtin_is_void( Type ), std.true_type, std.false_type >.type
	{
	}
	
	public template is_enum< Type > : std.conditional< builtin_is_enum( Type ), std.true_type, std.false_type >.type
	{
	}
	
	public template is_class< Type > : std.conditional< builtin_is_class( Type ), std.true_type, std.false_type >.type
	{
	}
	
	public template is_same< Type1, Type2 > : std.conditional< builtin_is_same( Type1, Type2 ), std.true_type, std.false_type >.type
	{
	}
	
	public template is_base_of< Type, BaseType > : std.conditional< builtin_is_base_of( Type, BaseType ), std.true_type, std.false_type >.type
	{
	}
	
	
	public template is_any_types_of< Value: bool = true, Type, First, Rest... >
	{
		public static const value: bool = Value;
	}
	public template is_any_types_of< Value: bool = false, Type, First, Next, Rest... >
	{
		public static const value: bool = is_any_types_of< std::is_same< Type, Next >.value, Type, Next, Rest >.value;
	}
	public template is_any_types_of< Type, First > : std.conditional< std.is_same< Type, First >.value, std.true_type, std.false_type >.type
	{
	}
	public template is_any_types_of< Type, First, Rest... > : is_any_types_of< std.is_same< Type, First >.value, Type, First, Rest >
	{
	}

	public template is_boolean< Type > : std.conditional< std.is_same< Type, bool >.value, std.true_type, std.false_type >.type
	{
	}
	public template is_integral< Type > : std.conditional< std.is_any_types_of< Type, int8, int16, int32, int64, uint8, uint16, uint32, uint64 >.value, std.true_type, std.false_type >.type
	{
	}
	public template is_floating< Type > : std.conditional< std.is_any_types_of< Type, float16, float32, float64 >.value, std.true_type, std.false_type >.type
	{
	}
	public template is_arithmetic< Type > : std.conditional< std.is_boolean< Type >.value || std.is_integral< Type >.value || std.is_floating< Type >.value, std.true_type, std.false_type >.type
	{
	}
}
