#include "bridge-tools.hpp"

int cmd(char *cmd, char *output, DWORD maxbuffer)
{
    HANDLE readHandle;
    HANDLE writeHandle;
    HANDLE stdHandle;
    DWORD bytesRead;
    DWORD retCode;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
   
    ZeroMemory(&sa,sizeof(SECURITY_ATTRIBUTES));
    ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si,sizeof(STARTUPINFO));
   
    sa.bInheritHandle=true;
    sa.lpSecurityDescriptor=NULL;
    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    si.cb=sizeof(STARTUPINFO);
    si.dwFlags=STARTF_USESHOWWINDOW;
    si.wShowWindow=SW_HIDE;
 
    if (!CreatePipe(&readHandle,&writeHandle,&sa,NULL))
    {
        OutputDebugString("cmd: CreatePipe failed!\n");
        return 0;
    }
 
    stdHandle=GetStdHandle(STD_OUTPUT_HANDLE);
 
    if (!SetStdHandle(STD_OUTPUT_HANDLE,writeHandle))
    {
        OutputDebugString("cmd: SetStdHandle(writeHandle) failed!\n");
        return 0;
    }
 
    if (!CreateProcess(NULL,cmd,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
    {
        OutputDebugString("cmd: CreateProcess failed!\n");
        return 0;
    }
 
    GetExitCodeProcess(pi.hProcess,&retCode);
    while (retCode==STILL_ACTIVE)
    {
        GetExitCodeProcess(pi.hProcess,&retCode);
    }
 
    if (!ReadFile(readHandle,output,maxbuffer,&bytesRead,NULL))
    {
        OutputDebugString("cmd: ReadFile failed!\n");
        return 0;
    }
    output[bytesRead]='\0';
 
    if (!SetStdHandle(STD_OUTPUT_HANDLE,stdHandle))
    {
        OutputDebugString("cmd: SetStdHandle(stdHandle) failed!\n");
        return 0;
    }
 
    if (!CloseHandle(readHandle))
    {
        OutputDebugString("cmd: CloseHandle(readHandle) failed!\n");
    }
    if (!CloseHandle(writeHandle))
    {
        OutputDebugString("cmd: CloseHandle(writeHandle) failed!\n");
    }
 
    return 1;
}

std::vector<std::string> split (const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}

inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

std::string getUnixFilePath(std::string windowsFilePath) {
	geode::log::info("Converting {} to Unix File path", windowsFilePath);
	std::string unixCmd = fmt::format("winepath -u \"{}\"", windowsFilePath);
	char unixPathBuffer[4098];

	cmd((char* )unixCmd.c_str(), unixPathBuffer, 4098);

	return trim_copy(std::string(unixPathBuffer));
}

std::string getWindowsFilePath(std::string unixFilePath) {
	std::string winCmd = fmt::format("winepath -w \"{}\"", unixFilePath);
	char winPathBuffer[4098];

	cmd((char* )winCmd.c_str(), winPathBuffer, 4098);

	return trim_copy(std::string(winPathBuffer));
}

std::string getWindowsBufferPath(std::string buffername) {
	return trim_copy((geode::Mod::get()->getSaveDir() / buffername).string());
}

std::string getUnixBufferPath(std::string buffername) {
	return trim_copy(getUnixFilePath(getWindowsBufferPath(buffername)));
}

std::string readBuffer(std::string bufferpath) {
	std::ifstream buffile(bufferpath);
	std::string buffer;
	std::getline(buffile, buffer);
	buffile.close();

	geode::log::info("Buffer {} contents are: {}", bufferpath, buffer);

	return trim_copy(buffer);
}

std::vector<std::string> readBufferLines(std::string bufferpath) {
	std::ifstream buffile(bufferpath);
	std::string line;
	std::vector<std::string> lines;

	while (std::getline(buffile, line)) {
		if (line == "") continue;
		lines.push_back(line);
	}

	return lines;
}

void spawnLinuxProcess(std::string binary, std::string cmd) {
    PROCESS_INFORMATION process_information;
	_STARTUPINFOA startup_info;
	ZeroMemory(&startup_info, sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);
	ZeroMemory(&process_information, sizeof(process_information));

    CreateProcess(
		(getWindowsFilePath("/bin/") + binary).c_str(), 
		(char* )cmd.c_str(), NULL, NULL, 
		TRUE, CREATE_NO_WINDOW, NULL, NULL, 
		&startup_info, &process_information
	);
}

bool dialogToolExists(std::string tool_name) {
    return std::filesystem::exists(getWindowsFilePath("/bin/") + tool_name);
}

bool kdialogExists() { 
    return dialogToolExists("kdialog");
}

bool zenityExists() { 
    return dialogToolExists("zenity");
}

bool yadExists() { 
    return dialogToolExists("yad");
}

bool isKDE() {
    return getenv("XDG_CURRENT_DESKTOP") == std::string("KDE");
}

bool isWine() {
    HMODULE hntdll = GetModuleHandle("ntdll.dll");
    auto proc_exists = (void *)GetProcAddress(hntdll, "wine_get_version");

    return proc_exists != nullptr;
}

std::string getWinePlatform() {
    static void (CDECL *pwine_get_host_version)(char **sysname, char **release);
    HMODULE hntdll = GetModuleHandle("ntdll.dll");
    pwine_get_host_version = (void (CDECL *)(char **, char **))GetProcAddress(hntdll, "wine_get_host_version");

    char * sysname;
    char * version;
    pwine_get_host_version(&sysname, &version);

    return std::string(sysname);
}

bool isLinux() {
    if (!isWine()) return false;

    return getWinePlatform() == "Linux";
}

std::string getPickModeBufferName(geode::utils::file::PickMode mode) {
    switch (mode) {
		case geode::utils::file::PickMode::OpenFile:
			return "open-file-buffer";
		case geode::utils::file::PickMode::SaveFile:
			return "save-file-buffer";
		case geode::utils::file::PickMode::OpenFolder:
			return "directory-buffer";
	}

    return "wtf???";
}