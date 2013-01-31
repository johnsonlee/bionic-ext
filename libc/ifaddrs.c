#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/if.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netdb.h>
#include <sys/cdefs.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

#ifndef NDEBUG
#  define DEBUG printf
#else
#  define DEBUG(...)
#endif

#ifndef IN6_IS_ADDR_LINKLOCAL
#define IN6_IS_ADDR_LINKLOCAL(a) \
	((((__const uint32_t *) (a))[0] & 0xffc00000) == 0xfe800000)
#endif

#ifndef HAVE_STRUCT_IFADDRS
struct ifaddrs {
	struct ifaddrs   *ifa_next;         /* Pointer to next struct */
	char             *ifa_name;         /* Interface name */
	unsigned int     ifa_flags;         /* Interface flags */
	struct sockaddr  *ifa_addr;         /* Interface address */
	struct sockaddr  *ifa_netmask;      /* Interface netmask */
#undef ifa_dstaddr
	struct sockaddr  *ifa_dstaddr;      /* P2P interface destination */
	void             *ifa_data;         /* Address specific data */
};
#endif /* HAVE_STRUCT_IFADDRS */

static int bionic_ifaddrs_parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= max)
			tb[rta->rta_type] = rta;

		rta = RTA_NEXT(rta,len);
	}

	return 0;
}

static void bionic_ifaddrs_recvaddrs(int fd, struct ifaddrs **ifa, __u32 seq)
{
	char buf[8192];
	struct sockaddr_nl nladdr;
	struct iovec iov = { buf, sizeof(buf) };
	struct ifaddrmsg *m;
	struct rtattr *rta_tb[IFA_MAX + 1];
	struct ifaddrs *I;

	while (1) {
		int status;
		struct nlmsghdr *h;
		struct msghdr msg = {
			(void*) &nladdr, sizeof(nladdr),
			&iov,	1,
			NULL,	0,
			0
		};

		status = recvmsg(fd, &msg, 0);

		if (status <= 0 || nladdr.nl_pid)
			continue;

		h = (struct nlmsghdr*) buf;

		while (NLMSG_OK(h, status)) {
			if (h->nlmsg_seq != seq)
				goto skip_it;

			switch (h->nlmsg_type) {
			case NLMSG_DONE:
				return;
			case NLMSG_ERROR:
				return;
			case RTM_NEWADDR:
				break;
			default:
				goto skip_it;
			}

			m = NLMSG_DATA(h);

#if HAVE_IPV6
			if (m->ifa_family != AF_INET &&
			    m->ifa_family != AF_INET6)
#else /* HAVE_IPV6 */
			if (m->ifa_family != AF_INET)
#endif /* !HAVE_IPV6 */
				goto skip_it;

			if (m->ifa_flags & IFA_F_TENTATIVE)
				goto skip_it;

			memset(rta_tb, 0, sizeof(rta_tb));
			bionic_ifaddrs_parse_rtattr(rta_tb, IFA_MAX, IFA_RTA(m), h->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifaddrmsg)));

			if (rta_tb[IFA_LOCAL] == NULL)
				rta_tb[IFA_LOCAL] = rta_tb[IFA_ADDRESS];

			if (rta_tb[IFA_LOCAL] == NULL)
				goto skip_it;

			if (!(I = calloc(1, sizeof(struct ifaddrs))))
				return;

			I->ifa_name = calloc(IFNAMSIZ, sizeof(char));
			I->ifa_flags = m->ifa_index;
			I->ifa_addr = calloc(1, sizeof(struct sockaddr_storage));
			I->ifa_addr->sa_family = m->ifa_family;
			I->ifa_dstaddr = calloc(1, sizeof(struct sockaddr_storage));
			I->ifa_netmask = calloc(1, sizeof(struct sockaddr_storage));

			switch (m->ifa_family) {
			case AF_INET: {
				struct sockaddr_in *sin = (struct sockaddr_in*) I->ifa_addr;
				memcpy(&sin->sin_addr, RTA_DATA(rta_tb[IFA_LOCAL]), sizeof(struct in_addr));
				break;
			}
			case AF_INET6: {
				struct sockaddr_in6 *sin = (struct sockaddr_in6*) I->ifa_addr;
				memcpy(&sin->sin6_addr, RTA_DATA(rta_tb[IFA_LOCAL]), sizeof(struct in6_addr));
				if (IN6_IS_ADDR_LINKLOCAL(&sin->sin6_addr))
					sin->sin6_scope_id = I->ifa_flags;
				break;
			}
			default:
				break;
			}

			I->ifa_next = *ifa;
			*ifa = I;

skip_it:
			h = NLMSG_NEXT(h, status);
		}

		if (msg.msg_flags & MSG_TRUNC)
			continue;
	}
}

/**
 * missed in `bioni/libc/include/ifaddrs.h'
 */
int getifaddrs(struct ifaddrs **ifa0)
{
	struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req;
	struct sockaddr_nl nladdr;
	static __u32 seq;
	struct ifaddrs *i;
	int fd;
	size_t sa_size;
	char host[NI_MAXHOST];

	if ((fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
		return -1;

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = RTM_GETADDR;
	req.nlh.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.nlh.nlmsg_seq = ++seq;
	req.g.rtgen_family = AF_UNSPEC;

	if (sendto(fd, (void*) &req, sizeof(req), 0, (struct sockaddr*) &nladdr, sizeof(nladdr)) < 0) {
		close(fd);
		return -1;
	}

	*ifa0 = NULL;
	bionic_ifaddrs_recvaddrs(fd, ifa0, seq);

	close(fd);
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	for (i = *ifa0; i; i = i->ifa_next) {
		struct ifreq ifr;
		ifr.ifr_ifindex = i->ifa_flags;

		switch (i->ifa_addr->sa_family) {
		case AF_INET:
			sa_size = sizeof(struct sockaddr_in);
			break;
		case AF_INET6:
			sa_size = sizeof(struct sockaddr_in6);
			break;
		default:
			sa_size = sizeof(struct sockaddr);
			break;
		}

		// get interface name
		ioctl(fd, SIOCGIFNAME, &ifr);
		memcpy(i->ifa_name, ifr.ifr_name, IFNAMSIZ);
		DEBUG("%s <%s>\n", i->ifa_name, i->ifa_addr->sa_family == AF_INET ? "IPv4" : "IPv6");

		// get interface flags
		ioctl(fd, SIOCGIFFLAGS, &ifr);
		i->ifa_flags = ifr.ifr_flags;

		// get destination / broad address
		if (i->ifa_flags & (IFF_BROADCAST | IFF_LOOPBACK)) {
			ioctl(fd, SIOCGIFBRDADDR, &ifr);
			memcpy(i->ifa_dstaddr, &ifr.ifr_broadaddr, sa_size);
		} else if (i->ifa_flags & IFF_POINTOPOINT) {
			ioctl(fd, SIOCGIFDSTADDR, &ifr);
			memcpy(i->ifa_dstaddr, &ifr.ifr_dstaddr, sa_size);
		}

		// get netmask
		ioctl(fd, SIOCGIFNETMASK, &ifr);
		memcpy(i->ifa_netmask, &ifr.ifr_netmask, sa_size);

		getnameinfo(i->ifa_addr, sa_size, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		DEBUG("\tinet addr: %s\n", host);

		getnameinfo(i->ifa_dstaddr, sa_size, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		DEBUG("\tBcast: %s\n", host);

		getnameinfo(i->ifa_netmask, sa_size, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		DEBUG("\tMask: %s\n", host);

#if 0
		DEBUG("\tinet addr: %s  Bcast: %s  Mask: %s\n",
				inet_ntoa(((struct sockaddr_in*) i->ifa_addr)->sin_addr),
				inet_ntoa(((struct sockaddr_in*) i->ifa_dstaddr)->sin_addr),
				inet_ntoa(((struct sockaddr_in*) i->ifa_netmask)->sin_addr));
#endif
	}

	close(fd);

	return 0;
}

/**
 * missed in `bioni/libc/include/ifaddrs.h'
 */
void freeifaddrs(struct ifaddrs *ifa0)
{
	struct ifaddrs *i;

	while (ifa0) {
		i = ifa0;
		ifa0 = i->ifa_next;

		if (i->ifa_name)
			free(i->ifa_name);

		if (i->ifa_addr)
			free(i->ifa_addr);

		if (i->ifa_dstaddr)
			free(i->ifa_dstaddr);

		if (i->ifa_netmask)
			free(i->ifa_netmask);

		free(i);
	}
}

