//****************************************************************************//

#include "glwidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <math.h>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      xRot(105),
      yRot(180),
      zRot(60),
      xL(0),
      yL(0),
      zL(0),
      scale(0.45),
      minX(-0.1),
      minY(-0.1),
      minZ(-0.1),
      maxX(0.1),
      maxY(0.1),
      maxZ(0.1),
      koef_2d(5)
{
    connect(m_thread, &MasterThread::response, this, &GLWidget::showResponse);
    connect(m_thread, &MasterThread::error, this, &GLWidget::processError);
}

GLWidget::~GLWidget()
{
    bins_data.clearAll();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void GLWidget::paintGL()
{
    glClearColor(169,25,50,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    glRotatef(xRot, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot, 0.0f, 1.0f, 0.0f);
    glRotatef(zRot, 0.0f, 0.0f, 1.0f);
    glScalef(scale, scale, scale);
    glTranslatef(xL, yL, zL);

    switch(chart_mod)
    {
        case 1:
        {
            drawAxis();
            draw_trajectory();
            break;
        }
        case 2:
        {
            drawAxis2D();
            draw_accelerations();
            break;
        }
        case 3:
        {
            drawAxis2D();
            draw_stationary();
            break;
        }
        case 4:
        {
            drawAxis2D();
            draw_velocities();
            break;
        }
        case 5:
        {
            drawAxis2D();
            draw_positions();
            break;
        }
    }

    renderText(30,30,"Input line: " + bufline);
    QString s1;
    if (bins_data.count < 200)
    {
        s1 = "calibrate ...  wait ... ";
        s1.append(QString::number(200 - bins_data.count));
    }
    else
    {
        s1 = "count: ";
        s1.append(QString::number(bins_data.count));
    }
    renderText(30,50,s1);

    glPopMatrix();
    this->update();
}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void GLWidget::drawAxis()//построение координатных осей
{
   glLineWidth(1.0f);
   glDisable(GL_DEPTH_TEST);
   glColor3f (0.85f, 0.85f, 0.85f);
   float ad = 0.7;
   float buf = abs(minZ-ad);
   for (float i = 0;i <= (maxZ+ad+buf);i+=(maxZ+ad+buf)/10)
   {
       glBegin(GL_LINE_STRIP);
           glColor3f(0.85f, 0.85f, 0.85f);
           glVertex3f( minX - ad, maxY + ad, minZ - ad  + i);
           glVertex3f( minX - ad, minY - ad, minZ - ad + i);
           glVertex3f( maxX + ad, minY - ad, minZ - ad + i);
           glColor3f(0.0,0.0,0.0);
           glVertex3f( maxX + ad + ad/20, minY - ad, minZ - ad + i);
       glEnd();
   }
   float buf1 = abs(minX - ad);
   for (float i = 0;i <= (maxX+ad+buf1);i+=(maxX+ad+buf1)/10)
   {
       glBegin(GL_LINE_STRIP);
           glColor3f(0.85f, 0.85f, 0.85f);
           glVertex3f( minX - ad + i, minY - ad, maxZ + ad);
           glVertex3f( minX - ad + i, minY - ad, minZ - ad);
           glVertex3f( minX - ad + i, maxY + ad, minZ - ad);
           glColor3f(0.0,0.0,0.0);
           glVertex3f( minX - ad + i , maxY + ad + ad/20, minZ - ad);
       glEnd();
   }
   float buf2 = abs(minY - ad);
   for (float i = 0;i <= (maxY+ad+buf2);i+=(maxY+ad+buf2)/10)
   {
       glBegin(GL_LINE_STRIP);
           glColor3f(0.85f, 0.85f, 0.85f);
           glVertex3f( minX - ad, minY - ad + i, maxZ + ad);
           glVertex3f( minX - ad, minY - ad + i, minZ - ad);
           glVertex3f( maxX + ad, minY - ad + i, minZ - ad);
           glColor3f(0.0,0.0,0.0);
           glVertex3f( maxX + ad + ad/20, minY - ad + i, minZ - ad);
       glEnd();
   }

   glColor3f (0.0, 0.0, 0.0);
   glBegin(GL_LINE_STRIP);
           glVertex3f(  minX - ad,  maxY + ad,  minZ - ad);
           glVertex3f(  maxX + ad,  maxY + ad,  minZ - ad);
           glVertex3f(  maxX + ad,  minY - ad,  minZ - ad);
           //renderText(coor/2,coor,"X",QFont("Arial",20,QFont::Bold,false));
           glVertex3f(  maxX + ad,  minY - ad,  maxZ + ad);
           //glVertex3f(  0,  0,  0);
   glEnd();
}

void GLWidget::drawAxis2D()
{
    glLineWidth(1.0f);
    glColor3f(0,0,0);
    glBegin(GL_LINES);
        glVertex2f(-koef_2d + 2,-koef_2d);
        glVertex2f( koef_2d - 2,-koef_2d);
    glEnd();
    glBegin(GL_LINES);
        glVertex2f(0,-koef_2d);
        glVertex2f(0, koef_2d * 4);
    glEnd();
    for (int i = -koef_2d*2; i < koef_2d*4; i++)
    {
        glBegin(GL_LINES);
            glVertex2f(-0.05,i*0.5);
            glVertex2f( 0.05,i*0.5);
        glEnd();
    }
    for (int i = 1; i < koef_2d; i++)
    {
        glBegin(GL_LINES);
            glVertex2f(i*0.5,-koef_2d - 0.05);
            glVertex2f(i*0.5,-koef_2d + 0.05);
        glEnd();
    }
    for (int i = 1; i < koef_2d; i++)
    {
        glBegin(GL_LINES);
            glVertex2f(-i*0.5,-koef_2d - 0.05);
            glVertex2f(-i*0.5,-koef_2d + 0.05);
        glEnd();
    }
}

void GLWidget::draw_trajectory()
{

    float x1 = 0.0;
    float y1 = 0.0;
    float z1 = 0.0;
    float x2 = 0.0;
    float y2 = 0.0;
    float z2 = 0.0;

    for (int i = 0; i < bins_data.count; i++)//pdata.count
    {
        glPointSize(15.0); // Задаем размер точки
        x1 = x2;y1 = y2;z1 = z2;
        x2 = bins_data.positions[i].x;
        y2 = bins_data.positions[i].y;
        z2 = bins_data.positions[i].z;
        glColor3f(1,0,0);
        glBegin(GL_LINE_STRIP);
            glVertex3f(x1,y1,z1);
            glVertex3f(x2,y2,z2);
        glEnd();
        // min
        if (minX > x2) { minX = x2; }
        if (minY > y2) { minY = y2; }
        if (minZ > z2) { minZ = z2; }
        //max
        if (maxX < x2) { maxX = x2; }
        if (maxY < y2) { maxY = y2; }
        if (maxZ < z2) { maxZ = z2; }
    }
    //CUBE-----------------//
    float cubeMatrix[8][3] =
    {
        {    0.0,    0.0,   0.0},//0
        {   0.35,    0.0,   0.0},//1
        {    0.0,   0.35,   0.0},//2
        {    0.0,    0.0,  0.35},//3
    };
    glPointSize(18);
    glBegin(GL_POINTS);
      glVertex3f(x2,y2,z2);
    glEnd();
    p_data pCub;
    //0
    pCub.x = cubeMatrix[0][0];
    pCub.y = cubeMatrix[0][1];
    pCub.z = cubeMatrix[0][2];
    pCub = bins_data.quatern2rotMatCOORD(bins_data.q_read,pCub);
    cubeMatrix[0][0] = pCub.x;
    cubeMatrix[0][1] = pCub.y;
    cubeMatrix[0][2] = pCub.z;
    //1
    pCub.x = cubeMatrix[1][0];
    pCub.y = cubeMatrix[1][1];
    pCub.z = cubeMatrix[1][2];
    pCub = bins_data.quatern2rotMatCOORD(bins_data.q_read,pCub);
    cubeMatrix[1][0] = pCub.x;
    cubeMatrix[1][1] = pCub.y;
    cubeMatrix[1][2] = pCub.z;
    //2
    pCub.x = cubeMatrix[2][0];
    pCub.y = cubeMatrix[2][1];
    pCub.z = cubeMatrix[2][2];
    pCub = bins_data.quatern2rotMatCOORD(bins_data.q_read,pCub);
    cubeMatrix[2][0] = pCub.x;
    cubeMatrix[2][1] = pCub.y;
    cubeMatrix[2][2] = pCub.z;
    //3
    pCub.x = cubeMatrix[3][0];
    pCub.y = cubeMatrix[3][1];
    pCub.z = cubeMatrix[3][2];
    pCub = bins_data.quatern2rotMatCOORD(bins_data.q_read,pCub);
    cubeMatrix[3][0] = pCub.x;
    cubeMatrix[3][1] = pCub.y;
    cubeMatrix[3][2] = pCub.z;
    p_data pcub1;
    p_data pcub2;
    pcub2.x = x2;
    pcub2.y = y2;
    pcub2.z = z2;
    //0
    pcub1.x = cubeMatrix[0][0];
    pcub1.y = cubeMatrix[0][1];
    pcub1.z = cubeMatrix[0][2];
    pcub1 = bins_data.transMat(pcub1,pcub2);
    cubeMatrix[0][0] = pcub1.x;
    cubeMatrix[0][1] = pcub1.y;
    cubeMatrix[0][2] = pcub1.z;
    //1
    pcub1.x = cubeMatrix[1][0];
    pcub1.y = cubeMatrix[1][1];
    pcub1.z = cubeMatrix[1][2];
    pcub1 = bins_data.transMat(pcub1,pcub2);
    cubeMatrix[1][0] = pcub1.x;
    cubeMatrix[1][1] = pcub1.y;
    cubeMatrix[1][2] = pcub1.z;
    //2
    pcub1.x = cubeMatrix[2][0];
    pcub1.y = cubeMatrix[2][1];
    pcub1.z = cubeMatrix[2][2];
    pcub1 = bins_data.transMat(pcub1,pcub2);
    cubeMatrix[2][0] = pcub1.x;
    cubeMatrix[2][1] = pcub1.y;
    cubeMatrix[2][2] = pcub1.z;
    //3
    pcub1.x = cubeMatrix[3][0];
    pcub1.y = cubeMatrix[3][1];
    pcub1.z = cubeMatrix[3][2];
    pcub1 = bins_data.transMat(pcub1,pcub2);
    cubeMatrix[3][0] = pcub1.x;
    cubeMatrix[3][1] = pcub1.y;
    cubeMatrix[3][2] = pcub1.z;
    glLineWidth(2.0f);
    glColor3f(0.0,0.4,0.9);
    glBegin(GL_LINE_STRIP); // Прямоугольник (по часовой)
      glColor3f(1,0,0);
      glVertex3f(cubeMatrix[0][0],cubeMatrix[0][1],cubeMatrix[0][2]);
      glVertex3f(cubeMatrix[1][0],cubeMatrix[1][1],cubeMatrix[1][2]);
    glEnd();
    glBegin(GL_LINE_STRIP);
      glColor3f(0,1,0);
      glVertex3f(cubeMatrix[0][0],cubeMatrix[0][1],cubeMatrix[0][2]);
      glVertex3f(cubeMatrix[2][0],cubeMatrix[2][1],cubeMatrix[2][2]);
    glEnd();
    glColor3f(0,0,1);
      glBegin(GL_LINE_STRIP);
      glVertex3f(cubeMatrix[0][0],cubeMatrix[0][1],cubeMatrix[0][2]);
      glVertex3f(cubeMatrix[3][0],cubeMatrix[3][1],cubeMatrix[3][2]);
    glEnd();
    glColor3f(1,0,0);
    glPointSize(1.9);
    glBegin(GL_POINTS);
      glVertex3f(cubeMatrix[1][0],cubeMatrix[1][1],cubeMatrix[1][2]);
    glEnd();
    glColor3f(0,1,0);
    glBegin(GL_POINTS);
      glVertex3f(cubeMatrix[2][0],cubeMatrix[2][1],cubeMatrix[2][2]);
    glEnd();
    glColor3f(0,0,1);
    glBegin(GL_POINTS);
      glVertex3f(cubeMatrix[3][0],cubeMatrix[3][1],cubeMatrix[3][2]);
    glEnd();
    //END-----------------//

}

void GLWidget::draw_accelerations()
{
//    QString sTX;
//    sTX = "X";
//    renderText(1190,50,sTX);
//    QString sTY;
//    sTY = "Y";
//    renderText(1190,70,sTY);
//    QString sTZ;
//    sTZ = "Z";
//    renderText(1190,90,sTZ);
//    glLineWidth(2.0f);
//    glColor3f(1,0,0);
//    glBegin(GL_LINES);
//        glVertex2f(4.75,4);
//        glVertex2f(4.75,5);
//    glEnd();
//    glColor3f(0,1,0);
//    glBegin(GL_LINE_STRIP);
//        glVertex2f(4.5,4);
//        glVertex2f(4.5,5);
//    glEnd();
//    glColor3f(0,0,1);
//    glBegin(GL_LINE_STRIP);
//        glVertex2f(4.25,4);
//        glVertex2f(4.25,5);
//    glEnd();
    float a_x1 = 0.0;
    float a_y1 = 0.0;
    float a_z1 = 0.0;
    float a_x2 = 0.0;
    float a_y2 = 0.0;
    float a_z2 = 0.0;
    float y1 = -koef_2d;
    float y2 = -koef_2d;
    for (int i = 0; i < bins_data.count; i++)//pdata.count
    {
        glPointSize(15.0); // Задаем размер точки

        y1 = y2;
        y2 = i*0.01-koef_2d;

        a_x1 = a_x2;
        a_y1 = a_y2;
        a_z1 = a_z2;
        a_x2 = bins_data.accelerations[i].x;
        a_y2 = bins_data.accelerations[i].y;
        a_z2 = bins_data.accelerations[i].z;


         glColor3f(1,0,0);
         glBegin(GL_LINES);
             glVertex2f(a_x1,y1);
             glVertex2f(a_x2,y2);
         glEnd();
         glColor3f(0,1,0);
         glBegin(GL_LINE_STRIP);
             glVertex2f(a_y1,y1);
             glVertex2f(a_y2,y2);
         glEnd();
         glColor3f(0,0,1);
         glBegin(GL_LINE_STRIP);
             glVertex2f(a_z1,y1);
             glVertex2f(a_z2,y2);
         glEnd();

    }
}

void GLWidget::draw_stationary()
{
//    QString sTX;
//    sTX = "magnitude";
//    renderText(1080,50,sTX);
//    QString sTY;
//    sTY = "stat_periods";
//    renderText(1080,70,sTY);
//    glLineWidth(2.0f);
//    glColor3f(1,0,0);
//    glBegin(GL_LINES);
//        glVertex2f(4.75,3);
//        glVertex2f(4.75,4);
//    glEnd();
//    glColor3f(0,0,0);
//    glBegin(GL_LINE_STRIP);
//        glVertex2f(4.5,3);
//        glVertex2f(4.5,4);
//    glEnd();
    float a_x1 = 0.0;
    float a_y1 = 0.0;
    float a_x2 = 0.0;
    float a_y2 = 0.0;
    glLineWidth(2.0f);
    float y1 = -koef_2d;
    float y2 = -koef_2d;
    for (int i = 0; i < bins_data.count; i++)//pdata.count
    {
        glPointSize(15.0); // Задаем размер точки

        y1 = y2;
        y2 = i*0.01-koef_2d;
        a_x1 = a_x2;
        a_y1 = a_y2;
        a_x2 = bins_data.statPeriods[i];
        a_y2 = bins_data.magnLp[i];
        glColor3f(0,0,0);
        glBegin(GL_LINES);
            glVertex2f(a_x1,y1);
            glVertex2f(a_x2,y2);
        glEnd();
        glColor3f(1,0,0);
        glBegin(GL_LINE_STRIP);
            glVertex2f(a_y1,y1);
            glVertex2f(a_y2,y2);
        glEnd();
        glColor3f(0,0,1);
    }
}

void GLWidget::draw_velocities()
{
//    QString sTX;
//    sTX = "X";
//    renderText(1190,50,sTX);
//    QString sTY;
//    sTY = "Y";
//    renderText(1190,70,sTY);
//    QString sTZ;
//    sTZ = "Z";
//    renderText(1190,90,sTZ);
//    glLineWidth(2.0f);
//    glColor3f(1,0,0);
//    glBegin(GL_LINES);
//        glVertex2f(4.75,4);
//        glVertex2f(4.75,5);
//    glEnd();
//    glColor3f(0,1,0);
//    glBegin(GL_LINE_STRIP);
//        glVertex2f(4.5,4);
//        glVertex2f(4.5,5);
//    glEnd();
//    glColor3f(0,0,1);
//    glBegin(GL_LINE_STRIP);
//        glVertex2f(4.25,4);
//        glVertex2f(4.25,5);
//    glEnd();
    float a_x1 = 0.0;
    float a_y1 = 0.0;
    float a_z1 = 0.0;
    float a_x2 = 0.0;
    float a_y2 = 0.0;
    float a_z2 = 0.0;
    glLineWidth(2.0f);
    float y1 = -koef_2d;
    float y2 = -koef_2d;
    for (int i = 0; i < bins_data.count; i++)//pdata.count
    {
        glPointSize(15.0); // Задаем размер точки
        y1 = y2;
        y2 = i*0.01-koef_2d;
        a_x1 = a_x2;
        a_y1 = a_y2;
        a_z1 = a_z2;
        a_x2 = bins_data.velocities[i].x;
        a_y2 = bins_data.velocities[i].y;
        a_z2 = bins_data.velocities[i].z;
        glColor3f(1,0,0);
        glBegin(GL_LINES);
            glVertex2f(a_x1,y1);
            glVertex2f(a_x2,y2);
        glEnd();
        glColor3f(0,1,0);
        glBegin(GL_LINE_STRIP);
            glVertex2f(a_y1,y1);
            glVertex2f(a_y2,y2);
        glEnd();
        glColor3f(0,0,1);
        glBegin(GL_LINE_STRIP);
            glVertex2f(a_z1,y1);
            glVertex2f(a_z2,y2);
        glEnd();
    }
}

void GLWidget::draw_positions()
{
//    QString sTX;
//    sTX = "X";
//    renderText(1190,50,sTX);
//    QString sTY;
//    sTY = "Y";
//    renderText(1190,70,sTY);
//    QString sTZ;
//    sTZ = "Z";
//    renderText(1190,90,sTZ);
//    glLineWidth(2.0f);
//    glColor3f(1,0,0);
//    glBegin(GL_LINES);
//        glVertex2f(4.75,4);
//        glVertex2f(4.75,5);
//    glEnd();
//    glColor3f(0,1,0);
//    glBegin(GL_LINE_STRIP);
//        glVertex2f(4.5,4);
//        glVertex2f(4.5,5);
//    glEnd();
//    glColor3f(0,0,1);
//    glBegin(GL_LINE_STRIP);
//        glVertex2f(4.25,4);
//        glVertex2f(4.25,5);
//    glEnd();
    float a_x1 = 0.0;
    float a_y1 = 0.0;
    float a_z1 = 0.0;
    float a_x2 = 0.0;
    float a_y2 = 0.0;
    float a_z2 = 0.0;
    glLineWidth(2.0f);
    float y1 = -koef_2d;
    float y2 = -koef_2d;
    for (int i = 0; i < bins_data.count; i++)//pdata.count
    {
        glPointSize(15.0); // Задаем размер точки
        y1 = y2;
        y2 = i*0.01-koef_2d;
        a_x1 = a_x2;
        a_y1 = a_y2;
        a_z1 = a_z2;
        a_x2 = bins_data.positions[i].x;
        a_y2 = bins_data.positions[i].y;
        a_z2 = bins_data.positions[i].z;
        glColor3f(1,0,0);
        glBegin(GL_LINES);
            glVertex2f(a_x1,y1);
            glVertex2f(a_x2,y2);
        glEnd();
        glColor3f(0,1,0);
        glBegin(GL_LINE_STRIP);
            glVertex2f(a_y1,y1);
            glVertex2f(a_y2,y2);
        glEnd();
        glColor3f(0,0,1);
        glBegin(GL_LINE_STRIP);
            glVertex2f(a_z1,y1);
            glVertex2f(a_z2,y2);
        glEnd();
    }
}

void GLWidget::mousePressEvent(QMouseEvent* pe)
{
    mousePos = pe->pos();

}

void GLWidget::mouseMoveEvent(QMouseEvent* pe)
{
   if ((pe->buttons() == Qt::LeftButton) && (chart_mod == 1))
   {
       xRot += 120/0.1*(GLfloat)(pe->y()-mousePos.y())/height();
       //yRotation += 180/0.1*(GLfloat)(pe->x()-mousePos.y())/height();
       zRot += 120/0.1*(GLfloat)(pe->x()-mousePos.x())/width();
       mousePos = pe->pos();
   }

   if (pe->buttons() == Qt::RightButton)
   {
       xL += (0.1/scale)*(GLfloat)(pe->x()-mousePos.x())/height();
       yL -= (0.1/scale)*(GLfloat)(pe->y()-mousePos.y())/width();
   }
  //updateGL();
  this->update();
}

void GLWidget::wheelEvent(QWheelEvent* pe)
{
   if ((pe->delta())>0)
   {
       if (scale < 3.0)
       {
            scale *= 1.2;
            koef_2d /=1.2;
       }
   }
   else
       if ((pe->delta())<0)
       {
           scale /= 1.2;
           koef_2d *= 1.2;
       }
   this->update();
}

void GLWidget::showResponse(const QString &s)
{
    bins_data.readStr(s);
    bufline = s;
}

void GLWidget::processError(const QString &s)
{
    QMessageBox::warning(0,"Warning", "Warning message text");
    //QMessageBox::warning(tr("Status: Not running, %1.").arg(s));
}

void GLWidget::runPort(QString name_port, int baudRate)
{

    if (m_thread->isRunning())
    {
        m_thread->stop();
    }
    else
    {
        m_thread->runPort(name_port, baudRate);
    }
}

void GLWidget::renderText(double x, double y, QString str)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Courier", 12));
    painter.drawText(x, y, str);
    painter.end();

}

void GLWidget::xRotat()
{
   xRot = 180;
   yRot = 0;
   zRot = 270;
}

void GLWidget::yRotat()
{
   xRot = 0;
   yRot = 180;
   zRot = 90;
}

void GLWidget::zRotat()
{
   xRot = 90;
   yRot = 180;
   zRot = 90;
}

void GLWidget::rot_2d()
{
    scale = 0.19;
    koef_2d = 5;
    xRot = 180;
    yRot = 0;
    zRot = 270;
}

void GLWidget::rot_3d()
{
    xRot = 105;
    yRot = 180;
    zRot = 60;
    scale = 0.45;
}

void GLWidget::selectSens(float sens)
{
    bins_data.statKoeff = sens;
}

void GLWidget::clearGraph()
{
    bins_data.clearAll();
}

void GLWidget::saveLog()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Save file..."), "",
                        tr("Text files (*.txt)"));
    if (fileName.isEmpty()) return;
    QFile f(fileName);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&f);
        for (int i = 0; i < bins_data.count; i++)
        {
            stream << bins_data.allData[i] << "\n";
        }
        f.close();
    }
    else
    {
        QMessageBox::warning(this,"Error",QString("Could not open file %1for"
                                                  "reading ") .arg(f.fileName()),QMessageBox::Ok) ;
    }

}

void GLWidget::readFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                        tr("Open file..."), "",
                        tr("Text files (*.txt)"));
    if (fileName.isEmpty()) return;
    QFile f(fileName);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!f.atEnd())
        {
            //читаем строку
            QString str = f.readLine().trimmed();;
            bins_data.readStr(str);
        }

        f.close();
    }
    else
    {
        QMessageBox::warning(this,"Error",QString("Could not open file %1for"
                                                  "reading ") .arg(f.fileName()),QMessageBox::Ok) ;
    }
}

void GLWidget::selectFilter(int mode)
{
    switch(mode)
    {
        case 1:
        {
            bins_data.filter_mode = 1;
            break;
        }
        case 2:
        {
            bins_data.filter_mode = 2;
            break;
        }
    }
}

