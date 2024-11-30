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

/* Author: Ryu Woon Jung (Leon) */

#if defined(_WIN32) || defined(_WIN64)
#define WINDLLEXPORT

#include "port_handler_windows.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define LATENCY_TIMER  16 // msec (USB latency timer)
                          // You should adjust the latency timer value. In Windows, the default latency timer of the usb serial is '16 msec'.
                          // When you are going to use sync / bulk read, the latency timer should be loosen.
                          // the lower latency timer value, the faster communication speed.

                          // Note:
                          // You can either checking or changing its value by:
                          // [Device Manager] -> [Port (COM & LPT)] -> the port you use but starts with COMx-> mouse right click -> properties
                          // -> [port settings] -> [details] -> change response time from 16 to the value you need

using namespace dynamixel;

PortHandlerWindows::PortHandlerWindows(const char *port_name)
  : serial_handle_(INVALID_HANDLE_VALUE),
  baudrate_(DEFAULT_BAUDRATE_),
  packet_start_time_(0.0),
  packet_timeout_(0.0),
  tx_time_per_byte_(0.0)
{
  is_using_ = false;

  char buffer[15];
  sprintf_s(buffer, sizeof(buffer), "\\\\.\\%s", port_name);
  setPortName(buffer);
}

bool PortHandlerWindows::openPort()
{
  return setBaudRate(baudrate_);
}

void PortHandlerWindows::closePort()
{
  if (serial_handle_ != INVALID_HANDLE_VALUE)
  {
    CloseHandle(serial_handle_);
    serial_handle_ = INVALID_HANDLE_VALUE;
  }
}

void PortHandlerWindows::clearPort()
{
  PurgeComm(serial_handle_, PURGE_RXABORT | PURGE_RXCLEAR);
}

void PortHandlerWindows::setPortName(const char *port_name)
{
  strcpy_s(port_name_, sizeof(port_name_), port_name);
}

char *PortHandlerWindows::getPortName()
{
  return port_name_;
}

bool PortHandlerWindows::setBaudRate(const int baudrate)
{
  closePort();

  baudrate_ = baudrate;
  return setupPort(baudrate);
}

int PortHandlerWindows::getBaudRate()
{
  return baudrate_;
}

int PortHandlerWindows::getBytesAvailable()
{
  DWORD retbyte = 2;
  BOOL res = DeviceIoControl(serial_handle_, GENERIC_READ | GENERIC_WRITE, NULL, 0, 0, 0, &retbyte, (LPOVERLAPPED)NULL);

  printf("%d", (int)res);
  return (int)retbyte;
}

int PortHandlerWindows::readPort(uint8_t *packet, int length)
{
  DWORD dwRead = 0;

  if (ReadFile(serial_handle_, packet, (DWORD)length, &dwRead, NULL) == FALSE)
    return -1;

  return (int)dwRead;
}

int PortHandlerWindows::writePort(uint8_t *packet, int length)
{
  DWORD dwWrite = 0;

  if (WriteFile(serial_handle_, packet, (DWORD)length, &dwWrite, NULL) == FALSE)
    return -1;

  return (int)dwWrite;
}

void PortHandlerWindows::setPacketTimeout(uint16_t packet_length)
{
  packet_start_time_ = getCurrentTime();
  packet_timeout_ = (tx_time_per_byte_ * (double)packet_length) + (LATENCY_TIMER * 2.0) + 2.0;
}

void PortHandlerWindows::setPacketTimeout(double msec)
{
  packet_start_time_ = getCurrentTime();
  packet_timeout_ = msec;
}

bool PortHandlerWindows::isPacketTimeout()
{
  if (getTimeSinceStart() > packet_timeout_)
  {
    packet_timeout_ = 0;
    return true;
  }
  return false;
}

double PortHandlerWindows::getCurrentTime()
{
  QueryPerformanceCounter(&counter_);
  QueryPerformanceFrequency(&freq_);
  return (double)counter_.QuadPart / (double)freq_.QuadPart * 1000.0;
}

double PortHandlerWindows::getTimeSinceStart()
{
  double time;

  time = getCurrentTime() - packet_start_time_;
  if (time < 0.0) packet_start_time_ = getCurrentTime();

  return time;
}

bool PortHandlerWindows::setupPort(int baudrate)
{
  DCB dcb;
  COMMTIMEOUTS timeouts;
  DWORD dwError;

  closePort();

  serial_handle_ = CreateFileA(port_name_, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (serial_handle_ == INVALID_HANDLE_VALUE)
  {
    printf("[PortHandlerWindows::SetupPort] Error opening serial port!\n");
    return false;
  }

  dcb.DCBlength = sizeof(DCB);
  if (GetCommState(serial_handle_, &dcb) == FALSE)
    goto DXL_HAL_OPEN_ERROR;

  // Set baudrate
  dcb.BaudRate = (DWORD)baudrate;
  dcb.ByteSize = 8;                    // Data bit = 8bit
  dcb.Parity = NOPARITY;             // No parity
  dcb.StopBits = ONESTOPBIT;           // Stop bit = 1
  dcb.fParity = NOPARITY;             // No Parity check
  dcb.fBinary = 1;                    // Binary mode
  dcb.fNull = 0;                    // Get Null byte
  dcb.fAbortOnError = 0;
  dcb.fErrorChar = 0;
  // Not using XOn/XOff
  dcb.fOutX = 0;
  dcb.fInX = 0;
  // Not using H/W flow control
  dcb.fDtrControl = DTR_CONTROL_DISABLE;
  dcb.fRtsControl = RTS_CONTROL_DISABLE;
  dcb.fDsrSensitivity = 0;
  dcb.fOutxDsrFlow = 0;
  dcb.fOutxCtsFlow = 0;

  if (SetCommState(serial_handle_, &dcb) == FALSE)
    goto DXL_HAL_OPEN_ERROR;

  if (SetCommMask(serial_handle_, 0) == FALSE) // Not using Comm event
    goto DXL_HAL_OPEN_ERROR;
  if (SetupComm(serial_handle_, 4096, 4096) == FALSE) // Buffer size (Rx,Tx)
    goto DXL_HAL_OPEN_ERROR;
  if (PurgeComm(serial_handle_, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR) == FALSE) // Clear buffer
    goto DXL_HAL_OPEN_ERROR;
  if (ClearCommError(serial_handle_, &dwError, NULL) == FALSE)
    goto DXL_HAL_OPEN_ERROR;

  if (GetCommTimeouts(serial_handle_, &timeouts) == FALSE)
    goto DXL_HAL_OPEN_ERROR;
  // Timeout (Not using timeout)
  // Immediatly return
  timeouts.ReadIntervalTimeout = 0;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant = 1; // must not be zero.
  timeouts.WriteTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = 0;
  if (SetCommTimeouts(serial_handle_, &timeouts) == FALSE)
    goto DXL_HAL_OPEN_ERROR;

  tx_time_per_byte_ = (1000.0 / (double)baudrate_) * 10.0;
  return true;

DXL_HAL_OPEN_ERROR:
  closePort();
  return false;
}

#endif
