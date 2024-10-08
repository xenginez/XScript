namespace std
{
	public template regex< StringType >
	{
		public using string_type = StringType;
		public using encode_type = string_type.encode_type;
		public using size_type = encode_type.size_type;

		public construct()
		{
			compile();
		}
		
		public func match( str: string_type, offset: size_type = 0 ) -> std::tuple< size_type, size_type >;

		private func compile() -> void;

		private var _pattern: string_type = string_type.from_utf8( "" );
	}
	
	public template iterator< Type > where( builtin_is_template_of( Type, std.regex ) )
	{
		public using regex_type = Type;
		public using size_type = regex_type.size_type;
		public using string_type = regex_type.string_type;
		public using value_type = std::tuple< size_type, size_type >;

		public func static generate( pattern: regex_type, str: string_type ) -> std.generator< value_type >
		{
			var result: value_type = { 0, 0 };

			while( result[0] != string_type.npos )
			{
				result = pattern.match( str, result[1] );
				yield result;
			}
		}
	}
}