#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

/** Structure for storing information about registered client.
Four informations of each registered client are to be stored in the database,
which are user name, file descriptor, round trip time and ping status.
Ping status is being stored to keep a track whether ping has been sent for a particular client or not. **/
struct client_registration{ ///Client registration database declaration.
    char username[100];
    int fd;
    double rtt;
    int ping_sent;
} registered_client[1000];

int registered_client_id[1000]; ///Array to store descriptor of registered clients

int connected_client_id[1000]; ///Array to store descriptor of connected clients

int registered_client_counter = 0, connected_client_counter = 0; ///Declaring separate counter for registered clients and connected clients.
struct timeval start_t, end_t; ///Variables to store time when the ping is sent and when the pong is received.


/** connection_handler() function runs when each thread is created.
This function runs until all the assigned tasks are done for that particular thread.
Receiving commands and appropriate actions against these commands are to be executed in this function. **/
void * connection_handler(void * client_sock) ///Thread handler function.
{
    int client_socket = *(int *) client_sock;///Getting client socket descriptor from main function.
    int message_length; ///Declaring variable for storing sending or receiving message length.
    char received_message[256], sending_message[256]; ///Declaring character array to store received message from client to server and sending message from server to client respectively.
    char command[100], username[100]; ///Declaring character array to store JOIN command and associated USERNAME.
    strcpy(command,"\0"); ///Initializing command to null.
    strcpy(username,"\0"); ///Initializing username to null.

    printf("Client (%d): Handler Assigned\n\n\n", client_socket);

/** In this loop continuous messages are being transferred either from server to client or client to server.
Server is taking action based on different commands from client. **/
    while(1)
    {
        bzero(received_message,256); ///Initializing zero to received message.
        message_length = read(client_socket,received_message,255); ///Receiving message from client.
        received_message[strlen(received_message)-2] = '\0'; ///Discarding extra control character coming from client at the end of received message.


        if(strlen(received_message) == 0)
        {}
        else if(strcmp(received_message,"LIST") == 0) ///Comparing if the received command is LIST or not.
        {
            if(registered_client[client_socket].fd == 0) ///Checking if the user is existing user or not.
            {
                printf("Unable Locate Client (%d) in Database. Discarding LIST.\n\n\n", client_socket);
                bzero(sending_message,256); ///Initializing zero to sending message.
                strcpy(sending_message,"Unregistered User. Use \"JOIN <username>\" to Register.\n"); ///Copying error message to sending message.
                message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                if (message_length < 0) ///Checking whether the message has been sent ot not.
                {
                    perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                }
                else
                {
                    printf("Message delivered to client successfully.\n\n\n");
                }
            }
            else
            {
                printf("Client (%d): LIST\n", client_socket);
                int i;
                bzero(sending_message,256); ///Initializing zero to sending message.
                sprintf(sending_message, "USERNAME     FD             RTT\n");
                write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                bzero(sending_message,256); ///Initializing zero to sending message.
                sprintf(sending_message, "-----------------------------------------------\n");
                write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                for(i = 1; i <= registered_client_counter; i++) ///Sending the full database to client.
                {
                    bzero(sending_message,256); ///Initializing zero to sending message.
                    if(registered_client[registered_client_id[i]].rtt == 0)
                    {
                        sprintf(sending_message, "%s %7d             N/A ms\n", registered_client[registered_client_id[i]].username, registered_client[registered_client_id[i]].fd);
                    }
                    else
                    {
                        sprintf(sending_message, "%s %7d %20.6lf ms\n", registered_client[registered_client_id[i]].username, registered_client[registered_client_id[i]].fd, registered_client[registered_client_id[i]].rtt);
                    }
                    message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                }
                if (message_length < 0) ///Checking whether the database has been sent ot not.
                {
                    perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                }
                else
                {
                    printf("Message delivered to client successfully.\n\n\n");
                }
            }
        }
        else if(strcmp(received_message,"PING") == 0) ///Comparing if the received command is PING or not.
        {
            if(registered_client[client_socket].fd == 0) ///Checking if the user is existing user or not.
            {
                printf("Unable Locate Client (%d) in Database. Discarding PING.\n\n\n", client_socket);
                bzero(sending_message,256); ///Initializing zero to sending message.
                strcpy(sending_message,"Unregistered User. Use \"JOIN <username>\" to Register.\n"); ///Copying error message to sending message.
                message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                if (message_length < 0) ///Checking whether the message has been sent ot not.
                {
                    perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                }
                else
                {
                    printf("Message delivered to client successfully.\n\n\n");
                }
            }
            else
            {
                printf("Client (%d): Unrecognizable Message. Sending PING.\n", client_socket);
                bzero(sending_message,256); ///Initializing zero to sending message.
                strcpy(sending_message,"PING\n"); ///Copying error message to sending message.
                message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                if (message_length < 0) ///Checking whether the message has been sent ot not.
                {
                    perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                }
                else
                {
                    printf("Message delivered to client successfully.\n\n\n");
                    gettimeofday(&start_t, NULL);
                }
                registered_client[client_socket].ping_sent = 1; ///The flag is set to 1 since, the PING message has already been sent.
            }
        }
        else if(strcmp(received_message,"PONG") == 0) ///Comparing if the received command is PONG or not.
        {
            if(registered_client[client_socket].fd == 0) ///Checking if the user is existing user or not.
            {
                printf("Unable Locate Client (%d) in Database. Discarding PONG.\n\n\n", client_socket);
                bzero(sending_message,256); ///Initializing zero to sending message.
                strcpy(sending_message,"Unregistered User. Use \"JOIN <username>\" to Register.\n"); ///Copying error message to sending message.
                message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                if (message_length < 0) ///Checking whether the message has been sent ot not.
                {
                    perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                }
                else
                {
                    printf("Message delivered to client successfully.\n\n\n");
                }
            }
            else
            {
                if(registered_client[client_socket].ping_sent) ///Checking if the PING message has already been sent or not.
                {
                    printf("Client (%d): PONG\n", client_socket);
                    gettimeofday(&end_t, NULL); ///Getting time when the PONG is received.
                    double sec = (double)(end_t.tv_sec - start_t.tv_sec); ///Calculating RTT in second.
                    double microsec = (double)(end_t.tv_usec - start_t.tv_usec); ///Calculating RTT in microsecond.
                    double total_time = (sec * 1000000 + microsec) / 1000; ///Converting RTT from microsecond to millisecond.
                    registered_client[client_socket].rtt = total_time; ///Assigning RTT into database for particular user.
                    registered_client[client_socket].ping_sent = 0; ///The flag is set to zero again since RTT is calculated and no new PING is sent yet.
                }
                else
                {
                    printf("Client (%d): Unsolicited PONG message. Resending PING.\n", client_socket);
                    bzero(sending_message,256); ///Initializing zero to sending message.
                    strcpy(sending_message,"PING\n"); ///Copying error message to sending message.
                    message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                    if (message_length < 0) ///Checking whether the message has been sent ot not.
                    {
                        perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                    }
                    else
                    {
                        printf("Message delivered to client successfully.\n\n\n");
                        gettimeofday(&start_t, NULL);
                    }
                    registered_client[client_socket].ping_sent = 1; ///Setting the flag to true since PING has already been sent again.
                }
            }
        }
        else if(strcmp(received_message,"LEAVE") == 0) ///Comparing if the received command is PONG or not.
        {
            printf("Client (%d): LEAVE\n", client_socket);
            if(registered_client[client_socket].fd == 0) ///Checking if the user is existing user or not.
            {
                printf("Unable Locate Client (%d) in Database. Disconnecting User.\n", client_socket);
                printf("Client (%d): Disconnecting User.\n\n\n", client_socket);
                bzero(sending_message,256); ///Initializing zero to sending message.
                strcpy(sending_message,"Connection closed by foreign host.\n"); ///Copying error message to sending message.
                message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                if (message_length < 0) ///Checking whether the message has been sent ot not.
                {
                    perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                }
                else
                {
                    printf("Message delivered to client successfully.\n\n\n");
                }
            }
            else
            {
                printf("Client (%d): Disconnecting User.\n\n\n", client_socket);
                bzero(sending_message,256); ///Initializing zero to sending message.
                strcpy(sending_message,"Connection closed by foreign host.\n"); ///Copying error message to sending message.
                message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                if (message_length < 0) ///Checking whether the message has been sent ot not.
                {
                    perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                }
                else
                {
                    printf("Message delivered to client successfully.\n\n\n");
                }
                strcpy(registered_client[client_socket].username,"\0"); ///Initializing null to username of leaving client.
                registered_client[client_socket].fd = 0; ///Initializing zero to descriptor of leaving client.
                registered_client[client_socket].rtt = 0; ///Initializing zero to rtt of leaving client.
                int i, j = 0;
                for(i = 1; i <= registered_client_counter; i++) ///Reshaping the database for leaving one client.
                {
                    if(j)
                    {
                        registered_client_id[i-1] = registered_client_id[i];
                    }
                    else if(registered_client_id[i] == client_socket)
                    {
                        j = 1;
                    }
                }
                registered_client_counter--; ///Decrementing registered client number for leaving one client.
            }
            int i, j = 0;
            for(i = 1; i < connected_client_counter ; i++) ///Reshaping the connected clients list for leaving one client.
            {
                if(j)
                {
                    connected_client_id[i-1] = connected_client_id[i];
                }
                else if(connected_client_id[i] == client_socket)
                {
                    j = 1;
                }
            }
            connected_client_counter--; ///Decrementing connected client number for leaving one client.
            break;
        }
        else
        {
            sscanf(received_message, "%s %s", command, username); ///Retrieving command and username from received message.

            if(strcmp(command,"JOIN") == 0) ///Comparing if the received command is JOIN or not.
            {
                if(registered_client[client_socket].fd != 0)
                {
                    printf("Client (%d): User Already Registered. Discarding JOIN.\n", client_socket);
                    bzero(sending_message,256); ///Initializing zero to sending message.
                    sprintf(sending_message, "User Already Registered: Username (%s), FD (%d)\n", registered_client[client_socket].username, registered_client[client_socket].fd);
                    message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                    if (message_length < 0) ///Checking whether the message has been sent ot not.
                    {
                        perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                    }
                    else
                    {
                        printf("Message delivered to client successfully.\n\n\n");
                    }
                }
                else
                {
                    int i, current_client_number;
                    for(i = 1; i <= connected_client_counter; i++)///Finding out the client number in connected client list for current client.
                    {
                        if(connected_client_id[i] == client_socket)
                        {
                            current_client_number = i;
                            break;
                        }
                    }
                    if(current_client_number > 20) ///Checking if the client number of current client is beyond 20 or not.
                    {
                        printf("Client (%d): Database Full. Disconnecting User.\n", client_socket);
                        bzero(sending_message,256); ///Initializing zero to sending message.
                        strcpy(sending_message,"Too Many Users. Disconnecting User.\nConnection closed by foreign host.\n"); ///Copying error message to sending message.
                        message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                        if (message_length < 0) ///Checking whether the message has been sent ot not.
                        {
                            perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                        }
                        else
                        {
                            printf("Message delivered to client successfully.\n\n\n");
                        }
                        int i, j = 0;
                        for(i = 1; i < connected_client_counter ; i++) ///Reshaping the connected clients list for leaving one client.
                        {
                            if(j)
                            {
                                connected_client_id[i-1] = connected_client_id[i];
                            }
                            else if(connected_client_id[i] == client_socket)
                            {
                                j = 1;
                            }
                        }
                        connected_client_counter--; ///Decrementing connected client number for leaving one client.
                        break;
                    }
                    else
                    {
                        printf("Client (%d): %s\n", client_socket, received_message);
                        if(registered_client_counter < 20) ///Checking if the number of registered client is beyond 20 or not.
                        {
                            if(registered_client[client_socket].fd == 0)
                            {
                                registered_client_counter++; ///Incrementing registered client number.
                                registered_client_id[registered_client_counter] = client_socket; ///Storing client descriptor into registered client list.
                            }
                            strcpy(registered_client[client_socket].username, username); ///Storing username into database.
                            registered_client[client_socket].fd = client_socket; ///Storing client descriptor into database.
                            strcpy(sending_message,"PING\n"); ///Copying error message to sending message.
                            message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                            if (message_length < 0) ///Checking whether the message has been sent ot not.
                            {
                                perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                            }
                            else
                            {
                                printf("Message delivered to client successfully.\n\n\n");
                                gettimeofday(&start_t, NULL); ///Getting the time when the PING has been sent to client.
                            }
                            registered_client[client_socket].ping_sent = 1; ///Setting the flag to true since PING has already been sent.
                        }
                        else
                        {
                            printf("Error: Too Many Clients Connected\n\n\n");
                            bzero(sending_message,256); ///Initializing zero to sending message.
                            strcpy(sending_message,"Too Many Users.\n"); ///Copying error message to sending message.
                            message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                            if (message_length < 0) ///Checking whether the message has been sent ot not.
                            {
                                perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                            }
                            else
                            {
                                printf("Message delivered to client successfully.\n\n\n");
                            }
                        }
                    }
                }
            }
            else
            {
                if(registered_client[client_socket].fd == 0) ///Checking if the user is existing user or not.
                {
                    printf("Unable Locate Client (%d) in Database. Discarding UNKNOWN Message.\n\n\n", client_socket);
                    bzero(sending_message,256); ///Initializing zero to sending message.
                    strcpy(sending_message,"Unregistered User. Use \"JOIN <username>\" to Register.\n"); ///Copying error message to sending message.
                    message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                    if (message_length < 0) ///Checking whether the message has been sent ot not.
                    {
                        perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                    }
                    else
                    {
                        printf("Message delivered to client successfully.\n\n\n");
                    }
                }
                else
                {
                    printf("Client (%d): Unrecognizable Message. Resending PING.\n\n\n", client_socket);
                    bzero(sending_message,256); ///Initializing zero to sending message.
                    strcpy(sending_message,"PING\n"); ///Copying error message to sending message.
                    message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                    if (message_length < 0) ///Checking whether the message has been sent or not.
                    {
                        perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                    }
                    else
                    {
                        printf("Message delivered to client successfully.\n\n\n");
                        gettimeofday(&start_t, NULL);
                    }
                    registered_client[client_socket].ping_sent = 1; ///Setting the flag to true since PING has already been sent again.
                }
            }
        }
    }

    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{

    printf("+----------------------------------------------+\n");
	printf("|             Name: Utpal Kumar Dey            |\n");
	printf("|                 Server Program               |\n");
	printf("+----------------------------------------------+\n\n\n\n");

	int server_socket, client_socket, port_number, message_length; ///Declaring variables for both server socket and client socket, port number and message length.
	socklen_t client_address_length; ///Declaring variable for client socket address length.
	char sending_message[256]; ///Declaring character array to store received message from client to server and sending message from server to client respectively.
	struct sockaddr_in server_address, client_address; ///Declaring variable for server socket address and client socket address respectively.

	pthread_t thread[100]; ///Declaring thread.
	int thread_counter;

	if(argc < 2) ///Checking whether initial input arguments are correct or not.
	{
	    fprintf(stderr,"!!!ERROR!!!, Not enough input provided.\n\n\n");
		exit(1);
	}
	else
	{
	    printf("Input seems okay.\n\n\n");
	}

	server_socket = socket(AF_INET, SOCK_STREAM, 0); ///Defining server socket.

	if (server_socket < 0) ///Checking whether server socket is opened properly or not.
	{
		perror("!!!ERROR!!!, Can not open server socket.\n\n\n");
	}
	else
	{
	    printf("Server socket opened successfully.\n\n\n");
	}

	bzero((char*) &server_address, sizeof(server_address)); ///Initializing zero to server address variable.
	port_number = atoi(argv[1]); ///Converting port number from character to integer.
	server_address.sin_family = AF_INET; ///Defining internet socket address family.
	server_address.sin_addr.s_addr = INADDR_ANY; ///Defining internet address.
	server_address.sin_port = htons(port_number); ///Assigning port number.

	if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) ///Checking whether server socket is bind properly or not.
	{
		perror("!!!ERROR!!!, Can not bind server socket.\n\n\n");
	}
	else
	{
	    printf("Server socket bind successfully.\n\n\n");
	    printf("Waiting for Incoming connections.....\n\n\n");
	}

    int i;
	for(i = 0; i < 1000; i++) ///Initializing zero to client registration database.
	{
	    strcpy(registered_client[i].username, "\0");
	    registered_client[i].fd = 0;
	    registered_client[i].rtt = 0;
	    registered_client[i].ping_sent = 0;
	}

	thread_counter = 0;

	while(1)
	{
	    listen(server_socket,5); ///Waiting for connection to be established.
        client_address_length = sizeof(client_address); ///Calculating length of client address.
        client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_length); ///Establishing server socket with client socket.

        if (client_socket < 0) ///Checking whether connection with client is successful or not.
        {
            perror("!!!ERROR!!!, Can not connect with client.\n\n\n");
        }
        else
        {
            if(registered_client_counter >=20 ) ///Checking if the number of registered client is exceeding 20 or not.
            {
                printf("Error: Too Many Clients Connected\n\n\n"); ///Printing out error message on server.
                bzero(sending_message,256); ///Initializing zero to sending message.
                strcpy(sending_message,"Connection closed by foreign host.\n"); ///Copying error message to sending message.
                message_length = write(client_socket,sending_message,strlen(sending_message)); ///Sending the message to client and getting the length.
                if (message_length < 0) ///Checking whether the message has been sent ot not.
                {
                    perror("!!!ERROR!!!, Can not send message to client.\n\n\n");
                }
                else
                {
                    printf("Message delivered to client successfully.\n\n\n");
                }
            }
            else
            {
                printf("Client (%d): Connection Accepted\n", client_socket); ///Printing out client establishment message.
                registered_client[client_socket].ping_sent = 0; ///Setting ping flag to false.
                connected_client_counter++; ///Increamenting number of connected clients.
                connected_client_id[connected_client_counter] = client_socket; ///Storing client descriptor for connected client.

                if(pthread_create(&thread[thread_counter++], NULL, connection_handler, (void*) &client_socket) < 0) ///Creating thread and checking whether thread is created properly or not.
                {
                    perror("could not create thread");
                    return 1;
                }
            }
        }
	}

    close(client_socket); ///Closing client socket
	close(server_socket); ///Closing server socket

	return 0;
}
