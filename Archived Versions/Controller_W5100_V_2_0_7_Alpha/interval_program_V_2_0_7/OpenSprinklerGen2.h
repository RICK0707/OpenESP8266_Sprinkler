// Arduino library code for OpenSprinkler Generation 2

/* OpenSprinkler Class Definition
   Creative Commons Attribution-ShareAlike 3.0 license
   Dec 2013 @ Rayshobby.net
*/

#ifndef _OpenSprinkler_h
#define _OpenSprinkler_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// <MOD> ====== Added libraries for W5100, Freetronics LCD, DS1307 RTC, SD Card =====
/*
#include <avr/eeprom.h>
#include "../Wire/Wire.h"

#include "tinyFAT.h"
#include "LiquidCrystal.h"
#include "Time.h"
#include "DS1307RTC.h"
#include "EtherCard.h"
#include "defines.h"
*/
#include <avr/eeprom.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <LiquidCrystal.h>
#include <MemoryFree.h>
#include <tinyFAT.h>
#include <avr/pgmspace.h>
#include "EtherCard_W5100.h"
#include "defines.h"
// </MOD> ===== Added libraries for W5100, Freetronics LCD, DS1307 RTC, SD Card =====


// Option Data Structure
struct OptionStruct{
  byte value; // each option is byte
  byte max;   // maximum value
  char* str;  // name string
  char* json_str; // json name
  byte flag;  // flag
};

struct StatusBits {
  byte enabled:1;           // operation enable (when set, controller operation is enabled)
  byte rain_delayed:1;      // rain delay bit (when set, rain delay is applied)
  byte rain_sensed:1;       // rain sensor bit (when set, it indicates that rain is detected)
  byte program_busy:1;      // HIGH means a program is being executed currently
  byte manual_mode:1;       // HIGH means the controller is in manual mode
  byte has_rtc:1;           // HIGH means the controller has a DS1307 RTC
  byte has_sd:1;            // HIGH means a microSD card is detected
  byte dummy:1;
  byte display_board:4;     // the board that is being displayed onto the lcd
  byte network_fails:4;     // number of network fails
}; 

struct StationDataBits {
  byte n_enabled:1;         // 0: enabled; 1: disabled
  byte n_actmaster:1;       // 0: this station activates master (if master station is defined); 1: this station does not activate master
  byte n_raindelay:1;       // 0: rain delay applies to this station; 1: rain delay is ignored for this station
  byte group_index:2;       // group index. 0: global serial group; 1: global concurrent group; 2-3: custom groups
  byte type:3;              // type of sprinkler station: 0-default, 1-radio frequency station, 2-7: custom types
};

class OpenSprinkler {
public:
  
  // ====== Data Members ======
  static LiquidCrystal lcd;
  static StatusBits status;
  static byte nboards, nstations;
  
  static OptionStruct options[];  // option values, max, name, and flag
    
  static char* days_str[];		// 3-letter name of each weekday
  static byte station_bits[]; // station activation bits. each byte corresponds to a board (8 stations)
                              // first byte-> master controller, second byte-> ext. board 1, and so on
  static byte masop_bits[];   // station master operation bits. each byte corresponds to a board (8 stations)
  static byte ignrain_bits[]; // ignore rain bits. each byte corresponds to a board (8 stations)
  static unsigned long raindelay_stop_time;   // time (in seconds) when raindelay is stopped
  static unsigned long button_lasttime;


  //<MOD> ====== Digital Outputs =====// 
  static int station_pins[];
  //</MOD> ===== Digital Outputs =====// 

  // ====== Member Functions ======
  // -- Setup --
  static void reboot();   // reboot the microcontroller
  static void begin();    // initialization, must call this function before calling other functions
  static byte start_network(byte mymac[], int http_port);  // initialize network with the given mac and port
  static void self_test(unsigned long ms);  // self-test function
  static void get_station_name(byte sid, char buf[]); // get station name
  static void set_station_name(byte sid, char buf[]); // set station name
  static void masop_load();  // load station master operation bits
  static void masop_save();  // save station master operation bits
  static void ignrain_load();  // load ignore rain bits
  static void ignrain_save();  // save ignore rain bits  
  // -- Controller status
  static void constatus_load();
  static void constatus_save();
  // -- Options --
  static void options_setup();
  static void options_load();
  static void options_save();

  // -- Operation --
  static void enable();     // enable controller operation
  static void disable();    // disable controller operation, all stations will be closed immediately
  static void raindelay_start();  // start raindelay
  static void raindelay_stop(); // stop rain delay
  static void rainsensor_status(); // update rainsensor stateus
  static byte weekday_today();  // returns index of today's weekday (Monday is 0) 
  // -- Station schedules --
  // Call functions below to set station bits
  // Then call apply_station_bits() to activate/deactivate valves
  static void set_station_bit(byte sid, byte value); // set station bit of one station (sid->station index, value->0/1)
  static void clear_all_station_bits(); // clear all station bits
  static void apply_all_station_bits(); // apply all station bits (activate/deactive values)

  // -- String functions --
  //static void password_set(char *pw);     // save password to eeprom
  static byte password_verify(char *pw);  // verify password
  static void eeprom_string_set(int start_addr, char* buf);
  static void eeprom_string_get(int start_addr, char* buf);
    
  // -- LCD functions --
  static void lcd_print_pgm(PGM_P PROGMEM str);           // print a program memory string
  static void lcd_print_line_clear_pgm(PGM_P PROGMEM str, byte line);
  static void lcd_print_time(byte line);                  // print current time
  static void lcd_print_ip(const byte *ip, int http_port);// print ip and port number
  static void lcd_print_station(byte line, char c);       // print station bits of the board selected by display_board

  // <MOD> ====== Added for debugging =====
  static void lcd_print_memory(byte line);                // print current free memory and an animated character to show activity
  // </MOD> ===== Added for debugging =====
 
  // -- Button and UI functions --
  static byte button_read(byte waitmode); // Read button value. options for 'waitmodes' are:
                                          // BUTTON_WAIT_NONE, BUTTON_WAIT_RELEASE, BUTTON_WAIT_HOLD
                                          // return values are 'OR'ed with flags
                                          // check defines.h for details

  // -- UI functions --
  static void ui_set_options(int oid);    // ui for setting options (oid-> starting option index)

private:
  static void lcd_print_option(int i);  // print an option to the lcd
  static void lcd_print_2digit(int v);  // print a integer in 2 digits
  static byte button_read_busy(byte pin_butt, byte waitmode, byte butt, byte is_holding);

  // <MOD> ====== Added for Freetronics LCD Shield =====
  static byte button_sample();          // new function to sample analog button input
  // </MOD> ===== Added for Freetronics LCD Shield =====
};

#endif
