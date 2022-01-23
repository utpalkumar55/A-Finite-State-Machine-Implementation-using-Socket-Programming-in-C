# Simple Finite State Machine Implementation using Socket Programming in C
This project implements C program to support a client/server model using Linux sockets following this finite state machine (FSM) [Finite State Machine.pdf](https://github.com/utpalkumar55/Simple-Finite-State-Machine-Implementation-using-Socket-Programming-in-C/files/7818561/Finite.State.Machine.pdf)
. The program consists of a server that implements a service using a simple text protocol that clients can use (through telnet) to see who is available on the service. A user can connect to the server using any separate machine by using the telnet service.

Server implementation guidelines-->
* The server establishes a socket connection over TCP using a port number passed in as an argument to the program.
* The server supports up to and including 20 “registered” client connections (registration is done with the JOIN command). Although more than 20 clients can connect to the server, no more than 20 clients may be registered in the database at the same time. In the case of more than 20 clients attempting to register, the server will print out an error message and simply close the connection.
* When a client connects to the server, a new thread is spawned to handle the socket using POSIX threads in Linux.
* The server supports the following case-sensitive commands which are all sent as plain ASCII text.
	* JOIN username
      * When a client wants to join the service, it first connects to the server using the telnet command with the hostname of the server and the port number, and then sends a JOIN request with the username. Usernames will only consist of alphanumeric characters and will not contain spaces or other “special” or control characters. The server stores this username as well as the client’s socket file descriptor in the “database”. A client may “connect” to the server but it must register for the service with the JOIN command before the client may receive any “services” provided by the server. If the database is “full”, then the server prints out a status message and sends appropriate message to the client. Once a client has already registered with a JOIN request, any subsequent JOIN requests from the same registered client is discarded with a status message sent back to the client.
	* PONG
      * After a client joins, the server initiates a PING request to the client and start a timer to record when the PING was sent. After the client receives the PING request, the client responds with a PONG response. Upon receipt of the PONG response, the server calculates the amount of time it took to receive the PONG and store this value as Round Trip Time (RTT) in the “database” associated with the user. If a client who is not registered for this service sends a PONG request, the server prints out a status message and sends appropriate message to the client with the JOIN request instructions. If a registered client sends an unsolicited PONG response, the server prints out a status message and resend the PING request to the client and start the timer to collect the RTT time.
	* LIST
      * If a registered client wants to know who is currently subscribed to the service, the client issues a LIST request. Upon receipt of the LIST request, the server sends a list of all registered clients on individual lines and return this list to the client. A client must be registered for the service to receive any “services”, such as this one, provided by the server. If a client who is not registered for this service sends a LIST request, the server prints out a status message and sends appropriate message to the client with the JOIN request instructions. If a registered client has not responded to the PING request (i.e., their RTT is 0), the server prints N/A in place of the zero time.
	* LEAVE
      * When a connected client wants to leave the service, the client issues a LEAVE request, at which time the server disconnects the client from the service. The database entry for registered clients is removed after the client has been disconnected. An unregistered client is disconnected from the service with a status message at the server anyway without modifying the database.
	* Unrecognizable messages
      * If the timer is on when a PING request has been sent for a registered client, but the PONG response has not been returned instead unrecognizable request returned, the server prints out a status message, resends the PING request, and start the timer again to collect the RTT time.
      * If the timer is not on for a registered client and unrecognizable request returned, the server prints out a status message, sends the PING request, and start the timer to collect the RTT time.
      * If a client who is not registered for this service sends an unrecognizable message, the server prints out a status message and sends appropriate message to the client with the JOIN request instructions.

Any client can issue the above commands after registering with the server.

Sample out put of the server and client of two scenarios are given below.
Server output of scenario 1

![scenario1server](https://user-images.githubusercontent.com/3108754/148307816-e526be08-4e77-45e7-bf70-19c2557b898c.JPG)

Client output of scenario 1

![scenario1client](https://user-images.githubusercontent.com/3108754/148307836-e646a953-e093-454c-b86b-c7bdc621eedd.JPG)

Server output of scenario 2

![scenario2server](https://user-images.githubusercontent.com/3108754/148307858-eb889401-df72-4200-952f-752edace93df.JPG)

Client1 output of scenario 2

![scenario2client1](https://user-images.githubusercontent.com/3108754/148307875-84a7de6b-e609-450a-bb91-8c8002a55ac7.JPG)

Client2 output of scenario 2

![scenario2client2](https://user-images.githubusercontent.com/3108754/148307897-6d072bfa-eb5f-4a1f-8e96-ebc87579a925.JPG)
