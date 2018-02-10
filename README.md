# Mail-Client-Server-Application
Email client / server application using socket programming.

## Files
emailserver.c
The Email Server responsible for responding to requests made by client.

emailclient.c
End user will run this Email Client to communicate with the server.

Use port number 8080.

To compile
```
$ make
```

To run
```
./emailserver 8080 &
./emailclient localhost 8080
```

Make requests such as Listusers, Adduser <userid>, SetUser <userid>, Read, Delete, Send <userid> to test.
Use Quit to exit.  