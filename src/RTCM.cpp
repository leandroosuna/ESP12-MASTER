#include "RTCM.h"

RTCMemory<dataRTC> RTCM::rtcMemory;

void RTCM::init()
{
    rtcMemory.begin();
}

void RTCM::saveLastIp(String ip)
{
    dataRTC* data = rtcMemory.getData();
    data->lastIp = ipEncode(ip);
    rtcMemory.save();
}

String RTCM::getLastIp()
{
    dataRTC* data = rtcMemory.getData();
    return ipDecode(data->lastIp);
}

uint32_t RTCM::ipEncode(String ip)
{
    ip.trim();
    char chars[3] = {'\0', '\0', '\0'};
    byte ipSegments[4] = {0, 0, 0, 0};

    int c = 0;
    int s = 0;
    for(unsigned int i = 0; i < ip.length(); i++)
    {
        if(ip[i] == '.')
        {
            ipSegments[s] = String(chars).toInt();
            s++;
            c = 0;
            chars[0] = '\0';
            chars[1] = '\0';
            chars[2] = '\0';   
        }
        else
        {
            chars[c] = ip[i];
            c++;
        }
    }
    ipSegments[s] = String(chars).toInt();
    
    uint32_t addr = (ipSegments[0] << 24) | (ipSegments[1] << 16) | (ipSegments[2] << 8) | ipSegments[3];

    return addr;
}

String RTCM::ipDecode(uint32_t ip)
{
    if(ip == 0)
    {
        return "e";
    }

    byte ipSegments[4] = {0, 0, 0, 0};
    ipSegments[0] = (ip >> 24) & 0xFF;
    ipSegments[1] = (ip >> 16) & 0xFF;
    ipSegments[2] = (ip >> 8) & 0xFF;
    ipSegments[3] = ip & 0xFF;

    String ipString = String(ipSegments[0]) + "." + String(ipSegments[1]) + "." + String(ipSegments[2]) + "." + String(ipSegments[3]);
    return ipString;
}
