#ifndef __NENX_SHM_PUBLISHER_H__
#define __NENX_SHM_PUBLISHER_H__
#ifdef __ATOMIC__
//#define __SEQ_CST__
#undef __SEQ_CST__
#include <new>
#include <atomic>
#endif
#include "RingFIFOBuffer.h"
#include "CommonData.h"
//#include "common.h"
#include "InserterInterface.h"
#include "InserterImplementation.h"
#include "utils.h"
#include "ParserImplementation.h"
#include "NetworkReceiverImplementationEFVI.h"
#ifdef __CONTROLABLE__
#include "Tsc.h"
#endif //__CONTROLABLE__
#ifndef __SEQUENCE_NUMBER_LOCAL__
#ifdef __SEQUENCE_NUMBER_QUEUE_SEQUENCE__
typedef struct stripped_message_header_struct_t {
  uint8_t  header_length;
  uint8_t  header_version;
  uint16_t message_id;
  uint32_t queue_sequence;
  uint64_t source_id;
  uint64_t queue_id;
  uint64_t time_stamp;
  uint8_t  payload;
} __attribute__((packed)) stripped_message_header_s_t, *pstripped_message_header_s_t;
#endif
#endif
//#define __CACHE_LINE_SIZE__  (64)
#ifdef __huge_pages__
#define __DO_CHOWN__
#define __HUGE_PAGE_SIZE__    (1ll * 1024 * 1024 * 1024)
//#define __HUGE_PAGE_SIZE__    (2ll * 1024 * 1024 * 1024)
#define __BLOCK_PAGE_SIZE__    (2ll * 1024 * 1024)
//#define __SHM_NAME_LENGTH__    (256)
//#undef __USE_SHM__
#ifdef __USE_SHM__
//#define SHM_NAME "/dev/shm/ef_vi_2mb.mmap"
#else
//#define SHM_NAME "/dev/hugepages/ef_vi_2mb.mmap"
//#define SHM_NAME "/dev/hugepages/ef_vi_2mb0.mmap"
#endif
//#define __num_data_pages__    (1024) - overflows in java tester code
#define __num_data_pages__    (512)
#else // __huge_pages__
#define  __HUGE_PAGE_SIZE__    (2ll * 1024 * 1024)
#define __BLOCK_PAGE_SIZE__    (4ll * 1024)
//#undef __USE_SHM__
#ifdef __USE_SHM__
//#define SHM_NAME "ef_vi.mmap"
#else
//#define SHM_NAME "/tmp/mmap/ef_vi_shared.mmap"
#endif
#define __num_data_pages__        (262144)
#endif
//#define __num_data_pages__  (__HUGE_PAGE_SIZE__/__BLOCK_PAGE_SIZE__)
#define __total_number_of_data_cache_lines__  ((__BLOCK_PAGE_SIZE__) * (__num_data_pages__) / (__CACHE_LINE_SIZE__))
// shared memory block
//  file control block
//    file_configuration_information
#define file_marker_position  (0ll) // starting at first page
#define file_marker_size  (sizeof(uint32_t))
#define file_version_position  (file_marker_position + file_marker_size)
#define file_version_size  (sizeof(int32_t))
#define layout_flags_position  (file_version_position + file_version_size)
#define layout_flags_size  (sizeof(int32_t))
#define page_size_position  (layout_flags_position + layout_flags_size)
#define page_size_size    (sizeof(uint32_t))
#define line_size_position  (page_size_position + page_size_size)
#define line_size_size    (sizeof(uint32_t))
#define data_size_position  (line_size_position + line_size_size)
#define data_size_size    (sizeof(uint32_t))
#define data_offset_position  (data_size_position + data_size_size)
#define data_offset_size  (sizeof(uint32_t))
//    writer_state_information - cache line aligned
#define writer_heartbeat_time_position  (((data_offset_position + data_offset_size + __CACHE_LINE_SIZE__ - 1) / __CACHE_LINE_SIZE__) * __CACHE_LINE_SIZE__)
#define writer_heartbeat_time_size  (sizeof(int64_t))
#define writer_session_id_position  (writer_heartbeat_time_position + writer_heartbeat_time_size)
#define writer_session_id_size    (sizeof(uint64_t))
#define writer_max_sequence_position  (writer_session_id_position + writer_session_id_size)
#define writer_max_sequence_size  (sizeof(int64_t))
#define writer_flags_position    (writer_max_sequence_position + writer_max_sequence_size)
#define writer_flags_size    (sizeof(uint32_t))
enum __writer_flags__ {
  writer_flags_connected  = 0x01,  // - writer is connected to its own data source
  writer_flags_in_session  = 0x02, // - writer is currently in session
  writer_flags_shutdown  = 0x04, // - writer’s current session has been shutdown (no further sequence numbers are being inserted)
  writer_flags_lagged  = 0x08  // - writer’s data source is lagged
};

#define commit_sequence_position  (((writer_flags_position + writer_flags_size + __CACHE_LINE_SIZE__ - 1) / __CACHE_LINE_SIZE__) * __CACHE_LINE_SIZE__) //cache line aligned
#define commit_sequence_size    (sizeof(uint64_t))
#define reserve_sequence_position  (((commit_sequence_position + commit_sequence_size + __CACHE_LINE_SIZE__ - 1) / __CACHE_LINE_SIZE__) * __CACHE_LINE_SIZE__) //cache line aligned
#define reserve_sequence_size    (sizeof(uint64_t))
//  data section  = page aligned
#define data_position      (((reserve_sequence_position + reserve_sequence_size + __BLOCK_PAGE_SIZE__ -1) / __BLOCK_PAGE_SIZE__) * __BLOCK_PAGE_SIZE__)
#define TOTAL_DATA_SIZE      (__BLOCK_PAGE_SIZE__ * __num_data_pages__)
#ifndef __x86_64__
  #error Current code for x86_64 only
#endif
//////////////////////////diagnostics////////////////////////////////////1145258561/////////////////////////////////////////////////
#define PrintUnsigned32BitValue(q) printf("\t%s\t= %llu\t(0x%08llX)\t[%016lX]\n",#q,q,q,ntoh64(*(uint64_t *)((uint8_t *)shared_memory_buffer_ptr + q)));
#define PrintUnsigned64BitValue(q) printf("\t%s\t= %llu\t(0x%016llX)\t[%016lX]\n",#q,q,q,ntoh64(*(uint64_t *)((uint8_t *)shared_memory_buffer_ptr + q)));

#if(0)
#define write_file_version(v) WriteMemory<int32_t>((shared_memory_buffer_ptr), (file_version_position), (v))
#define read_file_version() ReadMemory<uint32_t>((uint32_t *)(shared_memory_buffer_ptr), (file_version_position))
#define write_layout_flags(v) WriteMemory<int32_t>((shared_memory_buffer_ptr), (layout_flags_position), (v))
#define read_layout_flags() ReadMemory<uint32_t>((uint32_t *)(shared_memory_buffer_ptr), (layout_flags_position))
#define write_page_size(v) WriteMemory<uint32_t>((shared_memory_buffer_ptr), (page_size_position), (v))
#define read_page_size() ReadMemory<uint32_t>((shared_memory_buffer_ptr), (page_size_position))
#define write_line_size(v) WriteMemory<uint32_t>((shared_memory_buffer_ptr), (line_size_position), (v))
#define read_line_size() ReadMemory<uint32_t>((shared_memory_buffer_ptr), (line_size_position))
#define write_data_size(v) WriteMemory<uint32_t>((shared_memory_buffer_ptr), (data_size_position), (v))
#define read_data_size() ReadMemory<uint32_t>((shared_memory_buffer_ptr), (data_size_position))
#define write_data_offset(v) WriteMemory<uint64_t>((shared_memory_buffer_ptr), (data_offset_position), (v))
#define read_data_offset() ReadMemory<uint64_t>((shared_memory_buffer_ptr), (data_offset_position))
//              writer_state_information - cache line aligned
#define write_writer_heartbeat_time(v) WriteMemory<uint64_t>((shared_memory_buffer_ptr), (writer_heartbeat_time_position), (v))
#define read_writer_heartbeat_time() ReadMemory<uint64_t>((shared_memory_buffer_ptr), (writer_heartbeat_time_position))
#define write_writer_session_id(v) WriteMemory<uint64_t>((shared_memory_buffer_ptr), (writer_session_id_position), ntoh64(v))
#define read_writer_session_id() hton64(ReadMemory<uint64_t>((shared_memory_buffer_ptr), (writer_session_id_position)))
#define write_writer_max_sequence(v) WriteMemory<uint64_t>((shared_memory_buffer_ptr), (writer_max_sequence_position), (v))
#define read_writer_max_sequence() ReadMemory<uint64_t>((shared_memory_buffer_ptr), (writer_max_sequence_position))
#define write_writer_flags(v) WriteMemory<uint32_t>((shared_memory_buffer_ptr), (writer_flags_position), (v))
#define read_writer_flags() ReadMemory<uint32_t>((shared_memory_buffer_ptr), (writer_flags_position))
#define write_commit_sequence(v) WriteMemory<uint64_t>((shared_memory_buffer_ptr), (commit_sequence_position), (v))
#define read_commit_sequence() ReadMemory<uint64_t>((shared_memory_buffer_ptr), (commit_sequence_position))
#define write_reserve_sequence(v) WriteMemory<uint64_t>((shared_memory_buffer_ptr), (reserve_sequence_position), (v))
#define read_reserve_sequence() ReadMemory<uint64_t>((shared_memory_buffer_ptr), (reserve_sequence_position))
#else //(0)
#define write_file_version(v) WriteMemory<int32_t>((file_version_position), (v))
#define read_file_version() ReadMemory<uint32_t>((file_version_position))
#define write_layout_flags(v) WriteMemory<int32_t>((layout_flags_position), (v))
#define read_layout_flags() ReadMemory<uint32_t>((layout_flags_position))
#define write_page_size(v) WriteMemory<uint32_t>((page_size_position), (v))
#define read_page_size() ReadMemory<uint32_t>((page_size_position))
#define write_line_size(v) WriteMemory<uint32_t>((line_size_position), (v))
#define read_line_size() ReadMemory<uint32_t>((line_size_position))
#define write_data_size(v) WriteMemory<uint32_t>((data_size_position), (v))
#define read_data_size() ReadMemory<uint32_t>((data_size_position))
#define write_data_offset(v) WriteMemory<uint64_t>((data_offset_position), (v))
#define read_data_offset() ReadMemory<uint64_t>((data_offset_position))
//              writer_state_information - cache line aligned
#define write_writer_heartbeat_time(v) WriteMemory<uint64_t>((writer_heartbeat_time_position), (v))
#define read_writer_heartbeat_time() ReadMemory<uint64_t>((writer_heartbeat_time_position))
#define write_writer_session_id(v) WriteMemory<uint64_t>((writer_session_id_position), ntoh64(v))
#define read_writer_session_id() hton64(ReadMemory<uint64_t>((writer_session_id_position)))
#define write_writer_max_sequence(v) WriteMemory<uint64_t>((writer_max_sequence_position), (v))
#define read_writer_max_sequence() ReadMemory<uint64_t>((writer_max_sequence_position))
#define write_writer_flags(v) WriteMemory<uint32_t>((writer_flags_position), (v))
#define read_writer_flags() ReadMemory<uint32_t>((writer_flags_position))
#define write_commit_sequence(v) WriteMemory<uint64_t>((commit_sequence_position), (v))
#define read_commit_sequence() ReadMemory<uint64_t>((commit_sequence_position))
#define write_reserve_sequence(v) WriteMemory<uint64_t>((reserve_sequence_position), (v))
#define read_reserve_sequence() ReadMemory<uint64_t>((reserve_sequence_position))
#endif //(0)

void signal_handler(int signal_number);
char *ParseWriterFlags(uint32_t v);

////////////////////////////  MAIN SECTION  /////////////////////////////////

int main(int argc, char *argv[]);
#endif // __NENX_SHM_PUBLISHER_H__
// eof
