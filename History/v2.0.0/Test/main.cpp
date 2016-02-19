
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <iostream>
#include <strsafe.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Ws2_32.lib")
#include "md5.h"

#define version "dv1.0.0"

#define SELF_REMOVE_STRING  TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 5000 > Nul & Del \"%s\"")
#define SELF_NEWRENAME_STRING  TEXT("cmd.exe /C rename update.exe TekHak.exe")
void destroy(char option)
{
	TCHAR szModuleName[MAX_PATH];
	TCHAR szCmd[2 * MAX_PATH];
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };

	GetModuleFileName(NULL, szModuleName, MAX_PATH);

	if (option == 0)
		StringCbPrintf(szCmd, 2 * MAX_PATH, SELF_REMOVE_STRING, szModuleName);
	else if (option == 1)
		StringCbPrintf(szCmd, 2 * MAX_PATH, SELF_NEWRENAME_STRING, szModuleName);
	CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

bool login() {
	char option =0 ;
	while (option != '1' && option != '2') {
	std::cout << "[1]Login [2]Register: ";
	std::cin >> option;
	}

	char username[33], password[33];
	std::cout << "username: ";
	#undef max
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cin.getline(username, 32);
	std::cout << "password: ";
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode = 0;
	GetConsoleMode(hStdin, &mode);
	SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
	std::cin.getline(password, 32);
	std::cout << std::endl;

	if (strlen(username) == 0 || strlen(password) == 0) {
		std::cout << "Username or password cannot be empty!";
		Sleep(2500);
		exit(2);
	}
	if (strlen(username) > 32 || strlen(password) > 32) {
		std::cout << "Username or password is too long!";
		Sleep(2500);
		exit(2);
	}
	for (int i = 0;i < strlen(username);i++) {
		if ((username[i] >= 33 && username[i] <= 126) == false) {
			std::cout << "Invalid Username Character!";
			Sleep(2500);
			exit(2);
		}
	}
	for (int i = 0;i < strlen(password);i++) {
		if ((password[i] >= 33 && password[i] <= 126) == false) {
			std::cout << "Invalid Password Character!";
			Sleep(2500);
			exit(2);
		}
	}
	if (option == '2') {
		bool match = false;
		while (!match) {
			char temp[33];
			std::cout << "Retype password: ";
			//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cin.getline(temp, 32);
			std::cout << std::endl;
			match = (std::strcmp(password, temp) == 0);
		}
	}
	std::cout << std::endl;

	strcpy_s(password, md5(password).c_str());
	password[32] = 0;
	username[32] = 0;

	char options[2] = { option, 0 };
	#define DEFAULT_BUFLEN 128
	char msg[DEFAULT_BUFLEN];
	strcpy_s(msg, options);
	strcat_s(msg, " ");
	strcat_s(msg, username);
	strcat_s(msg, " ");
	strcat_s(msg, password);
	strcat_s(msg, " ");
	strcat_s(msg, version);
	strcat_s(msg, " ");
	//std::cout << msg << std::endl;

	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 0;
	}

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

#define DEFAULT_PORT "428"
	struct hostent *he = gethostbyname("SERVER");
	if (he == NULL)
		he = gethostbyname("tekhak.com");
	char *addr = inet_ntoa(*((struct in_addr *) he->h_addr_list[0]));
	//std::cout << addr << std::endl;
	iResult = getaddrinfo(addr, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 0;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	ptr = result;
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 0;
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
		return 0;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, msg, DEFAULT_BUFLEN, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Failed to request login: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 0;
	}

	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 0;
	}

	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	char flag;
	char currentVers[10];
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			flag = recvbuf[0];
			int i = 0;
			while (recvbuf[i + 2] != ' ') {
				currentVers[i] = recvbuf[i + 2];
				i++;
			}
			currentVers[i] = 0;
		}
		else if (iResult < 0) {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			return 0;
		}
	} while (iResult > 0);

	/*for (int i = 0;i < 128;i++)
	std::cout << (int)recvbuf[i] << " ";
	std::cout << std::endl;

	std::cout << flag << " v" << currentVers << std::endl;*/

	closesocket(ConnectSocket);
	WSACleanup();

	if (option == '1') {
		if (flag == '0') {
			std::cout << "Wrong username or password. Or such user does not exist." << std::endl;
			return false;
		}
		else if (flag == '1') {
			std::cout << "Authorized. Preparing for the download." << std::endl;
			if (strcmp(currentVers, version) && strcmp("UNKNOWN", currentVers)) {
				SetConsoleMode(hStdin, mode);
				std::cout << "\nTekHak v" << currentVers << " is going to be downloaded." << std::endl;
				char toChange = 0;
				std::cout << "Download changelog?" << std::endl;
				while (toChange != '1' && toChange != '2') {
					std::cout << "Yes[1] No[2]: ";
					std::cin >> toChange;
				}
				char toRead = 0;
				std::cout << "Download readme?" << std::endl;
				while (toRead != '1' && toRead != '2') {
					std::cout << "Yes[1] No[2]: ";
					std::cin >> toRead;
				}
				HRESULT updatehr = URLDownloadToFile(NULL, _T("https://www.tekhak.com/csgo/update.exe"), _T("update.exe"), 0, NULL);
				if (SUCCEEDED(updatehr)) {
					destroy(1);
					if (toChange == '1') {
						HRESULT changehr = URLDownloadToFile(NULL, _T("https://www.tekhak.com/csgo/changelog.txt"), _T("changelog.txt"), 0, NULL);
						if (!SUCCEEDED(changehr))
							std::cout << "Failed to download the changelog. Try again later." << std::endl;
					}
					if (toRead == '1') {
						HRESULT readhr = URLDownloadToFile(NULL, _T("https://www.tekhak.com/csgo/readme.txt"), _T("readme.txt"), 0, NULL);
						if (!SUCCEEDED(readhr))
							std::cout << "Failed to download the readme. Try again later." << std::endl;
					}
					std::cout << "Finished Downloading!" << std::endl;
					destroy(0);
					return false;
				}
				else
					std::cout << "Failed to download the update. Try again later." << std::endl;
			}
			//}
			return true;
		}
		else if (flag == '2') {
			std::cout << "Unauthorized. Ask administrator for authorization." << std::endl;
			return false;
		}
		else if (flag == '3') {
			std::cout << "Banned from this cheat." << std::endl;
			destroy(0);
			return false;
		}
	}
	else if (option == '2') {
		if (flag == '0') {
			std::cout << "Registration complete. Ask the administrator for approval." << std::endl;
		}
		else {
			std::cout << "Registration failed. Username already exists." << std::endl;
		}
		return false;
	}
	return false;
}

int main() {
	std::cout << "TekHak\nCounter Strike: Global Offensive Multi-Hack Downloader\nDeveloped by Kevin Park\n" << std::endl;

	if (!login()) {
		Sleep(2500);
		exit(2);
	}
}
