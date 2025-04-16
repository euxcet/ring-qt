#include "ring.h"

Ring::Ring(const QBluetoothDeviceInfo &info, TcpServer *server, DeviceInterface *interface) : info(info), server(server), interface(interface) {
    batteryTimer = nullptr;
}

void Ring::releaseResources(void) {
    if (!isConnected) {
        return;
    }
    isConnected = false;
    if (m_LowController) {
        m_LowController->disconnectFromDevice();
        delete m_LowController;
    }
    m_LowController = nullptr;
}

void Ring::dataFromClient(QByteArray data) {
    if (m_LowController && service) {
        service->writeCharacteristic(writeChannel, data);
    }
}

void Ring::connectDevice(void) {
    isConnected = false;
    qDebug() << "bind socket to receive";

    if (!m_LowController) {
        m_LowController = QLowEnergyController::createCentral(info);
        QLowEnergyAdvertisingParameters adv;
        adv.setInterval(20, 20);
        QLowEnergyAdvertisingData data;
        m_LowController->startAdvertising(adv, data);

        connect(m_LowController, &QLowEnergyController::errorOccurred, this, &Ring::handleError);
        connect(m_LowController, &QLowEnergyController::connected, this, &Ring::handleDeviceConnected);
        connect(m_LowController, &QLowEnergyController::disconnected, this, &Ring::handleDeviceDisconnected);
        connect(m_LowController, &QLowEnergyController::serviceDiscovered, this, &Ring::handleServiceDiscovered);
        connect(m_LowController, &QLowEnergyController::discoveryFinished, this, &Ring::handleDiscoveryFinished);
        connect(server, &TcpServer::dataFromClient, this, &Ring::dataFromClient);
    }
    qDebug() << "connect address:" << info.address();
    m_LowController->connectToDevice();
}

void Ring::disconnectDevice(void) {
    releaseResources();
}

void Ring::handleBatteryTimeout(void) {
    if (service) {
        service->writeCharacteristic(writeChannel, QByteArray::fromHex("00001200"));
    }
}

void Ring::handleDeviceConnected(void) {
    qDebug("Device connect success.");
    isConnected = true;
    m_LowController->discoverServices();
    interface->onConnected();
}

void Ring::handleDeviceDisconnected(void) {
    qDebug("Device disconnected.");
    releaseResources();
    interface->onDisconnected();
}

void Ring::handleServiceDiscovered(const QBluetoothUuid &uuid) {
    qDebug() << "Service discovered:" << uuid.toString();
}

void Ring::handleDiscoveryFinished(void) {
    qDebug() << "Services scan finished.";
    connectService("{bae80001-4f05-4503-8e65-3af1f7329d1f}");
}

void Ring::handleError(QLowEnergyController::Error error) {
    qDebug("error occured");
    qDebug() << error;
    releaseResources();
}

void Ring::connectService(QString uuid) {
    QList<QBluetoothUuid> services = m_LowController->services();

    for (const QBluetoothUuid &serviceUuid : services) {
        if (serviceUuid.toString() == uuid) {
            QLowEnergyService *service = m_LowController->createServiceObject(serviceUuid);
            connect(service, &QLowEnergyService::stateChanged, this, &Ring::serviceDetailsDiscovered);
            service->discoverDetails();
        }
    }
}

void Ring::serviceDetailsDiscovered(QLowEnergyService::ServiceState newState) {
    qDebug()<<"State : "<<newState;
    if (newState != QLowEnergyService::RemoteServiceDiscovered) {
        return;
    }

    service = qobject_cast<QLowEnergyService *>(sender());

    connect(service, &QLowEnergyService::characteristicChanged, this, &Ring::handleCharacteristicChanged);
    connect(service, &QLowEnergyService::characteristicRead, this, &Ring::handleCharacteristicRead);
    connect(service, &QLowEnergyService::characteristicWritten, this, &Ring::handleCharacteristicWritten);
    connect(service, &QLowEnergyService::descriptorWritten, this, &Ring::handleDescriptorWritten);

    for (const QLowEnergyCharacteristic &ch : service->characteristics()) {
        qDebug() << ch.uuid() << " " << ch.isValid() << " " << ch.properties();
        if(ch.properties() & QLowEnergyCharacteristic::WriteNoResponse || ch.properties() & QLowEnergyCharacteristic::Write){
            writeChannel = ch;
            service->writeCharacteristic(ch, QByteArray::fromHex("00001100"));
            service->writeCharacteristic(ch, QByteArray::fromHex("00001101"));
            service->writeCharacteristic(ch, QByteArray::fromHex("000040060007000702"));
            service->writeCharacteristic(ch, QByteArray::fromHex("00000000"));
        }
    }

    for (const QLowEnergyCharacteristic &ch : service->characteristics()) {
        if(ch.properties() & QLowEnergyCharacteristic::Notify) {
            QLowEnergyDescriptor notice = ch.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
            if (notice.isValid()) {
                service -> writeDescriptor(notice, QByteArray::fromHex("0100"));
            }
        }
    }
}

void Ring::handleCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value) {
    QString Value = value.toHex();

    server -> send(QByteArray::fromHex("33445566") + info.address().toString().toLocal8Bit() + value);
    qDebug() << "Ring-" + info.address().toString() + "-特性值发生变化：" + Value;
}

void Ring::handleCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value) {
    QString Value = value.toHex();
    // qDebug() << "Ring-" + info.address().toString() + "-特性值读取到的值：" + Value;
}

void Ring::handleCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value) {
    QString Value = value.toHex();
    // qDebug() << "Ring-" + info.address().toString() + "-特性值成功写入值：" + Value;
}

void Ring::handleDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    QString Charname = QString("%1").arg(descriptor.name());
    // qDebug() << "Ring-" + info.address().toString() + "-描述符成功写入值：" + QString(newValue);
}

