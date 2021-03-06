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
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Ws2_32.lib")

#include "PMemory.h"
#include "main.h"
#include "md5.h"
#include "BSP.h"

#undef max

PMemory Mem;
PModule modEngine, modClient;
HWND csgo;
HANDLE handle;
BSP bsp;
const char* version = "2.3.0";

DWORD DwLocalPlayer, DwEntityList, DwEnginePointer, DwViewAngle, DwGlow, DwViewMatrix, DwRadarBase/*, DwIGameResources*/;

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
		return Mem.Read<int>(PlayerWeapon + DwWeaponId);
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
		return Mem.Read<DWORD>(modClient.dwBase +DwEntityList + (DwEntitySize * PlayerNumber));
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
		return Mem.Read<int>(PlayerWeapon + 0x1670 + 0x20 - 0x04);
	}

}EntityList;


bool isPanic = false;

bool isTrigger, isTriggerToggle, isTriggerFriendly, isTriggerHold;
int triggerToggleKey, triggerDelay, triggerHoldKey;

bool isRcs, isRcsToggle, isRcsCross;
int rcsToggleKey, rcsAmount;

bool isAimbot, isAimbotToggle, isAimbotFriendly, isHoldAim, isAimWall;
int aimbotToggleKey, aimBone, aimbotHoldKey, aimSmooth;

bool isBhop, isBhopToggle;
int bhopToggleKey, bhopGameBind, bhopJumpBind;

bool isWall, isWallToggle, isWallFriendly, isWallInternal, isWallExternal, isBox, isHealth, isName, isWeapon, isChicken, isBomb;
int wallToggleKey;

bool isRadar, isRadarToggle, isFlash, isFlashToggle, isInfo, isInfoToggle;
int radarToggleKey, flashToggleKey, infoToggleKey, infoX, infoY;

bool isRage, isRageToggle, isRageFriendly;
int rageToggleKey;

int panelKey, panicKey;

bool isRunning, panelEnabled;

void loadConfig() {
	std::ifstream config("config.ini");
	if (config.is_open())
	{
		std::string line;
		while (getline(config, line)) {
			//Trigger
			if (line.find("isTrigger:") == 0)
				isTrigger = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isTriggerToggle:") == 0)
				isTriggerToggle = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("triggerToggleKey:") == 0)
				triggerToggleKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("triggerDelay:") == 0)
				triggerDelay = stoi(line.substr(line.find(":") + 1));
			else if (line.find("isTriggerFriendly:") == 0)
				isTriggerFriendly = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isTriggerHold:") == 0)
				isTriggerHold = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("triggerHoldKey:") == 0)
				triggerHoldKey = stoi(line.substr(line.find(":") + 1));
			//Rcs
			else if (line.find("isRcs:") == 0)
				isRcs = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isRcsToggle:") == 0)
				isRcsToggle = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("rcsToggleKey:") == 0)
				rcsToggleKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("rcsAmount:") == 0)
				rcsAmount = stoi(line.substr(line.find(":") + 1));
			else if (line.find("isRcsCross:") == 0)
				isRcsCross = (bool)stoi(line.substr(line.find(":") + 1));
			//Aimbot
			else if (line.find("isAimbot:") == 0)
				isAimbot = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isAimbotToggle:") == 0)
				isAimbotToggle = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("aimbotToggleKey:") == 0)
				aimbotToggleKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("aimBone:") == 0)
				aimBone = stoi(line.substr(line.find(":") + 1));
			else if (line.find("isAimbotFriendly:") == 0)
				isAimbotFriendly = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isHoldAim:") == 0)
				isHoldAim = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isAimWall:") == 0)
				isAimWall = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("aimbotHoldKey:") == 0)
				aimbotHoldKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("aimSmooth:") == 0)
				aimSmooth = stoi(line.substr(line.find(":") + 1));
			//Bhop
			else if (line.find("isBhop:") == 0)
				isBhop = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isBhopToggle:") == 0)
				isBhopToggle = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("bhopToggleKey:") == 0)
				bhopToggleKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("bhopGameBind:") == 0)
				bhopGameBind = stoi(line.substr(line.find(":") + 1));
			else if (line.find("bhopJumpBind:") == 0)
				bhopJumpBind = stoi(line.substr(line.find(":") + 1));
			//Wall
			else if (line.find("isWall:") == 0)
				isWall = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isWallToggle:") == 0)
				isWallToggle = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("wallToggleKey:") == 0)
				wallToggleKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("isWallFriendly:") == 0)
				isWallFriendly = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isWallInternal:") == 0)
				isWallInternal = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isWallExternal:") == 0)
				isWallExternal = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isBox:") == 0)
				isBox = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isHealth:") == 0)
				isHealth = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isName:") == 0)
				isName = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isWeapon:") == 0)
				isWeapon = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isChicken:") == 0)
				isChicken = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isBomb:") == 0)
				isBomb = (bool)stoi(line.substr(line.find(":") + 1));
			//Misc
			else if (line.find("isRadar:") == 0)
				isRadar = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isRadarToggle:") == 0)
				isRadarToggle = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("radarToggleKey:") == 0)
				radarToggleKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("isFlash:") == 0)
				isFlash = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isFlashToggle:") == 0)
				isFlashToggle = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("flashToggleKey:") == 0)
				flashToggleKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("isInfo:") == 0)
				isInfo = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isInfoToggle:") == 0)
				isInfoToggle = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("infoToggleKey:") == 0)
				infoToggleKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("infoX:") == 0)
				infoX = stoi(line.substr(line.find(":") + 1));
			else if (line.find("infoY:") == 0)
				infoY = stoi(line.substr(line.find(":") + 1));
			//Rage
			else if (line.find("isRage:") == 0)
				isRage = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("isRageToggle:") == 0)
				isRageToggle = (bool)stoi(line.substr(line.find(":") + 1));
			else if (line.find("rageToggleKey:") == 0)
				rageToggleKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("isRageFriendly:") == 0)
				isRageFriendly = (bool)stoi(line.substr(line.find(":") + 1));
			//Setting
			else if (line.find("panelKey:") == 0)
				panelKey = stoi(line.substr(line.find(":") + 1));
			else if (line.find("panicKey:") == 0)
				panicKey = stoi(line.substr(line.find(":") + 1));
		}
		config.close();
	}
}
void saveConfig() {
	std::ofstream config("config.ini");
	if (config.is_open())
	{
		config << "isTrigger:" << isTrigger << "\n";
		config << "isTriggerToggle:" << isTriggerToggle << "\n";
		config << "triggerToggleKey:" << triggerToggleKey << "\n";
		config << "triggerDelay:" << triggerDelay << "\n";
		config << "isTriggerFriendly:" << isTriggerFriendly << "\n";
		config << "isTriggerHold:" << isTriggerHold << "\n";
		config << "triggerHoldKey:" << triggerHoldKey << "\n";

		config << "isRcs:" << isRcs << "\n";
		config << "isRcsToggle:" << isRcsToggle << "\n";
		config << "rcsToggleKey:" << rcsToggleKey << "\n";
		config << "rcsAmount:" << rcsAmount << "\n";
		config << "isRcsCross:" << isRcsCross << "\n";

		config << "isAimbot:" << isAimbot << "\n";
		config << "isAimbotToggle:" << isAimbotToggle << "\n";
		config << "aimbotToggleKey:" << aimbotToggleKey << "\n";
		config << "aimBone:" << aimBone << "\n";
		config << "isAimbotFriendly:" << isAimbotFriendly << "\n";
		config << "isHoldAim:" << isHoldAim << "\n";
		config << "isAimWall:" << isAimWall << "\n";
		config << "aimbotHoldKey:" << aimbotHoldKey << "\n";
		config << "aimSmooth:" << aimSmooth << "\n";

		config << "isBhop:" << isBhop << "\n";
		config << "isBhopToggle:" << isBhopToggle << "\n";
		config << "bhopToggleKey:" << bhopToggleKey << "\n";
		config << "bhopGameBind:" << bhopGameBind << "\n";
		config << "bhopJumpBind:" << bhopJumpBind << "\n";

		config << "isWall:" << isWall << "\n";
		config << "isWallToggle:" << isWallToggle << "\n";
		config << "wallToggleKey:" << wallToggleKey << "\n";
		config << "isWallFriendly:" << isWallFriendly << "\n";
		config << "isWallInternal:" << isWallInternal << "\n";
		config << "isWallExternal:" << isWallExternal << "\n";
		config << "isBox:" << isBox << "\n";
		config << "isHealth:" << isHealth << "\n";
		config << "isName:" << isName << "\n";
		config << "isWeapon:" << isWeapon << "\n";
		config << "isChicken:" << isChicken << "\n";
		config << "isBomb:" << isBomb << "\n";

		config << "isRadar:" << isRadar << "\n";
		config << "isRadarToggle:" << isRadarToggle << "\n";
		config << "radarToggleKey:" << radarToggleKey << "\n";
		config << "isFlash:" << isFlash << "\n";
		config << "isFlashToggle:" << isFlashToggle << "\n";
		config << "flashToggleKey:" << flashToggleKey << "\n";
		config << "isInfo:" << isInfo << "\n";
		config << "isInfoToggle:" << isInfoToggle << "\n";
		config << "infoToggleKey:" << infoToggleKey << "\n";
		config << "infoX:" << infoX << "\n";
		config << "infoY:" << infoY << "\n";

		config << "isRage:" << isRage << "\n";
		config << "isRageToggle:" << isRageToggle << "\n";
		config << "rageToggleKey:" << rageToggleKey << "\n";
		config << "isRageFriendly:" << isRageFriendly << "\n";

		config << "panelKey:" << panelKey << "\n";
		config << "panicKey:" << panicKey << "\n";

		config.close();
	}
}
void resetConfig() {
	isTrigger = false;
	isTriggerToggle = false;
	triggerToggleKey = 97;
	triggerDelay = 1;
	isTriggerFriendly = false;
	isTriggerHold = false;
	triggerHoldKey = 5;

	isRcs = false;
	isRcsToggle = false;
	rcsToggleKey = 98;
	rcsAmount = 100;
	isRcsCross = false;

	isAimbot = false;
	isAimbotToggle = false;
	aimbotToggleKey = 99;
	aimBone = 10;
	isAimbotFriendly = false;
	isHoldAim = false;
	isAimWall = false;
	aimbotHoldKey = 5;
	aimSmooth = 4;

	isBhop = false;
	isBhopToggle = false;
	bhopToggleKey = 96;
	bhopGameBind = VK_SPACE;
	bhopJumpBind = VK_MENU;

	isWall = false;
	isWallToggle = false;
	wallToggleKey = 100;
	isWallFriendly = false;
	isWallInternal = true;
	isWallExternal = false;
	isBox = false;
	isHealth = true;
	isName = true;
	isWeapon = true;
	isChicken = false;
	isBomb = false;

	isRadar = false;
	isRadarToggle = false;
	radarToggleKey = 101;
	isFlash = false;
	isFlashToggle = false;
	flashToggleKey = 102;
	isInfo = false;
	isInfoToggle = false;
	infoToggleKey = 103;
	infoX = 10;
	infoY = 675;

	isRage = false;
	isRageToggle = false;
	rageToggleKey = 104;
	isRageFriendly = false;

	panelKey = VK_CAPITAL;
	panicKey = VK_HOME;
}
bool keyScanInProgess = false;
void keyScan(int *save)
{
	keyScanInProgess = true;
	while ((GetKeyState(VK_LBUTTON) & 0x100) != 0)
		Sleep(1);
	int key = 1;
	while ((GetKeyState(key) & 0x100) == 0) {
		if (key == 254) {
			key = 0;
			Sleep(1);
		}
		if (key == 24)
			key++;
		key++;
	}
	*save = key;
	while ((GetKeyState(key) & 0x100) != 0)
		Sleep(1);
	keyScanInProgess = false;
}

RECT m_Rect;
RECT c_Rect;
float SWidth;
float SHeight;
HWND EspHWND;
HINSTANCE CurrentInstance;

HDC MainHDC;
HGLRC MainHGLRC;
HPALETTE MainPalette;
LPCSTR WName = "CSGO Multihack Overlay";

float Get3D(float *myPos, float *enPos)
{
	return((float)(sqrtf((enPos[0] - myPos[0]) * (enPos[0] - myPos[0]) + (enPos[1] - myPos[1]) * (enPos[1] - myPos[1]) + (enPos[2] - myPos[2]) * (enPos[2] - myPos[2]))));
}

void ResizeWindow()
{
	SWidth = (float)(c_Rect.right - c_Rect.left);
	SHeight = (float)(c_Rect.bottom - c_Rect.top);
	SetWindowPos(EspHWND, 0, m_Rect.left, (int)(m_Rect.bottom - SHeight), (int)SWidth, (int)SHeight, 0);
	glViewport(m_Rect.left, m_Rect.top, (GLsizei)SWidth, (GLsizei)SHeight);
	glOrtho(c_Rect.left, c_Rect.right, c_Rect.bottom, c_Rect.top, 0, 1);
}

int font_list_base_2d = 2000;
void SetupGL()
{	
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,                                  // Version Number
		PFD_DRAW_TO_WINDOW |              // Format Must Support Window
		PFD_SUPPORT_OPENGL |              // Format Must Support OpenGL
		PFD_SUPPORT_COMPOSITION |         // Format Must Support Composition
		PFD_DOUBLEBUFFER,                 // Must Support Double Buffering
		PFD_TYPE_RGBA,                    // Request An RGBA Format
		32,                               // Select Our Color Depth
		0, 0, 0, 0, 0, 0,                 // Color Bits Ignored
		8,                                // An Alpha Buffer
		0,                                // Shift Bit Ignored
		0,                                // No Accumulation Buffer
		0, 0, 0, 0,                       // Accumulation Bits Ignored
		0,                               // No Z-Buffer (Depth Buffer)
		8,                                // Some Stencil Buffer
		0,                                // No Auxiliary Buffer
		PFD_MAIN_PLANE,                   // Main Drawing Layer
		0,                                // Reserved
		0, 0, 0                           // Layer Masks Ignored
	};
	int PixelFormat = ChoosePixelFormat(MainHDC, &pfd);
	if (!PixelFormat)
	{
		ExitThread(0);
	}
	if (!SetPixelFormat(MainHDC, PixelFormat, &pfd))
	{
		ExitThread(0);
	}
	if (!DescribePixelFormat(MainHDC, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
	{
		ExitThread(0);
	}
	MainHGLRC = wglCreateContext(MainHDC);
	if (!MainHGLRC)
	{
		ExitThread(0);
	}
	if (!wglMakeCurrent(MainHDC, MainHGLRC))
	{
		ExitThread(0);
	}
	glMatrixMode(GL_PROJECTION);
	ResizeWindow();
	glClearColor(0.f, 0.f, 0.f, 0.f);
	wglUseFontBitmaps(MainHDC, 0, 256, 1000);
	glListBase(1000);
}

bool WorldToScreen(float * from, float * to)
{
	WorldToScreenMatrix_t WorldToScreenMatrix;
	WorldToScreenMatrix = Mem.ReadTwo<WorldToScreenMatrix_t>(modClient.dwBase + DwViewMatrix);

	to[0] = WorldToScreenMatrix.flMatrix[0][0] * from[0] + WorldToScreenMatrix.flMatrix[0][1] * from[1] + WorldToScreenMatrix.flMatrix[0][2] * from[2] + WorldToScreenMatrix.flMatrix[0][3];
	to[1] = WorldToScreenMatrix.flMatrix[1][0] * from[0] + WorldToScreenMatrix.flMatrix[1][1] * from[1] + WorldToScreenMatrix.flMatrix[1][2] * from[2] + WorldToScreenMatrix.flMatrix[1][3];
	float w = WorldToScreenMatrix.flMatrix[3][0] * from[0] + WorldToScreenMatrix.flMatrix[3][1] * from[1] + WorldToScreenMatrix.flMatrix[3][2] * from[2] + WorldToScreenMatrix.flMatrix[3][3];

	if (w < 0.01f)
		return false;

	float invw = 1.0f / w;
	to[0] *= invw;
	to[1] *= invw;

	int width = (int)(m_Rect.right - m_Rect.left);
	int height = (int)(m_Rect.bottom - m_Rect.top);

	float x = width / 2.0f;
	float y = height / 2.0f;

	x += 0.5f * to[0] * width + 0.5f;
	y -= 0.5f * to[1] * height + 0.5f;

	to[0] = x + m_Rect.left;
	to[1] = y + m_Rect.top;

	return true;

}

//draw filled rectangle
void DrawFilledBox(float x, float y, float x2, float y2, int* RGB)
{
	glBegin(GL_QUADS);
	glColor4f((GLfloat)RGB[0], (GLfloat)RGB[1], (GLfloat)RGB[2], 1.0f);
	glVertex2f(x, y);
	glVertex2f(x, y2);
	glVertex2f(x2, y2);
	glVertex2f(x2, y);
	glEnd();
}

void DrawSnaplines(float x1, float y1, float x2, float y2)
{
	glLineWidth(1);
	glBegin(GL_LINES);
	glVertex2f(x2, y2);
	glVertex2f(x1 , y1);
	glEnd();
}

void DrawCross(float x, float y, float width, float height, float LineWidth, int* RGB)
{
	glLineWidth(LineWidth);
	glBegin(GL_LINES);
	glColor4f((GLfloat)RGB[0], (GLfloat)RGB[1], (GLfloat)RGB[2], 1.0f);
	glVertex2f(x, y + height);
	glVertex2f(x, y - height);
	glVertex2f(x + width, y);
	glVertex2f(x - width, y);
	glEnd();
}

void DrawBox(float x, float y, float x2, float y2, int* RGB)
{
	glLineWidth(2);
	glBegin(GL_LINE_LOOP);
	glColor4f((GLfloat)RGB[0], (GLfloat)RGB[1], (GLfloat)RGB[2], 1.0f);
	glVertex2f(x , y);
	glVertex2f(x , y2);
	glVertex2f(x2, y2);
	glVertex2f(x2, y);
	glEnd();
}

void DrawPanelStaticBox(int x1, int x2, int y1, int y2) {
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x1, y2+1);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(x1, y2);
	glVertex2f(x2, y2);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(x2, y2);
	glVertex2f(x2, y1);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(x2, y1);
	glVertex2f(x1, y1);
	glEnd();
}
 
void DrawPanelBox(int x, int y, int xoff, int yoff) {
	glBegin(GL_LINES);
	glVertex2f(x, y);
	glVertex2f(x, y+yoff+1);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(x, y+yoff);
	glVertex2f(x+xoff, y + yoff);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(x + xoff, y + yoff);
	glVertex2f(x + xoff, y);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(x + xoff, y);
	glVertex2f(x, y);
	glEnd();
}

void DrawPanelFill(int x, int y, int xoff, int yoff) {
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x, y + yoff);
	glVertex2f(x + xoff, y + yoff);
	glVertex2f(x + xoff, y);
	glEnd();
}
#define PanelX 800.0f
#define PanelY 650.0f
#define PanelSpacing 20.0f
bool isMouseClicked = false;
int color0[] = { 6, 115, 163 };
int color1[] = { 42, 170, 226 };
float xx = (PanelX - 30) / 3;
float yy = (PanelY - 90) / 2;
float x, y;
float panelBoxColor = 1.0f;

void DoCheckBox(bool *value, char* text, int &i, int &j, int line) {
	glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
	glRasterPos2f(x + xx*i + 30, y + yy*j + 60 + line*PanelSpacing);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);

	POINT p;
	if (GetCursorPos(&p) && p.x > x + xx*i + 150 && p.x < x + xx*i + 150 + 14 && p.y > y + yy*j + 48 + line*PanelSpacing && p.y < y + yy*j + 62 + line*PanelSpacing && (GetKeyState(VK_LBUTTON) & 0x100) != 0) {
		if (!isMouseClicked) {
			isMouseClicked = true;
			*value = !*value;
		}
	}

	DrawPanelBox(x + xx*i + 150, y + yy*j + 48 + line*PanelSpacing, 14, 14);
	if (*value)
		DrawPanelFill(x + xx*i + 151, y + yy*j + 50 + line*PanelSpacing, 11, 11);
}

void DoCheckNumberBox(bool *value, int *number, char* text, int &i, int &j, int line) {
	glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
	char buf[4];
	glRasterPos2f(x + xx*i + 30, y + yy*j + 60 + line*PanelSpacing);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);

	POINT p;
	if ((GetKeyState(VK_LBUTTON) & 0x100) != 0 && GetCursorPos(&p)) {
		if (p.x > x + xx*i + 150 && p.x < x + xx*i + 150 + 14 && p.y > y + yy*j + 48 + line*PanelSpacing && p.y < y + yy*j + 62 + line*PanelSpacing) {
			if (!isMouseClicked) {
				isMouseClicked = true;
				*value = !*value;
			}
		}
		else if (p.x > x + xx*i + 170 && p.x < x + xx*i + 170 + 29 && p.y > y + yy*j + 48 + line*PanelSpacing && p.y < y + yy*j + 62 + line*PanelSpacing) {
			glColor4f((GLfloat)color0[0] / 255, (GLfloat)color0[1] / 255, (GLfloat)color0[2] / 255, 1.0f);
			DrawPanelFill(x + xx*i + 170, y + yy*j + 48 + line*PanelSpacing, 29, 14);
			if (!isMouseClicked) {
				isMouseClicked = true;
				std::thread(&keyScan, number).detach();
			}
			glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
		}
	}

	DrawPanelBox(x + xx*i + 150, y + yy*j + 48 + line*PanelSpacing, 14, 14);
	if (*value)
		DrawPanelFill(x + xx*i + 151, y + yy*j + 50 + line*PanelSpacing, 11, 11);
	DrawPanelBox(x + xx*i + 170, y + yy*j + 48 + line*PanelSpacing, 29, 14); //Toggle Key
	snprintf(buf, 4, "%3d", *number);
	glRasterPos2f(x + xx*i + 172, y + yy*j + 60 + line*PanelSpacing);
	glCallLists(strlen(buf), GL_UNSIGNED_BYTE, buf);
}

void DoNumberBox(int *number, char* text, int &i, int &j, int line) {
	glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
	char buf[4];
	glRasterPos2f(x + xx*i + 30, y + yy*j + 60 + line*PanelSpacing);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);

	POINT p;
	if ((GetKeyState(VK_LBUTTON) & 0x100) != 0 && GetCursorPos(&p) && p.x > x + xx*i + 150 && p.x < x + xx*i + 150 + 29 && p.y > y + yy*j + 48 + line*PanelSpacing && p.y < y + yy*j + 62 + line*PanelSpacing) {
		glColor4f((GLfloat)color0[0] / 255, (GLfloat)color0[1] / 255, (GLfloat)color0[2] / 255, 1.0f);
		DrawPanelFill(x + xx*i + 150, y + yy*j + 48 + line*PanelSpacing, 29, 14);
		if (!isMouseClicked) {
			isMouseClicked = true;
			std::thread(&keyScan, number).detach();
		}
		glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
	}

	DrawPanelBox(x + xx*i + 150, y + yy*j + 48 + line*PanelSpacing, 29, 14); //Toggle Key
	snprintf(buf, 4, "%3d", *number);
	glRasterPos2f(x + xx*i + 152, y + yy*j + 60 + line*PanelSpacing);
	glCallLists(strlen(buf), GL_UNSIGNED_BYTE, buf);
}

void DoNumberCycleBox(int *number, char* text, int* cycle, char** cycleName, int size, int &i, int &j, int line) {
	glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
	static int loop = 0;
	char buf[32];
	glRasterPos2f(x + xx*i + 30, y + yy*j + 60 + line*PanelSpacing);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);

	POINT p;
	if ((GetKeyState(VK_LBUTTON) & 0x100) != 0 && GetCursorPos(&p) && p.x > x + xx*i + 150 && p.x < x + xx*i + 150 + 44 && p.y > y + yy*j + 48 + line*PanelSpacing && p.y < y + yy*j + 62 + line*PanelSpacing) {
		glColor4f((GLfloat)color0[0] / 255, (GLfloat)color0[1] / 255, (GLfloat)color0[2] / 255, 1.0f);
		DrawPanelFill(x + xx*i + 150, y + yy*j + 48 + line*PanelSpacing, 44, 14);
		if (!isMouseClicked) {
			isMouseClicked = true;
			if (size == loop + 1)
				*number = cycle[loop = 0];
			else
				*number = cycle[loop++];
			std::thread(&keyScan, number).detach();
		}
		glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
	}

	DrawPanelBox(x + xx*i + 150, y + yy*j + 48 + line*PanelSpacing, 44, 14); //Toggle Key
	glRasterPos2f(x + xx*i + 152, y + yy*j + 60 + line*PanelSpacing);
	glCallLists(strlen(cycleName[loop]), GL_UNSIGNED_BYTE, cycleName[loop]);
}

void DoNumberChangeBox(int *number, char* text, char* unit, int lowerBound, int upperBound, int &i, int &j, int line) {
	glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
	char buf[4];
	glRasterPos2f(x + xx*i + 30, y + yy*j + 60 + line*PanelSpacing);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);

	POINT p;
	if ((GetKeyState(VK_LBUTTON) & 0x100) != 0 && GetCursorPos(&p)) {

		if (p.x > x + xx*i + 150 + 29 && p.x < x + xx*i + 150 + 34 && p.y > y + yy*j + 60 + line*PanelSpacing - 11 && p.y < y + yy*j + 60 + line*PanelSpacing - 5)
		{
			glColor4f((GLfloat)color0[0] / 255, (GLfloat)color0[1] / 255, (GLfloat)color0[2] / 255, 1.0f);
			DrawPanelFill(x + xx*i + 150 + 29, y + yy*j + 60 + line*PanelSpacing - 11, 5, 6);
			if (!isMouseClicked && (*number) < upperBound) {
				isMouseClicked = true;
				(*number)++;
			}
			glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
		}
		else if (p.x > x + xx*i + 150 + 29 && p.x < x + xx*i + 150 + 34 && p.y > y + yy*j + 60 + line*PanelSpacing - 4 && p.y < y + yy*j + 60 + line*PanelSpacing + 2)
		{
			glColor4f((GLfloat)color0[0] / 255, (GLfloat)color0[1] / 255, (GLfloat)color0[2] / 255, 1.0f);
			DrawPanelFill(x + xx*i + 150 + 29, y + yy*j + 60 + line*PanelSpacing - 4, 5, 6);
			if (!isMouseClicked && (*number) > lowerBound) {
				isMouseClicked = true;
				(*number)--;
			}
			glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
		}
		else if (p.x > x + xx*i + 150 + 35 && p.x < x + xx*i + 150 + 40 && p.y > y + yy*j + 60 + line*PanelSpacing - 11 && p.y < y + yy*j + 60 + line*PanelSpacing - 5)
		{
			glColor4f((GLfloat)color0[0] / 255, (GLfloat)color0[1] / 255, (GLfloat)color0[2] / 255, 1.0f);
			DrawPanelFill(x + xx*i + 150 + 35, y + yy*j + 60 + line*PanelSpacing - 11, 5, 6);
			if (!isMouseClicked && (*number) + 4 < upperBound) {
				isMouseClicked = true;
				(*number)+=5;
			}
			glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
		}
		else if (p.x > x + xx*i + 150 + 35 && p.x < x + xx*i + 150 + 40 && p.y > y + yy*j + 60 + line*PanelSpacing - 4 && p.y < y + yy*j + 60 + line*PanelSpacing + 2)
		{
			glColor4f((GLfloat)color0[0] / 255, (GLfloat)color0[1] / 255, (GLfloat)color0[2] / 255, 1.0f);
			DrawPanelFill(x + xx*i + 150 + 35, y + yy*j + 60 + line*PanelSpacing - 4, 5, 6);
			if (!isMouseClicked && (*number) - 4 > lowerBound) {
				isMouseClicked = true;
				(*number)-=5;
			}
			glColor4f((GLfloat)panelBoxColor, (GLfloat)panelBoxColor, (GLfloat)panelBoxColor, 1.0f);
		}
	}

	DrawPanelBox(x + xx*i + 150, y + yy*j + 48 + line*PanelSpacing, 29, 14);
	snprintf(buf, 32, "%3d", *number);
	glRasterPos2f(x + xx*i + 152, y + yy*j + 60 + line*PanelSpacing);
	glCallLists(strlen(buf), GL_UNSIGNED_BYTE, buf);
	DrawPanelBox(x + xx*i + 150 + 29, y + yy*j + 60 + line*PanelSpacing - 12, 6, 7); //1 Up
	DrawPanelBox(x + xx*i + 150 + 29, y + yy*j + 60 + line*PanelSpacing - 5, 6, 7); //1 Down
	DrawPanelBox(x + xx*i + 150 + 35, y + yy*j + 60 + line*PanelSpacing - 12, 6, 7); //5 Up 
	DrawPanelBox(x + xx*i + 150 + 35, y + yy*j + 60 + line*PanelSpacing - 5, 6, 7); //5 Down
	glRasterPos2f(x + xx*i + 194, y + yy*j + 60 + line*PanelSpacing);
	glCallLists(strlen(unit), GL_UNSIGNED_BYTE, unit);
}

void DrawPanel() {
	
	if ((GetKeyState(VK_LBUTTON) & 0x8000) == 0 && isMouseClicked)
		isMouseClicked = false;

	x = (float)(c_Rect.right - c_Rect.left) / 2 - PanelX / 2;
	y = (float)(c_Rect.bottom - c_Rect.top) / 2 - PanelY / 2;

	glBegin(GL_QUADS);
	glColor4f((GLfloat)color0[0]/255, (GLfloat)color0[1]/255, (GLfloat)color0[2]/255, 1.0f);
	glVertex2f(x, y);
	glVertex2f(x, y+ PanelY);
	glVertex2f(x+ PanelX, y+ PanelY);
	glVertex2f(x+ PanelX, y);
	glEnd();
	glBegin(GL_QUADS);
	glColor4f((GLfloat)color1[0] / 255, (GLfloat)color1[1] / 255, (GLfloat)color1[2] / 255, 1.0f);
	glVertex2f(x + 10, y +30 );
	glVertex2f(x + 10, y + PanelY - 10);
	glVertex2f(x + PanelX - 10, y + PanelY -10);
	glVertex2f(x + PanelX-10, y+30);
	glEnd();
	glColor4f((GLfloat)1, (GLfloat)1, (GLfloat)1, 1.0f);
	glRasterPos2f(x + 10, y + 20);
	glCallLists(44, GL_UNSIGNED_BYTE, "TekHak for Counter-Strike: Global Offensive");
	glEnd();

	glLineWidth(1);
	for (int j = 0;j < 2;j++) {
		for (int i = 0;i < 3;i++) {
			glColor4f((GLfloat)1, (GLfloat)1, (GLfloat)1, 1.0f);
			DrawPanelStaticBox(x + xx*i + 20, x + xx*(i + 1) + 10, y + yy*j + 40, y + yy*(j + 1) + 30);

			if (i == 0 && j == 0) {
				DoCheckBox(&isTrigger, "Auto Shoot", i, j, 0);
				DoCheckNumberBox(&isTriggerToggle, &triggerToggleKey, "Toggle Key", i, j, 1);
				if (!isTrigger)
					panelBoxColor = 0.75f;
				DoNumberChangeBox(&triggerDelay, "Delay", "ms", 0, 999, i, j, 2);
				DoCheckBox(&isTriggerFriendly, "Friendly Fire", i, j, 3);
				DoCheckNumberBox(&isTriggerHold, &triggerHoldKey, "Hold Key", i, j, 4);
				panelBoxColor = 1.0f;

				DoCheckBox(&isRcs, "Recoil Control", i, j, 6);
				DoCheckNumberBox(&isTriggerToggle, &triggerToggleKey, "Toggle Key", i, j, 7);
				if (!isRcs)
					panelBoxColor = 0.75f;
				DoNumberChangeBox(&rcsAmount, "Control Amount", "Percent", 0, 200, i, j, 8);
				DoCheckBox(&isRcsCross, "Crosshair", i, j, 9);
				panelBoxColor = 1.0f;
			}
			else if (i == 1 && j == 0) {
				DoCheckBox(&isAimbot, "Aimbot", i, j, 0);
				DoCheckNumberBox(&isAimbotToggle, &aimbotToggleKey, "Toggle Key", i, j, 1);
				if(!isAimbot)
					panelBoxColor = 0.75f;
				int boneId[] = { 6, 5, 4, 3, 2, 0 };
				char *boneName[] = { "Head", "Neck", "Chest", "Belly", "Waist", "Hip" };
				DoNumberCycleBox(&aimBone, "Aim Position", boneId, boneName, 6, i, j, 2);
				DoCheckBox(&isAimbotFriendly, "Friendly", i, j, 3);
				DoNumberChangeBox(&aimSmooth, "Smooth Factor", "", 0, 100, i, j, 4);
				if(!isHoldAim)
					panelBoxColor = 0.75f;
				DoCheckNumberBox(&isHoldAim, &aimbotHoldKey, "Hold Key", i, j, 5);
				DoCheckBox(&isAimWall, "Through Wall", i, j, 6);
				panelBoxColor = 1.0f;

				DoCheckBox(&isBhop, "Bunny Hop", i, j, 8);
				DoCheckNumberBox(&isBhopToggle, &bhopToggleKey, "Toggle Key", i, j, 9);
				if (!isBhop)
					panelBoxColor = 0.75f;
				DoNumberBox(&bhopGameBind, "Jump Key", i, j, 10);
				DoNumberBox(&bhopJumpBind, "Bhop Key", i, j, 11);
				panelBoxColor = 1.0f;
			}
			else if (i == 2 && j == 0) {
				DoCheckBox(&isWall, "Wall", i, j, 0);
				DoCheckNumberBox(&isWallToggle, &wallToggleKey, "Toggle Key", i, j, 1);
				if (!isWall)
					panelBoxColor = 0.75f;
				DoCheckBox(&isWallInternal, "Internal", i, j, 2);
				DoCheckBox(&isWallExternal, "External", i, j, 3);
				DoCheckBox(&isWallFriendly, "Friendly", i, j, 4);
				DoCheckBox(&isBox, "Player Box", i, j, 5);
				DoCheckBox(&isHealth, "Player Health", i, j, 6);
				DoCheckBox(&isName, "Player Name", i, j, 7);
				DoCheckBox(&isWeapon, "Player Weapon", i, j, 8);
				if(!isWallInternal)
					panelBoxColor = 0.75f;
				DoCheckBox(&isBomb, "Bomb", i, j, 9);
				DoCheckBox(&isChicken, "Chicken", i, j, 10);
				panelBoxColor = 1.0f;
			}
			else if (i == 0 && j == 1) {
				DoCheckBox(&isRadar, "Radar", i, j, 0);
				DoCheckNumberBox(&isRadarToggle, &radarToggleKey, "Toggle Key", i, j, 1);

				DoCheckBox(&isFlash, "No Flash", i, j, 3);
				DoCheckNumberBox(&isFlashToggle, &flashToggleKey, "Toggle Key", i, j, 4);

				DoCheckBox(&isInfo, "Info", i, j, 6);
				DoCheckNumberBox(&isInfoToggle, &infoToggleKey, "Toggle Key", i, j, 7);
				if (!isInfo)
					panelBoxColor = 0.75f;
				DoNumberChangeBox(&infoX, "X-Coord", "pixels", 0, 1920, i, j, 8);
				DoNumberChangeBox(&infoY, "Y-Coord", "pixels", 0, 1080, i, j, 9);
				panelBoxColor = 1.0f;
			}
			else if (i == 1 && j == 1) {
				DoCheckBox(&isRage, "Rage Bot", i, j, 0);
				DoCheckNumberBox(&isRageToggle, &rageToggleKey, "Toggle Key", i, j, 1);
				if (!isRage)
					panelBoxColor = 0.75f;
				DoCheckBox(&isRageFriendly, "Friendly", i, j, 2);
				panelBoxColor = 1.0f;
			}
			else if (i == 2 && j == 1) {
				DoNumberBox(&panelKey, "Panel Key", i, j, 0);
				DoNumberBox(&panicKey, "Panic Key", i, j, 1);
			}
		}
	}
	glLineWidth(2);
	std::string buttons[3] = { "Save Configuration", "Load Previous Save", "Reset to Default" };
	for (int i = 0; i < 3; i++) {
		POINT p;
		if (GetCursorPos(&p) && p.x > x + xx*i + 20 && p.x < x + xx*(i + 1) + 10 && p.y > y + yy * 2 + 40 && p.y < y + yy * 2 + 70 && (GetKeyState(VK_LBUTTON) & 0x100) != 0)
		{
			glBegin(GL_QUADS);
			glColor4f((GLfloat)color0[0] / 255, (GLfloat)color0[1] / 255, (GLfloat)color0[2] / 255, 1.0f);
			glVertex2f(x + xx*i + 21, y + yy * 2 + 41);
			glVertex2f(x + xx*i + 21, y + yy * 2 + 69);
			glVertex2f(x + xx*(i + 1) + 9, y + yy * 2 + 69);
			glVertex2f(x + xx*(i + 1) + 9, y + yy * 2 + 41);
			glEnd();

			if (!isMouseClicked) {
				isMouseClicked = true;
				if (i == 0) //Save
					saveConfig();
				else if (i == 1) //Load
					loadConfig();
				else if (i == 2) //Reset
					resetConfig();
			}
		}

		glBegin(GL_LINE_LOOP);
		glColor4f((GLfloat)1, (GLfloat)1, (GLfloat)1, 1.0f);
		glVertex2f(x + xx*i + 20, y + yy*2 + 40);
		glVertex2f(x + xx*i + 20, y + yy*2 + 70);
		glVertex2f(x + xx*(i + 1) + 10, y + yy*2 + 70);
		glVertex2f(x + xx*(i + 1) + 10, y + yy*2 + 40);
		glEnd();

		glColor4f((GLfloat)1, (GLfloat)1, (GLfloat)1, 1.0f);
		glRasterPos2f(x + xx*i + 60, y + yy*2 + 60);
		glCallLists(strlen(buttons[i].c_str()), GL_UNSIGNED_BYTE, buttons[i].c_str());
		glEnd();
	}
}

/*void DrawBone(int first, int second, int playerId) {
	float firstBone[3];
	EntityList.GetBonePosition(playerId, firstBone, first);
	float secondBone[3];
	EntityList.GetBonePosition(playerId, secondBone, second);

	float firstScreen[2];
	float secondScreen[2];
	if (WorldToScreen(firstBone, firstScreen) && WorldToScreen(secondBone, secondScreen)) {
		DrawSnaplines(firstScreen[0], firstScreen[1], secondScreen[0], secondScreen[1]);
	}
}*/

LRESULT APIENTRY WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_CREATE:
	{
		MainHDC = GetDC(hWnd);
		SetupGL();
		return 0;
	}
	case WM_CLOSE:
	{
		DestroyWindow(hWnd);
	}
	case WM_DESTROY:
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(MainHGLRC);
		DeleteDC(MainHDC);
		ReleaseDC(hWnd, MainHDC);
		PostQuitMessage(0);
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		glClear(GL_COLOR_BUFFER_BIT);

		if (panelEnabled) {
			EnableWindow(csgo, false);
			DrawPanel();
		} else
			EnableWindow(csgo, true);

		if(isRunning) {
			if (isInfo) {
				float vector[3];
				char buf[35] = { 0 };
				glColor3f(204, 204, 0);
				NewPlayer.GetPosition(vector);
				snprintf(buf, 35, "Position: %5.2f %5.2f %4.2f", vector[0], vector[1], vector[2]);
				glRasterPos2f(infoX, infoY);
				glCallLists(35, GL_UNSIGNED_BYTE, buf);
				NewPlayer.GetAngles(vector);
				snprintf(buf, 35, "Angle: %3.2f %4.2f %4.2f     ", vector[0], vector[1], vector[2]);
				glRasterPos2f(infoX, infoY + 15);
				glCallLists(27, GL_UNSIGNED_BYTE, buf);
				NewPlayer.GetVelocity(vector);
				float velocity = sqrtf(vector[0] * vector[0] + vector[1] * vector[1]);
				snprintf(buf, 35, "Velocity: %3.2f  ", velocity);
				glRasterPos2f(infoX, infoY + 30);
				glCallLists(16, GL_UNSIGNED_BYTE, buf);
			}
			if (isRcs && isRcsCross) {
				float CrosshairPos2ScreenX = SWidth / 2;
				float CrosshairPos2ScreenY = SHeight / 2;
				float punch[3];
				NewPlayer.GetPunch(punch);
				float Punch2ScreenX = (SWidth / 358.f) * (punch[0] * 2);
				float Punch2ScreenY = (SHeight / 178.f) * (punch[1] * 2);
				CrosshairPos2ScreenX -= Punch2ScreenY;
				CrosshairPos2ScreenY += Punch2ScreenX;
				int CrossColor[3] = { 122, 122, 0 };
				DrawCross(CrosshairPos2ScreenX, CrosshairPos2ScreenY, 6, 6, 2, CrossColor);
			}
			DWORD pointerGlow = Mem.Read<DWORD>(modClient.dwBase + DwGlow);
			int objCount = Mem.Read<int>(modClient.dwBase + DwGlow + 0x4);
			if (pointerGlow != NULL  && isWall) {
				for (int i = 0; i < objCount; i++) {
					DWORD mObj = pointerGlow + i * sizeof(GlowObjectDefinition_t);
					GlowObjectDefinition_t glowObj = Mem.ReadNew<GlowObjectDefinition_t>(mObj);
					if (glowObj.pEntity != NULL) {
						int maxPlayer;
						DWORD ClientState = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
						ReadProcessMemory(handle, (LPVOID)(ClientState + 0x2F0), &maxPlayer, sizeof(int), NULL);
						for (int j = 0; j < maxPlayer; j++) {
							if (EntityList.GetBaseEntity(j) == 0x0)
								continue;
							if (glowObj.pEntity == EntityList.GetBaseEntity(j) && glowObj.pEntity != NewPlayer.GetDwLocalPlayer()) {
								if (!EntityList.IsDead(j) && !EntityList.IsDormant(j)) {
									float me3[4];
									float en3[4];
									EntityList.GetBonePosition(j, en3, 6);
									NewPlayer.GetPosition(me3);
									me3[2] += NewPlayer.GetViewOrigin();
									int color[4];
									if (!isWallFriendly && EntityList.GetTeam(j) == NewPlayer.GetTeam()) {
										color[0] = 0;
										color[1] = 0;
										color[2] = 255;
									}
									else if (EntityList.GetBaseEntity(j) == EntityList.GetBaseEntity(NewPlayer.GetCrosshairId())) {
										color[0] = 127;
										color[1] = 0;
										color[2] = 0;
									}
									else if (bsp.Visible(me3, en3)) {
										color[0] = 0;
										color[1] = 255;
										color[2] = 0;
									}
									else {
										color[0] = 255;
										color[1] = 0;
										color[2] = 0;
									}
									glColor4f(color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f, 1.0f);
									float boxPos[4];
									char chModel[64];
									DWORD model = Mem.Read<DWORD>(glowObj.pEntity + 0x6C);
									for (int i = 0;i < 64;i++)
										chModel[i] = Mem.Read<char>(model + 0x4 + i);

									int boneCount = -1;

									float bone3D[100][3];
									float bone2D[100][2];

									if (strstr(chModel, "ctm_fbi"))
										boneCount = ctm_fbi.size();
									else if (strstr(chModel, "ctm_gign"))
										boneCount = ctm_gign.size();
									else if (strstr(chModel, "ctm_gsg9"))
										boneCount = ctm_gsg9.size();
									else if (strstr(chModel, "ctm_idf"))
										boneCount = ctm_idf.size();
									else if (strstr(chModel, "ctm_sas"))
										boneCount = ctm_sas.size();
									else if (strstr(chModel, "ctm_st6"))
										boneCount = ctm_st6.size();
									else if (strstr(chModel, "ctm_swat"))
										boneCount = ctm_swat.size();
									else if (strstr(chModel, "tm_anarchist"))
										boneCount = tm_anarchist.size();
									else if (strstr(chModel, "tm_balkan"))
										boneCount = tm_balkan.size();
									else if (strstr(chModel, "tm_leet"))
										boneCount = tm_leet.size();
									else if (strstr(chModel, "tm_phoenix"))
										boneCount = tm_phoenix.size();
									else if (strstr(chModel, "tm_pirate"))
										boneCount = tm_pirate.size();
									else if (strstr(chModel, "tm_professional"))
										boneCount = tm_professional.size();
									else if (strstr(chModel, "tm_separatist"))
										boneCount = tm_separatist.size();

									for (int p = 0;p < 4;p++) {
										if (p >= 2)
											boxPos[p] = 0.0f;
										else
											boxPos[p] = std::numeric_limits<float>::max();
									}

									for (int b = 0; b < boneCount; b++)
									{
										EntityList.GetBonePosition(j, bone3D[b], b);
										if (!WorldToScreen(bone3D[b], bone2D[b]))
											continue;
										int parentId = -1;

										if (strstr(chModel, "ctm_fbi"))
											parentId = ctm_fbi[b];
										else if (strstr(chModel, "ctm_gign"))
											parentId = ctm_gign[b];
										else if (strstr(chModel, "ctm_gsg9"))
											parentId = ctm_gsg9[b];
										else if (strstr(chModel, "ctm_idf"))
											parentId = ctm_idf[b];
										else if (strstr(chModel, "ctm_sas"))
											parentId = ctm_sas[b];
										else if (strstr(chModel, "ctm_st6"))
											parentId = ctm_st6[b];
										else if (strstr(chModel, "ctm_swat"))
											parentId = ctm_swat[b];
										else if (strstr(chModel, "tm_anarchist"))
											parentId = tm_anarchist[b];
										else if (strstr(chModel, "tm_balkan"))
											parentId = tm_balkan[b];
										else if (strstr(chModel, "tm_leet"))
											parentId = tm_leet[b];
										else if (strstr(chModel, "tm_phoenix"))
											parentId = tm_phoenix[b];
										else if (strstr(chModel, "tm_pirate"))
											parentId = tm_pirate[b];
										else if (strstr(chModel, "tm_professional"))
											parentId = tm_professional[b];
										else if (strstr(chModel, "tm_separatist"))
											parentId = tm_separatist[b];

										if (parentId == -1)
											continue;

										EntityList.GetBonePosition(j, bone3D[b], b);
										if (!WorldToScreen(bone3D[b], bone2D[b]))
											continue;

										if (isWallExternal)
											DrawSnaplines(bone2D[b][0], bone2D[b][1], bone2D[parentId][0], bone2D[parentId][1]);

										if (bone2D[b][0] < boxPos[0] && bone2D[b][0]>0.0f)
											boxPos[0] = bone2D[b][0];
										else if (bone2D[parentId][0] < boxPos[0] && bone2D[parentId][0]>0.0f)
											boxPos[0] = bone2D[parentId][0];

										if (bone2D[b][0] > boxPos[2])
											boxPos[2] = bone2D[b][0];
										else if (bone2D[parentId][0] > boxPos[2])
											boxPos[2] = bone2D[parentId][0];

										if (bone2D[b][1] < boxPos[1] && bone2D[b][1]>0.0f)
											boxPos[1] = bone2D[b][1];
										else if (bone2D[parentId][1] < boxPos[1] && bone2D[parentId][1]>0.0f)
											boxPos[1] = bone2D[parentId][1];

										if (bone2D[b][1] > boxPos[3])
											boxPos[3] = bone2D[b][1];
										else if (bone2D[parentId][1] > boxPos[3])
											boxPos[3] = bone2D[parentId][1];
									}
									if (isWallFriendly || EntityList.GetTeam(j) != NewPlayer.GetTeam()) {
										float PlayerPos[3];
										NewPlayer.GetPosition(PlayerPos);
										float EnemyPos[3];
										EntityList.GetPosition(j, EnemyPos);
										float EnemyXY[3];

										if (WorldToScreen(EnemyPos, EnemyXY))
										{
											int BoxColor[3] = { 254, 0, 0 };
											float HealthBarWidth = (boxPos[2] - boxPos[0]) / 5;
											float HealthBarHeight = (boxPos[3] - boxPos[1]) / 5;
											if (isBox)
												DrawBox(boxPos[0] - HealthBarWidth, boxPos[1] - HealthBarHeight, boxPos[2] + HealthBarWidth, boxPos[3] + HealthBarHeight, BoxColor);
											if (isHealth) {
												int HealthBarBackColor[3] = { 254, 0, 0 };
												int HealthBarColor[3] = { 0, 254, 0 };
												float HealthHeight = ((boxPos[3] + HealthBarHeight) - (boxPos[1] - HealthBarHeight))*((100.0f - EntityList.GetHealth(j)) / 100.0f);
												DrawBox(boxPos[2] + HealthBarWidth, boxPos[3] + HealthBarHeight, boxPos[2] + HealthBarWidth*2, boxPos[1] - HealthBarHeight, BoxColor);
												DrawFilledBox(boxPos[2] + HealthBarWidth, boxPos[3] + HealthBarHeight, boxPos[2] + HealthBarWidth * 2, boxPos[1] - HealthBarHeight, HealthBarBackColor);
												DrawFilledBox(boxPos[2] + HealthBarWidth, boxPos[3] + HealthBarHeight, boxPos[2] + HealthBarWidth * 2, (boxPos[1] - HealthBarHeight) + (HealthHeight), HealthBarColor);
											}

											if (isName) {
												char name[32];
												bool null = false;
												EntityList.GetName(j, name);
												for (int i = 0; i < 32;i++) {
													if (name[i] == 0)
														null = true;
													if (null)
														name[i] = 0;
												}

												glColor3f(204, 204, 0);
												glRasterPos2f(boxPos[0] - HealthBarWidth, boxPos[1] - HealthBarHeight - 4);
												glCallLists(32, GL_UNSIGNED_BYTE, name);
											}
											if (isWeapon) {
												char weapon[32];
												bool null = false;
												int weaponId = EntityList.GetWeaponId(j);
												//_itoa(weaponId, weapon, 10);
												if (weaponId == 1)
													strcpy(weapon, "Desert Eagle");
												else if(weaponId == 2)
													strcpy(weapon, "Dual Berettas");
												else if (weaponId == 3)
													strcpy(weapon, "Five-SeveN");
												else if (weaponId == 4)
													strcpy(weapon, "Glock-18");
												else if (weaponId == 7)
													strcpy(weapon, "AK-47");
												else if (weaponId == 8)
													strcpy(weapon, "AUG");
												else if (weaponId == 9)
													strcpy(weapon, "AWP");
												else if (weaponId == 10)
													strcpy(weapon, "FAMAS");
												else if (weaponId == 11)
													strcpy(weapon, "G3SG1");
												else if (weaponId == 13)
													strcpy(weapon, "Galil AR");
												else if (weaponId == 14)
													strcpy(weapon, "M249");
												else if (weaponId == 16)
													strcpy(weapon, "M4A4");
												else if (weaponId == 17)
													strcpy(weapon, "MAC-10");
												else if (weaponId == 19)
													strcpy(weapon, "P90");
												else if (weaponId == 24)
													strcpy(weapon, "UMP-45");
												else if (weaponId == 25)
													strcpy(weapon, "XM1014");
												else if (weaponId == 26)
													strcpy(weapon, "PP-Bizon");
												else if (weaponId == 27)
													strcpy(weapon, "MAG-7");
												else if (weaponId == 28)
													strcpy(weapon, "Negev");
												else if (weaponId == 29)
													strcpy(weapon, "Sawed-Off");
												else if (weaponId == 30)
													strcpy(weapon, "Tec-9");
												else if (weaponId == 31)
													strcpy(weapon, "Zeus x27");
												else if (weaponId == 32)
													strcpy(weapon, "P2000");
												else if (weaponId == 33)
													strcpy(weapon, "MP7");
												else if (weaponId == 34)
													strcpy(weapon, "MP9");
												else if (weaponId == 35)
													strcpy(weapon, "Nova");
												else if (weaponId == 36)
													strcpy(weapon, "P250");
												else if (weaponId == 38)
													strcpy(weapon, "SCAR-20");
												else if (weaponId == 39)
													strcpy(weapon, "SG 553");
												else if (weaponId == 40)
													strcpy(weapon, "SSG 08");
												else if (weaponId == 42)
													strcpy(weapon, "Knife");
												else if (weaponId == 43)
													strcpy(weapon, "Flashbang");
												else if (weaponId == 44)
													strcpy(weapon, "High Explosive Grenade");
												else if (weaponId == 45)
													strcpy(weapon, "Smoke Grenade");
												else if (weaponId == 46)
													strcpy(weapon, "Molotov");
												else if (weaponId == 47)
													strcpy(weapon, "Decoy Grenade");
												else if (weaponId == 48)
													strcpy(weapon, "Incendiary Grenade");
												else if (weaponId == 49)
													strcpy(weapon, "C4 Explosive");
												else if (weaponId == 59)
													strcpy(weapon, "Knife");
												else if (weaponId == 60)
													strcpy(weapon, "M4A1-S");
												else if (weaponId == 61)
													strcpy(weapon, "USP-S");
												else if (weaponId == 63)
													strcpy(weapon, "CZ75-Auto");
												else if (weaponId == 500)
													strcpy(weapon, "Bayonet");
												else if (weaponId == 505)
													strcpy(weapon, "Flip Knife");
												else if (weaponId == 506)
													strcpy(weapon, "Gut Knife");
												else if (weaponId == 507)
													strcpy(weapon, "Karambit");
												else if (weaponId == 508)
													strcpy(weapon, "M9 Bayonet");
												else if (weaponId == 509)
													strcpy(weapon, "Huntsman Knife");
												else if (weaponId == 512)
													strcpy(weapon, "Falchion Knife");
												else if (weaponId == 515)
													strcpy(weapon, "Butterfly Knife");
												else if (weaponId == 516)
													strcpy(weapon, "Shadow Daggers");

												glColor3f(204, 204, 0);
												glRasterPos2f(boxPos[0] - HealthBarWidth, boxPos[3] + HealthBarHeight + 12);
												glCallLists(strlen(weapon), GL_UNSIGNED_BYTE, weapon);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		SwapBuffers(MainHDC);
		EndPaint(hWnd, &ps);
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

DWORD WINAPI MainWindow(LPVOID PARAMS)
{
	CurrentInstance = GetModuleHandle(NULL);
	MSG Msg;
	WNDCLASSEX WClass;
	WClass.cbSize = sizeof(WNDCLASSEX);
	WClass.style = 0;
	WClass.lpfnWndProc = WndProc;
	WClass.cbClsExtra = 0;
	WClass.cbWndExtra = 0;
	WClass.hInstance = CurrentInstance;
	WClass.hIcon = NULL;
	WClass.hCursor = NULL;
	WClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	WClass.lpszMenuName = NULL;
	WClass.lpszClassName = WName;
	WClass.hIconSm = NULL;
	if (!RegisterClassEx(&WClass))
	{
		std::cout << "RegisterClassEx";
		ExitThread(0);
	}
	EspHWND = CreateWindowEx(WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED, WName, WName, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, m_Rect.left, m_Rect.top - 1, (int)SWidth, (int)SHeight + 1, NULL, NULL, CurrentInstance, NULL);
	if (EspHWND == NULL)
	{
		std::cout << "EspHWND";
		ExitThread(0);
	}

	if (!SetLayeredWindowAttributes(EspHWND, RGB(0, 0, 0), 255, LWA_COLORKEY | LWA_ALPHA))
	{
		std::cout << "Layered";
		ExitThread(0);
	}

	BOOL IsEnabled;
	if (DwmIsCompositionEnabled(&IsEnabled) != S_OK)
	{
		std::cout << "CompEnabled";
		ExitThread(0);
	}

	if (!IsEnabled)
	{
		std::cout << "Enabled";
		ExitThread(0);
	}

	DWM_BLURBEHIND bb = { DWM_BB_ENABLE | DWM_BB_BLURREGION, true, CreateRectRgn(0, 0, -1, -1), true };
	if (DwmEnableBlurBehindWindow(EspHWND, &bb) != S_OK)
	{
		std::cout << "blur";
		ExitThread(0);
	}
	ShowWindow(EspHWND, 1);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	ExitThread(0);
}

DWORD WINAPI RedrawLoop(LPVOID PARAMS)
{
	while (true)
	{
		ResizeWindow();
		InvalidateRect(EspHWND, NULL, false);
		Sleep(1);
	}
	ExitThread(0);
}

/*struct Ray_t
{
	Vector m_Start; float w1;
	Vector m_Delta; float w2;
	Vector m_StartOffset; float w3;
	Vector m_Extents; float w4;

	bool m_IsRay;
	bool m_IsSwept;

	void Init(Vector const& start, Vector const& end)
	{
		VectorSubtract(end, start, m_Delta);

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		VectorClear(m_Extents);
		m_IsRay = true;

		VectorClear(m_StartOffset);
		VectorCopy(start, m_Start);
	}

	void Init(Vector const& start, Vector const& end, Vector const& mins, Vector const& maxs)
	{
		VectorSubtract(end, start, m_Delta);

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		VectorSubtract(maxs, mins, m_Extents);
		m_Extents *= 0.5f;
		m_IsRay = (m_Extents.LengthSqr() < 1e-6);

		VectorAdd(mins, maxs, m_StartOffset);
		m_StartOffset *= 0.5f;
		VectorAdd(start, m_StartOffset, m_Start);
		m_StartOffset *= -1.f;
	}
};

struct csurface_t
{
	const char *name;
	short surfaceProps;
	unsigned short flags;
};

struct cplane_t
{
	Vector normal;
	float dist;
	byte type;
	byte signbits;
	byte pad[2];
};

class CBaseTrace
{
public:
	Vector startpos;
	Vector endpos;
	cplane_t plane;
	float fraction;
	int contents;
	unsigned short dispFlags;
	bool allsolid;
	bool startsolid;
};

enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,				// NOTE: This does *not* test static props!!!
	TRACE_ENTITIES_ONLY,			// NOTE: This version will *not* test static props
	TRACE_EVERYTHING_FILTER_PROPS,	// NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
};


class CGameTrace : public CBaseTrace
{
public:
	float fractionleftsolid;
	csurface_t surface;
	int hitgroup;
	short physicsbone;
	DWORD m_pEnt;
	int hitbox;
};

typedef CGameTrace trace_t;

class ITraceFilter
{
public:
	virtual bool            ShouldHitEntity(DWORD pBaseEntity, int mask) = 0;
	virtual TraceType_t            GetTraceType() const = 0;
};

class CTraceFilter : public ITraceFilter
{
public:
	CTraceFilter()
	{
		m_pPassEnt = NewPlayer.GetDwLocalPlayer();
	}

	bool ShouldHitEntity(DWORD pHandleEntity, int contentsMask) {
		return NewPlayer.GetDwLocalPlayer() != m_pPassEnt;
	}
	TraceType_t GetTraceType() const {
		return TRACE_EVERYTHING; 
	}

	DWORD m_pPassEnt;
};

class IEngineTrace
{
public:
	virtual void TraceRay(const Ray_t &ray, unsigned int fMask, ITraceFilter *pTraceFilter, trace_t *pTrace) = 0;
};

IEngineTrace *enginetrace;

bool IsVisible(Vector Src, Vector Dst)
{
	Ray_t ray;
	ray.Init(Src, Dst);

	trace_t tr;
	CTraceFilter filter;

	//std::cout << std::hex << (enginetrace->TraceRay) << std::endl;
	typedef int TraceRay(const Ray_t &ray, unsigned int fMask, ITraceFilter *pTraceFilter, trace_t *pTrace);
	TraceRay* f = (TraceRay*)(Mem.Read<DWORD>(modClient.dwBase + 0x1C1226b0 + 32/*)));
	f(ray, 0x4600400B, &filter, &tr);

	return (tr.fraction > 0.97f); //97

}*/

void click() {
	mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL);
	Sleep(5);
	mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL);
}

void worldToAngle(float* myPos, float* enemyPos, float* ang) {
	float xD = enemyPos[0] - myPos[0];
	float yD = enemyPos[1] - myPos[1];
	if (xD < 0 && yD < 0)
		ang[1] = (atan(yD / xD) * (180 / 3.14159265f));
	else if(xD >= 0 && yD >=0)
		ang[1] = -180 + (atan(yD / xD) * (180 / 3.14159265f));
	else if(xD <= 0 && yD >=0)
		ang[1] = (atan(yD / xD) * (180 / 3.14159265f));
	else if(xD>0 && yD<0)
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


#define SELF_REMOVE_STRING  TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 5000 > Nul & Del \"%s\"")
#define SELF_OLDRENAME_STRING  TEXT("cmd.exe /C rename \"%s\" old.exe")
#define SELF_NEWRENAME_STRING  TEXT("cmd.exe /C rename update.exe TekHak.exe")
#define SELF_REMOVEUPDATE_STRING  TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 5000 > Nul & Del old.exe")
void destroy(char option)
{
	TCHAR szModuleName[MAX_PATH];
	TCHAR szCmd[2 * MAX_PATH];
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };

	GetModuleFileName(NULL, szModuleName, MAX_PATH);

	if(option==0)
		StringCbPrintf(szCmd, 2 * MAX_PATH, SELF_REMOVE_STRING, szModuleName);
	else if(option==1)
		StringCbPrintf(szCmd, 2 * MAX_PATH, SELF_OLDRENAME_STRING, szModuleName);
	else if (option == 2)
		StringCbPrintf(szCmd, 2 * MAX_PATH, SELF_NEWRENAME_STRING, szModuleName);
	else if (option == 3)
		StringCbPrintf(szCmd, 2 * MAX_PATH, SELF_REMOVEUPDATE_STRING, szModuleName);
	CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

bool login(char* motd) {
	/*char option =0 ;
	while (option != '1' && option != '2') {
		std::cout << "[1]Login [2]Register: ";
		std::cin >> option;
	}*/

	char username[33], password[33];
	std::cout << "username: ";
	std::cin.getline(username, 32);
	std::cout << "password: ";
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode = 0;
	GetConsoleMode(hStdin, &mode);
	SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
	std::cin.getline(password, 32);
	std::cout << std::endl;
	//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	if (strlen(username) == 0 || strlen(password)==0) {
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
		if ((username[i]>=33 && username[i]<=126) == false) {
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
	/*if (option == '2') {
		bool match = false;
		while (!match) {
			char temp[33];
			std::cout << "Retype password: ";
			//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cin.getline(temp, 32);
			std::cout << std::endl;
			match = (std::strcmp(password, temp) == 0);
		}
	}*/
	std::cout << std::endl;

	strcpy(password, md5(password).c_str());
	password[32] = 0;
	username[32] = 0;

	char options[2] = { '1', 0 };
	#define DEFAULT_BUFLEN 128
	char msg[DEFAULT_BUFLEN];
	strcpy(msg, options);
	strcat(msg, " ");
	strcat(msg, username);
	strcat(msg, " ");
	strcat(msg, password);
	strcat(msg, " ");
	strcat(msg, version);
	strcat(msg, " ");
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
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol);
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
			int j = 0;
			while (recvbuf[j + i + 3] != ' ') {
				if (recvbuf[j + i + 3] == '_')
					motd[j] = ' ';
				else
					motd[j] = recvbuf[j + i + 3];
				j++;
			}
			motd[j] = 0;
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

	//if (option == '1') {
		if (flag == '0') {
			std::cout << "Wrong username or password." << std::endl;
			return false;
		}
		else if (flag == '1') {
			std::cout << "Authorized. Enjoy your hacks." << std::endl;
			if (strcmp(currentVers, version) && strcmp("UNKNOWN", currentVers)) {
				SetConsoleMode(hStdin, mode);
				//char toUpdate = 0;
				std::cout << "\nUpdate v" << currentVers << " is going to be installed." << std::endl;
				/*while (toUpdate != '1' && toUpdate != '2') {
					std::cout << "Yes[1] No[2]: ";
					std::cin >> toUpdate;
				}*/
				//if (toUpdate == '1') {
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
						Sleep(1000);
						destroy(2);
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
						std::cout << "Finished Updating!" << std::endl;
						destroy(3);
						return false;
					} else
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
	//}
	/*else if (option == '2') {
		if (flag == '0') {
			std::cout << "Registration complete. Ask the administrator for approval." << std::endl;
		}
		else {
			std::cout << "Registration failed. Username already exists." << std::endl;
		}
		return false;
	}*/
	return false;
}

void v0();
void v1();
void v2();

char map[128];

void loadMap() {
	DWORD ClientState = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
	//ReadProcessMemory(handle, (LPVOID)(ClientState), map, sizeof(char) * 128, NULL);
	ReadProcessMemory(handle, (LPVOID)(ClientState + 0x26C), map, sizeof(char) * 128, NULL);

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
	//std::cout << tmp << std::endl;

	bsp.LoadBSP(tmp);
}

int main() {
	std::cout << "TekHak\nCounter Strike: Global Offensive Multi-Hack\nVersion " << version << "\nDeveloped by Kevin Park\n" << std::endl;
	
	char motd[128];
	if (!login(motd)) {
		Sleep(2500);
		exit(2);
	}

	resetConfig();
	loadConfig();

	std::cout << "Searching for csgo.exe..." << std::endl;
	while (!Mem.Attach("csgo.exe")) {
		std::cout << ".";
		Sleep(500);
	}
	std::cout << "csgo.exe Found" << std::endl;

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

	csgo = FindWindow(NULL, "Counter-Strike: Global Offensive");

	modClient = Mem.GetModule("client.dll");
	modEngine = Mem.GetModule("engine.dll");

	Sleep(300);
	
	std::cout << "\nSearching for the offsets" << std::endl;

	DWORD lpStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "x????xx????????x????xxxx", 24, 0xA3, 0, 0, 0, 0, 0xC7, 0x05, 0, 0, 0, 0, 0, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0x59, 0xC3, 0x6A, 0);
	DWORD lpP1 = Mem.Read<DWORD>(lpStart + 1);
	DwLocalPlayer = (lpP1 + 0x10) - modClient.dwBase;
	//std::cout << "0x" << std::hex << DwLocalPlayer << std::endl;

	DWORD elStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "x????xx?xxx", 11, 0x5, 0x0, 0x0, 0x0, 0x0, 0xC1, 0xE9, 0x0, 0x39, 0x48, 0x4);
	DWORD elP1 = Mem.Read<DWORD>(elStart + 1);
	BYTE elP2 = Mem.Read<BYTE>(elStart + 7);
	DwEntityList = (elP1 + elP2) - modClient.dwBase;
	//std::cout << "0x" << std::hex << DwEntityList << std::endl;

	DWORD epStart = Mem.FindPatternArr(modEngine.dwBase, modEngine.dwSize, "xxxxxxxx????xxxxxxxxxx????xxxx????xxx", 37, 0xF3, 0x0F, 0x5C, 0xC1, 0xF3, 0x0F, 0x10, 0x15, 0x0, 0x0, 0x0, 0x0, 0x0F, 0x2F, 0xD0, 0x76, 0x04, 0xF3, 0x0F, 0x58, 0xC1, 0xA1, 0x0, 0x0, 0x0, 0x0, 0xF3, 0x0F, 0x11, 0x80, 0x0, 0x0, 0x0, 0x0, 0xD9, 0x46, 0x04);
	DwEnginePointer = Mem.Read<DWORD>(epStart + 22) - modEngine.dwBase;
	DwViewAngle = Mem.Read<DWORD>(epStart + 30);
	//std::cout << "0x" << std::hex << DwViewAngle << std::endl;

	DWORD gpStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "x????xxx????xx????xx????xx", 26, 0xA1, 0, 0, 0, 0, 0xC7, 0x04, 0x02, 0, 0, 0, 0, 0x89, 0x35, 0, 0, 0, 0, 0x8D, 0xB7, 0, 0, 0, 0, 0xEB, 0x08);
	DwGlow = Mem.Read<DWORD>(gpStart + 1) - modClient.dwBase;
	//std::cout << "0x" << std::hex << DwGlow << std::endl;

	DWORD rpStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "x????xxxxxxx?xxx????xxxx????", 28, 0xA1, 0, 0, 0, 0, 0x8B, 0x0C, 0xB0, 0x8B, 0x01, 0xFF, 0x50, 0, 0x46, 0x3B, 0x35, 0, 0, 0, 0, 0x7C, 0xEA, 0x8B, 0x0D, 0, 0, 0, 0);
	DwRadarBase = Mem.Read<DWORD>(rpStart + 1) - modClient.dwBase;
	//std::cout << "0x" << std::hex << DwRadarBase << std::endl;

	DWORD vpStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "xxxxxxxxxxxxxxxxxxxxxxx????xxxxxx", 33, 0x53, 0x8B, 0xDC, 0x83, 0xEC, 0x08, 0x83, 0xE4, 0xF0, 0x83, 0xC4, 0x04, 0x55, 0x8B, 0x6B, 0x04, 0x89, 0x6C, 0x24, 0x04, 0x8B, 0xEC, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x81, 0xEC, 0x98, 0x03, 0x00, 0x00);
	DwViewMatrix = Mem.Read<DWORD>(vpStart + 0x4EE) +0x80 - modClient.dwBase;
	//std::cout << "0x" << std::hex << DwViewMatrix << std::endl;

	//DWORD ipStart = Mem.FindPatternArr(modEngine.dwBase, modEngine.dwSize, "xxxxx????xxxxx", 14, 0x89, 0x4D, 0xF4, 0x8B, 0x0D, 0, 0, 0, 0, 0x53, 0x56, 0x57, 0x8B, 0x01);
	//DwIGameResources = Mem.Read<DWORD>(ipStart + 0x5) - modEngine.dwBase;

	std::cout << "Offsets Found\n" << std::endl;

	std::cout << "Motd: " << motd << std::endl;

	Sleep(300);

	GetWindowRect(csgo, &m_Rect);
	GetClientRect(csgo, &c_Rect);

	CreateThread(0, 0x1002, &MainWindow, 0, 0, 0);
	CreateThread(0, 0x1001, &RedrawLoop, 0, 0, 0);

	std::thread V0(v0);
	std::thread V1(v1);
	std::thread V2(v2);

	int inGame;
	DWORD ClientState = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
	while (!isPanic) {
		ReadProcessMemory(handle, (LPVOID)(ClientState + 0xE8), &inGame, sizeof(int), NULL); //6: In Game 3: Loading
		if (inGame == 6 && !isRunning) {
			Sleep(5000);
			loadMap();
			isRunning = true;
		} else if (inGame != 6 && isRunning) {
			isRunning = false;
			Sleep(5000);
		}
		if (keyScanInProgess) {
			Sleep(10);
			continue;
		}
		if (GetAsyncKeyState(panicKey) & 0x8000)
		{
			std::cout << "Stopping the program" << std::endl;
			isPanic = true;
		}
		if (GetAsyncKeyState(panelKey) & 0x8000)
		{
			panelEnabled = !panelEnabled;
			while (GetAsyncKeyState(panelKey) & 0x8000)
				Sleep(1);
		}
		if (isBhopToggle && GetAsyncKeyState(bhopToggleKey) & 0x8000)
		{
			isBhop = !isBhop;
			while (GetAsyncKeyState(bhopToggleKey) & 0x8000)
				Sleep(1);
		}
		if (isTriggerToggle && GetAsyncKeyState(triggerToggleKey) & 0x8000)
		{
			isTrigger = !isTrigger;
			while(GetAsyncKeyState(triggerToggleKey) & 0x8000)
				Sleep(1);
		}
		if (isRcsToggle && GetAsyncKeyState(rcsToggleKey) & 0x8000)
		{
			isRcs = !isRcs;
			while (GetAsyncKeyState(rcsToggleKey) & 0x8000)
				Sleep(1);
		}
		if (isAimbotToggle && GetAsyncKeyState(aimbotToggleKey) & 0x8000)
		{
			isAimbot = !isAimbot;
			while (GetAsyncKeyState(aimbotToggleKey) & 0x8000)
				Sleep(1);
		}
		if (isWallToggle && GetAsyncKeyState(wallToggleKey) & 0x8000)
		{
			isWall = !isWall;
			while (GetAsyncKeyState(wallToggleKey) & 0x8000)
				Sleep(1);
		}
		if (isRageToggle && GetAsyncKeyState(rageToggleKey) & 0x8000)
		{
			isRage = !isRage;
			while (GetAsyncKeyState(rageToggleKey) & 0x8000)
				Sleep(1);
		}
		if (isRadarToggle && GetAsyncKeyState(radarToggleKey) & 0x8000)
		{
			isRadar = !isRadar;
			while (GetAsyncKeyState(radarToggleKey) & 0x8000)
				Sleep(1);
		}
		if(isInfoToggle && GetAsyncKeyState(infoToggleKey) & 0x8000)
		{
			isInfo = !isInfo;
			while (GetAsyncKeyState(infoToggleKey) & 0x8000)
				Sleep(1);
		}
		if (isFlashToggle && GetAsyncKeyState(flashToggleKey) & 0x8000)
		{
			isFlash = !isFlash;
			while (GetAsyncKeyState(flashToggleKey) & 0x8000)
				Sleep(1);
		}
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

#define THREADSLEEP 1

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
		if (!isRunning || panelEnabled || (!isBhop && !isFlash && !isTrigger)) {
			Sleep(250);
			continue;
		}
		if (isFlash) {
			if (NewPlayer.getFlashDuration() > 0.01f)
				NewPlayer.setFlashDuration(0.0f);
		}
		if (isBhop) {
			int flags = NewPlayer.GetFlags();
			if ((GetAsyncKeyState(bhopJumpBind) & 0x8000) && flags & 0x1 == 1) {
				keybd_event(bhopGameBind, MapVirtualKey(bhopGameBind, 0), 0, 0);
				Sleep(rand() % 10);
				keybd_event(bhopGameBind, MapVirtualKey(bhopGameBind, 0), KEYEVENTF_KEYUP, 0);
			}
		}
		if (isTrigger) {
			int MyTeam = NewPlayer.GetTeam();
			int CrossHairID = NewPlayer.GetCrosshairId();
			DWORD Enemy = Mem.Read<DWORD>(modClient.dwBase + DwEntityList + (CrossHairID * 0x10)); // CH = Crosshair.
			int EnemyHealth = Mem.Read<int>(Enemy + DwHealth); // Enemy in crosshair's 
			int EnemyTeam = Mem.Read<int>(Enemy + DwTeamNumber); // Enemy in crosshair's team, we need this to compare it to our own player's team)]

			if ((!isTriggerHold || GetAsyncKeyState(triggerHoldKey) & 0x8000) && (isTriggerFriendly || MyTeam != EnemyTeam) && NewPlayer.GetWeaponClip() > 0 && EnemyHealth > 0 && EnemyHealth <= 100)
			{
				Sleep(triggerDelay);
				click();
			}
		}
		Sleep(THREADSLEEP);
	}
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

void v1() {
	float oldAng[2];
	while (true) {
		if (isPanic)
			break;
		else if (!isRunning || panelEnabled)
			Sleep(250);
		else if (!isAimbot && !isRage) {
			int shotsFired = NewPlayer.GetShotsFired();
			if (isRcs && shotsFired > 1)
			{
				float MyPunch[3];
				NewPlayer.GetPunch(MyPunch);

				float CurrentAngle[3];
				NewPlayer.GetAngles(CurrentAngle);

				float TotalRCS[2];

				TotalRCS[0] = (MyPunch[0] - oldAng[0]) * (rcsAmount / 100.0f * 2.0f);
				TotalRCS[1] = (MyPunch[1] - oldAng[1]) * (rcsAmount / 100.0f * 2.0f);

				float viewAng[2] = { CurrentAngle[0] -= TotalRCS[0], CurrentAngle[1] -= TotalRCS[1] };
				angleFix(viewAng);

				NewPlayer.SetAngles(viewAng);

				oldAng[0] = MyPunch[0];
				oldAng[1] = MyPunch[1];
			}
			else
			{
				oldAng[1] = 0;
				oldAng[0] = 0;
			}
		}
		else {
			int maxPlayer;
			DWORD ClientState = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
			ReadProcessMemory(handle, (LPVOID)(ClientState + 0x2F0), &maxPlayer, sizeof(int), NULL);
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
						boneScreen[0] -= MyPunch[0] * (rcsAmount / 100.0f * 2.0f);
						boneScreen[1] -= MyPunch[1] * (rcsAmount / 100.0f * 2.0f);
						angleFix(boneScreen);
					}
					float currentAng[3];
					NewPlayer.GetAngles(currentAng);

					if (isRage && !isTrigger && !isAimbot) {
						if (bsp.Visible(myWorld, boneWorld) && EntityList.GetTeam(i) != 0 && (imT != enT || isRageFriendly) && !EntityList.IsDormant(i) && !EntityList.IsDead(i)) {
							NewPlayer.SetAngles(boneScreen);
							Sleep(5);
							if (EntityList.GetBaseEntity(i) == EntityList.GetBaseEntity(NewPlayer.GetCrosshairId())) {
								while (bsp.Visible(myWorld, boneWorld) && isRage && !EntityList.IsDormant(i) && !EntityList.IsDead(i) && NewPlayer.GetWeaponClip() > 0) {
									click();
									Sleep(8);
									EntityList.GetBonePosition(i, boneWorld, 6);
									NewPlayer.GetPosition(myWorld);
									myWorld[2] += NewPlayer.GetViewOrigin();
									worldToAngle(boneWorld, myWorld, boneScreen);

									if (isRcs) {
										float MyPunch[3];
										NewPlayer.GetPunch(MyPunch);
										boneScreen[0] -= MyPunch[0] * (rcsAmount / 100.0f * 2.0f);
										boneScreen[1] -= MyPunch[1] * (rcsAmount / 100.0f * 2.0f);
										angleFix(boneScreen);
									}

									NewPlayer.SetAngles(boneScreen);
									Sleep(5);
								}
							}
						}
					}
					else if (isAimbot) {
						if (fabs(boneScreen[0] - currentAng[0]) < 2.0f && fabs(boneScreen[1] - currentAng[1]) < 1.0f && (isAimWall || bsp.Visible(myWorld, boneWorld)) && ((isAimbotFriendly && NewPlayer.GetTeam() == EntityList.GetTeam(i) && imT == enT) || (imT != enT && NewPlayer.GetTeam() != EntityList.GetTeam(i)))) {
							float val0 = (boneScreen[0] - currentAng[0]) / (aimSmooth + 1), val1 = (boneScreen[1] - currentAng[1]) / (aimSmooth + 1);
							int aim = aimSmooth;
							while (fabs(boneScreen[0] - currentAng[0]) < 10.0f && fabs(boneScreen[1] - currentAng[1]) < 10.0f &&!EntityList.IsDead(i) && !EntityList.IsDormant(i) && (((isHoldAim && GetAsyncKeyState(aimbotHoldKey) & 0x8000)) || (NewPlayer.GetShotsFired() > 0 && NewPlayer.GetWeaponClip() > 0)) && (isAimWall || bsp.Visible(myWorld, boneWorld))) {
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
									boneScreen[0] -= MyPunch[0] * (rcsAmount / 100.0f * 2.0f);
									boneScreen[1] -= MyPunch[1] * (rcsAmount / 100.0f * 2.0f);
									angleFix(boneScreen);
								}
								NewPlayer.SetAngles(boneScreen);
								Sleep(10);
							}
						}
					}
				}
			}
		}
		Sleep(THREADSLEEP);
	}
}

void v2() {
	float glowThickness = 0.4f;

	while (true) {
		if (isPanic)
			break;
		else if (!isRunning || panelEnabled || (!isWall && !isRadar)) {
			Sleep(250);
			continue;
		}

		int maxPlayer;
		DWORD ClientState = Mem.Read<DWORD>(modEngine.dwBase + DwEnginePointer);
		ReadProcessMemory(handle, (LPVOID)(ClientState + 0x2F0), &maxPlayer, sizeof(int), NULL);
		if(isRadar && (!isWall || !isWallInternal))
			for (int j = 0; j < maxPlayer; j++) {
				if (EntityList.GetBaseEntity(j) == 0x0)
					continue;
				Mem.WriteNew<int>(EntityList.GetBaseEntity(j) + DwSpotted, 1);
			}
		else if(isWall && isWallInternal) {
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
						if (strstr(chModel, "ied") && isBomb) {
							glowObj.r = 127;
							glowObj.g = 127;
							glowObj.b = 0;
							glowObj.a = glowThickness;
							glowObj.m_bRenderWhenOccluded = true;
							glowObj.m_bRenderWhenUnoccluded = false;
							Mem.WriteNew<GlowObjectDefinition_t>(mObj, glowObj);
						}
						else if (strstr(chModel, "chicken") && isChicken) {
							glowObj.r = 255;
							glowObj.g = 255;
							glowObj.b = 255;
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
							//Vector me = { me3[0], me3[1], me3[2] };
							//Vector en = { en3[0], en3[1], en3[2] };

							if (!isWallFriendly && EntityList.GetTeam(glowObj.pEntity) == NewPlayer.GetTeam()) {
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
				Sleep(1);
			}
		}
	}
}