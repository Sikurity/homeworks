/**
 *  이름 : 이영식(3학년)
 *  힉번 : 2011004040
 *  전공 : 컴퓨터전공
 *  과제 : 컴퓨터 네트워크 소켓 프로그래밍 - 서버(Server)
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
#define MAX_CLIENT_NUM	10								/* 최대 클라이언트 수 10개 */
#define NAMESIZE		10								/* 클라이언트 ID 최대 길이, client.cpp 에서만 사용됨 */
#define MSGSIZE			500								/* 메시지 최대 길이 */
#define BUFSIZE			(NAMESIZE + 1 + MSGSIZE + 1)	/* 버퍼 크기 */ 
#define REQ_ID			"/getid\n"						/* ID 요청 명령어 */
#define END_MSG			"/quit\n"						/* 종료 명령어, 서버 <-> 클라이언트 양방향에서 모두 사용 됨 */
#define PULL_FILE		"/pull "						/* 파일 요청 명령어 */

using namespace std;

void *client_connection(void *arg);					// 클라이언트와 연결 되면서 실행될 통신 스레드 함수
void *send_file(void *arg);							// 클라이언트의 파일 요청시 실행될 전송 스레드 함수


void error_handling(char *message)					// 에러 발생 시 실행되는 함수
{
	fprintf(stderr, "%s\n", message);				// 에러 출력
	exit(1);										// 프로그램 종료
}

int client_num;										// 서버에 연결 된 클라이언트 수
int server_sock;									// 서버 소켓
int client_socks[MAX_CLIENT_NUM];					// 클라이언트 소켓 배열
bool bInTransit[MAX_CLIENT_NUM];					// 클라

map<int, FILE *> fpStorage;							// 각 클라이언트 별 파일접근자 저장소
pthread_mutex_t socketLock;							// 클라이언트 배열(client_socks)의 경쟁 상태 진입을 방지하기 위한 mutex
pthread_mutex_t senderLock;							// 여러 스레드에서 동시에 send사용 경쟁 상태 진입을 방지하기 위한 mutex

void sigint_handler(int signal_no)					// Ctrl-C 입력시 처리할 함수
{
	int i;

	printf("%d : Ctrl-C Pressed, 서버를 종료합니다\n", signal_no);

	pthread_mutex_lock(&socketLock);						// 소켓 저장 배열 client_socks의 경쟁 상태를 막기 위해 Lock

	for(i = 0 ; i < client_num ; i++)						// 모든 소켓에 대하여
	{
		pthread_mutex_lock(&senderLock);					//
		send(client_socks[i], END_MSG, sizeof(END_MSG), 0);	// 서버 종료 통보 완료
		pthread_mutex_unlock(&senderLock);					//
	}

	for( i = 0 ; i < client_num ; i++ )						// 모든 소켓에 대하여
		closesocket(client_socks[i]);
	pthread_mutex_unlock(&socketLock);						// 클라이언트 소켓 닫기 완료, Lock 해제

	closesocket(server_sock);								// 서버 소켓 닫음
	WSACleanup();											// Winsock 메모리 해제

															// 프로그램 종료
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

	if( pthread_mutex_init(&socketLock, NULL) || pthread_mutex_init(&senderLock, NULL) )	// 뮤텍스 초기화 실패 시, 에러 출력 후 종료
		error_handling("mutex init error");													//

	// Initialize Winsock
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);											// Winsock 초기화
	if(result != 0)																			// 실패 시 에러 출력 후 종료
	{																						//
		printf("WSAStartup failed with error: %d\n", result);								//
		return 1;																			//
	}																						//

	server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);								// 서버 소켓 생성
	if(server_sock == -1)																	// 실패 시 에러 출력 후 종료
	{																						//
		WSACleanup();																		//
		error_handling("socket() error");													//
	}																						//

	memset(&server_addr, 0, sizeof(server_addr));											// 소켓 설정 초기화
	server_addr.sin_family = AF_INET;														// IPv4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);										// 로컬 IP
	server_addr.sin_port = htons((argc < 2) ? DEFAULT_PORT : atoi(argv[1]));				// 지정해주지 않을 시 8000

	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)		// 소켓 설정 적용
	{																						// 실패 시 에러 출력 후 종료
		closesocket(server_sock);															// 
		WSACleanup();																		//
		error_handling("bind() error");														//
	}																						//

	if(listen(server_sock, SOMAXCONN) == -1)												// 소켓을 통해 접속 요청 확인
	{																						// 실패 시 에러 출력 후 종료
		closesocket(server_sock);															//
		WSACleanup();																		//
		error_handling("listen() error");													//
	}																						//

	signal(SIGINT, sigint_handler);															// 인터럽트 시 위에서 선언한 sigint_handler 함수가 실행되게 함
	client_num = 0;																			// 초기 클라이언트 개수 0으로 초기화
	while( true )																					// accept 반복
	{
		if(client_num < MAX_CLIENT_NUM)																// 최대 클라이언트 허용 수 이내로만 accept
		{
			printf("연결 대기 중... 종료하려면 Ctrl-C를 눌러주세요\n");
			client_addr_size = sizeof(client_addr);
			client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);	// accept 실행(Blocking)
			if(client_sock == -1)																	// accept 오류 시 반복문 종료 = 프로그램 종료
				break;

			pthread_mutex_lock(&socketLock);														// 소켓 저장 배열 client_socks의 경쟁 상태를 막기 위해 Lock
			client_socks[client_num++] = client_sock;												// 클라이언트 소켓 저장, 전체 클라이언트 개수 값 증가
			pthread_mutex_unlock(&socketLock);														// 저장 및 개수 증가 완료, Lock 해제

			pthread_create(&thread, NULL, client_connection, (void*)client_sock);					// 클라이언트와의 통신을 담당할 스레드 생성(함수 - client_connection)
			printf("IP : %s:%d - ID : %d와 연결되었습니다\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, client_sock);
		}
		else
		{
			printf("소켓 개수 제한... 종료하려면 Ctrl-C를 눌러주세요\n");							// 클라이언트 수가 최대 개수가 넘어가면 출력
			Sleep(5000);																			// 5초 동안 대기 시켜, 다른 스레드에 실행 기회를 더 부여
		}
	}

	pthread_mutex_lock(&socketLock);						// 서버 종료 통보를 위해 Lock

	for(i = 0 ; i < client_num ; i++)						// 모든 소켓에 대하여	
	{
		pthread_mutex_lock(&senderLock);					// 소켓 저장 배열 client_socks의 경쟁 상태를 막기 위해 Lock
		send(client_socks[i], END_MSG, sizeof(END_MSG), 0);	// 클라이언트 마다 서버 종료 통보
		pthread_mutex_unlock(&senderLock);					// 서버 종료 통보 완료, Lock 해제
	}

	for(i = 0 ; i < client_num ; i++)						// 모든 소켓에 대하여
		closesocket(client_socks[i]);						// 소켓 닫기

	pthread_mutex_unlock(&socketLock);						// 소켓 연결 해제 완료, Lock 해제

	closesocket(server_sock);								// 서버 소켓 닫기
	WSACleanup();											// Winsock 메모리 해제

	printf("메모리 해제 완료, 3초 후 서버가 종료됩니다\n");
	Sleep(3000);

	return 0;												// 서버 프로그램 종료
}

void *client_connection(void *arg)
{
	int i, data[2], client_sock, str_len;
	char message[BUFSIZE], *tmpStr, *filename;
	pthread_t send_thread;

	client_sock = (int)arg;
	filename = tmpStr = NULL;
	str_len = 0;
	while( true )																					// 프로그램 종료 전까지 recv 반복
	{
		if( (str_len = recv(client_sock, message, sizeof(message), 0)) > 0 )						// 버퍼 값에 변화가 있으면 값 반환, 없다면 Blocking
		{
			message[str_len] = NULL;
			if( strncmp(message, END_MSG, sizeof(END_MSG)) == 0 )									// END_MSG = /quit\n가 전달 된 경우, 반복문 종료 = 스레드 종료
				break;
			else if(strncmp(message, REQ_ID, sizeof(REQ_ID)) == 0)									// REQ_ID = /getid\n가 전달 된 경우, 클라이언트 소켓 값 전송
			{
				_itoa(client_sock, message, 10);													// 클라이언트 소켓 값 FROM (int) TO (char *)
				pthread_mutex_lock(&senderLock);													// 서로 다른 스레드에서 동시에 Send 하는 것을 막기 위해 Lock
				send(client_sock, message, strlen(message), 0);										// 클라이언트 소켓 값 전송
				pthread_mutex_unlock(&senderLock);													// 전송 끝, Lock 해제
			}
			else if( strncmp(message, PULL_FILE, strlen(PULL_FILE)) == 0 )							// PULL_FILE = /pull ~가 전달 된 경우, 파일 전송 스레드 생성 
			{
				tmpStr = (char *)malloc(sizeof(char) * (strlen(message + strlen(PULL_FILE)) + 1));	// 동적할당
				sprintf(tmpStr, "%s", message + strlen(PULL_FILE));									// tmpStr에 파일명 저장
				tmpStr[strlen(tmpStr) - 1] = NULL;													// 문자열 NULL 처리

				data[0] = client_sock;																// 클라이언트 소켓 값 전달
				data[1] = (int)tmpStr;																// 파일명 전달
				pthread_create(&send_thread, NULL, send_file, (void *)data);						// 파일 전송 스레드 생성
			}
			else if( strncmp(message, "HEAD_OF_FILE", strlen("HEAD_OF_FILE")) == 0 )				// HEAD_OF_FILE - 클라이언트가 서버로 파일을 보냄을 통보하는 것을 뜻함
			{
				if(fpStorage[client_sock] != NULL)													// 파일접근자가 초기화 되어있지
					fclose(fpStorage[client_sock]);													// 파일접근자를 닫는다

				if( filename != NULL && strlen(filename) < MSGSIZE )								// 파일 이름으로 할당된 동적 메모리가 이미 있다면
					free(filename);																	// 동적 메모리를 해제 해준다

				filename = (char *)malloc(sizeof(char) * (strlen(message + sizeof("HEAD_OF_FILE")) + 1));
				sprintf(filename, "%s", message + sizeof("HEAD_OF_FILE"));							// 메모리를 동적 할당하여 파일명을 저장 시킴

				fpStorage[client_sock] = fopen(filename, "w");										// 파일을 쓰기용으로 Open
			}
			else if( strncmp(message, "FILE", strlen("FILE")) == 0 )								// FILE - 전송된 데이터가 파일 데이터임을 뜻함
			{
				if(fpStorage[client_sock] != NULL)
					fputs(message + sizeof("FILE"), fpStorage[client_sock]);						// 전달된 데이터를 파일에 쓴다
			}
			else if( strncmp(message, "END_OF_FILE", strlen("END_OF_FILE")) == 0 )					// END_OF_FILE - 파일 전송이 끝났음을 뜻함
			{
				if(fpStorage[client_sock] != NULL)
				{
					fclose(fpStorage[client_sock]);													// 파일 닫음
					fpStorage[client_sock] = NULL;													// 클라이언트 파일접근자 초기화
				}
				else
					continue;

				if(filename != NULL && strlen(filename) < MSGSIZE)
				{
					sprintf(message, "%s - 서버에 저장 됨\n", filename);							// 클라이언트 들에 전달할 메시지 생성

					free(filename);																	// 파일명에 할당된 메모리 해제
					filename = NULL;																// 파일명 초기화
				}
				else
					continue;

				pthread_mutex_lock(&socketLock);						// 다른 소켓이 제거될 때, 소켓 배열 내에서의 위치가 변경 될 수 있으므로 Lock

				for(i = 0; i < client_num; i++)							// 모든 클라이언트 소켓에 대하여
				{
					pthread_mutex_lock(&senderLock);					// 서로 다른 스레드에서 동시에 Send 하는 것을 막기 위해 Lock
					send(client_socks[i], message, strlen(message), 0);	// 서버에 파일이 저장됐음을 클라이언트 들에게 통보
					pthread_mutex_unlock(&senderLock);					// 통보 완료 Lock 해제
				}

				pthread_mutex_unlock(&socketLock);						// 데이터 전송 완료, Lock 해제
			}
			else
			{
				pthread_mutex_lock(&socketLock);						// 다른 소켓이 제거될 때, 소켓 배열 내에서의 위치가 변경 될 수 있으므로 Lock

				for(i = 0; i < client_num; i++)							// 모든 클라이언트 소켓에 대하여
				{
					pthread_mutex_lock(&senderLock);					// 서로 다른 스레드에서 동시에 Send 하는 것을 막기 위해 Lock
					send(client_socks[i], message, strlen(message), 0);	// 들어온 메시지를 그대로 클라이언트 들에게 뿌려줌
					pthread_mutex_unlock(&senderLock);					// 통보 완료 Lock 해제
				}

				pthread_mutex_unlock(&socketLock);						// 데이터 전송 완료, Lock 해제
			}
		}
		else
			break;
	}

	pthread_mutex_lock(&socketLock);									// 제거할 소켓의 빈자리를 나머지 소켓들을 당겨서 채움, 변경 과정에서 문제 발생을 막기 위해 Lock 설정
	for(i = 0; i < client_num ; i++)									// 0번 소켓 ~ N - 1 소켓
	{																	//
		if(client_sock == client_socks[i])								// 전체 소켓 중 삭제할 소켓인 경우
		{																//
			while(i < client_num - 1)									// 해당 소켓 이후의 모든 소켓들에 대하여
			{															//
				client_socks[i] = client_socks[i + 1];					// 소켓을 앞 저장공간으로 이동, 저장시킨다
				i++;													// 다음 소켓으로 이동
			}															//
			break;														// 탐색 종료
		}																//
	}																	//
	client_num--;														// 소켓 개수 감소
	pthread_mutex_unlock(&socketLock);									// 소켓 공백 제거 완료, Lock 해제

	printf("ID : %d와 연결이 종료되었습니다(종료 : Ctrl-C)\n", client_sock);

	closesocket(client_sock);											// 클라이언트 소켓 닫음

	return 0;
}

void *send_file(void *arg)												// 클라이언트의 요청으로 서버에서 클라이언트로 파일을 보내는 스레드 함수
{
	int sock, str_len, file_size;
	char file[BUFSIZE] = {0, }, tmp[BUFSIZE] = {0,}, *filename;

	sock = ((int *)arg)[0];
	filename = (char *)(((int *)arg)[1]);

	snprintf(tmp, BUFSIZE - 1, "HEAD_OF_FILE_%s", filename);			// HEAD_OF_FILE_파일명
	pthread_mutex_lock(&senderLock);									// 서로 다른 스레드에서 동시에 Send 하는 것을 막기 위해 Lock
	send(sock, tmp, strlen(tmp) + 1, 0);								// HEAD_OF_FILE_파일명 전송
	pthread_mutex_unlock(&senderLock);									// 전송 완료 Lock 해제

	FILE *fp = fopen(filename, "r");

	if(fp != NULL)
	{
		fseek(fp, 0, SEEK_END);											// 파일 읽는 위치를 끝으로 이동
		file_size = ftell(fp);											// 파일 읽는 위치를 반환 = (파일 크기)
		fseek(fp, 0, SEEK_SET);											// 파일 읽는 위치를 처음으로 이동

		if(file_size > 0)												// 파일 크기가 0보다 큰 경우에만 진행
		{
			memset(file, 0, sizeof(file));
			memset(tmp, 0, sizeof(file));

			snprintf(tmp, BUFSIZE - 1, "FILE_SIZE_%d", file_size);		// FILE_SIZE_파일크기
			pthread_mutex_lock(&senderLock);							// 서로 다른 스레드에서 동시에 Send 하는 것을 막기 위해 Lock
			send(sock, tmp, strlen(tmp) + 1, 0);						// FILE_SIZE_파일크기 전송
			pthread_mutex_unlock(&senderLock);							// 전송 완료 Lock 해제

			memset(file, 0, sizeof(file));
			memset(tmp, 0, sizeof(file));
			while((str_len = fread(tmp, 1, MSGSIZE / 2, fp)) > 0)		// 한글 포함 유니코드는 하나 당 2 bytes 읽혀 (MSGSIZE / 2) 만큼 만 읽게 함
			{
				tmp[str_len] = NULL;
				sprintf(file, "FILE_");									// FILE_
				strcat(file, tmp);										// FILE_실제 파일 내용

				pthread_mutex_lock(&senderLock);						// 서로 다른 스레드에서 동시에 Send 하는 것을 막기 위해 Lock
				Sleep(1000);											// 과도하게 파일 스레드가 CPU 자원을 점유하는 것을 방지
				send(sock, file, strlen(file) + 1, 0);					// FILE_실제 파일 내용 전송
				pthread_mutex_unlock(&senderLock);						// 전송 완료 Lock 해제

				memset(file, 0, sizeof(file));
				memset(tmp, 0, sizeof(file));
			}

			fclose(fp);													// 파일 닫음
		}

		pthread_mutex_lock(&senderLock);								// 서로 다른 스레드에서 동시에 Send 하는 것을 막기 위해 Lock
		send(sock, "END_OF_FILE_O", strlen("END_OF_FILE_O") + 1, 0);	// END_OF_FILE_O 전송 - 정상 전송
		pthread_mutex_unlock(&senderLock);								// 전송 완료 Lock 해제
	}
	else
	{
		pthread_mutex_lock(&senderLock);								// 서로 다른 스레드에서 동시에 Send 하는 것을 막기 위해 Lock
		send(sock, "END_OF_FILE_X", strlen("END_OF_FILE_X") + 1, 0);	// END_OF_FILE_O 전송 - 전송 오류
		pthread_mutex_unlock(&senderLock);								// 전송 완료 Lock 해제
	}

	free(filename);														// 파일명 저장에 할당된 동적 메모리 해제

	return 0;															// 파일 전송 스레드 종료
}