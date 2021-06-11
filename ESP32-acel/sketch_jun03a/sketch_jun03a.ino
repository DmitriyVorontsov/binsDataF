#include "MPU9250.h"
#include "MadgwickAHRS.h"

#define TO_DEG 57.29577951308232087679815481410517033f
#define T_OUT 20 // каждый 20 миллисекунд будем проводить вычисления 
#define P_OUT 50 // каждый 50 миллисекунд будем выводить данные
#define FK 0.1 // коэффициент комплементарного фильтра

float q0, q1, q2, q3;
float ax, ay, az;
float gx, gy, gz;
float idol[4];
// an MPU9250 object with the MPU-9250 sensor on I2C bus 0 with address 0x68
MPU9250 IMU(Wire,0x68);

// Создаём объект для фильтра Madgwick
Madgwick filter;

// Переменная для хранения частоты выборок фильтра
float sampleRate = 100.00;
int status;
void setup() 
{
  // serial to display data
  Serial.begin(115200);
  while(!Serial) {}
  // start communication with IMU 
  status = IMU.begin();
  if (status < 0) 
  {
    Serial.println("IMU initialization unsuccessful");
  }
    // Инициализируем фильтр
    filter.begin();
    IMU.calibrateAccel();
    IMU.calibrateGyro();


//заполняем idol
  IMU.readSensor();
  ax=IMU.getAccelX_mss()/9.8; // Переводим из м/c^2 в g 
  ay=IMU.getAccelY_mss()/9.8;
  az=IMU.getAccelZ_mss()/9.8;
  gx=IMU.getGyroX_rads(); // рад/с
  gy=IMU.getGyroY_rads();
  gz=IMU.getGyroZ_rads();
   filter.setFrequency(sampleRate);
   filter.update(gx, gy, gz, ax, ay, az);
   filter.readQuaternion(q0, q1, q2, q3);
   idol[0]=q0;
   idol[1]=q1;
   idol[2]=q2;
   idol[3]=q3;
}


void OpredYglov()
{

  // Запоминаем текущее время
  unsigned long startMillis = millis();
  //Cчитываем данные с акселерометра
  IMU.readSensor();
  ax=IMU.getAccelX_mss()/9.8; // Переводим из м/c^2 в g 
  ay=IMU.getAccelY_mss()/9.8;
  az=IMU.getAccelZ_mss()/9.8;
  gx=IMU.getGyroX_rads(); // рад/с
  gy=IMU.getGyroY_rads();
  gz=IMU.getGyroZ_rads();

  filter.setFrequency(sampleRate);
  // Обновляем входные данные в фильтр
  filter.update(gx, gy, gz, ax, ay, az);

int val = Serial.read();
        // Если пришёл символ 's'
        if (val == 's')
        {

            filter.readQuaternion(q0, q1, q2, q3);

            // Выводим кватернион в serial-порт
            //q0=q0*(1-FK)+idol[0]*FK;
          
            Serial.print(q0);
            Serial.print(",");
            Serial.print(q1);
            Serial.print(",");
            Serial.print(q2);
            Serial.print(",");
            Serial.println(q3);
          /*
            Serial.print("+");
            Serial.print(q0);
            Serial.print(",");
            Serial.print(q1);
            Serial.print(",");
            Serial.print(q2);
            Serial.print(",");
            Serial.print(q3);
            Serial.print(",");
            Serial.print(ax);
            Serial.print(",");
            Serial.print(ay);
            Serial.print(",");
            Serial.print(az);
            Serial.println("+");
            */
       }
     
    unsigned long deltaMillis = millis() - startMillis;
    if(deltaMillis == 0) deltaMillis = 1;
    //delay(55);
    sampleRate = 1000 / (deltaMillis);
}

void loop() 
{
  OpredYglov();
}
