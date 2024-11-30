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
/// @file The file for Dynamixel Bulk Write
/// @author Zerom, Leon (RyuWoon Jung)
////////////////////////////////////////////////////////////////////////////////

#ifndef DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_GROUPBULKWRITE_H_
#define DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_GROUPBULKWRITE_H_


#include <map>
#include <vector>
#include "port_handler.h"
#include "packet_handler.h"

namespace dynamixel
{

////////////////////////////////////////////////////////////////////////////////
/// @brief The class for writing multiple Dynamixel data from different addresses with different lengths at once
////////////////////////////////////////////////////////////////////////////////
class WINDECLSPEC GroupBulkWrite
{
 private:
  PortHandler    *port_;
  PacketHandler  *ph_;

  std::vector<uint8_t>            id_list_;
  std::map<uint8_t, uint16_t>     address_list_;  // <id, start_address>
  std::map<uint8_t, uint16_t>     length_list_;   // <id, data_length>
  std::map<uint8_t, uint8_t *>    data_list_;     // <id, data>

  bool            is_param_changed_;

  uint8_t        *param_;
  uint16_t        param_length_;

  void    makeParam();

 public:
  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that Initializes instance for Bulk Write
  /// @param port PortHandler instance
  /// @param ph PacketHandler instance
  ////////////////////////////////////////////////////////////////////////////////
  GroupBulkWrite(PortHandler *port, PacketHandler *ph);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that calls clearParam function to clear the parameter list for Bulk Write
  ////////////////////////////////////////////////////////////////////////////////
  ~GroupBulkWrite() { clearParam(); }

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that returns PortHandler instance
  /// @return PortHandler instance
  ////////////////////////////////////////////////////////////////////////////////
  PortHandler     *getPortHandler()   { return port_; }

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that returns PacketHandler instance
  /// @return PacketHandler instance
  ////////////////////////////////////////////////////////////////////////////////
  PacketHandler   *getPacketHandler() { return ph_; }

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that adds id, start_address, data_length to the Bulk Write list
  /// @param id Dynamixel ID
  /// @param start_address Address of the data for write
  /// @param data_length Length of the data for write
  /// @return false
  /// @return   when the ID exists already in the list
  /// @return or true
  ////////////////////////////////////////////////////////////////////////////////
  bool    addParam    (uint8_t id, uint16_t start_address, uint16_t data_length, uint8_t *data);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that removes id from the Bulk Write list
  /// @param id Dynamixel ID
  ////////////////////////////////////////////////////////////////////////////////
  void    removeParam (uint8_t id);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that changes the data for write in id -> start_address -> data_length to the Bulk Write list
  /// @param id Dynamixel ID
  /// @param start_address Address of the data for write
  /// @param data_length Length of the data for write
  /// @param data for replacement
  /// @return false
  /// @return   when the ID doesn't exist in the list
  /// @return or true
  ////////////////////////////////////////////////////////////////////////////////
  bool    changeParam (uint8_t id, uint16_t start_address, uint16_t data_length, uint8_t *data);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that clears the Bulk Write list
  ////////////////////////////////////////////////////////////////////////////////
  void    clearParam  ();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief The function that transmits the Bulk Write instruction packet which might be constructed by GroupBulkWrite::addParam function
  /// @return COMM_NOT_AVAILABLE
  /// @return   when the list for Bulk Write is empty
  /// @return   when Protocol1.0 has been used
  /// @return or the other communication results which come from PacketHandler::bulkWriteTxOnly
  ////////////////////////////////////////////////////////////////////////////////
  int     txPacket();
};

}


#endif /* DYNAMIXEL_SDK_INCLUDE_DYNAMIXEL_SDK_GROUPBULKWRITE_H_ */
