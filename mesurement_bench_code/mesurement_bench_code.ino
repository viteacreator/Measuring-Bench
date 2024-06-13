#include <Wire.h>
#include <SD.h>
#include <SPI.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <GyverINA.h>
#include <GyverOLED.h>
#include "menu.h"
#include "button.h"

// #define MAX_ITEM_NAME_LENGTH 25
#define chipSelectPin 53

INA219 ina219_1(0.1f, 2.0f, 0x40);
INA219 ina219_2(0.1f, 2.0f, 0x41);

GyverOLED<SSD1306_128x64, OLED_BUFFER> display;

Menu mainMenu, subMenu1, subMenu2, subMenu3;
Menu *currentMenu;
Button upBtn, okBtn, dwBtn;

void menuParamInit() {
  rootMenu = &mainMenu;
  currentMenu = rootMenu;

  char mainMenuName[MAX_ITEM_NAME_LENGTH] = "Main menu";
  char mainMenuOptions[][MAX_ITEM_NAME_LENGTH] = {
    "1.SimpleLoggs",
    "2.Efficiency",
    "3.Capacity",
    "4.AmplFreqCaract",
    "5.SomethingElse"
  };
  initMenu(&mainMenu, 5, mainMenuOptions);
  setMenuName(&mainMenu, mainMenuName);

  char subMenu1Options[][MAX_ITEM_NAME_LENGTH] = {
    "0.<-Back",
    "1.S1(V,A,W),time",
    "2.S1,S2(V,A,W),time",
  };
  initMenu(&subMenu1, 3, subMenu1Options);

  char subMenu2Options[][MAX_ITEM_NAME_LENGTH] = {
    "0.<-Back",
    "1.S1/S2(V,A,W) % time",
  };
  initMenu(&subMenu2, 2, subMenu2Options);
}

void normalMenuPrint(Menu *currMenu) {
  display.clear();
  display.setCursorXY(30, 0);
  display.println(getMenuName(currMenu));
  display.setCursorXY(0, 8);
  for (int i = 0; i < getMaxItems(currMenu); i++) {
    if (i + 1 == getMenuState(currMenu)) {
      display.invertText(true);
      display.println(getItemName(currMenu, i + 1));
      display.invertText(false);
    } else display.println(getItemName(currMenu, i + 1));
  }
  display.line(4, 7, 120, 7);
  display.update();
}

void smallMenuPrint(Menu *currMenu) {
  display.setCursorXY(0, 0);
  display.println("      Main Menu");
  display.line(4, 63 - 7, 120, 63 - 7);
  display.setCursorXY(0, 9);
}

void confirmMenuPrint(Menu *currMenu) {
  display.setCursorXY(0, 0);
  display.roundRect(16, 16, 123 - 16, 63 - 15, OLED_FILL);
  display.invertText(true);
  display.setCursorXY(15 + 11, 24);
  display.print("Confirm Back");
  display.setCursorXY(24, 40);
  display.print("No");
  display.setCursorXY(84, 40);
  display.print("Yes");
  display.update();
  display.invertText(false);
}

void buttInit() {
  initBtn(&upBtn, 14);
  initBtn(&okBtn, 15);
  initBtn(&dwBtn, 16);
}

void setup() {  //--------------------------------------------------------------------------------------------
  Serial.begin(9600);

  display.init();
  menuParamInit();
  buttInit();

  display.clear();
  display.setScale(1);
  display.invertDisplay(false);
  display.setCursor(0, 0);
  //---------------------------------------------------
  //---------------------------------------------------
  // while (!SD.begin(chipSelectPin)) {
  //   Serial.println("SD card initialization failed!");
  //   display.println("SD failed");
  //   display.update();
  //   display.setCursor(0, 0);
  // }
  display.clear();
  display.setCursor(0, 0);
  Serial.println("SD card initialized successfully.");
  display.println("SD initialized");
  display.update();
  //---------------------------------------------------
  //---------------------------------------------------
  Serial.println(F("INA219..."));
  display.println("INA219...");
  display.update();

  // if (ina219_1.begin()) {
  //   Serial.println(F("ina1 connected!"));
  //   display.print("S1 ok, ");
  // } else {
  //   Serial.println(F("ina1 not found!"));
  //   display.print("S1 err, ");
  //   while (1)
  //     ;
  // }
  // display.update();
  // if (ina219_2.begin()) {
  //   Serial.println(F("ina2 connected!"));
  //   display.println("S2 ok");
  // } else {
  //   Serial.println(F("ina2 not found!"));
  //   display.println("S2 err");
  //   while (1)
  //     ;
  // }
  // display.update();

  // // Serial.print(F("Calibration value1: "));
  // // Serial.println(ina219_1.getCalibration());
  // // Serial.print(F("Calibration value2: "));
  // // Serial.print(ina219_2.getCalibration());
  // ina219_1.setResolution(INA219_VBUS, INA219_RES_12BIT_X4);      // Напряжение в 12ти битном режиме + 4х кратное усреднение
  // ina219_1.setResolution(INA219_VSHUNT, INA219_RES_12BIT_X128);  // Ток в 12ти битном режиме + 128х кратное усреднение
  // ina219_2.setResolution(INA219_VBUS, INA219_RES_12BIT_X4);      // Напряжение в 12ти битном режиме + 4х кратное усреднение
  // ina219_2.setResolution(INA219_VSHUNT, INA219_RES_12BIT_X128);  // Ток в 12ти битном режиме + 128х кратное усреднение

  // ---------------------------------------------------
  // ---------------------------------------------------
  File dataFile = SD.open("volamp.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("S1Volt S1Amp S2Volt S2Amp time");

    dataFile.close();
    Serial.println("Data saved to SD card.");
    display.println("          Logging...");
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

  normalMenuPrint(currentMenu);
  if (check(&upBtn, 1000)) {
    nextItem(currentMenu);
  }
  if (check(&dwBtn, 1000)) {
    prevItem(currentMenu);
  }

  // delay(2000);

  // confirmMenuPrint(&subMenu2, 6);
  // delay(2000);

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
  // File dataFile = SD.open("volamp.csv", FILE_WRITE);
  // if (dataFile) {
  //   // dataFile.print("Sensor 1 - Voltage: ");
  //   dataFile.print(voltage1, 3);
  //   // dataFile.print("V, Current:");
  //   dataFile.print(",");
  //   dataFile.print(current1, 3);
  //   // dataFile.println("A");
  //   dataFile.print(",");

  //   // dataFile.print("Sensor 2 - Voltage: ");
  //   dataFile.print(voltage2, 3);
  //   // dataFile.print("V, Current:");
  //   dataFile.print(",");
  //   dataFile.print(current2, 3);
  //   // dataFile.println("A");
  //   dataFile.print(",");

  //   dataFile.print(millis());
  //   // dataFile.println("s");
  //   dataFile.println(",");

  //   dataFile.close();
  //   Serial.println("Data saved to SD card.");
  //   display.println("          Logging...");
  // } else {
  //   Serial.println("Error opening data file on SD card.");
  //   display.println("          SD error");
  //   SD.begin(chipSelectPin);
  // }
}
