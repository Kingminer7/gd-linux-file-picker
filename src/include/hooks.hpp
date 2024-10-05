#pragma once
#include <bridge-tools.hpp>


bool linuxOpenFolder(std::filesystem::path const& path);
geode::Task<geode::Result<std::filesystem::path>> linuxFilePick(
    geode::utils::file::PickMode mode, const geode::utils::file::FilePickOptions &options
);
geode::Task<geode::Result<std::vector<std::filesystem::path>>> linuxPickMany(
	const geode::utils::file::FilePickOptions &options
);