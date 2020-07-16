//****************************************************************************///


#include <QSerialPortInfo>
#include "window.h"
#include "glwidget.h"
#include <QFileDialog>
#include <QTextStream>


Window::Window()
{
    currentGlWidget = new GLWidget();
    mainLayout = new QVBoxLayout(this);//общая компоновка
    container = new QHBoxLayout();
    btnContainer = new QHBoxLayout();//кнопки
    chooseContainer = new QHBoxLayout();

    container->addWidget(currentGlWidget);
    //создаем кнопки
    clearBtn = new QPushButton(tr("Clear"), this);
    connect(clearBtn, &QPushButton::clicked, currentGlWidget, &GLWidget::clearGraph);
    xBtn = new QPushButton(tr("X"), this);
    connect(xBtn, &QPushButton::clicked, currentGlWidget, &GLWidget::xRotat);
    yBtn = new QPushButton(tr("Y"), this);
    connect(yBtn, &QPushButton::clicked, currentGlWidget, &GLWidget::yRotat);
    zBtn = new QPushButton(tr("Z"), this);
    connect(zBtn, &QPushButton::clicked, currentGlWidget, &GLWidget::zRotat);
    //read button
    readLog = new QPushButton(tr("Read"),this);
    connect(readLog, &QPushButton::clicked, this, &Window::readFromFile);

    accLabel = new QLabel(tr("Select sens:"),this);
    accComboBox = new QComboBox();
    QList<QString> list;
    list << "0.01" << "0.015" << "0.020" << "0.025" << "0.030"<< "0.035" << "0.040"<< "0.045" << "0.050" << "0.055" << "0.060" << "0.065" << "0.070" << "0.075" << "0.080" << "0.085";
    accComboBox->addItems(list);
    selectAcc = new QPushButton(tr("Select"),this);
    connect(selectAcc, &QPushButton::clicked, this, &Window::selectSens);
    //connect(this, &Window::signalFromButton, this, &Window::slotMessage);

    //serial port name select
    m_serialPortLabel = new QLabel(tr("Serial port:"),this);
    m_serialPortLabel->setMaximumWidth(70);
    m_serialPortComboBox = new QComboBox();
    m_serialPortComboBox->setMaximumWidth(80);
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
        m_serialPortComboBox->addItem(info.portName());
    //baud rate select
    m_serialPortBaudRateLabel = new QLabel(tr("Baud Rate:"),this);
    m_serialPortBaudRateLabel->setMaximumWidth(70);
    m_serialPortBaudRateComboBox = new QComboBox();
    m_serialPortBaudRateComboBox->setMaximumWidth(80);
    m_serialPortBaudRateComboBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    m_serialPortBaudRateComboBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    m_serialPortBaudRateComboBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    m_serialPortBaudRateComboBox->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    m_serialPortBaudRateComboBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    //run Stop button
    runStopButton = new QPushButton(tr("Run"),this);
    runStopButton->setAutoFillBackground(true);
    QPalette palette = runStopButton->palette();
    palette.setColor(QPalette::Window, QColor(Qt::green));
    runStopButton->setPalette(palette);

    connect(runStopButton, &QPushButton::clicked, this, &Window::runPort);

    //save button
    saveLogButton = new QPushButton(tr("Save log"),this);
    connect(saveLogButton, &QPushButton::clicked, this, &Window::saveFile);


    //choose filter
    chooseFilterLabel = new QLabel(tr("Select filter: "),this);
    m_chooseFilter = new QComboBox(this);
    m_chooseFilter->setMaximumWidth(300);
    m_chooseFilter->addItem("standart");
    m_chooseFilter->addItem("matlabfunction");
    connect(m_chooseFilter, &QComboBox::currentTextChanged, this, &Window::chooseFilterChange);

    //proection
    selectProection = new QLabel(tr("Proection:"),this);
    //choose
    m_chooseChart = new QComboBox(this);
    m_chooseChart->setMaximumWidth(300);
    m_chooseChart->addItem("trajectory");
    m_chooseChart->addItem("accelerations");
    m_chooseChart->addItem("stationary periods");
    m_chooseChart->addItem("velocities");
    m_chooseChart->addItem("positons");
    connect(m_chooseChart, &QComboBox::currentTextChanged, this, &Window::chooseChartChange);

    chooseLabel = new QLabel(tr("Chart:"),this);
    chooseLabel->setMaximumWidth(70);

    chooseContainer->addWidget(chooseLabel);
    chooseContainer->addWidget(m_chooseChart);

    chooseContainer->addWidget(chooseFilterLabel);
    chooseContainer->addWidget(m_chooseFilter);

    chooseContainer->setAlignment(Qt::AlignLeft);

    btnContainer->addWidget(m_serialPortLabel);
    btnContainer->addWidget(m_serialPortComboBox);
    btnContainer->addWidget(m_serialPortBaudRateLabel);
    btnContainer->addWidget(m_serialPortBaudRateComboBox);
    btnContainer->addWidget(runStopButton);
    btnContainer->addWidget(clearBtn);
    btnContainer->addWidget(saveLogButton);
    btnContainer->addWidget(readLog);
    btnContainer->addWidget(accLabel);
    btnContainer->addWidget(accComboBox);
    btnContainer->addWidget(selectAcc);
    btnContainer->addWidget(selectProection);
    btnContainer->addWidget(xBtn);
    btnContainer->addWidget(yBtn);
    btnContainer->addWidget(zBtn);

    QString temp = "Input Data:  serial port - ";
    temp.append(m_serialPortComboBox->currentText());
    temp.append("  baud rate - ");
    temp.append(m_serialPortBaudRateComboBox->currentText());
    infoLabel = new QLabel(this);
    infoLabel->setText(temp);
    infoLabel->setMaximumHeight(20);
    chooseContainer->addWidget(infoLabel);


    //собираем компоновку
    QWidget *w = new QWidget;
    w->setLayout(container);
    mainLayout->addWidget(w);
    //mainLayout->addWidget(infoLabel);
    mainLayout->addLayout(chooseContainer);
    mainLayout->addLayout(btnContainer);
    setLayout(mainLayout);
    setWindowTitle(tr("binsdataQT"));
}

void Window::runPort()
{
    if (currentGlWidget->m_thread->isRunning())
    {
        runStopButton->setText(tr("Start"));
        runStopButton->setAutoFillBackground(true);
        QPalette palette = runStopButton->palette();
        palette.setColor(QPalette::Window, QColor(Qt::green));
        runStopButton->setPalette(palette);
        enableButtons();
    }
    else
    {
        runStopButton->setText(tr("Stop"));
        runStopButton->setAutoFillBackground(true);
        QPalette palette = runStopButton->palette();
        palette.setColor(QPalette::Window, QColor(Qt::red));
        runStopButton->setPalette(palette);
        currentGlWidget->update();
        disableButtons();
    }
    currentGlWidget->runPort(m_serialPortComboBox->currentText(),
                             m_serialPortBaudRateComboBox->currentText().toInt());



}

void Window::chooseChartChange()
{
    switch(m_chooseChart->currentIndex())
    {
        case 0:
        {
            currentGlWidget->rot_3d();
            currentGlWidget->chart_mod = 1;
            xBtn->setEnabled(true);
            yBtn->setEnabled(true);
            zBtn->setEnabled(true);
            break;
        }
        case 1:
        {
            currentGlWidget->rot_2d();
            currentGlWidget->chart_mod = 2;
            xBtn->setEnabled(false);
            yBtn->setEnabled(false);
            zBtn->setEnabled(false);
            break;
        }
        case 2:
        {
            currentGlWidget->rot_2d();
            currentGlWidget->chart_mod = 3;
            xBtn->setEnabled(false);
            yBtn->setEnabled(false);
            zBtn->setEnabled(false);
            break;
        }
        case 3:
        {
            currentGlWidget->rot_2d();
            currentGlWidget->chart_mod = 4;
            xBtn->setEnabled(false);
            yBtn->setEnabled(false);
            zBtn->setEnabled(false);
            break;
        }
        case 4:
        {
            currentGlWidget->rot_2d();
            currentGlWidget->chart_mod = 5;
            xBtn->setEnabled(false);
            yBtn->setEnabled(false);
            zBtn->setEnabled(false);
            break;
        }
    }
}

void Window::chooseFilterChange()
{
    switch(m_chooseFilter->currentIndex())
    {
        case 0:
        {
            currentGlWidget->selectFilter(1);
            break;
        }
        case 1:
        {
            currentGlWidget->selectFilter(2);
            break;
        }
    }
}

void Window::selectSens()
{
    float buf = accComboBox->currentText().toFloat();
    currentGlWidget->selectSens(buf);
}

void Window::saveFile()
{
    currentGlWidget->hide();
    currentGlWidget->saveLog();
    currentGlWidget->show();
    currentGlWidget->update();
}

void Window::readFromFile()
{
    currentGlWidget->hide();
    currentGlWidget->readFromFile();
    currentGlWidget->show();
    currentGlWidget->update();
}

void Window::enableButtons()
{
    readLog->setEnabled(true);//чтение лога
    m_serialPortComboBox->setEnabled(true);
    m_serialPortBaudRateComboBox->setEnabled(true);
    saveLogButton->setEnabled(true);//кнопка сохранения лога
}

void Window::disableButtons()
{
    readLog->setEnabled(false);//чтение лога
    m_serialPortComboBox->setEnabled(false);
    m_serialPortBaudRateComboBox->setEnabled(false);
    saveLogButton->setEnabled(false);//кнопка сохранения лога
}
