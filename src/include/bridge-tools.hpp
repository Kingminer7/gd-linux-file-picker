#pragma once
#include <Geode/Geode.hpp>
#include <fmt/format.h>


int cmd(char *cmd, char *output, DWORD maxbuffer);
std::vector<std::string> split (const std::string &s, char delim);
std::string trim_copy(std::string s);
std::string getUnixFilePath(std::string windowsFilePath);
std::string getWindowsFilePath(std::string unixFilePath);
std::string getWindowsBufferPath(std::string buffername);
std::string getUnixBufferPath(std::string buffername);
std::string readBuffer(std::string bufferpath);
std::vector<std::string> readBufferLines(std::string bufferpath);
void spawnLinuxProcess(std::string binary, std::string cmd);

bool dialogToolExists(std::string tool_name);
bool kdialogExists();
bool zenityExists();
bool yadExists();
bool isKDE();

std::string getPickModeBufferName(geode::utils::file::PickMode mode);

std::string getWinePlatform();
bool isWine();
bool isLinux();

#define FORMAT_BASH_DIALOG(bash_command, buffer_path) \
    fmt::format("bash -c \"echo $({}) > {}\"", bash_command, buffer_path)

#define RESERVE_BUFFER_AWAIT(current_action_buffer_path) \
    std::ofstream curfb(current_action_buffer_path); \
	curfb << "notyet"; \
	curfb.close();

#define WAIT_BUFFER_FILL(current_action_buffer_path_win) \
    while (readBuffer(current_action_buffer_path_win).substr(0, 6) == "notyet") { \
		Sleep(500); \
	}