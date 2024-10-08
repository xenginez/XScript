namespace std
{
	public class time
	{
		public func static now() -> std.time
		{
			return { ._time = x_time_now() };
		}
		
		public func const year() -> int32
		{
			return x_time_year( _time );
		}
		public func const month() -> int32
		{
			return x_time_month( _time );
		}
		public func const day() -> int32
		{
			return x_time_day( _time );
		}
		public func const weekday() -> int32
		{
			return x_time_weekday( _time );
		}
		public func const hour() -> int32
		{
			return x_time_hour( _time );
		}
		public func const minute() -> int32
		{
			return x_time_minute( _time );
		}
		public func const second() -> int32
		{
			return x_time_second( _time );
		}
		public func const millisecond() -> int32
		{
			return x_time_millisecond( _time );
		}
		
		public func add_year( val: int32 ) -> void
		{
			_time = x_time_add_year( _time, val );
		}
		public func add_month( val: int32 ) -> void
		{
			_time = x_time_add_month( _time, val );
		}
		public func add_day( val: int32 ) -> void
		{
			_time = x_time_add_day( _time, val );
		}
		public func add_hour( val: int32 ) -> void
		{
			_time = x_time_add_hour( _time, val );
		}
		public func add_minute( val: int32 ) -> void
		{
			_time = x_time_add_minute( _time, val );
		}
		public func add_second( val: int32 ) -> void
		{
			_time = x_time_add_second( _time, val );
		}
		public func add_millisecond( val: int32 ) -> void
		{
			_time = x_time_add_millisecond( _time, val );
		}
		
		public func const to_string( fmt: string ) -> string
		{
			return string.from_ptr( x_time_to_string( _time, fmt ) );
		}
		public func from_string( str: string, fmt: string ) -> void
		{
			_time = x_time_from_string( str, fmt );
		}

		public func const seconds( right: std.time ) -> int64
		{
			return x_time_second_clock( _time, right._time );
		}
		public func const milliseconds( righ: std.time ) -> int64
		{
			return x_time_millisecond_clock( _time, righttime._time );
		}

		public func virutal const is_utc() -> bool
		{
			return false;
		}

		public func static local_to_utc( t: std.time ) -> std.utc_time
		{
			return { ._time = x_time_to_utc( t._time ) };
		}
		public func static utc_to_local( t: std.utc_time ) -> std.time
		{
			return { ._time = x_time_from_utc( t._time ) };
		}


		private var _time: int64 = 0;
	}

	public class utc_time : std.time
	{
		public func override const is_utc() -> bool
		{
			return true;
		}
	}
}