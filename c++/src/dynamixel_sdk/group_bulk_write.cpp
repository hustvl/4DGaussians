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
#include "group_bulk_write.h"
#elif defined(__APPLE__)
#include "group_bulk_write.h"
#elif defined(_WIN32) || defined(_WIN64)
#define WINDLLEXPORT
#include "group_bulk_write.h"
#elif defined(ARDUINO) || defined(__OPENCR__) || defined(__OPENCM904__)
#include "../../include/dynamixel_sdk/group_bulk_write.h"
#endif

using namespace dynamixel;

GroupBulkWrite::GroupBulkWrite(PortHandler *port, PacketHandler *ph)
  : port_(port),
    ph_(ph),
    is_param_changed_(false),
    param_(0),
    param_length_(0)
{
  clearParam();
}

void GroupBulkWrite::makeParam()
{
  if (ph_->getProtocolVersion() == 1.0 || id_list_.size() == 0)
    return;

  if (param_ != 0)
    delete[] param_;
  param_ = 0;

  param_length_ = 0;
  for (unsigned int i = 0; i < id_list_.size(); i++)
    param_length_ += 1 + 2 + 2 + length_list_[id_list_[i]];

  param_ = new uint8_t[param_length_];

  int idx = 0;
  for (unsigned int i = 0; i < id_list_.size(); i++)
  {
    uint8_t id = id_list_[i];
    if (data_list_[id] == 0)
      return;

    param_[idx++] = id;
    param_[idx++] = DXL_LOBYTE(address_list_[id]);
    param_[idx++] = DXL_HIBYTE(address_list_[id]);
    param_[idx++] = DXL_LOBYTE(length_list_[id]);
    param_[idx++] = DXL_HIBYTE(length_list_[id]);
    for (int c = 0; c < length_list_[id]; c++)
      param_[idx++] = (data_list_[id])[c];
  }
}

bool GroupBulkWrite::addParam(uint8_t id, uint16_t start_address, uint16_t data_length, uint8_t *data)
{
  if (ph_->getProtocolVersion() == 1.0)
    return false;

  if (std::find(id_list_.begin(), id_list_.end(), id) != id_list_.end())   // id already exist
    return false;

  id_list_.push_back(id);
  address_list_[id]   = start_address;
  length_list_[id]    = data_length;
  data_list_[id]      = new uint8_t[data_length];
  for (int c = 0; c < data_length; c++)
    data_list_[id][c] = data[c];

  is_param_changed_   = true;
  return true;
}
void GroupBulkWrite::removeParam(uint8_t id)
{
  if (ph_->getProtocolVersion() == 1.0)
    return;

  std::vector<uint8_t>::iterator it = std::find(id_list_.begin(), id_list_.end(), id);
  if (it == id_list_.end())    // NOT exist
    return;

  id_list_.erase(it);
  address_list_.erase(id);
  length_list_.erase(id);
  delete[] data_list_[id];
  data_list_.erase(id);

  is_param_changed_   = true;
}
bool GroupBulkWrite::changeParam(uint8_t id, uint16_t start_address, uint16_t data_length, uint8_t *data)
{
  if (ph_->getProtocolVersion() == 1.0)
    return false;

  std::vector<uint8_t>::iterator it = std::find(id_list_.begin(), id_list_.end(), id);
  if (it == id_list_.end())    // NOT exist
    return false;

  address_list_[id]   = start_address;
  length_list_[id]    = data_length;
  delete[] data_list_[id];
  data_list_[id]      = new uint8_t[data_length];
  for (int c = 0; c < data_length; c++)
    data_list_[id][c] = data[c];

  is_param_changed_   = true;
  return true;
}
void GroupBulkWrite::clearParam()
{
  if (ph_->getProtocolVersion() == 1.0 || id_list_.size() == 0)
    return;

  for (unsigned int i = 0; i < id_list_.size(); i++)
    delete[] data_list_[id_list_[i]];

  id_list_.clear();
  address_list_.clear();
  length_list_.clear();
  data_list_.clear();
  if (param_ != 0)
    delete[] param_;
  param_ = 0;
}
int GroupBulkWrite::txPacket()
{
  if (ph_->getProtocolVersion() == 1.0 || id_list_.size() == 0)
    return COMM_NOT_AVAILABLE;

  if (is_param_changed_ == true || param_ == 0)
    makeParam();

  return ph_->bulkWriteTxOnly(port_, param_, param_length_);
}
