namespace std
{
	public template hash<Type>
	{
		public using hash_type = uint64;
		public using value_type = Type;

		public func static calc( val: value_type ) -> hash_type
		{
			return val.hashcode();
		}
	}

	public template iterator<Type> where( builtin_is_member_of( Type, size ) && builtin_is_member_of( Type, index_of ) )
	{
		public using container_type = Type;
		public using size_type = container_type.size_type;
		public using value_type = container_type.value_type;

		public func static generate( container: container_type ) -> std.generator<value_type>
		{
			for( var i: size_type = 0; i <container.size(); ++i )
			{
				yield container.index_of( i );
			}
		}
	}
	public template iterator<Type, SizeType: Type.size_type> where( builtin_is_member_of( Type, size ) && builtin_is_member_of( Type, index_of ) )
	{
		public using container_type = Type;
		public using size_type = container_type.size_type;
		public using value_type = container_type.value_type;

		public func static generate( container: container_type ) -> std.generator<std::tuple<size_type, value_type>>
		{
			for( var i: size_type = 0; i <container.size(); ++i )
			{
				yield { i, container.index_of( i ) };
			}
		}
	}
	
	public template reverse_iterator<Type> where( builtin_is_member_of( Type, size ) && builtin_is_member_of( Type, index_of ) )
	{
		public using container_type = Type;
		public using size_type = container_type.size_type;
		public using value_type = container_type.value_type;

		public func static generate( container: container_type ) -> std.generator<value_type>
		{
			for( var i: size_type = container.size() - 1; i>= 0; --i )
			{
				yield container.index_of( i );
			}
		}
	}
	public template reverse_iterator<Type, SizeType: Type.size_type> where( builtin_is_member_of( Type, size ) && builtin_is_member_of( Type, index_of ) )
	{
		public using container_type = Type;
		public using size_type = container_type.size_type;
		public using value_type = container_type.value_type;

		public func static generate( container: container_type ) -> std.generator<std::tuple<size_type, value_type>>
		{
			for( var i: size_type = container.size() - 1; i>= 0; --i )
			{
				yield { i, container.index_of( i ) };
			}
		}
	}
}