// can_stlink.cpp - common CAN low-level interface implementation for stlink

#include "can.h"
#include "stlink.h"
#include <stdio.h>
#include <string.h>
#include "bridge.h"


extern STLinkInterface* g_pstlink;
extern Brg* g_pbridge;


can_res_t can_init(can_mode_t mode, uint32_t baudrate)
{
	stlink_init(NULL);
	if ((g_pstlink == NULL) || (g_pbridge == NULL))
		return can_ERR_NOT_INIT;
	uint8_t com = COM_CAN;
	uint32_t stlink_hclk_khz = 0;
	uint32_t can_input_clk_khz = 0;
	// Get the current bridge input Clk
	Brg_StatusT bridge_status = g_pbridge->GetClk(com, &can_input_clk_khz, &stlink_hclk_khz);
	printf( "CAN input CLK: %d KHz, STLink HCLK: %d KHz \n", (int)can_input_clk_khz, (int)stlink_hclk_khz);
	Brg_CanInitT can_param;
	// N=sync+prop+seg1+seg2= 1+2+7+6= 16, 125000 bps (-> prescal = 24 = (CanClk = 48MHz)/(16*125000))
	can_param.BitTimeConf.PropSegInTq = 2;
	can_param.BitTimeConf.PhaseSeg1InTq = 7;
	can_param.BitTimeConf.PhaseSeg2InTq = 6;
	can_param.BitTimeConf.SjwInTq = 3;
	uint32_t prescaler = 0;
	uint32_t final_baudrate = 0;
	bridge_status = g_pbridge->GetCANbaudratePrescal(&can_param.BitTimeConf, baudrate, &prescaler, &final_baudrate);
	printf(" baudrate= %d\n", (int)baudrate);
	printf(" prescaler= %d\n", (int)prescaler);
	printf(" PropSegInTq= %d\n", (int)can_param.BitTimeConf.PropSegInTq);
	printf(" PhaseSeg1InTq= %d\n", (int)can_param.BitTimeConf.PhaseSeg1InTq);
	printf(" PhaseSeg2InTq= %d\n", (int)can_param.BitTimeConf.PhaseSeg2InTq);
	printf(" final_baudrate= %d\n", (int)final_baudrate);
	if (bridge_status == BRG_NO_ERR )
	{
		can_param.Prescaler = prescaler;
		can_param.Mode = (Brg_CanModeT)mode;
		can_param.bIsTxfpEn = false;
		can_param.bIsRflmEn = false;
		can_param.bIsNartEn = false;
		can_param.bIsAwumEn = false;
		can_param.bIsAbomEn = false;
		bridge_status = g_pbridge->InitCAN(&can_param, BRG_INIT_FULL);
	}
	else if (bridge_status == BRG_COM_FREQ_MODIFIED)
	{
		fprintf(stderr, "WARNING Bridge CAN init baudrate asked %d bps but applied %d bps.\n", (int)baudrate, (int)final_baudrate);
		return can_ERR_HW;
	}
	else if (bridge_status == BRG_COM_FREQ_NOT_SUPPORTED)
	{
		fprintf(stderr, "ERROR Bridge CAN init baudrate %d bps not possible (invalid prescaler: %d) change Bit Time or baudrate settings.\n", (int)baudrate, (int)prescaler);
		return can_ERR_HW;
	}
	return can_OK;
}

can_res_t can_done(void)
{
	stlink_done();
	return can_OK;
}

can_res_t can_set_filter_mask32(uint8_t bank, can_filter_mask32_t* pf)
{
	if ((g_pstlink == NULL) || (g_pbridge == NULL))
		return can_ERR_NOT_INIT;
	Brg_CanFilterConfT filter_conf;
	filter_conf.AssignedFifo = pf->fifo?CAN_MSG_RX_FIFO1:CAN_MSG_RX_FIFO0;
	filter_conf.bIsFilterEn = (pf->enabled)?true:false;
	filter_conf.FilterBankNb = bank;
	filter_conf.FilterMode = CAN_FILTER_ID_MASK;
	filter_conf.FilterScale = CAN_FILTER_32BIT;
	filter_conf.Id[0].ID = pf->id;
	filter_conf.Id[0].IDE = (pf->ext)?CAN_ID_EXTENDED:CAN_ID_STANDARD;
	filter_conf.Id[0].RTR = (pf->rtr)?CAN_REMOTE_FRAME:CAN_DATA_FRAME;
	filter_conf.Mask[0].ID = pf->mask;
	filter_conf.Mask[0].IDE = (pf->ext)?CAN_ID_EXTENDED:CAN_ID_STANDARD;
	filter_conf.Mask[0].RTR = (pf->rtr)?CAN_REMOTE_FRAME:CAN_DATA_FRAME;
	Brg_StatusT bridge_status = g_pbridge->InitFilterCAN(&filter_conf);
	if (bridge_status != BRG_NO_ERR)
	{
		fprintf(stderr, "CAN filter init failed (mask32, bank=%d, id=0x%x, mask=0x%x).\n", bank, pf->id, pf->mask);
		return can_ERR_HW;
	}
	return can_OK;
}

can_res_t can_start(void)
{
	if ((g_pstlink == NULL) || (g_pbridge == NULL))
		return can_ERR_NOT_INIT;
	Brg_StatusT bridge_status = g_pbridge->StartMsgReceptionCAN();
	if (bridge_status != BRG_NO_ERR )
	{
		printf("CAN StartMsgReceptionCAN failed.\n");
		return can_ERR_HW;
	}
	return can_OK;
}

can_res_t can_stop(void)
{
	if ((g_pstlink == NULL) || (g_pbridge == NULL))
		return can_ERR_NOT_INIT;
	Brg_StatusT bridge_status = g_pbridge->StopMsgReceptionCAN();
	if (bridge_status != BRG_NO_ERR )
	{
		printf("CAN StopMsgReceptionCAN failed.\n");
		return can_ERR_HW;
	}
	return can_OK;
}

can_res_t can_tx_msg(can_msg_t* pmsg)
{
	if ((g_pstlink == NULL) || (g_pbridge == NULL))
		return can_ERR_NOT_INIT;
	Brg_CanTxMsgT can_msg =
	{
		.IDE = pmsg->ext?CAN_ID_EXTENDED:CAN_ID_STANDARD,
		.ID = pmsg->id,
		.RTR = pmsg->rtr?CAN_REMOTE_FRAME:CAN_DATA_FRAME,
		.DLC = pmsg->dl,
	};
	Brg_StatusT bridge_status = g_pbridge->WriteMsgCAN(&can_msg, pmsg->data, pmsg->dl);
	return can_OK;
}

can_res_t can_rx_msg(can_msg_t* pmsg)
{
	if ((g_pstlink == NULL) || (g_pbridge == NULL))
		return can_ERR_NOT_INIT;
	uint16_t count = 0;
	Brg_StatusT bridge_status = g_pbridge->GetRxMsgNbCAN(&count);
	if (bridge_status != BRG_NO_ERR)
		return can_ERR_HW;
	if (count == 0)
		return can_ERR_RX_EMPTY;
	Brg_CanRxMsgT can_msg;
	uint8_t data[8];
	uint16_t data_size;
	bridge_status = g_pbridge->GetRxMsgCAN(&can_msg, 1, data, 8, &data_size);
	if (bridge_status == BRG_NO_ERR)
	{
		if (pmsg)
		{
			pmsg->id = can_msg.ID;
			pmsg->ext = can_msg.IDE;
			pmsg->rtr = can_msg.RTR;
			pmsg->dl = data_size;
			memcpy(pmsg->data, data, data_size);
		}
		return can_OK;
	}
	if (bridge_status == BRG_OVERRUN_ERR)
		return can_ERR_RX_OVER;
	return can_ERR_HW;
}
