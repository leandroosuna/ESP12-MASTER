#pragma once
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include "DDNSCredentials.h"
#include "RTCM.h"

class DDNSUpdater {
public:
    enum class UpdateState { Idle, UpdatingIP, UpdatingDDNS };

    static void checkUpdate();
    static void beginUpdate();
    static void process();
    static bool isUpdating();
    
private:
    static UpdateState state;
    static String publicIP;
    static WiFiClientSecure *client;
    static HTTPClient http;
    static WiFiClientSecure *client2;
    static HTTPClient http2;
    
    static unsigned long lastUpdateTime;
};