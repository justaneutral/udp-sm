
#include <gtest/gtest.h>
#include "CommonData.h"
#include "InputArguments.h"
#include "Logger.h"
#include "ProcessorState.h"
#include "Tsc.h"
#include "KeyboardReadInput.h"
#include "NetworkReceiverInterface.h"
#include "NetworkReceiverImplementationEFVI.h"
#include "ParserInterface.h"
#include "ParserImplementation.h"
#include "InserterInterface.h"
#include "InserterImplementation.h"
#include "UDPJubmoFrameReceiverImplementation.h"


using namespace std;

int TestArgc = 0;
char **TestArgv = (char **) NULL;;
Logger *LoggerPtr = (Logger *) NULL;
///////////////////////////////////////////////////////////////////////////////
namespace {
  template<class T>
  auto not_nullptr(T*p) -> testing::AssertionResult
  {
    if (p)
      return testing::AssertionSuccess();
    else
      return testing::AssertionFailure() << "pointer is null";
  }
}

TEST(TestSuite001, Test001_UDPJubmoFrameReceiver) {
  int argc = TestArgc;
  char **argv = TestArgv;
  int RetVal = -1;
  UDPJubmoFrameReceiverInterface *StreamReaderPtr = new UDPJubmoFrameReceiverImplementation(argc, argv);
  EXPECT_TRUE(not_nullptr(StreamReaderPtr));
  RetVal = StreamReaderPtr->GetReturnValue();
  ASSERT_EQ(RetVal, 0);
  ASSERT_EQ(1, StreamReaderPtr->Iterate()); // while
  RetVal = StreamReaderPtr->Uninitialize();
  ASSERT_EQ(0, RetVal);
  delete StreamReaderPtr;
  
  StreamReaderPtr = new UDPJubmoFrameReceiverImplementation(1, argv);
  EXPECT_TRUE(not_nullptr(StreamReaderPtr));
  ASSERT_NE(0, StreamReaderPtr->GetReturnValue());
  ASSERT_EQ(-1, StreamReaderPtr->Uninitialize());
  ASSERT_EQ(-1, StreamReaderPtr->GetReturnValue());
  ASSERT_EQ(0, StreamReaderPtr->Initialize(argc, argv));
  ASSERT_NE(0, StreamReaderPtr->Iterate());
  ASSERT_EQ(0, StreamReaderPtr->Uninitialize());
  delete StreamReaderPtr;
}

#if (1)
TEST(TestSuite001, Test001_UDPJubmoFrameReceiverDup) {
  int argc = TestArgc;
  char **argv = TestArgv;
  UDPJubmoFrameReceiverInterface *StreamReaderPtr = new UDPJubmoFrameReceiverImplementation(1, argv);
  StreamReaderPtr->Initialize(argc, argv);
  StreamReaderPtr->Initialize(argc, argv);
  EXPECT_TRUE(not_nullptr(StreamReaderPtr));
  ASSERT_EQ(0, StreamReaderPtr->GetReturnValue());
  ASSERT_NE(0, StreamReaderPtr->Iterate());
  ASSERT_EQ(0, StreamReaderPtr->Uninitialize());
  ASSERT_EQ(0, StreamReaderPtr->Uninitialize());
  delete StreamReaderPtr;

  StreamReaderPtr = new UDPJubmoFrameReceiverImplementation(1, argv);
  EXPECT_TRUE(not_nullptr(StreamReaderPtr));
  ASSERT_NE(0, StreamReaderPtr->GetReturnValue());
  ASSERT_EQ(-1, StreamReaderPtr->Uninitialize());
  ASSERT_EQ(-1, StreamReaderPtr->GetReturnValue());
  ASSERT_EQ(0, StreamReaderPtr->Initialize(argc, argv));
  ASSERT_NE(0, StreamReaderPtr->Iterate());
  ASSERT_EQ(0, StreamReaderPtr->Uninitialize());
  delete StreamReaderPtr;
}
#endif //(0)


TEST(TestSuite001, Test001_Inserter) {
  int argc = TestArgc;
  char **argv = TestArgv;
  int IterationCounter = 0;
  ///////////////////////////////////////////////////////////////////////////
  int ReturnValue = 0;
  /////1/////
  ASSERT_EQ(argc, 2);
  cout << "argv[1] = " << argv[1] << endl;
  ASSERT_STREQ(argv[1], "prgconfig.json");
  InputArguments ArgumentsTestInstance(argc, argv);
  ASSERT_EQ(ArgumentsTestInstance.ProcessorCoreId, 12);
  ASSERT_STREQ(ArgumentsTestInstance.SharedMemoryName, "/dev/hugepages/ef_vi_2mb0.mmap");
  ASSERT_STREQ(ArgumentsTestInstance.NetworkInterfaceName, "mint1");
  ASSERT_EQ(ArgumentsTestInstance.Verbose, false);
  ASSERT_EQ(ArgumentsTestInstance.IPv4MulticastUdpPort, 32002);
  ASSERT_STREQ(ArgumentsTestInstance.IPv4MulticastAddress, "239.2.3.12");
  ASSERT_EQ(ArgumentsTestInstance.ConfigDumpReceivedPacketsInAsciiFormat, false);
  ASSERT_EQ(ArgumentsTestInstance.ConfigDumpReceivedPacketsInHexadecimalFormat, false);
  ASSERT_STREQ(ArgumentsTestInstance.LoggingFileName, "log.txt");
  ASSERT_EQ(ArgumentsTestInstance.LoggingLevel, 3);
  /////2////
  Logger TestLoggerInstance;
  TestLoggerInstance.OpenLog((const char*) NULL, (LogLevelEnum) 0, 0);
  ASSERT_EQ(TestLoggerInstance.file, (FILE *) NULL);
  ASSERT_EQ(TestLoggerInstance.LoggingLevel, 0);
  ASSERT_EQ(TestLoggerInstance.Verbose, 0);
  TestLoggerInstance.CloseLog();
  TestLoggerInstance.OpenLog((const char*) ArgumentsTestInstance.LoggingFileName, (LogLevelEnum) ArgumentsTestInstance.LoggingLevel, ArgumentsTestInstance.Verbose);
  ASSERT_NE(TestLoggerInstance.file, (FILE *) NULL);
  ASSERT_EQ(TestLoggerInstance.LoggingLevel, ArgumentsTestInstance.LoggingLevel);
  ASSERT_EQ(TestLoggerInstance.Verbose, ArgumentsTestInstance.Verbose);
  LoggerPtr = &TestLoggerInstance;
  /////3////
  ProcessorState ProcessorTestInstance((Logger *) NULL);
  ReturnValue |= ProcessorTestInstance.StickThisThreadToProcessorCore(ArgumentsTestInstance.ProcessorCoreId);
  ASSERT_EQ(ReturnValue, 0);
  ProcessorState processor(&TestLoggerInstance);
  ReturnValue |= processor.StickThisThreadToProcessorCore(ArgumentsTestInstance.ProcessorCoreId);
  ASSERT_EQ(ReturnValue, 0);
  /////4////
  Tsc TscTestInstance;
  TscTestInstance.MeasureTscOverhead();
  uint64_t Overhead = TscTestInstance.MeasureTscOverhead();
  ASSERT_GT(Overhead, 0);
  TscTestInstance.BenchStart();
  uint64_t TimeDifferenceNanoseconds = TscTestInstance.BenchEnd() - TscTestInstance.GetTimestamp();
  ASSERT_GT(TimeDifferenceNanoseconds, 0);
 
  /////5////
  RingFIFOBuffer RingBufferTestInstance0(__RING_BUFFER_SIZE__, LoggerPtr);
  ASSERT_EQ(RingBufferTestInstance0.RetVal, 0);
  ASSERT_LE(__RING_BUFFER_SIZE__, RingBufferTestInstance0.GetCapacity());
  RingFIFOBuffer RingBufferTestInstance1(__RING_BUFFER_SIZE__, LoggerPtr);
  ASSERT_EQ(RingBufferTestInstance1.RetVal, 0);
  ASSERT_LE(__RING_BUFFER_SIZE__, RingBufferTestInstance1.GetCapacity());
  /////6/////
  Percentile PercentileTestInstance((Logger *) NULL);
  PercentileTestInstance.InitializePercentileTable(__NUM_PERCENTILES__, __NUM_PERCENTILE_MEASUREMENTS__, &TestLoggerInstance);
  for (int i=0; i<__NUM_PERCENTILE_MEASUREMENTS__; i++) PercentileTestInstance.AddNanosecondTimeStampMeasurement(i);
  PercentileTestInstance.PrintPercentileTableValues();
  PercentileTestInstance.ResetPercentileTableArray();
  /////7////
  CommonData CommonDataTestInstance(ArgumentsTestInstance.Verbose, ArgumentsTestInstance.SharedMemoryName, ArgumentsTestInstance.LoggingFileName, ArgumentsTestInstance.LoggingLevel);
  /////8/////
  NetworkReceiverInterface* NetworkReceiverInterfaceTestPtr = new NetworkReceiverImplementationEFVI;
  ReturnValue = NetworkReceiverInterfaceTestPtr->Initialize((CommonData *) NULL, &TestLoggerInstance, &TscTestInstance, &RingBufferTestInstance0, ArgumentsTestInstance.NetworkInterfaceName, ArgumentsTestInstance.IPv4MulticastUdpPort, ArgumentsTestInstance.IPv4MulticastAddress, ArgumentsTestInstance.ConfigDumpReceivedPacketsInHexadecimalFormat, ArgumentsTestInstance.ConfigDumpReceivedPacketsInAsciiFormat);
  ASSERT_EQ(ReturnValue, -1);
  ReturnValue =  NetworkReceiverInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = NetworkReceiverInterfaceTestPtr->Initialize(&CommonDataTestInstance, (Logger *) NULL, &TscTestInstance, &RingBufferTestInstance0, ArgumentsTestInstance.NetworkInterfaceName, ArgumentsTestInstance.IPv4MulticastUdpPort, ArgumentsTestInstance.IPv4MulticastAddress, ArgumentsTestInstance.ConfigDumpReceivedPacketsInHexadecimalFormat, ArgumentsTestInstance.ConfigDumpReceivedPacketsInAsciiFormat);
  ASSERT_EQ(ReturnValue, -1);
  ReturnValue =  NetworkReceiverInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = NetworkReceiverInterfaceTestPtr->Initialize((CommonData *) NULL, (Logger *) NULL, (Tsc *) NULL, &RingBufferTestInstance0, ArgumentsTestInstance.NetworkInterfaceName, ArgumentsTestInstance.IPv4MulticastUdpPort, ArgumentsTestInstance.IPv4MulticastAddress, ArgumentsTestInstance.ConfigDumpReceivedPacketsInHexadecimalFormat, ArgumentsTestInstance.ConfigDumpReceivedPacketsInAsciiFormat);
  ASSERT_EQ(ReturnValue, -1);
  ReturnValue =  NetworkReceiverInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = NetworkReceiverInterfaceTestPtr->Initialize((CommonData *) NULL, (Logger *) NULL, (Tsc *) NULL, (RingFIFOBuffer *) NULL, ArgumentsTestInstance.NetworkInterfaceName, ArgumentsTestInstance.IPv4MulticastUdpPort, ArgumentsTestInstance.IPv4MulticastAddress, ArgumentsTestInstance.ConfigDumpReceivedPacketsInHexadecimalFormat, ArgumentsTestInstance.ConfigDumpReceivedPacketsInAsciiFormat);
  ASSERT_EQ(ReturnValue, -1);
  ReturnValue =  NetworkReceiverInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = NetworkReceiverInterfaceTestPtr->Initialize((CommonData *) NULL, (Logger *) NULL, (Tsc *) NULL, (RingFIFOBuffer *) NULL, (char *) NULL, 0, (char *) NULL, 0, 0);
  ASSERT_EQ(ReturnValue, -1);
  ReturnValue =  NetworkReceiverInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = NetworkReceiverInterfaceTestPtr->Initialize(&CommonDataTestInstance, &TestLoggerInstance, &TscTestInstance, &RingBufferTestInstance0, ArgumentsTestInstance.NetworkInterfaceName, ArgumentsTestInstance.IPv4MulticastUdpPort, ArgumentsTestInstance.IPv4MulticastAddress, ArgumentsTestInstance.ConfigDumpReceivedPacketsInHexadecimalFormat, ArgumentsTestInstance.ConfigDumpReceivedPacketsInAsciiFormat);
  ASSERT_EQ(ReturnValue, -1);
  ReturnValue |=  NetworkReceiverInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, -1);
  delete NetworkReceiverInterfaceTestPtr;
  
  /////9////
  ParserInterface *ParserInterfaceTestPtr = new ParserImplementation((CommonData *) NULL, &TestLoggerInstance, &RingBufferTestInstance0, &RingBufferTestInstance1);
  delete ParserInterfaceTestPtr;
  ParserInterfaceTestPtr = new ParserImplementation(&CommonDataTestInstance, (Logger *) NULL, &RingBufferTestInstance0, &RingBufferTestInstance1);
  delete ParserInterfaceTestPtr;
  ParserInterfaceTestPtr = new ParserImplementation(&CommonDataTestInstance, &TestLoggerInstance, (RingFIFOBuffer *) NULL, &RingBufferTestInstance1);
  delete ParserInterfaceTestPtr;
  ParserInterfaceTestPtr = new ParserImplementation(&CommonDataTestInstance, &TestLoggerInstance, &RingBufferTestInstance0, (RingFIFOBuffer *) NULL);
  delete ParserInterfaceTestPtr;
  ParserInterfaceTestPtr = new ParserImplementation(&CommonDataTestInstance, &TestLoggerInstance, &RingBufferTestInstance0, &RingBufferTestInstance1);
 delete ParserInterfaceTestPtr;
 
  /////10////
  InserterInterface *InserterInterfaceTestPtr = new InserterImplementation; //&InserterInterfaceInst;
  ReturnValue = InserterInterfaceTestPtr->Initialize((CommonData *) NULL, &TestLoggerInstance, &PercentileTestInstance, &TscTestInstance, &RingBufferTestInstance1);
  ASSERT_EQ(ReturnValue, -1);
  ReturnValue = InserterInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, -1);
  ReturnValue = InserterInterfaceTestPtr->Initialize(&CommonDataTestInstance, (Logger *) NULL, &PercentileTestInstance, &TscTestInstance, &RingBufferTestInstance1);
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = InserterInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = InserterInterfaceTestPtr->Initialize(&CommonDataTestInstance, &TestLoggerInstance, (Percentile *) NULL, &TscTestInstance, &RingBufferTestInstance1);
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = InserterInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = InserterInterfaceTestPtr->Initialize(&CommonDataTestInstance, &TestLoggerInstance, &PercentileTestInstance, (Tsc *) NULL, &RingBufferTestInstance1);
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = InserterInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, 0);
  ReturnValue = InserterInterfaceTestPtr->Initialize(&CommonDataTestInstance, &TestLoggerInstance, &PercentileTestInstance, &TscTestInstance, (RingFIFOBuffer *) NULL);
  ASSERT_EQ(ReturnValue, -1);
  ReturnValue = InserterInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, -1);
  ReturnValue = InserterInterfaceTestPtr->Initialize(&CommonDataTestInstance, &TestLoggerInstance, &PercentileTestInstance, &TscTestInstance, &RingBufferTestInstance1);
  ASSERT_EQ(ReturnValue, 0);
  /////11////
  KeyboardReadInput KeyboardReadInputTestInstance(&CommonDataTestInstance, InserterInterfaceTestPtr, &TestLoggerInstance, &PercentileTestInstance, &RingBufferTestInstance0, &RingBufferTestInstance1);

  //////////////////////////////////////////////////////////////////////////

  InserterImplementation InserterImplementationTestiInstance;
  ReturnValue = InserterImplementationTestiInstance.Initialize(&CommonDataTestInstance, &TestLoggerInstance, &PercentileTestInstance, &TscTestInstance, &RingBufferTestInstance1);
  ASSERT_EQ(ReturnValue, 0);
  // test constant offsets
  ASSERT_EQ(__FileMarkerPosition__, 0);
  ASSERT_EQ(__FileVersionPosition__, 4);
  ASSERT_EQ(__LayoutFlagsPosition__, 8);
  ASSERT_EQ(__PageSizePosition__, 12);
  ASSERT_EQ(__LineSizePosition__, 16);
  ASSERT_EQ(__DataSizePosition__, 20);
  ASSERT_EQ(__DataOffsetPosition__, 24);
  ASSERT_EQ(__WriterHeartbeatTimePosition__, 64);
  ASSERT_EQ(__WriterSessionIdPosition__, 72);
  ASSERT_EQ(__WriterMaxSequencePosition__, 80);
  ASSERT_EQ(__WriterFlagsPosition__, 88);
  ASSERT_EQ(__CommitSequencePosition__, 128);
  ASSERT_EQ(__ReserveSequencePosition__, 192);
  ASSERT_EQ(__SharedMemoryDataPosition__, 2097152); //(2ll * 1024 * 1024)
  
  // test pointers
  ASSERT_NE(InserterImplementationTestiInstance.SharedMemoryBufferPtr, (uint8_t *) NULL);
  ASSERT_EQ((uint8_t *) InserterImplementationTestiInstance.SharedMemoryWriterFlagsPtr, (uint8_t *) InserterImplementationTestiInstance.SharedMemoryBufferPtr + __WriterFlagsPosition__);
  ASSERT_NE(InserterImplementationTestiInstance.AtomicWriterFlagsPtr, (std::atomic<uint32_t> *) NULL);
  ASSERT_EQ(InserterImplementationTestiInstance.SharedMemoryWriterSessionIdPtr, (uint8_t *)InserterImplementationTestiInstance.SharedMemoryBufferPtr + __WriterSessionIdPosition__);
  ASSERT_NE((std::atomic<uint64_t> *) 0, InserterImplementationTestiInstance.AtomicWriterSessionIdPtr);
  ASSERT_EQ(InserterImplementationTestiInstance.MaxSequencePtr, (uint8_t *)InserterImplementationTestiInstance.SharedMemoryBufferPtr + __WriterMaxSequencePosition__);
  ASSERT_NE((std::atomic<uint64_t> *) 0, InserterImplementationTestiInstance.AtomicMaxSequencePtr);
  ASSERT_EQ(InserterImplementationTestiInstance.SharedMemoryCommitSequencePtr, (uint8_t *)InserterImplementationTestiInstance.SharedMemoryBufferPtr + __CommitSequencePosition__);
  ASSERT_NE((std::atomic<uint64_t> *) 0, InserterImplementationTestiInstance.AtomicCommitSequencePtr);
  ASSERT_EQ(InserterImplementationTestiInstance.SharedMemoryReserveSequencePtr, (uint8_t *)InserterImplementationTestiInstance.SharedMemoryBufferPtr + __ReserveSequencePosition__);
  ASSERT_NE((std::atomic<uint64_t> *) 0, InserterImplementationTestiInstance.AtomicReserveSequencePtr);

  // test offsets for  writing to shm 
  ASSERT_EQ(InserterImplementationTestiInstance.NumberOfInsertedFramesSinceProgramStart, 0);
  //InserterImplementationTestiInstance.FramingMetaDataMessageSequenceNumber = 0;
  ASSERT_EQ(InserterImplementationTestiInstance.FramingMetaDataMessageSequenceNumber, 0);

  InserterImplementation InserterTmpImplementationTestInstance;
  for (int i = 0; IterationCounter < 20; i++) {
    InserterImplementationTestiInstance.FramingMetaDataTotalNumberOfMessageBytes = 964;
    InserterImplementationTestiInstance.ReceivedDataBuffer = (uint8_t *) new uint8_t[InserterImplementationTestiInstance.FramingMetaDataTotalNumberOfMessageBytes];
    ASSERT_NE((uint8_t *) NULL, InserterImplementationTestiInstance.ReceivedDataBuffer);
    memset(InserterImplementationTestiInstance.ReceivedDataBuffer, i & 0xff, InserterImplementationTestiInstance.FramingMetaDataTotalNumberOfMessageBytes);
    CommonDataTestInstance.CommonValueSequenceNumber = i;


    InserterTmpImplementationTestInstance.CommitSequenceValue = InserterImplementationTestiInstance.CommitSequenceValue;
    InserterTmpImplementationTestInstance.CurrentCacheLine = InserterImplementationTestiInstance.CurrentCacheLine;
    InserterTmpImplementationTestInstance.CacheLinesRemaining = InserterImplementationTestiInstance.CacheLinesRemaining;
    InserterTmpImplementationTestInstance.FramingMetaDataTotalLines = InserterImplementationTestiInstance.FramingMetaDataTotalLines;
    InserterTmpImplementationTestInstance.ReserveSequenceValue = InserterImplementationTestiInstance.ReserveSequenceValue;
    InserterTmpImplementationTestInstance.SharedMemoryBufferPtr = InserterImplementationTestiInstance.SharedMemoryBufferPtr;

    InserterImplementationTestiInstance.HandleSharedMemoryFrames();

    ASSERT_EQ(InserterImplementationTestiInstance.CommitSequenceValue, InserterImplementationTestiInstance.ReserveSequenceValue);
    ASSERT_EQ(InserterImplementationTestiInstance.CurrentCacheLine + InserterImplementationTestiInstance.CacheLinesRemaining, __TOTAL_NUMBER_OF_DATA_CACHE_LINES__);

    if ((i && i<10) ||  InserterImplementationTestiInstance.CurrentCacheLine < InserterTmpImplementationTestInstance.CurrentCacheLine) {
      std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
      std::cout << "FramingMetaDataMessageSequenceNumber == " << InserterImplementationTestiInstance.FramingMetaDataMessageSequenceNumber << std::endl;

      std::cout << "CommitSequenceValue = " << InserterTmpImplementationTestInstance.CommitSequenceValue << " => " <<  InserterImplementationTestiInstance.CommitSequenceValue << std::endl;
      std::cout << "CurrentCacheLine = " << InserterTmpImplementationTestInstance.CurrentCacheLine << " => " << InserterImplementationTestiInstance.CurrentCacheLine << std::endl;
      std::cout << "CacheLinesRemaining = " << InserterTmpImplementationTestInstance.CacheLinesRemaining << " => " << InserterImplementationTestiInstance.CacheLinesRemaining << std::endl;
      std::cout << "FramingMetaDataTotalLines = " << InserterTmpImplementationTestInstance.FramingMetaDataTotalLines << " => " << InserterImplementationTestiInstance.FramingMetaDataTotalLines << std::endl;
      std::cout << "ReserveSequenceValue = " << InserterTmpImplementationTestInstance.ReserveSequenceValue << " => " << InserterImplementationTestiInstance.ReserveSequenceValue << std::endl;
      std::cout << "SharedMemoryBufferPtr = " << InserterTmpImplementationTestInstance.SharedMemoryBufferPtr << " => " << InserterImplementationTestiInstance.SharedMemoryBufferPtr << std::endl;
      std::cout << "addr = " << ((uint16_t *)((uint8_t *)InserterImplementationTestiInstance.SharedMemoryBufferPtr + __SharedMemoryDataPosition__ + (InserterImplementationTestiInstance.CurrentCacheLine * __CACHE_LINE_SIZE__))) << std::endl; 
      std::cout << "aved ddr = " << ((uint16_t *)((uint8_t *)InserterTmpImplementationTestInstance.SharedMemoryBufferPtr + __SharedMemoryDataPosition__ + (InserterTmpImplementationTestInstance.CurrentCacheLine * __CACHE_LINE_SIZE__))) << std::endl; 
      std::cout << "htons(InserterImplementationTestiInstance.CacheLinesRemaining) = " << htons(InserterImplementationTestiInstance.CacheLinesRemaining) << std::endl;
      std::cout << "htons(InserterTmpImplementationTestInstance.CacheLinesRemaining) = " << htons(InserterTmpImplementationTestInstance.CacheLinesRemaining) << std::endl;
      //std::cout << "message body position = " << ((uint8_t *)InserterImplementationTestiInstance.SharedMemoryBufferPtr + __SharedMemoryDataPosition__ + (InserterImplementationTestiInstance.CurrentCacheLine * __CACHE_LINE_SIZE__) + sizeof(InserterImplementationTestiInstance.FramingMetaDataTotalLines)) << std::endl;
      std::cout << "c:" << InserterTmpImplementationTestInstance.CurrentCacheLine << " + r:" <<  InserterTmpImplementationTestInstance.CacheLinesRemaining <<  " = t:" << __TOTAL_NUMBER_OF_DATA_CACHE_LINES__ << std::endl;
      if(InserterTmpImplementationTestInstance.CacheLinesRemaining < InserterImplementationTestiInstance.FramingMetaDataTotalNumberOfMessageBytes) {
        ++IterationCounter;
        std::cout << "transition back: CurrentCacheLine: " << InserterTmpImplementationTestInstance.CurrentCacheLine << " => " << InserterImplementationTestiInstance.CurrentCacheLine << std::endl;
        ASSERT_NE(InserterImplementationTestiInstance.CommitSequenceValue, 0);
        ASSERT_EQ(InserterImplementationTestiInstance.CurrentCacheLine, 0);
      }
      std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
    }
    delete [] InserterImplementationTestiInstance.ReceivedDataBuffer;
    ASSERT_EQ(InserterImplementationTestiInstance.NumberOfInsertedFramesSinceProgramStart, i+1);
  }
  ASSERT_GT(IterationCounter, 10); 
  
  ///////////////////////////////////////////////////////////////////////////

  ReturnValue = InserterInterfaceTestPtr->Uninitialize();
  ASSERT_EQ(ReturnValue, 0);
 // ReturnValue |=  NetworkReceiverInterfaceTestPtr->Uninitialize();
 // ASSERT_EQ(ReturnValue, 0);

  delete InserterInterfaceTestPtr;
  //delete ParserInterfaceTestPtr;
  //delete NetworkReceiverInterfaceTestPtr;

  RingBufferTestInstance0.Uninitialize();
  RingBufferTestInstance1.Uninitialize();
  TestLoggerInstance.CloseLog();

}


#if(1)
TEST(TestSuite002Ringbuffer, Test001_Next64AlignedTest) {
  RingFIFOBuffer TestRingBuffer(__RING_BUFFER_SIZE__, LoggerPtr);
  ASSERT_EQ(TestRingBuffer.RetVal, 0);
  ASSERT_LE(__RING_BUFFER_SIZE__, TestRingBuffer.GetCapacity());
  uint64_t x = TestRingBuffer.Next64AlignedTest(0);
  ASSERT_EQ(0, x);
  x = TestRingBuffer.Next64AlignedTest(1);
  ASSERT_EQ(64, x);
  x = TestRingBuffer.Next64AlignedTest(65);
  ASSERT_EQ(128, x);
  x = TestRingBuffer.Next64AlignedTest(129);
  ASSERT_EQ(192, x);
  x = TestRingBuffer.Next64AlignedTest(193);
  ASSERT_EQ(256, x);
  x = TestRingBuffer.Next64AlignedTest(__RING_BUFFER_SIZE__);
  ASSERT_EQ(__RING_BUFFER_SIZE__, x);
  x = TestRingBuffer.Next64AlignedTest(__RING_BUFFER_SIZE__-31);
  ASSERT_EQ(__RING_BUFFER_SIZE__, x);
}



TEST(TestSuite002Ringbuffer, Test002_initialize_write_read_uninitialize) {
  RingFIFOBuffer TestRingBuffer(__RING_BUFFER_SIZE__, LoggerPtr);
  ASSERT_EQ(TestRingBuffer.RetVal, 0);
  ASSERT_LE(__RING_BUFFER_SIZE__, TestRingBuffer.GetCapacity());

  unsigned char *pxh = (unsigned char *) NULL;
  unsigned char *prh = (unsigned char *) NULL;
  int framelength_aligned, frameminlength, framemaxlength, i, j;
  int CurrentlySentBytes = 0xffff, total_sent_bytes = 0, currently_sent_frames = 0, PayloadBytesReceived = 0, currently_received_bytes = 0xffff, total_received_bytes = 0, currently_received_frames = 0;

  frameminlength = 1;
  framemaxlength = 9000;

  pxh = (unsigned char *) new unsigned char[framemaxlength];
  prh = (unsigned char *) new unsigned char[framemaxlength];

  if (pxh && prh) for ( int framelength = frameminlength; framelength <= framemaxlength; framelength += (framemaxlength>>7)) {
    framelength_aligned = TestRingBuffer.Next64AlignedTest(framelength+4);
    //ASSERT_EQ(67, framelength);
    CurrentlySentBytes = 0xffff;
    total_sent_bytes = 0;
    currently_sent_frames = 0;
    PayloadBytesReceived = 0;
    currently_received_bytes = 0xffff;
    total_received_bytes = 0;
    currently_received_frames = 0;
    i = 0;
    for (i = 0; CurrentlySentBytes &&  total_sent_bytes < __RING_BUFFER_SIZE__; ++i) {
      CurrentlySentBytes = TestRingBuffer.Write(pxh, framelength);
      if(CurrentlySentBytes) currently_sent_frames = i + 1;
      total_sent_bytes += CurrentlySentBytes;
      ASSERT_EQ(framelength * currently_sent_frames, total_sent_bytes);
      ASSERT_EQ(currently_sent_frames, TestRingBuffer.ringbuf.FramesStored);
      ASSERT_LE(framelength_aligned * currently_sent_frames, TestRingBuffer.ringbuf.NumberOfStoredBytes);
      ASSERT_LE(framelength_aligned * currently_sent_frames, TestRingBuffer.ringbuf.buffer_size - TestRingBuffer.GetCapacity());
      //TestRingBuffer.PrintStatus((const char *)"rb write test");
    }
    ASSERT_GE((__RING_BUFFER_SIZE__/framelength_aligned), currently_sent_frames);

    CurrentlySentBytes = TestRingBuffer.Write(pxh, framelength);
    ASSERT_EQ(0, CurrentlySentBytes);
    ASSERT_LE(total_sent_bytes, TestRingBuffer.ringbuf.NumberOfStoredBytes);
    ASSERT_GT(TestRingBuffer.ringbuf.SlipCounter, 0);
    //TestRingBuffer.PrintStatus((const char *)"rb max populated test");

    for (i = 0; currently_received_bytes &&  total_received_bytes < __RING_BUFFER_SIZE__; ++i) {
      currently_received_bytes = TestRingBuffer.Read(prh);
      if(currently_received_bytes) currently_received_frames = i + 1;
      total_received_bytes += currently_received_bytes;
      ASSERT_EQ(framelength * currently_received_frames, total_received_bytes);
      ASSERT_EQ(currently_received_frames, currently_sent_frames -  TestRingBuffer.ringbuf.FramesStored);
      //ASSERT_EQ(framelength_aligned * (currently_sent_frames - currently_received_frames), TestRingBuffer.ringbuf.buffer_size - TestRingBuffer.GetCapacity());
      //TestRingBuffer.PrintStatus((const char *)"rb read test");
    }
    ASSERT_GE((__RING_BUFFER_SIZE__/framelength_aligned), currently_received_frames);
    ASSERT_EQ(total_sent_bytes, total_received_bytes);
    ASSERT_EQ(currently_sent_frames, currently_received_frames);
 
    //TestRingBuffer.PrintStatus((const char *)"rb empty test");

    for (i = 0; i < currently_sent_frames * 10; i++) {
      for (j = 0; j < framelength; j++) {
        pxh[j] = (unsigned char)(0xff &(i + j));
      }
      for (int j = 0; j < framelength; j++) {
        pxh[j] =  prh[j] + 1;
        ASSERT_NE(pxh[j], prh[j]);
      }
      TestRingBuffer.Snap();
      CurrentlySentBytes = TestRingBuffer.Write(pxh, framelength);
      ASSERT_GT(CurrentlySentBytes, 0);
      ASSERT_GT(TestRingBuffer.ringbuf.FramesStored, 0);
      //TestRingBuffer.PrintStatus((const char *)"rb add before snap test");
      TestRingBuffer.Discard();
      ASSERT_EQ(TestRingBuffer.ringbuf.FramesStored, 0);
      //TestRingBuffer.PrintStatus((const char *)"rb snap test");
      CurrentlySentBytes = TestRingBuffer.Write(pxh, framelength);
      ASSERT_GT(CurrentlySentBytes, 0);
      ASSERT_GT(TestRingBuffer.ringbuf.FramesStored, 0);
      //TestRingBuffer.PrintStatus((const char *)"rb write before read test");
      PayloadBytesReceived = TestRingBuffer.Read(prh);
      ASSERT_EQ(PayloadBytesReceived, CurrentlySentBytes);
      ASSERT_EQ(PayloadBytesReceived, framelength);
      ASSERT_EQ(CurrentlySentBytes, framelength);
      for (j = 0; j < framelength; j++) {
        ASSERT_EQ(pxh[j], prh[j]);
      }
      ASSERT_EQ(TestRingBuffer.ringbuf.FramesStored, 0);
      if(TestRingBuffer.ringbuf.FramesStored == 0) {
        ASSERT_EQ(TestRingBuffer.ringbuf.NumberOfStoredBytes, 0);
      }
      //TestRingBuffer.PrintStatus((const char *)"rb read after write test");
    }
  }
  printf("Processed % ld frames\n", TestRingBuffer.ringbuf.NumberOfProcessedFrames);
  TestRingBuffer.Uninitialize();
  delete [] prh;
  delete [] pxh;
}
#endif //(0)(1)

#if(1)
TEST(TestSuite002Ringbuffer, Test003_initialize_write_read_uninitialize_print_status) {
  RingFIFOBuffer TestRingBuffer(__RING_BUFFER_SIZE__, LoggerPtr);
  ASSERT_EQ(TestRingBuffer.RetVal, 0);
  ASSERT_LE(__RING_BUFFER_SIZE__, TestRingBuffer.GetCapacity());

  unsigned char *pxh = (unsigned char *) NULL;
  unsigned char *prh = (unsigned char *) NULL;
  int framelength_aligned, frameminlength, framemaxlength, i, j;
  int CurrentlySentBytes = 0xffff, total_sent_bytes = 0, currently_sent_frames = 0, PayloadBytesReceived = 0, currently_received_bytes = 0xffff, total_received_bytes = 0, currently_received_frames = 0;

  frameminlength = 60;
  framemaxlength = 9000;

  pxh = (unsigned char *) new unsigned char[framemaxlength];
  prh = (unsigned char *) new unsigned char[framemaxlength];

  if (pxh && prh) for ( int framelength = frameminlength; framelength <= framemaxlength; framelength += (framelength>>2)) {
    framelength_aligned = TestRingBuffer.Next64AlignedTest(framelength+4);
    //ASSERT_EQ(67, framelength);
    CurrentlySentBytes = 0xffff;
    total_sent_bytes = 0;
    currently_sent_frames = 0;
    PayloadBytesReceived = 0;
    currently_received_bytes = 0xffff;
    total_received_bytes = 0;
    currently_received_frames = 0;
    i = 0;
    for (i = 0; CurrentlySentBytes &&  total_sent_bytes < __RING_BUFFER_SIZE__; ++i) {
      CurrentlySentBytes = TestRingBuffer.Write(pxh, framelength);
      if(CurrentlySentBytes) currently_sent_frames = i + 1;
      total_sent_bytes += CurrentlySentBytes;
      ASSERT_EQ(framelength * currently_sent_frames, total_sent_bytes);
      ASSERT_EQ(currently_sent_frames, TestRingBuffer.ringbuf.FramesStored);
      ASSERT_LE(framelength_aligned * currently_sent_frames, TestRingBuffer.ringbuf.NumberOfStoredBytes);
      ASSERT_LE(framelength_aligned * currently_sent_frames, TestRingBuffer.ringbuf.buffer_size - TestRingBuffer.GetCapacity());
      //TestRingBuffer.PrintStatus((const char *)"rb write test");
    }
    ASSERT_GE((__RING_BUFFER_SIZE__/framelength_aligned), currently_sent_frames);

    CurrentlySentBytes = TestRingBuffer.Write(pxh, framelength);
    ASSERT_EQ(0, CurrentlySentBytes);
    ASSERT_LE(total_sent_bytes, TestRingBuffer.ringbuf.NumberOfStoredBytes);
    ASSERT_GT(TestRingBuffer.ringbuf.SlipCounter, 0);
    //TestRingBuffer.PrintStatus((const char *)"rb max populated test");

    for (i = 0; currently_received_bytes &&  total_received_bytes < __RING_BUFFER_SIZE__; ++i) {
      currently_received_bytes = TestRingBuffer.Read(prh);
      if(currently_received_bytes) currently_received_frames = i + 1;
      total_received_bytes += currently_received_bytes;
      ASSERT_EQ(framelength * currently_received_frames, total_received_bytes);
      ASSERT_EQ(currently_received_frames, currently_sent_frames -  TestRingBuffer.ringbuf.FramesStored);
      //ASSERT_EQ(framelength_aligned * (currently_sent_frames - currently_received_frames), TestRingBuffer.ringbuf.buffer_size - TestRingBuffer.GetCapacity());
      //TestRingBuffer.PrintStatus((const char *)"rb read test");
    }
    ASSERT_GE((__RING_BUFFER_SIZE__/framelength_aligned), currently_received_frames);
    ASSERT_EQ(total_sent_bytes, total_received_bytes);
    ASSERT_EQ(currently_sent_frames, currently_received_frames);
 
    TestRingBuffer.PrintStatus((const char *)"rb empty test");

    for (i = 0; i < currently_sent_frames * 10; i++) {
      for (j = 0; j < framelength; j++) {
        pxh[j] = (unsigned char)(0xff &(i + j));
      }
      for (int j = 0; j < framelength; j++) {
        pxh[j] =  prh[j] + 1;
        ASSERT_NE(pxh[j], prh[j]);
      }
      TestRingBuffer.Snap();
      CurrentlySentBytes = TestRingBuffer.Write(pxh, framelength);
      ASSERT_GT(CurrentlySentBytes, 0);
      ASSERT_GT(TestRingBuffer.ringbuf.FramesStored, 0);
      //TestRingBuffer.PrintStatus((const char *)"rb add before snap test");
      TestRingBuffer.Discard();
      ASSERT_EQ(TestRingBuffer.ringbuf.FramesStored, 0);
      //TestRingBuffer.PrintStatus((const char *)"rb snap test");
      CurrentlySentBytes = TestRingBuffer.Write(pxh, framelength);
      ASSERT_GT(CurrentlySentBytes, 0);
      ASSERT_GT(TestRingBuffer.ringbuf.FramesStored, 0);
      //TestRingBuffer.PrintStatus((const char *)"rb write before read test");
      PayloadBytesReceived = TestRingBuffer.Read(prh);
      ASSERT_EQ(PayloadBytesReceived, CurrentlySentBytes);
      ASSERT_EQ(PayloadBytesReceived, framelength);
      ASSERT_EQ(CurrentlySentBytes, framelength);
      for (j = 0; j < framelength; j++) {
        ASSERT_EQ(pxh[j], prh[j]);
      }
      ASSERT_EQ(TestRingBuffer.ringbuf.FramesStored, 0);
      if(TestRingBuffer.ringbuf.FramesStored == 0) {
        ASSERT_EQ(TestRingBuffer.ringbuf.NumberOfStoredBytes, 0);
      }
      //TestRingBuffer.PrintStatus((const char *)"rb read after write test");
    }
  }
  TestRingBuffer.PrintStatus((const char *)"rb empty test completed");
  printf("Processed % ld frames\n", TestRingBuffer.ringbuf.NumberOfProcessedFrames);
  TestRingBuffer.Uninitialize();
  delete [] prh;
  delete [] pxh;
}

#endif //(0)(1)

#if (1)
TEST(TestSuite003, Test001_UDPJubmoFrameReceiver) {
  int argc = TestArgc;
  char **argv = TestArgv;
  UDPJubmoFrameReceiverInterface *StreamReaderPtr = new UDPJubmoFrameReceiverImplementation(argc, argv);
  EXPECT_TRUE(not_nullptr(StreamReaderPtr));
  ASSERT_EQ(0, StreamReaderPtr->GetReturnValue());
  ASSERT_NE(0, StreamReaderPtr->Iterate());
  ASSERT_EQ(0, StreamReaderPtr->Uninitialize());
  delete StreamReaderPtr;
  
  StreamReaderPtr = new UDPJubmoFrameReceiverImplementation(1, argv);
  EXPECT_TRUE(not_nullptr(StreamReaderPtr));
  ASSERT_NE(0, StreamReaderPtr->GetReturnValue());
  ASSERT_EQ(-1, StreamReaderPtr->Uninitialize());
  ASSERT_EQ(-1, StreamReaderPtr->GetReturnValue());
  ASSERT_EQ(0, StreamReaderPtr->Initialize(argc, argv));
  ASSERT_EQ(1, StreamReaderPtr->Iterate());
  ASSERT_EQ(0, StreamReaderPtr->Uninitialize());
  delete StreamReaderPtr;
}
#endif //(0)
//////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
  int RetVal = 0;
//#ifdef __CONTROLABLE__
//  std::cout << "Please undefine __CONTROLABLE__ in defines.h" << std::endl;
//#else //__CONTROLABLE__
  TestArgc = argc;
  TestArgv = argv;
  testing::InitGoogleTest(&argc, argv);
  RetVal = RUN_ALL_TESTS();
//#endif //__CONTROLABLE__
  return RetVal;
}

