namespace std
{
	public class path
	{
		public func static app_path() -> path
		{
			return { ._path = string.from_ptr( x_path_app_path() ) };
		}
		public func static temp_path() -> path
		{
			return { ._path = string.from_ptr( x_path_temp_path() ) };
		}
		public func static home_path() -> path
		{
			return { ._path = string.from_ptr( x_path_home_path() ) };
		}

		public func static copy( frompath: path, topath: path, recursive: bool = false, overwrite: bool = false ) -> void
		{
			x_path_copy( frompath.to_string(), topath.to_string(), recursive, overwrite );
		}

		public func create( recursive: bool ) -> void
		{
			x_path_create( _path, recursive );
		}
		public func remove() -> void
		{
			x_path_remove( _path );
		}

		public func const exists() -> bool
		{
			return x_path_exists( _path );
		}
		public func const is_file() -> bool
		{
			return x_path_is_file( _path );
		}
		public func const is_directory() -> bool
		{
			return x_path_is_directory( _path );
		}

		public func const override to_string() -> string
		{
			return _path;
		}

		private var _path: string = "";
	}

	public template iterator< Type: std.path >
	{
		public using size_type = uint64;
		public using value_type = std.path;

		public func static generate( _path: std.path ) -> std.generator< value_type >
		{
			var str: string = _path.to_string();
			var count: uint64 = x_path_entry_count( str );

			for( var i: uint64 = 0; i < count; ++i )
			{
				yield { ._path = string.from_ptr( x_path_at_entry( str, i ) ) };
			}
		}
	}
	
	public template recursive_iterator< Type: std.path >
	{
		public using size_type = uint64;
		public using value_type = std.path;

		public func static generate( _path: std.path ) -> std.generator< value_type >
		{
			var str: string = _path.to_string();
			var count: uint64 = x_path_entry_count( str );

			for( var i: uint64 = 0; i < count; ++i )
			{
				var p: std.path = { ._path = string.from_ptr( x_path_at_entry( str, i ) ) };

				yield p;

				yield recursive_iterator< std.path >.generate( p );
			}
		}
	}
}