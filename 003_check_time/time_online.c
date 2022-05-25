#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT      0x0600
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

#if defined(_WIN32)
#define ISVALIDSOCKET (s)        ((s) != INVALID_SOCKET)
#define CLOSESOCKET (s)          closesocket(s)
#define GETSOCKETERRNO ()        (WSAGetLastError())

#else
#define ISVALIDSOCKET(s)        ((s) >= 0)
#define CLOSESOCKET(s)          close(s)
#define SOCKET                  int
#define GETSOCKETERRNO()        (errno)
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

int main() {
    
#if defined(_WIN32)
    WSADATA d;
    if (WSAStratup(MAKEWORD(2, 2), &d)) {
        fprintf(stderr, "Failed to initialise. \n");
        return 1;
    }   
#endif    

    /**
     * find the local address our web server should bind to
     */
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;         // IPv4, for IPv6 use AF_INET6
    hints.ai_socktype = SOCK_STREAM;   // for TCP, for UDP use "SOCK_DGRAM"
    hints.ai_flags = AI_PASSIVE;       // bind to wildcard address i.e. listen to any avaiblable network

    struct addrinfo *bind_address;     // holds the return information from "getaddrinfo()"
    
    /**
     * getaddrinfo has many uses, one of them is it
     * generates an address that is suitable for bind()
     * to make it generate this we must pass first parameter 
     * as null and AI_PASSIVE flag set in hints.ai_flags
     * 
     * it's common to see program not using this function
     * but it make easy to convert our program from AF_INET
     * to AF_INET6 which otherwise very tedious 
     */
    getaddrinfo(0, "8080", &hints, &bind_address);

    
    printf("Creating socket...\n");

    /**
     * create and initialises new socket
     */
    SOCKET socket_listen = socket(bind_address->ai_family, 
                                  bind_address->ai_socktype, 
                                  bind_address->ai_protocol);



    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("%d\n", socket_listen);

    printf("Bind socket to local address with bind()...\n");

    /**
     * returns 0 on success and non-zero on failure.
     * fails only if the port we are binding to is 
     * already in use.
     * 
     * bind() associates a socket (i.e. tcp or udp) with 
     * a particular local IP address and port number
     */
    if (bind(socket_listen, 
             bind_address->ai_addr, 
             bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    freeaddrinfo(bind_address);


    printf("Listening...\n");

    /**
     * second argument 10 tells listen() how may
     * connection it is allowed to queue up
     */
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Waiting for connection...\n");
    
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);

    /**
     * accept() when called block your program until a new
     * connection is made. in other word, program will 
     * sleep until a connection is made to the listening socket
     * 
     * when new connection is made accept() will create a 
     * new socket for it and your original socket continues
     * to listen for new connections
     * 
     * new socket returned by accept() can be used to send or
     * receive data over newly established connection.
     * 
     * accept() also fills in address info of the client that 
     * connected.
     */
    SOCKET socket_client = accept(socket_listen,
                (struct sockaddr*) &client_address, &client_len);
    if (!ISVALIDSOCKET(socket_client)) {
        fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Client is connected... ");
    
    char address_buffer[100];
    
    /**
     * at this point, a TCP connection has been established
     * to a remote client. we can print the client's address
     * to the console
     * 
     * address length (i.e. client_len) is needed because
     * getnameinfo() can work with both IPv4 and IPv6 addresses
     * 
     * address_buffer[100] is where getnameinfo() writes its
     * hostname output to
     * 
     * next two arguments (i.e. 0, 0) specify a second buffer
     * and its length. getnameinfo() outputs the service name
     * into this buffer but we aren't bothered so 0s was 
     * given at this day
     * 
     * NI_NUMERICHOST flag specifies that we want to see the
     * hostname as an IP address
     */
    getnameinfo((struct sockaddr*)&client_address,
            client_len, address_buffer, sizeof(address_buffer), 0, 0,
            NI_NUMERICHOST);

    printf("%s\n", address_buffer);

    printf("Reading request...\n");

    char request[1024];
    
    /**
     * as in server-client model we expect the client
     * (i.e. a web browser) to send us an HTTP request
     * we read this request using the recv() function
     * 
     * request[1024] buffer stores the browser's HTTP request
     * 
     * recv() @returns the number of bytes received 
     * 
     * if nothing received then recv() will block until
     * it has something. if connection terminated recv()
     * @returns 0 or -1, depending on circumstances
     * 
     * ignoring the check for simplicity but you should
     * always check recv() > 0
     */
    int bytes_received = recv(socket_client, request, 1024, 0);
    printf("Received %d bytes.\n", bytes_received);


    /**
     * print the browser's request to the console
     * "%.*s" tells printf() that we want to print specific
     * number of character - i.e. bytes_recieved
     * 
     * common mistake to print data received from recv()
     * directly as a C string as there's no guatantee
     * that data received from recv() is null terminated.
     * if you try to print it printf(request) or printf("%s", request)
     * you will likely receive segmentation error
     */
    printf("%.*s", bytes_received, request);


    /**
     * now that the web browser has sent its request
     * we can send our response back:
     */
    printf("Sending response...\n");
    const char *response = 
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: test/plain\r\n\r\n"
        "Local time is: ";


    /**
     * you should generally check the number of bytes sent
     * was expected and you should attempt to send the rest
     * if it's not. we are ignoring for simplicity
     * 
     * also we are sending few bytes if send() cant handle 
     * that then something is probably very broken and 
     * resending wont help
     */
    int bytes_sent = send(socket_client, response, strlen(response), 0);
    printf("Send %d of %d bytes.\n", bytes_sent, (int)strlen(response));   


    /**
     * after the HTTP header and the beginning of our message
     * is sent, we can send the actual time.
     */
    time_t timer;
    time(&timer);
    char *time_msg = ctime(&timer);
    bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
    printf("Send %d of %d bytes.\n", bytes_sent, (int)strlen(time_msg));


    /**
     * we must finally close the connection to indicate
     * the browser that we have sent all of our data.
     * if we dont close the connection, the browser will
     * just wait for more data until it times out
     * 
     * at this point we could call accept() on socket_listen
     * to accept additional connections. (i.e. exactly what
     * a real server would do) but for simplicity we ignore 
     * this
     */
    printf("Closing connection...\n");
    CLOSESOCKET(socket_client);

    #if defined(_WIN32)
        WSACleanup();
    #endif

    
    return 0;
}


