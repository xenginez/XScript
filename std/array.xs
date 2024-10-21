namespace std
{
	public template array<Type>
	{
		public using size_type = int64;
		public using value_type = Type;
		
		public func clear() -> void;
		public func resize( count: size_type ) -> void;
		public func push_back( val: value_type ) -> void;
		public func pop_back() -> value_type;
		public func assign( values: value_type... ) -> void;
		
		public func remove( index: size_type ) -> value_type;
		public func insert( index: size_type, val: value_type ) -> void;
		public func remove_range( left: size_type, right: size_type ) -> void;
		public func insert_range( index: size_type, values: value_type... ) -> void;

		public func const index_of( index: size_type ) -> value_type;
		public func const front() -> value_type;
		public func const back() -> value_type;
		public func const data() -> intptr;

		public func const empty() -> bool;
		public func const size() -> size_type;
		public func const contains( val: value_type ) -> bool;

		private var _size: size_type = 0;
		private var _data: intptr = null;
	}
}