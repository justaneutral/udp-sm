
#ifndef __EF_VI_JUMBO_RX_H__
#define __EF_VI_JUMBO_RX_H__


extern "C" {
#include <etherfabric/vi.h>
#include <etherfabric/pd.h>
#include <etherfabric/memreg.h>
}
#include "defines.h"
#include "flags.h"
#include "utils.h"
#ifdef __CONTROLABLE__
#include "Tsc.h"
#endif // __CONTROLABLE__
#include "NetInterface.h"
#include "RingFIFOBuffer.h"
#include "CommonData.h"
#include "Logger.h"
#include "NetworkReceiverInterface.h"

/******************************************************************************/
#define PKT_POOL_SIZE        1 << 15
#define DEST_IPADDR          "239.2.3.1"
#define DEST_PORT            21001
#define REFILL_BATCH_SIZE    16
#define MAX_PKT_FRAGS        7
#define JUMBO_SIZE           9000
#define UDP_HEADER_LEN       42

/*
 * Hardware delivers at most ef_vi_receive_buffer_len() bytes to each
 * buffer (default 1792), and for best performance buffers should be
 * aligned on a 64-byte boundary.  Also, RX DMA will not cross a 4K
 * boundary.  The I/O address space may be discontiguous at 4K boundaries.
 * So easiest thing to do is to make buffers always be 2K in size.
 */
#define PKT_BUF_SIZE         2048

/*
 * Align address where data is delivered onto EF_VI_DMA_ALIGN boundary,
 * because that gives best performance.
 */
#define RX_DMA_OFF           ROUND_UP(sizeof(PacketBuffer_t), EF_VI_DMA_ALIGN)

/*
 * A packet buffer is a memory allocations on the host which the card will
 * read from when sending packets, or write to when receiving packets.
 *
 * The packet buffer structure consists of:
 *     ef_addr    I/O address corresponding to the start
 *                of this PacketBuffer_t struct
 *     rx_ptr     pointer to where received packets start
 *     id         packet buffer ID
 *     next       pointer to next buffer
 */
typedef struct PacketBufferStruct_t {
  ef_addr            efaddr;
  void*              rx_ptr;
  int                id;
  struct PacketBufferStruct_t* next;
} PacketBuffer_t;

/*
 * Each packet buffer can hold up to (2048 - rx_prefix_len) bytes. A single
 * jumbo frame will be split across multiple packet buffers, so we aggregate
 * all packet data into a single region. Additional buffer ID, fragment
 * lengths and number of fragments are for reference (are not necessary).
 *
 * The jumbo frame structure consists of:
 *     in_pkt         Flag indicating processing frame
 *     buf_ids        Array of UIDs of each originating packet buffer buffer
 *     frag_data_len  Array of number of bytes payload in each fragment
 *     n_frags        Total number of fragments which contributed to this frame
 *     payload_bytes  Total number of bytes payload data in this frame
 */
typedef struct jumbo {
  int in_pkt;
  int buf_ids[MAX_PKT_FRAGS];
  unsigned int frag_data_len[MAX_PKT_FRAGS];
  unsigned int n_frags;
  unsigned int payload_bytes;
  char data[JUMBO_SIZE];
}jumbo_t;

/*
 * A set of packet buffers consists of a memory region partitioned up into a
 * pool of packet buffers. Memory is first registered with the NIC and then
 * arranged into packet buffers.
 *
 * The struct of packet buffers consists of
 *     mem            a pointer to the memory region
 *     ef_memreg      memory registered for DMA
 *     mem_size       size of the memory region
 *     num            number of packet buffers allocated
 *     PacketBuffer_t pool of free packet buffers
 *     free_pool_n    number of buffers in free pool
 */
struct PktBuffersStruct_t {
  void* mem;
  struct ef_memreg memreg;
  size_t mem_size;
  int num;
  PacketBuffer_t* free_pool;
  int free_pool_n;
};

/*
 * There are 3 stages in setting up a VI:
 *     1) get a driver handle
 *     2) allocate a protection domain
 *     3) create an instance of a VI
 * Options can be set for the VI, and per-VI stats can also be maintained.
 *
 * The struct consists of
 *     dh              a driver handle
 *     pd              a protected domain
 *     vi              a VI instance
 *     pkt_mem         memory region being used for packet buffers
 *     fc              a cookie for identifying an installed filter
 *     rx_prefix_len   the length of the meta-data prefix region in
 *                     a packet buffer
 *     max_fill_level  The amount to fill the ring up to
 *
 */
typedef struct VirtualInterfaceResourceStruct_t {
  ef_driver_handle   dh;
  struct ef_pd       pd;
  struct ef_vi       vi;
  struct PktBuffersStruct_t pkt_mem;
  ef_filter_cookie   fc;
  int                rx_prefix_len;
  int                max_fill_level;
} VirtualInterfaceResource_t;

/*
 * Configuration options:
 *     NetworkInterfaceName         interface to receive packets on. No default
 *     ip            Destination IP. Default is 239.100.10.1
 *     port          Destination port. Default is 65456
 *     Verbose       toggle additional logging if this is set.
 *                   Default is to disable this
 *     ConfigDumpReceivedPacketsInAsciiFormat      Boolean flag. If set, write out packet payload as text
 *     ConfigDumpReceivedPacketsInHexadecimalFormat      Boolean flag. If set, write out packet payload as hex
 *
 */
typedef struct CfgOptsStruct_t {
  char NetworkInterfaceName[IFNAMSIZ];
  int IPv4MulticastPort;
  char IPv4MulticastAddress[INET_ADDRSTRLEN];
  char ip_local[INET_ADDRSTRLEN];
  int ConfigDumpReceivedPacketsInAsciiFormat;
  int ConfigDumpReceivedPacketsInHexadecimalFormat;
} CfgOpts_t;


///////////////////class definition ////////////
class NetworkReceiverImplementationEFVI : public NetworkReceiverInterface {
public:
  NetworkReceiverImplementationEFVI(void);
  ~NetworkReceiverImplementationEFVI(void);
  int SetDefaults(void);
  int Initialize(CommonData *CommonDataPtr, Logger *LoggerPtr, Tsc *TscPtr, RingFIFOBuffer *RingBufferPtr, char *NetworkInterfaceName, int port, char *IPv4MulticastAddress, int ConfigDumpReceivedPacketsInHexadecimalFormat, int ConfigDumpReceivedPacketsInAsciiFormat);
  void Iterate(void);
  int Uninitialize(void);
private:
  int IterateInternalEFVI(void);
  int HandleRxEventDiscard(ef_event* ev);
  inline int HandleRxEventAccept(ef_event* ev);
  void RefillRxRing(void);
  int AddFilter(void);
  int InitPktsMemory(void);
  int ConfigBuffer(VirtualInterfaceResource_t* vr, int BufferNumber);
  int InitVi(void);
//#ifdef __CONTROLABLE__
  void DumpJumboFrameInfo(jumbo_t* j, CfgOpts_t* cfg);
  void DumpJumboFrameInHexadecimalFormat(const void* pv, int len);
  void DumpJumboFramePayloadInTextAsciiFormat(jumbo_t* j);
  inline PacketBuffer_t* GetPktBufPtrFromId(int pkt_buf_i);
  inline void PktBufFree(PacketBuffer_t* pkt_buf);
//#endif // __CONTROLABLE__a
  int Counter;
  int ReturnValue;
  int Initialized;
  int EfViDriverIsOpen;
  int EfPdIsAllocated;
  int EfViIsAllocated;
  int EfMemIsAllocated;
public:
  CfgOpts_t cfg;
private:
  VirtualInterfaceResource_t VirtualResourceINstance;
  jumbo_t jumbo;
  NetInterface *NetworkInterfacePtr;
  Logger *LoggerPtr;
  CommonData *CommonDataPtr;
  RingFIFOBuffer *ringbuf;
  Tsc *TscPtr;
};

#endif // __EF_VI_JUMBO_RX_H__
// _eof_

