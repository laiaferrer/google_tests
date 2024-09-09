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
__u8 opcode = 0x14; 
__u8 flags = 0;
__u16 rsvd = 0;
__u32 nsid = 1;
__u32 cdw2 = 0;
__u32 cdw3 = 0;
__u32 cdw10 = 0;
__u32 cdw11 = 0;
__u32 cdw12 = 0;
__u32 cdw13 = 0;
__u32 cdw14 = 0;
__u32 cdw15 = 0;
__u32 data_len = 0;
void *data = NULL;
__u32 metadata_len = 0;
void *metadata = NULL;
__u32 timeout_ms = 1000;
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

TEST(StoreTest, ExistingKey) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 4;                           //key size
    cdw10 = strlen(kitty); 
    cdw2 = 0xcccccccc;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 0);
}

TEST(StoreTest, NotExistingKey) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 4;                           //key size
    cdw10 = strlen(kitty); 
    cdw2 = 0xcccccc89;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 0);
}

TEST(StoreTest, KeyLengthTooShort) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 0;                           //key size
    cdw10 = strlen(kitty); 
    cdw2 = 0xcccccc89;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret,134);
}

TEST(StoreTest, KeyLengthTooLong) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 18;                           //key size
    cdw10 = strlen(kitty); 
    cdw2 = 0xcccccc89;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 134);
}

TEST(StoreTest, ValueSizeZero) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = 0;
    cdw11 = 4;                           //key size
    cdw10 = 0; 
    cdw2 = 0xccccc789;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 0);
}

TEST(StoreTest, DataEqualsNull) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = NULL;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 4;                           //key size
    cdw10 = 0; 
    cdw2 = 0xcccccc89;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 0);
}

TEST(StoreTest, ValueSizeBiggerThanValue) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 4;                           //key size
    cdw10 = strlen(kitty) + 6; 
    cdw2 = 0xcccccc89;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 0);
}

TEST(StoreTest, ValueSizeSmallerThanValue) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 4;                           //key size
    cdw10 = strlen(kitty) - 3; 
    cdw2 = 0xcccccc89;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 0);
}

TEST(StoreTest, Bit8SetTo1KeyNotExists) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 260;                           //key size
    cdw10 = strlen(kitty); 
    cdw2 = 0xcc87cccc;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 135);
}

TEST(StoreTest, Bit8SetTo1KeyExists) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 260;                           //key size
    cdw10 = strlen(kitty); 
    cdw2 = 0xcccccccc;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 0);
}

TEST(StoreTest, Bit9SetTo1KeyExists) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 516;                           //key size
    cdw10 = strlen(kitty); 
    cdw2 = 0xcccccc89;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 137);
}

//I comment it bc otherwise it will only pass the first time because the others the file will be already created
/*
TEST(StoreTest, Bit9SetTo1KeyNotExists) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 516;                           //key size
    cdw10 = strlen(kitty); 
    cdw2 = 0xcc2c7889;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 0);
}
*/

TEST(StoreTest, Bit9AndBit8SetTo1KeyExists) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 772;                           //key size
    cdw10 = strlen(kitty); 
    cdw2 = 0xcccccccc;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 137);
}

TEST(StoreTest, Bit9AndBit8SetTo1KeyNotExists) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = strlen(kitty);
    cdw11 = 772;                           //key size
    cdw10 = strlen(kitty); 
    cdw2 = 0xccc47889;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 135);
}

TEST(StoreTest, CapacityExceeded) {
    int fd = open_nvme_device();
    ASSERT_GE(fd, 0) << "Could NOT open the NVMe device";
    char kitty[] = "kitty";
    nsid = 1;
    data = kitty;
    opcode = KV_OPC_STORE;
    data_len = 4096;
    cdw11 = 4;                           //key size
    cdw10 = 4096; 
    cdw2 = 0xcccccccc;                 //key value
    ret = nvme_io_passthru(fd, opcode, flags, rsvd, nsid, cdw2, cdw3,
                             cdw10, cdw11, cdw12, cdw13, cdw14, cdw15, data_len,
                             data, metadata_len, metadata, timeout_ms, &result);
    EXPECT_EQ(ret, 129);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
