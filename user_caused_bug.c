#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <err.h>

#define UPIO		0xAF
#define CMD_CREATE 	_IO(UPIO,	0x00)
#define CMD_OPEN 	_IO(UPIO,	0x01)
#define CMD_VAL  	_IOW(UPIO,	0x02, int)
#define CMD_MMAPSIZE	_IO(UPIO,	0x03)
#define CMD_BUG 	_IO(UPIO,	0x04)

struct child_struct {
	uint32_t id;
	int32_t value;
};

void print_child(struct child_struct *c)
{
	printf("child_struct: id=%u, value=%d\n", c->id, c->value);
}

int cmd_create_ioctl(int fd)
{
	int r;
	r = ioctl(fd, CMD_CREATE);
	if (r < 0) {
		perror("ioctl");
		return -1;
	}

	return r;
}

int cmd_open_ioctl(int p_fd, long mmap_size, struct child_struct **c)
{
	int r, c_fd;
	r = ioctl(p_fd, CMD_OPEN);
	if (r < 0) {
		perror("ioctl");
		return -1;
	}

	c_fd = r;
	*c = mmap(NULL, mmap_size, PROT_READ, MAP_SHARED, c_fd, 0);

	if (*c == MAP_FAILED) {
		perror("mmap");
		return -1;
	}

	return 0;
}

int cmd_val_ioctl(int fd, int32_t value)
{
	int r;
	r = ioctl(fd, CMD_VAL, value);
	if (r < 0) {
		perror("ioctl");
		return -1;
	}

	return r;
}

long cmd_mmap_ioctl(int fd)
{
	long mmap_size;
	mmap_size = ioctl(fd, CMD_MMAPSIZE);
	if (mmap_size < 0) {
		perror("ioctl");
		return -1;
	}

	return mmap_size;
}

int main(void)
{
	int r;
	int fd;
	long mmap_size;
	struct child_struct *c = NULL;

	printf("[+] Open /dev/up\n");
	fd = open("/dev/up", O_RDWR|O_NONBLOCK);

	if (fd < 0) {
		perror("open");
		return 1;
	}

	printf("[+] Create struct_child at LKM\n");
	r = cmd_create_ioctl(fd);
	if (r < 0)
		return 2;

	printf("[+] Get mmap_size\n");
	mmap_size = cmd_mmap_ioctl(fd);
	if (mmap_size < 0)
		return 3;

	printf("[+] Create anonymous inode\n");
	printf("[+] Map child_struct\n");
	r = cmd_open_ioctl(fd, mmap_size, &c);
	if (r < 0)
		return 4;

	print_child(c);
	
	r = ioctl(fd, CMD_BUG);

	return 0;
}
