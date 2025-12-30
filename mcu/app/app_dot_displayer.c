/**
 * @file		app_dot_displayer.c
 * @brief		定义操作该模块的函数
 * @author	王广平
 */

#define APP_DOT_DISPLAYER_C

/* 头文件引用 */
#include "app_dot_displayer.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "event_bus.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_spi.h"
#include "task.h"
#include <string.h>

/* ============================== 全局变量 ============================== */
static SPI_HandleTypeDef DOT_DISPLAYER_SPI = {0};

/* 运行期图案缓存（会根据 TurnCount 旋转） */
static u8 app_dotD_LEFT[8];
static u8 app_dotD_RIGHT[8];
static u8 app_dotD_START[8];
static u8 app_dotD_UP[8];

/* ============================== 内部函数 ============================== */
/**
 * @brief 初始化（并按 TurnCount 旋转）点阵显示用到的图案缓存
 * @note
 * - TurnCount 为 0 时不旋转；为 N 时顺时针旋转 N 次（每次 90°）
 * - 旋转函数输入/输出不能是同一块内存，这里使用 tmp 中转
 */
static RESULT_Init app_dotD_Pattern_Init(void)
{
  memcpy(app_dotD_LEFT, APP_DOTD_LEFT_ARROW, sizeof(app_dotD_LEFT));
  memcpy(app_dotD_RIGHT, APP_DOTD_RIGHT_ARROW, sizeof(app_dotD_RIGHT));
  memcpy(app_dotD_START, APP_DOTD_START, sizeof(app_dotD_START));
  memcpy(app_dotD_UP, APP_DOTD_UP, sizeof(app_dotD_UP));

  for (u8 i = 0; i < TurnCount; i++)
  {
    u8 tmp[8];

    app_dotD_TurnWrite(app_dotD_LEFT, tmp);
    memcpy(app_dotD_LEFT, tmp, sizeof(tmp));

    app_dotD_TurnWrite(app_dotD_RIGHT, tmp);
    memcpy(app_dotD_RIGHT, tmp, sizeof(tmp));

    app_dotD_TurnWrite(app_dotD_START, tmp);
    memcpy(app_dotD_START, tmp, sizeof(tmp));

    app_dotD_TurnWrite(app_dotD_UP, tmp);
    memcpy(app_dotD_UP, tmp, sizeof(tmp));
  }

  return ERR_Init_Finished;
}

/* ============================== 对外接口 ============================== */
RESULT_Init app_dotD_Init()
{
  RESULT_Init ret;

  /* 初始化 GPIO：CLK/DIN 复用推挽，CS 推挽输出 */
  ret = bsp_gpio_AFPP_Init(DOT_GPIOx, CLK_PIN | DIN_PIN);
  if (ret != ERR_Init_Finished)
    return ret;

  ret = bsp_gpio_OTPP_Init(DOT_GPIOx, CS_PIN);
  if (ret != ERR_Init_Finished)
    return ret;

  /* 片选拉高（空闲态） */
  HAL_GPIO_WritePin(DOT_GPIOx, CS_PIN, GPIO_PIN_SET);

  /* SPI 初始化 */
  ret = bsp_spi_Init(&DOT_DISPLAYER_SPI, DOT_SPI, SPI_MODE_MASTER,
                     SPI_DIRECTION_2LINES, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW,
                     SPI_PHASE_1EDGE, SPI_NSS_SOFT, SPI_BAUDRATEPRESCALER_32,
                     SPI_FIRSTBIT_MSB, SPI_TIMODE_DISABLE,
                     SPI_CRCCALCULATION_DISABLE, 0);
  if (ret != ERR_Init_Finished)
    return ret;

  /* MAX7219 初始化寄存器 */
  (void)app_dotD_WriteLine(0x0F, 0x00); // 关闭显示测试
  (void)app_dotD_WriteLine(0x09, 0x00); // 无译码模式
  (void)app_dotD_WriteLine(0x0A, 0x0F); // 亮度（0x00~0x0F）
  (void)app_dotD_WriteLine(0x0B, 0x07); // 扫描 8 行
  (void)app_dotD_WriteLine(0x0C, 0x01); // 开显示

  /* 初始化图案缓存 */
  ret = app_dotD_Pattern_Init();
  if (ret != ERR_Init_Finished)
    return ret;

  /* 清屏 */
  (void)app_dotD_Clear();

  return ERR_Init_Finished;
}

RESULT_RUN app_dotD_WriteLine(u8 addr, u8 data)
{
  u8 buf[2] = {addr, data};

  HAL_GPIO_WritePin(DOT_GPIOx, CS_PIN, GPIO_PIN_RESET);
  if (HAL_SPI_Transmit(&DOT_DISPLAYER_SPI, buf, 2, HAL_MAX_DELAY) != HAL_OK)
  {
    HAL_GPIO_WritePin(DOT_GPIOx, CS_PIN, GPIO_PIN_SET);
    return ERR_RUN_ERROR_CALL;
  }
  HAL_GPIO_WritePin(DOT_GPIOx, CS_PIN, GPIO_PIN_SET);

  return ERR_RUN_Finished;
}

RESULT_RUN app_dotD_WriteALL(const u8 arr[8])
{
  if (arr == NULL)
    return ERR_RUN_ERROR_UDIP;

  for (u8 row = 1; row <= 8; row++)
  {
    RESULT_RUN ret = app_dotD_WriteLine(row, arr[row - 1]);
    if (ret != ERR_RUN_Finished)
      return ret;
  }

  return ERR_RUN_Finished;
}

RESULT_RUN app_dotD_Clear()
{
  for (u8 row = 1; row <= 8; row++)
  {
    RESULT_RUN ret = app_dotD_WriteLine(row, 0x00);
    if (ret != ERR_RUN_Finished)
      return ret;
  }

  return ERR_RUN_Finished;
}

RESULT_RUN app_dotD_Show_LEFT() { return app_dotD_WriteALL(app_dotD_LEFT); }
RESULT_RUN app_dotD_Show_RIGHT() { return app_dotD_WriteALL(app_dotD_RIGHT); }
RESULT_RUN app_dotD_Show_UP() { return app_dotD_WriteALL(app_dotD_UP); }
RESULT_RUN app_dotD_Show_START() { return app_dotD_WriteALL(app_dotD_START); }

RESULT_RUN app_dotD_TurnWrite(const u8 old[8], u8 ret[8])
{
  if (old == NULL || ret == NULL)
    return ERR_RUN_ERROR_UDIP;

  for (u8 r = 0; r < 8; r++)
  {
    u8 newRow = 0;
    for (u8 c = 0; c < 8; c++)
    {
      u8 bit = (old[7 - c] >> r) & 1;
      newRow |= bit << (7 - c);
    }
    ret[r] = newRow;
  }

  return ERR_RUN_Finished;
}

/* 线程开发：根据事件组驱动点阵显示 */
void app_dotD_dispose_Task()
{
  EventGroupHandle_t evt = event_bus_getHandle();
  u8 state = 0; /* 简易状态机 */

  /* 上电自检：显示 START 200ms */
  (void)app_dotD_Show_START();
  vTaskDelay(pdMS_TO_TICKS(200));
  (void)app_dotD_Clear();

  while (1)
  {
    EventBits_t bits = xEventGroupWaitBits(
        evt,
        EVT_TURN_LEFT | EVT_TURN_RIGHT | EVT_TURN_BACK | EVT_UP | EVT_USER_COM,
        pdTRUE, pdFALSE, portMAX_DELAY);

    /* 回正优先级最高，避免回正时先闪另一图案 */
    if (bits & EVT_TURN_BACK)
      state = 0;
    else if (bits & EVT_TURN_LEFT)
      state = 1;
    else if (bits & EVT_TURN_RIGHT)
      state = 2;
    else if (bits & EVT_UP)
      state = 3;
    else if (bits & EVT_USER_COM)
      state = 4;

    switch (state)
    {
    case 1:
      (void)app_dotD_Show_LEFT();
      break;
    case 2:
      (void)app_dotD_Show_RIGHT();
      break;
    case 3:
      (void)app_dotD_Show_UP();
      break;
    case 4:
      (void)app_dotD_Show_START();
      break;
    case 0:
    default:
      (void)app_dotD_Clear();
      break;
    }
  }
}

