#ifndef MASTERTHREAD_H
#define MASTERTHREAD_H

#include <QMutex>
#include <QThread>
#include <QSerialPort>

class MasterThread : public QThread
{
    Q_OBJECT

public:
    explicit MasterThread(QObject *parent = nullptr);
    ~MasterThread();
    void runPort(const QString &portName, const int &serialPortBaudRate);
    void stop();

signals:
    void response(const QString &s);
    void error(const QString &s);

private:
    void run() override;
    QString m_portName;
    int m_serialPortBaudRate;
    bool m_quit = false;
    bool waitLine(QSerialPort *device, int msecs);
};


#endif // MASTERTHREAD_H
