#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



int init_server(int port){

	printf("\nWELCOME TO FTP SERVER V 0.1\n\n");
	
	int server_socket; 
	int sockoptval = 1;
	struct sockaddr_in server_add;
	
	if ( 0 > (server_socket = socket(AF_INET, SOCK_STREAM, 0))){
		perror("Socket not created\n");
		exit(EXIT_FAILURE);
	} 
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));
	printf("socket created successfully\n\n");
	
	memset((char *)&server_add, 0, sizeof(struct sockaddr_in));
	server_add.sin_family = AF_INET;
	server_add.sin_port = htons(port);
	server_add.sin_addr.s_addr = htonl(INADDR_ANY);

	if (0 > bind(server_socket, (struct sockaddr *)&server_add, sizeof(struct sockaddr_in))){
		perror("Bind failed");
		close(server_socket);
		exit(EXIT_FAILURE);
	}
	printf("port %i opened successfully\n\n", port);

	printf("Accepting Incoming Connections on %i:%i...\n", ntohl(server_add.sin_addr.s_addr), ntohs(server_add.sin_port));
	if (0 > listen(server_socket, 1)){
		perror("Listen failed");
		close(server_socket);
		exit(EXIT_FAILURE);
	}
	return server_socket;
}

void ftp_user(int conn_sock){
	char *rsp = "331 access allowed.\r\n";
	write(conn_sock, rsp, strlen(rsp));

}

void ftp_pass(int conn_sock){
	char *rsp = "230 login successfull.\r\n";
	write(conn_sock, rsp, strlen(rsp));

}

void ftp_syst(int conn_sock){
	char *rsp = "215\r\n";
	write(conn_sock, rsp, strlen(rsp));
	//write(conn_sock, "226 comp\n", 9);

}

void ftp_feat(int conn_sock){
	char *rsp = "211 End.\r\n";
	write(conn_sock, rsp, strlen(rsp));
}

void handle_command(int conn_sock, char *cmd){
	char *commd = cmd;
	while(++commd){
		if('\n' == (*commd)){
			*commd = '\0';
			break;
		}
	}
	printf("%s\n", cmd);
	if (!strncmp(cmd, "USER", 4)){
		ftp_user(conn_sock);
	}
	else if (!strncmp(cmd, "PASS", 4)){
		ftp_pass(conn_sock);
	}

	else if (!strncmp(cmd, "SYST", 4)){
		ftp_syst(conn_sock);
	}
	/*else if (!strncmp(cmd, "FEAT", 4)){
		ftp_feat(conn_sock);
	}*/
	else if (!strncmp(cmd, "QUIT", 4)){
		char *rsp = "221 connection terminated by client.\r\n";
		write(conn_sock, rsp, strlen(rsp));
	}
	else {
		char *rsp = "202 invalid command.\r\n";
		write(conn_sock, rsp, strlen(rsp));
	}

}


void ftp_protocol(int conn_sock){
	char *welcome_message = "220 Welcome to Simple FTP server!!\r\n";
	char buff[256];
	//char command[64];
	int cmd;
	int rd;

	if (-1 == (write(conn_sock, welcome_message, strlen(welcome_message)))){
		perror("write error");
		exit(EXIT_FAILURE);
	}
	cmd = 0;
	while ((rd = read(conn_sock, buff, sizeof(buff)))){

		if (-1 == rd){
			perror("read error");
			exit(EXIT_FAILURE);
		}
		handle_command(conn_sock, buff);
	}
	//sscanf(buff, "%s", command);
	printf("closing...\n");
	close(conn_sock);
}

int main(int argc, char *argv[]){
	if (2 != argc){
		printf("Format: ./my_ftpd <port>\nTerminating program...\n\n");
		exit(EXIT_FAILURE);
	}
	
	int pid;
	uint visitors = 0;
	int port;
	int server_socket;
	int connect_socket;
	//struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	
	if (0 == (port = atoi(argv[1]))){
		perror("Invalid port\n");
		close(server_socket);
		exit(EXIT_FAILURE);
	}
	server_socket = init_server(port);
	while((connect_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len))){
		printf("\nestablishing connection...\n");
		printf("there have been %i visitors\n", ++visitors);
		if(-1 == connect_socket){
			perror("accept error");
			close(server_socket);
			exit(EXIT_FAILURE);
		}
		printf("Connection from %i:%i\n", ntohl(client_addr.sin_addr.s_addr), ntohs(client_addr.sin_port));
		ftp_protocol(connect_socket);
	}

	printf("Closing socket\n");
	close(server_socket);
	
	return EXIT_SUCCESS;
}