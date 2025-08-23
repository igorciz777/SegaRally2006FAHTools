#include <iostream>
#include <filesystem>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <map>

typedef struct
{
    uint32_t name_offset;
    uint32_t offset;
    uint32_t size;
    uint32_t hash;
} fai_entry_t;

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t pad; // == 0
    uint32_t fai_size;
    uint32_t file_count;
    uint32_t unk1; // == 32
    uint32_t names_bytes;
    uint32_t names_offset;
} fai_header_t;

enum fah_version
{
    FAH_VER_1_00 = 0x100,
    FAH_VER_1_01 = 0x101,
    FAH_VER_2_00 = 0x200
};

uint32_t get_hash(const char* filename)
{
    uint32_t hash = 0;
    int pos = 0;
    while (*filename) {
        reinterpret_cast<char*>(&hash)[pos] += *filename;
        pos = (pos + 1) & 3;
        filename++;
    }
    return hash;
}

// fai - file archive index
// fab - file archive bytes
void unpack_file_archive(const char *data_fai, const char *data_fab, const char *output_dir)
{
    std::fstream fai_file(data_fai, std::ios::in | std::ios::binary);
    std::fstream fab_file(data_fab, std::ios::in | std::ios::binary);
    if (!fai_file || !fab_file)
    {
        std::cerr << "Failed to open files" << std::endl;
        return;
    }

    fai_header_t fai_header;
    fai_file.read(reinterpret_cast<char *>(&fai_header), sizeof(fai_header));

    if(fai_header.magic != 0x00484146) // "FAH"
    {
        std::cerr << "Not a valid FAH file" << std::endl;
        return;
    }

    // Read the FAI entries
    std::vector<fai_entry_t> fai_entries(fai_header.file_count);
    fai_file.read(reinterpret_cast<char *>(fai_entries.data()), sizeof(fai_entry_t) * fai_header.file_count);

    // Read the names from names offset to eof
    std::vector<char> names_data(fai_header.fai_size - fai_header.names_offset);
    fai_file.read(names_data.data(), fai_header.fai_size - fai_header.names_offset);

    // Create output directory
    std::filesystem::create_directory(output_dir);

    // Extract files from the FAB
    for (const auto &entry : fai_entries)
    {
        char filename[256];
        if(fai_header.version < FAH_VER_2_00) {
            strcpy(filename, names_data.data() + entry.name_offset);
        } else {
            strcpy(filename, names_data.data() + (entry.name_offset >> 0x10) * 4);
            strcat(filename, names_data.data() + fai_header.names_bytes + (entry.name_offset & 0xffff) * 4);
        }

        std::cout << "Extracting " << filename << std::endl;

        std::filesystem::path output_file = std::filesystem::path(output_dir) / filename;

        // Ensure subdirectory exists if filename contains '/'
        if (output_file.has_parent_path()) {
            std::filesystem::create_directories(output_file.parent_path());
        }

        fab_file.seekg(entry.offset);
        std::vector<char> buffer(entry.size);
        fab_file.read(buffer.data(), entry.size);

        std::ofstream out(output_file, std::ios::binary);
        out.write(buffer.data(), entry.size);
        out.close();
    }

    fab_file.close();
    fai_file.close();

    std::cout << "Extraction completed." << std::endl;
}

void create_csv_list(const char *data_fai, const char *data_fab, const char *output_csv)
{
    std::fstream fai_file(data_fai, std::ios::in | std::ios::binary);
    std::fstream fab_file(data_fab, std::ios::in | std::ios::binary);
    if (!fai_file || !fab_file)
    {
        std::cerr << "Failed to open files" << std::endl;
        return;
    }

    fai_header_t fai_header;
    fai_file.read(reinterpret_cast<char *>(&fai_header), sizeof(fai_header));

    if (fai_header.magic != 0x00484146) // "FAH"
    {
        std::cerr << "Not a valid FAH file" << std::endl;
        return;
    }

    // Read the FAI entries
    std::vector<fai_entry_t> fai_entries(fai_header.file_count);
    fai_file.read(reinterpret_cast<char *>(fai_entries.data()), sizeof(fai_entry_t) * fai_header.file_count);

    // Read the names from names offset to eof
    std::vector<char> names_data(fai_header.fai_size - fai_header.names_offset);
    fai_file.read(names_data.data(), fai_header.fai_size - fai_header.names_offset);

    // Create output CSV file
    std::ofstream out(output_csv);
    if (!out)
    {
        std::cerr << "Failed to create output CSV file" << std::endl;
        return;
    }

    // Write the CSV header
    out << "Filename,Offset (FAB),Size (Hex),Position (FAI)" << std::endl;

    for (size_t i = 0; i < fai_entries.size(); ++i)
    {
        const auto &entry = fai_entries[i];
        char filename[256];
        if (fai_header.version < FAH_VER_2_00)
        {
            strcpy(filename, names_data.data() + entry.name_offset);
        }
        else
        {
            strcpy(filename, names_data.data() + (entry.name_offset >> 0x10) * 4);
            strcat(filename, names_data.data() + fai_header.names_bytes + (entry.name_offset & 0xffff) * 4);
        }

        // Calculate positions in FAI and FAB files
        std::streampos fai_position = sizeof(fai_header) + i * sizeof(fai_entry_t);
        std::streampos fab_position = entry.offset;

        // Write details to the CSV file
        out << "\"" << filename << "\","
            << "0x" << std::hex << entry.offset << ","
            << "0x" << std::hex << entry.size << ","
            << "0x" << std::hex << fai_position << std::dec << std::endl;
    }

    out.close();
    fai_file.close();
    fab_file.close();

    std::cout << "CSV File List created successfully" << std::endl;
}

// standalone .bin FAH files found inside the main archive
void unpack_file_archive(const char *data_fah, const char *output_dir)
{
    std::fstream fah_file(data_fah, std::ios::in | std::ios::binary);
    if (!fah_file)
    {
        std::cerr << "Failed to open file" << std::endl;
        return;
    }

    fai_header_t fai_header;
    fah_file.read(reinterpret_cast<char *>(&fai_header), sizeof(fai_header));

    if(fai_header.magic != 0x00484146) // "FAH"
    {
        std::cerr << "Not a valid FAH file" << std::endl;
        return;
    }

    // Read the FAI entries
    std::vector<fai_entry_t> fai_entries(fai_header.file_count);
    fah_file.read(reinterpret_cast<char *>(fai_entries.data()), sizeof(fai_entry_t) * fai_header.file_count);

    // Read the names
    std::vector<char> names_data(fai_header.fai_size - fai_header.names_offset);
    fah_file.read(names_data.data(), fai_header.fai_size - fai_header.names_offset);

    // Create output directory
    std::filesystem::create_directory(output_dir);

    // Extract files from the FAB
    for (const auto &entry : fai_entries)
    {
        char filename[256];
        if(fai_header.version < FAH_VER_2_00) {
            strcpy(filename, names_data.data() + entry.name_offset);
        } else {
            strcpy(filename, names_data.data() + (entry.name_offset >> 0x10) * 4);
            strcat(filename, names_data.data() + fai_header.names_bytes + (entry.name_offset & 0xffff) * 4);
        }

        std::cout << "Extracting " << filename << std::endl;

        std::filesystem::path output_file = std::filesystem::path(output_dir) / filename;

        // Ensure subdirectory exists if filename contains '/'
        if (output_file.has_parent_path()) {
            std::filesystem::create_directories(output_file.parent_path());
        }

        fah_file.seekg(entry.offset);
        std::vector<char> buffer(entry.size);
        fah_file.read(buffer.data(), entry.size);

        std::ofstream out(output_file, std::ios::binary);
        out.write(buffer.data(), entry.size);
        out.close();
    }

    fah_file.close();

    std::cout << "Extraction completed." << std::endl;
}