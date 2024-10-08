namespace std
{
	public template iconv<FromString, ToString>
	{
		public using to_string_type = ToString;
		public using from_string_type = FromString;

		private construct()
		{
			_ptr = x_iconv_create();
		}
		private finalize()
		{
			x_iconv_release( _ptr );
		}
		
		public func static const default_codepage() -> string
		{
			return string.from_ptr( x_iconv_codepage() );
		}

		public func static const translate( from_codepage: string, to_codepage: string, fromstr: from_string_type, fromlen: uint64 = 0xFFFFFFFFFFFFFFFF ) -> to_string_type
		{
			var local ic: iconv = { ._to = to_codepage, ._from = from_codepage };
			var local buf: x.buffer = {};

			x_iconv_translate( _ptr, fromstr, fromlen, buf );

			return to_string_type.from_buffer( buf );
		}

		private var _to: string = "";
		private var _from: string = "";
		private var _ptr: intptr = null;
	}
}