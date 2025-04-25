#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "wasm_export.h"

#include "../shared.h"
#include "native_symbols.h"

#define SEGMENT_ALIGN 0x10000000
#define MMAP_SIZE     (SEGMENT_ALIGN * 3) // 2 segments + padding

#define FLASH_SIZE  0x200000   // 2MB
#define RAM_SIZE    0x42000    // 270KB

#define WASM_SIZE   0x050000   // 320KB
#define CONFIG_SIZE 0x010000   // 64KB

char* MMAP_AREA = NULL;
char* FLASH_AREA = NULL;
char* RAM_AREA = NULL;

#define WASM_AREA (FLASH_AREA + 0x190000)
#define CONFIG_AREA (FLASH_AREA + 0x1F0000)


#define ALIGN(n, m)  (((n) + (m) - 1) & ~((m) - 1)) 

#define LEN(a) (sizeof(a) / sizeof(a[0]))

int map_memory(void) {
    MMAP_AREA = mmap(NULL, MMAP_SIZE, 0,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (MMAP_AREA == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    FLASH_AREA = (char*)ALIGN((uintptr_t)MMAP_AREA, SEGMENT_ALIGN);
    if (mprotect(FLASH_AREA, FLASH_SIZE, PROT_READ | PROT_WRITE)) {
        perror("mprotect failed");
        return 1;
    }
    printf("flash mapped at %p\n", FLASH_AREA);

    RAM_AREA = (char*)ALIGN((uintptr_t)FLASH_AREA + FLASH_SIZE, SEGMENT_ALIGN);
    if (mprotect(RAM_AREA, RAM_SIZE, PROT_READ | PROT_WRITE)) {
        perror("mprotect failed");
        return 1;
    }
    printf("ram mapped at %p\n", RAM_AREA);

    return 0;
}

int initialize_memory(void) {
    memset(FLASH_AREA, 0, FLASH_SIZE);
    memset(RAM_AREA, 0, RAM_SIZE);

    // read ROOT_DIR/exploit/expoit.aot into WASM_AREA
    FILE* wasm_file = fopen("../exploit/exploit.aot", "rb");
    if (!wasm_file) {
        perror("Failed to open wasm file");
        return 1;
    }
    fseek(wasm_file, 0, SEEK_END);
    size_t wasm_size = ftell(wasm_file);
    fseek(wasm_file, 0, SEEK_SET);

    if (wasm_size > WASM_SIZE) {
        fprintf(stderr, "WASM file is too large\n");
        fclose(wasm_file);
        return 1;
    }
    fread(WASM_AREA, 1, wasm_size, wasm_file);
    fclose(wasm_file);
    printf("WASM file loaded into %p\n", WASM_AREA);

    static const char wifi_ssid[] = "my_wifi_ssid";
    static const char wifi_pass[] = "my_wifi_password";


    pico_config* config = (pico_config*)CONFIG_AREA;
    config->magic = 0x12345678;
    config->version = 1;
    config->crc = 0x87654321;
    config->bootloader_state = 0;
    config->os_b_len = 469812;
    config->os_a_len = 472812;

    config->wifi_ssid_len = strlen(wifi_ssid);
    config->wifi_pass_len = strlen(wifi_pass);
    memcpy(config->wifi_ssid, wifi_ssid, config->wifi_ssid_len);
    memcpy(config->wifi_pass, wifi_pass, config->wifi_pass_len);

    config->wasm_length = wasm_size;
    config->default_wasm_length = 0;
    return 0;
}

wasm_module_t module;
wasm_module_inst_t module_inst;
wasm_function_inst_t main_func;
wasm_exec_env_t exec_env;

int initialize_wasm_runtime(void) {
    pico_config* config = (pico_config*)CONFIG_AREA;

    const int wasm_heap_size = 80 * 1024; // 80KB
    // char* heap_addr = (char *)ALIGN((uintptr_t)RAM_AREA + rand() % (RAM_SIZE - wasm_heap_size), 8);
    char* heap_addr = (char *)RAM_AREA + 0x10000;
    printf("wasm heap = %p\n", heap_addr);

	RuntimeInitArgs args = {
		.mem_alloc_type = Alloc_With_Pool,
		.mem_alloc_option =
			{
				.pool =
					{
						.heap_buf = heap_addr,
                        .heap_size = wasm_heap_size,
					},
			},
		.native_module_name = "env",
		.native_symbols = native_symbols,
		.n_native_symbols = LEN(native_symbols),
	};
	// initialize the wasm runtime
	wasm_runtime_full_init(&args);

    uint32_t stack_size = 8192, heap_size = 40000;
	static char error_buf[128];

    module = wasm_runtime_load((uint8_t*)WASM_AREA, config->wasm_length, error_buf, sizeof(error_buf));
    if (!module) {
        fprintf(stderr, "Failed to load wasm module: %s\n", error_buf);
        return 1;
    }

    module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, error_buf, sizeof(error_buf));
    if (!module_inst) {
        fprintf(stderr, "Failed to instantiate wasm module: %s\n", error_buf);
        return 1;
    }

	main_func = wasm_runtime_lookup_function(module_inst, "main");
	if (!main_func) {
        fprintf(stderr, "Failed to find main function\n");
        return 1;
    }

    exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
    if (!exec_env) {
        fprintf(stderr, "Failed to create exec env: %s\n", wasm_runtime_get_exception(module_inst));
        return 1;
    }

    return 0;
}

int wasm_vprintf(const char* fmt, va_list args) {
    printf("WASM: ");
    vprintf(fmt, args);
    return 0;
}



int main() {
    if (map_memory()) {
        fprintf(stderr, "Failed to map memory\n");
        return 1;
    }

    if (initialize_memory()) {
        fprintf(stderr, "Failed to initialize memory\n");
        return 1;
    }
    printf("Memory initialized successfully\n\n");

    if (initialize_wasm_runtime()) {
        fprintf(stderr, "Failed to initialize wasm runtime\n");
        return 1;
    }
    printf("WASM runtime initialized successfully\n\n");

    mprotect(FLASH_AREA, FLASH_SIZE, PROT_READ | PROT_EXEC);
    mprotect(RAM_AREA, RAM_SIZE, PROT_READ | PROT_WRITE);

    // Call the main function in the wasm module
    bool res = wasm_runtime_call_wasm(exec_env, main_func, 2,
        (uint32_t[]){
            0, // argc
            0, // argv
        });


    if (!res) {
        fprintf(stderr, "Failed to call main function: %s\n", wasm_runtime_get_exception(module_inst));
        return 1;
    }
}
