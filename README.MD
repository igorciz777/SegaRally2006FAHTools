# Sega Rally 2006 File Archive Tools
A set of tools for "FAH" file archives found in Sega Rally 2006 [SLPM-66212] on the PS2.

All files are extracted with their filenames and subdirectories.

## Features
Currently, the tool can:
- Extract the contents of the main `DATA.FAB`/`DATA.FAI` archive.
- Extract contents of smaller FAH .bin archives found in the main archive.
- Extract files info to a CSV file.

Planned features include:
- Rebuilding the main `DATA.FAB`/`DATA.FAI` archive.
- Rebuilding the smaller FAH .bin archives.

## Usage

```shell
.\sega_fah [cmd] [file|text]
```

## Modes

- **-csv [DATA.FAI] [DATA.FAB] [output csv]**: Create a CSV file info list from the archive.
- **-ufab [DATA.FAI] [DATA.FAB] [output dir]**: Unpack the main file archive to a specified output directory.
- **-ubin [file.bin] [output dir]**: Unpack a FAH .bin file to a specified output directory.

## Examples

Create CSV file info:
```shell
.\sega_fah -csv DATA.FAI DATA.FAB output.csv
```
Unpack the main file archive:
```shell
.\sega_fah -ufab DATA.FAI DATA.FAB out
```
Unpack .bin file:
```shell
.\sega_fah -ubin file.bin out
```

## Building

```shell
g++ -o sega_fah src/main.cpp
```