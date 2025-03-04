#include "DDNSUpdater.h"

DDNSUpdater::UpdateState DDNSUpdater::state = UpdateState::Idle;
String DDNSUpdater::publicIP;
WiFiClientSecure *DDNSUpdater::client = nullptr;
HTTPClient DDNSUpdater::http;
WiFiClientSecure *DDNSUpdater::client2 = nullptr;
HTTPClient DDNSUpdater::http2;
unsigned long DDNSUpdater::lastUpdateTime = 0;

void DDNSUpdater::beginUpdate(){
    state = UpdateState::UpdatingIP;
}

void DDNSUpdater::checkUpdate() {
    if(state == UpdateState::Idle) {
        state = UpdateState::UpdatingIP;
    }
}
bool DDNSUpdater::isUpdating(){
    return state != UpdateState::Idle;
}
void DDNSUpdater::process() {
    switch(state) {

        case UpdateState::UpdatingIP:{

            String lastIp = RTCM::getLastIp();
            bool noRTCData = lastIp == "e" || lastIp == "0.0.0.0";

            client = new WiFiClientSecure;
            client->setInsecure();

            if(!http.connected()) {
                http.begin(*client, ipifyURL);
                http.setTimeout(5000);
            }
        
            String newIp;
            int httpCode = http.GET();
            if(httpCode  == HTTP_CODE_OK) {
                newIp = http.getString();
                newIp.trim();
                http.end();
                client->stop();
                
                if(noRTCData || newIp != lastIp) {
                    publicIP = newIp;
                    RTCM::saveLastIp(publicIP);
                    state = UpdateState::UpdatingDDNS;
                    lastUpdateTime = millis();
                }
                else {
                    state = UpdateState::Idle;
                }
                
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
                // Serial.println("Code"+httpCode);
                // Serial.println("Response: " + http.getString());
            }
            break;
        }
        case UpdateState::UpdatingDDNS: {
            client2 = new WiFiClientSecure;
            client2->setInsecure();
            Serial.println("updating DDNS to " + publicIP);
            String url = ddnsServer + ddnsUpdatePath +
                       "?u=" + ddnsUser +
                       "&p=" + ddnsPass +
                       "&hostname=" + hostname +
                       "&myip=" + publicIP;

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
                // Serial.println(
                //     "timeout, Response: " + http2.getString()
                // );
            }
            else
            {
                http2.end();
                client2->stop();
                state = UpdateState::Idle;
                // Serial.println();
                // Serial.println("Code"+String(httpCode));
                // Serial.println("Response: " + http.getString());
            }
            break;
        }

        case UpdateState::Idle:
        default:
            if(http.connected()) {
                http.end();
                client->stop();
            }
            if(http2.connected()) {
                http2.end();
                client2->stop();
            }
            
            break;
    }
}