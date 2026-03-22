// Coded by FatherDivine in c99 in a centOS 6.7 environment.
// Coded to help work on my C manual hand-coding skillz0rs.
// In the age of AI, we must make that distinction.
// Version 2: Added mounts via fstab. More to come when 
// I'm bored or not busy with real world things.

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <linux/types.h> // Defines the 'us' (unsigned short) and other types
#include <fcntl.h>
#include <sys/vfs.h>
#include <mntent.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sys/types.h>

#define MAX_CPU_TYPES 10

typedef struct {
	char model[128];
	int count;
} CPUMap;

void get_cpu_inventory(){
	FILE *fp = fopen("/proc/cpuinfo", "r");
	if (!fp) return;

	CPUMap inventory[MAX_CPU_TYPES];
	int type_count = 0;
	char line[256], current_model[128] = "";

	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "model name", 10) == 0) {
			char *name = strchr(line, ':') + 2;
			name[strcspn(name, "\n")] = 0;
			strncpy(current_model, name, 127);
		}
		// "physical id" identifies unique physical chips
		if (strncmp(line, "physical id", 11) == 0) {
			int found = 0;
			for (int i = 0; i < type_count; i++) {
				if (strcmp(inventory[i].model, current_model) == 0) {
					inventory[i].count++;
					found = 1;
					break;
				}
			}
			if (!found && type_count < MAX_CPU_TYPES) {
				strcpy(inventory[type_count].model, current_model);
				inventory[type_count].count = 1;
				type_count++;
			}
		}
	}
	fclose(fp);

	printf("--- CPU Inventory ---\n");
	for (int i = 0; i < type_count; i++) {
		printf("Type: %s | Physical Count: %d\n", inventory[i].model, inventory[i].count);
	}
	
}

void get_disk_info(const char *dev, const char *mnt) {
	// 1. Get Free Space
	struct statfs buf;
	if (statfs(mnt, &buf) == 0) {
		long long free_gb = (buf.f_bfree * buf.f_bsize) / (1024 * 1024 * 1024);
		printf("Mount: %s | Free: %lld GB | ", mnt, free_gb);
	}

	// 2. Get Serial (SCSI Method for RAID/Modern Disks)
	int fd = open(dev, O_RDONLY | O_NONBLOCK);
	if (fd >= 0) {
		unsigned char inq_cmd[] = {0x12, 1, 0x80, 0, 0xff, 0};
		unsigned char inq_resp[256];
		struct sg_io_hdr io_hdr = {0};
		io_hdr.interface_id = 'S';
		io_hdr.cmdp = inq_cmd;
		io_hdr.cmd_len = sizeof(inq_cmd);
		io_hdr.dxferp = inq_resp;
		io_hdr.dxfer_len = sizeof(inq_resp);
		io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;

		if (ioctl(fd, SG_IO, &io_hdr) == 0) {
			int len = inq_resp[3];
			printf("Serial: %.*s\n", len, &inq_resp[4]);
		} else {
			printf("Serial: [Incompatible Device]\n");
		}
		close(fd);
	} else {
		printf("Serial: [Permission Denied]\n");
	}
}

void scan_fstab() {
	printf("\n--- Storage Inventory (via fstab) ---\n");
	FILE *fp = setmntent("/etc/fstab", "r");
	if (!fp) return;

	struct mntent *ent;
	while ((ent = getmntent(fp)) != NULL) {
		// Skip pseudo filesystems (proc, sysfs, etc)
		if (ent->mnt_fsname[0] == '/') {
			get_disk_info(ent->mnt_fsname, ent->mnt_dir);
		}
	}
	endmntent(fp);
}

void get_memory_info() {
	struct sysinfo memInfo;
	sysinfo(&memInfo);
	long long totalPhysMem = memInfo.totalram;
	totalPhysMem *= memInfo.mem_unit;
	printf("Total Physical Memory: %lld bytes\n", totalPhysMem);
}

void get_cpu_info() {
	FILE *fp = fopen("/proc/cpuinfo", "r");
	char line[128];
	if (fp != NULL) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (strncmp(line, "model name", 10) == 0) {
				printf("CPU: %s", strchr(line, ':') +2);
				break;
			}
		}
		fclose(fp);
	}
}

void get_disk_serial(const char *dev) {
	struct hd_driveid hd;
	int fd = open(dev, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("Cannot open disk (need root)");
		return;
	}
	if (ioctl(fd, HDIO_GET_IDENTITY, &hd) == 0) {
		printf("Disk %s Serial: %.20s\n", dev, hd.serial_no);
	} else {
		perror("ioctl failed");
	}
	close(fd);
}

void get_scsi_serial(const char *dev) {
	int fd = open(dev, O_RDONLY);
	if (fd < 0) return;

	unsigned char inq_cmd[] = {0x12, 1, 0x80, 0, 0xff, 0}; // Inquiry Page 0x80
	unsigned char inq_resp[255];
	sg_io_hdr_t io_hdr = {0};

	io_hdr.interface_id = 'S';
	io_hdr.cmdp = inq_cmd;
	io_hdr.cmd_len = sizeof(inq_cmd);
	io_hdr.dxferp = inq_resp;
	io_hdr.dxfer_len = sizeof(inq_resp);
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;

	if (ioctl(fd, SG_IO, &io_hdr) == 0) {
		int len = inq_resp[3];
		printf("Serial: %.*s\n", len, &inq_resp[4]);
	}
	close(fd);
}

void get_system_serial() {
	FILE *fp = popen("dmidecode -s system-serial-number", "r");
	char serial[128];
	if (fp != NULL) {
		if (fgets(serial, sizeof(serial), fp) != NULL) {
			printf("System Serial: %s", serial);
		}
		pclose(fp);
	}
}

int main() {
	printf("--- System Report ---\n");
	get_system_serial();
	get_cpu_inventory();
	scan_fstab();
	get_cpu_info();
	get_memory_info();
	get_scsi_serial("/dev/sda");
	get_scsi_serial("/dev/sdb");
/* For legacy, non-raid drives
	get_disk_serial("/dev/sda");
	get_disk_serial("/dev/sdb");
*/
	return 0;
}
