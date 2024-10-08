namespace std
{
	public class lock
	{
		public construct()
		{
			_ptr = x_lock_create();
		}
		public finalize()
		{
			x_lock_release( _ptr );
		}

		public func unique_lock() -> void
		{
			x_lock_unique_lock( _ptr );
		}
		public func unique_unlock() -> void
		{
			x_lock_unique_unlock( _ptr );
		}
		public func unique_trylock() -> bool
		{
			return x_lock_unique_trylock( _ptr );
		}

		public func shared_lock() -> void
		{
			x_lock_shared_lock( _ptr );
		}
		public func shared_unlock() -> void
		{
			x_lock_shared_unlock( _ptr );
		}
		public func shared_trylock() -> bool
		{
			return x_lock_shared_trylock( _ptr );
		}
		
		private var _ptr: intptr = null;
	}

	public class read_lock
	{
		public construct()
		{
			shared_lock();
		}
		public finalize()
		{
			shared_unlock();
		}
		
		private var _lock: std.lock = null;
	}
	
	public class write_lock
	{
		public construct()
		{
			unique_lock();
		}
		public finalize()
		{
			unique_unlock();
		}
		
		private var _lock: std.lock = null;
	}
}