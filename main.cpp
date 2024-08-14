#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

// 定义 Base64 编码表
const string base64Chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

//自实现的数据转base64
string base64Encode(const vector<unsigned char>& data) {
	string encoded;
	int i = 0;
	int j = 0;
	unsigned char charArray3[3];
	unsigned char charArray4[4];
	int dataLength = data.size();
	while (dataLength--) {
		charArray3[i++] = *(data.data() + j++);
		if (i == 3) {
			charArray4[0] = (charArray3[0] & 0xfc) >> 2;
			charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
			charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
			charArray4[3] = charArray3[2] & 0x3f;

			for (i = 0; i < 4; i++) {
				encoded += base64Chars[charArray4[i]];
			}
			i = 0;
		}
	}
	if (i) {
		for (int k = i; k < 3; k++) {
			charArray3[k] = '\0';
		}
		charArray4[0] = (charArray3[0] & 0xfc) >> 2;
		charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
		charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
		charArray4[3] = charArray3[2] & 0x3f;

		for (int k = 0; k < i + 1; k++) {
			encoded += base64Chars[charArray4[k]];
		}

		while (i++ < 3) {
			encoded += '=';
		}
	}
	return encoded;
}

//一个存储当前处理过的base64字符串的全局变量
string parsedB64String = "";

// 窗口过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HWND hEditPath;
	static HWND hTextResult;
	static HWND hButtonConvert;
	static HWND hButtonCopy;
	static HWND hButtonCopyMD;
	static HWND hButtonCopyMDJ;

	switch (msg) {
		case WM_CREATE:
			hEditPath = CreateWindow(TEXT("EDIT"), TEXT(""),
			                         WS_CHILD | WS_VISIBLE | WS_BORDER,
			                         10, 10, 660, 40, hwnd, NULL, NULL, NULL);
			SendMessage(hEditPath, EM_SETLIMITTEXT, 512, 0);  // 将最大输入长度设置为 512 个字符

			hButtonConvert = CreateWindow(TEXT("BUTTON"), TEXT("转换"),
			                              WS_CHILD | WS_VISIBLE,
			                              680, 10, 80, 40, hwnd, (HMENU)1, NULL, NULL);

			hTextResult = CreateWindow(TEXT("EDIT"), TEXT(""),
			                           WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY,
			                           10, 60, 750, 300, hwnd, NULL, NULL, NULL);

			hButtonCopy = CreateWindow(TEXT("BUTTON"), TEXT("复制到剪贴板"),
			                           WS_CHILD | WS_VISIBLE,
			                           60, 380, 200, 40, hwnd, (HMENU)2, NULL, NULL);
			hButtonCopyMD = CreateWindow(TEXT("BUTTON"), TEXT("复制到MD(png)"),
			                             WS_CHILD | WS_VISIBLE,
			                             280, 380, 200, 40, hwnd, (HMENU)3, NULL, NULL);
			hButtonCopyMDJ = CreateWindow(TEXT("BUTTON"), TEXT("复制到MD(jpeg)"),
			                              WS_CHILD | WS_VISIBLE,
			                              500, 380, 200, 40, hwnd, (HMENU)4, NULL, NULL);
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == 1) {  // 转换按钮
				int len = GetWindowTextLength(hEditPath);
				vector<char> buffer(len + 1);
				GetWindowText(hEditPath, buffer.data(), len + 1);
				string imagePath(buffer.begin(), buffer.end());
				ifstream file(imagePath, ios::binary);
				if (!file) {
					MessageBox(hwnd, TEXT("无法打开文件"), TEXT("错误"), MB_OK | MB_ICONERROR);
					return 0;
				}
				vector<unsigned char> imageData((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
				file.close();
				parsedB64String = base64Encode(imageData);
				SetWindowText(hTextResult, parsedB64String.c_str());
			} else if (LOWORD(wParam) == 2) {  // 复制文本按钮
				int len = parsedB64String.length();
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(char));
				if (hMem) {
					LPTSTR pMem = (LPTSTR)GlobalLock(hMem);
					if (pMem) {
						memcpy(pMem, parsedB64String.c_str(), (len + 1) * sizeof(char));
						GlobalUnlock(hMem);

						OpenClipboard(hwnd);
						EmptyClipboard();
						SetClipboardData(CF_TEXT, hMem);
						CloseClipboard();
					}
				}
			} else if (LOWORD(wParam) == 3) {  // 新增：复制到 MD”按钮
				string mdContent = "![name](data:image/png;base64," + parsedB64String + ")";  // 构建要复制的内容
				int len = mdContent.length();
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(char));
				if (hMem) {
					LPTSTR pMem = (LPTSTR)GlobalLock(hMem);
					if (pMem) {
						memcpy(pMem, mdContent.c_str(), (len + 1) * sizeof(char));
						GlobalUnlock(hMem);
						OpenClipboard(hwnd);
						EmptyClipboard();
						SetClipboardData(CF_TEXT, hMem);
						CloseClipboard();
					}
				}
			} else if (LOWORD(wParam) == 4) { // 新增：复制到 MD”按钮（jpg）
				string mdContent = "![name](data:image/jpeg;base64," + parsedB64String + ")";  // 构建要复制的内容
				int len = mdContent.length();
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(char));
				if (hMem) {
					LPTSTR pMem = (LPTSTR)GlobalLock(hMem);
					if (pMem) {
						memcpy(pMem, mdContent.c_str(), (len + 1) * sizeof(char));
						GlobalUnlock(hMem);
						OpenClipboard(hwnd);
						EmptyClipboard();
						SetClipboardData(CF_TEXT, hMem);
						CloseClipboard();
					}
				}
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//高清显示
	SetProcessDPIAware();

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("Base64Converter");

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, TEXT("注册窗口类失败"), TEXT("错误"), MB_OK | MB_ICONERROR);
		return 1;
	}

	HWND hwnd = CreateWindow(TEXT("Base64Converter"), TEXT("图片转 Base64 工具 By 怜渠客(吾爱)"),
	                         WS_VISIBLE |
	                         (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME &
	                          ~WS_MAXIMIZEBOX), /*设置样式，查了很久很久。这样可以禁止窗口缩放和最大化*/
	                         CW_USEDEFAULT, CW_USEDEFAULT, 800, 500,
	                         NULL, NULL, hInstance, NULL);

	if (!hwnd) {
		MessageBox(NULL, TEXT("创建窗口失败"), TEXT("错误"), MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
