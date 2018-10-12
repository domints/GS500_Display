#ifndef __CMDS_LCD_H__
#define __CMDS_LCD_H__

#define CMD_LCD_FUEL 0x46 //'F', Fuel, 1 byte, fuel %, 0-100
#define CMD_LCD_GEAR 0x47 //'G', Gear, 1 byte, gear 0-6
#define CMD_LCD_TIME 0x54 //'T', Time get, 6 bytes
//day, month, year (after 2000), hour, minute, second
#define CMD_LCD_VOLT 0x56 //'V', Voltage, 2byte, whole part, decimal part

#define CMD_LCD_COLD_ON 0x43 //'C', show cold icon
#define CMD_LCD_COLD_OFF 0x63 //'c', hide cold icon
#define CMD_LCD_HEAT_ON 0x48 //'H', show heat icon
#define CMD_LCD_HEAT_OFF 0x68 //'h', hide heat icon
#define CMD_LCD_FUEL_ON 0x46 //'E', show empty fuel icon
#define CMD_LCD_FUEL_OFF 0x65 //'e', hide empty fuel icon
#define CMD_LCD_BATT_ON 0x42 //'B' show battery icon
#define CMD_LCD_BATT_OFF 0x62 //'b' hide battery icon

#define CMD_LCD_REFRESH 0x52 //'R' refresh LCD data, send ACK or response

#define CMD_LCD_ACK 0x00 //NULL
#define CMD_LCD_SETTIME 0x74 //'t', Set time, 6 bytes
//day, month, year (after 2000), hour, minute, second

#endif //__CMDS_LCD_H__
