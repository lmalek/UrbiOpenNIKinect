// Forked from: https://groups.google.com/d/msg/openni-dev/T_CeVW_d8ig/dsBKONIpNyQJ

#include "KinectMotors.h"
KinectMotors::KinectMotors()
{
    m_isOpen = false;
}

KinectMotors::~KinectMotors()
{
    Close();
}

bool KinectMotors::Open()
{
    const XnUSBConnectionString *paths;
    XnUInt32 count;
    XnStatus res;
    
    // Init OpenNI USB
    res = xnUSBInit();
    if (res != XN_STATUS_OK)
    {
        xnPrintError(res, "xnUSBInit failed");
        return false;
    }
    
    // Open all "Kinect motor" USB devices
    res = xnUSBEnumerateDevices(0x045E /* VendorID */, 0x02B0 /*ProductID*/, &paths, &count);
    if (res != XN_STATUS_OK)
    {
        xnPrintError(res, "xnUSBEnumerateDevices failed");
        return false;
    }
    
    // Open devices
    for (XnUInt32 index = 0; index < count; ++index)
    {
        res = xnUSBOpenDeviceByPath(paths[index], &m_devs[index]);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBOpenDeviceByPath failed");
            return false;
        }
    }
    
    m_num = count;
    XnUChar buf[1]; // output buffer
    
    // Init motors
    for (XnUInt32 index = 0; index < m_num; ++index)
    {
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x10, 0x00, 0x00, buf, sizeof(buf), 0);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBSendControl failed");
            Close();
            return false;
        }
        
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x06, 0x02, 0x00, NULL, 0, 0);
        if (res != XN_STATUS_OK) {
            xnPrintError(res, "xnUSBSendControl failed");
            Close();
            return false;
        }
    }
    
    m_isOpen = true;
    
    return true;
}

void KinectMotors::Close()
{
    if (m_isOpen) {
        for (XnUInt32 index = 0; index < m_num; ++index) {
            xnUSBCloseDevice(m_devs[index]);
        }
        m_isOpen = false;
    }
}

bool KinectMotors::Move(int angle)
{
    XnStatus res;
    
    // Send move control requests
    for (XnUInt32 index = 0; index < m_num; ++index)
    {
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x31, angle, 0x00, NULL, 0, 0);

        if (res != XN_STATUS_OK)
        {
            xnPrintError(res, "xnUSBSendControl failed");
            return false;
        }
    }
    return true;
}

