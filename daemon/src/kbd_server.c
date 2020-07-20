#include "kbd_server.h"
#include "kbd_common.h"
#include "array.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

static int s_server_handle = -1;
static int s_client_handle = -1;
static struct sockaddr_un s_server_addr;

static Array s_callback_array;

static void sigpipe_handler(int signum, siginfo_t *info, void *ptr);
static void register_sigpipe();
static void close_client();
static void close_server();

static void sigpipe_handler(int signum, siginfo_t *info, void *ptr)
{
    printf("sigpipe\n");
    if(server_has_connection())
    {
        close_client();

        // notify the callback listeners
        for(uint32_t callback_index = 0; callback_index < array_size(s_callback_array); ++callback_index)
        {
            ServerCallbacks* callbacks = array_get(s_callback_array, callback_index);
            if(callbacks->OnClientDisconnect != NULL)
            {
                callbacks->OnClientDisconnect();
            }
        }
    }
}

static void register_sigpipe()
{
    struct sigaction pipe_action;
    memset(&pipe_action, 0, sizeof(pipe_action));
    pipe_action.sa_sigaction = sigpipe_handler;
    pipe_action.sa_flags = SA_SIGINFO;

    sigaction(SIGPIPE, &pipe_action, NULL);
}

static void close_client()
{
    if(close(s_client_handle) == -1)
    {
        perror("close client error");
    }
    s_client_handle = -1;
}

static void close_server()
{
    if(close(s_server_handle) == -1)
    {
        perror("close server error");
    }
    s_server_handle = -1;
}

void server_setup()
{
    register_sigpipe();

    if ((s_server_handle = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(-1);
    }

    memset(&s_server_addr, 0, sizeof(s_server_addr));
    s_server_addr.sun_family = AF_UNIX;
    strncpy(s_server_addr.sun_path, SOCKET_ADDR, sizeof(s_server_addr.sun_path) - 1);

    unlink(SOCKET_ADDR);
    size_t addr_size = sizeof(s_server_addr.sun_family) + strlen(s_server_addr.sun_path);
    if (bind(s_server_handle, (struct sockaddr *)&s_server_addr, addr_size) == -1)
    {
        perror("bind error");
        exit(-1);
    }

    if (listen(s_server_handle, 1) == -1)
    {
        perror("listen error");
        exit(1);
    }

    s_callback_array = array_create(1, false);
}

void server_shutdown()
{
    array_destroy(s_callback_array);

    // TODO: do we need to flush our data?
    close_client();
    close_server();
}

void server_tick()
{
    if(s_client_handle == -1)
    {
        s_client_handle = accept(s_server_handle, NULL, NULL);
        if (s_client_handle == -1)
        {
            perror("accept error");
        }
        else
        {
            perror("accept");
            int flags = fcntl(s_client_handle, F_GETFL, 0);
            if(flags == -1)
            {
                perror("fcntl get error");
                close_client();
                return;
            }

            if(fcntl(s_client_handle, F_SETFL, flags | O_NONBLOCK) == -1)
            {
                perror("fcntl set error");
                close_client();
                return;
            }

            // notify the callback listeners
            for(uint32_t callback_index = 0; callback_index < array_size(s_callback_array); ++callback_index)
            {
                ServerCallbacks* callbacks = array_get(s_callback_array, callback_index);
                if(callbacks->OnClientConnect != NULL)
                {
                    callbacks->OnClientConnect();
                }
            }
        }
    }
}

void server_register_callbacks(ServerCallbacks* callbacks)
{
    array_push_back(s_callback_array, callbacks);
}

void server_unregister_callbacks(ServerCallbacks* callbacks)
{
    for(uint32_t callback_index = 0; callback_index < array_size(s_callback_array); ++callback_index)
    {
        if(array_get(s_callback_array, callback_index) == callbacks)
        {
            array_remove(s_callback_array, callback_index);
            break;
        }
    }
}

ssize_t server_write(const void* in_data, ssize_t in_size)
{
    if(server_has_connection())
    {
        // write our final state data out
        ssize_t write_size = write(s_client_handle, in_data, in_size);
        if (write_size != in_size)
        {
            if (write_size > 0)
            {
                printf("partial write");
            }
            else
            {
                switch(errno)
                {
                    case EWOULDBLOCK:
                    break;
                    default:
                        perror("write error");
                        close_client();
                        break;
                }
            }
            return write_size;
        }
    }
    return 0;
}

ssize_t server_read(void* out_data, ssize_t data_size)
{
    return 0;
}

bool server_has_connection()
{
    return s_client_handle != -1;;
}
