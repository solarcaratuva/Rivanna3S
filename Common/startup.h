

/**
 * This function should be treated as the main method and must exist for every board and should be placed in `<board>/Src/main.cpp`.
 * After hardware initialization is done, this function is called.
 * This function should never return.
 */
void app_main();


/**
 * This function should be the first function called upon startup. It sets up the clocks and any MCU specific hardware.
 * This function should be implemented in the `startup_init.c` file in the `Drivers/` directory for each board.
 */
extern "C" void startup_init();