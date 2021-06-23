#ifndef DATAWORK_H
#define DATAWORK_H

#include <QVector>

//структура кватерниона
struct Quaternion{
    float q0;
    float q1;
    float q2;
    float q3;
};
//структура данных
struct p_data{
    float x;
    float y;
    float z;
};

typedef float mtrx[3][3];

class datawork
{

public:
    datawork();
    ~datawork();
    void clearAll();

private:
    QString datastr;//входная строка
    p_data a_read;
    p_data g_read;
    //Quaternion q_read;
    void lineToData();
    void filt();
    void filtMatlab();

public:
    int count = 0;
    QVector <QString> allData;//все строки
    QVector <Quaternion> quaternions;//все кватернионы
    QVector <p_data> accelerations;//все ускорения
    QVector <p_data> velocities;//все скорости
    QVector <p_data> positions;//все позиции
    QVector <p_data> acc_nograv;//ускорения после удаления гравитации
    QVector <float> magnLp;//магнитуды ускорений
    QVector <int> statPeriods;//стационарные периоды

    float lpMag = 0;//хранение данный отфильтрованной магнитуды
    float statKoeff = 0.025;//коэффициэнт отсечения
    p_data i_vel{0.0,0.0,0.0};//переменная для интегрирования скоростей
    p_data i_pos{0.0,0.0,0.0};//переменная для интегрирования координат
    float samplePeriod = 0.01;
    void readStr(QString str);

    int filter_mode = 1;

    int countx=0, county=0, countz=0;   //Счетчики для фильтрации обратного импульса
    unsigned int zeroPeriod=8;          //Счетчик периодов спокойствия
    bool zeroTransX=0, zeroTransY=0, zeroTransZ=0;  //Переход через 0

    p_data quatern2rotMatCOORD(Quaternion quats, p_data poi);
    p_data transMat(p_data yp, p_data zp);
    Quaternion q_read;

};

#endif // DATAWORK_H
