#ifndef __X_LIB_H__
#define __X_LIB_H__

#if WIN32
#   define DLL_IMPORT
#   define DLL_EXPORT __declspec( dllexport )
#else
#   define DLL_IMPORT
#   define DLL_EXPORT __attribute__( ( visibility( "default" ) ) )
#endif

#ifdef X_EXPORT
#	define X_API DLL_EXPORT
#else
#	define X_API DLL_IMPORT
#endif // X_EXPORT

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

	typedef void * x_value;
	
	// math
	// file
	// time
	// config
	// window
	// vulkan
	// thread
	// network

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __X_LIB_H__
