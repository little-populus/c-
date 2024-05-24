#include <inode_info.hpp>
void print_inode_info(const ext2_inode &inode)
{
    std::cout << "\n\n\n";
    std::cout << "Inode Information:" << std::endl;
    std::cout << "------------------" << std::endl;

    std::cout << "File Mode: " << std::oct << inode.i_mode << std::dec << std::endl;
    std::cout << "Owner UID: " << inode.i_uid << std::endl;
    std::cout << "Size: " << inode.i_size << " bytes" << std::endl;

    // Convert timestamps to human-readable format
    std::time_t atime = inode.i_atime;
    std::time_t ctime = inode.i_ctime;
    std::time_t mtime = inode.i_mtime;
    std::time_t dtime = inode.i_dtime;

    std::cout << "Access Time: " << std::asctime(std::localtime(&atime));
    std::cout << "Creation Time: " << std::asctime(std::localtime(&ctime));
    std::cout << "Modification Time: " << std::asctime(std::localtime(&mtime));
    std::cout << "Deletion Time: " << std::asctime(std::localtime(&dtime));

    std::cout << "Group GID: " << inode.i_gid << std::endl;
    std::cout << "Links Count: " << inode.i_links_count << std::endl;
    std::cout << "Blocks Count: " << inode.i_blocks << std::endl;
    std::cout << "File Flags: " << std::hex << inode.i_flags << std::dec << std::endl;

    std::cout << "Block Pointers: ";
    for (int i = 0; i < 15; ++i)
    {
        std::cout << inode.i_block[i];
        if (i < 14)
            std::cout << ", ";
    }
    std::cout << std::endl;

    std::cout << "File ACL: " << inode.i_file_acl << std::endl;
    std::cout << "Fragment Address: " << inode.i_faddr << std::endl;
    std::cout << "\n\n\n";
}