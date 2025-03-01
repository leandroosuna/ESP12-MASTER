#include "MagicPacketSender.h"

// Set a default target MAC address; update this value as needed.
String MagicPacketSender::targetMac = "F0:2F:74:B3:71:4D";
IPAddress MagicPacketSender::broadcastIP(255, 255, 255, 255);

bool MagicPacketSender::parseMac(const String &macStr, uint8_t mac[6]) {
  int values[6];
  // sscanf expects the MAC to be in hex separated by colons.
  if (6 == sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x", 
                    &values[0], &values[1], &values[2],
                    &values[3], &values[4], &values[5])) {
    for (int i = 0; i < 6; i++) {
      mac[i] = (uint8_t) values[i];
    }
    return true;
  }
  return false;
}

void MagicPacketSender::wakeUpPC() {
  uint8_t mac[6];
  if (!parseMac(targetMac, mac)) {
    Serial.println("Invalid MAC address format!");
    return;
  }
  
  // Magic packet: 6 bytes of 0xFF followed by 16 repetitions of the MAC address (6 bytes each).
  uint8_t packet[102];
  memset(packet, 0xFF, 6);
  for (int i = 0; i < 16; i++) {
    memcpy(&packet[6 + i * 6], mac, 6);
  }
  
  WiFiUDP udp;
  udp.beginPacket(broadcastIP, udpPort);
  udp.write(packet, sizeof(packet));
  udp.endPacket();
  
  Serial.println("Magic packet sent to wake up the PC.");
}
