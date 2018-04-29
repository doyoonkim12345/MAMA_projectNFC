#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <Ticker.h>
#include <string.h>
#include <Wire.h>
#include <SimpleKalmanFilter.h>

Ticker flipper;
     
// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D6;
const uint8_t sda = D7;

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;

     
const char* ssid = "iptime";  //  network SSID (name)
const char* pass = "88051234";       // network password

IPAddress remote_ip(13, 124, 34, 210); // serverIP

unsigned int localPort = 6974;      // local port to listen for UDP packets
     
String sensor; //buffer to hold incoming and outgoing packets
     
WiFiUDP udp;

int16_t AccelX, AccelY, AccelZ, GyroX, GyroY, GyroZ;

//칼만필터 객체 생성
SimpleKalmanFilter AxFilter(2, 2, 1);
SimpleKalmanFilter AyFilter(2, 2, 1);
SimpleKalmanFilter AzFilter(2, 2, 1);
SimpleKalmanFilter GxFilter(2, 2, 1);
SimpleKalmanFilter GyFilter(2, 2, 1);
SimpleKalmanFilter GzFilter(2, 2, 1);

void setup()
  {
      Serial.begin(115200);
      Wire.begin(sda, scl);
      Serial.print("wificonnecting");
      WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED)
      {
         delay(500);
         Serial.print(".");
      }
         Serial.print("connected");
         udp.begin(localPort);
         MPU6050_Init();
 }

int t = millis();

void loop()
  {
    udp.beginPacket(remote_ip, localPort);
    Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
    
    String sensor = WiFi.macAddress() + " 0"
      +String(millis()-t) + " "
      +String(AxFilter.updateEstimate((float)AccelX/AccelScaleFactor)) + " "
      +String(AyFilter.updateEstimate((float)AccelY/AccelScaleFactor)) + " "
      +String(AzFilter.updateEstimate((float)AccelZ/AccelScaleFactor)) + " "
      +String(GxFilter.updateEstimate((float)GyroX/GyroScaleFactor)) + " "
      +String(GyFilter.updateEstimate((float)GyroY/GyroScaleFactor)) + " "
      +String(GzFilter.updateEstimate((float)GyroZ/GyroScaleFactor));

    udp.write(Str2Chr(sensor));

    udp.endPacket();
    delay(10);
  }


void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// read all 14 register
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());

  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}

//configure MPU6050
void MPU6050_Init(){
 // delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}

char* Str2Chr(String cmd){
    if(cmd.length()!=0){
        char *p = const_cast<char*>(cmd.c_str());
        return p;
    }
}

