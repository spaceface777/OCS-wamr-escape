#include <stdint.h>

typedef struct pico_config {
	uint32_t magic;	  // Magic number
	uint32_t version; // Version number
	uint32_t crc;	  // CRC checksum

	// bootloader data
	uint32_t bootloader_state; // Bootloader state
	uint32_t os_b_len;		   // Length of OS B
	uint32_t os_a_len;		   // Length of OS A

	// wifi configuration
	uint32_t wifi_ssid_len; // Length of WiFi SSID
	uint32_t wifi_pass_len; // Length of WiFi password
	uint8_t wifi_ssid[32];	// WiFi SSID
	uint8_t wifi_pass[64];	// WiFi password

	// wasm configuration
	uint32_t wasm_length;		  // Length of WASM
	uint32_t default_wasm_length; // Length of default WASM
} pico_config;

