// stlink.cpp - stlink bridge api implementation

#include "stlink.h"
#include <cstdlib>
#include <stdio.h>
#include "bridge.h"



extern "C" {


STLinkInterface *g_pstlink = NULL;

Brg* g_pbridge = NULL;


Brg_StatusT stlink_load_library(void);

Brg_StatusT stlink_enum_devices(int* pcount, int* pid, char* serial_number);

Brg_StatusT stlink_open_bridge(int id);



stlink_res_t stlink_init(char* serial_number)
{
	stlink_done();
	g_pstlink = new STLinkInterface(STLINK_BRIDGE);
	//g_pstlink->BindErrLog(&g_ErrLog);
	Brg_StatusT bridge_status = stlink_load_library();
	if (bridge_status == BRG_NO_ERR)
	{
		int count = 0;
		int id = -1;
		bridge_status = stlink_enum_devices(&count, &id, serial_number);
		if (bridge_status == BRG_NO_ERR)
			bridge_status = stlink_open_bridge(id);
	}
	return (stlink_res_t)bridge_status;
}

void stlink_done(void)
{
	if (g_pbridge != NULL)
	{
		g_pbridge->CloseBridge(COM_UNDEF_ALL);
		g_pbridge->CloseStlink();
		delete g_pbridge;
		g_pbridge = NULL;
	}
	if (g_pstlink != NULL)
	{
		delete g_pstlink;
		g_pstlink = NULL;
	}
}

Brg_StatusT stlink_load_library(void)
{
	if (g_pstlink == NULL)
		return BRG_NO_STLINK;
	char path[MAX_PATH];
#ifdef WIN32 //Defined for applications for Win32 and Win64.
	GetModuleFileNameA(NULL, path, MAX_PATH);
	// Remove process file name from the path
	char* pcend = strrchr(path,'\\');
	if (pcend != NULL)
	{
		*(pcend + 1) = '\0';
	}
#else
	strcpy(path, "");
#endif
	// Load STLinkUSBDriver library
	// STLinkUSBdriver (dll on windows) must be copied near test executable
	STLinkIf_StatusT ifstat = g_pstlink->LoadStlinkLibrary(path);
	if (ifstat != STLINKIF_NO_ERR)
	{
		fprintf(stderr, "STLinkUSBDriver library (dll) issue.\n");
	}
	return Brg::ConvSTLinkIfToBrgStatus(ifstat);
}

Brg_StatusT stlink_enum_devices(int* pcount, int* pid, char* serial_number)
{
	if (g_pstlink == NULL)
		return BRG_NO_STLINK;
	uint32_t count = 0;
	STLinkIf_StatusT ifstat = g_pstlink->EnumDevices(&count, false);
	if ((ifstat == STLINKIF_NO_ERR) || (ifstat == STLINKIF_PERMISSION_ERR))
	{
		if (pcount) *pcount = count;
		printf("%d BRIDGE device(s) found\n", (int)count);
		int id = -1;
		for (int i = 0; i < (signed)count; i++)
		{
			STLink_DeviceInfo2T devinfo2;
			ifstat = g_pstlink->GetDeviceInfo2(i, &devinfo2, sizeof(devinfo2));
			if (serial_number == NULL)
			{
				if (devinfo2.DeviceUsed == false)
				{
					printf("Bridge %d PID: 0X%04hx SN:%s\n", (int)i, (unsigned short)devinfo2.ProductId, devinfo2.EnumUniqueId);
					id = i;
					break;
				}
			}
			else
			{
				if (strcmp(devinfo2.EnumUniqueId, serial_number) == 0)
				{
					if (devinfo2.DeviceUsed == false)
					{
						printf("Bridge %d PID: 0X%04hx SN:%s\n", (int)i, (unsigned short)devinfo2.ProductId, devinfo2.EnumUniqueId);
						id = i;
						break;
					}
				}
			}
		}
		if (pid) *pid = id;
	}
	else if (ifstat == STLINKIF_CONNECT_ERR)
	{
		fprintf(stderr, "No STLink BRIDGE device detected.\n");
	}
	else
	{
		fprintf(stderr, "STLink enum error.\n");
	}
	return Brg::ConvSTLinkIfToBrgStatus(ifstat);
}

Brg_StatusT stlink_open_bridge(int id)
{
	if (g_pstlink == NULL)
		return BRG_NO_STLINK;
	g_pbridge = new Brg(*g_pstlink);
	g_pbridge->SetOpenModeExclusive(true);
	return g_pbridge->OpenStlink(id);
}


} // extern "C"
