// #include "CAN_RxDataHandle.h"
// #include "bsp_can.h"
// #include "bsp_usart.h"
// #include <stdio.h>
// #include <string.h>

// // ��ʼ��CAN���λ�����
// void can_blockbuffer_init(void)
// {
//   can_rb = bb_init(CAN_BLOCKBUFFER_SIZE, CAN_BLOCKBUFFER_UNIT_SIZE);
//   if (!can_rb)
//   {
//     printf("CAN blockbuffer init failed\n");
//     size_t total_size = CAN_BLOCKBUFFER_SIZE * CAN_BLOCKBUFFER_UNIT_SIZE +
//                         sizeof(struct blockbuffer);
//     printf("Required memory: %d bytes\n", total_size);
//     return;
//   }
// }

// // ����CAN����
// bool can_send_message(const can_message_t *msg)
// {
//   if (msg == NULL || msg->len > 8)
//   {
//     printf("CAN send failed: invalid msg\n");
//     return false;
//   }

//   CAN_TxHeaderTypeDef TxHeader;
//   uint32_t TxMailbox;

//   /* ��׼֡/��չ֡���� */
//   if (msg->extended_id)
//   {
//     TxHeader.IDE = CAN_ID_EXT;
//     TxHeader.ExtId = msg->id;
//     TxHeader.StdId = 0;
//   }
//   else
//   {
//     TxHeader.IDE = CAN_ID_STD;
//     TxHeader.StdId = msg->id;
//     TxHeader.ExtId = 0;
//   }

//   TxHeader.RTR = msg->remoteFrame ? CAN_RTR_REMOTE : CAN_RTR_DATA;
//   TxHeader.DLC = msg->len;
//   TxHeader.TransmitGlobalTime = DISABLE;
// }

// // ��FIFO���ݴ��뻺����
// void can_data_to_blockbuffer(void)
// {
//   CAN_RxHeaderTypeDef RxHeader;
//   uint8_t u8_rx_data[8];

//   /* �� FIFO0 ��ȡ�յ��ı��� */
//   if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, u8_rx_data) ==
//       HAL_OK)
//   {
//     can_message_t msg; // �ֲ�����
//     void *wrptr = bb_wrptr_get_only(can_rb);

//     /* ��� msg �ֶΣ�ע�ⳤ�ȱ߽� */
//     msg.id = (RxHeader.IDE == CAN_ID_EXT) ? RxHeader.ExtId : RxHeader.StdId;
//     msg.len = (RxHeader.DLC <= 8) ? RxHeader.DLC : 8;
//     msg.extended_id = (RxHeader.IDE == CAN_ID_EXT);
//     msg.remoteFrame = (RxHeader.RTR == CAN_RTR_REMOTE);
//     memcpy(msg.data, u8_rx_data, msg.len);

//     if (wrptr)
//     {
//       memcpy(wrptr, &msg, sizeof(can_message_t));
//       bb_wrptr_shift(can_rb);
//     }
//     else
//     {
//       /* ��д���������ȷ��д��ɹ� */
//       bb_rdptr_shift(can_rb);
//       wrptr = bb_wrptr_get_only(can_rb);
//       if (wrptr)
//       {
// 	memcpy(wrptr, &msg, sizeof(can_message_t));
// 	bb_wrptr_shift(can_rb);
//       }
//     }
//   }
// }
// bool can_read_message(can_message_t *msg)
// {
//   if (msg == NULL || !can_rb)
//     return false;

//   void *rdptr = bb_rdptr_get_only(can_rb);
//   if (rdptr)
//   {
//     memcpy(msg, rdptr, sizeof(can_message_t));
//     bb_rdptr_shift(can_rb);
//     return true;
//   }
//   return false;
// }

// // CAN����ص�
// void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
// {
//   printf("CAN Error: 0x%08lX\n", HAL_CAN_GetError(hcan));
// }

// // CAN�����жϻص�
// void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
// {
//   if (hcan == &hcan1)
//   {
//     can_data_to_blockbuffer();
//   }
// }

// // ��������
// void process_powertrain_message(can_message_t *msg)
// {
//   switch (msg->id)
//   {
//   case 0x00A1:
//     printf("Engine Speed: %d RPM\n", (msg->data[1] << 8) | msg->data[0]);
//     break;
//   case 0x00B2:
//     printf("Vehicle Speed: %d km/h\n", msg->data[0]);
//     break;
//   case 0x00C3:
//     printf("Gear Position: %d\n", msg->data[0]);
//     break;
//   default:
//     printf("Unknown Powertrain ID: 0x%03X\n", msg->id);
//     break;
//   }
// }

// // �����밲ȫ����
// void process_chassis_safety_message(can_message_t *msg)
// {
//   switch (msg->id)
//   {
//   case 0x0101:
//     printf("Brake Pedal: %d%%\n", msg->data[0]);
//     break;
//   case 0x0120:
//     printf("ABS Wheel Speed: %d %d %d %d\n", msg->data[0], msg->data[1],
//            msg->data[2], msg->data[3]);
//     break;
//   case 0x0150:
//     printf("Airbag Status: 0x%02X\n", msg->data[0]);
//     break;
//   default:
//     printf("Unknown Chassis ID: 0x%03X\n", msg->id);
//     break;
//   }
// }

// // ��������
// void process_body_message(can_message_t *msg)
// {
//   switch (msg->id)
//   {
//   case 0x0210:
//     printf("Door Status: 0x%02X\n", msg->data[0]);
//     break;
//   case 0x0230:
//     printf("Window Status: 0x%02X\n", msg->data[0]);
//     break;
//   case 0x0250:
//     printf("Light Switch: 0x%02X\n", msg->data[0]);
//     break;
//   default:
//     printf("Unknown Body ID: 0x%03X\n", msg->id);
//     break;
//   }
// }

// // ��Ϣ��������
// void process_infotainment_message(can_message_t *msg)
// {
//   switch (msg->id)
//   {
//   case 0x0310:
//     printf("Volume: %d\n", msg->data[0]);
//     break;
//   case 0x0320:
//     printf("Next Track\n");
//     break;
//   case 0x0330:
//     printf("Previous Track\n");
//     break;
//   default:
//     printf("Unknown Infotainment ID: 0x%03X\n", msg->id);
//     break;
//   }
// }
