#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "iot-dev";
const char* password =  "systemsecurity";
const char* api_key = "XC4zMQ5cM47H01gC4AM3";
const char* fingerprint = "EF:78:01:91:62:E4:71:6C:E2:3A:42:A1:72:DE:72:CE:C3:F7:B8:4D:17:79:6F:37:6C:10:66:DC:BB:EC:86:FB";
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDITCCAgmgAwIBAgIUWoQrWE9Ehk80MJ8PINneAjKPEJwwDQYJKoZIhvcNAQEL\n"\
"BQAwIDEeMBwGA1UEAwwVc3lzc2VjLmV3aS51dHdlbnRlLm5sMB4XDTE5MDUwNDE5\n"\
"NTIyM1oXDTE5MDYwMzE5NTIyM1owIDEeMBwGA1UEAwwVc3lzc2VjLmV3aS51dHdl\n"\
"bnRlLm5sMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAwNX4kuy4h7ZM\n"\
"l3LFIE4kZdnHEe2xo1WyDGzRjD1P3F8RYGNJIZ0gWNC+841mOj8Ko8fvX5wVlZbs\n"\
"UIHp1EdDtC63G2kRHwBJxVkwgmcgAzaM4JujxBhziEVle0kau54FiTuzHAVLhtvd\n"\
"O/exyTIrMsfiQOSKt86u+aoxqQltuElS7X2INHlEkVG/mLVNm2S3N2970D7kkY2e\n"\
"+q6aKwYPcV4a3kUkWQUjle3xIuEp6gtIXpcE8jFHfRXx23XZT09Wy6YFYMcDfdQJ\n"\
"VzZb5TCH14/sLRfwS7Fcg8ujDcPZZUwVl6DUg530XPwCCcndyQbWwQgMdXAevKUI\n"\
"GhXRux4HXwIDAQABo1MwUTAdBgNVHQ4EFgQUlg5wQl6M4mtrzUYEWfQXbUgeRqEw\n"\
"HwYDVR0jBBgwFoAUlg5wQl6M4mtrzUYEWfQXbUgeRqEwDwYDVR0TAQH/BAUwAwEB\n"\
"/zANBgkqhkiG9w0BAQsFAAOCAQEAuuuLoCUlh7W1geAXKPtrhKK0OtLoYP4wS1zM\n"\
"x49hrBwG7sOCU0BAFAw5AQGxM3fqYwC5JagCFFKQXpfsHaNNFT5jXg3fMwfj/zul\n"\
"42iHR2ZUvVvbjK69lCaDsidI7OyBTHRsnxerGmNUYpBzPYzL/85JvlM3ACVTm3a5\n"\
"xu0jM3wKvpDTrcC9PGyYE+54FwIPy3eSgqqo9u6eT4AMCoaYQS5dQpe9+Uj76SI1\n"\
"Jv+q/znPoKKHJFVXNC3RQlpRMP5r5/AOZ/YlLsMuKtiJ9qroevTB4lwENWdO1gSy\n"\
"SwYJuUeJJwW4YXO7aVZOEGWSlS2ilsDqPoHk4Son5+37shSk3g==\n"\
"-----END CERTIFICATE-----";

#define mqtt_port 1883
#define mqtt_username "wemos14"
#define mqtt_host "syssec.ewi.utwente.nl"
#define mqtt_password "T2LZ3oioPr"
#define mqtt_recv "led/wemos14/action"

WiFiClientSecure secureClient;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const char led = 4;

void connect(const char* host,const char* password, int port,const char* username){
  Serial.print("connecting to mqtt");
  mqttClient.setServer(host,port);
  delay(1000);
  while(!mqttClient.connected()){
    if(mqttClient.connect("ESP32Client-1234",username,password)){
      Serial.println("connected to mqtt server");
      mqttClient.subscribe(mqtt_recv);
      Serial.println("subscribed to mqtt topic");
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);

  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  /* we got '1' -> on */
  if ((char)payload[0] == '1') {
    digitalWrite(led, HIGH); 
  } else {
    /* we got '0' -> on */
    digitalWrite(led, LOW);
  }
}

void https_connect(){
  HTTPClient https;
  
  Serial.print("[HTTPS] begin...\n");
  
  if (https.begin("https://syssec.ewi.utwente.nl/startup")) {  // HTTPS
    https.addHeader("Content-Type", "application/json");
    char json[] = "{\"apikey\": \"XC4zMQ5cM47H01gC4AM3\"}";
    Serial.print("[HTTPS] POST...\n");
    // start connection and send HTTP header
    
    int httpCode = https.POST(json);

    // httpCode will be negative on error
    if (httpCode > 0) {

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();

        StaticJsonDocument<200> doc;
        deserializeJson(doc,payload);

        const char* host = doc["mqtt"]["host"];
        const char* password = doc["mqtt"]["password"];
        const int port = doc["mqtt"]["port"];
        const char* username = doc["mqtt"]["username"];

        connect(host,password, port, username);
        
      
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(ssid, password);
  delay(1000);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("connected to wifi");

  delay(100);

  mqttClient.setCallback(callback);
  secureClient.setCACert(root_ca);

  if (!secureClient.connect("syssec.ewi.utwente.nl", 443)) {
    Serial.println("connection failed");
  }
  if (!secureClient.verify(fingerprint,"syssec.ewi.utwente.nl")){
    Serial.println("Certificate incorrect!");
  } else{
    Serial.println("Certificate correct!");
  }

  https_connect();

}

void loop() {
  mqttClient.loop();
}

