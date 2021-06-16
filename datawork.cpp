#include "datawork.h"
#include <math.h>
#include <iostream>

#include <eigen3/Eigen/Dense>

typedef std::vector<int> vectori;
typedef std::vector<double> vectord;

void add_index_range(vectori &indices, int beg, int end, int inc = 1)
{
    for (int i = beg; i <= end; i += inc)
    {
       indices.push_back(i);
    }
}

void add_index_const(vectori &indices, int value, size_t numel)
{
    while (numel--)
    {
        indices.push_back(value);
    }
}

void append_vector(vectord &vec, const vectord &tail)
{
    vec.insert(vec.end(), tail.begin(), tail.end());
}

vectord subvector_reverse(const vectord &vec, int idx_end, int idx_start)
{
    vectord result(&vec[idx_start], &vec[idx_end+1]);
    std::reverse(result.begin(), result.end());
    return result;
}

inline int max_val(const vectori& vec)
{
    return std::max_element(vec.begin(), vec.end())[0];
}

void filter(vectord B, vectord A, const vectord &X, vectord &Y, vectord &Zi)
{
    if (A.empty())
    {
        throw std::domain_error("The feedback filter coefficients are empty.");
    }
    if (std::all_of(A.begin(), A.end(), [](double coef){ return coef == 0; }))
    {
        throw std::domain_error("At least one of the feedback filter coefficients has to be non-zero.");
    }
    if (A[0] == 0)
    {
        throw std::domain_error("First feedback coefficient has to be non-zero.");
    }

    // Normalize feedback coefficients if a[0] != 1;
    auto a0 = A[0];
    if (a0 != 1.0)
    {
        std::transform(A.begin(), A.end(), A.begin(), [a0](double v) { return v / a0; });
        std::transform(B.begin(), B.end(), B.begin(), [a0](double v) { return v / a0; });
    }

    size_t input_size = X.size();
    size_t filter_order = std::max(A.size(), B.size());
    B.resize(filter_order, 0);
    A.resize(filter_order, 0);
    Zi.resize(filter_order, 0);
    Y.resize(input_size);

    const double *x = &X[0];
    const double *b = &B[0];
    const double *a = &A[0];
    double *z = &Zi[0];
    double *y = &Y[0];

    for (size_t i = 0; i < input_size; ++i)
    {
        size_t order = filter_order - 1;
        while (order)
        {
            if (i >= order)
            {
                z[order - 1] = b[order] * x[i - order] - a[order] * y[i - order] + z[order];
            }
            --order;
        }
        y[i] = b[0] * x[i] + z[0];
    }
    Zi.resize(filter_order - 1);
}

void filtfilt(vectord B, vectord A, const vectord &X, vectord &Y)
{
    using namespace Eigen;

    int len = X.size();     // length of input
    int na = A.size();
    int nb = B.size();
    int nfilt = (nb > na) ? nb : na;
    int nfact = 3 * (nfilt - 1); // length of edge transients

    if (len <= nfact)
    {
        throw std::domain_error("Input data too short! Data must have length more than 3 times filter order.");
    }

    // set up filter's initial conditions to remove DC offset problems at the
    // beginning and end of the sequence
    B.resize(nfilt, 0);
    A.resize(nfilt, 0);

    vectori rows, cols;
    //rows = [1:nfilt-1           2:nfilt-1             1:nfilt-2];
    add_index_range(rows, 0, nfilt - 2);
    if (nfilt > 2)
    {
        add_index_range(rows, 1, nfilt - 2);
        add_index_range(rows, 0, nfilt - 3);
    }
    //cols = [ones(1,nfilt-1)         2:nfilt-1          2:nfilt-1];
    add_index_const(cols, 0, nfilt - 1);
    if (nfilt > 2)
    {
        add_index_range(cols, 1, nfilt - 2);
        add_index_range(cols, 1, nfilt - 2);
    }
    // data = [1+a(2)         a(3:nfilt)        ones(1,nfilt-2)    -ones(1,nfilt-2)];

    auto klen = rows.size();
    vectord data;
    data.resize(klen);
    data[0] = 1 + A[1];  int j = 1;
    if (nfilt > 2)
    {
        for (int i = 2; i < nfilt; i++)
            data[j++] = A[i];
        for (int i = 0; i < nfilt - 2; i++)
            data[j++] = 1.0;
        for (int i = 0; i < nfilt - 2; i++)
            data[j++] = -1.0;
    }

    vectord leftpad = subvector_reverse(X, nfact, 1);
    double _2x0 = 2 * X[0];
    std::transform(leftpad.begin(), leftpad.end(), leftpad.begin(), [_2x0](double val) {return _2x0 - val; });

    vectord rightpad = subvector_reverse(X, len - 2, len - nfact - 1);
    double _2xl = 2 * X[len-1];
    std::transform(rightpad.begin(), rightpad.end(), rightpad.begin(), [_2xl](double val) {return _2xl - val; });

    double y0;
    vectord signal1, signal2, zi;

    signal1.reserve(leftpad.size() + X.size() + rightpad.size());
    append_vector(signal1, leftpad);
    append_vector(signal1, X);
    append_vector(signal1, rightpad);

    // Calculate initial conditions
    MatrixXd sp = MatrixXd::Zero(max_val(rows) + 1, max_val(cols) + 1);
    for (size_t k = 0; k < klen; ++k)
    {
        sp(rows[k], cols[k]) = data[k];
    }
    auto bb = VectorXd::Map(B.data(), B.size());
    auto aa = VectorXd::Map(A.data(), A.size());
    MatrixXd zzi = (sp.inverse() * (bb.segment(1, nfilt - 1) - (bb(0) * aa.segment(1, nfilt - 1))));
    zi.resize(zzi.size());

    // Do the forward and backward filtering
    y0 = signal1[0];
    std::transform(zzi.data(), zzi.data() + zzi.size(), zi.begin(), [y0](double val){ return val*y0; });
    filter(B, A, signal1, signal2, zi);
    std::reverse(signal2.begin(), signal2.end());
    y0 = signal2[0];
    std::transform(zzi.data(), zzi.data() + zzi.size(), zi.begin(), [y0](double val){ return val*y0; });
    filter(B, A, signal2, signal1, zi);
    Y = subvector_reverse(signal1, signal1.size() - nfact - 1, nfact);
}

datawork::datawork()
{

}

datawork::~datawork()
{
    datastr.clear();
    memset(&a_read, 0, sizeof(a_read));
    memset(&q_read, 0, sizeof(q_read));
    allData.clear();//все строки
    quaternions.clear();//все кватернионы
    accelerations.clear();//все ускорения
    velocities.clear();//все скорости
    positions.clear();//все позиции
    acc_nograv.clear();//ускорения после удаления гравитации
    magnLp.clear();//магнитуды ускорений
    statPeriods.clear();//стационарные периоды
    count = 0;
    lpMag = 0;
}

void datawork::readStr(QString str)
{
    datastr = str;
    lineToData();

}

void datawork::lineToData()
{
    if ((datastr[0] == "+") && (datastr[datastr.size() - 1] == "+"))
    {
        allData.push_back(datastr);
        datastr.remove(0,1);
        datastr.remove(datastr.size() - 1,1);        
        if (datastr.contains("+", Qt::CaseInsensitive)) return;
        QString valueLine = "";
        float value = 0;
        int valueNumber = 0;
        for (int i = 0; i < datastr.length(); i++)
        {
            if ((datastr[i] != ','))
            {
                valueLine = valueLine.append(datastr[i]);
            }
            else
            {
                value = valueLine.toFloat();
                switch(valueNumber)
                {
                    case 0 : {
                        q_read.q0 = value;
                        break;
                    }
                    case 1 : {
                        q_read.q1 = value;
                        break;
                    }
                    case 2 : {
                        q_read.q2 = value;
                        break;
                    }
                    case 3 : {
                        q_read.q3 = value;
                        break;
                    }
                    case 4 : {
                        a_read.x = value;
                        break;
                    }
                    case 5 : {
                        a_read.y = value;
                        break;
                    }
                }

                valueNumber++;
                value = 0;
                valueLine = "";
            }

            if (i == datastr.length() - 1)
            {
                value = valueLine.toFloat();
                a_read.z = value;
                valueNumber++;
                value = 0;
                valueLine = "";
            }
        }
        switch(filter_mode)
        {
            case 1:
            {
                filt();
                break;
            }
            case 2:
            {
                filtMatlab();
                break;
            }
        }
    }
}

void datawork::filt()
{

    //delete gravity
    /*
    float g0 = 2 * (q_read.q1 * q_read.q3 - q_read.q0 * q_read.q2);
    float g1 = 2 * (q_read.q0 * q_read.q1 + q_read.q2 * q_read.q3);
    float g2 = q_read.q0 * q_read.q0 - q_read.q1 * q_read.q1 -
            q_read.q2 * q_read.q2 + q_read.q3 * q_read.q3;
    a_read.x = a_read.x - g0;
    a_read.y = a_read.y - g1;
    a_read.z = a_read.z - g2;
*/
    //std::cout << count << "  " << a_read.x << " " << a_read.y << " " << a_read.z << std::endl;
    acc_nograv.push_back(a_read);
    //accelerations magnitute
    float accMag;
    accMag = sqrt(a_read.x*a_read.x + a_read.y*a_read.y + a_read.z*a_read.z);
    //lpfilter acc magnitude
    float alpha = 0.9;
    lpMag  = lpMag*(1 - alpha) + accMag * alpha;
    //std::cout<<"lpMag "<<lpMag<<std::endl;
    magnLp.push_back(lpMag);
    //identify staionary periods
    if (lpMag > statKoeff)
        statPeriods.push_back(0);
    else statPeriods.push_back(1);



    //get velocity
//    if ((lpMag < statKoeff) || (count < 200)) //for calibrate
//    {
//        i_vel.x = 0;
//        i_vel.y = 0;
//        i_vel.z = 0;
//    }


    if (lpMag < statKoeff)
    {
        if(i_pos.x != 0) i_vel.x = i_pos.x;       //Для угловой скорости
        if(i_pos.y != 0) i_vel.y = i_pos.y;
        if(i_pos.z != 0) i_vel.z = i_pos.z;
        /*
                i_vel.x = 0;
                i_vel.y = 0;
                i_vel.z = 0;
*/
                //Сохраняем нулевые значения ускорения и кватернионы (за компанию)
                a_read.x=0;
                a_read.y=0;
                a_read.z=0;

                quaternions.push_back(q_read);
                accelerations.push_back(a_read);

              /*  zeroPeriod++;
                if(zeroPeriod>8)
                {
                    zeroTransX=false;
                    zeroTransY=false;
                    zeroTransZ=false;
                }*/
                std::cout << count << "  " << a_read.x << " " << a_read.y << " " << a_read.z << " " <<zeroPeriod <<std::endl;

//                if(countx>0) countx--;
//                if(county>0) county--;
//                if(countz>0) countz--;

    }
    else
    {
        //Смотрим резкий переход через 0
//        if((accelerations.back().x>0 && a_read.x<0) || (accelerations.back().x<0 && a_read.x>0)) {countx=3; a_read.x=0; i_vel.x = 0;}
//        if((accelerations.back().y>0 && a_read.y<0) || (accelerations.back().y<0 && a_read.y>0)) {county=3; a_read.y=0; i_vel.y = 0;}
//        if((accelerations.back().z>0 && a_read.z<0) || (accelerations.back().z<0 && a_read.z>0)) {countz=3; a_read.z=0; i_vel.z = 0;}

        //Отсекаем обратный импульс
//        if(countx>0) {a_read.x=0;}
//        if(county>0) {a_read.y=0;}
//        if(countz>0) {a_read.z=0;}

/*
        if(((accelerations.back().x>0 && a_read.x<0) || (accelerations.back().x<0 && a_read.x>0)) && zeroPeriod<8)  zeroTransX=true;
        if(((accelerations.back().y>0 && a_read.y<0) || (accelerations.back().y<0 && a_read.y>0)) && zeroPeriod<8)  zeroTransY=true;
        if(((accelerations.back().z>0 && a_read.z<0) || (accelerations.back().z<0 && a_read.z>0)) && zeroPeriod<8)  zeroTransZ=true;
        zeroPeriod=0;

        if(zeroTransX) {a_read.x=0; i_vel.x = 0;}
        if(zeroTransY) {a_read.y=0; i_vel.y = 0;}
        if(zeroTransZ) {a_read.z=0; i_vel.z = 0;}
*/
        std::cout << count << "  " << a_read.x << " " << a_read.y << " " << a_read.z << std::endl;
        quaternions.push_back(q_read);
        accelerations.push_back(a_read);

//        a_read.x*=9.8;
//        a_read.y*=9.8;
//        a_read.z*=9.8;

    i_vel.x = i_vel.x + a_read.x*samplePeriod;
    i_vel.y = i_vel.y + a_read.y*samplePeriod;
    i_vel.z = i_vel.z + a_read.z*samplePeriod;
    velocities.push_back(i_vel);


    //get position
//    i_pos.x = i_pos.x + i_vel.x*samplePeriod*10;
//    i_pos.y = i_pos.y + i_vel.y*samplePeriod*10;
//    i_pos.z = i_pos.z + i_vel.z*samplePeriod*10;
    i_pos.x = i_vel.x;        //Для угловой скорости
    i_pos.y = i_vel.y;
    i_pos.z = i_vel.z;
    positions.push_back(i_pos);

    //inc count
    count++;
    }
}

void datawork::clearAll()
{
    datastr.clear();
    memset(&a_read, 0, sizeof(a_read));
    memset(&q_read, 0, sizeof(q_read));
    allData.clear();//все строки
    quaternions.clear();//все кватернионы
    accelerations.clear();//все ускорения
    velocities.clear();//все скорости
    positions.clear();//все позиции
    acc_nograv.clear();//ускорения после удаления гравитации
    magnLp.clear();//магнитуды ускорений
    statPeriods.clear();//стационарные периоды
    count = 0;
    lpMag = 0;
}

void datawork::filtMatlab()
{
    //write data
    quaternions.push_back(q_read);
    accelerations.push_back(a_read);
    //delete gravity
    float g0 = 2 * (q_read.q1 * q_read.q3 - q_read.q0 * q_read.q2);
    float g1 = 2 * (q_read.q0 * q_read.q1 + q_read.q2 * q_read.q3);
    float g2 = q_read.q0 * q_read.q0 - q_read.q1 * q_read.q1 -
            q_read.q2 * q_read.q2 + q_read.q3 * q_read.q3;
    a_read.x = a_read.x - g0;
    a_read.y = a_read.y - g1;
    a_read.z = a_read.z - g2;
    std::cout << count << "  " << a_read.x << " " << a_read.y << " " << a_read.z << std::endl;
    acc_nograv.push_back(a_read);
    //accelerations magnitute
    float accMag;
    accMag = sqrt(a_read.x*a_read.x + a_read.y*a_read.y + a_read.z*a_read.z);
    //lpfilter acc magnitude
    float alpha = 0.9;
    lpMag  = lpMag*(1 - alpha) + accMag * alpha;
    magnLp.push_back(lpMag);
    //identify staionary periods
    if (lpMag > statKoeff)
        statPeriods.push_back(0);
    else statPeriods.push_back(1);
    //get velocity HP
    if (count > 200)
    {
        if ((lpMag < statKoeff) || (count < 200)) //for calibrate
        {
            i_vel.x = 0;
            i_vel.y = 0;
            i_vel.z = 0;
        }
        vectord b;
        vectord a;
        b.push_back(0.9976);
        b.push_back(-0.9976);
        a.push_back(1);
        a.push_back(-0.9951);
        i_vel.x = i_vel.x + a_read.x*samplePeriod*9.81;
        i_vel.y = i_vel.y + a_read.y*samplePeriod*9.81;
        i_vel.z = i_vel.z + a_read.z*samplePeriod*9.81;
        velocities.push_back(i_vel);
        vectord linVelHPX;
        vectord linVelHPY;
        vectord linVelHPZ;
        vectord linVelX;
        vectord linVelY;
        vectord linVelZ;
        for (int i = 0; i < count; i++)
        {
            linVelX.push_back(velocities[i].x);
            linVelY.push_back(velocities[i].y);
            linVelZ.push_back(velocities[i].z);
        }
        if (count <= 3 )
        {
            linVelHPX = linVelX;
            linVelHPY = linVelY;
            linVelHPZ = linVelZ;
        }
        else
        {
            filtfilt(b,a,linVelX,linVelHPX);
            filtfilt(b,a,linVelY,linVelHPY);
            filtfilt(b,a,linVelZ,linVelHPZ);
        }
        //get position
        if (count > 2)
        {
            i_pos.x = i_pos.x + linVelHPX[count]*samplePeriod;
            i_pos.y = i_pos.y + linVelHPY[count]*samplePeriod;
            i_pos.z = i_pos.z + linVelHPZ[count]*samplePeriod;
        }
        positions.push_back(i_pos);
    }
    else
    {
        velocities.push_back(i_vel);
        positions.push_back(i_pos);
    }
    //inc count
    count++;
}

p_data datawork::quatern2rotMatCOORD(Quaternion quats, p_data poi)
{
    mtrx m;
    p_data buf;
    Quaternion qB;
    const float n = 1.0f/sqrt(quats.q1*quats.q1+quats.q2*quats.q2+quats.q3*quats.q3+quats.q0*quats.q0);
    quats.q1 *= n;
    quats.q2 *= n;
    quats.q3 *= n;
    quats.q3 *= n;

    m[0][0] = 1.0f - 2.0f*quats.q2*quats.q2 - 2.0f*quats.q3*quats.q3;
    m[0][1] = 2.0f*quats.q1*quats.q2 - 2.0f*quats.q3*quats.q0;
    m[0][2] = 2.0f*quats.q1*quats.q3 + 2.0f*quats.q2*quats.q0;
    m[1][0] = 2.0f*quats.q1*quats.q2 + 2.0f*quats.q3*quats.q0;
    m[1][1] = 1.0f - 2.0f*quats.q1*quats.q1 - 2.0f*quats.q3*quats.q3;
    m[1][2] = 2.0f*quats.q2*quats.q3 - 2.0f*quats.q1*quats.q0;
    m[2][0] = 2.0f*quats.q1*quats.q3 - 2.0f*quats.q2*quats.q0;
    m[2][1] = 2.0f*quats.q2*quats.q3 + 2.0f*quats.q1*quats.q0;
    m[2][2] = 1.0f - 2.0f*quats.q1*quats.q1 - 2.0f*quats.q2*quats.q2;

    buf.x = poi.x*m[0][0] + poi.y*m[0][1] + poi.z*m[0][2];
    buf.y = poi.x*m[1][0] + poi.y*m[1][1] + poi.z*m[1][2];
    buf.z = poi.x*m[2][0] + poi.y*m[2][1] + poi.z*m[2][2];
    return buf;
}

p_data datawork::transMat(p_data yp, p_data zp)
{
    p_data Mat;
    Mat.x = yp.x + zp.x;
    Mat.y = yp.y + zp.y;
    Mat.z = yp.z + zp.z;
    return Mat;
}

