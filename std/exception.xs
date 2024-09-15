namespace std
{
	public class exception
	{
		private var _stacktrace: string = {};
		
		public func stacktrace() const -> string
		{
			return _stacktrace;
		}

		public func virtual what() const -> string;
	}
}