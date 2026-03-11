#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

//RX=D5,TX=D6
SoftwareSerial neo6(14, 12); 
TinyGPSPlus gps;

#define WIFI_SSID "SLT-Fiber" 
#define WIFI_PASSWORD "0112953757"
#define API_KEY "AIzaSyApEIhe1inDAWGLUZgheUkhw1Cg0dbwp-k"
#define DATABASE_URL "https://bus-project-3f559-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "nethushailukpitiya@gmail.com"
#define USER_PASSWORD "Nethu@12345"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  neo6.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("\nAuthenticated GPS Tracker Online");
}

void loop() {
  while (neo6.available() > 0) {
    if (gps.encode(neo6.read())) {
      if (gps.location.isValid() && gps.location.isUpdated()) {
        FirebaseJson json;
        json.set("lat", gps.location.lat());
        json.set("lng", gps.location.lng());
        
        if (Firebase.updateNode(fbdo, "/bus1", json)) {
          Serial.printf("Location Sent: %f, %f\n", gps.location.lat(), gps.location.lng());
        }
        delay(10000); 
      }
    }
  }
}
