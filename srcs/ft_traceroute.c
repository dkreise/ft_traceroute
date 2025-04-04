#include "../incs/ft_traceroute.h"

void print_results(hop_entry_t* results, int result_count, int names_conversion) {
    for (int i = 0; i < result_count; i++) {
        if (names_conversion) {
            if (results[i].host[0] != '\0') {
                printf(" %s (%s)", results[i].host, results[i].ip);
            } else {
                printf(" %s (%s)", results[i].ip, results[i].ip);
            }
        } else {
            printf(" %s", results[i].ip);
        }
        for (int j = 0; j < results[i].count; j++) {
            if (results[i].rtt[j] == -1) {
                printf(" *");
            } else {
                printf("  %.3f ms", results[i].rtt[j]);
            }
        }
    }
    printf("\n");
}

void traceroute(traceroute_info_t* info) {
    int udp_socket = info->udp_socket;
    int icmp_socket = info->icmp_socket;
    struct sockaddr_in dest_addr = info->dest_addr;
    int probes_per_hop = info->probes_per_hop;
    int destination_reached = 0;

    for (int ttl = info->first_ttl; ttl <= info->max_ttl; ttl++) {
        printf("%2d ", ttl);
        // Set the TTL for this packet
        setsockopt(udp_socket, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
        hop_entry_t results[probes_per_hop];
        for (int i = 0; i < probes_per_hop; i++) {
            results[i].rtt = malloc(probes_per_hop * sizeof(double));
            if (!results[i].rtt) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            results[i].count = 0;
        }
        int result_count = 0;

        for (int probe = 0; probe < probes_per_hop; probe++) {
            int udp_port = info->port + ttl + probe;
            // int random_offset = rand() % 1000;  // Random port in the range 33434 - 34434
            // int udp_port = base_port + random_offset;
            // printf("Sending UDP packet (probe %d) to port %d\n", probe + 1, udp_port);

            // Set the destination port to the random value
            dest_addr.sin_port = htons(udp_port);

            // Set up timeout
            struct timeval timeout;
            timeout.tv_sec = TIMEOUT;
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

            if (select_result == 0) { // Timeout occurred, no response
                if (result_count == 0) {
                    printf(" *");
                } else {
                    results[result_count - 1].rtt[results[result_count - 1].count++] = -1;
                }
            } else if (select_result > 0) { // Response received
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
                    if (getnameinfo((struct sockaddr*)&sender_addr, sizeof(sender_addr), results[result_count].host, sizeof(results[result_count].host), NULL, 0, 0) != 0) {
                        results[result_count].host[0] = '\0';
                    } 
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

            } else { // Error in select()
                perror("select");
                exit(EXIT_FAILURE);
            }
        }
        print_results(results, result_count, info->names_conversion);
        if (destination_reached) {
            return;
        }
        for (int i = 0; i < info->probes_per_hop; i++) {
            free(results[i].rtt);
        }
    }
}