#define _USE_MATH_DEFINES						// объявляет константу Пи
#define WIN32_LEAN_AND_MEAN						// исключаем редкоиспользуемые загаловочные файлы windows

#include <math.h>								// математические функции
#include <windows.h>							// типы и функции Windows
#include <d3d9.h>								// используем Direct3D
#include "PhongShading.h"

LPDIRECT3D9						pD3D;			// объект Direct3D
LPDIRECT3DDEVICE9				pD3DDevice;		// устройство Direct3D (работает с видеокартой)

int CameraDist = WND_WIDTH;						// расстояние камеры до начала координат
int Ambient = 70;								// фоновая освещенность
float RotAngle;									// текущий угол поворота фигуры

Vertex CubeVertex[8];							// вершины куба
Face CubeFaces[12];								// грани куба

Vertex ToreVerts[TORE_VERTS];					// Вершины тора
Face   ToreFaces[TORE_FACES];					// Грани тора

unsigned short Texture[256][256];				// текстура с картой Фонга

float ZBuffer[WND_WIDTH][WND_HEIGHT];			// w - буфер ( 1/z[i], где z[] - z-буффер)

// проектирует точки из 3D пространста в 2D плоскость экрана
void ProjectVertex(Vertex &v) {
	v.sx = WND_WIDTH  / 2 + v.rx * CameraDist / (v.rz + CameraDist);
	v.sy = WND_HEIGHT / 2 - v.ry * CameraDist / (v.rz + CameraDist);
}

// вращает точку по осям X, Y и Z на заданный угол
void RotateVertex(Vertex *v, float angle) {
	float mysin = sin(angle);
	float mycos = cos(angle);
	float x;

	// Поворачиваем саму точку
	v->rx = v->x;                   // ox
	v->rz = v->z*mycos-v->y*mysin;
	v->ry = v->z*mysin+v->y*mycos;
	x = v->rx;                      // oy
	v->rx = x*mycos-v->rz*mysin;
	v->rz = x*mysin+v->rz*mycos;
	x = v->rx;                      // oz
	v->rx = x*mycos-v->ry*mysin;
	v->ry = x*mysin+v->ry*mycos;
	// Поворачиваем нормаль к объекту в этой точке
	v->rnx = v->nx;                 // ox
	v->rnz = v->nz*mycos-v->ny*mysin;
	v->rny = v->nz*mysin+v->ny*mycos;
	x = v->rnx;                     // oy
	v->rnx = x*mycos-v->rnz*mysin;
	v->rnz = x*mysin+v->rnz*mycos;
	x = v->rnx;                     // oz
	v->rnx = x*mycos-v->rny*mysin;
	v->rny = x*mysin+v->rny*mycos;
}

// заносит пиксель в видеопамять
void PutPixelInBackBuf(int x, int y, D3DCOLOR Col, D3DLOCKED_RECT &Rect) {
	BYTE *pB = (BYTE*) Rect.pBits;

	pB[y * Rect.Pitch + 4*x] = (BYTE)Col;
}

// рисует затененный по Фонгу полигон
void DrawFace(Face *f, D3DLOCKED_RECT &Rect) {
	Vertex *a, *b, *c, *Vert;
	char *dest;
	int CurrSX, CurrSY, Len;
	float StartX, EndX, StartU, StartV, EndU, EndV;
	float StartDX, EndDX, StartDU, StartDV, StartDz1;
	float EndDU, EndDv, EndDz1, StartZ1, EndZ1;
	float x, u, v, z1, du, dv, dz1;
	float tmp, k;

	// Отсортируем вершины грани по sy
	a = f->v1; b = f->v2; c = f->v3;
	if(a->sy>b->sy) { Vert = a; a = b; b = Vert; }
	if(a->sy>c->sy) { Vert = a; a = c; c = Vert; }
	if(b->sy>c->sy) { Vert = b; b = c; c = Vert; }
	// Грань нулевой высоты рисовать не будем
	if(floor(c->sy)<=ceil(a->sy)) return;
	// Посчитаем du/dsx, dv/dsx, d(1/z)/dsx
	// Считаем по самой длинной линии (т.е. проходящей через среднюю по высоте)
	a->z1 = 1/(a->rz+CameraDist);
	b->z1 = 1/(b->rz+CameraDist);
	c->z1 = 1/(c->rz+CameraDist);
	k = (b->sy-a->sy)/(c->sy-a->sy);
	StartX = a->sx+(c->sx-a->sx)*k;
	StartU = a->u+(c->u-a->u)*k;
	StartV = a->v+(c->v-a->v)*k;
	StartZ1 = a->z1+(c->z1-a->z1)*k;
	EndX = b->sx;
	EndU = b->u;
	EndV = b->v;
	EndZ1 = b->z1;
	du = (StartU-EndU)/(StartX-EndX);
	dv = (StartV-EndV)/(StartX-EndX);
	dz1 = (StartZ1-EndZ1)/(StartX-EndX);

	StartX = a->sx;
	StartU = a->u;
	StartV = a->v;
	StartZ1 = a->z1;
	StartDX = (c->sx-a->sx)/(c->sy-a->sy);
	StartDU = (c->u-a->u)/(c->sy-a->sy);
	StartDV = (c->v-a->v)/(c->sy-a->sy);
	StartDz1 = (c->z1-a->z1)/(c->sy-a->sy);

	if(ceil(b->sy)>ceil(a->sy)){
		tmp = ceil(a->sy)-a->sy;
		EndX = a->sx;
		EndU = a->u;
		EndV = a->v;
		EndZ1 = a->z1;
		EndDX = (b->sx-a->sx)/(b->sy-a->sy);
		EndDU = (b->u-a->u)/(b->sy-a->sy);
		EndDv = (b->v-a->v)/(b->sy-a->sy);
		EndDz1 = (b->z1-a->z1)/(b->sy-a->sy);
	} 
	else {
		tmp = ceil(b->sy)-b->sy;
		EndX = b->sx;
		EndU = b->u;
		EndV = b->v;
		EndZ1 = b->z1;
		EndDX = (c->sx-b->sx)/(c->sy-b->sy);
		EndDU = (c->u-b->u)/(c->sy-b->sy);
		EndDv = (c->v-b->v)/(c->sy-b->sy);
		EndDz1 = (c->z1-b->z1)/(c->sy-b->sy);
	}

	// отрисовка грани по строкам
	for(CurrSY = (int) ceil(a->sy); CurrSY<ceil(c->sy); CurrSY++){
		if(CurrSY==ceil(b->sy)){
			EndX = b->sx;
			EndU = b->u;
			EndV = b->v;
			EndZ1 = b->z1;
			EndDX = (c->sx-b->sx)/(c->sy-b->sy);
			EndDU = (c->u-b->u)/(c->sy-b->sy);
			EndDv = (c->v-b->v)/(c->sy-b->sy);
			EndDz1 = (c->z1-b->z1)/(c->sy-b->sy);
		}
		// StartX должен находиться левее EndX
		if(StartX>EndX){
			x = EndX;
			u = EndU;
			v = EndV;
			z1 = EndZ1;
			Len = int(ceil(StartX)-ceil(EndX));
		} else{
			x = StartX;
			u = StartU;
			v = StartV;
			z1 = StartZ1;
			Len = int(ceil(EndX)-ceil(StartX));
		}
		// Считаем адрес начала строки
		dest = 0;
		dest+=CurrSY*WND_WIDTH+(int)ceil(x);
		// Текстурируем строку
		CurrSX = (int)ceil(x);
		if(Len){
			while(Len--){
				// Используем z-буфер для определения видимости текущей точки
				if(ZBuffer[CurrSY][CurrSX]<=z1) {
					POINT p = {CurrSX, CurrSY};
					unsigned char cu=(unsigned char)u;
					unsigned char cv=(unsigned char)v;          
					unsigned char ccol = (unsigned char) Texture[cu][cv];

					PutPixelInBackBuf(p.x, p.y, D3DCOLOR_XRGB(ccol,ccol,ccol), Rect);
					ZBuffer[CurrSY][CurrSX] = z1;
				}
				u+=du;
				v+=dv;
				z1+=dz1;
				dest++;
				CurrSX++;
			}
		}
		// Сдвигаем начальные и конечные значения x/u/v/(1/z)
		StartX+=StartDX;
		StartU+=StartDU;
		StartV+=StartDV;
		StartZ1+=StartDz1;
		EndX+=EndDX;
		EndU+=EndDU;
		EndV+=EndDv;
		EndZ1+=EndDz1;
	}
}

// рисует фигуру
void DrawShaded(Face *pFaces, int n) {
	LPDIRECT3DSURFACE9 BackBuf;
	D3DLOCKED_RECT Rect;

	pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuf);
	BackBuf->LockRect(&Rect, 0, D3DLOCK_DISCARD);
	for(int i = 0; i < n; i++) DrawFace(&pFaces[i], Rect);
	BackBuf->UnlockRect();
}

// Сгенерировать карту Фонга
void CalcPhongMap() {
for(int i = 0; i<256; i++)
	for (int j = 0; j<256; j++)
		Texture[i][j] =
			Ambient+(255-Ambient)*pow(sin(i*M_PI/256)*sin(j*M_PI/256), 4);
}

// считает нормаль к граням и вершинам
void CalcNormals(Face *fl, int numfaces, Vertex *vl, unsigned short numverts){
	float ax, ay, az, bx, by, bz, nx, ny, nz, l;
	int i;

	// Вычисление нормалей к граням
	for(i = 0; i<numfaces; i++){
		ax = fl[i].v3->x-fl[i].v1->x;
		ay = fl[i].v3->y-fl[i].v1->y;
		az = fl[i].v3->z-fl[i].v1->z;
		bx = fl[i].v2->x-fl[i].v1->x;
		by = fl[i].v2->y-fl[i].v1->y;
		bz = fl[i].v2->z-fl[i].v1->z;
		nx = fl[i].nx = ay*bz-az*by;
		ny = fl[i].ny = az*bx-ax*bz;
		nz = fl[i].nz = ax*by-ay*bx;
		// Нормализация
		l = sqrt(nx*nx+ny*ny+nz*nz);
		fl[i].nx/=l;
		fl[i].ny/=l;
		fl[i].nz/=l;
	}
	// Обнуляем нормали для каждой вершины
	for (i = 0; i<numverts; i++){
		vl[i].nx = 0;
		vl[i].ny = 0;
		vl[i].nz = 0;
		}
		// Нормаль каждой вершины - сумма нормалей всех прилежащих граней
		for (i = 0; i<numfaces; i++){
			fl[i].v1->nx+=fl[i].nx;
			fl[i].v1->ny+=fl[i].ny;
			fl[i].v1->nz+=fl[i].nz;
			fl[i].v2->nx+=fl[i].nx;
			fl[i].v2->ny+=fl[i].ny;
			fl[i].v2->nz+=fl[i].nz;
			fl[i].v3->nx+=fl[i].nx;
			fl[i].v3->ny+=fl[i].ny;
			fl[i].v3->nz+=fl[i].nz;
		}
		// Нормализуем нормали в каждой вершине
		for (i = 0; i<numverts; i++) {
		l = sqrt(vl[i].nx*vl[i].nx+vl[i].ny*vl[i].ny+vl[i].nz*vl[i].nz);
		vl[i].nx/=l;
		vl[i].ny/=l;
		vl[i].nz/=l;
	}
}

// считает координаты вершин куба и нормали
void InitCube(Face CubeFaces[]) {
	const int SideLen = 100;
	unsigned short hs = SideLen>>1, i; // Половинная длина стороны

	// Вычисление вершин
	for(i = 0; i<8; i++){
		CubeVertex[i].x = (float)-2*hs*(i&1)+hs;
		CubeVertex[i].y = (float)-2*hs*((i>>1)&1)+hs;
		CubeVertex[i].z = (float)-2*hs*((i>>2)&1)+hs;
		CubeVertex[i].u = 0;
		CubeVertex[i].v = 0;
	}

	// Вычисление граней
	CubeFaces[0].v1 = &CubeVertex[1];
	CubeFaces[0].v2 = &CubeVertex[0];
	CubeFaces[0].v3 = &CubeVertex[2];
	CubeFaces[1].v1 = &CubeVertex[1];
	CubeFaces[1].v2 = &CubeVertex[2];
	CubeFaces[1].v3 = &CubeVertex[3];
	CubeFaces[2].v1 = &CubeVertex[5];
	CubeFaces[2].v2 = &CubeVertex[1];
	CubeFaces[2].v3 = &CubeVertex[3];
	CubeFaces[3].v1 = &CubeVertex[5];
	CubeFaces[3].v2 = &CubeVertex[3];
	CubeFaces[3].v3 = &CubeVertex[7];
	CubeFaces[4].v1 = &CubeVertex[4];
	CubeFaces[4].v2 = &CubeVertex[5];
	CubeFaces[4].v3 = &CubeVertex[7];
	CubeFaces[5].v1 = &CubeVertex[4];
	CubeFaces[5].v2 = &CubeVertex[7];
	CubeFaces[5].v3 = &CubeVertex[6];
	CubeFaces[6].v1 = &CubeVertex[0];
	CubeFaces[6].v2 = &CubeVertex[4];
	CubeFaces[6].v3 = &CubeVertex[6];
	CubeFaces[7].v1 = &CubeVertex[0];
	CubeFaces[7].v2 = &CubeVertex[6];
	CubeFaces[7].v3 = &CubeVertex[2];
	CubeFaces[8].v1 = &CubeVertex[3];
	CubeFaces[8].v2 = &CubeVertex[2];
	CubeFaces[8].v3 = &CubeVertex[6];
	CubeFaces[9].v1 = &CubeVertex[3];
	CubeFaces[9].v2 = &CubeVertex[6];
	CubeFaces[9].v3 = &CubeVertex[7];
	CubeFaces[10].v1 = &CubeVertex[1];
	CubeFaces[10].v2 = &CubeVertex[5];
	CubeFaces[10].v3 = &CubeVertex[4];
	CubeFaces[11].v1 = &CubeVertex[1];
	CubeFaces[11].v2 = &CubeVertex[4];
	CubeFaces[11].v3 = &CubeVertex[0];

	// Вычисление нормалей
	CalcNormals(CubeFaces, 12, CubeVertex, 8);
}

// считает координаты вершин тора и нормали
void InitTore(int segs1, int segs2) {
	int tnum_verts,tnum_faces;
	float a1, a2;
	int i, j, n;

	// Вычисление вершин
	tnum_verts = tnum_faces = n = 0;
	for(i = 0; i<segs1; i++){
		for(j = 0; j<segs2; j++){
			a1 = i*(M_PI*2/segs1);
			a2 = j*(M_PI*2/segs2);
			ToreVerts[tnum_verts].x = (TORE_RADIUS+TORE_THICKNESS*cos(a2))*cos(a1);
			ToreVerts[tnum_verts].y = (TORE_RADIUS+TORE_THICKNESS*cos(a2))*sin(a1);
			ToreVerts[tnum_verts].z = TORE_THICKNESS*sin(a2);
			ToreVerts[tnum_verts].u = 0;
			ToreVerts[tnum_verts].v = 0;
			tnum_verts++;
		}
	}
	// Вычисление граней
	for(i = 0; i<segs1; i++){
		for(j = 0; j<segs2; j++){
			ToreFaces[tnum_faces].v1 = &ToreVerts[n+j];
			ToreFaces[tnum_faces].v2 = &ToreVerts[(n+((j+1)%segs2))%tnum_verts];
			ToreFaces[tnum_faces].v3 = &ToreVerts[(n+((j+1)%segs2)+segs2)%tnum_verts];
			tnum_faces++;
			ToreFaces[tnum_faces].v1 = &ToreVerts[(n+((j+1)%segs2)+segs2)%tnum_verts];
			ToreFaces[tnum_faces].v2 = &ToreVerts[(n+j+segs2)%tnum_verts];
			ToreFaces[tnum_faces].v3 = &ToreVerts[n+j];
			tnum_faces++;
		}
		n+=segs2;
	}
	// Вычисление нормалей
	CalcNormals(ToreFaces, tnum_faces, ToreVerts, tnum_verts);
}

// вращает вершины на заданный угол всем 3ем осям
void RotateMesh(Vertex pVerts[], int n) {
	for(int i = 0; i < n; i++) {
		RotateVertex(&pVerts[i], RotAngle);
		ProjectVertex(pVerts[i]);
		// 128 - половина размера Фонг-карты
		pVerts[i].u = (1+pVerts[i].rnx) * 128;
		pVerts[i].v = (1+pVerts[i].rny) * 128;
	}
}