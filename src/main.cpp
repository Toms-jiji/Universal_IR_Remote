/* 
* Date: 29-06-2022
* Author: Toms Jiji Varghese
* Email: toms.jiji.varghese@gmail.com
*/
#include <Arduino.h>
#include <Preferences.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRutils.h>

//  #define Manufacturing

Preferences preferences;
const uint16_t kIrLed = 13;  // The ESP GPIO pin to use that controls the IR LED.
IRac ac(kIrLed);             // Create a A/C object using GPIO to sending messages with.
int protocol_index;
decode_type_t protocol_AC;

bool AC_decode();
void preferences_write(const char* Namespace, const char* protocol_number, int number_of_message_bits);
void preferences_read(const char* Namespace, const char* protocol_key, int* protocol_number);
void send_ir_command();
bool init_universal_Remote(bool resetup);

struct commands
{
  int mode = 1;
  int temp = 25;
  int fanspeed = 3;
  int vertical_swing = -1;
  int horizontal_swing = -1;
  bool light= true;
  bool beep = true;  // Turn off any beep from the A/C if we can.
  bool econo = false;  // Turn off any economy modes if we can.
  bool filter = false;  // Turn off any Ion/Mold/Health filters if we can.
  bool turbo = false;  // Don't use any turbo/powerful/etc modes.
  bool quiet = false;  // Don't use any quiet/silent/etc modes.
  int sleep = -1;  // Don't set any sleep time or modes.
  bool clean = false;  // Turn off any Cleaning options if we can.
  int clock = -1;  // Don't set any current time if we can avoid it.
  bool power = false;  // Initially start with the unit off.
}AC_commands;

void setup() {
  Serial.begin(115200);
  delay(200);
  init_universal_Remote(false);
}


void loop() 
{
  Serial.flush();
  while(1)
  {
    AC_commands.power = true;
    AC_commands.temp = 27;
    Serial.println("Send a message to turn ON the A/C unit.");
    send_ir_command();
    delay(1000);

    AC_commands.power = false;
    AC_commands.temp = 20;
    Serial.println("Send a message to turn OFF the A/C unit.");
    send_ir_command();
    delay(1000);
  }
}


bool init_universal_Remote(bool resetup)
{
  #ifdef Manufacturing
    preferences_write("protocols", "protocol_number", 0);
    while(1)
      Serial.println("done\n");
  #endif

  #ifndef Manufacturing
    preferences_read("protocols", "protocol_number", &protocol_index);
    send_ir_command();
    if(protocol_index == 0 || resetup == true)
    {
      Serial.println("Protocol index =0");
      if(AC_decode())
      {
        Serial.println("Decoding Successful");
        preferences_read("protocols", "protocol_number", &protocol_index);
        protocol_AC = (decode_type_t)protocol_index;
        Serial.println("Protocol " + String(protocol_AC) + " / " + typeToString(protocol_AC) + " is supported.");
        ac.next.protocol = protocol_AC;
        return 0;
      }
      else
      {
        Serial.println("Decoding Unsuccessful");
        preferences_read("protocols", "protocol_number", &protocol_index);
        protocol_AC = (decode_type_t)protocol_index;
        Serial.println("Protocol " + String(protocol_AC) + " / " + typeToString(protocol_AC) + " is supported.");
        ac.next.protocol = protocol_AC;
        return 1;
      }
    }
  #endif
  return 0;
}

void send_ir_command()
{
  ac.next.protocol = protocol_AC;
  ac.next.model = 1;  // Some A/Cs have different models. Try just the first.
  switch (AC_commands.mode)
  {
    case -1: ac.next.mode = stdAc::opmode_t::kOff; 
    case  0: ac.next.mode = stdAc::opmode_t::kAuto;
    case  1: ac.next.mode = stdAc::opmode_t::kCool;
    case  2: ac.next.mode = stdAc::opmode_t::kHeat;
    case  3: ac.next.mode = stdAc::opmode_t::kDry; 
    case  4: ac.next.mode = stdAc::opmode_t::kFan; 
    default: ac.next.mode = stdAc::opmode_t::kCool;
  }
  ac.next.celsius = true;
  ac.next.degrees = AC_commands.temp;

  switch (AC_commands.fanspeed)
  {
    case  0: ac.next.fanspeed = stdAc::fanspeed_t::kAuto;  
    case  1: ac.next.fanspeed = stdAc::fanspeed_t::kMin;  
    case  2: ac.next.fanspeed = stdAc::fanspeed_t::kLow;  
    case  3: ac.next.fanspeed = stdAc::fanspeed_t::kMedium;   
    case  4: ac.next.fanspeed = stdAc::fanspeed_t::kHigh;   
    case  5: ac.next.fanspeed = stdAc::fanspeed_t::kMax;   
    default: ac.next.fanspeed = stdAc::fanspeed_t::kAuto;  
  }

  switch (AC_commands.vertical_swing)
  {
    case -1: ac.next.swingv = stdAc::swingv_t::kOff;   
    case  0: ac.next.swingv = stdAc::swingv_t::kAuto;  
    case  1: ac.next.swingv = stdAc::swingv_t::kHighest;  
    case  2: ac.next.swingv = stdAc::swingv_t::kHigh;  
    case  3: ac.next.swingv = stdAc::swingv_t::kMiddle;      
    case  4: ac.next.swingv = stdAc::swingv_t::kLow;    
    case  5: ac.next.swingv = stdAc::swingv_t::kLowest;   
    default: ac.next.swingv = stdAc::swingv_t::kHighest;  
  }

  switch (AC_commands.horizontal_swing)
  {
    case -1: ac.next.swingh = stdAc::swingh_t::kOff;   
    case  0: ac.next.swingh = stdAc::swingh_t::kAuto;  
    case  1: ac.next.swingh = stdAc::swingh_t::kLeftMax;  
    case  2: ac.next.swingh = stdAc::swingh_t::kLeft;  
    case  3: ac.next.swingh = stdAc::swingh_t::kMiddle;      
    case  4: ac.next.swingh = stdAc::swingh_t::kRight;    
    case  5: ac.next.swingh = stdAc::swingh_t::kRightMax;   
    case  6: ac.next.swingh = stdAc::swingh_t::kWide;   
    default: ac.next.swingh = stdAc::swingh_t::kOff;  
  }
  ac.next.light = AC_commands.light;  
  ac.next.beep = AC_commands.beep;
  ac.next.econo = AC_commands.econo;
  ac.next.filter = AC_commands.filter;
  ac.next.turbo = AC_commands.turbo;
  ac.next.quiet = AC_commands.quiet;
  ac.next.sleep = AC_commands.sleep;
  ac.next.clean = AC_commands.clean;
  ac.next.clock = AC_commands.clock;
  ac.next.power = AC_commands.power;

  ac.sendAc();
}

bool AC_decode()
{
  send_ir_command();
  Serial.println("Please input any character on serial monitor if the ac responds\n Setting up the AC configurations...");
    // For every protocol the library has ...

  for (int i = 1; i < kLastDecodeType; i++) {
    decode_type_t protocol = (decode_type_t)i;
    if (ac.isProtocolSupported(protocol)) {
      Serial.println("Protocol " + String(protocol) + " / " +
                     typeToString(protocol) + " is supported.");
      ac.next.protocol = protocol;
      ac.next.power = true;
      ac.sendAc();
      delay(5000);
    }
    if(Serial.available())
    {
      int temp;
      protocol_AC = (decode_type_t)i;
      Serial.println("Protocol: " + String(protocol));
      preferences_read("protocols", "protocol_number", &temp);
      while(temp!= i)
      {
        preferences_write("protocols", "protocol_number", i);
        Serial.println("rewriting");
        preferences_read("protocols", "protocol_number", &temp);
      }
      return true;
    }
  }
  return false;
}

void preferences_write(const char* Namespace, const char* protocol_key, int number_of_message_bits)
{
  preferences.begin(Namespace, false);
  preferences.putInt(protocol_key, number_of_message_bits);
  preferences.end();
}

void preferences_read(const char* Namespace, const char* protocol_key, int* protocol_number)
{
  preferences.begin(Namespace, true);
  *protocol_number=preferences.getInt(protocol_key);
  preferences.end();
}