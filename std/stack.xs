namespace std
{
	public template stack<Type>
	{
		public using size_type = int64;
		public using value_type = Type;
		
		public func clear() -> void;
		public func push( val: value_type ) -> void;
		public func top() -> value_type;
		public func pop() -> value_type;
		
		public func const empty() -> bool;
		public func const size() -> size_type;

		private var _size: size_type = 0;
		private var _data: intptr = null;
	}
}