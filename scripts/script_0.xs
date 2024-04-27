// script_0.ts

namespace sample
{
    public class script_0
    {
        public var var_0: int32 = 0;
        private static var var_1: int32 = 1;

        public func func_0( arg_0: int32, arg_1: float32 ) -> int32
        {
            return arg_0 * arg_1 + var_0 + var_1;
        };
        public static func func_1( arg: int32 )
        {
            var_1 = add( valr_1, arg );
        }

        public extren("xxx.dll", "name", void(int32, int32)) func add( left: int32, right: int32 );
    }
}
