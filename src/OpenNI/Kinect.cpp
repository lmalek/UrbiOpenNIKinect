/* 
 * File:   opennikinect.cpp
 * Author: lmalek
 * 
 * Created on 22 luty 2012, 09:59
 */

#include "Kinect.h"

using namespace OpenNI;

Kinect::Kinect() {
    m_isOpen = false;    
}

Kinect::Kinect(const Kinect& orig) {
}

Kinect::~Kinect() {
    Close();
    g_scriptNode.Release();
    g_DepthGenerator.Release();
    g_UserGenerator.Release();
    g_Player.Release();
    g_Context.Release();
}

void Kinect::Open() throw (std::runtime_error){
    OpenMotor();
}

void Kinect::Close() {
    CloseMotor();
}


void Kinect::OpenMotor() throw (std::runtime_error)
{
    const XnUSBConnectionString *paths;
    XnUInt32 count;
    XnStatus res;
    
    // Init OpenNI USB
    res = xnUSBInit();
    if (res != XN_STATUS_OK)
    {
        throw std::runtime_error("xnUSBInit failed");
    }
    
    // Open all "Kinect motor" USB devices
    res = xnUSBEnumerateDevices(0x045E /* VendorID */, 0x02B0 /*ProductID*/, &paths, &count);
    if (res != XN_STATUS_OK)
    {
       throw std::runtime_error("xnUSBEnumerateDevices failed");
    }
    
    // Open devices
    for (XnUInt32 index = 0; index < count; ++index)
    {
        res = xnUSBOpenDeviceByPath(paths[index], &m_devs[index]);
        if (res != XN_STATUS_OK) {
            
            throw std::runtime_error("xnUSBOpenDeviceByPath failed");
        }
    }
    
    m_num = count;
    XnUChar buf[1]; // output buffer
    
    // Init motors
    for (XnUInt32 index = 0; index < m_num; ++index)
    {
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x10, 0x00, 0x00, buf, sizeof(buf), 0);
        if (res != XN_STATUS_OK) {
            CloseMotor();
            throw std::runtime_error("xnUSBSendControl failed");
        }
        
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x06, 0x02, 0x00, NULL, 0, 0);
        if (res != XN_STATUS_OK) {
            CloseMotor();
            throw std::runtime_error("xnUSBSendControl failed");
        }
    }
    
    m_isOpen = true;
}

void Kinect::CloseMotor()
{
    if (m_isOpen) {
        for (XnUInt32 index = 0; index < m_num; ++index) {
            xnUSBCloseDevice(m_devs[index]);
        }
        m_isOpen = false;
    }
}