#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_MESSAGE_RECEIVED_LENGTH 700

bool receive_http_response()
{
    char buffer[MAX_MESSAGE_RECEIVED_LENGTH];
    int remaining_bytes = MAX_MESSAGE_RECEIVED_LENGTH;
    int received_bytes = 0;

    /* loop until there is nothing received or we've ran out of buffer space */
    nsapi_size_or_error_t result = remaining_bytes;
    while (result > 0 && remaining_bytes > 0)
    {
        nsapi_size_or_error_t result = _socket.recv(buffer + received_bytes, remaining_bytes);
        if (result < 0)
        {
            printf("Error! _socket.recv() returned: %d\r\n", result);
            return false;
        }

        received_bytes += result;
        remaining_bytes -= result;
    }

    printf("received %d bytes:\r\n%.*s\r\n\r\n", received_bytes, strstr(buffer, "\n") - buffer, buffer);

    return true;
}