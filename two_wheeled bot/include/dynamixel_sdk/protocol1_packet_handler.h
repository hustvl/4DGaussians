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
/// @file The file for Protocol 1.0 Dynamixel packet control
/// @author Zerom, Leon (RyuWoon Jung)
////////////////////////////////////////////////////////////////////////////////

#ifndef DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_PROTOCOL1PACKETHANDLER_H_
#define DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_PROTOCOL1PACKETHANDLER_H_


#include "packet_handler.h"

namespace dynamixel
{

////////////////////////////////////////////////////////////////////////////////
/// @brief The class for control Dynamixel by using Protocol1.0
////////////////////////////////////////////////////////////////////////////////
class WINDECLSPEC Protocol1PacketHandler : public PacketHandler
{
 private:
  static Protocol1PacketHandler *unique_instance_;

  Protocol1PacketHandler();

 public:
  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that returns Protocol1PacketHandler instance
  /// @return Protocol1PacketHandler instance
  ////////////////////////////////////////////////////////////////////////////////
  static Protocol1PacketHandler *getInstance() { return unique_instance_; }

  virtual ~Protocol1PacketHandler() { }

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that returns Protocol version used in Protocol1PacketHandler (1.0)
  /// @return 1.0
  ////////////////////////////////////////////////////////////////////////////////
  float   getProtocolVersion() { return 1.0; }

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that gets description of communication result
  /// @param result Communication result which might be gotten by the tx rx functions
  /// @return description of communication result in const char* (string)
  ////////////////////////////////////////////////////////////////////////////////
  const char *getTxRxResult     (int result);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that gets description of hardware error
  /// @param error Dynamixel hardware error which might be gotten by the tx rx functions
  /// @return description of hardware error in const char* (string)
  ////////////////////////////////////////////////////////////////////////////////
  const char *getRxPacketError  (uint8_t error);

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
  int txPacket        (PortHandler *port, uint8_t *txpacket);

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
  int rxPacket        (PortHandler *port, uint8_t *rxpacket);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits packet (txpacket) and receives packet (rxpacket) during designated time via PortHandler port
  /// @description The function calls Protocol1PacketHandler::txPacket(),
  /// @description and calls Protocol1PacketHandler::rxPacket() if it succeeds Protocol1PacketHandler::txPacket().
  /// @description It breaks out
  /// @description when it fails Protocol1PacketHandler::txPacket(),
  /// @description when txpacket is called by Protocol1PacketHandler::broadcastPing() / Protocol1PacketHandler::syncWriteTxOnly() / Protocol1PacketHandler::regWriteTxOnly / Protocol1PacketHandler::action
  /// @param port PortHandler instance
  /// @param txpacket packet for transmission
  /// @param rxpacket received packet
  /// @return COMM_SUCCESS
  /// @return   when it succeeds Protocol1PacketHandler::txPacket() and Protocol1PacketHandler::rxPacket()
  /// @return or the other communication results which come from Protocol1PacketHandler::txPacket() and Protocol1PacketHandler::rxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int txRxPacket      (PortHandler *port, uint8_t *txpacket, uint8_t *rxpacket, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that pings Dynamixel but doesn't take its model number
  /// @description The function calls Protocol1PacketHandler::ping() which gets Dynamixel model number,
  /// @description but doesn't carry the model number
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::ping()
  ////////////////////////////////////////////////////////////////////////////////
  int ping            (PortHandler *port, uint8_t id, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that pings Dynamixel and takes its model number
  /// @description The function makes an instruction packet with INST_PING,
  /// @description transmits the packet with Protocol1PacketHandler::txRxPacket(),
  /// @description and call Protocol1PacketHandler::readTxRx to read model_number in the rx buffer.
  /// @description It breaks out
  /// @description when it tries to transmit to BROADCAST_ID
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param error Dynamixel hardware error
  /// @return COMM_NOT_AVAILABLE
  /// @return   when it tries to transmit to BROADCAST_ID
  /// @return COMM_SUCCESS
  /// @return   when it succeeds to ping Dynamixel and get model_number from it
  /// @return or the other communication results which come from Protocol1PacketHandler::txRxPacket() and Protocol1PacketHandler::readTxRx()
  ////////////////////////////////////////////////////////////////////////////////
  int ping            (PortHandler *port, uint8_t id, uint16_t *model_number, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief (Available only in Protocol 2.0) The function that pings all connected Dynamixel
  /// @param port PortHandler instance
  /// @param id_list ID list of Dynamixels which are found by broadcast ping
  /// @return COMM_NOT_AVAILABLE
  ////////////////////////////////////////////////////////////////////////////////
  int broadcastPing   (PortHandler *port, std::vector<uint8_t> &id_list);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that makes Dynamixels run as written in the Dynamixel register
  /// @description The function makes an instruction packet with INST_ACTION,
  /// @description transmits the packet with Protocol1PacketHandler::txRxPacket().
  /// @description To use this function, Dynamixel register should be set by Protocol1PacketHandler::regWriteTxOnly() or Protocol1PacketHandler::regWriteTxRx()
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @return communication results which come from Protocol1PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int action          (PortHandler *port, uint8_t id);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief (Available only in Protocol 2.0) The function that makes Dynamixel reboot
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param error Dynamixel hardware error
  /// @return COMM_NOT_AVAILABLE
  ////////////////////////////////////////////////////////////////////////////////
  int reboot          (PortHandler *port, uint8_t id, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief (Available only in Protocol 2.0) The function that reset multi-turn revolution information of Dynamixel
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param error Dynamixel hardware error
  /// @return COMM_NOT_AVAILABLE
  ////////////////////////////////////////////////////////////////////////////////
  int clearMultiTurn  (PortHandler *port, uint8_t id, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that makes Dynamixel reset as it was produced in the factory
  /// @description The function makes an instruction packet with INST_FACTORY_RESET,
  /// @description transmits the packet with Protocol1PacketHandler::txRxPacket().
  /// @description Be careful of the use.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param option (Not available in Protocol 1.0) Reset option
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int factoryReset    (PortHandler *port, uint8_t id, uint8_t option, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_READ instruction packet
  /// @description The function makes an instruction packet with INST_READ,
  /// @description transmits the packet with Protocol1PacketHandler::txPacket().
  /// @description It breaks out
  /// @description when it tries to transmit to BROADCAST_ID
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @param length Length of the data for read
  /// @return COMM_NOT_AVAILABLE
  /// @return   when it tries to transmit to BROADCAST_ID
  /// @return or the other communication results which come from Protocol1PacketHandler::txPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int readTx          (PortHandler *port, uint8_t id, uint16_t address, uint16_t length);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that receives the packet and reads the data in the packet
  /// @description The function receives the packet which might be come by previous INST_READ instruction packet transmission,
  /// @description gets the data from the packet.
  /// @param port PortHandler instance
  /// @param length Length of the data for read
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::rxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int readRx          (PortHandler *port, uint8_t id, uint16_t length, uint8_t *data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_READ instruction packet, and read data from received packet
  /// @description The function makes an instruction packet with INST_READ,
  /// @description transmits and receives the packet with Protocol1PacketHandler::txRxPacket(),
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
  /// @return or the other communication results which come from Protocol1PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int readTxRx        (PortHandler *port, uint8_t id, uint16_t address, uint16_t length, uint8_t *data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::readTx() function for reading 1 byte data
  /// @description The function calls Protocol1PacketHandler::readTx() function for reading 1 byte data
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @return communication results which come from Protocol1PacketHandler::readTx()
  ////////////////////////////////////////////////////////////////////////////////
  int read1ByteTx     (PortHandler *port, uint8_t id, uint16_t address);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::readRx() function and reads 1 byte data on the packet
  /// @description The function calls Protocol1PacketHandler::readRx() function,
  /// @description gets 1 byte data from the packet.
  /// @param port PortHandler instance
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::readRx()
  ////////////////////////////////////////////////////////////////////////////////
  int read1ByteRx     (PortHandler *port, uint8_t id, uint8_t *data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::readTxRx() function for reading 1 byte data
  /// @description The function calls Protocol1PacketHandler::readTxRx(),
  /// @description gets 1 byte data from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @param length Length of the data for read
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int read1ByteTxRx       (PortHandler *port, uint8_t id, uint16_t address, uint8_t *data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::readTx() function for reading 2 byte data
  /// @description The function calls Protocol1PacketHandler::readTx() function for reading 2 byte data
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @return communication results which come from Protocol1PacketHandler::readTx()
  ////////////////////////////////////////////////////////////////////////////////
  int read2ByteTx     (PortHandler *port, uint8_t id, uint16_t address);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::readRx() function and reads 2 byte data on the packet
  /// @description The function calls Protocol1PacketHandler::readRx() function,
  /// @description gets 2 byte data from the packet.
  /// @param port PortHandler instance
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::readRx()
  ////////////////////////////////////////////////////////////////////////////////
  int read2ByteRx     (PortHandler *port, uint8_t id, uint16_t *data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::readTxRx() function for reading 2 byte data
  /// @description The function calls Protocol1PacketHandler::readTxRx(),
  /// @description gets 2 byte data from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @param length Length of the data for read
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int read2ByteTxRx       (PortHandler *port, uint8_t id, uint16_t address, uint16_t *data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::readTx() function for reading 4 byte data
  /// @description The function calls Protocol1PacketHandler::readTx() function for reading 4 byte data
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @return communication results which come from Protocol1PacketHandler::readTx()
  ////////////////////////////////////////////////////////////////////////////////
  int read4ByteTx     (PortHandler *port, uint8_t id, uint16_t address);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::readRx() function and reads 4 byte data on the packet
  /// @description The function calls Protocol1PacketHandler::readRx() function,
  /// @description gets 4 byte data from the packet.
  /// @param port PortHandler instance
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::readRx()
  ////////////////////////////////////////////////////////////////////////////////
  int read4ByteRx     (PortHandler *port, uint8_t id, uint32_t *data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::readTxRx() function for reading 4 byte data
  /// @description The function calls Protocol1PacketHandler::readTxRx(),
  /// @description gets 4 byte data from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for read
  /// @param length Length of the data for read
  /// @param data Data extracted from the packet
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int read4ByteTxRx       (PortHandler *port, uint8_t id, uint16_t address, uint32_t *data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_WRITE instruction packet with the data for write
  /// @description The function makes an instruction packet with INST_WRITE and the data for write,
  /// @description transmits the packet with Protocol1PacketHandler::txPacket().
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param length Length of the data for write
  /// @param data Data for write
  /// @return communication results which come from Protocol1PacketHandler::txPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int writeTxOnly     (PortHandler *port, uint8_t id, uint16_t address, uint16_t length, uint8_t *data);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_WRITE instruction packet with the data for write, and receives the packet
  /// @description The function makes an instruction packet with INST_WRITE and the data for write,
  /// @description transmits and receives the packet with Protocol1PacketHandler::txRxPacket(),
  /// @description gets the error from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param length Length of the data for write
  /// @param data Data for write
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int writeTxRx           (PortHandler *port, uint8_t id, uint16_t address, uint16_t length, uint8_t *data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::writeTxOnly() for writing 1 byte data
  /// @description The function calls Protocol1PacketHandler::writeTxOnly() for writing 1 byte data.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @return communication results which come from Protocol1PacketHandler::writeTxOnly()
  ////////////////////////////////////////////////////////////////////////////////
  int write1ByteTxOnly(PortHandler *port, uint8_t id, uint16_t address, uint8_t data);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::writeTxRx() for writing 1 byte data and receives the packet
  /// @description The function calls Protocol1PacketHandler::writeTxRx() for writing 1 byte data and receves the packet,
  /// @description gets the error from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::writeTxRx()
  ////////////////////////////////////////////////////////////////////////////////
  int write1ByteTxRx      (PortHandler *port, uint8_t id, uint16_t address, uint8_t data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::writeTxOnly() for writing 2 byte data
  /// @description The function calls Protocol1PacketHandler::writeTxOnly() for writing 2 byte data.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @return communication results which come from Protocol1PacketHandler::writeTxOnly()
  ////////////////////////////////////////////////////////////////////////////////
  int write2ByteTxOnly(PortHandler *port, uint8_t id, uint16_t address, uint16_t data);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::writeTxRx() for writing 2 byte data and receives the packet
  /// @description The function calls Protocol1PacketHandler::writeTxRx() for writing 2 byte data and receves the packet,
  /// @description gets the error from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::writeTxRx()
  ////////////////////////////////////////////////////////////////////////////////
  int write2ByteTxRx      (PortHandler *port, uint8_t id, uint16_t address, uint16_t data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::writeTxOnly() for writing 4 byte data
  /// @description The function calls Protocol1PacketHandler::writeTxOnly() for writing 4 byte data.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @return communication results which come from Protocol1PacketHandler::writeTxOnly()
  ////////////////////////////////////////////////////////////////////////////////
  int write4ByteTxOnly(PortHandler *port, uint8_t id, uint16_t address, uint32_t data);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls Protocol1PacketHandler::writeTxRx() for writing 4 byte data and receives the packet
  /// @description The function calls Protocol1PacketHandler::writeTxRx() for writing 4 byte data and receves the packet,
  /// @description gets the error from the packet.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param data Data for write
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::writeTxRx()
  ////////////////////////////////////////////////////////////////////////////////
  int write4ByteTxRx      (PortHandler *port, uint8_t id, uint16_t address, uint32_t data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_REG_WRITE instruction packet with the data for writing on the Dynamixel register
  /// @description The function makes an instruction packet with INST_REG_WRITE and the data for writing on the Dynamixel register,
  /// @description transmits the packet with Protocol1PacketHandler::txPacket().
  /// @description The data written in the register will act when INST_ACTION instruction packet is transmitted to the Dynamixel.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param length Length of the data for write
  /// @param data Data for write
  /// @return communication results which come from Protocol1PacketHandler::txPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int regWriteTxOnly  (PortHandler *port, uint8_t id, uint16_t address, uint16_t length, uint8_t *data);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits INST_REG_WRITE instruction packet with the data for writing on the Dynamixel register, and receives the packet
  /// @description The function makes an instruction packet with INST_REG_WRITE and the data for writing on the Dynamixel register,
  /// @description transmits and receives the packet with Protocol1PacketHandler::txRxPacket(),
  /// @description gets the error from the packet.
  /// @description The data written in the register will act when INST_ACTION instruction packet is transmitted to the Dynamixel.
  /// @param port PortHandler instance
  /// @param id Dynamixel ID
  /// @param address Address of the data for write
  /// @param length Length of the data for write
  /// @param data Data for write
  /// @param error Dynamixel hardware error
  /// @return communication results which come from Protocol1PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int regWriteTxRx        (PortHandler *port, uint8_t id, uint16_t address, uint16_t length, uint8_t *data, uint8_t *error = 0);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief (Available only in Protocol 2.0) The function that transmits Sync Read instruction packet
  /// @param port PortHandler instance
  /// @param start_address Address of the data for Sync Read
  /// @param data_length Length of the data for Sync Read
  /// @param param Parameter for Sync Read
  /// @param param_length Length of the data for Sync Read
  /// @return COMM_NOT_AVAILABLE
  ////////////////////////////////////////////////////////////////////////////////
  int syncReadTx      (PortHandler *port, uint16_t start_address, uint16_t data_length, uint8_t *param, uint16_t param_length);
  // SyncReadRx   -> GroupSyncRead class
  // SyncReadTxRx -> GroupSyncRead class

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits Sync Write instruction packet
  /// @description The function makes an instruction packet with INST_SYNC_WRITE,
  /// @description transmits the packet with Protocol1PacketHandler::txRxPacket().
  /// @param port PortHandler instance
  /// @param start_address Address of the data for Sync Write
  /// @param data_length Length of the data for Sync Write
  /// @param param Parameter for Sync Write {ID1, DATA0, DATA1, ..., DATAn, ID2, DATA0, DATA1, ..., DATAn, ID3, DATA0, DATA1, ..., DATAn}
  /// @param param_length Length of the data for Sync Write
  /// @return communication results which come from Protocol1PacketHandler::txRxPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int syncWriteTxOnly (PortHandler *port, uint16_t start_address, uint16_t data_length, uint8_t *param, uint16_t param_length);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief (Available only on Dynamixel MX / X series) The function that transmits Bulk Read instruction packet
  /// @description The function makes an instruction packet with INST_BULK_READ,
  /// @description transmits the packet with Protocol1PacketHandler::txPacket().
  /// @param port PortHandler instance
  /// @param param Parameter for Bulk Read {LEN1, ID1, ADDR1, LEN2, ID2, ADDR2, ...}
  /// @param param_length Length of the data for Bulk Read
  /// @return communication results which come from Protocol1PacketHandler::txPacket()
  ////////////////////////////////////////////////////////////////////////////////
  int bulkReadTx      (PortHandler *port, uint8_t *param, uint16_t param_length);
  // BulkReadRx   -> GroupBulkRead class
  // BulkReadTxRx -> GroupBulkRead class

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief (Available only in Protocol 2.0) The function that transmits Bulk Write instruction packet
  /// @param port PortHandler instance
  /// @param param Parameter for Bulk Write
  /// @param param_length Length of the data for Bulk Write
  /// @return COMM_NOT_AVAILABLE
  ////////////////////////////////////////////////////////////////////////////////
  int bulkWriteTxOnly (PortHandler *port, uint8_t *param, uint16_t param_length);
};

}


#endif /* DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_PROTOCOL1PACKETHANDLER_H_ */
