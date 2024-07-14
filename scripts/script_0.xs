// script_0.ts

import("std")

namespace sample
{
    enum enum_0
    {
        enum1 = 1,
        enum2,
        enum3 = enum1 + 2,
    };

    attribute(desc=int32,desc2=avav 123)
    public class script_0
    {
        public using int = int32;

        public var var_0: int32 = 0;
        private var static var_1: int32[] = { 0, 1, 2, 3 };

        public func func_0( arg_0: int32, arg_1: float32 ) -> int32
        {
            return arg_0 * arg_1 + var_0 + var_1[0];
        };
        public func static func_1( arg: int32 )
        {
            var_1[0] = add( var_1[1], arg );
            var i32: int32 = int32.parse( "123", 10 );
        };

        public func static add( left: int32, right: int32 ) -> int32 = extern( "dllname", "funcname" );
    }
}
