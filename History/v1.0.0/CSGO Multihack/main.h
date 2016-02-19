#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <stdint.h>
#include <math.h>  
#include <strsafe.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <GL/glut.h>
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glu32.lib")
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

const DWORD DwCrosshairId = 0x2410;
const DWORD DwVecViewOrigin = 0x104;
const DWORD DwVecOrigin = 0x134;
const DWORD DwVecPunch = 0x13E8;
const DWORD DwTeamNumber = 0xF0;
const DWORD DwShotsFired = 0x1D6C;
const DWORD DwBoneMatrix = 0xA78;
const DWORD DwEntitySize = 0x10;
const DWORD DwHealth = 0xFC;
const DWORD DwLifeState = 0x25B;
const DWORD DwVecVelocity = 0x110;
const DWORD DwIsDormant = 0xE9;
const DWORD DwFlash = 0x1DB4;
const DWORD DwSpotted = 0x935;
const DWORD DwActiveWeapon = 0x12C0;
const DWORD DwFlags = 0x100;
const DWORD DwMapname = 0x26C;

struct GlowObjectDefinition_t
{
	DWORD pEntity;
	float r;
	float g;
	float b;
	float a;
	uint8_t unk1[16];
	bool m_bRenderWhenOccluded;
	bool m_bRenderWhenUnoccluded;
	bool m_bFullBloom;
	uint8_t unk2[10];
};

typedef struct {
	float flMatrix[4][4];
}WorldToScreenMatrix_t;