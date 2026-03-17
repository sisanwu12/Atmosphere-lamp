# 指定 cmake 最低版本需求
cmake_minimum_required(VERSION 3.0.2)
# 指定项目名称
project(ATMOSPHERE_LAMP)

# 指定语言环境
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

ENABLE_LANGUAGE(ASM)

# 指定芯片架构
set(MCU cortex-m3)
set(MCU_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft")
add_compile_definitions(STM32F103xx)

# 指定各模块路径
set(MCU_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../mcu)
set(BSP_DIR ${MCU_DIR}/bsp)
set(USER_DIR ${MCU_DIR}/user)
set(LIB_DIR ${MCU_DIR}/Libraries)
set(LD_DIR ${LIB_DIR}/ld)
set(LIBX_DIR ${MCU_DIR}/libx)
set(APP_DIR ${MCU_DIR}/app)

# RTOS组
set(CRM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../crm)

# freeRTOS模块
set(FREERTOS_DIR ${CRM_DIR}/freeRTOS)
set(FRTOS_INC_DIR ${FREERTOS_DIR}/include)
set(FRTOS_PORT_DIR ${FREERTOS_DIR}/portable)
set(FRTOS_SRC_DIR ${FREERTOS_DIR}/src)
set(MEMMANG_DIR ${FRTOS_PORT_DIR}/MemMang)
set(ARM_CM3_DIR ${FRTOS_PORT_DIR}/GCC/ARM_CM3)

# 基本编译选项

# C 编译器
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Os -ffunction-sections -fdata-sections -Wall -Wextra -g")

# C++ 编译器
set(CMAKE_CXX_FLAGS "${MCU_FLAGS} -Os -ffunction-sections -fdata-sections -Wall -Wextra -g \
                       -fno-exceptions -fno-rtti -flto -fno-threadsafe-statics")

# 指定链接文件
set(CMAKE_EXE_LINKER_FLAGS "-T${LD_DIR}/STM32F103C8_FLASH.ld -Wl,--gc-sections -static")



# 指定启动文件
set(STARTUP_FILE ${LIB_DIR}/CMSIS/startup_stm32f103xb.s)

# 包含目录
include_directories(
    # 库文件夹
    ${LIB_DIR}/CMSIS
    ${LIB_DIR}/HAL_Driver
    ${LIB_DIR}/HAL_Driver/Legacy

		# freeRTOS文件夹
    ${FRTOS_INC_DIR}
    ${ARM_CM3_DIR}

    ${USER_DIR}
    ${BSP_DIR}
		${LIBX_DIR}
		${APP_DIR}
)

add_definitions(-DSTM32F103xx)

set(SRC_FILES
    # CMSIS
    ${LIB_DIR}/CMSIS/system_stm32f1xx.c
    ${LIB_DIR}/CMSIS/syscalls.c

    # STM32 HAL
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_cortex.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_rcc.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_rcc_ex.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_flash.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_flash_ex.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_gpio.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_gpio_ex.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_dma.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_can.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_spi.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_tim.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_tim_ex.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_uart.c
    ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_usart.c

    # FreeRTOS
    ${ARM_CM3_DIR}/port.c
    ${MEMMANG_DIR}/heap_4.c
    ${FRTOS_SRC_DIR}/croutine.c
    ${FRTOS_SRC_DIR}/event_groups.c
    ${FRTOS_SRC_DIR}/list.c
    ${FRTOS_SRC_DIR}/queue.c
    ${FRTOS_SRC_DIR}/tasks.c
    ${FRTOS_SRC_DIR}/timers.c

    # User
    ${USER_DIR}/event_bus.c
    ${USER_DIR}/freertos_hooks.c
    ${USER_DIR}/main.c
    ${USER_DIR}/stm32f1xx_it.c
    ${USER_DIR}/system_boot.c

    # BSP
    ${BSP_DIR}/bsp_can.c
    ${BSP_DIR}/bsp_gpio.c
    ${BSP_DIR}/bsp_max7219.c
    ${BSP_DIR}/bsp_spi.c
    ${BSP_DIR}/bsp_timer.c
    ${BSP_DIR}/bsp_usart.c

    # APP
    ${APP_DIR}/app_can.c
    ${APP_DIR}/app_debug.c
    ${APP_DIR}/app_display_policy.c
    ${APP_DIR}/app_dot_displayer.c
    ${APP_DIR}/app_gonio.c
    ${APP_DIR}/app_state.c
    ${APP_DIR}/app_trun_lamp.c
)


# 生成可执行文件（ELF）
add_executable(${PROJECT_NAME}.elf ${SRC_FILES} ${STARTUP_FILE})

# 生成 HEX 与 BIN
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
)
