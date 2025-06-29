#ifndef USBDEVICE_H
#define USBDEVICE_H

#include <QString>

class USBDevice
{
public:
//    USBDevice();
    USBDevice(QString name, QString serialN, QString firmware)
        : deviceName(name),serialNumber(serialN), deviceFirmware(firmware)
    {

    }

    QString getName()         const { return deviceName;     }
    QString getSerialNumber() const { return serialNumber;   }
    QString getFirmware()     const { return deviceFirmware; }

    void setName(QString name)           { deviceName = name;         }
    void setSerialNumber(QString serial) { serialNumber = serial;     }
    void setFirmware(QString firmware)   { deviceFirmware = firmware; }

private:
    QString deviceName;
    QString serialNumber;
    QString deviceFirmware;
};

#endif // USBDEVICE_H
