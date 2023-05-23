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

/// @brief If _DEBUG is defined, Helpers::DebugPrint will log to stdout.
///        Otherwise, Helpers::DebugPrint is a noop.
constexpr bool ENABLE_DEBUG_PRINT =
#ifdef _DEBUG
    true;
#else
    false;
#endif

/// @brief Used for logging messages recieved by the Leap Motion runtime to stdout.
void DebugPrint(const std::string &message);

/// @brief Converts a eLeapRS enum to a human-readable string.
std::string GetEnumString(eLeapRS res);

/// @brief Singleton that encapsulates a connection to a Leap Motion device.
class LeapConnection
{
   public:
    /// @brief RAII initializer for a Leap Motion connection.
    LeapConnection();

    /// @brief RAII cleanup for a Leap Motion connection.
    virtual ~LeapConnection();

    /// @brief Is the connection to the device valid?
    bool IsConnected() const;

    /// @brief Returns the current frame obtained by the Leap Motion device.
    LEAP_TRACKING_EVENT *GetFrame() const;

    /// @brief Returns a struct containing information about the Leap Motion device.
    LEAP_DEVICE_INFO *GetDeviceProperties() const;

   private:
    // Singleton enforcement
    static LeapConnection *m_instance;

    // State variables
    static bool m_isConnected;
    static volatile bool m_isRunning;
    static LEAP_CONNECTION m_connection;
    static LEAP_TRACKING_EVENT *m_lastFrame;
    static LEAP_DEVICE_INFO *m_lastDevice;
    static HANDLE m_pollingThread;
    static CRITICAL_SECTION m_dataLock;

    // Setters for m_lastDevice and m_lastFrame, respectively.
    static void SetDevice(const LEAP_DEVICE_INFO *deviceProps);
    static void SetFrame(const LEAP_TRACKING_EVENT *frame);

    // The actual message loop that the Leap Motion device "posts to."
    static void MessageLoop(void *unused);

    // Callbacks for each of the Leap Motion events.
    // Most of these are not important for my purposes, and are therefore empty.
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