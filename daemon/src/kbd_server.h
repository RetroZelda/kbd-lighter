#ifndef KBD_SERVER
#define KBD_SERVER

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

typedef void(*server_callback)();
typedef struct kbd_server_callbacks
{
    server_callback OnClientConnect;
    server_callback OnClientDisconnect;
} ServerCallbacks;

void server_setup();
void server_shutdown();
bool server_tick();

void server_register_callbacks(ServerCallbacks* callbacks);
void server_unregister_callbacks(ServerCallbacks* callbacks);

ssize_t server_write(const void* in_data, ssize_t in_size);
ssize_t server_read(void* out_data, ssize_t data_size);

bool server_has_connection();

#endif