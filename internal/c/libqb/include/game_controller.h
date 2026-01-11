//----------------------------------------------------------------------------------------------------------------------
// QB64-PE Game Controller Library
// Powered by libstem Gamepad (https://github.com/ThemsAllTook/libstem_gamepad)
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

/**
 * @file game_controller.h
 * @brief Game controller, keyboard, and mouse input functions for QB64-PE
 * 
 * This header provides functions for managing input devices including game controllers,
 * keyboards, and mice. Powered by libstem Gamepad library.
 */

/**
 * @name Device Type Constants
 * @brief Device type identifiers
 */
///@{
#define QUEUED_EVENTS_LIMIT 1024  ///< Maximum number of queued events per device
#define DEVICETYPE_CONTROLLER 1   ///< Joystick/Gamepad device
#define DEVICETYPE_KEYBOARD 2      ///< Keyboard device
#define DEVICETYPE_MOUSE 3        ///< Mouse device
///@}

/**
 * @struct device_struct
 * @brief Structure representing an input device (controller, keyboard, or mouse)
 */
struct device_struct {
    int32_t used;      ///< Whether this device slot is in use
    int32_t type;      ///< Device type (0=Unallocated, 1=Joystick/Gamepad, 2=Keyboard, 3=Mouse)
    char *name;        ///< Device name (FIXME: this is modified by game_controller.cpp)
    int32_t connected; ///< Whether the device is currently connected
    int32_t lastbutton; ///< Last button that was pressed
    int32_t lastaxis;    ///< Last axis that was moved
    int32_t lastwheel;   ///< Last wheel that was scrolled
    int32_t max_events;  ///< Maximum number of events that can be queued
    int32_t queued_events; ///< Current number of queued events
    uint8_t *events;      ///< Event queue (structure depends on device type and capabilities)
    int32_t event_size;   ///< Size of each event in bytes
    uint8_t STRIG_button_pressed[256]; ///< Button press flags (checked and cleared by STRIG function)
    void *handle_pointer;    ///< Device handle as pointer
    int64_t handle_int;      ///< Device handle as integer
    const char *description; ///< Device description from manufacturer
    int64_t product_id;      ///< Product ID
    int64_t vendor_id;       ///< Vendor ID
    int32_t buttons;         ///< Number of buttons
    int32_t axes;            ///< Number of axes
    int32_t balls;           ///< Number of trackballs
    int32_t hats;            ///< Number of hat switches
};

/**
 * @struct onstrig_struct
 * @brief Structure for ON STRIG event handlers
 */
struct onstrig_struct {
    uint32_t id;    ///< Event ID to trigger (0=no event)
    int64_t pass;   ///< Value to pass to triggered event (for ON ... CALL ...(x))
    uint8_t active; ///< Active state (0=OFF, 1=ON, 2=STOP)
    uint8_t state;  ///< Current state (0=untriggered, 1=triggered, 2=in progress (TIMER only), 2+=multiple events queued (KEY only))
};

/**
 * @brief Index of the last device in the devices array
 */
extern int32_t device_last;

/**
 * @brief Maximum number of devices
 */
extern int32_t device_max;

/**
 * @brief Array of input devices
 */
extern device_struct *devices;

/**
 * @brief Array of ON STRIG event handlers
 */
extern onstrig_struct *onstrig;

/**
 * @brief Flag indicating an ON STRIG event is in progress
 */
extern int32_t onstrig_inprogress;

/**
 * @name Device Event Access Functions
 * @brief Functions for reading and writing device event data
 */
///@{
/**
 * @brief Gets a button value from a device event
 * @param device Device structure
 * @param eventIndex Index of the event
 * @param objectIndex Index of the button
 * @return Button value (0 or 1)
 */
uint8_t getDeviceEventButtonValue(device_struct *device, int32_t eventIndex, int32_t objectIndex);

/**
 * @brief Sets a button value in a device event
 * @param device Device structure
 * @param eventIndex Index of the event
 * @param objectIndex Index of the button
 * @param value Button value to set (0 or 1)
 */
void setDeviceEventButtonValue(device_struct *device, int32_t eventIndex, int32_t objectIndex, uint8_t value);

/**
 * @brief Gets an axis value from a device event
 * @param device Device structure
 * @param eventIndex Index of the event
 * @param objectIndex Index of the axis
 * @return Axis value (typically -1.0 to 1.0)
 */
float getDeviceEventAxisValue(device_struct *device, int32_t eventIndex, int32_t objectIndex);

/**
 * @brief Sets an axis value in a device event
 * @param device Device structure
 * @param eventIndex Index of the event
 * @param objectIndex Index of the axis
 * @param value Axis value to set
 */
void setDeviceEventAxisValue(device_struct *device, int32_t eventIndex, int32_t objectIndex, float value);

/**
 * @brief Gets a wheel value from a device event
 * @param device Device structure
 * @param eventIndex Index of the event
 * @param objectIndex Index of the wheel
 * @return Wheel value
 */
float getDeviceEventWheelValue(device_struct *device, int32_t eventIndex, int32_t objectIndex);

/**
 * @brief Sets a wheel value in a device event
 * @param device Device structure
 * @param eventIndex Index of the event
 * @param objectIndex Index of the wheel
 * @param value Wheel value to set
 */
void setDeviceEventWheelValue(device_struct *device, int32_t eventIndex, int32_t objectIndex, float value);
///@}

/**
 * @brief Initializes a device structure
 * @param device Device structure to initialize
 * @note Sets up the device structure with default values
 */
void setupDevice(device_struct *device);

/**
 * @brief Creates a new event in a device's event queue
 * @param device Device structure
 * @return Index of the created event, or -1 on failure
 * @note Creates a new event slot in the device's event queue
 */
int32_t createDeviceEvent(device_struct *device);

/**
 * @brief Commits a device event to the queue
 * @param device Device structure
 * @note Finalizes and commits the current event to the device's event queue
 */
void commitDeviceEvent(device_struct *device);

/**
 * @brief Initializes the gamepad/input system
 * @note Must be called before using any input device functions
 */
void QB64_GAMEPAD_INIT();

/**
 * @brief Polls all input devices for new events
 * @note Should be called regularly (e.g., in the main loop) to update device state
 */
void QB64_GAMEPAD_POLL();

/**
 * @brief Shuts down the gamepad/input system
 * @note Cleans up resources. Should be called when input functionality is no longer needed.
 */
void QB64_GAMEPAD_SHUTDOWN();
