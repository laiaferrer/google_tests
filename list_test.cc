#include <gtest/gtest.h>
#include "libnvme.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/nvme_ioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    KV_OPC_STORE = 0x01,
    KV_OPC_RETRIEVE = 0x02,
    KV_OPC_LIST = 0x06,
    KV_OPC_DELETE = 0x10,
    KV_OPC_EXISTS = 0x14,
} kv_opcode_e;

const size_t BUFFER_SIZE = 4096;
int ret = 0;
__u32 result;

void DumpHex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}


int open_nvme_device() {
    int fd = open("/dev/ng0n1", O_RDWR);
    if (fd < 0) {
        perror("Error opening the NVMe device");
    }
    return fd;
}

void close_nvme_device(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}

class NVMeTest : public ::testing::Test {
protected:
    
    void TearDown() override {
        // Limpieza después de cada prueba
        close_nvme_device(fd);
    }

    int fd;
};

TEST(ListTest, ExistingKey) {
    void *list_buffer = malloc(BUFFER_SIZE);
    if (!list_buffer) {
        TearDown();
    }
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    memset(list_buffer, 0, BUFFER_SIZE);
    my_cmd.addr = (__u64)list_buffer;
    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_LIST;
    my_cmd.cdw11 = 4;                           //key size
    my_cmd.cdw2 = 0xcccccccc;                 //key value
    my_cmd.cdw10 = BUFFER_SIZE;
    my_cmd.data_len = BUFFER_SIZE;
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 0);
    DumpHex(list_buffer, BUFFER_SIZE);
    free(list_buffer);
}

TEST(ListTest, NotExistingKey) {
    void *list_buffer = malloc(BUFFER_SIZE);
    if (!list_buffer) {
        TearDown();
    }
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    memset(list_buffer, 0, BUFFER_SIZE);
    my_cmd.addr = (__u64)list_buffer;
    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_LIST;
    my_cmd.cdw11 = 4;                           //key size
    my_cmd.cdw2 = 0xccccccc2;                 //key value
    my_cmd.cdw10 = BUFFER_SIZE;
    my_cmd.data_len = BUFFER_SIZE;
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 0);
    DumpHex(list_buffer, BUFFER_SIZE);
    free(list_buffer);
}

TEST(ListTest, BufferCanNotFitAnyKey) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    void *list_buffer = malloc(4);
    if (!list_buffer) {
        TearDown();
    }

    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_LIST;
    my_cmd.cdw11 = 4;                           //key size
    my_cmd.cdw2 = 0xcccccc89;                 //key value
    my_cmd.cdw10 = 4;
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 0);
    DumpHex(list_buffer, 4);
    free(list_buffer);
}

TEST(ListTest, BufferCanFit1Key) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    void *list_buffer = malloc(12);
    if (!list_buffer) {
        TearDown();
    }

    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_LIST;
    my_cmd.cdw11 = 4;                           //key size
    my_cmd.cdw2 = 0xcccccc89;                 //key value
    my_cmd.cdw10 = 12;
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 0);
    DumpHex(list_buffer, 12);
    free(list_buffer);
}

TEST(ListTest, BufferSizeTooSmall) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    void *list_buffer = malloc(0);
    if (!list_buffer) {
        TearDown();
    }

    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_LIST;
    my_cmd.cdw11 = 4;                           //key size
    my_cmd.cdw2 = 0xcccccc89;                 //key value
    my_cmd.cdw10 = 0;
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 137);
    DumpHex(list_buffer, 0);
    free(list_buffer);
}

TEST(ListTest, KeyLengthTooSmall) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    void *list_buffer = malloc(8);
    if (!list_buffer) {
        TearDown();
    }

    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_LIST;
    my_cmd.cdw11 = 0;                           //key size
    my_cmd.cdw2 = 0xcccccc89;                 //key value
    my_cmd.cdw10 = 8;
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 134);
    DumpHex(list_buffer, 8);
    free(list_buffer);
}

TEST(ListTest, KeyLengthTooBig) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    void *list_buffer = malloc(8);
    if (!list_buffer) {
        TearDown();
    }

    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_LIST;
    my_cmd.cdw11 = 19;                           //key size
    my_cmd.cdw2 = 0xcccccc89;                 //key value
    my_cmd.cdw10 = 8;
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 134);
    DumpHex(list_buffer, 8);
    free(list_buffer);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
