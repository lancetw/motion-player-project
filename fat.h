/*
 * fat.h
 *
 *  Created on: 2011/02/27
 *      Author: Tonsuke
 */

#ifndef FAT_H_
#define FAT_H_

#include "stm32f4xx_conf.h"
#include <stddef.h>


//MBR
#define MBR_BootDsc    446
#define MBR_FileSysDsc 450
#define MBR_BpbSector  454

//BPB
//FAT16
#define BS_jmpBoot     0
#define BS_OEMName     3
#define BPB_BytsPerSec 11
#define BPB_SecPerClus 13
#define BPB_RsvdSecCnt 14
#define BPB_NumFATs    16
#define BPB_RootEndCnt 17
#define BPB_TotSec16   19
#define BPB_Media      21
#define BPB_FATSz16    22
#define BPB_SecPerTrk  24
#define BPB_NumHeads   26
#define BPB_HiddSec    28
#define BPB_TotSec32   32

#define BS_DrvNum      36
#define BS_Reserved1   37
#define BS_BootSig     38
#define BS_VolID       39
#define BS_VolLab      43
#define BS_FilSysType  54

//FAT32
#define BPB_bigSecPerFat    36
#define BPB_extFlags        40
#define BPB_fsVer           42
#define BPB_rootDirStrtClus 44
#define BPB_fsInfoSec       48
#define BPB_bkUpBootSec     50
#define BPB_reserved        52

//#define BS_DrvNum           64
//#define BS_Reserved1        65
//#define BS_BootSig          66
//#define BS_VolID            67
//#define BS_VolLab           71
//#define BS_FilSysType       82

#define DIR_ENTRY_SIZE   32


//Attributes
#define ATTRIBUTES     11

#define ATTR_READONLY  0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME    0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVER  0x20
#define ATTR_LFN       0x0F

// NT Reserved
#define NT_Reserved    12
#define NT_U2L_NAME    0x08
#define NT_U2L_EXT     0x10

//Entry info
#define ENTRY_EMPTY    0x00
#define ENTRY_DELETEDB 0x05
#define ENTRY_DIR      0x2E
#define ENTRY_DELETED  0xE5

#define CLUSTER_FAT16  26
#define FILESIZE       28

#define CLUSTER_FAT32_MSB 20
#define CLUSTER_FAT32_LSB 26

//LFN
#define LFN_SEQ_NUMBER   0
#define LFN_NAME_1ST     1
#define LFNATTR          11
#define LFN_CHECK_CODE   13
#define LFN_NAME_2ND     14
#define LFN_NAME_3RD     28

#define LFN_END          0x40
#define LFN_DELETED      0x80

#define LFN_WITHOUT_EXTENSION  0
#define LFN_WITH_EXTENSION     1

#define	SEEK_SET 0
#define	SEEK_CUR 1
#define	SEEK_END 2

#define FS_TYPE_FAT16  0x06
#define FS_TYPE_FAT32  0x0B

//ERROR
#define FS_ERROR_TYPE        1
#define FS_ERROR_CLUSTER_SIZE   2
#define FS_ERROR_BYTES_PER_CLUSTER 3


#define MAKE_BUF_NUM_SIZE 2048
#define MAKE_BUF_NUM_SECTOR (MAKE_BUF_NUM_SIZE / 512)
#define NUM_ENTRY_IN_BUF (MAKE_BUF_NUM_SIZE / 32)
#define MAX_ENTRY_COUNT (65536 / NUM_ENTRY_IN_BUF - 1)//100

extern const uint8_t partition_system_id[];

typedef struct __attribute__ ((packed)) {
	uint8_t code[446];
	struct {
		uint8_t boot_indicator;
		uint8_t start_head;
		uint16_t start_sector_cylinder;
		uint8_t systemID;
		uint8_t end_head;
		uint16_t end_sector_cylinder;
		uint32_t relative_sectors;
		uint32_t total_sectors;
	}partition_table[4];
	uint8_t signature[2];
}MBR_structTypedef;

typedef struct __attribute__ ((packed)) {
	uint8_t jmpOpCode[3];
	uint8_t OEMName[8];
	/* FAT16 */
	uint16_t bytesPerSector;	/* bytes/sector (512) */
	uint8_t sectorsPerCluster;	/* sectors/cluster */
	uint16_t reservedSectors;	/* reserved sector for BPB */
	uint8_t numberOfFATs;	/* the number of file allocation tables */
	uint16_t rootEntries;	/* the number of root entries (512) */
	uint16_t totalSectors;	/* the number of secters for this partition */
	uint8_t mediaDescriptor;	/* 0xf8: Hard Disk */
	uint16_t sectorsPerFAT;	/* number of sectors for FAT */
	uint16_t sectorsPerTrack;	/* sector/track (not used) */
	uint16_t heads;		/* heads number (not used) */
	uint32_t hiddenSectors;	/* hidden sector number */
	uint32_t bigTotalSectors;	/* total sector number */

	uint8_t driveNumber;
	uint8_t unused;
	uint8_t extBootSignature;
	uint32_t serialNumber;
	uint8_t volumeLabel[11];
	uint8_t fileSystemType[8];	/* "FAT1?   " */
	uint8_t loadProgramCode[448];
	uint16_t sig;		/* 0x55, 0xaa */
}BiosParameterBlockFAT16_structTypedef;

typedef struct __attribute__ ((packed)) {
    uint8_t    jmpOpCode[3];          /* 0xeb ?? 0x90 */
    uint8_t    OEMName[8];
    /* FAT32 */
	uint16_t bytesPerSector;	/* bytes/sector (512) */
	uint8_t sectorsPerCluster;	/* sectors/cluster */
	uint16_t reservedSectors;	/* reserved sector for BPB */
	uint8_t numberOfFATs;	/* the number of file allocation tables */
	uint16_t rootEntries;	/* the number of root entries (512) */
	uint16_t totalSectors;	/* the number of secters for this partition */
	uint8_t mediaDescriptor;	/* 0xf8: Hard Disk */
	uint16_t sectorsPerFAT;	/* number of sectors for FAT */
	uint16_t sectorsPerTrack;	/* sector/track (not used) */
	uint16_t heads;		/* heads number (not used) */
	uint32_t hiddenSectors;	/* hidden sector number */
	uint32_t bigTotalSectors;	/* total sector number */


    uint32_t    bigSectorsPerFAT;    /* sector/FAT for FAT32 */
    uint16_t    extFlags;            /* use index zero (follows) */
                                    /* bit 7      0: enable FAT mirroring, 1: disable mirroring */
                                    /* bit 0-3    active FAT number (bit 7 is 1) */
    uint16_t    FS_Version;
    uint32_t    rootDirStrtClus;     /* root directory cluster */
    uint16_t    FSInfoSec;           /* 0xffff: no FSINFO, other: FSINFO sector */
    uint16_t    bkUpBootSec;         /* 0xffff: no back-up, other: back up boot sector number */
    uint8_t    reserved[12];
    /* info */
    uint8_t    driveNumber;
    uint8_t    unused;
    uint8_t    extBootSignature;
    uint8_t    serialNumber[4];
    uint8_t    volumeLabel[11];
    uint8_t    fileSystemType[8];      /* "FAT32   " */
    uint8_t    loadProgramCode[420];
    uint8_t    sig[2];                 /* 0x55, 0xaa */
}BiosParameterBlockFAT32_structTypedef;


typedef struct {
	uint32_t lastDirCluster, lastDirEntry;
	uint16_t n;
	uint8_t set;
}fat_cache_typedef;

volatile struct {
	fat_cache_typedef cache;
	uint32_t reservedSectors, \
	         fatTable, \
	         userDataSector, \
	         currentDirCluster, \
	         currentDirEntry, \
	         biosParameterBlock, \
	         rootDirEntry;
	uint16_t *pfileList, \
			 fileCnt, \
			 bytesPerCluster, \
	         sectorsPerCluster, \
	         sectorsPerFAT;
	uint8_t fsType, \
			clusterDenomShift, \
			currentDirName[54];
} fat;

typedef struct frag_cluster{
	uint32_t pre, post;
} frag_cluster;

typedef struct fat_cache {
	frag_cluster *p_cluster_gap;
	uint16_t fragCnt;
} fat_cache;

typedef struct {
	uint8_t str_size, len;
	uint8_t str[54];
} fileNameStruct_TypeDef;


typedef volatile struct MY_FILE {
	size_t   clusterOrg, \
	         cluster, \
	         clusterCnt, \
           	 fileSize, \
           	 seekSector, \
           	 seekBytes, \
           	 dataSector;
	fat_cache cache;
} MY_FILE;

volatile struct {
	size_t (*getNCluster)(MY_FILE *fp, size_t, size_t);
} fat_func;


volatile uint8_t fbuf[512];
static const char root_str[] = {
		'R', '\0', \
		'o', '\0', \
		'o', '\0', \
		't', '\0', \
		'\0', '\0' \
};

extern int initFat(void);
size_t getNClusterCache(MY_FILE *fp, size_t count, size_t cluster);
extern size_t getCluster(uint32_t address, size_t cluster);
extern void setSFNname(char *name, int id);
extern int setExtensionName(char *name, int id);
extern uint8_t setLFNname(uint8_t *pLFNname, uint16_t id, uint8_t extension, uint8_t numBytes);
extern void makeFileList(void);
int getIdByName(const char *fileName);
uint16_t getListEntryPointByName(const char *fileName);
extern int getListEntryPoint(int id);
void sortListEntryPoint(void);
extern MY_FILE* my_fopen(int id);
extern void my_fclose(MY_FILE *fp);
extern int my_fseek(MY_FILE *fp, int64_t offset, int whence);
extern size_t read_file_sectors(MY_FILE *pfile, void *buf, size_t numSectors);
extern size_t my_fread(void *buf, size_t size, size_t count, MY_FILE *fp);
extern void changeDir(int id);


#endif /* FAT_H_ */
