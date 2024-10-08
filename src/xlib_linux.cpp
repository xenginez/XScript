#ifdef __linux__

#include "xlib.h"

x_path x_path_app_path()
{
	char tmp[32];
	char buf[512];

	snprintf( tmp, 32, "/proc/%d/exe", getpid() );
	buf[std::min<int>( readlink( tmp, buf, len ), len - 1 )] = '\0';

	return x::allocator::salloc( buf );
}
x_path x_path_home_path()
{
	char homedir[512];
	snprintf( homedir, 512, "%s", getenv( "HOME" ) );
	return x::allocator::salloc( homedir );
}

#endif // __linux__