//****************************************************************************//

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include "masterthread.h"
#include "datawork.h"

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = 0);
    ~GLWidget();


public:
    MasterThread *m_thread = new MasterThread;//поток чтения данных
    void xRotat();
    void yRotat();
    void zRotat();
    void rot_2d();
    void rot_3d();
    void selectSens(float sens);
    void clearGraph();
    void saveLog();
    void readFromFile();
    void selectFilter(int mode);

    int chart_mod = 1;

public slots:
    void runPort(QString name_port, int baudRate);

private slots:
    void showResponse(const QString &s);
    void processError(const QString &s);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void drawAxis();
    void drawAxis2D();
    void mousePressEvent(QMouseEvent* pe);
    void mouseMoveEvent(QMouseEvent* pe);
    void wheelEvent(QWheelEvent *);
    void draw_trajectory();
    void draw_accelerations();
    void draw_stationary();
    void draw_velocities();
    void draw_positions();



private:
    QPoint mousePos;
    int xRot;
    int yRot;
    int zRot;
    float minX,minY,minZ,maxX,maxY,maxZ;//max for 3d axis
    float koef_2d;//for 2d scale
    float xL,yL,zL,scale;


    void renderText(double x, double y, QString str);


    datawork bins_data;

    QString bufline;



};

#endif
