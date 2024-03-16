#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

static struct FAT32DriverState driver_state;

/**
 * Convert cluster number to logical block address
 * 
 * @param cluster Cluster number to convert
 * @return uint32_t Logical Block Address
 */
uint32_t cluster_to_lba(uint32_t cluster){
    return cluster*CLUSTER_BLOCK_COUNT;
}

/**
 * Checking whether filesystem signature is missing or not in boot sector
 * 
 * @return True if memcmp(boot_sector, fs_signature) returning inequality
 */
bool is_empty_storage(void){
    struct BlockBuffer boot_sector;
    read_blocks(&boot_sector, cluster_to_lba(BOOT_SECTOR), 1);
    return (memcmp(boot_sector.buf, (uint16_t*) fs_signature, BLOCK_SIZE) != 0);
}

/**
 * Create new FAT32 file system. Will write fs_signature into boot sector and 
 * proper FileAllocationTable (contain CLUSTER_0_VALUE, CLUSTER_1_VALUE, 
 * and initialized root directory) into cluster number 1
 */

void create_fat32(void){

    write_blocks(fs_signature, BOOT_SECTOR, 1);

    struct FAT32FileAllocationTable defaultFATTable = {0};

    defaultFATTable.cluster_map[0] = CLUSTER_0_VALUE;
    defaultFATTable.cluster_map[1] = CLUSTER_1_VALUE;
    defaultFATTable.cluster_map[2] = FAT32_FAT_END_OF_FILE;
    write_clusters(defaultFATTable.cluster_map, FAT_CLUSTER_NUMBER, 1);

    struct FAT32DirectoryTable defaultRootTable = {0};
    memcpy(defaultRootTable.table[0].name, "root", 8);
    defaultRootTable.table[0].attribute = ATTR_SUBDIRECTORY;
    defaultRootTable.table[0].user_attribute = UATTR_NOT_EMPTY;
    defaultRootTable.table[0].cluster_low = 0x2;
    defaultRootTable.table[0].cluster_high = 0x0;
    defaultRootTable.table[0].filesize = 0x0;

    memcpy(defaultRootTable.table[1].name, "root", 8);
    defaultRootTable.table[1].attribute = ATTR_SUBDIRECTORY;
    defaultRootTable.table[1].user_attribute = UATTR_NOT_EMPTY;
    defaultRootTable.table[1].cluster_low = 0x2;
    defaultRootTable.table[1].cluster_high = 0x0;
    defaultRootTable.table[1].filesize = 0x0;

    write_clusters(defaultRootTable.table, ROOT_CLUSTER_NUMBER, 1);
}

/**
 * Initialize file system driver state, if is_empty_storage() then create_fat32()
 * Else, read and cache entire FileAllocationTable (located at cluster number 1) into driver state
 */
void initialize_filesystem_fat32(void){
    if(is_empty_storage()){
        create_fat32();
    }
    read_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
    read_clusters(driver_state.dir_table_buf.table, ROOT_CLUSTER_NUMBER, 1);
}

/**
 * Write cluster operation, wrapper for write_blocks().
 * Recommended to use struct ClusterBuffer
 * 
 * @param ptr            Pointer to source data
 * @param cluster_number Cluster number to write
 * @param cluster_count  Cluster count to write, due limitation of write_blocks block_count 255 => max cluster_count = 63
 */
void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    write_blocks(ptr, cluster_to_lba(cluster_number), cluster_count*CLUSTER_BLOCK_COUNT);
}

/**
 * Read cluster operation, wrapper for read_blocks().
 * Recommended to use struct ClusterBuffer
 * 
 * @param ptr            Pointer to buffer for reading
 * @param cluster_number Cluster number to read
 * @param cluster_count  Cluster count to read, due limitation of read_blocks block_count 255 => max cluster_count = 63
 */
void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    read_blocks(ptr, cluster_to_lba(cluster_number), cluster_count*CLUSTER_BLOCK_COUNT);
}
