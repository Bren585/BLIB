#include <filesystem>
#include <iostream>
#include <set>
using namespace std::filesystem;
typedef std::set<std::string> filter;

void shallow_copy(const path& src, const path& dst, const filter& whitelist) {
    create_directories(dst);
    for (const auto& entry : directory_iterator(src)) {
        const auto& path = entry.path();

        if (entry.is_regular_file()) {
            if (whitelist.find(path.extension().string()) == whitelist.end()) continue;

            std::cout << "Copying file : " << path.filename() << " ... ";
            copy_file(path, dst / path.filename(), copy_options::overwrite_existing);
            std::cout << "Finished." << std::endl;
        }
    }
}

int main() {

    path master = "C:\\Users\\2240755\\Desktop\\Projects\\BLIB";
    path local = current_path() / "BLIB";

    path platform = "x64";
    path configs[2] = { "Debug", "Release" };

    path local_data = current_path() / "data";
    path local_shaders = local_data / "shaders";

#define DEPEND_COUNT 4
    path dependancies[DEPEND_COUNT] = {
        "DirectXTK-main",
        "cereal-master",
        "imgui",
        "FBX SDK",
    };

    try {
        // Ensure local folders exist
        create_directories(local);
        create_directories(local_data);
        create_directories(local_shaders);

        // Copy headers
        filter headers = { ".h", ".hpp" };
        shallow_copy(master, local, headers);

        // Copy libs
        filter libs = { ".lib", ".ibd", ".pbd" };
        for (int i = 0; i < 2; i++) {
            path rel = platform / configs[i];
            shallow_copy(master / rel, local / rel, libs);
        }

        // Copy shaders
        filter shaders = { ".cso" };
        shallow_copy(master / platform / configs[0], local_shaders, shaders);

        // Copy dependancies
        for (int i = 0; i < DEPEND_COUNT; i++) {
            std::cout << "Copying dependancy : " << dependancies[i] << " ... ";
            create_directories(local / dependancies[i]);
            copy(master / dependancies[i], local / dependancies[i],
                copy_options::overwrite_existing | copy_options::recursive);
            std::cout << "Finished." << std::endl;
        }

        std::cout << "Library update complete.\n";
    }
    catch (std::exception& e) {
        std::cerr << "Error updating library: " << e.what() << "\n";
        return 1;
    }

    return 0;
}