/*
 * Kinect module based on OpenNI for URBI
 * Copyright (C) 2012  Lukasz Malek
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  
 * File:   UKinectOpenNI.h
 * Author: lmalek
 *
 * Created on 21 marzec 2012, 09:55
 */

#ifndef UKINECTOPENNI_H
#define	UKINECTOPENNI_H

#include <urbi/uobject.hh>
#include <XnCppWrapper.h>
#include <string>

#ifdef _WIN32 // note the underscore: without it, it's not msdn official!
#  include <opencv.hpp>
#elif __unix__ // all unices
#  include <cv.h>
#endif

#include "KinectMotors.h"

#define MAX_NUM_USERS 10

class UKinectOpenNI : public urbi::UObject {
public:
    UKinectOpenNI(const std::string& name);
    virtual ~UKinectOpenNI();

    /**
     * Change the update data flag.
     * @return Return 0
     */
    virtual int update();

    /** 
     * Initialization function
     * Responsible for initalization of OpenNI camponents. Available in URBI
     * 
     * @param image Activate OpenNI component allowing to geather image from
     *              Kinect
     * @param depth Activate OpenNI component allowing to geather depth map
     *              from Kinect
     * @param user  Activete OpenNI component allwoing to recognize user
     *              skeleton
     */
    void init(bool image, bool depth, bool user);

    /** 
     * Activating image component.  
     * Not available in URBI
     */
    void activateImage();
    
    /**
     * Dectivate image component. 
     * Not available in URBI
     */
    void deactivateImage();

    /** 
     * Activating depth map component.
     * Not available in URBI
     */
    void activateDepth();
    
    /** 
     * Deactivating dempth map component.
     * Not available in URBI
     */
    void deactivateDepth();

    /** 
     * Activating user skeleton component.
     * Not available in URBI
     */
    void activateUsers();
    
    /**  
     * Deactivating user skeleton component.
     * Not available in URBI
     */
    void deactivateUsers();

    
    urbi::UVar image;           /**< Image from Kinect camera */      
    urbi::UVar imageWidth;      /**< Image width from Kinect camera */  
    urbi::UVar imageHeight;     /**< Image height from Kinect camera */  

    urbi::UVar depth;           /**< Depth map from Kinect camera, unit value in 25 mm */
    urbi::UVar depthWidth;      /**< Depth map width from Kinect camera */
    urbi::UVar depthHeight;     /**< Depth map height from Kinect camera */

    urbi::UVar skeleton;        /**< Skeletons image from Kinect camera */
    urbi::UVar skeletonWidth;   /**< Skeletons image width from Kinect camera */
    urbi::UVar skeletonHeight;  /**< Skeletons image height from Kinect camera */
    urbi::UVar numUsers;        /**< Number of users detected */
    urbi::UVar jointConfidence; /**< Confidence level for joints detection */

    urbi::UVar fps;             /**< Number of FPS */

    /**
     * Set the Kinect head to given angle
     * 
     * @param angle Absolut angle value from the ground level in degrees
     * @return Return true on succes and false on failure.
     */
    bool motorMove(int angle);
    
    /**
     * Set the Kinect led to given color
     * 
     * @param color value from 0 to 6 where: 0 - LED_OFF, 1 - LED_GREEN, 
     * 2 - LED_RED, 3 - LED_YELLOW (actually orange), 
     * 4 - LED_BLINK_YELLOW (actually orange), 5 - LED_BLINK_GREEN, 
     * 6 - LED_BLINK_RED_YELLOW (actually red/orange)
     * @return Return true on succes and false on failure.
     */
     bool setLed(int color);

    
    // image component functions -----------------------------------------------
    
    /**
     * Refresh all data from Kinect.
     * Update the data from kinect to all activated functionalities.
     */
    void refreshData();

    /**
     * If available update the image for URBI.
     */
    void getImage();
    
    /**
     * Change the notivy behaviour on image URBI object.
     * @param var Boolean value. True to enable notify. False to disable.
     */
    void changeNotifyImage(urbi::UVar & var);

    // depth component functions -----------------------------------------------
    
    /**
     * If available update the depth map for URBI.
     */
    void getDepth();
    
    /**
     * Change the notivy behaviour on depth map  URBI object.
     * @param var Boolean value. True to enable notify. False to disable.
     */
    void changeNotifyDepth(urbi::UVar & var);
    
    /**
     * Get pixel value from depth map.
     * @param x Coordinate of horizontal pixel posion on depth map
     * @param y Coordinate of vertical pixel posion on depth map
     */
    unsigned int getDepthXY(unsigned int x, unsigned int y);
    
    /**
     * Get median depth value from rectangle area.
     * @param x1 Coordinate of upper left pixel horizontal posion on depth map
     * @param y1 Coordinate of upper left pixel vertical posion on depth map
     * @param x2 Coordinate of lower right pixel horizontal posion on depth map
     * @param y2 Coordinate of lower right pixel vertical posion on depth map
     */
    unsigned int getDepthMedianFromArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
    
    /**
     * Fix calibration depth and image.
     * @param state If true depth map and image are calibrated.
     */
    void matchDepthToImage(bool state);

    // users component functions -----------------------------------------------
    
    /**
     * If available update users posture for URBI.
     */ 
    void getUsers();
    
    /**
     * Get skeletons and place them on given image.
     * @param src Image on whitch the skeletons should be painted.
     */
    void getSkeleton(urbi::UImage src);
    
    /**
     * Change the notivy behaviour on user skeletons URBI object.
     * @param var Boolean value. True to enable notify. False to disable.
     */    
    void changeNotifyUsers(urbi::UVar & var);
    
    /**
     * Allow to obtain the list of tracked user ID list
     * @return Return vector of user ID list.
     */
    std::vector<int> getUsersID();
    
    /**
     * Allow to obtain the list of visible user ID list. User is visible if
     * its torso is visible.
     * @param eJoint joint number that identify that user is visible
     * @return Return vector of user ID list.
     */
    std::vector<int> getVisibleUsersID(int eJoint);
    
    /**
     * Check if user posture of a given number is being tracked
     * @param nr User number
     * @return Return the true if user is tracked. Otherwise false.
     */
    bool isUserTracked(unsigned int nr);
    
    /**
     * Get the (X,Y,Z) position of the given body joint for given user
     * @param user User number
     * @param jointNumber Joint code/number @see <a href="http://openni.org/Documentation/Reference/_xn_types_8h_ac025301dbcbd9a91e532fa3d8991361d.html#ac025301dbcbd9a91e532fa3d8991361d">XnSkeletonJoint</a>
     * @return Return a 3 element vector with joint coordinate (X,Y,Z) in [m]
     */
    std::vector<float> getJointPosition(unsigned int user, unsigned int jointNumber); 
    
    /**
     * Get the (X,Y) coordinate on image of the given body joint for given user
     * @param user User number
     * @param jointNumber Joint code/number @see <a href="http://openni.org/Documentation/Reference/_xn_types_8h_ac025301dbcbd9a91e532fa3d8991361d.html#ac025301dbcbd9a91e532fa3d8991361d">XnSkeletonJoint</a>
     * @return Return a 3 element vector with joint coordinate (X,Y,Z)
     */
    std::vector<int> getJointImageCoordinate(unsigned int user, unsigned int jointNumber); 
   
private:

    bool mGetNewData;           /**< Flag that new data was should be obtained */

    // Mutex and conditional variable to synchronize threads
    boost::mutex getValMutex;

    void fpsChanged();
    
    KinectMotors motors;        /**< Direct kinect motor acces */

    xn::Context context;        /**< OpenNI context for connection with Kinect */

    // image component variables -----------------------------------------------
    bool imageActive;
    xn::ImageGenerator imageGenerator;
    xn::ImageMetaData imageMD;
    urbi::UBinary mBinImage; // Storage for last captured image.

    // depth component variables -----------------------------------------------
    bool depthActive;
    xn::DepthGenerator depthGenerator;
    xn::DepthMetaData depthMD;
    urbi::UBinary mBinDepth; // Storage for last captured image.

    // users component variables -----------------------------------------------
    bool usersActive;
    xn::UserGenerator userGenerator;
    urbi::UBinary mBinSkeleton; // Storage for last captured image.
    XnUInt16 nUsers;
    XnUserID aUsers[MAX_NUM_USERS];

    // user component functions
    void drawLimb(cv::Mat& processImage, XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2);
    XnSkeletonJoint jointNumberToSkeleton(unsigned int jointNumber);
    bool jointCoordinateInImageArea(XnPoint3D joint);
};

#endif	/* UKINECTOPENNI_H */

