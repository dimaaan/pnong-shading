#pragma once

// ��������� c ����������� � �������
struct Vertex {
	float x, y, z;        // ���������� �������
	float rx, ry, rz;     // ��������������� (����������) ���������� �������
	float sx, sy;         // �������� (���������������) ���������� �������
	float nx, ny, nz;     // ���������� ������� ������� � �������
	float rnx, rny, rnz;  // ��������������� (����������) ���������� �������
	float cc;             // ������������ �������
	float u, v;           // ��������������� �� ���������� ��������
	float uz, vz, z1;     // u/z, v/z, 1/z
};

// ��������� c ����������� � �����
struct Face {
	Vertex *v1, *v2, *v3; // ��������� �� ������� �����
	float nx, ny, nz;     // ���������� ������� ������� � �����
};

// ������� ����
const int WND_WIDTH = 640;
const int WND_HEIGHT = 480;

// ���������� ���������
extern LPDIRECT3D9				pD3D;
extern LPDIRECT3DDEVICE9		pD3DDevice;
extern float RotAngle;	

const int TORE_VERTS = 25;					// ���������� ������ � ����
const int TORE_FACES = 50;					// ���������� ������ � ����
const int TORE_RADIUS = 100;				// ������ ����
const int TORE_THICKNESS = 30;				// ������� ����
const int CUBE_FACES = 12;					// ������ � ����
const int CUBE_VERTS = 8;					// ������ � ����

extern Face CubeFaces[12];
extern Vertex CubeVertex[8];					// ������� ����
extern Face ToreFaces[TORE_FACES];
extern Vertex ToreVerts[TORE_VERTS];

extern float ZBuffer[WND_WIDTH][WND_HEIGHT];

// ���������� ������������ �������
void InitCube(Face CubeFaces[]);
void RotateMesh(Vertex pVerts[], int n);
void DrawShaded(Face *pFaces, int n);
void CalcPhongMap();
void InitTore(int segs1, int segs2);