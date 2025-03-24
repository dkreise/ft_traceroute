#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
// #include <bits/getopt_core.h> // not needed
#include <pthread.h>
#include <limits.h>
#include <signal.h>
#include <math.h>
#include <getopt.h>

// #define V_FLAG (1 << 0) // 0001
// #define F_FLAG (1 << 1) // 0010
// #define N_FLAG (1 << 2) // 0100
// #define W_FLAG (1 << 3) // 1000
// #define L_FLAG (1 << 4) // 10000
// #define T_FLAG (1 << 5) // 100000

#define HELP_MESSAGE "Usage:\n\
  ft_traceroute [options] host [ packetlen ]\n\
Options:\n\
  --help\t\tRead this help and exit\n\
Arguments:\n\
+     host\t\tThe host to traceroute to\n\
      packetlen\t\tThe full packet length (default is the length of an IP\n\
\t\t\theader plus 40). Can be ignored or increased to a minimal\n\
\t\t\tallowed value\n"
  
#endif

  // -f first_ttl  --first=first_ttl
  //                             Start from the first_ttl hop (instead from 1)
  // -m max_ttl  --max-hops=max_ttl
  //                             Set the max number of hops (max TTL to be
  //                             reached). Default is 30
  // -N squeries  --sim-queries=squeries
  //                             Set the number of probes to be tried
  //                             simultaneously (default is 16)
  // -n                          Do not resolve IP addresses to their domain names
  // -p port  --port=port        Set the destination port to use. It is either
  //                             initial udp port value for "default" method
  //                             (incremented by each probe, default is 33434), or
  //                             initial seq for "icmp" (incremented as well,
  //                             default from 1), or some constant destination
  //                             port for other methods (with default of 80 for
  //                             "tcp", 53 for "udp", etc.)
  // -w MAX,HERE,NEAR  --wait=MAX,HERE,NEAR
  //                             Wait for a probe no more than HERE (default 3)
  //                             times longer than a response from the same hop,
  //                             or no more than NEAR (default 10) times than some
  //                             next hop, or MAX (default 5.0) seconds (float
  //                             point values allowed too)
  // -q nqueries  --queries=nqueries
  //                             Set the number of probes per each hop. Default is
  //                             3
  // -z sendwait  --sendwait=sendwait
  //                             Minimal time interval between probes (default 0).
  //                             If the value is more than 10, then it specifies a
  //                             number in milliseconds, else it is a number of
  //                             seconds (float point values allowed too)
  // -V  --version               Print version info and exit
  // --help                      Read this help and exit

