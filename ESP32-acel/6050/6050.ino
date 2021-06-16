#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"
#include "BluetoothSerial.h"

#define FK 0.1 // коэффициент комплементарного фильтра
#define statKoeff 0.04

BluetoothSerial SerialBT;

float q0, q1, q2, q3;
int16_t ax, ay, az;
int16_t gx, gy, gz;
float axf, ayf, azf;
float gxf, gyf, gzf;
float angleX = 0;
float angleY = 0;
float angleZ = 0;
float xVel=0, yVel=0, zVel=0;           //Скорости
float xVelOld=0, yVelOld=0, zVelOld=0;  //Скорости в предыдущий момент времени
float xPos=0, yPos=0, zPos=0;           //Координаты


const float toDeg = 180.0 / M_PI;
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

MPU6050 mpu;

int samplePeriod = 0;

void initDMP(); 
void getAngles();
void setup() 
{
  Wire.begin();
  Wire.setClock(400000);
  Serial.begin(115200);
  while(!Serial) {}
  if (!SerialBT.begin("ESP32")) Serial.println("An error occurred initializing Bluetooth");
  else Serial.println("Bluetooth initialized");
  mpu.initialize();
  Serial.println(mpu.testConnection() ? "MPU6050 OK" : "MPU6050 FAIL");
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  initDMP();
  delay(50);
  Serial.println("Calibration...");
  mpu.CalibrateAccel(10);
  mpu.CalibrateGyro(10);
}

int count = 0;
unsigned long int st = millis();
void loop() 
{
  long int start = millis();
  getAngles();
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

//переводим в g
axf = ax / 16384.0f;
ayf = ay / 16384.0f;
azf = az / 16384.0f;

gxf=gx/32768.0f * 250;
gyf=gy/32768.0f * 250;
gzf=gz/32768.0f * 250;

//Удаляем гравитацию
axf-=gravity.x;
ayf-=gravity.y;
azf-=gravity.z;

//Магнитуда
   float accMag = sqrt(axf*axf + ayf*ayf + azf*azf);
//g -> м/сс
/*
   axf *= 9.8;
   ayf *= 9.8;
   azf *= 9.8;
 */  
 /*
   if(abs(axf)<0.1)axf=0;
   if(abs(ayf)<0.1)ayf=0;
   if(abs(azf)<0.1)azf=0;
   */
/*
   if(accMag < statKoeff)
   {
      xVel=0;
      yVel=0;
      zVel=0;

      xVelOld=0;
      yVelOld=0;
      zVelOld=0;

   }
   else
   {   
      xVel = xVel + axf*samplePeriod/1000.0;
      yVel = yVel + ayf*samplePeriod/1000.0;
      zVel = zVel + azf*samplePeriod/1000.0;

      xPos = xPos + xVel*samplePeriod/1000.0;
      yPos = yPos + yVel*samplePeriod/1000.0;
      zPos = zPos + zVel*samplePeriod/1000.0;
   }
   */
     // char Mesadge[100];
      String str='+'+String(q.w)+','+String(q.x)+','+String(q.y)+','+String(q.z)+','+String(gxf)+','+String(gyf)+','+String(gzf)+'+';
      //int delta = millis() - start;
     // int kol = sprintf(Mesadge,"%f %c %f %c %f %c %d %c", axf, ',', ayf, ',', azf,',',samplePeriod,'\n');
      
      //Копируем char в uint8_t т.к. это хочет write
      //uint8_t Mesadge2[100];
     // for(int i=0; i<kol; i++)Mesadge2[i] = Mesadge[i];
      Serial.println(str);
      SerialBT.println(str);
      //SerialBT.write(Mesadge2, kol);

   //Отправка нескольких измерений пакетом
      /*
      if(count = 4)
      {
        SerialBT.write(Mesadge2, kol);
        SerialBT.write(Mesadge2, kol);
        SerialBT.write(Mesadge2, kol);
        SerialBT.write(Mesadge2, kol);
        SerialBT.write(Mesadge2, kol);
        SerialBT.println(millis()-st);
        count=0;
      }
      else count++;
      */
 
      samplePeriod = (millis() - start);
      if(samplePeriod<10) delay(10-samplePeriod);
}



void initDMP() 
{
  devStatus = mpu.dmpInitialize();
  mpu.setDMPEnabled(true);
  mpuIntStatus = mpu.getIntStatus();
  packetSize = mpu.dmpGetFIFOPacketSize();
}
void getAngles() 
{
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) 
  {
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    angleX = ypr[2] * toDeg;
    angleY = ypr[1] * toDeg;
    angleZ = ypr[0] * toDeg;
  }
}
