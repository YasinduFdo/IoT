#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); 

  Serial.println("Initializing MPU6050 Fall Detection Module...");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { delay(10); }
  }
  Serial.println("MPU6050 Found!");
  
  // Set accelerometer range for sudden impacts
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Calculate total acceleration (vector sum)
  float totalAccel = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));

  // If total acceleration drops near 0 (freefall) or spikes massively (impact)
  if (totalAccel < 2.0 || totalAccel > 20.0) {
    Serial.println("⚠️ ALERT: Possible Fall Detected!");
    // Later, the team can call a Firebase upload function here
    delay(2000); // Prevent spamming alerts
  }

  delay(100);
}
