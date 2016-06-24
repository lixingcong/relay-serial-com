
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc, char **argv)
{
    struct sockaddr_rc blue_loc_addr = { 0 }, blue_rem_addr = { 0 };
    char blue_buffer[1024] = { 0 };
    int blue_fd, blue_fd_client, blue_bytes_read;
    socklen_t blue_opt = sizeof(blue_rem_addr);

    // allocate socket
    blue_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // bind socket to port 1 of the first available 
    // local bluetooth adapter
    blue_loc_addr.rc_family = AF_BLUETOOTH;
    blue_loc_addr.rc_bdaddr = *BDADDR_ANY;
    blue_loc_addr.rc_channel = (uint8_t) 1;
    bind(blue_fd, (struct sockaddr *)&blue_loc_addr, sizeof(blue_loc_addr));

    // put socket into listening mode
    listen(blue_fd, 1);

    // accept one connection
    blue_fd_client = accept(blue_fd, (struct sockaddr *)&blue_rem_addr, &blue_opt);

    ba2str( &blue_rem_addr.rc_bdaddr, blue_buffer );
    fprintf(stderr, "accepted connection from %s\n", blue_buffer);
    memset(blue_buffer, 0, sizeof(blue_buffer));

    // read data from the client
    blue_bytes_read = read(blue_fd_client, blue_buffer, sizeof(blue_buffer));
    if( blue_bytes_read > 0 ) {
        printf("received [%s]\n", blue_buffer);
    }

    // close connection
    close(blue_fd_client);
    close(blue_fd);
    return 0;
}
