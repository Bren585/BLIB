#include <filesystem>
#include <iostream>
#include <set>
using namespace std::filesystem;
typedef std::set<std::string> filter;

bool needs_update(const path& new_file, const path& old_file) {
    if (!exists(old_file)) return true;
    return last_write_time(new_file) > last_write_time(old_file);
}

void shallow_copy(const path& src, const path& dst, const filter& whitelist) {
    int skip_count = 0;
    int copy_count = 0;
    create_directories(dst);
    for (const auto& entry : directory_iterator(src)) {
        const auto& file = entry.path();

        if (entry.is_regular_file()) {
            if (whitelist.find(file.extension().string()) == whitelist.end()) continue;

            const path dst_file = dst / file.filename();
            if (needs_update(file, dst_file)) {
                std::cout << " - Copying file : " << file.filename() << " ... ";
                copy_file(file, dst_file, copy_options::overwrite_existing);
                std::cout << "Finished.\n";
                copy_count++;
            }
            else {
                skip_count++;
            }
        }
    }
    std::cout << "Copied " << copy_count << " files, skipped " << skip_count << ".\n";
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
        std::cout << "Copying Header Files.\n";
        shallow_copy(master, local, headers);

        // Copy libs
        filter libs = { ".lib", ".ibd", ".pbd" };
        std::cout << "Copying lib files.\n";
        for (int i = 0; i < 2; i++) {
            path rel = platform / configs[i];
            shallow_copy(master / rel, local / rel, libs);
        }

        // Copy shaders
        filter shaders = { ".cso" };
        std::cout << "Copying cso files.\n";
        shallow_copy(master / platform / configs[0], local_shaders, shaders);

        // Copy dependancies
        std::cout << "Copying Dependancies.\n";
        int skip_count = 0;
        int copy_count = 0;
        for (int i = 0; i < DEPEND_COUNT; i++) {
            const path master_filepath = master / dependancies[i];
            const path local_filepath = local / dependancies[i];

            if (needs_update(master_filepath, local_filepath)) {
                std::cout << " - Copying dependancy : " << dependancies[i] << " ... ";
                create_directories(local / dependancies[i]);
                copy(master / dependancies[i], local / dependancies[i], copy_options::overwrite_existing | copy_options::recursive);
                std::cout << "Finished." << std::endl;
                copy_count++;
            }
            else {
                skip_count++;
            }
        }
        std::cout << "Copied " << copy_count << " dependancies, skipped " << skip_count << ".\n";

        std::cout << "Library update complete.\n";
    }
    catch (std::exception& e) {
        std::cerr << "Error updating library: " << e.what() << "\n";
        return 1;
    }

    return 0;
}