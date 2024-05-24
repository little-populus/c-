#include <cstring>
#include <ext2fs/ext2_fs.h>
#include <fcntl.h>
#include <inode_info.hpp>
#include <iostream>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void print_bitmap(const unsigned char *bitmap, size_t size)
{
    int free = 0;
    for (size_t i = 0; i < size; ++i)
    {
        for (int bit = 0; bit < 8; ++bit)
        {
            if (bitmap[i] & (1 << bit))
            {
                printf("█"); // 已使用
            }
            else
            {
                printf("░"); // 未使用
                free++;      // 统计空闲块数
            }
        }
        // 每字节打印8个位后换行（根据需要调整换行逻辑）
        if ((i + 1) % 8 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
    printf("Total free blocks: %d\n", free);
}

void read_and_print_bitmaps(int fd, const struct ext2_super_block &super)
{
    size_t block_size;
    size_t bitmap_size;
    unsigned char *block_bitmap;
    unsigned char *inode_bitmap;

    // 计算块大小
    block_size = 1024 << super.s_log_block_size;
    bitmap_size = 1472 / 8;

    // 分配内存
    block_bitmap = (unsigned char *)malloc(bitmap_size);
    inode_bitmap = (unsigned char *)malloc(bitmap_size);

    if (!block_bitmap || !inode_bitmap)
    {
        perror("Failed to allocate memory for bitmaps");
        exit(1);
    }

    // 计算块组数量
    int group_count =
        (super.s_blocks_count - super.s_first_data_block + super.s_blocks_per_group - 1) / super.s_blocks_per_group;

    // 分配内存给块组描述符表
    ext2_group_desc *gdt = (ext2_group_desc *)malloc(group_count * sizeof(ext2_group_desc));
    if (!gdt)
    {
        perror("Failed to allocate memory for group descriptor table");
        exit(1);
    }

    // 读取块组描述符表
    lseek(fd, (super.s_first_data_block + 1) * block_size, SEEK_SET);
    read(fd, gdt, group_count * sizeof(ext2_group_desc));

    for (int i = 0; i < group_count; ++i)
    {
        std::cout << "Group " << i << ":\n";
        std::cout << "  Block bitmap block: " << gdt[i].bg_block_bitmap << "\n";
        std::cout << "  Inode bitmap block: " << gdt[i].bg_inode_bitmap << "\n";
        std::cout << "  Free blocks count: " << gdt[i].bg_free_blocks_count << "\n";
        std::cout << "  Free inodes count: " << gdt[i].bg_free_inodes_count << "\n";

        // 读取块位图
        lseek(fd, gdt[i].bg_block_bitmap * block_size, SEEK_SET);
        read(fd, block_bitmap, bitmap_size);

        // 读取索引节点位图
        lseek(fd, gdt[i].bg_inode_bitmap * block_size, SEEK_SET);
        read(fd, inode_bitmap, bitmap_size);

        // 打印块位图
        printf("Block Bitmap for Group %d:\n", i);
        print_bitmap(block_bitmap, bitmap_size);

        // 打印索引节点位图
        printf("Inode Bitmap for Group %d:\n", i);
        print_bitmap(inode_bitmap, bitmap_size);
    }

    // 读取根目录的索引节点信息
    lseek(fd, gdt[0].bg_inode_table * block_size + 2 * sizeof(ext2_inode), SEEK_SET);
    ext2_inode *inode = (ext2_inode *)malloc(sizeof(ext2_inode));
    read(fd, inode, sizeof(ext2_inode));
    print_inode_info(*inode);

    auto den_blk = inode->i_block[0];
    char *name = (char *)malloc(256);
    lseek(fd, den_blk * block_size, SEEK_SET);
    ext2_dir_entry_2 *start = (ext2_dir_entry_2 *)malloc(block_size);
    ext2_dir_entry_2 *dir_entry = start;
    read(fd, dir_entry, block_size);
    std::cout << "\n\n\n";
    printf("Root directory entry:\n");

    while (dir_entry->inode != 0)
    {
        printf("inode: %u\n", dir_entry->inode);
        printf("rec_len: %u\n", dir_entry->rec_len);
        printf("name_len: %u\n", dir_entry->name_len);
        std::memcpy(name, dir_entry->name, dir_entry->name_len);
        name[dir_entry->name_len] = '\0';
        printf("name: %s\n", name);
        dir_entry = (ext2_dir_entry_2 *)((char *)dir_entry + dir_entry->rec_len);
    }
    std::cout << "\n\n\n";

    // 释放内存
    free(name);
    free(dir_entry);
    free(inode);
    free(block_bitmap);
    free(inode_bitmap);
    free(gdt);
}

int main()
{
    int fd;
    struct ext2_super_block super;
    fd = open("mydisk", O_RDONLY);
    if (fd < 0)
    {
        perror("Failed to open the virtual disk");
        return 1;
    }

    // 读取超级块
    if (lseek(fd, 1024, SEEK_SET) < 0)
    {
        perror("Failed to seek to superblock");
        close(fd);
        return 1;
    }

    if (read(fd, &super, sizeof(super)) != sizeof(super))
    {
        perror("Failed to read superblock");
        close(fd);
        return 1;
    }

    read_and_print_bitmaps(fd, super);
    close(fd);

    // 打印超级块信息
    printf("Inodes count: %u\n", super.s_inodes_count);
    printf("Blocks count: %u\n", super.s_blocks_count);
    printf("Reserved blocks count: %u\n", super.s_r_blocks_count);
    printf("Free blocks count: %u\n", super.s_free_blocks_count);
    printf("Free inodes count: %u\n", super.s_free_inodes_count);
    printf("First data block: %u\n", super.s_first_data_block);
    printf("Block size: %u\n", 1024 << super.s_log_block_size);
    printf("Blocks per group: %u\n", super.s_blocks_per_group);
    printf("Inodes per group: %u\n", super.s_inodes_per_group);
    printf("Creator OS: %u\n", super.s_creator_os);
    printf("Revision level: %u\n", super.s_rev_level);
    printf("Volume name: %.16s\n", super.s_volume_name);
    printf("Last mount time: %u\n", super.s_mtime);
    printf("Last write time: %u\n", super.s_wtime);

    return 0;
}
