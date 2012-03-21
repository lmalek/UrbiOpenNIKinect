/* 
 * File:   UKinectDepth.h
 * Author: lmalek
 *
 * Created on 15 marzec 2012, 13:13
 */

#ifndef UKINECTDEPTH_H
#define	UKINECTDEPTH_H

#include "UKinectModule.h"

class UKinectDepth : public UKinectModule {
public:
    // Urbi constructor. Throw error in case of error.
    UKinectDepth(const std::string& name);
    virtual ~UKinectDepth();

    virtual int update();

    void init();
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
    /* 
     * mode = 0 - slave mode in multi generator work
     * mode = 1 - master mode in multi generator work
     * mode = 2 - single mode
     * depth resolution is 1 per 40 mm
     */
    void getData(int mode=0);
    
    unsigned int getXY(unsigned int x, unsigned int y);
    
    unsigned int getMedianFromArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) ;
    
    //
    void changeNotifyImage(urbi::UVar&);

    // Mutex and conditional variable to synchronize threads
    boost::mutex getValMutex;

    // Storage for last captured image.
    urbi::UBinary mBinImage;

    void fpsChanged();

    xn::DepthGenerator depthGenerator;
    xn::DepthMetaData depthMD;
};

#endif	/* UKINECTDEPTH_H */

