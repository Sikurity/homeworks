/**
*  이름 : 이영식(3학년)
*  힉번 : 2011004040
*  전공 : 컴퓨터전공
*  과제 : 컴퓨터 네트워크 소켓 프로그래밍 - 클라이언트
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

#define DEFAULT_PORT		8000							/* 기본 연결 서버 포트 */
#define DEFAULT_SERVER_IP	"127.0.0.1"						/* 기본 연결 서버 IP */
#define QUEUESIZE			20								/* 최대 메시지 보관 개수 20개 */
#define NAMESIZE			10								/* 클라이언트 ID 최대 길이, client.cpp 에서만 사용됨 */
#define MSGSIZE				500								/* 메시지 최대 길이 */
#define BUFSIZE				(NAMESIZE + 1 + MSGSIZE + 1)	/* 버퍼 크기 */ 
#define REQ_ID				"/getid\n"						/* ID 요청 명령어 */
#define END_MSG				"/quit\n"						/* 종료 명령어, 서버 <-> 클라이언트 양방향에서 모두 사용 됨 */
#define PULL				"/pull "						/* 파일 요청 명령어 */
#define PUSH				"/push"							/* 파일 전송 명령어 */
#define STOP				"/stop"							/* 파일 요청 취소 명령어 */

using namespace std;

void *send_message(void* arg);						// 서버한테 메시지 전송 스레드
void *recv_message(void* arg);						// 서버에서 메시지 수신 스레드
void *send_file(void* arg);							// 파일을 서버에 전송하는 스레드

void error_handling(char *message)					// 에러 발생 시 실행되는 함수
{
	fprintf(stderr, "%s\n", message);				// 에러 출력
	exit(1);										// 프로그램 종료
}

void gotoxy(int x, int y)							// 콘솔 커서 위치 변경 함수
{
	COORD Cur;
	Cur.X = x;
	Cur.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);
}

COORD getXY()										// 현재 콘솔 커서 위치 반환 함수
{
	COORD Cur;
	CONSOLE_SCREEN_BUFFER_INFO a;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &a);
	Cur.X = a.dwCursorPosition.X;
	Cur.Y = a.dwCursorPosition.Y;

	return Cur;
}

char name[NAMESIZE + 1];							// 클라이언트 이름, 서버와 연결된 후 서버에서 겹치지 않는 이름을 받아옴
int sock;											// 서버와 연결되는 소켓

pthread_t snd_thread, rcv_thread, file_thread;		// 각각 전송, 수신, 파일전송 스레드
pthread_mutex_t consoleLock, socketLock;			// 콘솔 사용 잠금, 소켓 사용 잠금

deque<char *> textStorage;							// 송수신 메시지 내역 보관
bool bPulled, bPushed;								// 파일 수신, 전송 중인지 저장, 파일 송수신은 한 번에 한 가지만 가능하도록 하기 위한 변수

void sigint_handler(int signal_no)					// Ctrl-C, 인터럽트가 발생했을 때 실행되는 함수, 소켓이 닫히므로 결국 프로그램이 종료 됨
{
	system("cls");									// 콘솔 창 지움

	closesocket(sock);								// 소켓 닫음
	WSACleanup();									// Winsock 초기화
}

int main(int argc, char **argv)
{
	int result;																			// 여러 함수들의 실행 결과를 저장하는 변수
	sockaddr_in serv_addr;																// 인자로 입력받은 서버 IP, 포트가 입력되는 변수
	WSADATA wsaData;																	// Winsock 변수
	void *thread_result;																// 스레드 실행결과가 저장되는 변수, 테스트를 위해서 만든 변수

																						// 콘솔 버퍼 크기 => 가로 - BUFSIZE / 2 : 한글의 경우 2byte씩 읽히므로 나누기 2를 해줌, 세로 - QUEUESIZE + 5 " 입력줄(1) + 메시지스택(20) + 상태창(4)
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), {BUFSIZE / 2, QUEUESIZE + 5});
	// 콘솔 창을 크기 변경할 경우 내용이 지워지는 것을 확인하여, 아예 크기 변경을 못하도록 변경
	SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU));

	if(pthread_mutex_init(&consoleLock, NULL) || pthread_mutex_init(&socketLock, NULL))	// 뮤텍스 초기화 실패 시 프로그램 종료
	{
		system("cls");
		error_handling("mutex init error");
	}

	// Initialize Winsock
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);										// Winsock 초기화
	if(result != 0)																		// 실패 시 프로그램 종료
	{
		system("cls");
		WSACleanup();
		printf("WSAStartup failed with error: %d\n", result);
		return 1;
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);												// 소켓 생성
	if(sock == -1)																		// 실패 시 프로그램 종료
	{
		system("cls");
		WSACleanup();
		error_handling("socket() error");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;														// 서버 정보 입력
	serv_addr.sin_addr.s_addr = inet_addr((argc < 3) ? DEFAULT_SERVER_IP : argv[1]);	// 지정해주지 않을 시 127.0.0.1
	serv_addr.sin_port = htons((argc < 3) ? DEFAULT_PORT : atoi(argv[2]));				// 지정해주지 않을 시 8000

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)			// 서버와 연결, 실패 시 종료
	{
		system("cls");
		closesocket(sock);
		WSACleanup();
		error_handling("connect() error!");
	}

	send(sock, REQ_ID, sizeof(REQ_ID), 0);												// 서버에게 이름 요청(REQ_ID)
	result = recv(sock, name, NAMESIZE, 0);												// 서버한테 이름 수신 기다림
	if(result > 0)																		// 수신이 정상적으로 이뤄진 경우, 이름 설정
	{
		name[result] = NULL;
		strcat(name, "user");
	}
	else																				// 수신 과정에서 오류 발생 시 종료
	{
		system("cls");
		closesocket(sock);
		WSACleanup();
		error_handling("cannot receive id from server!");
	}

	bPushed = bPulled = false;															// 파일 전송 진행 중인지 판별 변수 초기화
	textStorage = deque<char *>();														// 메시지 송수신 내역 Deque 생성

	system("cls");																		// 콘솔 창을 비움
	gotoxy(0, QUEUESIZE);																// 커서 위치 아래로 이동
	printf("종료:/quit 입력 or Ctrl-C, 파일전송:/push 파일명, 파일요청:/pull 파일명\n");// 여러 프로그램 이용 Tip 출력
	printf("USER ID : %s\n", name);														// 클라이언트 이름 출력
	printf("최근 파일 다운로드/업로드 상태 : 작업 없음");								// 최근 파일 다운로드/업로드 상태 출력
	gotoxy(0, 0);																		// 입력 열(row)로 이동
	printf("%s>", name);																// 클라이언트 이름 출력

	signal(SIGINT, sigint_handler);														// Ctrl-C 인터럽트 발생 시 실행 할 함수 설정

	pthread_create(&snd_thread, NULL, send_message, (void*)sock);						// 사용자 입력 데이터 전송을 처리하는 스레드 생성
	pthread_create(&rcv_thread, NULL, recv_message, (void*)sock);						// 서버로 부터 데이터 수신을 처리하는 스레드 생성

	pthread_join(snd_thread, &thread_result);											// 전송 스레드가 끝날 때까지 대기
	pthread_join(rcv_thread, &thread_result);											// 수신 스레드가 끝날 때까지 대기

	closesocket(sock);																	// 소켓 닫음
	WSACleanup();																		// Winsock 초기화

	return 0;																			// 메인 스레드 종료 = 프로그램 종료
}

void* send_message(void * arg)																					// 사용자 입력 전송 스레드 함수
{
	int sock;
	char *tmpStr;
	char message[MSGSIZE] = {0,}, total_message[BUFSIZE] = {0,};

	deque<char *>::reverse_iterator revIter, rendIter;

	sock = (int)arg;
	while(true)
	{
		fgets(message, MSGSIZE, stdin);																			// 사용자 입력이 있을 때 까지 Block

		if(strncmp(message, END_MSG, sizeof(END_MSG)) == 0)														// 입력이 END_MSG로 시작하는 경우
		{
			pthread_mutex_lock(&socketLock);																	// 동시 전송을 막기 위해 소켓 Lock
			send(sock, END_MSG, sizeof(END_MSG), 0);															// 연결 종료 통보
			pthread_mutex_unlock(&socketLock);																	// 통보 완료 Lock 해제
		}
		else if(strncmp(message, STOP, strlen(STOP)) == 0)														// 입력이 /stop으로 시작하는 경우
		{
			if(bPulled)																							// 파일 수신 대기 중인 경우
			{
				bPulled = false;																				// 대기를 해제

				tmpStr = (char *)malloc(sizeof(char) * 26);														// 결과 메시지 생성
				sprintf(tmpStr, "파일 수신을 취소했습니다\n");
			}
			else
			{
				tmpStr = (char *)malloc(sizeof(char) * 32);														// 결과 메시지 생성
				sprintf(tmpStr, "수신 대기 중인 파일이 없습니다\n");
			}
			/* 아래 내용은 다음 부터 [콘솔 창 갱신]으로 축약해서 설명 */
			while(textStorage.size() >= QUEUESIZE - 1)															// 메시지 보관함이 꽉 찼는지 확인, 꽉 찬 경우 가장 오래된 메시지 탈락시킴
			{
				free(textStorage.front());
				textStorage.pop_front();
			}
			textStorage.push_back(tmpStr);																		// 메시지 보관함에 결과 메시지 추가

			pthread_mutex_lock(&consoleLock);																	// 동시에 콘솔에 접근 하는 것을 막기 위해 Lock
			system("cls");																						// 콘솔 창 지움

			gotoxy(0, 1);																						// 메시지 보관함 내용들을 출력
			rendIter = textStorage.rend();
			for(revIter = textStorage.rbegin() ; revIter != rendIter ; revIter++)
				printf("%s", *revIter);

			gotoxy(0, QUEUESIZE);
			printf("종료:/quit 입력 or Ctrl-C, 파일전송:/push 파일명, 파일요청:/pull 파일명\n");
			printf("USER ID : %s\n", name);
			printf("최근 파일 다운로드/업로드 상태 : 수신 취소");

			gotoxy(0, 0);																						// 입력 열(row)로 이동
			printf("%s>", name);																				// 클라이언트 이름 출력
			pthread_mutex_unlock(&consoleLock);																	// 콘솔 Lock 해제
		}
		else if(strncmp(message, PUSH, strlen(PUSH)) == 0)														// 메시지가 /push로 시작하는 경우
		{
			if(bPushed || bPulled)																				// 파일 송수신이 이미 진행되고 있는 경우 이를 막음
			{
				tmpStr = (char *)malloc(sizeof(char) * 62);
				sprintf(tmpStr, "파일 동시 송수신은 불가능합니다(송신취소 명령 - 아직 미구현)\n");				// 결과 메시지 생성
			}
			else
			{
				bPushed = true;																					// 파일 송신 상태로 변경
				tmpStr = (char *)malloc(sizeof(char) * 24);
				sprintf(tmpStr, "파일 전송을 준비합니다\n");													// 결과 메시지 생성
				message[strlen(message) - 1] = NULL;															// 송신하려는 파일명
				pthread_create(&file_thread, NULL, send_file, (void*)(message + strlen("/push ")));				// 파일 송신 스레드 생성, 파일명도 전달
			}

			while(textStorage.size() >= QUEUESIZE - 1)															// 위에서 설명한 [콘솔 창 갱신]
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
			printf("종료:/quit 입력 or Ctrl-C, 파일전송:/push 파일명, 파일요청:/pull 파일명\n");
			printf("USER ID : %s\n", name);
			printf("최근 파일 다운로드/업로드 상태 : ");

			gotoxy(0, 0);
			printf("%s>", name);
			pthread_mutex_unlock(&consoleLock);
		}
		else if(strncmp(message, PULL, strlen(PULL)) == 0)														// 메시지가 /pull 로 시작하는 경우
		{
			if(bPushed || bPulled)																				// 파일 송수신이 이미 진행되고 있는 경우 이를 막음
			{
				tmpStr = (char *)malloc(sizeof(char) * 62);
				sprintf(tmpStr, "파일 동시 송수신은 불가능합니다(파일 수신 취소 명령 : /stop)\n");				// 결과 메시지 생성
			}
			else
			{
				bPulled = true;																					// 파일 수신 상태로 변경
				tmpStr = (char *)malloc(sizeof(char) * 24);
				sprintf(tmpStr, "파일 수신을 요청합니다\n");													// 결과 메시지 생성
				send(sock, message, strlen(message), 0);														// 서버에 파일 전송 요청
			}

			while(textStorage.size() >= QUEUESIZE - 1)															// 위에서 설명한 [콘솔 창 갱신]
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
			printf("종료:/quit 입력 or Ctrl-C, 파일전송:/push 파일명, 파일요청:/pull 파일명\n");
			printf("USER ID : %s\n", name);
			printf("최근 파일 다운로드/업로드 상태 : ");

			gotoxy(0, 0);
			printf("%s>", name);
			pthread_mutex_unlock(&consoleLock);
		}
		else																									// 위의 경우를 제외하고는 일반 메시지로써 서버에 전송
		{
			tmpStr = (char *)malloc(sizeof(char) * (strlen(name) + strlen(message) + 2));
			sprintf(total_message, "%s>%s", name, message);														// 메시지 앞에 클라이언트 이름을 붙여줌

			strcpy(tmpStr, total_message);																		// 입력된 메시지 생성

			while(textStorage.size() >= QUEUESIZE - 1)															// 위에서 설명한 [콘솔 창 갱신] 
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
			printf("종료:/quit 입력 or Ctrl-C, 파일전송:/push 파일명, 파일요청:/pull 파일명\n");
			printf("USER ID : %s\n", name);
			printf("최근 파일 다운로드/업로드 상태 : 작업 없음");

			gotoxy(0, 0);
			printf("%s>", name);
			pthread_mutex_unlock(&consoleLock);

			pthread_mutex_lock(&socketLock);
			send(sock, total_message, strlen(total_message), 0);
			pthread_mutex_unlock(&socketLock);
		}
	}
}

void* recv_message(void* arg)																					// 수신 데이터 처리 스레드 함수
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
		str_len = recv(sock, total_message, BUFSIZE - 1, 0);													// 버퍼에 서버에서 전송된 데이터가 추가되면 반환, 그렇지 않은 경우 Block

		if(str_len <= 0)																						// 정상적인 데이터가 아닌 경우 프로그램 종료 - 주로 서버가 종료되는 경우에 실행 됨
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
		else if(strcmp(total_message, END_MSG) == 0)															// /quit\n 이 전송된 경우, 서버에서 연결을 개별 종료하는 경우로 프로그램을 종료
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
		else if(strncmp(total_message, "HEAD_OF_FILE", strlen("HEAD_OF_FILE")) == 0)							// HEAD_OF_FILE 로 메시지가 시작될 경우, 서버에서 파일을 전송한다는 것을 알리는 메시지
		{
			if(fp != NULL)
				fclose(fp);

			if(filename != NULL && strlen(filename) < MSGSIZE)
				free(filename);

			if(bPulled)																							// 수신이 취소된 것이 아니라면
			{
				filename = (char *)malloc(sizeof(char) * (strlen(total_message + sizeof("HEAD_OF_FILE")) + 1));	// 함께 전달된 파일명 저장을 위한 공간 동적할당
				strcpy(filename, total_message + sizeof("HEAD_OF_FILE"));										// 파일명 저장
				fp = fopen(filename, "w");																		// 쓰기 용으로 파일 열기
			}
		}
		else if(strncmp(total_message, "FILE_SIZE", strlen("FILE_SIZE")) == 0)									// FILE_SIZE로 메시지가 시작될 경우, 파일 크기를 의미
		{
			if(bPulled)																							// 수신이 취소된 것이 아니라면
			{
				progress = 0;																					// 전달 된 데이터 크기 초기화
				file_size = atoi(total_message + sizeof("FILE_SIZE"));											// 파일 크기 int형으로 변환

				tmpStr = (char *)malloc(sizeof(char) * BUFSIZE + 1);											// 위에서 설명한 [콘솔 창 갱신] 
				for(i = 0 ; i < BUFSIZE ; i++)
					tmpStr[i] = ' ';
				tmpStr[BUFSIZE] = NULL;

				pthread_mutex_lock(&consoleLock);
				savedPos = getXY();

				gotoxy(0, QUEUESIZE + 2);
				printf("%s", tmpStr);

				gotoxy(0, QUEUESIZE + 2);
				printf("최근 파일 다운로드/업로드 상태 : [download] - %d%% 전송", (file_size ? 100 * progress / file_size : 100));	// 다운로드 상태 갱신

				gotoxy(savedPos.X, savedPos.Y);
				pthread_mutex_unlock(&consoleLock);

				free(tmpStr);
			}
		}
		else if(strncmp(total_message, "FILE", strlen("FILE")) == 0)											// FILE로 메시지가 시작될 경우, 파일 데이터를 의미
		{
			if(bPulled && fp != NULL)																			// 파일 수신이 취소되지 않았고, 준비한 파일이 유효한 경우
			{
				tmpStr = (char *)malloc(sizeof(char) * BUFSIZE + 1);
				for(i = 0 ; i < BUFSIZE ; i++)
					tmpStr[i] = ' ';
				tmpStr[BUFSIZE] = NULL;

				fputs(total_message + sizeof("FILE"), fp);														// 전달 된 데이터 파일에 쓰기
				progress += strlen(total_message + sizeof("FILE"));												// 전달 된 데이터 크기에 전송된 데이터 만큼 추가

				pthread_mutex_lock(&consoleLock);																// 위에서 설명한 [콘솔 창 갱신] 
				savedPos = getXY();

				gotoxy(0, QUEUESIZE + 2);
				printf("%s", tmpStr);

				gotoxy(0, QUEUESIZE + 2);
				printf("최근 파일 다운로드/업로드 상태 : [download] - %d%% 전송", (file_size ? 100 * progress / file_size : 100));	// 다운로드 상태 갱신

				gotoxy(savedPos.X, savedPos.Y);
				pthread_mutex_unlock(&consoleLock);

				free(tmpStr);
			}
		}
		else if(strncmp(total_message, "END_OF_FILE", strlen("END_OF_FILE")) == 0)								// END_OF_FILE로 메시지가 시작될 경우, 보낼 파일이 더 없음을 의미
		{
			result = bPulled && (total_message[sizeof("END_OF_FILE")] == 'O');									// 취소는 안됐는지 && 정상전송 됐는지

			tmpStr = (char *)malloc(sizeof(char) * BUFSIZE);
			if(result)																							// 정상 수신
				snprintf(tmpStr, BUFSIZE - 1, "파일 수신에 성공했습니다 - %s\n", filename);
			else if(bPulled)																					// 취소는 되지 않았으므로 수신 오류, 주로 서버에 없는 파일 요청시 발생
				snprintf(tmpStr, BUFSIZE - 1, "존재하지 않는 서버 파일! - %s\n", filename);
			else																								// 사용자가 수신을 취소한 경우
				snprintf(tmpStr, BUFSIZE - 1, "파일 수신이 취소됐습니다 - %s\n", filename);

			while(textStorage.size() >= QUEUESIZE - 1)															// 위에서 설명한 [콘솔 창 갱신] 
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
				printf("최근 파일 다운로드/업로드 상태 : 다운로드 성공 - %s", filename);						// 다운로드 상태 성공으로 갱신
			else
				printf("최근 파일 다운로드/업로드 상태 : 다운로드 실패 - %s", filename);						// 다운로드 상태 실패로 갱신

			gotoxy(savedPos.X, savedPos.Y);
			pthread_mutex_unlock(&consoleLock);

			if(fp != NULL)
				fclose(fp);																						// 파일 닫기

			if(filename != NULL && strlen(filename) < MSGSIZE)
			{
				free(filename);																					// 파일명에 할당 된 힙 공간 반환
				filename = NULL;																				// 파일명 초기화
			}

			free(tmpStr);

			bPulled = false;																					// 미수신 상태로 변경
		}
		else if(strncmp(total_message, name, strlen(name)))														// 위 메시지를 제외한 경우, 일반 메시지로 간주
		{
			total_message[str_len] = NULL;
			tmpStr = (char *)malloc(sizeof(char) * (strlen(total_message) + 1));								// 새 메시지를 위한 동적 할당
			strcpy(tmpStr, total_message);																		// 메시지 보관함에 메시지 추가
			while(textStorage.size() >= QUEUESIZE - 1)															// 위에서 설명한 [콘솔 창 갱신]
			{
				free(textStorage.front());
				textStorage.pop_front();
			}
			textStorage.push_back(tmpStr);

			tmpStr = (char *)malloc(sizeof(char) * (BUFSIZE / 2 + 1));											// 콘솔 창 내용을 지우기 위한 여백으로 된 배열 할당
			for(int i = 0 ; i < BUFSIZE / 2 ; i++)
				tmpStr[i] = ' ';																				// 콘솔 창 내용을 지우는 용도로 사용하기 위해 여백으로 채움
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

	while(textStorage.size())																					// 스레드 종료 전에 메시지 보관함 메모리 해제
	{
		free(textStorage.front());
		textStorage.pop_front();
	}
}

void *send_file(void* arg)																						// 파일 전송 스레드
{
	int i, progress, str_len, file_size;
	char file[BUFSIZE], tmp[BUFSIZE], *tmpStr, *filename;
	COORD savedPos;
	deque<char *>::reverse_iterator revIter, rendIter;

	filename = (char *)(arg);																					// 스레드 생성 시 전달된 파일명
	FILE *fp = fopen(filename, "r");																			// 읽기 모드로 파일 열기

	if(fp != NULL)																								// 파일이 정상적으로 열린 경우
	{
		fseek(fp, 0, SEEK_END);																					// 파일 읽는 위치를 끝으로 이동
		file_size = ftell(fp);																					// 파일 읽는 위치를 반환 = (파일 크기)
		fseek(fp, 0, SEEK_SET);																					// 파일 읽는 위치를 처음으로 이동
		progress = 0;																							// 전송한 데이터 양 0으로 초기화

		tmpStr = (char *)malloc(sizeof(char) * BUFSIZE + 1);													// 콘솔 창 내용을 지우기 위한 여백으로 된 배열 할당
		for(i = 0 ; i < BUFSIZE ; i++)
			tmpStr[i] = ' ';																					// 콘솔 창 내용을 지우는 용도로 사용하기 위해 여백으로 채움
		tmpStr[BUFSIZE] = NULL;

		snprintf(tmp, BUFSIZE - 1, "HEAD_OF_FILE_%s", filename);												// HEAD_OF_FILE_파일명
		pthread_mutex_lock(&socketLock);																		// 동시에 전송하는 것을 막기 위해 Lock
		send(sock, tmp, strlen(tmp), 0);																		// HEAD_OF_FILE_파일명 전송
		pthread_mutex_unlock(&socketLock);																		// 전송 완료 Lock 해제

		pthread_mutex_lock(&consoleLock);																		// 위에서 설명한 [콘솔 창 갱신]
		savedPos = getXY();

		gotoxy(0, QUEUESIZE + 2);
		printf("%s", tmpStr);

		gotoxy(0, QUEUESIZE + 2);
		printf("최근 파일 다운로드/업로드 상태 : [upload] - %d%% 전송", (file_size ? 100 * progress / file_size : 100));	// 다운로드 상태 갱신

		gotoxy(savedPos.X, savedPos.Y);
		pthread_mutex_unlock(&consoleLock);

		while((str_len = fread(tmp, 1, MSGSIZE / 2, fp)) > 0)													// 파일을 끝까지 읽어 들임
		{
			tmp[str_len] = NULL;
			sprintf(file, "FILE_");
			strcat(file, tmp);

			pthread_mutex_lock(&socketLock);																	// 동시에 전송 하는 것을 막기 위해 Lock
			Sleep(1000);																						// 과도하게 파일 스레드가 CPU 자원을 점유하는 것을 방지
			send(sock, file, strlen(file), 0);																	// FILE_실제 파일 내용 전송
			pthread_mutex_unlock(&socketLock);																	// 전송 완료 Lock 해제

			progress += str_len;																				// 전달 된 데이터 크기에 전송된 데이터 만큼 추가

			pthread_mutex_lock(&consoleLock);																	// 위에서 설명한 [콘솔 창 갱신]
			savedPos = getXY();

			gotoxy(0, QUEUESIZE + 2);
			printf("%s", tmpStr);

			gotoxy(0, QUEUESIZE + 2);
			printf("최근 파일 다운로드/업로드 상태 : [upload] - %d%% 전송", (file_size ? 100 * progress / file_size : 100)); // 다운로드 상태 갱신

			gotoxy(savedPos.X, savedPos.Y);
			pthread_mutex_unlock(&consoleLock);
		}

		fclose(fp);																								// 전송 완료, 파일 닫기

		pthread_mutex_lock(&socketLock);																		// 동시에 전송 하는 것을 막기 위해 Lock
		send(sock, "END_OF_FILE", strlen("END_OF_FILE"), 0);													// 파일 데이터가 끝났음을 통보
		pthread_mutex_unlock(&socketLock);																		// 전송 완료 Lock 해제

		pthread_mutex_lock(&consoleLock);																		// 위에서 설명한 [콘솔 창 갱신]
		savedPos = getXY();

		gotoxy(0, QUEUESIZE + 2);
		printf("%s", tmpStr);

		gotoxy(0, QUEUESIZE + 2);
		printf("최근 파일 다운로드/업로드 상태 : 업로드 전송성공 - %s", filename);								// 다운로드 상태 갱신

		gotoxy(savedPos.X, savedPos.Y);
		pthread_mutex_unlock(&consoleLock);

		free(tmpStr);
	}
	else																										// 파일 열기 불가능, 주로 없는 파일을 보내려 할 때 발생
	{
		tmpStr = (char *)malloc(sizeof(char) * BUFSIZE);
		snprintf(tmpStr, BUFSIZE - 1, "파일을 열 수 없습니다 - %s\n", filename);								// 결과 상태 메시지 추가
		while(textStorage.size() >= QUEUESIZE - 1)																// 위에서 설명한 [콘솔 창 갱신]
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
		printf("최근 파일 다운로드/업로드 상태 : 업로드 전송실패 - %s", filename);								// 다운로드 상태 갱신

		gotoxy(savedPos.X, savedPos.Y);
		pthread_mutex_unlock(&consoleLock);

		free(tmpStr);
	}

	bPushed = false;																							// 파일 미송신 상태로 변경

	return 0;																									// 파일 전송 스레드 종료
}