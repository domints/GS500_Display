#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/Crasng24pt7.h>
#include "numbers.h"
#include "icons.h"
#include "logo.h"

bool transmissionStarted = false;

Adafruit_PCD8544 display = Adafruit_PCD8544(PA2, PA3, PA4);
#define NOMINAL_CONTRAST 50
byte contrast = 35;

void setup() {
  Serial1.begin(9600);
  Serial.begin(9600);
  randomSeed(analogRead(0));
  pinMode(PA1, OUTPUT);
  digitalWrite(PA1, HIGH);
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
  if(Serial1.available() >= 2)
  {
    workSerial(false);
  }
}

void workSerial(bool hw)
{
  if(!transmissionStarted)
  {
    transmissionStarted = true;
    display.clearDisplay();
    display.display();
  }
  byte cmd = sRead(hw);
  if(cmd == 'G')
  {
    byte gear = sRead(hw);
    printGear(gear);
    //Serial.print("Gear ");
    //Serial.println(gear, HEX);
  }
  else if(cmd == 'F')
  {
    byte fuel = sRead(hw);
    printFuelGauge(fuel);
    //Serial.print("Fuel ");
    //Serial.println(fuel, HEX);
  }
  else if(cmd == 'C')
  {
    showFreezeIcon();
    //Serial.println("Freeze");
  }
  else if(cmd == 'c')
  {
    clearFreezeIcon();
    //Serial.println("NFreeze");
  }
  else if(cmd == 'E')
  {
    showFuelIcon();
    //Serial.println("Empty");
  }
  else if(cmd == 'e')
  {
    clearFuelIcon();
    //Serial.println("NEmpty");
  }
  else
  {
    //Serial.print("UNK: ");
    //Serial.println(cmd, HEX);
  }
}

byte sRead(bool hw)
{
  if(hw) return Serial.read();
  else return readFromCan();
}

void sWrite(bool hw, byte data)
{
  if(hw) Serial.write(data);
  else writeToCan(data);
}

void printGear(byte gear){
  display.fillRect(0, 0, 40, 56, WHITE);
  display.setTextSize(8);
  display.setCursor(0, 0);
  if(gear == 0) display.println("N");
  else if(gear > 6) display.println("!");
  else display.println(gear);
  display.display();
}

void showFreezeIcon()
{
  display.drawBitmap(0, 60, snowflake_ico, 24, 24, 1);
  display.display();
}

void clearFreezeIcon() 
{
  display.fillRect(0, 60, 24, 24, WHITE);
  display.display();
}

void showFuelIcon()
{
  display.drawBitmap(24, 60, fuel_ico, 24, 24, 1);
  display.display();
}

void clearFuelIcon() 
{
  display.fillRect(24, 60, 24, 24, WHITE);
  display.display();
}

void printFuelGauge(byte percentFuel)
{
  display.setTextSize(1);
  display.setCursor(42, 0);
  display.println("F");
  display.setCursor(42, 49);
  display.println("E");
  byte mapped = map(percentFuel, 0, 100, 0, 40);
  display.fillRect(42, 8 + 40 - mapped, 5, mapped, BLACK);
  display.display();
  //na wskaźnik zostaje 8 od góry => 40px
}

void setContrast(byte value)
{
  display.setContrast(value);
  display.display();
  contrast = value;
}

void writeToCan(byte v)
{
  int r1 = 0;
  int r2 = 0;
  for(byte i = 0; i < 4; i++)
  {
    if((v >> i) & 0x01)
    {
      r2 = r2 | (2 << (i*2));
    }
    else
    {
      r2 = r2 | (1 << (i*2));
    }
  }
  for(byte i = 4; i < 8; i++)
  {
    if((v >> i) & 0x01)
    {
      r1 = r1 | (2 << ((i-4)*2));
    }
    else
    {
      r1 = r1 | (1 << ((i-4)*2));
    }
  }
  Serial1.write(r1);
  Serial1.write(r2);
}

byte readFromCan()
{
  while(Serial1.available() < 2);
  byte v1 = Serial1.read();
  byte v2 = Serial1.read();
    int r = 0;
    int index = 0;
    for(int i = 0; i < 8; i = i + 2)
    {
        if((v2 >> i) & 0x02)
        {
            r = r | (1 << index); 
        }
        
        index++;
    }
    for(int i = 0; i < 8; i = i + 2)
    {
        if((v1 >> i) & 0x02)
        {
            r = r | (1 << index); 
        }
        
        index++;
    }
    return r;
}

