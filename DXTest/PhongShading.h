#pragma once

// структура c информацией о вершине
struct Vertex {
	float x, y, z;        // Координаты вершины
	float rx, ry, rz;     // Преобразованные (повернутые) координаты вершины
	float sx, sy;         // Экранные (спроецированные) координаты вершины
	float nx, ny, nz;     // Координаты вектора нормали к вершине
	float rnx, rny, rnz;  // Преобразованные (повернутые) координаты нормали
	float cc;             // Освещенность вершины
	float u, v;           // Соответствующие ей координаты текстуры
	float uz, vz, z1;     // u/z, v/z, 1/z
};

// структура c информацией о грани
struct Face {
	Vertex *v1, *v2, *v3; // Указатели на вершины грани
	float nx, ny, nz;     // Координаты вектора нормали к грани
};

// размеры окна
const int WND_WIDTH = 640;
const int WND_HEIGHT = 480;

// Глобальные перемнные
extern LPDIRECT3D9				pD3D;
extern LPDIRECT3DDEVICE9		pD3DDevice;
extern float RotAngle;	

const int TORE_VERTS = 25;					// количество вершин в торе
const int TORE_FACES = 50;					// количество граней в торе
const int TORE_RADIUS = 100;				// Радиус тора
const int TORE_THICKNESS = 30;				// Толщина тора
const int CUBE_FACES = 12;					// граней в кубе
const int CUBE_VERTS = 8;					// вершин в кубе

extern Face CubeFaces[12];
extern Vertex CubeVertex[8];					// вершины куба
extern Face ToreFaces[TORE_FACES];
extern Vertex ToreVerts[TORE_VERTS];

extern float ZBuffer[WND_WIDTH][WND_HEIGHT];

// объявления используемых функций
void InitCube(Face CubeFaces[]);
void RotateMesh(Vertex pVerts[], int n);
void DrawShaded(Face *pFaces, int n);
void CalcPhongMap();
void InitTore(int segs1, int segs2);