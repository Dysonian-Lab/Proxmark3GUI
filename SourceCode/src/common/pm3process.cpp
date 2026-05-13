#include "pm3process.h"

PM3Process::PM3Process(QThread* thread, QObject* parent): QProcess(parent)
{
    moveToThread(thread);
    setProcessChannelMode(PM3Process::MergedChannels);
    isRequiringOutput = false;
    requiredOutput = new QString();
    serialListener = new QTimer(); // if using new QTimer(this), the debug output will show "Cannot create children for a parent that is in a different thread."
    serialListener->moveToThread(this->thread());// I've tried many ways to creat a QTimer instance, but all of the instances are in the main thread(UI thread), so I have to move it manually
    serialListener->setInterval(1000);
    serialListener->setTimerType(Qt::VeryCoarseTimer);
    connect(serialListener, &QTimer::timeout, this, &PM3Process::onTimeout);
    connect(this, &PM3Process::readyRead, this, &PM3Process::onReadyRead);
    portInfo = nullptr;

    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
}

void PM3Process::connectPM3(const QString& path, const QStringList args)
{
    QString result;
    Util::ClientType clientType;
    setRequiringOutput(true);

    // stash for reconnect
    currPath = path;
    currArgs = args;

    // using "-f" option to make the client output flushed after every print.
    // single '\r' might appear. Don't use QProcess::Text there or '\r' is ignored.
    start(path, args, QProcess::Unbuffered | QProcess::ReadWrite);
    if(waitForStarted(10000))
    {
        waitForReadyRead(10000);
        setRequiringOutput(false);
        result = *requiredOutput;
        if(result.contains("[=]"))
        {
            clientType = Util::CLIENTTYPE_ICEMAN;
            setRequiringOutput(true);
            write("hw version\n");
            for(int i = 0; i < 50; i++)
            {
                waitForReadyRead(200);
                result += *requiredOutput;
                QString upperResult = result.toUpper();
                if(upperResult.contains("OS"))
                    break;
            }
            setRequiringOutput(false);
        }
        else
        {
            clientType = Util::CLIENTTYPE_OFFICIAL;
        }
        QString upperResult = result.toUpper();
        if(upperResult.contains("OS")) // make sure the PM3 is connected
        {
            emit changeClientType(clientType);
            // Find the specific line containing version info
            int osIdx = upperResult.indexOf("OS");
            QString osLine = result.mid(osIdx);
            osLine = osLine.left(osLine.indexOf("\n"));
            if (osLine.startsWith("os: ")) {
                // Old format: "os: Iceman/v4.21611 ..."
                result = osLine.mid(4, osLine.indexOf(" ", 4) - 4);
            } else {
                // New v4.21611 format: "OS......... Iceman/master/v4.21611-suspect ..."
                // Skip "OS" + dots + space, then take version until next space
                int firstSpace = osLine.indexOf(' ');
                if (firstSpace >= 0) {
                    result = osLine.mid(firstSpace + 1);
                    result = result.left(result.indexOf(' '));
                }
            }
            emit PM3StatedChanged(true, result);
        }
        else
        {
            qDebug() << "unexpected output:" << (result.isEmpty() ? "(empty)" : result);
            emit HWConnectFailed();
            kill();
        }
    }

    setRequiringOutput(false);
}

void PM3Process::reconnectPM3()
{
    connectPM3(currPath, currArgs);
}

void PM3Process::setRequiringOutput(bool st)
{
    isRequiringOutput = st;
    if(isRequiringOutput)
        requiredOutput->clear();
}

bool PM3Process::waitForReadyRead(int msecs)
{
    return QProcess::waitForReadyRead(msecs);
}

void PM3Process::setSerialListener(const QString& name, bool state)
{
    if(state)
    {
        currPort = name;
        portInfo = new QSerialPortInfo(name);
        serialListener->start();
        qDebug() << serialListener->thread();
    }
    else
    {
        serialListener->stop();
        if(portInfo != nullptr)
        {
            delete portInfo;
            portInfo = nullptr;
        }
    }
}

void PM3Process::setSerialListener(bool state)
{
    setSerialListener(currPort, state);
}

void PM3Process::onTimeout() //when the proxmark3 client is unexpectedly terminated or the PM3 hardware is removed, the isBusy() will return false(only tested on Windows);
{
//    isBusy() is a deprecated function because it will block the serial port when the port is not in use.
//    However, the PM3 client is supposed to use the target serial port exclusively, so it should be fine
//    isBusy() will always return false on Raspbian, in this case, check "Keep the client active" in the Settings panel.
//
//    qDebug()<<portInfo->isBusy();
    if(!portInfo->isBusy())
    {
        killPM3();
    }
}

void PM3Process::testThread()
{
    qDebug() << "PM3:" << QThread::currentThread();
}

qint64 PM3Process::write(QString data)
{
    return QProcess::write(data.toLatin1());
}

void PM3Process::onReadyRead()
{
    QString out = readAll();
    if(isRequiringOutput)
        requiredOutput->append(out);
    if(out != "")
    {
//        qDebug() << "PM3Process::onReadyRead:" << out;
        emit newOutput(out);

    }
}

void PM3Process::setProcEnv(const QStringList* env)
{
//    qDebug() << "passed Env List" << *env;
    this->setEnvironment(*env);
    //    qDebug() << "final Env List" << processEnvironment().toStringList();
}

void PM3Process::setWorkingDir(const QString& dir)
{
    // the working directory cannot be the default, or the client will failed to load the dll
    this->setWorkingDirectory(dir);
}

void PM3Process::killPM3()
{
    kill();
    emit PM3StatedChanged(false);
    setSerialListener(false);
}
