#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGroupBox>
#include<QTextEdit>
#include <QLabel>
#include <QMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QtNetwork>
#include <QBluetoothLocalDevice>

#include "ble.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int port, QWidget *parent = nullptr);
    ~MainWindow();



private slots:
    void on_pushButton_scan_clicked();
    Q_SLOT void handleScanFinished();
    Q_SLOT void handleFindServicesFinished();
    Q_SLOT void handleFindCharsFinished();

    void on_pushButton_connect_clicked();

    void on_pushButton_service_clicked();

    void on_pushButton_disconnect_clicked();



private:
    Ui::MainWindow *ui;

    BLE * m_ble;
    QThread *thread;
    QTcpServer *server;
    QTcpSocket *client;
};
#endif // MAINWINDOW_H
