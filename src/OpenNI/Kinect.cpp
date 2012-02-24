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
    sample_xml_path = "SamplesConfig.xml";
    pose_to_use = "Psi";
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

void Kinect::Open() throw (std::runtime_error) {
    OpenMotor();
}

void Kinect::Close() {
    CloseMotor();
}

bool Kinect::Opened();
void Kinect::SetMotorPosition(double pos);
void Kinect::SetLedMode(int NewMode);
bool Kinect::GetAcceleroData(float *x, float *y, float *z) {
    XnStatus res;
    XnUInt32  	nBufferSize = 10;
    XnUChar * pBuffer = new XnUChar[nBufferSize];
    XnUInt32 pnBytesReceived;

    float ux,uy,uz;

    // Send move control requests
    for (XnUInt32 index = 0; index < m_num; ++index)
    {
        res = xnUSBReceiveControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x32, 0x00, 0x00, pBuffer, nBufferSize, &pnBytesReceived, 0);

        if (res != XN_STATUS_OK)
        {
            xnPrintError(res, "xnUSBSendControl failed");
            return false;
        }
	else 
	{
		printf("position : ");
			for (int i=0; i<nBufferSize; i++)
				printf("%x ", pBuffer[i]);
		ux = (((uint16_t)pBuffer[2] << 8) | pBuffer[3])/819;
		uy = (((uint16_t)pBuffer[4] << 8) | pBuffer[5])/819;
		uz = (((uint16_t)pBuffer[6] << 8) | pBuffer[7])/819;		
	}
    }
}

void Kinect::Run();
void Kinect::Stop();

void Kinect::ParseColorBuffer();
void Kinect::ParseColorBuffer32();

void Kinect::ParseIRBuffer();

void Kinect::ParseDepthBuffer();

void Kinect::OpenMotor() throw (std::runtime_error) {
    const XnUSBConnectionString *paths;
    XnUInt32 count;
    XnStatus res;

    // Init OpenNI USB
    res = xnUSBInit();
    if (res != XN_STATUS_OK) {
        throw std::runtime_error("xnUSBInit failed");
    }

    // Open all "Kinect motor" USB devices
    res = xnUSBEnumerateDevices(0x045E /* VendorID */, 0x02B0 /*ProductID*/, &paths, &count);
    if (res != XN_STATUS_OK) {
        throw std::runtime_error("xnUSBEnumerateDevices failed");
    }

    // Open devices
    for (XnUInt32 index = 0; index < count; ++index) {
        res = xnUSBOpenDeviceByPath(paths[index], &m_devs[index]);
        if (res != XN_STATUS_OK) {

            throw std::runtime_error("xnUSBOpenDeviceByPath failed");
        }
    }

    m_num = count;
    XnUChar buf[1]; // output buffer

    // Init motors
    for (XnUInt32 index = 0; index < m_num; ++index) {
        res = xnUSBSendControl(m_devs[index], XN_USB_CONTROL_TYPE_VENDOR, 0x10, 0x00, 0x00, buf, sizeof (buf), 0);
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

void Kinect::CloseMotor() {
    if (m_isOpen) {
        for (XnUInt32 index = 0; index < m_num; ++index) {
            xnUSBCloseDevice(m_devs[index]);
        }
        m_isOpen = false;
    }
}

void Kinect::Init() throw (std::runtime_error) {
    XnStatus nRetVal = XN_STATUS_OK;
    xn::EnumerationErrors errors;
    nRetVal = g_Context.InitFromXmlFile(sample_xml_path.c_str(), g_scriptNode, &errors);
    if (nRetVal == XN_STATUS_NO_NODE_PRESENT) {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        throw std::runtime_error(strError);
    } else if (nRetVal != XN_STATUS_OK) {
        throw std::runtime_error(xnGetStatusString(nRetVal));
    }
}

void XN_CALLBACK_TYPE
Kinect::User_NewUser(xn::UserGenerator& generator,
             XnUserID nId, void* pCookie)
{
  printf("New User: %d\n", nId);
  g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(pose_to_use.c_str(), nId);
}

void XN_CALLBACK_TYPE
Kinect::User_LostUser(xn::UserGenerator& generator, XnUserID nId,
              void* pCookie)
{
  printf("User lost: %d\n", nId);
}

void XN_CALLBACK_TYPE
Kinect::Pose_Detected(xn::PoseDetectionCapability& pose, const XnChar* strPose,
              XnUserID nId, void* pCookie)
{
  printf("Pose %s for user %d\n", strPose, nId);
  g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
  g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

void XN_CALLBACK_TYPE
Kinect::Calibration_Start(xn::SkeletonCapability& capability, XnUserID nId,
                  void* pCookie)
{
  printf("Starting calibration for user %d\n", nId);
}

void XN_CALLBACK_TYPE
Kinect::Calibration_End(xn::SkeletonCapability& capability, XnUserID nId,
                XnBool bSuccess, void* pCookie)
{
  if (bSuccess)
  {
    printf("User calibrated\n");
    g_UserGenerator.GetSkeletonCap().StartTracking(nId);
  }
  else
  {
    printf("Failed to calibrate user %d\n", nId);
   g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(pose_to_use.c_str(), nId);
  }
}