#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <MFRC522.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

/* WIFI */
#define WIFI_SSID "SLT-Fiber"
#define WIFI_PASSWORD "0112653757"

/* FIREBASE */
#define API_KEY "AIzaSyApEIhe1inDAWGLUZgheUkhw1Cg0dbwp-k"
#define DATABASE_URL "https://bus-project-3f559-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "nethushailukpitiya@gmail.com"
#define USER_PASSWORD "Nethu@12345"

/* RFID */
#define SDA_1 4
#define SDA_2 2
#define RST_PIN 5

/* GPS */
#define GPS_RX 0
#define GPS_TX 15

/* ULTRASONIC */
#define TRIG 16
#define ECHO 15

MFRC522 rfid1(SDA_1, RST_PIN);
MFRC522 rfid2(SDA_2, RST_PIN);

TinyGPSPlus gps;
SoftwareSerial neo6(GPS_RX, GPS_TX);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String onBoard[50];
int passengerCount = 0;

String doorStatus = "CLOSED";
String lastDoorStatus = "";

void setup() {

Serial.begin(115200);

neo6.begin(9600);

pinMode(TRIG, OUTPUT);
pinMode(ECHO, INPUT);

/* WIFI */

WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
Serial.print("Connecting WiFi");

while (WiFi.status() != WL_CONNECTED){
delay(500);
Serial.print(".");
}

Serial.println("\nWiFi Connected");

/* FIREBASE */

config.api_key = API_KEY;
config.database_url = DATABASE_URL;
auth.user.email = USER_EMAIL;
auth.user.password = USER_PASSWORD;

Firebase.begin(&config,&auth);
Firebase.reconnectWiFi(true);

/* RFID */

SPI.begin();
rfid1.PCD_Init();
rfid2.PCD_Init();

Serial.println("System Ready");

}

void loop(){

readGPS();

/* RFID READERS */

scanRFID(rfid1,"Scanner_D2");
scanRFID(rfid2,"Scanner_D4");

/* DOOR SENSOR */

doorStatus = readDoor();

if(doorStatus != lastDoorStatus){

Serial.print("Door Status: ");
Serial.println(doorStatus);

FirebaseJson json;
json.set("door",doorStatus);

Firebase.updateNode(fbdo,"/bus1",json);

lastDoorStatus = doorStatus;

}

delay(100);

}

/* RFID SCAN FUNCTION */

void scanRFID(MFRC522 &rfid,String scanner){

rfid.PCD_Init();

if (!rfid.PICC_IsNewCardPresent()) return;
if (!rfid.PICC_ReadCardSerial()) return;

String uid="";

for(byte i=0;i<rfid.uid.size;i++){
if(rfid.uid.uidByte[i] < 0x10) uid += "0";
uid += String(rfid.uid.uidByte[i],HEX);
}

uid.toUpperCase();

int found=-1;

for(int i=0;i<50;i++){
if(onBoard[i]==uid){
found=i;
break;
}
}

Serial.println("------------------");
Serial.print("Source: ");
Serial.println(scanner);

Serial.print("ID: ");
Serial.println(uid);

if(found!=-1){

onBoard[found]="";

if(passengerCount>0) passengerCount--;

Serial.println("STATUS: EXIT");

}
else{

for(int i=0;i<50;i++){
if(onBoard[i]==""){
onBoard[i]=uid;
passengerCount++;
break;
}
}

Serial.println("STATUS: ENTRY");

}

/* FIREBASE UPDATE */

FirebaseJson json;

json.set("passengers",passengerCount);
json.set("last_uid",uid);

Firebase.updateNode(fbdo,"/bus1",json);

rfid.PICC_HaltA();
rfid.PCD_StopCrypto1();

delay(1200);

}

/* GPS */

void readGPS(){

while(neo6.available()){
gps.encode(neo6.read());
}

if(gps.location.isValid() && gps.location.isUpdated()){

float lat=gps.location.lat();
float lng=gps.location.lng();

Serial.print("LAT: ");
Serial.println(lat);

Serial.print("LNG: ");
Serial.println(lng);

FirebaseJson json;

json.set("lat",lat);
json.set("lng",lng);

Firebase.updateNode(fbdo,"/bus1",json);

}

}

/* DOOR SENSOR */

String readDoor(){

digitalWrite(TRIG,LOW);
delayMicroseconds(2);

digitalWrite(TRIG,HIGH);
delayMicroseconds(10);

digitalWrite(TRIG,LOW);

long duration=pulseIn(ECHO,HIGH);

long distance=duration*0.034/2;

if(distance<15){
return "OPEN";
}
else{
return "CLOSED";
}

}
