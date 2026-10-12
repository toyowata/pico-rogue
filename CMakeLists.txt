cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED PICO_SDK_PATH)
    if(DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    else()
        message(FATAL_ERROR "PICO_SDK_PATH is not set. Pass -DPICO_SDK_PATH=<path> or set the PICO_SDK_PATH environment variable.")
    endif()
endif()

# Pull in SDK (must be before project)
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico_rogue C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(pico_rogue
    pico_curses.c
    pico_mdport.c
    armor.c
    chase.c
    command.c
    daemon.c
    daemons.c
    extern.c
    fight.c
    init.c
    io.c
    list.c
    mach_dep.c
    main.c
    misc.c
    monsters.c
    move.c
    new_level.c
    options.c
    pack.c
    passages.c
    potions.c
    rings.c
    rip.c
    rooms.c
    save.c
    scrolls.c
    state.c
    sticks.c
    things.c
    vers.c
    weapons.c
    wizard.c
    xcrypt.c
)

target_include_directories(pico_rogue PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(pico_rogue PRIVATE
    HAVE_CONFIG_H=1
    PICO_BUILD=1
)

target_compile_options(pico_rogue PRIVATE
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-parentheses
    -Wno-pointer-sign
)

target_link_libraries(pico_rogue
    pico_stdlib
    hardware_watchdog
)

# Enable USB CDC serial, disable UART serial
pico_enable_stdio_usb(pico_rogue 1)
pico_enable_stdio_uart(pico_rogue 0)

# Create .uf2, .hex, .bin, .map output files
pico_add_extra_outputs(pico_rogue)
