#pragma once

#include <chrono>
#include <filesystem>

#include "type.h"

namespace x
{
    class zip
    {
    public:
        static bool compression( std::istream & is, std::string & os );
        static bool compression( std::istream & is, std::ostream & os );
        static bool compression( const std::string & is, std::string & os );
        static bool compression( const std::string & is, std::ostream & os );

    public:
        static bool decompression( std::istream & is, std::string & os );
        static bool decompression( std::istream & is, std::ostream & os );
        static bool decompression( const std::string & is, std::string & os );
        static bool decompression( const std::string & is, std::ostream & os );
        
    public:
        zip();
        zip( std::istream & stream );
        zip( const std::string & bytes );
        zip( const std::filesystem::path & path );
        ~zip();

    public:
        void load( std::istream & stream );
        void load( const std::string & bytes );
        void load( const std::filesystem::path & path );

    public:
        void save( std::string & bytes );
        void save( std::ostream & stream );
        void save( const std::filesystem::path & path );

    public:
        void reset();
        bool exist( const std::string & name );

    public:
        std::vector<std::string> namelist();
        std::size_t file_size( const std::string & name );
        std::size_t compress_size(const std::string & name );

    public:
        void extract( const std::filesystem::path & path, const std::string & name );
        void extractall( const std::filesystem::path & path );
        void extractall( const std::filesystem::path & path, const std::vector<std::string> & namelist );

    public:
        std::string read( const std::string & name );
        x::uint64 read( const std::string & name, std::ostream & stream );

    public:
        void write( const std::filesystem::path & path );
        void write( const std::filesystem::path & path, const std::string & name );

    public:
        void write_str( const std::string & name, const std::string & bytes );

    private:
        void start_read();
        void start_write();
        void append_comment();
        void remove_comment();

    private:
        void * _archive;
        std::vector<char> _buffer;
        std::vector<char> _comment;
    };

}
