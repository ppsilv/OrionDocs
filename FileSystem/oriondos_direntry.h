#ifndef ORIONDOS_DIRENTRY_H
#define ORIONDOS_DIRENTRY_H

#include <stdint.h>

/*
 * Entrada de diretório FAT16, 32 bytes, layout padrao intacto.
 * Os campos "reserved_nt" e os bits 6-7 de "attr" sao reservados
 * no padrao FAT e nunca usados pelo FatFS original -- e onde
 * o oriondos guarda permissao e tipo de arquivo, sem quebrar
 * compatibilidade de leitura em outro sistema.
 */
typedef struct __attribute__((packed)) {
    uint8_t  name[8];        /* 0x00 nome 8.3 */
    uint8_t  ext[3];         /* 0x08 extensao 8.3 */
    uint8_t  attr;           /* 0x0B atributo (RDO/HID/SYS/VOL/DIR/ARC + tipo nos bits 6-7) */
    uint8_t  reserved_nt;    /* 0x0C livre no FAT padrao -- aqui vira permissao (rwx) */
    uint8_t  create_time_ds; /* 0x0D decimos de segundo da criacao */
    uint16_t create_time;    /* 0x0E hora de criacao */
    uint16_t create_date;    /* 0x10 data de criacao */
    uint16_t access_date;    /* 0x12 data do ultimo acesso */
    uint16_t cluster_hi;     /* 0x14 cluster alto (sempre 0 em FAT16) */
    uint16_t write_time;     /* 0x16 hora da ultima escrita */
    uint16_t write_date;     /* 0x18 data da ultima escrita */
    uint16_t cluster_lo;     /* 0x1A cluster inicial */
    uint32_t file_size;      /* 0x1C tamanho em bytes */
} fat_dirent_t;

/* ---- atributos padrao FAT (bits 0-5, ja existentes no formato) ---- */
#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20

/* ---- extensao oriondos: tipo de arquivo, bits 6-7 de "attr" ---- */
#define ATTR_TYPE_MASK      0xC0
#define ATTR_TYPE_SHIFT     6

#define FTYPE_REGULAR   0x0  /* 00 */
#define FTYPE_LINK      0x1  /* 01 */
#define FTYPE_DEVICE    0x2  /* 10 */
#define FTYPE_RESERVED  0x3  /* 11 - reservado p/ uso futuro */

static inline uint8_t fat_get_type(const fat_dirent_t *e) {
    return (e->attr & ATTR_TYPE_MASK) >> ATTR_TYPE_SHIFT;
}

static inline void fat_set_type(fat_dirent_t *e, uint8_t type) {
    e->attr = (uint8_t)((e->attr & ~ATTR_TYPE_MASK) |
                         ((type << ATTR_TYPE_SHIFT) & ATTR_TYPE_MASK));
}

/* ---- extensao oriondos: permissao + payload de tipo, byte 0x0C ---- */
/*
 * bits 0-2: rwx (mono-usuario, sem owner/group/other)
 * bits 3-7: payload especifico do tipo:
 *   - FTYPE_DEVICE: indice na tabela de devices do kernel (0-31)
 *   - FTYPE_REGULAR / FTYPE_LINK: livre por enquanto
 */
#define PERM_MASK       0x07
#define PERM_R          0x04
#define PERM_W          0x02
#define PERM_X          0x01

#define DEV_INDEX_MASK  0xF8
#define DEV_INDEX_SHIFT 3

static inline uint8_t fat_get_perm(const fat_dirent_t *e) {
    return e->reserved_nt & PERM_MASK;
}

static inline void fat_set_perm(fat_dirent_t *e, uint8_t perm) {
    e->reserved_nt = (uint8_t)((e->reserved_nt & ~PERM_MASK) | (perm & PERM_MASK));
}

static inline uint8_t fat_get_dev_index(const fat_dirent_t *e) {
    return (e->reserved_nt & DEV_INDEX_MASK) >> DEV_INDEX_SHIFT;
}

static inline void fat_set_dev_index(fat_dirent_t *e, uint8_t idx) {
    e->reserved_nt = (uint8_t)((e->reserved_nt & ~DEV_INDEX_MASK) |
                                ((idx << DEV_INDEX_SHIFT) & DEV_INDEX_MASK));
}

#endif /* ORIONDOS_DIRENTRY_H */
