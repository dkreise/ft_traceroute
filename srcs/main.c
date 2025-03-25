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

    // for sending
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("socket (UDP)");
        exit(EXIT_FAILURE);
    }
    // for receiving
    int icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (icmp_socket < 0) {
        perror("socket (ICMP)");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = ipv4->sin_addr;  // Copying resolved IP

    for (int ttl = 1; ttl <= MAX_HOPS; ttl++) {
        printf("TTL: %d\n", ttl);
        // Set the TTL for this packet
        setsockopt(udp_socket, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        int udp_port = BASE_PORT + ttl;
        // int random_offset = rand() % 1000;  // Random port in the range 33434 - 34434
        // int udp_port = base_port + random_offset;
        printf("Sending UDP packet to port %d\n", udp_port);

        // Set the destination port to the random value
        dest_addr.sin_port = htons(udp_port);

        // Send the UDP packet
        sendto(udp_socket, NULL, 0, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        printf("Sent UDP packet\n");

        // (Wait for ICMP response and process it...)
        struct sockaddr_in sender_addr;
        socklen_t addr_len = sizeof(sender_addr);
        char buffer[512];

        // Set up timeout
        struct timeval timeout;
        timeout.tv_sec = 2;  // 2 seconds timeout
        timeout.tv_usec = 0;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(icmp_socket, &readfds);

        // Wait for data with timeout
        int select_result = select(icmp_socket + 1, &readfds, NULL, NULL, &timeout);

        if (select_result == 0) {
            // Timeout occurred, no response
            printf("No response for hop %d\n", ttl);
        } else if (select_result > 0) {
            // Response received
            ssize_t bytes_received = recvfrom(icmp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &addr_len);
            if (bytes_received < 0) {
                perror("recvfrom");
                exit(EXIT_FAILURE);
            }

            struct ip *ip_header = (struct ip *)buffer;
            struct icmp *icmp_header = (struct icmp *)(buffer + (ip_header->ip_hl << 2));

            // Check if it's an ICMP TTL expired or Destination Unreachable message
            if (icmp_header->icmp_type == ICMP_TIME_EXCEEDED) {
                printf("TTL expired from %s\n", inet_ntoa(sender_addr.sin_addr));
            } else if (icmp_header->icmp_type == ICMP_DEST_UNREACH) {
                printf("Destination unreachable from %s\n", inet_ntoa(sender_addr.sin_addr));
                break;  // If the destination is unreachable, we can exit
            } else if (icmp_header->icmp_type == ICMP_ECHOREPLY) {
                // We got the final response (destination reached)
                printf("Reached destination at %s\n", inet_ntoa(sender_addr.sin_addr));
                break;  // Exit the loop when the destination is reached
            } else {
                printf("Unknown ICMP type: %d\n", icmp_header->icmp_type);
                break;
            }

            // If we didn't get the final destination, print the hop info
            printf("Hop %d: %s\n", ttl, inet_ntoa(sender_addr.sin_addr));
        } else {
            // Error in select()
            perror("select");
            exit(EXIT_FAILURE);
        }
    }
    
    close(udp_socket);
    close(icmp_socket);
    freeaddrinfo(res);
    return(0);
}