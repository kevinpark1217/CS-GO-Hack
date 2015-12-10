#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <stdint.h>
#include <math.h>  
#include <strsafe.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <map>
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glu32.lib")
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

const DWORD DwCrosshairId = 0x0000A924;
const DWORD DwVecViewOffset = 0x00000104;
const DWORD DwVecOrigin = 0x00000134;
const DWORD DwVecPunch = 0x00002FF8;
const DWORD DwTeamNumber = 0xF0;
const DWORD DwShotsFired = 0x0000A280;
const DWORD DwBoneMatrix = 0x0000267C;
const DWORD DwEntitySize = 0x10;
const DWORD DwHealth = 0xFC;
const DWORD DwLifeState = 0x25B;
const DWORD DwVecVelocity = 0x110;
const DWORD DwIsDormant = 0xE9;
const DWORD DwFlash = 0x0000A2C4;
const DWORD DwFlashDuration = 0x0000A2C8;
const DWORD DwSpotted = 0x00000935;
const DWORD DwActiveWeapon = 0x00002EC8;
const DWORD DwFlags = 0x100;
const DWORD DwMapname = 0x00000284;
const DWORD DwWeaponId = 0x000032BC;
const DWORD DwClip = 0x000031D4;
const DWORD DwInGame = 0x00000100;
const DWORD DwMaxPlayer = 0x00000308;
const DWORD DwWeaponReserve = 0x31DC;

/*struct Vector
{
	float x, y, z;

	float LengthSqr(void)
	{
		return (x*x + y*y + z*z);
	}

	Vector& Vector::operator*=(float fl)
	{
		x *= fl;
		y *= fl;
		z *= fl;
		return *this;
	}
};

void VectorSubtract(const Vector& a, const Vector& b, Vector& c)
{
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
}

void VectorAdd(const Vector& a, const Vector& b, Vector& c)
{
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
}

void VectorCopy(const Vector& src, Vector& dst)
{
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
}

void VectorClear(Vector& a)
{
	a.x = a.y = a.z = 0.0f;
}*/

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
	uint8_t unk2[14];
};

typedef struct {
	float flMatrix[4][4];
}WorldToScreenMatrix_t;