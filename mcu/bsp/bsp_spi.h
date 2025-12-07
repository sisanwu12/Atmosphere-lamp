/**
 * @file		bsp_spi.h
 * @brief		定义抽象该模块的结构体以及声明操作该模块的函数
 * @author	王广平
 **/

/* 头文件引用 */
#include "ERR.h"
#include "__port_type__.h"
#include "stm32f1xx_hal_spi.h"

/* 函数声明 */

/**
 * @brief SPI 初始化函数
 *
 * @param hspi	传入SPI句柄
 * @param SPIx	指定SPI
 * @param Mode	指定SPI模式
 * @note
 * SPI_MODE_MASTER	主机模式
 * SPI_MODE_SLAVE		从机模式
 *
 * @param Direction		指定数据线方向模式
 * @note
 * SPI_DIRECTION_2LINES	全双工
 * SPI_DIRECTION_2LINES_RXONLY	只接收
 * SPI_DIRECTION_1LINE	半双工
 *
 * @param DataSize	指定单次传输的数据长度
 * @note
 * SPI_DATASIZE_8BIT		8bit
 * SPI_DATASIZE_16BIT		16bit
 *
 * @param CLKPolarity		指定时钟闲置时的电平
 * @note
 * SPI_POLARITY_LOW		低电平
 * SPI_POLARITY_HIGH	高电平
 * @param CLKPhase	指定采样的时钟边沿
 * @note
 * SPI_PHASE_1EDGE	上升沿采样
 * SPI_PHASE_2EDGE	下降沿采样
 *
 * @param NSS		指定片选管理
 * @note
 * SPI_NSS_SOFT		软件控制（手动操作片选线）
 * SPI_NSS_HARD_OUTPUT	硬件自动控制（主模式）
 * SPI_NSS_HARD_INPUT		硬件自动控制（从模式）
 *
 * @param BaudRatePrescaler		指定 SPI 时钟分频
 * @note
 * SPI_BAUDRATEPRESCALER_32 32分频
 * 可选：2、4、8、16、32、64、128、256
 *
 * @param FirstBit	指定传输顺序
 * @note
 * SPI_FIRSTBIT_MSB	高位先发
 * SPI_FIRSTBIT_LSB 低位先发
 * @param TIMode	指定是否启用 TI（美国德州仪器 TI）模式
 * @note
 * SPI_TIMODE_DISABLE		不启用
 * SPI_TIMODE_ENABLE		启用
 *
 * @param CRCCalculation	指定是否启用 CRC 校验
 * @note
 * SPI_CRCCALCULATION_DISABLE		不启用
 * SPI_CRCCALCULATION_ENABLE		启用
 * @param CRCPolynomial		指定 CRC 多项式的值
 * 未启用CRC则无效
 *
 * @return RESULT_Init 初始化结果
 */
RESULT_Init bsp_spi_Init(SPI_HandleTypeDef *hspi, SPI_TypeDef *SPIx, u32 Mode,
                         u32 Direction, u32 DataSize, u32 CLKPolarity,
                         u32 CLKPhase, u32 NSS, u32 BaudRatePrescaler,
                         u32 FirstBit, u32 TIMode, u32 CRCCalculation,
                         u32 CRCPolynomial);