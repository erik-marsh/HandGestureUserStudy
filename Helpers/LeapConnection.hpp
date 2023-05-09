#pragma once

#include <string>

#include "../LeapSDK/include/LeapC.h"

#define WIN32_LEAN_AND_MEAN
#define CloseWindow __WIN32_CLOSE_WINDOW  // name conflict with raylib
#define ShowCursor __WIN32_SHOW_CURSOR    // name conflict with raylib
#include <Windows.h>
#undef CloseWindow
#undef ShowCursor
#undef DrawText  // name conflict with raylib
#include <process.h>

namespace Helpers
{

#ifdef _DEBUG
constexpr bool ENABLE_DEBUG_PRINT = true;
#else
constexpr bool ENABLE_DEBUG_PRINT = false;
#endif

void DebugPrint(const std::string &message);

std::string GetEnumString(eLeapRS res);

class LeapConnection
{
   public:
    LeapConnection();
    virtual ~LeapConnection();

    bool IsConnected() const;
    LEAP_TRACKING_EVENT *GetFrame() const;
    LEAP_DEVICE_INFO *GetDeviceProperties() const;

   private:
    static LeapConnection *m_instance;

    static bool m_isConnected;
    static volatile bool m_isRunning;
    static LEAP_CONNECTION m_connection;
    static LEAP_TRACKING_EVENT *m_lastFrame;
    static LEAP_DEVICE_INFO *m_lastDevice;
    static HANDLE m_pollingThread;
    static CRITICAL_SECTION m_dataLock;

    static void SetDevice(const LEAP_DEVICE_INFO *deviceProps);
    static void SetFrame(const LEAP_TRACKING_EVENT *frame);

    static void MessageLoop(void *unused);

    static void OnConnected(const LEAP_CONNECTION_EVENT *connection_event);
    static void OnConnectionLost(const LEAP_CONNECTION_LOST_EVENT *connection_lost_event);
    static void OnDeviceFound(const LEAP_DEVICE_EVENT *device_event);
    static void OnDeviceLost(const LEAP_DEVICE_EVENT *device_event);
    static void OnDeviceFailure(const LEAP_DEVICE_FAILURE_EVENT *device_failure_event);
    static void OnPolicy(const LEAP_POLICY_EVENT *policy_event);
    static void OnFrame(const LEAP_TRACKING_EVENT *tracking_event);
    static void OnLogMessage(const LEAP_LOG_EVENT *log_event);
    static void OnLogMessages(const LEAP_LOG_EVENTS *log_events);
    static void OnConfigChange(const LEAP_CONFIG_CHANGE_EVENT *config_change_event);
    static void OnConfigResponse(const LEAP_CONFIG_RESPONSE_EVENT *config_response_event);
    static void OnImage(const LEAP_IMAGE_EVENT *image_event);
    static void OnPointMappingChange(
        const LEAP_POINT_MAPPING_CHANGE_EVENT *point_mapping_change_event);
    static void OnHeadPose(const LEAP_HEAD_POSE_EVENT *head_pose_event);
    static void OnIMU(const LEAP_IMU_EVENT *imu_event);
    static void OnTrackingMode(const LEAP_TRACKING_MODE_EVENT *mode_event);
};

}  // namespace Helpers