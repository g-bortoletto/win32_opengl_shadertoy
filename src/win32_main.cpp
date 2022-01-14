#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#pragma comment(lib, "opengl32.lib")

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "glad/glad.h"

static bool m_isRunning;

#define GL_TRUE 1
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_TYPE_RGBA_ARB                       0x202B

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

typedef BOOL wgl_choose_pixel_format_arb_proc(
	HDC hdc,
	const int* piAttribIList,
	const FLOAT* pfAttribFList,
	UINT nMaxFormats,
	int* piFormats,
	UINT* nNumFormats);
static wgl_choose_pixel_format_arb_proc* _wglChoosePixelFormatARB;
#define wglChoosePixelFormatARB _wglChoosePixelFormatARB

typedef HGLRC wgl_create_context_attribs_arb_proc(HDC hDC, HGLRC hshareContext, const int* attribList);
static wgl_create_context_attribs_arb_proc* _wglCreateContextAttribsARB;
#define wglCreateContextAttribsARB _wglCreateContextAttribsARB

float m_vertices[]
{
	-0.5f, -0.5f, 0.0f,
	-0.5f,  0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.5f,  0.5f, 0.0f,
};

GLuint m_indices[]
{
	0, 1, 2,
	1, 2, 3,
};

GLuint MakeShaderProgram(bool, bool, bool);











LRESULT MainWindowCallback(
	HWND window, 
	UINT message, 
	WPARAM wParam, 
	LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
		case WM_SIZE:
		{
			if (glViewport)
			{
				int width = (int)lParam;
				int height = (int)(lParam >> 32);
				glViewport(0, 0, width, height);
			}
		}
		break;
		case WM_QUIT:
		case WM_CLOSE:
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			m_isRunning = false;
		}
		break;
		default:
		{
			result = DefWindowProcW(window, message, wParam, lParam);
		}
	}

	return result;
}










static void LoadOpenGLExtensions()
{
	WNDCLASSW windowClass = {};
	windowClass.lpfnWndProc = DefWindowProcW;
	windowClass.hInstance = GetModuleHandle(0);
	windowClass.lpszClassName = L"StubWindowClass";

	if (!RegisterClassW(&windowClass))
	{
		return;
	}

	HWND window = CreateWindowW(
		windowClass.lpszClassName,
		L"🙂",
		0, 0, 0, 0, 0,
		0, 0, windowClass.hInstance, 0);

	if (!window)
	{
		UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return;
	}

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC stubWindowDC = GetDC(window);
	int suggestedPixelFormat = ChoosePixelFormat(stubWindowDC, &pfd);
	SetPixelFormat(stubWindowDC, suggestedPixelFormat, &pfd);

	HGLRC stubGLContext = wglCreateContext(stubWindowDC);
	wglMakeCurrent(stubWindowDC, stubGLContext);

	wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb_proc*)wglGetProcAddress("wglChoosePixelFormatARB");
	if (!wglChoosePixelFormatARB)
	{
		DestroyWindow(window);
		UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return;
	}

	wglCreateContextAttribsARB = (wgl_create_context_attribs_arb_proc*)wglGetProcAddress("wglCreateContextAttribsARB");
	if (!wglCreateContextAttribsARB)
	{
		DestroyWindow(window);
		UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return;
	}

	wglMakeCurrent(stubWindowDC, 0);
	wglDeleteContext(stubGLContext);
	ReleaseDC(window, stubWindowDC);
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}










static GLuint MakeShaderProgram(bool vertex = false, bool fragment = false, bool geometry = false)
{
	FILE* stream = 0;
	
	char* vertexSource = 0;
	GLuint vertexShader = 0;
	if (vertex)
	{
		stream = fopen("vertexshader.glsl", "rb");
		if (!stream)
		{
			return 0;
		}
		fseek(stream, 0, SEEK_END);
		long fileSize = ftell(stream);
		fseek(stream, 0, SEEK_SET);
		
		vertexSource = (char*)malloc(fileSize + 1);
		if (!vertexSource)
		{
			return 0;
		}

		fread(vertexSource, 1, fileSize, stream);
		vertexSource[fileSize - 1] = '\0';

		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSource, 0);
		glCompileShader(vertexShader);
		int success = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			return 0;
		}
	}

	char* fragmentSource = 0;
	GLuint fragmentShader = 0;
	if (fragment)
	{
		stream = fopen("fragmentshader.glsl", "rb");
		if (!stream)
		{
			return 0;
		}
		fseek(stream, 0, SEEK_END);
		long fileSize = ftell(stream);
		fseek(stream, 0, SEEK_SET);

		fragmentSource = (char*)malloc(fileSize + 1);
		if (!fragmentSource)
		{
			return 0;
		}

		fread(fragmentSource, 1, fileSize, stream);
		fragmentSource[fileSize - 1] = '\0';

		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, 0);
		glCompileShader(fragmentShader);
		int success = 0;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			return 0;
		}
	}

	char* geometrySource = 0;
	if (geometry)
	{
	}

	GLuint shaderProgram = 0;

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	int success = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		return 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	if (vertexSource)
	{
		free(vertexSource);
	}
	if (fragmentSource)
	{
		free(fragmentSource);
	}
	if (geometrySource)
	{
		free(geometrySource);
	}

	fclose(stream);

	return shaderProgram;
}










int wWinMain(
	HINSTANCE instance, 
	HINSTANCE previousInstance, 
	LPWSTR args, 
	int openParam)
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	WNDCLASSW windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = MainWindowCallback;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.lpszClassName = L"MainWindowClass";

	if (!RegisterClassW(&windowClass))
	{
		return 1;
	}

	HWND window = CreateWindowW(
		windowClass.lpszClassName,
		L"🙂",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		900, 900,
		0, 0, windowClass.hInstance, 0);

	if (!window)
	{
		return 2;
	}

	HDC windowDC = GetDC(window);

	LoadOpenGLExtensions();
	const int attribList[] =
	{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0,
	};

	int pixelFormat;
	UINT numFormats;
	HGLRC openGLContext;

	wglChoosePixelFormatARB(windowDC, attribList, 0, 1, &pixelFormat, &numFormats);
	PIXELFORMATDESCRIPTOR pfd = {};
	DescribePixelFormat(windowDC, pixelFormat, sizeof(pfd), &pfd);
	if (!SetPixelFormat(windowDC, pixelFormat, &pfd))
	{
		DestroyWindow(window);
		UnregisterClassW(windowClass.lpszClassName, windowClass.hInstance);
		return 3;
	}

	int modernGLAttribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0,
	};

	openGLContext = wglCreateContextAttribsARB(windowDC, 0, modernGLAttribs);
	wglMakeCurrent(windowDC, openGLContext);

	if (!gladLoadGL())
	{
		return 4;
	}

	GLuint shaderProgram = MakeShaderProgram(true, true, false);

	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW); 

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	int uniformTimeLocation = glGetUniformLocation(shaderProgram, "time");

	LARGE_INTEGER lastQuery;
	LARGE_INTEGER currentQuery;
	float deltaTime = 0;

	QueryPerformanceCounter(&currentQuery);

	m_isRunning = true;
	while (m_isRunning)
	{
		MSG msg;
		while (PeekMessageW(&msg, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
			if (WM_QUIT == msg.message)
			{
				m_isRunning = false;
			}
		}
		if (!m_isRunning)
		{
			break;
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glUniform1f(uniformTimeLocation, deltaTime);

		SwapBuffers(windowDC);
		if (GetAsyncKeyState(VK_F5))
		{
			shaderProgram = MakeShaderProgram(true, true, false);
		}

		deltaTime = (float)(currentQuery.QuadPart - currentQuery.QuadPart) / (float)frequency.QuadPart;
		lastQuery = currentQuery;
		QueryPerformanceCounter(&currentQuery);
	}

	wglMakeCurrent(windowDC, 0);
	if (openGLContext) wglDeleteContext(openGLContext);
	DestroyWindow(window);
	UnregisterClassW(windowClass.lpszClassName, windowClass.hInstance);

	return 0;
}