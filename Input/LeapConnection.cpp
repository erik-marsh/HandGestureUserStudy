#include "LeapConnection.hpp"

#include <iostream>
#include <stdexcept>

namespace Input::Leap
{
void DebugPrint(const std::string& message)
{
    if constexpr (ENABLE_DEBUG_PRINT) std::cout << message << "\n";
}

std::string GetEnumString(eLeapRS res)
{
    switch (res)
    {
        case eLeapRS_Success:
            return "eLeapRS_Success";
        case eLeapRS_UnknownError:
            return "eLeapRS_UnknownError";
        case eLeapRS_InvalidArgument:
            return "eLeapRS_InvalidArgument";
        case eLeapRS_InsufficientResources:
            return "eLeapRS_InsufficientResources";
        case eLeapRS_InsufficientBuffer:
            return "eLeapRS_InsufficientBuffer";
        case eLeapRS_Timeout:
            return "eLeapRS_Timeout";
        case eLeapRS_NotConnected:
            return "eLeapRS_NotConnected";
        case eLeapRS_HandshakeIncomplete:
            return "eLeapRS_HandshakeIncomplete";
        case eLeapRS_BufferSizeOverflow:
            return "eLeapRS_BufferSizeOverflow";
        case eLeapRS_ProtocolError:
            return "eLeapRS_ProtocolError";
        case eLeapRS_InvalidClientID:
            return "eLeapRS_InvalidClientID";
        case eLeapRS_UnexpectedClosed:
            return "eLeapRS_UnexpectedClosed";
        case eLeapRS_UnknownImageFrameRequest:
            return "eLeapRS_UnknownImageFrameRequest";
        case eLeapRS_UnknownTrackingFrameID:
            return "eLeapRS_UnknownTrackingFrameID";
        case eLeapRS_RoutineIsNotSeer:
            return "eLeapRS_RoutineIsNotSeer";
        case eLeapRS_TimestampTooEarly:
            return "eLeapRS_TimestampTooEarly";
        case eLeapRS_ConcurrentPoll:
            return "eLeapRS_ConcurrentPoll";
        case eLeapRS_NotAvailable:
            return "eLeapRS_NotAvailable";
        case eLeapRS_NotStreaming:
            return "eLeapRS_NotStreaming";
        case eLeapRS_CannotOpenDevice:
            return "eLeapRS_CannotOpenDevice";
        default:
            return "<Unknown eLeapRS>";
    }
}

LeapConnection::LeapConnection()
{
    if (m_isInitialized)
        throw std::runtime_error("Unable to create new LeapConnnection when one already exists.");
    m_isInitialized = true;

    if (LeapCreateConnection(nullptr, &m_connection) != eLeapRS_Success)
        throw std::runtime_error("Unable to create connection to Leap Motion.");

    if (LeapOpenConnection(m_connection) != eLeapRS_Success)
        throw std::runtime_error("Unable to open connection to Leap Motion.");

    m_isRunning = true;

    m_lastFrame = static_cast<LEAP_TRACKING_EVENT*>(malloc(sizeof(LEAP_TRACKING_EVENT)));
    *m_lastFrame = {};

    m_lastDevice = static_cast<LEAP_DEVICE_INFO*>(malloc(sizeof(LEAP_DEVICE_INFO)));
    *m_lastDevice = {};

    m_pollingThread = std::thread(&LeapConnection::MessageLoop);
}

LeapConnection::~LeapConnection()
{
    m_isRunning = false;
    LeapCloseConnection(m_connection);
    m_pollingThread.join();
    LeapDestroyConnection(m_connection);
    m_isInitialized = false;
}

bool LeapConnection::IsConnected() const { return m_isConnected; }

LEAP_TRACKING_EVENT* LeapConnection::GetFrame() const
{
    LEAP_TRACKING_EVENT* currentFrame;
    {
        std::lock_guard<std::mutex> guard(m_dataLock);
        currentFrame = m_lastFrame;
    }
    return currentFrame;
}

LEAP_DEVICE_INFO* LeapConnection::GetDeviceProperties() const
{
    LEAP_DEVICE_INFO* currentDevice;
    {
        std::lock_guard<std::mutex> guard(m_dataLock);
        currentDevice = m_lastDevice;
    }
    return currentDevice;
}

void LeapConnection::SetDevice(const LEAP_DEVICE_INFO* deviceProps)
{
    std::lock_guard<std::mutex> guard(m_dataLock);
    if (m_lastDevice && m_lastDevice->serial)
    {
        free(m_lastDevice->serial);
    }
    *m_lastDevice = *deviceProps;

    // copying a cstring
    m_lastDevice->serial = static_cast<char*>(malloc(deviceProps->serial_length));
    memcpy(m_lastDevice->serial, deviceProps->serial, deviceProps->serial_length);
}

void LeapConnection::SetFrame(const LEAP_TRACKING_EVENT* frame)
{
    std::lock_guard<std::mutex> guard(m_dataLock);
    *m_lastFrame = *frame;
}

void LeapConnection::MessageLoop()
{
    eLeapRS result;
    LEAP_CONNECTION_MESSAGE msg;
    while (m_isRunning)
    {
        constexpr unsigned int timeout = 1000;
        result = LeapPollConnection(m_connection, timeout, &msg);

        if (result != eLeapRS_Success)
        {
            printf("LeapC PollConnection call was %s.\n", GetEnumString(result).c_str());
            continue;
        }

        switch (msg.type)
        {
            case eLeapEventType_Connection:
                OnConnected(msg.connection_event);
                break;
            case eLeapEventType_ConnectionLost:
                OnConnectionLost(msg.connection_lost_event);
                break;
            case eLeapEventType_Device:
                OnDeviceFound(msg.device_event);
                break;
            case eLeapEventType_DeviceLost:
                OnDeviceLost(msg.device_event);
                break;
            case eLeapEventType_DeviceFailure:
                OnDeviceFailure(msg.device_failure_event);
                break;
            case eLeapEventType_Tracking:
                OnFrame(msg.tracking_event);
                break;
            case eLeapEventType_ImageComplete:
                // Ignore
                break;
            case eLeapEventType_ImageRequestError:
                // Ignore
                break;
            case eLeapEventType_LogEvent:
                OnLogMessage(msg.log_event);
                break;
            case eLeapEventType_Policy:
                OnPolicy(msg.policy_event);
                break;
            case eLeapEventType_ConfigChange:
                OnConfigChange(msg.config_change_event);
                break;
            case eLeapEventType_ConfigResponse:
                OnConfigResponse(msg.config_response_event);
                break;
            case eLeapEventType_Image:
                OnImage(msg.image_event);
                break;
            case eLeapEventType_PointMappingChange:
                OnPointMappingChange(msg.point_mapping_change_event);
                break;
            case eLeapEventType_TrackingMode:
                OnTrackingMode(msg.tracking_mode_event);
                break;
            case eLeapEventType_LogEvents:
                OnLogMessages(msg.log_events);
                break;
            case eLeapEventType_HeadPose:
                OnHeadPose(msg.head_pose_event);
                break;
            case eLeapEventType_IMU:
                OnIMU(msg.imu_event);
                break;
            default:
                // discard unknown message types
                printf("Encountered unknown message %i, discarding.\n", msg.type);
        }  // switch on msg.type
    }
}

bool LeapConnection::m_isInitialized = false;
bool LeapConnection::m_isConnected = false;
volatile bool LeapConnection::m_isRunning = false;
LEAP_CONNECTION LeapConnection::m_connection{};
LEAP_TRACKING_EVENT* LeapConnection::m_lastFrame = nullptr;
LEAP_DEVICE_INFO* LeapConnection::m_lastDevice = nullptr;
std::thread LeapConnection::m_pollingThread{};
std::mutex LeapConnection::m_dataLock{};

void LeapConnection::OnConnected(const LEAP_CONNECTION_EVENT* connection_event)
{
    DebugPrint("[Leap Motion] Connected.");
    m_isConnected = true;
}

void LeapConnection::OnConnectionLost(const LEAP_CONNECTION_LOST_EVENT* connection_lost_event)
{
    DebugPrint("[Leap Motion] Connection lost.");
    m_isConnected = false;
}

void LeapConnection::OnDeviceFound(const LEAP_DEVICE_EVENT* device_event)
{
    LEAP_DEVICE deviceHandle;
    // Open device using LEAP_DEVICE_REF from event struct.
    eLeapRS result = LeapOpenDevice(device_event->device, &deviceHandle);
    if (result != eLeapRS_Success)
    {
        DebugPrint("[Leap Motion] ERROR (" + GetEnumString(result) + "): Could not open device.");
        return;
    }

    // Create a struct to hold the device properties, we have to provide a buffer for the serial
    // string
    LEAP_DEVICE_INFO deviceProperties = {sizeof(deviceProperties)};
    // Start with a length of 1 (pretending we don't know a priori what the length is).
    // Currently device serial numbers are all the same length, but that could change in the future
    deviceProperties.serial_length = 1;
    deviceProperties.serial = static_cast<char*>(malloc(deviceProperties.serial_length));
    // This will fail since the serial buffer is only 1 character long
    // But deviceProperties is updated to contain the required buffer length
    result = LeapGetDeviceInfo(deviceHandle, &deviceProperties);
    if (result == eLeapRS_InsufficientBuffer)
    {
        // try again with correct buffer size
        deviceProperties.serial =
            static_cast<char*>(realloc(deviceProperties.serial, deviceProperties.serial_length));
        result = LeapGetDeviceInfo(deviceHandle, &deviceProperties);
        if (result != eLeapRS_Success)
        {
            DebugPrint("[Leap Motion] ERROR (" + GetEnumString(result) +
                       "): Failed to get device info.");
            free(deviceProperties.serial);
            return;
        }
    }
    SetDevice(&deviceProperties);

    free(deviceProperties.serial);
    LeapCloseDevice(deviceHandle);
    DebugPrint("[Leap Motion] Found device.");
}

void LeapConnection::OnDeviceLost(const LEAP_DEVICE_EVENT* device_event)
{
    DebugPrint("[Leap Motion] Device lost.");
}

void LeapConnection::OnDeviceFailure(const LEAP_DEVICE_FAILURE_EVENT* device_failure_event)
{
    DebugPrint("[Leap Motion] Device failure!");
}

void LeapConnection::OnPolicy(const LEAP_POLICY_EVENT* policy_event)
{
    DebugPrint("[Leap Motion] Policy event.");
}

void LeapConnection::OnFrame(const LEAP_TRACKING_EVENT* tracking_event)
{
    // DebugPrint("[Leap Motion] Got frame.");
    SetFrame(tracking_event);
}

void LeapConnection::OnLogMessage(const LEAP_LOG_EVENT* log_event)
{
    DebugPrint("[Leap Motion] Got log message.");
}

void LeapConnection::OnLogMessages(const LEAP_LOG_EVENTS* log_events)
{
    for (int i = 0; i < log_events->nEvents; i++) OnLogMessage(&log_events->events[i]);
}

void LeapConnection::OnConfigChange(const LEAP_CONFIG_CHANGE_EVENT* config_change_event)
{
    DebugPrint("[Leap Motion] Got config change.");
}

void LeapConnection::OnConfigResponse(const LEAP_CONFIG_RESPONSE_EVENT* config_response_event)
{
    DebugPrint("[Leap Motion] Got config response.");
}

void LeapConnection::OnImage(const LEAP_IMAGE_EVENT* image_event)
{
    // DebugPrint("[Leap Motion] Got image.");
}

void LeapConnection::OnPointMappingChange(
    const LEAP_POINT_MAPPING_CHANGE_EVENT* point_mapping_change_event)
{
    DebugPrint("[Leap Motion] Got point mapping change.");
}

void LeapConnection::OnHeadPose(const LEAP_HEAD_POSE_EVENT* head_pose_event)
{
    DebugPrint("[Leap Motion] Got head pose.");
}

void LeapConnection::OnIMU(const LEAP_IMU_EVENT* imu_event)
{
    DebugPrint("[Leap Motion] Got IMU update.");
}

void LeapConnection::OnTrackingMode(const LEAP_TRACKING_MODE_EVENT* mode_event)
{
    DebugPrint("[Leap Motion] Got tracking mode.");
}

}  // namespace Input::Leap