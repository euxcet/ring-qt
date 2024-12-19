#include "mainwindow.h"

#include <QApplication>
#include <QObject>
#include <QThread>
#include <Windows.h>
#include <TlHelp32.h>
#include <signal.h>

DWORD GetParentProcessID() {
    DWORD pid = GetCurrentProcessId();
    DWORD ppid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == pid) {
                    ppid = pe32.th32ParentProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return ppid;
}

class ParentProcessWatcher: public QThread {
public:
    ParentProcessWatcher(HANDLE hProcess): m_hProcess(hProcess) {}
protected:
    void run() override {
        if (m_hProcess != nullptr) {
            WaitForSingleObject(m_hProcess, INFINITE);
            QCoreApplication::quit();
        }
    }
private:
     HANDLE m_hProcess;
};

/**
 * @brief qDebug()函数重定向
 * @param type 消息类型
 * @param context
 * @param msg 消息
 */
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QDateTime current_time = QDateTime::currentDateTime();
    QString timeDD = current_time.toString("yyyy-MM-dd");
    QString timeSS = current_time.toString("hh:mm:ss");
    QString filePrefix = current_time.toString("yyyy-MM-dd_hh");
    QString typeStr = "unknownTypeMsg~";

    switch (type) {
    case QtDebugMsg:
        typeStr="D";break;
    case QtInfoMsg:
        typeStr="I";break;
    case QtWarningMsg:
        typeStr="W";break;
    case QtCriticalMsg:
        typeStr="C";break;
    case QtFatalMsg:
        typeStr="F";
        abort();
    }
    QFile file("./log/"+filePrefix+".txt");
    bool result = file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream out(&file);
    out <<timeSS<<"  "<<typeStr << "  "<< msg << "\n";


    // 将消息写入到应用程序输出中
    QStringList pathList = QString(context.file).split("\\");
    int pathi = pathList.count();
    QString curFileName;
    if(pathi >= 3){
        curFileName = pathList.at(2);
    }else{
        curFileName = pathList.at(0);
    }
    QTextStream console(stdout);
    console << QString("[%1:%2]: ").arg(curFileName).arg(context.line) << msg << "\n";
}

void handleSignal(int signal) {
    if (signal == SIGTERM || signal == SIGINT) {
        qDebug() << "Exit";
        QCoreApplication::quit();
    }
}

int main(int argc, char *argv[])
{
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] %{message}");
    QApplication a(argc, argv);
    QObject::connect(&a, &QApplication::aboutToQuit, [&](){
        qDebug() << "exit";
    });
    signal(SIGTERM, handleSignal);
    signal(SIGINT, handleSignal);
    int port = 5566;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    /*QFileInfo fileInfo("./log/log.txt");
    QDir dir("./log");
    if (!dir.exists()) {
        dir.mkdir(dir.absolutePath());
    }

    QTextStream t(stdout);
    t << "xxxxxxxxxxx" << fileInfo.absoluteFilePath() << "\n";

    qInstallMessageHandler(myMessageOutput); // 安装消息处理程序*/
    MainWindow w(port);
    // w.show();
    DWORD parentPID = GetParentProcessID();
    HANDLE hParentProcess = OpenProcess(SYNCHRONIZE, FALSE, parentPID);
    if (hParentProcess == NULL) {
        QCoreApplication::quit();
        return 0;
    }

    ParentProcessWatcher *watcher = new ParentProcessWatcher(hParentProcess);
    watcher->start();
    int ret = a.exec();
    qDebug() << ret << " ret";
    if (watcher->isRunning()) {
        watcher->terminate();
        watcher->wait();
    }
    CloseHandle(hParentProcess);
    delete watcher;
    return ret;
}
