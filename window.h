//****************************************************************************//

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QtWidgets>


class GLWidget;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

public:
    void saveFile();
    void readFromFile();
    void enableButtons();
    void disableButtons();

private slots:
    void runPort();
    void chooseChartChange();
    void selectSens();
    void chooseFilterChange();

private:
    QPushButton *clearBtn;//чистка экрана
    QPushButton *xBtn;//кнопки вращения
    QPushButton *yBtn;
    QPushButton *zBtn;
    QPushButton *readLog;//чтение лога
    QLabel      *accLabel;
    QComboBox   *accComboBox;//выбор чувствительности
    QPushButton *selectAcc;
    QLabel      *m_serialPortLabel;//выбор порта
    QComboBox   *m_serialPortComboBox;
    QLabel      *m_serialPortBaudRateLabel;//выбор скорости
    QComboBox   *m_serialPortBaudRateComboBox;
    QPushButton *runStopButton;//кнопка запуска
    QPushButton *saveLogButton;//кнопка сохранения лога
    QLabel      *selectProection;//выбор поекции
    QLabel      *infoLabel;//выходыне данные
    QComboBox   *m_chooseChart;//выбор отображаемого графика
    QLabel      *chooseLabel;
    QLabel      *chooseFilterLabel;//выбор способа фильтрации
    QComboBox   *m_chooseFilter;
    //layouts
    QVBoxLayout *mainLayout;//общая компоновка
    QHBoxLayout *container;
    QHBoxLayout *btnContainer;//кнопки
    QHBoxLayout *chooseContainer;
    GLWidget    *currentGlWidget;



};

#endif
