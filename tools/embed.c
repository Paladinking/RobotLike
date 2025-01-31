#define _CRT_SECURE_NO_DEPRECATE
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined _WIN32 && (defined _MSC_VER || defined UNICODE)
    #ifndef UNICODE
        #define UNICODE
    #endif
    #ifndef _UNICODE
        #define _UNICODE
    #endif

    #include <fcntl.h>
    #include <io.h>
    #include <wchar.h>

    typedef wchar_t *Filename_t;
    #define OPEN(filename, mode) _wfopen(filename, L##mode)
    #define ERROR_FMT(format, ...) fwprintf(stderr, L##format, __VA_ARGS__)
    #define ERROR_PRINT(msg) fwprintf(stderr, L##msg)
    #define OUT_FMT(format, ...) wprintf(L##format, __VA_ARGS__)
    #define REMOVE(filename) _wremove(filename)
    #define ENTRY wmain
    #define CHAR wchar_t
    #define LTR(c) L##c
    #define STRLEN wcslen
    #define STRCMP wcscmp
    #define STRCHR(s, l) wcschr(s, L##l)
    #define STRRCHR(s, l) wcsrchr(s, L##l)
    #define realpath(N, R) _wfullpath((R), (N), 256)
    #if defined _MSC_VER || defined __clang__
        #define STR_FORMAT L"%S"
        #define F_FORMAT L"%s"
    #else
        #define STR_FORMAT L"%s"
        #define F_FORMAT L"%S"
    #endif
#else
    #ifdef _WIN32
        #define realpath(N, R) _fullpath((R), (N), 256)
    #endif
    typedef char *Filename_t;
    #define OPEN(filename, mode) fopen(filename, mode)
    #define ERROR_FMT(...) fprintf(stderr, __VA_ARGS__)
    #define ERROR_PRINT(msg) fprintf(stderr, msg)
    #define OUT_FMT(...) printf(__VA_ARGS__)
    #define REMOVE(filename) remove(filename)
    #define ENTRY main
    #define CHAR char
    #define LTR(c) c
    #define STRLEN strlen
    #define STRCMP strcmp
    #define STRCHR(s, l) strchr(s, l)
    #define STRRCHR(s, l) strrchr(s, l)
    #define STR_FORMAT "%s"
    #define F_FORMAT "%s"
#endif

#define BIT32 0x1000000
#define BIT64 0x2000000
#define MACHINE_X86 0x1
#define MACHINE_ARM 0x2
#define TYPE_COFF 0x10000
#define TYPE_ELF 0x20000
#define TYPE_MACHO 0x30000

enum Format {
    COFF64 = BIT64 | TYPE_COFF | MACHINE_X86,
    COFF32 = BIT32 | TYPE_COFF | MACHINE_X86,
    COFF64_ARM = BIT64 | TYPE_COFF | MACHINE_ARM,
    COFF32_ARM = BIT32 | TYPE_COFF | MACHINE_ARM,
    ELF64 = BIT64 | TYPE_ELF | MACHINE_X86,
    ELF32 = BIT32 | TYPE_ELF | MACHINE_X86,
    ELF64_ARM = BIT64 | TYPE_ELF | MACHINE_ARM,
    ELF32_ARM = BIT32 | TYPE_ELF | MACHINE_ARM,
    MACHO64 = BIT64 | TYPE_MACHO | MACHINE_X86,
    MACHO32 = BIT32 | TYPE_MACHO | MACHINE_X86,
    MACHO64_ARM = BIT64 | TYPE_MACHO | MACHINE_ARM,
    MACHO32_ARM = BIT32 | TYPE_MACHO | MACHINE_ARM,
    NONE_FORMAT = 0
};

#define IS_64BIT(format) (((format) & 0xf000000) == BIT64)
#define IS_32BIT(format) (((format) & 0xf000000) == BIT32)
#define IS_COFF(format) (((format) & 0xff0000) == TYPE_COFF)
#define IS_ELF(format) (((format) & 0xff0000) == TYPE_ELF)
#define IS_MACHO(format) (((format) & 0xff0000) == TYPE_MACHO)

enum Format FORMATS[] = {COFF64, COFF32, COFF64_ARM, COFF32_ARM, ELF64, ELF32, ELF64_ARM,
                         ELF32_ARM, MACHO64, MACHO32, MACHO64_ARM, MACHO32_ARM};

const char *format_names[] = {"coff64", "coff32", "coff64-arm", "coff32-arm",
                              "elf64",  "elf32",  "elf64-arm", "elf32-arm",
                              "macho64", "macho32", "macho64-arm", "macho32-arm"};

#define FORMAT_NAME_COUNT 12


#define DEFAULTS(name, format) const enum Format DEFAULT_FORMAT = format; const char* HOST_NAME = name

#if (defined _WIN32 || defined __CYGWIN__) && (defined _M_ARM64 || defined __aarch64__)
DEFAULTS("Windows arm64", COFF64_ARM);
#elif (defined _WIN32 || defined __CYGWIN__) && (defined _M_X64 || defined __x86_64__)
DEFAULTS("Windows x64", COFF64);
#elif (defined _WIN32 || defined __CYGWIN__) && (defined _M_ARM || defined __arm__)
DEFAULTS("Windows arm", COFF32_ARM);
#elif (defined _WIN32 || defined __CYGWIN__) && (defined _M_IX86 || defined __i386__)
DEFAULTS("Windows x86", COFF32);
#elif (defined __linux__) && (defined _M_ARM || defined __arm__)
DEFAULTS("Linux arm", ELF32_ARM);
#elif (defined __linux__) && (defined _M_IX86 || defined __i386__)
DEFAULTS("Linux x86", ELF32);
#elif (defined __linux__) && (defined _M_ARM64 || defined __aarch64__)
DEFAULTS("Linux arm64", ELF64_ARM);
#elif (defined __linux__) && (defined _M_X64 || defined __x86_64__)
DEFAULTS("Linux x64", ELF64);
#elif (defined __APPLE__ && defined __MACH__) && (defined __x86_64__)
DEFAULTS("OSX x64", MACHO64);
#elif (defined __APPLE__ && defined __MACH__) && defined(__i386__)
DEFAULTS("OSX x86", MACHO32);
#elif (defined __APPLE__ && defined __MACH__) && defined(__arm__)
DEFAULTS("OSX arm", MACHO32_ARM);
#elif (defined __APPLE__ && defined __MACH__) && defined(__aarch64__)
DEFAULTS("OSX arm64", MACHO64_ARM);
#elif (defined _M_X64 || defined __x86_64__)
DEFAULTS("Unkown x64", ELF64);
#elif (defined _M_IX86 || defined __i386__)
DEFAULTS("Unkown x86", ELF32);
#elif (defined _M_ARM || defined __arm__)
DEFAULTS("Unkown arm", ELF32_ARM);
#elif (defined _M_ARM64 || defined __aarch64__)
DEFAULTS("Unkown arm64", ELF64_ARM);
#else
DEFAULTS("Unkown", ELF64);
#endif


#define WRITE_U32(ptr, val)                                                    \
    do {                                                                       \
        (ptr)[0] = (val) & 0xff;                                               \
        (ptr)[1] = ((val) >> 8) & 0xff;                                        \
        (ptr)[2] = ((val) >> 16) & 0xff;                                       \
        (ptr)[3] = ((val) >> 24) & 0xff;                                       \
    } while (0)

#define WRITE_U64(ptr, val)                                                    \
    do {                                                                       \
        (ptr)[0] = (val) & 0xff;                                               \
        (ptr)[1] = ((val) >> 8) & 0xff;                                        \
        (ptr)[2] = ((val) >> 16) & 0xff;                                       \
        (ptr)[3] = ((val) >> 24) & 0xff;                                       \
        (ptr)[4] = ((val) >> 32) & 0xff;                                       \
        (ptr)[5] = ((val) >> 40) & 0xff;                                       \
        (ptr)[6] = ((val) >> 48) & 0xff;                                       \
        (ptr)[7] = ((val) >> 56) & 0xff;                                       \
    } while (0)

#define ALIGN_DIFF(val, p) (((val) % (p) == 0) ? (0) : (p) - ((val) % (p)))
#define ALIGN_TO(val, p) (val + ALIGN_DIFF(val, p))

bool write_all(FILE *file, uint32_t size, const uint8_t *buf) {
    uint32_t written = 0;
    while (written < size) {
        uint32_t w = (uint32_t)fwrite(buf + written, 1, size - written, file);
        if (w == 0) {
            return false;
        }
        written += w;
    }
    return true;
}

bool write_all_files(FILE *out, const Filename_t *names, const uint64_t *sizes,
                     uint32_t no_entries, const Filename_t outname,
                     uint64_t header_size, bool null_terminate, uint32_t alignment) {
    for (uint32_t i = 0; i < no_entries; ++i) {
        header_size += ALIGN_TO(sizes[i], 8);
        FILE *in = OPEN(names[i], "rb");
        if (in == NULL) {
            ERROR_FMT("Could not open file " F_FORMAT LTR("\n"), names[i]);
            return false;
        }
        uint8_t size_buf[8];
        WRITE_U64(size_buf, sizes[i]);
        if (fwrite(size_buf, 1, 8, out) != 8) {
            ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
            fclose(in);
            return false;
        }
        uint64_t to_read = sizes[i];
        uint8_t data_buf[4096];
        while (to_read > 0) {
            uint32_t data_chunk = (uint32_t)fread(data_buf, 1, 4096, in);
            if (data_chunk == 0) {
                ERROR_FMT("Reading from " F_FORMAT LTR(" failed\n"), names[i]);
                fclose(in);
                return false;
            }
            if (!write_all(out, data_chunk, data_buf)) {
                ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
                fclose(in);
                return false;
            }
            if (null_terminate) {
                if (fwrite("\0", 1, 1, out) != 1) {
                    ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
                    fclose(in);
                    return false;
                }
                data_chunk++;
            }
            to_read -= data_chunk;
        }
        if (ALIGN_DIFF(sizes[i], 8) != 0) {
            uint8_t null_data[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
            uint32_t padding = ALIGN_DIFF(sizes[i], 8);
            if (fwrite(null_data, 1, padding, out) != padding) {
                ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
                return false;
            }
        }
        fclose(in);
    }
    header_size += 8 * no_entries;
    if (ALIGN_DIFF(header_size, alignment) != 0) {
        uint8_t null_data[] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
        uint32_t padding = ALIGN_DIFF(header_size, alignment);
        if (fwrite(null_data, 1, padding, out) != padding) {
            ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
            return false;
        }

    }
    return true;
}

void write_c_header(const char **symbol_names, uint64_t *input_sizes,
                    uint32_t input_count, Filename_t header, bool readonly) {
    FILE *out = OPEN(header, "w");
    if (out == NULL) {
        ERROR_FMT("Could not open " F_FORMAT LTR("\n"), header);
        return;
    }

    if (fwrite("#pragma once\n#include <stdint.h>\n\n", 1, 34, out) != 34) {
        ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), header);
        goto error;
    }
    if (fwrite("#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n", 1, 40, out) != 40) {
        ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), header);
        goto error;
    }
    for (uint32_t i = 0; i < input_count; ++i) {
        const char *format =
            "extern uint64_t %s_size;\nextern uint8_t %s[%llu];\n\n";
        if (readonly) {
            format = "extern const uint64_t %s_size;\nextern const uint8_t "
                     "%s[%llu];\n\n";
        }
        if (fprintf(out, format, symbol_names[i], symbol_names[i],
                    (unsigned long long)input_sizes[i]) < 0) {
            ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), header);
            goto error;
        }
    }
    if (fwrite("#ifdef __cplusplus\n}\n#endif\n", 1, 28, out) != 28) {
        ERROR_FMT("Write to " F_FORMAT LTR(" failed\n"), header);
        goto error;
    }

    fclose(out);
    return;
error:
    fclose(out);
    REMOVE(header);
}

const uint8_t MACHO64_HEADER[] = {
    0xcf, 0xfa, 0xed, 0xfe, // Magic
    0x7, 0x0, 0x0, 0x1, // cputype x86 | x64
    0x3, 0x0, 0x0, 0x0, // cpu subtype X86_64_ALL
    0x1, 0x0, 0x0, 0x0, // filetype relocatable object
    0x3, 0x0, 0x0, 0x0, // Number of load commands
    0x0, 0x1, 0x0, 0x0, // Total size of commands
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 // Flags and reserved
};

const uint8_t MACHO32_HEADER[] = {
    0xce, 0xfa, 0xed, 0xfe, // Magic
    0x7, 0x0, 0x0, 0x0, // cputype x86
    0x3, 0x0, 0x0, 0x0, // cpu subtype X86_ALL
    0x1, 0x0, 0x0, 0x0, // filetype relocatable object
    0x3, 0x0, 0x0, 0x0, // Number of load commands
    0xe4, 0x0, 0x0, 0x0, // Total size of commands
    0x0, 0x0, 0x0, 0x0 // Flags
};

const uint8_t MACHO64_COMMANDS[] = {
    0x19, 0x0, 0x0, 0x0, // Command type segment 64
    0x98, 0x0, 0x0, 0x0, // Size
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Name None
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Virtual address 0
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Virtual size
    0x20, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // File offset
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // File size
    0x7, 0x0, 0x0, 0x0, 0x7, 0x0, 0x0, 0x0, // Max and inital protection
    0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Number of sections and flags

    '_', '_', 'c', 'o', 'n', 's', 't', 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Section name
    '_', '_', 'D', 'A', 'T', 'A', 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Segment name __DATA 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Section virtual address
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Section size
    0x20, 0x1, 0x0, 0x0, // Section file offset
    0x3, 0x0, 0x0, 0x0, // Section alignment 8
    0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, // Relocations offset + count
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // flags and reserved
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // reserved

    0x2, 0x0, 0x0, 0x0, 0x18, 0x0, 0x0, 0x0, // Symtab Id and size
    0xFF, 0xFF, 0xFF, 0xFF, // Symbol talbe file offset
    0xFF, 0xFF, 0xFF, 0xFF, // Number of symbols
    0xFF, 0xFF, 0xFF, 0xFF, // String table file offset
    0xFF, 0xFF, 0xFF, 0xFF, // String table size

    0xB, 0x0, 0x0, 0x0, 0x50, 0x0, 0x0, 0x0, // Dymsymdtab Id and size
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // 0 Local symbols
    0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xFF, 0xFF, // External symbols
    0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, // 0 undefined symobols    
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // No toc
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // No modtab
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // No external reference data
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // No indirect symtab
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // No external relocations
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 // No local relocations
};

const uint8_t MACHO32_COMMANDS[] = {
    0x1, 0x0, 0x0, 0x0, // Command type segment,
    0x7c, 0x0, 0x0, 0x0, // Command size 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Name None
    0x0, 0x0, 0x0, 0x0, // Virtual address 0
    0xFF, 0xFF, 0xFF, 0xFF, // Virtual size
    0x0, 0x1, 0x0, 0x0, // File offset
    0xFF, 0xFF, 0xFF, 0xFF, // File size
    0x7, 0x0, 0x0, 0x0, 0x7, 0x0, 0x0, 0x0, // Max and inital protection
    0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Number of sections and flags

    '_', '_', 'c', 'o', 'n', 's', 't', 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Section name __const
    '_', '_', 'D', 'A', 'T', 'A', 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Segment name __DATA 
    0x0, 0x0, 0x0, 0x0, // Section virtual address
    0xFF, 0xFF, 0xFF, 0xFF, // Section size
    0x0, 0x1, 0x0, 0x0, // Section file offset
    0x3, 0x0, 0x0, 0x0, // Section alignment 8
    0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, // Relocations offset + count
    0x0, 0x0, 0x0, 0x0, // Flags
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Reserved
    
    0x2, 0x0, 0x0, 0x0, 0x18, 0x0, 0x0, 0x0, // Symtab Id and size
    0xFF, 0xFF, 0xFF, 0xFF, // Symbol talbe file offset
    0xFF, 0xFF, 0xFF, 0xFF, // Number of symbols
    0xFF, 0xFF, 0xFF, 0xFF, // String table file offset
    0xFF, 0xFF, 0xFF, 0xFF, // String table size

    0xB, 0x0, 0x0, 0x0, 0x50, 0x0, 0x0, 0x0, // Dymsymdtab Id and size
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // 0 Local symbols
    0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF, 0xFF, 0xFF, // External symbols
    0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x0, 0x0, // 0 undefined symobols    
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // No toc
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // No modtab
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // No external reference data
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // No indirect symtab
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // No external relocations
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 // No local relocations 
};

const uint8_t MACHO64_SYMTAB_ENTRY[] = {
    0xFF, 0xFF, 0xFF, 0xFF, // String table offset 
    0x0F, 0x1, // External symbol used in section 1
    0x0, 0x0, // n_desc = 0
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF // Value
};

const uint8_t MACHO32_SYMTAB_ENTRY[] = {
    0xFF, 0xFF, 0xFF, 0xFF, // String table offset,
    0x0F, 0x1, // External symbol used in section 1
    0x0, 0x0, // n_desc = 0
    0xFF, 0xFF, 0xFF, 0xFF // Value
};


void write_macho_header(enum Format format, uint32_t *size, uint8_t* data) {
    if (format == MACHO32 || format == MACHO32_ARM) {
        memcpy(data, MACHO32_HEADER, sizeof(MACHO32_HEADER));
        *size = sizeof(MACHO32_HEADER);
            
    } else {
        memcpy(data, MACHO64_HEADER, sizeof(MACHO64_HEADER));
        *size = sizeof(MACHO64_HEADER);
    }
    if (format == MACHO32_ARM || format == MACHO64_ARM) {
        data[4] = 0xc;
        WRITE_U32(data + 8, 0x0);
    }
}

void write_macho_commands(enum Format format, uint64_t data_size, uint32_t no_symbols, uint64_t strtable_size, bool readonly, uint64_t* size, uint8_t* data) {
    if (format == MACHO32 || format == MACHO32_ARM) {
        memcpy(data, MACHO32_COMMANDS, sizeof(MACHO32_COMMANDS));
        *size = sizeof(MACHO32_COMMANDS);
        WRITE_U32(data + 28, ALIGN_TO(data_size, 4));
        WRITE_U32(data + 36, ALIGN_TO(data_size, 4));
        WRITE_U32(data + 92, data_size);
        data_size += sizeof(MACHO32_HEADER) + sizeof(MACHO32_COMMANDS);
        data_size = ALIGN_TO(data_size, 4);
        WRITE_U32(data + 104, data_size);
        WRITE_U32(data + 132, data_size);
        WRITE_U32(data + 136, 2 * no_symbols);
        WRITE_U32(data + 140, data_size + 2 * sizeof(MACHO32_SYMTAB_ENTRY) * no_symbols);
        WRITE_U32(data + 144, strtable_size);
        WRITE_U32(data + 168, 2 * no_symbols);
        WRITE_U32(data + 172, 2 * no_symbols);
        if (!readonly) {
            memcpy(data + 56, "__data", 7);
        }
    } else {
        memcpy(data, MACHO64_COMMANDS, sizeof(MACHO64_COMMANDS));
        *size = sizeof(MACHO64_COMMANDS);
        WRITE_U64(data + 32, ALIGN_TO(data_size, 8));
        WRITE_U64(data + 48, ALIGN_TO(data_size, 8));
        WRITE_U64(data + 112, data_size);
        data_size += sizeof(MACHO64_HEADER) + sizeof(MACHO64_COMMANDS);
        data_size = ALIGN_TO(data_size, 8);
        WRITE_U32(data + 128, data_size);
        WRITE_U32(data + 160, data_size);
        WRITE_U32(data + 164, 2 * no_symbols);
        WRITE_U32(data + 168, data_size + 2 * sizeof(MACHO64_SYMTAB_ENTRY) * no_symbols);
        WRITE_U32(data + 172, strtable_size);
        WRITE_U32(data + 196, 2 * no_symbols);
        WRITE_U32(data + 200, 2 * no_symbols);
        if (!readonly) {
            memcpy(data + 72, "__data", 7);
        }
    }
}

typedef struct macho_symbol {
    char* name;
    uint64_t offset;
} macho_symbol_t;

int sym_cmp(const void* a, const void* b) {
    const macho_symbol_t *a1 = a;
    const macho_symbol_t *b1 = b;
    return strcmp(a1->name, b1->name);
}

uint8_t* write_macho_symbol_table(enum Format format, const uint64_t* sizes, char** names, uint32_t no_symbols, uint64_t* size) {
    if (format == MACHO32 || format == MACHO32_ARM) {
        *size = 2 * no_symbols * sizeof(MACHO32_SYMTAB_ENTRY) + 1;
    } else {
        *size = 2 * no_symbols * sizeof(MACHO64_SYMTAB_ENTRY) + 1;
    }
    uint64_t strtable_base = *size - 1;
    macho_symbol_t* sorted_names = malloc(2 * no_symbols * sizeof(macho_symbol_t));
    for (uint32_t i = 0; i < no_symbols; ++i) {
        uint32_t name_len = strlen(names[i]);
        *size += 2 * name_len + 9;
        sorted_names[2 * i].name = malloc(name_len + 7);
        sorted_names[2 * i].name[0] = '_';
        memcpy(sorted_names[2 * i].name + 1, names[i], name_len);
        memcpy(sorted_names[2 * i].name + 1 + name_len, "_size", 6);
        if (i == 0) {
            sorted_names[2 * i].offset = 0;
        } else {
            sorted_names[2 * i].offset = sorted_names[2 * (i - 1)].offset + ALIGN_TO(sizes[i - 1], 8) + 8;
        }
        sorted_names[2 * i + 1].offset = sorted_names[2 * i].offset + 8;
        sorted_names[2 * i + 1].name = malloc(name_len + 2);
        sorted_names[2 * i + 1].name[0] = '_';
        memcpy(sorted_names[2 * i + 1].name + 1, names[i], name_len + 1);
    }
    qsort(sorted_names, 2 * no_symbols, sizeof(macho_symbol_t), sym_cmp);
    uint8_t* data = malloc(*size);
    uint8_t* strtable = data + strtable_base;
    uint64_t data_base = 0;
    *strtable = '\0';
    strtable_base = 1;
    for (uint32_t i = 0; i < 2 * no_symbols; ++i) {
        uint32_t name_len = strlen(sorted_names[i].name);
        if (format == MACHO32 || format == MACHO32_ARM) {
            memcpy(data + data_base, MACHO32_SYMTAB_ENTRY, sizeof(MACHO32_SYMTAB_ENTRY));
            WRITE_U32(data + data_base, strtable_base);
            WRITE_U32(data + data_base + 8, sorted_names[i].offset);
            data_base += sizeof(MACHO32_SYMTAB_ENTRY);
        } else {
            memcpy(data + data_base, MACHO64_SYMTAB_ENTRY, sizeof(MACHO64_SYMTAB_ENTRY));
            WRITE_U32(data + data_base, strtable_base);
            WRITE_U64(data + data_base + 8, sorted_names[i].offset);
            data_base += sizeof(MACHO64_SYMTAB_ENTRY);
        }
        memcpy(strtable + strtable_base, sorted_names[i].name, name_len + 1);
        free(sorted_names[i].name);
        strtable_base += name_len + 1;
    }

    free(sorted_names);
    return data;
}


bool write_macho(char **names, const Filename_t *files,
               const uint64_t *size, const uint32_t no_symbols,
               const Filename_t outname, bool readonly, bool null_terminate, enum Format format) {
    FILE* out = OPEN(outname, "wb");
    bool m64 = IS_64BIT(format);
    if (out == NULL) {
        ERROR_FMT("Could not create file " F_FORMAT LTR("\n"), outname);
        return false;
    }
    uint8_t header[sizeof(MACHO64_HEADER)];
    uint32_t header_size;
    write_macho_header(format, &header_size, header);
    if (!write_all(out, header_size, header)) {
        ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
        goto error;
    }
    uint64_t strtable_size = 1;
    uint64_t data_size = 0;
    for (uint32_t i = 0; i < no_symbols; ++i) {
        data_size += ALIGN_TO(size[i], 8) + 8;
        strtable_size += 2 * strlen(names[i]) + 9;
        if (data_size > 0xffffffff || strtable_size > 0xffffffff) {
            ERROR_PRINT("Object does not fit all data\n");
            goto error;
        }
    }
    uint64_t commands_size;
    uint8_t commands[sizeof(MACHO64_COMMANDS)];
    write_macho_commands(format, data_size, no_symbols, strtable_size, readonly, &commands_size, commands);
    if (!write_all(out, commands_size, commands)) {
        ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
        goto error;
    }

    if (!write_all_files(out, files, size, no_symbols, outname, header_size + commands_size,
                null_terminate, m64 ? 8 : 4)) {
        goto error;
    }

    uint64_t symbols_size;
    uint8_t* data = write_macho_symbol_table(format, size, names, no_symbols, &symbols_size);
    if (!write_all(out, symbols_size, data)) {
        ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
        goto error;
    }
    free(data);

    fclose(out);
    return true;
error:
    fclose(out);
    REMOVE(outname);
    return false;
}

const uint8_t ELF64_HEADER[] = {
    0x7f, 'E',  'L',  'F',                   // Magic
    0x2,                                     // Class Elf 64
    0x1,                                     // Data encoding little endian
    0x1,                                     // Version 1
    0x0,  0x0,                               // ABI System V version 0
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // Padding
    0x1,  0x0,                               // Type relocatable object
    62,   0x0,                               // Machine AMD-64
    0x1,  0x0,  0x0,  0x0,                   // Object version 1
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  // Entry point none
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  // Program header offset
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Section header offset
    0x0,  0x0,  0x0,  0x0,                          // Flags 0
    0x40, 0x0,                                      // Elf header size 64
    0x0,  0x0,  0x0,  0x0, // Size and number of program headers
    0x40, 0x0,  0x6,  0x0, // Size and number of section headers
    0x4,  0x0              // Index of .shstrtab
};

const uint8_t ELF32_HEADER[] = {
    0x7f, 'E',  'L',  'F',                 // Magic
    0x1,                                   // Class Elf 32
    0x1,                                   // Data encoding little endian
    0x1,                                   // Version 1
    0x0,  0x0,                             // ABI System V version 0
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, // Padding
    0x1,  0x0,                             // Type relocatable object
    3,    0x0,                             // Machine I80386
    0x1,  0x0,  0x0,  0x0,                 // Object version 1
    0x0,  0x0,  0x0,  0x0,                 // Entry point none
    0x0,  0x0,  0x0,  0x0,                 // Program header offset
    0xFF, 0xFF, 0xFF, 0xFF,                // Section header offset
    0x0,  0x0,  0x0,  0x0,                 // Flags 0
    0x34, 0x0,                             // Elf header size 52
    0x0,  0x0,  0x0,  0x0,                 // Size and number of program headers
    0x28, 0x0,  0x6,  0x0,                 // Size and number of section headers
    0x4,  0x0                              // Index of .shstrtab
};
//.note.GNU-stack
const uint8_t ELF_SHSTRTAB[] = {
    '\0', '.', 'r',  'd', 'a', 't', 'a', '\0', '.', 's', 'y',  'm', 't',
    'a',  'b', '\0', '.', 's', 't', 'r', 't',  'a', 'b', '\0', '.', 's',
    'h',  's', 't',  'r', 't', 'a', 'b', '\0', '.', 'n', 'o',  't', 'e',
    '.',  'G', 'N',  'U', '-', 's', 't', 'a',  'c', 'k', '\0'};

const uint8_t ELF64_SECTION_HEADERS[] = {
    0x0, 0x0, 0x0, 0x0,                             // NULL
    0x0, 0x0, 0x0, 0x0,                             // NULL
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // NULL
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // NULL
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // NULL
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // NULL
    0x0, 0x0, 0x0, 0x0,                             // NULL
    0x0, 0x0, 0x0, 0x0,                             // NULL
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // NULL
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // NULL
                                                    //
    0x1, 0x0, 0x0, 0x0,                             // .(r)data name offset
    0x1, 0x0, 0x0, 0x0,                             // .(r)data type PROGBITS
    0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .(r)data flags A
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .(r)data address
    0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,        // .(r)data file offset
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // .(r)data size
    0x0, 0x0, 0x0, 0x0,                             // .(r)data link 0
    0x0, 0x0, 0x0, 0x0,                             // .(r)data info 0
    0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .(r)data alignment 8
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .(r)data entry size 0
                                                    //
    0x8, 0x0, 0x0, 0x0,                             // .symtab name offset
    0x2, 0x0, 0x0, 0x0,                             // .symtab type SYMTAB
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .symtab flags 0
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .symtab address 0
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // .symtab file offset
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // .symtab size
    0x3, 0x0, 0x0, 0x0,                             // .symtab link 0
    0x1, 0x0, 0x0, 0x0,                      // .symtab info 1 local symbol
    0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,  // .symtab alignment 8
    0x18, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // .symtab entry size 0x18

    0x10, 0x0, 0x0, 0x0,                            // .strtab name offset
    0x3, 0x0, 0x0, 0x0,                             // .strtab type STRTAB
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .strtab flags 0
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .strtab address 0
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // .strtab file offset
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // .strtab size
    0x0, 0x0, 0x0, 0x0,                             // strtab link 0
    0x0, 0x0, 0x0, 0x0,                             // strtab info 0
    0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .strtab alignment 1
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .strtab entry size 0

    0x18, 0x0, 0x0, 0x0,                            // .shstrtab name offset
    0x3, 0x0, 0x0, 0x0,                             // .shstrtab type STRTAB
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .shstrtab flags 0
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .shstrtab address 0
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // .shstrtab file offset
    0x32, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,        // .shstrtab size
    0x0, 0x0, 0x0, 0x0,                             // shstrtab link 0
    0x0, 0x0, 0x0, 0x0,                             // shstrtab info 0
    0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .shstrtab alignment 1
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .shstrtab entry size 0

    0x22, 0x0, 0x0, 0x0,                    // .note.GNU-stack name offset
    0x1, 0x0, 0x0, 0x0,                     // .note.GNU-stack type PROGBITS
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // .note.GNU-stack flags 0
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // .note.GNU-stack address 0
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // .note.GNU-stack offset
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,         // .note.GNU-stack size 0
    0x0, 0x0, 0x0, 0x0,                             // .note.GNU-stack link 0
    0x0, 0x0, 0x0, 0x0,                             // .note.GNU-stack info 0
    0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // .note.GNU-stack alignment 1
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0  // .note.GNU-stack entry size 0
};

const uint8_t ELF32_SECTION_HEADERS[] = {
    0x0, 0x0, 0x0, 0x0, // NULL
    0x0, 0x0, 0x0, 0x0, // NULL
    0x0, 0x0, 0x0, 0x0, // NULL
    0x0, 0x0, 0x0, 0x0, // NULL
    0x0, 0x0, 0x0, 0x0, // NULL
    0x0, 0x0, 0x0, 0x0, // NULL
    0x0, 0x0, 0x0, 0x0, // NULL
    0x0, 0x0, 0x0, 0x0, // NULL
    0x0, 0x0, 0x0, 0x0, // NULL
    0x0, 0x0, 0x0, 0x0, // NULL

    0x1, 0x0, 0x0, 0x0,     // .(r)data name offset
    0x1, 0x0, 0x0, 0x0,     // .(r)data type PROGBITS
    0x2, 0x0, 0x0, 0x0,     // .(r)data flags A
    0x0, 0x0, 0x0, 0x0,     // .(r)data address
    0x34, 0x0, 0x0, 0x0,    // .(r)data file offset
    0xFF, 0xFF, 0xFF, 0xFF, // .(r)data size
    0x0, 0x0, 0x0, 0x0,     // .(r)data link 0
    0x0, 0x0, 0x0, 0x0,     // .(r)data info 0
    0x8, 0x0, 0x0, 0x0,     // .(r)data alignment 8
    0x0, 0x0, 0x0, 0x0,     // .(r)data entry size 0
                            //
    0x8, 0x0, 0x0, 0x0,     // .symtab name offset
    0x2, 0x0, 0x0, 0x0,     // .symtab type SYMTAB
    0x0, 0x0, 0x0, 0x0,     // .symtab flags 0
    0x0, 0x0, 0x0, 0x0,     // .symtab address 0
    0xFF, 0xFF, 0xFF, 0xFF, // .symtab file offset
    0xFF, 0xFF, 0xFF, 0xFF, // .symtab size
    0x3, 0x0, 0x0, 0x0,     // .symtab link 0
    0x1, 0x0, 0x0, 0x0,     // .symtab info 1 local symbol
    0x4, 0x0, 0x0, 0x0,     // .symtab alignment 4
    0x10, 0x0, 0x0, 0x0,    // .symtab entry size 0x18

    0x10, 0x0, 0x0, 0x0,    // .strtab name offset
    0x3, 0x0, 0x0, 0x0,     // .strtab type STRTAB
    0x0, 0x0, 0x0, 0x0,     // .strtab flags 0
    0x0, 0x0, 0x0, 0x0,     // .strtab address 0
    0xFF, 0xFF, 0xFF, 0xFF, // .strtab file offset
    0xFF, 0xFF, 0xFF, 0xFF, // .strtab size
    0x0, 0x0, 0x0, 0x0,     // strtab link 0
    0x0, 0x0, 0x0, 0x0,     // strtab info 0
    0x1, 0x0, 0x0, 0x0,     // .strtab alignment 1
    0x0, 0x0, 0x0, 0x0,     // .strtab entry size 0

    0x18, 0x0, 0x0, 0x0,    // .shstrtab name offset
    0x3, 0x0, 0x0, 0x0,     // .shstrtab type STRTAB
    0x0, 0x0, 0x0, 0x0,     // .shstrtab flags 0
    0x0, 0x0, 0x0, 0x0,     // .shstrtab address 0
    0xFF, 0xFF, 0xFF, 0xFF, // .shstrtab file offset
    0x32, 0x0, 0x0, 0x0,    // .shstrtab size
    0x0, 0x0, 0x0, 0x0,     // shstrtab link 0
    0x0, 0x0, 0x0, 0x0,     // shstrtab info 0
    0x1, 0x0, 0x0, 0x0,     // .shstrtab alignment 1
    0x0, 0x0, 0x0, 0x0,     // .shstrtab entry size 0

    0x22, 0x0, 0x0, 0x0,                    // .note.GNU-stack name offset
    0x1, 0x0, 0x0, 0x0,                     // .note.GNU-stack type PROGBITS
    0x0, 0x0, 0x0, 0x0, // .note.GNU-stack flags 0
    0x0, 0x0, 0x0, 0x0, // .note.GNU-stack address 0
    0xFF, 0xFF, 0xFF, 0xFF, // .note.GNU-stack offset
    0x0, 0x0, 0x0, 0x0, // .note.GNU-stack size 0
    0x0, 0x0, 0x0, 0x0,                             // .note.GNU-stack link 0
    0x0, 0x0, 0x0, 0x0,                             // .note.GNU-stack info 0
    0x1, 0x0, 0x0, 0x0, // .note.GNU-stack alignment 1
    0x0, 0x0, 0x0, 0x0, // .note.GNU-stack entry size 0
};

const uint8_t ELF64_SYMTAB[] = {
    0xFF, 0xFF, 0xFF, 0xFF, // symbol size name
    0x10, 0x0,              // symbol size info GLOBAL NOTYPE, reserved 0
    0x1,  0x0,              // symbol size section index
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // symbol size value
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  // symbol size size
    0xFF, 0xFF, 0xFF, 0xFF,                         // symbol name
    0x10, 0x0, // symbo info GLOBAL NOTYPE, reserved 0
    0x1,  0x0, // symbol section index
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // symbol value
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  // symbol size
};

const uint8_t ELF32_SYMTAB[] = {
    0xFF, 0xFF, 0xFF, 0xFF, // symbol size name
    0xFF, 0xFF, 0xFF, 0xFF, // symbol size value
    0x0,  0x0,  0x0,  0x0,  // symbol size size
    0x10, 0x0,              // symbol size info GLOBAL NOTYPE, reserved 0
    0x1,  0x0,              // symbol size section index
    0xFF, 0xFF, 0xFF, 0xFF, // symbol name
    0xFF, 0xFF, 0xFF, 0xFF, // symbol value
    0x0,  0x0,  0x0,  0x0,  // symbol size
    0x10, 0x0,              // symbo info GLOBAL NOTYPE, reserved 0
    0x1,  0x0               // symbol section index
};

void write_elf64_header(uint64_t data_size, uint32_t no_symbols,
                        uint64_t strtab_size, uint8_t machine, uint8_t *out) {
    memcpy(out, ELF64_HEADER, sizeof(ELF64_HEADER));
    data_size += 8 * no_symbols + sizeof(ELF64_HEADER);
    data_size = ALIGN_TO(data_size, 8);
    data_size += 24 + no_symbols * sizeof(ELF64_SYMTAB) + sizeof(ELF_SHSTRTAB) +
        strtab_size;
    data_size = ALIGN_TO(data_size, 8);
    WRITE_U64(out + 40, data_size);
    out[18] = machine;
}

void write_elf32_header(uint32_t data_size, uint32_t no_symbols,
                        uint32_t strtab_size, uint8_t machine, uint8_t *out) {
    memcpy(out, ELF32_HEADER, sizeof(ELF32_HEADER));
    data_size += 8 * no_symbols + sizeof(ELF32_HEADER);
    data_size = ALIGN_TO(data_size, 4);
    data_size += 16 + no_symbols * sizeof(ELF32_SYMTAB) + sizeof(ELF_SHSTRTAB) +
                 strtab_size;
    data_size = ALIGN_TO(data_size, 4);
    WRITE_U32(out + 32, data_size);
    out[18] = machine;
}

uint8_t *write_elf64_symtab(const char **names, const uint64_t *size,
                            uint32_t no_symbols, uint64_t *data_size) {
    *data_size = 24 + sizeof(ELF64_SYMTAB) * no_symbols + 1;
    for (uint32_t i = 0; i < no_symbols; ++i) {
        *data_size += 2 * strlen(names[i]) + 7;
    }
    uint8_t *data = malloc(*data_size);
    uint8_t *strtab = data + 24 + sizeof(ELF64_SYMTAB) * no_symbols;
    *strtab = 0;
    uint32_t strtab_ix = 1;
    uint64_t data_ix = 0;
    memset(data, 0, 24);
    for (uint32_t i = 0; i < no_symbols; ++i) {
        uint8_t *base = data + 24 + sizeof(ELF64_SYMTAB) * i;
        memcpy(base, ELF64_SYMTAB, sizeof(ELF64_SYMTAB));
        uint32_t name_len = (uint32_t)strlen(names[i]);
        memcpy(strtab + strtab_ix, names[i], name_len);
        memcpy(strtab + strtab_ix + name_len, "_size", 6);
        memcpy(strtab + strtab_ix + name_len + 6, names[i], name_len + 1);
        WRITE_U32(base, strtab_ix);
        WRITE_U32(base + 24, strtab_ix + name_len + 6);
        strtab_ix += 2 * name_len + 7;
        WRITE_U64(base + 8, data_ix);
        WRITE_U64(base + 24 + 8, data_ix + 8);
        data_ix += 8 + ALIGN_TO(size[i], 8);
    }
    return data;
}

uint8_t *write_elf32_symtab(const char **names, const uint64_t *size,
                            uint32_t no_symbols, uint64_t *data_size) {
    *data_size = 16 + sizeof(ELF32_SYMTAB) * no_symbols + 1;
    for (uint32_t i = 0; i < no_symbols; ++i) {
        *data_size += 2 * strlen(names[i]) + 7;
    }
    uint8_t *data = malloc(*data_size);
    uint8_t *strtab = data + 16 + sizeof(ELF32_SYMTAB) * no_symbols;
    *strtab = 0;
    uint32_t strtab_ix = 1;
    uint32_t data_ix = 0;
    memset(data, 0, 16);
    for (uint32_t i = 0; i < no_symbols; ++i) {
        uint8_t *base = data + 16 + sizeof(ELF32_SYMTAB) * i;
        memcpy(base, ELF32_SYMTAB, sizeof(ELF32_SYMTAB));
        uint32_t name_len = (uint32_t)strlen(names[i]);
        memcpy(strtab + strtab_ix, names[i], name_len);
        memcpy(strtab + strtab_ix + name_len, "_size", 6);
        memcpy(strtab + strtab_ix + name_len + 6, names[i], name_len + 1);
        WRITE_U32(base, strtab_ix);
        WRITE_U32(base + 16, strtab_ix + name_len + 6);
        strtab_ix += 2 * name_len + 7;
        WRITE_U32(base + 4, data_ix);
        WRITE_U32(base + 16 + 4, data_ix + 8);
        data_ix += 8 + ALIGN_TO(size[i], 8);
    }
    return data;
}

void write_elf64_section_headers(uint64_t data_size, uint32_t no_symbols,
                                 uint64_t strtab_size, bool readonly,
                                 uint8_t *out) {
    memcpy(out, ELF64_SECTION_HEADERS, sizeof(ELF64_SECTION_HEADERS));
    if (!readonly) {
        out[72] |= 1;
    }
    data_size += 8 * no_symbols;
    WRITE_U64(out + 64 + 32, data_size);
    data_size = ALIGN_TO(data_size, 8);
    data_size += 0x40;
    WRITE_U64(out + 2 * 64 + 24, data_size);
    WRITE_U64(out + 2 * 64 + 32,
              (uint64_t)(24 + sizeof(ELF64_SYMTAB) * no_symbols));
    data_size += 24 + sizeof(ELF64_SYMTAB) * no_symbols;
    WRITE_U64(out + 3 * 64 + 24, data_size);
    WRITE_U64(out + 3 * 64 + 32, strtab_size);
    data_size += strtab_size;
    WRITE_U64(out + 4 * 64 + 24, data_size);
    data_size += sizeof(ELF_SHSTRTAB);
    WRITE_U64(out + 5 * 64 + 24, data_size);
}

void write_elf32_section_headers(uint32_t data_size, uint32_t no_symbols,
                                 uint32_t strtab_size, bool readonly,
                                 uint8_t *out) {
    memcpy(out, ELF32_SECTION_HEADERS, sizeof(ELF32_SECTION_HEADERS));
    if (!readonly) {
        out[48] |= 1;
    }
    data_size += 8 * no_symbols;
    WRITE_U32(out + 40 + 20, data_size);
    data_size = ALIGN_TO(data_size, 4);
    data_size += 0x34;
    WRITE_U32(out + 2 * 40 + 16, data_size);
    WRITE_U32(out + 2 * 40 + 20, 16 + sizeof(ELF32_SYMTAB) * no_symbols);
    data_size += 16 + sizeof(ELF32_SYMTAB) * no_symbols;
    WRITE_U32(out + 3 * 40 + 16, data_size);
    WRITE_U32(out + 3 * 40 + 20, strtab_size);
    data_size += strtab_size;
    WRITE_U32(out + 4 * 40 + 16, data_size);
    data_size += sizeof(ELF_SHSTRTAB);
    WRITE_U32(out + 5 * 40 + 16, data_size);
}

bool write_elf(const char **names, const Filename_t *files,
               const uint64_t *size, const uint32_t no_symbols,
               const Filename_t outname, bool readonly, bool null_terminate, enum Format format) {
    FILE *out = OPEN(outname, "wb");
    bool elf32 = IS_32BIT(format);
    if (out == NULL) {
        ERROR_FMT("Could not create file " F_FORMAT LTR("\n"), outname);
        return false;
    }

    uint64_t data_size = 0;
    uint64_t strtab_size = 1;
    for (uint32_t i = 0; i < no_symbols; ++i) {
        data_size += ALIGN_TO(size[i], 8);
        strtab_size += 2 * strlen(names[i]) + 7;
        if (strtab_size > UINT32_MAX) {
            ERROR_PRINT("Total length of all symbol names to large\n");
            goto error;
        }
        if (elf32 && data_size > UINT32_MAX) {
            ERROR_PRINT("Elf32 object does not fit all data\n");
            goto error;
        }
    }

    uint8_t header[sizeof(ELF64_HEADER)];
    uint32_t hsize;
    if (elf32) {
        hsize = sizeof(ELF32_HEADER);
        uint8_t machine = format == ELF32 ? 3 : 40;
        write_elf32_header(data_size, no_symbols, strtab_size, machine, header);
    } else {
        hsize = sizeof(ELF64_HEADER);
        uint8_t machine = format == ELF64 ? 62 : 183;
        write_elf64_header(data_size, no_symbols, strtab_size, machine, header);
    }
    if (!write_all(out, hsize, header)) {
        ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
        goto error;
    }

    if (!write_all_files(out, files, size, no_symbols, outname, hsize, null_terminate,
                         elf32 ? 4 : 8)) {
        goto error;
    }
    uint64_t symtab_size;
    uint8_t *symtab;
    if (elf32) {
        symtab = write_elf32_symtab(names, size, no_symbols, &symtab_size);
    } else {
        symtab = write_elf64_symtab(names, size, no_symbols, &symtab_size);
    }
    uint8_t shstrtab[sizeof(ELF_SHSTRTAB)];
    memcpy(shstrtab, ELF_SHSTRTAB, sizeof(ELF_SHSTRTAB));
    if (!readonly) {
        memcpy(shstrtab + 1, ".data", 6);
    }
    if (!write_all(out, symtab_size, symtab) ||
        !write_all(out, sizeof(ELF_SHSTRTAB), shstrtab)) {
        free(symtab);
        ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
        goto error;
    }
    free(symtab);
    uint64_t fsize = symtab_size + sizeof(ELF_SHSTRTAB);
    uint32_t align = elf32 ? 4 : 8;
    if (ALIGN_DIFF(fsize, align) != 0) {
        uint8_t padding[] = {0, 0, 0, 0, 0, 0, 0, 0};
        if (fwrite(padding, 1, ALIGN_DIFF(fsize, align), out) !=
            ALIGN_DIFF(fsize, align)) {
            ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
            goto error;
        }
    }
    uint8_t section_headers[sizeof(ELF64_SECTION_HEADERS)];
    uint32_t sh_size;
    if (elf32) {
        sh_size = sizeof(ELF32_SECTION_HEADERS);
        write_elf32_section_headers(data_size, no_symbols, strtab_size,
                                    readonly, section_headers);
    } else {
        sh_size = sizeof(ELF64_SECTION_HEADERS);
        write_elf64_section_headers(data_size, no_symbols, strtab_size,
                                    readonly, section_headers);
    }
    if (!write_all(out, sh_size, section_headers)) {
        ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
        goto error;
    }

    fclose(out);
    return true;
error:
    fclose(out);
    REMOVE(outname);
    return false;
}

const uint8_t COFF_HEADER[] = {
    0x64, 0x86,             // Machine x64
    0x1,  0x0,              // NumberOfSections 1
    0x0,  0x0,  0x0,  0x0,  // Timestamp 0
    0xFF, 0xFF, 0xFF, 0xFF, // Coff table file offset
    0xFF, 0xFF, 0xFF, 0xFF, // Number of symbols
    0x0,  0x0,              // Size of optional header 0
    0x0,  0x0               // Characteristics 0
};

const uint8_t COFF_SECTION_HEADER[] = {
    '.',  'r',  'd',  'a',  't', 'a', 0x0, 0x0, // Name
    0x0,  0x0,  0x0,  0x0,                      // VirtualSize
    0x0,  0x0,  0x0,  0x0,                      // VirtualAdress
    0xFF, 0xFF, 0xFF, 0xFF,                     // SizeOfRawData
    0x3c, 0x0,  0x0,  0x0,                      // PointerToRawData
    0x0,  0x0,  0x0,  0x0,                      // PointerToRelocations
    0x0,  0x0,  0x0,  0x0,                      // PointerToLinenumbers
    0x0,  0x0,                                  // NumberOfRelocations
    0x0,  0x0,                                  // NumberOfLinenumbers
    0x40, 0x0,  0x40, 0x40 // Characteristics Initialized, 8-byte aligned, read
};

const uint8_t COFF_SYMBOL_ENTRY[] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // name
    0x0, 0x0, 0x0, 0x0,                     // offset
    0x1, 0x0,                               // section number
    0x0, 0x0,                               // type
    0x2,                                    // storage class
    0x0                                     // Aux symbols
};

void write_coff_header(uint32_t data_size, uint8_t *data, uint32_t no_symbols,
                       enum Format format) {
    memcpy(data, COFF_HEADER, sizeof(COFF_HEADER));
    data_size = data_size + 8 * no_symbols;
    if ((20 + 40 + data_size) % 8 != 0) {
        data_size += 8 - ((20 + 40 + data_size) % 8);
    }
    uint32_t coff_offset = 20 + 40 + data_size;
    WRITE_U32(data + 8, coff_offset);
    WRITE_U32(data + 12, no_symbols * 2);
    if (format == COFF32) {
        data[0] = 0x4C;
        data[1] = 0x01;
    } else if (format == COFF32_ARM) {
        data[0] = 0xC0;
        data[1] = 0x01;
    } else if (format == COFF64_ARM) {
        data[0] = 0x64;
        data[1] = 0xaa;
    }
}

void write_coff_section_header(uint32_t data_size, uint32_t no_symbols,
                               bool readonly, uint8_t *data) {
    memcpy(data, COFF_SECTION_HEADER, sizeof(COFF_SECTION_HEADER));
    if (!readonly) {
        memcpy(data, ".data", 6);
        data[39] |= 0x80;
    }
    data_size = data_size + 8 * no_symbols;
    data[16] = data_size & 0xff;
    data[17] = (data_size >> 8) & 0xff;
    data[18] = (data_size >> 16) & 0xff;
    data[19] = (data_size >> 24) & 0xff;
}

uint8_t *write_coff_symbol_table(const char **names, const uint64_t *size,
                                 uint32_t no_symbols, bool coff32,
                                 uint32_t *data_size) {
    *data_size = 2 * sizeof(COFF_SYMBOL_ENTRY) * no_symbols + 4;
    for (uint32_t i = 0; i < no_symbols; ++i) {
        uint32_t name_len = (uint32_t)strlen(names[i]);
        if (coff32) {
            ++name_len;
        }
        if (name_len > 3) {
            if (name_len > 8) {
                *data_size += name_len * 2 + 7;
            } else {
                *data_size += name_len + 6;
            }
        }
    }

    uint8_t *data = malloc(*data_size);
    uint8_t *strtable = data + 2 * sizeof(COFF_SYMBOL_ENTRY) * no_symbols;
    uint32_t strtable_offset = 0x4;
    uint32_t offset = 0;
    for (uint32_t i = 0; i < no_symbols; ++i) {
        uint32_t name_len = (uint32_t)strlen(names[i]);
        if (coff32) {
            ++name_len;
        }
        uint32_t data_offset = 2 * sizeof(COFF_SYMBOL_ENTRY) * i;
        memcpy(data + data_offset, COFF_SYMBOL_ENTRY,
               sizeof(COFF_SYMBOL_ENTRY));
        memcpy(data + data_offset + sizeof(COFF_SYMBOL_ENTRY),
               COFF_SYMBOL_ENTRY, sizeof(COFF_SYMBOL_ENTRY));
        if (name_len > 3) {
            if (coff32) {
                *(strtable + strtable_offset) = '_';
                memcpy(strtable + strtable_offset + 1, names[i], name_len - 1);
            } else {
                memcpy(strtable + strtable_offset, names[i], name_len);
            }
            memcpy(strtable + strtable_offset + name_len, "_size", 6);
            WRITE_U32(data + data_offset + 4, strtable_offset);
            strtable_offset += name_len + 6;
        } else {
            if (coff32) {
                *(data + data_offset) = '_';
                memcpy(data + data_offset + 1, names[i], name_len - 1);
            } else {
                memcpy(data + data_offset, names[i], name_len);
            }
            memcpy(data + data_offset + name_len, "_size", 5);
        }
        WRITE_U32(data + data_offset + 8, offset);
        if (name_len > 8) {
            if (coff32) {
                *(strtable + strtable_offset) = '_';
                memcpy(strtable + strtable_offset + 1, names[i], name_len);
            } else {
                memcpy(strtable + strtable_offset, names[i], name_len + 1);
            }
            WRITE_U32(data + data_offset + sizeof(COFF_SYMBOL_ENTRY) + 4,
                      strtable_offset);
            strtable_offset += name_len + 1;
        } else {
            if (coff32) {
                *(data + data_offset + sizeof(COFF_SYMBOL_ENTRY)) = '_';
                memcpy(data + data_offset + sizeof(COFF_SYMBOL_ENTRY) + 1,
                       names[i], name_len - 1);
            } else {
                memcpy(data + data_offset + sizeof(COFF_SYMBOL_ENTRY), names[i],
                       name_len);
            }
        }
        WRITE_U32(data + data_offset + sizeof(COFF_SYMBOL_ENTRY) + 8,
                  offset + 8);
        offset += (uint32_t)ALIGN_TO(size[i], 8) + 8;
    }
    WRITE_U32(strtable, strtable_offset);
    return data;
}

bool write_coff(const char **names, const Filename_t *files,
                const uint64_t *size, const uint32_t no_symbols,
                const Filename_t outname, bool readonly, bool null_terminate, enum Format format) {
    FILE *out = OPEN(outname, "wb");
    if (out == NULL) {
        ERROR_FMT("Could not create file " F_FORMAT LTR("\n"), outname);
        return false;
    }

    uint8_t header[sizeof(COFF_HEADER) + sizeof(COFF_SECTION_HEADER)];
    uint64_t full_size = 0;
    for (uint64_t i = 0; i < no_symbols; ++i) {
        full_size += ALIGN_TO(size[i], 8);
        if (full_size > INT32_MAX) {
            ERROR_PRINT("COFF-objects only support up to 2 GB of data\n");
            return false;
        }
    }
    write_coff_header(full_size, header, no_symbols, format);
    write_coff_section_header(full_size, no_symbols, readonly,
                              header + sizeof(COFF_HEADER));
    if (fwrite(header, 1, sizeof(header), out) != sizeof(header)) {
        ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
        goto error;
    }
    if (!write_all_files(out, files, size, no_symbols, outname,
                         sizeof(COFF_HEADER) + sizeof(COFF_SECTION_HEADER), null_terminate,
                         8)) {
        goto error;
    }

    uint32_t symbol_table_size;
    uint8_t *symbol_table_buf = write_coff_symbol_table(
        names, size, no_symbols, IS_32BIT(format), &symbol_table_size);
    if (!write_all(out, symbol_table_size, symbol_table_buf)) {
        ERROR_FMT("Writing to " F_FORMAT LTR(" failed\n"), outname);
        goto error;
    }
    fclose(out);
    free(symbol_table_buf);

    return true;
error:
    fclose(out);
    REMOVE(outname);
    return false;
}

char *to_ascii(const CHAR *ptr) {
    size_t len = STRLEN(ptr);
    char *res = malloc(len + 1);
    for (size_t i = 0; i < len; ++i) {
        if (ptr[i] > 127) {
            free(res);
            return NULL;
        }
        res[i] = ptr[i];
    }
    res[len] = '\0';
    return res;
}

char *to_symbol(const CHAR *ptr, bool replace_dot, bool format) {
    size_t len = STRLEN(ptr);
    char *res = malloc(len + 1);
    for (size_t ix = 0; ix < len; ++ix) {
        if (ptr[ix] == LTR('.') && replace_dot) {
            res[ix] = '_';
        } else if ((ptr[ix] == LTR('.') && replace_dot) ||
                   ptr[ix] == LTR('_')) {
            res[ix] = '_';
        } else if (ptr[ix] >= LTR('a') && ptr[ix] <= LTR('z')) {
            res[ix] = (char)('a' + (ptr[ix] - LTR('a')));
        } else if (ptr[ix] >= LTR('A') && ptr[ix] <= LTR('Z')) {
            res[ix] = (char)('A' + (ptr[ix] - LTR('A')));
        } else if (ptr[ix] >= LTR('0') && ptr[ix] <= LTR('9')) {
            res[ix] = (char)('0' + (ptr[ix] - LTR('0')));
        } else if (format && ptr[ix] == LTR('%')) {
            if (ptr[ix + 1] != LTR('f') && ptr[ix + 1] != LTR('d') &&
                ptr[ix + 1] != LTR('n') && ptr[ix + 1] != LTR('e') &&
                ptr[ix + 1] != LTR('x')) {
                return NULL;
            }
            res[ix] = '%';
        } else {
            free(res);
            return NULL;
        }
    }
    res[len] = '\0';
    return res;
}

char *get_symbol(const char *format, const Filename_t filename, uint32_t n) {
    Filename_t f = realpath(filename, NULL);
    Filename_t file = f;
    char *dirs[10];
    char *ext = NULL;
    char *base = NULL;
    uint32_t dir_count = 0;
    while (file != NULL) {
        CHAR *c = STRCHR(file, '/');
#ifdef _WIN32
        CHAR *c2 = STRCHR(file, '\\');
        c = (c2 < c || c == NULL) ? c2 : c;
#endif
        if (c == NULL) {
            break;
        }
        CHAR old = *c;
        if (dir_count == 10) {
            free(dirs[9]);
            memmove(dirs + 1, dirs, 9 * sizeof(char *));
        } else {
            ++dir_count;
        }
        *c = LTR('\0');
        dirs[10 - dir_count] = to_symbol(file, true, false);
        file = c + 1;
        *c = old;
    }
    memmove(dirs, dirs + 10 - dir_count, dir_count * sizeof(char *));
    for (int i = dir_count; i < 10; ++i) {
        dirs[i] = NULL;
    }
    if (file != NULL) {
        uint32_t len = (uint32_t)STRLEN(file);
        for (uint32_t ix = len - 1;; --ix) {
            if (ix == 0) {
                ext = malloc(1);
                *ext = '\0';
                base = to_symbol(file, true, false);
                break;
            } else if (file[ix] == LTR('.')) {
                ext = to_symbol(file + ix + 1, false, false);
                file[ix] = LTR('\0');
                base = to_symbol(file, true, false);
                file[ix] = LTR('.');
                break;
            }
        }
    }
    char *res = NULL;
    char *dest = malloc(10);
    uint32_t cap = 10;
    size_t ix = 0;
    for (size_t i = 0; format[i] != 0; ++i) {
        if (format[i] != '%') {
            if (ix == cap) {
                dest = realloc(dest, cap * 2);
                cap *= 2;
            }
            dest[ix++] = format[i];
        } else {
            ++i;
            size_t len;
            char *b = "";
            if (format[i] == 'n') {
                char buf[32];
                b = buf;
                len = sprintf(buf, "%u", n);
            } else if (format[i] == 'f') {
                if (base == NULL) {
                    free(dest);
                    goto cleanup;
                }
                len = strlen(base);
                b = base;
            } else if (format[i] == 'e' || format[i] == 'x') {
                if (ext == NULL) {
                    free(dest);
                    goto cleanup;
                }
                len = strlen(ext);
                b = ext;
                if (len > 0 && format[i] == 'e') {
                    if (ix == cap) {
                        dest = realloc(dest, cap * 2);
                        cap *= 2;
                    }
                    dest[ix++] = '_';
                }
            } else if (format[i] == 'd') {
                int dir_ix = 0;
                if (format[i + 1] >= '0' && format[i + 1] <= '9') {
                    dir_ix = format[i + 1] - '0';
                    ++i;
                }
                if (dirs[dir_ix] == NULL) {
                    // Missing directory is not an error
                    continue;
                }
                len = strlen(dirs[dir_ix]);
                b = dirs[dir_ix];
            } else {
                len = 0;
            }
            while (ix + len >= cap) {
                dest = realloc(dest, cap * 2);
                cap *= 2;
            }
            memcpy(dest + ix, b, len);
            ix += len;
        }
    }
    if (ix == cap) {
        dest = realloc(dest, ix + 1);
    }
    dest[ix] = '\0';
    res = dest;
cleanup:
    free(f);
    free(ext);
    free(base);
    for (uint32_t i = 0; i < dir_count; ++i) {
        free(dirs[i]);
    }
    return res;
}

const CHAR* HELP_MESSAGE = LTR("usage: embed [options] file[:symbol]...\n")
                           LTR("Embeds a set of input files into an object file.\n")
                           LTR("Options: \n")
                           LTR(" -o --ouput <file>         Write output to <file>.\n")
                           LTR(" -s --symbol-format <fmt>  Use <fmt> to generate symbol names for all input files. \n")
                           LTR("                           <fmt> must generate valid symbol names, and can contain:\n")
                           LTR("                             %f  - Filename without extension\n")
                           LTR("                             %e  - File extension with a leading underscore\n")
                           LTR("                             %x  - File extension without leading underscore\n")
                           LTR("                             %d  - Directory of file. \n")
                           LTR("                             %dN - N-th (0-9) parent directory of file. %d0 is the same as %d\n")
                           LTR("                             %n  - Index of input in arguments\n")
                           LTR(" -f --object-format <obj>  Write the output using format <obj>. \n")
                           LTR("                           Supported formats are: elf32, elf64, elf32-arm, elf64-arm,\n")
                           LTR("                            coff32, coff64, coff32-arm, coff64-arm, macho32, macho64,\n")
                           LTR("                            macho32-arm, macho64-arm\n")
                           LTR(" -w --writable             Write the data to a writable section instead of a readonly section\n")
                           LTR(" -n --null-terminate       Add a null-terminator at the end of each embedded file\n")
                           LTR(" -H --header <header>      Generate a C header file <header> defining all symbols\n")
                           LTR(" -h --help                 Print this help message\n");

#define FLAG_COUNT 7
const CHAR FLAGS[] = {
    LTR('o'), LTR('s'), LTR('f'), LTR('w'), LTR('h'), LTR('H'), LTR('n')
};

const CHAR* LONG_FLAGS[] = {
    LTR("output"), LTR("symbol-format"), LTR("object-format"), LTR("writable"), LTR("help"), LTR("header"), LTR("null-terminate")
};

CHAR check_flag(CHAR* arg, int* i, uint32_t* offset) {
    *offset = 0;
    if (arg[1] == '-') {
        for (int j = 0; j < FLAG_COUNT; ++j) {
            if (STRCMP(arg + 2, LONG_FLAGS[j]) == 0) {
                if (FLAGS[j] != LTR('w') && FLAGS[j] != LTR('h') && FLAGS[j] != LTR('n')) {
                    *i += 1;
                }
                return FLAGS[j];
            }
        }
        return LTR('\0');
    }
    for (int j = 0; j < FLAG_COUNT; ++j) {
        if (arg[1] == FLAGS[j]) {
            if (arg[2] != LTR('\0')) {
                *offset = 2;
            } else if (FLAGS[j] != LTR('w') && FLAGS[j] != LTR('h') && FLAGS[j] != LTR('n'))  {
                *i += 1;
            }
            return FLAGS[j];
        }
    }
    return LTR('\0');
}

int ENTRY(int argc, CHAR **argv) {
#if defined _WIN32 && defined UNICODE && defined _O_U16TEXT
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
#endif
    int status = 1;
    if (argc < 2) {
        ERROR_FMT("" STR_FORMAT LTR(", %zd\n"), HOST_NAME, sizeof(CHAR));
        return 1;
    }

    Filename_t outname = NULL;
    Filename_t header = NULL;
    Filename_t *input_names = malloc(argc * sizeof(Filename_t));
    char **symbol_names = malloc(argc * sizeof(char *));
    uint64_t *input_sizes = malloc(argc * sizeof(uint64_t));
    char *format = NULL;
    enum Format out_format = NONE_FORMAT;
    bool readonly = true;
    bool null_terminate = false;

    uint32_t input_count = 0;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == LTR('-')) {
            uint32_t ix = 0;
            CHAR val = check_flag(argv[i], &i, &ix);
            if (val == LTR('\0')) {
                ERROR_FMT("Unkown flag '" F_FORMAT LTR("'\n"), argv[i]);
                goto cleanup;
            }
            if (i == argc) {
                ERROR_FMT("No value specified for '" F_FORMAT LTR("'\n"),
                        argv[i - 1]);
                goto cleanup;
            }

            if (val == LTR('o')) {
                if (outname != NULL) {
                    ERROR_PRINT("Extra output files specified\n");
                    goto cleanup;
                }
                uint32_t len = 1 + (uint32_t)STRLEN(argv[i] + ix);
                outname = malloc(len * sizeof(CHAR));
                memcpy(outname, argv[i] + ix, len * sizeof(CHAR));
            } else if (val == LTR('s')) {
                if (format != NULL) {
                    ERROR_PRINT("Multiple symbol formats specified\n");
                    goto cleanup;
                }
                format = to_symbol(argv[i] + ix, false, true);
                if (format == NULL) {
                    ERROR_FMT("Invalid symbol format " F_FORMAT LTR("\n"),
                           argv[i] + ix);
                    goto cleanup;
                }
            } else if (val == LTR('f')) {
                if (out_format != NONE_FORMAT) {
                    ERROR_PRINT("Multiple object formats specified\n");
                    goto cleanup;
                }
                char *format = to_ascii(argv[i] + ix);
                if (format != NULL) {
                    for (int i = 0; i < FORMAT_NAME_COUNT; ++i) {
                        if (strcmp(format, format_names[i]) == 0) {
                            out_format = FORMATS[i];
                            break;
                        }
                    }
                    free(format);
                }
                if (out_format == NONE_FORMAT) {
                    ERROR_FMT("Unsupported object format " F_FORMAT LTR("\n"),
                           argv[i] + ix);
                    goto cleanup;
                }
            } else if (val == LTR('w')) {
                readonly = false;
            } else if (val == LTR('H')) {
                if (header != NULL) {
                    ERROR_PRINT("Multiple header files specified\n");
                    goto cleanup;
                }
                header = argv[i] + ix;
            } else if (val == LTR('h')) {
                OUT_FMT("" F_FORMAT, HELP_MESSAGE);
                status = 0;
                goto cleanup;
            } else if (val == LTR('n')) {
                null_terminate = true;
            }
        } else {
            CHAR *sep = STRRCHR(argv[i], ':');
            if (sep != NULL && sep - argv[i] == 1 && ((argv[i][0] >= LTR('A') && argv[i][0] <= 'Z') || (argv[i][0] >= LTR('a') && argv[i][0] <= 'z'))) {
                // Found ':' part of drive letter, skip it
                sep = STRRCHR(argv[i] + 2, ':');
            }
            if (sep != NULL) {
                uint32_t len = (uint32_t)(sep - argv[i]);
                input_names[input_count] = malloc((len + 1) * sizeof(CHAR));
                memcpy(input_names[input_count], argv[i], len * sizeof(CHAR));
                input_names[input_count][len] = LTR('\0');
                *sep = LTR('\0');
                CHAR *symbol_name_root = sep + 1;
                if (*symbol_name_root == LTR('\0')) {
                    ERROR_FMT("Invalid argument " F_FORMAT LTR("\n"), argv[i]);
                    goto cleanup;
                }
                symbol_names[input_count] =
                    to_symbol(symbol_name_root, false, false);
                if (symbol_names[input_count] == NULL) {
                    ERROR_FMT("Invalid symbol name " F_FORMAT LTR("\n"),
                           symbol_name_root);
                    goto cleanup;
                }
            } else {
                uint32_t len = (uint32_t)STRLEN(argv[i]);
                input_names[input_count] = malloc((len + 1) * sizeof(CHAR));
                memcpy(input_names[input_count], argv[i],
                       (len + 1) * sizeof(CHAR));
                symbol_names[input_count] = NULL;
            }
            ++input_count;
        }
    }
    if (input_count == 0) {
        ERROR_PRINT("No input files specified\n");
        goto cleanup;
    }
    if (out_format == NONE_FORMAT) {
        out_format = DEFAULT_FORMAT;
    }
    if (outname == NULL) {
        outname = malloc(10 * sizeof(CHAR));
        if (IS_COFF(out_format)) {
            memcpy(outname, LTR("embed.obj"), 10 * sizeof(CHAR));
        } else {
            memcpy(outname, LTR("embed.o"), 8 * sizeof(CHAR));
        }
    }
    if (format == NULL) {
        format = malloc(5);
        memcpy(format, "%f%e", 5);
    }

    for (uint32_t i = 0; i < input_count; ++i) {
        FILE *f = OPEN(input_names[i], "rb");
        long size;
        if (f == NULL || fseek(f, 0, SEEK_END) != 0) {
            ERROR_FMT("Failed to open input file " F_FORMAT LTR("\n"), input_names[i]);
            if (f != NULL) {
                fclose(f);
            }
            goto cleanup;
        }
        if ((size = ftell(f)) < 0) {
            // This is likely the case on windows. COFF is limited to 4GB
            // anyways...
            ERROR_FMT("File " F_FORMAT LTR(" too large\n"), input_names[i]);
            fclose(f);
            goto cleanup;
        }
        if (null_terminate) {
            ++size;
        }
        input_sizes[i] = size;
        fclose(f);
    }

    for (uint32_t i = 0; i < input_count; ++i) {
        if (symbol_names[i] == NULL) {
            symbol_names[i] = get_symbol(format, input_names[i], i);
            if (symbol_names[i] == NULL) {
                ERROR_FMT("Format gives invalid symbol for " F_FORMAT LTR("\n"),
                       input_names[i]);
                goto cleanup;
            }
        }
    }

    for (uint32_t i = 0; i < input_count; ++i) {
        for (uint32_t j = i + 1; j < input_count; ++j) {
            if (strcmp(symbol_names[i], symbol_names[j]) == 0) {
                ERROR_FMT("Duplicate symbol name " STR_FORMAT LTR("\n"),
                       symbol_names[i]);
                goto cleanup;
            }
        }
    }

    if (IS_COFF(out_format)) {
        write_coff((const char **)symbol_names, input_names, input_sizes,
                   input_count, outname, readonly, null_terminate, out_format);
    } else if (IS_MACHO(out_format)) {
        write_macho(symbol_names, input_names, input_sizes,
                    input_count, outname, readonly, null_terminate, out_format);
    } else {
        write_elf((const char **)symbol_names, input_names, input_sizes,
                  input_count, outname, readonly, null_terminate, out_format);
    }
    if (header != NULL) {
        write_c_header((const char **)symbol_names, input_sizes, input_count,
                       header, readonly);
    }

    status = 0;
cleanup:
    free(outname);
    for (uint32_t i = 0; i < input_count; ++i) {
        free(input_names[i]);
        free(symbol_names[i]);
    }
    free(input_names);
    free(format);
    free(symbol_names);
    free(input_sizes);
    return status;
}
