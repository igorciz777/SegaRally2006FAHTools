#include "fah.hpp"
#include <string>
#include <iostream>
#include <unordered_map>
#include <functional>

void usage(const char* program)
{
    std::cout << std::endl;
    std::cout << "Usage: " << std::filesystem::path(program).filename().string() << " [cmd] [file|text]\n";
    std::cout << std::endl;
    std::cout << "File Archive File Commands:\n";
    std::cout << "  -csv [DATA.FAI] [DATA.FAB] [output csv] Create CSV File info list\n";
    std::cout << "  -ufab [DATA.FAI] [DATA.FAB] [output dir] Unpack File Archive\n";
    std::cout << "  -ifab [DATA.FAI] [DATA.FAB] [input dir] Import files back to archive\n";
    std::cout << "  -ubin [file.bin] [output dir] Unpack FAH bin File\n";
    std::cout << "  -ibin [file.bin] [input dir] Import files back to FAH File\n";
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    std::unordered_map<std::string, std::function<int(int, char**)>> commands;


    // FAH File commands
    commands["-ufab"] = [](int argc, char** argv) -> int {
        if (argc < 5) {
            std::cerr << "Usage: -ufab [DATA.FAI] [DATA.FAB] [output dir] Unpack File Archive\n";
            return 1;
        }
        unpack_file_archive(argv[2], argv[3], argv[4]);
        return 0;
    };

    commands["-ubin"] = [](int argc, char** argv) -> int {
        if (argc < 4) {
            std::cerr << "Usage: -ubin [file.bin] [output dir] Unpack FAH bin File\n";
            return 1;
        }
        unpack_file_archive(argv[2], argv[2], argv[3]);
        return 0;
    };
    commands["-ibin"] = [](int argc, char** argv) -> int {
        if (argc < 4) {
            std::cerr << "Usage: -ibin [file.bin] [input dir] Import files back to FAH File\n";
            return 1;
        }
        import_to_archive(argv[2], argv[2], argv[3]);
        return 0;
    };

    commands["-ifab"] = [](int argc, char** argv) -> int {
        if (argc < 5) {
            std::cerr << "Usage: -ifab [DATA.FAI] [DATA.FAB] [input dir] Import files back to archive\n";
            return 1;
        }
        import_to_archive(argv[2], argv[3], argv[4]);
        return 0;
    };

    commands["-csv"] = [](int argc, char** argv) -> int {
        if (argc < 4) {
            std::cerr << "Usage: -csv [DATA.FAI] [DATA.FAB] [output csv] Create CSV File info list\n";
            return 1;
        }
        create_csv_list(argv[2], argv[3], argv[4]);
        return 0;
    };

    std::string cmd = argv[1];
    auto it = commands.find(cmd);
    if (it != commands.end()) {
        return it->second(argc, argv);
    } else {
        usage(argv[0]);
        return 1;
    }
}