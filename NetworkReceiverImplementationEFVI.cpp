
#include "NetworkReceiverImplementationEFVI.h"

//#ifdef __CONTROLABLE__
//static void DumpJumboFramePayloadInTextAsciiFormat(jumbo_t* JumboFrameBufferPtr)
void NetworkReceiverImplementationEFVI::DumpJumboFramePayloadInTextAsciiFormat(jumbo_t* JumboFrameBufferPtr) {
  unsigned int i;
  LogInfo("payload (text format):\n");
  for (i = 0; i < JumboFrameBufferPtr->payload_bytes; i++) {
    LogInfo("%c", JumboFrameBufferPtr->data[i]);
  }
  LogInfo("\n");
  return;
}


//static void DumpJumboFrameInHexadecimalFormat(const void* BufferPtr, int Length)
void NetworkReceiverImplementationEFVI::DumpJumboFrameInHexadecimalFormat(const void* BufferPtr, int Length) {
  const unsigned char* p = (const unsigned char*) BufferPtr;
  int i;
  for (i = 0; i < Length; ++i) {
    const char* eos;
    switch (i & 15) {
    case 0:
      LogInfo("%08x  ", i);
      eos = "";
      break;
    case 1:
      eos = " ";
      break;
    case 15:
      eos = "\n";
      break;
    default:
      eos = (i & 1) ? " " : "";
      break;
    }
    LogInfo("%02x%s", (unsigned) p[i], eos);
  }
  LogInfo("\n");
}

//static void DumpJumboFrameInfo(jumbo_t* JumboFrameBufferPtr, CfgOpts_t* cfg)
void NetworkReceiverImplementationEFVI::DumpJumboFrameInfo(jumbo_t* JumboFrameBufferPtr, CfgOpts_t* cfg) {
  unsigned int i;
  LogInfo("----------------\n");
  LogInfo("Jumbo frame info\n");
  LogInfo("\tPacket received: n_frags=%d, bytes=%d\n", JumboFrameBufferPtr->n_frags, JumboFrameBufferPtr->payload_bytes);
  LogInfo("\tBuffer IDs: buf_ids=");
  for (i = 0; i <= JumboFrameBufferPtr->n_frags; i++) {
    LogInfo("%d ", JumboFrameBufferPtr->buf_ids[i]);
  }
  LogInfo("\n\tFragment lengths: ");
  for (i = 0; i <= JumboFrameBufferPtr->n_frags; i++) {
    LogInfo("%d ", JumboFrameBufferPtr->frag_data_len[i]);
  }
  LogInfo("\n");
  if (cfg->ConfigDumpReceivedPacketsInAsciiFormat) {
    DumpJumboFramePayloadInTextAsciiFormat(JumboFrameBufferPtr);
  }
  if (cfg->ConfigDumpReceivedPacketsInHexadecimalFormat) {
    DumpJumboFrameInHexadecimalFormat(JumboFrameBufferPtr->data, JumboFrameBufferPtr->payload_bytes);
  }
  LogInfo("----------------\n");
}
//#endif // __CONTROLABLE__
/******************************************************************************/
/* initialisation functions */

int NetworkReceiverImplementationEFVI::InitVi(void) {
  //int ViFlags, RetValLocal = 0;
  int RetValLocal = 0;
  enum ef_vi_flags ViFlags;
  VirtualInterfaceResource_t* VirtualResourceLocalPtr = &VirtualResourceINstance;
  CfgOpts_t* ConfigurationLocalPtr = &cfg;

  //TRY(ef_driver_open(&VirtualResourceLocalPtr->dh));
  if (0 == EfViDriverIsOpen) {
    RetValLocal = ef_driver_open(&VirtualResourceLocalPtr->dh);
    EfViDriverIsOpen = 1;
  }
  //TRY(ef_pd_alloc_by_name(&VirtualResourceLocalPtr->pd, VirtualResourceLocalPtr->dh, co->NetworkInterfaceName, EF_PD_DEFAULT));
  if (EfViDriverIsOpen && (0 == EfPdIsAllocated) && (0 == (RetValLocal = ef_pd_alloc_by_name(&VirtualResourceLocalPtr->pd, VirtualResourceLocalPtr->dh, ConfigurationLocalPtr->NetworkInterfaceName, EF_PD_DEFAULT)))) {
    EfPdIsAllocated = 1;
    ViFlags = EF_VI_FLAGS_DEFAULT;
    //TRY(ef_vi_alloc_from_pd(&VirtualResourceLocalPtr->vi, VirtualResourceLocalPtr->dh, &VirtualResourceLocalPtr->pd, VirtualResourceLocalPtr->dh, -1, -1, 0, NULL, -1, ViFlags));
    if (EfPdIsAllocated && (0 == EfViIsAllocated) && (0 == (RetValLocal =  ef_vi_alloc_from_pd(&VirtualResourceLocalPtr->vi, VirtualResourceLocalPtr->dh, &VirtualResourceLocalPtr->pd, VirtualResourceLocalPtr->dh, -1, -1, 0, NULL, -1, ViFlags)))) {
      EfViIsAllocated = 1;
      VirtualResourceLocalPtr->rx_prefix_len = ef_vi_receive_prefix_len(&VirtualResourceLocalPtr->vi);
      VirtualResourceLocalPtr->max_fill_level = ef_vi_receive_capacity(&VirtualResourceLocalPtr->vi);
    }
  }
  if (EfViIsAllocated == 0) {
    RetValLocal |= -1;
  }
  return RetValLocal;
}



//static int ConfigBuffer(VirtualInterfaceResource_t* VirtualResourceLocalPtr, int BufferNumber)
int NetworkReceiverImplementationEFVI::ConfigBuffer(VirtualInterfaceResource_t* VirtualResourceLocalPtr, int BufferNumber) {
  int rv = 0;
  PacketBuffer_t* pkt_buf;
   // derive the packet buffer pointer from the provided buffer ID
  //assert
  //LOGV("BufferNumber = %x, VirtualResourceLocalPtr->pkt_mem.num = %x\n", BufferNumber,  (unsigned) VirtualResourceLocalPtr->pkt_mem.num); //pddbg
  if ((unsigned) BufferNumber < (unsigned) VirtualResourceLocalPtr->pkt_mem.num) {
    pkt_buf = (PacketBuffer_t*)((char*) VirtualResourceLocalPtr->pkt_mem.mem + (size_t) BufferNumber * PKT_BUF_SIZE);
    pkt_buf->rx_ptr = (char*) pkt_buf + RX_DMA_OFF + VirtualResourceLocalPtr->rx_prefix_len;
    pkt_buf->id = BufferNumber;
    pkt_buf->efaddr = ef_memreg_dma_addr(&VirtualResourceLocalPtr->pkt_mem.memreg, BufferNumber * PKT_BUF_SIZE);
    // update pointer to free buffers
    pkt_buf->next = VirtualResourceLocalPtr->pkt_mem.free_pool;
    VirtualResourceLocalPtr->pkt_mem.free_pool = pkt_buf;
    ++(VirtualResourceLocalPtr->pkt_mem.free_pool_n);
  } else {
    rv = -1;
  }
  return rv;
}


// Packet memory is ensured to be correctly aligned, and is allocated. The memory is partitioned up into a list of packet buffers.
int NetworkReceiverImplementationEFVI::InitPktsMemory(void) {
  int RetValLocal = -1, i;
  VirtualInterfaceResource_t* VirtualResourceLocalPtr = &VirtualResourceINstance;
  VirtualResourceLocalPtr->pkt_mem.num = PKT_POOL_SIZE;
  VirtualResourceLocalPtr->pkt_mem.mem_size = VirtualResourceLocalPtr->pkt_mem.num * PKT_BUF_SIZE;
  VirtualResourceLocalPtr->pkt_mem.mem_size = ROUND_UP(VirtualResourceLocalPtr->pkt_mem.mem_size, __ram_buffer_page_size__);
  // register the memory so the NIC can access it. This is registered against the protected domain
  //TRY(posix_memalign(&VirtualResourceLocalPtr->pkt_mem.mem, __ram_buffer_page_size__, VirtualResourceLocalPtr->pkt_mem.mem_size));
  if (EfViIsAllocated && (0 == EfMemIsAllocated)  &&  (0 == (RetValLocal =  posix_memalign(&VirtualResourceLocalPtr->pkt_mem.mem, __ram_buffer_page_size__, VirtualResourceLocalPtr->pkt_mem.mem_size)))) {
    ef_memreg_alloc(&VirtualResourceLocalPtr->pkt_mem.memreg, VirtualResourceLocalPtr->dh, &VirtualResourceLocalPtr->pd, VirtualResourceLocalPtr->dh, VirtualResourceLocalPtr->pkt_mem.mem, VirtualResourceLocalPtr->pkt_mem.mem_size);
    EfMemIsAllocated = 1;
    memset(VirtualResourceLocalPtr->pkt_mem.mem, 0, VirtualResourceLocalPtr->pkt_mem.mem_size);
    // configure packet buffers 
    for (i = 0; (RetValLocal == 0) && (i < VirtualResourceLocalPtr->pkt_mem.num); ++i) {
      RetValLocal = ConfigBuffer(VirtualResourceLocalPtr, i);
    }
  }
  return RetValLocal;
}


 // Add the necessary filter to the NIC filter table. In this case, the app listens on a specified address.
int NetworkReceiverImplementationEFVI::AddFilter(void) {
  int RetValLocal = 0;
  VirtualInterfaceResource_t* VirtualResourceLocalPtr = &VirtualResourceINstance;
  CfgOpts_t* ConfigurationLocalPtr = &cfg;
  ef_filter_spec fs;

  struct sockaddr_in daddr;

  inet_pton(AF_INET, ConfigurationLocalPtr->IPv4MulticastAddress, &(daddr.sin_addr));
  daddr.sin_family = AF_INET;
  daddr.sin_port = htons(ConfigurationLocalPtr->IPv4MulticastPort);

  ef_filter_spec_init(&fs, EF_FILTER_FLAG_NONE);
  if (0 == (RetValLocal = ef_filter_spec_set_ip4_local(&fs, IPPROTO_UDP, daddr.sin_addr.s_addr, daddr.sin_port))) {
    RetValLocal = ef_vi_filter_add(&VirtualResourceLocalPtr->vi, VirtualResourceLocalPtr->dh, &fs, &(VirtualResourceLocalPtr->fc));
  }
  return RetValLocal;
}


// helper functions
void NetworkReceiverImplementationEFVI::RefillRxRing(void) {
  VirtualInterfaceResource_t* VirtualResourceLocalPtr = &VirtualResourceINstance;
  PacketBuffer_t* pkt_buf;
  int i;
  int refill_level = VirtualResourceLocalPtr->max_fill_level - REFILL_BATCH_SIZE;
  if (ef_vi_receive_space(&VirtualResourceLocalPtr->vi) < REFILL_BATCH_SIZE)
    return;
  do {
    for (i = 0; i < REFILL_BATCH_SIZE; ++i) {
      pkt_buf = VirtualResourceLocalPtr->pkt_mem.free_pool;
      VirtualResourceLocalPtr->pkt_mem.free_pool = VirtualResourceLocalPtr->pkt_mem.free_pool->next;
      --(VirtualResourceLocalPtr->pkt_mem.free_pool_n);
      ef_vi_receive_init(&VirtualResourceLocalPtr->vi, pkt_buf->efaddr + RX_DMA_OFF, pkt_buf->id);
    }
  } while (ef_vi_receive_fill_level(&VirtualResourceLocalPtr->vi) < refill_level);
  ef_vi_receive_push(&VirtualResourceLocalPtr->vi);
}


inline PacketBuffer_t* NetworkReceiverImplementationEFVI::GetPktBufPtrFromId(int pkt_buf_i) {
  VirtualInterfaceResource_t* VirtualResourceLocalPtr = &VirtualResourceINstance;
  //assert(
  if ((unsigned) pkt_buf_i < (unsigned) VirtualResourceLocalPtr->pkt_mem.num) {
    return (PacketBuffer_t *) ((char*) VirtualResourceLocalPtr->pkt_mem.mem + (size_t) pkt_buf_i * PKT_BUF_SIZE);
  } else {
    return (PacketBuffer_t *)NULL;
  }
}


inline void NetworkReceiverImplementationEFVI::PktBufFree(PacketBuffer_t* pkt_buf) {
  VirtualInterfaceResource_t* VirtualResourceLocalPtr = &VirtualResourceINstance;
  pkt_buf->next = VirtualResourceLocalPtr->pkt_mem.free_pool;
  VirtualResourceLocalPtr->pkt_mem.free_pool = pkt_buf;
  ++(VirtualResourceLocalPtr->pkt_mem.free_pool_n);
}

// event and packet handling

inline int NetworkReceiverImplementationEFVI::HandleRxEventAccept(ef_event* ev) {
  int rv = 0;
  PacketBuffer_t* pkt_buf = ( PacketBuffer_t*)NULL;
  int pkt_buf_id = EF_EVENT_RX_RQ_ID(*ev);
  jumbo_t* JumboFrameBufferLocalPtr = &jumbo;
  int is_start = EF_EVENT_RX_SOP(*ev);
  int is_cont = EF_EVENT_RX_CONT(*ev);
  int bytes = EF_EVENT_RX_BYTES(*ev) - VirtualResourceINstance.rx_prefix_len;
#ifdef __CONTROLABLE__
  if (TscPtr && (CommonDataPtr->ConfigurationFlags & _FLAG_GET_TIMING_)) {
    TscPtr->BenchStart();
  }
#endif //__CONTROLABLE__
  pkt_buf = GetPktBufPtrFromId(pkt_buf_id);
  if(pkt_buf) {
//#ifdef __CONTROLABLE__
    if (CommonDataPtr->ConfigurationFlags & _FLAG_VERBOSE_) {
      LogInfo("RX Handling: rx flags=%d, is_start=%d, is_cont=%d\n", ev->rx.flags, is_start, is_cont);
    }
//#endif //__CONTROLABLE__
    /* 
    * Check to ensure that we have not ended up in an inconsistent
    * state. Such a state is either
    *     we get SOP but in_ptk is TRUE
    *     we get !SOP but in_pkt is FALSE
    */
    if ((is_start && JumboFrameBufferLocalPtr->in_pkt) || (!is_start && !JumboFrameBufferLocalPtr->in_pkt)) {
      LogError("ERROR: bad state: is_start = %d, JumboFrameBufferLocalPtr->in_pkt = %d\n", is_start, JumboFrameBufferLocalPtr->in_pkt);
      return -1;
    }
    if (is_start) {
      JumboFrameBufferLocalPtr->in_pkt = 1;
      JumboFrameBufferLocalPtr->buf_ids[0] = pkt_buf_id;
      JumboFrameBufferLocalPtr->frag_data_len[0] = bytes - UDP_HEADER_LEN;
      JumboFrameBufferLocalPtr->payload_bytes = bytes - UDP_HEADER_LEN;
      JumboFrameBufferLocalPtr->n_frags = 0;
      memcpy(&JumboFrameBufferLocalPtr->data[0], (char *)pkt_buf->rx_ptr + UDP_HEADER_LEN, JumboFrameBufferLocalPtr->frag_data_len[0]);
    } else {
      JumboFrameBufferLocalPtr->n_frags++;
      JumboFrameBufferLocalPtr->buf_ids[JumboFrameBufferLocalPtr->n_frags] = pkt_buf_id;
      /* bytes is the total number of bytes from the jumbo frame which
      * have been received. Consequently, some arithmetic needs to be
      * done to determine the number of bytes per fragment */
      JumboFrameBufferLocalPtr->frag_data_len[JumboFrameBufferLocalPtr->n_frags] = bytes - UDP_HEADER_LEN - JumboFrameBufferLocalPtr->payload_bytes;
      memcpy(&JumboFrameBufferLocalPtr->data[JumboFrameBufferLocalPtr->payload_bytes], pkt_buf->rx_ptr, JumboFrameBufferLocalPtr->frag_data_len[JumboFrameBufferLocalPtr->n_frags]);
      JumboFrameBufferLocalPtr->payload_bytes += JumboFrameBufferLocalPtr->frag_data_len[JumboFrameBufferLocalPtr->n_frags];
    }
    if (!is_cont) {
      //give the shared memory thread to get the received data
      //pthread_mutex_lock(&lock);
      //NumberOfPayloadBytes = JumboFrameBufferLocalPtr->payload_bytes;
      //if (CommonDataPtr->ReceivedDataPtr)
      //{
        unsigned int NumberOfPayloadBytes = 0;
#ifdef __CONTROLABLE__
        CommonDataPtr->TotalNumberOfReceivedMessages++;
        if (TscPtr && (CommonDataPtr->ConfigurationFlags & _FLAG_GET_TIMING_)) {
          memcpy((void*)&JumboFrameBufferLocalPtr->data[(size_t)JumboFrameBufferLocalPtr->payload_bytes], (void*)TscPtr->GetTimestamp_ptr(), sizeof(uint64_t));
          if (ringbuf) {
            do {
              NumberOfPayloadBytes = ringbuf->Write((unsigned char *)JumboFrameBufferLocalPtr->data, (size_t)JumboFrameBufferLocalPtr->payload_bytes + sizeof(uint64_t));
            } while (NumberOfPayloadBytes == 0);
          }
        } else
#endif //__CONTROLABLE__
        {
          if (ringbuf) {
            do {
              NumberOfPayloadBytes = ringbuf->Write((unsigned char *)JumboFrameBufferLocalPtr->data, (size_t)JumboFrameBufferLocalPtr->payload_bytes);
            } while (NumberOfPayloadBytes == 0);
          }
        }
      //}
      JumboFrameBufferLocalPtr->in_pkt = 0;
//#ifdef __CONTROLABLE__
      if (CommonDataPtr->ConfigurationFlags & _FLAG_VERBOSE_) {
        if (CommonDataPtr->Verbose) {
          DumpJumboFrameInfo(JumboFrameBufferLocalPtr, &cfg);
        } else {
          LogInfo("packet received: n_frags=%d, bytes=%d\n", JumboFrameBufferLocalPtr->n_frags, JumboFrameBufferLocalPtr->payload_bytes);
          if (cfg.ConfigDumpReceivedPacketsInAsciiFormat) {
            DumpJumboFramePayloadInTextAsciiFormat(JumboFrameBufferLocalPtr);
          } else
          if (cfg.ConfigDumpReceivedPacketsInHexadecimalFormat) {
            DumpJumboFrameInHexadecimalFormat(JumboFrameBufferLocalPtr->data, JumboFrameBufferLocalPtr->payload_bytes);
          }
        }
      } else {
        if (CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_DEC_DUMP_) {
          DumpJumboFramePayloadInTextAsciiFormat(JumboFrameBufferLocalPtr);
        }
        if (CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_HEX_DUMP_) {
          DumpJumboFrameInHexadecimalFormat(JumboFrameBufferLocalPtr->data, JumboFrameBufferLocalPtr->payload_bytes);
        }
      }
//#endif //__CONTROLABLE__
    }
    PktBufFree(pkt_buf);
  } else {
    rv = -1;
  }
  return rv;
}


int NetworkReceiverImplementationEFVI::HandleRxEventDiscard(ef_event* ev) {
  PacketBuffer_t* pkt_buf;
  int pkt_buf_id = EF_EVENT_RX_DISCARD_RQ_ID(*ev);
  int bytes = EF_EVENT_RX_DISCARD_BYTES(*ev) - VirtualResourceINstance.rx_prefix_len;
  int discard_type = EF_EVENT_RX_DISCARD_TYPE(*ev);
  char* discard_str;
  switch (discard_type) {
  case EF_EVENT_RX_DISCARD_CSUM_BAD:
    discard_str = (char*) "BAD_CHECKSUM";
    break;
  case EF_EVENT_RX_DISCARD_MCAST_MISMATCH:
    discard_str = (char*)"MCAST_MISMATCH";
    break;
  case EF_EVENT_RX_DISCARD_CRC_BAD:
    discard_str = (char*)"BAD_CRC";
    break;
  case EF_EVENT_RX_DISCARD_TRUNC:
    discard_str = (char*)"TRUNC";
    break;
  case EF_EVENT_RX_DISCARD_RIGHTS:
    discard_str = (char*)"RIGHTS";
    break;
  case EF_EVENT_RX_DISCARD_EV_ERROR:
    discard_str = (char*)"EV_ERROR";
    break;
  case EF_EVENT_RX_DISCARD_OTHER:
    discard_str = (char*)"OTHER";
    break;
  default:
    discard_str = (char*)"UNKNOWN";
  break;
  }
  LogWarn("WARNING: discard %d bytes of type %d (%s)\n", bytes, discard_type, discard_str);
  pkt_buf = GetPktBufPtrFromId(pkt_buf_id);
  PktBufFree(pkt_buf);
  return 0;
}
//////////////////////////////  MAIN constructor ////////////////////////////////////////

//////////////////// class /////////////////////
NetworkReceiverImplementationEFVI::NetworkReceiverImplementationEFVI(void) {
  jumbo.in_pkt = 0;
  jumbo.n_frags = 0;
  jumbo.payload_bytes = 0;
  NetworkInterfacePtr = new NetInterface;
  ringbuf = (RingFIFOBuffer *) NULL;
  LoggerPtr = (Logger*)NULL;
  CommonDataPtr = (CommonData *) NULL;
  TscPtr = (Tsc *) NULL;
  SetDefaults();
  EfViDriverIsOpen = 0;
  EfPdIsAllocated = 0;
  EfViIsAllocated = 0;
  EfMemIsAllocated = 0;
}


NetworkReceiverImplementationEFVI::~NetworkReceiverImplementationEFVI(void) {
   if(NetworkInterfacePtr) delete NetworkInterfacePtr;
}


int NetworkReceiverImplementationEFVI::SetDefaults(void) {
  Counter = (int) 0;
  ReturnValue = (int) 0;
  cfg.ConfigDumpReceivedPacketsInAsciiFormat = 0;
  snprintf(cfg.IPv4MulticastAddress, sizeof(cfg.IPv4MulticastAddress), "%s", DEST_IPADDR);
  cfg.IPv4MulticastPort = DEST_PORT;
  return 0;
}

int NetworkReceiverImplementationEFVI::Initialize(CommonData *CommonDataPtr, Logger *LoggerPtr, Tsc *TscPtr, RingFIFOBuffer *RingBufferPtr, char *NetworkInterfaceName, int port, char *IPv4MulticastAddress, int ConfigDumpReceivedPacketsInHexadecimalFormat, int ConfigDumpReceivedPacketsInAsciiFormat) {
  int RetValLocal = 0;
  char SocketIPv4Address[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  Initialized = 0;
  this->CommonDataPtr = CommonDataPtr;
  this->ringbuf = RingBufferPtr;
  this->LoggerPtr = LoggerPtr;
  this->TscPtr = TscPtr;
  if (NetworkInterfaceName && strlen(NetworkInterfaceName) <= IFNAMSIZ) {
    strcpy(cfg.NetworkInterfaceName, NetworkInterfaceName);
    cfg.IPv4MulticastPort = port;
    if (IPv4MulticastAddress && strlen(IPv4MulticastAddress) <= INET_ADDRSTRLEN) {
      strcpy(cfg.IPv4MulticastAddress, IPv4MulticastAddress);
      cfg.ConfigDumpReceivedPacketsInHexadecimalFormat = ConfigDumpReceivedPacketsInHexadecimalFormat;
      cfg.ConfigDumpReceivedPacketsInAsciiFormat = ConfigDumpReceivedPacketsInAsciiFormat;
      if (!NetworkInterfacePtr) {
        LogError("ERROR: NetworkInterfacePtr is NULL !");
        RetValLocal = -1;
      } else {
        NetworkInterfacePtr->LoggerPtr = LoggerPtr;
        if (CommonDataPtr) CommonDataPtr->CommonFlagConnected = 0;
        LogInfo("=================================================================\n");
        LogInfo("EF_VI configuration settings:\n\tNetworkInterfaceName=%s, port=%d, ip=%s,\n\tConfigDumpReceivedPacketsInAsciiFormat=%d ConfigDumpReceivedPacketsInHexadecimalFormat=%d\n", cfg.NetworkInterfaceName, cfg.IPv4MulticastPort, cfg.IPv4MulticastAddress, cfg.ConfigDumpReceivedPacketsInAsciiFormat, cfg.ConfigDumpReceivedPacketsInHexadecimalFormat);
        LogInfo("=================================================================\n");
        if (NetworkInterfacePtr && 0 == (RetValLocal =  InitVi())) {
          if (0 == (RetValLocal = InitPktsMemory())) {
            /* refill ring before subscribing, so that packets are not dropped */
            RefillRxRing();
            if (0 == (RetValLocal = AddFilter())) {
              //opening a unix socket for IGMP support
              if (0 != NetworkInterfacePtr->SocketGetIPv4Address(cfg.NetworkInterfaceName, SocketIPv4Address)) {
                strcpy(SocketIPv4Address,"0.0.0.0");
              }
              LogInfo("interface local ip address = %s\n", SocketIPv4Address);
              ReturnValue = NetworkInterfacePtr->SocketInitializeIPv4Receiver(cfg.IPv4MulticastAddress, SocketIPv4Address, cfg.IPv4MulticastPort);
              if (0 != ReturnValue) {
                LogError("error internet socket opening\n");
              }
              LogDebug("EF_VI initialized:\n\tListening on interface %s\n\tListening to address %s, port %d\n", cfg.NetworkInterfaceName, cfg.IPv4MulticastAddress, cfg.IPv4MulticastPort);
              if (CommonDataPtr) {
                if (0 != ReturnValue) {
                  CommonDataPtr->CommonFlagConnected = 0;
                } else {
                  CommonDataPtr->CommonFlagConnected = 1;
                  Initialized = 1;
                }
              }
              RetValLocal = ReturnValue;
            }
          }
        }
      }
      if (!CommonDataPtr) RetValLocal = -1;
    } else {
      LogError("ERROR: IPv4MulticastAddress is NULL or longer than %d !\n", INET_ADDRSTRLEN);
      RetValLocal = -1;
    }
  } else {
    LogError("ERROR: NetworkInterfaceName is NULL or longer than %d !\n", IFNAMSIZ);
    RetValLocal = -1;
  }
  return RetValLocal;
}


void NetworkReceiverImplementationEFVI::Iterate(void) {
  if(Initialized && ringbuf && ringbuf->GetCapacity() >= 16384) {//0
    if (0 != IterateInternalEFVI()) {//1
      CommonDataPtr->ApplicationRunFlag = 0;
    }//1
  }//0
}

int NetworkReceiverImplementationEFVI::IterateInternalEFVI(void) {
  int i, NumberOfEvents, RetValLocal = 0;
  VirtualInterfaceResource_t* VirtualResourceLocalPtr = &VirtualResourceINstance;
  ef_event EfEventState[32];
  NumberOfEvents = ef_eventq_poll(&VirtualResourceLocalPtr->vi, EfEventState, sizeof(EfEventState) / sizeof(EfEventState[0]));
  if (NumberOfEvents > 0) {
    for (i=0; (RetValLocal == 0) && (i < NumberOfEvents); ++i) {
      switch (EF_EVENT_TYPE(EfEventState[i])) {
      case EF_EVENT_TYPE_RX:
        RetValLocal = HandleRxEventAccept(&EfEventState[i]);
        break;
      case EF_EVENT_TYPE_RX_DISCARD:
        RetValLocal = HandleRxEventDiscard(&EfEventState[i]);
        break;
      default:
        LogError("ERROR: unexpected event type=%d\n", (int) EF_EVENT_TYPE(EfEventState[i]));
        RetValLocal = -1;
      }
    }
  }
  // refill the RX ring.
  if (RetValLocal == 0) {
    RefillRxRing();
  }
  return RetValLocal;
}


int NetworkReceiverImplementationEFVI::Uninitialize(void) {
  int RetValLocal = 0;
  ReturnValue = 0;
  if (Initialized) {
    VirtualInterfaceResource_t* VirtualResourceLocalPtr = &VirtualResourceINstance;
    if (VirtualResourceLocalPtr->dh) {
      if (EfMemIsAllocated) {
      //undo ef_memreg_alloc
      RetValLocal = ef_memreg_free(&VirtualResourceLocalPtr->pkt_mem.memreg, VirtualResourceLocalPtr->dh);
      EfMemIsAllocated = 0;
      }
      if(EfViIsAllocated) {
        //undo ef_vi_alloc
        RetValLocal |=  ef_vi_free(&VirtualResourceLocalPtr->vi, VirtualResourceLocalPtr->dh);
        EfViIsAllocated = 0;
      }
      if (EfPdIsAllocated) {
        //undo ef_pd_alloc
        RetValLocal |= ef_pd_free(&VirtualResourceLocalPtr->pd, VirtualResourceLocalPtr->dh);
        EfPdIsAllocated = 0;
      }
      //undo ef_driver_open
      if (EfViDriverIsOpen) {
        RetValLocal |= ef_driver_close(VirtualResourceLocalPtr->dh);
        VirtualResourceLocalPtr->dh = (ef_driver_handle) NULL;
        EfViDriverIsOpen = 0;
      }
    }

    if (NetworkInterfacePtr && 0 == ReturnValue) {
      ReturnValue = NetworkInterfacePtr->SocketUninitialize();
    }
   if (CommonDataPtr) {
     CommonDataPtr->CommonFlagConnected = 0;
     CommonDataPtr = NULL;
    } else RetValLocal |= -1;
    Initialized = 0;
  }
  return RetValLocal | ReturnValue;
}
// _eof_

