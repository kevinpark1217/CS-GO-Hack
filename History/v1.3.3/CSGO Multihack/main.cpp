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
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Ws2_32.lib")

#include "PMemory.h"
#include "main.h"
#include "md5.h"
#include "BSP.h"

PMemory Mem;
PModule modEngine, modClient;
const char* version = "1.3.3";

// Needs to be updated when counter strike is updated.
DWORD DwLocalPlayer, DwEntityList, DwEnginePointer, DwViewAngle, DwGlow, DwViewMatrix = 0x04A17AC4, DwRadarBase = 0x04A571DC;

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

	float GetViewOrigin()
	{
		float Vecview[3];
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		Mem.Read<float*>(DwLocalPlayer + DwVecViewOrigin, true, 3, Vecview);
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
	int GetWeaponId() {
		DWORD DwLocalPlayer = GetDwLocalPlayer();
		DWORD MyWeaponEntityID = (Mem.Read<DWORD>(DwLocalPlayer + DwActiveWeapon)) & 0xfff;
		DWORD MyWeaponEntity = Mem.Read<DWORD>(modClient.dwBase + DwEntityList + (MyWeaponEntityID /*- 1*/) * 0x10);
		return Mem.Read<int>(MyWeaponEntity + 0x162C);
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

	bool IsDead(int PlayerNumber)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<bool>(BaseEntity + DwLifeState);
	}

	bool IsDormant(int PlayerNumber)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<bool>(BaseEntity + DwIsDormant);
	}

	int GetTeam(int PlayerNumber)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<int>(BaseEntity + DwTeamNumber);
	}

	int GetFlash(int PlayerNumber)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<int>(BaseEntity + DwFlash);
	}

	int GetHealth(int PlayerNumber)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<int>(BaseEntity + DwHealth);
	}

	void GetVelocity(int PlayerNumber, float* Buffer)
	{
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		Mem.Read<float*>(BaseEntity + DwVecVelocity, true, 3, Buffer);
	}

	bool isSpotted(int PlayerNumber) {
		DWORD BaseEntity = GetBaseEntity(PlayerNumber);
		return Mem.Read<bool>(BaseEntity + DwSpotted);
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

}EntityList;

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

void DrawSnaplines(float x1, float y1, float x2, float y2, int* RGB)
{
	glLineWidth(3);
	glLineWidth(1);
	glBegin(GL_LINES);
	glColor4f((GLfloat)RGB[0], (GLfloat)RGB[1], (GLfloat)RGB[2], 1.0f);
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
	glLineWidth(3);
	glBegin(GL_LINE_LOOP);
	glColor4f((GLfloat)RGB[0], (GLfloat)RGB[1], (GLfloat)RGB[2], 1.0f);
	glVertex2f(x , y);
	glVertex2f(x , y2);
	glVertex2f(x2, y2);
	glVertex2f(x2, y);
	glEnd();
}

void DrawCheckBox(float x, float y, float x2, float y2, bool state, char* message) {
	glLineWidth(3);
	glBegin(GL_QUADS);
	int tColor[] = { 0, 255, 0 };
	int fColor[] = { 255, 0, 0 };
	if(state)
		glColor4f((GLfloat)tColor[0], (GLfloat)tColor[1], (GLfloat)tColor[2], 1.0f);
	else
		glColor4f((GLfloat)fColor[0], (GLfloat)fColor[1], (GLfloat)fColor[2], 1.0f);
	glVertex2f(x, y);
	glVertex2f(x, y2);
	glVertex2f(x2, y2);
	glVertex2f(x2, y);
	glEnd();
	glRasterPos2f(x2+10, y+10);
	glCallLists(24, GL_UNSIGNED_BYTE, message);
}

void DrawBone(int first, int second, int playerId) {
	float firstBone[3];
	EntityList.GetBonePosition(playerId, firstBone, first);
	float secondBone[3];
	EntityList.GetBonePosition(playerId, secondBone, second);

	float firstScreen[2];
	float secondScreen[2];
	if (WorldToScreen(firstBone, firstScreen) && WorldToScreen(secondBone, secondScreen)) {
		int SnapColor[3] = { 225, 225, 225 };
		DrawSnaplines(firstScreen[0], firstScreen[1], secondScreen[0], secondScreen[1], SnapColor);
	}
}

bool calcBox(float* box, int playerNum) {
	float head[3];
	float head2[2];
	float left[3];
	float left2[2];
	float right[3];
	float right2[2];
	float leftf[3];
	float leftf2[2];
	float rightf[3];
	float rightf2[2];
	float gun[3];
	float gun2[2];
	EntityList.GetBonePosition(playerNum, head, 10);
	EntityList.GetBonePosition(playerNum, left, 7);
	EntityList.GetBonePosition(playerNum, right, 13);
	EntityList.GetBonePosition(playerNum, gun, 50);
	EntityList.GetBonePosition(playerNum, rightf,39);
	EntityList.GetBonePosition(playerNum, leftf, 40);
	head[2] += 10;
	if (WorldToScreen(head, head2) && WorldToScreen(left, left2) && WorldToScreen(right, right2) && WorldToScreen(gun, gun2) && WorldToScreen(leftf, leftf2) && WorldToScreen(rightf, rightf2)) {
		box[1] = head2[1];
		if (left2[0] < right2[0])
			box[0] = left2[0];
		else
			box[0] = right2[0];
		if (leftf2[0] < box[0])
			box[0] = leftf2[0];
		if (rightf2[0] < box[0])
			box[0] = rightf2[0];
		if (gun2[0] < box[0])
			box[0] = gun2[0];

		if (left2[0] > right2[0])
			box[2] = left2[0];
		else
			box[2] = right2[0];
		if (leftf2[0] > box[2])
			box[2] = leftf2[0];
		if (rightf2[0] > box[2])
			box[2] = rightf2[0];
		if (gun2[0] > box[2])
			box[2] = gun2[0];

		if (left2[1] > right2[1])
			box[3] = left2[1];
		else
			box[3] = right2[1];
		if (leftf2[1] > box[3])
			box[3] = leftf2[1];
		if (rightf2[1] > box[3])
			box[3] = rightf2[1];
		if (gun2[1] > box[3])
			box[3] = gun2[1];
		return true;
	}
	return false;
}

bool isPanic = false;
bool isRunning = false;

bool isBhop = false;
bool isTrigger = false;
bool isRcs = false;
bool isRage = false;
bool isAimbot = false;
bool isGlow = false;

bool isRadar = false;
bool isBox = false;
bool isHealth = false;
bool isBone = false;;
bool isName = false;
bool isCross = false;

bool isInfo = false;
bool isFlash = false;

#define DRAWY 450
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

		DrawCheckBox(10, DRAWY , 20, DRAWY + 10, isBhop, "0: Bunny Hop            ");
		DrawCheckBox(10, DRAWY + 15, 20, DRAWY + 25, isTrigger, "1: TriggerBot           ");
		DrawCheckBox(10, DRAWY + 30, 20, DRAWY + 40, isRcs, "2: Recoil Control System");
		DrawCheckBox(10, DRAWY + 45, 20, DRAWY + 55, isRage, "3: Rage Bot             ");
		DrawCheckBox(10, DRAWY + 60, 20, DRAWY + 70, isAimbot, "4: Legit Aimbot         ");
		DrawCheckBox(10, DRAWY + 75, 20, DRAWY + 85, isGlow, "5: Wall                 ");
		DrawCheckBox(10, DRAWY + 90, 20, DRAWY + 100, isRadar, "6: Radar                ");
		DrawCheckBox(10, DRAWY + 105, 20, DRAWY + 115, isBox, "7: Box                  ");
		DrawCheckBox(10, DRAWY + 120, 20, DRAWY + 130, isHealth, "8: Health Bar           ");
		DrawCheckBox(10, DRAWY + 135, 20, DRAWY + 145, isBone, "9: Bone                 ");
		DrawCheckBox(10, DRAWY + 150, 20, DRAWY + 160, isName, "/: Name                 ");
		DrawCheckBox(10, DRAWY + 165, 20, DRAWY + 175, isCross, "*: Crosshair            ");
		DrawCheckBox(10, DRAWY + 180, 20, DRAWY + 190, isInfo, "-: Information          ");
		DrawCheckBox(10, DRAWY + 195, 20, DRAWY + 205, isFlash, "+: No Flash             ");

		if (isInfo) {
			float vector[3];
			char buf[35] = { 0 };
			glColor3f(204, 204, 0);
			NewPlayer.GetPosition(vector);
			snprintf(buf, 35, "Position: %5.2f %5.2f %4.2f", vector[0], vector[1], vector[2]);
			glRasterPos2f(10, DRAWY + 225);
			glCallLists(35, GL_UNSIGNED_BYTE, buf);
			NewPlayer.GetAngles(vector);
			snprintf(buf, 35, "Angle: %3.2f %4.2f %4.2f     ", vector[0], vector[1], vector[2]);
			glRasterPos2f(10, DRAWY + 240);
			glCallLists(27, GL_UNSIGNED_BYTE, buf);
			NewPlayer.GetVelocity(vector);
			float velocity = sqrtf(vector[0] * vector[0] + vector[1] * vector[1]);
			snprintf(buf, 35, "Velocity: %3.2f  ", velocity);
			glRasterPos2f(10, DRAWY + 255);
			glCallLists(16, GL_UNSIGNED_BYTE, buf);
		}

		DWORD pointerGlow = Mem.Read<DWORD>(modClient.dwBase + DwGlow);
		int objectCount = Mem.Read<int>(modClient.dwBase + DwGlow + 0x4);
		if (pointerGlow != NULL) {
			for (int i = 0; i < objectCount; i++) {
				DWORD mObj = pointerGlow + i * sizeof(GlowObjectDefinition_t);
				GlowObjectDefinition_t glowObj = Mem.ReadNew<GlowObjectDefinition_t>(mObj);

				if (glowObj.pEntity != NULL) {
					for (int j = 1; j < 64; j++) {
						if (glowObj.pEntity == EntityList.GetBaseEntity(j)) {

							if (isCross) {
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
							
							if (EntityList.GetTeam(j) != NewPlayer.GetTeam() && EntityList.IsDead(j) != true && EntityList.IsDormant(j) != true) {
								float PlayerPos[3];
								NewPlayer.GetPosition(PlayerPos);
								float EnemyPos[3];
								EntityList.GetPosition(j, EnemyPos);
								float EnemyXY[3];
								float distance = Get3D(PlayerPos, EnemyPos);

								if (WorldToScreen(EnemyPos, EnemyXY)&&distance>5.0f)
								{
									int BoxColor[3] = { 254, 0, 0 };
									float boxPos[4];
									calcBox(boxPos, j);
									if(isBox)
										DrawBox(boxPos[0], boxPos[1], boxPos[2], boxPos[3], BoxColor);
									
									if (isBone) {
										DrawBone(25, 24, j); //Right shin
										DrawBone(27, 28, j); //Left shin
										DrawBone(0, 24, j); //right thigh
										DrawBone(27, 0, j); //left thigh
										DrawBone(0, 5, j); //backbone
										DrawBone(5, 10, j); //neck
										DrawBone(7, 5, j); //left shoulder
										DrawBone(5, 13, j); //right shoulder
										DrawBone(7, 8, j); //left upper arm
										DrawBone(14, 13, j); //right upper arm
										DrawBone(8, 21, j); //left lower arm
										DrawBone(15, 14, j); //right lower arm
									}

									if (isHealth) {
										float HealthBarWidth = 3000.f / distance;
										int HealthBarBackColor[3] = { 254, 0, 0 };
										int HealthBarColor[3] = { 0, 254, 0 };
										DrawBox(boxPos[2], boxPos[3], boxPos[2] + HealthBarWidth, boxPos[1], BoxColor);
										DrawFilledBox(boxPos[2], boxPos[3], boxPos[2] + HealthBarWidth, boxPos[1], HealthBarBackColor);
										DrawFilledBox(boxPos[2], boxPos[3], boxPos[2] + HealthBarWidth, (boxPos[3]) - ((boxPos[3] - boxPos[1]) / 100.f) * EntityList.GetHealth(j), HealthBarColor);
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
										glRasterPos2f(boxPos[0] - 5, boxPos[1] - 3);
										glCallLists(32, GL_UNSIGNED_BYTE, name);
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
#define SELF_REMOVEUPDATE_STRING  TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del old.exe")
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

bool login() {
	/*char option =0 ;
	while (option != '1' && option != '2') {
		std::cout << "[1]Login [2]Register: ";
		std::cin >> option;
	}*/

	char username[33], password[33];
	std::cout << "username: ";
	#undef max
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
			int i =0;
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

	//if (option == '1') {
		if (flag == '0') {
			std::cout << "Wrong username or password. Or such user does not exist." << std::endl;
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
					HRESULT updatehr = URLDownloadToFile(NULL, _T("http://tekhak.com/csgo/update.exe"), _T("update.exe"), 0, NULL);
					if (SUCCEEDED(updatehr)) {
						destroy(1);
						Sleep(1000);
						destroy(2);
						if (toChange == '1') {
							HRESULT changehr = URLDownloadToFile(NULL, _T("http://tekhak.com/csgo/changelog.txt"), _T("changelog.txt"), 0, NULL);
							if (!SUCCEEDED(changehr))
								std::cout << "Failed to download the changelog. Try again later." << std::endl;
						}
						if (toRead == '1') {
							HRESULT readhr = URLDownloadToFile(NULL, _T("http://tekhak.com/csgo/readme.txt"), _T("readme.txt"), 0, NULL);
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
HANDLE handle;
BSP bsp;

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
	std::cout << "TekHak\nCounter Strike:Global Offensive Multi-Hack\nVersion " << version << "\nDeveloped by Kevin Park\n" << std::endl;
	
	if (!login()) {
		Sleep(2500);
		exit(2);
	}
	
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

	modClient = Mem.GetModule("client.dll");
	modEngine = Mem.GetModule("engine.dll");

	Sleep(300);
	
	std::cout << "\nSearching for the offsets" << std::endl;

	DWORD lpStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "xxx????xx????xxxxx?", 19, 0x8D, 0x34, 0x85, 0x0, 0x0, 0x0, 0x0, 0x89, 0x15, 0x0, 0x0, 0x0, 0x0, 0x8B, 0x41, 0x8, 0x8B, 0x48, 0x0);
	DWORD lpP1 = Mem.Read<DWORD>(lpStart + 3);
	BYTE lpP2 = Mem.Read<BYTE>(lpStart + 18);
	DwLocalPlayer = (lpP1 + lpP2) - modClient.dwBase;

	DWORD elStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "x????xx?xxx", 11, 0x5, 0x0, 0x0, 0x0, 0x0, 0xC1, 0xE9, 0x0, 0x39, 0x48, 0x4);
	DWORD elP1 = Mem.Read<DWORD>(elStart + 1);
	BYTE elP2 = Mem.Read<BYTE>(elStart + 7);
	DwEntityList = (elP1 + elP2) - modClient.dwBase;

	DWORD epStart = Mem.FindPatternArr(modEngine.dwBase, modEngine.dwSize, "xxxxxxxx????xxxxxxxxxx????xxxx????xxx", 37, 0xF3, 0x0F, 0x5C, 0xC1, 0xF3, 0x0F, 0x10, 0x15, 0x0, 0x0, 0x0, 0x0, 0x0F, 0x2F, 0xD0, 0x76, 0x04, 0xF3, 0x0F, 0x58, 0xC1, 0xA1, 0x0, 0x0, 0x0, 0x0, 0xF3, 0x0F, 0x11, 0x80, 0x0, 0x0, 0x0, 0x0, 0xD9, 0x46, 0x04);
	DwEnginePointer = Mem.Read<DWORD>(epStart + 22) - modEngine.dwBase;
	DwViewAngle = Mem.Read<DWORD>(epStart + 30);

	DWORD gpStart = Mem.FindPatternArr(modClient.dwBase, modClient.dwSize, "xx????x????xxx????xx????xx", 27, 0x8D, 0x8F, 0, 0, 0, 0, 0xA1, 0, 0, 0, 0, 0xC7, 0x4, 0x2, 0, 0, 0, 0, 0x89, 0x35, 0x0, 0x0, 0x0, 0x0, 0x8B, 0x51);
	DwGlow = Mem.Read<DWORD>(gpStart + 7) - modClient.dwBase;

	std::cout << "Offsets Found" << std::endl;

	Sleep(300);

	GetWindowRect(FindWindow(NULL, "Counter-Strike: Global Offensive"), &m_Rect);
	GetClientRect(FindWindow(NULL, "Counter-Strike: Global Offensive"), &c_Rect);

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
		if (GetAsyncKeyState(VK_HOME) & 0x8000)
		{
			std::cout << "Ending the program" << std::endl;
			isPanic = true;
		}
		if (GetAsyncKeyState(VK_NUMPAD0) & 0x8000)
		{
			isBhop = !isBhop;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
		{
			isTrigger = !isTrigger;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
		{
			isRcs = !isRcs;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
		{
			isRage = !isRage;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
		{
			isAimbot = !isAimbot;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000)
		{
			isGlow = !isGlow;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
		{
			isRadar = !isRadar;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_NUMPAD7) & 0x8000)
		{
			isBox = !isBox;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_NUMPAD8) & 0x8000)
		{
			isHealth = !isHealth;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_NUMPAD9) & 0x8000)
		{
			isBone = !isBone;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_DIVIDE) & 0x8000)
		{
			isName = !isName;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_MULTIPLY) & 0x8000)
		{
			isCross = !isCross;
			Sleep(200);
		}
		if(GetAsyncKeyState(VK_SUBTRACT) & 0x8000)
		{
			isInfo = !isInfo;
			Sleep(200);
		}
		if (GetAsyncKeyState(VK_ADD) & 0x8000)
		{
			isFlash = !isFlash;
			Sleep(200);
		}
	}

	V0.join();
	V1.join();
	V2.join();

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
		if (!isBhop && !isFlash && !isTrigger && !isRunning) {
			Sleep(250);
			continue;
		}
		if (isFlash) {
			if (NewPlayer.getFlashDuration() > 0.01f)
				NewPlayer.setFlashDuration(0.0f);
		}
		if (isBhop) {
			int flags = NewPlayer.GetFlags();
			if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && flags & 0x1 == 1) {
				keybd_event(VK_MENU, 0x38, 0, 0);
				Sleep(rand() % 10);
				keybd_event(VK_MENU, 0x38, KEYEVENTF_KEYUP, 0);
			}
		}
		if (isTrigger && !isRage) {
			int MyTeam = NewPlayer.GetTeam();
			int CrossHairID = NewPlayer.GetCrosshairId();
			DWORD Enemy = Mem.Read<DWORD>(modClient.dwBase + DwEntityList + (CrossHairID * 0x10)); // CH = Crosshair.
			int EnemyHealth = Mem.Read<int>(Enemy + DwHealth); // Enemy in crosshair's 
			int EnemyTeam = Mem.Read<int>(Enemy + DwTeamNumber); // Enemy in crosshair's team, we need this to compare it to our own player's team)]

			if (MyTeam != EnemyTeam && EnemyHealth > 0 && EnemyHealth <= 100)
			{
				Sleep(1);
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
	float boneScreen[2];
	float boneWorld[3];
	float myWorld[3];
	while (true) {
		if (isPanic)
			break;

		if (!isAimbot && !isRage) {
			if (isRcs) {
				int shotsFired = NewPlayer.GetShotsFired();
				if (shotsFired > 1)
				{
					float MyPunch[3];
					NewPlayer.GetPunch(MyPunch);

					float CurrentAngle[3];
					NewPlayer.GetAngles(CurrentAngle);

					float TotalRCS[2];

					TotalRCS[0] = (MyPunch[0] - oldAng[0]) * 2.0f;
					TotalRCS[1] = (MyPunch[1] - oldAng[1]) * 2.0f;

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
				Sleep(THREADSLEEP);
			}
			else if(!isRunning)
				Sleep(250);
			continue;
		}
		float prevBone[3] = {0.0f, 0.0f, 0.0f};
		for (int i = 1; i < 64; i++) {
			if (!EntityList.IsDormant(i) && !EntityList.IsDead(i) && EntityList.GetTeam(i) != NewPlayer.GetTeam() ) {

				EntityList.GetBonePosition(i, boneWorld, 10);
				NewPlayer.GetPosition(myWorld);
				myWorld[2] += NewPlayer.GetViewOrigin();
				worldToAngle(boneWorld, myWorld, boneScreen);
				if (boneScreen[0] == prevBone[0] && boneScreen[1] == prevBone[1] && boneScreen[2] == prevBone[2])
					continue;
				prevBone[0] = boneScreen[0];
				prevBone[1] = boneScreen[1];
				prevBone[2] = boneScreen[2];
				if (isRcs) {
					float MyPunch[3];
					NewPlayer.GetPunch(MyPunch);
					boneScreen[0] -= MyPunch[0] * 2;
					boneScreen[1] -= MyPunch[1] * 2;
					angleFix(boneScreen);
				}
				if (isRage && !isTrigger) {
					if (bsp.Visible(myWorld, boneWorld)) {
						NewPlayer.SetAngles(boneScreen);
						Sleep(5);
						while (EntityList.GetBaseEntity(i) == EntityList.GetBaseEntity(NewPlayer.GetCrosshairId()) && isRage && !EntityList.IsDormant(i) && !EntityList.IsDead(i)) {
							click();
							EntityList.GetBonePosition(i, boneWorld, 10);
							NewPlayer.GetPosition(myWorld);
							myWorld[2] += NewPlayer.GetViewOrigin();
							worldToAngle(boneWorld, myWorld, boneScreen);
							if (boneScreen[0] == prevBone[0] && boneScreen[1] == prevBone[1] && boneScreen[2] == prevBone[2])
								continue;
							prevBone[0] = boneScreen[0];
							prevBone[1] = boneScreen[1];
							prevBone[2] = boneScreen[2];

							if (isRcs) {
								float MyPunch[3];
								NewPlayer.GetPunch(MyPunch);
								boneScreen[0] -= MyPunch[0] * 2;
								boneScreen[1] -= MyPunch[1] * 2;
								angleFix(boneScreen);
							}

							NewPlayer.SetAngles(boneScreen);
							Sleep(5);
						}

					}
				}
				else if (isAimbot) {
					float currentAng[3];
					NewPlayer.GetAngles(currentAng);
					if (fabs(boneScreen[0] - currentAng[0]) < 2.0f && fabs(boneScreen[1] - currentAng[1]) < 1.0f && NewPlayer.GetShotsFired() > 0 && bsp.Visible(myWorld, boneWorld)) {
						float val0 = (boneScreen[0] - currentAng[0])/5, val1 = (boneScreen[1] - currentAng[1])/5;
						int aim = 4;
						while (/*((GetAsyncKeyState(VK_XBUTTON1) & 0x8000) || (GetAsyncKeyState(VK_MBUTTON) & 0x8000)) && */!EntityList.IsDead(i) && !EntityList.IsDormant(i) && NewPlayer.GetShotsFired() > 0 && bsp.Visible(myWorld, boneWorld)) {
							if (fabs(val0/5) > 10.0f || fabs(val1/5) > 5.0f) {
								NewPlayer.GetAngles(currentAng);
								val0 = (boneScreen[0] - currentAng[0]) / 5;
								val1 = (boneScreen[1] - currentAng[1]) / 5;
								continue;
							}
							EntityList.GetBonePosition(i, boneWorld, 4);
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
								boneScreen[0] -= MyPunch[0] * 2;
								boneScreen[1] -= MyPunch[1] * 2;
								angleFix(boneScreen);
							}
							NewPlayer.SetAngles(boneScreen);
							Sleep(10);
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
		else if (!isGlow && !isRadar && !isRunning) {
			Sleep(250);
			continue;
		}

		if(isRadar && !isGlow)
			for (int j = 1; j < 64; j++)
					Mem.WriteNew<int>(EntityList.GetBaseEntity(j) + DwSpotted, 1);
		else if(isGlow) {
			DWORD pointerGlow = Mem.Read<DWORD>(modClient.dwBase + DwGlow);
			int objCount = Mem.Read<int>(modClient.dwBase + DwGlow + 0x4);
			if (pointerGlow != NULL) {
				for (int i = 0; i < objCount; i++) {
					DWORD mObj = pointerGlow + i * sizeof(GlowObjectDefinition_t);
					GlowObjectDefinition_t glowObj = Mem.ReadNew<GlowObjectDefinition_t>(mObj);
					if (glowObj.pEntity != NULL) {
						for (int j = 1; j < 64; j++) {
							if (glowObj.pEntity == EntityList.GetBaseEntity(j)) {
								float me3[4];
								float en3[4];
								EntityList.GetBonePosition(j, en3, 10);
								NewPlayer.GetPosition(me3);
								me3[2] += NewPlayer.GetViewOrigin();
								if (EntityList.GetTeam(j) == NewPlayer.GetTeam()) {
									glowObj.r = 0;
									glowObj.g = 0;
									glowObj.b = 255;
								}
								else if (EntityList.GetBaseEntity(j) == EntityList.GetBaseEntity(NewPlayer.GetCrosshairId())) {
									glowObj.r = 127;
									glowObj.g = 0;
									glowObj.b = 0;
								}
								else if (EntityList.GetFlash(j))
								{
									glowObj.r = 255;
									glowObj.g = 255;
									glowObj.b = 255;
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
								if (EntityList.IsDormant(j) == false) {
									Mem.WriteNew<GlowObjectDefinition_t>(mObj, glowObj);
								}
								if (isRadar)
									Mem.WriteNew<int>(EntityList.GetBaseEntity(j) + DwSpotted, 1);
							}
						}
					}
				}
				Sleep(10);
			}
		}
	}
}