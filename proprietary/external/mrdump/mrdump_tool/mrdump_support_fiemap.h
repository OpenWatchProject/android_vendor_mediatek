#pragma once

struct fiemap_info {
    __u32 lba;
    __u32 tot;
};

/* Function Prototypes */
unsigned int mrdump_fiemap_total_entries(int fd);
bool mrdump_fiemap_get_entries(int fd, unsigned int blksize, struct fiemap_info *mapinfo, unsigned int rows);
unsigned int mrdump_fiemap_get_lba_of_block(struct fiemap_info *myinfo, unsigned int num, unsigned int block);
int mrdump_fiemap_get_entry_tot(int fd, unsigned int blksize, unsigned int rows);
int mrdump_fiemap_get_entry_lba(int fd, unsigned int blksize, unsigned int rows);
