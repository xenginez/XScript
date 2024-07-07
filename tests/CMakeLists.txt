# ÃÌº”≤‚ ‘”√¿˝
add_executable(awaitable_test awaitable_test.cpp ../src/xlib.cpp ../src/allocator.cpp ../src/coroutine.cpp)
set_property(TARGET awaitable_test PROPERTY CXX_STANDARD 20)

# ÃÌº”≤‚ ‘
add_test(NAME awaitable_test COMMAND awaitable_test)
