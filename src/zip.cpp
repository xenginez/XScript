#include "zip.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdint.h>

#if defined(_MSC_VER) || defined(__MINGW64__)
#include <sys/utime.h>
FILE * MZ_FOPEN( char const * _FileName, char const * _Mode )
{
    FILE * result = nullptr;
    fopen_s( &result, _FileName, _Mode );
    return result;
}
FILE * __cdecl MZ_FREOPEN( char const * _FileName, char const * _Mode, FILE * _Stream )
{
    FILE * result = nullptr;
    freopen_s( &result, _FileName, _Mode, _Stream );
    return result;
}
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#define MZ_FTELL64 _ftelli64
#define MZ_FSEEK64 _fseeki64
#define MZ_FILE_STAT_STRUCT _stat64
#define MZ_FILE_STAT _stat64
#define MZ_FFLUSH fflush
#define MZ_DELETE_FILE remove
#elif defined(__MINGW32__)
#include <sys/utime.h>
#define MZ_FOPEN(f, m) fopen(f, m)
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#define MZ_FTELL64 ftello64
#define MZ_FSEEK64 fseeko64
#define MZ_FILE_STAT_STRUCT _stat
#define MZ_FILE_STAT _stat
#define MZ_FFLUSH fflush
#define MZ_FREOPEN(f, m, s) freopen(f, m, s)
#define MZ_DELETE_FILE remove
#elif defined(__TINYC__)
#include <sys/utime.h>
#define MZ_FOPEN(f, m) fopen(f, m)
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#define MZ_FTELL64 ftell
#define MZ_FSEEK64 fseek
#define MZ_FILE_STAT_STRUCT stat
#define MZ_FILE_STAT stat
#define MZ_FFLUSH fflush
#define MZ_FREOPEN(f, m, s) freopen(f, m, s)
#define MZ_DELETE_FILE remove
#elif defined(__USE_LARGEFILE64) 
#include <utime.h>
#define MZ_FOPEN(f, m) fopen64(f, m)
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#define MZ_FTELL64 ftello64
#define MZ_FSEEK64 fseeko64
#define MZ_FILE_STAT_STRUCT stat64
#define MZ_FILE_STAT stat64
#define MZ_FFLUSH fflush
#define MZ_FREOPEN(p, m, s) freopen64(p, m, s)
#define MZ_DELETE_FILE remove
#elif defined(__APPLE__)
#include <utime.h>
#define MZ_FOPEN(f, m) fopen(f, m)
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#define MZ_FTELL64 ftello
#define MZ_FSEEK64 fseeko
#define MZ_FILE_STAT_STRUCT stat
#define MZ_FILE_STAT stat
#define MZ_FFLUSH fflush
#define MZ_FREOPEN(p, m, s) freopen(p, m, s)
#define MZ_DELETE_FILE remove
#else
#endif 

#define MZ_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MZ_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MZ_CLEAR_OBJ(obj) memset(&(obj), 0, sizeof(obj))

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define MZ_READ_LE16(p) *((const mz_uint16 *)(p))
#define MZ_READ_LE32(p) *((const mz_uint32 *)(p))
#else
#define MZ_READ_LE16(p) ((mz_uint32)(((const mz_uint8 *)(p))[0]) | ((mz_uint32)(((const mz_uint8 *)(p))[1]) << 8U))
#define MZ_READ_LE32(p) ((mz_uint32)(((const mz_uint8 *)(p))[0]) | ((mz_uint32)(((const mz_uint8 *)(p))[1]) << 8U) | ((mz_uint32)(((const mz_uint8 *)(p))[2]) << 16U) | ((mz_uint32)(((const mz_uint8 *)(p))[3]) << 24U))
#endif
#define MZ_READ_LE64(p) (((mz_uint64)MZ_READ_LE32(p)) | (((mz_uint64)MZ_READ_LE32((const mz_uint8 *)(p) + sizeof(mz_uint32))) << 32U))

#define MZ_UINT16_MAX (0xFFFFU)
#define MZ_UINT32_MAX (0xFFFFFFFFU)

#define TINFL_DECOMPRESS_MEM_TO_MEM_FAILED ((size_t)(-1))

#define TINFL_LZ_DICT_SIZE 32768
#define tinfl_init(r) (r)->m_state = 0; 
#define tinfl_get_adler32(r) (r)->m_check_adler32

#define TDEFL_PUT_BITS(b, l) \
{ \
    mz_uint32 bits = b; \
    mz_uint32 len = l; \
    assert(bits <= ((1U << len) - 1U)); \
    d->m_bit_buffer |= (bits << d->m_bits_in); \
    d->m_bits_in += len; \
    while (d->m_bits_in >= 8) \
    { \
        if (d->m_pOutput_buf < d->m_pOutput_buf_end) \
            *d->m_pOutput_buf++ = (mz_uint8)(d->m_bit_buffer); \
        d->m_bit_buffer >>= 8; \
        d->m_bits_in -= 8; \
    } \
}

#define TDEFL_RLE_PREV_CODE_SIZE() \
{ \
    if (rle_repeat_count) \
    { \
        if (rle_repeat_count < 3) \
        { \
            d->m_huff_count[2][prev_code_size] = (mz_uint16)(d->m_huff_count[2][prev_code_size] + rle_repeat_count); \
            while (rle_repeat_count--) \
                packed_code_sizes[num_packed_code_sizes++] = prev_code_size; \
        } \
        else \
        { \
            d->m_huff_count[2][16] = (mz_uint16)(d->m_huff_count[2][16] + 1); \
            packed_code_sizes[num_packed_code_sizes++] = 16; \
            packed_code_sizes[num_packed_code_sizes++] = (mz_uint8)(rle_repeat_count - 3); \
        } \
        rle_repeat_count = 0; \
    } \
}

#define TDEFL_RLE_ZERO_CODE_SIZE() \
{ \
    if (rle_z_count) \
    { \
        if (rle_z_count < 3) \
        { \
            d->m_huff_count[2][0] = (mz_uint16)(d->m_huff_count[2][0] + rle_z_count); \
            while (rle_z_count--) \
                packed_code_sizes[num_packed_code_sizes++] = 0; \
        } \
        else if (rle_z_count <= 10) \
        { \
            d->m_huff_count[2][17] = (mz_uint16)(d->m_huff_count[2][17] + 1); \
            packed_code_sizes[num_packed_code_sizes++] = 17; \
            packed_code_sizes[num_packed_code_sizes++] = (mz_uint8)(rle_z_count - 3); \
        } \
        else \
        { \
            d->m_huff_count[2][18] = (mz_uint16)(d->m_huff_count[2][18] + 1); \
            packed_code_sizes[num_packed_code_sizes++] = 18; \
            packed_code_sizes[num_packed_code_sizes++] = (mz_uint8)(rle_z_count - 11); \
        } \
        rle_z_count = 0; \
    } \
}

#define TDEFL_PROBE \
{ \
    next_probe_pos = d->m_next[probe_pos]; \
    if ((!next_probe_pos) || ((dist = (mz_uint16)(lookahead_pos - next_probe_pos)) > max_dist)) \
        return; \
    probe_pos = next_probe_pos & TDEFL_LZ_DICT_SIZE_MASK; \
    if (TDEFL_READ_UNALIGNED_WORD(&d->m_dict[probe_pos + match_len - 1]) == c01) \
        break; \
}

#define TINFL_CR_BEGIN switch (r->m_state){ case 0:
#define TINFL_CR_RETURN(state_index, result) { status = result; r->m_state = state_index; goto common_exit; case state_index:; }
#define TINFL_CR_RETURN_FOREVER(state_index, result) for (;;) { TINFL_CR_RETURN(state_index, result); }
#define TINFL_CR_FINISH }

#define TINFL_GET_BYTE(state_index, c) \
{ \
    while (pIn_buf_cur >= pIn_buf_end) \
    { \
        TINFL_CR_RETURN( \
            state_index, (decomp_flags & TINFL_FLAG_HAS_MORE_INPUT) ? TINFL_STATUS_NEEDS_MORE_INPUT : TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS); \
    } \
    c = *pIn_buf_cur++; \
}

#define TINFL_NEED_BITS(state_index, n) do{ mz_uint32 c; TINFL_GET_BYTE(state_index, c); bit_buf |= (((mz_uint32)c) << num_bits); num_bits += 8; } while (num_bits < (mz_uint32)(n))
#define TINFL_SKIP_BITS(state_index, n) { if (num_bits < (mz_uint32)(n))TINFL_NEED_BITS(state_index, n); bit_buf >>= (n); num_bits -= (n); }

#define TINFL_GET_BITS(state_index, b, n) \
{ \
    if (num_bits < (mz_uint32)(n)) \
    { \
        TINFL_NEED_BITS(state_index, n); \
    } \
    b = bit_buf & ((1 << (n)) - 1); \
    bit_buf >>= (n); \
    num_bits -= (n); \
}

#define TINFL_HUFF_BITBUF_FILL(state_index, pHuff) \
do \
{ \
    temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]; \
    if (temp >= 0) \
    { \
        code_len = temp >> 9; \
        if ((code_len) && (num_bits >= code_len)) \
            break; \
    } \
    else if (num_bits > TINFL_FAST_LOOKUP_BITS) \
    { \
        code_len = TINFL_FAST_LOOKUP_BITS; \
        do \
        { \
            temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; \
        } while ((temp < 0) && (num_bits >= (code_len + 1))); \
        if (temp >= 0) \
            break; \
    } \
    TINFL_GET_BYTE(state_index, c); \
    bit_buf |= (((mz_uint32)c) << num_bits); \
    num_bits += 8; \
} while (num_bits < 15);

#define TINFL_HUFF_DECODE(state_index, sym, pHuff) \
{ \
    int temp; \
    mz_uint32 code_len, c; \
    if (num_bits < 15) \
    { \
        if ((pIn_buf_end - pIn_buf_cur) < 2) \
        { \
            TINFL_HUFF_BITBUF_FILL(state_index, pHuff); \
        } \
        else \
        { \
            bit_buf |= (((mz_uint32)pIn_buf_cur[0]) << num_bits) | (((mz_uint32)pIn_buf_cur[1]) << (num_bits + 8)); \
            pIn_buf_cur += 2; \
            num_bits += 16; \
        } \
    } \
    if ((temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0) \
        code_len = temp >> 9, temp &= 511; \
    else \
    { \
        code_len = TINFL_FAST_LOOKUP_BITS; \
        do \
        { \
            temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; \
        } while (temp < 0); \
    } \
    sym = temp; \
    bit_buf >>= code_len; \
    num_bits -= code_len; \
}

#define MZ_TOLOWER(c) ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c))

#define MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(array_ptr, element_size) (array_ptr)->m_element_size = element_size
#define MZ_ZIP_ARRAY_ELEMENT(array_ptr, element_type, index) ((element_type *)((array_ptr)->m_p))[index]

#define MZ_WRITE_LE16(p, v) mz_write_le16((mz_uint8 *)(p), (mz_uint16)(v))
#define MZ_WRITE_LE32(p, v) mz_write_le32((mz_uint8 *)(p), (mz_uint32)(v))
#define MZ_WRITE_LE64(p, v) mz_write_le64((mz_uint8 *)(p), (mz_uint64)(v))

#define MZ_ZIP64_MAX_LOCAL_EXTRA_FIELD_SIZE (sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 2)
#define MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE (sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 3)

#define UPDC32(octet, crc) (crc_32_tab[((crc) ^ static_cast<uint8_t>(octet)) & 0xff] ^ ((crc) >> 8))

namespace
{
    typedef int mz_bool;
    typedef signed short mz_int16;
    typedef int64_t mz_int64;
    typedef unsigned char mz_uint8;
    typedef unsigned short mz_uint16;
    typedef unsigned int mz_uint32;
    typedef  unsigned long long mz_uint64;

    typedef void * ( *mz_alloc_func )( void * opaque, size_t items, size_t size );
    typedef void ( *mz_free_func )( void * opaque, void * address );
    typedef void * ( *mz_realloc_func )( void * opaque, void * address, size_t items, size_t size );
    typedef mz_bool( *tdefl_put_buf_func_ptr )( const void * pBuf, int len, void * pUser );
    typedef int ( *tinfl_put_buf_func_ptr )( const void * pBuf, int len, void * pUser );
    typedef size_t( *mz_file_read_func )( void * pOpaque, mz_uint64 file_ofs, void * pBuf, size_t n );
    typedef size_t( *mz_file_write_func )( void * pOpaque, mz_uint64 file_ofs, const void * pBuf, size_t n );
    typedef mz_bool( *mz_file_needs_keepalive )( void * pOpaque );

    enum
    {
        MZ_DEFAULT_STRATEGY = 0,
        MZ_FILTERED = 1,
        MZ_HUFFMAN_ONLY = 2,
        MZ_RLE = 3,
        MZ_FIXED = 4,
        MZ_DEFLATED = 8,
        MZ_DEFAULT_LEVEL = 6,
        MZ_BEST_COMPRESSION = 9,
        MZ_UBER_COMPRESSION = 10,
    };
    enum
    {
        TDEFL_MAX_PROBES_MASK = 0xFFF,
        TDEFL_WRITE_ZLIB_HEADER = 0x01000,
        TDEFL_COMPUTE_ADLER32 = 0x02000,
        TDEFL_GREEDY_PARSING_FLAG = 0x04000,
        TDEFL_NONDETERMINISTIC_PARSING_FLAG = 0x08000,
        TDEFL_RLE_MATCHES = 0x10000,
        TDEFL_FILTER_MATCHES = 0x20000,
        TDEFL_FORCE_ALL_STATIC_BLOCKS = 0x40000,
        TDEFL_FORCE_ALL_RAW_BLOCKS = 0x80000
    };
    enum
    {
        TDEFL_MAX_HUFF_TABLES = 3,
        TDEFL_MAX_HUFF_SYMBOLS_0 = 288,
        TDEFL_MAX_HUFF_SYMBOLS_1 = 32,
        TDEFL_MAX_HUFF_SYMBOLS_2 = 19,
        TDEFL_LZ_DICT_SIZE = 32768,
        TDEFL_LZ_DICT_SIZE_MASK = TDEFL_LZ_DICT_SIZE - 1,
        TDEFL_MIN_MATCH_LEN = 3,
        TDEFL_MAX_MATCH_LEN = 258
    };
    enum
    {
        TDEFL_LZ_CODE_BUF_SIZE = 64 * 1024,
        TDEFL_OUT_BUF_SIZE = ( TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10,
        TDEFL_MAX_HUFF_SYMBOLS = 288,
        TDEFL_LZ_HASH_BITS = 15,
        TDEFL_LEVEL1_HASH_SIZE_MASK = 4095,
        TDEFL_LZ_HASH_SHIFT = ( TDEFL_LZ_HASH_BITS + 2 ) / 3,
        TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS
    };
    enum
    {
        TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
        TINFL_FLAG_HAS_MORE_INPUT = 2,
        TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,
        TINFL_FLAG_COMPUTE_ADLER32 = 8
    };
    enum
    {
        TINFL_MAX_HUFF_TABLES = 3,
        TINFL_MAX_HUFF_SYMBOLS_0 = 288,
        TINFL_MAX_HUFF_SYMBOLS_1 = 32,
        TINFL_MAX_HUFF_SYMBOLS_2 = 19,
        TINFL_FAST_LOOKUP_BITS = 10,
        TINFL_FAST_LOOKUP_SIZE = 1 << TINFL_FAST_LOOKUP_BITS
    };
    enum
    {
        MZ_ZIP_MAX_IO_BUF_SIZE = 64 * 1024,
        MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE = 512,
        MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE = 512
    };
    enum
    {
        TDEFL_MAX_SUPPORTED_HUFF_CODESIZE = 32
    };
    enum
    {

        MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG = 0x06054b50,
        MZ_ZIP_CENTRAL_DIR_HEADER_SIG = 0x02014b50,
        MZ_ZIP_LOCAL_DIR_HEADER_SIG = 0x04034b50,
        MZ_ZIP_LOCAL_DIR_HEADER_SIZE = 30,
        MZ_ZIP_CENTRAL_DIR_HEADER_SIZE = 46,
        MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE = 22,
        MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIG = 0x06064b50,
        MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG = 0x07064b50,
        MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE = 56,
        MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE = 20,
        MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID = 0x0001,
        MZ_ZIP_DATA_DESCRIPTOR_ID = 0x08074b50,
        MZ_ZIP_DATA_DESCRIPTER_SIZE64 = 24,
        MZ_ZIP_DATA_DESCRIPTER_SIZE32 = 16,
        MZ_ZIP_CDH_SIG_OFS = 0,
        MZ_ZIP_CDH_VERSION_MADE_BY_OFS = 4,
        MZ_ZIP_CDH_VERSION_NEEDED_OFS = 6,
        MZ_ZIP_CDH_BIT_FLAG_OFS = 8,
        MZ_ZIP_CDH_METHOD_OFS = 10,
        MZ_ZIP_CDH_FILE_TIME_OFS = 12,
        MZ_ZIP_CDH_FILE_DATE_OFS = 14,
        MZ_ZIP_CDH_CRC32_OFS = 16,
        MZ_ZIP_CDH_COMPRESSED_SIZE_OFS = 20,
        MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS = 24,
        MZ_ZIP_CDH_FILENAME_LEN_OFS = 28,
        MZ_ZIP_CDH_EXTRA_LEN_OFS = 30,
        MZ_ZIP_CDH_COMMENT_LEN_OFS = 32,
        MZ_ZIP_CDH_DISK_START_OFS = 34,
        MZ_ZIP_CDH_INTERNAL_ATTR_OFS = 36,
        MZ_ZIP_CDH_EXTERNAL_ATTR_OFS = 38,
        MZ_ZIP_CDH_LOCAL_HEADER_OFS = 42,
        MZ_ZIP_LDH_SIG_OFS = 0,
        MZ_ZIP_LDH_VERSION_NEEDED_OFS = 4,
        MZ_ZIP_LDH_BIT_FLAG_OFS = 6,
        MZ_ZIP_LDH_METHOD_OFS = 8,
        MZ_ZIP_LDH_FILE_TIME_OFS = 10,
        MZ_ZIP_LDH_FILE_DATE_OFS = 12,
        MZ_ZIP_LDH_CRC32_OFS = 14,
        MZ_ZIP_LDH_COMPRESSED_SIZE_OFS = 18,
        MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS = 22,
        MZ_ZIP_LDH_FILENAME_LEN_OFS = 26,
        MZ_ZIP_LDH_EXTRA_LEN_OFS = 28,
        MZ_ZIP_LDH_BIT_FLAG_HAS_LOCATOR = 1 << 3,
        MZ_ZIP_ECDH_SIG_OFS = 0,
        MZ_ZIP_ECDH_NUM_THIS_DISK_OFS = 4,
        MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS = 6,
        MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS = 8,
        MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS = 10,
        MZ_ZIP_ECDH_CDIR_SIZE_OFS = 12,
        MZ_ZIP_ECDH_CDIR_OFS_OFS = 16,
        MZ_ZIP_ECDH_COMMENT_SIZE_OFS = 20,
        MZ_ZIP64_ECDL_SIG_OFS = 0,
        MZ_ZIP64_ECDL_NUM_DISK_CDIR_OFS = 4,
        MZ_ZIP64_ECDL_REL_OFS_TO_ZIP64_ECDR_OFS = 8,
        MZ_ZIP64_ECDL_TOTAL_NUMBER_OF_DISKS_OFS = 16,
        MZ_ZIP64_ECDH_SIG_OFS = 0,
        MZ_ZIP64_ECDH_SIZE_OF_RECORD_OFS = 4,
        MZ_ZIP64_ECDH_VERSION_MADE_BY_OFS = 12,
        MZ_ZIP64_ECDH_VERSION_NEEDED_OFS = 14,
        MZ_ZIP64_ECDH_NUM_THIS_DISK_OFS = 16,
        MZ_ZIP64_ECDH_NUM_DISK_CDIR_OFS = 20,
        MZ_ZIP64_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS = 24,
        MZ_ZIP64_ECDH_CDIR_TOTAL_ENTRIES_OFS = 32,
        MZ_ZIP64_ECDH_CDIR_SIZE_OFS = 40,
        MZ_ZIP64_ECDH_CDIR_OFS_OFS = 48,
        MZ_ZIP_VERSION_MADE_BY_DOS_FILESYSTEM_ID = 0,
        MZ_ZIP_DOS_DIR_ATTRIBUTE_BITFLAG = 0x10,
        MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED = 1,
        MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_COMPRESSED_PATCH_FLAG = 32,
        MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION = 64,
        MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_LOCAL_DIR_IS_MASKED = 8192,
        MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_UTF8 = 1 << 11
    };
    typedef enum
    {
        TDEFL_STATUS_BAD_PARAM = -2,
        TDEFL_STATUS_PUT_BUF_FAILED = -1,
        TDEFL_STATUS_OKAY = 0,
        TDEFL_STATUS_DONE = 1
    } tdefl_status;
    typedef enum
    {
        TDEFL_NO_FLUSH = 0,
        TDEFL_SYNC_FLUSH = 2,
        TDEFL_FULL_FLUSH = 3,
        TDEFL_FINISH = 4
    } tdefl_flush;
    typedef enum
    {
        TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS = -4,
        TINFL_STATUS_BAD_PARAM = -3,
        TINFL_STATUS_ADLER32_MISMATCH = -2,
        TINFL_STATUS_FAILED = -1,
        TINFL_STATUS_DONE = 0,
        TINFL_STATUS_NEEDS_MORE_INPUT = 1,
        TINFL_STATUS_HAS_MORE_OUTPUT = 2
    } tinfl_status;
    typedef enum
    {
        MZ_ZIP_MODE_INVALID = 0,
        MZ_ZIP_MODE_READING = 1,
        MZ_ZIP_MODE_WRITING = 2,
        MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED = 3
    } mz_zip_mode;
    typedef enum
    {
        MZ_ZIP_FLAG_CASE_SENSITIVE = 0x0100,
        MZ_ZIP_FLAG_IGNORE_PATH = 0x0200,
        MZ_ZIP_FLAG_COMPRESSED_DATA = 0x0400,
        MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY = 0x0800,
        MZ_ZIP_FLAG_VALIDATE_LOCATE_FILE_FLAG = 0x1000,
        MZ_ZIP_FLAG_VALIDATE_HEADERS_ONLY = 0x2000,
        MZ_ZIP_FLAG_WRITE_ZIP64 = 0x4000,
        MZ_ZIP_FLAG_WRITE_ALLOW_READING = 0x8000,
        MZ_ZIP_FLAG_ASCII_FILENAME = 0x10000,
        MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE = 0x20000
    } mz_zip_flags;
    typedef enum
    {
        MZ_ZIP_TYPE_INVALID = 0,
        MZ_ZIP_TYPE_USER,
        MZ_ZIP_TYPE_MEMORY,
        MZ_ZIP_TYPE_HEAP,
        MZ_ZIP_TYPE_FILE,
        MZ_ZIP_TYPE_CFILE,
        MZ_ZIP_TOTAL_TYPES
    } mz_zip_type;
    typedef enum
    {
        MZ_ZIP_NO_ERROR = 0,
        MZ_ZIP_UNDEFINED_ERROR,
        MZ_ZIP_TOO_MANY_FILES,
        MZ_ZIP_FILE_TOO_LARGE,
        MZ_ZIP_UNSUPPORTED_METHOD,
        MZ_ZIP_UNSUPPORTED_ENCRYPTION,
        MZ_ZIP_UNSUPPORTED_FEATURE,
        MZ_ZIP_FAILED_FINDING_CENTRAL_DIR,
        MZ_ZIP_NOT_AN_ARCHIVE,
        MZ_ZIP_INVALID_HEADER_OR_CORRUPTED,
        MZ_ZIP_UNSUPPORTED_MULTIDISK,
        MZ_ZIP_DECOMPRESSION_FAILED,
        MZ_ZIP_COMPRESSION_FAILED,
        MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE,
        MZ_ZIP_CRC_CHECK_FAILED,
        MZ_ZIP_UNSUPPORTED_CDIR_SIZE,
        MZ_ZIP_ALLOC_FAILED,
        MZ_ZIP_FILE_OPEN_FAILED,
        MZ_ZIP_FILE_CREATE_FAILED,
        MZ_ZIP_FILE_WRITE_FAILED,
        MZ_ZIP_FILE_READ_FAILED,
        MZ_ZIP_FILE_CLOSE_FAILED,
        MZ_ZIP_FILE_SEEK_FAILED,
        MZ_ZIP_FILE_STAT_FAILED,
        MZ_ZIP_INVALID_PARAMETER,
        MZ_ZIP_INVALID_FILENAME,
        MZ_ZIP_BUF_TOO_SMALL,
        MZ_ZIP_INTERNAL_ERROR,
        MZ_ZIP_FILE_NOT_FOUND,
        MZ_ZIP_ARCHIVE_TOO_LARGE,
        MZ_ZIP_VALIDATION_FAILED,
        MZ_ZIP_WRITE_CALLBACK_FAILED,
        MZ_ZIP_TOTAL_ERRORS
    } mz_zip_error;

    typedef struct
    {
        tdefl_put_buf_func_ptr m_pPut_buf_func;
        void * m_pPut_buf_user;
        mz_uint32 m_flags, m_max_probes[2];
        int m_greedy_parsing;
        mz_uint32 m_adler32, m_lookahead_pos, m_lookahead_size, m_dict_size;
        mz_uint8 * m_pLZ_code_buf, * m_pLZ_flags, * m_pOutput_buf, * m_pOutput_buf_end;
        mz_uint32 m_num_flags_left, m_total_lz_bytes, m_lz_code_buf_dict_pos, m_bits_in, m_bit_buffer;
        mz_uint32 m_saved_match_dist, m_saved_match_len, m_saved_lit, m_output_flush_ofs, m_output_flush_remaining, m_finished, m_block_index, m_wants_to_finish;
        tdefl_status m_prev_return_status;
        const void * m_pIn_buf;
        void * m_pOut_buf;
        size_t * m_pIn_buf_size, * m_pOut_buf_size;
        tdefl_flush m_flush;
        const mz_uint8 * m_pSrc;
        size_t m_src_buf_left, m_out_buf_ofs;
        mz_uint8 m_dict[TDEFL_LZ_DICT_SIZE + TDEFL_MAX_MATCH_LEN - 1];
        mz_uint16 m_huff_count[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
        mz_uint16 m_huff_codes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
        mz_uint8 m_huff_code_sizes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
        mz_uint8 m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE];
        mz_uint16 m_next[TDEFL_LZ_DICT_SIZE];
        mz_uint16 m_hash[TDEFL_LZ_HASH_SIZE];
        mz_uint8 m_output_buf[TDEFL_OUT_BUF_SIZE];
    } tdefl_compressor;
    typedef struct
    {
        mz_uint8 m_code_size[TINFL_MAX_HUFF_SYMBOLS_0];
        mz_int16 m_look_up[TINFL_FAST_LOOKUP_SIZE], m_tree[TINFL_MAX_HUFF_SYMBOLS_0 * 2];
    } tinfl_huff_table;
    typedef struct
    {
        mz_uint32 m_state, m_num_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type, m_check_adler32, m_dist, m_counter, m_num_extra,
            m_table_sizes[TINFL_MAX_HUFF_TABLES];
        mz_uint32 m_bit_buf;
        size_t m_dist_from_out_buf_start;
        tinfl_huff_table m_tables[TINFL_MAX_HUFF_TABLES];
        mz_uint8 m_raw_header[4], m_len_codes[TINFL_MAX_HUFF_SYMBOLS_0 + TINFL_MAX_HUFF_SYMBOLS_1 + 137];
    } tinfl_decompressor;
    typedef struct
    {

        mz_uint32 m_file_index;
        mz_uint64 m_central_dir_ofs;
        mz_uint16 m_version_made_by;
        mz_uint16 m_version_needed;
        mz_uint16 m_bit_flag;
        mz_uint16 m_method;
        time_t m_time;
        mz_uint32 m_crc32;
        mz_uint64 m_comp_size;

        mz_uint64 m_uncomp_size;
        mz_uint16 m_internal_attr;
        mz_uint32 m_external_attr;
        mz_uint64 m_local_header_ofs;
        mz_uint32 m_comment_size;
        mz_bool m_is_directory;
        mz_bool m_is_encrypted;
        mz_bool m_is_supported;

        char m_filename[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];

        char m_comment[MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE];

    } mz_zip_archive_file_stat;
    typedef struct
    {
        void * m_p;
        size_t m_size, m_capacity;
        mz_uint32 m_element_size;
    } mz_zip_array;
    typedef struct
    {
        mz_zip_array m_central_dir;
        mz_zip_array m_central_dir_offsets;
        mz_zip_array m_sorted_central_dir_offsets;
        uint32_t m_init_flags;
        mz_bool m_zip64;

        mz_bool m_zip64_has_extended_info_fields;
        FILE * m_pFile;
        mz_uint64 m_file_archive_start_ofs;

        void * m_pMem;
        size_t m_mem_size;
        size_t m_mem_capacity;
    } mz_zip_internal_state;
    typedef struct
    {
        mz_uint64 m_archive_size;
        mz_uint64 m_central_directory_file_ofs;
        mz_uint32 m_total_files;
        mz_zip_mode m_zip_mode;
        mz_zip_type m_zip_type;
        mz_zip_error m_last_error;

        mz_uint64 m_file_offset_alignment;

        mz_alloc_func m_pAlloc;
        mz_free_func m_pFree;
        mz_realloc_func m_pRealloc;
        void * m_pAlloc_opaque;

        mz_file_read_func m_pRead;
        mz_file_write_func m_pWrite;
        mz_file_needs_keepalive m_pNeeds_keepalive;
        void * m_pIO_opaque;

        mz_zip_internal_state * m_pState;

    } mz_zip_archive;
    typedef struct
    {
        mz_zip_archive * pZip;
        mz_uint32 flags;

        int status;
        mz_uint32 file_crc32;
        mz_uint64 read_buf_size, read_buf_ofs, read_buf_avail, comp_remaining, out_buf_ofs, cur_file_ofs;
        mz_zip_archive_file_stat file_stat;
        void * pRead_buf;
        void * pWrite_buf;

        size_t out_blk_remain;

        tinfl_decompressor inflator;

    } mz_zip_reader_extract_iter_state;
    typedef struct
    {
        mz_uint16 m_key, m_sym_index;
    } tdefl_sym_freq;
    typedef struct
    {
        size_t m_size, m_capacity;
        mz_uint8 * m_pBuf;
        mz_bool m_expandable;
    } tdefl_output_buffer;
    typedef struct
    {
        mz_zip_archive * m_pZip;
        mz_uint64 m_cur_archive_file_ofs;
        mz_uint64 m_comp_size;
    } mz_zip_writer_add_state;

    typedef unsigned char mz_validate_uint16[sizeof( mz_uint16 ) == 2 ? 1 : -1];
    typedef unsigned char mz_validate_uint32[sizeof( mz_uint32 ) == 4 ? 1 : -1];
    typedef unsigned char mz_validate_uint64[sizeof( mz_uint64 ) == 8 ? 1 : -1];

    static const mz_uint16 s_tdefl_len_sym[256] = { 257, 258, 259, 260, 261, 262, 263, 264, 265, 265, 266, 266, 267, 267, 268, 268, 269, 269, 269, 269, 270,
        270, 270, 270, 271, 271, 271, 271, 272, 272, 272, 272, 273, 273, 273, 273, 273, 273, 273, 273, 274, 274, 274, 274, 274, 274, 274, 274, 275, 275, 275,
        275, 275, 275, 275, 275, 276, 276, 276, 276, 276, 276, 276, 276, 277, 277, 277, 277, 277, 277, 277, 277, 277, 277, 277, 277, 277, 277, 277, 277, 278,
        278, 278, 278, 278, 278, 278, 278, 278, 278, 278, 278, 278, 278, 278, 278, 279, 279, 279, 279, 279, 279, 279, 279, 279, 279, 279, 279, 279, 279, 279,
        279, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281,
        281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282,
        282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 283, 283, 283, 283, 283, 283, 283, 283, 283,
        283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 284, 284, 284, 284, 284, 284, 284,
        284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 285 };
    static const mz_uint8 s_tdefl_len_extra[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0 };
    static const mz_uint8 s_tdefl_small_dist_sym[512] = { 0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 10,
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12,
        12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
        13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
        14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
        14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
        17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
        17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
        17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17 };
    static const mz_uint8 s_tdefl_small_dist_extra[512] = { 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 };
    static const mz_uint8 s_tdefl_large_dist_sym[128] = { 0, 0, 18, 19, 20, 20, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25,
        25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
        28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29,
        29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29 };
    static const mz_uint8 s_tdefl_large_dist_extra[128] = { 0, 0, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
        11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13,
        13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
        13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13 };
    static const mz_uint8 s_tdefl_packed_code_size_syms_swizzle[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
    static const mz_uint32 mz_bitmasks[17] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };
    static const mz_uint32 s_tdefl_num_probes[11] = { 0, 1, 6, 32, 16, 32, 128, 256, 512, 768, 1500 };

    void * tdefl_compress_mem_to_heap( const void * pSrc_buf, size_t src_buf_len, size_t * pOut_len, int flags );
    size_t tdefl_compress_mem_to_mem( void * pOut_buf, size_t out_buf_len, const void * pSrc_buf, size_t src_buf_len, int flags );
    void * tdefl_write_image_to_png_file_in_memory_ex( const void * pImage, int w, int h, int num_chans, size_t * pLen_out, mz_uint32 level, mz_bool flip );
    void * tdefl_write_image_to_png_file_in_memory( const void * pImage, int w, int h, int num_chans, size_t * pLen_out );
    mz_bool tdefl_compress_mem_to_output( const void * pBuf, size_t buf_len, tdefl_put_buf_func_ptr pPut_buf_func, void * pPut_buf_user, int flags );
    tdefl_status tdefl_init( tdefl_compressor * d, tdefl_put_buf_func_ptr pPut_buf_func, void * pPut_buf_user, int flags );
    tdefl_status tdefl_compress( tdefl_compressor * d, const void * pIn_buf, size_t * pIn_buf_size, void * pOut_buf, size_t * pOut_buf_size, tdefl_flush flush );
    tdefl_status tdefl_compress_buffer( tdefl_compressor * d, const void * pIn_buf, size_t in_buf_size, tdefl_flush flush );
    tdefl_status tdefl_get_prev_return_status( tdefl_compressor * d );
    mz_uint32 tdefl_get_adler32( tdefl_compressor * d );
    mz_uint32 tdefl_create_comp_flags_from_zip_params( int level, int window_bits, int strategy );
    tdefl_compressor * tdefl_compressor_alloc( void );
    void tdefl_compressor_free( tdefl_compressor * pComp );
    void * tinfl_decompress_mem_to_heap( const void * pSrc_buf, size_t src_buf_len, size_t * pOut_len, int flags );
    size_t tinfl_decompress_mem_to_mem( void * pOut_buf, size_t out_buf_len, const void * pSrc_buf, size_t src_buf_len, int flags );
    int tinfl_decompress_mem_to_callback( const void * pIn_buf, size_t * pIn_buf_size, tinfl_put_buf_func_ptr pPut_buf_func, void * pPut_buf_user, int flags );
    tinfl_decompressor * tinfl_decompressor_alloc( void );
    void tinfl_decompressor_free( tinfl_decompressor * pDecomp );
    tinfl_status tinfl_decompress( tinfl_decompressor * r, const mz_uint8 * pIn_buf_next, size_t * pIn_buf_size, mz_uint8 * pOut_buf_start, mz_uint8 * pOut_buf_next, size_t * pOut_buf_size, const mz_uint32 decomp_flags );
    mz_bool mz_zip_reader_init( mz_zip_archive * pZip, mz_uint64 size, mz_uint32 flags );
    mz_bool mz_zip_reader_init_mem( mz_zip_archive * pZip, const void * pMem, size_t size, mz_uint32 flags );
    mz_bool mz_zip_reader_init_file( mz_zip_archive * pZip, const char * pFilename, mz_uint32 flags );
    mz_bool mz_zip_reader_init_file_v2( mz_zip_archive * pZip, const char * pFilename, mz_uint32 flags, mz_uint64 file_start_ofs, mz_uint64 archive_size );
    mz_bool mz_zip_reader_init_cfile( mz_zip_archive * pZip, FILE * pFile, mz_uint64 archive_size, mz_uint32 flags );
    mz_bool mz_zip_reader_end( mz_zip_archive * pZip );
    void mz_zip_zero_struct( mz_zip_archive * pZip );
    mz_zip_mode mz_zip_get_mode( mz_zip_archive * pZip );
    mz_zip_type mz_zip_get_type( mz_zip_archive * pZip );
    mz_uint32 mz_zip_reader_get_num_files( mz_zip_archive * pZip );
    mz_uint64 mz_zip_get_archive_size( mz_zip_archive * pZip );
    mz_uint64 mz_zip_get_archive_file_start_offset( mz_zip_archive * pZip );
    FILE * mz_zip_get_cfile( mz_zip_archive * pZip );
    size_t mz_zip_read_archive_data( mz_zip_archive * pZip, mz_uint64 file_ofs, void * pBuf, size_t n );
    mz_zip_error mz_zip_set_last_error( mz_zip_archive * pZip, mz_zip_error err_num );
    mz_zip_error mz_zip_peek_last_error( mz_zip_archive * pZip );
    mz_zip_error mz_zip_clear_last_error( mz_zip_archive * pZip );
    mz_zip_error mz_zip_get_last_error( mz_zip_archive * pZip );
    const char * mz_zip_get_error_string( mz_zip_error mz_err );
    mz_bool mz_zip_reader_is_file_a_directory( mz_zip_archive * pZip, mz_uint32 file_index );
    mz_bool mz_zip_reader_is_file_encrypted( mz_zip_archive * pZip, mz_uint32 file_index );
    mz_bool mz_zip_reader_is_file_supported( mz_zip_archive * pZip, mz_uint32 file_index );
    mz_uint32 mz_zip_reader_get_filename( mz_zip_archive * pZip, mz_uint32 file_index, char * pFilename, mz_uint32 filename_buf_size );
    int mz_zip_reader_locate_file( mz_zip_archive * pZip, const char * pName, const char * pComment, mz_uint32 flags );
    mz_bool mz_zip_reader_locate_file_v2( mz_zip_archive * pZip, const char * pName, const char * pComment, mz_uint32 flags, mz_uint32 * file_index );
    mz_bool mz_zip_reader_file_stat( mz_zip_archive * pZip, mz_uint32 file_index, mz_zip_archive_file_stat * pStat );
    mz_bool mz_zip_is_zip64( mz_zip_archive * pZip );
    size_t mz_zip_get_central_dir_size( mz_zip_archive * pZip );
    mz_bool mz_zip_reader_extract_to_mem_no_alloc( mz_zip_archive * pZip, mz_uint32 file_index, void * pBuf, size_t buf_size, mz_uint32 flags, void * pUser_read_buf, size_t user_read_buf_size );
    mz_bool mz_zip_reader_extract_file_to_mem_no_alloc( mz_zip_archive * pZip, const char * pFilename, void * pBuf, size_t buf_size, mz_uint32 flags, void * pUser_read_buf, size_t user_read_buf_size );
    mz_bool mz_zip_reader_extract_to_mem( mz_zip_archive * pZip, mz_uint32 file_index, void * pBuf, size_t buf_size, mz_uint32 flags );
    mz_bool mz_zip_reader_extract_file_to_mem( mz_zip_archive * pZip, const char * pFilename, void * pBuf, size_t buf_size, mz_uint32 flags );
    void * mz_zip_reader_extract_to_heap( mz_zip_archive * pZip, mz_uint32 file_index, size_t * pSize, mz_uint32 flags );
    void * mz_zip_reader_extract_file_to_heap( mz_zip_archive * pZip, const char * pFilename, size_t * pSize, mz_uint32 flags );
    mz_bool mz_zip_reader_extract_to_callback( mz_zip_archive * pZip, mz_uint32 file_index, mz_file_write_func pCallback, void * pOpaque, mz_uint32 flags );
    mz_bool mz_zip_reader_extract_file_to_callback( mz_zip_archive * pZip, const char * pFilename, mz_file_write_func pCallback, void * pOpaque, mz_uint32 flags );
    mz_zip_reader_extract_iter_state * mz_zip_reader_extract_iter_new( mz_zip_archive * pZip, mz_uint32 file_index, mz_uint32 flags );
    mz_zip_reader_extract_iter_state * mz_zip_reader_extract_file_iter_new( mz_zip_archive * pZip, const char * pFilename, mz_uint32 flags );
    size_t mz_zip_reader_extract_iter_read( mz_zip_reader_extract_iter_state * pState, void * pvBuf, size_t buf_size );
    mz_bool mz_zip_reader_extract_iter_free( mz_zip_reader_extract_iter_state * pState );
    mz_bool mz_zip_reader_extract_to_file( mz_zip_archive * pZip, mz_uint32 file_index, const char * pDst_filename, mz_uint32 flags );
    mz_bool mz_zip_reader_extract_file_to_file( mz_zip_archive * pZip, const char * pArchive_filename, const char * pDst_filename, mz_uint32 flags );
    mz_bool mz_zip_reader_extract_to_cfile( mz_zip_archive * pZip, mz_uint32 file_index, FILE * File, mz_uint32 flags );
    mz_bool mz_zip_reader_extract_file_to_cfile( mz_zip_archive * pZip, const char * pArchive_filename, FILE * pFile, mz_uint32 flags );
    mz_bool mz_zip_validate_file( mz_zip_archive * pZip, mz_uint32 file_index, mz_uint32 flags );
    mz_bool mz_zip_validate_archive( mz_zip_archive * pZip, mz_uint32 flags );
    mz_bool mz_zip_validate_mem_archive( const void * pMem, size_t size, mz_uint32 flags, mz_zip_error * pErr );
    mz_bool mz_zip_validate_file_archive( const char * pFilename, mz_uint32 flags, mz_zip_error * pErr );
    mz_bool mz_zip_end( mz_zip_archive * pZip );
    mz_bool mz_zip_writer_init( mz_zip_archive * pZip, mz_uint64 existing_size );
    mz_bool mz_zip_writer_init_v2( mz_zip_archive * pZip, mz_uint64 existing_size, mz_uint32 flags );
    mz_bool mz_zip_writer_init_heap( mz_zip_archive * pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size );
    mz_bool mz_zip_writer_init_heap_v2( mz_zip_archive * pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size, mz_uint32 flags );
    mz_bool mz_zip_writer_init_file( mz_zip_archive * pZip, const char * pFilename, mz_uint64 size_to_reserve_at_beginning );
    mz_bool mz_zip_writer_init_file_v2( mz_zip_archive * pZip, const char * pFilename, mz_uint64 size_to_reserve_at_beginning, mz_uint32 flags );
    mz_bool mz_zip_writer_init_cfile( mz_zip_archive * pZip, FILE * pFile, mz_uint32 flags );
    mz_bool mz_zip_writer_init_from_reader( mz_zip_archive * pZip, const char * pFilename );
    mz_bool mz_zip_writer_init_from_reader_v2( mz_zip_archive * pZip, const char * pFilename, mz_uint32 flags );
    mz_bool mz_zip_writer_add_mem( mz_zip_archive * pZip, const char * pArchive_name, const void * pBuf, size_t buf_size, mz_uint32 level_and_flags );
    mz_bool mz_zip_writer_add_mem_ex( mz_zip_archive * pZip, const char * pArchive_name, const void * pBuf, size_t buf_size, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32 );
    mz_bool mz_zip_writer_add_mem_ex_v2( mz_zip_archive * pZip, const char * pArchive_name, const void * pBuf, size_t buf_size, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32, time_t * last_modified, const char * user_extra_data_local, mz_uint32 user_extra_data_local_len, const char * user_extra_data_central, mz_uint32 user_extra_data_central_len );
    mz_bool mz_zip_writer_add_read_buf_callback( mz_zip_archive * pZip, const char * pArchive_name, mz_file_read_func read_callback, void * callback_opaque, mz_uint64 max_size, const time_t * pFile_time, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags, const char * user_extra_data_local, mz_uint32 user_extra_data_local_len, const char * user_extra_data_central, mz_uint32 user_extra_data_central_len );
    mz_bool mz_zip_writer_add_file( mz_zip_archive * pZip, const char * pArchive_name, const char * pSrc_filename, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags );
    mz_bool mz_zip_writer_add_cfile( mz_zip_archive * pZip, const char * pArchive_name, FILE * pSrc_file, mz_uint64 max_size, const time_t * pFile_time, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags, const char * user_extra_data_local, mz_uint32 user_extra_data_local_len, const char * user_extra_data_central, mz_uint32 user_extra_data_central_len );
    mz_bool mz_zip_writer_add_from_zip_reader( mz_zip_archive * pZip, mz_zip_archive * pSource_zip, mz_uint32 src_file_index );
    mz_bool mz_zip_writer_finalize_archive( mz_zip_archive * pZip );
    mz_bool mz_zip_writer_finalize_heap_archive( mz_zip_archive * pZip, void ** ppBuf, size_t * pSize );
    mz_bool mz_zip_writer_end( mz_zip_archive * pZip );
    mz_bool mz_zip_add_mem_to_archive_file_in_place( const char * pZip_filename, const char * pArchive_name, const void * pBuf, size_t buf_size, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags );
    mz_bool mz_zip_add_mem_to_archive_file_in_place_v2( const char * pZip_filename, const char * pArchive_name, const void * pBuf, size_t buf_size, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags, mz_zip_error * pErr );
    void * mz_zip_extract_archive_file_to_heap( const char * pZip_filename, const char * pArchive_name, size_t * pSize, mz_uint32 flags );
    void * mz_zip_extract_archive_file_to_heap_v2( const char * pZip_filename, const char * pArchive_name, const char * pComment, size_t * pSize, mz_uint32 flags, mz_zip_error * pErr );


    mz_uint32 mz_adler32( mz_uint32 adler, const unsigned char * ptr, size_t buf_len )
    {
        mz_uint32 i, s1 = (mz_uint32)( adler & 0xffff ), s2 = (mz_uint32)( adler >> 16 );
        size_t block_len = buf_len % 5552;
        if ( !ptr )
            return 1;
        while ( buf_len )
        {
            for ( i = 0; i + 7 < block_len; i += 8, ptr += 8 )
            {
                s1 += ptr[0], s2 += s1;
                s1 += ptr[1], s2 += s1;
                s1 += ptr[2], s2 += s1;
                s1 += ptr[3], s2 += s1;
                s1 += ptr[4], s2 += s1;
                s1 += ptr[5], s2 += s1;
                s1 += ptr[6], s2 += s1;
                s1 += ptr[7], s2 += s1;
            }
            for ( ; i < block_len; ++i )
                s1 += *ptr++, s2 += s1;
            s1 %= 65521U, s2 %= 65521U;
            buf_len -= block_len;
            block_len = 5552;
        }
        return ( s2 << 16 ) + s1;
    }
    mz_uint32 mz_crc32( mz_uint32 crc, const mz_uint8 * ptr, size_t buf_len )
    {
        static const mz_uint32 s_crc_table[256] = { 0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832,
            0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D,
            0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8,
            0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3,
            0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E,
            0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589,
            0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4,
            0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF,
            0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A,
            0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525,
            0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320,
            0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B,
            0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76,
            0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1,
            0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C,
            0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7,
            0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82,
            0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD,
            0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278,
            0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53,
            0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E,
            0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D };

        mz_uint32 crc32 = (mz_uint32)crc ^ 0xFFFFFFFF;
        const mz_uint8 * pByte_buf = (const mz_uint8 *)ptr;

        while ( buf_len >= 4 )
        {
            crc32 = ( crc32 >> 8 ) ^ s_crc_table[( crc32 ^ pByte_buf[0] ) & 0xFF];
            crc32 = ( crc32 >> 8 ) ^ s_crc_table[( crc32 ^ pByte_buf[1] ) & 0xFF];
            crc32 = ( crc32 >> 8 ) ^ s_crc_table[( crc32 ^ pByte_buf[2] ) & 0xFF];
            crc32 = ( crc32 >> 8 ) ^ s_crc_table[( crc32 ^ pByte_buf[3] ) & 0xFF];
            pByte_buf += 4;
            buf_len -= 4;
        }

        while ( buf_len )
        {
            crc32 = ( crc32 >> 8 ) ^ s_crc_table[( crc32 ^ pByte_buf[0] ) & 0xFF];
            ++pByte_buf;
            --buf_len;
        }

        return ~crc32;
    }
    void * miniz_def_alloc_func( void * opaque, size_t items, size_t size )
    {
        (void)opaque, (void)items, (void)size;
        return malloc( items * size );
    }
    void miniz_def_free_func( void * opaque, void * address )
    {
        (void)opaque, (void)address;
        free( address );
    }
    void * miniz_def_realloc_func( void * opaque, void * address, size_t items, size_t size )
    {
        (void)opaque, (void)address, (void)items, (void)size;
        return realloc( address, items * size );
    }
    static tdefl_sym_freq * tdefl_radix_sort_syms( mz_uint32 num_syms, tdefl_sym_freq * pSyms0, tdefl_sym_freq * pSyms1 )
    {
        mz_uint32 total_passes = 2, pass_shift, pass, i, hist[256 * 2];
        tdefl_sym_freq * pCur_syms = pSyms0, * pNew_syms = pSyms1;
        MZ_CLEAR_OBJ( hist );
        for ( i = 0; i < num_syms; i++ )
        {
            mz_uint32 freq = pSyms0[i].m_key;
            hist[freq & 0xFF]++;
            hist[256 + ( ( freq >> 8 ) & 0xFF )]++;
        }
        while ( ( total_passes > 1 ) && ( num_syms == hist[( total_passes - 1 ) * 256] ) )
            total_passes--;
        for ( pass_shift = 0, pass = 0; pass < total_passes; pass++, pass_shift += 8 )
        {
            const mz_uint32 * pHist = &hist[pass << 8];
            mz_uint32 offsets[256], cur_ofs = 0;
            for ( i = 0; i < 256; i++ )
            {
                offsets[i] = cur_ofs;
                cur_ofs += pHist[i];
            }
            for ( i = 0; i < num_syms; i++ )
                pNew_syms[offsets[( pCur_syms[i].m_key >> pass_shift ) & 0xFF]++] = pCur_syms[i];
            {
                tdefl_sym_freq * t = pCur_syms;
                pCur_syms = pNew_syms;
                pNew_syms = t;
            }
        }
        return pCur_syms;
    }
    static void tdefl_calculate_minimum_redundancy( tdefl_sym_freq * A, int n )
    {
        int root, leaf, next, avbl, used, dpth;
        if ( n == 0 )
            return;
        else if ( n == 1 )
        {
            A[0].m_key = 1;
            return;
        }
        A[0].m_key += A[1].m_key;
        root = 0;
        leaf = 2;
        for ( next = 1; next < n - 1; next++ )
        {
            if ( leaf >= n || A[root].m_key < A[leaf].m_key )
            {
                A[next].m_key = A[root].m_key;
                A[root++].m_key = (mz_uint16)next;
            }
            else
                A[next].m_key = A[leaf++].m_key;
            if ( leaf >= n || ( root < next && A[root].m_key < A[leaf].m_key ) )
            {
                A[next].m_key = (mz_uint16)( A[next].m_key + A[root].m_key );
                A[root++].m_key = (mz_uint16)next;
            }
            else
                A[next].m_key = (mz_uint16)( A[next].m_key + A[leaf++].m_key );
        }
        A[n - 2].m_key = 0;
        for ( next = n - 3; next >= 0; next-- )
            A[next].m_key = A[A[next].m_key].m_key + 1;
        avbl = 1;
        used = dpth = 0;
        root = n - 2;
        next = n - 1;
        while ( avbl > 0 )
        {
            while ( root >= 0 && (int)A[root].m_key == dpth )
            {
                used++;
                root--;
            }
            while ( avbl > used )
            {
                A[next--].m_key = (mz_uint16)( dpth );
                avbl--;
            }
            avbl = 2 * used;
            dpth++;
            used = 0;
        }
    }
    static void tdefl_huffman_enforce_max_code_size( int * pNum_codes, int code_list_len, int max_code_size )
    {
        int i;
        mz_uint32 total = 0;
        if ( code_list_len <= 1 )
            return;
        for ( i = max_code_size + 1; i <= TDEFL_MAX_SUPPORTED_HUFF_CODESIZE; i++ )
            pNum_codes[max_code_size] += pNum_codes[i];
        for ( i = max_code_size; i > 0; i-- )
            total += ( ( (mz_uint32)pNum_codes[i] ) << ( max_code_size - i ) );
        while ( total != ( 1UL << max_code_size ) )
        {
            pNum_codes[max_code_size]--;
            for ( i = max_code_size - 1; i > 0; i-- )
                if ( pNum_codes[i] )
                {
                    pNum_codes[i]--;
                    pNum_codes[i + 1] += 2;
                    break;
                }
            total--;
        }
    }
    static void tdefl_optimize_huffman_table( tdefl_compressor * d, int table_num, int table_len, int code_size_limit, int static_table )
    {
        int i, j, l, num_codes[1 + TDEFL_MAX_SUPPORTED_HUFF_CODESIZE];
        mz_uint32 next_code[TDEFL_MAX_SUPPORTED_HUFF_CODESIZE + 1];
        MZ_CLEAR_OBJ( num_codes );
        if ( static_table )
        {
            for ( i = 0; i < table_len; i++ )
                num_codes[d->m_huff_code_sizes[table_num][i]]++;
        }
        else
        {
            tdefl_sym_freq syms0[TDEFL_MAX_HUFF_SYMBOLS], syms1[TDEFL_MAX_HUFF_SYMBOLS], * pSyms;
            int num_used_syms = 0;
            const mz_uint16 * pSym_count = &d->m_huff_count[table_num][0];
            for ( i = 0; i < table_len; i++ )
                if ( pSym_count[i] )
                {
                    syms0[num_used_syms].m_key = (mz_uint16)pSym_count[i];
                    syms0[num_used_syms++].m_sym_index = (mz_uint16)i;
                }

            pSyms = tdefl_radix_sort_syms( num_used_syms, syms0, syms1 );
            tdefl_calculate_minimum_redundancy( pSyms, num_used_syms );

            for ( i = 0; i < num_used_syms; i++ )
                num_codes[pSyms[i].m_key]++;

            tdefl_huffman_enforce_max_code_size( num_codes, num_used_syms, code_size_limit );

            MZ_CLEAR_OBJ( d->m_huff_code_sizes[table_num] );
            MZ_CLEAR_OBJ( d->m_huff_codes[table_num] );
            for ( i = 1, j = num_used_syms; i <= code_size_limit; i++ )
                for ( l = num_codes[i]; l > 0; l-- )
                    d->m_huff_code_sizes[table_num][pSyms[--j].m_sym_index] = (mz_uint8)( i );
        }

        next_code[1] = 0;
        for ( j = 0, i = 2; i <= code_size_limit; i++ )
            next_code[i] = j = ( ( j + num_codes[i - 1] ) << 1 );

        for ( i = 0; i < table_len; i++ )
        {
            mz_uint32 rev_code = 0, code, code_size;
            if ( ( code_size = d->m_huff_code_sizes[table_num][i] ) == 0 )
                continue;
            code = next_code[code_size]++;
            for ( l = code_size; l > 0; l--, code >>= 1 )
                rev_code = ( rev_code << 1 ) | ( code & 1 );
            d->m_huff_codes[table_num][i] = (mz_uint16)rev_code;
        }
    }
    static void tdefl_start_dynamic_block( tdefl_compressor * d )
    {
        int num_lit_codes, num_dist_codes, num_bit_lengths;
        mz_uint32 i, total_code_sizes_to_pack, num_packed_code_sizes, rle_z_count, rle_repeat_count, packed_code_sizes_index;
        mz_uint8 code_sizes_to_pack[TDEFL_MAX_HUFF_SYMBOLS_0 + TDEFL_MAX_HUFF_SYMBOLS_1],
            packed_code_sizes[TDEFL_MAX_HUFF_SYMBOLS_0 + TDEFL_MAX_HUFF_SYMBOLS_1], prev_code_size = 0xFF;

        d->m_huff_count[0][256] = 1;

        tdefl_optimize_huffman_table( d, 0, TDEFL_MAX_HUFF_SYMBOLS_0, 15, false );
        tdefl_optimize_huffman_table( d, 1, TDEFL_MAX_HUFF_SYMBOLS_1, 15, false );

        for ( num_lit_codes = 286; num_lit_codes > 257; num_lit_codes-- )
            if ( d->m_huff_code_sizes[0][num_lit_codes - 1] )
                break;
        for ( num_dist_codes = 30; num_dist_codes > 1; num_dist_codes-- )
            if ( d->m_huff_code_sizes[1][num_dist_codes - 1] )
                break;

        memcpy( code_sizes_to_pack, &d->m_huff_code_sizes[0][0], num_lit_codes );
        memcpy( code_sizes_to_pack + num_lit_codes, &d->m_huff_code_sizes[1][0], num_dist_codes );
        total_code_sizes_to_pack = num_lit_codes + num_dist_codes;
        num_packed_code_sizes = 0;
        rle_z_count = 0;
        rle_repeat_count = 0;

        memset( &d->m_huff_count[2][0], 0, sizeof( d->m_huff_count[2][0] ) * TDEFL_MAX_HUFF_SYMBOLS_2 );
        for ( i = 0; i < total_code_sizes_to_pack; i++ )
        {
            mz_uint8 code_size = code_sizes_to_pack[i];
            if ( !code_size )
            {
                TDEFL_RLE_PREV_CODE_SIZE();
                if ( ++rle_z_count == 138 )
                {
                    TDEFL_RLE_ZERO_CODE_SIZE();
                }
            }
            else
            {
                TDEFL_RLE_ZERO_CODE_SIZE();
                if ( code_size != prev_code_size )
                {
                    TDEFL_RLE_PREV_CODE_SIZE();
                    d->m_huff_count[2][code_size] = (mz_uint16)( d->m_huff_count[2][code_size] + 1 );
                    packed_code_sizes[num_packed_code_sizes++] = code_size;
                }
                else if ( ++rle_repeat_count == 6 )
                {
                    TDEFL_RLE_PREV_CODE_SIZE();
                }
            }
            prev_code_size = code_size;
        }
        if ( rle_repeat_count )
        {
            TDEFL_RLE_PREV_CODE_SIZE();
        }
        else
        {
            TDEFL_RLE_ZERO_CODE_SIZE();
        }

        tdefl_optimize_huffman_table( d, 2, TDEFL_MAX_HUFF_SYMBOLS_2, 7, false );

        TDEFL_PUT_BITS( 2, 2 );

        TDEFL_PUT_BITS( num_lit_codes - 257, 5 );
        TDEFL_PUT_BITS( num_dist_codes - 1, 5 );

        for ( num_bit_lengths = 18; num_bit_lengths >= 0; num_bit_lengths-- )
            if ( d->m_huff_code_sizes[2][s_tdefl_packed_code_size_syms_swizzle[num_bit_lengths]] )
                break;
        num_bit_lengths = MZ_MAX( 4, ( num_bit_lengths + 1 ) );
        TDEFL_PUT_BITS( num_bit_lengths - 4, 4 );
        for ( i = 0; (int)i < num_bit_lengths; i++ )
            TDEFL_PUT_BITS( d->m_huff_code_sizes[2][s_tdefl_packed_code_size_syms_swizzle[i]], 3 );

        for ( packed_code_sizes_index = 0; packed_code_sizes_index < num_packed_code_sizes;)
        {
            mz_uint32 code = packed_code_sizes[packed_code_sizes_index++];
            assert( code < TDEFL_MAX_HUFF_SYMBOLS_2 );
            TDEFL_PUT_BITS( d->m_huff_codes[2][code], d->m_huff_code_sizes[2][code] );
            if ( code >= 16 )
                TDEFL_PUT_BITS( packed_code_sizes[packed_code_sizes_index++], "\02\03\07"[code - 16] );
        }
    }
    static void tdefl_start_static_block( tdefl_compressor * d )
    {
        mz_uint32 i;
        mz_uint8 * p = &d->m_huff_code_sizes[0][0];

        for ( i = 0; i <= 143; ++i )
            *p++ = 8;
        for ( ; i <= 255; ++i )
            *p++ = 9;
        for ( ; i <= 279; ++i )
            *p++ = 7;
        for ( ; i <= 287; ++i )
            *p++ = 8;

        memset( d->m_huff_code_sizes[1], 5, 32 );

        tdefl_optimize_huffman_table( d, 0, 288, 15, true );
        tdefl_optimize_huffman_table( d, 1, 32, 15, true );

        TDEFL_PUT_BITS( 1, 2 );
    }
    static mz_bool tdefl_compress_lz_codes( tdefl_compressor * d )
    {
        mz_uint32 flags;
        mz_uint8 * pLZ_codes;

        flags = 1;
        for ( pLZ_codes = d->m_lz_code_buf; pLZ_codes < d->m_pLZ_code_buf; flags >>= 1 )
        {
            if ( flags == 1 )
                flags = *pLZ_codes++ | 0x100;
            if ( flags & 1 )
            {
                mz_uint32 sym, num_extra_bits;
                mz_uint32 match_len = pLZ_codes[0], match_dist = ( pLZ_codes[1] | ( pLZ_codes[2] << 8 ) );
                pLZ_codes += 3;

                assert( d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]] );
                TDEFL_PUT_BITS( d->m_huff_codes[0][s_tdefl_len_sym[match_len]], d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]] );
                TDEFL_PUT_BITS( match_len & mz_bitmasks[s_tdefl_len_extra[match_len]], s_tdefl_len_extra[match_len] );

                if ( match_dist < 512 )
                {
                    sym = s_tdefl_small_dist_sym[match_dist];
                    num_extra_bits = s_tdefl_small_dist_extra[match_dist];
                }
                else
                {
                    sym = s_tdefl_large_dist_sym[match_dist >> 8];
                    num_extra_bits = s_tdefl_large_dist_extra[match_dist >> 8];
                }
                assert( d->m_huff_code_sizes[1][sym] );
                TDEFL_PUT_BITS( d->m_huff_codes[1][sym], d->m_huff_code_sizes[1][sym] );
                TDEFL_PUT_BITS( match_dist & mz_bitmasks[num_extra_bits], num_extra_bits );
            }
            else
            {
                mz_uint32 lit = *pLZ_codes++;
                assert( d->m_huff_code_sizes[0][lit] );
                TDEFL_PUT_BITS( d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit] );
            }
        }

        TDEFL_PUT_BITS( d->m_huff_codes[0][256], d->m_huff_code_sizes[0][256] );

        return ( d->m_pOutput_buf < d->m_pOutput_buf_end );
    }
    static mz_bool tdefl_compress_block( tdefl_compressor * d, mz_bool static_block )
    {
        if ( static_block )
            tdefl_start_static_block( d );
        else
            tdefl_start_dynamic_block( d );
        return tdefl_compress_lz_codes( d );
    }
    static int tdefl_flush_block( tdefl_compressor * d, int flush )
    {
        mz_uint32 saved_bit_buf, saved_bits_in;
        mz_uint8 * pSaved_output_buf;
        mz_bool comp_block_succeeded = false;
        int n, use_raw_block = ( ( d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS ) != 0 ) && ( d->m_lookahead_pos - d->m_lz_code_buf_dict_pos ) <= d->m_dict_size;
        mz_uint8 * pOutput_buf_start = ( ( d->m_pPut_buf_func == NULL ) && ( ( *d->m_pOut_buf_size - d->m_out_buf_ofs ) >= TDEFL_OUT_BUF_SIZE ) )
            ? ( (mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs )
            : d->m_output_buf;

        d->m_pOutput_buf = pOutput_buf_start;
        d->m_pOutput_buf_end = d->m_pOutput_buf + TDEFL_OUT_BUF_SIZE - 16;

        assert( !d->m_output_flush_remaining );
        d->m_output_flush_ofs = 0;
        d->m_output_flush_remaining = 0;

        *d->m_pLZ_flags = (mz_uint8)( *d->m_pLZ_flags >> d->m_num_flags_left );
        d->m_pLZ_code_buf -= ( d->m_num_flags_left == 8 );

        if ( ( d->m_flags & TDEFL_WRITE_ZLIB_HEADER ) && ( !d->m_block_index ) )
        {
            TDEFL_PUT_BITS( 0x78, 8 );
            TDEFL_PUT_BITS( 0x01, 8 );
        }

        TDEFL_PUT_BITS( flush == TDEFL_FINISH, 1 );

        pSaved_output_buf = d->m_pOutput_buf;
        saved_bit_buf = d->m_bit_buffer;
        saved_bits_in = d->m_bits_in;

        if ( !use_raw_block )
            comp_block_succeeded = tdefl_compress_block( d, ( d->m_flags & TDEFL_FORCE_ALL_STATIC_BLOCKS ) || ( d->m_total_lz_bytes < 48 ) );
        if ( ( ( use_raw_block ) || ( ( d->m_total_lz_bytes ) && ( ( d->m_pOutput_buf - pSaved_output_buf + 1U ) >= d->m_total_lz_bytes ) ) ) &&
             ( ( d->m_lookahead_pos - d->m_lz_code_buf_dict_pos ) <= d->m_dict_size ) )
        {
            mz_uint32 i;
            d->m_pOutput_buf = pSaved_output_buf;
            d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
            TDEFL_PUT_BITS( 0, 2 );
            if ( d->m_bits_in )
            {
                TDEFL_PUT_BITS( 0, 8 - d->m_bits_in );
            }
            for ( i = 2; i; --i, d->m_total_lz_bytes ^= 0xFFFF )
            {
                TDEFL_PUT_BITS( d->m_total_lz_bytes & 0xFFFF, 16 );
            }
            for ( i = 0; i < d->m_total_lz_bytes; ++i )
            {
                TDEFL_PUT_BITS( d->m_dict[( d->m_lz_code_buf_dict_pos + i ) & TDEFL_LZ_DICT_SIZE_MASK], 8 );
            }
        }

        else if ( !comp_block_succeeded )
        {
            d->m_pOutput_buf = pSaved_output_buf;
            d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
            tdefl_compress_block( d, true );
        }

        if ( flush )
        {
            if ( flush == TDEFL_FINISH )
            {
                if ( d->m_bits_in )
                {
                    TDEFL_PUT_BITS( 0, 8 - d->m_bits_in );
                }
                if ( d->m_flags & TDEFL_WRITE_ZLIB_HEADER )
                {
                    mz_uint32 i, a = d->m_adler32;
                    for ( i = 0; i < 4; i++ )
                    {
                        TDEFL_PUT_BITS( ( a >> 24 ) & 0xFF, 8 );
                        a <<= 8;
                    }
                }
            }
            else
            {
                mz_uint32 i, z = 0;
                TDEFL_PUT_BITS( 0, 3 );
                if ( d->m_bits_in )
                {
                    TDEFL_PUT_BITS( 0, 8 - d->m_bits_in );
                }
                for ( i = 2; i; --i, z ^= 0xFFFF )
                {
                    TDEFL_PUT_BITS( z & 0xFFFF, 16 );
                }
            }
        }

        assert( d->m_pOutput_buf < d->m_pOutput_buf_end );

        memset( &d->m_huff_count[0][0], 0, sizeof( d->m_huff_count[0][0] ) * TDEFL_MAX_HUFF_SYMBOLS_0 );
        memset( &d->m_huff_count[1][0], 0, sizeof( d->m_huff_count[1][0] ) * TDEFL_MAX_HUFF_SYMBOLS_1 );

        d->m_pLZ_code_buf = d->m_lz_code_buf + 1;
        d->m_pLZ_flags = d->m_lz_code_buf;
        d->m_num_flags_left = 8;
        d->m_lz_code_buf_dict_pos += d->m_total_lz_bytes;
        d->m_total_lz_bytes = 0;
        d->m_block_index++;

        if ( ( n = (int)( d->m_pOutput_buf - pOutput_buf_start ) ) != 0 )
        {
            if ( d->m_pPut_buf_func )
            {
                *d->m_pIn_buf_size = d->m_pSrc - (const mz_uint8 *)d->m_pIn_buf;
                if ( !( *d->m_pPut_buf_func )( d->m_output_buf, n, d->m_pPut_buf_user ) )
                    return ( d->m_prev_return_status = TDEFL_STATUS_PUT_BUF_FAILED );
            }
            else if ( pOutput_buf_start == d->m_output_buf )
            {
                int bytes_to_copy = (int)MZ_MIN( (size_t)n, (size_t)( *d->m_pOut_buf_size - d->m_out_buf_ofs ) );
                memcpy( (mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf, bytes_to_copy );
                d->m_out_buf_ofs += bytes_to_copy;
                if ( ( n -= bytes_to_copy ) != 0 )
                {
                    d->m_output_flush_ofs = bytes_to_copy;
                    d->m_output_flush_remaining = n;
                }
            }
            else
            {
                d->m_out_buf_ofs += n;
            }
        }

        return d->m_output_flush_remaining;
    }
    static mz_uint16 TDEFL_READ_UNALIGNED_WORD( const mz_uint8 * p )
    {
        mz_uint16 ret;
        memcpy( &ret, p, sizeof( mz_uint16 ) );
        return ret;
    }
    static mz_uint16 TDEFL_READ_UNALIGNED_WORD2( const mz_uint16 * p )
    {
        mz_uint16 ret;
        memcpy( &ret, p, sizeof( mz_uint16 ) );
        return ret;
    }
    static void tdefl_find_match( tdefl_compressor * d, mz_uint32 lookahead_pos, mz_uint32 max_dist, mz_uint32 max_match_len, mz_uint32 * pMatch_dist, mz_uint32 * pMatch_len )
    {
        mz_uint32 dist, pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK, match_len = *pMatch_len, probe_pos = pos, next_probe_pos, probe_len;
        mz_uint32 num_probes_left = d->m_max_probes[match_len >= 32];
        const mz_uint16 * s = (const mz_uint16 *)( d->m_dict + pos ), * p, * q;
        mz_uint16 c01 = TDEFL_READ_UNALIGNED_WORD( &d->m_dict[pos + match_len - 1] ), s01 = TDEFL_READ_UNALIGNED_WORD2( s );
        assert( max_match_len <= TDEFL_MAX_MATCH_LEN );
        if ( max_match_len <= match_len )
            return;
        for ( ;;)
        {
            for ( ;;)
            {
                if ( --num_probes_left == 0 )
                    return;
                TDEFL_PROBE;
                TDEFL_PROBE;
                TDEFL_PROBE;
            }
            if ( !dist )
                break;
            q = (const mz_uint16 *)( d->m_dict + probe_pos );
            if ( TDEFL_READ_UNALIGNED_WORD2( q ) != s01 )
                continue;
            p = s;
            probe_len = 32;
            do
            {
            } while ( ( TDEFL_READ_UNALIGNED_WORD2( ++p ) == TDEFL_READ_UNALIGNED_WORD2( ++q ) ) &&
                      ( TDEFL_READ_UNALIGNED_WORD2( ++p ) == TDEFL_READ_UNALIGNED_WORD2( ++q ) ) &&
                      ( TDEFL_READ_UNALIGNED_WORD2( ++p ) == TDEFL_READ_UNALIGNED_WORD2( ++q ) ) &&
                      ( TDEFL_READ_UNALIGNED_WORD2( ++p ) == TDEFL_READ_UNALIGNED_WORD2( ++q ) ) && ( --probe_len > 0 ) );
            if ( !probe_len )
            {
                *pMatch_dist = dist;
                *pMatch_len = MZ_MIN( max_match_len, (mz_uint32)TDEFL_MAX_MATCH_LEN );
                break;
            }
            else if ( ( probe_len = ( (mz_uint32)( p - s ) * 2 ) + (mz_uint32)( *(const mz_uint8 *)p == *(const mz_uint8 *)q ) ) > match_len )
            {
                *pMatch_dist = dist;
                if ( ( *pMatch_len = match_len = MZ_MIN( max_match_len, probe_len ) ) == max_match_len )
                    break;
                c01 = TDEFL_READ_UNALIGNED_WORD( &d->m_dict[pos + match_len - 1] );
            }
        }
    }
    static mz_uint32 TDEFL_READ_UNALIGNED_WORD32( const mz_uint8 * p )
    {
        mz_uint32 ret;
        memcpy( &ret, p, sizeof( mz_uint32 ) );
        return ret;
    }
    static mz_bool tdefl_compress_fast( tdefl_compressor * d )
    {
        mz_uint32 lookahead_pos = d->m_lookahead_pos, lookahead_size = d->m_lookahead_size, dict_size = d->m_dict_size, total_lz_bytes = d->m_total_lz_bytes,
            num_flags_left = d->m_num_flags_left;
        mz_uint8 * pLZ_code_buf = d->m_pLZ_code_buf, * pLZ_flags = d->m_pLZ_flags;
        mz_uint32 cur_pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK;

        while ( ( d->m_src_buf_left ) || ( ( d->m_flush ) && ( lookahead_size ) ) )
        {
            const mz_uint32 TDEFL_COMP_FAST_LOOKAHEAD_SIZE = 4096;
            mz_uint32 dst_pos = ( lookahead_pos + lookahead_size ) & TDEFL_LZ_DICT_SIZE_MASK;
            mz_uint32 num_bytes_to_process = (mz_uint32)MZ_MIN( d->m_src_buf_left, TDEFL_COMP_FAST_LOOKAHEAD_SIZE - lookahead_size );
            d->m_src_buf_left -= num_bytes_to_process;
            lookahead_size += num_bytes_to_process;

            while ( num_bytes_to_process )
            {
                mz_uint32 n = MZ_MIN( TDEFL_LZ_DICT_SIZE - dst_pos, num_bytes_to_process );
                memcpy( d->m_dict + dst_pos, d->m_pSrc, n );
                if ( dst_pos < ( TDEFL_MAX_MATCH_LEN - 1 ) )
                    memcpy( d->m_dict + TDEFL_LZ_DICT_SIZE + dst_pos, d->m_pSrc, MZ_MIN( n, ( TDEFL_MAX_MATCH_LEN - 1 ) - dst_pos ) );
                d->m_pSrc += n;
                dst_pos = ( dst_pos + n ) & TDEFL_LZ_DICT_SIZE_MASK;
                num_bytes_to_process -= n;
            }

            dict_size = MZ_MIN( TDEFL_LZ_DICT_SIZE - lookahead_size, dict_size );
            if ( ( !d->m_flush ) && ( lookahead_size < TDEFL_COMP_FAST_LOOKAHEAD_SIZE ) )
                break;

            while ( lookahead_size >= 4 )
            {
                mz_uint32 cur_match_dist, cur_match_len = 1;
                mz_uint8 * pCur_dict = d->m_dict + cur_pos;
                mz_uint32 first_trigram = TDEFL_READ_UNALIGNED_WORD32( pCur_dict ) & 0xFFFFFF;
                mz_uint32 hash = ( first_trigram ^ ( first_trigram >> ( 24 - ( TDEFL_LZ_HASH_BITS - 8 ) ) ) ) & TDEFL_LEVEL1_HASH_SIZE_MASK;
                mz_uint32 probe_pos = d->m_hash[hash];
                d->m_hash[hash] = (mz_uint16)lookahead_pos;

                if ( ( ( cur_match_dist = (mz_uint16)( lookahead_pos - probe_pos ) ) <= dict_size ) &&
                     ( ( TDEFL_READ_UNALIGNED_WORD32( d->m_dict + ( probe_pos &= TDEFL_LZ_DICT_SIZE_MASK ) ) & 0xFFFFFF ) == first_trigram ) )
                {
                    const mz_uint16 * p = (const mz_uint16 *)pCur_dict;
                    const mz_uint16 * q = (const mz_uint16 *)( d->m_dict + probe_pos );
                    mz_uint32 probe_len = 32;
                    do
                    {
                    } while ( ( TDEFL_READ_UNALIGNED_WORD2( ++p ) == TDEFL_READ_UNALIGNED_WORD2( ++q ) ) &&
                              ( TDEFL_READ_UNALIGNED_WORD2( ++p ) == TDEFL_READ_UNALIGNED_WORD2( ++q ) ) &&
                              ( TDEFL_READ_UNALIGNED_WORD2( ++p ) == TDEFL_READ_UNALIGNED_WORD2( ++q ) ) &&
                              ( TDEFL_READ_UNALIGNED_WORD2( ++p ) == TDEFL_READ_UNALIGNED_WORD2( ++q ) ) && ( --probe_len > 0 ) );
                    cur_match_len = ( (mz_uint32)( p - (const mz_uint16 *)pCur_dict ) * 2 ) + (mz_uint32)( *(const mz_uint8 *)p == *(const mz_uint8 *)q );
                    if ( !probe_len )
                        cur_match_len = cur_match_dist ? TDEFL_MAX_MATCH_LEN : 0;

                    if ( ( cur_match_len < TDEFL_MIN_MATCH_LEN ) || ( ( cur_match_len == TDEFL_MIN_MATCH_LEN ) && ( cur_match_dist >= 8U * 1024U ) ) )
                    {
                        cur_match_len = 1;
                        *pLZ_code_buf++ = (mz_uint8)first_trigram;
                        *pLZ_flags = (mz_uint8)( *pLZ_flags >> 1 );
                        d->m_huff_count[0][(mz_uint8)first_trigram]++;
                    }
                    else
                    {
                        mz_uint32 s0, s1;
                        cur_match_len = MZ_MIN( cur_match_len, lookahead_size );

                        assert( ( cur_match_len >= TDEFL_MIN_MATCH_LEN ) && ( cur_match_dist >= 1 ) && ( cur_match_dist <= TDEFL_LZ_DICT_SIZE ) );

                        cur_match_dist--;

                        pLZ_code_buf[0] = (mz_uint8)( cur_match_len - TDEFL_MIN_MATCH_LEN );
                        memcpy( &pLZ_code_buf[1], &cur_match_dist, sizeof( cur_match_dist ) );
                        pLZ_code_buf += 3;
                        *pLZ_flags = (mz_uint8)( ( *pLZ_flags >> 1 ) | 0x80 );

                        s0 = s_tdefl_small_dist_sym[cur_match_dist & 511];
                        s1 = s_tdefl_large_dist_sym[cur_match_dist >> 8];
                        d->m_huff_count[1][( cur_match_dist < 512 ) ? s0 : s1]++;

                        d->m_huff_count[0][s_tdefl_len_sym[cur_match_len - TDEFL_MIN_MATCH_LEN]]++;
                    }
                }
                else
                {
                    *pLZ_code_buf++ = (mz_uint8)first_trigram;
                    *pLZ_flags = (mz_uint8)( *pLZ_flags >> 1 );
                    d->m_huff_count[0][(mz_uint8)first_trigram]++;
                }

                if ( --num_flags_left == 0 )
                {
                    num_flags_left = 8;
                    pLZ_flags = pLZ_code_buf++;
                }

                total_lz_bytes += cur_match_len;
                lookahead_pos += cur_match_len;
                dict_size = MZ_MIN( dict_size + cur_match_len, (mz_uint32)TDEFL_LZ_DICT_SIZE );
                cur_pos = ( cur_pos + cur_match_len ) & TDEFL_LZ_DICT_SIZE_MASK;
                assert( lookahead_size >= cur_match_len );
                lookahead_size -= cur_match_len;

                if ( pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8] )
                {
                    int n;
                    d->m_lookahead_pos = lookahead_pos;
                    d->m_lookahead_size = lookahead_size;
                    d->m_dict_size = dict_size;
                    d->m_total_lz_bytes = total_lz_bytes;
                    d->m_pLZ_code_buf = pLZ_code_buf;
                    d->m_pLZ_flags = pLZ_flags;
                    d->m_num_flags_left = num_flags_left;
                    if ( ( n = tdefl_flush_block( d, 0 ) ) != 0 )
                        return ( n < 0 ) ? false : true;
                    total_lz_bytes = d->m_total_lz_bytes;
                    pLZ_code_buf = d->m_pLZ_code_buf;
                    pLZ_flags = d->m_pLZ_flags;
                    num_flags_left = d->m_num_flags_left;
                }
            }

            while ( lookahead_size )
            {
                mz_uint8 lit = d->m_dict[cur_pos];

                total_lz_bytes++;
                *pLZ_code_buf++ = lit;
                *pLZ_flags = (mz_uint8)( *pLZ_flags >> 1 );
                if ( --num_flags_left == 0 )
                {
                    num_flags_left = 8;
                    pLZ_flags = pLZ_code_buf++;
                }

                d->m_huff_count[0][lit]++;

                lookahead_pos++;
                dict_size = MZ_MIN( dict_size + 1, (mz_uint32)TDEFL_LZ_DICT_SIZE );
                cur_pos = ( cur_pos + 1 ) & TDEFL_LZ_DICT_SIZE_MASK;
                lookahead_size--;

                if ( pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8] )
                {
                    int n;
                    d->m_lookahead_pos = lookahead_pos;
                    d->m_lookahead_size = lookahead_size;
                    d->m_dict_size = dict_size;
                    d->m_total_lz_bytes = total_lz_bytes;
                    d->m_pLZ_code_buf = pLZ_code_buf;
                    d->m_pLZ_flags = pLZ_flags;
                    d->m_num_flags_left = num_flags_left;
                    if ( ( n = tdefl_flush_block( d, 0 ) ) != 0 )
                        return ( n < 0 ) ? false : true;
                    total_lz_bytes = d->m_total_lz_bytes;
                    pLZ_code_buf = d->m_pLZ_code_buf;
                    pLZ_flags = d->m_pLZ_flags;
                    num_flags_left = d->m_num_flags_left;
                }
            }
        }

        d->m_lookahead_pos = lookahead_pos;
        d->m_lookahead_size = lookahead_size;
        d->m_dict_size = dict_size;
        d->m_total_lz_bytes = total_lz_bytes;
        d->m_pLZ_code_buf = pLZ_code_buf;
        d->m_pLZ_flags = pLZ_flags;
        d->m_num_flags_left = num_flags_left;
        return true;
    }
    static void tdefl_record_literal( tdefl_compressor * d, mz_uint8 lit )
    {
        d->m_total_lz_bytes++;
        *d->m_pLZ_code_buf++ = lit;
        *d->m_pLZ_flags = (mz_uint8)( *d->m_pLZ_flags >> 1 );
        if ( --d->m_num_flags_left == 0 )
        {
            d->m_num_flags_left = 8;
            d->m_pLZ_flags = d->m_pLZ_code_buf++;
        }
        d->m_huff_count[0][lit]++;
    }
    static void tdefl_record_match( tdefl_compressor * d, mz_uint32 match_len, mz_uint32 match_dist )
    {
        mz_uint32 s0, s1;

        assert( ( match_len >= TDEFL_MIN_MATCH_LEN ) && ( match_dist >= 1 ) && ( match_dist <= TDEFL_LZ_DICT_SIZE ) );

        d->m_total_lz_bytes += match_len;

        d->m_pLZ_code_buf[0] = (mz_uint8)( match_len - TDEFL_MIN_MATCH_LEN );

        match_dist -= 1;
        d->m_pLZ_code_buf[1] = (mz_uint8)( match_dist & 0xFF );
        d->m_pLZ_code_buf[2] = (mz_uint8)( match_dist >> 8 );
        d->m_pLZ_code_buf += 3;

        *d->m_pLZ_flags = (mz_uint8)( ( *d->m_pLZ_flags >> 1 ) | 0x80 );
        if ( --d->m_num_flags_left == 0 )
        {
            d->m_num_flags_left = 8;
            d->m_pLZ_flags = d->m_pLZ_code_buf++;
        }

        s0 = s_tdefl_small_dist_sym[match_dist & 511];
        s1 = s_tdefl_large_dist_sym[( match_dist >> 8 ) & 127];
        d->m_huff_count[1][( match_dist < 512 ) ? s0 : s1]++;
        d->m_huff_count[0][s_tdefl_len_sym[match_len - TDEFL_MIN_MATCH_LEN]]++;
    }
    static mz_bool tdefl_compress_normal( tdefl_compressor * d )
    {
        const mz_uint8 * pSrc = d->m_pSrc;
        size_t src_buf_left = d->m_src_buf_left;
        tdefl_flush flush = d->m_flush;

        while ( ( src_buf_left ) || ( ( flush ) && ( d->m_lookahead_size ) ) )
        {
            mz_uint32 len_to_move, cur_match_dist, cur_match_len, cur_pos;

            if ( ( d->m_lookahead_size + d->m_dict_size ) >= ( TDEFL_MIN_MATCH_LEN - 1 ) )
            {
                mz_uint32 dst_pos = ( d->m_lookahead_pos + d->m_lookahead_size ) & TDEFL_LZ_DICT_SIZE_MASK, ins_pos = d->m_lookahead_pos + d->m_lookahead_size - 2;
                mz_uint32 hash = ( d->m_dict[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] << TDEFL_LZ_HASH_SHIFT ) ^ d->m_dict[( ins_pos + 1 ) & TDEFL_LZ_DICT_SIZE_MASK];
                mz_uint32 num_bytes_to_process = (mz_uint32)MZ_MIN( src_buf_left, TDEFL_MAX_MATCH_LEN - d->m_lookahead_size );
                const mz_uint8 * pSrc_end = pSrc + num_bytes_to_process;
                src_buf_left -= num_bytes_to_process;
                d->m_lookahead_size += num_bytes_to_process;
                while ( pSrc != pSrc_end )
                {
                    mz_uint8 c = *pSrc++;
                    d->m_dict[dst_pos] = c;
                    if ( dst_pos < ( TDEFL_MAX_MATCH_LEN - 1 ) )
                        d->m_dict[TDEFL_LZ_DICT_SIZE + dst_pos] = c;
                    hash = ( ( hash << TDEFL_LZ_HASH_SHIFT ) ^ c ) & ( TDEFL_LZ_HASH_SIZE - 1 );
                    d->m_next[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash];
                    d->m_hash[hash] = (mz_uint16)( ins_pos );
                    dst_pos = ( dst_pos + 1 ) & TDEFL_LZ_DICT_SIZE_MASK;
                    ins_pos++;
                }
            }
            else
            {
                while ( ( src_buf_left ) && ( d->m_lookahead_size < TDEFL_MAX_MATCH_LEN ) )
                {
                    mz_uint8 c = *pSrc++;
                    mz_uint32 dst_pos = ( d->m_lookahead_pos + d->m_lookahead_size ) & TDEFL_LZ_DICT_SIZE_MASK;
                    src_buf_left--;
                    d->m_dict[dst_pos] = c;
                    if ( dst_pos < ( TDEFL_MAX_MATCH_LEN - 1 ) )
                        d->m_dict[TDEFL_LZ_DICT_SIZE + dst_pos] = c;
                    if ( ( ++d->m_lookahead_size + d->m_dict_size ) >= TDEFL_MIN_MATCH_LEN )
                    {
                        mz_uint32 ins_pos = d->m_lookahead_pos + ( d->m_lookahead_size - 1 ) - 2;
                        mz_uint32 hash = ( ( d->m_dict[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] << ( TDEFL_LZ_HASH_SHIFT * 2 ) ) ^
                                           ( d->m_dict[( ins_pos + 1 ) & TDEFL_LZ_DICT_SIZE_MASK] << TDEFL_LZ_HASH_SHIFT ) ^ c ) &
                            ( TDEFL_LZ_HASH_SIZE - 1 );
                        d->m_next[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash];
                        d->m_hash[hash] = (mz_uint16)( ins_pos );
                    }
                }
            }
            d->m_dict_size = MZ_MIN( TDEFL_LZ_DICT_SIZE - d->m_lookahead_size, d->m_dict_size );
            if ( ( !flush ) && ( d->m_lookahead_size < TDEFL_MAX_MATCH_LEN ) )
                break;
            len_to_move = 1;
            cur_match_dist = 0;
            cur_match_len = d->m_saved_match_len ? d->m_saved_match_len : ( TDEFL_MIN_MATCH_LEN - 1 );
            cur_pos = d->m_lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK;
            if ( d->m_flags & ( TDEFL_RLE_MATCHES | TDEFL_FORCE_ALL_RAW_BLOCKS ) )
            {
                if ( ( d->m_dict_size ) && ( !( d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS ) ) )
                {
                    mz_uint8 c = d->m_dict[( cur_pos - 1 ) & TDEFL_LZ_DICT_SIZE_MASK];
                    cur_match_len = 0;
                    while ( cur_match_len < d->m_lookahead_size )
                    {
                        if ( d->m_dict[cur_pos + cur_match_len] != c )
                            break;
                        cur_match_len++;
                    }
                    if ( cur_match_len < TDEFL_MIN_MATCH_LEN )
                        cur_match_len = 0;
                    else
                        cur_match_dist = 1;
                }
            }
            else
            {
                tdefl_find_match( d, d->m_lookahead_pos, d->m_dict_size, d->m_lookahead_size, &cur_match_dist, &cur_match_len );
            }
            if ( ( ( cur_match_len == TDEFL_MIN_MATCH_LEN ) && ( cur_match_dist >= 8U * 1024U ) ) || ( cur_pos == cur_match_dist ) ||
                 ( ( d->m_flags & TDEFL_FILTER_MATCHES ) && ( cur_match_len <= 5 ) ) )
            {
                cur_match_dist = cur_match_len = 0;
            }
            if ( d->m_saved_match_len )
            {
                if ( cur_match_len > d->m_saved_match_len )
                {
                    tdefl_record_literal( d, (mz_uint8)d->m_saved_lit );
                    if ( cur_match_len >= 128 )
                    {
                        tdefl_record_match( d, cur_match_len, cur_match_dist );
                        d->m_saved_match_len = 0;
                        len_to_move = cur_match_len;
                    }
                    else
                    {
                        d->m_saved_lit = d->m_dict[cur_pos];
                        d->m_saved_match_dist = cur_match_dist;
                        d->m_saved_match_len = cur_match_len;
                    }
                }
                else
                {
                    tdefl_record_match( d, d->m_saved_match_len, d->m_saved_match_dist );
                    len_to_move = d->m_saved_match_len - 1;
                    d->m_saved_match_len = 0;
                }
            }
            else if ( !cur_match_dist )
                tdefl_record_literal( d, d->m_dict[MZ_MIN( cur_pos, sizeof( d->m_dict ) - 1 )] );
            else if ( ( d->m_greedy_parsing ) || ( d->m_flags & TDEFL_RLE_MATCHES ) || ( cur_match_len >= 128 ) )
            {
                tdefl_record_match( d, cur_match_len, cur_match_dist );
                len_to_move = cur_match_len;
            }
            else
            {
                d->m_saved_lit = d->m_dict[MZ_MIN( cur_pos, sizeof( d->m_dict ) - 1 )];
                d->m_saved_match_dist = cur_match_dist;
                d->m_saved_match_len = cur_match_len;
            }

            d->m_lookahead_pos += len_to_move;
            assert( d->m_lookahead_size >= len_to_move );
            d->m_lookahead_size -= len_to_move;
            d->m_dict_size = MZ_MIN( d->m_dict_size + len_to_move, (mz_uint32)TDEFL_LZ_DICT_SIZE );

            if ( ( d->m_pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8] ) ||
                 ( ( d->m_total_lz_bytes > 31 * 1024 ) &&
                 ( ( ( ( (mz_uint32)( d->m_pLZ_code_buf - d->m_lz_code_buf ) * 115 ) >> 7 ) >= d->m_total_lz_bytes ) || ( d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS ) ) ) )
            {
                int n;
                d->m_pSrc = pSrc;
                d->m_src_buf_left = src_buf_left;
                if ( ( n = tdefl_flush_block( d, 0 ) ) != 0 )
                    return ( n < 0 ) ? false : true;
            }
        }

        d->m_pSrc = pSrc;
        d->m_src_buf_left = src_buf_left;
        return true;
    }
    static tdefl_status tdefl_flush_output_buffer( tdefl_compressor * d )
    {
        if ( d->m_pIn_buf_size )
        {
            *d->m_pIn_buf_size = d->m_pSrc - (const mz_uint8 *)d->m_pIn_buf;
        }

        if ( d->m_pOut_buf_size )
        {
            size_t n = MZ_MIN( *d->m_pOut_buf_size - d->m_out_buf_ofs, d->m_output_flush_remaining );
            memcpy( (mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf + d->m_output_flush_ofs, n );
            d->m_output_flush_ofs += (mz_uint32)n;
            d->m_output_flush_remaining -= (mz_uint32)n;
            d->m_out_buf_ofs += n;

            *d->m_pOut_buf_size = d->m_out_buf_ofs;
        }

        return ( d->m_finished && !d->m_output_flush_remaining ) ? TDEFL_STATUS_DONE : TDEFL_STATUS_OKAY;
    }
    tdefl_status tdefl_compress( tdefl_compressor * d, const void * pIn_buf, size_t * pIn_buf_size, void * pOut_buf, size_t * pOut_buf_size, tdefl_flush flush )
    {
        if ( !d )
        {
            if ( pIn_buf_size )
                *pIn_buf_size = 0;
            if ( pOut_buf_size )
                *pOut_buf_size = 0;
            return TDEFL_STATUS_BAD_PARAM;
        }

        d->m_pIn_buf = pIn_buf;
        d->m_pIn_buf_size = pIn_buf_size;
        d->m_pOut_buf = pOut_buf;
        d->m_pOut_buf_size = pOut_buf_size;
        d->m_pSrc = (const mz_uint8 *)( pIn_buf );
        d->m_src_buf_left = pIn_buf_size ? *pIn_buf_size : 0;
        d->m_out_buf_ofs = 0;
        d->m_flush = flush;

        if ( ( ( d->m_pPut_buf_func != NULL ) == ( ( pOut_buf != NULL ) || ( pOut_buf_size != NULL ) ) ) || ( d->m_prev_return_status != TDEFL_STATUS_OKAY ) ||
             ( d->m_wants_to_finish && ( flush != TDEFL_FINISH ) ) || ( pIn_buf_size && *pIn_buf_size && !pIn_buf ) || ( pOut_buf_size && *pOut_buf_size && !pOut_buf ) )
        {
            if ( pIn_buf_size )
                *pIn_buf_size = 0;
            if ( pOut_buf_size )
                *pOut_buf_size = 0;
            return ( d->m_prev_return_status = TDEFL_STATUS_BAD_PARAM );
        }
        d->m_wants_to_finish |= ( flush == TDEFL_FINISH );

        if ( ( d->m_output_flush_remaining ) || ( d->m_finished ) )
            return ( d->m_prev_return_status = tdefl_flush_output_buffer( d ) );

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        if ( ( ( d->m_flags & TDEFL_MAX_PROBES_MASK ) == 1 ) && ( ( d->m_flags & TDEFL_GREEDY_PARSING_FLAG ) != 0 ) && ( ( d->m_flags & ( TDEFL_FILTER_MATCHES | TDEFL_FORCE_ALL_RAW_BLOCKS | TDEFL_RLE_MATCHES ) ) == 0 ) )
        {
            if ( !tdefl_compress_fast( d ) )
                return d->m_prev_return_status;
        }
        else
#endif 
        {
            if ( !tdefl_compress_normal( d ) )
                return d->m_prev_return_status;
        }

        if ( ( d->m_flags & ( TDEFL_WRITE_ZLIB_HEADER | TDEFL_COMPUTE_ADLER32 ) ) && ( pIn_buf ) )
            d->m_adler32 = (mz_uint32)mz_adler32( d->m_adler32, (const mz_uint8 *)pIn_buf, d->m_pSrc - (const mz_uint8 *)pIn_buf );

        if ( ( flush ) && ( !d->m_lookahead_size ) && ( !d->m_src_buf_left ) && ( !d->m_output_flush_remaining ) )
        {
            if ( tdefl_flush_block( d, flush ) < 0 )
                return d->m_prev_return_status;
            d->m_finished = ( flush == TDEFL_FINISH );
            if ( flush == TDEFL_FULL_FLUSH )
            {
                MZ_CLEAR_OBJ( d->m_hash );
                MZ_CLEAR_OBJ( d->m_next );
                d->m_dict_size = 0;
            }
        }

        return ( d->m_prev_return_status = tdefl_flush_output_buffer( d ) );
    }
    tdefl_status tdefl_compress_buffer( tdefl_compressor * d, const void * pIn_buf, size_t in_buf_size, tdefl_flush flush )
    {
        assert( d->m_pPut_buf_func );
        return tdefl_compress( d, pIn_buf, &in_buf_size, NULL, NULL, flush );
    }
    tdefl_status tdefl_init( tdefl_compressor * d, tdefl_put_buf_func_ptr pPut_buf_func, void * pPut_buf_user, int flags )
    {
        d->m_pPut_buf_func = pPut_buf_func;
        d->m_pPut_buf_user = pPut_buf_user;
        d->m_flags = (mz_uint32)( flags );
        d->m_max_probes[0] = 1 + ( ( flags & 0xFFF ) + 2 ) / 3;
        d->m_greedy_parsing = ( flags & TDEFL_GREEDY_PARSING_FLAG ) != 0;
        d->m_max_probes[1] = 1 + ( ( ( flags & 0xFFF ) >> 2 ) + 2 ) / 3;
        if ( !( flags & TDEFL_NONDETERMINISTIC_PARSING_FLAG ) )
            MZ_CLEAR_OBJ( d->m_hash );
        d->m_lookahead_pos = d->m_lookahead_size = d->m_dict_size = d->m_total_lz_bytes = d->m_lz_code_buf_dict_pos = d->m_bits_in = 0;
        d->m_output_flush_ofs = d->m_output_flush_remaining = d->m_finished = d->m_block_index = d->m_bit_buffer = d->m_wants_to_finish = 0;
        d->m_pLZ_code_buf = d->m_lz_code_buf + 1;
        d->m_pLZ_flags = d->m_lz_code_buf;
        *d->m_pLZ_flags = 0;
        d->m_num_flags_left = 8;
        d->m_pOutput_buf = d->m_output_buf;
        d->m_pOutput_buf_end = d->m_output_buf;
        d->m_prev_return_status = TDEFL_STATUS_OKAY;
        d->m_saved_match_dist = d->m_saved_match_len = d->m_saved_lit = 0;
        d->m_adler32 = 1;
        d->m_pIn_buf = NULL;
        d->m_pOut_buf = NULL;
        d->m_pIn_buf_size = NULL;
        d->m_pOut_buf_size = NULL;
        d->m_flush = TDEFL_NO_FLUSH;
        d->m_pSrc = NULL;
        d->m_src_buf_left = 0;
        d->m_out_buf_ofs = 0;
        if ( !( flags & TDEFL_NONDETERMINISTIC_PARSING_FLAG ) )
            MZ_CLEAR_OBJ( d->m_dict );
        memset( &d->m_huff_count[0][0], 0, sizeof( d->m_huff_count[0][0] ) * TDEFL_MAX_HUFF_SYMBOLS_0 );
        memset( &d->m_huff_count[1][0], 0, sizeof( d->m_huff_count[1][0] ) * TDEFL_MAX_HUFF_SYMBOLS_1 );
        return TDEFL_STATUS_OKAY;
    }
    tdefl_status tdefl_get_prev_return_status( tdefl_compressor * d )
    {
        return d->m_prev_return_status;
    }
    mz_uint32 tdefl_get_adler32( tdefl_compressor * d )
    {
        return d->m_adler32;
    }
    mz_bool tdefl_compress_mem_to_output( const void * pBuf, size_t buf_len, tdefl_put_buf_func_ptr pPut_buf_func, void * pPut_buf_user, int flags )
    {
        tdefl_compressor * pComp;
        mz_bool succeeded;
        if ( ( ( buf_len ) && ( !pBuf ) ) || ( !pPut_buf_func ) )
            return false;
        pComp = (tdefl_compressor *)malloc( sizeof( tdefl_compressor ) );
        if ( !pComp )
            return false;
        succeeded = ( tdefl_init( pComp, pPut_buf_func, pPut_buf_user, flags ) == TDEFL_STATUS_OKAY );
        succeeded = succeeded && ( tdefl_compress_buffer( pComp, pBuf, buf_len, TDEFL_FINISH ) == TDEFL_STATUS_DONE );
        free( pComp );
        return succeeded;
    }
    static mz_bool tdefl_output_buffer_putter( const void * pBuf, int len, void * pUser )
    {
        tdefl_output_buffer * p = (tdefl_output_buffer *)pUser;
        size_t new_size = p->m_size + len;
        if ( new_size > p->m_capacity )
        {
            size_t new_capacity = p->m_capacity;
            mz_uint8 * pNew_buf;
            if ( !p->m_expandable )
                return false;
            do
            {
                new_capacity = MZ_MAX( 128U, new_capacity << 1U );
            } while ( new_size > new_capacity );
            pNew_buf = (mz_uint8 *)realloc( p->m_pBuf, new_capacity );
            if ( !pNew_buf )
                return false;
            p->m_pBuf = pNew_buf;
            p->m_capacity = new_capacity;
        }
        memcpy( (mz_uint8 *)p->m_pBuf + p->m_size, pBuf, len );
        p->m_size = new_size;
        return true;
    }
    void * tdefl_compress_mem_to_heap( const void * pSrc_buf, size_t src_buf_len, size_t * pOut_len, int flags )
    {
        tdefl_output_buffer out_buf;
        MZ_CLEAR_OBJ( out_buf );
        if ( !pOut_len )
            return nullptr;
        else
            *pOut_len = 0;
        out_buf.m_expandable = true;
        if ( !tdefl_compress_mem_to_output( pSrc_buf, src_buf_len, tdefl_output_buffer_putter, &out_buf, flags ) )
            return NULL;
        *pOut_len = out_buf.m_size;
        return out_buf.m_pBuf;
    }
    size_t tdefl_compress_mem_to_mem( void * pOut_buf, size_t out_buf_len, const void * pSrc_buf, size_t src_buf_len, int flags )
    {
        tdefl_output_buffer out_buf;
        MZ_CLEAR_OBJ( out_buf );
        if ( !pOut_buf )
            return 0;
        out_buf.m_pBuf = (mz_uint8 *)pOut_buf;
        out_buf.m_capacity = out_buf_len;
        if ( !tdefl_compress_mem_to_output( pSrc_buf, src_buf_len, tdefl_output_buffer_putter, &out_buf, flags ) )
            return 0;
        return out_buf.m_size;
    }
    mz_uint32 tdefl_create_comp_flags_from_zip_params( int level, int window_bits, int strategy )
    {
        mz_uint32 comp_flags = s_tdefl_num_probes[( level >= 0 ) ? MZ_MIN( 10, level ) : MZ_DEFAULT_LEVEL] | ( ( level <= 3 ) ? TDEFL_GREEDY_PARSING_FLAG : 0 );
        if ( window_bits > 0 )
            comp_flags |= TDEFL_WRITE_ZLIB_HEADER;

        if ( !level )
            comp_flags |= TDEFL_FORCE_ALL_RAW_BLOCKS;
        else if ( strategy == MZ_FILTERED )
            comp_flags |= TDEFL_FILTER_MATCHES;
        else if ( strategy == MZ_HUFFMAN_ONLY )
            comp_flags &= ~TDEFL_MAX_PROBES_MASK;
        else if ( strategy == MZ_FIXED )
            comp_flags |= TDEFL_FORCE_ALL_STATIC_BLOCKS;
        else if ( strategy == MZ_RLE )
            comp_flags |= TDEFL_RLE_MATCHES;

        return comp_flags;
    }
    void * tdefl_write_image_to_png_file_in_memory_ex( const void * pImage, int w, int h, int num_chans, size_t * pLen_out, mz_uint32 level, mz_bool flip )
    {

        static const mz_uint32 s_tdefl_png_num_probes[11] = { 0, 1, 6, 32, 16, 32, 128, 256, 512, 768, 1500 };
        tdefl_compressor * pComp = (tdefl_compressor *)malloc( sizeof( tdefl_compressor ) );
        tdefl_output_buffer out_buf;
        int i, bpl = w * num_chans, y, z;
        mz_uint32 c;
        *pLen_out = 0;
        if ( !pComp )
            return NULL;
        MZ_CLEAR_OBJ( out_buf );
        out_buf.m_expandable = true;
        out_buf.m_capacity = 57 + MZ_MAX( 64, ( 1 + bpl ) * h );
        if ( NULL == ( out_buf.m_pBuf = (mz_uint8 *)malloc( out_buf.m_capacity ) ) )
        {
            free( pComp );
            return NULL;
        }

        for ( z = 41; z; --z )
            tdefl_output_buffer_putter( &z, 1, &out_buf );

        tdefl_init( pComp, tdefl_output_buffer_putter, &out_buf, s_tdefl_png_num_probes[MZ_MIN( 10, level )] | TDEFL_WRITE_ZLIB_HEADER );
        for ( y = 0; y < h; ++y )
        {
            tdefl_compress_buffer( pComp, &z, 1, TDEFL_NO_FLUSH );
            tdefl_compress_buffer( pComp, (mz_uint8 *)pImage + ( flip ? ( h - 1 - y ) : y ) * bpl, bpl, TDEFL_NO_FLUSH );
        }
        if ( tdefl_compress_buffer( pComp, NULL, 0, TDEFL_FINISH ) != TDEFL_STATUS_DONE )
        {
            free( pComp );
            free( out_buf.m_pBuf );
            return NULL;
        }

        *pLen_out = out_buf.m_size - 41;
        {
            static const mz_uint8 chans[] = { 0x00, 0x00, 0x04, 0x02, 0x06 };
            mz_uint8 pnghdr[41] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x44, 0x41, 0x54 };
            pnghdr[18] = (mz_uint8)( w >> 8 );
            pnghdr[19] = (mz_uint8)w;
            pnghdr[22] = (mz_uint8)( h >> 8 );
            pnghdr[23] = (mz_uint8)h;
            pnghdr[25] = chans[num_chans];
            pnghdr[33] = (mz_uint8)( *pLen_out >> 24 );
            pnghdr[34] = (mz_uint8)( *pLen_out >> 16 );
            pnghdr[35] = (mz_uint8)( *pLen_out >> 8 );
            pnghdr[36] = (mz_uint8)*pLen_out;
            c = (mz_uint32)mz_crc32( 0, pnghdr + 12, 17 );
            for ( i = 0; i < 4; ++i, c <<= 8 )
                ( (mz_uint8 *)( pnghdr + 29 ) )[i] = (mz_uint8)( c >> 24 );
            memcpy( out_buf.m_pBuf, pnghdr, 41 );
        }

        if ( !tdefl_output_buffer_putter( "\0\0\0\0\0\0\0\0\x49\x45\x4e\x44\xae\x42\x60\x82", 16, &out_buf ) )
        {
            *pLen_out = 0;
            free( pComp );
            free( out_buf.m_pBuf );
            return NULL;
        }
        c = (mz_uint32)mz_crc32( 0, out_buf.m_pBuf + 41 - 4, *pLen_out + 4 );
        for ( i = 0; i < 4; ++i, c <<= 8 )
            ( out_buf.m_pBuf + out_buf.m_size - 16 )[i] = (mz_uint8)( c >> 24 );

        *pLen_out += 57;
        free( pComp );
        return out_buf.m_pBuf;
    }
    void * tdefl_write_image_to_png_file_in_memory( const void * pImage, int w, int h, int num_chans, size_t * pLen_out )
    {
        return tdefl_write_image_to_png_file_in_memory_ex( pImage, w, h, num_chans, pLen_out, 6, false );
    }
    tdefl_compressor * tdefl_compressor_alloc()
    {
        return (tdefl_compressor *)malloc( sizeof( tdefl_compressor ) );
    }
    void tdefl_compressor_free( tdefl_compressor * pComp )
    {
        free( pComp );
    }
    tinfl_status tinfl_decompress( tinfl_decompressor * r, const mz_uint8 * pIn_buf_next, size_t * pIn_buf_size, mz_uint8 * pOut_buf_start, mz_uint8 * pOut_buf_next, size_t * pOut_buf_size, const mz_uint32 decomp_flags )
    {
        static const int s_length_base[31] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227,
            258, 0, 0 };
        static const int s_length_extra[31] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0 };
        static const int s_dist_base[32] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
            8193, 12289, 16385, 24577, 0, 0 };
        static const int s_dist_extra[32] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };
        static const mz_uint8 s_length_dezigzag[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
        static const int s_min_table_sizes[3] = { 257, 1, 4 };

        tinfl_status status = TINFL_STATUS_FAILED;
        mz_uint32 num_bits, dist, counter, num_extra;
        mz_uint32 bit_buf;
        const mz_uint8 * pIn_buf_cur = pIn_buf_next, * const pIn_buf_end = pIn_buf_next + *pIn_buf_size;
        mz_uint8 * pOut_buf_cur = pOut_buf_next, * const pOut_buf_end = pOut_buf_next + *pOut_buf_size;
        size_t out_buf_size_mask =
            ( decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF ) ? (size_t)-1 : ( ( pOut_buf_next - pOut_buf_start ) + *pOut_buf_size ) - 1,
            dist_from_out_buf_start;

        if ( ( ( out_buf_size_mask + 1 ) & out_buf_size_mask ) || ( pOut_buf_next < pOut_buf_start ) )
        {
            *pIn_buf_size = *pOut_buf_size = 0;
            return TINFL_STATUS_BAD_PARAM;
        }

        num_bits = r->m_num_bits;
        bit_buf = r->m_bit_buf;
        dist = r->m_dist;
        counter = r->m_counter;
        num_extra = r->m_num_extra;
        dist_from_out_buf_start = r->m_dist_from_out_buf_start;
        TINFL_CR_BEGIN

            bit_buf = num_bits = dist = counter = num_extra = r->m_zhdr0 = r->m_zhdr1 = 0;
        r->m_z_adler32 = r->m_check_adler32 = 1;
        if ( decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER )
        {
            TINFL_GET_BYTE( 1, r->m_zhdr0 );
            TINFL_GET_BYTE( 2, r->m_zhdr1 );
            counter = ( ( ( r->m_zhdr0 * 256 + r->m_zhdr1 ) % 31 != 0 ) || ( r->m_zhdr1 & 32 ) || ( ( r->m_zhdr0 & 15 ) != 8 ) );
            if ( !( decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF ) )
                counter |= ( ( ( 1U << ( 8U + ( r->m_zhdr0 >> 4 ) ) ) > 32768U ) || ( ( out_buf_size_mask + 1 ) < (mz_uint32)( 1U << ( 8U + ( r->m_zhdr0 >> 4 ) ) ) ) );
            if ( counter )
            {
                TINFL_CR_RETURN_FOREVER( 36, TINFL_STATUS_FAILED );
            }
        }

        do
        {
            TINFL_GET_BITS( 3, r->m_final, 3 );
            r->m_type = r->m_final >> 1;
            if ( r->m_type == 0 )
            {
                TINFL_SKIP_BITS( 5, num_bits & 7 );
                for ( counter = 0; counter < 4; ++counter )
                {
                    if ( num_bits )
                        TINFL_GET_BITS( 6, r->m_raw_header[counter], 8 )
                    else
                        TINFL_GET_BYTE( 7, r->m_raw_header[counter] )
                }
                if ( ( counter = ( r->m_raw_header[0] | ( r->m_raw_header[1] << 8 ) ) ) != (mz_uint32)( 0xFFFF ^ ( r->m_raw_header[2] | ( r->m_raw_header[3] << 8 ) ) ) )
                {
                    TINFL_CR_RETURN_FOREVER( 39, TINFL_STATUS_FAILED );
                }
                while ( ( counter ) && ( num_bits ) )
                {
                    TINFL_GET_BITS( 51, dist, 8 );
                    while ( pOut_buf_cur >= pOut_buf_end )
                    {
                        TINFL_CR_RETURN( 52, TINFL_STATUS_HAS_MORE_OUTPUT );
                    }
                    *pOut_buf_cur++ = (mz_uint8)dist;
                    counter--;
                }
                while ( counter )
                {
                    size_t n;
                    while ( pOut_buf_cur >= pOut_buf_end )
                    {
                        TINFL_CR_RETURN( 9, TINFL_STATUS_HAS_MORE_OUTPUT );
                    }
                    while ( pIn_buf_cur >= pIn_buf_end )
                    {
                        TINFL_CR_RETURN(
                            38, ( decomp_flags & TINFL_FLAG_HAS_MORE_INPUT ) ? TINFL_STATUS_NEEDS_MORE_INPUT : TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS );
                    }
                    n = MZ_MIN( MZ_MIN( (size_t)( pOut_buf_end - pOut_buf_cur ), (size_t)( pIn_buf_end - pIn_buf_cur ) ), counter );
                    memcpy( pOut_buf_cur, pIn_buf_cur, n );
                    pIn_buf_cur += n;
                    pOut_buf_cur += n;
                    counter -= (mz_uint32)n;
                }
            }
            else if ( r->m_type == 3 )
            {
                TINFL_CR_RETURN_FOREVER( 10, TINFL_STATUS_FAILED );
            }
            else
            {
                if ( r->m_type == 1 )
                {
                    mz_uint8 * p = r->m_tables[0].m_code_size;
                    mz_uint32 i;
                    r->m_table_sizes[0] = 288;
                    r->m_table_sizes[1] = 32;
                    memset( r->m_tables[1].m_code_size, 5, 32 );
                    for ( i = 0; i <= 143; ++i )
                        *p++ = 8;
                    for ( ; i <= 255; ++i )
                        *p++ = 9;
                    for ( ; i <= 279; ++i )
                        *p++ = 7;
                    for ( ; i <= 287; ++i )
                        *p++ = 8;
                }
                else
                {
                    for ( counter = 0; counter < 3; counter++ )
                    {
                        TINFL_GET_BITS( 11, r->m_table_sizes[counter], "\05\05\04"[counter] );
                        r->m_table_sizes[counter] += s_min_table_sizes[counter];
                    }
                    MZ_CLEAR_OBJ( r->m_tables[2].m_code_size );
                    for ( counter = 0; counter < r->m_table_sizes[2]; counter++ )
                    {
                        mz_uint32 s;
                        TINFL_GET_BITS( 14, s, 3 );
                        r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (mz_uint8)s;
                    }
                    r->m_table_sizes[2] = 19;
                }
                for ( ; (int)r->m_type >= 0; r->m_type-- )
                {
                    int tree_next, tree_cur;
                    tinfl_huff_table * pTable;
                    mz_uint32 i, j, used_syms, total, sym_index, next_code[17], total_syms[16];
                    pTable = &r->m_tables[r->m_type];
                    MZ_CLEAR_OBJ( total_syms );
                    MZ_CLEAR_OBJ( pTable->m_look_up );
                    MZ_CLEAR_OBJ( pTable->m_tree );
                    for ( i = 0; i < r->m_table_sizes[r->m_type]; ++i )
                        total_syms[pTable->m_code_size[i]]++;
                    used_syms = 0, total = 0;
                    next_code[0] = next_code[1] = 0;
                    for ( i = 1; i <= 15; ++i )
                    {
                        used_syms += total_syms[i];
                        next_code[i + 1] = ( total = ( ( total + total_syms[i] ) << 1 ) );
                    }
                    if ( ( 65536 != total ) && ( used_syms > 1 ) )
                    {
                        TINFL_CR_RETURN_FOREVER( 35, TINFL_STATUS_FAILED );
                    }
                    for ( tree_next = -1, sym_index = 0; sym_index < r->m_table_sizes[r->m_type]; ++sym_index )
                    {
                        mz_uint32 rev_code = 0, l, cur_code, code_size = pTable->m_code_size[sym_index];
                        if ( !code_size )
                            continue;
                        cur_code = next_code[code_size]++;
                        for ( l = code_size; l > 0; l--, cur_code >>= 1 )
                            rev_code = ( rev_code << 1 ) | ( cur_code & 1 );
                        if ( code_size <= TINFL_FAST_LOOKUP_BITS )
                        {
                            mz_int16 k = (mz_int16)( ( code_size << 9 ) | sym_index );
                            while ( rev_code < TINFL_FAST_LOOKUP_SIZE )
                            {
                                pTable->m_look_up[rev_code] = k;
                                rev_code += ( 1 << code_size );
                            }
                            continue;
                        }
                        if ( 0 == ( tree_cur = pTable->m_look_up[rev_code & ( TINFL_FAST_LOOKUP_SIZE - 1 )] ) )
                        {
                            pTable->m_look_up[rev_code & ( TINFL_FAST_LOOKUP_SIZE - 1 )] = (mz_int16)tree_next;
                            tree_cur = tree_next;
                            tree_next -= 2;
                        }
                        rev_code >>= ( TINFL_FAST_LOOKUP_BITS - 1 );
                        for ( j = code_size; j > ( TINFL_FAST_LOOKUP_BITS + 1 ); j-- )
                        {
                            tree_cur -= ( ( rev_code >>= 1 ) & 1 );
                            if ( !pTable->m_tree[-tree_cur - 1] )
                            {
                                pTable->m_tree[-tree_cur - 1] = (mz_int16)tree_next;
                                tree_cur = tree_next;
                                tree_next -= 2;
                            }
                            else
                                tree_cur = pTable->m_tree[-tree_cur - 1];
                        }
                        tree_cur -= ( ( rev_code >>= 1 ) & 1 );
                        pTable->m_tree[-tree_cur - 1] = (mz_int16)sym_index;
                    }
                    if ( r->m_type == 2 )
                    {
                        for ( counter = 0; counter < ( r->m_table_sizes[0] + r->m_table_sizes[1] );)
                        {
                            mz_uint32 s;
                            TINFL_HUFF_DECODE( 16, dist, &r->m_tables[2] );
                            if ( dist < 16 )
                            {
                                r->m_len_codes[counter++] = (mz_uint8)dist;
                                continue;
                            }
                            if ( ( dist == 16 ) && ( !counter ) )
                            {
                                TINFL_CR_RETURN_FOREVER( 17, TINFL_STATUS_FAILED );
                            }
                            num_extra = "\02\03\07"[dist - 16];
                            TINFL_GET_BITS( 18, s, num_extra );
                            s += "\03\03\013"[dist - 16];
                            memset( r->m_len_codes + counter, ( dist == 16 ) ? r->m_len_codes[counter - 1] : 0, s );
                            counter += s;
                        }
                        if ( ( r->m_table_sizes[0] + r->m_table_sizes[1] ) != counter )
                        {
                            TINFL_CR_RETURN_FOREVER( 21, TINFL_STATUS_FAILED );
                        }
                        memcpy( r->m_tables[0].m_code_size, r->m_len_codes, r->m_table_sizes[0] );
                        memcpy( r->m_tables[1].m_code_size, r->m_len_codes + r->m_table_sizes[0], r->m_table_sizes[1] );
                    }
                }
                for ( ;;)
                {
                    mz_uint8 * pSrc;
                    for ( ;;)
                    {
                        if ( ( ( pIn_buf_end - pIn_buf_cur ) < 4 ) || ( ( pOut_buf_end - pOut_buf_cur ) < 2 ) )
                        {
                            TINFL_HUFF_DECODE( 23, counter, &r->m_tables[0] );
                            if ( counter >= 256 )
                                break;
                            while ( pOut_buf_cur >= pOut_buf_end )
                            {
                                TINFL_CR_RETURN( 24, TINFL_STATUS_HAS_MORE_OUTPUT );
                            }
                            *pOut_buf_cur++ = (mz_uint8)counter;
                        }
                        else
                        {
                            int sym2;
                            mz_uint32 code_len;
                            if ( num_bits < 15 )
                            {
                                bit_buf |= ( ( (mz_uint32)MZ_READ_LE16( pIn_buf_cur ) ) << num_bits );
                                pIn_buf_cur += 2;
                                num_bits += 16;
                            }
                            if ( ( sym2 = r->m_tables[0].m_look_up[bit_buf & ( TINFL_FAST_LOOKUP_SIZE - 1 )] ) >= 0 )
                                code_len = sym2 >> 9;
                            else
                            {
                                code_len = TINFL_FAST_LOOKUP_BITS;
                                do
                                {
                                    sym2 = r->m_tables[0].m_tree[~sym2 + ( ( bit_buf >> code_len++ ) & 1 )];
                                } while ( sym2 < 0 );
                            }
                            counter = sym2;
                            bit_buf >>= code_len;
                            num_bits -= code_len;
                            if ( counter & 256 )
                                break;

                            if ( num_bits < 15 )
                            {
                                bit_buf |= ( ( (mz_uint32)MZ_READ_LE16( pIn_buf_cur ) ) << num_bits );
                                pIn_buf_cur += 2;
                                num_bits += 16;
                            }

                            if ( ( sym2 = r->m_tables[0].m_look_up[bit_buf & ( TINFL_FAST_LOOKUP_SIZE - 1 )] ) >= 0 )
                                code_len = sym2 >> 9;
                            else
                            {
                                code_len = TINFL_FAST_LOOKUP_BITS;
                                do
                                {
                                    sym2 = r->m_tables[0].m_tree[~sym2 + ( ( bit_buf >> code_len++ ) & 1 )];
                                } while ( sym2 < 0 );
                            }
                            bit_buf >>= code_len;
                            num_bits -= code_len;

                            pOut_buf_cur[0] = (mz_uint8)counter;
                            if ( sym2 & 256 )
                            {
                                pOut_buf_cur++;
                                counter = sym2;
                                break;
                            }
                            pOut_buf_cur[1] = (mz_uint8)sym2;
                            pOut_buf_cur += 2;
                        }
                    }
                    if ( ( counter &= 511 ) == 256 )
                        break;

                    num_extra = s_length_extra[counter - 257];
                    counter = s_length_base[counter - 257];
                    if ( num_extra )
                    {
                        mz_uint32 extra_bits;
                        TINFL_GET_BITS( 25, extra_bits, num_extra );
                        counter += extra_bits;
                    }

                    TINFL_HUFF_DECODE( 26, dist, &r->m_tables[1] );
                    num_extra = s_dist_extra[dist];
                    dist = s_dist_base[dist];
                    if ( num_extra )
                    {
                        mz_uint32 extra_bits;
                        TINFL_GET_BITS( 27, extra_bits, num_extra );
                        dist += extra_bits;
                    }

                    dist_from_out_buf_start = pOut_buf_cur - pOut_buf_start;
                    if ( ( dist == 0 || dist > dist_from_out_buf_start || dist_from_out_buf_start == 0 ) &&
                         ( decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF ) )
                    {
                        TINFL_CR_RETURN_FOREVER( 37, TINFL_STATUS_FAILED );
                    }

                    pSrc = pOut_buf_start + ( ( dist_from_out_buf_start - dist ) & out_buf_size_mask );

                    if ( ( MZ_MAX( pOut_buf_cur, pSrc ) + counter ) > pOut_buf_end )
                    {
                        while ( counter-- )
                        {
                            while ( pOut_buf_cur >= pOut_buf_end )
                            {
                                TINFL_CR_RETURN( 53, TINFL_STATUS_HAS_MORE_OUTPUT );
                            }
                            *pOut_buf_cur++ = pOut_buf_start[( dist_from_out_buf_start++ - dist ) & out_buf_size_mask];
                        }
                        continue;
                    }

                    else if ( ( counter >= 9 ) && ( counter <= dist ) )
                    {
                        const mz_uint8 * pSrc_end = pSrc + ( counter & ~7 );
                        do
                        {
                            memcpy( pOut_buf_cur, pSrc, sizeof( mz_uint32 ) * 2 );
                            pOut_buf_cur += 8;
                        } while ( ( pSrc += 8 ) < pSrc_end );
                        if ( ( counter &= 7 ) < 3 )
                        {
                            if ( counter )
                            {
                                pOut_buf_cur[0] = pSrc[0];
                                if ( counter > 1 )
                                    pOut_buf_cur[1] = pSrc[1];
                                pOut_buf_cur += counter;
                            }
                            continue;
                        }
                    }
                    while ( counter > 2 )
                    {
                        pOut_buf_cur[0] = pSrc[0];
                        pOut_buf_cur[1] = pSrc[1];
                        pOut_buf_cur[2] = pSrc[2];
                        pOut_buf_cur += 3;
                        pSrc += 3;
                        counter -= 3;
                    }
                    if ( counter > 0 )
                    {
                        pOut_buf_cur[0] = pSrc[0];
                        if ( counter > 1 )
                            pOut_buf_cur[1] = pSrc[1];
                        pOut_buf_cur += counter;
                    }
                }
            }
        } while ( !( r->m_final & 1 ) );

        TINFL_SKIP_BITS( 32, num_bits & 7 );
        while ( ( pIn_buf_cur > pIn_buf_next ) && ( num_bits >= 8 ) )
        {
            --pIn_buf_cur;
            num_bits -= 8;
        }
        bit_buf &= (mz_uint32)( ( ( (mz_uint64)1 ) << num_bits ) - (mz_uint64)1 );
        assert( !num_bits );

        if ( decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER )
        {
            for ( counter = 0; counter < 4; ++counter )
            {
                mz_uint32 s;
                if ( num_bits )
                    TINFL_GET_BITS( 41, s, 8 )
                else
                    TINFL_GET_BYTE( 42, s )
                    r->m_z_adler32 = ( r->m_z_adler32 << 8 ) | s;
            }
        }
        TINFL_CR_RETURN_FOREVER( 34, TINFL_STATUS_DONE );

        TINFL_CR_FINISH

            common_exit :

        if ( ( status != TINFL_STATUS_NEEDS_MORE_INPUT ) && ( status != TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS ) )
        {
            while ( ( pIn_buf_cur > pIn_buf_next ) && ( num_bits >= 8 ) )
            {
                --pIn_buf_cur;
                num_bits -= 8;
            }
        }
        r->m_num_bits = num_bits;
        r->m_bit_buf = bit_buf & (mz_uint32)( ( ( (mz_uint64)1 ) << num_bits ) - (mz_uint64)1 );
        r->m_dist = dist;
        r->m_counter = counter;
        r->m_num_extra = num_extra;
        r->m_dist_from_out_buf_start = dist_from_out_buf_start;
        *pIn_buf_size = pIn_buf_cur - pIn_buf_next;
        *pOut_buf_size = pOut_buf_cur - pOut_buf_next;
        if ( ( decomp_flags & ( TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_COMPUTE_ADLER32 ) ) && ( status >= 0 ) )
        {
            const mz_uint8 * ptr = pOut_buf_next;
            size_t buf_len = *pOut_buf_size;
            mz_uint32 i, s1 = r->m_check_adler32 & 0xffff, s2 = r->m_check_adler32 >> 16;
            size_t block_len = buf_len % 5552;
            while ( buf_len )
            {
                for ( i = 0; i + 7 < block_len; i += 8, ptr += 8 )
                {
                    s1 += ptr[0], s2 += s1;
                    s1 += ptr[1], s2 += s1;
                    s1 += ptr[2], s2 += s1;
                    s1 += ptr[3], s2 += s1;
                    s1 += ptr[4], s2 += s1;
                    s1 += ptr[5], s2 += s1;
                    s1 += ptr[6], s2 += s1;
                    s1 += ptr[7], s2 += s1;
                }
                for ( ; i < block_len; ++i )
                    s1 += *ptr++, s2 += s1;
                s1 %= 65521U, s2 %= 65521U;
                buf_len -= block_len;
                block_len = 5552;
            }
            r->m_check_adler32 = ( s2 << 16 ) + s1;
            if ( ( status == TINFL_STATUS_DONE ) && ( decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER ) && ( r->m_check_adler32 != r->m_z_adler32 ) )
                status = TINFL_STATUS_ADLER32_MISMATCH;
        }
        return status;
    }
    void * tinfl_decompress_mem_to_heap( const void * pSrc_buf, size_t src_buf_len, size_t * pOut_len, int flags )
    {
        tinfl_decompressor decomp;
        void * pBuf = NULL, * pNew_buf;
        size_t src_buf_ofs = 0, out_buf_capacity = 0;
        *pOut_len = 0;
        tinfl_init( &decomp );
        for ( ;;)
        {
            size_t src_buf_size = src_buf_len - src_buf_ofs, dst_buf_size = out_buf_capacity - *pOut_len, new_out_buf_capacity;
            tinfl_status status = tinfl_decompress( &decomp, (const mz_uint8 *)pSrc_buf + src_buf_ofs, &src_buf_size, (mz_uint8 *)pBuf,
                                                    pBuf ? (mz_uint8 *)pBuf + *pOut_len : NULL, &dst_buf_size, ( flags & ~TINFL_FLAG_HAS_MORE_INPUT ) | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF );
            if ( ( status < 0 ) || ( status == TINFL_STATUS_NEEDS_MORE_INPUT ) )
            {
                free( pBuf );
                *pOut_len = 0;
                return NULL;
            }
            src_buf_ofs += src_buf_size;
            *pOut_len += dst_buf_size;
            if ( status == TINFL_STATUS_DONE )
                break;
            new_out_buf_capacity = out_buf_capacity * 2;
            if ( new_out_buf_capacity < 128 )
                new_out_buf_capacity = 128;
            pNew_buf = realloc( pBuf, new_out_buf_capacity );
            if ( !pNew_buf )
            {
                free( pBuf );
                *pOut_len = 0;
                return NULL;
            }
            pBuf = pNew_buf;
            out_buf_capacity = new_out_buf_capacity;
        }
        return pBuf;
    }
    size_t tinfl_decompress_mem_to_mem( void * pOut_buf, size_t out_buf_len, const void * pSrc_buf, size_t src_buf_len, int flags )
    {
        tinfl_decompressor decomp;
        tinfl_status status;
        tinfl_init( &decomp );
        status = tinfl_decompress( &decomp, (const mz_uint8 *)pSrc_buf, &src_buf_len, (mz_uint8 *)pOut_buf, (mz_uint8 *)pOut_buf, &out_buf_len,
                                   ( flags & ~TINFL_FLAG_HAS_MORE_INPUT ) | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF );
        return ( status != TINFL_STATUS_DONE ) ? TINFL_DECOMPRESS_MEM_TO_MEM_FAILED : out_buf_len;
    }
    int tinfl_decompress_mem_to_callback( const void * pIn_buf, size_t * pIn_buf_size, tinfl_put_buf_func_ptr pPut_buf_func, void * pPut_buf_user, int flags )
    {
        int result = 0;
        tinfl_decompressor decomp;
        mz_uint8 * pDict = (mz_uint8 *)malloc( TINFL_LZ_DICT_SIZE );
        size_t in_buf_ofs = 0, dict_ofs = 0;
        if ( !pDict )
            return TINFL_STATUS_FAILED;
        tinfl_init( &decomp );
        for ( ;;)
        {
            size_t in_buf_size = *pIn_buf_size - in_buf_ofs, dst_buf_size = TINFL_LZ_DICT_SIZE - dict_ofs;
            tinfl_status status = tinfl_decompress( &decomp, (const mz_uint8 *)pIn_buf + in_buf_ofs, &in_buf_size, pDict, pDict + dict_ofs, &dst_buf_size,
                                                    ( flags & ~( TINFL_FLAG_HAS_MORE_INPUT | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF ) ) );
            in_buf_ofs += in_buf_size;
            if ( ( dst_buf_size ) && ( !( *pPut_buf_func )( pDict + dict_ofs, (int)dst_buf_size, pPut_buf_user ) ) )
                break;
            if ( status != TINFL_STATUS_HAS_MORE_OUTPUT )
            {
                result = ( status == TINFL_STATUS_DONE );
                break;
            }
            dict_ofs = ( dict_ofs + dst_buf_size ) & ( TINFL_LZ_DICT_SIZE - 1 );
        }
        free( pDict );
        *pIn_buf_size = in_buf_ofs;
        return result;
    }
    tinfl_decompressor * tinfl_decompressor_alloc()
    {
        tinfl_decompressor * pDecomp = (tinfl_decompressor *)malloc( sizeof( tinfl_decompressor ) );
        if ( pDecomp )
            tinfl_init( pDecomp );
        return pDecomp;
    }
    void tinfl_decompressor_free( tinfl_decompressor * pDecomp )
    {
        free( pDecomp );
    }
    static void mz_zip_array_init( mz_zip_array * pArray, mz_uint32 element_size )
    {
        memset( pArray, 0, sizeof( mz_zip_array ) );
        pArray->m_element_size = element_size;
    }
    static void mz_zip_array_clear( mz_zip_archive * pZip, mz_zip_array * pArray )
    {
        pZip->m_pFree( pZip->m_pAlloc_opaque, pArray->m_p );
        memset( pArray, 0, sizeof( mz_zip_array ) );
    }
    static mz_bool mz_zip_array_ensure_capacity( mz_zip_archive * pZip, mz_zip_array * pArray, size_t min_new_capacity, mz_uint32 growing )
    {
        void * pNew_p;
        size_t new_capacity = min_new_capacity;
        assert( pArray->m_element_size );
        if ( pArray->m_capacity >= min_new_capacity )
            return true;
        if ( growing )
        {
            new_capacity = MZ_MAX( 1, pArray->m_capacity );
            while ( new_capacity < min_new_capacity )
                new_capacity *= 2;
        }
        if ( NULL == ( pNew_p = pZip->m_pRealloc( pZip->m_pAlloc_opaque, pArray->m_p, pArray->m_element_size, new_capacity ) ) )
            return false;
        pArray->m_p = pNew_p;
        pArray->m_capacity = new_capacity;
        return true;
    }
    static mz_bool mz_zip_array_reserve( mz_zip_archive * pZip, mz_zip_array * pArray, size_t new_capacity, mz_uint32 growing )
    {
        if ( new_capacity > pArray->m_capacity )
        {
            if ( !mz_zip_array_ensure_capacity( pZip, pArray, new_capacity, growing ) )
                return false;
        }
        return true;
    }
    static mz_bool mz_zip_array_resize( mz_zip_archive * pZip, mz_zip_array * pArray, size_t new_size, mz_uint32 growing )
    {
        if ( new_size > pArray->m_capacity )
        {
            if ( !mz_zip_array_ensure_capacity( pZip, pArray, new_size, growing ) )
                return false;
        }
        pArray->m_size = new_size;
        return true;
    }
    static mz_bool mz_zip_array_ensure_room( mz_zip_archive * pZip, mz_zip_array * pArray, size_t n )
    {
        return mz_zip_array_reserve( pZip, pArray, pArray->m_size + n, true );
    }
    static mz_bool mz_zip_array_push_back( mz_zip_archive * pZip, mz_zip_array * pArray, const void * pElements, size_t n )
    {
        size_t orig_size = pArray->m_size;
        if ( !mz_zip_array_resize( pZip, pArray, orig_size + n, true ) )
            return false;
        if ( n > 0 )
            memcpy( (mz_uint8 *)pArray->m_p + orig_size * pArray->m_element_size, pElements, n * pArray->m_element_size );
        return true;
    }
    static time_t mz_zip_dos_to_time_t( int dos_time, int dos_date )
    {
        struct tm tm;
        memset( &tm, 0, sizeof( tm ) );
        tm.tm_isdst = -1;
        tm.tm_year = ( ( dos_date >> 9 ) & 127 ) + 1980 - 1900;
        tm.tm_mon = ( ( dos_date >> 5 ) & 15 ) - 1;
        tm.tm_mday = dos_date & 31;
        tm.tm_hour = ( dos_time >> 11 ) & 31;
        tm.tm_min = ( dos_time >> 5 ) & 63;
        tm.tm_sec = ( dos_time << 1 ) & 62;
        return mktime( &tm );
    }
    static void mz_zip_time_t_to_dos_time( time_t time, mz_uint16 * pDOS_time, mz_uint16 * pDOS_date )
    {
#ifdef _MSC_VER
        struct tm tm_struct;
        struct tm * tm = &tm_struct;
        errno_t err = localtime_s( tm, &time );
        if ( err )
        {
            *pDOS_date = 0;
            *pDOS_time = 0;
            return;
        }
#else
        struct tm * tm = localtime( &time );
#endif 

        * pDOS_time = (mz_uint16)( ( ( tm->tm_hour ) << 11 ) + ( ( tm->tm_min ) << 5 ) + ( ( tm->tm_sec ) >> 1 ) );
        *pDOS_date = (mz_uint16)( ( ( tm->tm_year + 1900 - 1980 ) << 9 ) + ( ( tm->tm_mon + 1 ) << 5 ) + tm->tm_mday );
    }
    static mz_bool mz_zip_get_file_modified_time( const char * pFilename, time_t * pTime )
    {
        struct MZ_FILE_STAT_STRUCT file_stat;
        if ( MZ_FILE_STAT( pFilename, &file_stat ) != 0 )
            return false;

        *pTime = file_stat.st_mtime;

        return true;
    }
    static mz_bool mz_zip_set_file_times( const char * pFilename, time_t access_time, time_t modified_time )
    {
        struct utimbuf t;

        memset( &t, 0, sizeof( t ) );
        t.actime = access_time;
        t.modtime = modified_time;

        return !utime( pFilename, &t );
    }
    static mz_bool mz_zip_set_error( mz_zip_archive * pZip, mz_zip_error err_num )
    {
        if ( pZip )
            pZip->m_last_error = err_num;
        return false;
    }
    static mz_bool mz_zip_reader_init_internal( mz_zip_archive * pZip, mz_uint32 flags )
    {
        (void)flags;
        if ( ( !pZip ) || ( pZip->m_pState ) || ( pZip->m_zip_mode != MZ_ZIP_MODE_INVALID ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( !pZip->m_pAlloc )
            pZip->m_pAlloc = miniz_def_alloc_func;
        if ( !pZip->m_pFree )
            pZip->m_pFree = miniz_def_free_func;
        if ( !pZip->m_pRealloc )
            pZip->m_pRealloc = miniz_def_realloc_func;

        pZip->m_archive_size = 0;
        pZip->m_central_directory_file_ofs = 0;
        pZip->m_total_files = 0;
        pZip->m_last_error = MZ_ZIP_NO_ERROR;

        if ( NULL == ( pZip->m_pState = (mz_zip_internal_state *)pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, sizeof( mz_zip_internal_state ) ) ) )
            return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );

        memset( pZip->m_pState, 0, sizeof( mz_zip_internal_state ) );
        MZ_ZIP_ARRAY_SET_ELEMENT_SIZE( &pZip->m_pState->m_central_dir, sizeof( mz_uint8 ) );
        MZ_ZIP_ARRAY_SET_ELEMENT_SIZE( &pZip->m_pState->m_central_dir_offsets, sizeof( mz_uint32 ) );
        MZ_ZIP_ARRAY_SET_ELEMENT_SIZE( &pZip->m_pState->m_sorted_central_dir_offsets, sizeof( mz_uint32 ) );
        pZip->m_pState->m_init_flags = flags;
        pZip->m_pState->m_zip64 = false;
        pZip->m_pState->m_zip64_has_extended_info_fields = false;

        pZip->m_zip_mode = MZ_ZIP_MODE_READING;

        return true;
    }
    static mz_bool mz_zip_reader_filename_less( const mz_zip_array * pCentral_dir_array, const mz_zip_array * pCentral_dir_offsets, mz_uint32 l_index, mz_uint32 r_index )
    {
        const mz_uint8 * pL = &MZ_ZIP_ARRAY_ELEMENT( pCentral_dir_array, mz_uint8, MZ_ZIP_ARRAY_ELEMENT( pCentral_dir_offsets, mz_uint32, l_index ) ), * pE;
        const mz_uint8 * pR = &MZ_ZIP_ARRAY_ELEMENT( pCentral_dir_array, mz_uint8, MZ_ZIP_ARRAY_ELEMENT( pCentral_dir_offsets, mz_uint32, r_index ) );
        mz_uint32 l_len = MZ_READ_LE16( pL + MZ_ZIP_CDH_FILENAME_LEN_OFS ), r_len = MZ_READ_LE16( pR + MZ_ZIP_CDH_FILENAME_LEN_OFS );
        mz_uint8 l = 0, r = 0;
        pL += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
        pR += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
        pE = pL + MZ_MIN( l_len, r_len );
        while ( pL < pE )
        {
            if ( ( l = MZ_TOLOWER( *pL ) ) != ( r = MZ_TOLOWER( *pR ) ) )
                break;
            pL++;
            pR++;
        }
        return ( pL == pE ) ? ( l_len < r_len ) : ( l < r );
    }
    static void mz_zip_reader_sort_central_dir_offsets_by_filename( mz_zip_archive * pZip )
    {
        mz_zip_internal_state * pState = pZip->m_pState;
        const mz_zip_array * pCentral_dir_offsets = &pState->m_central_dir_offsets;
        const mz_zip_array * pCentral_dir = &pState->m_central_dir;
        mz_uint32 * pIndices;
        mz_uint32 start, end;
        const mz_uint32 size = pZip->m_total_files;

        if ( size <= 1U )
            return;

        pIndices = &MZ_ZIP_ARRAY_ELEMENT( &pState->m_sorted_central_dir_offsets, mz_uint32, 0 );

        start = ( size - 2U ) >> 1U;
        for ( ;;)
        {
            mz_uint64 child, root = start;
            for ( ;;)
            {
                if ( ( child = ( root << 1U ) + 1U ) >= size )
                    break;
                child += ( ( ( child + 1U ) < size ) && ( mz_zip_reader_filename_less( pCentral_dir, pCentral_dir_offsets, pIndices[child], pIndices[child + 1U] ) ) );
                if ( !mz_zip_reader_filename_less( pCentral_dir, pCentral_dir_offsets, pIndices[root], pIndices[child] ) )
                    break;
                std::swap( pIndices[root], pIndices[child] );
                root = child;
            }
            if ( !start )
                break;
            start--;
        }

        end = size - 1;
        while ( end > 0 )
        {
            mz_uint64 child, root = 0;
            std::swap( pIndices[end], pIndices[0] );
            for ( ;;)
            {
                if ( ( child = ( root << 1U ) + 1U ) >= end )
                    break;
                child += ( ( ( child + 1U ) < end ) && mz_zip_reader_filename_less( pCentral_dir, pCentral_dir_offsets, pIndices[child], pIndices[child + 1U] ) );
                if ( !mz_zip_reader_filename_less( pCentral_dir, pCentral_dir_offsets, pIndices[root], pIndices[child] ) )
                    break;
                std::swap( pIndices[root], pIndices[child] );
                root = child;
            }
            end--;
        }
    }
    static mz_bool mz_zip_reader_locate_header_sig( mz_zip_archive * pZip, mz_uint32 record_sig, mz_uint32 record_size, mz_int64 * pOfs )
    {
        mz_int64 cur_file_ofs;
        mz_uint32 buf_u32[4096 / sizeof( mz_uint32 )];
        mz_uint8 * pBuf = (mz_uint8 *)buf_u32;
        if ( pZip->m_archive_size < record_size )
            return false;
        cur_file_ofs = MZ_MAX( (mz_int64)pZip->m_archive_size - (mz_int64)sizeof( buf_u32 ), 0 );
        for ( ;;)
        {
            int i, n = (int)MZ_MIN( sizeof( buf_u32 ), pZip->m_archive_size - cur_file_ofs );

            if ( pZip->m_pRead( pZip->m_pIO_opaque, cur_file_ofs, pBuf, n ) != (mz_uint32)n )
                return false;

            for ( i = n - 4; i >= 0; --i )
            {
                mz_uint32 s = MZ_READ_LE32( pBuf + i );
                if ( s == record_sig )
                {
                    if ( ( pZip->m_archive_size - ( cur_file_ofs + i ) ) >= record_size )
                        break;
                }
            }

            if ( i >= 0 )
            {
                cur_file_ofs += i;
                break;
            }
            if ( ( !cur_file_ofs ) || ( ( pZip->m_archive_size - cur_file_ofs ) >= ( MZ_UINT16_MAX + record_size ) ) )
                return false;

            cur_file_ofs = MZ_MAX( cur_file_ofs - ( sizeof( buf_u32 ) - 3 ), 0 );
        }

        *pOfs = cur_file_ofs;
        return true;
    }
    static mz_bool mz_zip_reader_read_central_dir( mz_zip_archive * pZip, mz_uint32 flags )
    {
        mz_uint32 cdir_size = 0, cdir_entries_on_this_disk = 0, num_this_disk = 0, cdir_disk_index = 0;
        mz_uint64 cdir_ofs = 0;
        mz_int64 cur_file_ofs = 0;
        const mz_uint8 * p;

        mz_uint32 buf_u32[4096 / sizeof( mz_uint32 )];
        mz_uint8 * pBuf = (mz_uint8 *)buf_u32;
        mz_bool sort_central_dir = ( ( flags & MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY ) == 0 );
        mz_uint32 zip64_end_of_central_dir_locator_u32[( MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE + sizeof( mz_uint32 ) - 1 ) / sizeof( mz_uint32 )];
        mz_uint8 * pZip64_locator = (mz_uint8 *)zip64_end_of_central_dir_locator_u32;

        mz_uint32 zip64_end_of_central_dir_header_u32[( MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE + sizeof( mz_uint32 ) - 1 ) / sizeof( mz_uint32 )];
        mz_uint8 * pZip64_end_of_central_dir = (mz_uint8 *)zip64_end_of_central_dir_header_u32;

        mz_uint64 zip64_end_of_central_dir_ofs = 0;
        if ( pZip->m_archive_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE )
            return mz_zip_set_error( pZip, MZ_ZIP_NOT_AN_ARCHIVE );

        if ( !mz_zip_reader_locate_header_sig( pZip, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE, &cur_file_ofs ) )
            return mz_zip_set_error( pZip, MZ_ZIP_FAILED_FINDING_CENTRAL_DIR );
        if ( pZip->m_pRead( pZip->m_pIO_opaque, cur_file_ofs, pBuf, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE ) != MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );

        if ( MZ_READ_LE32( pBuf + MZ_ZIP_ECDH_SIG_OFS ) != MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG )
            return mz_zip_set_error( pZip, MZ_ZIP_NOT_AN_ARCHIVE );

        if ( cur_file_ofs >= ( MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE + MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE ) )
        {
            if ( pZip->m_pRead( pZip->m_pIO_opaque, cur_file_ofs - MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE, pZip64_locator,
                 MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE ) == MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE )
            {
                if ( MZ_READ_LE32( pZip64_locator + MZ_ZIP64_ECDL_SIG_OFS ) == MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG )
                {
                    zip64_end_of_central_dir_ofs = MZ_READ_LE64( pZip64_locator + MZ_ZIP64_ECDL_REL_OFS_TO_ZIP64_ECDR_OFS );
                    if ( zip64_end_of_central_dir_ofs > ( pZip->m_archive_size - MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE ) )
                        return mz_zip_set_error( pZip, MZ_ZIP_NOT_AN_ARCHIVE );

                    if ( pZip->m_pRead( pZip->m_pIO_opaque, zip64_end_of_central_dir_ofs, pZip64_end_of_central_dir, MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE ) ==
                         MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE )
                    {
                        if ( MZ_READ_LE32( pZip64_end_of_central_dir + MZ_ZIP64_ECDH_SIG_OFS ) == MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIG )
                        {
                            pZip->m_pState->m_zip64 = true;
                        }
                    }
                }
            }
        }

        pZip->m_total_files = MZ_READ_LE16( pBuf + MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS );
        cdir_entries_on_this_disk = MZ_READ_LE16( pBuf + MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS );
        num_this_disk = MZ_READ_LE16( pBuf + MZ_ZIP_ECDH_NUM_THIS_DISK_OFS );
        cdir_disk_index = MZ_READ_LE16( pBuf + MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS );
        cdir_size = MZ_READ_LE32( pBuf + MZ_ZIP_ECDH_CDIR_SIZE_OFS );
        cdir_ofs = MZ_READ_LE32( pBuf + MZ_ZIP_ECDH_CDIR_OFS_OFS );

        if ( pZip->m_pState->m_zip64 )
        {
            mz_uint32 zip64_total_num_of_disks = MZ_READ_LE32( pZip64_locator + MZ_ZIP64_ECDL_TOTAL_NUMBER_OF_DISKS_OFS );
            mz_uint64 zip64_cdir_total_entries = MZ_READ_LE64( pZip64_end_of_central_dir + MZ_ZIP64_ECDH_CDIR_TOTAL_ENTRIES_OFS );
            mz_uint64 zip64_cdir_total_entries_on_this_disk = MZ_READ_LE64( pZip64_end_of_central_dir + MZ_ZIP64_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS );
            mz_uint64 zip64_size_of_end_of_central_dir_record = MZ_READ_LE64( pZip64_end_of_central_dir + MZ_ZIP64_ECDH_SIZE_OF_RECORD_OFS );
            mz_uint64 zip64_size_of_central_directory = MZ_READ_LE64( pZip64_end_of_central_dir + MZ_ZIP64_ECDH_CDIR_SIZE_OFS );

            if ( zip64_size_of_end_of_central_dir_record < ( MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE - 12 ) )
                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

            if ( zip64_total_num_of_disks != 1U )
                return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_MULTIDISK );
            if ( zip64_cdir_total_entries > MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_TOO_MANY_FILES );

            pZip->m_total_files = (mz_uint32)zip64_cdir_total_entries;

            if ( zip64_cdir_total_entries_on_this_disk > MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_TOO_MANY_FILES );

            cdir_entries_on_this_disk = (mz_uint32)zip64_cdir_total_entries_on_this_disk;
            if ( zip64_size_of_central_directory > MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE );

            cdir_size = (mz_uint32)zip64_size_of_central_directory;

            num_this_disk = MZ_READ_LE32( pZip64_end_of_central_dir + MZ_ZIP64_ECDH_NUM_THIS_DISK_OFS );

            cdir_disk_index = MZ_READ_LE32( pZip64_end_of_central_dir + MZ_ZIP64_ECDH_NUM_DISK_CDIR_OFS );

            cdir_ofs = MZ_READ_LE64( pZip64_end_of_central_dir + MZ_ZIP64_ECDH_CDIR_OFS_OFS );
        }

        if ( pZip->m_total_files != cdir_entries_on_this_disk )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_MULTIDISK );

        if ( ( ( num_this_disk | cdir_disk_index ) != 0 ) && ( ( num_this_disk != 1 ) || ( cdir_disk_index != 1 ) ) )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_MULTIDISK );

        if ( cdir_size < pZip->m_total_files * MZ_ZIP_CENTRAL_DIR_HEADER_SIZE )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

        if ( ( cdir_ofs + (mz_uint64)cdir_size ) > pZip->m_archive_size )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

        pZip->m_central_directory_file_ofs = cdir_ofs;

        if ( pZip->m_total_files )
        {
            mz_uint32 i, n;

            if ( ( !mz_zip_array_resize( pZip, &pZip->m_pState->m_central_dir, cdir_size, false ) ) ||
                 ( !mz_zip_array_resize( pZip, &pZip->m_pState->m_central_dir_offsets, pZip->m_total_files, false ) ) )
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );

            if ( sort_central_dir )
            {
                if ( !mz_zip_array_resize( pZip, &pZip->m_pState->m_sorted_central_dir_offsets, pZip->m_total_files, false ) )
                    return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            }

            if ( pZip->m_pRead( pZip->m_pIO_opaque, cdir_ofs, pZip->m_pState->m_central_dir.m_p, cdir_size ) != cdir_size )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
            p = (const mz_uint8 *)pZip->m_pState->m_central_dir.m_p;
            for ( n = cdir_size, i = 0; i < pZip->m_total_files; ++i )
            {
                mz_uint32 total_header_size, disk_index, bit_flags, filename_size, ext_data_size;
                mz_uint64 comp_size, decomp_size, local_header_ofs;

                if ( ( n < MZ_ZIP_CENTRAL_DIR_HEADER_SIZE ) || ( MZ_READ_LE32( p ) != MZ_ZIP_CENTRAL_DIR_HEADER_SIG ) )
                    return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

                MZ_ZIP_ARRAY_ELEMENT( &pZip->m_pState->m_central_dir_offsets, mz_uint32, i ) =
                    (mz_uint32)( p - (const mz_uint8 *)pZip->m_pState->m_central_dir.m_p );

                if ( sort_central_dir )
                    MZ_ZIP_ARRAY_ELEMENT( &pZip->m_pState->m_sorted_central_dir_offsets, mz_uint32, i ) = i;

                comp_size = MZ_READ_LE32( p + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS );
                decomp_size = MZ_READ_LE32( p + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS );
                local_header_ofs = MZ_READ_LE32( p + MZ_ZIP_CDH_LOCAL_HEADER_OFS );
                filename_size = MZ_READ_LE16( p + MZ_ZIP_CDH_FILENAME_LEN_OFS );
                ext_data_size = MZ_READ_LE16( p + MZ_ZIP_CDH_EXTRA_LEN_OFS );

                if ( ( !pZip->m_pState->m_zip64_has_extended_info_fields ) && ( ext_data_size ) &&
                     ( MZ_MAX( MZ_MAX( comp_size, decomp_size ), local_header_ofs ) == MZ_UINT32_MAX ) )
                {

                    mz_uint32 extra_size_remaining = ext_data_size;

                    if ( extra_size_remaining )
                    {
                        const mz_uint8 * pExtra_data;
                        void * buf = NULL;

                        if ( MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size + ext_data_size > n )
                        {
                            buf = malloc( ext_data_size );
                            if ( buf == NULL )
                                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );

                            if ( pZip->m_pRead( pZip->m_pIO_opaque, cdir_ofs + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size, buf, ext_data_size ) !=
                                 ext_data_size )
                            {
                                free( buf );
                                return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
                            }

                            pExtra_data = (mz_uint8 *)buf;
                        }
                        else
                        {
                            pExtra_data = p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size;
                        }

                        do
                        {
                            mz_uint32 field_id;
                            mz_uint32 field_data_size;

                            if ( extra_size_remaining < ( sizeof( mz_uint16 ) * 2 ) )
                            {
                                free( buf );
                                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
                            }

                            field_id = MZ_READ_LE16( pExtra_data );
                            field_data_size = MZ_READ_LE16( pExtra_data + sizeof( mz_uint16 ) );

                            if ( ( field_data_size + sizeof( mz_uint16 ) * 2 ) > extra_size_remaining )
                            {
                                free( buf );
                                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
                            }

                            if ( field_id == MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID )
                            {
                                pZip->m_pState->m_zip64 = true;
                                pZip->m_pState->m_zip64_has_extended_info_fields = true;
                                break;
                            }

                            pExtra_data += sizeof( mz_uint16 ) * 2 + field_data_size;
                            extra_size_remaining = extra_size_remaining - sizeof( mz_uint16 ) * 2 - field_data_size;
                        } while ( extra_size_remaining );

                        free( buf );
                    }
                }
                if ( ( comp_size != MZ_UINT32_MAX ) && ( decomp_size != MZ_UINT32_MAX ) )
                {
                    if ( ( ( !MZ_READ_LE32( p + MZ_ZIP_CDH_METHOD_OFS ) ) && ( decomp_size != comp_size ) ) || ( decomp_size && !comp_size ) )
                        return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
                }

                disk_index = MZ_READ_LE16( p + MZ_ZIP_CDH_DISK_START_OFS );
                if ( ( disk_index == MZ_UINT16_MAX ) || ( ( disk_index != num_this_disk ) && ( disk_index != 1 ) ) )
                    return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_MULTIDISK );

                if ( comp_size != MZ_UINT32_MAX )
                {
                    if ( ( (mz_uint64)MZ_READ_LE32( p + MZ_ZIP_CDH_LOCAL_HEADER_OFS ) + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + comp_size ) > pZip->m_archive_size )
                        return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
                }

                bit_flags = MZ_READ_LE16( p + MZ_ZIP_CDH_BIT_FLAG_OFS );
                if ( bit_flags & MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_LOCAL_DIR_IS_MASKED )
                    return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION );

                if ( ( total_header_size = MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + MZ_READ_LE16( p + MZ_ZIP_CDH_FILENAME_LEN_OFS ) +
                     MZ_READ_LE16( p + MZ_ZIP_CDH_EXTRA_LEN_OFS ) + MZ_READ_LE16( p + MZ_ZIP_CDH_COMMENT_LEN_OFS ) ) > n )
                    return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

                n -= total_header_size;
                p += total_header_size;
            }
        }

        if ( sort_central_dir )
            mz_zip_reader_sort_central_dir_offsets_by_filename( pZip );

        return true;
    }
    void mz_zip_zero_struct( mz_zip_archive * pZip )
    {
        if ( pZip )
            MZ_CLEAR_OBJ( *pZip );
    }
    static mz_bool mz_zip_reader_end_internal( mz_zip_archive * pZip, mz_bool set_last_error )
    {
        mz_bool status = true;

        if ( !pZip )
            return false;

        if ( ( !pZip->m_pState ) || ( !pZip->m_pAlloc ) || ( !pZip->m_pFree ) || ( pZip->m_zip_mode != MZ_ZIP_MODE_READING ) )
        {
            if ( set_last_error )
                pZip->m_last_error = MZ_ZIP_INVALID_PARAMETER;

            return false;
        }

        if ( pZip->m_pState )
        {
            mz_zip_internal_state * pState = pZip->m_pState;
            pZip->m_pState = NULL;

            mz_zip_array_clear( pZip, &pState->m_central_dir );
            mz_zip_array_clear( pZip, &pState->m_central_dir_offsets );
            mz_zip_array_clear( pZip, &pState->m_sorted_central_dir_offsets );

            if ( pState->m_pFile )
            {
                if ( pZip->m_zip_type == MZ_ZIP_TYPE_FILE )
                {
                    if ( MZ_FCLOSE( pState->m_pFile ) == EOF )
                    {
                        if ( set_last_error )
                            pZip->m_last_error = MZ_ZIP_FILE_CLOSE_FAILED;
                        status = false;
                    }
                }
                pState->m_pFile = NULL;
            }

            pZip->m_pFree( pZip->m_pAlloc_opaque, pState );
        }
        pZip->m_zip_mode = MZ_ZIP_MODE_INVALID;

        return status;
    }
    mz_bool mz_zip_reader_end( mz_zip_archive * pZip )
    {
        return mz_zip_reader_end_internal( pZip, true );
    }
    mz_bool mz_zip_reader_init( mz_zip_archive * pZip, mz_uint64 size, mz_uint32 flags )
    {
        if ( ( !pZip ) || ( !pZip->m_pRead ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( !mz_zip_reader_init_internal( pZip, flags ) )
            return false;

        pZip->m_zip_type = MZ_ZIP_TYPE_USER;
        pZip->m_archive_size = size;

        if ( !mz_zip_reader_read_central_dir( pZip, flags ) )
        {
            mz_zip_reader_end_internal( pZip, false );
            return false;
        }

        return true;
    }
    static size_t mz_zip_mem_read_func( void * pOpaque, mz_uint64 file_ofs, void * pBuf, size_t n )
    {
        mz_zip_archive * pZip = (mz_zip_archive *)pOpaque;
        size_t s = ( file_ofs >= pZip->m_archive_size ) ? 0 : (size_t)MZ_MIN( pZip->m_archive_size - file_ofs, n );
        memcpy( pBuf, (const mz_uint8 *)pZip->m_pState->m_pMem + file_ofs, s );
        return s;
    }
    mz_bool mz_zip_reader_init_mem( mz_zip_archive * pZip, const void * pMem, size_t size, mz_uint32 flags )
    {
        if ( !pMem )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE )
            return mz_zip_set_error( pZip, MZ_ZIP_NOT_AN_ARCHIVE );

        if ( !mz_zip_reader_init_internal( pZip, flags ) )
            return false;

        pZip->m_zip_type = MZ_ZIP_TYPE_MEMORY;
        pZip->m_archive_size = size;
        pZip->m_pRead = mz_zip_mem_read_func;
        pZip->m_pIO_opaque = pZip;
        pZip->m_pNeeds_keepalive = NULL;

        pZip->m_pState->m_pMem = (void *)pMem;

        pZip->m_pState->m_mem_size = size;

        if ( !mz_zip_reader_read_central_dir( pZip, flags ) )
        {
            mz_zip_reader_end_internal( pZip, false );
            return false;
        }

        return true;
    }
    static size_t mz_zip_file_read_func( void * pOpaque, mz_uint64 file_ofs, void * pBuf, size_t n )
    {
        mz_zip_archive * pZip = (mz_zip_archive *)pOpaque;
        mz_int64 cur_ofs = MZ_FTELL64( pZip->m_pState->m_pFile );

        file_ofs += pZip->m_pState->m_file_archive_start_ofs;

        if ( ( (mz_int64)file_ofs < 0 ) || ( ( ( cur_ofs != (mz_int64)file_ofs ) ) && ( MZ_FSEEK64( pZip->m_pState->m_pFile, (mz_int64)file_ofs, SEEK_SET ) ) ) )
            return 0;

        return MZ_FREAD( pBuf, 1, n, pZip->m_pState->m_pFile );
    }
    mz_bool mz_zip_reader_init_file( mz_zip_archive * pZip, const char * pFilename, mz_uint32 flags )
    {
        return mz_zip_reader_init_file_v2( pZip, pFilename, flags, 0, 0 );
    }
    mz_bool mz_zip_reader_init_file_v2( mz_zip_archive * pZip, const char * pFilename, mz_uint32 flags, mz_uint64 file_start_ofs, mz_uint64 archive_size )
    {
        mz_uint64 file_size;
        FILE * pFile;

        if ( ( !pZip ) || ( !pFilename ) || ( ( archive_size ) && ( archive_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE ) ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        pFile = MZ_FOPEN( pFilename, "rb" );
        if ( !pFile )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_OPEN_FAILED );

        file_size = archive_size;
        if ( !file_size )
        {
            if ( MZ_FSEEK64( pFile, 0, SEEK_END ) )
            {
                MZ_FCLOSE( pFile );
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_SEEK_FAILED );
            }

            file_size = MZ_FTELL64( pFile );
        }

        if ( file_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE )
        {
            MZ_FCLOSE( pFile );
            return mz_zip_set_error( pZip, MZ_ZIP_NOT_AN_ARCHIVE );
        }

        if ( !mz_zip_reader_init_internal( pZip, flags ) )
        {
            MZ_FCLOSE( pFile );
            return false;
        }

        pZip->m_zip_type = MZ_ZIP_TYPE_FILE;
        pZip->m_pRead = mz_zip_file_read_func;
        pZip->m_pIO_opaque = pZip;
        pZip->m_pState->m_pFile = pFile;
        pZip->m_archive_size = file_size;
        pZip->m_pState->m_file_archive_start_ofs = file_start_ofs;

        if ( !mz_zip_reader_read_central_dir( pZip, flags ) )
        {
            mz_zip_reader_end_internal( pZip, false );
            return false;
        }

        return true;
    }
    mz_bool mz_zip_reader_init_cfile( mz_zip_archive * pZip, FILE * pFile, mz_uint64 archive_size, mz_uint32 flags )
    {
        mz_uint64 cur_file_ofs;

        if ( ( !pZip ) || ( !pFile ) )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_OPEN_FAILED );

        cur_file_ofs = MZ_FTELL64( pFile );

        if ( !archive_size )
        {
            if ( MZ_FSEEK64( pFile, 0, SEEK_END ) )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_SEEK_FAILED );

            archive_size = MZ_FTELL64( pFile ) - cur_file_ofs;

            if ( archive_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE )
                return mz_zip_set_error( pZip, MZ_ZIP_NOT_AN_ARCHIVE );
        }

        if ( !mz_zip_reader_init_internal( pZip, flags ) )
            return false;

        pZip->m_zip_type = MZ_ZIP_TYPE_CFILE;
        pZip->m_pRead = mz_zip_file_read_func;

        pZip->m_pIO_opaque = pZip;
        pZip->m_pState->m_pFile = pFile;
        pZip->m_archive_size = archive_size;
        pZip->m_pState->m_file_archive_start_ofs = cur_file_ofs;

        if ( !mz_zip_reader_read_central_dir( pZip, flags ) )
        {
            mz_zip_reader_end_internal( pZip, false );
            return false;
        }

        return true;
    }
    static const mz_uint8 * mz_zip_get_cdh( mz_zip_archive * pZip, mz_uint32 file_index )
    {
        if ( ( !pZip ) || ( !pZip->m_pState ) || ( file_index >= pZip->m_total_files ) )
            return NULL;
        return &MZ_ZIP_ARRAY_ELEMENT(
            &pZip->m_pState->m_central_dir, mz_uint8, MZ_ZIP_ARRAY_ELEMENT( &pZip->m_pState->m_central_dir_offsets, mz_uint32, file_index ) );
    }
    mz_bool mz_zip_reader_is_file_encrypted( mz_zip_archive * pZip, mz_uint32 file_index )
    {
        mz_uint32 m_bit_flag;
        const mz_uint8 * p = mz_zip_get_cdh( pZip, file_index );
        if ( !p )
        {
            mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
            return false;
        }

        m_bit_flag = MZ_READ_LE16( p + MZ_ZIP_CDH_BIT_FLAG_OFS );
        return ( m_bit_flag & ( MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED | MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION ) ) != 0;
    }
    mz_bool mz_zip_reader_is_file_supported( mz_zip_archive * pZip, mz_uint32 file_index )
    {
        mz_uint32 bit_flag;
        mz_uint32 method;

        const mz_uint8 * p = mz_zip_get_cdh( pZip, file_index );
        if ( !p )
        {
            mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
            return false;
        }

        method = MZ_READ_LE16( p + MZ_ZIP_CDH_METHOD_OFS );
        bit_flag = MZ_READ_LE16( p + MZ_ZIP_CDH_BIT_FLAG_OFS );

        if ( ( method != 0 ) && ( method != MZ_DEFLATED ) )
        {
            mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_METHOD );
            return false;
        }

        if ( bit_flag & ( MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED | MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION ) )
        {
            mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION );
            return false;
        }

        if ( bit_flag & MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_COMPRESSED_PATCH_FLAG )
        {
            mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_FEATURE );
            return false;
        }

        return true;
    }
    mz_bool mz_zip_reader_is_file_a_directory( mz_zip_archive * pZip, mz_uint32 file_index )
    {
        mz_uint32 filename_len, attribute_mapping_id, external_attr;
        const mz_uint8 * p = mz_zip_get_cdh( pZip, file_index );
        if ( !p )
        {
            mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
            return false;
        }

        filename_len = MZ_READ_LE16( p + MZ_ZIP_CDH_FILENAME_LEN_OFS );
        if ( filename_len )
        {
            if ( *( p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_len - 1 ) == '/' )
                return true;
        }

        attribute_mapping_id = MZ_READ_LE16( p + MZ_ZIP_CDH_VERSION_MADE_BY_OFS ) >> 8;
        (void)attribute_mapping_id;

        external_attr = MZ_READ_LE32( p + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS );
        if ( ( external_attr & MZ_ZIP_DOS_DIR_ATTRIBUTE_BITFLAG ) != 0 )
        {
            return true;
        }

        return false;
    }
    static mz_bool mz_zip_file_stat_internal( mz_zip_archive * pZip, mz_uint32 file_index, const mz_uint8 * pCentral_dir_header, mz_zip_archive_file_stat * pStat, mz_bool * pFound_zip64_extra_data )
    {
        mz_uint32 n;
        const mz_uint8 * p = pCentral_dir_header;

        if ( pFound_zip64_extra_data )
            *pFound_zip64_extra_data = false;

        if ( ( !p ) || ( !pStat ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
        pStat->m_file_index = file_index;
        pStat->m_central_dir_ofs = MZ_ZIP_ARRAY_ELEMENT( &pZip->m_pState->m_central_dir_offsets, mz_uint32, file_index );
        pStat->m_version_made_by = MZ_READ_LE16( p + MZ_ZIP_CDH_VERSION_MADE_BY_OFS );
        pStat->m_version_needed = MZ_READ_LE16( p + MZ_ZIP_CDH_VERSION_NEEDED_OFS );
        pStat->m_bit_flag = MZ_READ_LE16( p + MZ_ZIP_CDH_BIT_FLAG_OFS );
        pStat->m_method = MZ_READ_LE16( p + MZ_ZIP_CDH_METHOD_OFS );
        pStat->m_time = mz_zip_dos_to_time_t( MZ_READ_LE16( p + MZ_ZIP_CDH_FILE_TIME_OFS ), MZ_READ_LE16( p + MZ_ZIP_CDH_FILE_DATE_OFS ) );
        pStat->m_crc32 = MZ_READ_LE32( p + MZ_ZIP_CDH_CRC32_OFS );
        pStat->m_comp_size = MZ_READ_LE32( p + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS );
        pStat->m_uncomp_size = MZ_READ_LE32( p + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS );
        pStat->m_internal_attr = MZ_READ_LE16( p + MZ_ZIP_CDH_INTERNAL_ATTR_OFS );
        pStat->m_external_attr = MZ_READ_LE32( p + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS );
        pStat->m_local_header_ofs = MZ_READ_LE32( p + MZ_ZIP_CDH_LOCAL_HEADER_OFS );
        n = MZ_READ_LE16( p + MZ_ZIP_CDH_FILENAME_LEN_OFS );
        n = MZ_MIN( n, MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE - 1 );
        memcpy( pStat->m_filename, p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n );
        pStat->m_filename[n] = '\0';

        n = MZ_READ_LE16( p + MZ_ZIP_CDH_COMMENT_LEN_OFS );
        n = MZ_MIN( n, MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE - 1 );
        pStat->m_comment_size = n;
        memcpy( pStat->m_comment,
                p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + MZ_READ_LE16( p + MZ_ZIP_CDH_FILENAME_LEN_OFS ) + MZ_READ_LE16( p + MZ_ZIP_CDH_EXTRA_LEN_OFS ), n );
        pStat->m_comment[n] = '\0';
        pStat->m_is_directory = mz_zip_reader_is_file_a_directory( pZip, file_index );
        pStat->m_is_encrypted = mz_zip_reader_is_file_encrypted( pZip, file_index );
        pStat->m_is_supported = mz_zip_reader_is_file_supported( pZip, file_index );

        if ( MZ_MAX( MZ_MAX( pStat->m_comp_size, pStat->m_uncomp_size ), pStat->m_local_header_ofs ) == MZ_UINT32_MAX )
        {

            mz_uint32 extra_size_remaining = MZ_READ_LE16( p + MZ_ZIP_CDH_EXTRA_LEN_OFS );

            if ( extra_size_remaining )
            {
                const mz_uint8 * pExtra_data = p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + MZ_READ_LE16( p + MZ_ZIP_CDH_FILENAME_LEN_OFS );

                do
                {
                    mz_uint32 field_id;
                    mz_uint32 field_data_size;

                    if ( extra_size_remaining < ( sizeof( mz_uint16 ) * 2 ) )
                        return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

                    field_id = MZ_READ_LE16( pExtra_data );
                    field_data_size = MZ_READ_LE16( pExtra_data + sizeof( mz_uint16 ) );

                    if ( ( field_data_size + sizeof( mz_uint16 ) * 2 ) > extra_size_remaining )
                        return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

                    if ( field_id == MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID )
                    {
                        const mz_uint8 * pField_data = pExtra_data + sizeof( mz_uint16 ) * 2;
                        mz_uint32 field_data_remaining = field_data_size;

                        if ( pFound_zip64_extra_data )
                            *pFound_zip64_extra_data = true;

                        if ( pStat->m_uncomp_size == MZ_UINT32_MAX )
                        {
                            if ( field_data_remaining < sizeof( mz_uint64 ) )
                                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

                            pStat->m_uncomp_size = MZ_READ_LE64( pField_data );
                            pField_data += sizeof( mz_uint64 );
                            field_data_remaining -= sizeof( mz_uint64 );
                        }

                        if ( pStat->m_comp_size == MZ_UINT32_MAX )
                        {
                            if ( field_data_remaining < sizeof( mz_uint64 ) )
                                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

                            pStat->m_comp_size = MZ_READ_LE64( pField_data );
                            pField_data += sizeof( mz_uint64 );
                            field_data_remaining -= sizeof( mz_uint64 );
                        }

                        if ( pStat->m_local_header_ofs == MZ_UINT32_MAX )
                        {
                            if ( field_data_remaining < sizeof( mz_uint64 ) )
                                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

                            pStat->m_local_header_ofs = MZ_READ_LE64( pField_data );
                            pField_data += sizeof( mz_uint64 );
                            field_data_remaining -= sizeof( mz_uint64 );
                        }

                        break;
                    }

                    pExtra_data += sizeof( mz_uint16 ) * 2 + field_data_size;
                    extra_size_remaining = extra_size_remaining - sizeof( mz_uint16 ) * 2 - field_data_size;
                } while ( extra_size_remaining );
            }
        }

        return true;
    }
    static mz_bool mz_zip_string_equal( const char * pA, const char * pB, mz_uint32 len, mz_uint32 flags )
    {
        mz_uint32 i;
        if ( flags & MZ_ZIP_FLAG_CASE_SENSITIVE )
            return 0 == memcmp( pA, pB, len );
        for ( i = 0; i < len; ++i )
            if ( MZ_TOLOWER( pA[i] ) != MZ_TOLOWER( pB[i] ) )
                return false;
        return true;
    }
    static int mz_zip_filename_compare( const mz_zip_array * pCentral_dir_array, const mz_zip_array * pCentral_dir_offsets, mz_uint32 l_index, const char * pR, mz_uint32 r_len )
    {
        const mz_uint8 * pL = &MZ_ZIP_ARRAY_ELEMENT( pCentral_dir_array, mz_uint8, MZ_ZIP_ARRAY_ELEMENT( pCentral_dir_offsets, mz_uint32, l_index ) ), * pE;
        mz_uint32 l_len = MZ_READ_LE16( pL + MZ_ZIP_CDH_FILENAME_LEN_OFS );
        mz_uint8 l = 0, r = 0;
        pL += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
        pE = pL + MZ_MIN( l_len, r_len );
        while ( pL < pE )
        {
            if ( ( l = MZ_TOLOWER( *pL ) ) != ( r = MZ_TOLOWER( *pR ) ) )
                break;
            pL++;
            pR++;
        }
        return ( pL == pE ) ? (int)( l_len - r_len ) : ( l - r );
    }
    static mz_bool mz_zip_locate_file_binary_search( mz_zip_archive * pZip, const char * pFilename, mz_uint32 * pIndex )
    {
        mz_zip_internal_state * pState = pZip->m_pState;
        const mz_zip_array * pCentral_dir_offsets = &pState->m_central_dir_offsets;
        const mz_zip_array * pCentral_dir = &pState->m_central_dir;
        mz_uint32 * pIndices = &MZ_ZIP_ARRAY_ELEMENT( &pState->m_sorted_central_dir_offsets, mz_uint32, 0 );
        const uint32_t size = pZip->m_total_files;
        const mz_uint32 filename_len = (mz_uint32)strlen( pFilename );

        if ( pIndex )
            *pIndex = 0;

        if ( size )
        {
            mz_int64 l = 0, h = (mz_int64)size - 1;

            while ( l <= h )
            {
                mz_int64 m = l + ( ( h - l ) >> 1 );
                uint32_t file_index = pIndices[(uint32_t)m];

                int comp = mz_zip_filename_compare( pCentral_dir, pCentral_dir_offsets, file_index, pFilename, filename_len );
                if ( !comp )
                {
                    if ( pIndex )
                        *pIndex = file_index;
                    return true;
                }
                else if ( comp < 0 )
                    l = m + 1;
                else
                    h = m - 1;
            }
        }

        return mz_zip_set_error( pZip, MZ_ZIP_FILE_NOT_FOUND );
    }
    int mz_zip_reader_locate_file( mz_zip_archive * pZip, const char * pName, const char * pComment, mz_uint32 flags )
    {
        mz_uint32 index;
        if ( !mz_zip_reader_locate_file_v2( pZip, pName, pComment, flags, &index ) )
            return -1;
        else
            return (int)index;
    }
    mz_bool mz_zip_reader_locate_file_v2( mz_zip_archive * pZip, const char * pName, const char * pComment, mz_uint32 flags, mz_uint32 * pIndex )
    {
        mz_uint32 file_index;
        size_t name_len, comment_len;

        if ( pIndex )
            *pIndex = 0;

        if ( ( !pZip ) || ( !pZip->m_pState ) || ( !pName ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
        if ( ( ( pZip->m_pState->m_init_flags & MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY ) == 0 ) && ( pZip->m_zip_mode == MZ_ZIP_MODE_READING ) &&
             ( ( flags & ( MZ_ZIP_FLAG_IGNORE_PATH | MZ_ZIP_FLAG_CASE_SENSITIVE ) ) == 0 ) && ( !pComment ) && ( pZip->m_pState->m_sorted_central_dir_offsets.m_size ) )
        {
            return mz_zip_locate_file_binary_search( pZip, pName, pIndex );
        }
        name_len = strlen( pName );
        if ( name_len > MZ_UINT16_MAX )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        comment_len = pComment ? strlen( pComment ) : 0;
        if ( comment_len > MZ_UINT16_MAX )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        for ( file_index = 0; file_index < pZip->m_total_files; file_index++ )
        {
            const mz_uint8 * pHeader = &MZ_ZIP_ARRAY_ELEMENT(
                &pZip->m_pState->m_central_dir, mz_uint8, MZ_ZIP_ARRAY_ELEMENT( &pZip->m_pState->m_central_dir_offsets, mz_uint32, file_index ) );
            mz_uint32 filename_len = MZ_READ_LE16( pHeader + MZ_ZIP_CDH_FILENAME_LEN_OFS );
            const char * pFilename = (const char *)pHeader + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
            if ( filename_len < name_len )
                continue;
            if ( comment_len )
            {
                mz_uint32 file_extra_len = MZ_READ_LE16( pHeader + MZ_ZIP_CDH_EXTRA_LEN_OFS ),
                    file_comment_len = MZ_READ_LE16( pHeader + MZ_ZIP_CDH_COMMENT_LEN_OFS );
                const char * pFile_comment = pFilename + filename_len + file_extra_len;
                if ( ( file_comment_len != comment_len ) || ( !mz_zip_string_equal( pComment, pFile_comment, file_comment_len, flags ) ) )
                    continue;
            }
            if ( ( flags & MZ_ZIP_FLAG_IGNORE_PATH ) && ( filename_len ) )
            {
                int ofs = filename_len - 1;
                do
                {
                    if ( ( pFilename[ofs] == '/' ) || ( pFilename[ofs] == '\\' ) || ( pFilename[ofs] == ':' ) )
                        break;
                } while ( --ofs >= 0 );
                ofs++;
                pFilename += ofs;
                filename_len -= ofs;
            }
            if ( ( filename_len == name_len ) && ( mz_zip_string_equal( pName, pFilename, filename_len, flags ) ) )
            {
                if ( pIndex )
                    *pIndex = file_index;
                return true;
            }
        }

        return mz_zip_set_error( pZip, MZ_ZIP_FILE_NOT_FOUND );
    }
    mz_bool mz_zip_reader_extract_to_mem_no_alloc( mz_zip_archive * pZip, mz_uint32 file_index, void * pBuf, size_t buf_size, mz_uint32 flags, void * pUser_read_buf, size_t user_read_buf_size )
    {
        int status = TINFL_STATUS_DONE;
        mz_uint64 needed_size, cur_file_ofs, comp_remaining, out_buf_ofs = 0, read_buf_size, read_buf_ofs = 0, read_buf_avail;
        mz_zip_archive_file_stat file_stat;
        void * pRead_buf;
        mz_uint32 local_header_u32[( MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof( mz_uint32 ) - 1 ) / sizeof( mz_uint32 )];
        mz_uint8 * pLocal_header = (mz_uint8 *)local_header_u32;
        tinfl_decompressor inflator;

        if ( ( !pZip ) || ( !pZip->m_pState ) || ( ( buf_size ) && ( !pBuf ) ) || ( ( user_read_buf_size ) && ( !pUser_read_buf ) ) || ( !pZip->m_pRead ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( !mz_zip_reader_file_stat( pZip, file_index, &file_stat ) )
            return false;
        if ( ( file_stat.m_is_directory ) || ( !file_stat.m_comp_size ) )
            return true;
        if ( file_stat.m_bit_flag & ( MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED | MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION |
             MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_COMPRESSED_PATCH_FLAG ) )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION );
        if ( ( !( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) ) && ( file_stat.m_method != 0 ) && ( file_stat.m_method != MZ_DEFLATED ) )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_METHOD );
        needed_size = ( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) ? file_stat.m_comp_size : file_stat.m_uncomp_size;
        if ( buf_size < needed_size )
            return mz_zip_set_error( pZip, MZ_ZIP_BUF_TOO_SMALL );
        cur_file_ofs = file_stat.m_local_header_ofs;
        if ( pZip->m_pRead( pZip->m_pIO_opaque, cur_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE ) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );

        if ( MZ_READ_LE32( pLocal_header ) != MZ_ZIP_LOCAL_DIR_HEADER_SIG )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

        cur_file_ofs +=
            MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS ) + MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS );
        if ( ( cur_file_ofs + file_stat.m_comp_size ) > pZip->m_archive_size )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

        if ( ( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) || ( !file_stat.m_method ) )
        {

            if ( pZip->m_pRead( pZip->m_pIO_opaque, cur_file_ofs, pBuf, (size_t)needed_size ) != needed_size )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );

            if ( ( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) == 0 )
            {
                if ( mz_crc32( 0, (const mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size ) != file_stat.m_crc32 )
                    return mz_zip_set_error( pZip, MZ_ZIP_CRC_CHECK_FAILED );
            }

            return true;
        }
        tinfl_init( &inflator );

        if ( pZip->m_pState->m_pMem )
        {

            pRead_buf = (mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
            read_buf_size = read_buf_avail = file_stat.m_comp_size;
            comp_remaining = 0;
        }
        else if ( pUser_read_buf )
        {

            if ( !user_read_buf_size )
                return false;
            pRead_buf = (mz_uint8 *)pUser_read_buf;
            read_buf_size = user_read_buf_size;
            read_buf_avail = 0;
            comp_remaining = file_stat.m_comp_size;
        }
        else
        {

            read_buf_size = MZ_MIN( file_stat.m_comp_size, (mz_uint64)MZ_ZIP_MAX_IO_BUF_SIZE );
            if ( ( ( sizeof( size_t ) == sizeof( mz_uint32 ) ) ) && ( read_buf_size > 0x7FFFFFFF ) )
                return mz_zip_set_error( pZip, MZ_ZIP_INTERNAL_ERROR );

            if ( NULL == ( pRead_buf = pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, (size_t)read_buf_size ) ) )
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );

            read_buf_avail = 0;
            comp_remaining = file_stat.m_comp_size;
        }

        do
        {

            size_t in_buf_size, out_buf_size = (size_t)( file_stat.m_uncomp_size - out_buf_ofs );
            if ( ( !read_buf_avail ) && ( !pZip->m_pState->m_pMem ) )
            {
                read_buf_avail = MZ_MIN( read_buf_size, comp_remaining );
                if ( pZip->m_pRead( pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail ) != read_buf_avail )
                {
                    status = TINFL_STATUS_FAILED;
                    mz_zip_set_error( pZip, MZ_ZIP_DECOMPRESSION_FAILED );
                    break;
                }
                cur_file_ofs += read_buf_avail;
                comp_remaining -= read_buf_avail;
                read_buf_ofs = 0;
            }
            in_buf_size = (size_t)read_buf_avail;
            status = tinfl_decompress( &inflator, (mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size, (mz_uint8 *)pBuf, (mz_uint8 *)pBuf + out_buf_ofs,
                                       &out_buf_size, TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF | ( comp_remaining ? TINFL_FLAG_HAS_MORE_INPUT : 0 ) );
            read_buf_avail -= in_buf_size;
            read_buf_ofs += in_buf_size;
            out_buf_ofs += out_buf_size;
        } while ( status == TINFL_STATUS_NEEDS_MORE_INPUT );

        if ( status == TINFL_STATUS_DONE )
        {

            if ( out_buf_ofs != file_stat.m_uncomp_size )
            {
                mz_zip_set_error( pZip, MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE );
                status = TINFL_STATUS_FAILED;
            }
            else if ( mz_crc32( 0, (const mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size ) != file_stat.m_crc32 )
            {
                mz_zip_set_error( pZip, MZ_ZIP_CRC_CHECK_FAILED );
                status = TINFL_STATUS_FAILED;
            }
        }

        if ( ( !pZip->m_pState->m_pMem ) && ( !pUser_read_buf ) )
            pZip->m_pFree( pZip->m_pAlloc_opaque, pRead_buf );

        return status == TINFL_STATUS_DONE;
    }
    mz_bool mz_zip_reader_extract_file_to_mem_no_alloc( mz_zip_archive * pZip, const char * pFilename, void * pBuf, size_t buf_size, mz_uint32 flags, void * pUser_read_buf, size_t user_read_buf_size )
    {
        mz_uint32 file_index;
        if ( !mz_zip_reader_locate_file_v2( pZip, pFilename, NULL, flags, &file_index ) )
            return false;
        return mz_zip_reader_extract_to_mem_no_alloc( pZip, file_index, pBuf, buf_size, flags, pUser_read_buf, user_read_buf_size );
    }
    mz_bool mz_zip_reader_extract_to_mem( mz_zip_archive * pZip, mz_uint32 file_index, void * pBuf, size_t buf_size, mz_uint32 flags )
    {
        return mz_zip_reader_extract_to_mem_no_alloc( pZip, file_index, pBuf, buf_size, flags, NULL, 0 );
    }
    mz_bool mz_zip_reader_extract_file_to_mem( mz_zip_archive * pZip, const char * pFilename, void * pBuf, size_t buf_size, mz_uint32 flags )
    {
        return mz_zip_reader_extract_file_to_mem_no_alloc( pZip, pFilename, pBuf, buf_size, flags, NULL, 0 );
    }
    void * mz_zip_reader_extract_to_heap( mz_zip_archive * pZip, mz_uint32 file_index, size_t * pSize, mz_uint32 flags )
    {
        mz_uint64 comp_size, uncomp_size, alloc_size;
        const mz_uint8 * p = mz_zip_get_cdh( pZip, file_index );
        void * pBuf;

        if ( pSize )
            *pSize = 0;

        if ( !p )
        {
            mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
            return NULL;
        }

        comp_size = MZ_READ_LE32( p + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS );
        uncomp_size = MZ_READ_LE32( p + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS );

        alloc_size = ( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) ? comp_size : uncomp_size;
        if ( ( ( sizeof( size_t ) == sizeof( mz_uint32 ) ) ) && ( alloc_size > 0x7FFFFFFF ) )
        {
            mz_zip_set_error( pZip, MZ_ZIP_INTERNAL_ERROR );
            return NULL;
        }

        if ( NULL == ( pBuf = pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, (size_t)alloc_size ) ) )
        {
            mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            return NULL;
        }

        if ( !mz_zip_reader_extract_to_mem( pZip, file_index, pBuf, (size_t)alloc_size, flags ) )
        {
            pZip->m_pFree( pZip->m_pAlloc_opaque, pBuf );
            return NULL;
        }

        if ( pSize )
            *pSize = (size_t)alloc_size;
        return pBuf;
    }
    void * mz_zip_reader_extract_file_to_heap( mz_zip_archive * pZip, const char * pFilename, size_t * pSize, mz_uint32 flags )
    {
        mz_uint32 file_index;
        if ( !mz_zip_reader_locate_file_v2( pZip, pFilename, NULL, flags, &file_index ) )
        {
            if ( pSize )
                *pSize = 0;
            return nullptr;
        }
        return mz_zip_reader_extract_to_heap( pZip, file_index, pSize, flags );
    }
    mz_bool mz_zip_reader_extract_to_callback( mz_zip_archive * pZip, mz_uint32 file_index, mz_file_write_func pCallback, void * pOpaque, mz_uint32 flags )
    {
        int status = TINFL_STATUS_DONE;
        mz_uint32 file_crc32 = 0;
        mz_uint64 read_buf_size, read_buf_ofs = 0, read_buf_avail, comp_remaining, out_buf_ofs = 0, cur_file_ofs;
        mz_zip_archive_file_stat file_stat;
        void * pRead_buf = NULL;
        void * pWrite_buf = NULL;
        mz_uint32 local_header_u32[( MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof( mz_uint32 ) - 1 ) / sizeof( mz_uint32 )];
        mz_uint8 * pLocal_header = (mz_uint8 *)local_header_u32;

        if ( ( !pZip ) || ( !pZip->m_pState ) || ( !pCallback ) || ( !pZip->m_pRead ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( !mz_zip_reader_file_stat( pZip, file_index, &file_stat ) )
            return false;
        if ( ( file_stat.m_is_directory ) || ( !file_stat.m_comp_size ) )
            return true;
        if ( file_stat.m_bit_flag & ( MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED | MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION |
             MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_COMPRESSED_PATCH_FLAG ) )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION );
        if ( ( !( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) ) && ( file_stat.m_method != 0 ) && ( file_stat.m_method != MZ_DEFLATED ) )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_METHOD );
        cur_file_ofs = file_stat.m_local_header_ofs;
        if ( pZip->m_pRead( pZip->m_pIO_opaque, cur_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE ) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );

        if ( MZ_READ_LE32( pLocal_header ) != MZ_ZIP_LOCAL_DIR_HEADER_SIG )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

        cur_file_ofs +=
            MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS ) + MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS );
        if ( ( cur_file_ofs + file_stat.m_comp_size ) > pZip->m_archive_size )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
        if ( pZip->m_pState->m_pMem )
        {
            pRead_buf = (mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
            read_buf_size = read_buf_avail = file_stat.m_comp_size;
            comp_remaining = 0;
        }
        else
        {
            read_buf_size = MZ_MIN( file_stat.m_comp_size, (mz_uint64)MZ_ZIP_MAX_IO_BUF_SIZE );
            if ( NULL == ( pRead_buf = pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, (size_t)read_buf_size ) ) )
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );

            read_buf_avail = 0;
            comp_remaining = file_stat.m_comp_size;
        }

        if ( ( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) || ( !file_stat.m_method ) )
        {

            if ( pZip->m_pState->m_pMem )
            {
                if ( ( ( sizeof( size_t ) == sizeof( mz_uint32 ) ) ) && ( file_stat.m_comp_size > MZ_UINT32_MAX ) )
                    return mz_zip_set_error( pZip, MZ_ZIP_INTERNAL_ERROR );

                if ( pCallback( pOpaque, out_buf_ofs, pRead_buf, (size_t)file_stat.m_comp_size ) != file_stat.m_comp_size )
                {
                    mz_zip_set_error( pZip, MZ_ZIP_WRITE_CALLBACK_FAILED );
                    status = TINFL_STATUS_FAILED;
                }
                else if ( !( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) )
                {
                    file_crc32 = (mz_uint32)mz_crc32( file_crc32, (const mz_uint8 *)pRead_buf, (size_t)file_stat.m_comp_size );
                }

                cur_file_ofs += file_stat.m_comp_size;
                out_buf_ofs += file_stat.m_comp_size;
                comp_remaining = 0;
            }
            else
            {
                while ( comp_remaining )
                {
                    read_buf_avail = MZ_MIN( read_buf_size, comp_remaining );
                    if ( pZip->m_pRead( pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail ) != read_buf_avail )
                    {
                        mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
                        status = TINFL_STATUS_FAILED;
                        break;
                    }
                    if ( !( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) )
                    {
                        file_crc32 = (mz_uint32)mz_crc32( file_crc32, (const mz_uint8 *)pRead_buf, (size_t)read_buf_avail );
                    }
                    if ( pCallback( pOpaque, out_buf_ofs, pRead_buf, (size_t)read_buf_avail ) != read_buf_avail )
                    {
                        mz_zip_set_error( pZip, MZ_ZIP_WRITE_CALLBACK_FAILED );
                        status = TINFL_STATUS_FAILED;
                        break;
                    }

                    cur_file_ofs += read_buf_avail;
                    out_buf_ofs += read_buf_avail;
                    comp_remaining -= read_buf_avail;
                }
            }
        }
        else
        {
            tinfl_decompressor inflator;
            tinfl_init( &inflator );

            if ( NULL == ( pWrite_buf = pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, TINFL_LZ_DICT_SIZE ) ) )
            {
                mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
                status = TINFL_STATUS_FAILED;
            }
            else
            {
                do
                {
                    mz_uint8 * pWrite_buf_cur = (mz_uint8 *)pWrite_buf + ( out_buf_ofs & ( TINFL_LZ_DICT_SIZE - 1 ) );
                    size_t in_buf_size, out_buf_size = TINFL_LZ_DICT_SIZE - ( out_buf_ofs & ( TINFL_LZ_DICT_SIZE - 1 ) );
                    if ( ( !read_buf_avail ) && ( !pZip->m_pState->m_pMem ) )
                    {
                        read_buf_avail = MZ_MIN( read_buf_size, comp_remaining );
                        if ( pZip->m_pRead( pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail ) != read_buf_avail )
                        {
                            mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
                            status = TINFL_STATUS_FAILED;
                            break;
                        }
                        cur_file_ofs += read_buf_avail;
                        comp_remaining -= read_buf_avail;
                        read_buf_ofs = 0;
                    }

                    in_buf_size = (size_t)read_buf_avail;
                    status = tinfl_decompress( &inflator, (const mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size, (mz_uint8 *)pWrite_buf, pWrite_buf_cur,
                                               &out_buf_size, comp_remaining ? TINFL_FLAG_HAS_MORE_INPUT : 0 );
                    read_buf_avail -= in_buf_size;
                    read_buf_ofs += in_buf_size;

                    if ( out_buf_size )
                    {
                        if ( pCallback( pOpaque, out_buf_ofs, pWrite_buf_cur, out_buf_size ) != out_buf_size )
                        {
                            mz_zip_set_error( pZip, MZ_ZIP_WRITE_CALLBACK_FAILED );
                            status = TINFL_STATUS_FAILED;
                            break;
                        }

                        file_crc32 = (mz_uint32)mz_crc32( file_crc32, pWrite_buf_cur, out_buf_size );
                        if ( ( out_buf_ofs += out_buf_size ) > file_stat.m_uncomp_size )
                        {
                            mz_zip_set_error( pZip, MZ_ZIP_DECOMPRESSION_FAILED );
                            status = TINFL_STATUS_FAILED;
                            break;
                        }
                    }
                } while ( ( status == TINFL_STATUS_NEEDS_MORE_INPUT ) || ( status == TINFL_STATUS_HAS_MORE_OUTPUT ) );
            }
        }

        if ( ( status == TINFL_STATUS_DONE ) && ( !( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) ) )
        {

            if ( out_buf_ofs != file_stat.m_uncomp_size )
            {
                mz_zip_set_error( pZip, MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE );
                status = TINFL_STATUS_FAILED;
            }
            else if ( file_crc32 != file_stat.m_crc32 )
            {
                mz_zip_set_error( pZip, MZ_ZIP_DECOMPRESSION_FAILED );
                status = TINFL_STATUS_FAILED;
            }
        }

        if ( !pZip->m_pState->m_pMem )
            pZip->m_pFree( pZip->m_pAlloc_opaque, pRead_buf );

        if ( pWrite_buf )
            pZip->m_pFree( pZip->m_pAlloc_opaque, pWrite_buf );

        return status == TINFL_STATUS_DONE;
    }
    mz_bool mz_zip_reader_extract_file_to_callback( mz_zip_archive * pZip, const char * pFilename, mz_file_write_func pCallback, void * pOpaque, mz_uint32 flags )
    {
        mz_uint32 file_index;
        if ( !mz_zip_reader_locate_file_v2( pZip, pFilename, NULL, flags, &file_index ) )
            return false;

        return mz_zip_reader_extract_to_callback( pZip, file_index, pCallback, pOpaque, flags );
    }
    mz_zip_reader_extract_iter_state * mz_zip_reader_extract_iter_new( mz_zip_archive * pZip, mz_uint32 file_index, mz_uint32 flags )
    {
        mz_zip_reader_extract_iter_state * pState;
        mz_uint32 local_header_u32[( MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof( mz_uint32 ) - 1 ) / sizeof( mz_uint32 )];
        mz_uint8 * pLocal_header = (mz_uint8 *)local_header_u32;
        if ( ( !pZip ) || ( !pZip->m_pState ) )
            return NULL;
        pState = (mz_zip_reader_extract_iter_state *)pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, sizeof( mz_zip_reader_extract_iter_state ) );
        if ( !pState )
        {
            mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            return NULL;
        }
        if ( !mz_zip_reader_file_stat( pZip, file_index, &pState->file_stat ) )
        {
            pZip->m_pFree( pZip->m_pAlloc_opaque, pState );
            return NULL;
        }
        if ( pState->file_stat.m_bit_flag & ( MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED | MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION |
             MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_COMPRESSED_PATCH_FLAG ) )
        {
            mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION );
            pZip->m_pFree( pZip->m_pAlloc_opaque, pState );
            return NULL;
        }
        if ( ( !( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) ) && ( pState->file_stat.m_method != 0 ) && ( pState->file_stat.m_method != MZ_DEFLATED ) )
        {
            mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_METHOD );
            pZip->m_pFree( pZip->m_pAlloc_opaque, pState );
            return NULL;
        }
        pState->pZip = pZip;
        pState->flags = flags;
        pState->status = TINFL_STATUS_DONE;
        pState->file_crc32 = 0;
        pState->read_buf_ofs = 0;
        pState->out_buf_ofs = 0;
        pState->pRead_buf = NULL;
        pState->pWrite_buf = NULL;
        pState->out_blk_remain = 0;
        pState->cur_file_ofs = pState->file_stat.m_local_header_ofs;
        if ( pZip->m_pRead( pZip->m_pIO_opaque, pState->cur_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE ) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE )
        {
            mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
            pZip->m_pFree( pZip->m_pAlloc_opaque, pState );
            return NULL;
        }

        if ( MZ_READ_LE32( pLocal_header ) != MZ_ZIP_LOCAL_DIR_HEADER_SIG )
        {
            mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
            pZip->m_pFree( pZip->m_pAlloc_opaque, pState );
            return NULL;
        }

        pState->cur_file_ofs +=
            MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS ) + MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS );
        if ( ( pState->cur_file_ofs + pState->file_stat.m_comp_size ) > pZip->m_archive_size )
        {
            mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
            pZip->m_pFree( pZip->m_pAlloc_opaque, pState );
            return NULL;
        }
        if ( pZip->m_pState->m_pMem )
        {
            pState->pRead_buf = (mz_uint8 *)pZip->m_pState->m_pMem + pState->cur_file_ofs;
            pState->read_buf_size = pState->read_buf_avail = pState->file_stat.m_comp_size;
            pState->comp_remaining = pState->file_stat.m_comp_size;
        }
        else
        {
            if ( !( ( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) || ( !pState->file_stat.m_method ) ) )
            {

                pState->read_buf_size = MZ_MIN( pState->file_stat.m_comp_size, (mz_uint64)MZ_ZIP_MAX_IO_BUF_SIZE );
                if ( NULL == ( pState->pRead_buf = pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, (size_t)pState->read_buf_size ) ) )
                {
                    mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
                    pZip->m_pFree( pZip->m_pAlloc_opaque, pState );
                    return NULL;
                }
            }
            else
            {

                pState->read_buf_size = 0;
            }
            pState->read_buf_avail = 0;
            pState->comp_remaining = pState->file_stat.m_comp_size;
        }

        if ( !( ( flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) || ( !pState->file_stat.m_method ) ) )
        {

            tinfl_init( &pState->inflator );
            if ( NULL == ( pState->pWrite_buf = pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, TINFL_LZ_DICT_SIZE ) ) )
            {
                mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
                if ( pState->pRead_buf )
                    pZip->m_pFree( pZip->m_pAlloc_opaque, pState->pRead_buf );
                pZip->m_pFree( pZip->m_pAlloc_opaque, pState );
                return NULL;
            }
        }

        return pState;
    }
    mz_zip_reader_extract_iter_state * mz_zip_reader_extract_file_iter_new( mz_zip_archive * pZip, const char * pFilename, mz_uint32 flags )
    {
        mz_uint32 file_index;
        if ( !mz_zip_reader_locate_file_v2( pZip, pFilename, NULL, flags, &file_index ) )
            return NULL;
        return mz_zip_reader_extract_iter_new( pZip, file_index, flags );
    }
    size_t mz_zip_reader_extract_iter_read( mz_zip_reader_extract_iter_state * pState, void * pvBuf, size_t buf_size )
    {
        size_t copied_to_caller = 0;
        if ( ( !pState ) || ( !pState->pZip ) || ( !pState->pZip->m_pState ) || ( !pvBuf ) )
            return 0;

        if ( ( pState->flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) || ( !pState->file_stat.m_method ) )
        {

            copied_to_caller = (size_t)MZ_MIN( buf_size, pState->comp_remaining );
            if ( pState->pZip->m_pState->m_pMem )
            {

                memcpy( pvBuf, pState->pRead_buf, copied_to_caller );
                pState->pRead_buf = ( (mz_uint8 *)pState->pRead_buf ) + copied_to_caller;
            }
            else
            {

                if ( pState->pZip->m_pRead( pState->pZip->m_pIO_opaque, pState->cur_file_ofs, pvBuf, copied_to_caller ) != copied_to_caller )
                {

                    mz_zip_set_error( pState->pZip, MZ_ZIP_FILE_READ_FAILED );
                    pState->status = TINFL_STATUS_FAILED;
                    copied_to_caller = 0;
                }
            }

            if ( !( pState->flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) )
                pState->file_crc32 = (mz_uint32)mz_crc32( pState->file_crc32, (const mz_uint8 *)pvBuf, copied_to_caller );

            pState->cur_file_ofs += copied_to_caller;
            pState->out_buf_ofs += copied_to_caller;
            pState->comp_remaining -= copied_to_caller;
        }
        else
        {
            do
            {

                mz_uint8 * pWrite_buf_cur = (mz_uint8 *)pState->pWrite_buf + ( pState->out_buf_ofs & ( TINFL_LZ_DICT_SIZE - 1 ) );
                size_t in_buf_size, out_buf_size = TINFL_LZ_DICT_SIZE - ( pState->out_buf_ofs & ( TINFL_LZ_DICT_SIZE - 1 ) );

                if ( !pState->out_blk_remain )
                {

                    if ( ( !pState->read_buf_avail ) && ( !pState->pZip->m_pState->m_pMem ) )
                    {

                        pState->read_buf_avail = MZ_MIN( pState->read_buf_size, pState->comp_remaining );
                        if ( pState->pZip->m_pRead( pState->pZip->m_pIO_opaque, pState->cur_file_ofs, pState->pRead_buf, (size_t)pState->read_buf_avail ) !=
                             pState->read_buf_avail )
                        {
                            mz_zip_set_error( pState->pZip, MZ_ZIP_FILE_READ_FAILED );
                            pState->status = TINFL_STATUS_FAILED;
                            break;
                        }
                        pState->cur_file_ofs += pState->read_buf_avail;
                        pState->comp_remaining -= pState->read_buf_avail;
                        pState->read_buf_ofs = 0;
                    }
                    in_buf_size = (size_t)pState->read_buf_avail;
                    pState->status = tinfl_decompress( &pState->inflator, (const mz_uint8 *)pState->pRead_buf + pState->read_buf_ofs, &in_buf_size,
                                                       (mz_uint8 *)pState->pWrite_buf, pWrite_buf_cur, &out_buf_size, pState->comp_remaining ? TINFL_FLAG_HAS_MORE_INPUT : 0 );
                    pState->read_buf_avail -= in_buf_size;
                    pState->read_buf_ofs += in_buf_size;
                    pState->out_blk_remain = out_buf_size;
                }

                if ( pState->out_blk_remain )
                {

                    size_t to_copy = MZ_MIN( ( buf_size - copied_to_caller ), pState->out_blk_remain );
                    memcpy( (uint8_t *)pvBuf + copied_to_caller, pWrite_buf_cur, to_copy );

                    pState->file_crc32 = (mz_uint32)mz_crc32( pState->file_crc32, pWrite_buf_cur, to_copy );
                    pState->out_blk_remain -= to_copy;
                    if ( ( pState->out_buf_ofs += to_copy ) > pState->file_stat.m_uncomp_size )
                    {
                        mz_zip_set_error( pState->pZip, MZ_ZIP_DECOMPRESSION_FAILED );
                        pState->status = TINFL_STATUS_FAILED;
                        break;
                    }
                    copied_to_caller += to_copy;
                }
            } while ( ( copied_to_caller < buf_size ) && ( ( pState->status == TINFL_STATUS_NEEDS_MORE_INPUT ) || ( pState->status == TINFL_STATUS_HAS_MORE_OUTPUT ) ) );
        }
        return copied_to_caller;
    }
    mz_bool mz_zip_reader_extract_iter_free( mz_zip_reader_extract_iter_state * pState )
    {
        int status;
        if ( ( !pState ) || ( !pState->pZip ) || ( !pState->pZip->m_pState ) )
            return false;
        if ( ( pState->status == TINFL_STATUS_DONE ) && ( !( pState->flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) ) )
        {

            if ( pState->out_buf_ofs != pState->file_stat.m_uncomp_size )
            {
                mz_zip_set_error( pState->pZip, MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE );
                pState->status = TINFL_STATUS_FAILED;
            }
            else if ( pState->file_crc32 != pState->file_stat.m_crc32 )
            {
                mz_zip_set_error( pState->pZip, MZ_ZIP_DECOMPRESSION_FAILED );
                pState->status = TINFL_STATUS_FAILED;
            }
        }
        if ( !pState->pZip->m_pState->m_pMem )
            pState->pZip->m_pFree( pState->pZip->m_pAlloc_opaque, pState->pRead_buf );
        if ( pState->pWrite_buf )
            pState->pZip->m_pFree( pState->pZip->m_pAlloc_opaque, pState->pWrite_buf );
        status = pState->status;
        pState->pZip->m_pFree( pState->pZip->m_pAlloc_opaque, pState );

        return status == TINFL_STATUS_DONE;
    }
    static size_t mz_zip_file_write_callback( void * pOpaque, mz_uint64 ofs, const void * pBuf, size_t n )
    {
        (void)ofs;

        return MZ_FWRITE( pBuf, 1, n, (FILE *)pOpaque );
    }
    mz_bool mz_zip_reader_extract_to_file( mz_zip_archive * pZip, mz_uint32 file_index, const char * pDst_filename, mz_uint32 flags )
    {
        mz_bool status;
        mz_zip_archive_file_stat file_stat;
        FILE * pFile;

        if ( !mz_zip_reader_file_stat( pZip, file_index, &file_stat ) )
            return false;

        if ( ( file_stat.m_is_directory ) || ( !file_stat.m_is_supported ) )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_FEATURE );

        pFile = MZ_FOPEN( pDst_filename, "wb" );
        if ( !pFile )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_OPEN_FAILED );

        status = mz_zip_reader_extract_to_callback( pZip, file_index, mz_zip_file_write_callback, pFile, flags );

        if ( MZ_FCLOSE( pFile ) == EOF )
        {
            if ( status )
                mz_zip_set_error( pZip, MZ_ZIP_FILE_CLOSE_FAILED );

            status = false;
        }

        if ( status )
            mz_zip_set_file_times( pDst_filename, file_stat.m_time, file_stat.m_time );

        return status;
    }
    mz_bool mz_zip_reader_extract_file_to_file( mz_zip_archive * pZip, const char * pArchive_filename, const char * pDst_filename, mz_uint32 flags )
    {
        mz_uint32 file_index;
        if ( !mz_zip_reader_locate_file_v2( pZip, pArchive_filename, NULL, flags, &file_index ) )
            return false;

        return mz_zip_reader_extract_to_file( pZip, file_index, pDst_filename, flags );
    }
    mz_bool mz_zip_reader_extract_to_cfile( mz_zip_archive * pZip, mz_uint32 file_index, FILE * pFile, mz_uint32 flags )
    {
        mz_zip_archive_file_stat file_stat;

        if ( !mz_zip_reader_file_stat( pZip, file_index, &file_stat ) )
            return false;

        if ( ( file_stat.m_is_directory ) || ( !file_stat.m_is_supported ) )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_FEATURE );

        return mz_zip_reader_extract_to_callback( pZip, file_index, mz_zip_file_write_callback, pFile, flags );
    }
    mz_bool mz_zip_reader_extract_file_to_cfile( mz_zip_archive * pZip, const char * pArchive_filename, FILE * pFile, mz_uint32 flags )
    {
        mz_uint32 file_index;
        if ( !mz_zip_reader_locate_file_v2( pZip, pArchive_filename, NULL, flags, &file_index ) )
            return false;

        return mz_zip_reader_extract_to_cfile( pZip, file_index, pFile, flags );
    }
    static size_t mz_zip_compute_crc32_callback( void * pOpaque, mz_uint64 file_ofs, const void * pBuf, size_t n )
    {
        mz_uint32 * p = (mz_uint32 *)pOpaque;
        (void)file_ofs;
        *p = (mz_uint32)mz_crc32( *p, (const mz_uint8 *)pBuf, n );
        return n;
    }
    mz_bool mz_zip_validate_file( mz_zip_archive * pZip, mz_uint32 file_index, mz_uint32 flags )
    {
        mz_zip_archive_file_stat file_stat;
        mz_zip_internal_state * pState;
        const mz_uint8 * pCentral_dir_header;
        mz_bool found_zip64_ext_data_in_cdir = false;
        mz_bool found_zip64_ext_data_in_ldir = false;
        mz_uint32 local_header_u32[( MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof( mz_uint32 ) - 1 ) / sizeof( mz_uint32 )];
        mz_uint8 * pLocal_header = (mz_uint8 *)local_header_u32;
        mz_uint64 local_header_ofs = 0;
        mz_uint32 local_header_filename_len, local_header_extra_len, local_header_crc32;
        mz_uint64 local_header_comp_size, local_header_uncomp_size;
        mz_uint32 uncomp_crc32 = 0;
        mz_bool has_data_descriptor;
        mz_uint32 local_header_bit_flags;

        mz_zip_array file_data_array;
        mz_zip_array_init( &file_data_array, 1 );

        if ( ( !pZip ) || ( !pZip->m_pState ) || ( !pZip->m_pAlloc ) || ( !pZip->m_pFree ) || ( !pZip->m_pRead ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( file_index > pZip->m_total_files )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        pState = pZip->m_pState;

        pCentral_dir_header = mz_zip_get_cdh( pZip, file_index );

        if ( !mz_zip_file_stat_internal( pZip, file_index, pCentral_dir_header, &file_stat, &found_zip64_ext_data_in_cdir ) )
            return false;
        if ( ( file_stat.m_is_directory ) || ( !file_stat.m_uncomp_size ) )
            return true;
        if ( file_stat.m_is_encrypted )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION );
        if ( ( file_stat.m_method != 0 ) && ( file_stat.m_method != MZ_DEFLATED ) )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_METHOD );

        if ( !file_stat.m_is_supported )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_FEATURE );
        local_header_ofs = file_stat.m_local_header_ofs;
        if ( pZip->m_pRead( pZip->m_pIO_opaque, local_header_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE ) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );

        if ( MZ_READ_LE32( pLocal_header ) != MZ_ZIP_LOCAL_DIR_HEADER_SIG )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

        local_header_filename_len = MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS );
        local_header_extra_len = MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS );
        local_header_comp_size = MZ_READ_LE32( pLocal_header + MZ_ZIP_LDH_COMPRESSED_SIZE_OFS );
        local_header_uncomp_size = MZ_READ_LE32( pLocal_header + MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS );
        local_header_crc32 = MZ_READ_LE32( pLocal_header + MZ_ZIP_LDH_CRC32_OFS );
        local_header_bit_flags = MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_BIT_FLAG_OFS );
        has_data_descriptor = ( local_header_bit_flags & 8 ) != 0;

        if ( local_header_filename_len != strlen( file_stat.m_filename ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

        if ( ( local_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + local_header_filename_len + local_header_extra_len + file_stat.m_comp_size ) >
             pZip->m_archive_size )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

        if ( !mz_zip_array_resize( pZip, &file_data_array, MZ_MAX( local_header_filename_len, local_header_extra_len ), false ) )
        {
            mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            goto handle_failure;
        }

        if ( local_header_filename_len )
        {
            if ( pZip->m_pRead( pZip->m_pIO_opaque, local_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE, file_data_array.m_p, local_header_filename_len ) !=
                 local_header_filename_len )
            {
                mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
                goto handle_failure;
            }

            if ( memcmp( file_stat.m_filename, file_data_array.m_p, local_header_filename_len ) != 0 )
            {
                mz_zip_set_error( pZip, MZ_ZIP_VALIDATION_FAILED );
                goto handle_failure;
            }
        }

        if ( ( local_header_extra_len ) && ( ( local_header_comp_size == MZ_UINT32_MAX ) || ( local_header_uncomp_size == MZ_UINT32_MAX ) ) )
        {
            mz_uint32 extra_size_remaining = local_header_extra_len;
            const mz_uint8 * pExtra_data = (const mz_uint8 *)file_data_array.m_p;

            if ( pZip->m_pRead( pZip->m_pIO_opaque, local_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + local_header_filename_len, file_data_array.m_p,
                 local_header_extra_len ) != local_header_extra_len )
            {
                mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
                goto handle_failure;
            }

            do
            {
                mz_uint32 field_id, field_data_size, field_total_size;

                if ( extra_size_remaining < ( sizeof( mz_uint16 ) * 2 ) )
                {
                    mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
                    goto handle_failure;
                }

                field_id = MZ_READ_LE16( pExtra_data );
                field_data_size = MZ_READ_LE16( pExtra_data + sizeof( mz_uint16 ) );
                field_total_size = field_data_size + sizeof( mz_uint16 ) * 2;

                if ( field_total_size > extra_size_remaining )
                {
                    mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
                    goto handle_failure;
                }

                if ( field_id == MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID )
                {
                    const mz_uint8 * pSrc_field_data = pExtra_data + sizeof( mz_uint32 );

                    if ( field_data_size < sizeof( mz_uint64 ) * 2 )
                    {
                        mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
                        goto handle_failure;
                    }

                    local_header_uncomp_size = MZ_READ_LE64( pSrc_field_data );
                    local_header_comp_size = MZ_READ_LE64( pSrc_field_data + sizeof( mz_uint64 ) );

                    found_zip64_ext_data_in_ldir = true;
                    break;
                }

                pExtra_data += field_total_size;
                extra_size_remaining -= field_total_size;
            } while ( extra_size_remaining );
        }

        if ( ( has_data_descriptor ) && ( !local_header_comp_size ) && ( !local_header_crc32 ) )
        {
            mz_uint8 descriptor_buf[32];
            mz_bool has_id;
            const mz_uint8 * pSrc;
            mz_uint32 file_crc32;
            mz_uint64 comp_size = 0, uncomp_size = 0;

            mz_uint32 num_descriptor_uint32s = ( ( pState->m_zip64 ) || ( found_zip64_ext_data_in_ldir ) ) ? 6 : 4;

            if ( pZip->m_pRead( pZip->m_pIO_opaque,
                 local_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + local_header_filename_len + local_header_extra_len + file_stat.m_comp_size,
                 descriptor_buf, sizeof( mz_uint32 ) * num_descriptor_uint32s ) != ( sizeof( mz_uint32 ) * num_descriptor_uint32s ) )
            {
                mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
                goto handle_failure;
            }

            has_id = ( MZ_READ_LE32( descriptor_buf ) == MZ_ZIP_DATA_DESCRIPTOR_ID );
            pSrc = has_id ? ( descriptor_buf + sizeof( mz_uint32 ) ) : descriptor_buf;

            file_crc32 = MZ_READ_LE32( pSrc );

            if ( ( pState->m_zip64 ) || ( found_zip64_ext_data_in_ldir ) )
            {
                comp_size = MZ_READ_LE64( pSrc + sizeof( mz_uint32 ) );
                uncomp_size = MZ_READ_LE64( pSrc + sizeof( mz_uint32 ) + sizeof( mz_uint64 ) );
            }
            else
            {
                comp_size = MZ_READ_LE32( pSrc + sizeof( mz_uint32 ) );
                uncomp_size = MZ_READ_LE32( pSrc + sizeof( mz_uint32 ) + sizeof( mz_uint32 ) );
            }

            if ( ( file_crc32 != file_stat.m_crc32 ) || ( comp_size != file_stat.m_comp_size ) || ( uncomp_size != file_stat.m_uncomp_size ) )
            {
                mz_zip_set_error( pZip, MZ_ZIP_VALIDATION_FAILED );
                goto handle_failure;
            }
        }
        else
        {
            if ( ( local_header_crc32 != file_stat.m_crc32 ) || ( local_header_comp_size != file_stat.m_comp_size ) ||
                 ( local_header_uncomp_size != file_stat.m_uncomp_size ) )
            {
                mz_zip_set_error( pZip, MZ_ZIP_VALIDATION_FAILED );
                goto handle_failure;
            }
        }

        mz_zip_array_clear( pZip, &file_data_array );

        if ( ( flags & MZ_ZIP_FLAG_VALIDATE_HEADERS_ONLY ) == 0 )
        {
            if ( !mz_zip_reader_extract_to_callback( pZip, file_index, mz_zip_compute_crc32_callback, &uncomp_crc32, 0 ) )
                return false;
            if ( uncomp_crc32 != file_stat.m_crc32 )
            {
                mz_zip_set_error( pZip, MZ_ZIP_VALIDATION_FAILED );
                return false;
            }
        }

        return true;

handle_failure:
        mz_zip_array_clear( pZip, &file_data_array );
        return false;
    }
    mz_bool mz_zip_validate_archive( mz_zip_archive * pZip, mz_uint32 flags )
    {
        mz_zip_internal_state * pState;
        uint32_t i;

        if ( ( !pZip ) || ( !pZip->m_pState ) || ( !pZip->m_pAlloc ) || ( !pZip->m_pFree ) || ( !pZip->m_pRead ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        pState = pZip->m_pState;
        if ( !pState->m_zip64 )
        {
            if ( pZip->m_total_files > MZ_UINT16_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );

            if ( pZip->m_archive_size > MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );
        }
        else
        {
            if ( pZip->m_total_files >= MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );

            if ( pState->m_central_dir.m_size >= MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );
        }

        for ( i = 0; i < pZip->m_total_files; i++ )
        {
            if ( MZ_ZIP_FLAG_VALIDATE_LOCATE_FILE_FLAG & flags )
            {
                mz_uint32 found_index;
                mz_zip_archive_file_stat stat;

                if ( !mz_zip_reader_file_stat( pZip, i, &stat ) )
                    return false;

                if ( !mz_zip_reader_locate_file_v2( pZip, stat.m_filename, NULL, 0, &found_index ) )
                    return false;
                if ( found_index != i )
                    return mz_zip_set_error( pZip, MZ_ZIP_VALIDATION_FAILED );
            }

            if ( !mz_zip_validate_file( pZip, i, flags ) )
                return false;
        }

        return true;
    }
    mz_bool mz_zip_validate_mem_archive( const void * pMem, size_t size, mz_uint32 flags, mz_zip_error * pErr )
    {
        mz_bool success = true;
        mz_zip_archive zip;
        mz_zip_error actual_err = MZ_ZIP_NO_ERROR;

        if ( ( !pMem ) || ( !size ) )
        {
            if ( pErr )
                *pErr = MZ_ZIP_INVALID_PARAMETER;
            return false;
        }

        mz_zip_zero_struct( &zip );

        if ( !mz_zip_reader_init_mem( &zip, pMem, size, flags ) )
        {
            if ( pErr )
                *pErr = zip.m_last_error;
            return false;
        }

        if ( !mz_zip_validate_archive( &zip, flags ) )
        {
            actual_err = zip.m_last_error;
            success = false;
        }

        if ( !mz_zip_reader_end_internal( &zip, success ) )
        {
            if ( !actual_err )
                actual_err = zip.m_last_error;
            success = false;
        }

        if ( pErr )
            *pErr = actual_err;

        return success;
    }
    mz_bool mz_zip_validate_file_archive( const char * pFilename, mz_uint32 flags, mz_zip_error * pErr )
    {
        mz_bool success = true;
        mz_zip_archive zip;
        mz_zip_error actual_err = MZ_ZIP_NO_ERROR;

        if ( !pFilename )
        {
            if ( pErr )
                *pErr = MZ_ZIP_INVALID_PARAMETER;
            return false;
        }

        mz_zip_zero_struct( &zip );

        if ( !mz_zip_reader_init_file_v2( &zip, pFilename, flags, 0, 0 ) )
        {
            if ( pErr )
                *pErr = zip.m_last_error;
            return false;
        }

        if ( !mz_zip_validate_archive( &zip, flags ) )
        {
            actual_err = zip.m_last_error;
            success = false;
        }

        if ( !mz_zip_reader_end_internal( &zip, success ) )
        {
            if ( !actual_err )
                actual_err = zip.m_last_error;
            success = false;
        }

        if ( pErr )
            *pErr = actual_err;

        return success;
    }
    static void mz_write_le16( mz_uint8 * p, mz_uint16 v )
    {
        p[0] = (mz_uint8)v;
        p[1] = (mz_uint8)( v >> 8 );
    }
    static void mz_write_le32( mz_uint8 * p, mz_uint32 v )
    {
        p[0] = (mz_uint8)v;
        p[1] = (mz_uint8)( v >> 8 );
        p[2] = (mz_uint8)( v >> 16 );
        p[3] = (mz_uint8)( v >> 24 );
    }
    static void mz_write_le64( mz_uint8 * p, mz_uint64 v )
    {
        mz_write_le32( p, (mz_uint32)v );
        mz_write_le32( p + sizeof( mz_uint32 ), (mz_uint32)( v >> 32 ) );
    }
    static size_t mz_zip_heap_write_func( void * pOpaque, mz_uint64 file_ofs, const void * pBuf, size_t n )
    {
        mz_zip_archive * pZip = (mz_zip_archive *)pOpaque;
        mz_zip_internal_state * pState = pZip->m_pState;
        mz_uint64 new_size = MZ_MAX( file_ofs + n, pState->m_mem_size );

        if ( !n )
            return 0;
        if ( ( sizeof( size_t ) == sizeof( mz_uint32 ) ) && ( new_size > 0x7FFFFFFF ) )
        {
            mz_zip_set_error( pZip, MZ_ZIP_FILE_TOO_LARGE );
            return 0;
        }

        if ( new_size > pState->m_mem_capacity )
        {
            void * pNew_block;
            size_t new_capacity = MZ_MAX( 64, pState->m_mem_capacity );

            while ( new_capacity < new_size )
                new_capacity *= 2;

            if ( NULL == ( pNew_block = pZip->m_pRealloc( pZip->m_pAlloc_opaque, pState->m_pMem, 1, new_capacity ) ) )
            {
                mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
                return 0;
            }

            pState->m_pMem = pNew_block;
            pState->m_mem_capacity = new_capacity;
        }
        memcpy( (mz_uint8 *)pState->m_pMem + file_ofs, pBuf, n );
        pState->m_mem_size = (size_t)new_size;
        return n;
    }
    static mz_bool mz_zip_writer_end_internal( mz_zip_archive * pZip, mz_bool set_last_error )
    {
        mz_zip_internal_state * pState;
        mz_bool status = true;

        if ( ( !pZip ) || ( !pZip->m_pState ) || ( !pZip->m_pAlloc ) || ( !pZip->m_pFree ) ||
             ( ( pZip->m_zip_mode != MZ_ZIP_MODE_WRITING ) && ( pZip->m_zip_mode != MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED ) ) )
        {
            if ( set_last_error )
                mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
            return false;
        }

        pState = pZip->m_pState;
        pZip->m_pState = NULL;
        mz_zip_array_clear( pZip, &pState->m_central_dir );
        mz_zip_array_clear( pZip, &pState->m_central_dir_offsets );
        mz_zip_array_clear( pZip, &pState->m_sorted_central_dir_offsets );

        if ( pState->m_pFile )
        {
            if ( pZip->m_zip_type == MZ_ZIP_TYPE_FILE )
            {
                if ( MZ_FCLOSE( pState->m_pFile ) == EOF )
                {
                    if ( set_last_error )
                        mz_zip_set_error( pZip, MZ_ZIP_FILE_CLOSE_FAILED );
                    status = false;
                }
            }

            pState->m_pFile = NULL;
        }

        if ( ( pZip->m_pWrite == mz_zip_heap_write_func ) && ( pState->m_pMem ) )
        {
            pZip->m_pFree( pZip->m_pAlloc_opaque, pState->m_pMem );
            pState->m_pMem = NULL;
        }

        pZip->m_pFree( pZip->m_pAlloc_opaque, pState );
        pZip->m_zip_mode = MZ_ZIP_MODE_INVALID;
        return status;
    }
    mz_bool mz_zip_writer_init_v2( mz_zip_archive * pZip, mz_uint64 existing_size, mz_uint32 flags )
    {
        mz_bool zip64 = ( flags & MZ_ZIP_FLAG_WRITE_ZIP64 ) != 0;

        if ( ( !pZip ) || ( pZip->m_pState ) || ( !pZip->m_pWrite ) || ( pZip->m_zip_mode != MZ_ZIP_MODE_INVALID ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( flags & MZ_ZIP_FLAG_WRITE_ALLOW_READING )
        {
            if ( !pZip->m_pRead )
                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
        }

        if ( pZip->m_file_offset_alignment )
        {

            if ( pZip->m_file_offset_alignment & ( pZip->m_file_offset_alignment - 1 ) )
                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
        }

        if ( !pZip->m_pAlloc )
            pZip->m_pAlloc = miniz_def_alloc_func;
        if ( !pZip->m_pFree )
            pZip->m_pFree = miniz_def_free_func;
        if ( !pZip->m_pRealloc )
            pZip->m_pRealloc = miniz_def_realloc_func;

        pZip->m_archive_size = existing_size;
        pZip->m_central_directory_file_ofs = 0;
        pZip->m_total_files = 0;

        if ( NULL == ( pZip->m_pState = (mz_zip_internal_state *)pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, sizeof( mz_zip_internal_state ) ) ) )
            return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );

        memset( pZip->m_pState, 0, sizeof( mz_zip_internal_state ) );

        MZ_ZIP_ARRAY_SET_ELEMENT_SIZE( &pZip->m_pState->m_central_dir, sizeof( mz_uint8 ) );
        MZ_ZIP_ARRAY_SET_ELEMENT_SIZE( &pZip->m_pState->m_central_dir_offsets, sizeof( mz_uint32 ) );
        MZ_ZIP_ARRAY_SET_ELEMENT_SIZE( &pZip->m_pState->m_sorted_central_dir_offsets, sizeof( mz_uint32 ) );

        pZip->m_pState->m_zip64 = zip64;
        pZip->m_pState->m_zip64_has_extended_info_fields = zip64;

        pZip->m_zip_type = MZ_ZIP_TYPE_USER;
        pZip->m_zip_mode = MZ_ZIP_MODE_WRITING;

        return true;
    }
    mz_bool mz_zip_writer_init( mz_zip_archive * pZip, mz_uint64 existing_size )
    {
        return mz_zip_writer_init_v2( pZip, existing_size, 0 );
    }
    mz_bool mz_zip_writer_init_heap_v2( mz_zip_archive * pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size, mz_uint32 flags )
    {
        pZip->m_pWrite = mz_zip_heap_write_func;
        pZip->m_pNeeds_keepalive = NULL;

        if ( flags & MZ_ZIP_FLAG_WRITE_ALLOW_READING )
            pZip->m_pRead = mz_zip_mem_read_func;

        pZip->m_pIO_opaque = pZip;

        if ( !mz_zip_writer_init_v2( pZip, size_to_reserve_at_beginning, flags ) )
            return false;

        pZip->m_zip_type = MZ_ZIP_TYPE_HEAP;

        if ( 0 != ( initial_allocation_size = MZ_MAX( initial_allocation_size, size_to_reserve_at_beginning ) ) )
        {
            if ( NULL == ( pZip->m_pState->m_pMem = pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, initial_allocation_size ) ) )
            {
                mz_zip_writer_end_internal( pZip, false );
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            }
            pZip->m_pState->m_mem_capacity = initial_allocation_size;
        }

        return true;
    }
    mz_bool mz_zip_writer_init_heap( mz_zip_archive * pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size )
    {
        return mz_zip_writer_init_heap_v2( pZip, size_to_reserve_at_beginning, initial_allocation_size, 0 );
    }
    static size_t mz_zip_file_write_func( void * pOpaque, mz_uint64 file_ofs, const void * pBuf, size_t n )
    {
        mz_zip_archive * pZip = (mz_zip_archive *)pOpaque;
        mz_int64 cur_ofs = MZ_FTELL64( pZip->m_pState->m_pFile );

        file_ofs += pZip->m_pState->m_file_archive_start_ofs;

        if ( ( (mz_int64)file_ofs < 0 ) || ( ( ( cur_ofs != (mz_int64)file_ofs ) ) && ( MZ_FSEEK64( pZip->m_pState->m_pFile, (mz_int64)file_ofs, SEEK_SET ) ) ) )
        {
            mz_zip_set_error( pZip, MZ_ZIP_FILE_SEEK_FAILED );
            return 0;
        }

        return MZ_FWRITE( pBuf, 1, n, pZip->m_pState->m_pFile );
    }
    mz_bool mz_zip_writer_init_file( mz_zip_archive * pZip, const char * pFilename, mz_uint64 size_to_reserve_at_beginning )
    {
        return mz_zip_writer_init_file_v2( pZip, pFilename, size_to_reserve_at_beginning, 0 );
    }
    mz_bool mz_zip_writer_init_file_v2( mz_zip_archive * pZip, const char * pFilename, mz_uint64 size_to_reserve_at_beginning, mz_uint32 flags )
    {
        FILE * pFile;

        pZip->m_pWrite = mz_zip_file_write_func;
        pZip->m_pNeeds_keepalive = NULL;

        if ( flags & MZ_ZIP_FLAG_WRITE_ALLOW_READING )
            pZip->m_pRead = mz_zip_file_read_func;

        pZip->m_pIO_opaque = pZip;

        if ( !mz_zip_writer_init_v2( pZip, size_to_reserve_at_beginning, flags ) )
            return false;

        if ( NULL == ( pFile = MZ_FOPEN( pFilename, ( flags & MZ_ZIP_FLAG_WRITE_ALLOW_READING ) ? "w+b" : "wb" ) ) )
        {
            mz_zip_writer_end( pZip );
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_OPEN_FAILED );
        }

        pZip->m_pState->m_pFile = pFile;
        pZip->m_zip_type = MZ_ZIP_TYPE_FILE;

        if ( size_to_reserve_at_beginning )
        {
            mz_uint64 cur_ofs = 0;
            char buf[4096];

            MZ_CLEAR_OBJ( buf );

            do
            {
                size_t n = (size_t)MZ_MIN( sizeof( buf ), size_to_reserve_at_beginning );
                if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_ofs, buf, n ) != n )
                {
                    mz_zip_writer_end( pZip );
                    return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
                }
                cur_ofs += n;
                size_to_reserve_at_beginning -= n;
            } while ( size_to_reserve_at_beginning );
        }

        return true;
    }
    mz_bool mz_zip_writer_init_cfile( mz_zip_archive * pZip, FILE * pFile, mz_uint32 flags )
    {
        pZip->m_pWrite = mz_zip_file_write_func;
        pZip->m_pNeeds_keepalive = NULL;

        if ( flags & MZ_ZIP_FLAG_WRITE_ALLOW_READING )
            pZip->m_pRead = mz_zip_file_read_func;

        pZip->m_pIO_opaque = pZip;

        if ( !mz_zip_writer_init_v2( pZip, 0, flags ) )
            return false;

        pZip->m_pState->m_pFile = pFile;
        pZip->m_pState->m_file_archive_start_ofs = MZ_FTELL64( pZip->m_pState->m_pFile );
        pZip->m_zip_type = MZ_ZIP_TYPE_CFILE;

        return true;
    }
    mz_bool mz_zip_writer_init_from_reader_v2( mz_zip_archive * pZip, const char * pFilename, mz_uint32 flags )
    {
        mz_zip_internal_state * pState;

        if ( ( !pZip ) || ( !pZip->m_pState ) || ( pZip->m_zip_mode != MZ_ZIP_MODE_READING ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( flags & MZ_ZIP_FLAG_WRITE_ZIP64 )
        {
            if ( !pZip->m_pState->m_zip64 )
                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
        }
        if ( pZip->m_pState->m_zip64 )
        {
            if ( pZip->m_total_files == MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_TOO_MANY_FILES );
        }
        else
        {
            if ( pZip->m_total_files == MZ_UINT16_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_TOO_MANY_FILES );

            if ( ( pZip->m_archive_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + MZ_ZIP_LOCAL_DIR_HEADER_SIZE ) > MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_TOO_LARGE );
        }

        pState = pZip->m_pState;

        if ( pState->m_pFile )
        {
            if ( pZip->m_pIO_opaque != pZip )
                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

            if ( pZip->m_zip_type == MZ_ZIP_TYPE_FILE )
            {
                if ( !pFilename )
                    return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
                if ( NULL == ( pState->m_pFile = MZ_FREOPEN( pFilename, "r+b", pState->m_pFile ) ) )
                {

                    mz_zip_reader_end_internal( pZip, false );
                    return mz_zip_set_error( pZip, MZ_ZIP_FILE_OPEN_FAILED );
                }
            }

            pZip->m_pWrite = mz_zip_file_write_func;
            pZip->m_pNeeds_keepalive = NULL;
        }
        else if ( pState->m_pMem )
        {

            if ( pZip->m_pIO_opaque != pZip )
                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

            pState->m_mem_capacity = pState->m_mem_size;
            pZip->m_pWrite = mz_zip_heap_write_func;
            pZip->m_pNeeds_keepalive = NULL;
        }

        else if ( !pZip->m_pWrite )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        pZip->m_archive_size = pZip->m_central_directory_file_ofs;
        pZip->m_central_directory_file_ofs = 0;


        mz_zip_array_clear( pZip, &pZip->m_pState->m_sorted_central_dir_offsets );

        pZip->m_zip_mode = MZ_ZIP_MODE_WRITING;

        return true;
    }
    mz_bool mz_zip_writer_init_from_reader( mz_zip_archive * pZip, const char * pFilename )
    {
        return mz_zip_writer_init_from_reader_v2( pZip, pFilename, 0 );
    }
    mz_bool mz_zip_writer_add_mem( mz_zip_archive * pZip, const char * pArchive_name, const void * pBuf, size_t buf_size, mz_uint32 level_and_flags )
    {
        return mz_zip_writer_add_mem_ex( pZip, pArchive_name, pBuf, buf_size, NULL, 0, level_and_flags, 0, 0 );
    }
    static mz_bool mz_zip_writer_add_put_buf_callback( const void * pBuf, int len, void * pUser )
    {
        mz_zip_writer_add_state * pState = (mz_zip_writer_add_state *)pUser;
        if ( (int)pState->m_pZip->m_pWrite( pState->m_pZip->m_pIO_opaque, pState->m_cur_archive_file_ofs, pBuf, len ) != len )
            return false;

        pState->m_cur_archive_file_ofs += len;
        pState->m_comp_size += len;
        return true;
    }
    static mz_uint32 mz_zip_writer_create_zip64_extra_data( mz_uint8 * pBuf, mz_uint64 * pUncomp_size, mz_uint64 * pComp_size, mz_uint64 * pLocal_header_ofs )
    {
        mz_uint8 * pDst = pBuf;
        mz_uint32 field_size = 0;

        MZ_WRITE_LE16( pDst + 0, MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID );
        MZ_WRITE_LE16( pDst + 2, 0 );
        pDst += sizeof( mz_uint16 ) * 2;

        if ( pUncomp_size )
        {
            MZ_WRITE_LE64( pDst, *pUncomp_size );
            pDst += sizeof( mz_uint64 );
            field_size += sizeof( mz_uint64 );
        }

        if ( pComp_size )
        {
            MZ_WRITE_LE64( pDst, *pComp_size );
            pDst += sizeof( mz_uint64 );
            field_size += sizeof( mz_uint64 );
        }

        if ( pLocal_header_ofs )
        {
            MZ_WRITE_LE64( pDst, *pLocal_header_ofs );
            pDst += sizeof( mz_uint64 );
            field_size += sizeof( mz_uint64 );
        }

        MZ_WRITE_LE16( pBuf + 2, field_size );

        return (mz_uint32)( pDst - pBuf );
    }
    static mz_bool mz_zip_writer_create_local_dir_header( mz_zip_archive * pZip, mz_uint8 * pDst, mz_uint16 filename_size, mz_uint16 extra_size, mz_uint64 uncomp_size, mz_uint64 comp_size, mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags, mz_uint16 dos_time, mz_uint16 dos_date )
    {
        (void)pZip;
        memset( pDst, 0, MZ_ZIP_LOCAL_DIR_HEADER_SIZE );
        MZ_WRITE_LE32( pDst + MZ_ZIP_LDH_SIG_OFS, MZ_ZIP_LOCAL_DIR_HEADER_SIG );
        MZ_WRITE_LE16( pDst + MZ_ZIP_LDH_VERSION_NEEDED_OFS, method ? 20 : 0 );
        MZ_WRITE_LE16( pDst + MZ_ZIP_LDH_BIT_FLAG_OFS, bit_flags );
        MZ_WRITE_LE16( pDst + MZ_ZIP_LDH_METHOD_OFS, method );
        MZ_WRITE_LE16( pDst + MZ_ZIP_LDH_FILE_TIME_OFS, dos_time );
        MZ_WRITE_LE16( pDst + MZ_ZIP_LDH_FILE_DATE_OFS, dos_date );
        MZ_WRITE_LE32( pDst + MZ_ZIP_LDH_CRC32_OFS, uncomp_crc32 );
        MZ_WRITE_LE32( pDst + MZ_ZIP_LDH_COMPRESSED_SIZE_OFS, MZ_MIN( comp_size, MZ_UINT32_MAX ) );
        MZ_WRITE_LE32( pDst + MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS, MZ_MIN( uncomp_size, MZ_UINT32_MAX ) );
        MZ_WRITE_LE16( pDst + MZ_ZIP_LDH_FILENAME_LEN_OFS, filename_size );
        MZ_WRITE_LE16( pDst + MZ_ZIP_LDH_EXTRA_LEN_OFS, extra_size );
        return true;
    }
    static mz_bool mz_zip_writer_create_central_dir_header( mz_zip_archive * pZip, mz_uint8 * pDst, mz_uint16 filename_size, mz_uint16 extra_size, mz_uint16 comment_size, mz_uint64 uncomp_size, mz_uint64 comp_size, mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags, mz_uint16 dos_time, mz_uint16 dos_date, mz_uint64 local_header_ofs, mz_uint32 ext_attributes )
    {
        (void)pZip;
        memset( pDst, 0, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE );
        MZ_WRITE_LE32( pDst + MZ_ZIP_CDH_SIG_OFS, MZ_ZIP_CENTRAL_DIR_HEADER_SIG );
        MZ_WRITE_LE16( pDst + MZ_ZIP_CDH_VERSION_NEEDED_OFS, method ? 20 : 0 );
        MZ_WRITE_LE16( pDst + MZ_ZIP_CDH_BIT_FLAG_OFS, bit_flags );
        MZ_WRITE_LE16( pDst + MZ_ZIP_CDH_METHOD_OFS, method );
        MZ_WRITE_LE16( pDst + MZ_ZIP_CDH_FILE_TIME_OFS, dos_time );
        MZ_WRITE_LE16( pDst + MZ_ZIP_CDH_FILE_DATE_OFS, dos_date );
        MZ_WRITE_LE32( pDst + MZ_ZIP_CDH_CRC32_OFS, uncomp_crc32 );
        MZ_WRITE_LE32( pDst + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS, MZ_MIN( comp_size, MZ_UINT32_MAX ) );
        MZ_WRITE_LE32( pDst + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS, MZ_MIN( uncomp_size, MZ_UINT32_MAX ) );
        MZ_WRITE_LE16( pDst + MZ_ZIP_CDH_FILENAME_LEN_OFS, filename_size );
        MZ_WRITE_LE16( pDst + MZ_ZIP_CDH_EXTRA_LEN_OFS, extra_size );
        MZ_WRITE_LE16( pDst + MZ_ZIP_CDH_COMMENT_LEN_OFS, comment_size );
        MZ_WRITE_LE32( pDst + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS, ext_attributes );
        MZ_WRITE_LE32( pDst + MZ_ZIP_CDH_LOCAL_HEADER_OFS, MZ_MIN( local_header_ofs, MZ_UINT32_MAX ) );
        return true;
    }
    static mz_bool mz_zip_writer_add_to_central_dir( mz_zip_archive * pZip, const char * pFilename, mz_uint16 filename_size, const void * pExtra, mz_uint16 extra_size, const void * pComment, mz_uint16 comment_size, mz_uint64 uncomp_size, mz_uint64 comp_size, mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags, mz_uint16 dos_time, mz_uint16 dos_date, mz_uint64 local_header_ofs, mz_uint32 ext_attributes, const char * user_extra_data, mz_uint32 user_extra_data_len )
    {
        mz_zip_internal_state * pState = pZip->m_pState;
        mz_uint32 central_dir_ofs = (mz_uint32)pState->m_central_dir.m_size;
        size_t orig_central_dir_size = pState->m_central_dir.m_size;
        mz_uint8 central_dir_header[MZ_ZIP_CENTRAL_DIR_HEADER_SIZE];

        if ( !pZip->m_pState->m_zip64 )
        {
            if ( local_header_ofs > 0xFFFFFFFF )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_TOO_LARGE );
        }
        if ( ( (mz_uint64)pState->m_central_dir.m_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size + extra_size + user_extra_data_len + comment_size ) >=
             MZ_UINT32_MAX )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE );

        if ( !mz_zip_writer_create_central_dir_header( pZip, central_dir_header, filename_size, (mz_uint16)( extra_size + user_extra_data_len ), comment_size,
             uncomp_size, comp_size, uncomp_crc32, method, bit_flags, dos_time, dos_date, local_header_ofs, ext_attributes ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INTERNAL_ERROR );

        if ( ( !mz_zip_array_push_back( pZip, &pState->m_central_dir, central_dir_header, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE ) ) ||
             ( !mz_zip_array_push_back( pZip, &pState->m_central_dir, pFilename, filename_size ) ) ||
             ( !mz_zip_array_push_back( pZip, &pState->m_central_dir, pExtra, extra_size ) ) ||
             ( !mz_zip_array_push_back( pZip, &pState->m_central_dir, user_extra_data, user_extra_data_len ) ) ||
             ( !mz_zip_array_push_back( pZip, &pState->m_central_dir, pComment, comment_size ) ) ||
             ( !mz_zip_array_push_back( pZip, &pState->m_central_dir_offsets, &central_dir_ofs, 1 ) ) )
        {

            mz_zip_array_resize( pZip, &pState->m_central_dir, orig_central_dir_size, false );
            return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
        }

        return true;
    }
    static mz_bool mz_zip_writer_validate_archive_name( const char * pArchive_name )
    {
        if ( *pArchive_name == '/' )
            return false;

        return true;
    }
    static mz_uint32 mz_zip_writer_compute_padding_needed_for_file_alignment( mz_zip_archive * pZip )
    {
        mz_uint32 n;
        if ( !pZip->m_file_offset_alignment )
            return 0;
        n = (mz_uint32)( pZip->m_archive_size & ( pZip->m_file_offset_alignment - 1 ) );
        return (mz_uint32)( ( pZip->m_file_offset_alignment - n ) & ( pZip->m_file_offset_alignment - 1 ) );
    }
    static mz_bool mz_zip_writer_write_zeros( mz_zip_archive * pZip, mz_uint64 cur_file_ofs, mz_uint32 n )
    {
        char buf[4096];
        memset( buf, 0, MZ_MIN( sizeof( buf ), n ) );
        while ( n )
        {
            mz_uint32 s = MZ_MIN( sizeof( buf ), n );
            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_file_ofs, buf, s ) != s )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            cur_file_ofs += s;
            n -= s;
        }
        return true;
    }
    mz_bool mz_zip_writer_add_mem_ex( mz_zip_archive * pZip, const char * pArchive_name, const void * pBuf, size_t buf_size, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32 )
    {
        return mz_zip_writer_add_mem_ex_v2(
            pZip, pArchive_name, pBuf, buf_size, pComment, comment_size, level_and_flags, uncomp_size, uncomp_crc32, NULL, NULL, 0, NULL, 0 );
    }
    mz_bool mz_zip_writer_add_mem_ex_v2( mz_zip_archive * pZip, const char * pArchive_name, const void * pBuf, size_t buf_size, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32, time_t * last_modified, const char * user_extra_data, mz_uint32 user_extra_data_len, const char * user_extra_data_central, mz_uint32 user_extra_data_central_len )
    {
        mz_uint16 method = 0, dos_time = 0, dos_date = 0;
        mz_uint32 level, ext_attributes = 0, num_alignment_padding_bytes;
        mz_uint64 local_dir_header_ofs = pZip->m_archive_size, cur_archive_file_ofs = pZip->m_archive_size, comp_size = 0;
        size_t archive_name_size;
        mz_uint8 local_dir_header[MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
        tdefl_compressor * pComp = NULL;
        mz_bool store_data_uncompressed;
        mz_zip_internal_state * pState;
        mz_uint8 * pExtra_data = NULL;
        mz_uint32 extra_size = 0;
        mz_uint8 extra_data[MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE];
        mz_uint16 bit_flags = 0;

        if ( (int)level_and_flags < 0 )
            level_and_flags = MZ_DEFAULT_LEVEL;

        if ( uncomp_size || ( buf_size && !( level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) ) )
            bit_flags |= MZ_ZIP_LDH_BIT_FLAG_HAS_LOCATOR;

        if ( !( level_and_flags & MZ_ZIP_FLAG_ASCII_FILENAME ) )
            bit_flags |= MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_UTF8;

        level = level_and_flags & 0xF;
        store_data_uncompressed = ( ( !level ) || ( level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) );

        if ( ( !pZip ) || ( !pZip->m_pState ) || ( pZip->m_zip_mode != MZ_ZIP_MODE_WRITING ) || ( ( buf_size ) && ( !pBuf ) ) || ( !pArchive_name ) ||
             ( ( comment_size ) && ( !pComment ) ) || ( level > MZ_UBER_COMPRESSION ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        pState = pZip->m_pState;

        if ( pState->m_zip64 )
        {
            if ( pZip->m_total_files == MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_TOO_MANY_FILES );
        }
        else
        {
            if ( pZip->m_total_files == MZ_UINT16_MAX )
            {
                pState->m_zip64 = true;

            }
            if ( ( buf_size > 0xFFFFFFFF ) || ( uncomp_size > 0xFFFFFFFF ) )
            {
                pState->m_zip64 = true;

            }
        }

        if ( ( !( level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) ) && ( uncomp_size ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( !mz_zip_writer_validate_archive_name( pArchive_name ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_FILENAME );

        if ( last_modified != NULL )
        {
            mz_zip_time_t_to_dos_time( *last_modified, &dos_time, &dos_date );
        }
        else
        {
            time_t cur_time;
            time( &cur_time );
            mz_zip_time_t_to_dos_time( cur_time, &dos_time, &dos_date );
        }

        if ( !( level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) )
        {
            uncomp_crc32 = (mz_uint32)mz_crc32( 0, (const mz_uint8 *)pBuf, buf_size );
            uncomp_size = buf_size;
            if ( uncomp_size <= 3 )
            {
                level = 0;
                store_data_uncompressed = true;
            }
        }

        archive_name_size = strlen( pArchive_name );
        if ( archive_name_size > MZ_UINT16_MAX )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_FILENAME );

        num_alignment_padding_bytes = mz_zip_writer_compute_padding_needed_for_file_alignment( pZip );
        if ( ( (mz_uint64)pState->m_central_dir.m_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size + MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE +
             comment_size ) >= MZ_UINT32_MAX )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE );

        if ( !pState->m_zip64 )
        {

            if ( ( pZip->m_archive_size + num_alignment_padding_bytes + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + archive_name_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
                 archive_name_size + comment_size + user_extra_data_len + pState->m_central_dir.m_size + MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE +
                 user_extra_data_central_len + MZ_ZIP_DATA_DESCRIPTER_SIZE32 ) > 0xFFFFFFFF )
            {
                pState->m_zip64 = true;

            }
        }

        if ( ( archive_name_size ) && ( pArchive_name[archive_name_size - 1] == '/' ) )
        {

            ext_attributes |= MZ_ZIP_DOS_DIR_ATTRIBUTE_BITFLAG;
            if ( ( buf_size ) || ( uncomp_size ) )
                return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
        }

        if ( ( !mz_zip_array_ensure_room( pZip, &pState->m_central_dir,
             MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size + comment_size + ( pState->m_zip64 ? MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE : 0 ) ) ) ||
             ( !mz_zip_array_ensure_room( pZip, &pState->m_central_dir_offsets, 1 ) ) )
            return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );

        if ( ( !store_data_uncompressed ) && ( buf_size ) )
        {
            if ( NULL == ( pComp = (tdefl_compressor *)pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, sizeof( tdefl_compressor ) ) ) )
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
        }

        if ( !mz_zip_writer_write_zeros( pZip, cur_archive_file_ofs, num_alignment_padding_bytes ) )
        {
            pZip->m_pFree( pZip->m_pAlloc_opaque, pComp );
            return false;
        }

        local_dir_header_ofs += num_alignment_padding_bytes;
        if ( pZip->m_file_offset_alignment )
        {
            assert( ( local_dir_header_ofs & ( pZip->m_file_offset_alignment - 1 ) ) == 0 );
        }
        cur_archive_file_ofs += num_alignment_padding_bytes;

        MZ_CLEAR_OBJ( local_dir_header );

        if ( !store_data_uncompressed || ( level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA ) )
        {
            method = MZ_DEFLATED;
        }

        if ( pState->m_zip64 )
        {
            if ( uncomp_size >= MZ_UINT32_MAX || local_dir_header_ofs >= MZ_UINT32_MAX )
            {
                pExtra_data = extra_data;
                extra_size = mz_zip_writer_create_zip64_extra_data( extra_data, ( uncomp_size >= MZ_UINT32_MAX ) ? &uncomp_size : NULL,
                                                                    ( uncomp_size >= MZ_UINT32_MAX ) ? &comp_size : NULL, ( local_dir_header_ofs >= MZ_UINT32_MAX ) ? &local_dir_header_ofs : NULL );
            }

            if ( !mz_zip_writer_create_local_dir_header( pZip, local_dir_header, (mz_uint16)archive_name_size, (mz_uint16)( extra_size + user_extra_data_len ), 0,
                 0, 0, method, bit_flags, dos_time, dos_date ) )
                return mz_zip_set_error( pZip, MZ_ZIP_INTERNAL_ERROR );

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, local_dir_header_ofs, local_dir_header, sizeof( local_dir_header ) ) != sizeof( local_dir_header ) )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            cur_archive_file_ofs += sizeof( local_dir_header );

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size ) != archive_name_size )
            {
                pZip->m_pFree( pZip->m_pAlloc_opaque, pComp );
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
            }
            cur_archive_file_ofs += archive_name_size;

            if ( pExtra_data != NULL )
            {
                if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, extra_data, extra_size ) != extra_size )
                    return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

                cur_archive_file_ofs += extra_size;
            }
        }
        else
        {
            if ( ( comp_size > MZ_UINT32_MAX ) || ( cur_archive_file_ofs > MZ_UINT32_MAX ) )
                return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );
            if ( !mz_zip_writer_create_local_dir_header(
                pZip, local_dir_header, (mz_uint16)archive_name_size, (mz_uint16)user_extra_data_len, 0, 0, 0, method, bit_flags, dos_time, dos_date ) )
                return mz_zip_set_error( pZip, MZ_ZIP_INTERNAL_ERROR );

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, local_dir_header_ofs, local_dir_header, sizeof( local_dir_header ) ) != sizeof( local_dir_header ) )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            cur_archive_file_ofs += sizeof( local_dir_header );

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size ) != archive_name_size )
            {
                pZip->m_pFree( pZip->m_pAlloc_opaque, pComp );
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
            }
            cur_archive_file_ofs += archive_name_size;
        }

        if ( user_extra_data_len > 0 )
        {
            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, user_extra_data, user_extra_data_len ) != user_extra_data_len )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            cur_archive_file_ofs += user_extra_data_len;
        }

        if ( store_data_uncompressed )
        {
            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, pBuf, buf_size ) != buf_size )
            {
                pZip->m_pFree( pZip->m_pAlloc_opaque, pComp );
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
            }

            cur_archive_file_ofs += buf_size;
            comp_size = buf_size;
        }
        else if ( buf_size )
        {
            mz_zip_writer_add_state state;

            state.m_pZip = pZip;
            state.m_cur_archive_file_ofs = cur_archive_file_ofs;
            state.m_comp_size = 0;

            if ( ( tdefl_init( pComp, mz_zip_writer_add_put_buf_callback, &state, tdefl_create_comp_flags_from_zip_params( level, -15, MZ_DEFAULT_STRATEGY ) ) !=
                 TDEFL_STATUS_OKAY ) ||
                 ( tdefl_compress_buffer( pComp, pBuf, buf_size, TDEFL_FINISH ) != TDEFL_STATUS_DONE ) )
            {
                pZip->m_pFree( pZip->m_pAlloc_opaque, pComp );
                return mz_zip_set_error( pZip, MZ_ZIP_COMPRESSION_FAILED );
            }

            comp_size = state.m_comp_size;
            cur_archive_file_ofs = state.m_cur_archive_file_ofs;
        }

        pZip->m_pFree( pZip->m_pAlloc_opaque, pComp );
        pComp = NULL;

        if ( uncomp_size )
        {
            mz_uint8 local_dir_footer[MZ_ZIP_DATA_DESCRIPTER_SIZE64];
            mz_uint32 local_dir_footer_size = MZ_ZIP_DATA_DESCRIPTER_SIZE32;

            assert( bit_flags & MZ_ZIP_LDH_BIT_FLAG_HAS_LOCATOR );

            MZ_WRITE_LE32( local_dir_footer + 0, MZ_ZIP_DATA_DESCRIPTOR_ID );
            MZ_WRITE_LE32( local_dir_footer + 4, uncomp_crc32 );
            if ( pExtra_data == NULL )
            {
                if ( comp_size > MZ_UINT32_MAX )
                    return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );

                MZ_WRITE_LE32( local_dir_footer + 8, comp_size );
                MZ_WRITE_LE32( local_dir_footer + 12, uncomp_size );
            }
            else
            {
                MZ_WRITE_LE64( local_dir_footer + 8, comp_size );
                MZ_WRITE_LE64( local_dir_footer + 16, uncomp_size );
                local_dir_footer_size = MZ_ZIP_DATA_DESCRIPTER_SIZE64;
            }

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, local_dir_footer, local_dir_footer_size ) != local_dir_footer_size )
                return false;

            cur_archive_file_ofs += local_dir_footer_size;
        }

        if ( pExtra_data != NULL )
        {
            extra_size = mz_zip_writer_create_zip64_extra_data( extra_data, ( uncomp_size >= MZ_UINT32_MAX ) ? &uncomp_size : NULL,
                                                                ( uncomp_size >= MZ_UINT32_MAX ) ? &comp_size : NULL, ( local_dir_header_ofs >= MZ_UINT32_MAX ) ? &local_dir_header_ofs : NULL );
        }

        if ( !mz_zip_writer_add_to_central_dir( pZip, pArchive_name, (mz_uint16)archive_name_size, pExtra_data, (mz_uint16)extra_size, pComment, comment_size,
             uncomp_size, comp_size, uncomp_crc32, method, bit_flags, dos_time, dos_date, local_dir_header_ofs, ext_attributes, user_extra_data_central,
             user_extra_data_central_len ) )
            return false;

        pZip->m_total_files++;
        pZip->m_archive_size = cur_archive_file_ofs;

        return true;
    }
    mz_bool mz_zip_writer_add_read_buf_callback( mz_zip_archive * pZip, const char * pArchive_name, mz_file_read_func read_callback, void * callback_opaque, mz_uint64 max_size, const time_t * pFile_time, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags, const char * user_extra_data, mz_uint32 user_extra_data_len, const char * user_extra_data_central, mz_uint32 user_extra_data_central_len )
    {
        mz_uint16 gen_flags = ( level_and_flags & MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE ) ? 0 : MZ_ZIP_LDH_BIT_FLAG_HAS_LOCATOR;
        mz_uint32 uncomp_crc32 = 0, level, num_alignment_padding_bytes;
        mz_uint16 method = 0, dos_time = 0, dos_date = 0, ext_attributes = 0;
        mz_uint64 local_dir_header_ofs, cur_archive_file_ofs = pZip->m_archive_size, uncomp_size = 0, comp_size = 0;
        size_t archive_name_size;
        mz_uint8 local_dir_header[MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
        mz_uint8 * pExtra_data = NULL;
        mz_uint32 extra_size = 0;
        mz_uint8 extra_data[MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE];
        mz_zip_internal_state * pState;
        mz_uint64 file_ofs = 0, cur_archive_header_file_ofs;

        if ( !( level_and_flags & MZ_ZIP_FLAG_ASCII_FILENAME ) )
            gen_flags |= MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_UTF8;

        if ( (int)level_and_flags < 0 )
            level_and_flags = MZ_DEFAULT_LEVEL;
        level = level_and_flags & 0xF;
        if ( ( !pZip ) || ( !pZip->m_pState ) || ( pZip->m_zip_mode != MZ_ZIP_MODE_WRITING ) || ( !pArchive_name ) || ( ( comment_size ) && ( !pComment ) ) ||
             ( level > MZ_UBER_COMPRESSION ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        pState = pZip->m_pState;

        if ( ( !pState->m_zip64 ) && ( max_size > MZ_UINT32_MAX ) )
        {
            pState->m_zip64 = true;
        }
        if ( level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( !mz_zip_writer_validate_archive_name( pArchive_name ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_FILENAME );

        if ( pState->m_zip64 )
        {
            if ( pZip->m_total_files == MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_TOO_MANY_FILES );
        }
        else
        {
            if ( pZip->m_total_files == MZ_UINT16_MAX )
            {
                pState->m_zip64 = true;

            }
        }

        archive_name_size = strlen( pArchive_name );
        if ( archive_name_size > MZ_UINT16_MAX )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_FILENAME );

        num_alignment_padding_bytes = mz_zip_writer_compute_padding_needed_for_file_alignment( pZip );
        if ( ( (mz_uint64)pState->m_central_dir.m_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size + MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE +
             comment_size ) >= MZ_UINT32_MAX )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE );

        if ( !pState->m_zip64 )
        {

            if ( ( pZip->m_archive_size + num_alignment_padding_bytes + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + archive_name_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
                 archive_name_size + comment_size + user_extra_data_len + pState->m_central_dir.m_size + MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE + 1024 +
                 MZ_ZIP_DATA_DESCRIPTER_SIZE32 + user_extra_data_central_len ) > 0xFFFFFFFF )
            {
                pState->m_zip64 = true;

            }
        }

        if ( pFile_time )
        {
            mz_zip_time_t_to_dos_time( *pFile_time, &dos_time, &dos_date );
        }

        if ( max_size <= 3 )
            level = 0;

        if ( !mz_zip_writer_write_zeros( pZip, cur_archive_file_ofs, num_alignment_padding_bytes ) )
        {
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
        }

        cur_archive_file_ofs += num_alignment_padding_bytes;
        local_dir_header_ofs = cur_archive_file_ofs;

        if ( pZip->m_file_offset_alignment )
        {
            assert( ( cur_archive_file_ofs & ( pZip->m_file_offset_alignment - 1 ) ) == 0 );
        }

        if ( max_size && level )
        {
            method = MZ_DEFLATED;
        }

        MZ_CLEAR_OBJ( local_dir_header );
        if ( pState->m_zip64 )
        {
            if ( max_size >= MZ_UINT32_MAX || local_dir_header_ofs >= MZ_UINT32_MAX )
            {
                pExtra_data = extra_data;
                if ( level_and_flags & MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE )
                    extra_size = mz_zip_writer_create_zip64_extra_data( extra_data, ( max_size >= MZ_UINT32_MAX ) ? &uncomp_size : NULL,
                                                                        ( max_size >= MZ_UINT32_MAX ) ? &comp_size : NULL, ( local_dir_header_ofs >= MZ_UINT32_MAX ) ? &local_dir_header_ofs : NULL );
                else
                    extra_size =
                    mz_zip_writer_create_zip64_extra_data( extra_data, NULL, NULL, ( local_dir_header_ofs >= MZ_UINT32_MAX ) ? &local_dir_header_ofs : NULL );
            }

            if ( !mz_zip_writer_create_local_dir_header( pZip, local_dir_header, (mz_uint16)archive_name_size, (mz_uint16)( extra_size + user_extra_data_len ), 0,
                 0, 0, method, gen_flags, dos_time, dos_date ) )
                return mz_zip_set_error( pZip, MZ_ZIP_INTERNAL_ERROR );

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, local_dir_header, sizeof( local_dir_header ) ) != sizeof( local_dir_header ) )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            cur_archive_file_ofs += sizeof( local_dir_header );

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size ) != archive_name_size )
            {
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
            }

            cur_archive_file_ofs += archive_name_size;

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, extra_data, extra_size ) != extra_size )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            cur_archive_file_ofs += extra_size;
        }
        else
        {
            if ( ( comp_size > MZ_UINT32_MAX ) || ( cur_archive_file_ofs > MZ_UINT32_MAX ) )
                return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );
            if ( !mz_zip_writer_create_local_dir_header(
                pZip, local_dir_header, (mz_uint16)archive_name_size, (mz_uint16)user_extra_data_len, 0, 0, 0, method, gen_flags, dos_time, dos_date ) )
                return mz_zip_set_error( pZip, MZ_ZIP_INTERNAL_ERROR );

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, local_dir_header, sizeof( local_dir_header ) ) != sizeof( local_dir_header ) )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            cur_archive_file_ofs += sizeof( local_dir_header );

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size ) != archive_name_size )
            {
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
            }

            cur_archive_file_ofs += archive_name_size;
        }

        if ( user_extra_data_len > 0 )
        {
            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, user_extra_data, user_extra_data_len ) != user_extra_data_len )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            cur_archive_file_ofs += user_extra_data_len;
        }

        if ( max_size )
        {
            void * pRead_buf = pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, MZ_ZIP_MAX_IO_BUF_SIZE );
            if ( !pRead_buf )
            {
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            }

            if ( !level )
            {
                while ( 1 )
                {
                    size_t n = read_callback( callback_opaque, file_ofs, pRead_buf, MZ_ZIP_MAX_IO_BUF_SIZE );
                    if ( n == 0 )
                        break;

                    if ( ( n > MZ_ZIP_MAX_IO_BUF_SIZE ) || ( file_ofs + n > max_size ) )
                    {
                        pZip->m_pFree( pZip->m_pAlloc_opaque, pRead_buf );
                        return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
                    }
                    if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, pRead_buf, n ) != n )
                    {
                        pZip->m_pFree( pZip->m_pAlloc_opaque, pRead_buf );
                        return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
                    }
                    file_ofs += n;
                    uncomp_crc32 = (mz_uint32)mz_crc32( uncomp_crc32, (const mz_uint8 *)pRead_buf, n );
                    cur_archive_file_ofs += n;
                }
                uncomp_size = file_ofs;
                comp_size = uncomp_size;
            }
            else
            {
                mz_bool result = false;
                mz_zip_writer_add_state state;
                tdefl_compressor * pComp = (tdefl_compressor *)pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, sizeof( tdefl_compressor ) );
                if ( !pComp )
                {
                    pZip->m_pFree( pZip->m_pAlloc_opaque, pRead_buf );
                    return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
                }

                state.m_pZip = pZip;
                state.m_cur_archive_file_ofs = cur_archive_file_ofs;
                state.m_comp_size = 0;

                if ( tdefl_init( pComp, mz_zip_writer_add_put_buf_callback, &state, tdefl_create_comp_flags_from_zip_params( level, -15, MZ_DEFAULT_STRATEGY ) ) !=
                     TDEFL_STATUS_OKAY )
                {
                    pZip->m_pFree( pZip->m_pAlloc_opaque, pComp );
                    pZip->m_pFree( pZip->m_pAlloc_opaque, pRead_buf );
                    return mz_zip_set_error( pZip, MZ_ZIP_INTERNAL_ERROR );
                }

                for ( ;;)
                {
                    tdefl_status status;
                    tdefl_flush flush = TDEFL_NO_FLUSH;

                    size_t n = read_callback( callback_opaque, file_ofs, pRead_buf, MZ_ZIP_MAX_IO_BUF_SIZE );
                    if ( ( n > MZ_ZIP_MAX_IO_BUF_SIZE ) || ( file_ofs + n > max_size ) )
                    {
                        mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
                        break;
                    }

                    file_ofs += n;
                    uncomp_crc32 = (mz_uint32)mz_crc32( uncomp_crc32, (const mz_uint8 *)pRead_buf, n );

                    if ( pZip->m_pNeeds_keepalive != NULL && pZip->m_pNeeds_keepalive( pZip->m_pIO_opaque ) )
                        flush = TDEFL_FULL_FLUSH;

                    if ( n == 0 )
                        flush = TDEFL_FINISH;

                    status = tdefl_compress_buffer( pComp, pRead_buf, n, flush );
                    if ( status == TDEFL_STATUS_DONE )
                    {
                        result = true;
                        break;
                    }
                    else if ( status != TDEFL_STATUS_OKAY )
                    {
                        mz_zip_set_error( pZip, MZ_ZIP_COMPRESSION_FAILED );
                        break;
                    }
                }

                pZip->m_pFree( pZip->m_pAlloc_opaque, pComp );

                if ( !result )
                {
                    pZip->m_pFree( pZip->m_pAlloc_opaque, pRead_buf );
                    return false;
                }

                uncomp_size = file_ofs;
                comp_size = state.m_comp_size;
                cur_archive_file_ofs = state.m_cur_archive_file_ofs;
            }

            pZip->m_pFree( pZip->m_pAlloc_opaque, pRead_buf );
        }

        if ( !( level_and_flags & MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE ) )
        {
            mz_uint8 local_dir_footer[MZ_ZIP_DATA_DESCRIPTER_SIZE64];
            mz_uint32 local_dir_footer_size = MZ_ZIP_DATA_DESCRIPTER_SIZE32;

            MZ_WRITE_LE32( local_dir_footer + 0, MZ_ZIP_DATA_DESCRIPTOR_ID );
            MZ_WRITE_LE32( local_dir_footer + 4, uncomp_crc32 );
            if ( pExtra_data == NULL )
            {
                if ( comp_size > MZ_UINT32_MAX )
                    return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );

                MZ_WRITE_LE32( local_dir_footer + 8, comp_size );
                MZ_WRITE_LE32( local_dir_footer + 12, uncomp_size );
            }
            else
            {
                MZ_WRITE_LE64( local_dir_footer + 8, comp_size );
                MZ_WRITE_LE64( local_dir_footer + 16, uncomp_size );
                local_dir_footer_size = MZ_ZIP_DATA_DESCRIPTER_SIZE64;
            }

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_file_ofs, local_dir_footer, local_dir_footer_size ) != local_dir_footer_size )
                return false;

            cur_archive_file_ofs += local_dir_footer_size;
        }

        if ( level_and_flags & MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE )
        {
            if ( pExtra_data != NULL )
            {
                extra_size = mz_zip_writer_create_zip64_extra_data( extra_data, ( max_size >= MZ_UINT32_MAX ) ? &uncomp_size : NULL,
                                                                    ( max_size >= MZ_UINT32_MAX ) ? &comp_size : NULL, ( local_dir_header_ofs >= MZ_UINT32_MAX ) ? &local_dir_header_ofs : NULL );
            }

            if ( !mz_zip_writer_create_local_dir_header( pZip, local_dir_header, (mz_uint16)archive_name_size, (mz_uint16)( extra_size + user_extra_data_len ),
                 ( max_size >= MZ_UINT32_MAX ) ? MZ_UINT32_MAX : uncomp_size, ( max_size >= MZ_UINT32_MAX ) ? MZ_UINT32_MAX : comp_size, uncomp_crc32, method,
                 gen_flags, dos_time, dos_date ) )
                return mz_zip_set_error( pZip, MZ_ZIP_INTERNAL_ERROR );

            cur_archive_header_file_ofs = local_dir_header_ofs;

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_header_file_ofs, local_dir_header, sizeof( local_dir_header ) ) != sizeof( local_dir_header ) )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            if ( pExtra_data != NULL )
            {
                cur_archive_header_file_ofs += sizeof( local_dir_header );

                if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_header_file_ofs, pArchive_name, archive_name_size ) != archive_name_size )
                {
                    return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
                }

                cur_archive_header_file_ofs += archive_name_size;

                if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_archive_header_file_ofs, extra_data, extra_size ) != extra_size )
                    return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

                cur_archive_header_file_ofs += extra_size;
            }
        }

        if ( pExtra_data != NULL )
        {
            extra_size = mz_zip_writer_create_zip64_extra_data( extra_data, ( uncomp_size >= MZ_UINT32_MAX ) ? &uncomp_size : NULL,
                                                                ( uncomp_size >= MZ_UINT32_MAX ) ? &comp_size : NULL, ( local_dir_header_ofs >= MZ_UINT32_MAX ) ? &local_dir_header_ofs : NULL );
        }

        if ( !mz_zip_writer_add_to_central_dir( pZip, pArchive_name, (mz_uint16)archive_name_size, pExtra_data, (mz_uint16)extra_size, pComment, comment_size,
             uncomp_size, comp_size, uncomp_crc32, method, gen_flags, dos_time, dos_date, local_dir_header_ofs, ext_attributes, user_extra_data_central,
             user_extra_data_central_len ) )
            return false;

        pZip->m_total_files++;
        pZip->m_archive_size = cur_archive_file_ofs;

        return true;
    }
    static size_t mz_file_read_func_stdio( void * pOpaque, mz_uint64 file_ofs, void * pBuf, size_t n )
    {
        FILE * pSrc_file = (FILE *)pOpaque;
        mz_int64 cur_ofs = MZ_FTELL64( pSrc_file );

        if ( ( (mz_int64)file_ofs < 0 ) || ( ( ( cur_ofs != (mz_int64)file_ofs ) ) && ( MZ_FSEEK64( pSrc_file, (mz_int64)file_ofs, SEEK_SET ) ) ) )
            return 0;

        return MZ_FREAD( pBuf, 1, n, pSrc_file );
    }
    mz_bool mz_zip_writer_add_cfile( mz_zip_archive * pZip, const char * pArchive_name, FILE * pSrc_file, mz_uint64 max_size, const time_t * pFile_time, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags, const char * user_extra_data, mz_uint32 user_extra_data_len, const char * user_extra_data_central, mz_uint32 user_extra_data_central_len )
    {
        return mz_zip_writer_add_read_buf_callback( pZip, pArchive_name, mz_file_read_func_stdio, pSrc_file, max_size, pFile_time, pComment, comment_size,
                                                    level_and_flags, user_extra_data, user_extra_data_len, user_extra_data_central, user_extra_data_central_len );
    }
    mz_bool mz_zip_writer_add_file( mz_zip_archive * pZip, const char * pArchive_name, const char * pSrc_filename, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags )
    {
        FILE * pSrc_file = NULL;
        mz_uint64 uncomp_size = 0;
        time_t file_modified_time;
        time_t * pFile_time = NULL;
        mz_bool status;

        memset( &file_modified_time, 0, sizeof( file_modified_time ) );

        pFile_time = &file_modified_time;
        if ( !mz_zip_get_file_modified_time( pSrc_filename, &file_modified_time ) )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_STAT_FAILED );

        pSrc_file = MZ_FOPEN( pSrc_filename, "rb" );
        if ( !pSrc_file )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_OPEN_FAILED );

        MZ_FSEEK64( pSrc_file, 0, SEEK_END );
        uncomp_size = MZ_FTELL64( pSrc_file );
        MZ_FSEEK64( pSrc_file, 0, SEEK_SET );

        status = mz_zip_writer_add_cfile( pZip, pArchive_name, pSrc_file, uncomp_size, pFile_time, pComment, comment_size, level_and_flags, NULL, 0, NULL, 0 );

        MZ_FCLOSE( pSrc_file );

        return status;
    }
    static mz_bool mz_zip_writer_update_zip64_extension_block( mz_zip_array * pNew_ext, mz_zip_archive * pZip, const mz_uint8 * pExt, uint32_t ext_len, mz_uint64 * pComp_size, mz_uint64 * pUncomp_size, mz_uint64 * pLocal_header_ofs, mz_uint32 * pDisk_start )
    {

        if ( !mz_zip_array_reserve( pZip, pNew_ext, ext_len + 64, false ) )
            return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );

        mz_zip_array_resize( pZip, pNew_ext, 0, false );

        if ( ( pUncomp_size ) || ( pComp_size ) || ( pLocal_header_ofs ) || ( pDisk_start ) )
        {
            mz_uint8 new_ext_block[64];
            mz_uint8 * pDst = new_ext_block;
            mz_write_le16( pDst, MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID );
            mz_write_le16( pDst + sizeof( mz_uint16 ), 0 );
            pDst += sizeof( mz_uint16 ) * 2;

            if ( pUncomp_size )
            {
                mz_write_le64( pDst, *pUncomp_size );
                pDst += sizeof( mz_uint64 );
            }

            if ( pComp_size )
            {
                mz_write_le64( pDst, *pComp_size );
                pDst += sizeof( mz_uint64 );
            }

            if ( pLocal_header_ofs )
            {
                mz_write_le64( pDst, *pLocal_header_ofs );
                pDst += sizeof( mz_uint64 );
            }

            if ( pDisk_start )
            {
                mz_write_le32( pDst, *pDisk_start );
                pDst += sizeof( mz_uint32 );
            }

            mz_write_le16( new_ext_block + sizeof( mz_uint16 ), (mz_uint16)( ( pDst - new_ext_block ) - sizeof( mz_uint16 ) * 2 ) );

            if ( !mz_zip_array_push_back( pZip, pNew_ext, new_ext_block, pDst - new_ext_block ) )
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
        }

        if ( ( pExt ) && ( ext_len ) )
        {
            mz_uint32 extra_size_remaining = ext_len;
            const mz_uint8 * pExtra_data = pExt;

            do
            {
                mz_uint32 field_id, field_data_size, field_total_size;

                if ( extra_size_remaining < ( sizeof( mz_uint16 ) * 2 ) )
                    return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

                field_id = MZ_READ_LE16( pExtra_data );
                field_data_size = MZ_READ_LE16( pExtra_data + sizeof( mz_uint16 ) );
                field_total_size = field_data_size + sizeof( mz_uint16 ) * 2;

                if ( field_total_size > extra_size_remaining )
                    return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

                if ( field_id != MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID )
                {
                    if ( !mz_zip_array_push_back( pZip, pNew_ext, pExtra_data, field_total_size ) )
                        return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
                }

                pExtra_data += field_total_size;
                extra_size_remaining -= field_total_size;
            } while ( extra_size_remaining );
        }

        return true;
    }
    mz_bool mz_zip_writer_add_from_zip_reader( mz_zip_archive * pZip, mz_zip_archive * pSource_zip, mz_uint32 src_file_index )
    {
        mz_uint32 n, bit_flags, num_alignment_padding_bytes, src_central_dir_following_data_size;
        mz_uint64 src_archive_bytes_remaining, local_dir_header_ofs;
        mz_uint64 cur_src_file_ofs, cur_dst_file_ofs;
        mz_uint32 local_header_u32[( MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof( mz_uint32 ) - 1 ) / sizeof( mz_uint32 )];
        mz_uint8 * pLocal_header = (mz_uint8 *)local_header_u32;
        mz_uint8 new_central_header[MZ_ZIP_CENTRAL_DIR_HEADER_SIZE];
        size_t orig_central_dir_size;
        mz_zip_internal_state * pState;
        void * pBuf;
        const mz_uint8 * pSrc_central_header;
        mz_zip_archive_file_stat src_file_stat;
        mz_uint32 src_filename_len, src_comment_len, src_ext_len;
        mz_uint32 local_header_filename_size, local_header_extra_len;
        mz_uint64 local_header_comp_size, local_header_uncomp_size;
        mz_bool found_zip64_ext_data_in_ldir = false;
        if ( ( !pZip ) || ( !pZip->m_pState ) || ( pZip->m_zip_mode != MZ_ZIP_MODE_WRITING ) || ( !pSource_zip->m_pRead ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        pState = pZip->m_pState;
        if ( ( pSource_zip->m_pState->m_zip64 ) && ( !pZip->m_pState->m_zip64 ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
        if ( NULL == ( pSrc_central_header = mz_zip_get_cdh( pSource_zip, src_file_index ) ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( MZ_READ_LE32( pSrc_central_header + MZ_ZIP_CDH_SIG_OFS ) != MZ_ZIP_CENTRAL_DIR_HEADER_SIG )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

        src_filename_len = MZ_READ_LE16( pSrc_central_header + MZ_ZIP_CDH_FILENAME_LEN_OFS );
        src_comment_len = MZ_READ_LE16( pSrc_central_header + MZ_ZIP_CDH_COMMENT_LEN_OFS );
        src_ext_len = MZ_READ_LE16( pSrc_central_header + MZ_ZIP_CDH_EXTRA_LEN_OFS );
        src_central_dir_following_data_size = src_filename_len + src_ext_len + src_comment_len;
        if ( ( pState->m_central_dir.m_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + src_central_dir_following_data_size + 32 ) >= MZ_UINT32_MAX )
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE );

        num_alignment_padding_bytes = mz_zip_writer_compute_padding_needed_for_file_alignment( pZip );

        if ( !pState->m_zip64 )
        {
            if ( pZip->m_total_files == MZ_UINT16_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_TOO_MANY_FILES );
        }
        else
        {

            if ( pZip->m_total_files == MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_TOO_MANY_FILES );
        }

        if ( !mz_zip_file_stat_internal( pSource_zip, src_file_index, pSrc_central_header, &src_file_stat, NULL ) )
            return false;

        cur_src_file_ofs = src_file_stat.m_local_header_ofs;
        cur_dst_file_ofs = pZip->m_archive_size;
        if ( pSource_zip->m_pRead( pSource_zip->m_pIO_opaque, cur_src_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE ) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );

        if ( MZ_READ_LE32( pLocal_header ) != MZ_ZIP_LOCAL_DIR_HEADER_SIG )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );

        cur_src_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE;
        local_header_filename_size = MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS );
        local_header_extra_len = MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS );
        local_header_comp_size = MZ_READ_LE32( pLocal_header + MZ_ZIP_LDH_COMPRESSED_SIZE_OFS );
        local_header_uncomp_size = MZ_READ_LE32( pLocal_header + MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS );
        src_archive_bytes_remaining = local_header_filename_size + local_header_extra_len + src_file_stat.m_comp_size;
        if ( ( local_header_extra_len ) && ( ( local_header_comp_size == MZ_UINT32_MAX ) || ( local_header_uncomp_size == MZ_UINT32_MAX ) ) )
        {
            mz_zip_array file_data_array;
            const mz_uint8 * pExtra_data;
            mz_uint32 extra_size_remaining = local_header_extra_len;

            mz_zip_array_init( &file_data_array, 1 );
            if ( !mz_zip_array_resize( pZip, &file_data_array, local_header_extra_len, false ) )
            {
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            }

            if ( pSource_zip->m_pRead( pSource_zip->m_pIO_opaque, src_file_stat.m_local_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + local_header_filename_size,
                 file_data_array.m_p, local_header_extra_len ) != local_header_extra_len )
            {
                mz_zip_array_clear( pZip, &file_data_array );
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
            }

            pExtra_data = (const mz_uint8 *)file_data_array.m_p;

            do
            {
                mz_uint32 field_id, field_data_size, field_total_size;

                if ( extra_size_remaining < ( sizeof( mz_uint16 ) * 2 ) )
                {
                    mz_zip_array_clear( pZip, &file_data_array );
                    return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
                }

                field_id = MZ_READ_LE16( pExtra_data );
                field_data_size = MZ_READ_LE16( pExtra_data + sizeof( mz_uint16 ) );
                field_total_size = field_data_size + sizeof( mz_uint16 ) * 2;

                if ( field_total_size > extra_size_remaining )
                {
                    mz_zip_array_clear( pZip, &file_data_array );
                    return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
                }

                if ( field_id == MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID )
                {
                    const mz_uint8 * pSrc_field_data = pExtra_data + sizeof( mz_uint32 );

                    if ( field_data_size < sizeof( mz_uint64 ) * 2 )
                    {
                        mz_zip_array_clear( pZip, &file_data_array );
                        return mz_zip_set_error( pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED );
                    }

                    local_header_uncomp_size = MZ_READ_LE64( pSrc_field_data );
                    local_header_comp_size = MZ_READ_LE64( pSrc_field_data + sizeof( mz_uint64 ) );

                    found_zip64_ext_data_in_ldir = true;
                    break;
                }

                pExtra_data += field_total_size;
                extra_size_remaining -= field_total_size;
            } while ( extra_size_remaining );

            mz_zip_array_clear( pZip, &file_data_array );
        }

        if ( !pState->m_zip64 )
        {
            mz_uint64 approx_new_archive_size = cur_dst_file_ofs + num_alignment_padding_bytes + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + src_archive_bytes_remaining +
                ( sizeof( mz_uint32 ) * 4 ) + pState->m_central_dir.m_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
                src_central_dir_following_data_size + MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE + 64;

            if ( approx_new_archive_size >= MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );
        }
        if ( !mz_zip_writer_write_zeros( pZip, cur_dst_file_ofs, num_alignment_padding_bytes ) )
            return false;

        cur_dst_file_ofs += num_alignment_padding_bytes;

        local_dir_header_ofs = cur_dst_file_ofs;
        if ( pZip->m_file_offset_alignment )
        {
            assert( ( local_dir_header_ofs & ( pZip->m_file_offset_alignment - 1 ) ) == 0 );
        }
        if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_dst_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE ) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

        cur_dst_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE;
        if ( NULL ==
             ( pBuf = pZip->m_pAlloc( pZip->m_pAlloc_opaque, 1, (size_t)MZ_MAX( 32U, MZ_MIN( (mz_uint64)MZ_ZIP_MAX_IO_BUF_SIZE, src_archive_bytes_remaining ) ) ) ) )
            return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );

        while ( src_archive_bytes_remaining )
        {
            n = (mz_uint32)MZ_MIN( (mz_uint64)MZ_ZIP_MAX_IO_BUF_SIZE, src_archive_bytes_remaining );
            if ( pSource_zip->m_pRead( pSource_zip->m_pIO_opaque, cur_src_file_ofs, pBuf, n ) != n )
            {
                pZip->m_pFree( pZip->m_pAlloc_opaque, pBuf );
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
            }
            cur_src_file_ofs += n;

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_dst_file_ofs, pBuf, n ) != n )
            {
                pZip->m_pFree( pZip->m_pAlloc_opaque, pBuf );
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
            }
            cur_dst_file_ofs += n;

            src_archive_bytes_remaining -= n;
        }
        bit_flags = MZ_READ_LE16( pLocal_header + MZ_ZIP_LDH_BIT_FLAG_OFS );
        if ( bit_flags & 8 )
        {

            if ( ( pSource_zip->m_pState->m_zip64 ) || ( found_zip64_ext_data_in_ldir ) )
            {



                if ( pSource_zip->m_pRead( pSource_zip->m_pIO_opaque, cur_src_file_ofs, pBuf, ( sizeof( mz_uint32 ) * 6 ) ) != ( sizeof( mz_uint32 ) * 6 ) )
                {
                    pZip->m_pFree( pZip->m_pAlloc_opaque, pBuf );
                    return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
                }

                n = sizeof( mz_uint32 ) * ( ( MZ_READ_LE32( pBuf ) == MZ_ZIP_DATA_DESCRIPTOR_ID ) ? 6 : 5 );
            }
            else
            {

                mz_bool has_id;

                if ( pSource_zip->m_pRead( pSource_zip->m_pIO_opaque, cur_src_file_ofs, pBuf, sizeof( mz_uint32 ) * 4 ) != sizeof( mz_uint32 ) * 4 )
                {
                    pZip->m_pFree( pZip->m_pAlloc_opaque, pBuf );
                    return mz_zip_set_error( pZip, MZ_ZIP_FILE_READ_FAILED );
                }

                has_id = ( MZ_READ_LE32( pBuf ) == MZ_ZIP_DATA_DESCRIPTOR_ID );

                if ( pZip->m_pState->m_zip64 )
                {

                    const mz_uint32 * pSrc_descriptor = (const mz_uint32 *)( (const mz_uint8 *)pBuf + ( has_id ? sizeof( mz_uint32 ) : 0 ) );
                    const mz_uint32 src_crc32 = pSrc_descriptor[0];
                    const mz_uint64 src_comp_size = pSrc_descriptor[1];
                    const mz_uint64 src_uncomp_size = pSrc_descriptor[2];

                    mz_write_le32( (mz_uint8 *)pBuf, MZ_ZIP_DATA_DESCRIPTOR_ID );
                    mz_write_le32( (mz_uint8 *)pBuf + sizeof( mz_uint32 ) * 1, src_crc32 );
                    mz_write_le64( (mz_uint8 *)pBuf + sizeof( mz_uint32 ) * 2, src_comp_size );
                    mz_write_le64( (mz_uint8 *)pBuf + sizeof( mz_uint32 ) * 4, src_uncomp_size );

                    n = sizeof( mz_uint32 ) * 6;
                }
                else
                {

                    n = sizeof( mz_uint32 ) * ( has_id ? 4 : 3 );
                }
            }

            if ( pZip->m_pWrite( pZip->m_pIO_opaque, cur_dst_file_ofs, pBuf, n ) != n )
            {
                pZip->m_pFree( pZip->m_pAlloc_opaque, pBuf );
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );
            }

            cur_src_file_ofs += n;
            cur_dst_file_ofs += n;
        }
        pZip->m_pFree( pZip->m_pAlloc_opaque, pBuf );
        orig_central_dir_size = pState->m_central_dir.m_size;

        memcpy( new_central_header, pSrc_central_header, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE );

        if ( pState->m_zip64 )
        {
            const mz_uint8 * pSrc_ext = pSrc_central_header + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + src_filename_len;
            mz_zip_array new_ext_block;

            mz_zip_array_init( &new_ext_block, sizeof( mz_uint8 ) );

            MZ_WRITE_LE32( new_central_header + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS, MZ_UINT32_MAX );
            MZ_WRITE_LE32( new_central_header + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS, MZ_UINT32_MAX );
            MZ_WRITE_LE32( new_central_header + MZ_ZIP_CDH_LOCAL_HEADER_OFS, MZ_UINT32_MAX );

            if ( !mz_zip_writer_update_zip64_extension_block(
                &new_ext_block, pZip, pSrc_ext, src_ext_len, &src_file_stat.m_comp_size, &src_file_stat.m_uncomp_size, &local_dir_header_ofs, NULL ) )
            {
                mz_zip_array_clear( pZip, &new_ext_block );
                return false;
            }

            MZ_WRITE_LE16( new_central_header + MZ_ZIP_CDH_EXTRA_LEN_OFS, new_ext_block.m_size );

            if ( !mz_zip_array_push_back( pZip, &pState->m_central_dir, new_central_header, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE ) )
            {
                mz_zip_array_clear( pZip, &new_ext_block );
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            }

            if ( !mz_zip_array_push_back( pZip, &pState->m_central_dir, pSrc_central_header + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, src_filename_len ) )
            {
                mz_zip_array_clear( pZip, &new_ext_block );
                mz_zip_array_resize( pZip, &pState->m_central_dir, orig_central_dir_size, false );
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            }

            if ( !mz_zip_array_push_back( pZip, &pState->m_central_dir, new_ext_block.m_p, new_ext_block.m_size ) )
            {
                mz_zip_array_clear( pZip, &new_ext_block );
                mz_zip_array_resize( pZip, &pState->m_central_dir, orig_central_dir_size, false );
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            }

            if ( !mz_zip_array_push_back(
                pZip, &pState->m_central_dir, pSrc_central_header + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + src_filename_len + src_ext_len, src_comment_len ) )
            {
                mz_zip_array_clear( pZip, &new_ext_block );
                mz_zip_array_resize( pZip, &pState->m_central_dir, orig_central_dir_size, false );
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            }

            mz_zip_array_clear( pZip, &new_ext_block );
        }
        else
        {

            if ( cur_dst_file_ofs > MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );

            if ( local_dir_header_ofs >= MZ_UINT32_MAX )
                return mz_zip_set_error( pZip, MZ_ZIP_ARCHIVE_TOO_LARGE );

            MZ_WRITE_LE32( new_central_header + MZ_ZIP_CDH_LOCAL_HEADER_OFS, local_dir_header_ofs );

            if ( !mz_zip_array_push_back( pZip, &pState->m_central_dir, new_central_header, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE ) )
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );

            if ( !mz_zip_array_push_back(
                pZip, &pState->m_central_dir, pSrc_central_header + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, src_central_dir_following_data_size ) )
            {
                mz_zip_array_resize( pZip, &pState->m_central_dir, orig_central_dir_size, false );
                return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
            }
        }
        if ( pState->m_central_dir.m_size >= MZ_UINT32_MAX )
        {

            mz_zip_array_resize( pZip, &pState->m_central_dir, orig_central_dir_size, false );
            return mz_zip_set_error( pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE );
        }

        n = (mz_uint32)orig_central_dir_size;
        if ( !mz_zip_array_push_back( pZip, &pState->m_central_dir_offsets, &n, 1 ) )
        {
            mz_zip_array_resize( pZip, &pState->m_central_dir, orig_central_dir_size, false );
            return mz_zip_set_error( pZip, MZ_ZIP_ALLOC_FAILED );
        }

        pZip->m_total_files++;
        pZip->m_archive_size = cur_dst_file_ofs;

        return true;
    }
    mz_bool mz_zip_writer_finalize_archive( mz_zip_archive * pZip )
    {
        mz_zip_internal_state * pState;
        mz_uint64 central_dir_ofs, central_dir_size;
        mz_uint8 hdr[256];

        if ( ( !pZip ) || ( !pZip->m_pState ) || ( pZip->m_zip_mode != MZ_ZIP_MODE_WRITING ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        pState = pZip->m_pState;

        if ( pState->m_zip64 )
        {
            if ( ( pZip->m_total_files > MZ_UINT32_MAX ) || ( pState->m_central_dir.m_size >= MZ_UINT32_MAX ) )
                return mz_zip_set_error( pZip, MZ_ZIP_TOO_MANY_FILES );
        }
        else
        {
            if ( ( pZip->m_total_files > MZ_UINT16_MAX ) ||
                 ( ( pZip->m_archive_size + pState->m_central_dir.m_size + MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE ) > MZ_UINT32_MAX ) )
                return mz_zip_set_error( pZip, MZ_ZIP_TOO_MANY_FILES );
        }

        central_dir_ofs = 0;
        central_dir_size = 0;
        if ( pZip->m_total_files )
        {

            central_dir_ofs = pZip->m_archive_size;
            central_dir_size = pState->m_central_dir.m_size;
            pZip->m_central_directory_file_ofs = central_dir_ofs;
            if ( pZip->m_pWrite( pZip->m_pIO_opaque, central_dir_ofs, pState->m_central_dir.m_p, (size_t)central_dir_size ) != central_dir_size )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            pZip->m_archive_size += central_dir_size;
        }

        if ( pState->m_zip64 )
        {
            mz_uint64 rel_ofs_to_zip64_ecdr = pZip->m_archive_size;

            MZ_CLEAR_OBJ( hdr );
            MZ_WRITE_LE32( hdr + MZ_ZIP64_ECDH_SIG_OFS, MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIG );
            MZ_WRITE_LE64( hdr + MZ_ZIP64_ECDH_SIZE_OF_RECORD_OFS, MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE - sizeof( mz_uint32 ) - sizeof( mz_uint64 ) );
            MZ_WRITE_LE16( hdr + MZ_ZIP64_ECDH_VERSION_MADE_BY_OFS, 0x031E );
            MZ_WRITE_LE16( hdr + MZ_ZIP64_ECDH_VERSION_NEEDED_OFS, 0x002D );
            MZ_WRITE_LE64( hdr + MZ_ZIP64_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS, pZip->m_total_files );
            MZ_WRITE_LE64( hdr + MZ_ZIP64_ECDH_CDIR_TOTAL_ENTRIES_OFS, pZip->m_total_files );
            MZ_WRITE_LE64( hdr + MZ_ZIP64_ECDH_CDIR_SIZE_OFS, central_dir_size );
            MZ_WRITE_LE64( hdr + MZ_ZIP64_ECDH_CDIR_OFS_OFS, central_dir_ofs );
            if ( pZip->m_pWrite( pZip->m_pIO_opaque, pZip->m_archive_size, hdr, MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE ) !=
                 MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            pZip->m_archive_size += MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE;
            MZ_CLEAR_OBJ( hdr );
            MZ_WRITE_LE32( hdr + MZ_ZIP64_ECDL_SIG_OFS, MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG );
            MZ_WRITE_LE64( hdr + MZ_ZIP64_ECDL_REL_OFS_TO_ZIP64_ECDR_OFS, rel_ofs_to_zip64_ecdr );
            MZ_WRITE_LE32( hdr + MZ_ZIP64_ECDL_TOTAL_NUMBER_OF_DISKS_OFS, 1 );
            if ( pZip->m_pWrite( pZip->m_pIO_opaque, pZip->m_archive_size, hdr, MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE ) !=
                 MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE )
                return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

            pZip->m_archive_size += MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE;
        }
        MZ_CLEAR_OBJ( hdr );
        MZ_WRITE_LE32( hdr + MZ_ZIP_ECDH_SIG_OFS, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG );
        MZ_WRITE_LE16( hdr + MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS, MZ_MIN( MZ_UINT16_MAX, pZip->m_total_files ) );
        MZ_WRITE_LE16( hdr + MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS, MZ_MIN( MZ_UINT16_MAX, pZip->m_total_files ) );
        MZ_WRITE_LE32( hdr + MZ_ZIP_ECDH_CDIR_SIZE_OFS, MZ_MIN( MZ_UINT32_MAX, central_dir_size ) );
        MZ_WRITE_LE32( hdr + MZ_ZIP_ECDH_CDIR_OFS_OFS, MZ_MIN( MZ_UINT32_MAX, central_dir_ofs ) );

        if ( pZip->m_pWrite( pZip->m_pIO_opaque, pZip->m_archive_size, hdr, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE ) != MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_WRITE_FAILED );

        if ( ( pState->m_pFile ) && ( MZ_FFLUSH( pState->m_pFile ) == EOF ) )
            return mz_zip_set_error( pZip, MZ_ZIP_FILE_CLOSE_FAILED );

        pZip->m_archive_size += MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE;

        pZip->m_zip_mode = MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED;
        return true;
    }
    mz_bool mz_zip_writer_finalize_heap_archive( mz_zip_archive * pZip, void ** ppBuf, size_t * pSize )
    {
        if ( ( !ppBuf ) || ( !pSize ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        *ppBuf = NULL;
        *pSize = 0;

        if ( ( !pZip ) || ( !pZip->m_pState ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( pZip->m_pWrite != mz_zip_heap_write_func )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        if ( !mz_zip_writer_finalize_archive( pZip ) )
            return false;

        *ppBuf = pZip->m_pState->m_pMem;
        *pSize = pZip->m_pState->m_mem_size;
        pZip->m_pState->m_pMem = NULL;
        pZip->m_pState->m_mem_size = pZip->m_pState->m_mem_capacity = 0;

        return true;
    }
    mz_bool mz_zip_writer_end( mz_zip_archive * pZip )
    {
        return mz_zip_writer_end_internal( pZip, true );
    }
    mz_bool mz_zip_add_mem_to_archive_file_in_place( const char * pZip_filename, const char * pArchive_name, const void * pBuf, size_t buf_size, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags )
    {
        return mz_zip_add_mem_to_archive_file_in_place_v2( pZip_filename, pArchive_name, pBuf, buf_size, pComment, comment_size, level_and_flags, NULL );
    }
    mz_bool mz_zip_add_mem_to_archive_file_in_place_v2( const char * pZip_filename, const char * pArchive_name, const void * pBuf, size_t buf_size, const void * pComment, mz_uint16 comment_size, mz_uint32 level_and_flags, mz_zip_error * pErr )
    {
        mz_bool status, created_new_archive = false;
        mz_zip_archive zip_archive;
        struct MZ_FILE_STAT_STRUCT file_stat;
        mz_zip_error actual_err = MZ_ZIP_NO_ERROR;

        mz_zip_zero_struct( &zip_archive );
        if ( (int)level_and_flags < 0 )
            level_and_flags = MZ_DEFAULT_LEVEL;

        if ( ( !pZip_filename ) || ( !pArchive_name ) || ( ( buf_size ) && ( !pBuf ) ) || ( ( comment_size ) && ( !pComment ) ) ||
             ( ( level_and_flags & 0xF ) > MZ_UBER_COMPRESSION ) )
        {
            if ( pErr )
                *pErr = MZ_ZIP_INVALID_PARAMETER;
            return false;
        }

        if ( !mz_zip_writer_validate_archive_name( pArchive_name ) )
        {
            if ( pErr )
                *pErr = MZ_ZIP_INVALID_FILENAME;
            return false;
        }

        if ( MZ_FILE_STAT( pZip_filename, &file_stat ) != 0 )
        {

            if ( !mz_zip_writer_init_file_v2( &zip_archive, pZip_filename, 0, level_and_flags ) )
            {
                if ( pErr )
                    *pErr = zip_archive.m_last_error;
                return false;
            }

            created_new_archive = true;
        }
        else
        {

            if ( !mz_zip_reader_init_file_v2( &zip_archive, pZip_filename, level_and_flags | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY, 0, 0 ) )
            {
                if ( pErr )
                    *pErr = zip_archive.m_last_error;
                return false;
            }

            if ( !mz_zip_writer_init_from_reader_v2( &zip_archive, pZip_filename, level_and_flags ) )
            {
                if ( pErr )
                    *pErr = zip_archive.m_last_error;

                mz_zip_reader_end_internal( &zip_archive, false );

                return false;
            }
        }

        status = mz_zip_writer_add_mem_ex( &zip_archive, pArchive_name, pBuf, buf_size, pComment, comment_size, level_and_flags, 0, 0 );
        actual_err = zip_archive.m_last_error;
        if ( !mz_zip_writer_finalize_archive( &zip_archive ) )
        {
            if ( !actual_err )
                actual_err = zip_archive.m_last_error;

            status = false;
        }

        if ( !mz_zip_writer_end_internal( &zip_archive, status ) )
        {
            if ( !actual_err )
                actual_err = zip_archive.m_last_error;

            status = false;
        }

        if ( ( !status ) && ( created_new_archive ) )
        {

            int ignoredStatus = MZ_DELETE_FILE( pZip_filename );
            (void)ignoredStatus;
        }

        if ( pErr )
            *pErr = actual_err;

        return status;
    }
    void * mz_zip_extract_archive_file_to_heap_v2( const char * pZip_filename, const char * pArchive_name, const char * pComment, size_t * pSize, mz_uint32 flags, mz_zip_error * pErr )
    {
        mz_uint32 file_index;
        mz_zip_archive zip_archive;
        void * p = NULL;

        if ( pSize )
            *pSize = 0;

        if ( ( !pZip_filename ) || ( !pArchive_name ) )
        {
            if ( pErr )
                *pErr = MZ_ZIP_INVALID_PARAMETER;

            return NULL;
        }

        mz_zip_zero_struct( &zip_archive );
        if ( !mz_zip_reader_init_file_v2( &zip_archive, pZip_filename, flags | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY, 0, 0 ) )
        {
            if ( pErr )
                *pErr = zip_archive.m_last_error;

            return NULL;
        }

        if ( mz_zip_reader_locate_file_v2( &zip_archive, pArchive_name, pComment, flags, &file_index ) )
        {
            p = mz_zip_reader_extract_to_heap( &zip_archive, file_index, pSize, flags );
        }

        mz_zip_reader_end_internal( &zip_archive, p != NULL );

        if ( pErr )
            *pErr = zip_archive.m_last_error;

        return p;
    }
    void * mz_zip_extract_archive_file_to_heap( const char * pZip_filename, const char * pArchive_name, size_t * pSize, mz_uint32 flags )
    {
        return mz_zip_extract_archive_file_to_heap_v2( pZip_filename, pArchive_name, NULL, pSize, flags, NULL );
    }
    mz_zip_mode mz_zip_get_mode( mz_zip_archive * pZip )
    {
        return pZip ? pZip->m_zip_mode : MZ_ZIP_MODE_INVALID;
    }
    mz_zip_type mz_zip_get_type( mz_zip_archive * pZip )
    {
        return pZip ? pZip->m_zip_type : MZ_ZIP_TYPE_INVALID;
    }
    mz_zip_error mz_zip_set_last_error( mz_zip_archive * pZip, mz_zip_error err_num )
    {
        mz_zip_error prev_err;

        if ( !pZip )
            return MZ_ZIP_INVALID_PARAMETER;

        prev_err = pZip->m_last_error;

        pZip->m_last_error = err_num;
        return prev_err;
    }
    mz_zip_error mz_zip_peek_last_error( mz_zip_archive * pZip )
    {
        if ( !pZip )
            return MZ_ZIP_INVALID_PARAMETER;

        return pZip->m_last_error;
    }
    mz_zip_error mz_zip_clear_last_error( mz_zip_archive * pZip )
    {
        return mz_zip_set_last_error( pZip, MZ_ZIP_NO_ERROR );
    }
    mz_zip_error mz_zip_get_last_error( mz_zip_archive * pZip )
    {
        mz_zip_error prev_err;

        if ( !pZip )
            return MZ_ZIP_INVALID_PARAMETER;

        prev_err = pZip->m_last_error;

        pZip->m_last_error = MZ_ZIP_NO_ERROR;
        return prev_err;
    }
    const char * mz_zip_get_error_string( mz_zip_error mz_err )
    {
        switch ( mz_err )
        {
        case MZ_ZIP_NO_ERROR:
            return "no error";
        case MZ_ZIP_UNDEFINED_ERROR:
            return "undefined error";
        case MZ_ZIP_TOO_MANY_FILES:
            return "too many files";
        case MZ_ZIP_FILE_TOO_LARGE:
            return "file too large";
        case MZ_ZIP_UNSUPPORTED_METHOD:
            return "unsupported method";
        case MZ_ZIP_UNSUPPORTED_ENCRYPTION:
            return "unsupported encryption";
        case MZ_ZIP_UNSUPPORTED_FEATURE:
            return "unsupported feature";
        case MZ_ZIP_FAILED_FINDING_CENTRAL_DIR:
            return "failed finding central directory";
        case MZ_ZIP_NOT_AN_ARCHIVE:
            return "not a ZIP archive";
        case MZ_ZIP_INVALID_HEADER_OR_CORRUPTED:
            return "invalid header or archive is corrupted";
        case MZ_ZIP_UNSUPPORTED_MULTIDISK:
            return "unsupported multidisk archive";
        case MZ_ZIP_DECOMPRESSION_FAILED:
            return "decompression failed or archive is corrupted";
        case MZ_ZIP_COMPRESSION_FAILED:
            return "compression failed";
        case MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE:
            return "unexpected decompressed size";
        case MZ_ZIP_CRC_CHECK_FAILED:
            return "CRC-32 check failed";
        case MZ_ZIP_UNSUPPORTED_CDIR_SIZE:
            return "unsupported central directory size";
        case MZ_ZIP_ALLOC_FAILED:
            return "allocation failed";
        case MZ_ZIP_FILE_OPEN_FAILED:
            return "file open failed";
        case MZ_ZIP_FILE_CREATE_FAILED:
            return "file create failed";
        case MZ_ZIP_FILE_WRITE_FAILED:
            return "file write failed";
        case MZ_ZIP_FILE_READ_FAILED:
            return "file read failed";
        case MZ_ZIP_FILE_CLOSE_FAILED:
            return "file close failed";
        case MZ_ZIP_FILE_SEEK_FAILED:
            return "file seek failed";
        case MZ_ZIP_FILE_STAT_FAILED:
            return "file stat failed";
        case MZ_ZIP_INVALID_PARAMETER:
            return "invalid parameter";
        case MZ_ZIP_INVALID_FILENAME:
            return "invalid filename";
        case MZ_ZIP_BUF_TOO_SMALL:
            return "buffer too small";
        case MZ_ZIP_INTERNAL_ERROR:
            return "internal error";
        case MZ_ZIP_FILE_NOT_FOUND:
            return "file not found";
        case MZ_ZIP_ARCHIVE_TOO_LARGE:
            return "archive is too large";
        case MZ_ZIP_VALIDATION_FAILED:
            return "validation failed";
        case MZ_ZIP_WRITE_CALLBACK_FAILED:
            return "write calledback failed";
        default:
            break;
        }

        return "unknown error";
    }
    mz_bool mz_zip_is_zip64( mz_zip_archive * pZip )
    {
        if ( ( !pZip ) || ( !pZip->m_pState ) )
            return false;

        return pZip->m_pState->m_zip64;
    }
    size_t mz_zip_get_central_dir_size( mz_zip_archive * pZip )
    {
        if ( ( !pZip ) || ( !pZip->m_pState ) )
            return 0;

        return pZip->m_pState->m_central_dir.m_size;
    }
    mz_uint32 mz_zip_reader_get_num_files( mz_zip_archive * pZip )
    {
        return pZip ? pZip->m_total_files : 0;
    }
    mz_uint64 mz_zip_get_archive_size( mz_zip_archive * pZip )
    {
        if ( !pZip )
            return 0;
        return pZip->m_archive_size;
    }
    mz_uint64 mz_zip_get_archive_file_start_offset( mz_zip_archive * pZip )
    {
        if ( ( !pZip ) || ( !pZip->m_pState ) )
            return 0;
        return pZip->m_pState->m_file_archive_start_ofs;
    }
    FILE * mz_zip_get_cfile( mz_zip_archive * pZip )
    {
        if ( ( !pZip ) || ( !pZip->m_pState ) )
            return 0;
        return pZip->m_pState->m_pFile;
    }
    size_t mz_zip_read_archive_data( mz_zip_archive * pZip, mz_uint64 file_ofs, void * pBuf, size_t n )
    {
        if ( ( !pZip ) || ( !pZip->m_pState ) || ( !pBuf ) || ( !pZip->m_pRead ) )
            return mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );

        return pZip->m_pRead( pZip->m_pIO_opaque, file_ofs, pBuf, n );
    }
    mz_uint32 mz_zip_reader_get_filename( mz_zip_archive * pZip, mz_uint32 file_index, char * pFilename, mz_uint32 filename_buf_size )
    {
        mz_uint32 n;
        const mz_uint8 * p = mz_zip_get_cdh( pZip, file_index );
        if ( !p )
        {
            if ( filename_buf_size )
                pFilename[0] = '\0';
            mz_zip_set_error( pZip, MZ_ZIP_INVALID_PARAMETER );
            return 0;
        }
        n = MZ_READ_LE16( p + MZ_ZIP_CDH_FILENAME_LEN_OFS );
        if ( filename_buf_size )
        {
            n = MZ_MIN( n, filename_buf_size - 1 );
            memcpy( pFilename, p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n );
            pFilename[n] = '\0';
        }
        return n + 1;
    }
    mz_bool mz_zip_reader_file_stat( mz_zip_archive * pZip, mz_uint32 file_index, mz_zip_archive_file_stat * pStat )
    {
        return mz_zip_file_stat_internal( pZip, file_index, mz_zip_get_cdh( pZip, file_index ), pStat, NULL );
    }
    mz_bool mz_zip_end( mz_zip_archive * pZip )
    {
        if ( !pZip )
            return false;

        if ( pZip->m_zip_mode == MZ_ZIP_MODE_READING )
            return mz_zip_reader_end( pZip );
        else if ( ( pZip->m_zip_mode == MZ_ZIP_MODE_WRITING ) || ( pZip->m_zip_mode == MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED ) )
            return mz_zip_writer_end( pZip );

        return false;
    }

    namespace detail
    {

#ifdef _WIN32
        static constexpr char directory_separator = '\\';
        static constexpr char alt_directory_separator = '/';
#else
        static constexpr char directory_separator = '/';
        static constexpr char alt_directory_separator = '\\';
#endif

        std::string join_path( const std::vector<std::string> & parts )
        {
            std::string joined;
            std::size_t i = 0;
            for ( auto part : parts )
            {
                joined.append( part );

                if ( i++ != parts.size() - 1 )
                {
                    joined.append( 1, '/' );
                }
            }
            return joined;
        }

        std::vector<std::string> split_path( const std::string & path, char delim = directory_separator )
        {
            std::vector<std::string> split;
            std::string::size_type previous_index = 0;
            auto separator_index = path.find( delim );

            while ( separator_index != std::string::npos )
            {
                auto part = path.substr( previous_index, separator_index - previous_index );
                if ( part != ".." )
                {
                    split.push_back( part );
                }
                else
                {
                    split.pop_back();
                }
                previous_index = separator_index + 1;
                separator_index = path.find( delim, previous_index );
            }

            split.push_back( path.substr( previous_index ) );

            if ( split.size() == 1 && delim == directory_separator )
            {
                auto alternative = split_path( path, alt_directory_separator );
                if ( alternative.size() > 1 )
                {
                    return alternative;
                }
            }

            return split;
        }

        uint32_t crc32buf( const char * buf, std::size_t len )
        {
            uint32_t oldcrc32 = 0xFFFFFFFF;

            uint32_t crc_32_tab[] = {
                0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
                0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
                0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
                0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
                0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
                0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
                0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
                0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
                0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
                0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
                0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
                0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
                0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
                0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
                0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
                0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
                0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
                0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
                0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
                0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
                0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
                0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
            };

            for ( ; len; --len, ++buf )
            {
                oldcrc32 = UPDC32( *buf, oldcrc32 );
            }

            return ~oldcrc32;
    }

        std::size_t write_callback( void * opaque, std::uint64_t file_ofs, const void * pBuf, std::size_t n )
        {
            auto buffer = static_cast<std::vector<char> *>( opaque );

            if ( file_ofs + n > buffer->size() )
            {
                auto new_size = static_cast<std::vector<char>::size_type>( file_ofs + n );
                buffer->resize( new_size );
            }

            for ( std::size_t i = 0; i < n; i++ )
            {
                ( *buffer )[static_cast<std::size_t>( file_ofs + i )] = ( static_cast<const char *>( pBuf ) )[i];
            }

            return n;
        }

    }  // namespace detail

    struct zip_info
    {
        std::string filename;
        time_t time;
        std::string comment;
        std::string extra;
        uint16_t create_system = 0;
        uint16_t create_version = 0;
        uint16_t extract_version = 0;
        uint16_t flag_bits = 0;
        std::size_t volume = 0;
        uint32_t internal_attr = 0;
        uint32_t external_attr = 0;
        std::size_t header_offset = 0;
        uint32_t crc = 0;
        std::size_t compress_size = 0;
        std::size_t file_size = 0;
    };

    zip_info getinfo( mz_zip_archive * archive_, int index )
    {
        mz_zip_archive_file_stat stat;
        mz_zip_reader_file_stat( archive_, static_cast<mz_uint32>( index ), &stat );

        zip_info result;

        result.filename = std::string( stat.m_filename, stat.m_filename + std::strlen( stat.m_filename ) );
        result.comment = std::string( stat.m_comment, stat.m_comment + stat.m_comment_size );
        result.compress_size = static_cast<std::size_t>( stat.m_comp_size );
        result.file_size = static_cast<std::size_t>( stat.m_uncomp_size );
        result.header_offset = static_cast<std::size_t>( stat.m_local_header_ofs );
        result.crc = stat.m_crc32;
        result.time = stat.m_time;
        result.flag_bits = stat.m_bit_flag;
        result.internal_attr = stat.m_internal_attr;
        result.external_attr = stat.m_external_attr;
        result.extract_version = stat.m_version_needed;
        result.create_version = stat.m_version_made_by;
        result.volume = stat.m_file_index;
        result.create_system = stat.m_method;

        return result;
    }

    std::vector<zip_info> infolist( mz_zip_archive * archive_ )
    {
        std::vector<zip_info> info;

        for ( std::size_t i = 0; i < mz_zip_reader_get_num_files( archive_ ); i++ )
        {
            info.push_back( getinfo( archive_, static_cast<int>( i ) ) );
        }

        return info;
    }

    zip_info getinfo( mz_zip_archive * archive_, const std::string & name )
    {
        int index = mz_zip_reader_locate_file( archive_, name.c_str(), nullptr, 0 );

        if ( index == -1 )
        {
            throw std::runtime_error( "not found" );
        }

        return getinfo( archive_, index );
    }
}

bool x::zip::compression( std::istream & is, std::string & os )
{
    char buf[512];
    bool result = true;
    auto comp = new tdefl_compressor;

    if ( tdefl_init( comp, []( const void * pBuf, int len, void * pUser ) ->mz_bool { ( (std::string *)pUser )->append( reinterpret_cast<const char *>( pBuf ), len ); return true; }, &os, 0 ) == TDEFL_STATUS_OKAY )
    {
        while ( 1 )
        {
            auto sz = is.readsome( buf, 512 );

            if ( sz == 0 )
                break;

            if ( tdefl_compress_buffer( comp, buf, sz, TDEFL_FINISH ) != TDEFL_STATUS_DONE )
            {
                result = false;
                break;
            }
        }
    }

    delete comp;

    return result;
}

bool x::zip::compression( std::istream & is, std::ostream & os )
{
    char buf[512];
    bool result = true;
    auto comp = new tdefl_compressor;

    if ( tdefl_init( comp, []( const void * pBuf, int len, void * pUser ) ->mz_bool { ( (std::ostream *)pUser )->write( reinterpret_cast<const char *>( pBuf ), len ); return true; }, &os, 0 ) == TDEFL_STATUS_OKAY )
    {
        while ( 1 )
        {
            auto sz = is.readsome( buf, 512 );

            if ( sz == 0 )
                break;

            if ( tdefl_compress_buffer( comp, buf, sz, TDEFL_FINISH ) != TDEFL_STATUS_DONE )
            {
                result = false;
                break;
            }
        }
    }

    delete comp;

    return result;
}

bool x::zip::compression( const std::string & is, std::string & os )
{
    bool result = false;
    auto comp = new tdefl_compressor;

    if ( tdefl_init( comp, []( const void * pBuf, int len, void * pUser ) ->mz_bool { ( (std::string *)pUser )->append( reinterpret_cast<const char *>( pBuf ), len ); return true; }, &os, 0 ) == TDEFL_STATUS_OKAY )
    {
        if ( tdefl_compress_buffer( comp, is.c_str(), is.size(), TDEFL_FINISH ) == TDEFL_STATUS_DONE )
        {
            result = true;
        }
    }

    delete comp;

    return result;
}

bool x::zip::compression( const std::string & is, std::ostream & os )
{
    bool result = false;
    auto comp = new tdefl_compressor;

    if ( tdefl_init( comp, []( const void * pBuf, int len, void * pUser ) ->mz_bool { ( (std::ostream *)pUser )->write( reinterpret_cast<const char *>( pBuf ), len ); return true; }, &os, 0 ) == TDEFL_STATUS_OKAY )
    {
        if ( tdefl_compress_buffer( comp, is.c_str(), is.size(), TDEFL_FINISH ) == TDEFL_STATUS_DONE )
        {
            result = true;
        }
    }

    delete comp;

    return result;
}

bool x::zip::decompression( std::istream & is, std::string & os )
{
    char buf[512];
    bool result = true;
    tinfl_decompressor decomp;
    size_t in_buf_ofs = 0, dict_ofs = 0;
    mz_uint8 * dict = (mz_uint8 *)malloc( TINFL_LZ_DICT_SIZE );
    
    tinfl_init( &decomp );

    while( 1 )
    {
        size_t in_buf_size = is.readsome( buf, 512 );
        size_t dst_buf_size = TINFL_LZ_DICT_SIZE - dict_ofs;

        if ( in_buf_size == 0 )
            break;

        if ( tinfl_decompress( &decomp, (const mz_uint8 *)buf, &in_buf_size, dict, dict + dict_ofs, &dst_buf_size, 0 ) == TINFL_STATUS_DONE )
        {
            os.append( reinterpret_cast<const char *>( dict + dict_ofs ), dst_buf_size );
        }
        else
        {
            result = false;
            break;
        }
        
        dict_ofs = ( dict_ofs + dst_buf_size ) & ( TINFL_LZ_DICT_SIZE - 1 );
    }
    free( dict );

    return result;
}

bool x::zip::decompression( std::istream & is, std::ostream & os )
{
    char buf[512];
    bool result = true;
    tinfl_decompressor decomp;
    size_t in_buf_ofs = 0, dict_ofs = 0;
    mz_uint8 * dict = (mz_uint8 *)malloc( TINFL_LZ_DICT_SIZE );

    tinfl_init( &decomp );

    while ( 1 )
    {
        size_t in_buf_size = is.readsome( buf, 512 );
        size_t dst_buf_size = TINFL_LZ_DICT_SIZE - dict_ofs;

        if ( in_buf_size == 0 )
            break;

        if ( tinfl_decompress( &decomp, (const mz_uint8 *)buf, &in_buf_size, dict, dict + dict_ofs, &dst_buf_size, 0 ) == TINFL_STATUS_DONE )
        {
            os.write( reinterpret_cast<const char *>( dict + dict_ofs ), dst_buf_size );
        }
        else
        {
            result = false;
            break;
        }

        dict_ofs = ( dict_ofs + dst_buf_size ) & ( TINFL_LZ_DICT_SIZE - 1 );
    }
    free( dict );

    return result;
}

bool x::zip::decompression( const std::string & is, std::string & os )
{
    bool result = true;
    tinfl_decompressor decomp;
    size_t in_buf_ofs = 0, dict_ofs = 0;
    mz_uint8 * dict = (mz_uint8 *)malloc( TINFL_LZ_DICT_SIZE );

    tinfl_init( &decomp );

    while ( in_buf_ofs < is.size() )
    {
        size_t in_buf_size = is.size() - in_buf_ofs;
        size_t dst_buf_size = TINFL_LZ_DICT_SIZE - dict_ofs;

        if ( tinfl_decompress( &decomp, (const mz_uint8 *)is.c_str() + in_buf_ofs, &in_buf_size, dict, dict + dict_ofs, &dst_buf_size, 0) == TINFL_STATUS_DONE )
        {
            os.append( reinterpret_cast<const char *>( dict + dict_ofs ), dst_buf_size );
        }
        else
        {
            result = false;
            break;
        }

        in_buf_ofs += ( is.size() - in_buf_ofs ) - in_buf_size;
        dict_ofs = ( dict_ofs + dst_buf_size ) & ( TINFL_LZ_DICT_SIZE - 1 );
    }
    free( dict );

    return result;
}

bool x::zip::decompression( const std::string & is, std::ostream & os )
{
    bool result = true;
    tinfl_decompressor decomp;
    size_t in_buf_ofs = 0, dict_ofs = 0;
    mz_uint8 * dict = (mz_uint8 *)malloc( TINFL_LZ_DICT_SIZE );

    tinfl_init( &decomp );

    while ( in_buf_ofs < is.size() )
    {
        size_t in_buf_size = is.size() - in_buf_ofs;
        size_t dst_buf_size = TINFL_LZ_DICT_SIZE - dict_ofs;

        if ( tinfl_decompress( &decomp, (const mz_uint8 *)is.c_str() + in_buf_ofs, &in_buf_size, dict, dict + dict_ofs, &dst_buf_size, 0 ) == TINFL_STATUS_DONE )
        {
            os.write( reinterpret_cast<const char *>( dict + dict_ofs ), dst_buf_size );
        }
        else
        {
            result = false;
            break;
        }

        in_buf_ofs += in_buf_size;
        dict_ofs = ( dict_ofs + dst_buf_size ) & ( TINFL_LZ_DICT_SIZE - 1 );
    }
    free( dict );

    return result;
}

x::zip::zip()
    : _archive( new mz_zip_archive() )
{
    reset();
}

x::zip::zip( std::istream & stream )
    : zip()
{
    load( stream );
}

x::zip::zip( const std::filesystem::path & path )
    : zip()
{
    load( path );
}

x::zip::zip( const std::string & bytes )
    : zip()
{
    load( bytes );
}

x::zip::~zip()
{
    reset();

    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    delete archive_;
}

void x::zip::load( std::istream & stream )
{
    reset();

    while ( !stream.eof() )
    {
        _buffer.push_back( static_cast<char>( stream.get() ) );
    }

    remove_comment();
    start_read();
}

void x::zip::load( const std::filesystem::path & path )
{
    std::ifstream stream( path, std::ios::binary );
    load( stream );
}

void x::zip::load( const std::string & bytes )
{
    reset();
    _buffer.assign( bytes.begin(), bytes.end() );
    remove_comment();
    start_read();
}

void x::zip::save( const std::filesystem::path & path )
{
    std::ofstream stream( path, std::ios::binary );
    save( stream );
}

void x::zip::save( std::ostream & stream )
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    if ( archive_->m_zip_mode == MZ_ZIP_MODE_WRITING )
    {
        mz_zip_writer_finalize_archive( archive_ );
    }

    if ( archive_->m_zip_mode == MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED )
    {
        mz_zip_writer_end( archive_ );
    }

    if ( archive_->m_zip_mode == MZ_ZIP_MODE_INVALID )
    {
        start_read();
    }

    append_comment();

    stream.write( reinterpret_cast<const char *>( _buffer.data() ), _buffer.size() );
}

void x::zip::save( std::string & bytes )
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    if ( archive_->m_zip_mode == MZ_ZIP_MODE_WRITING )
    {
        mz_zip_writer_finalize_archive( archive_ );
    }

    if ( archive_->m_zip_mode == MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED )
    {
        mz_zip_writer_end( archive_ );
    }

    if ( archive_->m_zip_mode == MZ_ZIP_MODE_INVALID )
    {
        start_read();
    }

    append_comment();

    bytes.assign( _buffer.begin(), _buffer.end() );
}

void x::zip::reset()
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    switch ( archive_->m_zip_mode )
    {
    case MZ_ZIP_MODE_READING:
        mz_zip_reader_end( archive_ );
        break;
    case MZ_ZIP_MODE_WRITING:
        mz_zip_writer_finalize_archive( archive_ );
        mz_zip_writer_end( archive_ );
        break;
    case MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED:
        mz_zip_writer_end( archive_ );
        break;
    case MZ_ZIP_MODE_INVALID:
        break;
    }

    if ( archive_->m_zip_mode != MZ_ZIP_MODE_INVALID )
    {
        throw std::runtime_error( "" );
    }

    _buffer.clear();
    _comment.clear();

    start_write();
    mz_zip_writer_finalize_archive( archive_ );
    mz_zip_writer_end( archive_ );
}

bool x::zip::exist( const std::string & name )
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    if ( archive_->m_zip_mode != MZ_ZIP_MODE_READING )
    {
        start_read();
    }

    int index = mz_zip_reader_locate_file( archive_, name.c_str(), nullptr, 0 );

    return index != -1;
}

std::vector<std::string> x::zip::namelist()
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    if ( archive_->m_zip_mode != MZ_ZIP_MODE_READING )
    {
        start_read();
    }

    std::vector<std::string> names;

    for ( auto & info : infolist( archive_ ) )
    {
        names.push_back( info.filename );
    }

    return names;
}

std::size_t x::zip::file_size( const std::string & name )
{
    return getinfo( reinterpret_cast<mz_zip_archive *>( _archive ), name ).file_size;
}

std::size_t x::zip::compress_size( const std::string & name )
{
    return getinfo( reinterpret_cast<mz_zip_archive *>( _archive ), name ).compress_size;
}

void x::zip::extract( const std::filesystem::path & path, const std::string & name )
{
    std::fstream stream( path / name, std::ios::binary | std::ios::out );
    read( name, stream );
}

void x::zip::extractall( const std::filesystem::path & path )
{
    extractall( path, namelist() );
}

void x::zip::extractall( const std::filesystem::path & path, const std::vector<std::string> & namelist )
{
    for ( const auto & name : namelist )
    {
        extract( path, name );
    }
}

std::string x::zip::read( const std::string & name )
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    if ( archive_->m_zip_mode != MZ_ZIP_MODE_READING )
    {
        start_read();
    }

    auto info = getinfo( archive_, name );

    std::string result;
    mz_zip_reader_extract_file_to_callback( archive_, info.filename.c_str(), []( void * pOpaque, mz_uint64 file_ofs, const void * pBuf, size_t n ) -> size_t
    {
        auto result = (std::string *)pOpaque;
        result->append( reinterpret_cast<const char *>( pBuf ), n );
        return n;
    }, &result, 0 );

    return result;
}

x::uint64 x::zip::read( const std::string & name, std::ostream & stream )
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    if ( archive_->m_zip_mode != MZ_ZIP_MODE_READING )
    {
        start_read();
    }

    auto info = getinfo( archive_, name );

    auto pos = stream.tellp();

    mz_zip_reader_extract_file_to_callback( archive_, info.filename.c_str(), []( void * pOpaque, mz_uint64 file_ofs, const void * pBuf, size_t n ) -> size_t
    {
        auto stream = (std::ostream *)pOpaque;
        stream->write( reinterpret_cast<const char *>( pBuf ), n );
        return n;
    }, & stream, 0 );

    return stream.tellp() - pos;
}

void x::zip::write( const std::filesystem::path & path )
{
    write( path, path.filename().string() );
}

void x::zip::write( std::istream & istream, const std::string & name )
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    if ( archive_->m_zip_mode != MZ_ZIP_MODE_WRITING )
    {
        start_write();
    }

    char buf[4096];
    std::streamsize size = 0;

    while ( !istream.eof() )
    {
        size = istream.readsome( buf, 4096 );

        if ( !mz_zip_writer_add_mem( archive_, name.c_str(), buf, size, MZ_BEST_COMPRESSION ) )
        {
            throw std::runtime_error( "write error" );
        }
    }
}

void x::zip::write( const std::filesystem::path & path, const std::string & name )
{
    std::fstream file( path, std::ios::binary | std::ios::in );
    if ( file.is_open() )
    {
        write( file, name );
    }
}

void x::zip::write_str( const std::string & name, const std::string & bytes )
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    if ( archive_->m_zip_mode != MZ_ZIP_MODE_WRITING )
    {
        start_write();
    }

    if ( !mz_zip_writer_add_mem( archive_, name.c_str(), bytes.data(), bytes.size(), MZ_BEST_COMPRESSION ) )
    {
        throw std::runtime_error( "write error" );
    }
}

void x::zip::start_read()
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    if ( archive_->m_zip_mode == MZ_ZIP_MODE_READING )
        return;

    if ( archive_->m_zip_mode == MZ_ZIP_MODE_WRITING )
    {
        mz_zip_writer_finalize_archive( archive_ );
    }

    if ( archive_->m_zip_mode == MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED )
    {
        mz_zip_writer_end( archive_ );
    }

    if ( !mz_zip_reader_init_mem( archive_, _buffer.data(), _buffer.size(), 0 ) )
    {
        throw std::runtime_error( "bad zip" );
    }
}

void x::zip::start_write()
{
    auto archive_ = reinterpret_cast<mz_zip_archive *>( _archive );

    if ( archive_->m_zip_mode == MZ_ZIP_MODE_WRITING )
        return;

    switch ( archive_->m_zip_mode )
    {
    case MZ_ZIP_MODE_READING:
    {
        mz_zip_archive archive_copy;
        std::memset( &archive_copy, 0, sizeof( mz_zip_archive ) );
        std::vector<char> buffer_copy( _buffer.begin(), _buffer.end() );

        if ( !mz_zip_reader_init_mem( &archive_copy, buffer_copy.data(), buffer_copy.size(), 0 ) )
        {
            throw std::runtime_error( "bad zip" );
        }

        mz_zip_reader_end( archive_ );

        archive_->m_pWrite = &detail::write_callback;
        archive_->m_pIO_opaque = &_buffer;
        _buffer.clear();

        if ( !mz_zip_writer_init( archive_, 0 ) )
        {
            throw std::runtime_error( "bad zip" );
        }

        for ( unsigned int i = 0; i < static_cast<unsigned int>( archive_copy.m_total_files ); i++ )
        {
            if ( !mz_zip_writer_add_from_zip_reader( archive_, &archive_copy, i ) )
            {
                throw std::runtime_error( "fail" );
            }
        }

        mz_zip_reader_end( &archive_copy );
    }
    break;
    case MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED:
        mz_zip_writer_end( archive_ );
    case MZ_ZIP_MODE_INVALID:
    case MZ_ZIP_MODE_WRITING:
        archive_->m_pWrite = &detail::write_callback;
        archive_->m_pIO_opaque = &_buffer;

        if ( !mz_zip_writer_init( archive_, 0 ) )
        {
            throw std::runtime_error( "bad zip" );
        }
        break;
    }
}

void x::zip::append_comment()
{
    if ( !_comment.empty() )
    {
        auto comment_length = std::min( static_cast<uint16_t>( _comment.size() ), std::numeric_limits<uint16_t>::max() );
        _buffer[_buffer.size() - 2] = static_cast<char>( comment_length );
        _buffer[_buffer.size() - 1] = static_cast<char>( comment_length >> 8 );
        std::copy( _comment.begin(), _comment.end(), std::back_inserter( _buffer ) );
    }
}

void x::zip::remove_comment()
{
    if ( _buffer.empty() )
        return;

    std::size_t position = _buffer.size() - 1;

    for ( ; position >= 3; position-- )
    {
        if ( _buffer[position - 3] == static_cast<char>( 'P' ) &&
             _buffer[position - 2] == static_cast<char>( 'K' ) &&
             _buffer[position - 1] == static_cast<char>( '\x05' ) &&
             _buffer[position] == static_cast<char>( '\x06' ) )
        {
            position = position + 17;
            break;
        }
    }

    if ( position == 3 )
    {
        throw std::runtime_error( "didn't find end of central directory signature" );
    }

    uint16_t length = static_cast<uint16_t>( _buffer[position + 1] );
    length = static_cast<uint16_t>( length << 8 ) + static_cast<uint16_t>( _buffer[position] );
    position += 2;

    if ( length != 0 )
    {
        _comment = { _buffer.data() + position, _buffer.data() + position + length };
        _buffer.resize( _buffer.size() - length );
        _buffer[_buffer.size() - 1] = static_cast<char>( 0 );
        _buffer[_buffer.size() - 2] = static_cast<char>( 0 );
    }
}
