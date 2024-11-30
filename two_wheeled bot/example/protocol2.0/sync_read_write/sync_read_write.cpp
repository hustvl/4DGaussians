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

//
// *********     Sync Read and Sync Write Example      *********
//
//
// Available Dynamixel model on this example : All models using Protocol 2.0
// This example is tested with two Dynamixel PRO 54-200, and an USB2DYNAMIXEL
// Be sure that Dynamixel PRO properties are already set as %% ID : 1 / Baudnum : 1 (Baudrate : 57600)
//

#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <termios.h>
#define STDIN_FILENO 0
#elif defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#include <string>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <thread>

#include "dynamixel_sdk.h"                                  // Uses Dynamixel SDK library

// Control table address
#define ADDR_PRO_TORQUE_ENABLE          64                 // Control table address is different in Dynamixel model
#define ADDR_PRO_GOAL_VELOCITY          104
#define ADDR_PRO_PRESENT_VELOCITY       128

// Data Byte Length
#define LEN_PRO_GOAL_VELOCITY           4
#define LEN_PRO_PRESENT_VELOCITY        4

// Protocol version
#define PROTOCOL_VERSION                2.0                 // See which protocol version is used in the Dynamixel

// Default setting
#define DXL1_ID                         1                   // Dynamixel#1 ID: 1
#define DXL2_ID                         2                   // Dynamixel#2 ID: 2
#define BAUDRATE                        2
#define DEVICENAME                      "COM3"      // Check which port is being used on your controller
                                                            // ex) Windows: "COM1"   Linux: "/dev/ttyUSB0" Mac: "/dev/tty.usbserial-*"

#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque
#define DXL_MINIMUM_VELOCITY_VALUE      0            
#define DXL_MAXIMUM_VELOCITY_VALUE      200                  // Real value = 265          
#define DXL_MOVING_STATUS_THRESHOLD     10                  // Dynamixel moving status threshold

#define ESC_ASCII_VALUE                 0x1b

int getch()
{
#if defined(__linux__) || defined(__APPLE__)
  struct termios oldt, newt;
  int ch;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return ch;
#elif defined(_WIN32) || defined(_WIN64)
  return _getch();
#endif
}

int kbhit(void)
{
#if defined(__linux__) || defined(__APPLE__)
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
#elif defined(_WIN32) || defined(_WIN64)
  return _kbhit();
#endif
}

int getInputVelocity(int dxl_goal_velocity[]) {
    int mode = 0;

    while (1) {
        printf("Enter the mode: (1) Forward/Backward, (2) Reciprocal Motion, (3) Rotation:\n");
        if (scanf("%d", &mode) != 1) {
            printf("Invalid input. Please enter a valid number.\n");
            continue;
        }

        if (mode == 1) {
            int velocity = 0;
            printf("Enter the desired velocity for the two-wheeled bot:\n");
            if (scanf("%d", &velocity) != 1 || abs(velocity) > DXL_MAXIMUM_VELOCITY_VALUE) {
                printf("Invalid input. Please enter a valid velocity within range [%d, %d].\n",
                    DXL_MINIMUM_VELOCITY_VALUE, DXL_MAXIMUM_VELOCITY_VALUE);
                continue;
            }
            dxl_goal_velocity[0] = velocity;
            dxl_goal_velocity[1] = velocity;
            return 0;
        }
        else if (mode == 2) {
            int vel = 0;
            printf("Enter the positive velocity value for reciprocal motion:\n");
            if (scanf("%d", &vel) != 1 || vel <= 0 || vel > DXL_MAXIMUM_VELOCITY_VALUE) {
                printf("Invalid input. Please enter a positive velocity within range [1, %d].\n",
                    DXL_MAXIMUM_VELOCITY_VALUE);
                continue;
            }
            return vel;
        }
        else if (mode == 3) {
            char direction[4];
            printf("Enter rotating direction (cw/ccw):\n");
            if (scanf("%3s", direction) != 1) {
                printf("Invalid input. Please enter 'cw' or 'ccw'.\n");
                continue;
            }
            if (strcmp(direction, "cw") == 0) {
                dxl_goal_velocity[0] = 30;
                dxl_goal_velocity[1] = 0;
            }
            else if (strcmp(direction, "ccw") == 0) {
                dxl_goal_velocity[0] = 0;
                dxl_goal_velocity[1] = 30;
            }
            else {
                printf("Invalid direction. Please enter 'cw' or 'ccw'.\n");
                continue;
            }
            return 0;
        }
        else {
            printf("Invalid mode. Please select 1, 2, or 3.\n");
            break;
        }
    }
}



int main()
{
  // Initialize PortHandler instance
  // Set the port path
  // Get methods and members of PortHandlerLinux or PortHandlerWindows
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);

  // Initialize PacketHandler instance
  // Set the protocol version
  // Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

  // Initialize GroupSyncWrite instance
  dynamixel::GroupSyncWrite groupSyncWrite(portHandler, packetHandler, ADDR_PRO_GOAL_VELOCITY, LEN_PRO_GOAL_VELOCITY);

  // Initialize Groupsyncread instance for Present Velocity
  dynamixel::GroupSyncRead groupSyncRead(portHandler, packetHandler, ADDR_PRO_PRESENT_VELOCITY, LEN_PRO_PRESENT_VELOCITY);

  int index = 0;
  int dxl_comm_result = COMM_TX_FAIL;               // Communication result
  bool dxl_addparam_result = false;                 // addParam result
  bool dxl_getdata_result = false;                  // GetParam result
  int dxl_goal_velocity[2] = {0, 0};  // Goal velocity for dxl


  uint8_t dxl_error = 0;                            // Dynamixel error
  uint8_t param_goal_velocity[4];
  int32_t dxl1_present_velocity = 0, dxl2_present_velocity = 0;                         // Present velocity

  // Open port
  if (portHandler->openPort())
  {
    printf("Succeeded to open the port!\n");
  }
  else
  {
    printf("Failed to open the port!\n");
    printf("Press any key to terminate...\n");
    getch();
    return 0;
  }

  // Set port baudrate
  if (portHandler->setBaudRate(BAUDRATE))
  {
    printf("Succeeded to change the baudrate!\n");
  }
  else
  {
    printf("Failed to change the baudrate!\n");
    printf("Press any key to terminate...\n");
    getch();
    return 0;
  }

  // Enable Dynamixel#1 Torque
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL1_ID, ADDR_PRO_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS)
  {
    printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
  }
  else if (dxl_error != 0)
  {
    printf("%s\n", packetHandler->getRxPacketError(dxl_error));
  }
  else
  {
    printf("Dynamixel#%d has been successfully connected \n", DXL1_ID);
  }

  // Enable Dynamixel#2 Torque
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL2_ID, ADDR_PRO_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS)
  {
    printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
  }
  else if (dxl_error != 0)
  {
    printf("%s\n", packetHandler->getRxPacketError(dxl_error));
  }
  else
  {
    printf("Dynamixel#%d has been successfully connected \n", DXL2_ID);
  }

  // Add parameter storage for Dynamixel#1 present velocity value
  dxl_addparam_result = groupSyncRead.addParam(DXL1_ID);
  if (dxl_addparam_result != true)
  {
    fprintf(stderr, "[ID:%03d] groupSyncRead addparam failed", DXL1_ID);
    return 0;
  }

  // Add parameter storage for Dynamixel#2 present velocity value
  dxl_addparam_result = groupSyncRead.addParam(DXL2_ID);
  if (dxl_addparam_result != true)
  {
    fprintf(stderr, "[ID:%03d] groupSyncRead addparam failed", DXL2_ID);
    return 0;
  }

  while (1)
  {
      printf("Press any key to continue! (or press ESC to quit!)\n");
      if (getch() == ESC_ASCII_VALUE)
          break;
      int vel = 0;
      vel = getInputVelocity(dxl_goal_velocity);

      if (vel > 0) {     //mode 2
          int time = 15000; 
          int stop_time = 1000; 


          // Move forward
          dxl_goal_velocity[0] = vel;
          dxl_goal_velocity[1] = vel;
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          groupSyncWrite.addParam(DXL1_ID, param_goal_velocity);
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          groupSyncWrite.addParam(DXL2_ID, param_goal_velocity);
          groupSyncWrite.txPacket();
          groupSyncWrite.clearParam();
          std::this_thread::sleep_for(std::chrono::milliseconds(time));

          // Stop
          dxl_goal_velocity[0] = 0;
          dxl_goal_velocity[1] = 0;
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          groupSyncWrite.addParam(DXL1_ID, param_goal_velocity);
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          groupSyncWrite.addParam(DXL2_ID, param_goal_velocity);
          groupSyncWrite.txPacket();
          groupSyncWrite.clearParam();
          std::this_thread::sleep_for(std::chrono::milliseconds(stop_time));

          // Move backward
          dxl_goal_velocity[0] = -vel;
          dxl_goal_velocity[1] = -vel;
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          groupSyncWrite.addParam(DXL1_ID, param_goal_velocity);
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          groupSyncWrite.addParam(DXL2_ID, param_goal_velocity);
          groupSyncWrite.txPacket();
          groupSyncWrite.clearParam();
          std::this_thread::sleep_for(std::chrono::milliseconds(time));

          // Stop
          dxl_goal_velocity[0] = 0;
          dxl_goal_velocity[1] = 0;
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          groupSyncWrite.addParam(DXL1_ID, param_goal_velocity);
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          groupSyncWrite.addParam(DXL2_ID, param_goal_velocity);
          groupSyncWrite.txPacket();
          groupSyncWrite.clearParam();
      }


      else if(vel == 0) {     //mode 1 & 3
          int time = 15000;
          int stop_time = 1000;

          
          

          // Move
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          groupSyncWrite.addParam(DXL1_ID, param_goal_velocity);
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          groupSyncWrite.addParam(DXL2_ID, param_goal_velocity);
          groupSyncWrite.txPacket();
          groupSyncWrite.clearParam();
          std::this_thread::sleep_for(std::chrono::milliseconds(time));

          // Stop
          dxl_goal_velocity[0] = 0;
          dxl_goal_velocity[1] = 0;
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[0]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[0]));
          groupSyncWrite.addParam(DXL1_ID, param_goal_velocity);
          param_goal_velocity[0] = DXL_LOBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[1] = DXL_HIBYTE(DXL_LOWORD(dxl_goal_velocity[1]));
          param_goal_velocity[2] = DXL_LOBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          param_goal_velocity[3] = DXL_HIBYTE(DXL_HIWORD(dxl_goal_velocity[1]));
          groupSyncWrite.addParam(DXL2_ID, param_goal_velocity);
          groupSyncWrite.txPacket();
          groupSyncWrite.clearParam();
          std::this_thread::sleep_for(std::chrono::milliseconds(stop_time));
      }
  }

  // Disable Dynamixel#1 Torque
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL1_ID, ADDR_PRO_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS)
  {
    printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
  }
  else if (dxl_error != 0)
  {
    printf("%s\n", packetHandler->getRxPacketError(dxl_error));
  }

  // Disable Dynamixel#2 Torque
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL2_ID, ADDR_PRO_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS)
  {
    printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
  }
  else if (dxl_error != 0)
  {
    printf("%s\n", packetHandler->getRxPacketError(dxl_error));
  }

  // Close port
  portHandler->closePort();

  return 0;
}
