#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN            6
#define NUMPIXELS      12
#define sensorPin      0
#define BRIGHTNESS 50

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);
SoftwareSerial mySerial(2, 3);

String temp = "";
int brightnesscount = 0;
int current = 0;
bool writeon = false;

void setup()
{

  Serial.begin(9600);   //시리얼모니터
  mySerial.begin(9600); //블루투스 시리얼

  pixels.setBrightness(BRIGHTNESS);
  pixels.begin();
  pixels.show(); // Initialize all pixels to 'off'

}
void loop()
{

  String temp = "";
  char cmd = 0;
  int cmdint = 0;
  while (mySerial.available()) {

    cmd = (char)mySerial.read();
    if (isDigit(cmd)) {
      delay(10);
      temp += cmd;
    }

  }
  cmdint = temp.toInt();
if(cmdint > 128 || cmdint < 0){
  return;
}
  if (!temp.equals("")) {
    //delay(10);
    Serial.println(cmdint);

    if (cmdint == 1) {
      // 불켜짐
      writeon = true;
    }

    if (writeon) {
      if (cmdint == 0) {
        // 불꺼짐
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Moderately bright green color.
          pixels.show(); // This sends the updated pixel color to the hardware.
        }
        pixels.clear();
        writeon = false;
      } else if (cmdint == 4) {
        current = millis();
        AlarmMode();
      } else {
        LedLighter(cmdint, cmdint);
        //시리얼 모니터 내용을 블루투스 측에
        delay(50);
      }
    }
  }
}

struct UColor
{
  int r, g, b;
};
struct UColor takeColor(int amp)
{
  struct UColor color;

  if ( amp < 10 )
  {
    color = {0, 0, 0};
  }
  else if ( amp < 20 )
  {
    color = {128, 0, 120};
  }
  else if ( amp < 30 )
  {
    color = {8, 11, 84};
  }
  else if ( amp < 40 )
  {
    color = {0, 0, 255};
  }
  else if ( amp < 50 )
  {
    color = {135, 206, 236};
  }
  else if ( amp < 60 )
  {
    color = {0, 255, 0};
  }
  else if ( amp < 70 )
  {
    color = {255, 255, 0};
  }
  else if ( amp < 80 )
  {
    color = {255, 187, 0};
  }
  else if ( amp < 90 )
  {
    color = {255, 0, 0};
  }
  else
  {
    color = {255, 255, 255};
  }

  return color;
}

void LedLighter(int pixelnum, int amp) {
  UColor color = takeColor(amp);
  pixelnum = ((float)pixelnum / 30) * 12;

  for (int i = 0; i < pixelnum; i++) {
    pixels.setPixelColor(i, pixels.Color(color.r, color.g, color.b)); // Moderately bright green color.
    //Serial.print(i);
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(20);

  }
  for (int i = pixelnum; i != 0; i--) {
    pixels.setPixelColor(i - 1, pixels.Color(0, 0, 0)); // Moderately bright green color.
    //Serial.print(i);
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(20);
  }
  pixels.clear();
}

void AlarmMode() {
  if (millis() - current > 6000 * 60) {
    brightnesscount = 0;
  }
  brightnesscount += 1;
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255)); // Moderately bright green color.
    pixels.setBrightness(brightnesscount * BRIGHTNESS / 5);
    //Serial.print(i);
    pixels.show(); // This sends the updated pixel color to the hardware.

  }
  pixels.setBrightness(BRIGHTNESS);
}

void ASMRMode() {

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 0)); // Moderately bright green color.
    //Serial.print(i);
    pixels.show(); // This sends the updated pixel color to the hardware.

  }

}

