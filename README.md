dump文件默认生成在对应exe所在位置

在 Visual Studio 中，你可以通过以下步骤确保生成 PDB 文件：

- 右键点击你的项目，选择“属性 (Properties)”。
- 转到 **C/C++ > 常规**，并确保“调试信息格式 (Debug Information Format)”设置为 **程序数据库 (/Zi)**。
- 转到 **链接器 > 调试**，并确保“生成调试信息 (Generate Debug Info)”设置为 **是 (/DEBUG)**。

示例代码:

```C++
int _tmain(int argc, _TCHAR* argv[])
{
 #include"DumpHelper.h"
int main(int argc, char** argv) {
	//启动全局异常捕获生成dump
	AC::DumpHelper::Start(); //默认生成在可执行文件dmp文件夹里
	//AC::DumpHelper::Snapshot("current.dmp");主动生成内存快照
	//触发一个异常
	int a = 0;
	int b = 5 / a;
	return 0;
}


    return 0;
}

```





