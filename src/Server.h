#pragma once
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include "Credentials.h"
#include "DDNSUpdater.h"
#include "MagicPacketSender.h"
struct ClientSession
{
    String id;
    String role;
    unsigned long expires;
};

class Server
{
public:
    static void init();
    static void cleanExpiredSessions();

private:
    static AsyncWebServer server;
    static ClientSession sessions[5];
    static int numSessions;

    static void handleRoot(AsyncWebServerRequest *request);
    static void handleLogin(AsyncWebServerRequest *request);
    static void handleLogout(AsyncWebServerRequest *request);
    static void handleData(AsyncWebServerRequest *request);
    static void handleGuestUpdate(AsyncWebServerRequest *request);
    static void handleNotFound(AsyncWebServerRequest *request);

    static void handleAdminRedirect(AsyncWebServerRequest *request);
    static void handleGuestRedirect(AsyncWebServerRequest *request);
    static void handleDDNSUpdate(AsyncWebServerRequest *request);
    static void handleWakeOnLAN(AsyncWebServerRequest *request);
    static void handleMc(AsyncWebServerRequest *request);

    static ClientSession* authenticateRequest(AsyncWebServerRequest *request);
    static String generateSessionId();
    static String getSessionIdFromCookie(AsyncWebServerRequest *request);
    static void sendFile(AsyncWebServerRequest *request, String path);
    static String getContentType(String filename);
};