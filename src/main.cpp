#include <Arduino.h>
#include "Server.h"

void setup()
{
    Serial.begin(115200);
    Server::init();
}
int c = 0;
void loop()
{
    c++;
    if (c % 10000 == 0)
    {
        Server::cleanExpiredSessions();
    }
    DDNSUpdater::process();
    delay(1); 
}