//============ Copyright (c) Valve Corporation, All rights reserved. ============

#pragma comment(lib, "lib/deca_sdk")

#include "headers/openvr_driver.h"
#include "include/deca/move.h"
#include "driverlog.h"

#include <vector>
#include <thread>
#include <chrono>
#include <math.h>
#include "vector3.h"
#include "basis.h"
#include "quaternion.h"
#include <string>   
#include <iostream>
#include <cstdio>
//#if defined( _WINDOWS )
//#include <windows.h>
//#endif

using namespace vr;


#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
#define HMD_DLL_IMPORT extern "C" __declspec( dllimport )
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define HMD_DLL_IMPORT extern "C" 
#else
#error "Unsupported Platform."
#endif


inline HmdQuaternion_t HmdQuaternion_Init(double w, double x, double y, double z)
{
	HmdQuaternion_t quat;
	quat.w = w;
	quat.x = x;
	quat.y = y;
	quat.z = z;
	return quat;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class DecaMoveVirtualTrackerDriver : public vr::ITrackedDeviceServerDriver
{
private:
	deca_move_callbacks callbacks = {};
	deca_move deca_move = nullptr;
public:

	struct Context {
		deca_move_state state = kDecaMoveStateClosed;
		bool TrackingStarted = false;
		Vector3 Position;
		Quat Orientation;
		deca_move_feedback feedback;
		float battery_level;
	} context;


	DecaMoveVirtualTrackerDriver()
	{
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
		m_propertyHandle = vr::k_ulInvalidPropertyContainer;

		
		m_sSerialNumber = "VirtualTracker01";
		m_sModelNumber = "DecaMoveVirtualTracker";
	}

	virtual ~DecaMoveVirtualTrackerDriver()
	{
	}


	virtual EVRInitError Activate( vr::TrackedDeviceIndex_t unObjectId )
	{
		//Create the virtual Vive tracker

		m_unObjectId = unObjectId;
		m_propertyHandle = vr::VRProperties()->TrackedDeviceToPropertyContainer( m_unObjectId );
		
		//vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, Prop_ModelNumber_String, m_sModelNumber.c_str() );
		//vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, Prop_RenderModelName_String, "arrow");

		//vr::VRProperties()->SetStringProperty(m_propertyHandle, Prop_ModelNumber_String, "Vive Tracker Pro MV");
		//vr::VRProperties()->SetStringProperty(m_propertyHandle, Prop_RenderModelName_String, "{htc}vr_tracker_vive_1_0");
		//vr::VRProperties()->SetInt32Property(m_propertyHandle, Prop_DeviceClass_Int32, TrackedDeviceClass_GenericTracker);
		//vr::VRProperties()->SetStringProperty(m_propertyHandle, Prop_ControllerType_String, "vive_tracker_waist");

		//// return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
		//vr::VRProperties()->SetUint64Property( m_propertyHandle, Prop_CurrentUniverseId_Uint64, 2);

		//// avoid "not fullscreen" warnings from vrmonitor
		//vr::VRProperties()->SetBoolProperty( m_propertyHandle, Prop_IsOnDesktop_Bool, false);

		////even though we won't ever track we want to pretend to be the right hand so binding will work as expected
		//vr::VRProperties()->SetInt32Property( m_propertyHandle, Prop_ControllerRoleHint_Int32, TrackedControllerRole_OptOut);

		std::string l_registeredType("htc/vive_tracker");
		l_registeredType.append(m_sSerialNumber);
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_RegisteredDeviceType_String, l_registeredType.c_str());

		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_TrackingSystemName_String, "lighthouse");
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ModelNumber_String, "Vive Tracker Pro MV");
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_SerialNumber_String, m_sSerialNumber.c_str()); // Changed
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_RenderModelName_String, "{htc}vr_tracker_vive_1_0");
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_WillDriftInYaw_Bool, false);
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ManufacturerName_String, "HTC");
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_TrackingFirmwareVersion_String, "1541800000 RUNNER-WATCHMAN$runner-watchman@runner-watchman 2018-01-01 FPGA 512(2.56/0/0) BL 0 VRC 1541800000 Radio 1518800000"); // Changed
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_HardwareRevision_String, "product 128 rev 2.5.6 lot 2000/0/0 0"); // Changed
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ConnectedWirelessDongle_String, "D0000BE000"); // Changed
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_DeviceIsWireless_Bool, true);
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_DeviceIsCharging_Bool, false);
		vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_DeviceBatteryPercentage_Float, 1.f); // Always charged

		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_Firmware_UpdateAvailable_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_Firmware_ManualUpdate_Bool, false);
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_Firmware_ManualUpdateURL_String, "https://developer.valvesoftware.com/wiki/SteamVR/HowTo_Update_Firmware");
		vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_HardwareRevision_Uint64, 2214720000); // Changed
		vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_FirmwareVersion_Uint64, 1541800000); // Changed
		vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_FPGAVersion_Uint64, 512); // Changed
		vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_VRCVersion_Uint64, 1514800000); // Changed
		vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_RadioVersion_Uint64, 1518800000); // Changed
		vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_DongleVersion_Uint64, 8933539758); // Changed, based on vr::Prop_ConnectedWirelessDongle_String above
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_DeviceProvidesBatteryStatus_Bool, true);
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_DeviceCanPowerOff_Bool, true);
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_Firmware_ProgrammingTarget_String, m_sSerialNumber.c_str());
		vr::VRProperties()->SetInt32Property(m_propertyHandle, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_GenericTracker);
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_Firmware_ForceUpdateRequired_Bool, false);
		//vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_ParentDriver_Uint64, 8589934597); // Strange value from dump
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ResourceRoot_String, "htc");

		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_InputProfilePath_String, "{htc}/input/vive_tracker_profile.json");
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_Identifiable_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_Firmware_RemindUpdate_Bool, false);
		vr::VRProperties()->SetInt32Property(m_propertyHandle, vr::Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_Invalid);

		vr::VRProperties()->SetInt32Property(m_propertyHandle, vr::Prop_ControllerHandSelectionPriority_Int32, -1);
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceOff_String, "{htc}/icons/tracker_status_off.png");
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceSearching_String, "{htc}/icons/tracker_status_searching.gif");
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{htc}/icons/tracker_status_searching_alert.gif");
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceReady_String, "{htc}/icons/tracker_status_ready.png");
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{htc}/icons/tracker_status_ready_alert.png");
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceNotReady_String, "{htc}/icons/tracker_status_error.png");
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceStandby_String, "{htc}/icons/tracker_status_standby.png");
		vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceAlertLow_String, "{htc}/icons/tracker_status_ready_low.png");
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasDisplayComponent_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasCameraComponent_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasDriverDirectModeComponent_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasVirtualDisplayComponent_Bool, false);

		callbacks.orientation_update_cb = [](deca_move_quaternion quat, deca_move_accuracy accuracy, float yawCalibration, void* userData) {
			((Context*)userData)->Orientation = Quat(quat.x, quat.y, quat.z, quat.w);
			((Context*)userData)->TrackingStarted = true;
			
		};
		
		callbacks.position_update_cb = [](float position_x, float position_y, float position_z, void* userData) {
			((Context*)userData)->Position.x = position_x;
			((Context*)userData)->Position.y = position_y;
			((Context*)userData)->Position.z = position_z;
		};
		  
 
		callbacks.state_update_cb = [](deca_move_state state, void* userData) {
			((Context*)userData)->state = state;
		
		};
		callbacks.battery_update_cb = [](float charge, void* userData){
			((Context*)userData)->battery_level = charge;
		};

		callbacks.user_data = &context;

		deca_move_env_desc envDesc = {};

		if (deca_move == nullptr)
			auto status = decaMoveInit(envDesc, callbacks, &deca_move);



		return VRInitError_None;
	}


	void RunServer() {
	}

	virtual void Deactivate()
	{
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
		decaMoveRelease(deca_move);
	}

	virtual void EnterStandby()
	{
	}

	void *GetComponent( const char *pchComponentNameAndVersion )
	{
		// override this to add a component to a driver
		return NULL;
	}

	virtual void PowerOff()
	{
	}

	/** debug request from a client */
	virtual void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize )
	{
		if ( unResponseBufferSize >= 1 )
			pchResponseBuffer[0] = 0;
	}
	
//	ofstream outdata; // outdata is like cin
//	void AddLog()
//	{
//
//		outdata.open("d:\dmovevtracker.log"); // opens the file
//		if (!outdata) { // file couldn't be opened
//			cerr << "Error: file could not be opened" << endl;
//			exit(1);
//		}
//	}
	double X_Axis_Offset = 0;
	double player_hip_height = -0.73; // this is in metres??

	Vector3 offset_global = Vector3(0.0, 0.0, 0.0);
	Vector3 offset_local_device = Vector3(0.0, 0.0, 0.0);
	Vector3 offset_local_tracker = Vector3(0.0, player_hip_height,0.0);

	Vector3 local_rot_euler = Vector3(X_Axis_Offset, 0.0, 0.0);
	Vector3 global_rot_euler = Vector3(0.0, 0.0, 0.0);

	virtual DriverPose_t GetPose()
	{

		int anchor_device_id = 0;

		DriverPose_t pose = { 0 };
		pose.result = context.TrackingStarted ? TrackingResult_Running_OK : TrackingResult_Uninitialized;
		pose.poseIsValid = true;
		pose.deviceIsConnected = true;


		Basis offset_basis;

		TrackedDevicePose_t poses[64];
		uint32_t arraySize = 64; //This is max amount of tracked devices, so should always be safe length.
		vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, &poses[0], arraySize);
		
		//if (poses) {
			TrackedDevicePose_t anchor = poses[anchor_device_id];
			for (int i = 0; i < 3; i++) {
				pose.vecPosition[i] = anchor.mDeviceToAbsoluteTracking.m[i][3];
			}

			offset_basis.set(
				anchor.mDeviceToAbsoluteTracking.m[0][0],
				anchor.mDeviceToAbsoluteTracking.m[0][1],
				anchor.mDeviceToAbsoluteTracking.m[0][2],
				anchor.mDeviceToAbsoluteTracking.m[1][0],
				anchor.mDeviceToAbsoluteTracking.m[1][1],
				anchor.mDeviceToAbsoluteTracking.m[1][2],
				anchor.mDeviceToAbsoluteTracking.m[2][0],
				anchor.mDeviceToAbsoluteTracking.m[2][1],
				anchor.mDeviceToAbsoluteTracking.m[2][2]
			);
		/*}
		else {
			offset_basis.set_euler(Vector3());
			for (int i = 0; i < 3; i++) {
				pose.vecPosition[i] = 0.0;
				pose.vecVelocity[i] = 0.0;
			}
		}*/
		DriverLog(("Button:" + std::to_string((int)context.feedback)).c_str());
		pose.qWorldFromDriverRotation = quaternion::init(0, 0, 0, 1);
		pose.qDriverFromHeadRotation = quaternion::init(0, 0, 0, 1);
		
		Quat quat = context.Orientation;
		if (context.feedback == kDecaMoveFeedbackSingleClick)
		{
			axistoggle++;
			if (axistoggle > 2)
			{
				axistoggle = 0;

			}
			//X_Axis_Offset += 0.05; 
			context.feedback = kDecaMoveFeedbackLeavingSleep;
		}
		quat = Quat(Vector3(1, 0, 0), -Math_PI / 2.0) * quat;

		
		quat = ProcessRawDecaMoveQuat(quat);
		//z is deca light forward or back

		////quat = Quat(global_rot_euler) * quat;
		////quat = quat * Quat(local_rot_euler);

		pose.qRotation = quaternion::from_Quat(quat);

		Basis final_tracker_basis = Basis(quat);
	//	Basis last_basis = final_tracker_basis;

		for (int i = 0; i < 3; i++)
		{
			pose.vecPosition[i] += offset_global.get_axis(i);
			pose.vecPosition[i] += offset_basis.xform(offset_local_device).get_axis(i);
			pose.vecPosition[i] += final_tracker_basis.xform(offset_local_tracker).get_axis(i);
			//pose.vecPosition[i] += context.Position.get_axis(i);
		}

		return pose;
	}

	int axistoggle = 1;
	//Turn the raw quat from the deca move into something aligned the player
	Quat ProcessRawDecaMoveQuat(Quat quat)
	{

		//remove every axis except for rotation
		Vector3 euler = quat.get_euler();
		//Vector3 normalized = euler.normalized();
		
		for (int i = 0; i < 3; i++)
		{
			if (i != axistoggle)
			{
				euler[i] = 0;
			}
		}
		
	////	float rotation = (float)atan2(euler.y, euler.x);
	////
	////	float x = cos(rotation);
	////	float y = sin(rotation);

		//DriverLog(("Rotation: " + std::to_string(rotation) + " X: " + std::to_string(x) + " Y: " + std::to_string(y)).c_str());

	////	Vector3 newEuler = Vector3(x, y, 0);
	////
	////	quat = Quat(newEuler);

	//	euler.set_axis(Vector3::Axis::AXIS_Z, 0);


		//Zee CHANGES## flipped eulers becuase they were wrong.??.. but sometimes orientation is still off, noticably y hip movement is off
	//	euler.x = Math_PI * 2 - euler.x;
	//	euler.z = Math_PI * 2 - euler.z;
		// END



		DriverLog(("X: " + std::to_string(euler.x) + " Y: " + std::to_string(euler.y) + " Z: " + std::to_string(euler.z)).c_str());
		return Quat(euler);
	}
	
	void RunFrame()
	{
		// In a real driver, this should happen from some pose tracking thread.
		// The RunFrame interval is unspecified and can be very irregular if some other
		// driver blocks it for some periodic task.
		if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid)
		{
			vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(DriverPose_t));
			
			
		}
	}

	void ProcessEvent( const vr::VREvent_t & vrEvent )
	{

	}

	std::string GetSerialNumber() const { return m_sSerialNumber; }

private:
	vr::TrackedDeviceIndex_t m_unObjectId;
	vr::PropertyContainerHandle_t m_propertyHandle;

	std::string m_sSerialNumber;
	std::string m_sModelNumber;


};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class ServerDriver: public IServerTrackedDeviceProvider
{
public:
	virtual EVRInitError Init( vr::IVRDriverContext *pDriverContext ) ;
	virtual void Cleanup() ;
	virtual const char * const *GetInterfaceVersions() { return vr::k_InterfaceVersions; }
	virtual void RunFrame() ;
	virtual bool ShouldBlockStandbyMode()  { return false; }
	virtual void EnterStandby()  {}
	virtual void LeaveStandby()  {}

private:
	DecaMoveVirtualTrackerDriver* m_tracker_hip = nullptr;
};

ServerDriver g_serverDriverNull;

EVRInitError ServerDriver::Init( vr::IVRDriverContext *pDriverContext )
{
	//Driver INIT?
	VR_INIT_SERVER_DRIVER_CONTEXT( pDriverContext );
	InitDriverLog( vr::VRDriverLog() );

	m_tracker_hip = new DecaMoveVirtualTrackerDriver();
	vr::VRServerDriverHost()->TrackedDeviceAdded(m_tracker_hip->GetSerialNumber().c_str(), vr::TrackedDeviceClass_GenericTracker, m_tracker_hip);

	return VRInitError_None;
}

void ServerDriver::Cleanup() 
{
	CleanupDriverLog();
	delete m_tracker_hip;
	m_tracker_hip = NULL;
}


void ServerDriver::RunFrame()
{
	
	if (m_tracker_hip)
		m_tracker_hip->RunFrame();

	vr::VREvent_t vrEvent;
	while ( vr::VRServerDriverHost()->PollNextEvent( &vrEvent, sizeof( vrEvent ) ) )
	{
		if (m_tracker_hip)
			m_tracker_hip->ProcessEvent(vrEvent);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
HMD_DLL_EXPORT void *HmdDriverFactory( const char *pInterfaceName, int *pReturnCode )
{
	if( 0 == strcmp( IServerTrackedDeviceProvider_Version, pInterfaceName ) )
	{
		return &g_serverDriverNull;
	}

	if( pReturnCode )
		*pReturnCode = VRInitError_Init_InterfaceNotFound;

	return NULL;
}
