#ifndef BLE_H
#define BLE_H

#include <QObject>
#include <QVariant>
#include <QList>


#include <QLowEnergyController>
#include <QThread>
#include <QTcpSocket>
#include <QNetworkInterface>

#include <QBluetoothDeviceDiscoveryAgent> //发现设备
#include <QBluetoothUuid>                 //蓝牙uuid
#include <QBluetoothDeviceInfo>           //设备信息
#include <QLowEnergyController>           //ble controller
#include <QLowEnergyDescriptor>           //ble 描述符
#include <QLowEnergyService>              //ble 服务
#include <QLowEnergyCharacteristic>       //ble特性
#include <QBluetoothLocalDevice>

// class RingThread : public QThread {
//     Q_OBJECT
// public:
//     explicit RingThread(QLowEnergyService* service, QLowEnergyCharacteristic ch) : service(service), ch(ch) {}

//     void run() {
//         qDebug() << "Thread start";
//         socket = new QTcpSocket();
//         socket -> abort();
//         socket -> connectToHost("127.0.0.1", 5566);
//         socket -> waitForConnected();
//         qDebug() << socket -> state();

//         connect(service, &QLowEnergyService::characteristicChanged, this, &RingThread::handleCharacteristicChanged);
//         while (true) {
//             QThread::sleep(1);
//         }
//     }
//     void stop() {

//     }


// private slots:
//     void handleCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value) {
//         QString Charuuid = c.uuid().toString();
//         QString Value = value.toHex();
//         qDebug() << "BLE设备-" + Charuuid + "特性值发生变化：" + Value;
//     }

// private:
//     QTcpSocket *socket;
//     QLowEnergyService *service;
//     QLowEnergyCharacteristic ch;
// };

class BLE : public QObject
{
    Q_OBJECT
public:
    explicit BLE(int port, QObject *parent = nullptr);
    // explicit BLE();
    ~BLE();

    void init(void);

    //开始扫描设备
    void startScanDevices();
    //连接设备
    void connectDevice(QString address);
    //连接服务
    void connectService(QString uuid);
    void disconnectDevice(void);

    bool getScanning(){
        return isScanning;
    }

    QList<QBluetoothDeviceInfo> getDevices(){
        return bleDevicesList;
    }

    QList<QBluetoothUuid> getServicesUUID(){
        return servicesUUIDList;
    }

    QList<QLowEnergyCharacteristic> getChars(){
        return characterList;
    }

    QList<QString> getMac();

public slots:
    void doWork();

private slots:
    // QLowEnergyService related
    void serviceDetailsDiscovered(QLowEnergyService::ServiceState newState);
    void handleDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void handleControllerError(QLowEnergyController::Error error);
    void handleServiceDiscovered(const QBluetoothUuid &uuid);
    void handleBluetoothError(void);
    void handleScanFinished(void);
    void handleDeviceConnected(void);
    void handleDeviceDisconnected(void);
    void handleDiscoveryFinished(void);
    void handleError(QLowEnergyController::Error error);

    // 声明特征值改变处理函数
    void handleCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    // 声明特征读取成功处理函数
    void handleCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    // 声明特性写入成功处理函数
    void handleCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value);
    // 声明描述符写入成功处理函数
    void handleDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    Q_SLOT void handleServiceError(QLowEnergyService::ServiceError newError);

signals:
    void signal_scanFinished();

    void signal_findservicesFinished();

    void signal_findcharsFinished();

private:
    int port;
    QTcpSocket *socket;
    // RingThread *thread;
    QBluetoothDeviceDiscoveryAgent * m_DiscoveryAgent;  //设备发现对象

    bool isScanning = false;

    QLowEnergyController *          m_LowController = nullptr;    //中心控制器

    QList<QBluetoothDeviceInfo >    bleDevicesList;

    QBluetoothDeviceInfo            nowDevice;

    QString                         previousAddress;        //之前的设备

    QList<QLowEnergyService>        servicesList;

    QList<QLowEnergyCharacteristic> characterList;

    QList<QBluetoothUuid>           servicesUUIDList;

    QLowEnergyCharacteristic writeChannel;

    QLowEnergyService* service;

    void releaseResources();
    bool isConnected = false;
    void handleDevice(QBluetoothDeviceInfo deviceInfo);
    QBluetoothLocalDevice localDevice;
    int RINGTYPE_YX = 0;
    int RINGTYPE_DC = 1;
    int ringType = RINGTYPE_DC;

};






#endif // BLE_H


