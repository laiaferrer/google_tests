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

TEST(ExistTest, ExistingKey) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_EXISTS;
    my_cmd.cdw11 = 4;                           //key size
    my_cmd.cdw2 = 0xcccccccc;                 //key value
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 0);
}

TEST(ExistTest, NotExistingKey) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_EXISTS;
    my_cmd.cdw11 = 4;                           //key size
    my_cmd.cdw2 = 0xeeeeeeee;                 //key value
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 135);
}

TEST(ExistTest, NotExistingKeySimilarToExisting) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_EXISTS;
    my_cmd.cdw11 = 4;                           //key size
    my_cmd.cdw2 = 0x0b;                 //key value
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 135);
}

TEST(ExistTest, KeyLengthTooShort) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_EXISTS;
    my_cmd.cdw11 = 0;                           //key size
    my_cmd.cdw2 = 0xcccccccc;                 //key value
    my_cmd.cdw3 = 0x1ddddddd;                 //key value
    my_cmd.cdw14 = 0xaaaaaaaa;
    my_cmd.cdw15 = 0xeeeeeee1;                 //key value
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 134);
}
//this crashed bc the data length is only 8 bits so if you try to put anything greater than 16 it crashes
TEST(ExistTest, KeyLengthTooLong) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_EXISTS;
    my_cmd.cdw11 = 17;                           //key size
    my_cmd.cdw2 = 0x0b;                 //key value
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 134);
}

TEST(ExistTest, ShorterKeyThanDataLength) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_EXISTS;
    my_cmd.cdw11 = 8;                           //key size
    my_cmd.cdw2 = 0xccccc;                 //key value
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 135);
}
TEST(ExistTest, LongerKeyThanDataLength) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    struct nvme_passthru_cmd my_cmd = {0,};
    my_cmd.nsid = 1;
    my_cmd.opcode = KV_OPC_EXISTS;
    my_cmd.cdw11 = 2;                           //key size
    my_cmd.cdw2 = 0xcccccccc;                 //key value
    ret = nvme_submit_io_passthru(fd, &my_cmd, &result);
    EXPECT_EQ(ret, 135);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
