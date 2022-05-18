#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    struct ifaddrs *addresses;

    /*
       call to getifaddrs() function allocates memory
       and fill in a linked list of addresses (i.e. "struct ifaddrs")
       return 0 on success or -1 on failure */
    if (getifaddrs(&addresses) == -1) {
        printf("getifaddrs call failed\n");
        return -1;
    }

    /*
        we use new pointer "address" to walk through
        the linked list of "addresses"
        after getting each address, we set 
        "address = address->ifa_next" to get the next 
        address, we stop the loop when "address == 0" */
    struct ifaddrs *address = addresses;
    while(address) {
        int family = address->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6) {
            printf("%s\t", address->ifa_name);
            printf("%s\t", family == AF_INET ? "IPv4" : "IPv6");

            char ap[100];

            const int family_size = family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            getnameinfo(address->ifa_addr, family_size, ap, sizeof(ap), 0, 0, NI_NUMERICHOST);
            printf("\t%s\n", ap);
        }
        address = address->ifa_next;
    }

    freeifaddrs(addresses);
    return 0;


}