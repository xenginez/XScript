namespace std
{
	public template priority_queue<Type>
	{
		public using size_type = int64;
		public using value_type = Type;
		
		public func clear() -> void;
		public func enqueue( val: value_type ) -> void;
		public func dequeue() -> value_type;
		public func try_dequeue( out: ref value_type ) -> bool;
		
		public func const data() -> intptr;

		public func const empty() -> bool;
		public func const size() -> size_type;
		public func const contains( val: value_type ) -> bool;

		private var _size: size_type = 0;
		private var _data: intptr = null;
	}
}