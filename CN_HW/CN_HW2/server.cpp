/**
 *  �̸� : �̿���(3�г�)
 *  ���� : 2011004040
 *  ���� : ��ǻ������
 *  ���� : ��ǻ�� ��Ʈ��ũ ���� ���α׷��� - ����(Server)
 */

#define _CRT_NO_TIME_T
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <winsock.h>
#include <pthread.h>
#include <map>

#define DEFAULT_PORT	8000
#define MAX_CLIENT_NUM	10								/* �ִ� Ŭ���̾�Ʈ �� 10�� */
#define NAMESIZE		10								/* Ŭ���̾�Ʈ ID �ִ� ����, client.cpp ������ ���� */
#define MSGSIZE			500								/* �޽��� �ִ� ���� */
#define BUFSIZE			(NAMESIZE + 1 + MSGSIZE + 1)	/* ���� ũ�� */ 
#define REQ_ID			"/getid\n"						/* ID ��û ��ɾ� */
#define END_MSG			"/quit\n"						/* ���� ��ɾ�, ���� <-> Ŭ���̾�Ʈ ����⿡�� ��� ��� �� */
#define PULL_FILE		"/pull "						/* ���� ��û ��ɾ� */

using namespace std;

void *client_connection(void *arg);					// Ŭ���̾�Ʈ�� ���� �Ǹ鼭 ����� ��� ������ �Լ�
void *send_file(void *arg);							// Ŭ���̾�Ʈ�� ���� ��û�� ����� ���� ������ �Լ�


void error_handling(char *message)					// ���� �߻� �� ����Ǵ� �Լ�
{
	fprintf(stderr, "%s\n", message);				// ���� ���
	exit(1);										// ���α׷� ����
}

int client_num;										// ������ ���� �� Ŭ���̾�Ʈ ��
int server_sock;									// ���� ����
int client_socks[MAX_CLIENT_NUM];					// Ŭ���̾�Ʈ ���� �迭
bool bInTransit[MAX_CLIENT_NUM];					// Ŭ��

map<int, FILE *> fpStorage;							// �� Ŭ���̾�Ʈ �� ���������� �����
pthread_mutex_t socketLock;							// Ŭ���̾�Ʈ �迭(client_socks)�� ���� ���� ������ �����ϱ� ���� mutex
pthread_mutex_t senderLock;							// ���� �����忡�� ���ÿ� send��� ���� ���� ������ �����ϱ� ���� mutex

void sigint_handler(int signal_no)					// Ctrl-C �Է½� ó���� �Լ�
{
	int i;

	printf("%d : Ctrl-C Pressed, ������ �����մϴ�\n", signal_no);

	pthread_mutex_lock(&socketLock);						// ���� ���� �迭 client_socks�� ���� ���¸� ���� ���� Lock

	for(i = 0 ; i < client_num ; i++)						// ��� ���Ͽ� ���Ͽ�
	{
		pthread_mutex_lock(&senderLock);					//
		send(client_socks[i], END_MSG, sizeof(END_MSG), 0);	// ���� ���� �뺸 �Ϸ�
		pthread_mutex_unlock(&senderLock);					//
	}

	for( i = 0 ; i < client_num ; i++ )						// ��� ���Ͽ� ���Ͽ�
		closesocket(client_socks[i]);
	pthread_mutex_unlock(&socketLock);						// Ŭ���̾�Ʈ ���� �ݱ� �Ϸ�, Lock ����

	closesocket(server_sock);								// ���� ���� ����
	WSACleanup();											// Winsock �޸� ����

															// ���α׷� ����
}

int main(int argc, char **argv)
{
	WSADATA wsaData;
	int i, result;
	int client_sock;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int client_addr_size;
	pthread_t thread;																					//

	if( pthread_mutex_init(&socketLock, NULL) || pthread_mutex_init(&senderLock, NULL) )	// ���ؽ� �ʱ�ȭ ���� ��, ���� ��� �� ����
		error_handling("mutex init error");													//

	// Initialize Winsock
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);											// Winsock �ʱ�ȭ
	if(result != 0)																			// ���� �� ���� ��� �� ����
	{																						//
		printf("WSAStartup failed with error: %d\n", result);								//
		return 1;																			//
	}																						//

	server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);								// ���� ���� ����
	if(server_sock == -1)																	// ���� �� ���� ��� �� ����
	{																						//
		WSACleanup();																		//
		error_handling("socket() error");													//
	}																						//

	memset(&server_addr, 0, sizeof(server_addr));											// ���� ���� �ʱ�ȭ
	server_addr.sin_family = AF_INET;														// IPv4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);										// ���� IP
	server_addr.sin_port = htons((argc < 2) ? DEFAULT_PORT : atoi(argv[1]));				// ���������� ���� �� 8000

	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)		// ���� ���� ����
	{																						// ���� �� ���� ��� �� ����
		closesocket(server_sock);															// 
		WSACleanup();																		//
		error_handling("bind() error");														//
	}																						//

	if(listen(server_sock, SOMAXCONN) == -1)												// ������ ���� ���� ��û Ȯ��
	{																						// ���� �� ���� ��� �� ����
		closesocket(server_sock);															//
		WSACleanup();																		//
		error_handling("listen() error");													//
	}																						//

	signal(SIGINT, sigint_handler);															// ���ͷ�Ʈ �� ������ ������ sigint_handler �Լ��� ����ǰ� ��
	client_num = 0;																			// �ʱ� Ŭ���̾�Ʈ ���� 0���� �ʱ�ȭ
	while( true )																					// accept �ݺ�
	{
		if(client_num < MAX_CLIENT_NUM)																// �ִ� Ŭ���̾�Ʈ ��� �� �̳��θ� accept
		{
			printf("���� ��� ��... �����Ϸ��� Ctrl-C�� �����ּ���\n");
			client_addr_size = sizeof(client_addr);
			client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);	// accept ����(Blocking)
			if(client_sock == -1)																	// accept ���� �� �ݺ��� ���� = ���α׷� ����
				break;

			pthread_mutex_lock(&socketLock);														// ���� ���� �迭 client_socks�� ���� ���¸� ���� ���� Lock
			client_socks[client_num++] = client_sock;												// Ŭ���̾�Ʈ ���� ����, ��ü Ŭ���̾�Ʈ ���� �� ����
			pthread_mutex_unlock(&socketLock);														// ���� �� ���� ���� �Ϸ�, Lock ����

			pthread_create(&thread, NULL, client_connection, (void*)client_sock);					// Ŭ���̾�Ʈ���� ����� ����� ������ ����(�Լ� - client_connection)
			printf("IP : %s:%d - ID : %d�� ����Ǿ����ϴ�\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, client_sock);
		}
		else
		{
			printf("���� ���� ����... �����Ϸ��� Ctrl-C�� �����ּ���\n");							// Ŭ���̾�Ʈ ���� �ִ� ������ �Ѿ�� ���
			Sleep(5000);																			// 5�� ���� ��� ����, �ٸ� �����忡 ���� ��ȸ�� �� �ο�
		}
	}

	pthread_mutex_lock(&socketLock);						// ���� ���� �뺸�� ���� Lock

	for(i = 0 ; i < client_num ; i++)						// ��� ���Ͽ� ���Ͽ�	
	{
		pthread_mutex_lock(&senderLock);					// ���� ���� �迭 client_socks�� ���� ���¸� ���� ���� Lock
		send(client_socks[i], END_MSG, sizeof(END_MSG), 0);	// Ŭ���̾�Ʈ ���� ���� ���� �뺸
		pthread_mutex_unlock(&senderLock);					// ���� ���� �뺸 �Ϸ�, Lock ����
	}

	for(i = 0 ; i < client_num ; i++)						// ��� ���Ͽ� ���Ͽ�
		closesocket(client_socks[i]);						// ���� �ݱ�

	pthread_mutex_unlock(&socketLock);						// ���� ���� ���� �Ϸ�, Lock ����

	closesocket(server_sock);								// ���� ���� �ݱ�
	WSACleanup();											// Winsock �޸� ����

	printf("�޸� ���� �Ϸ�, 3�� �� ������ ����˴ϴ�\n");
	Sleep(3000);

	return 0;												// ���� ���α׷� ����
}

void *client_connection(void *arg)
{
	int i, data[2], client_sock, str_len;
	char message[BUFSIZE], *tmpStr, *filename;
	pthread_t send_thread;

	client_sock = (int)arg;
	filename = tmpStr = NULL;
	str_len = 0;
	while( true )																					// ���α׷� ���� ������ recv �ݺ�
	{
		if( (str_len = recv(client_sock, message, sizeof(message), 0)) > 0 )						// ���� ���� ��ȭ�� ������ �� ��ȯ, ���ٸ� Blocking
		{
			message[str_len] = NULL;
			if( strncmp(message, END_MSG, sizeof(END_MSG)) == 0 )									// END_MSG = /quit\n�� ���� �� ���, �ݺ��� ���� = ������ ����
				break;
			else if(strncmp(message, REQ_ID, sizeof(REQ_ID)) == 0)									// REQ_ID = /getid\n�� ���� �� ���, Ŭ���̾�Ʈ ���� �� ����
			{
				_itoa(client_sock, message, 10);													// Ŭ���̾�Ʈ ���� �� FROM (int) TO (char *)
				pthread_mutex_lock(&senderLock);													// ���� �ٸ� �����忡�� ���ÿ� Send �ϴ� ���� ���� ���� Lock
				send(client_sock, message, strlen(message), 0);										// Ŭ���̾�Ʈ ���� �� ����
				pthread_mutex_unlock(&senderLock);													// ���� ��, Lock ����
			}
			else if( strncmp(message, PULL_FILE, strlen(PULL_FILE)) == 0 )							// PULL_FILE = /pull ~�� ���� �� ���, ���� ���� ������ ���� 
			{
				tmpStr = (char *)malloc(sizeof(char) * (strlen(message + strlen(PULL_FILE)) + 1));	// �����Ҵ�
				sprintf(tmpStr, "%s", message + strlen(PULL_FILE));									// tmpStr�� ���ϸ� ����
				tmpStr[strlen(tmpStr) - 1] = NULL;													// ���ڿ� NULL ó��

				data[0] = client_sock;																// Ŭ���̾�Ʈ ���� �� ����
				data[1] = (int)tmpStr;																// ���ϸ� ����
				pthread_create(&send_thread, NULL, send_file, (void *)data);						// ���� ���� ������ ����
			}
			else if( strncmp(message, "HEAD_OF_FILE", strlen("HEAD_OF_FILE")) == 0 )				// HEAD_OF_FILE - Ŭ���̾�Ʈ�� ������ ������ ������ �뺸�ϴ� ���� ����
			{
				if(fpStorage[client_sock] != NULL)													// ���������ڰ� �ʱ�ȭ �Ǿ�����
					fclose(fpStorage[client_sock]);													// ���������ڸ� �ݴ´�

				if( filename != NULL && strlen(filename) < MSGSIZE )								// ���� �̸����� �Ҵ�� ���� �޸𸮰� �̹� �ִٸ�
					free(filename);																	// ���� �޸𸮸� ���� ���ش�

				filename = (char *)malloc(sizeof(char) * (strlen(message + sizeof("HEAD_OF_FILE")) + 1));
				sprintf(filename, "%s", message + sizeof("HEAD_OF_FILE"));							// �޸𸮸� ���� �Ҵ��Ͽ� ���ϸ��� ���� ��Ŵ

				fpStorage[client_sock] = fopen(filename, "w");										// ������ ��������� Open
			}
			else if( strncmp(message, "FILE", strlen("FILE")) == 0 )								// FILE - ���۵� �����Ͱ� ���� ���������� ����
			{
				if(fpStorage[client_sock] != NULL)
					fputs(message + sizeof("FILE"), fpStorage[client_sock]);						// ���޵� �����͸� ���Ͽ� ����
			}
			else if( strncmp(message, "END_OF_FILE", strlen("END_OF_FILE")) == 0 )					// END_OF_FILE - ���� ������ �������� ����
			{
				if(fpStorage[client_sock] != NULL)
				{
					fclose(fpStorage[client_sock]);													// ���� ����
					fpStorage[client_sock] = NULL;													// Ŭ���̾�Ʈ ���������� �ʱ�ȭ
				}
				else
					continue;

				if(filename != NULL && strlen(filename) < MSGSIZE)
				{
					sprintf(message, "%s - ������ ���� ��\n", filename);							// Ŭ���̾�Ʈ �鿡 ������ �޽��� ����

					free(filename);																	// ���ϸ� �Ҵ�� �޸� ����
					filename = NULL;																// ���ϸ� �ʱ�ȭ
				}
				else
					continue;

				pthread_mutex_lock(&socketLock);						// �ٸ� ������ ���ŵ� ��, ���� �迭 �������� ��ġ�� ���� �� �� �����Ƿ� Lock

				for(i = 0; i < client_num; i++)							// ��� Ŭ���̾�Ʈ ���Ͽ� ���Ͽ�
				{
					pthread_mutex_lock(&senderLock);					// ���� �ٸ� �����忡�� ���ÿ� Send �ϴ� ���� ���� ���� Lock
					send(client_socks[i], message, strlen(message), 0);	// ������ ������ ��������� Ŭ���̾�Ʈ �鿡�� �뺸
					pthread_mutex_unlock(&senderLock);					// �뺸 �Ϸ� Lock ����
				}

				pthread_mutex_unlock(&socketLock);						// ������ ���� �Ϸ�, Lock ����
			}
			else
			{
				pthread_mutex_lock(&socketLock);						// �ٸ� ������ ���ŵ� ��, ���� �迭 �������� ��ġ�� ���� �� �� �����Ƿ� Lock

				for(i = 0; i < client_num; i++)							// ��� Ŭ���̾�Ʈ ���Ͽ� ���Ͽ�
				{
					pthread_mutex_lock(&senderLock);					// ���� �ٸ� �����忡�� ���ÿ� Send �ϴ� ���� ���� ���� Lock
					send(client_socks[i], message, strlen(message), 0);	// ���� �޽����� �״�� Ŭ���̾�Ʈ �鿡�� �ѷ���
					pthread_mutex_unlock(&senderLock);					// �뺸 �Ϸ� Lock ����
				}

				pthread_mutex_unlock(&socketLock);						// ������ ���� �Ϸ�, Lock ����
			}
		}
		else
			break;
	}

	pthread_mutex_lock(&socketLock);									// ������ ������ ���ڸ��� ������ ���ϵ��� ��ܼ� ä��, ���� �������� ���� �߻��� ���� ���� Lock ����
	for(i = 0; i < client_num ; i++)									// 0�� ���� ~ N - 1 ����
	{																	//
		if(client_sock == client_socks[i])								// ��ü ���� �� ������ ������ ���
		{																//
			while(i < client_num - 1)									// �ش� ���� ������ ��� ���ϵ鿡 ���Ͽ�
			{															//
				client_socks[i] = client_socks[i + 1];					// ������ �� ����������� �̵�, �����Ų��
				i++;													// ���� �������� �̵�
			}															//
			break;														// Ž�� ����
		}																//
	}																	//
	client_num--;														// ���� ���� ����
	pthread_mutex_unlock(&socketLock);									// ���� ���� ���� �Ϸ�, Lock ����

	printf("ID : %d�� ������ ����Ǿ����ϴ�(���� : Ctrl-C)\n", client_sock);

	closesocket(client_sock);											// Ŭ���̾�Ʈ ���� ����

	return 0;
}

void *send_file(void *arg)												// Ŭ���̾�Ʈ�� ��û���� �������� Ŭ���̾�Ʈ�� ������ ������ ������ �Լ�
{
	int sock, str_len, file_size;
	char file[BUFSIZE] = {0, }, tmp[BUFSIZE] = {0,}, *filename;

	sock = ((int *)arg)[0];
	filename = (char *)(((int *)arg)[1]);

	snprintf(tmp, BUFSIZE - 1, "HEAD_OF_FILE_%s", filename);			// HEAD_OF_FILE_���ϸ�
	pthread_mutex_lock(&senderLock);									// ���� �ٸ� �����忡�� ���ÿ� Send �ϴ� ���� ���� ���� Lock
	send(sock, tmp, strlen(tmp) + 1, 0);								// HEAD_OF_FILE_���ϸ� ����
	pthread_mutex_unlock(&senderLock);									// ���� �Ϸ� Lock ����

	FILE *fp = fopen(filename, "r");

	if(fp != NULL)
	{
		fseek(fp, 0, SEEK_END);											// ���� �д� ��ġ�� ������ �̵�
		file_size = ftell(fp);											// ���� �д� ��ġ�� ��ȯ = (���� ũ��)
		fseek(fp, 0, SEEK_SET);											// ���� �д� ��ġ�� ó������ �̵�

		if(file_size > 0)												// ���� ũ�Ⱑ 0���� ū ��쿡�� ����
		{
			memset(file, 0, sizeof(file));
			memset(tmp, 0, sizeof(file));

			snprintf(tmp, BUFSIZE - 1, "FILE_SIZE_%d", file_size);		// FILE_SIZE_����ũ��
			pthread_mutex_lock(&senderLock);							// ���� �ٸ� �����忡�� ���ÿ� Send �ϴ� ���� ���� ���� Lock
			send(sock, tmp, strlen(tmp) + 1, 0);						// FILE_SIZE_����ũ�� ����
			pthread_mutex_unlock(&senderLock);							// ���� �Ϸ� Lock ����

			memset(file, 0, sizeof(file));
			memset(tmp, 0, sizeof(file));
			while((str_len = fread(tmp, 1, MSGSIZE / 2, fp)) > 0)		// �ѱ� ���� �����ڵ�� �ϳ� �� 2 bytes ���� (MSGSIZE / 2) ��ŭ �� �а� ��
			{
				tmp[str_len] = NULL;
				sprintf(file, "FILE_");									// FILE_
				strcat(file, tmp);										// FILE_���� ���� ����

				pthread_mutex_lock(&senderLock);						// ���� �ٸ� �����忡�� ���ÿ� Send �ϴ� ���� ���� ���� Lock
				Sleep(1000);											// �����ϰ� ���� �����尡 CPU �ڿ��� �����ϴ� ���� ����
				send(sock, file, strlen(file) + 1, 0);					// FILE_���� ���� ���� ����
				pthread_mutex_unlock(&senderLock);						// ���� �Ϸ� Lock ����

				memset(file, 0, sizeof(file));
				memset(tmp, 0, sizeof(file));
			}

			fclose(fp);													// ���� ����
		}

		pthread_mutex_lock(&senderLock);								// ���� �ٸ� �����忡�� ���ÿ� Send �ϴ� ���� ���� ���� Lock
		send(sock, "END_OF_FILE_O", strlen("END_OF_FILE_O") + 1, 0);	// END_OF_FILE_O ���� - ���� ����
		pthread_mutex_unlock(&senderLock);								// ���� �Ϸ� Lock ����
	}
	else
	{
		pthread_mutex_lock(&senderLock);								// ���� �ٸ� �����忡�� ���ÿ� Send �ϴ� ���� ���� ���� Lock
		send(sock, "END_OF_FILE_X", strlen("END_OF_FILE_X") + 1, 0);	// END_OF_FILE_O ���� - ���� ����
		pthread_mutex_unlock(&senderLock);								// ���� �Ϸ� Lock ����
	}

	free(filename);														// ���ϸ� ���忡 �Ҵ�� ���� �޸� ����

	return 0;															// ���� ���� ������ ����
}