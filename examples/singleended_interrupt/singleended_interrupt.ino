#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

// RFDuino pin connected to ADS ALERT/RDY pin
#define GPIO_RDY 2

static volatile uint16_t total_isr;
static volatile uint16_t cur_isr;
static volatile uint16_t cur_val;

int isr(uint32_t ulPin)
{
  total_isr++;
  cur_isr++;
  cur_val = ads.getLastConversionResults();
  return 0;
}

void setup(void) 
{
  Serial.begin(115200);
  total_isr = cur_isr = 0;
  
  pinMode(GPIO_RDY, INPUT);
  RFduino_pinWakeCallback(GPIO_RDY, LOW, isr);
  ads.beginContinuous(0);
  while (1) {
    Serial.write('b');
  }
  while(!Serial.available());
  while(Serial.available())
    Serial.read();
}

static void write_int(uint16_t val)
{
    uint8_t c;
    c = (val & 0xff00) >> 16;
    Serial.write(c);
    c = val & 0xff;
    Serial.write(c);
}

void loop(void)
{
  if (cur_isr > 0) {
    cur_isr = 0;
    write_int(cur_val);
  }
}
