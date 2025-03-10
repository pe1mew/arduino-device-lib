/*--------------------------------------------------------------------
  Original code: Copyright © 2017 The Things Network
  Use of this source code is governed by the MIT license that can be 
  found in the LICENSE file.

  Additions by Remko Welling (pe1mew@pe1mew.nl)

  This code is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  --------------------------------------------------------------------*/

/*!

 \file RN2483Network.h
 \brief Library to configure and use Microchip RN2483 with a LoRaWAN network
 \date 18-12-2019
 \author Remko Welling (pe1mew@pe1mew.nl)
 \version See release history
  
 ## Release histroy
 
 Version|Date        |Note
 -------|------------|----
 <      | 18-12-2019 | Initial fork from 
        |            | 
        |            | 
        |            | 

 Work progress
 -------------
 \todo Complete changing TheThingsnetwork class to a generic name
 \todo Add doxygen comments
 \todo make class inheritable by moving private to protected
 
 */



#ifndef _RN2483_NETWORK_H_
#define _RN2483_NETWORK_H_

#include <Arduino.h>
#include <Stream.h>
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD)
    #include <avr/pgmspace.h>
#else
    #include <pgmspace.h>
#endif

#define LORAWAN_DEFAULT_SF 7
#define LORAWAN_DEFAULT_FSB 2
#define LORAWAN_RETX "7"

#define LORAWAN_PWRIDX_EU868 "1"
#define LORAWAN_PWRIDX_US915 "5"
#define LORAWAN_PWRIDX_AU915 "5"
#define LORAWAN_PWRIDX_AS920_923 "1" // TODO: should be 0, but the current RN2903AS firmware doesn't accept that value (probably still using EU868: 1=14dBm)
#define LORAWAN_PWRIDX_AS923_925 "1" // TODO: should be 0
#define LORAWAN_PWRIDX_KR920_923 "1" // TODO: should be 0
#define LORAWAN_PWRIDX_IN865_867 "1" // TODO: should be 0

#define LORAWAN_BUFFER_SIZE 300

typedef uint8_t port_t;

enum ttn_response_t
{
  LORAWAN_ERROR_SEND_COMMAND_FAILED = (-1),
  LORAWAN_ERROR_UNEXPECTED_RESPONSE = (-10),
  LORAWAN_SUCCESSFUL_TRANSMISSION = 1,
  LORAWAN_SUCCESSFUL_RECEIVE = 2
};

enum ttn_fp_t
{
  TTN_FP_EU868,
  TTN_FP_US915,
  TTN_FP_AU915,
  TTN_FP_AS920_923,
  TTN_FP_AS923_925,
  TTN_FP_KR920_923,
  TTN_FP_IN865_867
};

class RN2483Network
{
private:
  Stream *modemStream;
  Stream *debugStream;
  ttn_fp_t fp;
  uint8_t sf;
  uint8_t fsb;
  bool adr;
  char buffer[512];
  bool baudDetermined = false;
  void (*messageCallback)(const uint8_t *payload, size_t size, port_t port);

  void clearReadBuffer();
  size_t readLine(char *buffer, size_t size, uint8_t attempts = 3);
  size_t readResponse(uint8_t prefixTable, uint8_t indexTable, uint8_t index, char *buffer, size_t size);
  size_t readResponse(uint8_t table, uint8_t index, char *buffer, size_t size);

  void debugPrintIndex(uint8_t index, const char *value = NULL);
  void debugPrintMessage(uint8_t type, uint8_t index, const char *value = NULL);

  void autoBaud();
  void configureEU868();
  void configureUS915(uint8_t fsb);
  void configureAU915(uint8_t fsb);
  void configureAS920_923();
  void configureAS923_925();
  void configureKR920_923();
  void configureIN865_867();
  void configureChannels(uint8_t fsb);
  bool setSF(uint8_t sf);
  bool waitForOk();

  void sendCommand(uint8_t table, uint8_t index, bool appendSpace, bool print = true);
  bool sendMacSet(uint8_t index, const char *value);
  bool sendChSet(uint8_t index, uint8_t channel, const char *value);
  bool sendJoinSet(uint8_t type);
  bool sendPayload(uint8_t mode, uint8_t port, uint8_t *payload, size_t len);
  void sendGetValue(uint8_t table, uint8_t prefix, uint8_t index);

public:
  bool needsHardReset = false;

  RN2483Network(Stream &modemStream, Stream &debugStream, ttn_fp_t fp, uint8_t sf = LORAWAN_DEFAULT_SF, uint8_t fsb = LORAWAN_DEFAULT_FSB);
  void reset(bool adr = true);
  void resetHard(uint8_t resetPin);
  void showStatus();
  size_t getHardwareEui(char *buffer, size_t size);
  size_t getAppEui(char *buffer, size_t size);
  uint16_t getVDD();
  void onMessage(void (*cb)(const uint8_t *payload, size_t size, port_t port));
  bool provision(const char *appEui, const char *appKey);
  bool join(const char *appEui, const char *appKey, int8_t retries = -1, uint32_t retryDelay = 10000);
  bool join(int8_t retries = -1, uint32_t retryDelay = 10000);
  bool personalize(const char *devAddr, const char *nwkSKey, const char *appSKey);
  bool personalize();
  ttn_response_t sendBytes(const uint8_t *payload, size_t length, port_t port = 1, bool confirm = false, uint8_t sf = 0);
  ttn_response_t poll(port_t port = 1, bool confirm = false);
  void sleep(uint32_t mseconds);
  void wake();
  void saveState();
  void linkCheck(uint16_t seconds);
  uint8_t getLinkCheckGateways();
  uint8_t getLinkCheckMargin();
};

#endif
