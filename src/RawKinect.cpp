// Forked from: https://groups.google.com/d/msg/openni-dev/T_CeVW_d8ig/dsBKONIpNyQJ

#include "RawKinect.h"

RawKinect::RawKinect() {
    m_isOpen = false;
}

RawKinect::~RawKinect() {
    close();
}

bool RawKinect::open() {
    const XnUSBConnectionString *paths;
    XnUInt32 count;
    XnStatus res;

    // Init OpenNI USB 
    res = xnUSBInit();
    if (res != XN_STATUS_OK || res != XN_STATUS_USB_ALREADY_INIT) {
        xnPrintError(res, "xnUSBInit failed");
        return false;
    }

    // Open all "Kinect motor" USB devices
    res = xnUSBEnumerateDevices(0x045E /* VendorID */, 0x02B0 /*ProductID*/, &paths, &count);
    if (res != XN_STATUS_OK) {
        xnPrintError(res, "xnUSBEnumerateDevices failed");
        return false;
    }

    // Open devices
    for (XnUInt32 index = 0; index < count; ++index) {
        res = xnUSBOpenDeviceByPath(paths[index], &m_devs[index]);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBOpenDeviceByPath failed");
            return false;
        }
    }

    m_num = count;
    XnUChar buf[1]; // output buffer

    // Init motors
    for (XnUInt32 index = 0; index < m_num; ++index) {
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x10, 0x00, 0x00, buf, sizeof (buf), 0);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBSendControl failed");
            close();
            return false;
        }

        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x06, 0x02, 0x00, NULL, 0, 0);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBSendControl failed");
            close();
            return false;
        }
    }

    m_isOpen = true;

    return true;
}

void RawKinect::close() {
    if (m_isOpen) {
        for (XnUInt32 index = 0; index < m_num; ++index) {
            xnUSBCloseDevice(m_devs[index]);
        }
        m_isOpen = false;
    }
}

bool RawKinect::moveMotor(int angle) {
    XnStatus res;

    // Send move control requests
    for (XnUInt32 index = 0; index < m_num; ++index) {
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x31, angle, 0x00, NULL, 0, 0);

        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBSendControl failed");
            return false;
        }
    }
    return true;
}

bool RawKinect::setLed(RawKinect::LedColor color) {
    XnStatus res;

    // Send move control requests
    for (XnUInt32 index = 0; index < m_num; ++index) {
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x06, color, 0x00, NULL, 0, 0);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBSendControl failed");
            return false;
        }
    }
    return true;
}

bool RawKinect::getAccelerometer(float &x, float &y, float &z) {
    XnStatus res;
    XnUInt32 nBufferSize = 10;
    XnUChar * pBuffer = new XnUChar[nBufferSize];
    XnUInt32 pnBytesReceived;
    int16_t ux, uy, uz;

    // Send move control requests
    for (XnUInt32 index = 0; index < m_num; ++index) {
        res = xnUSBReceiveControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x32, 0x00, 0x00, pBuffer, nBufferSize, &pnBytesReceived, 0);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "[RawKinect]::getAccelerometer() : xnUSBReceiveControl failed");
            return false;
        }
        ux = (int16_t) (((short) pBuffer[2] << 8) | pBuffer[3]);
        uy = (int16_t) (((short) pBuffer[4] << 8) | pBuffer[5]);
        uz = (int16_t) (((short) pBuffer[6] << 8) | pBuffer[7]);
        // recalculate gravitation vector 
        x = 1.0 * ux / 819;
        y = 1.0 * uy / 819;
        z = 1.0 * uz / 819;
    }
    return true;
}