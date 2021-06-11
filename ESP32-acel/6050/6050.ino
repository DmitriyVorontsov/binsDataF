#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"
#include "BluetoothSerial.h"

#define TO_DEG 57.29577951308232087679815481410517033f
#define FK 0.1 // коэффициент комплементарного фильтра
#define statKoeff 0.02

BluetoothSerial SerialBT;

float q0, q1, q2, q3;
int16_t ax, ay, az;
int16_t gx, gy, gz;
float axf, ayf, azf;
float gxf, gyf, gzf;
float angleX = 0;
float angleY = 0;
float angleZ = 0;
float xVel=0, yVel=0, zVel=0;
float xPos=0, yPos=0, zPos=0;


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

float samplePeriod = 0.0;

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
  mpu.CalibrateAccel(6);
  mpu.CalibrateGyro(6);
}

int count = 0;
unsigned long int st = millis();
void loop() 
{
  long int start = millis();
  getAngles();
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

axf = ax / 16384.0;
ayf = ay / 16384.0;
azf = az / 16384.0;

    float g0 = 2 * (q.x * q.z - q.w * q.y);
    float g1 = 2 * (q.w * q.x + q.y * q.z);
    float g2 = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;
   axf = axf - g0;
   ayf = ayf - g1;
   azf = azf - g2;


   float accMag;
   accMag = sqrt(axf*axf + ayf*ayf + azf*azf);

   axf *= 9.8;
   ayf *= 9.8;
   azf *= 9.8;

   if(accMag < statKoeff)
   {
      xVel=0;
      yVel=0;
      zVel=0;
   }
   else
   {
      xVel = xVel + axf*samplePeriod;
      yVel = yVel + ayf*samplePeriod;
      zVel = zVel + azf*samplePeriod;

      xPos = xPos + xVel*samplePeriod;
      yPos = yPos + yVel*samplePeriod;
      zPos = zPos + zVel*samplePeriod;
   }
      char Mesadge[100];
      //String str=String(xPos)+','+String(yPos)+','+String(zPos)+','+String(millis() - start); 
      int delta = millis() - start;
      int kol = sprintf(Mesadge,"%f %c %f %c %f %c", xPos, ',', yPos, ',', zPos,'\n');
      uint8_t Mesadge2[100];
      for(int i=0; i<kol; i++)Mesadge2[i] = Mesadge[i];
      /*
      SerialBT.print(xPos);SerialBT.print(",");
      SerialBT.print(yPos);SerialBT.print(",");
      SerialBT.println(zPos);
      */
      SerialBT.write(Mesadge2, kol);
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
      //SerialBT.println(str);
      //delay(500);
      samplePeriod = (millis() - start) / 1000.0;
      

/*  Serial.print(angleX); Serial.print(",  ");
  Serial.print(angleY); Serial.print(",  ");
  Serial.println(angleZ);*/
 /* 
  Serial.print("+");
  Serial.print(q.w); Serial.print(",");Serial.print(q.x); Serial.print(",");
  Serial.print(q.y); Serial.print(",");Serial.print(q.z);Serial.print(",");
  Serial.print(axf);Serial.print(",");Serial.print(ayf);Serial.print(",");Serial.print(azf);
  Serial.println("+");
*/
  
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
