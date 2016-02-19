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
		if (Angles[0] <= 89.0f && Angles[0] >= -89.0f && Angles[1] < 180.0f && Angles[1] >= -180.f) {
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
		return Mem.Read<int>(PlayerWeapon + DwWeaponId + 0x4);
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
		return Mem.Read<int>(PlayerWeapon + DwWeaponId + 0x4);
	}

}EntityList;

int weaponType(int weaponId) {

	switch (weaponId)
	{
	case 1: //one tap
	case 25: //shotguns
	case 27:
	case 29:
	case 35:
	case 40: //scout
		return 0;
	case 2: //pistols
	case 3:
	case 4:
	case 30:
	case 32:
	case 36:
	case 61:
	case 63:
		return 1;
	case 7: //rifles
	case 8:
	case 10:
	case 13:
	case 14:
	case 16:
	case 17:
	case 19:
	case 24:
	case 26:
	case 28:
	case 33:
	case 34:
	case 39:
	case 60:
	case 64:
		return 2;
	case 9: //awp
	case 11:
	case 38:
		return 3;
	}
	return 4;
}

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

void click() {
	mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL);
	Sleep(5);
	mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL);
}

/*void MouseMove(int x, int y)
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
}*/

int type = 0;
bool isAimOn = false, isPanic = false, isRunning = false, isWall = false, isRadar = false, isRcs = false, isAimbot = false,  isAimShoot = false;
int aimBone = 6, aimSmooth = 20;

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

	//lepton.join();

	std::thread V2(v2);
	std::thread V1(v1);
	std::thread V0(v0);

	int inGame, weaponMode = -1;
	DWORD ClientState = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
	while (!isPanic) {
		ReadProcessMemory(handle, (LPVOID)(ClientState + DwInGame), &inGame, sizeof(int), NULL); //6: In Game 3: Loading
		if (inGame == 6 && !isRunning) {
			Sleep(5000);
			loadMap();
			isRunning = true;
		}
		else if (inGame != 6 && isRunning) {
			isRunning = false;
			Sleep(5000);
		}
		
		type = weaponType(NewPlayer.GetWeaponId());
		if (isAimOn == false)
			type = 4;
		if (type != weaponMode) {
			if (type == 0) {
				isRcs = false;
				isAimbot = true;
				isAimShoot = true;
				aimBone = 6;
				aimSmooth = 20;
				std::cout << "One Tap Mode Loaded" << std::endl;
			}
			else if (type == 1) {
				isRcs = true;
				isAimbot = true;
				isAimShoot = true;
				aimBone = 6;
				aimSmooth = 25;
				std::cout << "Pistol Mode Loaded" << std::endl;
			}
			else  if (type == 2) {
				isRcs = true;
				isAimbot = true;
				isAimShoot = false;
				aimBone = 6;
				aimSmooth = 40;
				std::cout << "Rifle Mode Loaded" << std::endl;
			}
			else if (type == 3) {
				isRcs = false;
				isAimbot = true;
				isAimShoot = true;
				aimBone = 5;
				aimSmooth = 20;
				std::cout << "Awp Mode Loaded" << std::endl;
			}
			else if (type == 4) {
				isRcs = false;
				isAimbot = false;
				std::cout << "Hacks Disabled" << std::endl;
			}
			weaponMode = type;
		}

		if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
		{
			isWall = !isWall;
			std::cout << "Wall ";
			if (isWall)
				std::cout << "enabled" << std::endl;
			else
				std::cout << "disabled" << std::endl;
			while (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
				Sleep(10);
		}
		if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
		{
			isRadar = !isRadar;
			std::cout << "Radar ";
			if (isRadar)
				std::cout << "enabled" << std::endl;
			else
				std::cout << "disabled" << std::endl;
			while (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
				Sleep(10);
		}
		if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
		{
			isAimOn = !isAimOn;
			std::cout << "Aimbot ";
			if (isAimOn)
				std::cout << "enabled" << std::endl;
			else
				std::cout << "disabled" << std::endl;
			while (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
				Sleep(10);
		}
		if (GetAsyncKeyState(VK_HOME) & 0x8000)
		{
			std::cout << "Stopping the program" << std::endl;
			isPanic = true;
		}

		Sleep(10);
	}

	EnableWindow(csgo, true);
	V0.join();
	std::cout << "Thread 0 Complete" << std::endl;
	V1.join();
	std::cout << "Thread 1 Complete" << std::endl;
	V2.join();
	std::cout << "Thread 2 Complete" << std::endl;

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

void v0() {
	while (true) {
		if (isPanic)
			break;
		if (!isRunning) {
			Sleep(250);
			continue;
		}
		int flags = NewPlayer.GetFlags();
		if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && flags & 0x1 == 1) {
			keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), 0, 0);
			Sleep(rand() % 10);
			keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), KEYEVENTF_KEYUP, 0);
		}
		Sleep(1);
	}
}

void v1() {
	float oldAng[2];
	while (true) {
		if (isPanic)
			break;
		else if (!isRunning || !isAimbot)
			Sleep(250);
		else {
			int maxPlayer;
			DWORD ClientState = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
			ReadProcessMemory(handle, (LPVOID)(ClientState + DwMaxPlayer), &maxPlayer, sizeof(int), NULL);
			for (int i = 0; i < maxPlayer; i++) {
				if (EntityList.GetBaseEntity(i) == 0x0)
					continue;
				float boneScreen[2];
				float boneWorld[3];
				float myWorld[3];
				if (!EntityList.IsDormant(i) && !EntityList.IsDead(i)) {
					char chModel[64];
					DWORD model = Mem.Read<DWORD>(EntityList.GetBaseEntity(i) + 0x6C);
					for (int i = 0;i < 64;i++)
						chModel[i] = Mem.Read<char>(model + 0x4 + i);
					char myChModel[64];
					DWORD myModel = Mem.Read<DWORD>(NewPlayer.GetDwLocalPlayer() + 0x6C);
					for (int i = 0;i < 64;i++)
						myChModel[i] = Mem.Read<char>(myModel + 0x4 + i);
					bool imT = strstr(myChModel, "/tm_");
					bool enT = strstr(chModel, "/tm_");
					bool imCT = strstr(myChModel, "/ctm_");
					bool enCT = strstr(chModel, "/ctm_");
					if ((!imT && !imCT) || (!enT && !enCT))
						continue;

					EntityList.GetBonePosition(i, boneWorld, 6);
					NewPlayer.GetPosition(myWorld);
					myWorld[2] += NewPlayer.GetViewOrigin();
					worldToAngle(boneWorld, myWorld, boneScreen);
					if (isRcs) {
						float MyPunch[3];
						NewPlayer.GetPunch(MyPunch);
						boneScreen[0] -= MyPunch[0] * 2.0f;
						boneScreen[1] -= MyPunch[1] * 2.0f;
						angleFix(boneScreen);
					}
					float currentAng[3];
					NewPlayer.GetAngles(currentAng);
					if (fabs(boneScreen[0] - currentAng[0]) < 2.0f && fabs(boneScreen[1] - currentAng[1]) < 1.0f && bsp.Visible(myWorld, boneWorld) && imT != enT && NewPlayer.GetTeam() != EntityList.GetTeam(i)) {
						float val0 = (boneScreen[0] - currentAng[0]) / (aimSmooth + 1), val1 = (boneScreen[1] - currentAng[1]) / (aimSmooth + 1);
						int aim = aimSmooth;
						while (fabs(boneScreen[0] - currentAng[0]) < 10.0f && fabs(boneScreen[1] - currentAng[1]) < 10.0f &&!EntityList.IsDead(i) && !EntityList.IsDormant(i) && ((GetAsyncKeyState(VK_XBUTTON1) & 0x8000) || (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) || NewPlayer.GetShotsFired() > 0 || ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && NewPlayer.GetWeaponId() == 1)) && NewPlayer.GetWeaponClip() > 0 && bsp.Visible(myWorld, boneWorld)) {
							EntityList.GetBonePosition(i, boneWorld, aimBone);
							NewPlayer.GetPosition(myWorld);
							myWorld[2] += NewPlayer.GetViewOrigin();
							worldToAngle(boneWorld, myWorld, boneScreen);
							if (aim > 0) {
								boneScreen[0] -= val0*aim;
								boneScreen[1] -= val1*aim;
								aim--;
							}
							if (isRcs) {
								float MyPunch[3];
								NewPlayer.GetPunch(MyPunch);
								boneScreen[0] -= MyPunch[0] * 2.0f;
								boneScreen[1] -= MyPunch[1] * 2.0f;
								angleFix(boneScreen);
							}
							NewPlayer.SetAngles(boneScreen);
							Sleep(5);
							if (aim == 0 && isAimShoot && !(NewPlayer.GetWeaponId() == 1)) {
								click();
								aim--;
								if (type == 1)
									break;
							}
						}
					}
				}
			}
		}
		Sleep(5);
	}
}

void v2() {
	float glowThickness = 0.4f;

	while (true) {
		if (isPanic)
			break;
		else if (!isRunning || (!isWall && !isRadar)) {
			Sleep(250);
			continue;
		}

		int maxPlayer;
		DWORD ClientState = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
		ReadProcessMemory(handle, (LPVOID)(ClientState + DwMaxPlayer), &maxPlayer, sizeof(int), NULL);
		if (isRadar && !isWall)
			for (int j = 0; j < maxPlayer; j++) {
				if (EntityList.GetBaseEntity(j) == 0x0)
					continue;
				Mem.WriteNew<int>(EntityList.GetBaseEntity(j) + DwSpotted, 1);
			}
		else if (isWall) {
			DWORD pointerGlow = Mem.Read<DWORD>(modClient.dwBase + DwGlow);
			int objCount = Mem.Read<int>(modClient.dwBase + DwGlow + 0x4);
			if (pointerGlow != NULL) {
				for (int i = 0; i < objCount; i++) {
					DWORD mObj = pointerGlow + i * sizeof(GlowObjectDefinition_t);
					GlowObjectDefinition_t glowObj = Mem.ReadNew<GlowObjectDefinition_t>(mObj);
					if (glowObj.pEntity != NULL) {
						char chModel[64];
						DWORD model = Mem.Read<DWORD>(glowObj.pEntity + 0x6C);
						for (int a = 0;a < 64;a++)
							chModel[a] = Mem.Read<char>(model + 0x4 + a);
						if (strstr(chModel, "ied")) {
							glowObj.r = 127;
							glowObj.g = 127;
							glowObj.b = 0;
							glowObj.a = glowThickness;
							glowObj.m_bRenderWhenOccluded = true;
							glowObj.m_bRenderWhenUnoccluded = false;
							Mem.WriteNew<GlowObjectDefinition_t>(mObj, glowObj);
						}
						else if (strstr(chModel, "player")) {
							float me3[4];
							float en3[4];
							EntityList.GetBonePosition(glowObj.pEntity, en3, 6);
							NewPlayer.GetPosition(me3);
							me3[2] += NewPlayer.GetViewOrigin();

							if (EntityList.GetTeam(glowObj.pEntity) == NewPlayer.GetTeam()) {
								glowObj.r = 0;
								glowObj.g = 0;
								glowObj.b = 255;
							}
							else if (glowObj.pEntity == EntityList.GetBaseEntity(NewPlayer.GetCrosshairId())) {
								glowObj.r = 127;
								glowObj.g = 0;
								glowObj.b = 0;
							}
							else if (bsp.Visible(me3, en3)) {
								glowObj.r = 0;
								glowObj.g = 255;
								glowObj.b = 0;
							}
							else {
								glowObj.r = 255;
								glowObj.g = 0;
								glowObj.b = 0;
							}
							glowObj.a = glowThickness;
							glowObj.m_bRenderWhenOccluded = true;
							glowObj.m_bRenderWhenUnoccluded = false;
							if (!EntityList.IsDead(glowObj.pEntity) && !EntityList.IsDormant(glowObj.pEntity)) {
								Mem.WriteNew<GlowObjectDefinition_t>(mObj, glowObj);
								if (isRadar)
									Mem.WriteNew<int>(glowObj.pEntity + DwSpotted, 1);
							}
						}
					}
				}
				Sleep(20);
			}
		}
	}
}