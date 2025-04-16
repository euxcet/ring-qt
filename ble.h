#ifndef BLE_H
#define BLE_H

#include <QObject>
#include <QTime>
#include <QCoreApplication>
#include <QVariant>
#include <QList>


#include <QLowEnergyController>
#include <QThread>
#include <QTcpSocket>
#include <QNetworkInterface>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothUuid>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyDescriptor>
#include <QLowEnergyService>
#include <QLowEnergyCharacteristic>
#include <QBluetoothLocalDevice>
#include "ring.h"
#include "tcpserver.h"
#include "deviceinterface.h"

class BLE : public QObject, DeviceInterface
{
    Q_OBJECT
public:
    explicit BLE(int port, QObject *parent = nullptr);
    // explicit BLE();
    ~BLE();

    void startScanDevices();
    void onConnected(void);
    void onDisconnected(void);
    void connectService(QString uuid);
    bool getScanning() { return isScanning; }
    QList<QString> getMac();

private slots:
    void handleDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void handleControllerError(QLowEnergyController::Error error);
    void handleBluetoothError(void);
    void handleScanFinished(void);


signals:
    void signal_scanFinished();

    void signal_findservicesFinished();

    void signal_findcharsFinished();

private:
    BLE *ble;
    void releaseResources();
    void handleDevice(QBluetoothDeviceInfo deviceInfo);

    int port;
    TcpServer server;
    // QTcpSocket *socket;

    QBluetoothDeviceDiscoveryAgent * m_DiscoveryAgent;

    QList<Ring*> rings;
    QBluetoothLocalDevice localDevice;
    bool isScanning = false;
    bool reconnecting = false;
};






#endif // BLE_H


