///////////////////////////////////////////////////////////////////////////////
// FILE:          FirstLightImagingCameras.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Device adapter for C-RED2 & C-RED3 cameras for USB, Matrox, 
//				  Sapera and Edt grabbers.
//                
// AUTHOR:        JTU, 13/11/2019
//
// COPYRIGHT:     First Light Imaging Ltd, (2011-2019)
// LICENSE:       License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES. 
//

#pragma once

#include <string>

#include "DeviceBase.h"
#include "DeviceThreads.h"
#include "FliSdk_C.h"

class FliThreadImp;

class FirstLightImagingCameras : public CCameraBase<FirstLightImagingCameras>
{
public:

	friend class FliThreadImp;

	FirstLightImagingCameras(std::string cameraName);
	~FirstLightImagingCameras();

	// Inherited via CCameraBase
	int Initialize();
	int Shutdown();
	void GetName(char* name) const;
	long GetImageBufferSize() const;
	unsigned GetBitDepth() const;
	int GetBinning() const;
	int SetBinning(int binSize);
	void SetExposure(double exp_ms);
	double GetExposure() const;
	int SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize);
	int GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize);
	int ClearROI();
	int IsExposureSequenceable(bool& isSequenceable) const;
	const unsigned char* GetImageBuffer();
	unsigned GetImageWidth() const;
	unsigned GetImageHeight() const;
	unsigned GetImageBytesPerPixel() const;
	int SnapImage();
	int StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow);
	int StopSequenceAcquisition();
	void OnThreadExiting() throw();
	bool IsCapturing();

	void refreshValues();
	void imageReceived(const uint8_t* image);

private:
	int onMaxExposure(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onMaxFps(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onFps(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onCameraChange(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onDetectCameras(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onSendCommand(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onBinning(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onSetMaxExposure(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onSetMaxFps(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onBuildBias(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onApplyBias(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onApplySensorTemp(MM::PropertyBase* pProp, MM::ActionType eAct);
	int onShutdown(MM::PropertyBase* pProp, MM::ActionType eAct);

	// Property Actions for CBlue1
    int onTemperatureSelector(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onCoolingSetpoint(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onFrameRateMin(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onFrameRateMax(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onExposureTime(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onExposureTimeMin(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onExposureTimeMax(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onGain(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onConvEfficiency(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onPixelFormat(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onReverseX(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onReverseY(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onFanMode(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onDeviceStatus(MM::PropertyBase* pProp, MM::ActionType eAct);

    // Add new property handlers for CBlue acquisition
    int onAcquisitionMode(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onAcquisitionStart(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onAcquisitionStop(MM::PropertyBase* pProp, MM::ActionType eAct);
    int onAcquisitionAbort(MM::PropertyBase* pProp, MM::ActionType eAct);

	void createProperties();

	void updateImageDimensions();

private:
	bool			_initialized;
	std::string		_cameraName;
	CameraModel_C	_cameraModel;
	bool			_credTwo;
	bool			_credThree;
	bool			_cblueOne;
	double			_fpsTrigger;
	double			_maxExposure;
	double			_maxFps;
	double			_sensorTemp;
	double			_fps;
	long			_numImages;
	bool			_croppingEnabled;
	const char**	_listOfCameras;
	uint8_t			_nbCameras;
	FliThreadImp*	_refreshThread;
	callbackHandler	_callbackCtx;
	bool			_isCapturing;

	unsigned int _currentWidth;
	unsigned int _currentHeight;
	unsigned int _bytesPerPixel;
};

class FliThreadImp : public MMDeviceThreadBase
{
public:
	FliThreadImp(FirstLightImagingCameras* camera);
	~FliThreadImp();

	int svc();
	void exit();

private:
	bool mustExit();

private:
	MMThreadLock				_lock;
	bool						_exit;
	FirstLightImagingCameras* _camera;
};