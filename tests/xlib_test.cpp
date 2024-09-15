#include <iostream>

#include <xlib.h>

template<typename T> class xlib_stacktrace_test
{
public:
	static void func()
	{
		std::cout << x_os_stacktrace() << std::endl;
	}
};

void func1()
{
	xlib_stacktrace_test<int>::func();
}
void func2()
{
	func1();
}
void func3()
{
	func2();
}

int main()
{
	func3();

	return 0;
}
