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

    typedef unsigned char byte;
    typedef char int8;
    typedef short int16;
    typedef int int32;
    typedef long long int64;
    typedef unsigned char uint8;
    typedef unsigned short uint16;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;
    typedef unsigned short float16;
    typedef float float32;
    typedef double float64;
    typedef void * intptr;
    typedef const char * strptr;

    typedef intptr x_zip;
    typedef strptr x_path;
    typedef intptr x_file;
    typedef intptr x_lock;
    typedef intptr x_font;
    typedef intptr x_image;
    typedef intptr x_iconv;
    typedef strptr x_string;
    typedef intptr x_atomic;
    typedef intptr x_buffer;
    typedef intptr x_window;
    typedef intptr x_socket;
    typedef intptr x_uchardet;
    typedef intptr x_condition;
    typedef intptr x_coroutine;

    typedef enum
    {
        INPUT_NONE,
        INPUT_KEY_TAB,
        INPUT_KEY_LEFT_ARROW,
        INPUT_KEY_RIGHT_ARROW,
        INPUT_KEY_UP_ARROW,
        INPUT_KEY_DOWN_ARROW,
        INPUT_KEY_PAGE_UP,
        INPUT_KEY_PAGE_DOWN,
        INPUT_KEY_HOME,
        INPUT_KEY_END,
        INPUT_KEY_INSERT,
        INPUT_KEY_DELETE,
        INPUT_KEY_BACKSPACE,
        INPUT_KEY_SPACE,
        INPUT_KEY_ENTER,
        INPUT_KEY_ESC,
        INPUT_KEY_LEFT_CTRL,
        INPUT_KEY_LEFT_SHIFT,
        INPUT_KEY_LEFT_ALT,
        INPUT_KEY_LEFT_SUPER,
        INPUT_KEY_RIGHT_CTRL,
        INPUT_KEY_RIGHT_SHIFT,
        INPUT_KEY_RIGHT_ALT,
        INPUT_KEY_RIGHT_SUPER,
        INPUT_KEY_MENU,
        INPUT_KEY_0,
        INPUT_KEY_1,
        INPUT_KEY_2,
        INPUT_KEY_3,
        INPUT_KEY_4,
        INPUT_KEY_5,
        INPUT_KEY_6,
        INPUT_KEY_7,
        INPUT_KEY_8,
        INPUT_KEY_9,
        INPUT_KEY_A,
        INPUT_KEY_B,
        INPUT_KEY_C,
        INPUT_KEY_D,
        INPUT_KEY_E,
        INPUT_KEY_F,
        INPUT_KEY_G,
        INPUT_KEY_H,
        INPUT_KEY_I,
        INPUT_KEY_J,
        INPUT_KEY_K,
        INPUT_KEY_L,
        INPUT_KEY_M,
        INPUT_KEY_N,
        INPUT_KEY_O,
        INPUT_KEY_P,
        INPUT_KEY_Q,
        INPUT_KEY_R,
        INPUT_KEY_S,
        INPUT_KEY_T,
        INPUT_KEY_U,
        INPUT_KEY_V,
        INPUT_KEY_W,
        INPUT_KEY_X,
        INPUT_KEY_Y,
        INPUT_KEY_Z,
        INPUT_KEY_F1,
        INPUT_KEY_F2,
        INPUT_KEY_F3,
        INPUT_KEY_F4,
        INPUT_KEY_F5,
        INPUT_KEY_F6,
        INPUT_KEY_F7,
        INPUT_KEY_F8,
        INPUT_KEY_F9,
        INPUT_KEY_F10,
        INPUT_KEY_F11,
        INPUT_KEY_F12,
        INPUT_KEY_APOSTROPHE,               // ' SHIFT( " )
        INPUT_KEY_COMMA,                    // , SHIFT( < )
        INPUT_KEY_MINUS,                    // - SHIFT( _ )
        INPUT_KEY_PERIOD,					// . SHIFT( > )
        INPUT_KEY_SLASH,					// / SHIFT( ? )
        INPUT_KEY_SEMICOLON,				// ; SHIFT( : )
        INPUT_KEY_EQUAL,					// = SHIFT( + )
        INPUT_KEY_LEFT_BRACKET,             // [ SHIFT( { )
        INPUT_KEY_RIGHT_BRACKET,			// ] SHIFT( } )
        INPUT_KEY_BACKSLASH,				// \ SHIFT( | )
        INPUT_KEY_GRAVE_ACCENT,             // ` SHIFT( ~ )
        INPUT_KEY_CAPS_LOCK,
        INPUT_KEY_SCROLL_LOCK,
        INPUT_KEY_NUM_LOCK,
        INPUT_KEY_PRINT_SCREEN,
        INPUT_KEY_PAUSE,
        INPUT_KEY_PAD_0,
        INPUT_KEY_PAD_1,
        INPUT_KEY_PAD_2,
        INPUT_KEY_PAD_3,
        INPUT_KEY_PAD_4,
        INPUT_KEY_PAD_5,
        INPUT_KEY_PAD_6,
        INPUT_KEY_PAD_7,
        INPUT_KEY_PAD_8,
        INPUT_KEY_PAD_9,
        INPUT_KEY_PAD_DEC,                  // .
        INPUT_KEY_PAD_DIV,                  // /
        INPUT_KEY_PAD_MUL,                  // *
        INPUT_KEY_PAD_SUB,                  // -
        INPUT_KEY_PAD_ADD,                  // +
        INPUT_KEY_PAD_ENTER,
        INPUT_KEY_PAD_EQUAL,
        INPUT_MOUSE_LEFT,
        INPUT_MOUSE_RIGHT,
        INPUT_MOUSE_MIDDLE,
        INPUT_MOUSE_WHEEL,
        INPUT_MOUSE_MOVE,
        INPUT_MOUSE_X1,
        INPUT_MOUSE_X2,
        INPUT_MOUSE_X3,
        INPUT_MOUSE_X4,
        INPUT_MOUSE_X5,
        INPUT_GAMEPAD_START,				// Menu (Xbox)   + (Switch)   Start (PS)
        INPUT_GAMEPAD_BACK,                 // View (Xbox)   - (Switch)   Share (PS)
        INPUT_GAMEPAD_FACE_LEFT,			// X (Xbox)      Y (Switch)   Square (PS)
        INPUT_GAMEPAD_FACE_RIGHT,			// B (Xbox)      A (Switch)   Circle (PS)
        INPUT_GAMEPAD_FACE_UP,              // Y (Xbox)      X (Switch)   Triangle (PS)
        INPUT_GAMEPAD_FACE_DOWN,			// A (Xbox)      B (Switch)   Cross (PS)
        INPUT_GAMEPAD_DPAD_LEFT,			// D-pad Left
        INPUT_GAMEPAD_DPAD_RIGHT,			// D-pad Right
        INPUT_GAMEPAD_DPAD_UP,              // D-pad Up
        INPUT_GAMEPAD_DPAD_DOWN,			// D-pad Down
        INPUT_GAMEPAD_L1,					// L (Xbox)   L  (Switch)  L1 (PS)
        INPUT_GAMEPAD_R1,					// R (Xbox)   R  (Switch)  R1 (PS)
        INPUT_GAMEPAD_L2,					// L (Xbox)   ZL (Switch)  L2 (PS)
        INPUT_GAMEPAD_R2,					// R (Xbox)   ZR (Switch)  R2 (PS)
        INPUT_GAMEPAD_L3,					// L (Xbox)   L3 (Switch)  L3 (PS)
        INPUT_GAMEPAD_R3,					// R (Xbox)   R3 (Switch)  R3 (PS)
        INPUT_GAMEPAD_L_JOYSTICK,
        INPUT_GAMEPAD_R_JOYSTICK,
        INPUT_TOUCH_F0,
        INPUT_TOUCH_F1,
        INPUT_TOUCH_F2,
        INPUT_TOUCH_F3,
        INPUT_TOUCH_F4,
        INPUT_TOUCH_F5,
        INPUT_TOUCH_F6,
        INPUT_TOUCH_F7,
        INPUT_TOUCH_F8,
        INPUT_TOUCH_F9,
        INPUT_TOUCH_COUNT,
        INPUT_UNICODE_CHAR,
        INPUT_MAX_ELEMENT_COUNT,
    } x_input;

    typedef struct { int32 x; int32 y; } x_pos;
    typedef struct { int32 w; int32 h; } x_size;
    typedef struct { float32 x; float32 y; } x_offset;
    typedef struct { x_pos pos; x_size size; } x_rect;
    typedef union { int32 unicode; uint32 ctl; x_pos pos; x_offset off; } x_inputval;


#define SEEK_POS_BEG                        uint32( 1 )
#define SEEK_POS_CUR                        uint32( 2 )
#define SEEK_POS_END                        uint32( 3 )

#define FILE_MODE_READ                      uint32( 0x01 )
#define FILE_MODE_WRITE                     uint32( 0x02 )
#define FILE_MODE_APPAND                    uint32( 0x08 )
#define FILE_MODE_TRUNCATE                  uint32( 0x10 )

#define WINDOW_STATE_NORMAL                 uint32( 1 )
#define WINDOW_STATE_HIDDEN                 uint32( 2 )
#define WINDOW_STATE_MINIMIZE               uint32( 3 )
#define WINDOW_STATE_MAXIMIZE               uint32( 4 )
#define WINDOW_STATE_FULLSCREEN             uint32( 5 )

#define SOCKET_AF_INET                      uint32( 1 )
#define SOCKET_AF_INET6                     uint32( 2 )

#define SOCKET_PROTOCOL_UDP                 uint32( 1 )
#define SOCKET_PROTOCOL_TCP                 uint32( 2 )
#define SOCKET_PROTOCOL_ICMP                uint32( 3 )

    // os
    int x_main( int argc, const char ** argv );
    uint8 x_os_arch();
    x_string x_os_name();

    // zip
    x_zip x_zip_create();
    void x_zip_load( x_zip zip, x_path path );
    void x_zip_save( x_zip zip, x_path path );
    bool x_zip_exist( x_zip zip, x_string name );
    uint64 x_zip_file_size( x_zip zip, x_string name );
    uint64 x_zip_compress_size( x_zip zip, x_string name );
    void x_zip_extract( x_zip zip, x_string name, x_path path );
    void x_zip_extract_all( x_zip zip, x_path path );
    uint64 x_zip_read( x_zip zip, x_string name, x_buffer buf );
    void x_zip_write( x_zip zip, x_string name, x_buffer buf );
    void x_zip_write_str( x_zip zip, x_string name, x_string str );
    void x_zip_release( x_zip zip );
    void x_zip_compression( x_buffer inbuf, x_buffer outbuf );
    void x_zip_decompression( x_buffer inbuf, x_buffer outbuf );

    // path
    x_path x_path_app_path();
    x_path x_path_temp_path();
    x_path x_path_home_path();
    void x_path_copy( x_path frompath, x_path topath, bool recursive, bool overwrite );
    void x_path_create( x_path path, bool recursive );
    void x_path_remove( x_path path );
    bool x_path_exists( x_path path );
    bool x_path_is_file( x_path path );
    bool x_path_is_directory( x_path path );
    uint64 x_path_entry_count( x_path path );
    x_path x_path_at_entry( x_path path, uint64 idx );

	// file
    x_file x_file_create();
    bool x_file_open( x_file file, x_path path, uint32 mode );
    uint64 x_file_size( x_file file );
    x_string x_file_name( x_file file );
    int64 x_file_time( x_file file );
    void x_file_rename( x_file file, x_string name );
    bool x_file_is_eof( x_file file );
    uint64 x_file_read_tell( x_file file );
    uint64 x_file_write_tell( x_file file );
    void x_file_read_seek( x_file file, int32 off, uint32 pos );
    void x_file_write_seek( x_file file, int32 off, uint32 pos );
    uint64 x_file_read( x_file file, intptr buffer, uint64 size );
    uint64 x_file_write( x_file file, intptr buffer, uint64 size );
    void x_file_close( x_file file );
    void x_file_release( x_file file );

	// time
    int64 x_time_now();
    void x_time_sleep_for( int64 milliseconds );
    void x_time_sleep_until( int64 time );
    int32 x_time_year( int64 time );
    int32 x_time_month( int64 time );
    int32 x_time_day( int64 time );
    int32 x_time_weekday( int64 time );
    int32 x_time_hour( int64 time );
    int32 x_time_minute( int64 time );
    int32 x_time_second( int64 time );
    int32 x_time_millisecond( int64 time );
    int64 x_time_add_year( int64 time, int32 year );
    int64 x_time_add_month( int64 time, int32 month );
    int64 x_time_add_day( int64 time, int32 day );
    int64 x_time_add_hour( int64 time, int32 hour );
    int64 x_time_add_minute( int64 time, int32 minute );
    int64 x_time_add_second( int64 time, int32 seconds );
    int64 x_time_add_millisecond( int64 time, int32 milliseconds );
    int64 x_time_second_clock( int64 lefttime, int64 righttime );
    int64 x_time_millisecond_clock( int64 lefttime, int64 righttime );
    int64 x_time_to_utc( int64 time );
    int64 x_time_from_utc( int64 time );
    x_string x_time_to_string( int64 time, x_string fmt );
    int64 x_time_from_string( x_string str, x_string fmt );

    // lock
    x_lock x_lock_create();
    void x_lock_unique_lock( x_lock lock );
    void x_lock_unique_unlock( x_lock lock );
    bool x_lock_unique_trylock( x_lock lock );
    void x_lock_shared_lock( x_lock lock );
    void x_lock_shared_unlock( x_lock lock );
    bool x_lock_shared_trylock( x_lock lock );
    void x_lock_release( x_lock lock );

    // font
    x_font x_font_create();

    // image
    x_image x_image_create();

    // iconv
    x_iconv x_iconv_create( x_string fromcode, x_string tocode );
    uint64 x_iconv_iconv( x_iconv iconv, x_string inbuf, uint64 inbytes, x_buffer outbuf );
    void x_iconv_release( x_iconv iconv );

    // atomic
    x_atomic x_atomic_create();
    bool x_atomic_compare_exchange( x_atomic atomic, intptr exp, intptr val );
    intptr x_atomic_exchange( x_atomic atomic, intptr val );
    intptr x_atomic_load( x_atomic atomic );
    void x_atomic_store( x_atomic atomic, intptr val );
    void x_atomic_release( x_atomic atomic );

    // window
    x_window x_window_create();
    float32 x_window_dpi_scale( x_window window );
    void x_window_set_parent( x_window window, x_window parent );
    x_window x_window_get_parent( x_window window );
    x_string x_window_get_title( x_window window );
    void x_window_set_title( x_window window, x_string title );
    x_pos x_window_get_pos( x_window window );
    void x_window_set_pos( x_window window, x_pos pos );
    x_size x_window_get_size( x_window window );
    void x_window_set_size( x_window window, x_size size );
    x_rect x_window_get_rect( x_window window );
    void x_window_set_rect( x_window window, x_rect rect );
    void x_window_show( x_window window );
    void x_window_hide( x_window window );
    void x_window_show_minimize( x_window window );
    void x_window_show_maximize( x_window window );
    void x_window_show_fullscreen( x_window window, bool monitor );
    bool x_window_is_hidden( x_window window );
    int32 x_window_get_show_state( x_window window );
    void x_window_mouse_show( x_window window );
    void x_window_mouse_hide( x_window window );
    bool x_window_mouse_is_hidden( x_window window );
    x_inputval x_window_input( x_window window, x_input key );
    void x_window_close( x_window window );
    void x_window_release( x_window window );

	// socket
    x_buffer x_socket_getaddrinfo( uint32 protocol, x_string name, x_string service );
    x_socket x_socket_create( uint32 protocol, uint32 family );
    int32 x_socket_bind( x_socket socket, x_string sockname, uint16 port );
    int32 x_socket_listen( x_socket socket );
    x_socket x_socket_accept( x_socket socket );
    int32 x_socket_connect( x_socket socket, x_string peername, uint16 port );
    uint64 x_socket_recv( x_socket socket, intptr buffer, uint64 size );
    uint64 x_socket_send( x_socket socket, intptr buffer, uint64 size );
    uint64 x_socket_sendto( x_socket socket, x_string peername, uint16 port, intptr buffer, uint64 size );
    x_string x_socket_getsockname( x_socket socket );
    uint16 x_socket_getsockport( x_socket socket );
    x_string x_socket_getpeername( x_socket socket );
    uint16 x_socket_getpeerport( x_socket socket );
    x_string x_socket_getsockopt( x_socket socket, int32 key );
    int32 x_socket_setsockopt( x_socket socket, int32 key, x_string value );
    void x_socket_close( x_socket socket );
    void x_socket_release( x_socket socket );

    // uchardet
    x_uchardet x_uchardet_create();
    int32 x_uchardet_handle_data( x_uchardet uchardet, x_string data, uint32 len );
    void x_uchardet_data_end( x_uchardet uchardet );
    x_string x_uchardet_get_charset( x_uchardet uchardet );
    void x_uchardet_release( x_uchardet uchardet );

    // condition
    x_condition x_condition_create();
    void x_condition_wait( x_condition cond );
    void x_condition_wait_for( x_condition cond, uint64 milliseconds );
    void x_condition_notify_one( x_condition cond );
    void x_condition_notify_all( x_condition cond );
    void x_condition_release( x_condition cond );

    // coroutine
    x_coroutine x_coroutine_create( uint64 size );
    void x_coroutine_sleep_until( x_coroutine coroutine, int64 time );
    void x_coroutine_file_read( x_coroutine coroutine, x_file file, intptr buffer, uint64 size );
    void x_coroutine_file_write( x_coroutine coroutine, x_file file, intptr buffer, uint64 size );
    void x_coroutine_socket_accept( x_coroutine coroutine, x_socket socket );
    void x_coroutine_socket_connect( x_coroutine coroutine, x_socket socket, x_string peername, uint16 port );
    void x_coroutine_socket_recv( x_coroutine coroutine, x_socket socket, intptr buffer, uint64 size );
    void x_coroutine_socket_send( x_coroutine coroutine, x_socket socket, intptr buffer, uint64 size );
    void x_coroutine_socket_sendto( x_coroutine coroutine, x_socket socket, x_string peername, uint16 port, intptr buffer, uint64 size );
    void x_coroutine_release( x_coroutine coroutine );
    
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __X_LIB_H__
