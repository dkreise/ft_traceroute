#include "../incs/ft_traceroute.h"

void traceroute(traceroute_info_t* info) {
    int udp_socket = info->udp_socket;
    int icmp_socket = info->icmp_socket;
    struct sockaddr_in dest_addr = info->dest_addr;
    int destination_reached = 0;

    for (int ttl = 1; ttl <= info->max_ttl; ttl++) {
        printf("%d ", ttl);
        // Set the TTL for this packet
        setsockopt(udp_socket, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
        hop_entry_t results[info->probes_per_hop];
        int result_count = 0;

        for (int probe = 0; probe < info->probes_per_hop; probe++) {
            int udp_port = info->port + ttl + probe;
            // int random_offset = rand() % 1000;  // Random port in the range 33434 - 34434
            // int udp_port = base_port + random_offset;
            // printf("Sending UDP packet (probe %d) to port %d\n", probe + 1, udp_port);

            // Set the destination port to the random value
            dest_addr.sin_port = htons(udp_port);

            // Set up timeout
            struct timeval timeout;
            timeout.tv_sec = 5;  // 5 seconds timeout
            timeout.tv_usec = 0;

            // (Wait for ICMP response and process it...)
            struct sockaddr_in sender_addr;
            socklen_t addr_len = sizeof(sender_addr);
            char buffer[512];

            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(icmp_socket, &readfds);

            struct timeval start_time, end_time;
            gettimeofday(&start_time, NULL);

            // Send the UDP packet
            sendto(udp_socket, NULL, 0, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

            // Wait for data with timeout
            int select_result = select(icmp_socket + 1, &readfds, NULL, NULL, &timeout);

            if (select_result == 0) {
                // Timeout occurred, no response
                if (result_count == 0) {
                    printf(" *");
                } else {
                    results[result_count - 1].rtt[results[result_count - 1].count++] = -1;
                }
            } else if (select_result > 0) {
                // Response received
                ssize_t bytes_received = recvfrom(icmp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &addr_len);
                if (bytes_received < 0) {
                    perror("recvfrom");
                    exit(EXIT_FAILURE);
                }

                gettimeofday(&end_time, NULL);
                // Calculate the difference in time
                double seconds = (double)(end_time.tv_sec - start_time.tv_sec);
                double microseconds = (double)(end_time.tv_usec - start_time.tv_usec);
                double milliseconds = (seconds * 1000) + (microseconds / 1000);

                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(sender_addr.sin_addr), ip, INET_ADDRSTRLEN);
                int new = 1;
                // Check if IP is already stored
                for (int i = 0; i < result_count; i++) {
                    if (strcmp(results[i].ip, ip) == 0) {
                        results[i].rtt[results[i].count++] = milliseconds;
                        new = 0;
                        break;
                    }
                }
                 // If IP is new, add to results
                if (new) {
                    strcpy(results[result_count].ip, ip);
                    results[result_count].rtt[0] = milliseconds;
                    results[result_count].count = 1;
                    result_count++;
                }
                
                struct ip *ip_header = (struct ip *)buffer;
                struct icmp *icmp_header = (struct icmp *)(buffer + (ip_header->ip_hl << 2));

                if (icmp_header->icmp_type == ICMP_TIME_EXCEEDED) {
                    destination_reached = 0;
                } else if (icmp_header->icmp_type == ICMP_DEST_UNREACH || icmp_header->icmp_type == ICMP_ECHOREPLY) {
                    destination_reached = 1;
                } else {
                    printf("Unknown ICMP type: %d\n", icmp_header->icmp_type);
                    return;
                }

            } else {
                // Error in select()
                perror("select");
                exit(EXIT_FAILURE);
            }
        }
        for (int i = 0; i < result_count; i++) {
            printf(" %s", results[i].ip);
            for (int j = 0; j < results[i].count; j++) {
                if (results[i].rtt[j] == -1) {
                    printf(" *");
                } else {
                    printf("  %.3f ms", results[i].rtt[j]);
                }
            }
        }
        printf("\n");
        if (destination_reached) {
            return;
        }
    }
}