#include "mqtt_reporter.h"

AsyncMqttClient mqttClient;
volatile SemaphoreHandle_t xMutex = NULL;

void onConnected(bool persisted) {
    Serial.printf("onConnected: %d\n", persisted);

    if(xMutex) {
        xSemaphoreGive(xMutex);
    }
}
void onPublish(uint32_t packetId) {
    Serial.printf("onPublish: %x\n", packetId);
    if(xMutex) {
        xSemaphoreGive(xMutex);
    }
}

void onDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.printf("onDisconnect: %x\n", reason);
    if(xMutex) {
        xSemaphoreGive(xMutex);
    }
}

void MqttReporter::begin() {
    mqttClient.onConnect(onConnected);
    mqttClient.onPublish(onPublish);
    mqttClient.onDisconnect(onDisconnect);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);

    xMutex = xSemaphoreCreateMutex();
    xSemaphoreTake(xMutex, 0);

    pinMode(BUILTIN_LED, OUTPUT);
}

void MqttReporter::report(mqtt_data_map_t &dataMap) {
    size_t tReport = millis();
    digitalWrite(BUILTIN_LED, LOW);

    // Get the lower 3 bytes of 6 byte MAC
    // aa:bb:cc:dd:ee:ff -> dd:ee:ff
    // (3 byte shift, byte-swap and 1 byte shift)
    uint32_t partialMac = __bswap_32(ESP.getEfuseMac() >> 24) >> 8;
    String clientId = String("eee-") + String(partialMac, HEX);

    // Init WiFi
    WiFi.setHostname(clientId.c_str());
    if(!WiFi.isConnected()) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    if(WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi connection failed.");
        return;
    }

    // Init MQTT client
    size_t tConnect = millis();
    mqttClient.setClientId(clientId.c_str());
    mqttClient.connect();
    if(xSemaphoreTake(xMutex, 3000) == pdTRUE) {
        Serial.println("mqttClient connected");
    } else {
        Serial.println("mqttClient connect() timeout");
    }
    Serial.printf("tConnect: %u\n", millis() - tConnect);

    // Publish data
    uint8_t qos = 0;
    for(auto iter = dataMap.begin(); iter != dataMap.end(); iter++) {
        if(std::next(iter) == dataMap.end()) {
            qos = 1; // FIXME: dirty hack to trigger onPublish() cb
        }
        mqttClient.publish(
            String(MQTT_TOPIC_PREFIX + clientId + "/" + iter->first).c_str(),
            qos, false, iter->second.c_str());
    }
    if(xSemaphoreTake(xMutex, 6000) == pdTRUE) {
        Serial.println("Successfully published");
    } else {
        Serial.println("publish() not ACKed");
    }

    // Disconnect from the MQTT server
    size_t tDisconnect = millis();
    mqttClient.disconnect(false);
    if(xSemaphoreTake(xMutex, 6000) == pdTRUE) {
        Serial.println("mqttClient disconnected");
    } else {
        Serial.println("mqttClient disconnect() timeout");
    }
    Serial.printf("tDisconnect: %u\n", millis() - tDisconnect);

    // Disconnect from the AP
    WiFi.disconnect(true);

    digitalWrite(BUILTIN_LED, HIGH);
    Serial.printf("tReport: %lu\n", millis() - tReport);
}
