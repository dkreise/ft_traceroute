#include "../incs/ft_traceroute.h"

void init_traceroute_info(traceroute_info_t *info) {
    info->udp_socket = -1;
    info->icmp_socket = -1;
    info->ipv4 = NULL;
    info->res = NULL;
    info->first_ttl = FIRST_TTL_DEFAULT;
    info->max_ttl = MAX_HOPS_DEFAULT;
    info->probes_per_hop = PROBES_PER_HOP_DEFAULT;
    info->port = PORT_DEFAULT;
    info->names_conversion = NAMES_DEFAULT;
    info->errors = 0;
}

int main(int argc, char **argv) {
    int opt;
    traceroute_info_t info;
    struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    init_traceroute_info(&info);
    // parse_options(argc, argv, &info);
    while ((opt = getopt_long(argc, argv, "hm:q:f:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                printf(HELP_MESSAGE);
                return(0);
            case 'm':
                if (optarg) {
                    if (!is_num(optarg)) {
                        fprintf(stderr, "Cannot handle `-m' option with arg `%s' (argc %d)\n", optarg, optind - 1);
                        return(1);
                    }
                    info.max_ttl = atoi(optarg);
                    if (info.max_ttl < 1 || info.max_ttl > MAX_MAX_HOPS) {
                        fprintf(stderr, "max hops cannot be more than 255 and less than 1\n");
                        return(1);
                    }
                } else {
                    fprintf(stderr, "Option `-m' (argc %d) requires an argument: `-m max_ttl'\n", optind - 1);
                    return(1);
                }
                break;
            case 'q':
                if (optarg) {
                    if (!is_num(optarg)) {
                        fprintf(stderr, "Cannot handle `-q' option with arg `%s' (argc %d)\n", optarg, optind - 1);
                        return(1);
                    }
                    info.probes_per_hop = atoi(optarg);
                    if (info.probes_per_hop < 1 || info.probes_per_hop > 10) {
                        fprintf(stderr, "no more than 10 probes per hop\n");
                        return(1);
                    }
                } else {
                    fprintf(stderr, "Option `-q' (argc %d) requires an argument: `-q probes_per_hop'\n", optind - 1);
                    return(1);
                }
                break;
            case 'f':
                if (optarg) {
                    if (!is_num(optarg)) {
                        fprintf(stderr, "Cannot handle `-f' option with arg `%s' (argc %d)\n", optarg, optind - 1);
                        return(1);
                    }
                    info.first_ttl = atoi(optarg);
                    if (info.first_ttl < 1 || info.first_ttl > MAX_MAX_HOPS) {
                        fprintf(stderr, "first ttl cannot be more than 255 and less than 1\n");
                        return(1);
                    }
                } else {
                    fprintf(stderr, "Option `-f' (argc %d) requires an argument: `-f first_ttl'\n", optind - 1);
                    return(1);
                }
                break;
            default:
                printf(HELP_MESSAGE);
                return(1);
        }
    }

    // If no options were given, check for a destination argument
    if (optind < argc) {
        printf("Target: %s\n", argv[optind]);  // The destination IP/hostname
    } else {
        if (argc > 1)
            fprintf(stderr, "Specify \"host\" missing argument.\n");
        else
            printf(HELP_MESSAGE);
        return(1);
    }

    if (info.first_ttl > info.max_ttl) {
        fprintf(stderr, "first hop out of range\n");
        return(1);
    }

    const char *destination = argv[optind];
    int status;

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if ((status = getaddrinfo(destination, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "%s: %s\n", destination, gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);
    printf("IP address: %s\n", ip);

    // for sending
    info.udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (info.udp_socket < 0) {
        perror("socket (UDP)");
        exit(EXIT_FAILURE);
    }
    // for receiving
    info.icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (info.icmp_socket < 0) {
        perror("socket (ICMP)");
        exit(EXIT_FAILURE);
    }

    // struct sockaddr_in dest_addr;
    memset(&info.dest_addr, 0, sizeof(info.dest_addr));
    info.dest_addr.sin_family = AF_INET;
    info.dest_addr.sin_addr = ipv4->sin_addr;  // Copying resolved IP

    traceroute(&info);
    
    close(info.udp_socket);
    close(info.icmp_socket);
    freeaddrinfo(res);
    return(0);
}