/** 
@file GrabDetector.h
*/ 

#ifndef _GRAB_DETECTOR_H_
#define _GRAB_DETECTOR_H_

#include <OpenNI.h>
#include "DllInterface.h"

/**
@brief Primesense Labs namespace
*/
namespace PSLabs{
	/**
	 *	Interface for a listener to grab detection events
	 */
	class IGrabEventListener{
	public:
		enum GrabEventType { GRAB_EVENT, RELEASE_EVENT, NO_EVENT};
		typedef struct EventParams{
			GrabEventType	Type;
		}EventParams;

		virtual void			DLL_CALL	ProcessGrabEvent(const EventParams& params) = 0;
	};

	/**
	 *	Class to be used for grab detection functionality
	 *	This is an abstract class. Instance should be created only with @ref PSLabs::CreateGrabDetector
	 */
	class IGrabDetector : public DLLInterface{
	public:
		/**
		*	Registers new listener for grab events
		*	@param [in] eventListener pointer to the event listener
		*	@return STATUS_OK on success, or error code if failed or not initialized
		*/ 
		virtual openni::Status	DLL_CALL	AddListener(IGrabEventListener *eventListener) = 0;

		/**
		*	Removes the given grab events listener
		*	@param [in] eventListener pointer to the event listener
		*	@return STATUS_OK on success, or error code if failed or not initialized
		*/ 
		virtual openni::Status	DLL_CALL	RemoveListener(IGrabEventListener *eventListener) = 0;

		/**
		*	Sets the side (left/right) of the currently tracked hand. The use of this function is not mandatory
		*	but it helps achieve better detection results
		*	@param [in] side	+1: right hand
		*						-1: left hand
		*						 0: auto-detect (default)
		*/ 
		virtual void			DLL_CALL	SetHandSide(int side) = 0;

		/**
		*	Sets the device exposure parameters to be optimal for grab detection
		*	@param [in] useOptimal whether to activate or disable the optimal parameters
		*	@param [in] depthHandle handle to the active depth stream
		*	@param [in] depthHandle handle to the active image stream
		*	@return STATUS_OK on success, or error code when setting any of the parameters failed
		*/
		virtual	openni::Status	DLL_CALL	SetOptimalExposure(bool useOptimal, OniStreamHandle depthHandle, OniStreamHandle imageHandle) = 0;

		/**
		*	Sets the position of the hand for which we do grab detection. The hand will be searched for in this location
		*	when calling @ref UpdateFrame
		*	@param [in] posX X position of COM in Real World coordinates
		*	@param [in] posY Y position of COM in Real World coordinates
		*	@param [in] posZ Z position of COM in Real World coordinates
		*/ 
		virtual void			DLL_CALL	SetHandPosition(float posX, float posY, float posZ) = 0;

		/**
		*	Gets last known Real World hand coordinates
		*	@param [out] posX X position of COM in Real World coordinates
		*	@param [out] posY Y position of COM in Real World coordinates
		*	@param [out] posZ Z position of COM in Real World coordinates
		*	@return STATUS_OK on success, STATUS_OUT_OF_FLOW if algorithm was reset, or error code when device failed to initialize
		*/ 
		virtual openni::Status	DLL_CALL	GetHandPosition(float* posX, float* posY, float* posZ) const = 0;

		/**
		*	Gets the status of the GrabDetector with details of the last event
		*	@param [out] resultParams Will be filled with the details of the last grab\release event
		*	@return STATUS_OK on success, or error code when device failed to initialize
		*/ 
		virtual openni::Status	DLL_CALL	GetLastEvent(IGrabEventListener::EventParams* resultParams) const = 0 ;


		/**
		*	Resets the grab detection algorithms
		*/ 
		virtual void			DLL_CALL	Reset() = 0;

		/**
		*	Updates the algorithm with the given depth and image frames. The frames must be synchronized. and in VGA resolution
		*	This function updates the tracking/gesture/grabbing algorithms using the latest hand position
		*	@param [in] pDepthFrame Pointer to the (mandatory) depth frame
		*	@param [in] pImageFrame Pointer to the (optional) image frame
		*/ 
		virtual	void			DLL_CALL	UpdateFrame(OniFrame* pDepthFrame, OniFrame* pImageFrame) = 0;

		/**
		*	Updates the algorithm with the given depth and image frames. The frames must be synchronized. and in VGA resolution
		*	This function updates the tracking/gesture/grabbing algorithms using the latest hand position
		*	@param [in] depthFrame Reference to the depth frame
		*	@param [in] imageFrame Reference to the image frame
		*/
		inline	void			DLL_CALL	UpdateFrame(openni::VideoFrameRef& depthFrame, openni::VideoFrameRef& imageFrame)
		{
			UpdateFrame(depthFrame._getFrame(), imageFrame._getFrame());
		}

		/**
		*	Updates the algorithm with the given depth frame. The frame must be in VGA resolution
		*	This function updates the tracking/gesture/grabbing algorithms using the latest hand position
		*	@param [in] depthFrame Reference to the depth frame
		*/
		inline	void			DLL_CALL	UpdateFrame(openni::VideoFrameRef& depthFrame)
		{
			UpdateFrame(depthFrame._getFrame(), NULL);
		}
	};

	/**
	*	Creates and returns new GrabDetector for the given device
	*	@param [in] device The device we will work on
	*	@param [in] dataFolder The folder with the grab detection data file (grab_gesture.dat). Default folder is "Data"
	*	@return Pointer to the newly created GrabDetector, or NULL on failure
	*/ 
	ONI_C_API_EXPORT	IGrabDetector*	DLL_CALL CreateGrabDetectorFromHandle(OniDeviceHandle device, const char* dataFolder = "Data/");
	/**
	*	Creates and returns new GrabDetector for the given device
	*	@param [in] device The device we will work on
	*	@param [in] dataFolder The folder with the grab detection data file (grab_gesture.dat). Default folder is "Data"
	*	@return Pointer to the newly created GrabDetector, or NULL on failure
	*/ 
	static				IGrabDetector*	DLL_CALL CreateGrabDetector(openni::Device& device, const char* dataFolder = "Data/")
	{
		return CreateGrabDetectorFromHandle(device._getHandle(), dataFolder);
	}
	/**
	*	Frees the previously initialized grab detector
	*	@param [in] grabDetector The grab detector
	*/ 
	ONI_C_API_EXPORT	void			DLL_CALL ReleaseGrabDetector(IGrabDetector* grabDetector);
}	


#endif