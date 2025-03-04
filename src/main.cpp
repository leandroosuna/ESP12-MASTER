#include <Arduino.h>
#include "Server.h"

void setup()
{
    Serial.begin(115200);
    RTCM::init();
    Server::init();
    DDNSUpdater::checkUpdate();
}
int c = 0;
int seconds = 0;
void loop()
{
    c++;
    if (c % 1000 == 0)
    {
        seconds++;
        Server::cleanExpiredSessions();

        if(seconds % 3600 == 0)
        {
            DDNSUpdater::checkUpdate();
        }
    }

    
    DDNSUpdater::process();
    delay(1); 
}