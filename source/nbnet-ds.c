/* README: To make this work, compile and execute the server in nbnet/examples/echo/ before launching the game with the Citra emulator.
           Remember to compile it with network support if you're building Citra from source!
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include <fcntl.h>

#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <3ds.h>

#define NBNET_IMPL

// NOTE: This is here to avoid nbnet spamming warnings during compilation
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wint-conversion"


//#include "nbnet/nbnet.h"
#include "nbnet/examples/echo/shared.h"
//#include "nbnet/net_drivers/udp.h"

#pragma GCC diagnostic pop // enable warnings again

#define SOC_ALIGNMENT 0x1000
// TODO: Find ideal size for soc memory?
#define SOC_SIZE (SOC_ALIGNMENT * 10)

u32 *soc_buffer;
s32 soc_descriptor = -1;
int nbn_started = 0;

static const char *log_type_strings[] = {
    "INFO",
    "ERROR",
    "DEBUG",
    "TRACE"
};

// Basic logging function
void Log(int type, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    printf("[%s] ", log_type_strings[type]);
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
}

int main()
{
    	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

        printf("TEST NDNET\n");

        soc_buffer = memalign(SOC_ALIGNMENT, SOC_SIZE);

        if (soc_buffer)
        {
            printf("Memory initialization OK\n");

            if (socInit(soc_buffer, SOC_SIZE))
            {
                printf("Error trying to init SOC service\n");
            }
            else
            {
                printf("SOC service initialized\n");
                
                NBN_GameClient_Init(ECHO_PROTOCOL_NAME, "127.0.0.1", ECHO_EXAMPLE_PORT);
                NBN_GameClient_RegisterMessage(ECHO_MESSAGE_TYPE, EchoMessage);

                if (NBN_GameClient_Start() < 0)
                {
                    printf("Failed so start NBN client\n");
                    // Deinit the client
                    NBN_GameClient_Deinit();
                }
                else
                {
                    nbn_started = 1;
                    printf("NBN client started\n");
                }
            }
        }
                                

        while (aptMainLoop())
	{
            gspWaitForVBlank();
            gfxSwapBuffers();
            hidScanInput();

            u32 kDown = hidKeysDown();

            if (kDown & KEY_START)
                break;

            static bool running = true;
            static bool connected = false;
            static bool disconnected = false;
            
            if (nbn_started && running)
            {
                // Number of seconds between client ticks
                // NOTE: This may be useful for a more involved example, but we're already locked at
                // 60fps by the aptMainLoop() function so we don't sleep to simplify the example
                double dt = 1.0 / ECHO_TICK_RATE;

                // Update client clock
                NBN_GameClient_AddTime(dt);
                int ev;
                    
                while ((ev = NBN_GameClient_Poll()) != NBN_NO_EVENT)
                {
                    if (ev < 0)
                    {
                        Log(LOG_ERROR, "An error occured while polling client events.");
                        running = false;
                        break;
                    }
                    switch (ev)
                    {
                    case NBN_CONNECTED:
                        Log(LOG_INFO, "Connected");
                        connected = true;
                        break;

                    case NBN_DISCONNECTED:
                        Log(LOG_INFO, "Disconnected");
                        connected = false;
                        disconnected = true;
                        break;

                    case NBN_MESSAGE_RECEIVED:
                        ; // empty statement to avoid label preceding declaration error
                        // Get info about the received message
                        NBN_MessageInfo msg_info = NBN_GameClient_GetReceivedMessageInfo();
                        assert(msg_info.type == ECHO_MESSAGE_TYPE);
                            
                        // Retrieve the received message
                        EchoMessage *msg = (EchoMessage *)msg_info.data;
                        Log(LOG_INFO, "Received echo: %s (%d bytes)", msg->data, msg->length);
                        NBN_GameClient_DestroyMessage(ECHO_MESSAGE_TYPE, msg);
                            
                        break;
                    }
                }

                if (disconnected)
                    break;

                if (connected)
                {
                    char *msg = "Testing from Citra\n";
                    unsigned int length = strlen(msg); // Compute message length

                    // Create new reliable EchoMessage
                    EchoMessage *echo = NBN_GameClient_CreateReliableMessage(ECHO_MESSAGE_TYPE);

                    if (echo == NULL)
                        return -1;

                    // Fill EchoMessage with message length and message data
                    echo->length = length + 1;
                    memcpy(echo->data, msg, length + 1);

                    // Send it to the server
                    if (NBN_GameClient_SendMessage() < 0)
                        Log(LOG_ERROR, "Unable to send message to server");
                    else
                        Log(LOG_INFO, "Sent message to server");
                }

                // Pack all enqueued messages as packets and send them
                if (NBN_GameClient_SendPackets() < 0)
                {
                    Log(LOG_ERROR, "Failed to send packets. Exit");

                    // Stop main loop
                    running = false;
                    break;
                }
            }        
        }
        // Stop the client
        NBN_GameClient_Stop();

        // Deinit the client
        NBN_GameClient_Deinit();

        socExit();
        gfxExit();
        return 0;
}
