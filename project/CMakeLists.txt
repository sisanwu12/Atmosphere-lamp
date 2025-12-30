# 指定 cmake 最低版本需求
cmake_minimum_required(VERSION 4.0.2)
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
set(CMAKE_EXE_LINKER_FLAGS "-T${LD_DIR}/STM32F103XB_FLASH.ld -Wl,--gc-sections -static")



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

# 源文件
file(GLOB SRC_FILES
    # 库文件夹
    ${LIB_DIR}/CMSIS/*.c
    ${LIB_DIR}/HAL_Driver/*.c
    ${LIB_DIR}/HAL_Driver/Legacy/*.c

	  # freeRTOS文件夹
    ${ARM_CM3_DIR}/*.c
    ${MEMMANG_DIR}/*.c
    ${FRTOS_SRC_DIR}/*.c

    ${USER_DIR}/*.c
    ${BSP_DIR}/*.c
		${LIBX_DIR}/*.c
		${APP_DIR}/*.c
)

# NOTE:
# HAL_Driver 目录里自带的 `stm32f1xx_hal_timebase_tim_template.c` 会覆盖 HAL 的
# SysTick 时基，改用 TIM2 作为 HAL_Delay() 的时基。
# 本工程使用 FreeRTOS（SysTick 作为 RTOS Tick），同时在启动阶段也需要 HAL_Delay，
# 因此这里排除该模板文件，统一使用 SysTick 作为 HAL 的 Tick 时基。
list(REMOVE_ITEM SRC_FILES ${LIB_DIR}/HAL_Driver/stm32f1xx_hal_timebase_tim_template.c)


# 生成可执行文件（ELF）
add_executable(${PROJECT_NAME}.elf ${SRC_FILES} ${STARTUP_FILE})

# 生成 HEX 与 BIN
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
)

# 去除警告
target_compile_options(${PROJECT_NAME}.elf PRIVATE -w)
