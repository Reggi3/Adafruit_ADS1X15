#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/time.h>


#define CHECK_RET(msg, required, cmd) {       \
    int ret = cmd;              \
    if (ret != required) {            \
        fprintf(stderr, msg);   \
        return -1;              \
    }                           \
}

#define CHECK(msg, cmd) {       \
    int ret = cmd;              \
    if (ret == -1) {            \
        fprintf(stderr, msg);   \
        return -1;              \
    }                           \
}

long long microsecs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec + tv.tv_sec * 1000000LL;
}

const int NUM_PACKETS = 500;

int main(int argc, char **argv)
{
    long long size_of_packet = 0;
    long long total;
    int i;
    int pfd;
    struct pollfd pollfds[2];
    int n;
    char buf[128];
    struct sockaddr addr;
    struct stat the_stat;
    int flags;
    int count = 0;
    long long start = -1;
    int btsock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, 0);

    if (btsock == -1) {
        fprintf(stderr, "failed to open bluetooth socket\n");
        return -1;
    }
    addr.sa_family=AF_BLUETOOTH;
    memcpy(&addr.sa_data, "\0\0\0\0\0\0\0\0\4\0\1\0\0\0", 14);
    CHECK("bind failed", bind(btsock, &addr, 14));
    CHECK("setsockopt failed", setsockopt(btsock, 0x112 /* SOL_?? */, 4, "\1\0", 2));
    CHECK_RET("fstat failed", 0, fstat(btsock, &the_stat));
    flags = fcntl(btsock, F_GETFL);
    CHECK_RET("failed fcntl rdonly nonblocking", 0, fcntl(btsock, F_SETFL, O_RDONLY)); /* O_NONBLOCK - used by gatttool, but we can have a simpler life */
    memcpy(&addr.sa_data, "\0\0\344M\220x6\345\4\0\2\0\0\0", 14);
    connect(btsock, &addr, 14);
    pfd = eventfd(0, O_NONBLOCK|O_CLOEXEC); /* strace said eventfd2, but no
                                               such symbol when linking with default libraries */
    CHECK_RET("failed write to poll fd", 8, write(pfd, "\1\0\0\0\0\0\0\0", 8));
    CHECK_RET("failed second write to poll fd", 8, write(pfd, "\1\0\0\0\0\0\0\0", 8));
    while (1) {
        pollfds[0].fd = pfd;
        pollfds[0].events = POLLIN;
        pollfds[1].fd =  btsock;
        pollfds[1].events = POLLOUT;
        n = poll(pollfds, 2, 4294967295);
        if (pollfds[0].revents != 0) {
            read(pfd, buf, 8);
        }
        if (pollfds[1].revents != 0) {
            if (start == -1) {
                start = microsecs();
            }
            n = read(btsock, buf, sizeof(buf));
            if (size_of_packet == 0) {
                size_of_packet = n - 3;
            }
            for (i = 0 ; i < n ; ++i) {
                printf("%02X ", buf[i]);
            }
            printf("\n");
            count++;
            if (count == NUM_PACKETS) {
                total = microsecs() - start;
                printf("total usecs: %lld\n", total);
                printf("size of packet: %d\n", size_of_packet);
                printf("bits per second: %f\n", (double)(NUM_PACKETS * size_of_packet * 8 * 1000000) / total);
                _exit(0);
            }
        }
    }
#if 0
    getsockopt(3, SOL_SOCKET, SO_ERROR, [0], [4]) = 0
    getsockopt(3, SOL_SOCKET, 0x27 /* SO_??? */, [31], [4]) = 0
    getsockopt(3, SOL_SOCKET, 0x26 /* SO_??? */, [0], [4]) = 0
    getsockname(3, {sa_family=AF_BLUETOOTH, sa_data="\0\0\336\356\327\334\204\f\4\0\0\0\0\0"}, [14]) = 0
    getpeername(3, {sa_family=AF_BLUETOOTH, sa_data="\0\0\344M\220x6\345\4\0\0\0\0\0"}, [14]) = 0
    getsockopt(3, SOL_TCP, TCP_NODELAY, "\240\2\240\2\377\377\0\1\3\0?\0", [12]) = 0
#endif
    return 0;
}
