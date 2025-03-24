#include "../incs/ft_traceroute.h"

int main(int argc, char **argv) {
    int opt;
    struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}  // End of options
    };

    while ((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                printf(HELP_MESSAGE);
                return(0);
            default:  // Unknown option
                // fprintf(stderr, "Unknown option. Use --help for usage info.\n");
                printf(HELP_MESSAGE);
                return(1);
        }
    }

    // If no options were given, check for a destination argument
    if (optind < argc) {
        printf("Target: %s\n", argv[optind]);  // The destination IP/hostname
    } else {
        fprintf(stderr, "Error: No destination provided.\n");
        return 1;
    }

    const char *destination = argv[optind];
    int status;

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    // hints.ai_socktype = SOCK_RAW;
    // hints.ai_protocol = IPPROTO_ICMP;

    if ((status = getaddrinfo(destination, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);
    printf("IP address: %s\n", ip);

    freeaddrinfo(res);
    return(0);
}