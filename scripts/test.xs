import("std")

attribute(desc=int32,desc2=avav 123)
namespace test
{
	public using type1 = int32;
	public using type2 = int32(int32);
	public using type3 = int32(int32, int32);
	public using type4 = int32<int32>;
	public using type5 = int32<int32,int32>;
	private using type6 = int32[];
	protected using type7 = int32[,];

	public enum enum_test
	{
		enum_test1,
		enum_test2 = 1,
		enum_test3 = 3,
		enum_test4 = enum_test | enum_test3,
	};

	public template template_test<T, Args...> : object where
	{
		
	}
	{
		
	};

	public class class_test
	{
		public var variable1;
		public var variable2 = 0;
		public var variable3: int32;
		public var variable4: int32 = 0;
		public var variable5: int32 = {};
		public var variable6: int32 = {0};
		public var variable7: int32[] = {0};
		public var local variable8: int32 = {0};
		public var static variable9: int32 = {0};
		public var thread variable10: int32 = {0};

		public func function1()
		{
		};
		public func function2(arg)
		{
		};
		public func function3(arg: int32)
		{
		};
		public func function4(arg: int32)->int32
		{
		};
		public func function5(arg1: int32, arg2: int32)->int32
		{
		};
		public func function6(arg1: int32, arg2: ref int32)->int32
		{
		};
		public func function7(arg1: int32, arg2: ref const int32)->int32
		{
		};
		public func const function8(arg1: int32, arg2: ref const int32)->int32
		{
		};
		public func async function9(arg1: int32, arg2: ref const int32)->int32
		{
			{
				await function2(1, 2);
				yield 1;
				new int32{};
				if ( false )
				{
				}
				else
				{
				}
				while ( true )
				{
				}
				for ( var i:int32 = 0; i < 10; ++i )
				{
					if( i == 5 )
						break;
					if( i == 6 )
						return;
					if( i == 7 )
						continue;
				}
				foreach ( arg : vec )
				{
				}
				switch( 10 )
				{
					case 1:
					{
					}
					case 2:
					{
					}
					case 3:
					{
					}
					case 4:
					{
					}
					default:
					{
					}
				}
				
				var local0: int32 = {};
				var local local1: int32 = {};
				
				local0 = local1;
				local0 += local1;
				local0 -= local1;
				local0 *= local1;
				local0 /= local1;
				local0 %= local1;
				local0 ^= local1;
				local0 &= local1;
				local0 |= local1;
				local0 <<= local1;
				local0 >>= local1;
				local0 == local1;
				local0 != local1;
				local0 <= local1;
				local0 >= local1;
				local0 || local1;
				local0 && local1;
				local0 + local1;
				local0 - local1;
				local0 * local1;
				local0 / local1;
				local0 % local1;
				local0 | local1;
				local0 & local1;
				local0 ^ local1;
				local0 << local1;
				local0 >> local1;
				local0++;
				++local1;
				local0--;
				--local1;
				!local0;
				~local0;
				
				sizeof( local0 );
				typeof( local0 );
				local0 as int32;
				local0 is int32;

				local0 = this.variable7[0];
				this.function10( 1, local0 );
				local0 = null;
				local0 = false;
				local0 = true;
				local0 = 0 * ( 1 + 2 ) / 1;
				local0 = 123;
				local0 = 0xabc;
				local0 = 0b1001001;
				local0 = 123.456;
				local0 = +123;
				local0 = +0xabc;
				local0 = +0b1001001;
				local0 = +123.456;
				local0 = -123;
				local0 = -0xabc;
				local0 = -0b1001001;
				local0 = -123.456;
				local0 = "fdjh\tskg";
				local0 = R"(println("hello")
				;)";
			}
		};
		public func final function10(arg1: int32, arg2: ref const int32)->int32
		{
		};
		public func static function11(arg1: int32, arg2: ref const int32)->int32
		{
		};
		public func virtual function12(arg1: int32, arg2: ref const int32)->int32
		{
		};
		public func static function13(arg1: int32, arg2: ref const int32)->int32 = extern("dllname.dll", "addfunc");
	};
}
