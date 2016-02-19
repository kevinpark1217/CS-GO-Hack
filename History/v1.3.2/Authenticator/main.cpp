#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define DEFAULT_PORT "428"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <process.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")

struct addrinfo *result = NULL, *ptr = NULL, hints;

const char* current = "1.3.2";

char login(char* username, char* password) {
	std::ifstream myfile("B:/Services/csgo/auths.txt");
	if (myfile.is_open())
	{
		std::string line;
		std::string arr[4];
		while (getline(myfile, line))
		{
			int i = 0;
			std::stringstream ssin(line);
			while (ssin.good() && i < 4) {
				ssin >> arr[i];
				++i;
			}
			char *usern = (char*)arr[0].c_str();
			char *passw = (char*)arr[1].c_str();
			if (strlen(usern) != 0 && strlen(passw) != 0) {
				bool user = (std::strcmp(username, usern) == 0);
				bool pass = (std::strcmp(password, passw) == 0);
				if (user && pass)
					return arr[2][0];
			}
		}
		myfile.close();
	}
	return '0';
}

char exist(char* username, char* password) {
	std::ifstream myfile("B:/Services/csgo/auths.txt");
	if (myfile.is_open())
	{
		std::string line;
		std::string arr[4];
		while (getline(myfile, line))
		{
			int i = 0;
			std::stringstream ssin(line);
			while (ssin.good() && i < 4) {
				ssin >> arr[i];
				++i;
			}
			char *usern = (char*)arr[0].c_str();
			char *passw = (char*)arr[1].c_str();
			if (strlen(username) != 0) {
				bool user = (std::strcmp(username, usern) == 0);
				if (user)
					return arr[2][0];
			}
		}
		myfile.close();
	}
	return '0';
}

void regis(char* username, char* password) {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 80, "%y-%m-%d-%H-%M-%S", timeinfo);

	std::ofstream myfile("B:/Services/csgo/auths.txt", std::ofstream::app);
	if (myfile.is_open()) {
		myfile << username << " " << password << " 2 " << buffer << "\n";
		myfile.close();
	}
}

void attempt(char* username, char* ip, char sucess, char* clientVers) {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 80, "%y-%m-%d-%H-%M-%S", timeinfo);

	std::ofstream myfile("B:/Services/csgo/attempts.txt", std::ofstream::app);
	if (myfile.is_open()) {
		myfile << username << " " << ip << " " << sucess << " v" << clientVers << " " << buffer << "\n";
		myfile.close();
	}
}

unsigned __stdcall ClientSession(void *data)
{
	SOCKET ClientSocket = (SOCKET)data;
	SOCKADDR_IN client_info = { 0 };
	int addrsize = sizeof(client_info);

	getpeername(ClientSocket, (struct sockaddr*)&client_info, &addrsize);
	char *ip = inet_ntoa(client_info.sin_addr);

	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		//WSACleanup();
		return 1;
	}

#define DEFAULT_BUFLEN 128
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	char option = 0;
	char username[33];
	char password[33];
	char clientVers[10];
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		//std::cout << recvbuf << std::endl;
		if (iResult > 0) {
			option = recvbuf[0];
			int i = 0;
			while (recvbuf[i + 2] != ' ') {
				username[i] = recvbuf[i + 2];
				i++;
			}
			username[i] = 0;
			//std::cout << username;
			int j = 0;
			while (recvbuf[i  + j + 3] != ' ') {
				password[j] = recvbuf[j + i + 3];
				j++;
			}
			password[j] = 0;
			//std::cout << password;
			int k = 0;
			while (recvbuf[i +j +k + 4] != ' ') {
				clientVers[k] = recvbuf[i + j + k + 4];
				k++;
			}
			clientVers[k] = 0;

		}
		else if (iResult<0) {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			return 1;
		}
	} while (iResult > 0);

	//std::cout << option << " " << username << " " << password << " " << ip << " v" << clientVers << std::endl;
	char authed;
	if (option == '1') {
		authed = login(username, password);
		attempt(username, ip, authed, clientVers);
	}
	else if (option == '2') {
		authed = exist(username, password);
		if (authed == '0') {
			regis(username, password);
		}
	}
	//std::cout << authed << std::endl;
	char autheds[2] = { authed, 0 };
	char msg[DEFAULT_BUFLEN];
	strcpy(msg, autheds);
	strcat(msg, " ");
	strcat(msg, current);
	strcat(msg, " ");

	iResult = send(ClientSocket, msg, DEFAULT_BUFLEN, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	closesocket(ClientSocket);
}

int main() {
	WSADATA wsaData;

	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	SOCKET ClientSocket;
	while ((ClientSocket = accept(ListenSocket, NULL, NULL))) {
		unsigned threadID;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ClientSession, (void*)ClientSocket, 0, &threadID);
	}

	closesocket(ListenSocket);
	WSACleanup();

	return 0;
}