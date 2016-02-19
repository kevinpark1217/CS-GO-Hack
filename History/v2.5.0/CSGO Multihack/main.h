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
#include <map>
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glu32.lib")
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

const DWORD DwCrosshairId = 0x8CF4;
const DWORD DwVecViewOffset = 0x104;
const DWORD DwVecOrigin = 0x134;
const DWORD DwVecPunch = 0x13E8;
const DWORD DwTeamNumber = 0xF0;
const DWORD DwShotsFired = 0x8650;
const DWORD DwBoneMatrix = 0xA78;
const DWORD DwEntitySize = 0x10;
const DWORD DwHealth = 0xFC;
const DWORD DwLifeState = 0x25B;
const DWORD DwVecVelocity = 0x110;
const DWORD DwIsDormant = 0xE9;
const DWORD DwFlash = 0x8694;
const DWORD DwFlashDuration = 0x8698;
const DWORD DwSpotted = 0x935;
const DWORD DwActiveWeapon = 0x12C0;
const DWORD DwFlags = 0x100;
const DWORD DwMapname = 0x26C;
const DWORD DwWeaponId = 0x1690;
const DWORD DwClip = 0x15C0;

const DWORD DwResourceTeam = 0x0F30;
const DWORD DwResourceAlive = 0x1138;
const DWORD DwResourceHealth = 0x117C;
const DWORD DwResourceArmor = 0x1834;
const DWORD DwResourceHelmet = 0x17F1;
const DWORD DwResourceKit = 0x17B0;
const DWORD DwResourceRank = 0x1A3C;
const DWORD DwResourceWins = 0x1B40;
const DWORD DwResourceSpent = 0x462C;

const char* ranks[] =
{
	"No Rank",
	"Silver I",
	"Silver II",
	"Silver III",
	"Silver IV",
	"Silver Elite",
	"Silver Elite Master",

	"Gold Nova I",
	"Gold Nova II",
	"Gold Nova III",
	"Gold Nova Master",
	"Master Guardian I",
	"Master Guardian II",

	"Master Guardian Elite",
	"Distinguished Master Guardian",
	"Legendary Eagle",
	"Legendary Eagle Master",
	"Supreme Master First Class",
	"The Global Elite"
};

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
	uint8_t unk2[10];
};

typedef struct {
	float flMatrix[4][4];
}WorldToScreenMatrix_t;

std::map<int, int> ctm_fbi;
std::map<int, int> ctm_gign;
std::map<int, int> ctm_gsg9;
std::map<int, int> ctm_idf;
std::map<int, int> ctm_sas;
std::map<int, int> ctm_st6;
std::map<int, int> ctm_swat;

std::map<int, int> tm_anarchist;
std::map<int, int> tm_balkan;
std::map<int, int> tm_leet;
std::map<int, int> tm_phoenix;
std::map<int, int> tm_pirate;
std::map<int, int> tm_professional;
std::map<int, int> tm_separatist;

struct ctm_fbiInit
{
	ctm_fbiInit()
	{
		ctm_fbi[0] = -1;
		ctm_fbi[1] = 0;
		ctm_fbi[2] = 1;
		ctm_fbi[3] = 2;
		ctm_fbi[4] = 3;
		ctm_fbi[5] = 4;
		ctm_fbi[6] = 5;
		ctm_fbi[7] = 4;
		ctm_fbi[8] = 7;
		ctm_fbi[9] = 8;
		ctm_fbi[10] = 9;
		ctm_fbi[11] = 10;
		ctm_fbi[12] = 11;
		ctm_fbi[13] = 12;
		ctm_fbi[14] = 13;
		ctm_fbi[15] = 10;
		ctm_fbi[16] = 15;
		ctm_fbi[17] = 16;
		ctm_fbi[18] = 17;
		ctm_fbi[19] = 10;
		ctm_fbi[20] = 19;
		ctm_fbi[21] = 20;
		ctm_fbi[22] = 21;
		ctm_fbi[23] = 10;
		ctm_fbi[24] = 23;
		ctm_fbi[25] = 24;
		ctm_fbi[26] = 10;
		ctm_fbi[27] = 26;
		ctm_fbi[28] = 27;
		ctm_fbi[29] = 28;
		ctm_fbi[30] = 10;
		ctm_fbi[31] = 9;
		ctm_fbi[32] = 9;
		ctm_fbi[33] = 8;
		ctm_fbi[34] = 8;
		ctm_fbi[35] = 4;
		ctm_fbi[36] = 35;
		ctm_fbi[37] = 36;
		ctm_fbi[38] = 37;
		ctm_fbi[39] = 38;
		ctm_fbi[40] = 39;
		ctm_fbi[41] = 40;
		ctm_fbi[42] = 41;
		ctm_fbi[43] = 38;
		ctm_fbi[44] = 43;
		ctm_fbi[45] = 44;
		ctm_fbi[46] = 45;
		ctm_fbi[47] = 38;
		ctm_fbi[48] = 47;
		ctm_fbi[49] = 48;
		ctm_fbi[50] = 49;
		ctm_fbi[51] = 38;
		ctm_fbi[52] = 51;
		ctm_fbi[53] = 52;
		ctm_fbi[54] = 38;
		ctm_fbi[55] = 54;
		ctm_fbi[56] = 55;
		ctm_fbi[57] = 56;
		ctm_fbi[58] = 38;
		ctm_fbi[59] = 37;
		ctm_fbi[60] = 37;
		ctm_fbi[61] = 36;
		ctm_fbi[62] = 36;
		ctm_fbi[63] = 0;
		ctm_fbi[64] = 63;
		ctm_fbi[65] = 64;
		ctm_fbi[66] = 65;
		ctm_fbi[67] = 63;
		ctm_fbi[68] = 63;
		ctm_fbi[69] = 0;
		ctm_fbi[70] = 69;
		ctm_fbi[71] = 70;
		ctm_fbi[72] = 71;
		ctm_fbi[73] = 69;
		ctm_fbi[74] = 69;
		ctm_fbi[75] = 38;
		ctm_fbi[76] = 10;
		ctm_fbi[77] = -1;
		ctm_fbi[78] = 66;
		ctm_fbi[79] = 72;
		ctm_fbi[80] = 3;
		ctm_fbi[81] = 3;
	}
} Ctm_fbiInit;

struct ctm_gignInit
{
	ctm_gignInit()
	{
		ctm_gign[0] = -1;
		ctm_gign[1] = 0;
		ctm_gign[2] = 1;
		ctm_gign[3] = 2;
		ctm_gign[4] = 3;
		ctm_gign[5] = 4;
		ctm_gign[6] = 5;
		ctm_gign[7] = 5;
		ctm_gign[8] = 4;
		ctm_gign[9] = 8;
		ctm_gign[10] = 9;
		ctm_gign[11] = 10;
		ctm_gign[12] = 11;
		ctm_gign[13] = 12;
		ctm_gign[14] = 13;
		ctm_gign[15] = 14;
		ctm_gign[16] = 11;
		ctm_gign[17] = 16;
		ctm_gign[18] = 17;
		ctm_gign[19] = 18;
		ctm_gign[20] = 11;
		ctm_gign[21] = 20;
		ctm_gign[22] = 21;
		ctm_gign[23] = 22;
		ctm_gign[24] = 11;
		ctm_gign[25] = 24;
		ctm_gign[26] = 25;
		ctm_gign[27] = 11;
		ctm_gign[28] = 27;
		ctm_gign[29] = 28;
		ctm_gign[30] = 29;
		ctm_gign[31] = 11;
		ctm_gign[32] = 10;
		ctm_gign[33] = 10;
		ctm_gign[34] = 9;
		ctm_gign[35] = 9;
		ctm_gign[36] = 8;
		ctm_gign[37] = 4;
		ctm_gign[38] = 37;
		ctm_gign[39] = 38;
		ctm_gign[40] = 39;
		ctm_gign[41] = 40;
		ctm_gign[42] = 41;
		ctm_gign[43] = 42;
		ctm_gign[44] = 43;
		ctm_gign[45] = 40;
		ctm_gign[46] = 45;
		ctm_gign[47] = 46;
		ctm_gign[48] = 47;
		ctm_gign[49] = 40;
		ctm_gign[50] = 49;
		ctm_gign[51] = 50;
		ctm_gign[52] = 51;
		ctm_gign[53] = 40;
		ctm_gign[54] = 53;
		ctm_gign[55] = 54;
		ctm_gign[56] = 40;
		ctm_gign[57] = 56;
		ctm_gign[58] = 57;
		ctm_gign[59] = 58;
		ctm_gign[60] = 40;
		ctm_gign[61] = 39;
		ctm_gign[62] = 39;
		ctm_gign[63] = 38;
		ctm_gign[64] = 38;
		ctm_gign[65] = 3;
		ctm_gign[66] = 65;
		ctm_gign[67] = 0;
		ctm_gign[68] = 67;
		ctm_gign[69] = 68;
		ctm_gign[70] = 69;
		ctm_gign[71] = 67;
		ctm_gign[72] = 67;
		ctm_gign[73] = 0;
		ctm_gign[74] = 73;
		ctm_gign[75] = 74;
		ctm_gign[76] = 75;
		ctm_gign[77] = 73;
		ctm_gign[78] = 73;
		ctm_gign[79] = 40;
		ctm_gign[80] = 11;
		ctm_gign[81] = -1;
		ctm_gign[82] = 70;
		ctm_gign[83] = 76;
		ctm_gign[84] = 3;
		ctm_gign[85] = 3;
	}
} Ctm_gignInit;

struct ctm_gsg9Init
{
	ctm_gsg9Init()
	{
		ctm_gsg9[0] = -1;
		ctm_gsg9[1] = 0;
		ctm_gsg9[2] = 1;
		ctm_gsg9[3] = 2;
		ctm_gsg9[4] = 3;
		ctm_gsg9[5] = 4;
		ctm_gsg9[6] = 5;
		ctm_gsg9[7] = 4;
		ctm_gsg9[8] = 7;
		ctm_gsg9[9] = 8;
		ctm_gsg9[10] = 9;
		ctm_gsg9[11] = 10;
		ctm_gsg9[12] = 11;
		ctm_gsg9[13] = 12;
		ctm_gsg9[14] = 13;
		ctm_gsg9[15] = 10;
		ctm_gsg9[16] = 15;
		ctm_gsg9[17] = 16;
		ctm_gsg9[18] = 17;
		ctm_gsg9[19] = 10;
		ctm_gsg9[20] = 19;
		ctm_gsg9[21] = 20;
		ctm_gsg9[22] = 21;
		ctm_gsg9[23] = 10;
		ctm_gsg9[24] = 23;
		ctm_gsg9[25] = 24;
		ctm_gsg9[26] = 10;
		ctm_gsg9[27] = 26;
		ctm_gsg9[28] = 27;
		ctm_gsg9[29] = 28;
		ctm_gsg9[30] = 10;
		ctm_gsg9[31] = 9;
		ctm_gsg9[32] = 9;
		ctm_gsg9[33] = 8;
		ctm_gsg9[34] = 8;
		ctm_gsg9[35] = 4;
		ctm_gsg9[36] = 35;
		ctm_gsg9[37] = 36;
		ctm_gsg9[38] = 37;
		ctm_gsg9[39] = 38;
		ctm_gsg9[40] = 39;
		ctm_gsg9[41] = 40;
		ctm_gsg9[42] = 41;
		ctm_gsg9[43] = 38;
		ctm_gsg9[44] = 43;
		ctm_gsg9[45] = 44;
		ctm_gsg9[46] = 45;
		ctm_gsg9[47] = 38;
		ctm_gsg9[48] = 47;
		ctm_gsg9[49] = 48;
		ctm_gsg9[50] = 49;
		ctm_gsg9[51] = 38;
		ctm_gsg9[52] = 51;
		ctm_gsg9[53] = 52;
		ctm_gsg9[54] = 38;
		ctm_gsg9[55] = 54;
		ctm_gsg9[56] = 55;
		ctm_gsg9[57] = 56;
		ctm_gsg9[58] = 38;
		ctm_gsg9[59] = 37;
		ctm_gsg9[60] = 37;
		ctm_gsg9[61] = 36;
		ctm_gsg9[62] = 36;
		ctm_gsg9[63] = 0;
		ctm_gsg9[64] = 63;
		ctm_gsg9[65] = 64;
		ctm_gsg9[66] = 65;
		ctm_gsg9[67] = 63;
		ctm_gsg9[68] = 63;
		ctm_gsg9[69] = 0;
		ctm_gsg9[70] = 69;
		ctm_gsg9[71] = 70;
		ctm_gsg9[72] = 71;
		ctm_gsg9[73] = 69;
		ctm_gsg9[74] = 69;
		ctm_gsg9[75] = 38;
		ctm_gsg9[76] = 10;
		ctm_gsg9[77] = -1;
		ctm_gsg9[78] = 66;
		ctm_gsg9[79] = 72;
		ctm_gsg9[80] = 3;
		ctm_gsg9[81] = 3;
	}
} Ctm_gsg9Init;

struct ctm_idfInit
{
	ctm_idfInit()
	{
		ctm_idf[0] = -1;
		ctm_idf[1] = 0;
		ctm_idf[2] = 1;
		ctm_idf[3] = 2;
		ctm_idf[4] = 3;
		ctm_idf[5] = 4;
		ctm_idf[6] = 5;
		ctm_idf[7] = 5;
		ctm_idf[8] = 4;
		ctm_idf[9] = 8;
		ctm_idf[10] = 9;
		ctm_idf[11] = 10;
		ctm_idf[12] = 11;
		ctm_idf[13] = 12;
		ctm_idf[14] = 13;
		ctm_idf[15] = 14;
		ctm_idf[16] = 11;
		ctm_idf[17] = 16;
		ctm_idf[18] = 17;
		ctm_idf[19] = 18;
		ctm_idf[20] = 11;
		ctm_idf[21] = 20;
		ctm_idf[22] = 21;
		ctm_idf[23] = 22;
		ctm_idf[24] = 11;
		ctm_idf[25] = 24;
		ctm_idf[26] = 25;
		ctm_idf[27] = 11;
		ctm_idf[28] = 27;
		ctm_idf[29] = 28;
		ctm_idf[30] = 29;
		ctm_idf[31] = 11;
		ctm_idf[32] = 10;
		ctm_idf[33] = 10;
		ctm_idf[34] = 9;
		ctm_idf[35] = 9;
		ctm_idf[36] = 8;
		ctm_idf[37] = 36;
		ctm_idf[38] = 4;
		ctm_idf[39] = 38;
		ctm_idf[40] = 39;
		ctm_idf[41] = 40;
		ctm_idf[42] = 41;
		ctm_idf[43] = 42;
		ctm_idf[44] = 43;
		ctm_idf[45] = 44;
		ctm_idf[46] = 41;
		ctm_idf[47] = 46;
		ctm_idf[48] = 47;
		ctm_idf[49] = 48;
		ctm_idf[50] = 41;
		ctm_idf[51] = 50;
		ctm_idf[52] = 51;
		ctm_idf[53] = 52;
		ctm_idf[54] = 41;
		ctm_idf[55] = 54;
		ctm_idf[56] = 55;
		ctm_idf[57] = 41;
		ctm_idf[58] = 57;
		ctm_idf[59] = 58;
		ctm_idf[60] = 59;
		ctm_idf[61] = 41;
		ctm_idf[62] = 40;
		ctm_idf[63] = 40;
		ctm_idf[64] = 39;
		ctm_idf[65] = 39;
		ctm_idf[66] = 38;
		ctm_idf[67] = 66;
		ctm_idf[68] = 0;
		ctm_idf[69] = 68;
		ctm_idf[70] = 69;
		ctm_idf[71] = 70;
		ctm_idf[72] = 68;
		ctm_idf[73] = 68;
		ctm_idf[74] = 0;
		ctm_idf[75] = 74;
		ctm_idf[76] = 75;
		ctm_idf[77] = 76;
		ctm_idf[78] = 74;
		ctm_idf[79] = 74;
		ctm_idf[80] = 41;
		ctm_idf[81] = 11;
		ctm_idf[82] = -1;
		ctm_idf[83] = 71;
		ctm_idf[84] = 77;
		ctm_idf[85] = 3;
		ctm_idf[86] = 3;
	}
} Ctm_idfInit;

struct ctm_sasInit
{
	ctm_sasInit()
	{
		ctm_sas[0] = -1;
		ctm_sas[1] = 0;
		ctm_sas[2] = 1;
		ctm_sas[3] = 2;
		ctm_sas[4] = 3;
		ctm_sas[5] = 4;
		ctm_sas[6] = 5;
		ctm_sas[7] = 4;
		ctm_sas[8] = 7;
		ctm_sas[9] = 8;
		ctm_sas[10] = 9;
		ctm_sas[11] = 10;
		ctm_sas[12] = 11;
		ctm_sas[13] = 12;
		ctm_sas[14] = 13;
		ctm_sas[15] = 10;
		ctm_sas[16] = 15;
		ctm_sas[17] = 16;
		ctm_sas[18] = 17;
		ctm_sas[19] = 10;
		ctm_sas[20] = 19;
		ctm_sas[21] = 20;
		ctm_sas[22] = 21;
		ctm_sas[23] = 10;
		ctm_sas[24] = 23;
		ctm_sas[25] = 24;
		ctm_sas[26] = 10;
		ctm_sas[27] = 26;
		ctm_sas[28] = 27;
		ctm_sas[29] = 28;
		ctm_sas[30] = 10;
		ctm_sas[31] = 9;
		ctm_sas[32] = 9;
		ctm_sas[33] = 8;
		ctm_sas[34] = 8;
		ctm_sas[35] = 4;
		ctm_sas[36] = 35;
		ctm_sas[37] = 36;
		ctm_sas[38] = 37;
		ctm_sas[39] = 38;
		ctm_sas[40] = 39;
		ctm_sas[41] = 40;
		ctm_sas[42] = 41;
		ctm_sas[43] = 38;
		ctm_sas[44] = 43;
		ctm_sas[45] = 44;
		ctm_sas[46] = 45;
		ctm_sas[47] = 38;
		ctm_sas[48] = 47;
		ctm_sas[49] = 48;
		ctm_sas[50] = 49;
		ctm_sas[51] = 38;
		ctm_sas[52] = 51;
		ctm_sas[53] = 52;
		ctm_sas[54] = 38;
		ctm_sas[55] = 54;
		ctm_sas[56] = 55;
		ctm_sas[57] = 56;
		ctm_sas[58] = 38;
		ctm_sas[59] = 37;
		ctm_sas[60] = 37;
		ctm_sas[61] = 36;
		ctm_sas[62] = 36;
		ctm_sas[63] = 0;
		ctm_sas[64] = 63;
		ctm_sas[65] = 64;
		ctm_sas[66] = 65;
		ctm_sas[67] = 63;
		ctm_sas[68] = 63;
		ctm_sas[69] = 0;
		ctm_sas[70] = 69;
		ctm_sas[71] = 70;
		ctm_sas[72] = 71;
		ctm_sas[73] = 69;
		ctm_sas[74] = 69;
		ctm_sas[75] = 38;
		ctm_sas[76] = 10;
		ctm_sas[77] = -1;
		ctm_sas[78] = 66;
		ctm_sas[79] = 72;
		ctm_sas[80] = 3;
		ctm_sas[81] = 3;
	}
} Ctm_sasInit;

struct ctm_st6Init
{
	ctm_st6Init()
	{
		ctm_st6[0] = -1;
		ctm_st6[1] = 0;
		ctm_st6[2] = 1;
		ctm_st6[3] = 2;
		ctm_st6[4] = 3;
		ctm_st6[5] = 4;
		ctm_st6[6] = 5;
		ctm_st6[7] = 4;
		ctm_st6[8] = 7;
		ctm_st6[9] = 8;
		ctm_st6[10] = 9;
		ctm_st6[11] = 10;
		ctm_st6[12] = 11;
		ctm_st6[13] = 12;
		ctm_st6[14] = 13;
		ctm_st6[15] = 10;
		ctm_st6[16] = 15;
		ctm_st6[17] = 16;
		ctm_st6[18] = 17;
		ctm_st6[19] = 10;
		ctm_st6[20] = 19;
		ctm_st6[21] = 20;
		ctm_st6[22] = 21;
		ctm_st6[23] = 10;
		ctm_st6[24] = 23;
		ctm_st6[25] = 24;
		ctm_st6[26] = 10;
		ctm_st6[27] = 26;
		ctm_st6[28] = 27;
		ctm_st6[29] = 28;
		ctm_st6[30] = 10;
		ctm_st6[31] = 9;
		ctm_st6[32] = 9;
		ctm_st6[33] = 8;
		ctm_st6[34] = 8;
		ctm_st6[35] = 4;
		ctm_st6[36] = 35;
		ctm_st6[37] = 36;
		ctm_st6[38] = 37;
		ctm_st6[39] = 38;
		ctm_st6[40] = 39;
		ctm_st6[41] = 40;
		ctm_st6[42] = 41;
		ctm_st6[43] = 38;
		ctm_st6[44] = 43;
		ctm_st6[45] = 44;
		ctm_st6[46] = 45;
		ctm_st6[47] = 38;
		ctm_st6[48] = 47;
		ctm_st6[49] = 48;
		ctm_st6[50] = 49;
		ctm_st6[51] = 38;
		ctm_st6[52] = 51;
		ctm_st6[53] = 52;
		ctm_st6[54] = 38;
		ctm_st6[55] = 54;
		ctm_st6[56] = 55;
		ctm_st6[57] = 56;
		ctm_st6[58] = 38;
		ctm_st6[59] = 37;
		ctm_st6[60] = 37;
		ctm_st6[61] = 36;
		ctm_st6[62] = 36;
		ctm_st6[63] = 0;
		ctm_st6[64] = 63;
		ctm_st6[65] = 64;
		ctm_st6[66] = 65;
		ctm_st6[67] = 63;
		ctm_st6[68] = 63;
		ctm_st6[69] = 0;
		ctm_st6[70] = 69;
		ctm_st6[71] = 70;
		ctm_st6[72] = 71;
		ctm_st6[73] = 69;
		ctm_st6[74] = 69;
		ctm_st6[75] = 38;
		ctm_st6[76] = 10;
		ctm_st6[77] = -1;
		ctm_st6[78] = 66;
		ctm_st6[79] = 72;
		ctm_st6[80] = 3;
		ctm_st6[81] = 3;
	}
} Ctm_st6Init;

struct ctm_swatInit
{
	ctm_swatInit()
	{
		ctm_swat[0] = -1;
		ctm_swat[1] = 0;
		ctm_swat[2] = 1;
		ctm_swat[3] = 2;
		ctm_swat[4] = 3;
		ctm_swat[5] = 4;
		ctm_swat[6] = 5;
		ctm_swat[7] = 6;
		ctm_swat[8] = 5;
		ctm_swat[9] = 4;
		ctm_swat[10] = 9;
		ctm_swat[11] = 10;
		ctm_swat[12] = 11;
		ctm_swat[13] = 12;
		ctm_swat[14] = 13;
		ctm_swat[15] = 14;
		ctm_swat[16] = 15;
		ctm_swat[17] = 12;
		ctm_swat[18] = 17;
		ctm_swat[19] = 18;
		ctm_swat[20] = 19;
		ctm_swat[21] = 12;
		ctm_swat[22] = 21;
		ctm_swat[23] = 22;
		ctm_swat[24] = 23;
		ctm_swat[25] = 12;
		ctm_swat[26] = 25;
		ctm_swat[27] = 26;
		ctm_swat[28] = 12;
		ctm_swat[29] = 28;
		ctm_swat[30] = 29;
		ctm_swat[31] = 30;
		ctm_swat[32] = 12;
		ctm_swat[33] = 11;
		ctm_swat[34] = 11;
		ctm_swat[35] = 10;
		ctm_swat[36] = 10;
		ctm_swat[37] = 4;
		ctm_swat[38] = 37;
		ctm_swat[39] = 38;
		ctm_swat[40] = 39;
		ctm_swat[41] = 40;
		ctm_swat[42] = 41;
		ctm_swat[43] = 42;
		ctm_swat[44] = 43;
		ctm_swat[45] = 40;
		ctm_swat[46] = 45;
		ctm_swat[47] = 46;
		ctm_swat[48] = 47;
		ctm_swat[49] = 40;
		ctm_swat[50] = 49;
		ctm_swat[51] = 50;
		ctm_swat[52] = 51;
		ctm_swat[53] = 40;
		ctm_swat[54] = 53;
		ctm_swat[55] = 54;
		ctm_swat[56] = 40;
		ctm_swat[57] = 56;
		ctm_swat[58] = 57;
		ctm_swat[59] = 58;
		ctm_swat[60] = 40;
		ctm_swat[61] = 39;
		ctm_swat[62] = 39;
		ctm_swat[63] = 38;
		ctm_swat[64] = 38;
		ctm_swat[65] = 0;
		ctm_swat[66] = 65;
		ctm_swat[67] = 66;
		ctm_swat[68] = 67;
		ctm_swat[69] = 65;
		ctm_swat[70] = 65;
		ctm_swat[71] = 0;
		ctm_swat[72] = 71;
		ctm_swat[73] = 72;
		ctm_swat[74] = 73;
		ctm_swat[75] = 71;
		ctm_swat[76] = 71;
		ctm_swat[77] = 40;
		ctm_swat[78] = 12;
		ctm_swat[79] = -1;
		ctm_swat[80] = 68;
		ctm_swat[81] = 74;
		ctm_swat[82] = 3;
		ctm_swat[83] = 3;
	}
} Ctm_swatInit;


struct tm_anarchistInit
{
	tm_anarchistInit()
	{
		tm_anarchist[0] = -1;
		tm_anarchist[1] = 0;
		tm_anarchist[2] = 1;
		tm_anarchist[3] = 2;
		tm_anarchist[4] = 3;
		tm_anarchist[5] = 4;
		tm_anarchist[6] = 5;
		tm_anarchist[7] = 6;
		tm_anarchist[8] = 5;
		tm_anarchist[9] = 4;
		tm_anarchist[10] = 9;
		tm_anarchist[11] = 10;
		tm_anarchist[12] = 11;
		tm_anarchist[13] = 12;
		tm_anarchist[14] = 13;
		tm_anarchist[15] = 14;
		tm_anarchist[16] = 15;
		tm_anarchist[17] = 12;
		tm_anarchist[18] = 17;
		tm_anarchist[19] = 18;
		tm_anarchist[20] = 19;
		tm_anarchist[21] = 12;
		tm_anarchist[22] = 21;
		tm_anarchist[23] = 22;
		tm_anarchist[24] = 23;
		tm_anarchist[25] = 12;
		tm_anarchist[26] = 25;
		tm_anarchist[27] = 26;
		tm_anarchist[28] = 12;
		tm_anarchist[29] = 28;
		tm_anarchist[30] = 29;
		tm_anarchist[31] = 30;
		tm_anarchist[32] = 12;
		tm_anarchist[33] = 11;
		tm_anarchist[34] = 11;
		tm_anarchist[35] = 10;
		tm_anarchist[36] = 10;
		tm_anarchist[37] = 4;
		tm_anarchist[38] = 37;
		tm_anarchist[39] = 38;
		tm_anarchist[40] = 39;
		tm_anarchist[41] = 40;
		tm_anarchist[42] = 41;
		tm_anarchist[43] = 42;
		tm_anarchist[44] = 43;
		tm_anarchist[45] = 40;
		tm_anarchist[46] = 45;
		tm_anarchist[47] = 46;
		tm_anarchist[48] = 47;
		tm_anarchist[49] = 40;
		tm_anarchist[50] = 49;
		tm_anarchist[51] = 50;
		tm_anarchist[52] = 51;
		tm_anarchist[53] = 40;
		tm_anarchist[54] = 53;
		tm_anarchist[55] = 54;
		tm_anarchist[56] = 40;
		tm_anarchist[57] = 56;
		tm_anarchist[58] = 57;
		tm_anarchist[59] = 58;
		tm_anarchist[60] = 40;
		tm_anarchist[61] = 39;
		tm_anarchist[62] = 39;
		tm_anarchist[63] = 38;
		tm_anarchist[64] = 38;
		tm_anarchist[65] = 3;
		tm_anarchist[66] = 65;
		tm_anarchist[67] = 0;
		tm_anarchist[68] = 67;
		tm_anarchist[69] = 68;
		tm_anarchist[70] = 69;
		tm_anarchist[71] = 67;
		tm_anarchist[72] = 67;
		tm_anarchist[73] = 0;
		tm_anarchist[74] = 73;
		tm_anarchist[75] = 74;
		tm_anarchist[76] = 75;
		tm_anarchist[77] = 73;
		tm_anarchist[78] = 73;
		tm_anarchist[79] = 40;
		tm_anarchist[80] = 12;
		tm_anarchist[81] = -1;
		tm_anarchist[82] = 70;
		tm_anarchist[83] = 76;
		tm_anarchist[84] = 3;
	}
} Tm_anarchistInit;

struct tm_balkanInit
{
	tm_balkanInit()
	{
		tm_balkan[0] = -1;
		tm_balkan[1] = 0;
		tm_balkan[2] = 1;
		tm_balkan[3] = 2;
		tm_balkan[4] = 3;
		tm_balkan[5] = 4;
		tm_balkan[6] = 5;
		tm_balkan[7] = 4;
		tm_balkan[8] = 7;
		tm_balkan[9] = 8;
		tm_balkan[10] = 9;
		tm_balkan[11] = 10;
		tm_balkan[12] = 11;
		tm_balkan[13] = 12;
		tm_balkan[14] = 13;
		tm_balkan[15] = 10;
		tm_balkan[16] = 15;
		tm_balkan[17] = 16;
		tm_balkan[18] = 17;
		tm_balkan[19] = 10;
		tm_balkan[20] = 19;
		tm_balkan[21] = 20;
		tm_balkan[22] = 21;
		tm_balkan[23] = 10;
		tm_balkan[24] = 23;
		tm_balkan[25] = 24;
		tm_balkan[26] = 10;
		tm_balkan[27] = 26;
		tm_balkan[28] = 27;
		tm_balkan[29] = 28;
		tm_balkan[30] = 10;
		tm_balkan[31] = 9;
		tm_balkan[32] = 9;
		tm_balkan[33] = 8;
		tm_balkan[34] = 8;
		tm_balkan[35] = 4;
		tm_balkan[36] = 35;
		tm_balkan[37] = 36;
		tm_balkan[38] = 37;
		tm_balkan[39] = 38;
		tm_balkan[40] = 39;
		tm_balkan[41] = 40;
		tm_balkan[42] = 41;
		tm_balkan[43] = 38;
		tm_balkan[44] = 43;
		tm_balkan[45] = 44;
		tm_balkan[46] = 45;
		tm_balkan[47] = 38;
		tm_balkan[48] = 47;
		tm_balkan[49] = 48;
		tm_balkan[50] = 49;
		tm_balkan[51] = 38;
		tm_balkan[52] = 51;
		tm_balkan[53] = 52;
		tm_balkan[54] = 38;
		tm_balkan[55] = 54;
		tm_balkan[56] = 55;
		tm_balkan[57] = 56;
		tm_balkan[58] = 38;
		tm_balkan[59] = 37;
		tm_balkan[60] = 37;
		tm_balkan[61] = 36;
		tm_balkan[62] = 36;
		tm_balkan[63] = 0;
		tm_balkan[64] = 63;
		tm_balkan[65] = 64;
		tm_balkan[66] = 65;
		tm_balkan[67] = 63;
		tm_balkan[68] = 63;
		tm_balkan[69] = 0;
		tm_balkan[70] = 69;
		tm_balkan[71] = 70;
		tm_balkan[72] = 71;
		tm_balkan[73] = 69;
		tm_balkan[74] = 69;
		tm_balkan[75] = 38;
		tm_balkan[76] = 10;
		tm_balkan[77] = -1;
		tm_balkan[78] = 66;
		tm_balkan[79] = 72;
		tm_balkan[80] = 3;
	}
} Tm_balkanInit;

struct tm_leetInit
{
	tm_leetInit()
	{
		tm_leet[0] = -1;
		tm_leet[1] = 0;
		tm_leet[2] = 1;
		tm_leet[3] = 2;
		tm_leet[4] = 3;
		tm_leet[5] = 4;
		tm_leet[6] = 5;
		tm_leet[7] = 4;
		tm_leet[8] = 7;
		tm_leet[9] = 8;
		tm_leet[10] = 9;
		tm_leet[11] = 10;
		tm_leet[12] = 11;
		tm_leet[13] = 12;
		tm_leet[14] = 13;
		tm_leet[15] = 10;
		tm_leet[16] = 15;
		tm_leet[17] = 16;
		tm_leet[18] = 17;
		tm_leet[19] = 10;
		tm_leet[20] = 19;
		tm_leet[21] = 20;
		tm_leet[22] = 21;
		tm_leet[23] = 10;
		tm_leet[24] = 23;
		tm_leet[25] = 24;
		tm_leet[26] = 10;
		tm_leet[27] = 26;
		tm_leet[28] = 27;
		tm_leet[29] = 28;
		tm_leet[30] = 10;
		tm_leet[31] = 9;
		tm_leet[32] = 9;
		tm_leet[33] = 8;
		tm_leet[34] = 8;
		tm_leet[35] = 4;
		tm_leet[36] = 35;
		tm_leet[37] = 36;
		tm_leet[38] = 37;
		tm_leet[39] = 38;
		tm_leet[40] = 39;
		tm_leet[41] = 40;
		tm_leet[42] = 41;
		tm_leet[43] = 38;
		tm_leet[44] = 43;
		tm_leet[45] = 44;
		tm_leet[46] = 45;
		tm_leet[47] = 38;
		tm_leet[48] = 47;
		tm_leet[49] = 48;
		tm_leet[50] = 49;
		tm_leet[51] = 38;
		tm_leet[52] = 51;
		tm_leet[53] = 52;
		tm_leet[54] = 38;
		tm_leet[55] = 54;
		tm_leet[56] = 55;
		tm_leet[57] = 56;
		tm_leet[58] = 38;
		tm_leet[59] = 37;
		tm_leet[60] = 37;
		tm_leet[61] = 36;
		tm_leet[62] = 36;
		tm_leet[63] = 0;
		tm_leet[64] = 63;
		tm_leet[65] = 64;
		tm_leet[66] = 65;
		tm_leet[67] = 63;
		tm_leet[68] = 63;
		tm_leet[69] = 0;
		tm_leet[70] = 69;
		tm_leet[71] = 70;
		tm_leet[72] = 71;
		tm_leet[73] = 69;
		tm_leet[74] = 69;
		tm_leet[75] = 38;
		tm_leet[76] = 10;
		tm_leet[77] = -1;
		tm_leet[78] = 66;
		tm_leet[79] = 72;
		tm_leet[80] = 3;
	}
} Tm_leetInit;

struct tm_phoenixInit
{
	tm_phoenixInit()
	{
		tm_phoenix[0] = -1;
		tm_phoenix[1] = 0;
		tm_phoenix[2] = 1;
		tm_phoenix[3] = 2;
		tm_phoenix[4] = 3;
		tm_phoenix[5] = 4;
		tm_phoenix[6] = 5;
		tm_phoenix[7] = 4;
		tm_phoenix[8] = 7;
		tm_phoenix[9] = 8;
		tm_phoenix[10] = 9;
		tm_phoenix[11] = 10;
		tm_phoenix[12] = 11;
		tm_phoenix[13] = 12;
		tm_phoenix[14] = 13;
		tm_phoenix[15] = 10;
		tm_phoenix[16] = 15;
		tm_phoenix[17] = 16;
		tm_phoenix[18] = 17;
		tm_phoenix[19] = 10;
		tm_phoenix[20] = 19;
		tm_phoenix[21] = 20;
		tm_phoenix[22] = 21;
		tm_phoenix[23] = 10;
		tm_phoenix[24] = 23;
		tm_phoenix[25] = 24;
		tm_phoenix[26] = 10;
		tm_phoenix[27] = 26;
		tm_phoenix[28] = 27;
		tm_phoenix[29] = 28;
		tm_phoenix[30] = 10;
		tm_phoenix[31] = 9;
		tm_phoenix[32] = 9;
		tm_phoenix[33] = 8;
		tm_phoenix[34] = 8;
		tm_phoenix[35] = 4;
		tm_phoenix[36] = 35;
		tm_phoenix[37] = 36;
		tm_phoenix[38] = 37;
		tm_phoenix[39] = 38;
		tm_phoenix[40] = 39;
		tm_phoenix[41] = 40;
		tm_phoenix[42] = 41;
		tm_phoenix[43] = 38;
		tm_phoenix[44] = 43;
		tm_phoenix[45] = 44;
		tm_phoenix[46] = 45;
		tm_phoenix[47] = 38;
		tm_phoenix[48] = 47;
		tm_phoenix[49] = 48;
		tm_phoenix[50] = 49;
		tm_phoenix[51] = 38;
		tm_phoenix[52] = 51;
		tm_phoenix[53] = 52;
		tm_phoenix[54] = 38;
		tm_phoenix[55] = 54;
		tm_phoenix[56] = 55;
		tm_phoenix[57] = 56;
		tm_phoenix[58] = 38;
		tm_phoenix[59] = 37;
		tm_phoenix[60] = 37;
		tm_phoenix[61] = 36;
		tm_phoenix[62] = 36;
		tm_phoenix[63] = 0;
		tm_phoenix[64] = 63;
		tm_phoenix[65] = 64;
		tm_phoenix[66] = 65;
		tm_phoenix[67] = 63;
		tm_phoenix[68] = 63;
		tm_phoenix[69] = 0;
		tm_phoenix[70] = 69;
		tm_phoenix[71] = 70;
		tm_phoenix[72] = 71;
		tm_phoenix[73] = 69;
		tm_phoenix[74] = 69;
		tm_phoenix[75] = 38;
		tm_phoenix[76] = 10;
		tm_phoenix[77] = -1;
		tm_phoenix[78] = 66;
		tm_phoenix[79] = 72;
		tm_phoenix[80] = 3;
	}
} Tm_phoenixInit;

struct tm_pirateInit
{
	tm_pirateInit()
	{
		tm_pirate[0] = -1;
		tm_pirate[1] = 0;
		tm_pirate[2] = 1;
		tm_pirate[3] = 2;
		tm_pirate[4] = 3;
		tm_pirate[5] = 4;
		tm_pirate[6] = 5;
		tm_pirate[7] = 5;
		tm_pirate[8] = 5;
		tm_pirate[9] = 4;
		tm_pirate[10] = 9;
		tm_pirate[11] = 10;
		tm_pirate[12] = 11;
		tm_pirate[13] = 12;
		tm_pirate[14] = 13;
		tm_pirate[15] = 14;
		tm_pirate[16] = 15;
		tm_pirate[17] = 12;
		tm_pirate[18] = 17;
		tm_pirate[19] = 18;
		tm_pirate[20] = 19;
		tm_pirate[21] = 12;
		tm_pirate[22] = 21;
		tm_pirate[23] = 22;
		tm_pirate[24] = 23;
		tm_pirate[25] = 12;
		tm_pirate[26] = 25;
		tm_pirate[27] = 26;
		tm_pirate[28] = 12;
		tm_pirate[29] = 28;
		tm_pirate[30] = 29;
		tm_pirate[31] = 30;
		tm_pirate[32] = 12;
		tm_pirate[33] = 11;
		tm_pirate[34] = 11;
		tm_pirate[35] = 10;
		tm_pirate[36] = 10;
		tm_pirate[37] = 4;
		tm_pirate[38] = 37;
		tm_pirate[39] = 38;
		tm_pirate[40] = 39;
		tm_pirate[41] = 40;
		tm_pirate[42] = 41;
		tm_pirate[43] = 42;
		tm_pirate[44] = 43;
		tm_pirate[45] = 40;
		tm_pirate[46] = 45;
		tm_pirate[47] = 46;
		tm_pirate[48] = 47;
		tm_pirate[49] = 40;
		tm_pirate[50] = 49;
		tm_pirate[51] = 50;
		tm_pirate[52] = 51;
		tm_pirate[53] = 40;
		tm_pirate[54] = 53;
		tm_pirate[55] = 54;
		tm_pirate[56] = 40;
		tm_pirate[57] = 56;
		tm_pirate[58] = 57;
		tm_pirate[59] = 58;
		tm_pirate[60] = 40;
		tm_pirate[61] = 39;
		tm_pirate[62] = 39;
		tm_pirate[63] = 38;
		tm_pirate[64] = 38;
		tm_pirate[65] = 0;
		tm_pirate[66] = 65;
		tm_pirate[67] = 66;
		tm_pirate[68] = 67;
		tm_pirate[69] = 65;
		tm_pirate[70] = 65;
		tm_pirate[71] = 0;
		tm_pirate[72] = 71;
		tm_pirate[73] = 72;
		tm_pirate[74] = 73;
		tm_pirate[75] = 71;
		tm_pirate[76] = 71;
		tm_pirate[77] = 40;
		tm_pirate[78] = 12;
		tm_pirate[79] = -1;
		tm_pirate[80] = 68;
		tm_pirate[81] = 74;
		tm_pirate[82] = 4;
	}
} Tm_pirateInit;

struct tm_professionalInit
{
	tm_professionalInit()
	{
		tm_professional[0] = -1;
		tm_professional[1] = 0;
		tm_professional[2] = 1;
		tm_professional[3] = 2;
		tm_professional[4] = 3;
		tm_professional[5] = 4;
		tm_professional[6] = 5;
		tm_professional[7] = 5;
		tm_professional[8] = 4;
		tm_professional[9] = 8;
		tm_professional[10] = 9;
		tm_professional[11] = 10;
		tm_professional[12] = 11;
		tm_professional[13] = 12;
		tm_professional[14] = 13;
		tm_professional[15] = 14;
		tm_professional[16] = 11;
		tm_professional[17] = 16;
		tm_professional[18] = 17;
		tm_professional[19] = 18;
		tm_professional[20] = 11;
		tm_professional[21] = 20;
		tm_professional[22] = 21;
		tm_professional[23] = 22;
		tm_professional[24] = 11;
		tm_professional[25] = 24;
		tm_professional[26] = 25;
		tm_professional[27] = 11;
		tm_professional[28] = 27;
		tm_professional[29] = 28;
		tm_professional[30] = 29;
		tm_professional[31] = 11;
		tm_professional[32] = 10;
		tm_professional[33] = 10;
		tm_professional[34] = 9;
		tm_professional[35] = 9;
		tm_professional[36] = 4;
		tm_professional[37] = 36;
		tm_professional[38] = 37;
		tm_professional[39] = 38;
		tm_professional[40] = 39;
		tm_professional[41] = 40;
		tm_professional[42] = 41;
		tm_professional[43] = 42;
		tm_professional[44] = 39;
		tm_professional[45] = 44;
		tm_professional[46] = 45;
		tm_professional[47] = 46;
		tm_professional[48] = 39;
		tm_professional[49] = 48;
		tm_professional[50] = 49;
		tm_professional[51] = 50;
		tm_professional[52] = 39;
		tm_professional[53] = 52;
		tm_professional[54] = 53;
		tm_professional[55] = 39;
		tm_professional[56] = 55;
		tm_professional[57] = 56;
		tm_professional[58] = 57;
		tm_professional[59] = 39;
		tm_professional[60] = 38;
		tm_professional[61] = 38;
		tm_professional[62] = 37;
		tm_professional[63] = 37;
		tm_professional[64] = 4;
		tm_professional[65] = 3;
		tm_professional[66] = 65;
		tm_professional[67] = 3;
		tm_professional[68] = 0;
		tm_professional[69] = 68;
		tm_professional[70] = 69;
		tm_professional[71] = 70;
		tm_professional[72] = 68;
		tm_professional[73] = 68;
		tm_professional[74] = 0;
		tm_professional[75] = 74;
		tm_professional[76] = 75;
		tm_professional[77] = 76;
		tm_professional[78] = 74;
		tm_professional[79] = 74;
		tm_professional[80] = 39;
		tm_professional[81] = 11;
		tm_professional[82] = -1;
		tm_professional[83] = 71;
		tm_professional[84] = 77;
		tm_professional[85] = 3;
	}
} Tm_professionalInit;

struct tm_separatistInit
{
	tm_separatistInit()
	{
		tm_separatist[0] = -1;
		tm_separatist[1] = 0;
		tm_separatist[2] = 1;
		tm_separatist[3] = 2;
		tm_separatist[4] = 3;
		tm_separatist[5] = 4;
		tm_separatist[6] = 5;
		tm_separatist[7] = 4;
		tm_separatist[8] = 7;
		tm_separatist[9] = 8;
		tm_separatist[10] = 9;
		tm_separatist[11] = 10;
		tm_separatist[12] = 11;
		tm_separatist[13] = 12;
		tm_separatist[14] = 13;
		tm_separatist[15] = 10;
		tm_separatist[16] = 15;
		tm_separatist[17] = 16;
		tm_separatist[18] = 17;
		tm_separatist[19] = 10;
		tm_separatist[20] = 19;
		tm_separatist[21] = 20;
		tm_separatist[22] = 21;
		tm_separatist[23] = 10;
		tm_separatist[24] = 23;
		tm_separatist[25] = 24;
		tm_separatist[26] = 10;
		tm_separatist[27] = 26;
		tm_separatist[28] = 27;
		tm_separatist[29] = 28;
		tm_separatist[30] = 10;
		tm_separatist[31] = 9;
		tm_separatist[32] = 9;
		tm_separatist[33] = 8;
		tm_separatist[34] = 8;
		tm_separatist[35] = 4;
		tm_separatist[36] = 35;
		tm_separatist[37] = 36;
		tm_separatist[38] = 37;
		tm_separatist[39] = 38;
		tm_separatist[40] = 39;
		tm_separatist[41] = 40;
		tm_separatist[42] = 41;
		tm_separatist[43] = 38;
		tm_separatist[44] = 43;
		tm_separatist[45] = 44;
		tm_separatist[46] = 45;
		tm_separatist[47] = 38;
		tm_separatist[48] = 47;
		tm_separatist[49] = 48;
		tm_separatist[50] = 49;
		tm_separatist[51] = 38;
		tm_separatist[52] = 51;
		tm_separatist[53] = 52;
		tm_separatist[54] = 38;
		tm_separatist[55] = 54;
		tm_separatist[56] = 55;
		tm_separatist[57] = 56;
		tm_separatist[58] = 38;
		tm_separatist[59] = 37;
		tm_separatist[60] = 37;
		tm_separatist[61] = 36;
		tm_separatist[62] = 36;
		tm_separatist[63] = 0;
		tm_separatist[64] = 63;
		tm_separatist[65] = 64;
		tm_separatist[66] = 65;
		tm_separatist[67] = 63;
		tm_separatist[68] = 63;
		tm_separatist[69] = 0;
		tm_separatist[70] = 69;
		tm_separatist[71] = 70;
		tm_separatist[72] = 71;
		tm_separatist[73] = 69;
		tm_separatist[74] = 69;
		tm_separatist[75] = 38;
		tm_separatist[76] = 10;
		tm_separatist[77] = -1;
		tm_separatist[78] = 66;
		tm_separatist[79] = 72;
		tm_separatist[80] = 3;
	}
} Tm_separatistInit;