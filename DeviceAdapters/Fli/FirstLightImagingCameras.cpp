///////////////////////////////////////////////////////////////////////////////
// FILE:          FirstLightImagingCameras.cpp
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

#include "FirstLightImagingCameras.h"

#ifdef __unix__
#include "unistd.h"
#endif

#include <cstdlib>
#include <iostream>
#include <chrono>


#include "FliSdk_utils.h"
#include "ModuleInterface.h"

//---------------------------------------------------------------
int roundUp(int numToRound, int multiple)
{
	if (multiple == 0)
		return numToRound;

	int remainder = numToRound % multiple;
	if (remainder == 0)
		return numToRound;

	if (remainder < multiple / 2)
		return numToRound - remainder;
	else
		return numToRound + multiple - remainder;
}

//---------------------------------------------------------------
MODULE_API void InitializeModuleData()
{
	RegisterDevice("FliSdk", MM::CameraDevice, "First Light Imaging camera");
}

//---------------------------------------------------------------
MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
	if (!deviceName)
		return 0;

	return new FirstLightImagingCameras(deviceName);
}

//---------------------------------------------------------------
MODULE_API void DeleteDevice(MM::Device* pDevice)
{
	delete pDevice;
}

//---------------------------------------------------------------
void onImageReceived(const uint8_t* image, void* ctx)
{
	FirstLightImagingCameras* context = static_cast<FirstLightImagingCameras*>(ctx);
	context->imageReceived(image);
}

//---------------------------------------------------------------
// FirstLightImagingCameras::FirstLightImagingCameras(std::string cameraName) :
// 	_initialized(false),
// 	_cameraName(cameraName),
// 	_cameraModel(undefined),
// 	_nbCameras(0),
// 	_credTwo(false),
// 	_credThree(false),
// 	_cblueOne(false),
// 	_isCapturing(false)
// {
// 	std::cout << "In FirstLightImagingCameras initialization" << std::endl;
// 	FliSdk_init();
// 	std::cout << "after FliSdk_init()" << std::endl;
// 	uint8_t nbGrabbers = 0;
// 	FliSdk_detectGrabbers(&nbGrabbers);
// 	std::cout << "after FliSdk_detectGrabbers(), nbGrabbers = " << std::to_string(nbGrabbers) << std::endl;

// 	if(nbGrabbers > 0)
// 	{
// 		_listOfCameras = FliSdk_detectCameras(&_nbCameras);
// 		std::cout << "after FliSdk_detectCameras(), nbCameras = " << std::to_string(_nbCameras) << std::endl;

// 		if(_nbCameras > 0)
// 		{
// 			FliSdk_setCamera(_listOfCameras[0]);
// 			FliSdk_setBufferSize(500);
// 			FliSdk_update();
// 			_cameraModel = FliSdk_getCameraModel();
// 			std::cout << "after FliSdk_getCameraModel(), _cameraModel = " << std::to_string(_cameraModel) << std::endl;

// 			if(_cameraModel == C_Red2)
// 			{
// 				_credTwo = true;
// 				_credThree = false;
// 				_cblueOne = false;
// 			}
// 			else if(_cameraModel == C_Red3)
// 			{
// 				_credTwo = false;
// 				_credThree = true;
// 				_cblueOne = false;
// 			}
// 			else if(_cameraModel == C_Blue1)
// 			{
// 				_credTwo = false;
// 				_credThree = false;
// 				_cblueOne = true;
// 			}

// 			//_callbackCtx = FliSdk_addCallbackNewImage(onImageReceived, 0, this);
// 			//FliSdk_start();
// 			//_refreshThread = new FliThreadImp(this);
// 			//_refreshThread->activate();
// 		}
// 	}

// 	createProperties();
// 	std::cout << "after createProperties()" << std::endl;
// }


FirstLightImagingCameras::FirstLightImagingCameras(std::string cameraName) :
	_initialized(false),
	_cameraName(cameraName),
	_cameraModel(undefined),
	_nbCameras(0),
	_credTwo(false),
	_credThree(false),
	_cblueOne(false),
	_isCapturing(false)
{
	auto start = std::chrono::high_resolution_clock::now();

	std::cout << "In FirstLightImagingCameras initialization" << std::endl;
	auto stepStart = start;

	// Initialize SDK
	FliSdk_init();
	auto stepEnd = std::chrono::high_resolution_clock::now();
	std::cout << "after FliSdk_init(), duration = " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(stepEnd - stepStart).count() << " ms" << std::endl;

	stepStart = stepEnd;

	// Detect grabbers
	uint8_t nbGrabbers = 0;
	FliSdk_detectGrabbers(&nbGrabbers);
	stepEnd = std::chrono::high_resolution_clock::now();
	std::cout << "after FliSdk_detectGrabbers(), nbGrabbers = " << std::to_string(nbGrabbers)
              << ", duration = " << std::chrono::duration_cast<std::chrono::milliseconds>(stepEnd - stepStart).count() << " ms" << std::endl;

	stepStart = stepEnd;

	if(nbGrabbers > 0)
	{
		// Detect cameras
		_listOfCameras = FliSdk_detectCameras(&_nbCameras);
		stepEnd = std::chrono::high_resolution_clock::now();
		std::cout << "after FliSdk_detectCameras(), nbCameras = " << std::to_string(_nbCameras)
                  << ", duration = " << std::chrono::duration_cast<std::chrono::milliseconds>(stepEnd - stepStart).count() << " ms" << std::endl;

		stepStart = stepEnd;

		if(_nbCameras > 0)
		{
			// Set camera
			FliSdk_setCamera(_listOfCameras[0]);
			stepEnd = std::chrono::high_resolution_clock::now();
			std::cout << "after FliSdk_setCamera(), _listOfCameras[0] = " << _listOfCameras[0]
                      << ", duration = " << std::chrono::duration_cast<std::chrono::milliseconds>(stepEnd - stepStart).count() << " ms" << std::endl;

			stepStart = stepEnd;

			FliSdk_setBufferSize(500);
			stepEnd = std::chrono::high_resolution_clock::now();
			std::cout << "after FliSdk_setBufferSize()"
                      << ", duration = " << std::chrono::duration_cast<std::chrono::milliseconds>(stepEnd - stepStart).count() << " ms" << std::endl;

			stepStart = stepEnd;

			FliSdk_update();
			stepEnd = std::chrono::high_resolution_clock::now();
			std::cout << "after FliSdk_update()"
                      << ", duration = " << std::chrono::duration_cast<std::chrono::milliseconds>(stepEnd - stepStart).count() << " ms" << std::endl;

			stepStart = stepEnd;

			_cameraModel = FliSdk_getCameraModel();
			stepEnd = std::chrono::high_resolution_clock::now();
			std::cout << "after FliSdk_getCameraModel(), _cameraModel = " << std::to_string(_cameraModel)
                      << ", duration = " << std::chrono::duration_cast<std::chrono::milliseconds>(stepEnd - stepStart).count() << " ms" << std::endl;

			stepStart = stepEnd;

			if(_cameraModel == C_Red2)
			{
				_credTwo = true;
			}
			else if(_cameraModel == C_Red3)
			{
				_credThree = true;
			}
			else if(_cameraModel == C_Blue1)
			{
				_cblueOne = true;
			}

 			_callbackCtx = FliSdk_addCallbackNewImage(onImageReceived, 0, this);
 			FliSdk_start();
 			_refreshThread = new FliThreadImp(this);
 			_refreshThread->activate();
		}
	}

	// Create properties
	//createProperties();
	//stepEnd = std::chrono::high_resolution_clock::now();
	//std::cout << "after createProperties(), duration = " 
 //             << std::chrono::duration_cast<std::chrono::milliseconds>(stepEnd - stepStart).count() << " ms" << std::endl;

	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Total initialization time = " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
}


//---------------------------------------------------------------
FirstLightImagingCameras::~FirstLightImagingCameras()
{
	_refreshThread->exit();
	_refreshThread->wait();
	delete _refreshThread;
	FliSdk_exit();
}

//---------------------------------------------------------------
void FirstLightImagingCameras::createProperties()
{
	CPropertyAction* pAct = nullptr;

	std::vector<std::string> boolvalues;
	boolvalues.push_back("0");
	boolvalues.push_back("1");

	CreateStringProperty("Camera Status", "", true, nullptr);

	pAct = new CPropertyAction(this, &FirstLightImagingCameras::onCameraChange);
	CreateStringProperty("Cameras", _listOfCameras[0], false, pAct);
	std::vector<std::string> values;
	for(int i = 0; i < _nbCameras; ++i)
		values.push_back(std::string(_listOfCameras[i]));
	SetAllowedValues("Cameras", values);

	pAct = new CPropertyAction(this, &FirstLightImagingCameras::onShutdown);
	CreateIntegerProperty("Shutdown", 0, false, pAct);
	SetAllowedValues("Shutdown", boolvalues);

	if (_credTwo || _credThree)
	{
		pAct = new CPropertyAction(this, &FirstLightImagingCameras::onMaxExposure);
		CreateFloatProperty("MaximumExposureMs", _maxExposure, true, pAct, false);

		pAct = new CPropertyAction(this, &FirstLightImagingCameras::onMaxFps);
		CreateFloatProperty("MaximumFps", _maxFps, true, pAct, false);

		pAct = new CPropertyAction(this, &FirstLightImagingCameras::onSetMaxExposure);
		CreateIntegerProperty("Set max exposure", 0, false, pAct);
		SetAllowedValues("Set max exposure", boolvalues);

		pAct = new CPropertyAction(this, &FirstLightImagingCameras::onSetMaxFps);
		CreateIntegerProperty("Set max fps", 0, false, pAct);
		SetAllowedValues("Set max fps", boolvalues);

		pAct = new CPropertyAction(this, &FirstLightImagingCameras::onBuildBias);
		CreateIntegerProperty("Build bias", 0, false, pAct);
		SetAllowedValues("Build bias", boolvalues);

		pAct = new CPropertyAction(this, &FirstLightImagingCameras::onApplyBias);
		CreateIntegerProperty("Apply bias", 0, false, pAct);
		SetAllowedValues("Apply bias", boolvalues);

		pAct = new CPropertyAction(this, &FirstLightImagingCameras::onFps);
		CreateFloatProperty("FPS", _fps, false, pAct, false);

		pAct = new CPropertyAction(this, &FirstLightImagingCameras::onSendCommand);
		CreateStringProperty("Send command", "", false, pAct);
	}

	if (_credTwo)
	{
		pAct = new CPropertyAction(this, &FirstLightImagingCameras::onApplySensorTemp);
		CreateFloatProperty("Set sensor temp", _sensorTemp, false, pAct);

		CreateFloatProperty("Sensor Temp", 0.0, true, nullptr, false);
	}
	else if (_credThree) {
		CreateFloatProperty("Set sensor temp", 0, true, nullptr);
	}
	else if (_cblueOne) {

		// Temperature Selector
        CreateProperty("Temperature Selector", "Sensor", MM::String, false, new CPropertyAction(this, &FirstLightImagingCameras::onTemperatureSelector));
		AddAllowedValue("Temperature Selector", "Sensor");
		AddAllowedValue("Temperature Selector", "CPU");
		AddAllowedValue("Temperature Selector", "Power");
		AddAllowedValue("Temperature Selector", "Frontend");
		AddAllowedValue("Temperature Selector", "Heatsink");
		AddAllowedValue("Temperature Selector", "Case");

        // Cooling Setpoint
        CreateProperty("Cooling Setpoint", "0", MM::Float, false, new CPropertyAction(this, &FirstLightImagingCameras::onCoolingSetpoint));

        // Frame Rate Properties
        CreateProperty("Frame Rate", "0", MM::Float, false, new CPropertyAction(this, &FirstLightImagingCameras::onFps));
        CreateProperty("Frame Rate Min", "0", MM::Float, true, new CPropertyAction(this, &FirstLightImagingCameras::onFrameRateMin));
        CreateProperty("Frame Rate Max", "0", MM::Float, true, new CPropertyAction(this, &FirstLightImagingCameras::onFrameRateMax));

        // Exposure Properties
        CreateProperty("Exposure Time (ms)", "0", MM::Float, false, new CPropertyAction(this, &FirstLightImagingCameras::onExposureTime));
        CreateProperty("Exposure Time Min", "0", MM::Float, true, new CPropertyAction(this, &FirstLightImagingCameras::onExposureTimeMin));
        CreateProperty("Exposure Time Max", "0", MM::Float, true, new CPropertyAction(this, &FirstLightImagingCameras::onExposureTimeMax));

        // Gain Control
        CreateProperty("Gain", "0", MM::Float, false, new CPropertyAction(this, &FirstLightImagingCameras::onGain));

        // Conversion Efficiency
		CreateProperty("Conversion Efficiency", "Low", MM::String, false, new CPropertyAction(this, &FirstLightImagingCameras::onConvEfficiency));
		AddAllowedValue("Conversion Efficiency", "Low");
		AddAllowedValue("Conversion Efficiency", "High");

        // Pixel Format
        CreateProperty("Pixel Format", "Mono8", MM::String, false, new CPropertyAction(this, &FirstLightImagingCameras::onPixelFormat));
        AddAllowedValue("Pixel Format", "Mono8");
        AddAllowedValue("Pixel Format", "Mono12");

        // Reverse X/Y
        CreateProperty("Reverse X", "0", MM::Integer, false, new CPropertyAction(this, &FirstLightImagingCameras::onReverseX));
		AddAllowedValue("Reverse X", "0"); // 0 for false
		AddAllowedValue("Reverse X", "1"); // 1 for true

		CreateProperty("Reverse Y", "0", MM::Integer, false, new CPropertyAction(this, &FirstLightImagingCameras::onReverseY));
		AddAllowedValue("Reverse Y", "0"); // 0 for false
		AddAllowedValue("Reverse Y", "1"); // 1 for true

        // Fan Mode
        CreateProperty("Fan Mode", "Auto", MM::String, false, new CPropertyAction(this, &FirstLightImagingCameras::onFanMode));
        AddAllowedValue("Fan Mode", "Auto");
        AddAllowedValue("Fan Mode", "Manual");

        // Device Status
        CreateProperty("Device Status", "", MM::String, true, new CPropertyAction(this, &FirstLightImagingCameras::onDeviceStatus));
	}
	

	pAct = new CPropertyAction(this, &FirstLightImagingCameras::onDetectCameras);
	CreateIntegerProperty("Detect cameras", 0, false, pAct);
	SetAllowedValues("Detect cameras", boolvalues);

	pAct = new CPropertyAction(this, &FirstLightImagingCameras::onBinning);
	CreateProperty(MM::g_Keyword_Binning, "1", MM::Integer, false, pAct);
}

// --------------------------------------------------------------
// CBlue1 functions
int FirstLightImagingCameras::onTemperatureSelector(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onTemperatureSelector"<< std::endl;
    if (_cblueOne)
    {
		if (eAct == MM::AfterSet)
        {
            std::string selector;
            pProp->Get(selector);

            DeviceTemperatureSelectorEnum tempSelector;
            if (selector == "Sensor")
                tempSelector = DeviceTemperatureSelector_Sensor;
            else if (selector == "CPU")
                tempSelector = DeviceTemperatureSelector_CPU;
            else if (selector == "Power")
                tempSelector = DeviceTemperatureSelector_Power;
            else if (selector == "Frontend")
                tempSelector = DeviceTemperatureSelector_Frontend;
            else if (selector == "Heatsink")
                tempSelector = DeviceTemperatureSelector_Heatsink;
            else if (selector == "Case")
                tempSelector = DeviceTemperatureSelector_Case;
            else
                return DEVICE_INVALID_PROPERTY_VALUE;

            Cblue1_setDeviceTemperatureSelector(tempSelector);
        }
		else if (eAct == MM::BeforeGet)
		{
			DeviceTemperatureSelectorEnum tempSelector;
			Cblue1_getDeviceTemperatureSelector(&tempSelector);

			std::string selector;
			switch (tempSelector)
			{
			case DeviceTemperatureSelector_Sensor:
				selector = "Sensor";
				break;
			case DeviceTemperatureSelector_CPU:
				selector = "CPU";
				break;
			case DeviceTemperatureSelector_Power:
				selector = "Power";
				break;
			case DeviceTemperatureSelector_Frontend:
				selector = "Frontend";
				break;
			case DeviceTemperatureSelector_Heatsink:
				selector = "Heatsink";
				break;
			case DeviceTemperatureSelector_Case:
				selector = "Case";
				break;
			default:
				return DEVICE_ERR; // Unknown value
			}

			pProp->Set(selector.c_str());
		}
	}
    return DEVICE_OK;
}

int FirstLightImagingCameras::onCoolingSetpoint(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onCoolingSetpoint"<< std::endl;
    if (_cblueOne)
    {
        if (eAct == MM::AfterSet)
        {
            double setpoint;
            pProp->Get(setpoint);
            Cblue1_setDeviceCoolingSetpoint(setpoint);
        }
        else if (eAct == MM::BeforeGet)
        {
            double setpoint;
            Cblue1_getDeviceCoolingSetpoint(&setpoint);
            pProp->Set(setpoint);
        }
    }
    return DEVICE_OK;
}


int FirstLightImagingCameras::onFrameRateMin(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onFrameRateMin"<< std::endl;
    if (_cblueOne && eAct == MM::BeforeGet)
    {
        double frameRateMin;
        Cblue1_getAcquisitionFrameRateMinReg(&frameRateMin);
        pProp->Set(frameRateMin);
    }
    return DEVICE_OK;
}

int FirstLightImagingCameras::onFrameRateMax(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onFrameRateMax"<< std::endl;
    if (_cblueOne && eAct == MM::BeforeGet)
    {
        double frameRateMax;
        Cblue1_getAcquisitionFrameRateMaxReg(&frameRateMax);
        pProp->Set(frameRateMax);
    }
    return DEVICE_OK;
}

int FirstLightImagingCameras::onExposureTime(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onExposureTime"<< std::endl;
    if (_cblueOne)
    {
        if (eAct == MM::AfterSet)
        {
            double exposure;
            pProp->Get(exposure);
            CblueSfnc_setExposureTime(exposure * 1000); // Convert ms to microseconds
        }
        else if (eAct == MM::BeforeGet)
        {
            double exposure;
            CblueSfnc_getExposureTime(&exposure);
            pProp->Set(exposure / 1000); // Convert microseconds to ms
        }
    }
    return DEVICE_OK;
}

int FirstLightImagingCameras::onExposureTimeMin(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onExposureTimeMin"<< std::endl;
    if (_cblueOne && eAct == MM::BeforeGet)
    {
        double exposureMin;
        Cblue1_getExposureTimeMinReg(&exposureMin);
        pProp->Set(exposureMin / 1000.0); // Convert microseconds to ms
    }
    return DEVICE_OK;
}

int FirstLightImagingCameras::onExposureTimeMax(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onExposureTimeMax"<< std::endl;
    if (_cblueOne && eAct == MM::BeforeGet)
    {
        double exposureMax;
        Cblue1_getExposureTimeMaxReg(&exposureMax);
        pProp->Set(exposureMax / 1000.0); // Convert microseconds to ms
    }
    return DEVICE_OK;
}

int FirstLightImagingCameras::onGain(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onGain" << std::endl;
    if (_cblueOne)
    {
        if (eAct == MM::AfterSet)
        {
            double gain;
            pProp->Get(gain);

            // Handle analog and digital gains separately
            if (gain <= 24)
            {
                CblueSfnc_setGainSelector(GainSelectorEnum::GainSelector_AnalogAll);
                CblueSfnc_setGain(gain);
            }
            else
            {
                CblueSfnc_setGainSelector(GainSelectorEnum::GainSelector_AnalogAll);
                CblueSfnc_setGain(24);
                CblueSfnc_setGainSelector(GainSelectorEnum::GainSelector_DigitalAll);
                CblueSfnc_setGain(gain - 24);
            }
        }
        else if (eAct == MM::BeforeGet)
        {
            double analogGain = 0, digitalGain = 0;
            CblueSfnc_setGainSelector(GainSelectorEnum::GainSelector_AnalogAll);
            CblueSfnc_getGain(&analogGain);
            CblueSfnc_setGainSelector(GainSelectorEnum::GainSelector_DigitalAll);
            CblueSfnc_getGain(&digitalGain);

            pProp->Set(analogGain + digitalGain);
        }
    }
    return DEVICE_OK;
}

int FirstLightImagingCameras::onConvEfficiency(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onConvEfficiency" << std::endl;
	if (_cblueOne)
    {
        if (eAct == MM::AfterSet)
        {
            std::string efficiency;
            pProp->Get(efficiency);

            ConversionEfficiencyEnum convEff;
            if (efficiency == "Low")
                convEff = ConversionEfficiency_Low;
            else if (efficiency == "High")
                convEff = ConversionEfficiency_High;
            else
                return DEVICE_INVALID_PROPERTY_VALUE;

            Cblue1_setConversionEfficiency(convEff);
        }
        else if (eAct == MM::BeforeGet)
        {
            ConversionEfficiencyEnum convEff;
            Cblue1_getConversionEfficiency(&convEff);

            std::string efficiency = (convEff == ConversionEfficiency_Low) ? "Low" : "High";
            pProp->Set(efficiency.c_str());
        }
    }
    return DEVICE_OK;
}

int FirstLightImagingCameras::onPixelFormat(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onPixelFormat" << std::endl;
    if (_cblueOne)
    {
        if (eAct == MM::AfterSet)
        {
            std::string format;
            pProp->Get(format);

            PixelFormatEnum pixelFormat = (format == "Mono8") ? PixelFormatEnum::PixelFormat_Mono8 : PixelFormatEnum::PixelFormat_Mono12;
            CblueSfnc_setPixelFormat(pixelFormat);
        }
        else if (eAct == MM::BeforeGet)
        {
            PixelFormatEnum pixelFormat;
            CblueSfnc_getPixelFormat(&pixelFormat);

            pProp->Set((pixelFormat == PixelFormatEnum::PixelFormat_Mono8) ? "Mono8" : "Mono12");
        }
    }
    return DEVICE_OK;
}

int FirstLightImagingCameras::onReverseX(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onReverseX" << std::endl;
    if (_cblueOne)
    {
        if (eAct == MM::AfterSet)
        {
            long reverseX;
            pProp->Get(reverseX);
            CblueSfnc_setReverseX(reverseX != 0); // Convert Integer to boolean
        }
        else if (eAct == MM::BeforeGet)
        {
            bool reverseX;
            CblueSfnc_getReverseX(&reverseX);
            pProp->Set(reverseX ? 1L : 0L); // Convert boolean to Integer
        }
    }
    return DEVICE_OK;
}

int FirstLightImagingCameras::onReverseY(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onReverseY" << std::endl;
    if (_cblueOne)
    {
        if (eAct == MM::AfterSet)
        {
            long reverseY;
            pProp->Get(reverseY);
            CblueSfnc_setReverseY(reverseY != 0); // Convert Integer to boolean
        }
        else if (eAct == MM::BeforeGet)
        {
            bool reverseY;
            CblueSfnc_getReverseY(&reverseY);
            pProp->Set(reverseY ? 1L : 0L); // Convert boolean to Integer
        }
    }
    return DEVICE_OK;
}


int FirstLightImagingCameras::onFanMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onFanMode" << std::endl;
    if (_cblueOne)
    {
        if (eAct == MM::AfterSet)
        {
            std::string mode;
            pProp->Get(mode);

            DeviceFanModeEnum fanMode = (mode == "Auto") ? DeviceFanModeEnum::DeviceFanMode_Automatic : DeviceFanModeEnum::DeviceFanMode_Manual;
            Cblue1_setDeviceFanMode(fanMode);
        }
        else if (eAct == MM::BeforeGet)
        {
            DeviceFanModeEnum fanMode;
            Cblue1_getDeviceFanMode(&fanMode);

            pProp->Set((fanMode == DeviceFanModeEnum::DeviceFanMode_Automatic) ? "Auto" : "Manual");
        }
    }
    return DEVICE_OK;
}

int FirstLightImagingCameras::onDeviceStatus(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onDeviceStatus" << std::endl;
    if (_cblueOne && eAct == MM::BeforeGet)
    {
        char status[256];
        Cblue1_getDeviceStatus(status);
        pProp->Set(status);
    }
    return DEVICE_OK;
}


//---------------------------------------------------------------
void FirstLightImagingCameras::imageReceived(const uint8_t* image)
{
	std::cout << "In imageReceived (before _isCapturing check)." << std::endl;
	if (!_isCapturing)
		return;

	std::cout << "In imageReceived" << std::endl;
	unsigned int w = GetImageWidth();
	unsigned int h = GetImageHeight();
	unsigned int b = GetImageBytesPerPixel();

	Metadata md;
	char label[MM::MaxStrLength];
	GetLabel(label);
	md.put(MM::g_Keyword_Metadata_CameraLabel, label);
	md.put(MM::g_Keyword_Metadata_ROI_X, CDeviceUtils::ConvertToString((long)w));
	md.put(MM::g_Keyword_Metadata_ROI_Y, CDeviceUtils::ConvertToString((long)h));

	MM::Core* core = GetCoreCallback();

	int ret = core->InsertImage(this, image, w, h, b, 1, md.Serialize().c_str(), false);
	if (ret == DEVICE_BUFFER_OVERFLOW)
	{
		// do not stop on overflow - just reset the buffer
		core->ClearImageBuffer(this);
		core->InsertImage(this, image, w, h, b, 1, md.Serialize().c_str(), false);
	}
}

//---------------------------------------------------------------
int FirstLightImagingCameras::Initialize()
{
	std::cout << "In Initialize" << std::endl;
	if (_initialized)
		return DEVICE_OK;

	_initialized = true;

	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::Shutdown()
{
	std::cout << "In Shutdown" << std::endl;
	_initialized = false;
	return DEVICE_OK;
}

//---------------------------------------------------------------
void FirstLightImagingCameras::GetName(char* name) const
{
	std::cout << "In GetName" << std::endl;
	CDeviceUtils::CopyLimitedString(name, _cameraName.c_str());
}

//---------------------------------------------------------------
long FirstLightImagingCameras::GetImageBufferSize() const
{
	std::cout << "In GetImageBufferSize" <<::std::endl;
	uint16_t width, height;
	FliSdk_getCurrentImageDimension(&width, &height);
	return width * height * 2;
}

//---------------------------------------------------------------
unsigned FirstLightImagingCameras::GetBitDepth() const
{
	std::cout << "In GetBitDepth" <<::std::endl;
	return 16;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::GetBinning() const
{
	std::cout << "In GetBinning" <<::std::endl;
	return 1;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::SetBinning(int binSize)
{
	std::cout << "In SetBinning" <<::std::endl;
	(void)binSize;
	return DEVICE_OK;
}

//---------------------------------------------------------------
void FirstLightImagingCameras::SetExposure(double exp_ms)
{
	std::cout << "In SetExposure" <<::std::endl;
	if (_credTwo)
		Cred2_setTint(exp_ms / 1000.0);
	else if (_credThree)
		Cred3_setTint(exp_ms / 1000.0);
	else if (_cblueOne)
		CblueSfnc_setExposureTime(exp_ms / 1000.0);
}

//---------------------------------------------------------------
double FirstLightImagingCameras::GetExposure() const
{
	std::cout << "In GetExposure" <<::std::endl;
	double tint;

	if (_credTwo)
		Cred2_getTint(&tint);
	else if (_credThree)
		Cred3_getTint(&tint);
	else if (_cblueOne)
		CblueSfnc_getExposureTime(&tint);
	else
		tint = 0;

	return tint*1000;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize)
{
	std::cout << "In SetROI" <<::std::endl;
	if (_credTwo || _credThree) {
		CroppingData_C cropping;
		cropping.col1 = roundUp(x, 32);
		cropping.col2 = roundUp(x + xSize, 32) - 1;
		cropping.row1 = roundUp(y, 4);
		cropping.row2 = roundUp(y + ySize, 4) - 1;
		FliSdk_setCroppingState(true, cropping);
	}
	else if (_cblueOne) {
			CblueSfnc_setOffsetX(roundUp(x, 16));
			CblueSfnc_setOffsetY(roundUp(y, 8));
			CblueSfnc_setWidth(roundUp(x+xSize, 16));
			CblueSfnc_setHeight(roundUp(y+ySize, 8));
	}
	_croppingEnabled = true;
	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize)
{
	std::cout << "In GetROI" <<::std::endl;
	if (!_croppingEnabled)
	{
		x = 0;
		xSize = GetImageWidth();
		y = 0;
		ySize = GetImageHeight();
	}
	else
	{
		if (_credTwo || _credThree) {
			CroppingData_C cropping;
			bool enabled;
			FliSdk_getCroppingState(&enabled, &cropping);
			x = cropping.col1;
			xSize = cropping.col2 - x;
			y = cropping.row1;
			ySize = cropping.row2 - y;
		}
		else if (_cblueOne) {
			int64_t offsetX, offsetY, width, height;
			CblueSfnc_getOffsetX(&offsetX);
			CblueSfnc_getOffsetY(&offsetY);
			CblueSfnc_getWidth(&width);
			CblueSfnc_getHeight(&height);

			x = static_cast<unsigned>(offsetX);
			y = static_cast<unsigned>(offsetY);
			xSize = static_cast<unsigned>(width);
			ySize = static_cast<unsigned>(height);
		}
	}
	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::ClearROI()
{
	std::cout << "In ClearROI" <<::std::endl;
	if (_credTwo || _credThree) {
		CroppingData_C cropping;
		cropping.col1 = 0;
		cropping.col2 = 0;
		cropping.row1 = 0;
		cropping.row2 = 0;
		FliSdk_setCroppingState(false, cropping);
	}
	else if (_cblueOne) {
		// Reset ROI to full image dimensions
		int64_t widthMax, heightMax;
		CblueSfnc_getWidthMax(&widthMax);
		CblueSfnc_getHeightMax(&heightMax);

		CblueSfnc_setOffsetX(0);
		CblueSfnc_setOffsetY(0);
		CblueSfnc_setWidth(widthMax);
		CblueSfnc_setHeight(heightMax);
	}
	_croppingEnabled = false;
	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::IsExposureSequenceable(bool& isSequenceable) const
{
	(void)isSequenceable;
	return 0;
}

//---------------------------------------------------------------
const unsigned char* FirstLightImagingCameras::GetImageBuffer()
{
	std::cout << "In GetImageBuffer" <<::std::endl;
	return FliSdk_getRawImage(-1);
}

//---------------------------------------------------------------
unsigned FirstLightImagingCameras::GetImageWidth() const
{
	std::cout << "In GetImageWidth" <<::std::endl;
	uint16_t width_;
	if (_credTwo || _credThree) 
    {
        uint16_t width, height;
        FliSdk_getCurrentImageDimension(&width, &height);
        width_ = width;
    } 
    else if (_cblueOne) 
    {
        int64_t width;
        CblueSfnc_getWidth(&width);

		// Safely cast after checking range
        if (width >= 0 && width <= 65535) // Hardcoded max value for uint16_t
        {
            width_ = static_cast<uint16_t>(width);
        } 
        else 
        {
            std::cerr << "Width value out of range for uint16_t: " << width << std::endl;
            width_ = 0; // Fallback to 0 or handle error appropriately
        }
    }
	return width_;
}

//---------------------------------------------------------------
unsigned FirstLightImagingCameras::GetImageHeight() const
{
	std::cout << "In GetImageHeight" <<::std::endl;
	uint16_t height_;
	if (_credTwo || _credThree) 
    {
        uint16_t width, height;
        FliSdk_getCurrentImageDimension(&width, &height);
        height_ = height;
    } 
    else if (_cblueOne) 
    {
        int64_t height;
        CblueSfnc_getHeight(&height);

		// Safely cast after checking range
        if (height >= 0 && height <= 65535) // Hardcoded max value for uint16_t
        {
            height_ = static_cast<uint16_t>(height);
        } 
        else 
        {
            std::cerr << "Width value out of range for uint16_t: " << height << std::endl;
            height_ = 0; // Fallback to 0 or handle error appropriately
        }
    }
	return height_;
}

//---------------------------------------------------------------
unsigned FirstLightImagingCameras::GetImageBytesPerPixel() const
{
	return 2;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::SnapImage()
{
	std::cout << "In SnapImage" <<::std::endl;
	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::StartSequenceAcquisition(long /*numImages*/, double /*interval_ms*/, bool /*stopOnOverflow*/)
{
	std::cout << "In StartSequenceAcquisition" <<::std::endl;
	_isCapturing = true;
	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::StopSequenceAcquisition()
{
	std::cout << "In StopSequenceAcquisition" <<::std::endl;
	_isCapturing = false;
	return DEVICE_OK;
}

//---------------------------------------------------------------
void FirstLightImagingCameras::OnThreadExiting() throw()
{
	std::cout << "In OnThreadExiting" <<::std::endl;
}

//---------------------------------------------------------------
bool FirstLightImagingCameras::IsCapturing()
{
	std::cout << "In isCapturing" <<::std::endl;
	return _isCapturing;
}

//---------------------------------------------------------------
void FirstLightImagingCameras::refreshValues()
{
	std::cout << "In refreshValues" <<::std::endl;
	if (_credTwo)
	{
		double val;
		Cred2_getTempSnake(&val);
		OnPropertyChanged("Sensor Temp", std::to_string((long double)val).c_str());
		double consigne;
		Cred2_getTempSnakeSetPoint(&consigne);
		OnPropertyChanged("Set sensor temp", std::to_string((long double)consigne).c_str());
		char status[200];
		char diag[200];
		FliCamera_getStatusDetailed(status, diag);
		std::string s;
		s.append(status);
		s.append("-");
		s.append(diag);
		OnPropertyChanged("Camera Status", s.c_str());
	}
	else if (_credThree)
	{
		char status[200];
		char diag[200];
		FliCamera_getStatusDetailed(status, diag);
		std::string s;
		s.append(status);
		s.append("-");
		s.append(diag);
		OnPropertyChanged("Camera Status", s.c_str());
	}
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onMaxExposure(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onMaxExposure" <<::std::endl;
	if (eAct == MM::BeforeGet)
	{
		double tintMin;
		if (_credTwo)
			Cred2_getTintRange(&tintMin, &_maxExposure);
		else if (_credThree)
			Cred3_getTintRange(&tintMin, &_maxExposure);

		pProp->Set(_maxExposure *1000);
	}
	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onMaxFps(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onMaxFps" <<::std::endl;
	if (eAct == MM::BeforeGet)
	{
		FliCamera_getFpsMax(&_maxFps);
		pProp->Set(_maxFps);
	}
	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onFps(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onFps" <<::std::endl;
	if (_credTwo || _credThree) {
		if (eAct == MM::BeforeGet)
		{
			FliCamera_getFps(&_fps);
			pProp->Set(_fps);
		}
		else if (eAct == MM::AfterSet)
		{
			pProp->Get(_fps);
			FliCamera_setFps(_fps);
		}
	}
    if (_cblueOne)
    {
        if (eAct == MM::AfterSet)
        {
            double fps;
            pProp->Get(fps);
            CblueSfnc_setAcquisitionFrameRate(fps);
        }
        else if (eAct == MM::BeforeGet)
        {
            double fps;
            CblueSfnc_getAcquisitionFrameRate(&fps);
            pProp->Set(fps);
        }
    }
	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onCameraChange(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onCameraChange" << std::endl;
	if (eAct == MM::AfterSet)
	{
		std::string camera;
		pProp->Get(camera);

		FliSdk_setCamera(camera.c_str());
		FliSdk_update();

		_cameraModel = FliSdk_getCameraModel();

		if(_cameraModel == C_Red2)
		{
			_credTwo = true;
			_credThree = false;
			_cblueOne = false;
		}
		else if(_cameraModel == C_Red3)
		{
			_credTwo = false;
			_credThree = true;
			_cblueOne = false;
		}
		else if(_cameraModel == C_Blue1)
		{
			_credTwo = false;
			_credThree = false;
			_cblueOne = true;
		}
	}

	return 0;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onDetectCameras(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onDetectCameras" << std::endl;
	if (eAct == MM::AfterSet)
	{
		std::string detect;
		pProp->Get(detect);

		_refreshThread->exit();
		_refreshThread->wait();
		delete _refreshThread;
		FliSdk_stop();

		if (detect == "1")
		{
			_credTwo = false;
			_credThree = false;
			uint8_t nbGrabbers = 0;
			FliSdk_detectGrabbers(&nbGrabbers);

			if(nbGrabbers > 0)
			{
				_nbCameras = 0;
				_listOfCameras = FliSdk_detectCameras(&_nbCameras);

				if(_nbCameras > 0)
				{
					FliSdk_setCamera(_listOfCameras[0]);
					FliSdk_update();
					_cameraModel = FliSdk_getCameraModel();

					if(_cameraModel == C_Red2)
					{
						_credTwo = true;
						_credThree = false;
						_cblueOne = false;
					}
					else if(_cameraModel == C_Red3)
					{
						_credTwo = false;
						_credThree = true;
						_cblueOne = false;
					}
					else if(_cameraModel == C_Blue1)
					{
						_credTwo = false;
						_credThree = false;
						_cblueOne = true;
					}

					FliSdk_start();
					_refreshThread = new FliThreadImp(this);
					_refreshThread->activate();
				}
			}

			createProperties();
		}
	}
	else if (eAct == MM::BeforeGet)
	{
		double enabled = 0;
		pProp->Set(enabled);
	}

	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onSendCommand(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onSendCommand" << std::endl;
	static char response[200];
	if (eAct == MM::AfterSet)
	{
		std::string command;
		pProp->Get(command);
		command.append("\n");
		FliCamera_sendCommand(command.c_str(), response);
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(response);
	}

	return DEVICE_OK;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onBinning(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onBinning" << std::endl;
	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
		ret = DEVICE_OK;
		break;
	case MM::BeforeGet:
	{
		ret = DEVICE_OK;
		double bin = 2;
		pProp->Set(bin);
		break;
	}
	default:
		break;
	}
	return ret;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onSetMaxExposure(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onSetMaxExposure" << std::endl;
	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
		ret = DEVICE_OK;
		if (_credTwo)
			Cred2_setTint(_maxExposure);
		else if (_credThree)
			Cred3_setTint(_maxExposure);
		else if (_cblueOne)
			Cred3_setTint(_maxExposure);
		break;
	case MM::BeforeGet:
	{
		ret = DEVICE_OK;
		double enabled = 0;
		pProp->Set(enabled);
		break;
	}
	default:
		break;
	}
	return ret;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onSetMaxFps(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onSetMaxFps" << std::endl;
	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
		ret = DEVICE_OK;
		FliCamera_setFps(_maxFps);
		break;
	case MM::BeforeGet:
	{
		ret = DEVICE_OK;
		double enabled = 0;
		pProp->Set(enabled);
		break;
	}
	default:
		break;
	}
	return ret;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onBuildBias(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onBuildBias" << std::endl;
	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
		ret = DEVICE_OK;
		FliCamera_buildBias();
		break;
	case MM::BeforeGet:
	{
		ret = DEVICE_OK;
		double enabled = 0;
		pProp->Set(enabled);
		break;
	}
	default:
		break;
	}
	return ret;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onApplyBias(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onApplyBias" << std::endl;
	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		ret = DEVICE_OK;
		double val;
		pProp->Get(val);
		bool enabled = val == 0 ? false : true;
		FliCamera_enableBias(enabled);
		break;
	}
	case MM::BeforeGet:
	{
		ret = DEVICE_OK;
		bool enabled = false;
		FliCamera_getBiasState(&enabled);
		pProp->Set((double)enabled);
		break;
	}
	default:
		break;
	}
	return ret;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onApplySensorTemp(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onApplySensorTemp" << std::endl;
	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		ret = DEVICE_OK;
		double val;
		pProp->Get(val);
		Cred2_setSensorTemp(val);
		break;
	}
	case MM::BeforeGet:
	{
		ret = DEVICE_OK;
		break;
	}
	default:
		break;
	}
	return ret;
}

//---------------------------------------------------------------
int FirstLightImagingCameras::onShutdown(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	std::cout << "In onShutdown" << std::endl;
	int ret = DEVICE_ERR;
	switch (eAct)
	{
	case MM::AfterSet:
	{
		ret = DEVICE_OK;
		double val;
		pProp->Get(val);
		FliCamera_shutDown();
		break;
	}
	case MM::BeforeGet:
	{
		ret = DEVICE_OK;
		break;
	}
	default:
		break;
	}
	return ret;
}

////---------------------------------------------------------------
FliThreadImp::FliThreadImp(FirstLightImagingCameras* camera) : _camera(camera), _exit(false)
{

}

//---------------------------------------------------------------
FliThreadImp::~FliThreadImp()
{

}

//---------------------------------------------------------------
void FliThreadImp::exit()
{
	MMThreadGuard(this->_lock);
	_exit = true;
}

//---------------------------------------------------------------
bool FliThreadImp::mustExit()
{
	MMThreadGuard(this->_lock);
	return _exit;
}

//---------------------------------------------------------------
int FliThreadImp::svc()
{
	while(!mustExit())
	{
		// _camera->refreshValues();
		CDeviceUtils::SleepMs(1000);
	}

	return 0;
}