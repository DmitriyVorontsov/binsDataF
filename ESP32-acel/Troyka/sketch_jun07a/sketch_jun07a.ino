#include <TroykaIMU.h>
//#include "BluetoothSerial.h"
#define K 0.2

//BluetoothSerial SerialBT;

// Создаём объект для фильтра Madgwick
Madgwick filter;
// Создаём объект для работы с гироскопом
Gyroscope gyroscope;
// Создаём объект для работы с акселерометром
Accelerometer accelerometer;
 
// Переменные для данных с гироскопа и акселерометра и компаса
float gx, gy, gz, ax, ay, az;
float q0, q1, q2, q3;
float fil[3];
float qfil[4];
             
// Переменные для хранения самолётных углов ориентации
float yaw, pitch, roll;
 
// Переменная для хранения частоты выборок фильтра
float sampleRate = 100.0;
 
void setup() 
{
    // Открываем последовательный порт
    Serial.begin(115200);
    // Инициализируем гироскоп
    gyroscope.begin();
    // Инициализируем акселерометр
    accelerometer.begin();
    // Инициализируем фильтр
    filter.begin();

    filter.setFrequency(sampleRate);
    accelerometer.readAccelerationGXYZ(ax, ay, az);
    gyroscope.readRotationRadXYZ(gx, gy, gz);
    filter.update(gx, gy, gz, ax, ay, az);
    filter.readQuaternion(q0, q1, q2, q3);
    fil[0]=ax;
    fil[1]=ay;
    fil[2]=az;

    qfil[0]=q0;
    qfil[1]=q1;
    qfil[2]=q2;
    qfil[3]=q3;
    
    //SerialBT.begin("ESP32");
}
unsigned long filMillis = millis();
void loop() 
{
    // Запоминаем текущее время
    unsigned long startMillis = millis();

    if((startMillis - filMillis)>=1000)
    {
          fil[0]=ax;
          fil[1]=ay;
          fil[2]=az;

          qfil[0]=q0;
          qfil[1]=q1;
          qfil[2]=q2;
          qfil[3]=q3;
          filMillis=startMillis;
    }
 
    // Считываем данные с акселерометра в единицах G
    accelerometer.readAccelerationGXYZ(ax, ay, az);
    // Считываем данные с гироскопа в радианах в секунду
    gyroscope.readRotationRadXYZ(gx, gy, gz);

   ax=ax*(1-K)+fil[0]*K;
   ay=ay*(1-K)+fil[1]*K;
   az=az*(1-K)+fil[2]*K;

    
    // Устанавливаем частоту фильтра
    filter.setFrequency(sampleRate);
    // Обновляем входные данные в фильтр
    filter.update(gx, gy, gz, ax, ay, az);
    
    filter.readQuaternion(q0, q1, q2, q3);

    q0=q0*(1-K)+qfil[0]*K;
    q1=q1*(1-K)+qfil[1]*K;
    q2=q2*(1-K)+qfil[2]*K;
    q3=q3*(1-K)+qfil[3]*K;
   
            // Выводим кватернион в serial-порт
            //char str[43];
            //sprintf(str,"%c %5.2f %c %5.2f %c %5.2f %c %5.2f %c %5.2f %c %5.2f %c %5.2f %c", '+',q0,',',q1,',',q2,',',q3,',',)
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

/*
        int val = Serial.read();
        // Если пришёл символ 's'
        if (val == 's') 
        {
            filter.readQuaternion(q0, q1, q2, q3);
            // Выводим кватернион в serial-порт
            Serial.print(q0);
            Serial.print(",");
            Serial.print(q1);
            Serial.print(",");
            Serial.print(q2);
            Serial.print(",");
            Serial.println(q3);
        }

 */       
    //SerialBT.println("+")
    // Вычисляем затраченное время на обработку данных
    unsigned long deltaMillis = millis() - startMillis;
    // Вычисляем частоту обработки фильтра
   if(deltaMillis == 0 )deltaMillis=1;
   delay(55);
    sampleRate = 1000 / (deltaMillis+55);
}
