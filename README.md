Unique ID Server
===============

### A uniqueid server 

Based on the Twitter Snowflake algorithm

Runs a TCP server. Connect to that server and it will send you an ID and immediately close the connection.

    Build: make
    Usage: uniqueidserver <port> <machineid>

The machine ID should be a number between 0 and 31. The number is used when generating IDs. This enables you to run multiple ID servers and ensures that IDs created on different machines will never clash.

### The TCP server based on https://github.com/3ft9/uniqidserver
### The ID algorithm based on https://github.com/liexusong/ukey
