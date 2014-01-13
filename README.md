Unique ID Server
===============

### A uniqueid server 

Based on the Twitter Snowflake algorithm

Runs a TCP server. Connect to that server and it will send you an ID and immediately close the connection.

    Build: make
    Usage: uniqueidserver <port> <machineid>

The machine ID should be a number between 0 and 31. The number is used when generating IDs. This enables you to run multiple ID servers and ensures that IDs created on different machines will never clash.

PHP Client Example:

    <?php
    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    $con = socket_connect($socket, ID_SERVER_IP, ID_SERVER_PORT);

    if ($con) {
        $hear = socket_read($socket,1024);
        $hear = trim($hear);

        if (!empty($hear)) {
            echo $id;
        } else {
            echo 'Server Error';
        }

        socket_shutdown($socket);
        socket_close($socket);
    } else {
        echo 'Connect Error';
    }
    ?>

The TCP server based on https://github.com/3ft9/uniqidserver

The ID algorithm based on https://github.com/liexusong/ukey
