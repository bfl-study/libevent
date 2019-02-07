#include <stdio.h>

#include <event2/event-config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <event.h>

static void fifo_read(evutil_socket_t fd, short event, void * arg)
{
    char buf[255];
    int len;
    struct event *ev = arg;
    event_add(ev, NULL);
    fprintf(stderr, "fifo_read called with fd: %d, event: %d, arg: %p\n",
            (int)fd, event, arg);
    len = read(fd, buf, sizeof(buf) - 1);
    if(len == -1)
    {
        perror("read");
        return;
    }
    else if(len == 0)
    {
        fprintf(stderr, "Connection closed!\n");
        return;
    }

    buf[len] = '\0';
    fprintf(stdout, "Read: %s\n", buf);
}


int main()
{
    struct event evfifo;
    struct stat st;
    const char *fifo = "event.fifo";
    int socket;

    if(lstat(fifo, &st) == 0)
    {
        if((st.st_mode & S_IFMT) == S_IFREG)
        {
            errno = EEXIST;
            perror("lstat");
            exit(1);
        }
    }
    unlink(fifo);
    if(mkfifo(fifo, 06006) == -1)
    {
        perror("mkfifo");
        exit(1);
    }

    socket = open(fifo, O_RDONLY | O_NONBLOCK, 0);
    if(socket == -1)
    {
        perror("open");
        exit(1);
    }
    fprintf(stderr, "write data to %s\n", fifo);

    event_init();
    event_set(&evfifo, socket, EV_READ, fifo_read, &evfifo);
    event_add(&evfifo, NULL);
    event_dispatch();

    return 0;
}

