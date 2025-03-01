#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>

class MagicPacketSender {
public:
  // Set this to the MAC address of the PC you want to wake (format: "00:11:22:33:44:55")
  static String targetMac;
  // Broadcast IP address (usually 255.255.255.255)
  static IPAddress broadcastIP;
  // UDP port to use (commonly 9 for WoL)
  static const int udpPort = 9;

  // Call this static method to send a Wake-on-LAN magic packet.
  static void wakeUpPC();

private:
  // Helper: parse a MAC address string into a 6-byte array.
  static bool parseMac(const String &macStr, uint8_t mac[6]);
};
