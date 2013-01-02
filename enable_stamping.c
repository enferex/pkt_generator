#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/net_tstamp.h>
#include <linux/sockios.h>
#include <sys/socket.h>

static void set_settings(int sd)
{
	struct ifreq dev = {0};
	struct hwtstamp_config cfg = {0};

	cfg.rx_filter = HWTSTAMP_FILTER_ALL;
	cfg.tx_type = HWTSTAMP_TX_OFF;

	strncpy(dev.ifr_name, "eth2", sizeof(dev.ifr_name));
	dev.ifr_data = (void *)&cfg;
	
	if (ioctl(sd, SIOCSHWTSTAMP, &dev) == -1)
	{
		perror("ioctl()");
		exit(EXIT_FAILURE);
	}

	printf("Settings:\n"
		   "    HWTSTAMP_TX: %d\n"
		   "    HWTSTAMP_RX: %d\n"
		   "    Flags:       %d\n",
		   cfg.tx_type,
		   cfg.rx_filter,
		   cfg.flags);
}

int main(void)
{
	int sd;
	
	if ((sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
	    perror("socket()");
		exit(EXIT_FAILURE);
	}

	set_settings(sd);
	close(sd);

	return 0;
}
