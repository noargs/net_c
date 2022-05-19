/**
 * socket has to be initalized in "window" in order to use them
 * however in unix they are initialised by default.
 * 
 * below code is an effort to introduce cross platform code with
 * the help of macros, which would generate different code, 
 * depending on OS after macro expansion in preprocessed phase 
 * before being fed into compiler
 * 
 * i.e. code inside #if defined(_WIN_32) will only run on Windows
 * 
 * (unix) $ gcc main.c -o socket_init; ./socket_init
 * 
 * (windows) $ gcc main.c -o socket_init.exe -lws2_32; sock_init.exe 
 * 
 */


/**
 * @brief necessary window initialisation
 * 
 */

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif

#include <stdio.h>

int main() {

#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        fprintf(stderr, "Failed to initialise. \n");
        return 1;
    }
#endif

    printf("Ready to use socket API.\n");


#if defined(_WIN32)
    WSACleanup();    
#endif

}