/*******************************************************************************
* Copyright 2017 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

////////////////////////////////////////////////////////////////////////////////
/// @file The file for Dynamixel packet control
/// @author Zerom, Leon (RyuWoon Jung)
////////////////////////////////////////////////////////////////////////////////

#ifndef DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_PACKETHANDLER_H_
#define DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_PACKETHANDLER_H_

#if defined(ARDUINO) || defined(__OPENCR__) || defined(__OPENCM904__)
#include <Arduino.h>

#define ERROR_PRINT  SerialBT2.print
#else
#define ERROR_PRINT  printf

#endif

#include <stdio.h>
#include <vector>
#include "port_handler.h"

#define BROADCAST_ID        0xFE    // 254
#define MAX_ID              0xFC    // 252

/* Macro for Control Table Value */
#define DXL_MAKEWORD(a, b)  ((uint16_t)(((uint8_t)(((uint64_t)(a)) & 0xff)) | ((uint16_t)((uint8_t)(((uint64_t)(b)) & 0xff))) << 8))
#define DXL_MAKEDWORD(a, b) ((uint32_t)(((uint16_t)(((uint64_t)(a)) & 0xffff)) | ((uint32_t)((uint16_t)(((uint64_t)(b)) & 0xffff))) << 16))
#define DXL_LOWORD(l)       ((uint16_t)(((uint64_t)(l)) & 0xffff))
#define DXL_HIWORD(l)       ((uint16_t)((((uint64_t)(l)) >> 16) & 0xffff))
#define DXL_LOBYTE(w)       ((uint8_t)(((uint64_t)(w)) & 0xff))
#define DXL_HIBYTE(w)       ((uint8_t)((((uint64_t)(w)) >> 8) & 0xff))

/* Instruction for DXL Protocol */
#define INST_PING               1
#define INST_READ               2
#define INST_WRITE              3
#define INST_REG_WRITE          4
#define INST_ACTION             5
#define INST_FACTORY_RESET      6
#define INST_SYNC_WRITE         131     // 0x83
#define INST_BULK_READ          146     // 0x92
// --- Only for 2.0 --- //
#define INST_REBOOT             8
#define INST_CLEAR              16      // 0x10
#define INST_STATUS             85      // 0x55
#define INST_SYNC_READ          130     // 0x82
#define INST_BULK_WRITE         147     // 0x93

// Communication Result
#define COMM_SUCCESS        0       // tx or rx packet communication success
#define COMM_PORT_BUSY      -1000   // Port is busy (in use)
#define COMM_TX_FAIL        -1001   // Failed transmit instruction packet
#define COMM_RX_FAIL        -1002   // Failed get status packet
#define COMM_TX_ERROR       -2000   // Incorrect instruction packet
#define COMM_RX_WAITING     -3000   // Now recieving status packet
#define COMM_RX_TIMEOUT     -3001   // There is no status packet
#define COMM_RX_CORRUPT     -3002   // Incorrect status packet
#define COMM_NOT_AVAILABLE  -9000   //

namespace dynamixel
{

////////////////////////////////////////////////////////////////////////////////
/// @brief The class that inherits Protocol1PacketHandler class or Protocol2PacketHandler class
////////////////////////////////////////////////////////////////////////////////
class WINDECLSPEC PacketHandler
{
 protected:
  PacketHandler() { }

 public:
  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that returns PacketHandler instance
  /// @return PacketHandler instance
  ////////////////////////////////////////////////////////////////////////////////
  static PacketHandler *getPacketHandler(float protocol_version = 2.0);

  virtual ~PacketHandler() { }

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that returns Protocol version
  /// @return protocol version
  ////////////////////////////////////////////////////////////////////////////////
  virtual float   getProtocolVersion() = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that gets description of communication result
  /// @param result Communication result which might be gotten by the tx rx functions
  /// @return description of communication result in const char* (string)
  ////////////////////////////////////////////////////////////////////////////////
  virtual const char *getTxRxResult     (int result) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that gets description of hardware error
  /// @param error Dynamixel hardware error which might be gotten by the tx rx functions
  /// @return description of hardware error in const char* (string)
  ////////////////////////////////////////////////////////////////////////////////
  virtual const char *getRxPacketError  (uint8_t error) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits the instruction packet txpacket via PortHandler port.
  /// @description The function clears the port buffer by PortHandler::clearPort() function,
  /// @description   then transmits txpacket by PortHandler::writePort() function.
  /// @description The function activates only when the port is not busy and when the packet is already written on the port buffer
  /// @param port PortHandler instance
  /// @param txpacket packet for transmission
  /// @return COMM_PORT_BUSY
  /// @return   when the port is already in use
  /// @return COMM_TX_ERROR
  /// @return   when txpacket is out of range described by TXPACKET_MAX_LEN
  /// @return COMM_TX_FAIL
  /// @return   when written packet is shorter than expected
  /// @return or COMM_SUCCESS
  ////////////////////////////////////////////////////////////////////////////////
  virtual int txPacket        (PortHandler *port, uint8_t *txpacket) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that receives packet (rxpacket) during designated time via PortHandler port
  /// @description The function repeatedly tries to receive rxpacket by PortHandler::readPort() function.
  /// @description It breaks out
  /// @description when PortHandler::isPacketTimeout() shows the timeout,
  /// @description when rxpacket seemed as corrupted, or
  /// @description when nothing received
  /// @param port PortHandler instance
  /// @param rxpacket received packet
  /// @return COMM_RX_CORRUPT
  /// @return   when it received the packet but it couldn't find header in the packet
  /// @return   when it found header in the packet but the id, length or error value is out of range
  /// @return   when it received the packet but it is shorted than expected
  /// @return COMM_RX_TIMEOUT
  /// @return   when there is no rxpacket received until PortHandler::isPacketTimeout() shows the timeout
  /// @return COMM_SUCCESS
  /// @return   when rxpacket passes checksum test
  /// @return or COMM_RX_FAIL
  ////////////////////////////////////////////////////////////////////////////////
  virtual int rxPacket        (PortHandler *port, uint8_t *rxpacket) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits packet (txpacket) and receives packet (rxpacket) during designated time via PortHandler port
  /// @description The function calls PacketHandler::txPacket(),
  /// @description and calls PacketHandler::rxPacket() if it succeeds PacketHandler::txPacket().
  /// @description It breaks out
  /// @description when it fails PacketHandler::txPacket(),
  /// @description when txpacket is called by PacketHandler::broadcastPing() / PacketHandler::syncWriteTxOnly() / PacketHandler::regWriteTxOnly / PacketHandler::action
  /// @param port PortHandler instance
  /// @param txpacket packet for transmission
  /// @param rxpacket received packet
  /// @return COMM_SUCCESS
  /// @return   when it succeeds PacketHandler::txPacket() and PacketHandler::rxPacket()
  /// @return or the other communication results which come from PacketHandler::txPacket() and PacketHandler::rxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int txRxPacket      (PortHandler *port, uint8_t *txpacket, uint8_t *rxpacket, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that pings Dynamixel but doesn't take its model number
  /// @description The function calls PacketHandler::ping() which gets Dynamixel model number,
  /// @description but doesn't carry the model number
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::ping()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int ping            (PortHandler *port, uint8_t id, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that pings Dynamixel and takes its model number
  /// @description The function makes an instruction packet with INST_PING,
  /// @description transmits the packet with PacketHandler::txRxPacket(),
  /// @description and call PacketHandler::readTxRx to read model_number in the rx buffer.
  /// @description It breaks out
  /// @description when it tries to transmit to BROADCAST_ID
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param error Dynamixel hardware error
  /// @return COMM_NOT_AVAILABLE
  /// @return   when it tries to transmit to BROADCAST_ID
  /// @return COMM_SUCCESS
  /// @return   when it succeeds to ping Dynamixel and get model_number from it
  /// @return or the other communication results which come from PacketHandler::txRxPacket() and PacketHandler::readTxRx()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int ping            (PortHandler *port, uint8_t id, uint16_t *model_number, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief (Available only in Protocol 2.0) The function that pings all connected Dynamixel
  /// @param port PortHandler instance
  /// @param id_list ID list of Dynamixels which are found by broadcast ping
  /// @return COMM_NOT_AVAILABLE
  ////////////////////////////////////////////////////////////////////////////////
  virtual int broadcastPing   (PortHandler *port, std::vector<uint8_t> &id_list) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that makes Dynamixels run as written in the Dynamixel register
  /// @description The function makes an instruction packet with INST_ACTION,
  /// @description transmits the packet with PacketHandler::txRxPacket().
  /// @description To use this function, Dynamixel register should be set by PacketHandler::regWriteTxOnly() or PacketHandler::regWriteTxRx()
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @return communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int action          (PortHandler *port, uint8_t id) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that makes Dynamixel reboot
  /// @description The function makes an instruction packet with INST_REBOOT,
  /// @description transmits the packet with PacketHandler::txRxPacket(),
  /// @description then Dynamixel reboots.
  /// @description During reboot, its LED will blink.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param error Dynamixel hardware error
  /// @return COMM_NOT_AVAILABLE
  ////////////////////////////////////////////////////////////////////////////////
  virtual int reboot          (PortHandler *port, uint8_t id, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that reset multi-turn revolution information of Dynamixel
  /// @description The function makes an instruction packet with INST_CLEAR,
  /// @description transmits the packet with PacketHandler::txRxPacket().
  /// @description Applied Products : MX with Protocol 2.0 (Firmware v42 or above),
  /// @description Dynamixel X-series (Firmware v42 or above).
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int clearMultiTurn  (PortHandler *port, uint8_t id, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that makes Dynamixel reset as it was produced in the factory
  /// @description The function makes an instruction packet with INST_FACTORY_RESET,
  /// @description transmits the packet with PacketHandler::txRxPacket().
  /// @description Be careful of the use.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param option Reset option
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int factoryReset    (PortHandler *port, uint8_t id, uint8_t option = 0, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_READ instruction packet
  /// @description The function makes an instruction packet with INST_READ,
  /// @description transmits the packet with PacketHandler::txPacket().
  /// @description It breaks out
  /// @description when it tries to transmit to BROADCAST_ID
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @param length Length of the data for read
  /// @return COMM_NOT_AVAILABLE
  /// @return   when it tries to transmit to BROADCAST_ID
  /// @return or the other communication results which come from PacketHandler::txPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int readTx          (PortHandler *port, uint8_t id, uint16_t address, uint16_t length) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that receives the packet and reads the data in the packet
  /// @description The function receives the packet which might be come by previous INST_READ instruction packet transmission,
  /// @description gets the data from the packet.
  /// @param port PortHandler instance
  /// @param length Length of the data for read
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::rxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int readRx          (PortHandler *port, uint8_t id, uint16_t length, uint8_t *data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_READ instruction packet, and read data from received packet
  /// @description The function makes an instruction packet with INST_READ,
  /// @description transmits and receives the packet with PacketHandler::txRxPacket(),
  /// @description gets the data from the packet.
  /// @description It breaks out
  /// @description when it tries to transmit to BROADCAST_ID
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @param length Length of the data for read
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return COMM_NOT_AVAILABLE
  /// @return   when it tries to transmit to BROADCAST_ID
  /// @return or the other communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int readTxRx        (PortHandler *port, uint8_t id, uint16_t address, uint16_t length, uint8_t *data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::readTx() function for reading 1 byte data
  /// @description The function calls PacketHandler::readTx() function for reading 1 byte data
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @return communication results which come from PacketHandler::readTx()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int read1ByteTx     (PortHandler *port, uint8_t id, uint16_t address) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::readRx() function and reads 1 byte data on the packet
  /// @description The function calls PacketHandler::readRx() function,
  /// @description gets 1 byte data from the packet.
  /// @param port PortHandler instance
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::readRx()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int read1ByteRx     (PortHandler *port, uint8_t id, uint8_t *data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::readTxRx() function for reading 1 byte data
  /// @description The function calls PacketHandler::readTxRx(),
  /// @description gets 1 byte data from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @param length Length of the data for read
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int read1ByteTxRx   (PortHandler *port, uint8_t id, uint16_t address, uint8_t *data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::readTx() function for reading 2 byte data
  /// @description The function calls PacketHandler::readTx() function for reading 2 byte data
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @return communication results which come from PacketHandler::readTx()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int read2ByteTx     (PortHandler *port, uint8_t id, uint16_t address) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::readRx() function and reads 2 byte data on the packet
  /// @description The function calls PacketHandler::readRx() function,
  /// @description gets 2 byte data from the packet.
  /// @param port PortHandler instance
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::readRx()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int read2ByteRx     (PortHandler *port, uint8_t id, uint16_t *data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::readTxRx() function for reading 2 byte data
  /// @description The function calls PacketHandler::readTxRx(),
  /// @description gets 2 byte data from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @param length Length of the data for read
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int read2ByteTxRx   (PortHandler *port, uint8_t id, uint16_t address, uint16_t *data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::readTx() function for reading 4 byte data
  /// @description The function calls PacketHandler::readTx() function for reading 4 byte data
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @return communication results which come from PacketHandler::readTx()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int read4ByteTx     (PortHandler *port, uint8_t id, uint16_t address) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::readRx() function and reads 4 byte data on the packet
  /// @description The function calls PacketHandler::readRx() function,
  /// @description gets 4 byte data from the packet.
  /// @param port PortHandler instance
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::readRx()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int read4ByteRx     (PortHandler *port, uint8_t id, uint32_t *data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::readTxRx() function for reading 4 byte data
  /// @description The function calls PacketHandler::readTxRx(),
  /// @description gets 4 byte data from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @param length Length of the data for read
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int read4ByteTxRx   (PortHandler *port, uint8_t id, uint16_t address, uint32_t *data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_WRITE instruction packet with the data for write
  /// @description The function makes an instruction packet with INST_WRITE and the data for write,
  /// @description transmits the packet with PacketHandler::txPacket().
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param length Length of the data for write
  /// @param data Data for write
  /// @return communication results which come from PacketHandler::txPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int writeTxOnly     (PortHandler *port, uint8_t id, uint16_t address, uint16_t length, uint8_t *data) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_WRITE instruction packet with the data for write, and receives the packet
  /// @description The function makes an instruction packet with INST_WRITE and the data for write,
  /// @description transmits and receives the packet with PacketHandler::txRxPacket(),
  /// @description gets the error from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param length Length of the data for write
  /// @param data Data for write
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int writeTxRx       (PortHandler *port, uint8_t id, uint16_t address, uint16_t length, uint8_t *data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::writeTxOnly() for writing 1 byte data
  /// @description The function calls PacketHandler::writeTxOnly() for writing 1 byte data.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @return communication results which come from PacketHandler::writeTxOnly()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int write1ByteTxOnly(PortHandler *port, uint8_t id, uint16_t address, uint8_t data) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::writeTxRx() for writing 1 byte data and receives the packet
  /// @description The function calls PacketHandler::writeTxRx() for writing 1 byte data and receves the packet,
  /// @description gets the error from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::writeTxRx()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int write1ByteTxRx  (PortHandler *port, uint8_t id, uint16_t address, uint8_t data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::writeTxOnly() for writing 2 byte data
  /// @description The function calls PacketHandler::writeTxOnly() for writing 2 byte data.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @return communication results which come from PacketHandler::writeTxOnly()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int write2ByteTxOnly(PortHandler *port, uint8_t id, uint16_t address, uint16_t data) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::writeTxRx() for writing 2 byte data and receives the packet
  /// @description The function calls PacketHandler::writeTxRx() for writing 2 byte data and receves the packet,
  /// @description gets the error from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::writeTxRx()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int write2ByteTxRx  (PortHandler *port, uint8_t id, uint16_t address, uint16_t data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::writeTxOnly() for writing 4 byte data
  /// @description The function calls PacketHandler::writeTxOnly() for writing 4 byte data.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @return communication results which come from PacketHandler::writeTxOnly()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int write4ByteTxOnly(PortHandler *port, uint8_t id, uint16_t address, uint32_t data) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls PacketHandler::writeTxRx() for writing 4 byte data and receives the packet
  /// @description The function calls PacketHandler::writeTxRx() for writing 4 byte data and receves the packet,
  /// @description gets the error from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::writeTxRx()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int write4ByteTxRx  (PortHandler *port, uint8_t id, uint16_t address, uint32_t data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_REG_WRITE instruction packet with the data for writing on the Dynamixel register
  /// @description The function makes an instruction packet with INST_REG_WRITE and the data for writing on the Dynamixel register,
  /// @description transmits the packet with PacketHandler::txPacket().
  /// @description The data written in the register will act when INST_ACTION instruction packet is transmitted to the Dynamixel.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param length Length of the data for write
  /// @param data Data for write
  /// @return communication results which come from PacketHandler::txPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int regWriteTxOnly  (PortHandler *port, uint8_t id, uint16_t address, uint16_t length, uint8_t *data) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_REG_WRITE instruction packet with the data for writing on the Dynamixel register, and receives the packet
  /// @description The function makes an instruction packet with INST_REG_WRITE and the data for writing on the Dynamixel register,
  /// @description transmits and receives the packet with PacketHandler::txRxPacket(),
  /// @description gets the error from the packet.
  /// @description The data written in the register will act when INST_ACTION instruction packet is transmitted to the Dynamixel.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param length Length of the data for write
  /// @param data Data for write
  /// @param error Dynamixel hardware error
  /// @return communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int regWriteTxRx    (PortHandler *port, uint8_t id, uint16_t address, uint16_t length, uint8_t *data, uint8_t *error = 0) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_SYNC_READ instruction packet
  /// @description The function makes an instruction packet with INST_SYNC_READ,
  /// @description transmits the packet with PacketHandler::txPacket().
  /// @param port PortHandler instance
  /// @param start_address Address of the data for Sync Read
  /// @param data_length Length of the data for Sync Read
  /// @param param Parameter for Sync Read
  /// @param param_length Length of the data for Sync Read
  /// @return communication results which come from PacketHandler::txPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int syncReadTx      (PortHandler *port, uint16_t start_address, uint16_t data_length, uint8_t *param, uint16_t param_length) = 0;
  // SyncReadRx   -> GroupSyncRead class
  // SyncReadTxRx -> GroupSyncRead class

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_SYNC_WRITE instruction packet
  /// @description The function makes an instruction packet with INST_SYNC_WRITE,
  /// @description transmits the packet with PacketHandler::txRxPacket().
  /// @param port PortHandler instance
  /// @param start_address Address of the data for Sync Write
  /// @param data_length Length of the data for Sync Write
  /// @param param Parameter for Sync Write
  /// @param param_length Length of the data for Sync Write
  /// @return communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int syncWriteTxOnly (PortHandler *port, uint16_t start_address, uint16_t data_length, uint8_t *param, uint16_t param_length) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_BULK_READ instruction packet
  /// @description The function makes an instruction packet with INST_BULK_READ,
  /// @description transmits the packet with PacketHandler::txPacket().
  /// @param port PortHandler instance
  /// @param param Parameter for Bulk Read
  /// @param param_length Length of the data for Bulk Read
  /// @return communication results which come from PacketHandler::txPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int bulkReadTx      (PortHandler *port, uint8_t *param, uint16_t param_length) = 0;
  // BulkReadRx   -> GroupBulkRead class
  // BulkReadTxRx -> GroupBulkRead class

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_BULK_WRITE instruction packet
  /// @description The function makes an instruction packet with INST_BULK_WRITE,
  /// @description transmits the packet with PacketHandler::txRxPacket().
  /// @param port PortHandler instance
  /// @param param Parameter for Bulk Write
  /// @param param_length Length of the data for Bulk Write
  /// @return communication results which come from PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  virtual int bulkWriteTxOnly (PortHandler *port, uint8_t *param, uint16_t param_length) = 0;
};

}


#endif /* DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_PACKETHANDLER_H_ */
