#include "mainwindow.h"
#include "ui_mainwindow.h"


//using namespace QtCharts;

MainWindow::MainWindow(int port, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_ble = new BLE(port, this);
    connect(m_ble, &BLE::signal_scanFinished, this, &MainWindow::handleScanFinished);
    if(!m_ble->getScanning())
        m_ble->startScanDevices();
}

MainWindow::~MainWindow()
{
    delete m_ble;
    delete ui;
}

void MainWindow::handleScanFinished()
{
}

void MainWindow::on_pushButton_scan_clicked()
{
}


void MainWindow::handleFindServicesFinished()
{
}

void MainWindow::on_pushButton_connect_clicked()
{
}

void MainWindow::handleFindCharsFinished()
{
}
void MainWindow::on_pushButton_service_clicked()
{
}


void MainWindow::on_pushButton_disconnect_clicked()
{
}



