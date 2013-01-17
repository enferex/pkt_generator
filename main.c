/******************************************************************************
 * main.c
 * For the `pkt_generator' program.
 *
 * This program acts as either a sender or receiver sending or receiving UDP packets of
 * (specified length) with a text string representing a monotonically increasing
 * value (a counter).  
 *
 * This operates on port 6666 (not quite the beast).
 *
 * For example, if 10 packets are sent, strings "1" to "10" are the payload for
 * packets 1 to 10 respectively.  Strings instead of binary values makes parsing
 * with `tcpdump -X' pretty trivial.
 *
 * Receive or sending information is output to a file 'pkt_generator.log'
 *
 * mattdavis9@gmail.com
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX(_v1, _v2) ((_v1) > (_v2) ? (_v1) : (_v2))
#define LOG "pkt_generator.log"

#define ERR(_expr, _fail, _msg) \
do                              \
{                               \
    if ((_expr) == (_fail))     \
    {                           \
        perror("Error: " _msg); \
        exit(EXIT_FAILURE);     \
    }                           \
} while(0)

static void usage(const char *execname)
{
    printf("%s address <-i iface> [-b bytes] [-p packets] <-s | -c>\n"
           "  address:    IP address/hostname of the client to send data to\n"
		   "  -i interface: Interface name (e.g. eth0)\n"
           "  -b bytes:     Maximum payload size\n"
           "  -p packets:   Number of packets to send\n"
		   "  -s server:    Send packets\n"
		   "  -c client:    Recv packets (-b -p options are ignored)\n",
           execname);
    exit(EXIT_SUCCESS);
}

/* Send a series of UDP packets */
static void do_server(
	int 		sd,
	FILE       *fp,
	int 		n_packets,
	const char *target,
	char       *buf,
	size_t      buf_len)
{
	int i;
    ssize_t ret;
    struct addrinfo hints = {0}, *host;

	/* Prepare the target */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    if (getaddrinfo(target, "6666", &hints, &host) < 0)
    {
        perror("getaddrinfo()");
        exit(EXIT_FAILURE);
    }

	/* Send! */
	for (i=0; i<n_packets; ++i)
	{
		fprintf(fp, "[%d of %d] ", i+1, n_packets);
		snprintf(buf, buf_len, "%d", i+1);
		ERR((ret=sendto(sd, buf, buf_len, 0,
			            host->ai_addr, sizeof(*host->ai_addr))),
			-1, "sendto()");
		fprintf(fp, "Sent %ld of %ld bytes\n", ret, buf_len);
		fflush(NULL);
	}
}

/* Listen for packets */
static void do_client(int sd, FILE *fp, char *buf, size_t buf_len)
{
	int i;
	ssize_t ret;
	socklen_t sender_len;
	struct sockaddr_in myself, sender;

	memset(&myself, 0, sizeof(myself));
	myself.sin_family = AF_INET;
	myself.sin_port = htons(6666);
	myself.sin_addr.s_addr = htonl(INADDR_ANY);

	ERR(bind(sd, (const struct sockaddr *)&myself, sizeof(myself)),
		-1, "bind()");

	/* Recv! */
	i = 0;
	sender_len = sizeof(sender);
	while ((ret = recvfrom(sd, buf, buf_len, 0,
						  (struct sockaddr *)&sender, &sender_len)) >= 0)
	{
		fprintf(fp, "[%d] Recv %ld bytes with index %s\n",++i, ret, buf);
		fflush(NULL);
	}
}

/* Globals */
FILE *global_log;
int global_socket;

void handler(int signum)
{
	printf("Exiting...\n");
	fflush(NULL);
	fclose(global_log);
	close(global_socket);
}

int main(int argc, char **argv)
{
    int i, n_payload, n_packets, is_server, is_client;
	char *buf;
	size_t buf_len;
	struct ifreq ifr;
	const char *iface = NULL;
    const char *target = NULL; /* Where to send or recv data from */

    is_server = is_client = n_packets = n_payload = 0;

    for (i=1; i<argc; ++i)
    {
        if (strncmp("-b", argv[i], 2) == 0 && (i+1 < argc))
          n_payload = atoi(argv[++i]);
        else if (strncmp("-p", argv[i], 2) == 0 && (i+1 < argc))
          n_packets = atoi(argv[++i]);
        else if (strncmp("-i", argv[i], 2) == 0 && (i+1 < argc))
          iface = argv[++i];
        else if (argv[i][0] != '-')
          target = argv[i];
        else if (strncmp("-s", argv[i], 2) == 0)
		  is_server = 1;
		else if (strncmp("-c", argv[i], 2) == 0)
		  is_client = 1;
		else
          usage(argv[0]);
    }

	if ((!is_server && !is_client) || (is_server && is_client))
	{
		printf("Please choose server or client mode\n");
		usage(argv[0]);
	}

	if (!iface)
	{
		printf("Please specify an interface to use\n");
		usage(argv[0]);
	}
	
	printf("Using interface: %s\n", iface);

	if (is_server)
	{
		if (!target || n_payload < 0 || n_payload < 0)
		  usage(argv[0]);

		printf("Requested payload size:    %d bytes\n"
			   "Requested packets to send: %d\n"
			   "Target:                    %s\n",
			   n_payload, n_packets, target);
	}

	/* Install exit handler */
	signal(SIGKILL, handler);
	signal(SIGTERM, handler);

	/* Create socket */
    ERR(global_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP),
	    -1, "socket()");

	/* Bind socket to specific interface */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name)-1);
	ERR(setsockopt(global_socket, SOL_SOCKET, SO_BINDTODEVICE,
				   (void *)&ifr, sizeof(ifr)),
		-1, "setsockopt(): Binding to device");

	/* Buffer we send (size of the most character in the string representation
     * of INT_MAX.  This makes reading network data dumps easy.  Plus we are
     * planning on sending adequate amounts of data, so a buffer of 10 bytes
     * seems pretty small anyways.
     */
    buf_len = MAX(strlen("2147483647"), n_payload);
    buf = calloc(1, buf_len);

	/* Create a log file to store data into */
	ERR((global_log=fopen(LOG, "w")), NULL, "fopen()");

	if (is_server)
	  do_server(global_socket, global_log, n_packets, target, buf, buf_len);
	else
	  do_client(global_socket, global_log, buf, buf_len);

	fflush(NULL);
    free(buf);
    close(global_socket);
	fclose(global_log);

    return 0;
}
