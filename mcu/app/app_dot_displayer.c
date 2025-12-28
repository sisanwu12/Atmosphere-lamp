/**
 * @file		app_dot_displayer.c
 * @brief		定义操作该模块的函数
 * @author	王广平
 */

#define APP_DOT_DISPLAYER_C

/* 头文件引用 */
#include "app_dot_displayer.h"
#include "ERR.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "event_bus.h"
#include <string.h>
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_spi.h"

/* 全局变量 */
static SPI_HandleTypeDef DOT_DISPLAYER_SPI = {0};

/* 所用图案 */
static u8 app_dotD_LEFT[8];  /* 左转箭头图案 */
static u8 app_dotD_RIGHT[8]; /* 右转箭头图案 */
static u8 app_dotD_START[8]; /* 开始图案 */
static u8 app_dotD_UP[8];    /* 加速图案 */

/* 函数定义 */

/* 图案初始化函数 */
/**
 * @brief 初始化（并按 TurnCount 旋转）点阵显示用到的图案缓存
 *
 * @note
 * - APP_DOTD_* 是“原始图案”，放在只读区；app_dotD_* 是运行期使用的可写缓存
 * - TurnCount 为 0 时不旋转；为 N 时顺时针旋转 N 次（每次 90°）
 * - 旋转函数的输入/输出不能是同一块内存，因此这里使用临时缓冲区中转
 */
static RESULT_Init app_dotD_Pattern_Init(void)
{
  /* 1) 先拷贝一份原始图案到运行期缓存 */
  memcpy(app_dotD_LEFT, APP_DOTD_ARROW, sizeof(app_dotD_LEFT));
  memcpy(app_dotD_RIGHT, APP_DOTD_ARROW, sizeof(app_dotD_RIGHT));
  memcpy(app_dotD_START, APP_DOTD_START, sizeof(app_dotD_START));
  memcpy(app_dotD_UP, APP_DOTD_UP, sizeof(app_dotD_UP));

  /* 2) 根据安装方向旋转缓存图案（0 次时自然不进循环） */
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

RESULT_Init app_dotD_Init()
{
  RESULT_Init RET;
  /* 初始化GPIO */
  /* 时钟线与数据线初始化 */
  RET = bsp_gpio_AFPP_Init(DOT_GPIOx, CLK_PIN | DIN_PIN);
  if (RET != ERR_Init_Finished)
    return RET;
  /* 片选线初始化 */
  RET = bsp_gpio_OTPP_Init(DOT_GPIOx, CS_PIN);
  if (RET != ERR_Init_Finished)
    return RET;
  /* 拉高片选线 */
  HAL_GPIO_WritePin(DOT_GPIOx, CS_PIN, GPIO_PIN_SET);

  /* SPI 初始化 */
  RET = bsp_spi_Init(&DOT_DISPLAYER_SPI, DOT_SPI, SPI_MODE_MASTER,
                     SPI_DIRECTION_2LINES, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW,
                     SPI_PHASE_1EDGE, SPI_NSS_SOFT, SPI_BAUDRATEPRESCALER_32,
                     SPI_FIRSTBIT_MSB, SPI_TIMODE_DISABLE,
                     SPI_CRCCALCULATION_DISABLE, 0);
  if (RET != ERR_Init_Finished)
    return RET;

  app_dotD_WriteLine(0x0F, 0x00); // 关闭显示测试
  app_dotD_WriteLine(0x09, 0x00); // 无译码模式
  app_dotD_WriteLine(0x0A, 0x0F); // 亮度
  app_dotD_WriteLine(0x0B, 0x07); // 扫描 8 行
  app_dotD_WriteLine(0x0C, 0x01); // 开显示

  /* 改变图案方向 */
  RET = app_dotD_Pattern_Init();
  if (RET != ERR_Init_Finished)
    return RET;

  /* 清屏函数 */
  (void)app_dotD_Clear();

  /* 初始化完成 */
  return ERR_Init_Finished;
}

RESULT_RUN app_dotD_WriteLine(u8 addr, u8 data)
{
  RESULT_RUN RET;
  u8 buf[2] = {addr, data};

  HAL_GPIO_WritePin(DOT_GPIOx, CS_PIN, GPIO_PIN_RESET);
  RET = (RESULT_RUN)HAL_SPI_Transmit(&DOT_DISPLAYER_SPI, buf, 2, HAL_MAX_DELAY);
  if (RET != ERR_RUN_Finished)
    return RET;
  HAL_GPIO_WritePin(DOT_GPIOx, CS_PIN, GPIO_PIN_SET);

  return ERR_RUN_Finished;
}

RESULT_RUN app_dotD_WriteALL(const u8 arr[8])
{
  RESULT_RUN ret = ERR_RUN_Finished;

  /* MAX7219 行地址是 1~8；图案数组下标是 0~7 */
  for (u8 row = 1; row <= 8; row++)
  {
    ret = app_dotD_WriteLine(row, arr[row - 1]);
    if (ret != ERR_RUN_Finished)
      return ret;
  }
  return ret;
}

RESULT_RUN app_dotD_Clear()
{
  RESULT_RUN ret = ERR_RUN_Finished;

  for (u8 row = 1; row <= 8; row++)
  {
    ret = app_dotD_WriteLine(row, 0x00);
    if (ret != ERR_RUN_Finished)
      return ret;
  }

  return ret;
}

RESULT_RUN app_dotD_Show_LEFT() { return app_dotD_WriteALL(app_dotD_LEFT); }
RESULT_RUN app_dotD_Show_RIGHT() { return app_dotD_WriteALL(app_dotD_RIGHT); }
RESULT_RUN app_dotD_Show_UP() { return app_dotD_WriteALL(app_dotD_UP); }
RESULT_RUN app_dotD_Show_START() { return app_dotD_WriteALL(app_dotD_START); }

RESULT_RUN app_dotD_TurnWrite(const u8 old[8], u8 ret[8])
{
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

void app_dotD_dispose_Task()
{
  EventGroupHandle_t evt = event_bus_getHandle();
  u8 state = 0; /* 简易状态机 */
  while (1)
  {
    EventBits_t bits = xEventGroupWaitBits(
        evt,
        EVT_TURN_LEFT | EVT_TURN_RIGHT | EVT_TURN_BACK | EVT_UP | EVT_USER_COM,
        pdTRUE, pdFALSE, portMAX_DELAY);

    if (bits & EVT_TURN_LEFT)
    {
      state = 1; /* 左转状态 */
    }
    else if (bits & EVT_TURN_RIGHT)
    {
      state = 2; /* 右转状态 */
    }
    else if (bits & EVT_UP)
    {
      state = 3;
    }
    else if (bits & EVT_USER_COM)
    {
      state = 4;
    }
    else if (bits & EVT_TURN_BACK)
    {
      state = 0; /* 回正状态/常态 */
    }
    switch (state)
    {
    case 1: // 左转状态
      app_dotD_Show_LEFT();
      break;

    case 2: // 右转状态
      app_dotD_Show_RIGHT();
      break;

    case 3: // 加速状态
      app_dotD_Show_UP();
      break;

    case 4: // 加速状态
      app_dotD_Show_START();
      break;

    case 0: // 回正状态/常态
    default:
      app_dotD_Clear();
      break;
    }
  }
}
