import("std")

public namespace test
{
	public using func_type = const ( int ) -> int;

	public func static test_func( arg: func_type ) -> int
	{
		return arg( 1 );
	};
	
	public func static test_func2()
	{
		var b: int = 0;

		test_func( func [b]( i: int ) -> int
		{
			var a: int = i + b;
		});
	};

	public class test_class
	{
		public var b: int = 0;

		public func test_func3()
		{
			// test_func( func [b, this]( i: int ) -> int
			// {
			// 	var a: int = i + b;
			// });
		}
	};

}
