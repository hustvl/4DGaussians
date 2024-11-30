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

/* Author: zerom, Ryu Woon Jung (Leon) */

#include <stdio.h>
#include <algorithm>

#if defined(__linux__)
#include "group_bulk_read.h"
#elif defined(__APPLE__)
#include "group_bulk_read.h"
#elif defined(_WIN32) || defined(_WIN64)
#define WINDLLEXPORT
#include "group_bulk_read.h"
#elif defined(ARDUINO) || defined(__OPENCR__) || defined(__OPENCM904__)
#include "../../include/dynamixel_sdk/group_bulk_read.h"
#endif

using namespace dynamixel;

GroupBulkRead::GroupBulkRead(PortHandler *port, PacketHandler *ph)
  : port_(port),
    ph_(ph),
    last_result_(false),
    is_param_changed_(false),
    param_(0)
{
  clearParam();
}

void GroupBulkRead::makeParam()
{
  if (id_list_.size() == 0)
    return;

  if (param_ != 0)
    delete[] param_;
  param_ = 0;

  if (ph_->getProtocolVersion() == 1.0)
  {
    param_ = new uint8_t[id_list_.size() * 3];  // ID(1) + ADDR(1) + LENGTH(1)
  }
  else    // 2.0
  {
    param_ = new uint8_t[id_list_.size() * 5];  // ID(1) + ADDR(2) + LENGTH(2)
  }

  int idx = 0;
  for (unsigned int i = 0; i < id_list_.size(); i++)
  {
    uint8_t id = id_list_[i];
    if (ph_->getProtocolVersion() == 1.0)
    {
      param_[idx++] = (uint8_t)length_list_[id];    // LEN
      param_[idx++] = id;                           // ID
      param_[idx++] = (uint8_t)address_list_[id];   // ADDR
    }
    else    // 2.0
    {
      param_[idx++] = id;                               // ID
      param_[idx++] = DXL_LOBYTE(address_list_[id]);    // ADDR_L
      param_[idx++] = DXL_HIBYTE(address_list_[id]);    // ADDR_H
      param_[idx++] = DXL_LOBYTE(length_list_[id]);     // LEN_L
      param_[idx++] = DXL_HIBYTE(length_list_[id]);     // LEN_H
    }
  }
}

bool GroupBulkRead::addParam(uint8_t id, uint16_t start_address, uint16_t data_length)
{
  if (std::find(id_list_.begin(), id_list_.end(), id) != id_list_.end())   // id already exist
    return false;

  id_list_.push_back(id);
  length_list_[id]    = data_length;
  address_list_[id]   = start_address;
  data_list_[id]      = new uint8_t[data_length];
  error_list_[id]     = new uint8_t[1];

  is_param_changed_   = true;
  return true;
}

void GroupBulkRead::removeParam(uint8_t id)
{
  std::vector<uint8_t>::iterator it = std::find(id_list_.begin(), id_list_.end(), id);
  if (it == id_list_.end())    // NOT exist
    return;

  id_list_.erase(it);
  address_list_.erase(id);
  length_list_.erase(id);
  delete[] data_list_[id];
  delete[] error_list_[id];
  data_list_.erase(id);
  error_list_.erase(id);

  is_param_changed_   = true;
}

void GroupBulkRead::clearParam()
{
  if (id_list_.size() == 0)
    return;

  for (unsigned int i = 0; i < id_list_.size(); i++)
  {
    delete[] data_list_[id_list_[i]];
    delete[] error_list_[id_list_[i]];
  }

  id_list_.clear();
  address_list_.clear();
  length_list_.clear();
  data_list_.clear();
  error_list_.clear();
  if (param_ != 0)
    delete[] param_;
  param_ = 0;
}

int GroupBulkRead::txPacket()
{
  if (id_list_.size() == 0)
    return COMM_NOT_AVAILABLE;

  if (is_param_changed_ == true || param_ == 0)
    makeParam();

  if (ph_->getProtocolVersion() == 1.0)
  {
    return ph_->bulkReadTx(port_, param_, id_list_.size() * 3);
  }
  else    // 2.0
  {
    return ph_->bulkReadTx(port_, param_, id_list_.size() * 5);
  }
}

int GroupBulkRead::rxPacket()
{
  int cnt            = id_list_.size();
  int result          = COMM_RX_FAIL;

  last_result_ = false;

  if (cnt == 0)
    return COMM_NOT_AVAILABLE;

  for (int i = 0; i < cnt; i++)
  {
    uint8_t id = id_list_[i];

    result = ph_->readRx(port_, id, length_list_[id], data_list_[id], error_list_[id]);
    if (result != COMM_SUCCESS)
      return result;
  }

  if (result == COMM_SUCCESS)
    last_result_ = true;

  return result;
}

int GroupBulkRead::txRxPacket()
{
  int result         = COMM_TX_FAIL;

  result = txPacket();
  if (result != COMM_SUCCESS)
    return result;

  return rxPacket();
}

bool GroupBulkRead::isAvailable(uint8_t id, uint16_t address, uint16_t data_length)
{
  uint16_t start_addr;

  if (last_result_ == false || data_list_.find(id) == data_list_.end())
    return false;

  start_addr = address_list_[id];

  if (address < start_addr || start_addr + length_list_[id] - data_length < address)
    return false;

  return true;
}

uint32_t GroupBulkRead::getData(uint8_t id, uint16_t address, uint16_t data_length)
{
  if (isAvailable(id, address, data_length) == false)
    return 0;

  uint16_t start_addr = address_list_[id];

  switch(data_length)
  {
    case 1:
      return data_list_[id][address - start_addr];

    case 2:
      return DXL_MAKEWORD(data_list_[id][address - start_addr], data_list_[id][address - start_addr + 1]);

    case 4:
      return DXL_MAKEDWORD(DXL_MAKEWORD(data_list_[id][address - start_addr + 0], data_list_[id][address - start_addr + 1]),
                           DXL_MAKEWORD(data_list_[id][address - start_addr + 2], data_list_[id][address - start_addr + 3]));

    default:
      return 0;
  }
}

bool GroupBulkRead::getError(uint8_t id, uint8_t* error)
{
  // TODO : check protocol version, last_result_, data_list
  // if (last_result_ == false || error_list_.find(id) == error_list_.end())

  return error[0] = error_list_[id][0];
}