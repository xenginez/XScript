namespace std
{
	public class file
	{
		public var static const POS_BEG: uint32 = 1;
		public var static const POS_CUR: uint32 = 2;
		public var static const POS_END: uint32 = 3;

		public var static const MODE_READ: uint32 = 0x01;
		public var static const MODE_WRITE: uint32 = 0x02;
		public var static const MODE_APPAND: uint32 = 0x08;
		public var static const MODE_TRUNCATE: uint32 = 0x10;

		public construct()
		{
			_ptr = x_file_create();
		}
		public finalize()
		{
			x_file_release( _ptr );
		}
		
		public func open( path: string, mode: uint32 = MODE_READ | MODE_WRITE ) -> void
		{
			return x_file_open( _ptr, path, mode );
		}
		public func close() -> void
		{
			x_file_close( _ptr );
		}
		
		public func const eof() -> bool
		{
			return x_file_is_eof( _ptr );
		}
		public func const size() -> uint64
		{
			return x_file_size( _ptr );
		}
		public func const name() -> string
		{
			return string.from_ptr( x_file_name( _ptr ) );
		}
		public func const time() -> std.time
		{
			return x_file_time( _ptr );
		}

		public func rename( newname: string ) -> void
		{
			x_file_rename( _ptr, newname );
		}

		public func const tellg() -> uint64
		{
			return x_file_read_tell( _ptr );
		}
		public func const tellp() -> uint64
		{
			return x_file_write_tell( _ptr );
		}
		public func seekg( int32: offset, pos: uint32 = POS_CUR ) -> void
		{
			x_file_read_seek( _ptr, offset, pos );
		}
		public func seekp( int32: offset, pos: uint32 = POS_CUR ) -> void
		{
			x_file_write_seek( _ptr, offset, pos );
		}
		
		public func read( size: uint64 ) -> std.buffer;
		public func write( size: uint64, buf: std.buffer ) -> uint64;
		
		public func async_read( size: uint64 ) -> std.future< std.buffer >;
		public func async_write( size: uint64, buf: std.buffer ) -> std.future< uint64 >;

		private var _ptr: intptr = null;
	}
}