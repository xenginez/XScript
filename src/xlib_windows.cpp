#ifdef _WIN32

#include "xlib.h"

#include <array>
#include <deque>

#define NOMINMAX
#include <WinSock2.h>
#include <mswsock.h>
#include <Windows.h>
#include <ws2tcpip.h>

#include "allocator.h"
#include "coroutine.h"

#pragma comment(lib, "Ws2_32.lib")

namespace
{
    struct overlapped;
    struct file_info;
    struct window_info;
    struct socket_info;
    struct iocp_scheduler;
    struct timer_scheduler;

    struct overlapped
    {
        OVERLAPPED overlap;
        uint32 type = 0;
        std::coroutine_handle<> handle = nullptr;
        DWORD bytes = 0;
        DWORD flags = 0;
        WSABUF buffer = {};
        union
        {
            file_info * file;
            socket_info * socket;
            window_info * window;
        };
        union
        {
            struct
            {
                x::await_result<uint64> * result;
            } read;
            struct
            {
                x::await_result<uint64> * result;
            } write;
            struct
            {
                x::await_result<uint64> * result;
            } recv;
            struct
            {
                x::await_result<uint64> * result;
            } send;
            struct
            {
                int size;
            } sendto;
            struct
            {
                x::await_result<x_socket> * result;
                socket_info * client;
            } accept;
            struct
            {
                x::await_result<int32> * result;
            } connect;
        };
    };

    struct file_info
    {
        uint32 mode = 0;
        uint64 readoff = 0;
        uint64 writeoff = 0;
        std::filesystem::path path = {};
        HANDLE handle = INVALID_HANDLE_VALUE;
    };
	struct window_info
	{
		HWND hwnd = nullptr;
        x_rect rect = {};
		uint32 state = 0;
		bool cursor = true;
		window_info * parent = nullptr;
        std::array<x_inputval, x_input::INPUT_MAX_ELEMENT_COUNT> inputs;
	};
    struct socket_info
    {
        uint32 type = 0;
        SOCKET handle = 0;
        addrinfo info = {};
        sockaddr_in sockaddr = {};
        sockaddr_in peeraddr = {};
    };
    struct iocp_scheduler
    {
    public:
        iocp_scheduler()
        {
            ( void )::WSAStartup( MAKEWORD( 2, 2 ), &_wsadata );

            _iocp = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );

            DWORD bytes = 0;
            GUID AcceptExGuid = WSAID_ACCEPTEX;
            GUID ConnectExGuid = WSAID_CONNECTEX;
            GUID GetAcceptExSockaddrsGuid = WSAID_GETACCEPTEXSOCKADDRS;

            WSAIoctl( 0, SIO_GET_EXTENSION_FUNCTION_POINTER, &AcceptExGuid, sizeof( AcceptExGuid ), &_AcceptEx, sizeof( _AcceptEx ), &bytes, nullptr, nullptr );
            WSAIoctl( 0, SIO_GET_EXTENSION_FUNCTION_POINTER, &ConnectExGuid, sizeof( ConnectExGuid ), &_ConnectEx, sizeof( _ConnectEx ), &bytes, nullptr, nullptr );
            WSAIoctl( 0, SIO_GET_EXTENSION_FUNCTION_POINTER, &GetAcceptExSockaddrsGuid, sizeof( GetAcceptExSockaddrsGuid ), &_GetAcceptExSockaddrs, sizeof( _GetAcceptExSockaddrs ), &bytes, nullptr, nullptr );

            _thread = std::jthread( runtask );
        }
        ~iocp_scheduler()
        {
            _thread.request_stop();

            ::CloseHandle( _iocp );

            ::WSACleanup();
        }

    public:
        static iocp_scheduler * instance()
        {
            static iocp_scheduler io;
            return &io;
        }

    public:
        static HANDLE iocp_handle()
        {
            return instance()->_iocp;
        }
        static void enqueue( overlapped * lap )
        {
            instance()->_overlaps.push_back( lap );
        }
        static overlapped * dequeue()
        {
            auto lap = instance()->_overlaps.front();
            instance()->_overlaps.pop_front();
            return lap;
        }

    public:
        static BOOL AcceptEx( SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped )
        {
            return instance()->_AcceptEx( sListenSocket, sAcceptSocket, lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived, lpOverlapped );
        }
        static BOOL ConnectEx( SOCKET s, const struct sockaddr * name, int namelen, PVOID lpSendBuffer, DWORD dwSendDataLength, LPDWORD lpdwBytesSent, LPOVERLAPPED lpOverlapped )
        {
            return instance()->_ConnectEx( s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped );
        }
        static void GetAcceptExSockaddrs( PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, struct sockaddr ** LocalSockaddr, LPINT LocalSockaddrLength, struct sockaddr ** RemoteSockaddr, LPINT RemoteSockaddrLength )
        {
            instance()->_GetAcceptExSockaddrs( lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwReceiveDataLength, LocalSockaddr, LocalSockaddrLength, RemoteSockaddr, RemoteSockaddrLength );
        }

    private:
        static void runtask( std::stop_token token )
        {
            DWORD bytes = 0;
            ULONG_PTR key = 0;
            overlapped * overlap = nullptr;

            while ( !token.stop_requested() )
            {
                if ( ::GetQueuedCompletionStatus( iocp_scheduler::iocp_handle(), &bytes, &key, (LPOVERLAPPED *)&overlap, 10 ) )
                {
                    socket_info * client = nullptr;

                    switch ( overlap->type )
                    {
                    case SELECT_EVENT_READ:
                        overlap->read.result->resume( overlap->bytes );
                        break;
                    case SELECT_EVENT_WRITE:
                        overlap->write.result->resume( overlap->bytes );
                        break;
                    case SELECT_EVENT_RECV:
                        overlap->recv.result->resume( overlap->bytes );
                        break;
                    case SELECT_EVENT_SEND:
                        overlap->send.result->resume( overlap->bytes );
                        break;
                    case SELECT_EVENT_ACCEPT:
                        overlap->accept.result->resume( overlap->accept.client );
                        break;
                    case SELECT_EVENT_CONNECT:
                        overlap->connect.result->resume( 0 );
                        break;
                    default:
                        break;
                    }
                }
            }
        }

    private:
        HANDLE _iocp;
        WSADATA _wsadata;
        std::jthread _thread;
        std::deque<overlapped *> _overlaps;
        LPFN_ACCEPTEX _AcceptEx;
        LPFN_CONNECTEX _ConnectEx;
        LPFN_GETACCEPTEXSOCKADDRS _GetAcceptExSockaddrs;
    };
    struct timer_scheduler
    {
    public:
        using clock = std::chrono::system_clock;
        using duration = clock::duration;
        using time_point = clock::time_point;

    public:
        static constexpr x::int64 N = 60;
        static constexpr x::int64 Si = 10;

    public:
        struct timer
        {
            time_point time;
            x::await_result<void> * result;
        };
        using timer_iterator = std::list<timer>::iterator;
        struct timewheel
        {
            x::int64 interval = 0;
            std::list<timer_iterator> timers;
        };

    private:
        timer_scheduler()
        {
            _now = clock::now();
            _id = SetTimer( nullptr, 0, USER_TIMER_MINIMUM, time_tick );
        }
        ~timer_scheduler()
        {
            KillTimer( nullptr, _id );
        }

    public:
        static timer_scheduler * instance()
        {
            static timer_scheduler timer;
            return &timer;
        }

    public:
        static void sleep( const time_point & time, x::await_result<void> * result )
        {
            x::int64 timeout = std::chrono::duration_cast<std::chrono::milliseconds>( time - instance()->_now ).count();

            x::int64 ticks = timeout / Si;

            x::int64 slot = ( instance()->_slot + ( ticks % N ) ) % N;

            instance()->_timewheels[slot].timers.push_back( instance()->_timers.insert( instance()->_timers.end(), { time, result } ) );
        }

    private:
        static void time_tick( HWND, UINT, UINT_PTR, DWORD )
        {
            auto now = clock::now();
            auto slot = instance()->_slot;
            auto tick = std::chrono::duration_cast<std::chrono::milliseconds>( clock::now() - instance()->_now ).count();

            while ( tick > 0 )
            {
                instance()->_timewheels[slot].interval += tick;

                for( auto it = instance()->_timewheels[slot].timers.begin(); it != instance()->_timewheels[slot].timers.end(); )
                {
                    if ( ( *it )->time <= now )
                    {
                        ( *it )->result->resume();
                        instance()->_timers.erase( ( *it ) );
                        it = instance()->_timewheels[slot].timers.erase( it );
                    }
                    else
                    {
                        ++it;
                    }
                }

                if ( instance()->_timewheels[slot].interval >= Si )
                {
                    tick = instance()->_timewheels[slot].interval - Si;
                    instance()->_timewheels[slot].interval = 0;
                    slot = ( slot + 1 ) % N;
                }
                else
                {
                    break;
                }
            }

            instance()->_now = now;
            instance()->_slot = slot;
        }

    private:
        UINT_PTR _id = 0;
        x::uint64 _slot = 0;
        time_point _now = {};
        std::list<timer> _timers;
        std::array<timewheel, N> _timewheels;
    };
    uint32 key_event( WPARAM wParam )
    {
        switch ( wParam )
        {
        case VK_TAB: return x_input::INPUT_KEY_TAB;
        case VK_LEFT: return x_input::INPUT_KEY_LEFT_ARROW;
        case VK_RIGHT: return x_input::INPUT_KEY_RIGHT_ARROW;
        case VK_UP: return x_input::INPUT_KEY_UP_ARROW;
        case VK_DOWN: return x_input::INPUT_KEY_DOWN_ARROW;
        case VK_PRIOR: return x_input::INPUT_KEY_PAGE_UP;
        case VK_NEXT: return x_input::INPUT_KEY_PAGE_DOWN;
        case VK_HOME: return x_input::INPUT_KEY_HOME;
        case VK_END: return x_input::INPUT_KEY_END;
        case VK_INSERT: return x_input::INPUT_KEY_INSERT;
        case VK_DELETE: return x_input::INPUT_KEY_DELETE;
        case VK_BACK: return x_input::INPUT_KEY_BACKSPACE;
        case VK_SPACE: return x_input::INPUT_KEY_SPACE;
        case VK_RETURN: return x_input::INPUT_KEY_ENTER;
        case VK_ESCAPE: return x_input::INPUT_KEY_ESC;
        case VK_OEM_7: return x_input::INPUT_KEY_APOSTROPHE;
        case VK_OEM_COMMA: return x_input::INPUT_KEY_COMMA;
        case VK_OEM_MINUS: return x_input::INPUT_KEY_MINUS;
        case VK_OEM_PERIOD: return x_input::INPUT_KEY_PERIOD;
        case VK_OEM_2: return x_input::INPUT_KEY_SLASH;
        case VK_OEM_1: return x_input::INPUT_KEY_SEMICOLON;
        case VK_OEM_PLUS: return x_input::INPUT_KEY_EQUAL;
        case VK_OEM_4: return x_input::INPUT_KEY_LEFT_BRACKET;
        case VK_OEM_5: return x_input::INPUT_KEY_BACKSLASH;
        case VK_OEM_6: return x_input::INPUT_KEY_RIGHT_BRACKET;
        case VK_OEM_3: return x_input::INPUT_KEY_GRAVE_ACCENT;
        case VK_CAPITAL: return x_input::INPUT_KEY_CAPS_LOCK;
        case VK_SCROLL: return x_input::INPUT_KEY_SCROLL_LOCK;
        case VK_NUMLOCK: return x_input::INPUT_KEY_NUM_LOCK;
        case VK_SNAPSHOT: return x_input::INPUT_KEY_PRINT_SCREEN;
        case VK_PAUSE: return x_input::INPUT_KEY_PAUSE;
        case VK_NUMPAD0: return x_input::INPUT_KEY_PAD_0;
        case VK_NUMPAD1: return x_input::INPUT_KEY_PAD_1;
        case VK_NUMPAD2: return x_input::INPUT_KEY_PAD_2;
        case VK_NUMPAD3: return x_input::INPUT_KEY_PAD_3;
        case VK_NUMPAD4: return x_input::INPUT_KEY_PAD_4;
        case VK_NUMPAD5: return x_input::INPUT_KEY_PAD_5;
        case VK_NUMPAD6: return x_input::INPUT_KEY_PAD_6;
        case VK_NUMPAD7: return x_input::INPUT_KEY_PAD_7;
        case VK_NUMPAD8: return x_input::INPUT_KEY_PAD_8;
        case VK_NUMPAD9: return x_input::INPUT_KEY_PAD_9;
        case VK_DECIMAL: return x_input::INPUT_KEY_PAD_DEC;
        case VK_DIVIDE: return x_input::INPUT_KEY_PAD_DIV;
        case VK_MULTIPLY: return x_input::INPUT_KEY_PAD_MUL;
        case VK_SUBTRACT: return x_input::INPUT_KEY_PAD_SUB;
        case VK_ADD: return x_input::INPUT_KEY_PAD_ADD;
        case VK_LSHIFT: return x_input::INPUT_KEY_LEFT_SHIFT;
        case VK_LCONTROL: return x_input::INPUT_KEY_LEFT_CTRL;
        case VK_LMENU: return x_input::INPUT_KEY_LEFT_ALT;
        case VK_LWIN: return x_input::INPUT_KEY_LEFT_SUPER;
        case VK_RSHIFT: return x_input::INPUT_KEY_RIGHT_SHIFT;
        case VK_RCONTROL: return x_input::INPUT_KEY_RIGHT_CTRL;
        case VK_RMENU: return x_input::INPUT_KEY_RIGHT_ALT;
        case VK_RWIN: return x_input::INPUT_KEY_RIGHT_SUPER;
        case VK_APPS: return x_input::INPUT_KEY_MENU;
        case '0': return x_input::INPUT_KEY_0;
        case '1': return x_input::INPUT_KEY_1;
        case '2': return x_input::INPUT_KEY_2;
        case '3': return x_input::INPUT_KEY_3;
        case '4': return x_input::INPUT_KEY_4;
        case '5': return x_input::INPUT_KEY_5;
        case '6': return x_input::INPUT_KEY_6;
        case '7': return x_input::INPUT_KEY_7;
        case '8': return x_input::INPUT_KEY_8;
        case '9': return x_input::INPUT_KEY_9;
        case 'A': return x_input::INPUT_KEY_A;
        case 'B': return x_input::INPUT_KEY_B;
        case 'C': return x_input::INPUT_KEY_C;
        case 'D': return x_input::INPUT_KEY_D;
        case 'E': return x_input::INPUT_KEY_E;
        case 'F': return x_input::INPUT_KEY_F;
        case 'G': return x_input::INPUT_KEY_G;
        case 'H': return x_input::INPUT_KEY_H;
        case 'I': return x_input::INPUT_KEY_I;
        case 'J': return x_input::INPUT_KEY_J;
        case 'K': return x_input::INPUT_KEY_K;
        case 'L': return x_input::INPUT_KEY_L;
        case 'M': return x_input::INPUT_KEY_M;
        case 'N': return x_input::INPUT_KEY_N;
        case 'O': return x_input::INPUT_KEY_O;
        case 'P': return x_input::INPUT_KEY_P;
        case 'Q': return x_input::INPUT_KEY_Q;
        case 'R': return x_input::INPUT_KEY_R;
        case 'S': return x_input::INPUT_KEY_S;
        case 'T': return x_input::INPUT_KEY_T;
        case 'U': return x_input::INPUT_KEY_U;
        case 'V': return x_input::INPUT_KEY_V;
        case 'W': return x_input::INPUT_KEY_W;
        case 'X': return x_input::INPUT_KEY_X;
        case 'Y': return x_input::INPUT_KEY_Y;
        case 'Z': return x_input::INPUT_KEY_Z;
        case VK_F1: return x_input::INPUT_KEY_F1;
        case VK_F2: return x_input::INPUT_KEY_F2;
        case VK_F3: return x_input::INPUT_KEY_F3;
        case VK_F4: return x_input::INPUT_KEY_F4;
        case VK_F5: return x_input::INPUT_KEY_F5;
        case VK_F6: return x_input::INPUT_KEY_F6;
        case VK_F7: return x_input::INPUT_KEY_F7;
        case VK_F8: return x_input::INPUT_KEY_F8;
        case VK_F9: return x_input::INPUT_KEY_F9;
        case VK_F10: return x_input::INPUT_KEY_F10;
        case VK_F11: return x_input::INPUT_KEY_F11;
        case VK_F12: return x_input::INPUT_KEY_F12;
        }

        return x_input::NONE;
    }
	LRESULT x_windows_process( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
	{
        POINT pt;
        TRACKMOUSEEVENT tme;
        TOUCHINPUT inputs[15];
        auto info = (window_info *)GetWindowLongPtrA( hWnd, GWLP_USERDATA );

        switch ( Msg )
        {
        case WM_DESTROY: break;
        case WM_LBUTTONDOWN:
            SetCapture( hWnd );
            break;
        case WM_RBUTTONDOWN:
            SetCapture( hWnd );
            break;
        case WM_MBUTTONDOWN:
            SetCapture( hWnd );
            break;
        case WM_MOUSEWHEEL: break;
        case WM_LBUTTONUP:
            ReleaseCapture();
            break;
        case WM_RBUTTONUP:
            ReleaseCapture();
            break;
        case WM_MBUTTONUP:
            ReleaseCapture();
            break;
        case WM_MOUSEMOVE:
            tme.cbSize = sizeof( tme );
            tme.dwFlags = TME_HOVER | TME_LEAVE;
            tme.dwHoverTime = 50;
            tme.hwndTrack = hWnd;
            TrackMouseEvent( &tme );

            GetCursorPos( &pt );
            info->inputs[x_input::INPUT_MOUSE_MOVE].pos = { pt.x, pt.y };
            break;
        case WM_MOUSEHOVER:
            break;
        case WM_MOUSELEAVE:
            break;
        case WM_SETFOCUS:
            break;
        case WM_KILLFOCUS:
            break;
        case WM_MOUSEACTIVATE:
            break;
        case WM_ACTIVATEAPP:
            break;
        case WM_ACTIVATE:
            break;
        case WM_KEYDOWN:
			key_event( wParam );
            break;
        case WM_KEYUP:
			key_event( wParam );
            break;
        case WM_CHAR:
            info->inputs[x_input::INPUT_UNICODE_CHAR].unicode = (wchar_t)wParam;
            break;
        case WM_TOUCH:
            if ( GetTouchInputInfo( (HTOUCHINPUT)lParam, LOWORD( wParam ), inputs, sizeof( TOUCHINPUT ) ) )
            {
                info->inputs[x_input::INPUT_TOUCH_COUNT].ctl = LOWORD( wParam );

                for ( size_t i = 0; i < LOWORD( wParam ); i++ )
                    info->inputs[x_input::INPUT_TOUCH_F0 + i].pos = { inputs[i].x, inputs[i].y };
            }
            else
            {
                info->inputs[x_input::INPUT_TOUCH_COUNT].ctl = 0;

                for ( size_t i = 0; i < LOWORD( wParam ); i++ )
                    info->inputs[x_input::INPUT_TOUCH_F0 + i].pos = { 0, 0 };
            }
            break;
        case WM_CLOSE:
            UnregisterTouchWindow( hWnd );
            DestroyWindow( hWnd );
            break;
        default:
            break;
        }

        return DefWindowProcA( hWnd, Msg, wParam, lParam );
	}
}

int x_main( int argc, const char ** argv )
{
	WNDCLASSEXA wc;
	wc.cbSize = sizeof( wc );
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = x_windows_process;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandleA( nullptr );
	wc.hIcon = nullptr;
	wc.hCursor = LoadCursorA( nullptr, IDC_ARROW );
	wc.hbrBackground = static_cast<HBRUSH>( GetStockObject( COLOR_WINDOW ) );
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = "x_window_class";
	wc.hIconSm = nullptr;
	RegisterClassExA( &wc );

	while ( 1 )
	{
        MSG msg = {};
		if ( GetMessageA( &msg, nullptr, 0, 0 ) )
		{
			if ( msg.message == WM_QUIT )
				break;

			TranslateMessage( &msg );

			DispatchMessageA( &msg );
		}

        //x::scheduler::instance()->maintick();
	}

    //x::scheduler::instance()->shutdown();

	return 0;
}
uint8 x_os_arch()
{
	return sizeof( void * ) * CHAR_BIT;
}
x_string x_os_name()
{
	return x::allocator::salloc( "windows" );
}

x_string utf8_ansi( x_string utf_str )
{
    std::string gbk_str;

    int len = MultiByteToWideChar( CP_UTF8, 0, utf_str, -1, NULL, 0 );

    std::wstring wstr( len, 0 );

    len = MultiByteToWideChar( CP_UTF8, 0, utf_str, -1, wstr.data(), len );

    len = WideCharToMultiByte( CP_ACP, 0, wstr.data(), -1, NULL, 0, NULL, 0 );

    gbk_str.resize( len );
    len = WideCharToMultiByte( CP_ACP, 0, wstr.data(), -1, gbk_str.data(), len, NULL, 0 );

    return x::allocator::salloc( gbk_str.c_str() );
}
x_string ansi_utf8( x_string gbk_str )
{
    std::string utf_str;

    int len = MultiByteToWideChar( CP_ACP, 0, gbk_str, -1, NULL, 0 );

    std::wstring wstr( len, 0 );

    len = MultiByteToWideChar( CP_ACP, 0, gbk_str, -1, wstr.data(), len );

    len = WideCharToMultiByte( CP_UTF8, 0, wstr.data(), -1, NULL, 0, NULL, 0 );

    utf_str.resize( len );
    len = WideCharToMultiByte( CP_UTF8, 0, wstr.data(), -1, utf_str.data(), len, NULL, 0 );

    return x::allocator::salloc( utf_str.c_str() );
}
x_string wide_ansi( x_string wide_str )
{
    std::string ansi_str;

    int len = WideCharToMultiByte( CP_ACP, 0, (const wchar_t *)wide_str, -1, NULL, 0, NULL, 0 );

    ansi_str.resize( len );

    len = WideCharToMultiByte( CP_ACP, 0, (const wchar_t *)wide_str, -1, ansi_str.data(), len, NULL, 0 );

    return x::allocator::salloc( ansi_str.c_str() );
}
x_string ansi_wide( x_string ansi_str )
{
    std::wstring wide_str;

    int len = MultiByteToWideChar( CP_ACP, 0, ansi_str, -1, NULL, 0 );

    wide_str.resize( len );

    len = MultiByteToWideChar( CP_ACP, 0, ansi_str, -1, wide_str.data(), len );

    return x::allocator::salloc( (const char *)wide_str.c_str() );
}

x_file x_file_create()
{
    return new file_info;
}
bool x_file_open( x_file file, x_string path, uint32 mode )
{
    auto info = (file_info *)file;

    DWORD dwDesiredAccess = 0, dwCreationDisposition = 0;

    if ( mode & FILE_MODE_READ )
    {
        dwDesiredAccess |= GENERIC_READ;
        dwCreationDisposition = OPEN_EXISTING;
    }
    if ( mode & FILE_MODE_WRITE )
    {
        dwDesiredAccess |= GENERIC_WRITE;
        dwCreationDisposition = CREATE_NEW;
    }
    if ( mode & FILE_MODE_TRUNCATE )
    {
        dwDesiredAccess |= GENERIC_WRITE;
        dwCreationDisposition = TRUNCATE_EXISTING;
    }

    info->path = path;
    info->mode = mode;
    info->handle = CreateFileA( path, dwDesiredAccess, 0, nullptr, dwCreationDisposition, FILE_FLAG_OVERLAPPED, nullptr );

    if ( info->handle != INVALID_HANDLE_VALUE  )
    {
        if ( info->mode & FILE_MODE_APPAND )
        {
            auto end_off = x_file_size( file );
            info->readoff = end_off;
            info->writeoff = end_off;
        }

        ::CreateIoCompletionPort( info->handle, iocp_scheduler::iocp_handle(), (ULONG_PTR)info, 0 );
    }

    return info->handle != INVALID_HANDLE_VALUE;
}
uint64 x_file_size( x_file file )
{
    auto info = (file_info *)file;

    return std::filesystem::file_size( info->path );
}
x_string x_file_name( x_file file )
{
    auto info = (file_info *)file;

    return x::allocator::salloc( info->path.filename().string() );
}
int64 x_file_time( x_file file )
{
    auto info = (file_info *)file;

    auto time = std::filesystem::last_write_time( info->path );

    return std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::time_point_cast<std::chrono::milliseconds>( time ).time_since_epoch() ).count();
}
void x_file_rename( x_file file, x_string name )
{
    auto info = (file_info *)file;

    std::filesystem::rename( info->path, name );
}
bool x_file_is_eof( x_file file )
{
    return ( (file_info *)file )->readoff >= x_file_size( file );
}
uint64 x_file_read_tell( x_file file )
{
    return ( (file_info *)file )->readoff;
}
uint64 x_file_write_tell( x_file file )
{
    return ( (file_info *)file )->writeoff;
}
void x_file_read_seek( x_file file, int32 off, uint32 pos )
{
    auto info = (file_info *)file;

    switch ( pos )
    {
    case SEEK_POS_BEG:
        info->readoff = off;
        break;
    case SEEK_POS_CUR:
        info->readoff += off;
        break;
    case SEEK_POS_END:
        info->readoff = x_file_size( file ) + off;
        break;
    default:
        break;
    }
}
void x_file_write_seek( x_file file, int32 off, uint32 pos )
{
    auto info = (file_info *)file;

    switch ( pos )
    {
    case SEEK_POS_BEG:
        info->writeoff = off;
        break;
    case SEEK_POS_CUR:
        info->writeoff += off;
        break;
    case SEEK_POS_END:
        info->writeoff = x_file_size( file ) + off;
        break;
    default:
        break;
    }
}
uint64 x_file_read( x_file file, intptr buffer, uint64 size )
{
    auto info = (file_info *)file;

    LONG low = 0, high = 0;

    if ( info->readoff > LONG_MAX )
    {
        low = info->readoff & 0xFFFFFFFF;
        high = ( info->readoff >> 32 );
    }
    else
    {
        low = (LONG)info->readoff;
        high = 0;
    }

    if ( ::SetFilePointer( info->handle, low, &high, FILE_BEGIN ) != INVALID_SET_FILE_POINTER )
    {
        DWORD BytesToRead = (DWORD)size, BytesRead = 0;

        if ( ::ReadFile( info->handle, buffer, BytesToRead, &BytesRead, nullptr ) )
        {
            info->readoff += BytesRead;
            return BytesRead;
        }
    }

    return 0;
}
uint64 x_file_write( x_file file, intptr buffer, uint64 size )
{
    auto info = (file_info *)file;

    LONG low = 0, high = 0;

    if ( info->readoff > LONG_MAX )
    {
        low = info->readoff & 0xFFFFFFFF;
        high = ( info->readoff >> 32 );
    }
    else
    {
        low = (LONG)info->readoff;
        high = 0;
    }

    if ( ::SetFilePointer( info->handle, low, &high, FILE_BEGIN ) != INVALID_SET_FILE_POINTER )
    {
        DWORD BytesToWrite = (DWORD)size, BytesWrite = 0;

        if ( ::WriteFile( info->handle, buffer, BytesToWrite, &BytesWrite, nullptr ) )
        {
            info->writeoff += BytesWrite;
            return BytesWrite;
        }
    }

    return 0;
}
void x_file_close( x_file file )
{
    auto info = (file_info *)file;

    CloseHandle( info->handle );
}
void x_file_release( x_file file )
{
    delete (file_info *)file;
}

x_window x_window_create()
{
	auto win = new window_info;

	RECT rect;
	auto desk = GetDesktopWindow();
	GetWindowRect( desk, &rect );

	win->rect.pos = { rect.left, rect.top };
	win->rect.size = { (int)( ( rect.right - rect.left ) * 0.7f ), (int)( ( rect.bottom - rect.top ) * 0.7f ) };
	win->hwnd = CreateWindowExA( WS_EX_LAYERED, "x_window_class", "", WS_POPUP, win->rect.pos.x, win->rect.pos.y, win->rect.size.w, win->rect.size.h, nullptr, nullptr, GetModuleHandleA( nullptr ), nullptr );

	SetWindowLongPtrA( win->hwnd, GWLP_USERDATA, (LONG_PTR)win );

    RegisterTouchWindow( win->hwnd, 0 );

	return win;
}
float32 x_window_dpi_scale( x_window window )
{
	auto win = (window_info *)window;

	return (float32)GetDpiForWindow( win->hwnd ) / (float32)USER_DEFAULT_SCREEN_DPI;
}
void x_window_set_parent( x_window window, x_window parent )
{
	auto win = (window_info *)window;

	win->parent = (window_info *)parent;

	SetParent( win->hwnd, ( (window_info *)parent )->hwnd );
}
x_window x_window_get_parent( x_window window )
{
	auto win = (window_info *)window;

	return win->parent;
}
x_pos x_window_get_pos( x_window window )
{
	auto win = (window_info *)window;

	return win->rect.pos;
}
void x_window_set_pos( x_window window, x_pos p )
{
	auto win = (window_info *)window;
	
	win->rect.pos = p;

	MoveWindow( (HWND)window, win->rect.pos.x, win->rect.pos.y, win->rect.size.w, win->rect.size.h, 1 );
}
x_size x_window_get_size( x_window window )
{
	auto win = (window_info *)window;

	return win->rect.size;
}
void x_window_set_size( x_window window, x_size s )
{
	auto win = (window_info *)window;

	win->rect.size = s;

	MoveWindow( (HWND)window, win->rect.pos.x, win->rect.pos.y, win->rect.size.w, win->rect.size.h, 1 );
}
x_rect x_window_get_rect( x_window window )
{
    auto win = (window_info *)window;

    return win->rect;
}
void x_window_set_rect( x_window window, x_rect rect )
{
    auto win = (window_info *)window;

    win->rect = rect;

    MoveWindow( (HWND)window, win->rect.pos.x, win->rect.pos.y, win->rect.size.w, win->rect.size.h, 1 );
}
void x_window_show( x_window window )
{
	auto win = (window_info *)window;

	win->state = WINDOW_STATE_NORMAL;

	ShowWindow( (HWND)window, SW_SHOW );
}
void x_window_hide( x_window window )
{
	auto win = (window_info *)window;

	win->state = WINDOW_STATE_HIDDEN;

	ShowWindow( (HWND)window, SW_HIDE );
}
void x_window_show_minimize( x_window window )
{
	auto win = (window_info *)window;

	win->state = WINDOW_STATE_MINIMIZE;

	ShowWindow( (HWND)window, SW_MINIMIZE );
}
void x_window_show_maximize( x_window window )
{
	auto win = (window_info *)window;

	win->state = WINDOW_STATE_MAXIMIZE;

	ShowWindow( (HWND)window, SW_MAXIMIZE );
}
void x_window_show_fullscreen( x_window window, bool monitor )
{
	auto win = (window_info *)window;

	win->state = WINDOW_STATE_FULLSCREEN;

	auto style = GetWindowLongA( (HWND)window, GWL_STYLE );
	auto exstyle = GetWindowLongA( (HWND)window, GWL_EXSTYLE );

	SetWindowLongA( (HWND)window, GWL_STYLE, style & ~( WS_CAPTION | WS_THICKFRAME ) );
	SetWindowLongA( (HWND)window, GWL_EXSTYLE, exstyle & ~( WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE ) );

	RECT rect = {};

	if ( monitor )
	{
		MONITORINFO info;
		info.cbSize = sizeof( info );
		GetMonitorInfoA( MonitorFromWindow( (HWND)window, MONITOR_DEFAULTTONEAREST ), &info );
		rect = info.rcMonitor;
	}
	else
	{
		auto desk = GetDesktopWindow();
		GetWindowRect( desk, &rect );
	}

	SetWindowPos( (HWND)window, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED );
}
bool x_window_is_hidden( x_window window )
{
	auto win = (window_info *)window;

	return win->state == WINDOW_STATE_HIDDEN;
}
int32 x_window_get_show_state( x_window window )
{
	auto win = (window_info *)window;

	return win->state;
}
void x_window_mouse_show( x_window window )
{
	auto win = (window_info *)window;

	win->cursor = true;

	ShowCursor( TRUE );
}
void x_window_mouse_hide( x_window window )
{
	auto win = (window_info *)window;

	win->cursor = false;

	ShowCursor( FALSE );
}
bool x_window_mouse_is_hidden( x_window window )
{
	auto win = (window_info *)window;

	return !win->cursor;
}
x_inputval x_window_input( x_window window, x_input key )
{
    auto win = (window_info *)window;

    return win->inputs[key];
}
void x_window_close( x_window window )
{
    auto win = (window_info *)window;

    if ( win->hwnd != nullptr )
    {
        CloseWindow( win->hwnd );

        win->hwnd = nullptr;
    }
}
void x_window_release( x_window window )
{
    delete (window_info *)window;
}

x_socket x_socket_create( uint32 protocol )
{
    auto info = new socket_info;

    info->type = protocol;
    info->info.ai_family = AF_INET;
    switch ( protocol )
    {
    case SOCKET_PROTOCOL_UDP:
        info->info.ai_socktype = SOCK_DGRAM;
        info->info.ai_protocol = IPPROTO_UDP;
        break;
    case SOCKET_PROTOCOL_TCP:
        info->info.ai_socktype = SOCK_STREAM;
        info->info.ai_protocol = IPPROTO_TCP;
        break;
    default:
        break;
    }

    info->handle = ::socket( info->info.ai_family, info->info.ai_socktype, info->info.ai_protocol );

    ::CreateIoCompletionPort( (HANDLE)info->handle, iocp_scheduler::iocp_handle(), (ULONG_PTR)info, 0 );

    return info;
}
int32 x_socket_bind( x_socket socket, x_string sockname, uint16 port )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    info->sockaddr.sin_family = AF_INET;
    info->sockaddr.sin_port = ::htons( port );
    ::inet_pton( AF_INET, sockname, &info->sockaddr.sin_addr.s_addr );

    return ::bind( info->handle, (sockaddr *)&info->sockaddr, sizeof( info->sockaddr ) );
}
int32 x_socket_listen( x_socket socket )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    return ::listen( info->handle, SOMAXCONN );
}
x_socket x_socket_accept( x_socket socket )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    auto client_info = new socket_info;

    int len = sizeof( client_info->peeraddr );
    client_info->info = info->info;
    client_info->sockaddr = info->sockaddr;
    client_info->handle = ::accept( info->handle, (sockaddr *)&client_info->peeraddr, &len );

    CreateIoCompletionPort( (HANDLE)client_info->handle, iocp_scheduler::iocp_handle(), (ULONG_PTR)client_info, 0 );

    return client_info;
}
int32 x_socket_connect( x_socket socket, x_string peername, uint16 port )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    info->peeraddr.sin_family = AF_INET;
    info->peeraddr.sin_port = ::htons( port );
    ::inet_pton( AF_INET, peername, &info->peeraddr.sin_addr.s_addr );

    return ::connect(info->handle, (sockaddr *)&info->peeraddr, sizeof( info->peeraddr ) );
}
uint64 x_socket_recv( x_socket socket, intptr buffer, uint64 size )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    int len = 0, fromlen = sizeof( info->peeraddr );
    if ( info->type == SOCKET_PROTOCOL_TCP )
        len = ::recv( info->handle, (char *)buffer, (int)size, 0 );
    else
        len = ::recvfrom( info->handle, (char *)buffer, (int)size, 0, (sockaddr *)&info->peeraddr, &fromlen );

    return len;
}
uint64 x_socket_send( x_socket socket, intptr buffer, uint64 size )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    int len = 0, fromlen = sizeof( info->peeraddr );
    if ( info->type == SOCKET_PROTOCOL_TCP )
        len = ::send( info->handle, (char *)buffer, (int)size, 0 );
    else
        len = ::sendto( info->handle, (char *)buffer, (int)size, 0, (sockaddr *)&info->peeraddr, sizeof( info->peeraddr ) );

    return len;
}
uint64 x_socket_sendto( x_socket socket, x_string peername, uint16 port, intptr buffer, uint64 size )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    if ( info->type == SOCKET_PROTOCOL_UDP )
    {
        info->peeraddr.sin_family = AF_INET;
        info->peeraddr.sin_port = ::htons( port );
        ::inet_pton( AF_INET, peername, &info->peeraddr.sin_addr.s_addr );

        return ::sendto( info->handle, (char *)buffer, (int)size, 0, (sockaddr *)&info->peeraddr, sizeof( info->peeraddr ) );
    }

    return 0;
}
x_string x_socket_getsockname( x_socket socket )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    char buf[64]; ::memset( buf, 0, 64 );

    return x::allocator::salloc( ::inet_ntop( AF_INET, &info->sockaddr.sin_addr, buf, 64 ) );
}
uint16 x_socket_getsockport( x_socket socket )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    return ::ntohs( info->sockaddr.sin_port );
}
x_string x_socket_getpeername( x_socket socket )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    char buf[64]; ::memset( buf, 0, 64 );

    return x::allocator::salloc( ::inet_ntop( AF_INET, &info->peeraddr.sin_addr, buf, 64 ) );
}
uint16 x_socket_getpeerport( x_socket socket )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    return ::ntohs( info->peeraddr.sin_port );
}
x_string x_socket_getsockopt( x_socket socket, int32 key )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    int len = 128;
    char buf[128];
    ::memset( buf, 0, 128 );
    ::getsockopt( info->handle, SOL_SOCKET, key, buf, &len );

    return x::allocator::salloc( buf );
}
int32 x_socket_setsockopt( x_socket socket, int32 key, x_string value )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    return ::setsockopt( info->handle, SOL_SOCKET, key, value, (int)::strlen( value ) );
}
void x_socket_close( x_socket socket )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    ::closesocket( info->handle );

    info->handle = 0;
}
void x_socket_release( x_socket socket )
{
    delete reinterpret_cast<socket_info *>( socket );
}

x_awaitable x_awaitable_file_read( x_file file, intptr buffer, uint64 size )
{
    auto info = (file_info *)file;

    auto lap = iocp_scheduler::dequeue();
    lap->type = SELECT_EVENT_READ;
    lap->file = info;
    lap->buffer.buf = reinterpret_cast<CHAR *>( buffer );
    lap->buffer.len = (ULONG)size;

    return x::to_awaitable<uint64>( [lap]( x::await_result<uint64> * result )
    {
        lap->read.result = result;

        ReadFile( lap->file->handle, lap->buffer.buf, lap->buffer.len, &lap->bytes, (LPOVERLAPPED)lap );

    } ).address();
}
x_awaitable x_awaitable_file_write( x_file file, intptr buffer, uint64 size )
{
    auto info = (file_info *)file;

    auto lap = iocp_scheduler::dequeue();
    lap->type = SELECT_EVENT_WRITE;
    lap->file = info;
    lap->buffer.buf = reinterpret_cast<CHAR *>( buffer );
    lap->buffer.len = (ULONG)size;

    return x::to_awaitable<uint64>( [lap]( x::await_result<uint64> * result )
    {
        lap->write.result = result;

        WriteFile( lap->file->handle, lap->buffer.buf, lap->buffer.len, &lap->bytes, (LPOVERLAPPED)lap );

    } ).address();
}
x_awaitable x_awaitable_timer_sleep( int64 milliseconds )
{
    auto time = timer_scheduler::clock::now() + std::chrono::milliseconds( milliseconds );

    return x::to_awaitable<void>( [time]( x::await_result<void> * result )
    {
        if ( time > timer_scheduler::clock::now() )
        {
            timer_scheduler::sleep( time, result );
        }
        else
        {
            result->resume();
        }
    } ).address();
}
x_awaitable x_awaitable_socket_accept( x_socket socket )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    auto lap = iocp_scheduler::dequeue();
    lap->socket = info;
    lap->type = SELECT_EVENT_ACCEPT;
    lap->accept.client = reinterpret_cast<socket_info *>( x_socket_create( info->type ) );

    lap->accept.client->sockaddr = info->sockaddr;

    return x::to_awaitable<x_socket>( [lap]( x::await_result<x_socket> * result )
    {
        lap->accept.result = result;

        iocp_scheduler::AcceptEx( lap->socket->handle, lap->accept.client->handle, lap->buffer.buf, 0, sizeof( sockaddr_in ), sizeof( sockaddr_in ), &lap->bytes, (LPOVERLAPPED)lap );

    } ).address();
}
x_awaitable x_awaitable_socket_connect( x_socket socket, x_string peername, uint16 port )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    info->peeraddr.sin_family = AF_INET;
    info->peeraddr.sin_port = ::htons( port );
    ::inet_pton( AF_INET, peername, &info->peeraddr.sin_addr.s_addr );

    auto lap = iocp_scheduler::dequeue();
    lap->socket = info;
    lap->type = SELECT_EVENT_CONNECT;

    return x::to_awaitable<int32>( [lap]( x::await_result<int32> * result )
    {
        lap->connect.result = result;

        iocp_scheduler::ConnectEx( lap->socket->handle, (sockaddr *)&lap->socket->peeraddr, sizeof( lap->socket->peeraddr ), &lap->buffer, 1, &lap->bytes, (LPOVERLAPPED)lap );
    } ).address();
}
x_awaitable x_awaitable_socket_recv( x_socket socket, intptr buffer, uint64 size )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    auto lap = iocp_scheduler::dequeue();
    lap->type = SELECT_EVENT_RECV;
    lap->socket = info;
    lap->buffer.buf = reinterpret_cast<CHAR *>( buffer );
    lap->buffer.len = (ULONG)size;

    return x::to_awaitable<uint64>( [lap]( x::await_result<uint64> * result )
    {
        lap->recv.result = result;

        if ( lap->socket->type == SOCKET_PROTOCOL_TCP )
            WSARecv( lap->socket->handle, &lap->buffer, 1, &lap->bytes, &lap->flags, (LPOVERLAPPED)lap, nullptr );
        else
            WSARecvFrom( lap->socket->handle, &lap->buffer, 1, &lap->bytes, &lap->flags, (sockaddr *)&lap->socket->peeraddr, &lap->sendto.size, (LPOVERLAPPED)lap, nullptr );
    } ).address();
}
x_awaitable x_awaitable_socket_send( x_socket socket, intptr buffer, uint64 size )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    auto lap = iocp_scheduler::dequeue();
    lap->type = SELECT_EVENT_SEND;
    lap->socket = info;
    lap->buffer.buf = reinterpret_cast<CHAR *>( buffer );
    lap->buffer.len = (ULONG)size;
    lap->sendto.size = sizeof( sockaddr_in );

    return x::to_awaitable<uint64>( [lap]( x::await_result<uint64> * result )
    {
        lap->send.result = result;

        ::WSASend( lap->socket->handle, &lap->buffer, 1, &lap->bytes, lap->flags, (LPOVERLAPPED)lap, nullptr );
    } ).address();
}
x_awaitable x_awaitable_socket_sendto( x_socket socket, x_string peername, uint16 port, intptr buffer, uint64 size )
{
    auto info = reinterpret_cast<socket_info *>( socket );

    info->peeraddr.sin_family = AF_INET;
    info->peeraddr.sin_port = ::htons( port );
    ::inet_pton( AF_INET, peername, &info->peeraddr.sin_addr.s_addr );

    auto lap = iocp_scheduler::dequeue();
    lap->type = SELECT_EVENT_SEND;
    lap->socket = info;
    lap->buffer.buf = reinterpret_cast<CHAR *>( buffer );
    lap->buffer.len = (ULONG)size;
    lap->sendto.size = sizeof( sockaddr_in );

    return x::to_awaitable<uint64>( [lap]( x::await_result<uint64> * result )
    {
        lap->send.result = result;

        ::WSASendTo( lap->socket->handle, &lap->buffer, 1, &lap->bytes, lap->flags, (sockaddr *)&lap->socket->peeraddr, lap->sendto.size, (LPOVERLAPPED)lap, nullptr );
    } ).address();
}

#endif
