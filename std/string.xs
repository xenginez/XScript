namespace std
{
	public class ansi_encode
	{
		public using unit_type = int8;
		public using size_type = int64;
		public using code_type = int8;
		
		public func static advance() -> void;
		public func static distance() -> void;
		public func static insert() -> void;
		public func static walk() -> void;
		public func static num_of_bytes() -> void;
	}

	public class wide_encode
	{
		public using unit_type = int16;
		public using size_type = int64;
		public using code_type = int16;
	}

	public class utf8_encode
	{
		public using unit_type = int8;
		public using size_type = int64;
		public using code_type = uint64;
	}

	public class utf16_encode
	{
		public using unit_type = int16;
		public using size_type = int64;
		public using code_type = uint64;
	}

	public class utf32_encode
	{
		public using unit_type = int32;
		public using size_type = int64;
		public using code_type = uint64;
	}

	public template basic_string< EncodeType >
	{
		public using encode_type = EncodeType;
		public using string_type = basic_string< encode_type >;
		public using size_type = encode_type.size_type;
		public using code_type = encode_type.code_type;

		public var static const npos: size_type = -1;
		
		public func static from_ptr( ptr: intptr ) -> string_type;
		public func static from_buf( buf: x.buffer ) -> string_type;
		public func static from_ansi( str: basic_string< ansi_encode > ) -> string_type;
		public func static from_wide( str: basic_string< wide_encode > ) -> string_type;
		public func static from_utf8( str: basic_string< utf8_encode > ) -> string_type;
		public func static from_utf16( str: basic_string< utf16_encode > ) -> string_type;
		public func static from_utf32( str: basic_string< utf32_encode > ) -> string_type;
		
		public func const to_ansi() -> basic_string< ansi_encode >;
		public func const to_wide() -> basic_string< wide_encode >;
		public func const to_utf8() -> basic_string< utf8_encode >;
		public func const to_utf16() -> basic_string< utf16_encode >;
		public func const to_utf32() -> basic_string< utf32_encode >;
	}
	
	public using ansi_string = basic_string< ansi_encode >;
	public using wide_string = basic_string< wide_encode >;
	public using utf8_string = basic_string< utf8_encode >;
	public using utf16_string = basic_string< utf16_encode >;
	public using utf32_string = basic_string< utf32_encode >;
}