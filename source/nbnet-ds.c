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

#define SOC_ALIGNMENT 0x1000
#define SOC_SIZE 0x100000

u32 *soc_buffer;
s32 client_socket = -1;
s32 server_socket = -1;
bool all_good = false;

struct sockaddr_in broadcast_sockaddr;
struct sockaddr_in system_sockaddr;

int main()
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

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

            client_socket = socket(AF_INET, SOCK_DGRAM, 0);
            server_socket = socket(AF_INET, SOCK_DGRAM, 0);

            if (client_socket == -1 || server_socket == -1)
            {
                printf("Error trying to get socket descriptor: %s\n", strerror(errno));
            }
            else
            {

                struct in_addr broadcast_addr;
                
                        
                int getip_ret = SOCU_GetIPInfo(NULL, NULL, &broadcast_addr);
                if (getip_ret < 0)
                {
                    printf("Error trying to get IP info: %s\n", strerror(errno));
                }
                else
                {                            
                    broadcast_sockaddr.sin_addr = broadcast_addr;
                    broadcast_sockaddr.sin_port = htons(12345);
                    broadcast_sockaddr.sin_family = AF_INET;

                    system_sockaddr.sin_addr.s_addr = INADDR_ANY;
                    system_sockaddr.sin_port = htons(12345);
                    system_sockaddr.sin_family = AF_INET;


                    if (fcntl(client_socket, F_SETFL, O_NONBLOCK, 1) || fcntl(server_socket, F_SETFL, O_NONBLOCK, 1))
                    {
                        printf("Error trying to set descriptor flags: %s\n", strerror(errno));
                    }
                    else
                    {

                        if (bind(server_socket, (const struct sockaddr *)&system_sockaddr, sizeof(system_sockaddr)))
                        {
                            printf("Error trying to bind socket for listening: %s\n", strerror(errno));
                        }
                        else
                        {
                            all_good = true;                                    
                        }                                        
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
            // All good
            unsigned char buf[128];
            struct sockaddr sender_addr;
            socklen_t sender_addr_len = sizeof(sender_addr);
            int recv_ret;

            while ((recv_ret = recvfrom(server_socket, buf, 128, 0, &sender_addr, &sender_addr_len)) != -1)
            {
                printf("Received %d bytes from address %s\n", recv_ret, inet_ntoa(*((struct in_addr*)&sender_addr)));
            }

            switch (errno)
            {

            case EAGAIN:
                // NOTE: depending on the platform we may have to check EWOULDBLOCK too
                break;
            default:
                printf("Error polling for packets: %s\n", strerror(errno));
                all_good = false;
            }
                

            const char *msg = "Potato";
            int bytes_sent = sendto(client_socket, msg, strlen(msg) + 1, 0, (const struct sockaddr *)&broadcast_sockaddr, sizeof(broadcast_sockaddr));

            if (bytes_sent == -1)
            {
                printf("Error sending datagram: %s\n", strerror(errno));
            }
            else
            {
                printf("Sent %d bytes to broadcast address\n", bytes_sent);
            }                
        }
        else
        {
            static bool shown_error = false;
            if (!shown_error)
            {
                printf("Something went wrong. Not sending packets.\n");
                shown_error = true;
            }
        }                           
    }

    socExit();
    gfxExit();
    return 0;
}
