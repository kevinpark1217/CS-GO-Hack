#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <limits>
#include <urlmon.h>
#include <tchar.h>
#include <chrono>
#include <fstream>
#include <Shlwapi.h>
#include <wininet.h> //for uploadFile function
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment (lib,"wininet.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "PMemory.h"
#include "main.h"
#include "BSP.h"

#undef max

PMemory Mem;
PModule modEngine, modClient;
HWND csgo;
HANDLE handle;
BSP bsp;

DWORD DwLocalPlayer, DwEntityList, DwEnginePointer, DwViewAngle, DwGlow, DwViewMatrix, DwRadarBase, DwIGameResources;

struct
{

	DWORD GetDwLocalPlayer()
	{
		return Mem.Read<DWORD>(modClient.dwBase + DwLocalPlayer);
	}

	void SetAngles(float* Angles)
	{
		if (Angles[0] <= 89.0f && Angles[0] >= -89.0f && Angles[1] < 180.0f && Angles[1] >= -180.0f) {
			DWORD AnglePointer = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
			Mem.Write<float*>(AnglePointer + DwViewAngle, Angles, true, 2);
		}
		else
			std::cout << "INVALID AMGLE: " << Angles[0] << " " << Angles[1] << std::endl;
	}

	void GetVelocity(float* Buffer)
	{
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		Mem.Read<float*>(DwLocalPlayer + DwVecVelocity, true, 3, Buffer);
	}

	float GetZVelocity()
	{
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		float Velocity[3];
		Mem.Read<float*>(DwLocalPlayer + DwVecVelocity, true, 3, Velocity);
		return Velocity[2];
	}

	void GetAngles(float* Angles)
	{
		DWORD AnglePointer = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
		Mem.Read<float*>(AnglePointer + DwViewAngle, true, 3, Angles);
	}

	int GetFlags()
	{
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		return Mem.Read<int>(DwLocalPlayer + DwFlags);
	}

	int GetTeam()
	{
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		return Mem.Read<int>(DwLocalPlayer + DwTeamNumber);
	}

	int GetCrosshairId()
	{
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		return Mem.Read<int>(DwLocalPlayer + DwCrosshairId) - 1;
	}

	void GetPunch(float* Punch)
	{
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		Mem.Read<float*>(DwLocalPlayer + DwVecPunch, true, 2, Punch);
	}

	void SetPunch(float* Punch) //SHADY METHOD
	{
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		Mem.Write<float*>(DwLocalPlayer + DwVecPunch, Punch, true, 2);
	}

	int GetHealth()
	{
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		return Mem.Read<int>(DwLocalPlayer + DwHealth);
	}

	float GetViewOrigin()
	{
		float Vecview[3];
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		Mem.Read<float*>(DwLocalPlayer + DwVecViewOffset, true, 3, Vecview);
		return Vecview[2];
	}

	void GetPosition(float* Position)
	{
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		Mem.Read<float*>(DwLocalPlayer + DwVecOrigin, true, 3, Position);
	}

	int GetShotsFired()
	{
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		return Mem.Read<int>(DwLocalPlayer + DwShotsFired);
	}
	float getFlashDuration() {
		float flashDur[2] = { 0.0f, 0.0f };
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		Mem.Read<float*>(DwLocalPlayer + DwFlashDuration, true, 1, flashDur);
		return flashDur[0];
	}
	void setFlashDuration(float duration) {
		float flashDur[2] = { duration, 0.0f };
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		Mem.Write<float*>(DwLocalPlayer + DwFlashDuration, flashDur, true, 1);
	}

	DWORD GetDwWeapon() {
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		DWORD PlayerWeaponIndex = (Mem.Read<DWORD>(DwLocalPlayer + DwActiveWeapon)) & 0xfff;
		return Mem.Read<DWORD>(modClient.dwBase + DwEntityList + (PlayerWeaponIndex * 0x10) - 0x10);
	}

	int GetWeaponId() {
		DWORD PlayerWeapon = GetDwWeapon();
		return Mem.Read<int>(PlayerWeapon + DwWeaponId/* + 0x4*/);
	}

	int GetWeaponClip() {
		DWORD PlayerWeapon = GetDwWeapon();
		return Mem.Read<int>(PlayerWeapon + DwClip);
	}

}NewPlayer;

struct
{
	DWORD GetBaseEntity(int PlayerNumber)
	{
		return Mem.Read<DWORD>(modClient.dwBase + DwEntityList + (DwEntitySize * PlayerNumber));
	}

	void GetPosition(int PlayerNumber, float* Position)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		Mem.Read<float*>(BaseEntity + DwVecOrigin, true, 3, Position);
	}

	void GetPosition(DWORD BaseEntity, float* Position)
	{
		Mem.Read<float*>(BaseEntity + DwVecOrigin, true, 3, Position);
	}

	bool IsDead(int PlayerNumber)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<bool>(BaseEntity + DwLifeState);
	}

	bool IsDead(DWORD BaseEntity)
	{
		return Mem.Read<bool>(BaseEntity + DwLifeState);
	}

	bool IsDormant(int PlayerNumber)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<bool>(BaseEntity + DwIsDormant);
	}

	bool IsDormant(DWORD BaseEntity)
	{
		return Mem.Read<bool>(BaseEntity + DwIsDormant);
	}

	int GetTeam(int PlayerNumber)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<int>(BaseEntity + DwTeamNumber);
	}

	int GetTeam(DWORD BaseEntity)
	{
		return Mem.Read<int>(BaseEntity + DwTeamNumber);
	}

	int GetHealth(int PlayerNumber)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<int>(BaseEntity + DwHealth);
	}

	int GetHealth(DWORD BaseEntity)
	{
		return Mem.Read<int>(BaseEntity + DwHealth);
	}

	void GetVelocity(int PlayerNumber, float* Buffer)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		Mem.Read<float*>(BaseEntity + DwVecVelocity, true, 3, Buffer);
	}

	void GetVelocity(DWORD BaseEntity, float* Buffer)
	{
		Mem.Read<float*>(BaseEntity + DwVecVelocity, true, 3, Buffer);
	}

	bool isSpotted(int PlayerNumber) {
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<bool>(BaseEntity + DwSpotted);
	}

	bool isSpotted(DWORD BaseEntity) {
		return Mem.Read<bool>(BaseEntity + DwSpotted);
	}

	void GetBonePosition(DWORD BaseEntity, float* BonePosition, int TargetBone)
	{
		DWORD BoneMatrix = Mem.Read<DWORD>(BaseEntity + DwBoneMatrix);
		Mem.Read<float*>(BoneMatrix + 0x30 * TargetBone + 0x0C, true, 1, &BonePosition[0]);
		Mem.Read<float*>(BoneMatrix + 0x30 * TargetBone + 0x1C, true, 1, &BonePosition[1]);
		Mem.Read<float*>(BoneMatrix + 0x30 * TargetBone + 0x2C, true, 1, &BonePosition[2]);
	}

	void GetBonePosition(int PlayerNumber, float* BonePosition, int TargetBone)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		DWORD BoneMatrix = Mem.Read<DWORD>(BaseEntity + DwBoneMatrix);
		Mem.Read<float*>(BoneMatrix + 0x30 * TargetBone + 0x0C, true, 1, &BonePosition[0]);
		Mem.Read<float*>(BoneMatrix + 0x30 * TargetBone + 0x1C, true, 1, &BonePosition[1]);
		Mem.Read<float*>(BoneMatrix + 0x30 * TargetBone + 0x2C, true, 1, &BonePosition[2]);
	}

	void GetName(int PlayerNumber, char* name)
	{
		int Radar = Mem.Read<int>(modClient.dwBase + DwRadarBase);
		int RadarPointer = Mem.Read<int>(Radar + 0x50);
		for (int i = 0; i < 32; i++)
		{
			name[i] = Mem.Read<char>(RadarPointer + (0x1E0 * (PlayerNumber + 1) + 0x24 + i * 2));
		}
	}

	int GetWeaponId(int PlayerNumber) {
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		DWORD PlayerWeaponIndex = (Mem.Read<DWORD>(BaseEntity + DwActiveWeapon)) & 0xfff;
		DWORD PlayerWeapon = Mem.Read<DWORD>(modClient.dwBase + DwEntityList + (PlayerWeaponIndex * 0x10) - 0x10);
		return Mem.Read<int>(PlayerWeapon + DwWeaponId/* + 0x4*/);
	}

}EntityList;

void worldToAngle(float* myPos, float* enemyPos, float* ang) {
	float xD = enemyPos[0] - myPos[0];
	float yD = enemyPos[1] - myPos[1];
	if (xD < 0 && yD < 0)
		ang[1] = (atan(yD / xD) * (180 / 3.14159265f));
	else if (xD >= 0 && yD >= 0)
		ang[1] = -180 + (atan(yD / xD) * (180 / 3.14159265f));
	else if (xD <= 0 && yD >= 0)
		ang[1] = (atan(yD / xD) * (180 / 3.14159265f));
	else if (xD>0 && yD<0)
		ang[1] = 180 + (atan(yD / xD) * (180 / 3.14159265f));
	if (ang[1] >= 180.00f)
		ang[1] = 179.99f;
	else if (ang[1] <= -180.0f)
		ang[1] = -179.99f;
	float xyD = sqrt(xD*xD + yD*yD);
	ang[0] = atan((enemyPos[2] - myPos[2]) / xyD) * (180 / 3.14159265f);
	ang[1] = (float)ang[1];
	ang[0] = (float)ang[0];
}

void v0();
void v1();
void v2();

char map[128];

void loadMap() {
	DWORD ClientState = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
	ReadProcessMemory(handle, (LPVOID)(ClientState + DwMapname), map, sizeof(char) * 128, NULL);

	char filename[MAX_PATH];
	if (GetModuleFileNameEx(handle, NULL, filename, MAX_PATH) == 0) {
		exit(2);
	}
	std::string tmp = filename;
	int pos = tmp.find("csgo");
	tmp = tmp.substr(0, pos);
	tmp += "csgo/maps/";
	tmp = tmp + map;
	tmp += ".bsp";

	bsp.LoadBSP(tmp);
}

double mouseValue = 0.3; //laptop
int xCursor = 730;
int yCursor = 80;
int waitTime = 8000;

/*double mouseValue = 1; //desktop
int xCursor = 1022;
int yCursor = 125;
int waitTime = 14000;*/

void click() {
	mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL);
	Sleep(5);
	mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL);
}

void MouseMove(double x, double y)
{
	double fScreenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;
	double fScreenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1;
	double fx = x*(65535.0f / fScreenWidth);
	double fy = y*(65535.0f / fScreenHeight);
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	Input.mi.dx = fx;
	Input.mi.dy = fy;
	::SendInput(1, &Input, sizeof(INPUT));
}

bool isRunning = false;

void face(float y) {
	float currentAng[3];
	while (fabs(currentAng[1] - y) > 0.5f) {
		if (NewPlayer.GetHealth() == 0)
			break;

		if (currentAng[1] > y && currentAng[1] - y < 180)
			MouseMove(mouseValue, 0);
		else if (currentAng[1] < y && y - currentAng[1] < 180)
			MouseMove(-mouseValue, 0);
		else if (currentAng[1] > y)
			MouseMove(-mouseValue, 0);
		else if (currentAng[1] < y)
			MouseMove(mouseValue, 0);

		Sleep(5);
		NewPlayer.GetAngles(currentAng);
	}
}

void toCheckPoint(float x, float y, float z) {
	float myPos[3];
	float dest[3] = { x, y, z };
	float ang[3];
	float currentAng[3];
	NewPlayer.GetAngles(currentAng);
	NewPlayer.GetPosition(myPos);
	worldToAngle(dest, myPos, ang);
	keybd_event(0x57, MapVirtualKey(0x57, 0), 0, 0);
	while (fabs(currentAng[1] - ang[1]) > 0.5f) {
		if (NewPlayer.GetHealth() == 0)
			break;

		if (currentAng[1] > ang[1] && currentAng[1] - ang[1] < 180)
			MouseMove(mouseValue, 0);
		else if (currentAng[1] < ang[1] && ang[1] - currentAng[1] < 180)
			MouseMove(-mouseValue, 0);
		else if(currentAng[1] > ang[1])
			MouseMove(-mouseValue, 0);
		else if (currentAng[1] < ang[1])
			MouseMove(mouseValue, 0);

		Sleep(5);
		NewPlayer.GetAngles(currentAng);
		NewPlayer.GetPosition(myPos);
		worldToAngle(dest, myPos, ang);
	}
	while (fabs(myPos[0] - x) > 10 || fabs(myPos[1] - y) > 10 || fabs(myPos[2] - z) > 100) {
		if (NewPlayer.GetHealth() == 0)
			break;
		NewPlayer.GetPosition(myPos);
		ang[0] = 0.0f;
		NewPlayer.SetAngles(ang);
		Sleep(10);
		NewPlayer.GetPosition(myPos);
		worldToAngle(dest, myPos, ang);
	}
	keybd_event(0x57, MapVirtualKey(0x57, 0), KEYEVENTF_KEYUP, 0);
}

void jumpOver() {
	keybd_event(0x57, MapVirtualKey(0x57, 0), 0, 0);
	keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), 0, 0);
	Sleep(1000);
	keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), KEYEVENTF_KEYUP, 0);
	keybd_event(0x57, MapVirtualKey(0x57, 0), KEYEVENTF_KEYUP, 0);
}

/*void disconnect() {
	keybd_event(VK_OEM_3, MapVirtualKey(VK_OEM_3, 0), 0, 0);
	Sleep(100);
	keybd_event(VK_OEM_3, MapVirtualKey(VK_OEM_3, 0), KEYEVENTF_KEYUP, 0);
	Sleep(1000);

	keybd_event(0x44, MapVirtualKey(0x44, 0), 0, 0); //D
	Sleep(100);
	keybd_event(0x44, MapVirtualKey(0x44, 0), KEYEVENTF_KEYUP, 0);
	Sleep(100);

	keybd_event(0x49, MapVirtualKey(0x49, 0), 0, 0); //I
	Sleep(100);
	keybd_event(0x49, MapVirtualKey(0x49, 0), KEYEVENTF_KEYUP, 0);
	Sleep(100);

	keybd_event(0x53, MapVirtualKey(0x53, 0), 0, 0); //S
	Sleep(100);
	keybd_event(0x53, MapVirtualKey(0x53, 0), KEYEVENTF_KEYUP, 0);
	Sleep(100);

	keybd_event(0x43, MapVirtualKey(0x43, 0), 0, 0); //C
	Sleep(100);
	keybd_event(0x43, MapVirtualKey(0x43, 0), KEYEVENTF_KEYUP, 0);
	Sleep(100);

	keybd_event(0x4F, MapVirtualKey(0x4F, 0), 0, 0); //O
	Sleep(100);
	keybd_event(0x4F, MapVirtualKey(0x4F, 0), KEYEVENTF_KEYUP, 0);
	Sleep(100);

	keybd_event(0x4E, MapVirtualKey(0x4E, 0), 0, 0); //N
	Sleep(100);
	keybd_event(0x4E, MapVirtualKey(0x4E, 0), KEYEVENTF_KEYUP, 0);
	Sleep(100);

	keybd_event(0x4E, MapVirtualKey(0x4E, 0), 0, 0); //N
	Sleep(100);
	keybd_event(0x4E, MapVirtualKey(0x4E, 0), KEYEVENTF_KEYUP, 0);
	Sleep(100);

	keybd_event(0x45, MapVirtualKey(0x45, 0), 0, 0); //E
	Sleep(100);
	keybd_event(0x45, MapVirtualKey(0x45, 0), KEYEVENTF_KEYUP, 0);
	Sleep(100);

	keybd_event(0x43, MapVirtualKey(0x43, 0), 0, 0); //C
	Sleep(100);
	keybd_event(0x43, MapVirtualKey(0x43, 0), KEYEVENTF_KEYUP, 0);
	Sleep(100);

	keybd_event(0x54, MapVirtualKey(0x54, 0), 0, 0); //T
	Sleep(100);
	keybd_event(0x54, MapVirtualKey(0x54, 0), KEYEVENTF_KEYUP, 0);
	Sleep(100);

	keybd_event(VK_RETURN, MapVirtualKey(VK_RETURN, 0), 0, 0); //ENTER
	Sleep(100);
	keybd_event(VK_RETURN, MapVirtualKey(VK_RETURN, 0), KEYEVENTF_KEYUP, 0);
	Sleep(5000);

	keybd_event(VK_OEM_3, MapVirtualKey(VK_OEM_3, 0), 0, 0);
	Sleep(100);
	keybd_event(VK_OEM_3, MapVirtualKey(VK_OEM_3, 0), KEYEVENTF_KEYUP, 0);
	Sleep(1000);
}

void reconnect() {
	SetCursorPos(xCursor, yCursor);
	click();
}*/

int main() {

	std::cout << "TekHak\nCounter Strike: Global Offensive Lite-Hack\nDeveloped by TekHak\n" << std::endl;

	std::cout << "Searching for csgo.exe..." << std::endl;
	while (!Mem.Attach("csgo.exe")) {
		std::cout << ".";
		Sleep(500);
	}
	std::cout << "csgo.exe Found" << std::endl;

	csgo = FindWindow(NULL, "Counter-Strike: Global Offensive");

	modClient = Mem.GetModule("client.dll");
	modEngine = Mem.GetModule("engine.dll");

	Sleep(300);

	std::cout << "\nSearching for the offsets" << std::endl;

	DWORD elStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "x????xx?xxx", 11, 0x5, 0x0, 0x0, 0x0, 0x0, 0xC1, 0xE9, 0x0, 0x39, 0x48, 0x4);
	DWORD elP1 = Mem.Read<DWORD>(elStart + 1);
	BYTE elP2 = Mem.Read<BYTE>(elStart + 7);
	DwEntityList = (elP1 + elP2) - modClient.dwBase;
	//std::cout << "0x" << std::hex << DwEntityList << std::endl;

	DWORD lpStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "x????xx????????x????xxxx", 24, 0xA3, 0, 0, 0, 0, 0xC7, 0x05, 0, 0, 0, 0, 0, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0x59, 0xC3, 0x6A, 0);
	DWORD lpP1 = Mem.Read<DWORD>(lpStart + 1);
	DwLocalPlayer = (lpP1 + 0x10) - modClient.dwBase;
	//std::cout << "0x" << std::hex << DwLocalPlayer << std::endl;

	DWORD gpStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "x????xxxx????xx", 15, 0xE8, 0, 0, 0, 0, 0x83, 0xC4, 0x04, 0xB8, 0, 0, 0, 0, 0xC3, 0xCC);
	DwGlow = Mem.Read<DWORD>(gpStart + 9) - modClient.dwBase;
	//std::cout << "0x" << std::hex << DwGlow << " 0x" << sizeof(GlowObjectDefinition_t) << std::endl;

	DWORD epStart = Mem.FindPatternArr(modEngine.dwBase, modEngine.dwSize, "xxxxxxxx????xxxxxxxxxx????xxxx????xxx", 37, 0xF3, 0x0F, 0x5C, 0xC1, 0xF3, 0x0F, 0x10, 0x15, 0x0, 0x0, 0x0, 0x0, 0x0F, 0x2F, 0xD0, 0x76, 0x04, 0xF3, 0x0F, 0x58, 0xC1, 0xA1, 0x0, 0x0, 0x0, 0x0, 0xF3, 0x0F, 0x11, 0x80, 0x0, 0x0, 0x0, 0x0, 0xD9, 0x46, 0x04);
	DwEnginePointer = Mem.Read<DWORD>(epStart + 22) - modEngine.dwBase;
	DwViewAngle = Mem.Read<DWORD>(epStart + 30);
	//std::cout << "0x" << std::hex << DwViewAngle << std::endl;

	DWORD vpStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "xxxxxxxxxxxxxxxxxxxxxxx????xxxxxx", 33, 0x53, 0x8B, 0xDC, 0x83, 0xEC, 0x08, 0x83, 0xE4, 0xF0, 0x83, 0xC4, 0x04, 0x55, 0x8B, 0x6B, 0x04, 0x89, 0x6C, 0x24, 0x04, 0x8B, 0xEC, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x81, 0xEC, 0x98, 0x03, 0x00, 0x00);
	DwViewMatrix = Mem.Read<DWORD>(vpStart + 0x4EE) + 0x80 - modClient.dwBase;
	//std::cout << "0x" << std::hex << DwViewMatrix << std::endl;

	DWORD rpStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "x????xxxxxxx?xxx????xxxx????", 28, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x0C, 0xB0, 0x8B, 0x01, 0xFF, 0x50, 0x00, 0x46, 0x3B, 0x35, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xEA, 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00);
	DwRadarBase = Mem.Read<DWORD>(rpStart + 1) - modClient.dwBase;
	//std::cout << "0x" << std::hex << DwRadarBase << std::endl;

	std::cout << "Offsets Found\n" << std::endl;

	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);

	HANDLE _process;
	DWORD pID;
	do
		if (!strcmp(entry.szExeFile, "csgo.exe")) {
			pID = entry.th32ProcessID;
			CloseHandle(handle);

			_process = OpenProcess(PROCESS_ALL_ACCESS, false, pID);
		}
	while (Process32Next(handle, &entry));

	Sleep(300);

	POINT p;
	if (GetCursorPos(&p))
		std::cout << p.x << " " << p.y << std::endl;

	int inGame, weaponMode = -1, maxPlayer;
	while (true) {
		DWORD ClientState = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
		ReadProcessMemory(handle, (LPVOID)(ClientState + DwInGame), &inGame, sizeof(int), NULL);
		if (inGame == 6 && !isRunning) {
			Sleep(6000);
			loadMap();
			isRunning = true;
			keybd_event(VK_ESCAPE, MapVirtualKey(VK_ESCAPE, 0), 0, 0); //Escape 1
			Sleep(100);
			keybd_event(VK_ESCAPE, MapVirtualKey(VK_ESCAPE, 0), KEYEVENTF_KEYUP, 0);
			Sleep(2000);
			keybd_event(VK_ESCAPE, MapVirtualKey(VK_ESCAPE, 0), 0, 0); //Escape 2
			Sleep(100);
			keybd_event(VK_ESCAPE, MapVirtualKey(VK_ESCAPE, 0), KEYEVENTF_KEYUP, 0);
		}
		else if ((inGame != 6 && isRunning)) {
			isRunning = false;
			Sleep(5000);
			continue;
		}
		else if (!isRunning) {
			/*SetCursorPos(473, 257);
			click();
			Sleep(1000);
			SetCursorPos(1433, 1000);
			click();
			Sleep(1000);*/

			SetCursorPos(350, 200);
			click();
			continue;
		}
		else if (NewPlayer.GetHealth() == 0) {
			Sleep(100);
			continue;
		}

		/*ReadProcessMemory(handle, (LPVOID)(ClientState + DwMaxPlayer), &maxPlayer, sizeof(int), NULL);
		bool isExist = false;
		for (int j = 0; j < maxPlayer; j++) {
			if (EntityList.GetBaseEntity(j) == 0x0)
				continue;
			char name[64];
			EntityList.GetName(j, name);
			if (strcmp(name, "Dedication") == 0)
				isExist = true;
		}
		if (!isExist) {
			disconnect();
			std::cout << "Disconnect" << std::endl;
			Sleep(waitTime);
			reconnect();
			std::cout << "Reconnect" << std::endl;
			continue;
		}*/


		/*float position[3];
		NewPlayer.GetPosition(position);
		if (position[0] > -1200.0f && position[1] > -1000.0f && position[1] < -650.f && position[0] < -250.0f) {
			std::cout << "T Spawn" << std::endl;
			float myPos[3];
			float dest[3] = { -1200.0f, -800.0f, 194.0f };
			float ang[3];
			NewPlayer.GetPosition(myPos);
			worldToAngle(dest, myPos, ang);
			face(ang[1]);

			toCheckPoint(-1200.0f, -800.0f, 194.0f);
			toCheckPoint(-1470.0f, -395.0f, 199.0f);
			toCheckPoint(-1470.0f, -190.0f, 196.0f);
			jumpOver();
			toCheckPoint(-1660.0f, 480.0f, 70.0f);
			toCheckPoint(-1670.0f, 1045.0f, 97.0f);
			toCheckPoint(-1970.0f, 1285.0f, 96.0f);
			toCheckPoint(-1990.0f, 1910.0f, 66.0f);
			toCheckPoint(-1767.0f, 2268.0f, 71.0f);
			toCheckPoint(-1696.0f, 2595.0f, 71.0f);
			toCheckPoint(-1385.0f, 2777.0f, 71.0f);
			face(-120.0f);
			//Disconnect Modules
			std::cout << "T Spawn: Done" << std::endl;
			disconnect();
			std::cout << "Disconnect" << std::endl;
			Sleep(waitTime);
			reconnect();
			std::cout << "Reconnect" << std::endl;
			
		}
		if (position[0] > 60.0f && position[1] > 2000.0f && position[1] < 2530.f && position[0] < 480.0f) {
			float myPos[3];
			float dest[3] = { 490.0f, 2185.0f, -62.0f };
			float ang[3];
			NewPlayer.GetPosition(myPos);
			worldToAngle(dest, myPos, ang);
			face(ang[1]);

			std::cout << "CT Spawn" << std::endl;
			toCheckPoint(490.0f, 2185.0f, -62.0f);
			toCheckPoint(1210.0f, 2090.0f, 69.0f);
			toCheckPoint(1315.0f, 1300.0f, 66.0f);
			//jumpOver();
			toCheckPoint(630.0f, 730.0f, 67.0f);
			toCheckPoint(660.0f, 375.0f, 68.0f);
			toCheckPoint(510.0f, 185.0f, 69.0f);
			toCheckPoint(-70.0f, 450.0f, 68.0f);
			toCheckPoint(-150.0f, 1430.0f, 64.0f);
			//Disconnect Modules
			std::cout << "Disconnect" << std::endl;
			disconnect();
			Sleep(waitTime);
			reconnect();
			std::cout << "Reconnect" << std::endl;
		}*/
	}

	return 0;
}

void angleFix(float* angle) {
	if (angle[1] >= 180.0f)
		angle[1] = -180.0f + (angle[1] - 180.0f);
	else if (angle[1] < -180.0f)
		angle[1] = 180.0f + (angle[1] + 180.0f);
	if (angle[0] > 89.0f)
		angle[0] = 89.0f;
	else if (angle[0] < -89.0f)
		angle[0] = -89.0f;
}
