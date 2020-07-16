#include "masterthread.h"
#include <QMessageBox>
#include <iostream>

MasterThread::MasterThread(QObject *parent) :
    QThread(parent)
{
}

MasterThread::~MasterThread()
{
    m_quit = true;
    stop();
    quit();
}

void MasterThread::runPort(const QString &portName, const int &serialPortBaudRate)
{
    m_portName = portName;
    m_serialPortBaudRate = serialPortBaudRate;
    m_quit = false;
    start();
}

void MasterThread::run()
{
    //QMessageBox::warning(0,"Warning", "Warning message text");
    bool currentPortNameChanged = false;
    QString currentPortName;
    if (currentPortName != m_portName)
    {
        currentPortName = m_portName;
        currentPortNameChanged = true;
    }
    QSerialPort serial;
    if (currentPortName.isEmpty())
    {
        QMessageBox::warning(0,"Warning", tr("No port name specified"));
        emit error(tr("No port name specified"));        
        return;
    }
    while (!m_quit)
    {
        //открыли порт и установили скорость
        if (currentPortNameChanged)
        {
            serial.close();
            serial.setPortName(currentPortName);

            if (!serial.open(QIODevice::ReadWrite))
            {
                std::cout << "erroe" <<  std::endl;
                QMessageBox::warning(0,"Warning", tr("Can't open %1, error code %2")
                                     .arg(m_portName).arg(serial.error()));
                emit error(tr("Can't open %1, error code %2")
                           .arg(m_portName).arg(serial.error()));
                return;
            }
            serial.setBaudRate(m_serialPortBaudRate);
        }
        //read line
        while((waitLine(&serial,-1)) && (!m_quit))
        {
           QString response= serial.readLine().trimmed();
           emit this->response(response);
        }
        //m_quit = false;
    }
}

void MasterThread::stop()
{
    m_quit = true;
}

bool MasterThread::waitLine(QSerialPort *device, int msecs)
{
    msecs = -1;
  if(!device->canReadLine())
  {
    while(device->waitForReadyRead(msecs) &&   !device->canReadLine() );
  }
  return device->canReadLine();
}

