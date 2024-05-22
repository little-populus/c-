#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
char buff[512];
struct partition {
    uint8_t drive;
    uint8_t head;
    uint8_t sector;
    uint8_t cylinder;
    uint8_t sys_type;
    uint8_t end_head;
    uint8_t end_sector;
    uint8_t end_cylinder;
    uint32_t start_sector;
    uint32_t num_sectors;
};
int main()
{
    int fd = open("mydisk", O_RDONLY);
    read(fd, buff, 512);
    partition* p = (partition*)&buff[0x1be];
    std::cout << "p->start_sector: " << p->start_sector << std::endl;
    std::cout << "p->num_sectors: " << p->num_sectors << std::endl;
    close(fd);
    return 0;
}