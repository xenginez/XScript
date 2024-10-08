namespace std
{
	public template hashmap< KeyType, ValueType >
	{
		public using key_type = KeyType;
		public using value_type = ValueType;
		public using size_type = int64;
		
		public func clear() -> void;
		
		public func remove( key: key_type ) -> value_type;
		public func insert( key: key_type, val: value_type ) -> void;
		
		public func const empty() -> bool;
		public func const size() -> size_type;
		public func const contains( key: key_type ) -> bool;
		public func const find( key: key_type ) -> value_type;
		
		private var _size: size_type = 0;
	}
}