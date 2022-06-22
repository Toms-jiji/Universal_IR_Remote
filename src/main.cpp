/* IR Transmitter code
 * Date: 22-06-2022
 * Author: Toms Jiji Varghese
 */

#include <Preferences.h>
#include <IRsend.h>
#define Serial_prints

IRsend irsend(13);
Preferences preferences;
uint8_t data[]={0x88,0xC0,0x05,0x10};
int data_bits=28;
//uint8_t data[]={0x11,0x22,0x33,0x44};

void preferences_write(const char* Namespace, const char* key, const char* key_bits, uint8_t *data_to_be_written, int number_of_message_bits);
void preferences_read(const char* Namespace, const char* key, const char* key_bits, char* buffer,size_t data_length, int* number_of_message_bits);
void Ac_Send_Code(const char* Namespace, const char* key, const char* key_bits);
size_t get_data_length(const char* Namespace, const char* key);

void setup() {
  Serial.begin(921600);
  Serial.println();
  pinMode(13, OUTPUT);
  irsend.begin();
  preferences_write("LG","LG_on","LG_on_bits",data,data_bits);

  #ifdef Serial_prints
    Serial.println("Done writing");
  #endif
}

void loop() {
Ac_Send_Code("LG","LG_on","LG_on_bits");
delay(1000);
}

void preferences_write(const char* Namespace, const char* key, const char* key_bits, uint8_t *data_to_be_written, int number_of_message_bits)
{
  preferences.begin(Namespace, false);
  preferences.putBytes(key, data_to_be_written,sizeof(data_to_be_written)); 
  preferences.putInt(key_bits, number_of_message_bits);
  preferences.end();
}

void preferences_read(const char* Namespace, const char* key, const char* key_bits, char* buffer,size_t data_length, int* number_of_message_bits)
{
  preferences.begin(Namespace, true);
  preferences.getBytes(key,buffer,data_length);
  *number_of_message_bits=preferences.getInt(key_bits);
  preferences.end();
}

void Ac_Send_Code(const char* Namespace, const char* key, const char* key_bits) {
  uint64_t TX_code=0x00;
  int number_of_message_bits=0;
  size_t buffer_length = get_data_length(Namespace, key);
  char buffer[buffer_length];
  preferences_read(Namespace, key, key_bits, buffer, buffer_length, &number_of_message_bits);

  for(int i=buffer_length;i>0;i--)
    TX_code = TX_code | (buffer[buffer_length - i] << (8*(i-1)));
  TX_code=(TX_code>>(buffer_length*8 - number_of_message_bits)) & ((1<<number_of_message_bits)-1);

  #ifdef Serial_prints
    Serial.print("\t code to send : ");
    Serial.print(TX_code, BIN);
    Serial.print(" : ");
    Serial.println(TX_code, HEX);
  #endif

  irsend.sendLG(TX_code, number_of_message_bits);
  
}

size_t get_data_length(const char* Namespace, const char* key){
  preferences.begin("LG", true);
  size_t data_length= preferences.getBytesLength("LG_on");
  preferences.end();
  return data_length;
}