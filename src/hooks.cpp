#include <hooks.hpp>


bool linuxOpenFolder(std::filesystem::path const& path) {
	std::string windowsPath = path.string();
	auto unixPathBuffer = getUnixFilePath(windowsPath);

	auto cmd = std::string("Z:\\bin\\dbus-send --session --print-reply --dest=org.freedesktop.FileManager1") 
		+ " --type=method_call /org/freedesktop/FileManager1 org.freedesktop.FileManager1.ShowItems" 
		+ " array:string:\"file://" + unixPathBuffer + "\" string:\"\"";

	geode::log::info("Opening File Manager using dbus-send: {}", cmd);

	spawnLinuxProcess("dbus-send", cmd);

	return true;
}

geode::Task<geode::Result<std::filesystem::path>> linuxFilePick(
	geode::utils::file::PickMode mode, const geode::utils::file::FilePickOptions &options
) {
	using RetTask = geode::Task<geode::Result<std::filesystem::path>>;

	if (!zenityExists() && !yadExists() && !kdialogExists()) {
		return RetTask::immediate(
			geode::Err(std::string("You don't have kdialog, zenity or yad detected on your system. ") +
			"Please install one of them from the package manager of your Linux distro.")
		);
	}

	std::string current_action_buffer_name = getPickModeBufferName(mode);

	auto current_action_buffer_path = getUnixBufferPath(current_action_buffer_name);
	auto current_action_buffer_path_win = getWindowsBufferPath(current_action_buffer_name);

	std::string bash_command_base;

	switch (mode) {
		case geode::utils::file::PickMode::OpenFile:
			if (isKDE() && kdialogExists()) {
				bash_command_base = "kdialog --getopenfilename --title 'Open File'";
			}
			else if (yadExists()) {
				bash_command_base = "yad --file --title='Open File'";
			}
			else if (zenityExists()) {
				bash_command_base = "zenity --file-selection";
			}
			break;
		
		case geode::utils::file::PickMode::SaveFile:
			if (isKDE() && kdialogExists()) {
				bash_command_base = "kdialog --getsavefilename --title 'Save File'";
			}
			else if (yadExists()) {
				bash_command_base = "yad --file --save --title='Save File'";
			}
			else if (zenityExists()) {
				bash_command_base = "zenity --save --file-selection";
			}
			break;
		
		case geode::utils::file::PickMode::OpenFolder:
			if (isKDE() && kdialogExists()) {
				bash_command_base = "kdialog --getopenfilename '' 'inode/directory'";
			}
			else if (yadExists()) {
				bash_command_base = "yad --file --directory --title='Open Directory'";
			}
			else if (zenityExists()) {
				bash_command_base = "zenity --file-selection --directory";
			}
			break;
	}

	std::string bash_command = FORMAT_BASH_DIALOG(bash_command_base, current_action_buffer_path);

	RESERVE_BUFFER_AWAIT(current_action_buffer_path_win);
	spawnLinuxProcess("sh", bash_command);
	WAIT_BUFFER_FILL(current_action_buffer_path_win);

	auto action_contents = readBuffer(current_action_buffer_path_win);

	if (action_contents == "") {
		return RetTask::immediate(geode::Err("Dialog cancelled"));
	}

	auto windowed_path = getWindowsFilePath(action_contents);

	return RetTask::immediate(geode::Ok(windowed_path));
}

geode::Task<geode::Result<std::vector<std::filesystem::path>>> linuxPickMany(
	const geode::utils::file::FilePickOptions &options
) {
    using RetTask = geode::Task<geode::Result<std::vector<std::filesystem::path>>>;

	if (!zenityExists() && !yadExists() && !kdialogExists()) {
		return RetTask::immediate(
			geode::Err(std::string("You don't have kdialog, zenity or yad detected on your system. ") +
			"Please install one of them from the package manager of your Linux distro.")
		);
	}

	std::string current_action_buffer_path = getUnixBufferPath("pick-many-buffer");;
	std::string current_action_buffer_path_win = getWindowsBufferPath("pick-many-buffer");
	std::string bash_command_base;

	std::string parser;

	if (isKDE() && kdialogExists()) {
		bash_command_base = "kdialog --getopenfilename --multiple --title 'Open multiple files'";
		parser = "kdialog";
	}
	else if (yadExists()) {
		bash_command_base = "yad --file --multiple --title='Open multiple files'";
		parser = "zenity";
	}
	else if (zenityExists()) {
		bash_command_base = "zenity --file-selection --multiple";
		parser = "zenity";
	}

	auto bash_command = FORMAT_BASH_DIALOG(bash_command_base, current_action_buffer_path);

	RESERVE_BUFFER_AWAIT(current_action_buffer_path_win);
	spawnLinuxProcess("sh", bash_command);
	WAIT_BUFFER_FILL(current_action_buffer_path_win);

	if (readBuffer(current_action_buffer_path_win) == "") {
		return RetTask::immediate(geode::Err("Dialog cancelled"));
	}

	std::vector<std::filesystem::path> paths;
	std::vector<std::string> lines;

	if (parser == "kdialog") {
		lines = readBufferLines(current_action_buffer_path_win);
	}
	else {
		auto raw = readBuffer(current_action_buffer_path_win);
		lines = split(raw, '|');
	}

	for (auto line : lines) {
		paths.push_back(getWindowsFilePath(trim_copy(line)));
	}

	return RetTask::immediate(geode::Ok(paths));
}