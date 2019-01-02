/*
 * Copyright © 2019 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup base Initialization and manipulation of libjoystick contexts
 */

/**
 * @defgroup event Handling events
 */

/**
 * @defgroup device Querying and manipulating devices
 */

/**
 * @ingroup base
 * @struct js_ctx
 *
 * A handle for accessing a libjoystick context.
 *
 * This struct is refcounted, see js_ctx_ref() and js_ctx_unref().
 */
struct js_ctx;

/**
 * @ingroup device
 * @struct js_device
 *
 * A handle for accessing a libjoystick device.
 *
 * This struct is refcounted, see js_device_ref() and js_device_unref().
 */
struct js_device;

/**
 * @ingroup device
 * @struct js_button
 *
 * A handle for accessing a button. A button in libjoystick is defined as a
 * physical entity that can be pressed and usually triggers a single action
 * when pressed. A button only has a single physical dimension, digital
 * buttons only have two logical values - down and up.
 *
 * @note A D-Pad is a separate entity in libjoystick.
 *
 * This struct is refcounted, see js_button_ref() and js_button_unref().
 */
struct js_button;

/**
 * @ingroup device
 * @struct js_dpad
 *
 * A handle for accessing a d-pad, usually a group of 4 buttons arranged in
 * a cross-like shape. libjoystick supports 4 and 8 buttons per d-pad.
 *
 * This struct is refcounted, see js_dpad_ref() and js_dpad_unref().
 */
struct js_dpad;

/**
 * @ingroup device
 * @struct js_axis
 *
 * A handle for accessing a axis. Axes in libjoystick have multiple
 * dimensions, the default is for x, y and z. Each dimension can provide a
 * continuous value, normalized into a signed 16-bit range.
 *
 * This struct is refcounted, see js_axis_ref() and js_axis_unref().
 */
struct js_axis;

/**
 * @ingroup device
 * @struct js_touch
 *
 * A handle for accessing a touchscreen or touchpad.
 *
 * This struct is refcounted, see js_touch_ref() and js_touch_unref().
 */
struct js_touch;

/**
 * @ingroup event
 * @struct js_event
 *
 * The base event type.
 *
 * @warning Unlike other structs events are considered transient and
 * <b>not</b> refcounted. Use js_event_destroy() to release an event.
 */
struct js_event;

/**
 * @ingroup base
 * @struct js_interface
 *
 * libjoystick does not open file descriptors directly, instead
 * open_restricted() and close_restricted() are called for each path that
 * must be opened.
 *
 * @see js_ctx_udev_create_context
 */
struct js_interface {
	/**
	 * Open the device at the given path with the flags provided and
	 * return the fd.
	 *
	 * @param path The device path to open
	 * @param flags Flags as defined by open(2)
	 * @param user_data The user_data provided in
	 * js_ctx_udev_create_context()
	 *
	 * @return The file descriptor, or a negative errno on failure.
	 */
	int (*open_restricted)(const char *path, int flags, void *user_data);
	/**
	 * Close the file descriptor.
	 *
	 * @param fd The file descriptor to close
	 * @param user_data The user_data provided in
	 * js_ctx_udev_create_context()
	 */
	void (*close_restricted)(int fd, void *user_data);
};

/**
 * @ingroup base
 *
 * Create a new libjoystick context from udev. This context is inactive
 * until a seat has been assigned, see js_ctx_udev_assign_seat().
 *
 * The returned context has a refcount of at least 1, use js_ctx_unref() to
 * release it.
 */
struct js_ctx *
js_ctx_udev_create_context(const struct js_interface *interface,
			   void *userdata);

/**
 * Assign a seat to this context. Immediately after, any devices available
 * on this seat will appear as device added events. In the future, devices
 * removed and/or added will appear as the respective events.
 *
 * This function may only be called once per context.
 *
 * @return 0 on success or a negative errno on failure.
 */
int
js_ctx_udev_assign_seat(struct js_ctx *ctx, const char *seat);

/**
 * @ingroup base
 *
 * libjoystick keeps a single file descriptor for all events. Call
 * js_ctx__dispatch() if any events become available on this fd.
 *
 * @return The file descriptor used to notify of pending events.
 */
int
js_ctx_get_fd(struct js_ctx *js);

/**
 * @ingroup base
 *
 * Main event dispatchment function. Reads events from available
 * devices and process them internally. Use js_ctx_get_event() to retrieve
 * the events.
 *
 * Dispatching does not necessarily queue libjoystick events. This function
 * should be called immediately once data is available on the file
 * descriptor returned by js_ctx_get_fd(). Any delay in calling
 * js_ctx_dispatch() may result in lost events.
 *
 * @param ctx A previously initialized libjoystick context
 */
void
js_ctx_dispatch(struct js_ctx *ctx);

/**
 * @ingroup base
 *
 * This function returns NULL when no more events are pending, otherwise the
 * events are returned in the order received by libjoystick.
 *
 * @return the next event available.
 */
struct js_event *
js_ctx_get_event(struct js_ctx *ctx);

/**
 * @ingroup base
 *
 * Set caller-specific data associated with this context. libjoystick does
 * not manage, look at, or modify this data. The caller must ensure the
 * data is valid.
 *
 * @param ctx A previously initialized libjoystick context
 * @param user_data Caller-specific data passed to the various callback
 * interfaces.
 */
void
js_ctx_set_user_data(struct js_ctx *ctx, void *user_data);

/**
 * @ingroup base
 *
 * Get the caller-specific data associated with this context, if any.
 *
 * @param ctx A previously initialized libjoystick context
 * @return The caller-specific data previously assigned in
 * js_ctx_set_user_data().
 */
void *
js_ctx_get_user_data(struct js_ctx *ctx);

/**
 * @ingroup base
 *
 * Add a reference to the context. A context is destroyed whenever the
 * reference count reaches 0. See @ref js_ctx_unref.
 *
 * @param ctx A previously initialized valid libjoystick context
 * @return The passed libjoystick context
 */
struct js_ctx *
js_ctx_ref(struct js_ctx *ctx);

/**
 * @ingroup base
 *
 * Dereference the context. After this, the context may have been destroyed,
 * if the last reference was dereferenced. If so, the context is invalid and
 * may not be interacted with.
 *
 * @param ctx A previously initialized libjoystick context
 * @return Always NULL
 */
struct js_ctx *
js_ctx_unref(struct js_ctx *ctx);

/**
 * @ingroup device
 *
 * @return the device name
 */
const char *
js_device_get_name(struct js_device *device);

/**
 * @ingroup device
 *
 * @return the user index assigned to this device
 */
unsigned int
js_device_get_user_index(struct js_device *device);

/**
 * @ingroup device
 *
 * The device type describes the physical device.
 *
 * These device types are not mutually exclusive, a device may have multiple
 * types set.
 */
enum js_device_type {
	JS_TYPE_JOYSTICK = 1,
	JS_TYPE_GAMEPAD,
	JS_TYPE_WHEEL,      /**< Vroom vroom */
	JS_TYPE_THROTTLE,   /**< The T in HOTAS */
	JS_TYPE_PEDALS,     /**< Foot pedals */
	JS_TYPE_REMOTE,     /**< Wiimote */
};

/**
 * @ingroup device
 *
 * Check for a device type. Note that a device may have more than one device
 * type set, a caller should check the most specific one that applies for
 * the caller's usage first.
 *
 * @return true if the device has the given type, false otherwise.
 */
bool
js_device_has_type(struct js_device *device,
		   enum js_device_type type);

/**
 * @ingroup device
 *
 * @return the number of buttons available on this device.
 */
size_t
js_device_get_button_count(struct js_device *device);

/**
 * @ingroup device
 *
 * @return the button with the given 0-based index
 */
struct js_button *
js_device_get_button(struct js_device *device, unsigned int index);

/**
 * @ingroup device
 *
 * @return the number of axes available on this device.
 */
size_t
js_device_get_axis_count(struct js_device *device);

/**
 * @ingroup device
 *
 * @return the axis with the given 0-based index
 */
struct js_axis *
js_device_get_axis(struct js_device *device, unsigned int index);

/**
 * @ingroup device
 *
 * @return the number of dpads available on this device.
 */
size_t
js_device_get_dpad_count(struct js_device *device);

/**
 * @ingroup device
 *
 * @return the dpad with the given 0-based index
 */
struct js_dpad *
js_device_get_dpad(struct js_device *device, unsigned int index);


/**
 * @ingroup device
 */
enum js_button_capability {
	/**
	 * The button is assigned to the left hand.
	 */
	JS_BUTTON_CAP_LEFT = 1,

	/**
	 * The button is assigned to the right hand.
	 */
	JS_BUTTON_CAP_RIGHT,

	/**
	 * The button is a trigger button.
	 */
	JS_BUTTON_CAP_TRIGGER,

	/**
	 * The button is a shoulder button.
	 */
	JS_BUTTON_CAP_SHOULDER,

	/**
	 * The button is an analog button. Analog buttons support multiple
	 * values between the logical "down" and "up" states.
	 */
	JS_BUTTON_CAP_ANALOG,

	/**
	 * The button is the designated "Start" button
	 */
	JS_BUTTON_CAP_START,

	/**
	 * The button is the designated "Select" button
	 */
	JS_BUTTON_CAP_SELECT,

	/**
	 * The button is the designated "System" or "Home" button
	 */
	JS_BUTTON_CAP_SYSTEM,

	/**
	 * This button is used to confirm dialogues.
	 */
	JS_BUTTON_CAP_OK,

	/**
	 * This button is used to cancel dialogs.
	 */
	JS_BUTTON_CAP_CANCEL,

	/**
	 * This button is used to proceed to the next page.
	 */
	JS_BUTTON_CAP_FORWARD,

	/**
	 * This button is used to return to the previous page.
	 */
	JS_BUTTON_CAP_BACK,

	/**
	 * This button is inaccessible in the current device configuration. An
	 * example for such a button are SL and SR on the Nintendo Joy Con when
	 * attached to the console.
	 */
	JS_BUTTON_CAP_INACCESSIBLE,
};

/**
 * @ingroup device
 *
 * @return true if the button has the given capability, false otherwise
 */
bool
js_button_has_capability(struct js_button *button,
			 enum js_button_capability cap);

/**
 * @ingroup device
 *
 * Compare two buttons for perceived priority. This can be used to provide
 * primary and secondary functions (or labels) to buttons.
 *
 * The decision when one button has higher priority than another is
 * device-specific. There may not be a global priority order. For example,
 * on gamepads L1 trigger is higher priority as L2 trigger but equal
 * priority to R1 trigger.
 *
 * @return -1, 0, 1 if b1 is lower priority, equal priority, higher priority
 * than b2, respectively.
 */
int
js_button_compare_priority(struct js_button *b1, struct js_button *b2);

/**
 * @ingroup device
 *
 */
enum js_axis_capability {
	/**
	 * The axis is assigned to the left hand.
	 */
	JS_AXIS_CAP_LEFT = 1 << 16,

	/**
	 * The axis is assigned to the right hand.
	 */
	JS_AXIS_CAP_RIGHT,

	/**
	 * The axis is an analog axis. Analog axis support multiple
	 * values between the logical "on" and "off" states.
	 */
	JS_AXIS_CAP_ANALOG,
};

/**
 * @ingroup device
 *
 * @return true if the axis has the given capability, false otherwise
 */
bool
js_axis_has_capability(struct js_axis *axis, enum js_axis_capability cap);

/**
 * @ingroup device
 *
 */
enum js_dpad_capability {
	/**
	 * The dpad is assigned to the left hand.
	 */
	JS_DPAD_CAP_LEFT = 1 << 24,

	/**
	 * The dpad is assigned to the right hand.
	 */
	JS_DPAD_CAP_RIGHT,

	/**
	 * The dpad has 8 directions, not just 4.
	 */
	JS_DPAD_CAP_8BUTTON,
};

/**
 * @ingroup device
 *
 * @return true if the dpad has the given capability, false otherwise
 */
bool
js_dpad_has_capability(struct js_dpad *dpad, enum js_dpad_capability cap);

/**
 * @ingroup event
 *
 * Event type for events returned by js_ctx_get_event().
 */
enum js_event_type {
	/**
	 * A new device has been added to the context. Events from the device
	 * will not be available until the next time js_ctx_dispatch() is
	 * called and data is available from this device.
	 *
	 * This event is set for all devices available on context creation.
	 */
	JS_EVENT_DEVICE_ADDED = 1,

	/**
	 * A device has been removed and no more events from this device will be
	 * queued.
	 */
	JS_EVENT_DEVICE_REMOVED,

	/**
	 * A device has changed its capabilities, either by adding or removing
	 * accessories.
	 */
	JS_EVENT_DEVICE_CHANGED,

	/**
	 * Marks the end of a hardware scanout cycle. All previous events
	 * accumulated represent the state of the device at the time of the
	 * sync.
	 */
	JS_EVENT_SYNC = 100,

	/**
	 * One ore more axes on the device have changed state. See
	 * js_event_axes_get_state().
	 */
	JS_EVENT_AXIS = 200,

	/**
	 * One ore more buttons on the device have changed state. See
	 * js_event_button_get_state().
	 */
	JS_EVENT_BUTTON = 300,

	/**
	 * One ore more accelerometer axes on the device have changed state. See
	 * js_event_axelerometer_get_state().
	 */
	JS_EVENT_ACCELEROMETER = 400,

	/**
	 * One ore more dpads on the device have changed state. See
	 * js_event_dpad_get_state().
	 */
	JS_EVENT_DPAD = 500,
};

/**
 * @ingroup event
 *
 * Get the type of the event.
 *
 * @param event An event retrieved by js_ctx_get_event().
 */
enum js_event_type
js_event_get_type(struct js_event *event);

/**
 * @ingroup event
 *
 * Destroy the event, freeing all associated resources. Resources obtained
 * from this event must be considered invalid after this call.
 *
 * @warning Unlike other structs events are considered transient and
 * <b>not</b> refcounted. Calling js_event_destroy() <b>will</b> destroy the
 * event.
 *
 * @param event An event retrieved by js_ctx_get_event().
 */
void
js_event_destroy(struct js_event *event);

/**
 * @ingroup event
 *
 * The returned device's refcount is not increased, the device will be
 * released when the event is destroyed. Use js_device_ref() to keep a
 * reference to the device.
 *
 * @return the device associated with this event.
 */
struct js_device *
js_event_get_device(struct js_event *event);

/**
 * @ingroup event
 *
 * Test if a specific axis has changed in an event. This function always
 * returns false if the axis does not exist on this device.
 *
 * @return true if the axis state has changed since the last event, false
 * otherwise
 */
bool
js_event_axis_has_changed(struct js_event *event, struct js_axis *axis);

/**
 * @ingroup event
 *
 * Return the value of given axis.
 *
 * If x, y, or z is NULL, that axis is ignored. If the respective axis does
 * not exist on this device, all values are set to 0. If the axis does
 * not have x, y, or z, the respective value is set to 0.
 *
 * The axis value is normalized to a 16-bit signed integer, where 0 is the
 * device-neutral state.
 *
 * @return true if the axis value has changed since the last event, false
 * otherwise
 */
bool
js_event_axis_get_value(struct js_event *event,
			struct js_axis *axis,
			int16_t *x, int16_t *y, int16_t *z);

/**
 * @ingroup event
 *
 * Test if a specific button value has changed in an event. This function
 * always returns false if the button does not exist on this device.
 *
 * Use this function for pressure-sensitive buttons with the @ref
 * JS_BUTTON_CAP_ANALOG capability and where the value of the button is
 * required..
 *
 * @return true if the button value has changed since the last event, false
 * otherwise
 *
 * @see js_event_button_state_has_changed
 */
bool
js_event_button_value_has_changed(struct js_event *event,
				  struct js_button *button);

/**
 * @ingroup event
 *
 * Test if a specific button has changed state in an event. This function
 * always returns false if the button does not exist on this device.
 *
 * The button state is one of logically down or up. See
 * js_event_button_value_has_changed() for a fine-grained value.
 *
 * @return true if the button state has changed since the last event, false
 * otherwise
 *
 * @see js_event_button_value_has_changed
 */
bool
js_event_button_state_has_changed(struct js_event *event,
				  struct js_button *button);

/**
 * @ingroup event
 *
 * Return the value of given button.
 *
 * If the respective button does not exist on this device, the value
 * returned is 0. The value of the button is normalized to a 16-bit unsigned
 * integer value, with 0 representing logically up and 0xffff representing
 * logically down.
 *
 * Use this function for pressure-sensitive buttons with the @ref
 * JS_BUTTON_CAP_ANALOG capability. Otherwise, use
 * js_event_button_get_state().
 *
 * @return true if the button value has changed since the last event, false
 * otherwise
 */
bool
js_event_button_get_value(struct js_event *event, struct js_button *button,
			  uint16_t *state);

/**
 * @ingroup event
 *
 * Return the logical state of the given button.
 *
 * The button state is one of logically down or up. See
 * js_event_button_get_value() for a fine-grained value.
 *
 * libjoystick uses implementation-defined pressure-thresholds to determine
 * when a button with @ref JS_BUTTON_CAP_ANALOG is logically down and up.
 *
 * @return true if the button state has changed since the last event, false
 * otherwise
 */
bool
js_event_button_get_state(struct js_event *event, struct js_button *button,
			  bool *state);


#define _js_bit(_x) (1UL << _x)

/**
 * @ingroup event
 *
 * The cardinal direction for the dpad button.
 */
enum js_dpad_direction {
	JS_DPAD_N = _js_bit(1),
	JS_DPAD_E = _js_bit(2),
	JS_DPAD_S = _js_bit(3),
	JS_DPAD_W = _js_bit(4),
	JS_DPAD_NE = _js_bit(5),
	JS_DPAD_SE = _js_bit(6),
	JS_DPAD_SW = _js_bit(7),
	JS_DPAD_NW = _js_bit(8),
};

/**
 * @ingroup event
 *
 * Return the logical state of the given dpad direction. The returned state
 * is a bitmask of the dpad directions currently logically down.
 *
 * Unknown bits, i.e. those without a defined mask, must be ignored by the caller.
 *
 * @return true if the dpad direction state has changed since the last event, false
 * otherwise
 */
bool
js_event_dpad_get_state(struct js_event *event, struct js_dpad *dpad,
			uint32_t *state);

#ifdef __cplusplus
}
#endif
