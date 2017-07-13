/**
*  �̸� : �̿���(3�г�)
*  ���� : 2011004040
*  ���� : ��ǻ������
*  ���� : ��ǻ�� ��Ʈ��ũ ���� ���α׷��� - Ŭ���̾�Ʈ
*/

#define _CRT_NO_TIME_T
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <winsock.h>
#include <pthread.h>
#include <queue>

#define DEFAULT_PORT		8000							/* �⺻ ���� ���� ��Ʈ */
#define DEFAULT_SERVER_IP	"127.0.0.1"						/* �⺻ ���� ���� IP */
#define QUEUESIZE			20								/* �ִ� �޽��� ���� ���� 20�� */
#define NAMESIZE			10								/* Ŭ���̾�Ʈ ID �ִ� ����, client.cpp ������ ���� */
#define MSGSIZE				500								/* �޽��� �ִ� ���� */
#define BUFSIZE				(NAMESIZE + 1 + MSGSIZE + 1)	/* ���� ũ�� */ 
#define REQ_ID				"/getid\n"						/* ID ��û ��ɾ� */
#define END_MSG				"/quit\n"						/* ���� ��ɾ�, ���� <-> Ŭ���̾�Ʈ ����⿡�� ��� ��� �� */
#define PULL				"/pull "						/* ���� ��û ��ɾ� */
#define PUSH				"/push"							/* ���� ���� ��ɾ� */
#define STOP				"/stop"							/* ���� ��û ��� ��ɾ� */

using namespace std;

void *send_message(void* arg);						// �������� �޽��� ���� ������
void *recv_message(void* arg);						// �������� �޽��� ���� ������
void *send_file(void* arg);							// ������ ������ �����ϴ� ������

void error_handling(char *message)					// ���� �߻� �� ����Ǵ� �Լ�
{
	fprintf(stderr, "%s\n", message);				// ���� ���
	exit(1);										// ���α׷� ����
}

void gotoxy(int x, int y)							// �ܼ� Ŀ�� ��ġ ���� �Լ�
{
	COORD Cur;
	Cur.X = x;
	Cur.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);
}

COORD getXY()										// ���� �ܼ� Ŀ�� ��ġ ��ȯ �Լ�
{
	COORD Cur;
	CONSOLE_SCREEN_BUFFER_INFO a;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &a);
	Cur.X = a.dwCursorPosition.X;
	Cur.Y = a.dwCursorPosition.Y;

	return Cur;
}

char name[NAMESIZE + 1];							// Ŭ���̾�Ʈ �̸�, ������ ����� �� �������� ��ġ�� �ʴ� �̸��� �޾ƿ�
int sock;											// ������ ����Ǵ� ����

pthread_t snd_thread, rcv_thread, file_thread;		// ���� ����, ����, �������� ������
pthread_mutex_t consoleLock, socketLock;			// �ܼ� ��� ���, ���� ��� ���

deque<char *> textStorage;							// �ۼ��� �޽��� ���� ����
bool bPulled, bPushed;								// ���� ����, ���� ������ ����, ���� �ۼ����� �� ���� �� ������ �����ϵ��� �ϱ� ���� ����

void sigint_handler(int signal_no)					// Ctrl-C, ���ͷ�Ʈ�� �߻����� �� ����Ǵ� �Լ�, ������ �����Ƿ� �ᱹ ���α׷��� ���� ��
{
	system("cls");									// �ܼ� â ����

	closesocket(sock);								// ���� ����
	WSACleanup();									// Winsock �ʱ�ȭ
}

int main(int argc, char **argv)
{
	int result;																			// ���� �Լ����� ���� ����� �����ϴ� ����
	sockaddr_in serv_addr;																// ���ڷ� �Է¹��� ���� IP, ��Ʈ�� �ԷµǴ� ����
	WSADATA wsaData;																	// Winsock ����
	void *thread_result;																// ������ �������� ����Ǵ� ����, �׽�Ʈ�� ���ؼ� ���� ����

																						// �ܼ� ���� ũ�� => ���� - BUFSIZE / 2 : �ѱ��� ��� 2byte�� �����Ƿ� ������ 2�� ����, ���� - QUEUESIZE + 5 " �Է���(1) + �޽�������(20) + ����â(4)
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), {BUFSIZE / 2, QUEUESIZE + 5});
	// �ܼ� â�� ũ�� ������ ��� ������ �������� ���� Ȯ���Ͽ�, �ƿ� ũ�� ������ ���ϵ��� ����
	SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU));

	if(pthread_mutex_init(&consoleLock, NULL) || pthread_mutex_init(&socketLock, NULL))	// ���ؽ� �ʱ�ȭ ���� �� ���α׷� ����
	{
		system("cls");
		error_handling("mutex init error");
	}

	// Initialize Winsock
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);										// Winsock �ʱ�ȭ
	if(result != 0)																		// ���� �� ���α׷� ����
	{
		system("cls");
		WSACleanup();
		printf("WSAStartup failed with error: %d\n", result);
		return 1;
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);												// ���� ����
	if(sock == -1)																		// ���� �� ���α׷� ����
	{
		system("cls");
		WSACleanup();
		error_handling("socket() error");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;														// ���� ���� �Է�
	serv_addr.sin_addr.s_addr = inet_addr((argc < 3) ? DEFAULT_SERVER_IP : argv[1]);	// ���������� ���� �� 127.0.0.1
	serv_addr.sin_port = htons((argc < 3) ? DEFAULT_PORT : atoi(argv[2]));				// ���������� ���� �� 8000

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)			// ������ ����, ���� �� ����
	{
		system("cls");
		closesocket(sock);
		WSACleanup();
		error_handling("connect() error!");
	}

	send(sock, REQ_ID, sizeof(REQ_ID), 0);												// �������� �̸� ��û(REQ_ID)
	result = recv(sock, name, NAMESIZE, 0);												// �������� �̸� ���� ��ٸ�
	if(result > 0)																		// ������ ���������� �̷��� ���, �̸� ����
	{
		name[result] = NULL;
		strcat(name, "user");
	}
	else																				// ���� �������� ���� �߻� �� ����
	{
		system("cls");
		closesocket(sock);
		WSACleanup();
		error_handling("cannot receive id from server!");
	}

	bPushed = bPulled = false;															// ���� ���� ���� ������ �Ǻ� ���� �ʱ�ȭ
	textStorage = deque<char *>();														// �޽��� �ۼ��� ���� Deque ����

	system("cls");																		// �ܼ� â�� ���
	gotoxy(0, QUEUESIZE);																// Ŀ�� ��ġ �Ʒ��� �̵�
	printf("����:/quit �Է� or Ctrl-C, ��������:/push ���ϸ�, ���Ͽ�û:/pull ���ϸ�\n");// ���� ���α׷� �̿� Tip ���
	printf("USER ID : %s\n", name);														// Ŭ���̾�Ʈ �̸� ���
	printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : �۾� ����");								// �ֱ� ���� �ٿ�ε�/���ε� ���� ���
	gotoxy(0, 0);																		// �Է� ��(row)�� �̵�
	printf("%s>", name);																// Ŭ���̾�Ʈ �̸� ���

	signal(SIGINT, sigint_handler);														// Ctrl-C ���ͷ�Ʈ �߻� �� ���� �� �Լ� ����

	pthread_create(&snd_thread, NULL, send_message, (void*)sock);						// ����� �Է� ������ ������ ó���ϴ� ������ ����
	pthread_create(&rcv_thread, NULL, recv_message, (void*)sock);						// ������ ���� ������ ������ ó���ϴ� ������ ����

	pthread_join(snd_thread, &thread_result);											// ���� �����尡 ���� ������ ���
	pthread_join(rcv_thread, &thread_result);											// ���� �����尡 ���� ������ ���

	closesocket(sock);																	// ���� ����
	WSACleanup();																		// Winsock �ʱ�ȭ

	return 0;																			// ���� ������ ���� = ���α׷� ����
}

void* send_message(void * arg)																					// ����� �Է� ���� ������ �Լ�
{
	int sock;
	char *tmpStr;
	char message[MSGSIZE] = {0,}, total_message[BUFSIZE] = {0,};

	deque<char *>::reverse_iterator revIter, rendIter;

	sock = (int)arg;
	while(true)
	{
		fgets(message, MSGSIZE, stdin);																			// ����� �Է��� ���� �� ���� Block

		if(strncmp(message, END_MSG, sizeof(END_MSG)) == 0)														// �Է��� END_MSG�� �����ϴ� ���
		{
			pthread_mutex_lock(&socketLock);																	// ���� ������ ���� ���� ���� Lock
			send(sock, END_MSG, sizeof(END_MSG), 0);															// ���� ���� �뺸
			pthread_mutex_unlock(&socketLock);																	// �뺸 �Ϸ� Lock ����
		}
		else if(strncmp(message, STOP, strlen(STOP)) == 0)														// �Է��� /stop���� �����ϴ� ���
		{
			if(bPulled)																							// ���� ���� ��� ���� ���
			{
				bPulled = false;																				// ��⸦ ����

				tmpStr = (char *)malloc(sizeof(char) * 26);														// ��� �޽��� ����
				sprintf(tmpStr, "���� ������ ����߽��ϴ�\n");
			}
			else
			{
				tmpStr = (char *)malloc(sizeof(char) * 32);														// ��� �޽��� ����
				sprintf(tmpStr, "���� ��� ���� ������ �����ϴ�\n");
			}
			/* �Ʒ� ������ ���� ���� [�ܼ� â ����]���� ����ؼ� ���� */
			while(textStorage.size() >= QUEUESIZE - 1)															// �޽��� �������� �� á���� Ȯ��, �� �� ��� ���� ������ �޽��� Ż����Ŵ
			{
				free(textStorage.front());
				textStorage.pop_front();
			}
			textStorage.push_back(tmpStr);																		// �޽��� �����Կ� ��� �޽��� �߰�

			pthread_mutex_lock(&consoleLock);																	// ���ÿ� �ֿܼ� ���� �ϴ� ���� ���� ���� Lock
			system("cls");																						// �ܼ� â ����

			gotoxy(0, 1);																						// �޽��� ������ ������� ���
			rendIter = textStorage.rend();
			for(revIter = textStorage.rbegin() ; revIter != rendIter ; revIter++)
				printf("%s", *revIter);

			gotoxy(0, QUEUESIZE);
			printf("����:/quit �Է� or Ctrl-C, ��������:/push ���ϸ�, ���Ͽ�û:/pull ���ϸ�\n");
			printf("USER ID : %s\n", name);
			printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : ���� ���");

			gotoxy(0, 0);																						// �Է� ��(row)�� �̵�
			printf("%s>", name);																				// Ŭ���̾�Ʈ �̸� ���
			pthread_mutex_unlock(&consoleLock);																	// �ܼ� Lock ����
		}
		else if(strncmp(message, PUSH, strlen(PUSH)) == 0)														// �޽����� /push�� �����ϴ� ���
		{
			if(bPushed || bPulled)																				// ���� �ۼ����� �̹� ����ǰ� �ִ� ��� �̸� ����
			{
				tmpStr = (char *)malloc(sizeof(char) * 62);
				sprintf(tmpStr, "���� ���� �ۼ����� �Ұ����մϴ�(�۽���� ��� - ���� �̱���)\n");				// ��� �޽��� ����
			}
			else
			{
				bPushed = true;																					// ���� �۽� ���·� ����
				tmpStr = (char *)malloc(sizeof(char) * 24);
				sprintf(tmpStr, "���� ������ �غ��մϴ�\n");													// ��� �޽��� ����
				message[strlen(message) - 1] = NULL;															// �۽��Ϸ��� ���ϸ�
				pthread_create(&file_thread, NULL, send_file, (void*)(message + strlen("/push ")));				// ���� �۽� ������ ����, ���ϸ� ����
			}

			while(textStorage.size() >= QUEUESIZE - 1)															// ������ ������ [�ܼ� â ����]
			{
				free(textStorage.front());
				textStorage.pop_front();
			}
			textStorage.push_back(tmpStr);

			pthread_mutex_lock(&consoleLock);
			system("cls");

			gotoxy(0, 1);
			rendIter = textStorage.rend();
			for(revIter = textStorage.rbegin() ; revIter != rendIter ; revIter++)
				printf("%s", *revIter);

			gotoxy(0, QUEUESIZE);
			printf("����:/quit �Է� or Ctrl-C, ��������:/push ���ϸ�, ���Ͽ�û:/pull ���ϸ�\n");
			printf("USER ID : %s\n", name);
			printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : ");

			gotoxy(0, 0);
			printf("%s>", name);
			pthread_mutex_unlock(&consoleLock);
		}
		else if(strncmp(message, PULL, strlen(PULL)) == 0)														// �޽����� /pull �� �����ϴ� ���
		{
			if(bPushed || bPulled)																				// ���� �ۼ����� �̹� ����ǰ� �ִ� ��� �̸� ����
			{
				tmpStr = (char *)malloc(sizeof(char) * 62);
				sprintf(tmpStr, "���� ���� �ۼ����� �Ұ����մϴ�(���� ���� ��� ��� : /stop)\n");				// ��� �޽��� ����
			}
			else
			{
				bPulled = true;																					// ���� ���� ���·� ����
				tmpStr = (char *)malloc(sizeof(char) * 24);
				sprintf(tmpStr, "���� ������ ��û�մϴ�\n");													// ��� �޽��� ����
				send(sock, message, strlen(message), 0);														// ������ ���� ���� ��û
			}

			while(textStorage.size() >= QUEUESIZE - 1)															// ������ ������ [�ܼ� â ����]
			{
				free(textStorage.front());
				textStorage.pop_front();
			}
			textStorage.push_back(tmpStr);

			pthread_mutex_lock(&consoleLock);
			system("cls");

			gotoxy(0, 1);
			rendIter = textStorage.rend();
			for(revIter = textStorage.rbegin() ; revIter != rendIter ; revIter++)
				printf("%s", *revIter);

			gotoxy(0, QUEUESIZE);
			printf("����:/quit �Է� or Ctrl-C, ��������:/push ���ϸ�, ���Ͽ�û:/pull ���ϸ�\n");
			printf("USER ID : %s\n", name);
			printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : ");

			gotoxy(0, 0);
			printf("%s>", name);
			pthread_mutex_unlock(&consoleLock);
		}
		else																									// ���� ��츦 �����ϰ�� �Ϲ� �޽����ν� ������ ����
		{
			tmpStr = (char *)malloc(sizeof(char) * (strlen(name) + strlen(message) + 2));
			sprintf(total_message, "%s>%s", name, message);														// �޽��� �տ� Ŭ���̾�Ʈ �̸��� �ٿ���

			strcpy(tmpStr, total_message);																		// �Էµ� �޽��� ����

			while(textStorage.size() >= QUEUESIZE - 1)															// ������ ������ [�ܼ� â ����] 
			{
				free(textStorage.front());
				textStorage.pop_front();
			}
			textStorage.push_back(tmpStr);

			pthread_mutex_lock(&consoleLock);
			system("cls");

			gotoxy(0, 1);
			rendIter = textStorage.rend();
			for(revIter = textStorage.rbegin() ; revIter != rendIter ; revIter++)
				printf("%s", *revIter);

			gotoxy(0, QUEUESIZE);
			printf("����:/quit �Է� or Ctrl-C, ��������:/push ���ϸ�, ���Ͽ�û:/pull ���ϸ�\n");
			printf("USER ID : %s\n", name);
			printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : �۾� ����");

			gotoxy(0, 0);
			printf("%s>", name);
			pthread_mutex_unlock(&consoleLock);

			pthread_mutex_lock(&socketLock);
			send(sock, total_message, strlen(total_message), 0);
			pthread_mutex_unlock(&socketLock);
		}
	}
}

void* recv_message(void* arg)																					// ���� ������ ó�� ������ �Լ�
{
	int i, sock, progress, str_len, file_size;
	char total_message[BUFSIZE], *tmpStr, *filename;
	bool result;

	FILE *fp;
	COORD savedPos;
	deque<char *>::reverse_iterator revIter, rendIter;

	filename = NULL;
	fp = NULL;
	sock = (int)arg;
	while(true)
	{
		str_len = recv(sock, total_message, BUFSIZE - 1, 0);													// ���ۿ� �������� ���۵� �����Ͱ� �߰��Ǹ� ��ȯ, �׷��� ���� ��� Block

		if(str_len <= 0)																						// �������� �����Ͱ� �ƴ� ��� ���α׷� ���� - �ַ� ������ ����Ǵ� ��쿡 ���� ��
		{
			system("cls");

			while(textStorage.size())
			{
				free(textStorage.front());
				textStorage.pop_front();
			}

			closesocket(sock);
			WSACleanup();

			exit(1);
		}
		else if(strcmp(total_message, END_MSG) == 0)															// /quit\n �� ���۵� ���, �������� ������ ���� �����ϴ� ���� ���α׷��� ����
		{
			system("cls");

			while(textStorage.size())
			{
				free(textStorage.front());
				textStorage.pop_front();
			}

			closesocket(sock);
			WSACleanup();

			exit(1);
		}
		else if(strncmp(total_message, "HEAD_OF_FILE", strlen("HEAD_OF_FILE")) == 0)							// HEAD_OF_FILE �� �޽����� ���۵� ���, �������� ������ �����Ѵٴ� ���� �˸��� �޽���
		{
			if(fp != NULL)
				fclose(fp);

			if(filename != NULL && strlen(filename) < MSGSIZE)
				free(filename);

			if(bPulled)																							// ������ ��ҵ� ���� �ƴ϶��
			{
				filename = (char *)malloc(sizeof(char) * (strlen(total_message + sizeof("HEAD_OF_FILE")) + 1));	// �Բ� ���޵� ���ϸ� ������ ���� ���� �����Ҵ�
				strcpy(filename, total_message + sizeof("HEAD_OF_FILE"));										// ���ϸ� ����
				fp = fopen(filename, "w");																		// ���� ������ ���� ����
			}
		}
		else if(strncmp(total_message, "FILE_SIZE", strlen("FILE_SIZE")) == 0)									// FILE_SIZE�� �޽����� ���۵� ���, ���� ũ�⸦ �ǹ�
		{
			if(bPulled)																							// ������ ��ҵ� ���� �ƴ϶��
			{
				progress = 0;																					// ���� �� ������ ũ�� �ʱ�ȭ
				file_size = atoi(total_message + sizeof("FILE_SIZE"));											// ���� ũ�� int������ ��ȯ

				tmpStr = (char *)malloc(sizeof(char) * BUFSIZE + 1);											// ������ ������ [�ܼ� â ����] 
				for(i = 0 ; i < BUFSIZE ; i++)
					tmpStr[i] = ' ';
				tmpStr[BUFSIZE] = NULL;

				pthread_mutex_lock(&consoleLock);
				savedPos = getXY();

				gotoxy(0, QUEUESIZE + 2);
				printf("%s", tmpStr);

				gotoxy(0, QUEUESIZE + 2);
				printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : [download] - %d%% ����", (file_size ? 100 * progress / file_size : 100));	// �ٿ�ε� ���� ����

				gotoxy(savedPos.X, savedPos.Y);
				pthread_mutex_unlock(&consoleLock);

				free(tmpStr);
			}
		}
		else if(strncmp(total_message, "FILE", strlen("FILE")) == 0)											// FILE�� �޽����� ���۵� ���, ���� �����͸� �ǹ�
		{
			if(bPulled && fp != NULL)																			// ���� ������ ��ҵ��� �ʾҰ�, �غ��� ������ ��ȿ�� ���
			{
				tmpStr = (char *)malloc(sizeof(char) * BUFSIZE + 1);
				for(i = 0 ; i < BUFSIZE ; i++)
					tmpStr[i] = ' ';
				tmpStr[BUFSIZE] = NULL;

				fputs(total_message + sizeof("FILE"), fp);														// ���� �� ������ ���Ͽ� ����
				progress += strlen(total_message + sizeof("FILE"));												// ���� �� ������ ũ�⿡ ���۵� ������ ��ŭ �߰�

				pthread_mutex_lock(&consoleLock);																// ������ ������ [�ܼ� â ����] 
				savedPos = getXY();

				gotoxy(0, QUEUESIZE + 2);
				printf("%s", tmpStr);

				gotoxy(0, QUEUESIZE + 2);
				printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : [download] - %d%% ����", (file_size ? 100 * progress / file_size : 100));	// �ٿ�ε� ���� ����

				gotoxy(savedPos.X, savedPos.Y);
				pthread_mutex_unlock(&consoleLock);

				free(tmpStr);
			}
		}
		else if(strncmp(total_message, "END_OF_FILE", strlen("END_OF_FILE")) == 0)								// END_OF_FILE�� �޽����� ���۵� ���, ���� ������ �� ������ �ǹ�
		{
			result = bPulled && (total_message[sizeof("END_OF_FILE")] == 'O');									// ��Ҵ� �ȵƴ��� && �������� �ƴ���

			tmpStr = (char *)malloc(sizeof(char) * BUFSIZE);
			if(result)																							// ���� ����
				snprintf(tmpStr, BUFSIZE - 1, "���� ���ſ� �����߽��ϴ� - %s\n", filename);
			else if(bPulled)																					// ��Ҵ� ���� �ʾ����Ƿ� ���� ����, �ַ� ������ ���� ���� ��û�� �߻�
				snprintf(tmpStr, BUFSIZE - 1, "�������� �ʴ� ���� ����! - %s\n", filename);
			else																								// ����ڰ� ������ ����� ���
				snprintf(tmpStr, BUFSIZE - 1, "���� ������ ��ҵƽ��ϴ� - %s\n", filename);

			while(textStorage.size() >= QUEUESIZE - 1)															// ������ ������ [�ܼ� â ����] 
			{
				free(textStorage.front());
				textStorage.pop_front();
			}
			textStorage.push_back(tmpStr);

			tmpStr = (char *)malloc(sizeof(char) * (BUFSIZE / 2 + 1));
			for(int i = 0 ; i < BUFSIZE / 2 ; i++)
				tmpStr[i] = ' ';
			tmpStr[BUFSIZE / 2] = NULL;

			pthread_mutex_lock(&consoleLock);

			savedPos = getXY();
			gotoxy(0, 1);
			for(i = 1 ; i < QUEUESIZE ; i++)
				printf("%s", tmpStr);

			gotoxy(0, 1);
			rendIter = textStorage.rend();
			for(revIter = textStorage.rbegin() ; revIter != rendIter ; revIter++)
				printf("%s", *revIter);

			gotoxy(0, QUEUESIZE + 2);
			printf("%s", tmpStr);

			gotoxy(0, QUEUESIZE + 2);
			if(result)
				printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : �ٿ�ε� ���� - %s", filename);						// �ٿ�ε� ���� �������� ����
			else
				printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : �ٿ�ε� ���� - %s", filename);						// �ٿ�ε� ���� ���з� ����

			gotoxy(savedPos.X, savedPos.Y);
			pthread_mutex_unlock(&consoleLock);

			if(fp != NULL)
				fclose(fp);																						// ���� �ݱ�

			if(filename != NULL && strlen(filename) < MSGSIZE)
			{
				free(filename);																					// ���ϸ� �Ҵ� �� �� ���� ��ȯ
				filename = NULL;																				// ���ϸ� �ʱ�ȭ
			}

			free(tmpStr);

			bPulled = false;																					// �̼��� ���·� ����
		}
		else if(strncmp(total_message, name, strlen(name)))														// �� �޽����� ������ ���, �Ϲ� �޽����� ����
		{
			total_message[str_len] = NULL;
			tmpStr = (char *)malloc(sizeof(char) * (strlen(total_message) + 1));								// �� �޽����� ���� ���� �Ҵ�
			strcpy(tmpStr, total_message);																		// �޽��� �����Կ� �޽��� �߰�
			while(textStorage.size() >= QUEUESIZE - 1)															// ������ ������ [�ܼ� â ����]
			{
				free(textStorage.front());
				textStorage.pop_front();
			}
			textStorage.push_back(tmpStr);

			tmpStr = (char *)malloc(sizeof(char) * (BUFSIZE / 2 + 1));											// �ܼ� â ������ ����� ���� �������� �� �迭 �Ҵ�
			for(int i = 0 ; i < BUFSIZE / 2 ; i++)
				tmpStr[i] = ' ';																				// �ܼ� â ������ ����� �뵵�� ����ϱ� ���� �������� ä��
			tmpStr[BUFSIZE / 2] = NULL;

			pthread_mutex_lock(&consoleLock);

			savedPos = getXY();
			gotoxy(0, 1);
			for(i = 1 ; i < QUEUESIZE ; i++)
				printf("%s", tmpStr);

			gotoxy(0, 1);
			rendIter = textStorage.rend();
			for(revIter = textStorage.rbegin() ; revIter != rendIter ; revIter++)
				printf("%s", *revIter);

			gotoxy(savedPos.X, savedPos.Y);
			pthread_mutex_unlock(&consoleLock);
			free(tmpStr);
		}
	}

	while(textStorage.size())																					// ������ ���� ���� �޽��� ������ �޸� ����
	{
		free(textStorage.front());
		textStorage.pop_front();
	}
}

void *send_file(void* arg)																						// ���� ���� ������
{
	int i, progress, str_len, file_size;
	char file[BUFSIZE], tmp[BUFSIZE], *tmpStr, *filename;
	COORD savedPos;
	deque<char *>::reverse_iterator revIter, rendIter;

	filename = (char *)(arg);																					// ������ ���� �� ���޵� ���ϸ�
	FILE *fp = fopen(filename, "r");																			// �б� ���� ���� ����

	if(fp != NULL)																								// ������ ���������� ���� ���
	{
		fseek(fp, 0, SEEK_END);																					// ���� �д� ��ġ�� ������ �̵�
		file_size = ftell(fp);																					// ���� �д� ��ġ�� ��ȯ = (���� ũ��)
		fseek(fp, 0, SEEK_SET);																					// ���� �д� ��ġ�� ó������ �̵�
		progress = 0;																							// ������ ������ �� 0���� �ʱ�ȭ

		tmpStr = (char *)malloc(sizeof(char) * BUFSIZE + 1);													// �ܼ� â ������ ����� ���� �������� �� �迭 �Ҵ�
		for(i = 0 ; i < BUFSIZE ; i++)
			tmpStr[i] = ' ';																					// �ܼ� â ������ ����� �뵵�� ����ϱ� ���� �������� ä��
		tmpStr[BUFSIZE] = NULL;

		snprintf(tmp, BUFSIZE - 1, "HEAD_OF_FILE_%s", filename);												// HEAD_OF_FILE_���ϸ�
		pthread_mutex_lock(&socketLock);																		// ���ÿ� �����ϴ� ���� ���� ���� Lock
		send(sock, tmp, strlen(tmp), 0);																		// HEAD_OF_FILE_���ϸ� ����
		pthread_mutex_unlock(&socketLock);																		// ���� �Ϸ� Lock ����

		pthread_mutex_lock(&consoleLock);																		// ������ ������ [�ܼ� â ����]
		savedPos = getXY();

		gotoxy(0, QUEUESIZE + 2);
		printf("%s", tmpStr);

		gotoxy(0, QUEUESIZE + 2);
		printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : [upload] - %d%% ����", (file_size ? 100 * progress / file_size : 100));	// �ٿ�ε� ���� ����

		gotoxy(savedPos.X, savedPos.Y);
		pthread_mutex_unlock(&consoleLock);

		while((str_len = fread(tmp, 1, MSGSIZE / 2, fp)) > 0)													// ������ ������ �о� ����
		{
			tmp[str_len] = NULL;
			sprintf(file, "FILE_");
			strcat(file, tmp);

			pthread_mutex_lock(&socketLock);																	// ���ÿ� ���� �ϴ� ���� ���� ���� Lock
			Sleep(1000);																						// �����ϰ� ���� �����尡 CPU �ڿ��� �����ϴ� ���� ����
			send(sock, file, strlen(file), 0);																	// FILE_���� ���� ���� ����
			pthread_mutex_unlock(&socketLock);																	// ���� �Ϸ� Lock ����

			progress += str_len;																				// ���� �� ������ ũ�⿡ ���۵� ������ ��ŭ �߰�

			pthread_mutex_lock(&consoleLock);																	// ������ ������ [�ܼ� â ����]
			savedPos = getXY();

			gotoxy(0, QUEUESIZE + 2);
			printf("%s", tmpStr);

			gotoxy(0, QUEUESIZE + 2);
			printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : [upload] - %d%% ����", (file_size ? 100 * progress / file_size : 100)); // �ٿ�ε� ���� ����

			gotoxy(savedPos.X, savedPos.Y);
			pthread_mutex_unlock(&consoleLock);
		}

		fclose(fp);																								// ���� �Ϸ�, ���� �ݱ�

		pthread_mutex_lock(&socketLock);																		// ���ÿ� ���� �ϴ� ���� ���� ���� Lock
		send(sock, "END_OF_FILE", strlen("END_OF_FILE"), 0);													// ���� �����Ͱ� �������� �뺸
		pthread_mutex_unlock(&socketLock);																		// ���� �Ϸ� Lock ����

		pthread_mutex_lock(&consoleLock);																		// ������ ������ [�ܼ� â ����]
		savedPos = getXY();

		gotoxy(0, QUEUESIZE + 2);
		printf("%s", tmpStr);

		gotoxy(0, QUEUESIZE + 2);
		printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : ���ε� ���ۼ��� - %s", filename);								// �ٿ�ε� ���� ����

		gotoxy(savedPos.X, savedPos.Y);
		pthread_mutex_unlock(&consoleLock);

		free(tmpStr);
	}
	else																										// ���� ���� �Ұ���, �ַ� ���� ������ ������ �� �� �߻�
	{
		tmpStr = (char *)malloc(sizeof(char) * BUFSIZE);
		snprintf(tmpStr, BUFSIZE - 1, "������ �� �� �����ϴ� - %s\n", filename);								// ��� ���� �޽��� �߰�
		while(textStorage.size() >= QUEUESIZE - 1)																// ������ ������ [�ܼ� â ����]
		{
			free(textStorage.front());
			textStorage.pop_front();
		}
		textStorage.push_back(tmpStr);

		tmpStr = (char *)malloc(sizeof(char) * (BUFSIZE / 2 + 1));
		for(int i = 0 ; i < BUFSIZE / 2 ; i++)
			tmpStr[i] = ' ';
		tmpStr[BUFSIZE / 2] = NULL;

		pthread_mutex_lock(&consoleLock);

		savedPos = getXY();
		gotoxy(0, 1);
		for(i = 1 ; i < QUEUESIZE ; i++)
			printf("%s", tmpStr);

		gotoxy(0, 1);
		rendIter = textStorage.rend();
		for(revIter = textStorage.rbegin() ; revIter != rendIter ; revIter++)
			printf("%s", *revIter);

		gotoxy(0, QUEUESIZE + 2);
		printf("%s", tmpStr);

		gotoxy(0, QUEUESIZE + 2);
		printf("�ֱ� ���� �ٿ�ε�/���ε� ���� : ���ε� ���۽��� - %s", filename);								// �ٿ�ε� ���� ����

		gotoxy(savedPos.X, savedPos.Y);
		pthread_mutex_unlock(&consoleLock);

		free(tmpStr);
	}

	bPushed = false;																							// ���� �̼۽� ���·� ����

	return 0;																									// ���� ���� ������ ����
}