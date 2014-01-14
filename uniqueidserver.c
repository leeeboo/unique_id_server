/**
 * Based on the Twitter Snowflake algorithm
 *
 * Unique ID server. Runs a TCP server. Connect to that server and it will send
 * you an ID and immediately close the connection.
 *
 * Usage: uniqueidserver <port> <machineid>
 *
 * The machine ID should be a number between 0 and 31. The number is used when
 * generating IDs. This enables you to run multiple ID servers and ensures that
 * IDs created on different machines will never clash.
 * 
 * Source: https://github.com/leeeboo/unique_id_server
 */
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef unsigned long long ukey_uint64;

typedef struct {
    int worker_id;
    int datacenter_id;

    long sequence;
    ukey_uint64 last_timestamp;

    /* Various once initialized variables */
    ukey_uint64 twepoch;
    unsigned char worker_id_bits;
    unsigned char datacenter_id_bits;
    unsigned char sequence_bits;
    int worker_id_shift;
    int datacenter_id_shift;
    int timestamp_left_shift;
    int sequence_mask;
} ukey_context_t;


/* True global resources - no need for thread safety here */
static ukey_uint64 twepoch;
static ukey_context_t *context;

int ukey_startup(ukey_uint64 twepoch, int worker_id, int datacenter_id)
{
    context = malloc(sizeof(ukey_context_t));
    if (!context) {
        return -1;
    }

    context->twepoch = twepoch;
    context->worker_id = worker_id;
    context->datacenter_id = datacenter_id;

    context->sequence = 0;
    context->last_timestamp = 0ULL;

    context->worker_id_bits = 5;
    context->datacenter_id_bits = 5;
    context->sequence_bits = 12;

    context->worker_id_shift = context->sequence_bits;
    context->datacenter_id_shift = context->sequence_bits +
        context->worker_id_bits;
    context->timestamp_left_shift = context->sequence_bits +
        context->worker_id_bits + context->datacenter_id_bits;
    context->sequence_mask = -1 ^ (-1 << context->sequence_bits);

    return 0;
}

static ukey_uint64 really_time()
{
    struct timeval tv;
    ukey_uint64 retval;

    if (gettimeofday(&tv, NULL) == -1) {
        return 0ULL;
    }

    retval = (ukey_uint64)tv.tv_sec * 1000ULL + 
        (ukey_uint64)tv.tv_usec / 1000ULL;

    return retval;
}


static ukey_uint64 skip_next_millis()
{
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 1000; /* one millisecond */

    select(0, NULL, NULL, NULL, &tv); /* wait here */

    return really_time();
}

static void skeleton_daemon()
{
    int pid;
    if(pid = fork())
        exit(0);
    else if(pid < 0)
        exit(1);
    setsid();

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
    {
        close (x);
    }
}

int main(int argc, char ** argv)
{

    int listenfd, connfd, port, machine_id, datacenter_id;
    struct sockaddr_in servaddr, cliaddr;
    struct timeval tv;
    socklen_t clilen;
    char id_buf[14];

    ukey_uint64 timestamp;
    ukey_uint64 retval;

    if (argc != 4) {
        printf("Usage: uniqidserver <port> <datacenterid> <machineid>\n");
        return 1;
    }

    port = atoi(argv[1]);

    datacenter_id = atoi(argv[2]);

    if (datacenter_id < 0 || datacenter_id > 31) {
        fprintf(stderr, "WARN: Not using a datacenter ID\n");
        datacenter_id = 1;
    }

    machine_id = atoi(argv[3]);

    if (machine_id < 0 || machine_id > 31) {
        fprintf(stderr, "WARN: Not using a machine ID\n");
        machine_id = 1;
    }

    if (ukey_startup(1288834974657LL, machine_id, datacenter_id) == -1) {
        printf("startup error!\n");
        return 1;
    }

    char str[18];

    printf("%d\n", datacenter_id);
    printf("%d\n", machine_id);

    skeleton_daemon();

    while (1) {

        listenfd = socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port);
        bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        listen(listenfd, 1024);
        clilen = sizeof(cliaddr);

        while (1) {
            connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
            if (connfd) {
                if (gettimeofday(&tv, NULL) == -1) {
                    sendto(connfd, "ERR", 3, 0, (struct sockaddr *)&cliaddr, clilen);
                } else {
                    // With machine ID.
                    timestamp = really_time();

                    if (context->last_timestamp == timestamp) {
                        context->sequence = (context->sequence + 1) & context->sequence_mask;

                        if (context->sequence == 0) {
                            timestamp = skip_next_millis();
                        }

                    } else {
                        context->sequence = 0; /* Back to zero */
                    }

                    context->last_timestamp = timestamp;

                    retval = ((timestamp - context->twepoch) << context->timestamp_left_shift) |
                        (context->datacenter_id << context->datacenter_id_shift) |
                        (context->worker_id << context->worker_id_shift) |
                        context->sequence;

                    sprintf(str, "%lld", retval);

                    sendto(connfd, str, 18, 0, (struct sockaddr *)&cliaddr, clilen);
                }
                close(connfd);
            }
        }

        close(listenfd);
    }

    return 0;
}
