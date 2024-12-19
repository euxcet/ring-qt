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

// void MainWindow::acceptConnection() {
// client = server -> nextPendingConnection();
// connect(client, SIGNAL(readyRead()), this,)
// }

MainWindow::~MainWindow()
{
    delete m_ble;
    delete ui;
}

void MainWindow::handleScanFinished()
{
    auto list = m_ble->getDevices();
    if (list.isEmpty())
        return;

    ui->listWidget_dev->clear();
    for (const auto& device : list) {
        QString itemText = QString("%1 %2 rssi:%3 ")
        .arg(device.name())
            .arg(device.address().toString())
            .arg(device.rssi());

        ui->listWidget_dev->addItem(itemText);
    }

    // 连接m_ble的signal_findservicesFinished信号到handleFindServicesFinished槽函数
    connect(m_ble, &BLE::signal_findservicesFinished, this, &MainWindow::handleFindServicesFinished);
    connect(m_ble, &BLE::signal_findcharsFinished, this, &MainWindow::handleFindCharsFinished);

    QBluetoothLocalDevice localDevice;
    if (localDevice.isValid()) {
        QList<QBluetoothAddress> pairedDevices = localDevice.connectedDevices();
        for (const QBluetoothAddress &address: pairedDevices) {
            qDebug() << "Paired " << address.toString();
        }
        for (const auto& device : list) {
            bool vis = false;
            for (const QBluetoothAddress &address: pairedDevices) {
                if (address == device.address()) {
                    vis = true;
                }
            }
            if (vis && device.name().startsWith("BCL")) {
                m_ble->connectDevice(device.address().toString());
                break;
            }
        }
    }

}

void MainWindow::on_pushButton_scan_clicked()
{
    connect(m_ble, &BLE::signal_scanFinished, this, &MainWindow::handleScanFinished);
    if(!m_ble->getScanning())
        m_ble->startScanDevices();
}


void MainWindow::handleFindServicesFinished()
{
    auto list = m_ble->getServicesUUID();
    if (list.isEmpty())
    {
        return;
    }

    ui->listWidget_services->clear();
    for (const auto& service : list) {
        ui->listWidget_services->addItem(service.toString());
    }
    m_ble->connectService("{bae80001-4f05-4503-8e65-3af1f7329d1f}");
}

void MainWindow::on_pushButton_connect_clicked()
{
    QRegularExpression macRegex("([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})");
    // 连接m_ble的signal_findservicesFinished信号到handleFindServicesFinished槽函数
    connect(m_ble, &BLE::signal_findservicesFinished, this, &MainWindow::handleFindServicesFinished);
    connect(m_ble, &BLE::signal_findcharsFinished, this, &MainWindow::handleFindCharsFinished);

    //m_ble->connectDevice(ui->listWidget_dev->currentItem()->text());
    // 提取并连接MAC地址
    QListWidgetItem* currentItem = ui->listWidget_dev->currentItem();
    if (currentItem) {
        QString itemText = currentItem->text();                     // 获取当前项的文本

        QRegularExpressionMatch match = macRegex.match(itemText);   // 使用正则表达式匹配MAC地址
        if (match.hasMatch()) {
            QString macAddress = match.captured(0);                  // 提取MAC地址
            qDebug() << "Extracted MAC Address:" << macAddress;

            m_ble->connectDevice(macAddress);
        }
    }
}

void MainWindow::handleFindCharsFinished()
{
    auto list = m_ble->getChars();
    if (list.isEmpty())
        return;

    ui->listWidget_character->clear();
    for (const auto& characteristic : list) {
        ui->listWidget_character->addItem(characteristic.uuid().toString());
    }
}
void MainWindow::on_pushButton_service_clicked()
{
    // connect(m_ble, &BLE::signal_findcharsFinished, this, &MainWindow::handleFindCharsFinished);
    // m_ble->connectService("{bae80001-4f05-4503-8e65-3af1f7329d1f}");
}


void MainWindow::on_pushButton_disconnect_clicked()
{
    m_ble->disconnectDevice();
}



