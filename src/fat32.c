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

static struct FAT32DriverState driver_state = {0};

/**
 * Initialize DirectoryTable value with 
 * - Entry-0: DirectoryEntry about itself
 * - Entry-1: Parent DirectoryEntry
 * 
 * @param dir_table          Pointer to directory table
 * @param name               8-byte char for directory name
 * @param parent_dir_cluster Parent directory cluster number
 */
void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){
    memcpy(dir_table->table[0].name, name, 8);
    dir_table->table[0].attribute = ATTR_SUBDIRECTORY;
    dir_table->table[0].user_attribute = UATTR_NOT_EMPTY; 
    dir_table->table[0].cluster_low = parent_dir_cluster & 0xFFFF;  // Same as the parent for "."
    dir_table->table[0].cluster_high = (parent_dir_cluster >> 16) & 0xFFFF;
    dir_table->table[0].filesize = 0;


    memcpy(dir_table->table[1].name, "..", 2); 
    dir_table->table[1].attribute = ATTR_SUBDIRECTORY;
    dir_table->table[1].user_attribute = UATTR_NOT_EMPTY; 
    dir_table->table[1].cluster_low = parent_dir_cluster & 0xFFFF; 
    dir_table->table[1].cluster_high = (parent_dir_cluster >> 16) & 0xFFFF; 
}   
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

    driver_state.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    driver_state.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    driver_state.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;
    write_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

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
    else{
        read_clusters(driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
        read_clusters(driver_state.dir_table_buf.table, ROOT_CLUSTER_NUMBER, 1);
    }
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

void copy_long_cluster(void* target,int fileClusterNumber){
    read_clusters(target,fileClusterNumber,1);
    if(driver_state.fat_table.cluster_map[fileClusterNumber] != FAT32_FAT_END_OF_FILE){
        copy_long_cluster(target+CLUSTER_SIZE,driver_state.fat_table.cluster_map[fileClusterNumber]);
    }
}

int8_t read(struct FAT32DriverRequest request){
    uint8_t returnCode = 2;
    read_clusters(driver_state.dir_table_buf.table,request.parent_cluster_number,1);
    if (driver_state.dir_table_buf.table[0].user_attribute != UATTR_NOT_EMPTY){
        return -1;
    }
    uint32_t current_cluster_number = request.parent_cluster_number;
    unsigned int i = 2;
    while(current_cluster_number != FAT32_FAT_END_OF_FILE && returnCode == 2){
        while (i<CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) && returnCode == 2){
            if(memcmp(driver_state.dir_table_buf.table[i].name,request.name,8)==0){
                if(memcmp(driver_state.dir_table_buf.table[i].ext,request.ext,3)==0){
                    returnCode = 0;
                    copy_long_cluster(request.buf,i);
                }
                else returnCode = 1;
                
            }
            if(driver_state.dir_table_buf.table[i].user_attribute != UATTR_NOT_EMPTY){
                returnCode = 4;
            }
            i++;
        }
        current_cluster_number = driver_state.fat_table.cluster_map[current_cluster_number];
        read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number,1);
        i=0;
    }
    if(returnCode == 4)returnCode = 2;
    return returnCode;
}

int8_t read_directory(struct FAT32DriverRequest request){
    uint8_t returnCode = 2;
    read_clusters(driver_state.dir_table_buf.table,request.parent_cluster_number,1);
    if (driver_state.dir_table_buf.table[0].user_attribute != UATTR_NOT_EMPTY){
        return -1;
    }
    uint32_t current_cluster_number = request.parent_cluster_number;
    unsigned int i = 2;
    while(current_cluster_number != FAT32_FAT_END_OF_FILE && returnCode == 2){
        while (i<CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) && returnCode == 2){
            if(memcmp(driver_state.dir_table_buf.table[i].name,request.name,8)==0){
                if (driver_state.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
                    returnCode = 0;
                    copy_long_cluster(request.buf,i);
                }
                else returnCode = 1;
            }
            if(driver_state.dir_table_buf.table[i].user_attribute != UATTR_NOT_EMPTY){
                returnCode = 4;
            }
            i++;
        }
        current_cluster_number = driver_state.fat_table.cluster_map[current_cluster_number];
        read_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number,1);
        i=0;
    }
    if(returnCode == 4)returnCode = 2;
    return returnCode;
}

uint32_t get_empty_cluster() {
  for (uint32_t i = 8; i < CLUSTER_MAP_SIZE; i++) {
    if (driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY) {
      driver_state.fat_table.cluster_map[i] = FAT32_FAT_END_OF_FILE; // Mark as allocated
      return i;
    }
  }
  return -1; 
}


int8_t write(struct FAT32DriverRequest request){
    read_clusters(driver_state.dir_table_buf.table,request.parent_cluster_number,1);
    if (driver_state.dir_table_buf.table[0].user_attribute != UATTR_NOT_EMPTY){
        return 2;
    }

    // check if already exists
    uint32_t dirLen = sizeof(struct FAT32DirectoryTable)/sizeof(struct FAT32DirectoryEntry);
    for(uint32_t i = 1 ; i < dirLen ; i++){
        struct FAT32DirectoryEntry current_entry = driver_state.dir_table_buf.table[i];
        bool entry_valid = current_entry.undelete; 
        bool name_match = memcmp(current_entry.name, request.name, 8) == 0;
        bool ext_match = memcmp(current_entry.ext, request.ext, 3) == 0;

        if (entry_valid && name_match && ext_match) {
            return 1; 
        }
    }

    uint32_t clusterNeeded = ((request.buffer_size)% CLUSTER_SIZE) == 0 ? ((request.buffer_size)/ CLUSTER_SIZE): ((request.buffer_size)/ CLUSTER_SIZE) +1;


    int entry_num = -1;
    for (uint32_t i = 1; i < dirLen && entry_num == -1; i++) {
        if (driver_state.dir_table_buf.table[i].undelete == 0) {
            entry_num = i;
        }
    }
    if (entry_num == -1) {
        return -1;
    }

    uint32_t start_cluster = get_empty_cluster();
    // if(start_cluster == -1){
    //     return -1;
    // }

    struct FAT32DirectoryEntry new_entry = {0}; // Initialize with zeros

    memcpy(new_entry.name, request.name, 8); 
    memcpy(new_entry.ext, request.ext, 3);
    new_entry.cluster_low = start_cluster & 0xFFFF;
    new_entry.cluster_high = (start_cluster >> 16) & 0xFFFF;
    new_entry.undelete = 1;
    new_entry.user_attribute  = UATTR_NOT_EMPTY;

    if (request.buffer_size == 0) {
        new_entry.attribute = ATTR_SUBDIRECTORY;  // Create directory
        new_entry.filesize = 0;
        driver_state.dir_table_buf.table[entry_num] = new_entry;

        struct FAT32DirectoryTable new_dir_table = {0};
        init_directory_table(&new_dir_table,request.name,request.parent_cluster_number);
        write_clusters(&new_dir_table,start_cluster,1);
        write_clusters(&driver_state.dir_table_buf, request.parent_cluster_number, 1);
        write_clusters(&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
    }
    else{
        new_entry.attribute = 0;
        new_entry.filesize = request.buffer_size;
        driver_state.dir_table_buf.table[entry_num] = new_entry;

        write_clusters(request.buf,start_cluster,1);

        for(uint32_t i = 1; i < clusterNeeded;i++){
            uint32_t next_cluster = get_empty_cluster();
            driver_state.fat_table.cluster_map[start_cluster] = next_cluster;
            int offset = CLUSTER_SIZE*(i);
            write_clusters(request.buf + offset , next_cluster,1);
            start_cluster = next_cluster;
        }
        write_clusters(&driver_state.dir_table_buf, request.parent_cluster_number, 1);
        write_clusters(&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
    }
    
    return 0;
}