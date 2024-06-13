#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <GyverINA.h>

const int chipSelectPin = 53;

INA219 ina219_1(0.1f, 2.0f, 0x40);
INA219 ina219_2(0.1f, 2.0f, 0x41);

// OLED display
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

void setup() {  //--------------------------------------------------------------------------------------------
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  //---------------------------------------------------
  //---------------------------------------------------
  while (!SD.begin(chipSelectPin)) {
    Serial.println("SD card initialization failed!");
    display.println("SD failed");
    display.display();
    display.setCursor(0, 0);
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  Serial.println("SD card initialized successfully.");
  display.println("SD initialized");
  display.display();
  //---------------------------------------------------
  //---------------------------------------------------
  Serial.println(F("INA219..."));
  display.println("INA219...");
  display.display();

  if (ina219_1.begin()) {
    Serial.println(F("ina1 connected!"));
    display.print("S1 ok, ");
  } else {
    Serial.println(F("ina1 not found!"));
    display.print("S1 err, ");
    while (1)
      ;
  }
  display.display();
  if (ina219_2.begin()) {
    Serial.println(F("ina2 connected!"));
    display.println("S2 ok");
  } else {
    Serial.println(F("ina2 not found!"));
    display.println("S2 err");
    while (1)
      ;
  }
  display.display();

  // Serial.print(F("Calibration value1: "));
  // Serial.println(ina219_1.getCalibration());
  // Serial.print(F("Calibration value2: "));
  // Serial.print(ina219_2.getCalibration());
  ina219_1.setResolution(INA219_VBUS, INA219_RES_12BIT_X4);      // Напряжение в 12ти битном режиме + 4х кратное усреднение
  ina219_1.setResolution(INA219_VSHUNT, INA219_RES_12BIT_X128);  // Ток в 12ти битном режиме + 128х кратное усреднение
  ina219_2.setResolution(INA219_VBUS, INA219_RES_12BIT_X4);      // Напряжение в 12ти битном режиме + 4х кратное усреднение
  ina219_2.setResolution(INA219_VSHUNT, INA219_RES_12BIT_X128);  // Ток в 12ти битном режиме + 128х кратное усреднение

  //---------------------------------------------------
  //---------------------------------------------------
  File dataFile = SD.open("volamp.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("S1Volt S1Amp S2Volt S2Amp time");

    dataFile.close();
    Serial.println("Data saved to SD card.");
    display.println("          SD saved");
  } else {
    Serial.println("Error opening data file on SD card.");
    display.println("          SD error");
  }

  //---------------------------------------------------

  // Serial.println("");

  delay(2000);
}

void loop() {  //--------------------------------------------------------------------------------------------
  // Serial.println("Start");

  display.clearDisplay();
  // display.setTextSize(0);
  // display.setTextColor(WHITE);
  display.setCursor(0, 0);

  logDataToSDCard();
  delay(100);

  display.display();

}  //--------------------------------------------------------------------------------------------

void logDataToSDCard() {
  // Read values from sensor 1
  float voltage1 = ina219_1.getVoltage();
  float current1 = ina219_1.getCurrent();

  // Read values from sensor 2
  float voltage2 = ina219_2.getVoltage();
  float current2 = ina219_2.getCurrent();

  // Print to the serial monitor
  Serial.print("S1:");
  Serial.print(voltage1);
  Serial.print("V,");
  Serial.print(current1);
  Serial.println("A");
  Serial.print("S2:");
  Serial.print(voltage2);
  Serial.print("V,");
  Serial.print(current2);
  Serial.println("A");

  // Print to the display
  display.print("S1:");
  display.print(voltage1, 2);
  display.print("V ");
  display.print(current1, 3);
  display.print("A");
  display.println();
  display.print("S2:");
  display.print(voltage2, 2);
  display.print("V ");
  display.print(current2, 3);
  display.println("A");

  // Save the values to the SD card
  File dataFile = SD.open("volamp.csv", FILE_WRITE);
  if (dataFile) {
    // dataFile.print("Sensor 1 - Voltage: ");
    dataFile.print(voltage1, 3);
    // dataFile.print("V, Current:");
    dataFile.print(",");
    dataFile.print(current1, 3);
    // dataFile.println("A");
    dataFile.print(",");

    // dataFile.print("Sensor 2 - Voltage: ");
    dataFile.print(voltage2, 3);
    // dataFile.print("V, Current:");
    dataFile.print(",");
    dataFile.print(current2, 3);
    // dataFile.println("A");
    dataFile.print(",");

    dataFile.print(millis());
    // dataFile.println("s");
    dataFile.println(",");

    dataFile.close();
    Serial.println("Data saved to SD card.");
    display.println("          Logging...");
  } else {
    Serial.println("Error opening data file on SD card.");
    display.println("          SD error");
    SD.begin(chipSelectPin);
  }
}
