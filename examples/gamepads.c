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

/**
 * This example shows any gamepads and their configuration
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <libjoystick.h>

static void
print_axes(struct js_device *device)
{
	size_t count;

	count = js_device_get_axis_count(device);

	for (size_t i = 0; i < count; i++) {
		struct js_axis *axis = js_device_get_axis(device, i);
		const char *where = "center";

		if (js_axis_has_capability(axis, JS_AXIS_CAP_LEFT))
			where = "left";
		else if (js_axis_has_capability(axis, JS_AXIS_CAP_RIGHT))
			where = "right";

		printf("Axis %zd%sis located %s\n", i,
		       js_axis_has_capability(axis, JS_AXIS_CAP_ANALOG) ? "" : " (digital) ",
		       where);
	}
}

static void
print_device(struct js_event *event)
{
	struct js_device *device;

	if (js_event_get_type(event) != JS_EVENT_DEVICE_ADDED)
		return;

	device = js_event_get_device(event);

	if (!js_device_has_type(device, JS_TYPE_GAMEPAD))
		return;

	printf("%s is a Gamepad\n", js_device_get_name(device));

	print_axes(device);
}

static int
open_restricted(const char *path, int flags, void *user_data)
{
	int fd = open(path, flags);
	return fd != -1 ? fd : -errno;
}

static void
close_restricted(int fd, void *userdata) {
	close(fd);
}

static const struct js_interface interface = {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted,
};

static bool stop = false;

static void
sighandler(int signal)
{
	stop = true;
}

int
main(void) {
	struct js_ctx *ctx;
	struct js_event *event;

	signal(SIGINT, sighandler);

	ctx = js_ctx_udev_create_context(&interface, NULL);
	js_ctx_udev_assign_seat(ctx, "seat0");

	event = js_ctx_get_event(ctx);
	if (!event) {
		fprintf(stderr, "No compatible gaming devices detected\n");
		return 1;
	}

	while (event && !stop) {
		print_device(event);

		js_event_destroy(event);
		event = js_ctx_get_event(ctx);
	}

	js_ctx_unref(ctx);

	return 0;
}
