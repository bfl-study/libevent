#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>
#include <event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
#include <event2/util.h>
#include <signal.h>

#define MYHTTPD_SIGNATURE "myhttpd v 0.0.1"

void httpd_handler(struct evhttp_request *req, void *arg)
{
    char output[2048] = "\0";
    char tmp[1024];

    const char *uri;
    uri = evhttp_request_uri(req);
    sprintf(tmp, "uri=%s\n", uri);
    strcat(output, tmp);

    sprintf(tmp, "uri=%s\n", req->uri);
    strcat(output, tmp);

    char *decoded_uri;
    decoded_uri = evhttp_decode_uri(uri);
    sprintf(tmp, "decoded_uri=%s\n", decoded_uri);
    strcat(output, tmp);

    struct evkeyvalq params;
    evhttp_parse_query(decoded_uri, &params);

    sprintf(tmp, "q=%s\n", evhttp_find_header(&params, "q"));
    strcat(output, tmp);
    sprintf(tmp, "s=%s\n", evhttp_find_header(&params, "s"));
    strcat(output, tmp);

    free(decoded_uri);

    char * post_data = (char *)EVBUFFER_DATA(req->input_buffer);
    sprintf(tmp, "post_data=%s\n", post_data);
    strcat(output, tmp);

    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "It works!\n%s\n", output);
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);

}

void show_help()
{
    char * help = "http://localhost:8080\n"
        "-l <ip_addr>   interface to listen on, default is 0.0.0.0\n"
        "-p <num>       port number to listen on, default is 1984\n"
        "-d             run as a deamon\n"
        "-t <second>    timeout for http request, default is 120s\n"
        "-h             print this help and exit\n"
        "\n";
    fprintf(stderr, "%s", help);
}

void signal_handler(int sig)
{
    switch(sig)
    {
    case SIGTERM:
    case SIGHUP:
    case SIGQUIT:
    case SIGINT:
        event_loopbreak();
        break;
    }
}

int main(int argc, char * argv[])
{
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    char* httpd_option_listen = "0.0.0.0";
    int httpd_option_port = 8080;
    int httpd_option_daemon = 0;
    int httpd_option_timeout = 120;

    int c;
    while((c = getopt(argc, argv, "l:p:dt:h")) != -1)
    {
        switch(c)
        {
        case 'l':
            httpd_option_listen = optarg;
            break;
        case 'p':
            httpd_option_daemon = atoi(optarg);
        case 'd':
            httpd_option_daemon = 1;
        case 't':
            httpd_option_timeout = atoi(optarg);
        case 'h':
        default:
            show_help();
            exit(EXIT_SUCCESS);
            
        }
    }

    if(httpd_option_daemon)
    {
        pid_t pid;
        pid = fork();
        if(pid < 0)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if(pid > 0)
        {
            exit(EXIT_SUCCESS);
        }
    }

    event_init();
    struct evhttp * httpd;
    httpd = evhttp_start(httpd_option_listen, httpd_option_port);
    evhttp_set_timeout(httpd, httpd_option_timeout);
    evhttp_set_gencb(httpd, httpd_handler, NULL);

    event_dispatch();

    evhttp_free(httpd);

    return 0;
}

