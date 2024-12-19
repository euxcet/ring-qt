#include <ble.h>
#include <QThread>
#include <QtBluetooth/QLowEnergyAdvertisingData>
#include <QtBluetooth/QLowEnergyAdvertisingParameters>

// BLE::BLE()
// : QObject()
BLE::BLE(int port, QObject *parent)
    : QObject(parent), port(port)
{
    init();
}

void BLE::releaseResources() {
    if (!isConnected) return;

    if (isConnected) isConnected = false;
    //if (service) service->disconnect();
    if (m_LowController) m_LowController->disconnectFromDevice();
    if (m_DiscoveryAgent && m_DiscoveryAgent->isActive()) m_DiscoveryAgent->stop();
    //delete service;
    delete m_LowController;
    m_LowController = nullptr;
    delete m_DiscoveryAgent;
    servicesList.clear();
    characterList.clear();
    bleDevicesList.clear();
}

BLE::~BLE()
{
    releaseResources();
    if (socket) {
        socket->close();
        delete socket;
    }
}

QList<QString> BLE::getMac() {
    QList<QString> macs;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface interface, interfaces) {
        if (interface.isValid()) {
            macs.append(interface.hardwareAddress().toUpper());
        }
    }
    return macs;
}

void BLE::handleDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration){
        qDebug()<< "device discovered :" <<info.name()<<info.address().toString();
        ////        // 获取设备的广播包数据
        //        const auto advertisementData = info.serviceData();
        //        // 打印广播包数据
        //        for (const auto &key : advertisementData.keys()) {
        //            qDebug() <<"llllll"<< key.toString() << ":" << advertisementData.value(key).toHex();
        //        }
        bleDevicesList.append(info);
        handleDevice(info);
    }

}

void BLE::handleDevice(QBluetoothDeviceInfo deviceInfo) {
    if (isConnected) return;
    qDebug() << "handle " << deviceInfo.name() << "," << deviceInfo.address();
    if (ringType == RINGTYPE_YX) {
        if (localDevice.isValid()) {
            QList<QBluetoothAddress> pairedDevices = localDevice.connectedDevices();
            for (const QBluetoothAddress &address: pairedDevices) {
                qDebug() << "Paired " << address.toString();
            }
            for (const QBluetoothAddress &address: pairedDevices) {
                if (address == deviceInfo.address()) {
                    if (deviceInfo.name().startsWith("BCL") && !isConnected) {
                        connectDevice(deviceInfo.address().toString());
                        break;
                    }
                }
            }
        }
    } else {
        if (deviceInfo.name().startsWith("Lenovo") && !isConnected) {
            connectDevice(deviceInfo.address().toString());
        }
    }
}

void BLE::handleBluetoothError(void)
{
    QBluetoothDeviceDiscoveryAgent::Error error = m_DiscoveryAgent->error();
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError){
        qDebug() << "蓝牙已关闭。";
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError) {
        qDebug() << "设备读写错误。";
    } else {
        qDebug() << "其他蓝牙错误。";
    }
    releaseResources();
    // isConnected = false;
    startScanDevices();
}


void BLE::handleScanFinished(void) {
    if (bleDevicesList.isEmpty())
        qDebug("No Low Energy devices found...");
    else
        qDebug("Scan finished!");

    //emit signal_scanFinished();
    isScanning = false;
    if (!isConnected) {
        //QThread::msleep(2000);
        startScanDevices();
    }
}

void BLE::handleDeviceConnected(void) {
    qDebug("Device connect success.");
    //isConnected = true;
    isConnected = true;
    m_LowController->discoverServices();
}

void BLE::handleControllerError(QLowEnergyController::Error /* error */) {
    qDebug() << "Error: " << m_LowController->errorString();
    isConnected = false;
    releaseResources();
    startScanDevices();
}

void BLE::handleDeviceDisconnected(void) {
    qDebug("Device disconnected.");
    if (socket -> state() == QAbstractSocket::ConnectedState) {
        socket -> write("Disconnected");
        socket->flush();
    }
    releaseResources();
    startScanDevices();
}

// 定义服务发现处理函数
void BLE::handleServiceDiscovered(const QBluetoothUuid &uuid) {
    qDebug() << "Service discovered:" << uuid.toString();
    // 执行其他服务发现处理逻辑
    // m_LowController->discoverServices();
}

// 定义服务发现处理函数
void BLE::handleDiscoveryFinished(void) {
    qDebug() << "Services scan finished.";

    servicesUUIDList = m_LowController->services();
    for (const auto& s : servicesUUIDList) {
        qDebug() << s.toString();
    }
    if (ringType == RINGTYPE_YX) {
        connectService("{bae80001-4f05-4503-8e65-3af1f7329d1f}");
    } else if (ringType == RINGTYPE_DC) {
        while (true) {
            try {
                connectService("{c1d02500-2d20-400a-95d2-6a2f7bca0c25}");
                break;
            } catch (...) {
                qDebug("error");
                QThread::msleep(500);
            }
        }
        while (true) {
            try {
                connectService("{a6ed0201-d344-460a-8075-b9e8ec90d71b}");
                break;
            } catch (...) {
                qDebug("error");
                QThread::msleep(500);
            }
        }
    }
    //emit signal_findservicesFinished();
}


void BLE::init(void)
{


    socket = new QTcpSocket();
    socket -> abort();
    socket -> connectToHost("127.0.0.1", port);
    socket -> waitForConnected();

}

//开始扫描设备
void BLE::startScanDevices()
{
    // QThread::msleep(500);
    if (isConnected) return;
    //清空设备列表
    bleDevicesList.clear();

    m_DiscoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    m_DiscoveryAgent->setLowEnergyDiscoveryTimeout(50);
    //    m_DiscoveryAgent->setInquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry);

    connect(m_DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BLE::handleDeviceDiscovered);
    connect(m_DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::error, this, &BLE::handleBluetoothError);
    connect(m_DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BLE::handleScanFinished);

    //开始扫描
    m_DiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    if(m_DiscoveryAgent->isActive()){
        qDebug("Scanning.\n");
        isScanning = true;
    }
}

void BLE::disconnectDevice(void)
{
    m_LowController->disconnectFromDevice();
    delete m_LowController;
    m_LowController = nullptr;
}
//连接设备
void BLE::connectDevice(QString address)
{
    for(auto dev : bleDevicesList){
        if(dev.address().toString() == address){
            nowDevice = dev;
        }
    }
    if(!nowDevice.isValid()){
        qDebug("Not a valid device");
        return;
    }

    servicesList.clear();
    characterList.clear();

    if (m_LowController && previousAddress != nowDevice.address().toString()) {
        m_LowController->disconnectFromDevice();
        delete m_LowController;
        m_LowController = nullptr;
    }

    // 通过 socket 发送 mac address
    /*
    qDebug("mac address");
    qDebug(address.toLocal8Bit());*/
    socket->write(address.toLocal8Bit());

    if (!m_LowController) {
        m_LowController = QLowEnergyController::createCentral(nowDevice);
        QLowEnergyAdvertisingParameters adv;
        adv.setInterval(20, 20);
        QLowEnergyAdvertisingData data;
        m_LowController->startAdvertising(adv, data);
        //m_LowController.

        connect(m_LowController, &QLowEnergyController::connected, this, &BLE::handleDeviceConnected);

        connect(m_LowController, &QLowEnergyController::errorOccurred, this, &BLE::handleError);

        connect(m_LowController, &QLowEnergyController::disconnected, this, &BLE::handleDeviceDisconnected);

        connect(m_LowController, &QLowEnergyController::serviceDiscovered, this, &BLE::handleServiceDiscovered);

        connect(m_LowController, &QLowEnergyController::discoveryFinished, this, &BLE::handleDiscoveryFinished);
    }
    qDebug() << (" connect address:" + address);
    m_LowController->connectToDevice();
    previousAddress = nowDevice.address().toString();
}
void BLE::handleError(QLowEnergyController::Error error) {
    qDebug("error occured");
    qDebug() << error;
    //isConnected = false;
    releaseResources();
    startScanDevices();
}

void BLE::connectService(QString uuid)
{
    QLowEnergyService *service = nullptr;

    for(int i=0;i<servicesUUIDList.length();i++)
    {
        if(servicesUUIDList.at(i).toString() == uuid){
            service = m_LowController->createServiceObject(servicesUUIDList.at(i));
        }
    }

    if (!service)
        return;
    characterList.clear();
    qDebug() << "out dicxover details ";

    if (service->state() == QLowEnergyService::RemoteService) {
        connect(service, &QLowEnergyService::stateChanged,this, &BLE::serviceDetailsDiscovered);
        qDebug() << "before dicxover details ";
        service->discoverDetails();
        qDebug() << "after dicxover details ";
        return;
    }

}


// 定义特征值改变处理函数
void BLE::handleCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value) {
    QString Charuuid = c.uuid().toString();
    QString Value = value.toHex().toUpper();
    QList<QString> macs = getMac();
    if (ringType == RINGTYPE_YX) {
        if (int(value.at(2)) == -110 && int(value.at(3)) == 0) {
            qDebug() << "value changed, size: "  << Value.size();
            for (int i = 10; i < Value.size(); i += 14) {
                QString remote = Value.mid(i + 2, 12);
                foreach (QString mac, macs) {
                    if (mac.size() == 17 && remote[0] == mac[15] && remote[1] == mac[16] && remote[2] == mac[12] && remote[3] == mac[13] &&
                        remote[4] == mac[9] && remote[5] == mac[10] && remote[6] == mac[6] && remote[7] == mac[7] &&
                        remote[8] == mac[3] && remote[9] == mac[4] && remote[10] == mac[0] && remote[11] == mac[1]) {
                        QByteArray msg = QByteArray::fromHex("00009202");
                        msg += value.mid((i + 2) / 2, 6);
                        service->writeCharacteristic(writeChannel, msg);
                        break;
                    }

                }
            }
        }
        if (int(value.at(2)) == -110 && int(value.at(3)) == 2) {
            qDebug() << "MAC RESPONSE " << Value;
            service->writeCharacteristic(writeChannel, QByteArray::fromHex("00004006"));
        }
        qDebug() << "value:" << value;
        if (socket -> state() == QAbstractSocket::ConnectedState) {
            socket -> write(value);
        }
    } else if (ringType == RINGTYPE_DC) {
        if (Charuuid == "{c1d02505-2d20-400a-95d2-6a2f7bca0c25}") { // touch
            qDebug() << "touch: " << Value;
            qDebug() << value.length();
            QByteArray toSendData = QByteArray::fromHex("b1e0");
            toSendData.append(value.length());
            toSendData.append(value);
            if (socket -> state() == QAbstractSocket::ConnectedState) {
                socket -> write(toSendData);
            }
        } else if (Charuuid == "{a6ed0202-d344-460a-8075-b9e8ec90d71b}") { // imu
            qDebug() << "imu: " << Value;
            qDebug() << value.length();
            QByteArray toSendData = QByteArray::fromHex("5990");
            toSendData.append(value.length());
            toSendData.append(value);
            if (socket -> state() == QAbstractSocket::ConnectedState) {
                socket -> write(toSendData);
            }
        }
    }
    qDebug() << "BLE设备-" + Charuuid + "特性值发生变化：" + Value;
}

// 定义特征读取成功处理函数
void BLE::handleCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value) {
    QString Charname = c.uuid().toString();
    QString Value = value.toHex();
    qDebug() << "BLE设备-" + Charname + "特性值读取到的值：" + Value;
}

// 定义特性写入成功处理函数
void BLE::handleCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value) {
    QString Charname = c.uuid().toString();
    QString Value = value.toHex();
    qDebug() << "BLE设备-" + Charname + "特性值成功写入值：" + Value;
}

// 定义描述符写入成功处理函数
void BLE::handleDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    QString Charname = QString("%1").arg(descriptor.name());
    qDebug() << "BLE设备-" + Charname + "描述符成功写入值：" + QString(newValue);
}


// 处理服务错误的槽函数
void BLE::handleServiceError(QLowEnergyService::ServiceError newError)
{
    if (QLowEnergyService::NoError == newError) {
        qDebug() << "没有发生错误。\n";
    } else if (QLowEnergyService::OperationError == newError) {
        qDebug() << "错误: 当服务没有准备好时尝试进行操作!\n";
    } else if (QLowEnergyService::CharacteristicReadError == newError) {
        qDebug() << "尝试读取特征值失败!\n";
    } else if (QLowEnergyService::CharacteristicWriteError == newError) {
        qDebug() << "尝试为特性写入新值失败!\n";
    } else if (QLowEnergyService::DescriptorReadError == newError) {
        qDebug() << "尝试读取描述符值失败!\n";
    } else if (QLowEnergyService::DescriptorWriteError == newError) {
        qDebug() << "尝试向描述符写入新值失败!\n";
    } else if (QLowEnergyService::UnknownError == newError) {
        qDebug() << "与服务交互时发生未知错误!\n";
    } else {
        qDebug() << "其它错误";
    }
}


void BLE::serviceDetailsDiscovered(QLowEnergyService::ServiceState newState)
{
    qDebug()<<"State : "<<newState;

    if (newState != QLowEnergyService::RemoteServiceDiscovered) {
        if (newState != QLowEnergyService::RemoteServiceDiscovering) {
        }
        return;
    }
    qDebug() << "after return";


    qDebug() << "wait for connect over";

    service = qobject_cast<QLowEnergyService *>(sender());
    if (!service){
        socket->abort();
        socket->close();
        delete socket;
        return;
    }
    qDebug() << "wait for connect over222";
    if (socket -> state() == QAbstractSocket::ConnectedState) {
        socket -> write("Connected");
        socket->flush();
    }

    //BLE设备特征值改变
    connect(service, &QLowEnergyService::characteristicChanged, this, &BLE::handleCharacteristicChanged);
        //当特征读取请求成功返回其值时，发出此信号。
    connect(service, &QLowEnergyService::characteristicRead, this, &BLE::handleCharacteristicRead);
    //当特性值成功更改为newValue时，会发出此信号。
    connect(service, &QLowEnergyService::characteristicWritten, this, &BLE::handleCharacteristicWritten);
        //描述符修改
    connect(service, &QLowEnergyService::descriptorWritten, this, &BLE::handleDescriptorWritten);
    // 连接错误信号
    connect(service, SIGNAL(error(QLowEnergyService::ServiceError)),
            this, SLOT(handleServiceError(QLowEnergyService::ServiceError)));

    characterList = service->characteristics();
    emit signal_findcharsFinished();
    qDebug() << "wait for connect over222";

    if (ringType == RINGTYPE_YX) {
        for (const QLowEnergyCharacteristic &ch : characterList) {
            qDebug() << ch.uuid() << " " << ch.isValid() << " " << ch.properties();
            if(ch.properties() & QLowEnergyCharacteristic::WriteNoResponse || ch.properties() & QLowEnergyCharacteristic::Write){
                writeChannel = ch;
                service->writeCharacteristic(ch,QByteArray::fromHex("00001100"));
                service->writeCharacteristic(ch,QByteArray::fromHex("00001101"));
                service->writeCharacteristic(ch,QByteArray::fromHex("00009200"));
            }
        }

        for (const QLowEnergyCharacteristic &ch : characterList) {
            if(ch.properties() & QLowEnergyCharacteristic::Notify) {
                QLowEnergyDescriptor notice = ch.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                if (notice.isValid()) {
                    service -> writeDescriptor(notice, QByteArray::fromHex("0100"));
                }
            }
        }
    } else if (ringType == RINGTYPE_DC)  {
        for (const QLowEnergyCharacteristic &ch : characterList) {
            if(ch.properties() & QLowEnergyCharacteristic::Notify) {
                QLowEnergyDescriptor notice = ch.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                if (notice.isValid()) {
                    qDebug() << "subscribe:" << ch.uuid() << " " << ch.isValid() << " " << ch.properties();
                    service -> writeDescriptor(notice, QByteArray::fromHex("0100"));
                }
            }
        }

        QThread::msleep(1000);
        for (const QLowEnergyCharacteristic &ch : characterList) {
            qDebug() << "not filtered:" << ch.uuid() << " " << ch.isValid() << " " << ch.properties();
            if(ch.properties() & QLowEnergyCharacteristic::WriteNoResponse && ch.properties() & QLowEnergyCharacteristic::Write){
                qDebug() << ch.uuid() << " " << ch.isValid() << " " << ch.properties();
                writeChannel = ch;
                qDebug() << "start write ENSPP:";
                service->writeCharacteristic(ch,QString("ENSPP\r\n").toUtf8());
                // QThread::msleep(1000);
                service->writeCharacteristic(ch,QString("ENFAST\r\n").toUtf8());
                // QThread::msleep(1000);
                service->writeCharacteristic(ch,QString("TPOPS=1,1,1\r\n").toUtf8());
                // QThread::msleep(1000);
                service->writeCharacteristic(ch,QString("IMUARG=0,0,0,200\r\n").toUtf8());
                // QThread::msleep(1000);
                service->writeCharacteristic(ch,QString("ENDB6AX\r\n").toUtf8());
                // QThread::msleep(1000);
            }
        }
    }
}

void BLE::doWork() {
    qDebug() << "Do work in thread";
    connectService("{bae80001-4f05-4503-8e65-3af1f7329d1f}");
}



