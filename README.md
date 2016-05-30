# mastermind-client-server
[![forthebadge](http://forthebadge.com/images/badges/uses-git.svg)](http://forthebadge.com) [![forthebadge](http://forthebadge.com/images/badges/designed-in-ms-paint.svg)](http://forthebadge.com)

This is a C implementation of a Mastermind game client and server. It was built as an exercise in sockets and pthreads for Computer Systems (COMP30023).

The server can handle multiple connections.


### Running the thing
Build executables:
```sh
$ make
```

Run the server on a given port. Ensure this port is open if you plan on accepting external clients.
You can provide a static secret code if you're boring, otherwise just give it the port.
The code is 4 chars of the set {A, B, C, D, E, F}, duplicates allowed.

```sh
$ ./server 1337 BEEF
```
The server should now be running.

Open up a client by providing the hostname and port of the sever.
```sh
$ ./client localhost 1337
```

Have fun!
