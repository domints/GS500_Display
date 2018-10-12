#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/Crasng24pt7.h>
#include "cmds.h"
#include "display.h"
#include "menu.h"
#include "icons.h"
#include "logo.h"

// ------ MODULE SETTINGS ------
#define ADDR 0x01
// ------ END OF MODULE SETTINGS ------

// ------ PROGRAM SETTINGS ------
#define HIGH_TRESHOLD 2250
#define LOW_TRESHOLD 1720
#define BTN_SCAN_RATE 50 //Button scan rate in ms
#define BTN_HOLD_TRESHOLD 1000 //Button hold treshold in ms

#define ICON_BLINK_RATE 500 //Icon blink rate in ms
// ------ END OF PROGRAM SETTINGS ------

// ------ CALCULATED MACROS -------
#define BTN_HOLD_CYCLES (BTN_HOLD_TRESHOLD / BTN_SCAN_RATE)
#define BTN_SCAN_RATE_US (BTN_SCAN_RATE * 1000)
#define ICON_BLINK_RATE_US (ICON_BLINK_RATE * 1000)
// ------ END OF CALCULATED MACROS -------

/* DISPLAY MODES
 * 0 -> Normal (Gear, Fuel, Clock, Voltage)
 * 1 -> Menu
 */
#define DM_NORMAL 0
#define DM_MENU 1
int displayMode = DM_NORMAL;

bool transmissionStarted = false;

volatile byte gear = 0;
volatile byte fuel = 0;
volatile byte voltageWhole = 0;
volatile byte voltageDecimal = 0;
volatile byte day = 0;
volatile byte month = 0;
volatile byte year = 0;
volatile byte hour = 0;
volatile byte minute = 0;
volatile byte second = 0;
volatile bool dayC = false;
volatile bool monthC = false;
volatile bool yearC = false;
volatile bool hourC = false;
volatile bool minuteC = false;
volatile bool batteryIcon = false;
volatile bool fuelIcon = false;
volatile bool coldIcon = false;
volatile bool heatIcon = false;

volatile bool battShown = false;
volatile bool fuelShown = false;
volatile bool coldShown = false;
volatile bool heatShown = false;

volatile bool lastBtn1 = false;
volatile bool lastBtn2 = false;

volatile int btn1Cycles = 0;
volatile int btn2Cycles = 0;

Adafruit_PCD8544 display = Adafruit_PCD8544(PA2, PA3, PA4);
#define NOMINAL_CONTRAST 53
byte contrast = 35;

HardwareTimer btnTimer(2);
HardwareTimer iconTimer(4);

void setupTimers()
{
  btnTimer.pause();
  btnTimer.setPeriod(BTN_SCAN_RATE_US);
  btnTimer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  btnTimer.setCompare(TIMER_CH1, 1);
  btnTimer.attachCompare1Interrupt(checkButtons);
  btnTimer.refresh();
  btnTimer.resume();

  iconTimer.pause();
  iconTimer.setPeriod(ICON_BLINK_RATE_US);
  iconTimer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  iconTimer.setCompare(TIMER_CH1, 1);
  iconTimer.attachCompare1Interrupt(printIcons);
  iconTimer.refresh();
}

void setup() {
  Serial1.begin(115200);
  Serial.begin(115200);
  randomSeed(analogRead(0));
  pinMode(PA1, OUTPUT);
  digitalWrite(PA1, HIGH);
  pinMode(PB0, INPUT_ANALOG);
  pinMode(PB1, INPUT_ANALOG);
  setupTimers();
  display.begin();
  display.setContrast(0);
  display.clearDisplay();
  display.setRotation(1);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
  display.drawBitmap(0, 0, boot_logo, 48, 84, 1);
  display.display();
  while(contrast < NOMINAL_CONTRAST)
  {
    setContrast(contrast + 1);
    delay(300);
  }
}

void loop() {
  if(Serial1.available())
  {
    if(Serial1.read() == ADDR)
    {
      byte msgSize = Serial1.read();
      while(msgSize -= workSerial(false));
    }
    else
    {
      byte msgSize = Serial1.read();
      for(byte i = 0; i < msgSize; i++)
      {
        while(!Serial1.available());
        Serial1.read();
      }
    }
  }
}

byte workSerial(bool hw)
{
  if(!transmissionStarted)
  {
    transmissionStarted = true;
    display.clearDisplay();
    display.display();
  }
  byte cmd = sRead(hw);
  byte hasRead = 1;
  if(cmd == CMD_LCD_GEAR)
  {
    gear = sRead(hw);
    //printGear(gear);
    //Serial.print("Gear ");
    //Serial.println(gear, HEX);
    hasRead++;
  }
  else if(cmd == CMD_LCD_FUEL)
  {
    fuel = sRead(hw);
    //printFuelGauge(fuel);
    //Serial.print("Fuel ");
    //Serial.println(fuel, HEX);
    hasRead++;
  }
  else if(cmd == CMD_LCD_VOLT)
  {
    voltageWhole = sRead(hw);
    voltageDecimal = sRead(hw);
    hasRead+=2;
  }
  else if(cmd == CMD_LCD_TIME)
  {
    if(displayMode == DM_MENU)
    {
      for(int i = 0; i < 6; i++)
      {
        sRead(hw);
      }
    }
    else
    {
      day = sRead(hw);
      month = sRead(hw);
      year = sRead(hw);
      hour = sRead(hw);
      minute = sRead(hw);
      second = sRead(hw);
    }
    hasRead+=6;
  }
  else if(cmd == CMD_LCD_COLD_ON)
  {
    coldIcon = true;
  }
  else if(cmd == CMD_LCD_COLD_OFF)
  {
    coldIcon = false;
  }
  else if(cmd == CMD_LCD_FUEL_ON)
  {
    fuelIcon = true;
  }
  else if(cmd == CMD_LCD_FUEL_OFF)
  {
    fuelIcon = false;
  }
  else if(cmd == CMD_LCD_BATT_ON)
  {
    batteryIcon = true;
  }
  else if(cmd == CMD_LCD_BATT_OFF)
  {
    batteryIcon = false;
  }
  else if(cmd == CMD_LCD_HEAT_ON)
  {
    heatIcon = true;
  }
  else if(cmd == CMD_LCD_HEAT_OFF)
  {
    heatIcon = false;
  }
  else if(cmd == CMD_LCD_REFRESH)
  {
    refreshNormal();
    sendAck();
  }
  else
  {
    //Serial1.print("UNK: ");
    //Serial1.println(cmd, HEX);
  }

  return hasRead;
}

void sendAck()
{
  if(dayC || monthC || yearC || hourC || minuteC)
  {
    sWrite(CMD_LCD_ACK);
    sWrite(0x07);
    sWrite(CMD_LCD_SETTIME);
    swrite(day);
    sWrite(month);
    sWrite(year);
    sWrite(hour);
    sWrite(minute);
    sWrite(0x00);
  }
  else
  {
    sWrite(CMD_LCD_ACK);
    sWrite(CMD_LCD_ACK);
  }
}

void refreshNormal()
{
  if(displayMode == DM_NORMAL)
  {
    printGear();
    printFuelGauge();
    reprintBottom();
    display.display();
  }
}

byte sRead(bool hw)
{
  if(hw) return Serial.read();
  else return Serial1.read();
}

void sWrite(byte data)
{
    Serial1.write(data);
    Serial1.read();
}

void printGear(){
  display.fillRect(0, 0, 40, 56, WHITE);
  display.setTextSize(8);
  display.setCursor(0, 0);
  if(gear == 0) display.println("N");
  else if(gear > 6) display.println("!");
  else display.println(gear);
  display.display();
}

void printFuelGauge()
{
  display.setTextSize(1);
  display.setCursor(42, 0);
  display.println("F");
  display.setCursor(42, 49);
  display.println("E");
  byte mapped = map(fuel, 0, 100, 0, 39);
  display.fillRect(42, 8, 5, 40, WHITE);
  display.fillRect(42, 8 + 39 - mapped, 5, mapped, BLACK);
  display.display();
  //na wskaźnik zostaje 8 od góry => 40px
}

void reprintBottom()
{
  if(batteryIcon || fuelIcon || heatIcon || coldIcon)
  {
    if(!battShown && !fuelShown && !heatShown && !coldShown)
    {
      clearBottom();
      iconTimer.refresh();
      //printIcons();
      iconTimer.resume();
    }
  }
  else
  {
    if(battShown || fuelShown || heatShown || coldShown)
    {
      iconTimer.pause();
      battShown = fuelShown = heatShown = coldShown = false;
    }

    clearBottom();
    display.setTextSize(1);
    display.setCursor(0, 61);
    printByteZ(hour);
    display.print(':');
    printByteZ(minute);
    display.print(':');
    printByteZ(second);
    display.setCursor(0, 72);
    printByteZ(voltageWhole);
    display.print('.');
    printByteZ(voltageDecimal);
    display.print(" V");
  }
}

void printIcons() {
  if(batteryIcon && (fuelShown || (!fuelShown && !battShown)))
  {
    clearRightIcon();
    showBatteryIcon();
    battShown = true;
    fuelShown = false;
  }
  else if(fuelIcon && (battShown || (!fuelShown && !battShown)))
  {
    clearRightIcon();
    showFuelIcon();
    battShown = false;
    fuelShown = true;
  }

  if(heatIcon && (coldShown || (!coldShown && !heatShown)))
  {
    clearLeftIcon();
    showHeatIcon();
    heatShown = true;
    coldShown = false;
  }
  else if(coldIcon && (heatShown || (!coldShown && !heatShown)))
  {
    clearLeftIcon();
    showFreezeIcon();
    heatShown = false;
    coldShown = true;
  }
}

void printByteZ(byte value)
{
  if(value < 10)
    display.print('0');

  display.print(value);
}

void printByteZZ(byte value)
{
  if(value < 100)
    display.print('0');
  if(value < 10)
    display.print('0');

  display.print(value);
}

void showFreezeIcon()
{
  display.drawBitmap(0, 60, snowflake_ico, 24, 24, 1);
}

void showFuelIcon()
{
  display.drawBitmap(24, 60, fuel_ico, 24, 24, 1);
}

void showBatteryIcon()
{
  display.drawBitmap(24, 60, battery_ico, 24, 24, 1);
}

void showHeatIcon() 
{
  display.drawBitmap(0, 60, overheat_ico, 24, 24, 1);
}

void clearRightIcon() 
{
  display.fillRect(24, 60, 24, 24, WHITE);
}

void clearLeftIcon() 
{
  display.fillRect(0, 60, 24, 24, WHITE);
}

void clearBottom()
{
  display.fillRect(0, 60, 48, 24, WHITE);
}

void setContrast(byte value)
{
  display.setContrast(value);
  display.display();
  contrast = value;
}

void checkButtons(void)
{
  int btn1 = analogRead(PB0);
  int btn2 = analogRead(PB1);
  if(btn1 > HIGH_TRESHOLD)
  {
    if(lastBtn1 == true)
    {
      btn1Cycles++;
      if(btn1Cycles > BTN_HOLD_CYCLES)
      {
        Btn1Hold();
        btn1Cycles = -100;
      }
    }
    lastBtn1 = true;
  }
  else if(btn1 < LOW_TRESHOLD)
  {
    lastBtn1 = false;
    if(btn1Cycles > 0)
    {
      Btn1Click();
    }
    btn1Cycles = 0;
  }

  if(btn2 > HIGH_TRESHOLD)
  {
    if(lastBtn2 == true)
    {
      btn2Cycles++;
      if(btn2Cycles > BTN_HOLD_CYCLES)
      {
        Btn2Hold();
        btn2Cycles = -100;
      }
    }
    lastBtn2 = true;
  }
  else if(btn2 < LOW_TRESHOLD)
  {
    lastBtn2 = false;
    if(btn2Cycles > 0)
    {
      Btn2Click();
    }
    btn2Cycles = 0;
  }
}

// --- LEFT BUTTON --
void Btn1Click()
{
  if(displayMode == DM_MENU)
  {
    goUp();
  }
}

void Btn1Hold() 
{
  if(displayMode == DM_NORMAL)
  {
    displayMode = DM_MENU;
    reprintMenu();
  }
  else if(displayMode == DM_MENU)
  {
    if(goBack())
    {
      displayMode = DM_NORMAL;
      refreshNormal();
    }
  }
}
// --- LEFT BUTTON --

// --- RIGHT BUTTON --
void Btn2Click()
{
  if(displayMode == DM_MENU)
  {
    goDown();
  }
}

void Btn2Hold() 
{
  if(displayMode == DM_NORMAL)
  {
    displayMode = DM_MENU;
    reprintMenu();
  }
  else if(displayMode == DM_MENU)
  {
    goIn();
  }
}
// --- RIGHT BUTTON --

// --- menu HELPERS --
void setDay(byte v)
{
  dayC = true;
  day = v;
}

void setMonth(byte v)
{
  monthC = true;
  month = v;
}
void setYear(byte v)
{
  yearC = true;
  year = v;
}
void setHour(byte v)
{
  hourC = true;
  hour = v;
}
void setMinute(byte v)
{
  minuteC = true;
  minute = v;
}
// --- menu HELPERS
