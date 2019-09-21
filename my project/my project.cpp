#include <bits/stdc++.h>
#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>
#include <regex>
using namespace std;

struct Flag {
	bool c, w, l, s, a, x;
}flag;

struct Result {
	int character, word, line;
	int blankLine, commentLine, codeLine;
}res;

string pattern;

void dirList(const string& strPath);
bool getPathWin(TCHAR* filepath);
void display();
bool isValidChar(char a);
void countNum(char* filepath);

//改用string写，用char会出现一些莫名奇怪的错误，例如相对路径与绝对路径的问题
void dirList(const string& strPath) {  
	string strRawPath = strPath;
	strRawPath.append("\\");

	string strFindPath = strRawPath;
	strFindPath.append("*.*");

	WIN32_FIND_DATAA winFindData;

	HANDLE hTemp = FindFirstFileA(strFindPath.c_str(), &winFindData);

	if (INVALID_HANDLE_VALUE == hTemp)
		return;
	while (FindNextFileA(hTemp, &winFindData)) {
		string strOldName = winFindData.cFileName;
		if ("." == strOldName || ".." == strOldName)
			continue;
		//如果是目录，则递归继续操作
		if (winFindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
			string strAgain = strPath;
			strAgain.append("\\");
			strAgain.append(winFindData.cFileName);
			dirList(strAgain);
			continue;
		}
		//获得绝对路径
		strOldName = strRawPath;
		strOldName.append(winFindData.cFileName);
		string strNewName = strOldName;
		//可以判断文件类型，从而过滤掉不要的文件
		if (pattern != "") {
			if (!regex_search(strNewName, regex(pattern)))
				continue;
		}

		cout << strNewName << endl;
		FILE* fp = fopen(strNewName.data(), "r");
		countNum((char *)strNewName.data());
		display();
		memset(&res, 0, sizeof(res));

		//strNewName.append(".temp");
		//更名以及删除文件,隐藏功能
		//rename(strOldName.c_str(), strNewName.c_str());
		//DeleteFileA(strNewName.c_str());
	}
	FindClose(hTemp);
}

bool getPathWin(TCHAR* szFileName) {
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = 0;
	ofn.lpstrDefExt = 0;
	ofn.lpstrFile = (LPWSTR)szFileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrInitialDir = 0;
	ofn.lpstrTitle = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn) == NULL) {
		printf("something wrong!\n");
		return false;
		//exit(0);
	}
	else {
		return true;
	}
}

void display() {
	if (flag.c)
		printf("%d character(s).\n", res.character);
	if (flag.w)
		printf("%d word(s).\n", res.word);
	if (flag.l)
		printf("%d line(s).\n", res.line);
	if (flag.a) {
		printf("%d blank line(s).\n", res.blankLine);
		printf("%d code line(s).\n", res.codeLine);
		printf("%d comment line(s).\n", res.commentLine);
	}
	printf("--------------------------------------------\n");
}

bool isValidChar(char a) {
	return a != ' ' && a != '\n' && a != '\t' && a != '\r' && a != '}' && a != '{';  //只想到这么些
}

void countNum(char *filepath) {
	FILE* fp = fopen(filepath, "r");
	if (fp == NULL) {
		printf("Maybe there is something wrong about the filepath!\n");
		printf("Please reopen this program and correct your filepath.\n");
		exit(0);
	}

	char bufferLine[500];
	while (!feof(fp)) {
		bufferLine[0] = '\0';
		fgets(bufferLine, 500, fp);   //fgets will save '\n'
		res.line++;

		//记录有效字符和单词总数，及判断空行
		int len = strlen(bufferLine);
		//cout << len << endl;
		bool leftBlank = true, isEmpty = true;
		for (int i = 0; i < len; ++i) {    //不用处理单词破折符（跨行单词，因为是程序文件），要处理行前空格 ?，还有最后一行最后一个单词
			if (isValidChar(bufferLine[i]))
				isEmpty = false;

			if (isalnum(bufferLine[i])) {  //必须处理多余空格，这里只记录数字和字母字符，如果要记录所有字符只需要把每行长度相加即可
				res.character++;
				if (leftBlank)
					res.word++, leftBlank = false;
			}
			else
				leftBlank = true;
		}
		if (isEmpty)
			res.blankLine++;
		else {
			bool leftQuo = false, slantingBar = false;
			for (int i = 0; i < len; ++i) {
				if (bufferLine[i] == '"')
					leftQuo = true;
				if (bufferLine[i] == '/' && bufferLine[i + 1] == '/') {
					slantingBar = true;
				}
				//处理掉类似于printf("//") 和 //"blahblah" 和 '/' 的情况，看哪个先出现
				if (leftQuo && !slantingBar) {
					res.codeLine++;
					break;
				}
				else if (!leftQuo && slantingBar) {
					res.commentLine++;
					break;
				}
			}
			//if there is no slanting bar or quo, and this line is non-empty, then increase the number of codeline
			if (!leftQuo && !slantingBar)
				res.codeLine++;
		}
	}
}

void TcharToChar(const TCHAR* tchar, char* _char) {
	int iLength;
	//获取字节长度   
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	//将tchar值赋给_char    
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}

void CharToTchar(const char* _char, TCHAR* tchar) {
	int iLength;
	iLength = MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, tchar, iLength);
}

int main(int argc, char *argv[]) {
	char filepath[200];
	TCHAR t_filepath[200];
	strcpy(filepath, argv[argc - 1]);
	if (strcmp(filepath, "-x") == 0) {		   //特殊处理-x
		getPathWin(t_filepath);                //用到新技术，老旧的API太麻烦，新的API又只支持TCHAR，后面的fopen只能用char，所以这里要转换一下
		TcharToChar(t_filepath, filepath);
		cout << filepath << endl;

		FILE* fp = fopen(filepath, "r");
		memset(&flag, 1, sizeof(flag));
		countNum(filepath);
		display();                             //将就着用控制台输出，C语言画框框太麻烦

		return 0;
	}

	//get all flags
	for (int i = 0; i < argc - 1; ++i) {
		if (strcmp(argv[i], "-c") == 0)
			flag.c = true;
		if (strcmp(argv[i], "-w") == 0)
			flag.w = true;
		if (strcmp(argv[i], "-l") == 0)
			flag.l = true;
		if (strcmp(argv[i], "-s") == 0)
			flag.s = true;
		if (strcmp(argv[i], "-a") == 0)
			flag.a = true;
		if (strcmp(argv[i], "-x") == 0)
			flag.x = true;
	}
	if (flag.s) {
		pattern = "";
		int lastBackSlash = 2, fin = 0;
		//int len = strlen(filepath);
		for (int i = 0; filepath[i] != '\0'; ++i) {
			//cout << i << endl;
			if (filepath[i] == '\\')
				lastBackSlash = i;
			if (filepath[i] == '.') {
				fin = 1;
				for (int j = i + 1; filepath[j] != '\0' && filepath[j] != '\\'; ++j)
					pattern += filepath[j];
			}
			if (fin)	break;
		}
		if (fin) {
			for (int i = lastBackSlash; filepath[i] != '\0'; ++i)
				filepath[i] = '\0';
		}
		//cout << filepath << endl;
		//cout << pattern << endl;
		dirList(filepath);
	}
	else {
		countNum(filepath);
		display();
	}

	return 0;
}