
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sched.h>
#include "can.h"


//#define LOOPBACK


#ifdef _WIN32

#include <windows.h>
#include <conio.h>


// line buffer for stdout
char _lbf[1024];


#else

#include <fcntl.h>

#endif //_WIN32


int main(int argc, char** argv)
{

#ifdef _WIN32
	setvbuf(stdout, _lbf, _IOLBF, sizeof(_lbf)); // setup stdout - line buffered (this is necessary for old mingw)
#endif //_WIN32

	can_res_t cres;

#ifdef LOOPBACK
	if ((cres = can_init(can_mode_LOOPBACK, 1000000)) == can_OK)
#else // LOOPBACK
	if ((cres = can_init(can_mode_NORMAL, 1000000)) == can_OK)
#endif // LOOPBACK
	{

		fflush(stdout);

		can_filter_mask32_t filter =
		{
			.fifo = 0,
//			.id = 0x001,   // canopen nodeid 0x01
//			.mask = 0x07f, // canopen nodeid 0x01
			.id = 0x000,   // all messages
			.mask = 0x000, // all messages
			.ext = 0,
			.rtr = 0,
			.enabled = 1,
		};

		cres = can_set_filter_mask32(0, &filter);
		cres = can_start();

#ifndef _WIN32
		fcntl(0, F_SETFL, O_NONBLOCK); // non-blocking stdin
#endif // _WIN32

		int i = 0;
		while (1)
		{
#ifdef LOOPBACK
			// transmit any message in loopbackmode
			can_msg_t txmsg = { .id = 0x701, .dl = 8, .data = {1,2,3,4,5,6,7,8} };
			can_tx_msg(&txmsg);
#endif // LOOPBACK

			char ch;
#ifdef _WIN32
			if (kbhit() && (ch = getchar())) // test any key pressed and read char
#else // _WIN32
			ch = getchar(); // read char
			if (ch != 0) // !=0 means any key pressed
#endif // _WIN32
			{
				break; // exit loop
			}
			can_msg_t msg;
			memset(&msg, 0, sizeof(msg));
			if ((cres = can_rx_msg(&msg)) == can_OK) // receive message
			{
				// trace message
				printf("%08x ID=0x%03x dl=%d", i, msg.id, msg.dl);
				for (int j = 0; j < msg.dl; j++)
					printf(" %02x", msg.data[j]);
				printf("\n");
//				fflush(stdout);
			}
			else
			{
				switch (cres)
				{
				case can_ERR_RX_EMPTY:
					sched_yield();
					break;
				case can_ERR_RX_OVER:
					printf("RX overflow\n");
					break;
				default:
					printf("Unknown error\n");
					break;
				}
			}
			i++;
		}
		can_stop(); // stop reception
		can_done(); // shutdown
	}

	return 0;
}
