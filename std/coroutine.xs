namespace std
{
	private class coroutine : object
	{
		friend std.scheduler;
		
		public var static const STATUS_START: int32 = 0;
		public var static const STATUS_DONE: int32 = -1;
		public var static const STATUS_EXCEPT: int32 = -2;

		protected var _value: any = {};
		protected var _state: int32 = STATUS_START;
		protected var _owner: std.coroutine = null;
		
		public func const done() -> bool
		{
			return _state == STATUS_DONE;
		}

		public func const except() -> bool
		{
			return _state == STATUS_EXCEPT;
		}
		
		public func resume() -> std.coroutine
		{
			if( done() )
				return _owner;

			try
			{
				_state = run( _state );
			}
			catch( var e: std.exception )
			{
				_value = e;
				_state = STATUS_EXCEPT;
			}

			return this;
		}

		protected func virtual run( state: int32 ) -> int32;
	}
	
	public template future<__: void> : std.coroutine
	{
		public func wait() -> void
		{
			while( !done() )
				resume();
		}
	}

	public template future<Type> : std.future<void>
	{
		public func const result() -> Type
		{
			return _value as Type;
		}
	}
	
	public template generator<Type> : std.coroutine
	{
		public func const result() -> Type
		{
			return _value as Type;
		}
	}

	public template when_all<Type> : std.future<void>
	{
		private var _futures: std.future<Type>[] = {};

		public func const result() -> Type[]
		{
			var results: Type[] = {};

			foreach( it : _futures )
			{
				results.push_back(it.result());
			}

			return results;
		}
		
		protected func override run( state: int32 ) -> int32;
		{
			if( state <0 )
				return state;

			foreach( it : _futures )
			{
				if( !it.done() )
				{
					it.resume();

					return state + 1;
				}
			}

			return std.coroutine.STATUS_DONE;
		}
	}

	public class scheduler
	{
		public func sleep_for( millisec: int64 ) -> std.future<void>;
		public func sleep_until( time: std.time ) -> std.future<void>;

		public func transfer_main() -> std.future<void>;
		public func transfer_pool() -> std.future<void>;
		public func transfer_alone( name: string = "" ) -> std.future<void>;

		public func remove_alone( name: string ) -> void;
	}
}
