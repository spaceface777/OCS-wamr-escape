#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <wasm_export.h>

void wasm__sleep(wasm_exec_env_t exec_env, int ms) {
	(void)exec_env; // Unused parameter
    usleep(ms * 1000);
}

#define EXPORT_CUSTOM_WASM_API_WITH_SIG(symbol, signature) {#symbol, (void*)wasm__##symbol, signature, NULL}

// Define the native symbols
NativeSymbol native_symbols[] = {
	EXPORT_CUSTOM_WASM_API_WITH_SIG(sleep, "(i)"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(get_own_id, "()i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(get_network_map, "()i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(get_uptime, "()i"),

	// EXPORT_CUSTOM_WASM_API_WITH_SIG(laser_right_detect, "()i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(laser_right_get, "()i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(laser_right_set, "(i)i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(laser_left_detect, "()i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(laser_left_get, "()i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(laser_left_set, "(i)i"),

	// EXPORT_CUSTOM_WASM_API_WITH_SIG(servo_angle_set, "(f)i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(servo_angle_get, "()f"),

	// EXPORT_CUSTOM_WASM_API_WITH_SIG(led_set_color, "(i)i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(led_set_rgb, "(iii)i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(led_get_color, "()i"),

	// EXPORT_CUSTOM_WASM_API_WITH_SIG(CV_sensor_get_voltage, "()f"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(CV_sensor_get_power, "()f"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(CV_sensor_get_current, "()f"),

	// EXPORT_CUSTOM_WASM_API_WITH_SIG(belt_small_set_speed, "(f)i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(belt_small_get_speed, "()f"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(belt_small_get_encoder_count, "()I"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(belt_small_get_encoder_freq, "()F"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(belt_big_set_speed, "(f)i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(belt_big_get_speed, "()f"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(belt_big_get_encoder_count, "()I"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(belt_big_get_encoder_freq, "()F"),

	// EXPORT_CUSTOM_WASM_API_WITH_SIG(RFID_check_tag, "()i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(RFID_write_data_block, "(ii)i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(RFID_read_data_block, "(ii)i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(RFID_get_uid, "(i)i"),

	// EXPORT_CUSTOM_WASM_API_WITH_SIG(next_event, "()i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(next_message_address, "(i)i"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(subscribe_to_event, "(i)"),
	// EXPORT_CUSTOM_WASM_API_WITH_SIG(unsubscribe_from_event, "(i)"),

	// EXPORT_CUSTOM_WASM_API_WITH_SIG(send_packet, "(iii)i"),
};
