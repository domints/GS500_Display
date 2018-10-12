#include "menu.h"
#include "display.h"

byte selected = 0;
int level1 = -1;
int level2 = -1;

bool isEditor;
byte minEdit;
byte maxEdit;
byte* editedValuePtr;
byte editedValue;

void reprintMenu()
{
  display.clearDisplay();
  display.setTextSize(1);
  if(level1 == -1)
  {
    plevel1();
  }
  else if(level2 == -1)
  {
    plevel2();
  }

  display.display();
}

void goUp()
{
  if(isEditor)
  {
    editedValue++;
    return;
  }
  if(selected > 0)
  {
    selected--;
    reprintMenu();
  }
}

void goDown()
{
  if(isEditor)
  {
    editedValue--;
    return;
  }
  if(selected < getMaxSelection())
  {
    selected++;
    reprintMenu();
  }
}

void goIn()
{
  if(level1 == -1)
  {
      level1 = selected;
      selected = 0;
  }
  else if(level2 == -1)
  {
    level2 = selected;
    selected = 0;
  }

  reprintMenu();
}

bool goBack()
{
  if(level1 == -1)
    return true;

  if(level2 == -1)
    level1 = -1;

  reprintMenu();
  return false;
}

byte getMaxSelection()
{
  if(level1 == -1)
    return 0;

  if(level1 == 0 && level2 == -1)
    return 4;
}

void menuBounds(const char *header)
{
  display.setCursor(0, 0);
  display.setTextColor(WHITE, BLACK);
  display.print(header);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0, 8 * (selected + 1));
  display.print('>');
}

void menuEntry(byte pos, const char *text)
{
  display.setCursor(6, 8 * (pos + 1));
  display.print(text);
}

void menuEntry(byte pos, const char *text, byte param)
{
  display.setCursor(6, 8 * (pos + 1));
  display.print(text);
  display.print(param);
}

void plevel1()
{
  menuBounds("  MENU  ");
  menuEntry(0, "Time");
}

void plevel2()
{
  if(level1 == 0)
  {
    plevel2_s0();
  }
}

void plevel2_s0()
{
  menuBounds("  TIME  ");
  menuEntry(0, "Hr: ", hour);
  menuEntry(1, "Min:", minute);
  menuEntry(2, "Day:", day);
  menuEntry(3, "Mon:", month);
  menuEntry(4, "Yr: ", year);
}

void editor(byte* value, const char *title)
{
  
}

