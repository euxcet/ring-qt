#ifndef RING_H
#define RING_H
#include <QObject>
#include <QVariant>
#include <QList>
#include <QTimer>

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
#include <QtBluetooth/QLowEnergyAdvertisingData>
#include <QtBluetooth/QLowEnergyAdvertisingParameters>
#include "deviceinterface.h"
#include "tcpserver.h"

class BLE;

class Ring : public QObject
{
    Q_OBJECT
public:
    Ring(const QBluetoothDeviceInfo &info, TcpServer *server, DeviceInterface *interface);
    void connectDevice(void);
    void disconnectDevice(void);

    QBluetoothDeviceInfo info;

private slots:
    void dataFromClient(QByteArray data);

    void handleServiceDiscovered(const QBluetoothUuid &uuid);
    void handleDeviceConnected(void);
    void handleDeviceDisconnected(void);
    void handleDiscoveryFinished(void);
    void handleError(QLowEnergyController::Error error);

    void serviceDetailsDiscovered(QLowEnergyService::ServiceState newState);

    void handleCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void handleCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void handleCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void handleDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);

    void handleBatteryTimeout(void);

private:
    DeviceInterface *interface;
    QTimer *batteryTimer;
    void releaseResources(void);
    void connectService(QString uuid);
    bool isConnected = false;
    QLowEnergyCharacteristic writeChannel;

    TcpServer *server;
    QLowEnergyController *m_LowController = nullptr;
    QLowEnergyService *service;
};

#endif // RING_H
