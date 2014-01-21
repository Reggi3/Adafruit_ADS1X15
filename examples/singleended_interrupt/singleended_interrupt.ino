#include <Wire.h>

#include <Adafruit_ADS1015.h>

#include <RFduinoBLE.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

// RFDuino pin connected to ADS ALERT/RDY pin
#define GPIO_RDY 2

#define NUM_CHANNELS 2

#if NUM_CHANNELS == 1
const uint8_t channels[1] = {0};
#elif NUM_CHANNELS == 2
const uint8_t channels[2] = {0, 1};
#endif

static volatile uint16_t total_isr;
static volatile uint16_t cur_isr;
static volatile uint16_t cur_val;
static uint8_t channel_num;

int isr(uint32_t ulPin)
{
  total_isr++;
  cur_isr++;
  cur_val = ads.getLastConversionResults();
  channel_num = (channel_num + 1) % NUM_CHANNELS;
  ads.setContinuousMode(channels[channel_num]);
  return 0;
}

void setup(void)
{
  Serial.begin(115200);
  total_isr = cur_isr = 0;
  channel_num = 0;
  
  pinMode(GPIO_RDY, INPUT);
  RFduino_pinWakeCallback(GPIO_RDY, LOW, isr);
  ads.beginContinuousMode(0);

  RFduinoBLE.begin();

  //while(!Serial.available());
  //while(Serial.available()) {
  //  Serial.read();
  //}
}

#if 0
static void write_int_to_serial(uint16_t val)
{
    uint8_t c;
    c = val & 0xff;
    Serial.write(c);
    c = (val & 0xff00) >> 8;
    Serial.write(c);
}
#endif

static bool connected;

void RFduinoBLE_onConnect() {
  connected = true;
}

static void write_int_to_BLE(uint16_t val)
{
    RFduinoBLE.send((char *)&val, 2);
}

void loop(void)
{
  if (cur_isr > 0) {
    cur_isr = 0;
    //write_int_to_serial(cur_val);
    write_int_to_BLE(cur_val);
    Serial.println(cur_val);
  }
}
