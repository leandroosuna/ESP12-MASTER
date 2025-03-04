#pragma once
#include <RTCMemory.h>

typedef struct 
{
    uint32_t lastIp;
} dataRTC;


class RTCM
{
    public:
        static void init();
        static void saveLastIp(String ip);
        static String getLastIp();
    
    private:
        static uint32_t ipEncode(String ip);
        static String ipDecode(uint32_t ip);
        static RTCMemory<dataRTC> rtcMemory;
        
        

};