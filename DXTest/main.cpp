#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include "PhongShading.h"

#define WND_CLS_NAME	"Direct3D Window"
#define WND_CAPTION		"Затенение по Фонгу (нажмите ввод для др. фигуры)"

HWND hWnd;

// если истина рисуем куб, ложь - тор
bool DrawCube = true;

//--------------------------------------------------------------------------------------------
// процедура отрисовки
void Render() {

	// очищаем z-буффер
	for(int i = 0; i<WND_WIDTH; i++) {
		for(int q = 0; q<WND_HEIGHT; q++) ZBuffer[i][q] = 0;
	}

	RotAngle += 0.005f; // увеличиваем угол поворота
	// очищаем экран
	pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);

	if(DrawCube) { // вращаем и рисуем куб
		RotateMesh(CubeVertex, CUBE_VERTS);
		DrawShaded(CubeFaces, CUBE_FACES);
	}
	else {			// вращаем и рисуем тор
		RotateMesh(ToreVerts, TORE_VERTS);
		DrawShaded(ToreFaces, TORE_FACES);
	}

	// показываем кадр на экране
	pD3DDevice->Present(NULL, NULL, NULL, NULL);
}
//--------------------------------------------------------------------------------------------
// процедура инициализации Direct3D
void InitDX() {
	HRESULT hr;
	D3DPRESENT_PARAMETERS PresentParams;
	D3DDISPLAYMODE d3ddm;

	// создаем объект Direct3D
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(!pD3D) {
		MessageBox(hWnd, "DirectX 9 не установлен!", "", MB_OK);
		return;
	}

	// получаем режим дисплея
	hr = pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
    if(FAILED(hr)) {
		return;
	}
	
	// проверяем поддерживается ли режим
	hr = pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
									d3ddm.Format, D3DUSAGE_DEPTHSTENCIL,
									D3DRTYPE_SURFACE, D3DFMT_D16);
	if(FAILED(hr)) {
		return;
	}

	// заполняем структуру PresentParams, которая содержит различные параметры
	// отображения
	memset(&PresentParams, 0, sizeof(PresentParams));

    PresentParams.BackBufferFormat       = d3ddm.Format;
	PresentParams.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	PresentParams.Windowed               = true;
    PresentParams.EnableAutoDepthStencil = true;
    PresentParams.AutoDepthStencilFormat = D3DFMT_D16;
    PresentParams.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
	PresentParams.Flags					 = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	
	// создаем устрйоство Direct3D
	hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &PresentParams, &pD3DDevice);
    if(FAILED(hr)) {
		return;
	}

	// считаем карту Фонга
	CalcPhongMap();
	// считаем координаты вершин отображаемых фигур
	InitCube(CubeFaces);
	InitTore(5, 5);
}
//--------------------------------------------------------------------------------------------
// завершает работу DirectX
void ReleaseDX() {
	if(!pD3DDevice) {
        pD3DDevice->Release();
		pD3DDevice = NULL;
	}

	if(!pD3D ) {
        pD3D->Release();
		pD3D = NULL;
	}
}
//--------------------------------------------------------------------------------------------
// оконная процедура
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_KEYDOWN: // если нажата клавиша
			switch(wParam) {
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;
				case VK_RETURN:
					DrawCube = !DrawCube;
					break;
			}
			break;
		case WM_CLOSE:
			PostQuitMessage(0);
        case WM_DESTROY:
            PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
			break;
	}

	return 0;
}
//--------------------------------------------------------------------------------------------
// точка входа в программу
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX	winClass;
	MSG			uMsg;

	// создаем окно
    memset(&uMsg, 0, sizeof(uMsg));
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.lpszClassName = WND_CLS_NAME;
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = 0;
    winClass.hIconSm	   = 0;
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if(!RegisterClassEx(&winClass))return E_FAIL;

	hWnd = CreateWindowEx(NULL, WND_CLS_NAME, WND_CAPTION, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0, WND_WIDTH, WND_HEIGHT, NULL, NULL, hInstance, NULL);
	if(!hWnd) return E_FAIL;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

	// инициализаруем DirectX
	InitDX();

	// запускаем цикл обработки сообщений
	while(uMsg.message != WM_QUIT) {
		if(PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
		    Render();
	}

	// завершаем работу DirectX
	ReleaseDX();

	UnregisterClass(WND_CLS_NAME, winClass.hInstance);
	return (int) uMsg.wParam;
}