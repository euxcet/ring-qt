#ifndef DEVICEINTERFACE_H
#define DEVICEINTERFACE_H

class DeviceInterface {
public:
    virtual ~DeviceInterface() {}
    virtual void onConnected() = 0;
    virtual void onDisconnected() = 0;
};

#endif // DEVICEINTERFACE_H
