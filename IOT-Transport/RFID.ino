#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <MFRC522.h>
#include <SPI.h>

#define SDA_1 4 // D2
#define SDA_2 2 // D4
#define RST_PIN 5 // D1

MFRC522 mfrc522_1(SDA_1, RST_PIN);
MFRC522 mfrc522_2(SDA_2, RST_PIN);

String onBoard[50]; 
int passengerCount = 0;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  WiFi.begin("Navindu's iPhone", "123456789"); //
  while (WiFi.status() != WL_CONNECTED) { delay(500); }

  config.api_key = "AIzaSyApEIhe1inDAWGLUZgheUkhw1Cg0dbwp-k"; //
  config.database_url = "https://bus-project-3f559-default-rtdb.asia-southeast1.firebasedatabase.app/"; //
  auth.user.email = "nethushailukpitiya@gmail.com"; //
  auth.user.password = "Nethu@12345"; //

  Firebase.begin(&config, &auth);
  SPI.begin();
  
  // Explicitly initialize both
  mfrc522_1.PCD_Init();
  mfrc522_2.PCD_Init();
  Serial.println("System Ready - D2 and D4 Active.");
}

void loop() {
  // Check D2
  if (mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial()) {
    handleToggle(mfrc522_1, "Scanner_D2");
  }
  
  // Check D4
  if (mfrc522_2.PICC_IsNewCardPresent() && mfrc522_2.PICC_ReadCardSerial()) {
    handleToggle(mfrc522_2, "Scanner_D4");
  }
}

void handleToggle(MFRC522 &rfid, String scannerName) {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  int foundIndex = -1;
  for (int i = 0; i < 50; i++) {
    if (onBoard[i] == uid) {
      foundIndex = i;
      break;
    }
  }

  // Serial Print - Only prints ONCE per scan now
  Serial.println("-----------------------");
  Serial.print("Source: "); Serial.println(scannerName);
  Serial.print("ID: "); Serial.println(uid);

  if (foundIndex != -1) {
    onBoard[foundIndex] = "";
    if (passengerCount > 0) passengerCount--;
    Serial.println("STATUS: EXIT (Count Decreased)");
  } else {
    for (int i = 0; i < 50; i++) {
      if (onBoard[i] == "") {
        onBoard[i] = uid;
        passengerCount++;
        Serial.println("STATUS: ENTRY (Count Increased)");
        break;
      }
    }
  }

  FirebaseJson json;
  json.set("passengers", passengerCount);
  json.set("last_uid", uid);
  Firebase.updateNode(fbdo, "/bus1", json); //
  
  // CRITICAL: Cleanup for the next scan
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1(); 
  delay(3000); // Prevents multiple rapid prints in Serial Monitor
}
