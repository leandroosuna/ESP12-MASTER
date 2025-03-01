#include "Server.h"


AsyncWebServer Server::server(80);
ClientSession Server::sessions[5];
int Server::numSessions = 0;

IPAddress localIP(192,168,1,100);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(1,1,1,1);
IPAddress dns2(1,0,0,1);

void Server::init()
{
    if (!SPIFFS.begin())
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    WiFi.config(localIP, gateway, subnet, dns, dns2);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    pinMode(LED_BUILTIN, OUTPUT);
    int c = 0;
    bool blink = LOW;
    
    while (WiFi.status() != WL_CONNECTED)
    {
        if(c % 100 == 0)
        {
            Serial.print(".");
            digitalWrite(LED_BUILTIN, blink);
            blink = !blink;
        }
        c++;
        delay(1);
    }
    
    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/login", HTTP_POST, handleLogin);
    server.on("/logout", HTTP_GET, handleLogout);
    server.on("/data", HTTP_GET, handleData);
    server.on("/guestupdate", HTTP_GET, handleGuestUpdate);
    server.on("/admin.html", HTTP_GET, handleAdminRedirect);
    server.on("/guest.html", HTTP_GET, handleGuestRedirect);
    server.on("/ddnsupdate", HTTP_GET, handleDDNSUpdate);
    server.on("/wakeonlan", HTTP_GET, handleWakeOnLAN);

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
}

void Server::cleanExpiredSessions()
{
    unsigned long currentTime = millis();
    for (int i = 0; i < numSessions;)
    {
        if (sessions[i].expires <= currentTime)
        {
            for (int j = i; j < numSessions - 1; j++)
            {
                sessions[j] = sessions[j + 1];
            }
            numSessions--;
        }
        else
        {
            i++;
        }
    }
}

void Server::handleRoot(AsyncWebServerRequest *request)
{
    sendFile(request, "/login.html");
}

void Server::handleLogin(AsyncWebServerRequest *request)
{
    if (request->hasParam("username", true) && request->hasParam("password", true))
    {
        String username = request->getParam("username", true)->value();
        String password = request->getParam("password", true)->value();

        for (const User &user : users)
        {
            if (username == user.username && password == user.password)
            {
                if (numSessions >= 5)
                {
                    request->send(503, "text/plain", "Too many active sessions");
                    return;
                }

                ClientSession newSession;
                newSession.id = generateSessionId();
                newSession.role = user.role;
                newSession.expires = millis() + 900000; // 15 minutes

                sessions[numSessions++] = newSession;

                String redirectPath = (user.role == "admin") ? "/admin.html" : "/guest.html";
                AsyncWebServerResponse *response = request->beginResponse(302);
                response->addHeader("Location", redirectPath);
                response->addHeader("Set-Cookie", "SESSIONID=" + newSession.id + "; Path=/; HttpOnly");
                request->send(response);
                return;
            }
        }
    }
    request->send(401, "text/plain", "Invalid credentials");
}

void Server::handleLogout(AsyncWebServerRequest *request)
{
    String sessionId = getSessionIdFromCookie(request);
    for (int i = 0; i < numSessions; i++)
    {
        if (sessions[i].id == sessionId)
        {
            for (int j = i; j < numSessions - 1; j++)
            {
                sessions[j] = sessions[j + 1];
            }
            numSessions--;
            break;
        }
    }

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Logged out");
    response->addHeader("Set-Cookie", "SESSIONID=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
    request->send(response);
}

void Server::handleData(AsyncWebServerRequest *request)
{
    ClientSession *session = authenticateRequest(request);
    if (!session)
    {
        request->send(401, "text/plain", "Unauthorized");
        return;
    }

    if (session->role != "admin")
    {
        request->send(403, "text/plain", "Forbidden");
        return;
    }

    request->send(200, "text/plain", "Admin data: Sensitive information");
}

void Server::handleGuestUpdate(AsyncWebServerRequest *request)
{
    ClientSession *session = authenticateRequest(request);
    if (!session)
    {
        request->send(401, "text/plain", "Unauthorized");
        return;
    }

    if (session->role != "guest" && session->role != "admin")
    {
        request->send(403, "text/plain", "Forbidden");
        return;
    }

    request->send(200, "text/plain", "Guest update received");
}

void Server::handleAdminRedirect(AsyncWebServerRequest *request) {
    ClientSession* session = authenticateRequest(request);
    if (!session || session->role != "admin") {
        request->redirect("/");
        return;
    }
    sendFile(request, "/admin.html");
}

void Server::handleGuestRedirect(AsyncWebServerRequest *request) {
    ClientSession* session = authenticateRequest(request);
    if (!session || (session->role != "guest" && session->role != "admin")) {
        request->redirect("/");
        return;
    }
    sendFile(request, "/guest.html");
}

void Server::handleDDNSUpdate(AsyncWebServerRequest *request) {
    ClientSession* session = authenticateRequest(request);
    if (!session || session->role != "admin") {
        request->send(403, "text/plain", "Forbidden");
        return;
    }
    
    if(!DDNSUpdater::isUpdating()) {
        DDNSUpdater::beginUpdate();
        request->send(202, "text/plain", "DDNS update initiated");
    } else {
        request->send(429, "text/plain", "Update already in progress");
    }
}
void Server::handleWakeOnLAN(AsyncWebServerRequest *request) {
    ClientSession* session = authenticateRequest(request);
    if (!session || session->role != "admin") {
        request->send(403, "text/plain", "Forbidden");
        return;
    }
    MagicPacketSender::wakeUpPC();
    request->send(200, "text/plain", "Magic packet sent");
}

ClientSession *Server::authenticateRequest(AsyncWebServerRequest *request)
{
    String sessionId = getSessionIdFromCookie(request);
    for (int i = 0; i < numSessions; i++)
    {
        if (sessions[i].id == sessionId && millis() < sessions[i].expires)
        {
            sessions[i].expires = millis() + 900000; // Refresh session
            return &sessions[i];
        }
    }
    return nullptr;
}

String Server::generateSessionId()
{
    return String(millis()) + "-" + String(ESP.random());
}

String Server::getSessionIdFromCookie(AsyncWebServerRequest *request)
{
    if (request->hasHeader("Cookie"))
    {
        String cookie = request->getHeader("Cookie")->value();
        int start = cookie.indexOf("SESSIONID=");
        if (start != -1)
        {
            start += 10;
            int end = cookie.indexOf(";", start);
            if (end == -1)
                end = cookie.length();
            return cookie.substring(start, end);
        }
    }
    return "";
}

void Server::sendFile(AsyncWebServerRequest *request, String path)
{
    if (path.endsWith("/"))
        path += "index.html";

    if (SPIFFS.exists(path))
    {
        request->send(SPIFFS, path, getContentType(path));
    }
    else
    {
        request->send(404, "text/plain", "File not found");
    }
}

String Server::getContentType(String filename)
{
    if (filename.endsWith(".html"))
        return "text/html";
    if (filename.endsWith(".css"))
        return "text/css";
    if (filename.endsWith(".js"))
        return "application/javascript";
    if (filename.endsWith(".ico"))
        return "image/x-icon";
    return "text/plain";
}

void Server::handleNotFound(AsyncWebServerRequest *request)
{
    if (SPIFFS.exists(request->url()))
    {
        sendFile(request, request->url());
    }
    else
    {
        request->send(404, "text/plain", "Not found");
    }
}