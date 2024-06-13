#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <GyverINA.h>
//#include "INA219ADC.h"
#include "INA226ADC.h"
#include "string.h"

#define LOOPDELAY 1  //ms
#define DISPUPDATEPERIOD 1600
#define CHIPSELECTPIN 53

#define DATABUFFERROWS 100  //for colectedData buffer max 255
#define DATABUFFERCOLOMNS 4
//#define PAKAGETOSDSIZE 10  // 10 rows of data from ram

enum colData {
  S1voltage,
  S1current,
  S2voltage,
  S2current
};
//-----------------------------------------------------------------------------------------------------------
uint16_t updateDispPeriod = 0;
uint16_t prevUpdateDispTime = 0;

uint16_t sensorReadPeriod = 0;
uint16_t prevSensorReadTime = 0;

uint16_t SDWritePeriod = 0;
uint16_t prevSDWritePeriodTime = 0;

uint16_t loopPeriod = 0;
uint16_t prevLoopPeriodTime = 0;

uint16_t updateLoopPeriod = 0;
uint16_t prewUpdateLoopPeriod = 0;

uint16_t dispWritePeriod = 0;
uint16_t prewDispWritePeriod = 0;

uint16_t dispUpdate = 0;  //for fixed display update
uint32_t prewDispUpdate = 0;
//-----------------------------------------------------------------------------------------------------------

//uint8_t displayShowFlag = 0;
bool SDLoggingState;

//-----------------------------------------------------------------------------------------------------------

int16_t colectedData[DATABUFFERROWS][DATABUFFERCOLOMNS];
uint32_t colectedDataTiming[DATABUFFERROWS];
//char colectedDataStringed[(DATABUFFERCOLOMNS + 1) * 5 + 1];

int16_t displaiedData[DATABUFFERCOLOMNS];

//-----------------------------------------------------------------------------------------------------------
// INA219 ina219_1(0.1f, 2.0f, 0x40);
// INA219 ina219_2(0.1f, 2.0f, 0x41);

INA226 ina226_1(0.1f, 2.0f, 0x40);
INA226 ina226_2(0.1f, 2.0f, 0x41);

Adafruit_SSD1306 display(128, 64);

void setup() {  //-------------------------------------------------------------------------------------------
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //0x3C, 0x3D

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  //---------------------------------------------------
  //---------------------------------------------------
  while (!SD.begin(CHIPSELECTPIN)) {
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
  // Serial.println(F("INA219..."));
  // display.println("INA219...");
  Serial.println(F("INA226..."));
  display.println("INA226...");

  display.display();

  if (ina226_1.begin()) {
    Serial.println(F("ina1 connected!"));
    display.print("S1 ok, ");
    display.display();
  } else {
    Serial.println(F("ina1 not found!"));
    display.print("S1 err, ");
    display.display();
    // while (1)
    //   ;
  }

  if (ina226_2.begin()) {
    Serial.println(F("ina2 connected!"));
    display.println("S2 ok");
    display.display();
  } else {
    Serial.println(F("ina2 not found!"));
    display.println("S2 err");
    display.display();
    // while (1)
    //   ;
  }

  // Serial.print(F("Calibration value1: "));
  // Serial.println(ina226_1.getCalibration());
  // Serial.print(F("Calibration value2: "));
  // Serial.print(ina226_2.getCalibration());
  // ina226_1.setResolution(INA219_VBUS, INA219_RES_12BIT_X2);  // Напряжение в 12ти битном режиме + 4х кратное усреднение
  // ina226_1.setResolution(INA219_VSHUNT, INA219_RES_12BIT);   // Ток в 12ти битном режиме + 128х кратное усреднение
  // ina226_2.setResolution(INA219_VBUS, INA219_RES_12BIT_X2);  // Напряжение в 12ти битном режиме + 4х кратное усреднение
  // ina226_2.setResolution(INA219_VSHUNT, INA219_RES_12BIT);   // Ток в 12ти битном режиме + 128х кратное усреднение

  ina226_1.setSampleTime(INA226_VBUS, INA226_CONV_140US);    // Напряжение в 12ти битном режиме + 4х кратное усреднение
  ina226_1.setSampleTime(INA226_VSHUNT, INA226_CONV_140US);  // Ток в 12ти битном режиме + 128х кратное усреднение
  ina226_2.setSampleTime(INA226_VBUS, INA226_CONV_140US);    // Напряжение в 12ти битном режиме + 4х кратное усреднение
  ina226_2.setSampleTime(INA226_VSHUNT, INA226_CONV_140US);  // Ток в 12ти битном режиме + 128х кратное усреднение
  ina226_1.setAveraging(INA226_AVG_X1);
  ina226_2.setAveraging(INA226_AVG_X1);
  //---------------------------------------------------
  //firstLogDataToSDCard();
  //---------------------------------------------------
  //reset all prew timers

  prewUpdateLoopPeriod = millis();
  prevSensorReadTime = millis();
  prevSDWritePeriodTime = millis();
  prewDispWritePeriod = millis();
  prevUpdateDispTime = millis();
  //---------------------------------------------------

  display.display();
  delay(1000);
}

void loop() {  //--------------------------------------------------------------------------------------------
  // Serial.println("Start");
  display.clearDisplay();
  // display.setTextSize(1);
  // display.setTextColor(WHITE);
  display.setCursor(0, 0);

  updateLoopPeriod = millis() - prewUpdateLoopPeriod;
  prewUpdateLoopPeriod = millis();

  prevSensorReadTime = millis();
  sensorsRead();
  sensorReadPeriod = millis() - prevSensorReadTime;

  prevSDWritePeriodTime = millis();
  logDataToSDCard();
  SDWritePeriod = millis() - prevSDWritePeriodTime;

  if (millis() - prewDispUpdate >= DISPUPDATEPERIOD) {
    prewDispUpdate = millis();  // for constant update

    prewDispWritePeriod = millis();
    printToDisplay();
    dispWritePeriod = millis() - prewDispWritePeriod;  // for measuring display-print time

    updateDispPeriod = millis() - prevUpdateDispTime;  // for measuring display-update time
    prevUpdateDispTime = millis();
  }

  // delay(LOOPDELAY);

}  //--------------------------------------------------------------------------------------------

void computeDataForDisp() {
  float S1volt = 0;
  float S1curr = 0;
  float S2volt = 0;
  float S2curr = 0;
  float pondere = 1 / (float)DATABUFFERROWS;

  for (uint8_t iteratie = 0; iteratie < DATABUFFERROWS; iteratie++) {
    S1volt += pondere * (float)colectedData[iteratie][S1voltage];
    S1curr += pondere * (float)colectedData[iteratie][S1current];
    S2volt += pondere * (float)colectedData[iteratie][S2voltage];
    S2curr += pondere * (float)colectedData[iteratie][S2current];
  }

  displaiedData[S1voltage] = (uint16_t)S1volt;
  displaiedData[S1current] = (int16_t)S1curr;
  displaiedData[S2voltage] = (uint16_t)S2volt;
  displaiedData[S2current] = (int16_t)S2curr;
}

void printToDisplay() {

  computeDataForDisp();

  // Print to the display
  display.print("S1:");
  display.print(displaiedData[S1voltage]);
  display.print("mV ");
  // display.print((uint16_t)(voltage1*100));
  // display.print("cV ");
  display.print(displaiedData[S1current]);
  display.print("mA");
  display.println();
  display.print("S2:");
  display.print(displaiedData[S2voltage]);
  display.print("mV ");
  display.print(displaiedData[S2current]);
  display.println("mA");

  if (SDLoggingState) {
    // Serial.println("Data saved to SD card.");
    display.println("          Logging...");
  } else {
    // Serial.println("Error opening data file on SD card.");
    display.println("          SD error");
  }

  display.print("Du:");
  display.print(updateDispPeriod);
  display.print(" Lu:");
  display.println(updateLoopPeriod);

  display.print("Dw:");
  display.print(dispWritePeriod);
  display.print(" Sr:");
  display.print(sensorReadPeriod);
  display.print(" SDw:");
  display.print(SDWritePeriod);
  display.println();  //"ms");

  display.display();
}

void sensorsRead() {
  // Read values from sensors
  for (uint8_t iteratie = 0; iteratie < DATABUFFERROWS; iteratie++) {

    colectedData[iteratie][S1voltage] = (uint16_t)((ina226_1.getVoltage()) * 1000);
    colectedData[iteratie][S1current] = (int16_t)((ina226_1.getCurrent()) * 1000);
    colectedData[iteratie][S2voltage] = (uint16_t)((ina226_2.getVoltage()) * 1000);
    colectedData[iteratie][S2current] = (int16_t)((ina226_2.getCurrent()) * 1000);
    colectedDataTiming[iteratie] = millis();
  }
}

void logDataToSDCard() {
  // Save the values to the SD card
  File dataFile = SD.open("volamp.csv", FILE_WRITE);
  if (dataFile) {
    //------------------------------------------------------------------------------------
    // String dataString = "";

    // // Iterate through the data
    // for (uint8_t iteratie = 0; iteratie < DATABUFFERROWS; iteratie++) {
    //     // Append data for each row
    //     dataString += String((uint16_t)(colectedData[iteratie][S1voltage])) + "," +
    //                   String((int16_t)(colectedData[iteratie][S1current])) + "," +
    //                   String((uint16_t)(colectedData[iteratie][S2voltage])) + "," +
    //                   String((int16_t)(colectedData[iteratie][S2current])) + "," +
    //                   String(colectedDataTiming[iteratie]) + "\n";
    // }

    // // Write the entire data string to the file
    // dataFile.print(dataString);
    //------------------------------------------------------------------------------------
    // Prepare a character array to hold the entire matrix data
    const size_t bufferSize = DATABUFFERROWS * (DATABUFFERCOLOMNS * (6 + 1) + 10 + 1) + 1;  // 6 digits per int16_t, 1 for comma, 10 digits per uint32_t, 1 for newline, 1 for null terminator
    char dataBuffer[bufferSize];
    size_t bufferIndex = 0;

    //Iterate through the data
    for (uint8_t iteratie = 0; iteratie < DATABUFFERROWS; iteratie++) {
      // Append data for each row
      bufferIndex += snprintf(dataBuffer + bufferIndex, bufferSize - bufferIndex - 1,  //-1 will prevent the null terminator on final data
                              "%d,%d,%d,%d,%lu\n",
                              (int16_t)(colectedData[iteratie][S1voltage]),
                              (int16_t)(colectedData[iteratie][S1current]),
                              (int16_t)(colectedData[iteratie][S2voltage]),
                              (int16_t)(colectedData[iteratie][S2current]),
                              (uint32_t)colectedDataTiming[iteratie]);
    }

    // Write the entire data buffer to the file
    dataFile.write((uint8_t*)dataBuffer, bufferIndex);
    //------------------------------------------------------------------------------------
    dataFile.close();
    // Serial.println("Data saved to SD card.");
    // display.println("          Logging...");
    SDLoggingState = 1;
  } else {
    // Serial.println("Error opening data file on SD card.");
    // display.println("          SD error");
    SDLoggingState = 0;
    SD.begin(CHIPSELECTPIN);
  }
}

void firstLogDataToSDCard() {
  // Save the values to the SD card
  File dataFile = SD.open("volamp.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("S1mVolt,S1mAmp,S2mVolt,S2mAmp,TimeIn_msFromStart");
    dataFile.close();
    display.println("          Logged...");
  } else {
    SD.begin(CHIPSELECTPIN);
    display.println("          SD error");
  }
}