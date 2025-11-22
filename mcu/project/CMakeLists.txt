# 指定 cmake 最低版本需求
cmake_minimum_required(VERSION 4.0.2)
# 指定项目名称
project(ATMOSPHERE_LAMP)

# 指定语言环境
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

ENABLE_LANGUAGE(ASM)

# 指定芯片架构
set(MCU cortex-m3)
set(MCU_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft")
add_compile_definitions(STM32F103xx)

# 指定各模块路径
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(BSP_DIR ${PROJECT_ROOT_DIR}/bsp)
set(USER_DIR ${PROJECT_ROOT_DIR}/user)
set(LIB_DIR ${PROJECT_ROOT_DIR}/Libraries)
set(LD_DIR ${LIB_DIR}/ld)

# 基本编译选项
set(CMAKE_C_FLAGS "${MCU_FLAGS} -O2 -ffunction-sections -fdata-sections -Wall -g")

set(CMAKE_EXE_LINKER_FLAGS "-T${LD_DIR}/STM32F103X6_FLASH.ld -Wl,--gc-sections -static")



# 指定启动文件
set(STARTUP_FILE ${LIB_DIR}/CMSIS/startup_stm32f103x6.s)

# 包含目录
include_directories(
    # 库文件夹
    ${LIB_DIR}/CMSIS
    ${LIB_DIR}/HAL_Driver
    ${LIB_DIR}/HAL_Driver/Legacy

    ${USER_DIR}
    ${BSP_DIR}
)

add_definitions(-DSTM32F103xx)

# 源文件
file(GLOB SRC_FILES
    # 库文件夹
    ${LIB_DIR}/CMSIS/*.c
    ${LIB_DIR}/HAL_Driver/*.c
    ${LIB_DIR}/HAL_Driver/Legacy/*.c

    ${USER_DIR}/*.c
    ${BSP_DIR}/*.c
)


# 生成可执行文件（ELF）
add_executable(${PROJECT_NAME}.elf ${SRC_FILES} ${STARTUP_FILE})

# 生成 HEX 与 BIN
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
)