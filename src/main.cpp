#include "bridge-tools.hpp"
#include <Geode/Geode.hpp>
#include <exception>
#include <hooks.hpp>

using namespace geode::prelude;

$execute {
	if (!isLinux()) {
		MessageBox(
			NULL, 
			"Your platform doesn't smell like Linux! Not enabling.", 
			"Linux File Picker", 
			MB_ICONERROR
		);
		throw std::exception("Not Linux");
	}

	Mod::get()->hook(
    	reinterpret_cast<void*>(
			geode::addresser::getNonVirtual(
				geode::modifier::Resolve<std::filesystem::path const&>::func(
					&utils::file::openFolder
				)
			)
		),
		&linuxOpenFolder,
		"utils::file::openFolder",
		tulip::hook::TulipConvention::Stdcall		
	);

	Mod::get()->hook(
        reinterpret_cast<void*>(
            geode::addresser::getNonVirtual(
                geode::modifier::Resolve<
					file::PickMode, 
					const file::FilePickOptions &
				>::func(&utils::file::pick)
            )
        ),
        &linuxFilePick,
        "utils::file::pick",
        tulip::hook::TulipConvention::Stdcall		
    );

	Mod::get()->hook(
        reinterpret_cast<void*>(
            geode::addresser::getNonVirtual(
                geode::modifier::Resolve<
					const file::FilePickOptions &
				>::func(&utils::file::pickMany)
            )
        ),
        &linuxPickMany,
        "utils::file::pickMany",
        tulip::hook::TulipConvention::Stdcall		
    );

}