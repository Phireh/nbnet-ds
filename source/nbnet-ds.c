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
#include <netdb.h>

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

bool all_good = false;

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
                printf("SOC service initialized. Trying to get a broadcast socket...\n");

                soc_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

                if (soc_descriptor == -1)
                {
                    printf("Error trying to get socket descriptor: %s\n", strerror(errno));
                }
                else
                {
                    if (fcntl(soc_descriptor, F_SETFL, O_NONBLOCK, 1))
                    {
                        printf("Error trying to set descriptor flags: %s\n", strerror(errno));
                    }
                    else
                    {
                        // All good!
                        // NOTE: Apparently SO_BROADCAST is unneeded in devkitpro?
                        // https://libctru.devkitpro.org/socket_8h_source.html - line 45

                        struct addrinfo hints;
                        struct addrinfo *res;

                        memset(&hints, 0, sizeof(hints));
                        hints.ai_family = AF_UNSPEC;
                        hints.ai_socktype = SOCK_DGRAM;


                        if (getaddrinfo("255.255.255.255", "12345", &hints, &res))
                        {
                            printf("Error trying to get broadcast address: %s\n", gai_strerror(errno));
                        }
                        else
                        {
                            // All good
                            //bind(soc_descriptor, (struct sockaddr*) &broadcast_addr, sizeof(broadcast_addr));

                            char human_str[128];
                            inet_ntop(res->ai_family, res->ai_addr, human_str, 128);
                            printf("Our IP is %s\n", human_str);
                            
                        }                        
                    }
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


            if (all_good)
            {
                
            }
        }

        socExit();
        gfxExit();
        return 0;
}
