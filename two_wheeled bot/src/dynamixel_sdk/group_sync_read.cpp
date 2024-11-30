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

#include <algorithm>

#if defined(__linux__)
#include "group_sync_read.h"
#elif defined(__APPLE__)
#include "group_sync_read.h"
#elif defined(_WIN32) || defined(_WIN64)
#define WINDLLEXPORT
#include "group_sync_read.h"
#elif defined(ARDUINO) || defined(__OPENCR__) || defined(__OPENCM904__)
#include "../../include/dynamixel_sdk/group_sync_read.h"
#endif

using namespace dynamixel;

GroupSyncRead::GroupSyncRead(PortHandler *port, PacketHandler *ph, uint16_t start_address, uint16_t data_length)
  : port_(port),
    ph_(ph),
    last_result_(false),
    is_param_changed_(false),
    param_(0),
    start_address_(start_address),
    data_length_(data_length)
{
  clearParam();
}

void GroupSyncRead::makeParam()
{
  if (ph_->getProtocolVersion() == 1.0 || id_list_.size() == 0)
    return;

  if (param_ != 0)
    delete[] param_;
  param_ = 0;

  param_ = new uint8_t[id_list_.size() * 1];  // ID(1)

  int idx = 0;
  for (unsigned int i = 0; i < id_list_.size(); i++)
    param_[idx++] = id_list_[i];
}

bool GroupSyncRead::addParam(uint8_t id)
{
  if (ph_->getProtocolVersion() == 1.0)
    return false;

  if (std::find(id_list_.begin(), id_list_.end(), id) != id_list_.end())   // id already exist
    return false;

  id_list_.push_back(id);
  data_list_[id] = new uint8_t[data_length_];
  error_list_[id] = new uint8_t[1];

  is_param_changed_   = true;
  return true;
}
void GroupSyncRead::removeParam(uint8_t id)
{
  if (ph_->getProtocolVersion() == 1.0)
    return;

  std::vector<uint8_t>::iterator it = std::find(id_list_.begin(), id_list_.end(), id);
  if (it == id_list_.end())    // NOT exist
    return;

  id_list_.erase(it);
  delete[] data_list_[id];
  delete[] error_list_[id];
  data_list_.erase(id);
  error_list_.erase(id);

  is_param_changed_   = true;
}
void GroupSyncRead::clearParam()
{
  if (ph_->getProtocolVersion() == 1.0 || id_list_.size() == 0)
    return;

  for (unsigned int i = 0; i < id_list_.size(); i++)
  {
    delete[] data_list_[id_list_[i]];
    delete[] error_list_[id_list_[i]];
  }

  id_list_.clear();
  data_list_.clear();
  error_list_.clear();
  if (param_ != 0)
    delete[] param_;
  param_ = 0;
}

int GroupSyncRead::txPacket()
{
  if (ph_->getProtocolVersion() == 1.0 || id_list_.size() == 0)
    return COMM_NOT_AVAILABLE;

  if (is_param_changed_ == true || param_ == 0)
    makeParam();

  return ph_->syncReadTx(port_, start_address_, data_length_, param_, (uint16_t)id_list_.size() * 1);
}

int GroupSyncRead::rxPacket()
{
  last_result_ = false;

  if (ph_->getProtocolVersion() == 1.0)
    return COMM_NOT_AVAILABLE;

  int cnt            = id_list_.size();
  int result         = COMM_RX_FAIL;

  if (cnt == 0)
    return COMM_NOT_AVAILABLE;

  for (int i = 0; i < cnt; i++)
  {
    uint8_t id = id_list_[i];

    result = ph_->readRx(port_, id, data_length_, data_list_[id], error_list_[id]);
    if (result != COMM_SUCCESS)
      return result;
  }

  if (result == COMM_SUCCESS)
    last_result_ = true;

  return result;
}

int GroupSyncRead::txRxPacket()
{
  if (ph_->getProtocolVersion() == 1.0)
    return COMM_NOT_AVAILABLE;

  int result         = COMM_TX_FAIL;

  result = txPacket();
  if (result != COMM_SUCCESS)
    return result;

  return rxPacket();
}

bool GroupSyncRead::isAvailable(uint8_t id, uint16_t address, uint16_t data_length)
{
  if (ph_->getProtocolVersion() == 1.0 || last_result_ == false || data_list_.find(id) == data_list_.end())
    return false;

  if (address < start_address_ || start_address_ + data_length_ - data_length < address)
    return false;

  return true;
}

uint32_t GroupSyncRead::getData(uint8_t id, uint16_t address, uint16_t data_length)
{
  if (isAvailable(id, address, data_length) == false)
    return 0;

  switch(data_length)
  {
    case 1:
      return data_list_[id][address - start_address_];

    case 2:
      return DXL_MAKEWORD(data_list_[id][address - start_address_], data_list_[id][address - start_address_ + 1]);

    case 4:
      return DXL_MAKEDWORD(DXL_MAKEWORD(data_list_[id][address - start_address_ + 0], data_list_[id][address - start_address_ + 1]),
                 DXL_MAKEWORD(data_list_[id][address - start_address_ + 2], data_list_[id][address - start_address_ + 3]));

    default:
      return 0;
  }
}

bool GroupSyncRead::getError(uint8_t id, uint8_t* error)
{
  // TODO : check protocol version, last_result_, data_list
  // if (ph_->getProtocolVersion() == 1.0 || last_result_ == false || error_list_.find(id) == error_list_.end())

  return error[0] = error_list_[id][0];
}