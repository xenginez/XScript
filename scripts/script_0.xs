// script_0.ts

import "std"

namespace sample
{
    class script_0 {
        public var var_0 : int = 0;
        private static var var_1 = 1;

        public func func_0( arg_0 : int, arg_1 : float ) -> void {
            return arg_0 * arg_1 + var_0;
        };
        public static func func_1(arg: int) {
            var_1 = arg;
        }
    }
}
