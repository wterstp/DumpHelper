#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include<string>
//
// Created by dzl on 2024/9/30.
//
namespace AC {
	class  DumpHelper
	{
	public:
		/// <summary>
		/// �����Ƿ��¼����ת��
		/// Ĭ�Ϸ�
		/// </summary>
		/// <param name="value">�Ƿ��¼����ת��</param>
		static void SetIsDumpCrash(bool value);
		/// <summary>
		/// ���ñ���ת��·��
		/// Ĭ��Ϊ".\\"
		/// </summary>
		/// <param name="directory">����ת��·��</param>
		static void SetDumpDirectory(const std::string& directory);
		/// <summary>
		/// ����ת���ļ�����
		/// ���������Զ�ɾ�����ļ�
		/// </summary>
		/// <param name="count">����ļ�������Ĭ��10</param>
		static void SetDumpMaxFileCount(int count);
		/// <summary>
		/// ��ȡ�ڴ����ת��,��������ʱ�̼�¼�������Ǳ�������£���
		/// </summary>
		/// <param name="path">�����ļ���</param>
		static void Snapshot(const std::string& path);
		/// <summary>
	  /// һ������(�ŵ�����ͷ)��
	  /// </summary>
	  /// <param name="path">����</param>
		static void Start();
	};
}






//? --------------------  ʵ��--------------------------------



#include <Windows.h>
#include <Windows.h>
#include "time.h"
#include <vector>
#include <DbgHelp.h>
#pragma comment(lib,"Dbghelp.lib")
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <stdint.h>
#include <string>
#define MAX_PATH_LEN 256
#ifdef _WIN32
#define ACCESS(fileName,accessMode) _access(fileName,accessMode)
#define MKDIR(path) _mkdir(path)
#else
#define ACCESS(fileName,accessMode) access(fileName,accessMode)
#define MKDIR(path) mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif
namespace AC {
	static std::string _directory = ".\\";
	static int _fileCount = 10;
	//��ȡ�ļ����µ������ļ���
	static void GetFolderFiles(const std::string& path, std::vector<std::string>& files)
	{
		//�ļ����
		intptr_t hFile = 0;
		//�ļ���Ϣ
		struct _finddata_t fileinfo;
		std::string p;
		if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
			// "\\*"��ָ��ȡ�ļ����µ��������͵��ļ��������ȡ�ض����͵��ļ�����pngΪ�������á�\\*.png��
		{
			do
			{
				//�����Ŀ¼,����֮
				//�������,�����б�
				if ((fileinfo.attrib & _A_SUBDIR))
				{
					if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
						GetFolderFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
				else
				{
					files.push_back(path + "\\" + fileinfo.name);
				}
			} while (_findnext(hFile, &fileinfo) == 0);
			_findclose(hFile);
		}
	}
	//���ɶ༶Ŀ¼���м�Ŀ¼���������Զ�����
	static bool CreateMultiLevelDirectory(const std::string& directoryPath) {
		uint32_t dirPathLen = directoryPath.length();
		if (dirPathLen > MAX_PATH_LEN)
		{
			return false;
		}
		char tmpDirPath[MAX_PATH_LEN] = { 0 };
		for (uint32_t i = 0; i < dirPathLen; ++i)
		{
			tmpDirPath[i] = directoryPath[i];
			if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
			{
				if (ACCESS(tmpDirPath, 0) != 0)
				{
					int32_t ret = MKDIR(tmpDirPath);
					if (ret != 0)
					{
						return false;
					}
				}
			}
		}
		return true;
	}
	/// <summary>
	/// ����dmp�ļ�
	/// </summary>
	/// <param name="exceptionPointers">�쳣��Ϣ</param>
	/// <param name="path">�ļ�·���������ļ�����</param>
	/// <returns></returns>
	static int GenerateDump(EXCEPTION_POINTERS* exceptionPointers, const std::string& path)
	{
		HANDLE hFile = ::CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			MINIDUMP_EXCEPTION_INFORMATION minidumpExceptionInformation;
			minidumpExceptionInformation.ThreadId = GetCurrentThreadId();
			minidumpExceptionInformation.ExceptionPointers = exceptionPointers;
			minidumpExceptionInformation.ClientPointers = TRUE;
			bool isMiniDumpGenerated = MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				hFile,
				(MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithHandleData | MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules | MiniDumpWithDataSegs),
				&minidumpExceptionInformation,
				nullptr,
				nullptr);

			CloseHandle(hFile);
			if (!isMiniDumpGenerated)
			{
				printf("MiniDumpWriteDump failed\n");
			}
		}
		else
		{
			printf("Failed to create dump file\n");
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}
	//ȫ���쳣����ص�
	static LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
	{
		time_t t;
		struct tm* local;
		char path[1024] = { 0 };
		char ext[MAX_PATH] = { 0 };
		t = time(NULL);
		local = localtime(&t);
		//����Ŀ¼
		if (!CreateMultiLevelDirectory(_directory))
		{
			printf("Failed to create directory %s\n", _directory.c_str());
			return EXCEPTION_EXECUTE_HANDLER;
		}
		//���������ļ�
		std::vector<std::string> files;
		std::vector<std::string> dmpFiles;
		GetFolderFiles(_directory, files);
		for (auto i : files)
		{
			_splitpath(i.c_str(), NULL, NULL, NULL, ext);
			if (strcmp(ext, ".dmp") == 0)
			{
				dmpFiles.push_back(i);
			}
		}
		if (dmpFiles.size() >= _fileCount)
		{
			if (!DeleteFileA(dmpFiles.front().c_str()))
			{
				printf("Failed to delete old file %s\n", dmpFiles.front().c_str());
			}
		}
		//�����ļ���
		sprintf_s(path, MAX_PATH, "%s[%04d-%02d-%02d]_%02d-%02d.%02d.dmp",
			_directory.c_str(),
			1900 + local->tm_year,
			local->tm_mon + 1,
			local->tm_mday,
			local->tm_hour,
			local->tm_min,
			local->tm_sec);


		//sprintf(path, "%s%d%02d%02d%02d%02d%02d.dmp", _directory.c_str(), 1900 + local->tm_year, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
		if (strlen(path) > MAX_PATH)
		{
			printf("File path was too long! %s\n", path);
			return EXCEPTION_EXECUTE_HANDLER;
		}
		//����dmp�ļ�
		return GenerateDump(lpExceptionInfo, path);
	}
	void DumpHelper::SetIsDumpCrash(bool value)
	{
		if (value)
			SetUnhandledExceptionFilter(ExceptionFilter);
		else
		{
			SetUnhandledExceptionFilter(NULL);
		}
	}
	void DumpHelper::SetDumpDirectory(const std::string& directory)
	{
		char exePath[MAX_PATH];
		GetModuleFileNameA(NULL, exePath, MAX_PATH); // ��ȡ��ִ���ļ���·��

		std::string exeDirectory = exePath;
		size_t pos = exeDirectory.find_last_of("\\/");
		if (pos != std::string::npos) {
			exeDirectory = exeDirectory.substr(0, pos + 1); // ��ȡ��Ŀ¼
		}

		_directory = exeDirectory + directory + "\\";// ����ṩ��Ŀ¼Ϊ�գ�ʹ�ÿ�ִ���ļ�Ŀ¼
		if (_directory.back() != '\\') {
			_directory.push_back('\\');
		}
	}
	void DumpHelper::SetDumpMaxFileCount(int count)
	{
		_fileCount = count;
	}
	void DumpHelper::Snapshot(const std::string& path)
	{
		__try
		{
			//ͨ�������쳣��ȡ��ջ
			RaiseException(0xE0000001, 0, 0, 0);
		}
		__except (GenerateDump(GetExceptionInformation(), path)) {}
	}

	void DumpHelper::Start()
	{
		//����dump�ļ�����Ŀ¼
		SetDumpDirectory("dmp");
		SetIsDumpCrash(true);
	}

}



