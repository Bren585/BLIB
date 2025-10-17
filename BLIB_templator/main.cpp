#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cstdlib>

using namespace std::filesystem;

#define TEMPLATE_NAME "BLIB_template"

// Replace all occurrences of 'from' with 'to' in a string
void replace_all(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

// Update text inside a file (like .sln or .vcxproj)
void update_file_contents(const path& file_path, const std::string& old_name, const std::string& new_name) {
    std::ifstream in(file_path);
    if (!in) throw std::runtime_error("Failed to open " + file_path.string());

    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    replace_all(content, old_name, new_name);

    std::ofstream out(file_path);
    if (!out) throw std::runtime_error("Failed to write " + file_path.string());
    out << content;
}

int main() {
    std::string template_path = "C:\\Users\\2240755\\Desktop\\Projects\\BLIB\\";
    std::string project_name;
    std::string dest_folder;

    // Get input from user
    std::cout << "New project name: ";
    std::getline(std::cin, project_name);
    std::cout << "Destination folder: ";
    std::getline(std::cin, dest_folder);
    std::cout << "Creating Project '" << project_name << " in " << dest_folder << "\n";

    try {
        path dest_path = (current_path() / path(dest_folder)).lexically_normal();
        create_directories(dest_path);

        // Unzip template using PowerShell
        std::string cmd = "powershell -Command \"Expand-Archive -Path '" + template_path + TEMPLATE_NAME + ".zip" +
            "' -DestinationPath '" + dest_path.string() + "' -Force\"";
        if (std::system(cmd.c_str()) != 0) {
            throw std::runtime_error("Failed to unzip template");
        }

        // Rename project folder
        rename(dest_path / TEMPLATE_NAME, dest_path / project_name);
        path project_path = dest_path / project_name;

        // Rename .sln and .vcxproj files
        path old_sln, old_vcxproj, old_vcxproj_filters, old_vcxproj_users;
        for (auto& entry : directory_iterator(project_path)) {
            auto path = entry.path();
            if      (path.extension() == ".sln"     ) old_sln               = path;
            else if (path.extension() == ".vcxproj" ) old_vcxproj           = path;
            else if (path.extension() == ".filters" ) old_vcxproj_filters   = path;
            else if (path.extension() == ".user"    ) old_vcxproj_users     = path;
        }

        if (old_sln.empty() || old_vcxproj.empty() || old_vcxproj_filters.empty()) {
            throw std::runtime_error("Template is missing solution or project files");
        }

        path new_sln = project_path / (project_name + ".sln");
        path new_vcxproj = project_path / (project_name + ".vcxproj");
        path new_vcxproj_filters = project_path / (project_name + ".vcxproj.filters");

        rename(old_sln, new_sln);
        rename(old_vcxproj, new_vcxproj);
        rename(old_vcxproj_filters, new_vcxproj_filters);
        remove(old_vcxproj_users);

        // Update internal references inside the files
        std::string old_name = old_sln.stem().string(); 
        update_file_contents(new_sln, old_name, project_name);
        update_file_contents(new_vcxproj, old_name, project_name);
        update_file_contents(new_vcxproj_filters, old_name, project_name);

        std::cout << "Project '" << project_name << " created successfully at " << project_path << "\n";
        std::cout << "Updating...\n";
        current_path(project_path);
        if (std::system("BLIB_update.exe") == 0) {
            std::cout << "Update Sucessful.\n";
        }
        else {
            throw std::runtime_error("Failed to update template.");
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cin.ignore();
        return 1;
    }
    std::cin.ignore();
    return 0;
}
