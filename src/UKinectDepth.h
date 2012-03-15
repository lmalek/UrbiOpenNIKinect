/* 
 * File:   UKinectDepth.h
 * Author: lmalek
 *
 * Created on 15 marzec 2012, 13:13
 */

#ifndef UKINECTDEPTH_H
#define	UKINECTDEPTH_H

#include "UKinectModule.h"

class UKinectDepth : public UKinectModule{
public:
    // Urbi constructor. Throw error in case of error.
    UKinectDepth(const std::string& name);
    virtual ~UKinectDepth();

    virtual int update();
    
    void init(int);
private:    
    // Our image variable and dimensions
    urbi::UVar image;
    urbi::UVar width;
    urbi::UVar height;
    urbi::UVar fps;
    urbi::UVar notify;
    urbi::UVar flip;

    bool mGetNewFrame; //
    unsigned int mFrame; // ID of already grabed frame
    unsigned int mAccessFrame; // ID of already retrieved frame

    // Called on access.
    void getImage();

    //
    void changeNotifyImage(urbi::UVar&);

    // Mutex and conditional variable to synchronize threads
    boost::mutex getValMutex;

    // Storage for last captured image.
    urbi::UBinary mBinImage;

    void fpsChanged();

    xn::DepthGenerator depthGenerator;
    xn::DepthMetaData depthMD;
    uint16_t* depthBufor;
};

#endif	/* UKINECTDEPTH_H */

