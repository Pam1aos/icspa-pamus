#include "common.h"
#include "memory.h"
#include "string.h"
#include <elf.h>

#ifdef HAS_DEVICE_IDE
#define ELF_OFFSET_IN_DISK 0
#endif

#define STACK_SIZE (1 << 20)

#define PMEM_SIZE       0x8000000u
#define USER_MEM_BASE   0x1000000u

void ide_read(uint8_t *, uint32_t, uint32_t);
void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader()
{
    Elf32_Ehdr *elf;
    Elf32_Phdr *ph, *eph;
    uint8_t *write;

#ifdef HAS_DEVICE_IDE
    uint8_t buf[4096];
    ide_read(buf, ELF_OFFSET_IN_DISK, 4096);
    elf = (void *)buf;
    Log("ELF loading from hard disk.");
#else
    elf = (void *)0x0;
    Log("ELF loading from ram disk.");
#endif

    if (elf->e_ident[0] != 0x7f || elf->e_ident[1] != 'E' ||
        elf->e_ident[2] != 'L'  || elf->e_ident[3] != 'F')
    {
        Log("FATAL: bad ELF magic at elf=%p: %02x %02x %02x %02x",
            elf, elf->e_ident[0], elf->e_ident[1], elf->e_ident[2], elf->e_ident[3]);
        assert(0 && "elf header looks corrupted, check ram-disk load path / linear addr 0 mapping");
    }

    ph = (void *)elf + elf->e_phoff;
    eph = ph + elf->e_phnum;
    for (; ph < eph; ph++)
    {
        if (ph->p_type == PT_LOAD)
        {
            assert(ph->p_memsz >= ph->p_filesz);
            assert(ph->p_memsz < 0x1000000u &&
                   "p_memsz looks insanely large, elf header/phdr likely corrupted");

#ifndef IA32_PAGE
            write = (void *)ph->p_vaddr;
#else
            uint32_t phys = mm_malloc(ph->p_vaddr, ph->p_memsz);

            if (phys < USER_MEM_BASE || phys + ph->p_memsz > PMEM_SIZE)
            {
                Log("FATAL: mm_malloc returned out-of-range phys=0x%x "
                    "(valid range [0x%x, 0x%x)), p_vaddr=0x%x p_memsz=0x%x",
                    phys, USER_MEM_BASE, PMEM_SIZE, ph->p_vaddr, ph->p_memsz);
                assert(0 && "mm_malloc returned invalid physical address");
            }
            write = (void *)(phys + KOFFSET);
#endif

#ifndef HAS_DEVICE_IDE
            memcpy((void *)write, (void *)elf + ph->p_offset, ph->p_filesz);
#else
            ide_read((void *)write, ph->p_offset, ph->p_filesz);
#endif
            if (ph->p_memsz > ph->p_filesz)
            {
                memset((void *)(write + ph->p_filesz), 0,
                       ph->p_memsz - ph->p_filesz);
            }

#ifdef IA32_PAGE
            extern uint32_t brk;
            uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
            if (brk < new_brk)
            {
                brk = new_brk;
            }
#endif
        }
    }

    volatile uint32_t entry = elf->e_entry;

#ifdef IA32_PAGE
    mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);
#ifdef HAS_DEVICE_VGA
    create_video_mapping();
#endif
    write_cr3(get_ucr3());
#endif
    return entry;
}