#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/fs/ext2/ext2.h>
#include <kernel/fs/vfs.h>
#include <lib/output.h>
#include <lib/vesa.h>
#include <lib/bmp.h>

uint32_t bmpGetPixel(uint32_t x, uint32_t y, bmpImage_t bmpImage) {
    y = bmpImage.hdr.biHeight - 1 - y;
    return bmpImage.data[x + (y * vesa::pitch / sizeof(uint32_t))];
}

bmpImage_t drawBMP(const char *path) { 
    bmpFileHdr_t bmpFileHdr; 
    
    int file = vfs::open(path, 0, 0);
    vfs::read(file, (char*)&bmpFileHdr, sizeof(bmpFileHdr));

    kprintDS("[KDEBUG]", "height %d", bmpFileHdr.biHeight);
    kprintDS("[KDEBUG]", "width %d", bmpFileHdr.biWidth);
    kprintDS("[KDEBUG]", "bpp %d", bmpFileHdr.biBitCount);
    kprintDS("[KDEBUG]", "offset %d", bmpFileHdr.bfOffset);
    kprintDS("[KDEBUG]", "size %d", bmpFileHdr.bfSize);

    uint32_t *data = (uint32_t*)(pmm::alloc(bmpFileHdr.bfSize / 0x1000) + HIGH_VMA);

    bmpImage_t bmpImage = { bmpFileHdr, data };

    vfs::read(file, (char*)data, bmpFileHdr.bfSize);

    for(uint32_t i = 0; i < bmpFileHdr.biHeight; i++) {
        for(uint32_t j = 0; j < bmpFileHdr.biWidth; j++)
            vesa::setPixel(j, i, bmpGetPixel(j, i, bmpImage)); 
    }

    return bmpImage;
}
