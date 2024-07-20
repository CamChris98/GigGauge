#include <Wire.h>
#include <SparkFun_ADXL345.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>


ADXL345 adxl = ADXL345();
LiquidCrystal_I2C lcd(0x27, 16, 2);

int range = 2; // Range: 2g, 4g, 8g, 16g
int CS_PIN = 10;
float ax, ay, az; // Accelerations
float vx = 0, vy = 0, vz = 0; // Velocities
float sx = 0, sy = 0, sz = 0; // Displacements
unsigned long previousTime = 0;
const float accelerationScale = 0.0039; // Scale factor for 10-bit ADXL345 readings (LSB/g)
const float g = 9.81; // Acceleration due to gravity (m/s^2)

float distanceMiles = 0.0; 

const int chipSelect = 2;
const byte rxPin = 9;
const byte txPin = 8;
SoftwareSerial bluetooth(rxPin,txPin); //pins for bluetooth module rx and tx pins


void setup() {


  pinMode(rxPin, INPUT); //bluetooth receiving pin 
  pinMode(txPin,OUTPUT); // bluetooth sending pin

  Serial.begin(9600);
  adxl.powerOn(); //accelerometer
  adxl.setRangeSetting(range); //accelerometer 
  previousTime = millis(); //accelerometer

  bluetooth.begin(9600); //bluetooth 

  lcd.init(); //lcd 
  lcd.backlight();//lcd 

  // Initialize SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization failed!");
    while (1);
  }
  Serial.println("Initialization done.");


}

void loop() {
  int x, y, z;
  adxl.readAccel(&x, &y, &z);

  // Convert to acceleration in m/s^2
  ax = x * accelerationScale * g;
  ay = y * accelerationScale * g;
  az = z * accelerationScale * g;

  // Get the current time
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - previousTime) / 1000.0; // Time in seconds
  previousTime = currentTime;

  // Integrate acceleration to get velocity
  vx += ax * deltaTime;
  vy += ay * deltaTime;
  vz += az * deltaTime;

  // Integrate velocity to get displacement
  sx += vx * deltaTime;
  sy += vy * deltaTime;
  sz += vz * deltaTime;

  // Convert displacement to miles
  distanceMiles = sqrt(sx * sx + sy * sy + sz * sz) / 1609.34; // 1 mile = 1609.34 meters

  Serial.print("Distance traveled (miles): ");
  Serial.println(distanceMiles);
  lcd.setCursor(1,0);
  lcd.print("Miles: ");
  lcd.print(distanceMiles);
  delay(3000);

  Serial.print("X: ");
  Serial.print(x);
  

  Serial.print("    Y: ");
  Serial.print(y);
  
  Serial.print("    Z: ");
  Serial.println(z);
  
  delay(250);

 File dataFile = SD.open("data.txt", FILE_WRITE);
 
  if (dataFile) {
    dataFile.println(distanceMiles);
    dataFile.close();
    Serial.println("Data written to file.");
  } else {
    Serial.println("Error opening data.txt for writing.");
  }

 if (bluetooth.available()) {
    char data = bluetooth.read();
    Serial.print("Received: ");
    Serial.println(data);


  if (Serial.available()) {
    char data = Serial.read();
    bluetooth.write(distanceMiles);  // Send data to the smartphone
  }
  delay(5000);  // Wait before reading again
  }
}
