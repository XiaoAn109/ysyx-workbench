#include <cpu/cpu.h>
#include <elf.h>

// #define ELF_HEADER_SHOW
// #define ELF_SECTION_HEADER_SHOW


FILE *elf_fp = NULL;
Elf64_Ehdr elf_header;
Elf64_Shdr *section_headers = NULL;
char *string_table = NULL;
Elf64_Sym *sym_entries = NULL;
char *dynstr_string_table = NULL;
char *strtab_string_table = NULL;
uint64_t entry_num;

void get_sh_type(int sh_type, char *section_header_name);
void get_sh_flags(uint32_t sh_flags, char *section_header_name);
void get_st_binding(uint32_t st_info, char *symbol_binding);
void get_st_type(uint32_t st_info, char *symbol_type);
void get_st_shndx(uint32_t st_shndx, char *Ndx);
void show_symbol_table(int sym_ind, char *str_string_table);

void init_elf(const char* elf_file) {
    if (elf_file != NULL) {
        FILE* fp = fopen(elf_file, "r");
        Assert(fp, "Can not open '%s'", elf_file);
        elf_fp = fp;
    }
    else {
        Log("No ELF file given.");
        return;
    }
    Log("Read ELF Info from %s", elf_file);
    //ELF header
    assert(fread(&elf_header, sizeof(Elf64_Ehdr), 1, elf_fp) == 1);
    Assert(!strncmp((const char *)elf_header.e_ident, ELFMAG, 4), "%s is not an ELF file\n", elf_file);
    int ostype = (elf_header.e_ident[EI_CLASS] == ELFCLASS64) ? 64 : 32;
    Log("Magic Check Passed, this is a %dbit elf file.", ostype);
#ifdef ELF_HEADER_SHOW
    Print(ANSI_FG_GREEN, "[ELF Header Information:]");
    printf("Entry point address:\t\t\t0x%016lx\n", elf_header.e_entry);
    printf("Start of program headers:\t\t%ld (bytes into file)\n", elf_header.e_phoff);
    printf("Start of section headers:\t\t%ld (bytes into file)\n", elf_header.e_shoff);
    printf("Flags:\t\t\t\t\t0x%x\n", elf_header.e_flags);
    printf("Size of this header:\t\t\t%d (bytes)\n", elf_header.e_ehsize);
    printf("Size of program headers:\t\t%d (bytes)\n", elf_header.e_phentsize);
    printf("Number of program headers:\t\t%d\n", elf_header.e_phnum);
    printf("Size of section headers:\t\t%d (bytes)\n", elf_header.e_shentsize);
    printf("Number of section headers:\t\t%d\n", elf_header.e_shnum);
    printf("Section header string table index:\t%d\n", elf_header.e_shstrndx);
#endif
    //ELF section headers
    section_headers = (Elf64_Shdr *)malloc(sizeof(Elf64_Shdr)*elf_header.e_shnum);
    // Elf64_Shdr section_headers[elf_header.e_shnum];
    fseek(elf_fp, elf_header.e_shoff, SEEK_SET);
    //把每个sections的内容读取到数组中
    assert(fread(section_headers, sizeof(Elf64_Shdr), elf_header.e_shnum, elf_fp) == elf_header.e_shnum);
    int str_tab_ind = elf_header.e_shstrndx;//获取字符串表在段表中的索引elf_header.e_shstrndx，用来读取段名
    fseek(elf_fp, section_headers[str_tab_ind].sh_offset, SEEK_SET);//将指针移动到字符串表
    //ELF string table
    string_table = (char *)malloc(section_headers[str_tab_ind].sh_size);//构造字符数组用来存储字符串表里的字符
    assert(fread(string_table, 1, section_headers[str_tab_ind].sh_size, elf_fp) == section_headers[str_tab_ind].sh_size);//将字符串表里面的字符全部读出来
#ifdef ELF_SECTION_HEADER_SHOW
    Print(ANSI_FG_GREEN, "[ELF Section Headers Information:]");
    printf("[Nr] Name\t\t     Type\t      Addr\t\tOffset\tSize\tES\tFlg\tLk\tInf\tAl\n");
    //遍历section_headers段表里的每个section,输出相应的信息
    for (int i = 0; i < elf_header.e_shnum; i++) {
        printf("[%2d] ", i);
        printf("%-24s", &string_table[section_headers[i].sh_name]);
        char sh_type[20];
        memset(sh_type, 0, sizeof(sh_type));
        get_sh_type(section_headers[i].sh_type, sh_type);
        printf("%-16s ", sh_type);
        printf("%016lx\t", section_headers[i].sh_addr);
        printf("%06lx\t", section_headers[i].sh_offset);
        printf("%06lx\t", section_headers[i].sh_size);
        printf("%02lx\t", section_headers[i].sh_entsize);
        char sh_flags[20];
        memset(sh_flags, 0, sizeof(sh_flags));
        get_sh_flags(section_headers[i].sh_flags, sh_flags);
        printf(" %s\t", sh_flags);
        printf(" %d\t", section_headers[i].sh_link);
        printf("  %d\t", section_headers[i].sh_info);
        printf(" %ld", section_headers[i].sh_addralign);
        printf("\n");
    }
#endif
    //ELF symbol table

    int dynsym_ind = -1;//默认.dynsym符号表索引为-1
    int symtab_ind = -1;//默认.symtab符号表索引为-1
    int dynstr_ind = -1;//默认.dynstr字符串表索引为-1
    int strtab_ind = -1;//默认.strtab字符串索引为-1

    //遍历段表section_headers获取符号表.dynsym;.symtab;.dynstr;.strtab四张表在段表中的索引
    for (int i = 0; i < elf_header.e_shnum; i++) {
        if (section_headers[i].sh_type == SHT_DYNSYM)//是.dynsym符号表
            dynsym_ind = i;
        else if (section_headers[i].sh_type == SHT_SYMTAB)//是.symtab符号表
            symtab_ind = i;
        if (strcmp(&string_table[section_headers[i].sh_name], ".strtab") == 0)//是.strtab字符串表
            strtab_ind = i;
        else if (strcmp(&string_table[section_headers[i].sh_name], ".dynstr") == 0)//是.dynstr字符串表
            dynstr_ind = i;
    }



    //如果.dynsym段存在,且.dynstr存在
    if ((dynsym_ind != -1) && (dynstr_ind != -1)) {
        //符号表大小section_headers[dynsym_ind].sh_size每个entry大小section_headers[dynsym_ind].sh_entsize
        // 计算entry数目entry_num
        entry_num = section_headers[dynsym_ind].sh_size / section_headers[dynsym_ind].sh_entsize;
        fseek(elf_fp, section_headers[dynstr_ind].sh_offset, SEEK_SET);//将指针移动到.dynstr字符串表对应的偏移地址
        //开辟堆内存用来存储字符串表
        dynstr_string_table = (char *)malloc(section_headers[dynstr_ind].sh_size);
        //将数据读到字符串表里
        assert(fread(dynstr_string_table, 1, section_headers[dynstr_ind].sh_size, elf_fp) == section_headers[dynstr_ind].sh_size);
        fseek(elf_fp, section_headers[symtab_ind].sh_offset, SEEK_SET);//将指针移动到符号表对应的偏移地址
        sym_entries = (Elf64_Sym *)malloc(sizeof(Elf64_Sym)*entry_num);//开辟堆内存用来存储符号表中所有entry
        assert(fread(sym_entries, sizeof(Elf64_Sym), entry_num, elf_fp) == entry_num);//读符号表
#ifdef CONFIG_FTRACE
        printf("Symbol table '.dynsym' contains %ld entries\n", entry_num);
        show_symbol_table(dynsym_ind, dynstr_string_table);
    } else {
        printf("No Dynamic linker symbol table!\n");
    }
    printf("\n");
#else
    } else {

    }
#endif
    //如果.symtab段存在，且.strtab存在
    if ((symtab_ind != -1) && (strtab_ind != -1)) {
        entry_num = section_headers[symtab_ind].sh_size / section_headers[symtab_ind].sh_entsize;
        fseek(elf_fp, section_headers[strtab_ind].sh_offset, SEEK_SET);
        strtab_string_table = (char *)malloc(section_headers[strtab_ind].sh_size);
        assert(fread(strtab_string_table, 1, section_headers[strtab_ind].sh_size, elf_fp) == section_headers[strtab_ind].sh_size);
        fseek(elf_fp, section_headers[symtab_ind].sh_offset, SEEK_SET);//将指针移动到符号表对应的偏移地址
        sym_entries = (Elf64_Sym *)malloc(sizeof(Elf64_Sym)*entry_num);//开辟堆内存用来存储符号表中所有entry
        assert(fread(sym_entries, sizeof(Elf64_Sym), entry_num, elf_fp) == entry_num);//读符号表
#ifdef CONFIG_FTRACE
        printf("Symbol table '.symtab' contains %ld entries\n", entry_num);
        show_symbol_table(symtab_ind, strtab_string_table);
    } else {
        printf("No symbol table!\n");
    }
#else
    } else {

    }
#endif
}
void free_close_elf() {
    if(sym_entries) free(strtab_string_table);
    if(dynstr_string_table) free(dynstr_string_table);
    if(string_table) free(string_table);
    if(section_headers) free(section_headers);
    if(elf_fp) fclose(elf_fp);
}

// char *func_name(vaddr_t pc) {
char *func_name(vaddr_t pc) {
    char *func = "???";
    char *start = "_start";
    for (int i = 0; i < entry_num; i++) {
        char symbol_type[20];
        memset(symbol_type, 0, sizeof(symbol_type));
        get_st_type(sym_entries[i].st_info, symbol_type);
        if(strcmp(symbol_type, "FUNC") == 0) {
            if(pc >= elf_header.e_entry && pc < elf_header.e_entry + 0x10) {
                func = start;
            }
            if(pc >= sym_entries[i].st_value && pc < (sym_entries[i].st_value + sym_entries[i].st_size)) {
                func = &strtab_string_table[sym_entries[i].st_name];
            }
        }
    }
    return func;
}

void get_sh_type(int sh_type, char *section_header_name) {
    switch (sh_type) {
        case SHT_NULL:
            strcpy(section_header_name, "NULL");
            break;
        case SHT_PROGBITS:
            strcpy(section_header_name, "PROGBITS");
            break;
        case SHT_SYMTAB:
            strcpy(section_header_name, "SYMTAB");
            break;
        case SHT_STRTAB:
            strcpy(section_header_name, "STRTAB");
            break;
        case SHT_RELA:
            strcpy(section_header_name, "RELA");
            break;
        case SHT_HASH:
            strcpy(section_header_name, "HASH");
            break;
        case SHT_DYNAMIC:
            strcpy(section_header_name, "DYNAMIC");
            break;
        case SHT_NOTE:
            strcpy(section_header_name, "NOTE");
            break;
        case SHT_NOBITS:
            strcpy(section_header_name, "NOBITS");
            break;
        case SHT_REL:
            strcpy(section_header_name, "REL");
            break;
        case SHT_SHLIB:
            strcpy(section_header_name, "SHLIB");
            break;
        case SHT_DYNSYM:
            strcpy(section_header_name, "DYNSYM");
            break;
        case SHT_INIT_ARRAY:
            strcpy(section_header_name, "INIT_ARRAY");
            break;
        case SHT_FINI_ARRAY:
            strcpy(section_header_name, "FINI_ARRAY");
            break;
        case SHT_PREINIT_ARRAY:
            strcpy(section_header_name, "PREINIT_ARRAY");
            break;
        case SHT_GROUP:
            strcpy(section_header_name, "GROUP");
            break;
        case SHT_SYMTAB_SHNDX:
            strcpy(section_header_name, "SYMTAB_SHNDX");
            break;
        case SHT_NUM:
            strcpy(section_header_name, "NUM");
            break;
        case SHT_GNU_HASH:
            strcpy(section_header_name, "GNU_HASH");
            break;
        case SHT_GNU_versym:
            strcpy(section_header_name, "VERSYM");
            break;
        case SHT_GNU_verneed:
            strcpy(section_header_name, "VERNEED");
            break;
        case SHT_MIPS_GPTAB:
            strcpy(section_header_name, "RISCV_ATTRIBUTES");
            break;
        default:
            strcpy(section_header_name, "UNKNOWNTYPE");
    }
}
void get_sh_flags(uint32_t sh_flags, char *section_header_name) {
    if ((sh_flags & SHF_WRITE) >> 0)
        strcat(section_header_name, "W");
    if ((sh_flags & SHF_ALLOC) >> 1)
        strcat(section_header_name, "A");
    if ((sh_flags & SHF_EXECINSTR) >> 2)
        strcat(section_header_name, "X");
    if ((sh_flags & SHF_MERGE) >> 4)
        strcat(section_header_name, "M");
    if ((sh_flags & SHF_STRINGS) >> 5)
        strcat(section_header_name, "S");
    if ((sh_flags & SHF_INFO_LINK) >> 6)
        strcat(section_header_name, "I");
    if ((sh_flags & SHF_LINK_ORDER) >> 7)
        strcat(section_header_name, "L");
    if ((sh_flags & SHF_OS_NONCONFORMING) >> 8)
        strcat(section_header_name, "O");
    if ((sh_flags & SHF_GROUP) >> 9)
        strcat(section_header_name, "G");
    if ((sh_flags & SHF_TLS) >> 10)
        strcat(section_header_name, "T");
    if ((sh_flags & SHF_COMPRESSED) >> 11)
        strcat(section_header_name, "C");
    //特殊flag因为对应的位和上面的flag对应的位不重叠，所以可以单独处理
    switch (sh_flags) {
        case SHF_MASKOS:
            strcat(section_header_name, "o");
            break;
        case SHF_MASKPROC:
            strcat(section_header_name,"p");
            break;
        case SHF_EXCLUDE:
            strcat(section_header_name, "E");
            break;
    }
}
void get_st_type(uint32_t st_info, char *symbol_type) {
    uint8_t st_type = st_info & 0x0000000f;//低4位表示符号类型
    switch (st_type) {
        case STT_NOTYPE:
            strcpy(symbol_type, "NOTYPE");
            break;
        case STT_OBJECT:
            strcpy(symbol_type, "OBJECT");
            break;
        case STT_FUNC:
            strcpy(symbol_type,"FUNC");
            break;
        case STT_SECTION:
            strcpy(symbol_type, "SECTION");
            break;
        case STT_FILE:
            strcpy(symbol_type,"FILE");
            break;
        case STT_COMMON:
            strcpy(symbol_type,"COMMON");
            break;
        case STT_TLS:
            strcpy(symbol_type, "TLS");
            break;
        case STT_NUM:
            strcpy(symbol_type,"NUM");
            break;
        case STT_LOOS:
            strcpy(symbol_type,"LOOS");
            break;
        case STT_HIOS:
            strcpy(symbol_type,"HIOS");
            break;
        case STT_LOPROC:
            strcpy(symbol_type,"LOPROC");
            break;
        case STT_HIPROC:
            strcpy(symbol_type,"HIPROC");
            break;
        default:
            strcpy(symbol_type,"UNKNOWTYPE");
    }
}
//获取符号表entry对应的st_info段，用来计算符号类型和绑定信息
void get_st_binding(uint32_t st_info, char *symbol_binding) {
    uint8_t st_binding = (st_info & (~0x0000000f)) >> 4;//高28位表示符号绑定信息
    switch (st_binding) {
        case STB_LOCAL:
            strcpy(symbol_binding,"LOCAL");
            break;
        case STB_GLOBAL:
            strcpy(symbol_binding,"GLOBAL");
            break;
        case STB_WEAK:
            strcpy(symbol_binding, "WEAK");
            break;
        case STB_NUM:
            strcpy(symbol_binding,"NUM");
            break;
        case STB_LOOS:
            strcpy(symbol_binding,"LOOS");
            break;
        case STB_HIOS:
            strcpy(symbol_binding, "HIOS");
            break;
        case STB_LOPROC:
            strcpy(symbol_binding, "LOPROC");
            break;
        case STB_HIPROC:
            strcpy(symbol_binding, "HIPROC");
            break;
        default:
            strcpy(symbol_binding,"UNKNOWNBIND");
    }
}
//获取符号所在的段在段表的索引，并对特殊符号进行特殊处理
void get_st_shndx(uint32_t st_shndx, char *Ndx) {
    switch (st_shndx) {
        case SHN_UNDEF:
            strcpy(Ndx, "UNDEF");break;
        case SHN_COMMON:
            strcpy(Ndx, "COMMON");break;
        case SHN_ABS:
            strcpy(Ndx, "ABS");break;
        default:
            sprintf(Ndx, "%d", st_shndx);
    }
}
//输出符号表信息，输入符号表在段表中的索引sym_ind，符号表entry数目entry_num，符号表对应的字符串表string_table
void show_symbol_table(int sym_ind, char *str_string_table) {

    printf("Num:\tValue\t\t\tSize\tType\tBind\tVis\t"
            "Ndx\tName\n");
    //遍历符号表里的每个entry，并且输出entry的信息
    for (int i = 0; i < entry_num; i++) {
        printf("%3d:\t", i);
        printf("%016lx\t", sym_entries[i].st_value);
        printf("%4ld\t", sym_entries[i].st_size);
        char symbol_type[20];
        memset(symbol_type, 0, sizeof(symbol_type));
        get_st_type(sym_entries[i].st_info, symbol_type);
        char symbol_binding[20];
        memset(symbol_binding, 0, sizeof(symbol_binding));
        get_st_binding(sym_entries[i].st_info, symbol_binding);
        printf("%s\t", symbol_type);
        printf("%s\t", symbol_binding);
        printf("DEFAULT\t");
        char Ndx[10];
        memset(Ndx, 0, sizeof(Ndx));
        get_st_shndx(sym_entries[i].st_shndx, Ndx);
        printf("%4s\t", Ndx);
        //根据entry的st_name属性在符号表对应的字符串表表里找到entry的name
        if(strcmp(symbol_type, "SECTION") == 0) {
            printf("%-24s", &string_table[section_headers[i].sh_name]);
        }
        else printf("%s", &str_string_table[sym_entries[i].st_name]);
        printf("\n");
    }
}
