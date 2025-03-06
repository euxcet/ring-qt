#include <ble.h>
#include <QThread>
#include <QtBluetooth/QLowEnergyAdvertisingData>
#include <QtBluetooth/QLowEnergyAdvertisingParameters>

BLE::BLE(int port, QObject *parent)
    : QObject(parent), port(port) {
    reconnecting = true;
    socket = new QTcpSocket();
    socket -> abort();
    socket -> connectToHost("127.0.0.1", port);
    socket -> waitForConnected();

    m_DiscoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    m_DiscoveryAgent->setLowEnergyDiscoveryTimeout(50);

    connect(m_DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BLE::handleDeviceDiscovered);
    connect(m_DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BLE::handleScanFinished);

}

void BLE::releaseResources() {
    if (m_DiscoveryAgent && m_DiscoveryAgent->isActive()) {
        m_DiscoveryAgent->stop();
    }
    delete m_DiscoveryAgent;
}

BLE::~BLE() {
    releaseResources();
    if (socket) {
        socket->close();
        delete socket;
    }
}


void BLE::handleDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration){
        qDebug()<< "device discovered :" <<info.name()<<info.address().toString();
        if (localDevice.isValid()) {
            QList<QBluetoothAddress> pairedDevices = localDevice.connectedDevices();
            for (const QBluetoothAddress &address: pairedDevices) {
                if (address == info.address() && info.name().startsWith("BCL")) {
                    int valid = true;
                    for (Ring *visRing: rings) {
                        if (visRing->info.address().toString() == info.address().toString()) {
                            valid = false;
                        }
                    }
                    if (valid) {
                        Ring *ring = new Ring(info, socket, this);
                        ring->connectDevice();
                        rings.append(ring);
                    }
                }
            }
        }
    }

}

void BLE::onConnected(void) {
    reconnecting = false;
}

void BLE::onDisconnected(void) {
    reconnecting = true;
    for (Ring* ring: rings) {
        ring->disconnectDevice();
        delete ring;
    }
    rings.clear();
    startScanDevices();
}

void BLE::handleBluetoothError(void) {
    QBluetoothDeviceDiscoveryAgent::Error error = m_DiscoveryAgent->error();
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError){
        qDebug() << "蓝牙已关闭。";
    } else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError) {
        qDebug() << "设备读写错误。";
    } else {
        qDebug() << "其他蓝牙错误。";
    }
    releaseResources();
    startScanDevices();
}


void BLE::handleScanFinished(void) {
    qDebug() << "Scan finished! " << reconnecting;
    isScanning = false;

    QTime _Timer = QTime::currentTime().addMSecs(2000);
    while (QTime::currentTime() < _Timer) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    if (reconnecting) {
        startScanDevices();
    }
}

void BLE::handleControllerError(QLowEnergyController::Error /* error */) {
    releaseResources();
    startScanDevices();
}

void BLE::startScanDevices() {
    m_DiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    if (m_DiscoveryAgent->isActive()){
        qDebug("Scanning.\n");
        isScanning = true;
    }
}
