#include "DDNSUpdater.h"

DDNSUpdater::UpdateState DDNSUpdater::state = UpdateState::Idle;
String DDNSUpdater::publicIP;
WiFiClientSecure *DDNSUpdater::client = nullptr;
HTTPClient DDNSUpdater::http;
WiFiClientSecure *DDNSUpdater::client2 = nullptr;
HTTPClient DDNSUpdater::http2;
unsigned long DDNSUpdater::lastUpdateTime = 0;

void DDNSUpdater::beginUpdate() {
    if(state == UpdateState::Idle) {
        state = UpdateState::StartRequest;
        publicIP = "";
        lastUpdateTime = millis();
        Serial.println("ddns update begin");
    }
}

bool DDNSUpdater::isUpdating() {
    return state != UpdateState::Idle;
}

void DDNSUpdater::process() {
    switch(state) {

        case UpdateState::StartRequest:
            client = new WiFiClientSecure;
            client->setInsecure();

            if(!http.connected()) {
                http.begin(*client, ipifyURL);
                http.setTimeout(5000);
            }
            state = UpdateState::GettingIP;
            break;
        case UpdateState::GettingIP:{
            Serial.println("getting IP");
            int httpCode = http.GET();
            if(httpCode  == HTTP_CODE_OK) {
                publicIP = http.getString();
                Serial.println("public IP: " + publicIP);
                publicIP.trim();
                http.end();
                client->stop();
                state = UpdateState::UpdatingDDNS;
                lastUpdateTime = millis();
            }
            else if(millis() - lastUpdateTime > 10000) {
                http.end();
                client->stop();
                state = UpdateState::Idle;
            }
            else
            {
                http.end();
                client->stop();
                state = UpdateState::Idle;
                Serial.println("Code"+httpCode);
                Serial.println("Response: " + http.getString());
            }
            break;
        }
        case UpdateState::UpdatingDDNS: {
            client2 = new WiFiClientSecure;
            client2->setInsecure();
            Serial.println("updating DDNS");
            String url = ddnsServer + ddnsUpdatePath +
                       "?u=" + ddnsUser +
                       "&p=" + ddnsPass +
                       "&hostname=" + hostname +
                       "&myip=" + publicIP;
            Serial.println("url: " + url);

            if(!http2.connected()) {
                http2.begin(*client2, url);
                http2.setTimeout(5000);
            }
            
            int httpCode = http2.GET();
            
            if(httpCode == HTTP_CODE_OK) {
                Serial.println("OK, Response: " + http2.getString());
                http2.end();
                client2->stop();
                state = UpdateState::Idle;
                lastUpdateTime = millis();
            }
            else if(millis() - lastUpdateTime > 10000) {
                http2.end();
                state = UpdateState::Idle;
                client2->stop();
                Serial.println(
                    "timeout, Response: " + http2.getString()
                );
            }
            else
            {
                http2.end();
                client2->stop();
                state = UpdateState::Idle;
                Serial.println();
                Serial.println("Code"+String(httpCode));
                Serial.println("Response: " + http.getString());
            }
            break;
        }

        case UpdateState::Idle:
        default:
            if(http.connected()) {
                http.end();
            }
            break;
    }
}