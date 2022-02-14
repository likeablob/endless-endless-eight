#include <Esp.h>
#include <WiFi.h>
#include <byteswap.h>
#include <freertos/semphr.h>
#include <map>

#include <AsyncMqttClient.h>

#include "user_config.h"

typedef std::map<String, String> mqtt_data_map_t;

namespace MqttReporter {
void begin();
void report(mqtt_data_map_t &dataMap);
} // namespace MqttReporter
