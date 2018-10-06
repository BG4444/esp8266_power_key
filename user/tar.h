#ifndef TAR_H
#define TAR_H
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "tcp_streamer.h"
#include "strbuf.h"

typedef struct tar_header_
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
                                /* 500 */
} __attribute__((packed)) tar_header;

void vaddr_flash_read512(uint32 base, uint32* buf, uint32 size);

bool find_file_in_tar(const strBuf *name, uint32* base, uint32* size, strBuf *mtime);
void send_item(tcp_streamer* s);
#endif // TAR_H
