namespace std
{
	public class condition
	{
		public construct()
		{
			_ptr = x_condition_create();
		}
		public finalize()
		{
			x_condition_release( _ptr );
		}
		
		public func wait() -> void
		{
			x_condition_wait( _ptr );
		}

		public func wait_for( milliseconds: uint64 ) -> void
		{
			x_condition_wait_for( _ptr, milliseconds );
		}
		
		public func notify_one() -> void
		{
			x_condition_notify_one( _ptr );
		}

		public func notify_all() -> void
		{
			x_condition_notify_all( _ptr );
		}

		private var _ptr: intptr = null;
	}
}