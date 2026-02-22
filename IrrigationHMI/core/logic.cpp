#include <algorithm>
#include "logic.h"
#include "../comm/protocol.h"

namespace app {

static const char WEB_PAGE[] PROGMEM = R"HTML(
<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Irrigation HMI</title></head><body><h2>Irrigation HMI</h2><pre id='s'>loading...</pre>
<script>
async function r(){let d=await (await fetch('/api/state')).json();document.getElementById('s').textContent=JSON.stringify(d,null,2);} setInterval(r,1000); r();
</script></body></html>
)HTML";

bool LogicController::begin(SystemState *state, EventBus *bus, RS485Transport *transport) {
    state_ = state;
    bus_ = bus;
    transport_ = transport;
    stateMutex_ = xSemaphoreCreateMutex();

    prefs_.begin("irrig", false);
    loadSettings();
    wifiAutoConnect();
    startWebServer();

    addEvent(*state_, "Boot: HMI master started");
    discoverModules();
    return true;
}

void LogicController::loadSettings() {
    state_->settings.wifiSsid = prefs_.getString("wifi_ssid", "");
    state_->settings.wifiPass = prefs_.getString("wifi_pass", "");
    state_->settings.language = static_cast<Lang>(prefs_.getUChar("lang", 0));
}

void LogicController::saveWifiCreds(const char *ssid, const char *pass) {
    prefs_.putString("wifi_ssid", ssid);
    prefs_.putString("wifi_pass", pass);
    state_->settings.wifiSsid = ssid;
    state_->settings.wifiPass = pass;
}

void LogicController::wifiAutoConnect() {
    WiFi.mode(WIFI_STA);
    if (state_->settings.wifiSsid.length()) {
        WiFi.begin(state_->settings.wifiSsid.c_str(), state_->settings.wifiPass.c_str());
        uint32_t start = millis();
        while (millis() - start < 8000 && WiFi.status() != WL_CONNECTED) {
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
    if (WiFi.status() == WL_CONNECTED) {
        state_->wifi.connected = true;
        state_->wifi.apMode = false;
        state_->wifi.ssid = WiFi.SSID();
        state_->wifi.ip = WiFi.localIP();
        addEvent(*state_, "WiFi connected");
    } else {
        startApMode();
    }
}

void LogicController::startApMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Irrigation-HMI-Setup");
    state_->wifi.connected = false;
    state_->wifi.apMode = true;
    state_->wifi.ssid = "Irrigation-HMI-Setup";
    state_->wifi.ip = WiFi.softAPIP();
    addEvent(*state_, "WiFi AP mode started");
}

void LogicController::startWebServer() {
    if (webStarted_) return;

    web_.on("/", HTTP_GET, [&]() { web_.send_P(200, "text/html", WEB_PAGE); });
    web_.on("/api/state", HTTP_GET, [&]() { web_.send(200, "application/json", buildStateJson()); });

    web_.on("/api/channel", HTTP_POST, [&]() {
        if (!web_.hasArg("plain")) {
            web_.send(400, "application/json", "{\"ok\":false}");
            return;
        }
        String b = web_.arg("plain");
        int m = b.indexOf("\"module\":");
        int c = b.indexOf("\"channel\":");
        int s = b.indexOf("\"state\":");
        if (m < 0 || c < 0 || s < 0) {
            web_.send(400, "application/json", "{\"ok\":false}");
            return;
        }
        AppEvent ev{};
        ev.type = AppEventType::SetChannelState;
        ev.moduleAddr = b.substring(m + 9).toInt();
        ev.channelId = b.substring(c + 10).toInt();
        ev.valueBool = b.substring(s + 8).startsWith("true") || b.substring(s + 8).toInt() == 1;
        bus_->publish(ev, 0);
        web_.send(200, "application/json", "{\"ok\":true}");
    });

    web_.on("/api/rescan", HTTP_POST, [&]() {
        AppEvent ev{};
        ev.type = AppEventType::ModuleRescan;
        bus_->publish(ev, 0);
        web_.send(200, "application/json", "{\"ok\":true}");
    });

    web_.on("/api/wifi", HTTP_POST, [&]() {
        if (!web_.hasArg("plain")) {
            web_.send(400, "application/json", "{\"ok\":false}");
            return;
        }
        String b = web_.arg("plain");
        int s = b.indexOf("\"ssid\":\"");
        int p = b.indexOf("\"pass\":\"");
        if (s < 0 || p < 0) {
            web_.send(400, "application/json", "{\"ok\":false}");
            return;
        }
        String ssid = b.substring(s + 8);
        ssid = ssid.substring(0, ssid.indexOf('"'));
        String pass = b.substring(p + 8);
        pass = pass.substring(0, pass.indexOf('"'));

        AppEvent ev{};
        ev.type = AppEventType::WifiSaveCreds;
        strlcpy(ev.ssid, ssid.c_str(), sizeof(ev.ssid));
        strlcpy(ev.pass, pass.c_str(), sizeof(ev.pass));
        bus_->publish(ev, 0);
        web_.send(200, "application/json", "{\"ok\":true}");
    });

    web_.begin();
    webStarted_ = true;
}

String LogicController::buildStateJson() {
    String out = "{\"wifi\":{\"connected\":" + String(state_->wifi.connected ? "true" : "false") +
                 ",\"apMode\":" + String(state_->wifi.apMode ? "true" : "false") +
                 ",\"ssid\":\"" + state_->wifi.ssid + "\"},\"modules\":[";
    for (size_t i = 0; i < state_->modules.size(); ++i) {
        const auto &m = state_->modules[i];
        out += "{\"addr\":" + String(m.address) + ",\"uid\":" + String((unsigned long)m.uid) + ",\"channels\":[";
        for (size_t c = 0; c < m.channels.size(); ++c) {
            const auto &ch = m.channels[c];
            out += "{\"id\":" + String(ch.id) + ",\"state\":" + String(ch.state ? "true" : "false") + "}";
            if (c + 1 < m.channels.size()) out += ",";
        }
        out += "]}";
        if (i + 1 < state_->modules.size()) out += ",";
    }
    out += "]}";
    return out;
}

void LogicController::sendSetChannelState(uint8_t moduleAddr, uint8_t channelId, bool state) {
    uint8_t payload[2] = {channelId, static_cast<uint8_t>(state ? 1 : 0)};
    auto frame = protocol::buildFrame(moduleAddr, 0x20, payload, sizeof(payload));
    transport_->sendFrame(frame);
}

void LogicController::discoverModules() {
    uint8_t payload[1] = {0x01};
    auto frame = protocol::buildFrame(0xFF, 0x01, payload, sizeof(payload));
    transport_->sendFrame(frame);

    // Демонстрационное динамическое наполнение (заменяется ответами от реальных модулей).
    if (xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
        if (state_->modules.empty()) {
            ModuleInfo m1{};
            m1.uid = 0x1001;
            m1.address = 1;
            m1.firmware = "1.2.0";
            m1.online = true;
            m1.lastSeenMs = millis();
            for (uint8_t i = 0; i < 8; ++i) m1.channels.push_back({i, String("CH ") + (i + 1), false});
            state_->modules.push_back(m1);

            ModuleInfo m2{};
            m2.uid = 0x1002;
            m2.address = 2;
            m2.firmware = "1.1.3";
            m2.online = true;
            m2.lastSeenMs = millis();
            for (uint8_t i = 0; i < 4; ++i) m2.channels.push_back({i, String("CH ") + (i + 1), false});
            state_->modules.push_back(m2);
        } else {
            for (auto &m : state_->modules) m.lastSeenMs = millis();
        }
        xSemaphoreGive(stateMutex_);
    }
    addEvent(*state_, "Discovery done");
}

void LogicController::updateModulePresence() {
    if (xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(20)) != pdTRUE) return;
    uint32_t now = millis();
    for (auto &m : state_->modules) {
        m.online = (now - m.lastSeenMs) < 15000;
    }
    state_->modules.erase(std::remove_if(state_->modules.begin(), state_->modules.end(),
                                         [&](const ModuleInfo &m) { return !m.online; }),
                          state_->modules.end());
    xSemaphoreGive(stateMutex_);
}

void LogicController::applyLanguage(Lang lang) {
    state_->settings.language = lang;
    prefs_.putUChar("lang", static_cast<uint8_t>(lang));
}

void LogicController::handleEvent(const AppEvent &event) {
    switch (event.type) {
        case AppEventType::DiscoverModules:
        case AppEventType::ModuleRescan:
            discoverModules();
            break;
        case AppEventType::SetChannelState: {
            if (xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
                for (auto &m : state_->modules) {
                    if (m.address == event.moduleAddr) {
                        for (auto &ch : m.channels) {
                            if (ch.id == event.channelId) {
                                ch.state = event.valueBool;
                                sendSetChannelState(m.address, ch.id, ch.state);
                                addEvent(*state_, "SET_CHANNEL_STATE sent");
                                break;
                            }
                        }
                    }
                }
                xSemaphoreGive(stateMutex_);
            }
            break;
        }
        case AppEventType::WifiSaveCreds:
            saveWifiCreds(event.ssid, event.pass);
            wifiAutoConnect();
            break;
        case AppEventType::WifiConnect:
            wifiAutoConnect();
            break;
        case AppEventType::WifiStartAp:
            startApMode();
            break;
        case AppEventType::SetLanguage:
            applyLanguage(event.language);
            break;
        default:
            break;
    }
}

void LogicController::handleWebServer() {
    if (webStarted_) web_.handleClient();
}

void LogicController::tick() {
    state_->comm = transport_->stats();
    handleWebServer();

    if (millis() - lastDiscoverMs_ > 10000) {
        lastDiscoverMs_ = millis();
        discoverModules();
    }
    updateModulePresence();
}

} // namespace app
