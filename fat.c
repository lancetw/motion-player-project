/*
 * fat.c
 *
 *  Created on: 2011/02/27
 *      Author: Tonsuke
 */

#include "fat.h"
#include "sd.h"
#include "lcd.h"
#include "usart.h"
#include "mjpeg.h"
#include <stdlib.h>
#include <string.h>

#include "settings.h"

#undef MY_DEBUG

int initFat()
{
	uint32_t var;

	SDBlockRead((uint8_t*)fbuf, 0);						// MBR読み込み

	if(fbuf[MBR_FileSysDsc] == 0x6 || fbuf[MBR_FileSysDsc] == 0x4){
		fat.fsType = FS_TYPE_FAT16;
	} else if(fbuf[MBR_FileSysDsc] == 0xb || fbuf[MBR_FileSysDsc] == 0xc) {
		fat.fsType = FS_TYPE_FAT32;
	} else {
		return ERROR_FS_TYPE;
	}

	fat.biosParameterBlock = *((uint32_t*)&fbuf[MBR_BpbSector]); // BPB開始セクタ

	SDBlockRead((void*)fbuf, fat.biosParameterBlock);		// BPB読み込み

	var = *((uint16_t*)&fbuf[BPB_BytsPerSec]); // バイト/セクタ

	//if(var16 != 0x200) return -1;							// 1セクタ512バイトであること

	// セクタ/クラスタ
	if((fat.sectorsPerCluster = fbuf[BPB_SecPerClus]) < 8){
		return ERROR_CLUSTER_SIZE; // クラスタサイズが4KBより小さい場合はエラー
	}

	fat.bytesPerCluster = fat.sectorsPerCluster * 512;			// バイト/クラスタ

	fat.clusterDenomShift = 0;
	while(!(fat.bytesPerCluster & (1 << fat.clusterDenomShift++))){}; // Cluster bytes right shift denominator
	fat.clusterDenomShift = fat.clusterDenomShift - 1;

	fat.reservedSectors = *((uint16_t*)&fbuf[BPB_RsvdSecCnt]); // 予約領域

	if(fat.fsType == FS_TYPE_FAT16){
		fat_func.getNCluster = getNClusterCache;

		fat.sectorsPerFAT = *((uint16_t*)&fbuf[BPB_FATSz16]); // セクタ/FAT

		fat.rootDirEntry = fat.biosParameterBlock + fat.reservedSectors + fat.sectorsPerFAT * 2; // RDE(ルートディレクトリエントリ)開始セクタ
		fat.userDataSector = fat.rootDirEntry + 0x20;	// ユーザデータセクタ
	} else {
		fat_func.getNCluster = getNClusterCache;

		fat.sectorsPerFAT = *((uint32_t*)&fbuf[BPB_bigSecPerFat]);

		fat.userDataSector = fat.biosParameterBlock + fat.reservedSectors + fat.sectorsPerFAT * 2;	// ユーザデータセクタ

		var = *((uint32_t*)&fbuf[BPB_rootDirStrtClus]);

		fat.rootDirEntry = (uint32_t)(var - 2) * fat.sectorsPerCluster + fat.userDataSector; // カレントディレクトリエントリを更新
	}
	fat.fatTable = fat.biosParameterBlock + fat.reservedSectors;

	fat.currentDirEntry = fat.rootDirEntry;		// 現在のディレクトリエントリをルートに

	makeFileList();							// ルートディレクトリエントリのファイルリスト作成

	memcpy((char*)fat.currentDirName, (char*)root_str, sizeof(root_str));

	return 0;
}

size_t getNClusterCache(MY_FILE *fp, size_t count, size_t cluster)
{
	if(fp->cache.fragCnt == 0){ // no fragmentation
		return (cluster + count);
	}

	int i;
	uint32_t avail, lastClusterGap = fp->clusterOrg;

	for(i = 0;i < (fp->cache.fragCnt - 1);i++){ // search which cache contains cluster
		if(lastClusterGap <= cluster && cluster <= fp->cache.p_cluster_gap[i].pre){
			break;
		}
		lastClusterGap = fp->cache.p_cluster_gap[i].post;
	}

	for(;i < fp->cache.fragCnt;i++){
		avail = fp->cache.p_cluster_gap[i].pre - cluster;
		if(count <= avail){
			return (cluster + count);
		}
		count = count - avail - 1;
		cluster = fp->cache.p_cluster_gap[i].post;
	}

	return (fp->cache.p_cluster_gap[i - 1].post + count);
}

size_t getCluster(uint32_t address, size_t cluster)
{
	uint32_t var32, i, temp = address / fat.bytesPerCluster;

	if(fat.fsType == FS_TYPE_FAT16){
		for(i = 0;i < temp;i++){
			var32 = cluster;
			SDBlockRead((uint8_t*)fbuf, fat.fatTable + (var32 >> 8)); // FATテーブルから次のクラスタ番号を取得

			var32 = (var32 << 1) & 0x1ff;
			cluster = *((uint16_t*)&fbuf[var32]);
		}
	} else { // FAT32
		for(i = 0;i < temp;i++){
			var32 = cluster;

			SDBlockRead((uint8_t*)fbuf, fat.fatTable + (var32 >> 7)); // FATテーブルから次のクラスタ番号を取得

			var32 = (var32 << 2) & 0x1ff;
			cluster  = *((uint32_t*)&fbuf[var32]);
		}
	}

	return cluster;
}

void setSFNname(char *name, int id){
	uint16_t entryPointOffset;
	entryPointOffset = getListEntryPoint(id);
	strncpy(name, (char*)&fbuf[entryPointOffset], 8);
	strtok(name, " ");
}

// set extension name
// return 1: archive 0: directry
int setExtensionName(char *name, int id){
	int ret = 0;
	uint16_t entryPointOffset;
	entryPointOffset = getListEntryPoint(id);

	if(!(fbuf[entryPointOffset + ATTRIBUTES] & _BV(ATTR_DIRECTORY))){
		if(fbuf[entryPointOffset + 8] != 0x20){
			strncpy(name, (char*)&fbuf[entryPointOffset + 8], 3);
		} else {
			strcpy(name, "---");
		}
		ret = 1;
	} else {
		ret = 0;
	}
	return ret;
}

uint8_t setLFNname(uint8_t *pLFNname, uint16_t id, uint8_t extension, uint8_t numBytes)
{
	uint8_t LFNnameLen = 0, LFNseq = 0, i = 0;
	uint16_t LFNentryPointOffset;

	numBytes -= numBytes % 26;

	LFNentryPointOffset = getListEntryPoint(id); // リスト上にあるIDファイルエントリの先頭位置をセット

	do{
		if(LFNentryPointOffset == 0){
			SDBlockRead((uint8_t*)fbuf, ((*(fat.pfileList + id) / 16) + fat.currentDirEntry - ++i));	// リストのエントリ位置のセクタを読み込む
			LFNentryPointOffset = 512;
		}
		LFNentryPointOffset -= 32;

		if (fbuf[LFNentryPointOffset + ATTRIBUTES] != 0x0F || \
		   (fbuf[LFNentryPointOffset + LFN_SEQ_NUMBER] & 0x1F) != ++LFNseq || \
		   (fbuf[LFNentryPointOffset + LFN_SEQ_NUMBER] & _BV(7)) ){
			return 0;
		}

		memcpy((void*)(pLFNname + LFNnameLen), (const void*)&fbuf[LFNentryPointOffset + LFN_NAME_1ST], 10);
		memcpy((void*)(pLFNname + LFNnameLen + 10), (const void*)&fbuf[LFNentryPointOffset + LFN_NAME_2ND], 12);
		memcpy((void*)(pLFNname + LFNnameLen + 22), (const void*)&fbuf[LFNentryPointOffset + LFN_NAME_3RD], 4);

		if((LFNnameLen += 26) >= numBytes) break;
	} while( !(fbuf[LFNentryPointOffset + LFN_SEQ_NUMBER] & _BV(6)) );

	if(!extension){
		i = LFNnameLen;
		do{
			if(*(pLFNname + i) == '.'){
				*(pLFNname + i) = '\0';
				break;
			}
		}while(i-- > 0);
	}

	*(pLFNname + LFNnameLen) = '\0';
	*(pLFNname + LFNnameLen + 1) = '\0';

	return 1;
}


void makeFileList()
{
#define MAKE_BUF_NUM_SIZE 4096
#define MAKE_BUF_NUM_SECTOR (MAKE_BUF_NUM_SIZE / 512)
#define NUM_ENTRY_IN_BUF (MAKE_BUF_NUM_SIZE / 32)
#define MAX_ENTRY_COUNT 100

	int i, j, k, loop_brake = 0, entryIdx = 0;
	uint16_t *p = fat.pfileList;
	uint8_t buf[MAKE_BUF_NUM_SIZE];

	fat.fileCnt = 0;
	if(fat.currentDirEntry == fat.rootDirEntry){ // exception for settings item
		fat.fileCnt++;
	}
	for(i = 0;i < 2;i++){	// ループ一回目はファイルの個数を数える。二回目でファイルの個数分メモリを確保し、各ファイルのエントリ位置を記録する。
		if(i != 0){
			fat.pfileList = (uint16_t*)malloc(fat.fileCnt * sizeof(uint16_t));
			p = fat.pfileList;
			if(fat.currentDirEntry == fat.rootDirEntry){ // exception for settings item
				*p++ = 0xffff;
			}
		}

		for(k = 0;k < MAX_ENTRY_COUNT;k++){ // MAX_SEARCH_ENTRY = MAX_ENTRY_COUNT * NUM_ENTRY_IN_BUF
			SDMultiBlockRead((uint8_t*)buf, (fat.currentDirEntry + k * MAKE_BUF_NUM_SECTOR), MAKE_BUF_NUM_SECTOR);
			for(j = 0;j < MAKE_BUF_NUM_SIZE;j += 32){
				if(i != 0){
					entryIdx++;	// エントリインデックスを+1
				}
				if((buf[j + ATTRIBUTES] & (_BV(ATTR_VOLUME) | _BV(ATTR_HIDDEN) | _BV(ATTR_SYSTEM)) ) != 0){ // ボリューム属性、隠し属性、システム属性はリストに加えない。
					continue;
				}
				if((i == 0) && (buf[j] == ENTRY_EMPTY)){ // 空きエントリだったらカウント終了
					loop_brake = 1;
					break;
				}
				if((buf[j] == ENTRY_DELETED) || (buf[j] == ENTRY_DELETEDB) || \
				   ((j == 0 && k == 0) && fat.currentDirEntry != fat.rootDirEntry)){
					continue; //削除済みエントリ、カレントディレクトリ(エントリ先頭)は無視する
				}
				if(i != 0){
					*p++ = entryIdx - 1;	// 現在ファイルのエントリ位置を記録
				} else {
					fat.fileCnt++;
				}
			}
			if((i == 0) && loop_brake){
				break;
			}
		}
	}
	cursor.pages = (fat.fileCnt - 1) / PAGE_NUM_ITEMS;
	sortListEntryPoint();
}

int getIdByName(const char *fileName)
{
	int i;
	uint16_t entryPointOffset;
	char *name, *extension, fileNameCopy[strlen(fileName)];

	memcpy((void*)fileNameCopy, fileName, strlen(fileName));

	if(!strchr(fileNameCopy, '.')){
		name = (char*)fileName; // 拡張子がなければnameはfileNameを指定
		extension = '\0';
	} else {
		name = strtok((char*)fileNameCopy, "."); // .(ピリオド)があればNULLに置き換える
		extension = &fileNameCopy[strlen(name) + 1]; // 拡張子があればextensionに格納先のポインタを指定する
	}

	for(i = 0;i < fat.fileCnt;i++){
		if((i == 0) && (fat.currentDirEntry == fat.rootDirEntry)){  // exception for settings item
			continue;
		}
		entryPointOffset = getListEntryPoint(i);
		if(strncmp((char*)&fbuf[entryPointOffset], (const char*)name, strlen(name) ) == 0){ // nameが一致した場合
			if(!extension){ // 拡張子がない場合
				return i;
			}
			else if(strncmp(extension, (char*)&fbuf[entryPointOffset + 8], 3) == 0){ // 拡張子あり且つextensionが一致した場合
				return i;
			}
		}
	}

	return -1;
}


uint16_t getListEntryPointByName(const char *fileName)
{
	int i;
	uint16_t entryPointOffset;
	char *name, *extension, fileNameCopy[strlen(fileName)];

	memcpy((void*)fileNameCopy, fileName, strlen(fileName));

	if(!strchr(fileNameCopy, '.')){
		name = (char*)fileName; // 拡張子がなければnameはfileNameを指定
		extension = '\0';
	} else {
		name = strtok((char*)fileNameCopy, "."); // .(ピリオド)があればNULLに置き換える
		extension = &fileNameCopy[strlen(name) + 1]; // 拡張子があればextensionに格納先のポインタを指定する
	}

	for(i = 0;i < fat.fileCnt;i++){
		entryPointOffset = getListEntryPoint(i);
		if(strncmp((char*)&fbuf[entryPointOffset], (const char*)name, strlen(name) ) == 0){
			if(!extension){ // 拡張子がない場合
				return entryPointOffset;
			}else if(strncmp(extension, (char*)&fbuf[entryPointOffset + 8], 3) == 0){ // 拡張子あり且つextensionが一致した場合
				return entryPointOffset;
			}
		}
	}

	return 0xFFFF;
}

int getListEntryPoint(int id)
{
	SDBlockRead((uint8_t*)fbuf, ((*(fat.pfileList + id) / 16) + fat.currentDirEntry));	// リストのエントリ位置のセクタを読み込む
	return ( (*(fat.pfileList + id) % 16) * 32 ); // リストのファイルエントリの先頭位置を返す
}

int strcmp_filename(fileNameStruct_TypeDef *str0, fileNameStruct_TypeDef *str1)
{
	int i, cmp;

	int len0 = str0->len, \
		len1 = str1->len, \
		n = len0 <= len1 ? len0 : len1;

	for(i = 0;i < n;i++){
		cmp =   str0->str_size <= sizeof(uint8_t) ? *((uint8_t*)&(str0->str)[i]) : *((uint16_t*)&(str0->str)[i * 2]);
		cmp -=  str1->str_size <= sizeof(uint8_t) ? *((uint8_t*)&(str1->str)[i]) : *((uint16_t*)&(str1->str)[i * 2]);
		if(cmp > 0){
			return 1;
		} else if(cmp < 0){
			return -1;
		}
	}

	if(n == len0){
		return -1;
	} else if(n == len1){
		return 1;
	}

	return 0;
}

void sortListEntryPointSpecifiedArea(int idx, int count)
{
	uint32_t i, j;
	uint16_t entryPointOffset, temp, *p16;
	char str0[54], str1[54];

	fileNameStruct_TypeDef fileNameStr0, fileNameStr1;
	fileNameStr0.str = (uint8_t*)str0;
	fileNameStr1.str = (uint8_t*)str1;

	count += idx;

	for(i = count - 1;i > idx;i--){
		for(j = idx;j < i;j++){
			str0[0] = str1[0] = '\0';

			if(!setLFNname((uint8_t*)str0, j, LFN_WITHOUT_EXTENSION, sizeof(str0))){ // SFN
				entryPointOffset = getListEntryPoint(j);
				strncpy(str0, (char*)&fbuf[entryPointOffset], 8);
				strtok(str0, " ");
				fileNameStr0.str_size = sizeof(uint8_t);
				fileNameStr0.len = strlen(str0);
			} else { // LFN
				fileNameStr0.str_size = sizeof(uint16_t);
				p16 = (uint16_t*)str0;
				fileNameStr0.len = 0;
				while(*p16++ != 0x0000 || fileNameStr0.len <= 54){
					fileNameStr0.len++;
				}
			}

			if(!setLFNname((uint8_t*)str1, j + 1, LFN_WITHOUT_EXTENSION, sizeof(str1))){ // SFN
				entryPointOffset = getListEntryPoint(j + 1);
				strncpy(str1, (char*)&fbuf[entryPointOffset], 8);
				strtok(str1, " ");
				fileNameStr1.str_size = sizeof(uint8_t);
				fileNameStr1.len = strlen(str1);
			} else { // LFN
				fileNameStr1.str_size = sizeof(uint16_t);
				p16 = (uint16_t*)str1;
				fileNameStr1.len = 0;
				while(*p16++ != 0x0000 || fileNameStr1.len <= 54){
					fileNameStr1.len++;
				}
			}

			if(strcmp_filename(&fileNameStr0, &fileNameStr1) > 0){
				temp = fat.pfileList[j];
				fat.pfileList[j] = fat.pfileList[j + 1];
				fat.pfileList[j + 1] = temp;
			}
		}
	}
}

void sortListEntryPoint()
{
	if(fat.fileCnt <= 2) return;

	int i, j, directries = 0, archives = 0, idx_dir = 0;
	uint16_t entryPointOffset, temp;
	char fileExtStr[4];

	for(i = 0;i < fat.fileCnt;i++){
		if((i == 0) && (fat.currentDirEntry == fat.rootDirEntry)){  // exception for settings item
			continue;
		}
		entryPointOffset = getListEntryPoint(i);
		if(fbuf[entryPointOffset + ATTRIBUTES] & _BV(ATTR_DIRECTORY)){
			directries++;
		}else{
			archives++;
		}
	}

	for(i = 0;i < fat.fileCnt;i++){ // collect directries and arrange from top
		if((i == 0) && (fat.currentDirEntry == fat.rootDirEntry)){ // exception for settings item
			idx_dir++;
			directries++;
			continue;
		}
		entryPointOffset = getListEntryPoint(i);
		if(fbuf[entryPointOffset + ATTRIBUTES] & _BV(ATTR_DIRECTORY)){
			temp = fat.pfileList[idx_dir];
			fat.pfileList[idx_dir] = fat.pfileList[i];
			fat.pfileList[i] = temp;
			if(++idx_dir >= directries){
				break;
			}
		}
	}

	if(fat.currentDirEntry == fat.rootDirEntry){ // current == root, entire directries be sorted except #0 settings
//		sortListEntryPointSpecifiedArea(0, directries);
		sortListEntryPointSpecifiedArea(1, directries - 1);  // exception for settings item
	} else { // skip ".." directry path from sort
		sortListEntryPointSpecifiedArea(1, directries - 1);
	}
	if(archives <= 1){
		return; // no need to sort archives
	}

	if(!settings_group.filer_conf.sort){
		return;
	}

	struct struct_ext_item{ // for extension arrangement
		uint16_t id;
		char name[4];
		struct struct_ext_item *next;
	};
	struct struct_ext_item start_ext_item, *ptr_ext_item, *temp1_ext_item, *temp2_ext_item, *new_ptr_ext_item;
	int ext_item_list_len = 0;

	ptr_ext_item = &start_ext_item;
	ptr_ext_item->next = NULL;
	ptr_ext_item = &start_ext_item;

	for(i = directries;i < fat.fileCnt;i++){ // make extension item list by extension names
		fileExtStr[0] = '\0';
		entryPointOffset = getListEntryPoint(i);
		if(fbuf[entryPointOffset + 8] != 0x20){
			strncpy(fileExtStr, (char*)&fbuf[entryPointOffset + 8], 3);
		} else {
			strcpy(fileExtStr, "---");
		}

		// make item list by extension name
		ext_item_list_len++;
		new_ptr_ext_item = malloc(sizeof(struct struct_ext_item));
		new_ptr_ext_item->id = fat.pfileList[i];
		strcpy(new_ptr_ext_item->name, fileExtStr);

		ptr_ext_item->next = new_ptr_ext_item;
		new_ptr_ext_item->next = NULL;
		ptr_ext_item = new_ptr_ext_item;
	}

	for(i = ext_item_list_len - 1;i >= 0;i--){ // collect extension items to make extension groups
		j = 0;
		for(ptr_ext_item = &start_ext_item;ptr_ext_item != NULL;ptr_ext_item = ptr_ext_item->next){
			if(j++ >= i){
				break;
			}
			if(strcmp(ptr_ext_item->next->name, ptr_ext_item->next->next->name) > 0){
				temp2_ext_item = ptr_ext_item->next->next->next;
				temp1_ext_item = ptr_ext_item->next;
				ptr_ext_item->next = ptr_ext_item->next->next;
				ptr_ext_item->next->next = temp1_ext_item;
				temp1_ext_item->next = temp2_ext_item;
			}
		//			debug.printf("\r\n%s %d", ptr_ext_item->name, ptr_ext_item->id);
		}
	}

	// find extension name gap and sort
	i = directries;
	int nameCnt = 0, nameGap = directries;
	ptr_ext_item = start_ext_item.next;
	strcpy(fileExtStr, ptr_ext_item->name);
	for(;ptr_ext_item != NULL;ptr_ext_item = ptr_ext_item->next){
		fat.pfileList[i++] = ptr_ext_item->id;
		if(strncmp(fileExtStr, ptr_ext_item->name, 3) == 0){
			nameCnt++;
		} else {
			sortListEntryPointSpecifiedArea(nameGap, nameCnt);
			strncpy(fileExtStr, ptr_ext_item->name, 3);
			nameGap = i - 1;
			nameCnt = 1;

		}
	}
	sortListEntryPointSpecifiedArea(nameGap, nameCnt);

	// delete list
	ptr_ext_item = start_ext_item.next;
	do{
		temp1_ext_item = ptr_ext_item->next;
		free((void*)ptr_ext_item);
		ptr_ext_item = temp1_ext_item;
	}while(ptr_ext_item != NULL);
}

MY_FILE* my_fopen(int id)
{
	int i, j;
	void *new;
	uint32_t var, cluster, preCluster, cont, fatsize;
	uint16_t entryPointOffset;
	uint8_t clusterBuf[1024];

	MY_FILE *fp;

	if(id < 0){
		return '\0';
	}

	entryPointOffset = getListEntryPoint(id);
	if(fbuf[entryPointOffset + ATTRIBUTES] & _BV(ATTR_DIRECTORY)){
		return '\0';
	}

	fp = malloc(sizeof(MY_FILE));

	if(fat.fsType == FS_TYPE_FAT16){
		fp->clusterOrg  = fbuf[entryPointOffset + CLUSTER_FAT16];
		fp->clusterOrg |= (uint16_t)fbuf[entryPointOffset + CLUSTER_FAT16 + 1] << 8;
	} else {
		fp->clusterOrg  = fbuf[entryPointOffset + CLUSTER_FAT32_LSB];
		fp->clusterOrg |= (uint32_t)fbuf[entryPointOffset + CLUSTER_FAT32_LSB + 1] << 8;
		fp->clusterOrg |= (uint32_t)fbuf[entryPointOffset + CLUSTER_FAT32_MSB] << 16;
		fp->clusterOrg |= (uint32_t)fbuf[entryPointOffset + CLUSTER_FAT32_MSB + 1] << 24;
	}

	fp->cluster = fp->clusterOrg;
	fp->dataSector = (fp->cluster - 2) * fat.sectorsPerCluster + fat.userDataSector;

	fp->fileSize = *((uint32_t*)&fbuf[entryPointOffset + FILESIZE]);

	fp->seekSector = 0;
	fp->seekBytes = 0;
	fp->clusterCnt = 0;

#ifdef MY_DEBUG
	debug.printf("\r\n\nfile ID:%d", id);
	debug.printf("\r\nclusterOrg:%d", fp->clusterOrg);
#endif
	// cacche file fragments
	preCluster = fp->clusterOrg;
	cluster = fp->clusterOrg;
	cont = cluster;
	i = 0, j = 0;
	fp->cache.fragCnt = 0;
	fp->cache.p_cluster_gap = '\0';
	fatsize = fat.fsType == FS_TYPE_FAT16 ? sizeof(uint16_t):sizeof(uint32_t);
	do{ // check entire clusters in file to detect fragmentation.
		SDMultiBlockRead((uint8_t*)clusterBuf, \
				fat.fatTable + (cluster / (sizeof(clusterBuf) / fatsize) * sizeof(clusterBuf) / 512 ), \
				sizeof(clusterBuf) / 512); //

		var = (cluster * fatsize) & (sizeof(clusterBuf) - 1);
		for(;;){
			if(fat.fsType == FS_TYPE_FAT16){ // FAT16
				cluster = *((uint16_t*)&clusterBuf[var]);
			} else { // FAT32
				cluster = *((uint32_t*)&clusterBuf[var]);
			}
			if(cluster == (fat.fsType == FS_TYPE_FAT16 ? 0xFFF7 : 0x0FFFFFF7)){ // bad cluster contain
				debug.printf("\r\nBad cluster contain");
				return fp;
			}
			if(cluster >= (fat.fsType == FS_TYPE_FAT16 ? 0xFFF8 : 0x0FFFFFF8)){ // reached cluster terminate
//#ifdef MY_DEBUG
				if(fp->cache.fragCnt){
					debug.printf("\r\n\nfragment count:%d", fp->cache.fragCnt);
					for(i = 0;i < fp->cache.fragCnt;i++){
						debug.printf("\r\n%02d pre:%08d post:%08d", i, fp->cache.p_cluster_gap[i].pre, fp->cache.p_cluster_gap[i].post);
					}
					debug.printf("\r\n");
				}
//#endif
#ifdef MY_DEBUG
	my_fseek(fp, -1, SEEK_END);
	debug.printf("\r\nlasr cluste:%d", fp->cluster);
	my_fseek(fp, 0, SEEK_SET);
#endif
				return fp;
			}

			if(cluster != ++cont){
//#ifdef MY_DEBUG
//				debug.printf("\r\nFragment Detected");
//#endif
				fp->cache.fragCnt++;
				new = malloc(fp->cache.fragCnt * sizeof(frag_cluster));
				if(fp->cache.p_cluster_gap != '\0'){
					memcpy(new, fp->cache.p_cluster_gap, (fp->cache.fragCnt - 1) * sizeof(frag_cluster));
					free((void*)fp->cache.p_cluster_gap);
				}
				fp->cache.p_cluster_gap = new;
				fp->cache.p_cluster_gap[fp->cache.fragCnt - 1].pre = preCluster;
				fp->cache.p_cluster_gap[fp->cache.fragCnt - 1].post = cluster;
//#ifdef MY_DEBUG
//				debug.printf("\r\nfp->cache.p_cluster_gap[fp->cache.fragCnt - 1].pre:%p", &fp->cache.p_cluster_gap[fp->cache.fragCnt - 1].pre);
//				debug.printf("\r\nfp->cache.p_cluster_gap[fp->cache.fragCnt - 1].post:%p", &fp->cache.p_cluster_gap[fp->cache.fragCnt - 1].post);
//#endif
				cont = cluster;
				preCluster = cluster;
				break;
			}
			preCluster = cluster;
			var += fatsize;
			if(var >= sizeof(clusterBuf)) {
				break;
			}
		}
	}while(i++ < fp->fileSize / (fat.bytesPerCluster / (sizeof(clusterBuf) / fatsize)));


	return fp;
}

void my_fclose(MY_FILE *fp)
{
	free((void*)fp->cache.p_cluster_gap);
	fp->cache.p_cluster_gap = '\0';
	free((void*)fp);
	fp = '\0';
}

int my_fseek(MY_FILE *fp, int64_t offset, int whence)
{
	uint32_t seekCluster;

	switch (whence) {
		case SEEK_SET:
			fp->seekBytes = offset;
			fp->cluster = fp->clusterOrg;
			fp->clusterCnt = 0;
			if(offset == 0){
				fp->dataSector = (uint32_t)(fp->cluster - 2) * fat.sectorsPerCluster + fat.userDataSector;
				fp->seekSector = 0;
				return 0;
			}
			break;
		case SEEK_CUR:
			fp->seekBytes += offset;
			break;
		case SEEK_END:
			fp->seekBytes = fp->fileSize + offset - 1;
			fp->cluster = fp->clusterOrg;
			fp->clusterCnt = 0;
			break;
		default:
			return 1;
	}
	fp->seekSector = (fp->seekBytes >> 9) & (fat.sectorsPerCluster - 1);
	seekCluster = fp->seekBytes >> fat.clusterDenomShift;

	if(seekCluster > fp->clusterCnt){
		fp->cluster = fat_func.getNCluster(fp, seekCluster - fp->clusterCnt, fp->cluster);
		fp->clusterCnt = seekCluster;
		fp->dataSector = (uint32_t)(fp->cluster - 2) * fat.sectorsPerCluster + fat.userDataSector;
	}

	if(fp->seekBytes >= fp->fileSize){
		return 0;
	}

	return 1;
}

size_t readFileSectors(MY_FILE *pfile, void *buf, size_t numSectors)
{
	uint32_t numAvailSectors, numRestSectors, dataBlockAddress, \
			 seekSector, cluster , ret = 0;

	seekSector = pfile->seekSector;
	cluster = pfile->cluster;

	dataBlockAddress = (pfile->cluster - 2) * fat.sectorsPerCluster + fat.userDataSector; // クラスタ番号のデータ領域先頭セクタをセット

	numAvailSectors = fat.sectorsPerCluster - (pfile->seekSector & (fat.sectorsPerCluster - 1));

	if(numSectors > numAvailSectors){ // セクタ数が読込み可能セクタより多い場合
		numRestSectors = numSectors - numAvailSectors; // 残りセクタ＝セクタ数 - 読込み可能セクタ

		ret = SDMultiBlockRead(buf, dataBlockAddress + pfile->seekSector, numAvailSectors); // 読込み可能セクタ数分バッファリング

		pfile->seekSector = 0;
		pfile->cluster = fat_func.getNCluster(pfile, 1, pfile->cluster);

		dataBlockAddress = (pfile->cluster - 2) * fat.sectorsPerCluster + fat.userDataSector; // クラスタ番号のデータ領域先頭セクタをセット

		ret += SDMultiBlockRead((uint32_t*)buf + numAvailSectors * (512 / sizeof(uint32_t)), dataBlockAddress, numRestSectors);

		pfile->seekSector += numRestSectors;
	} else {
		ret = SDMultiBlockRead(buf, dataBlockAddress + pfile->seekSector, numSectors);

		pfile->seekSector += numSectors;
		if(pfile->seekSector >= fat.sectorsPerCluster){
			pfile->seekSector = pfile->seekSector & (fat.sectorsPerCluster - 1);
			pfile->cluster = fat_func.getNCluster(pfile, 1, pfile->cluster);
		}
	}

	pfile->seekSector = seekSector;
	pfile->cluster = cluster;

	my_fseek(pfile, numSectors << 9, SEEK_CUR);

	return ret;
}


size_t my_fread(void *buf, size_t size, size_t count, MY_FILE *fp)
{
	size_t n = size * count;

	if(n <= 0){
		return 0;
	}

	void *pbuf;
	int cnt;
	uint32_t var, cluster, strn1, strn2, rest;

	if(fp->fileSize < (fp->seekBytes + n)){ // 要求サイズがファイルサイズを超えていた場合
		n = fp->fileSize - fp->seekBytes;
	}

	strn1 = 512 - (fp->seekBytes & 511); //１回の読込みでコピーできるバイト数
	strn2 = 0;
	if(n > strn1){ // 一回の読込みでコピーできるバイト数を超えていたときの処理(読込むべきデータ列が次のセクタにまたがっていた場合の処理）
		if(n <= 512){ // 読込むべきバイト数が512以下ならば２回目の読み込みバイト数は１回目の残り
			strn2 = n - strn1;
		} else {
			if(strn1 != 512){
				pbuf = buf;
				SDBlockRead((uint8_t*)fbuf, fp->dataSector + fp->seekSector);
				memcpy(pbuf, (uint8_t*)&fbuf[fp->seekBytes & 511], strn1); // 一回目の読込み
				pbuf += strn1; // ポインタを一回目の読込み分進める
				var = n - strn1; // 読込み数から一回目の読込み分引く
				cnt = var >> fat.clusterDenomShift, rest = var & (fat.bytesPerCluster - 1) ; // セクタ数と残りセクタ
				if(++fp->seekSector > fat.sectorsPerCluster){ // 次のシークセクタが次のクラスタにまたがっていた場合の処理
					cluster = fat_func.getNCluster(fp, 1, fp->cluster);
				}
				my_fseek(fp, strn1, SEEK_CUR);
			} else { // strn1 == 512
				pbuf = buf;
				strn1 = 0;
				cnt = n >> fat.clusterDenomShift, rest = n & (fat.bytesPerCluster - 1);
			}

			while(cnt--){
				readFileSectors(fp, pbuf, fat.sectorsPerCluster);
				pbuf += fat.bytesPerCluster;
			}

			if(rest != 0){
				readFileSectors(fp, pbuf, rest >> 9);
				pbuf += (rest >> 9) << 9;
				rest = rest & 511;
				if(rest != 0){
					my_fread(pbuf, size, rest, fp);
				}
			}
			return ( size == 1 ? n : (n / size) );
		}
		SDBlockRead((uint8_t*)fbuf, fp->dataSector + fp->seekSector);
		memcpy(buf, (uint8_t*)&fbuf[fp->seekBytes & 511], strn1); // 一回目の読込み
		if(++fp->seekSector > fat.sectorsPerCluster){ // 次のシークセクタが次のクラスタにまたがっていた場合の処理
			cluster = fat_func.getNCluster(fp, 1, fp->cluster);
			SDBlockRead((uint8_t*)fbuf, ((uint32_t)(cluster - 2) * fat.sectorsPerCluster + fat.userDataSector));
		} else {
			SDBlockRead((uint8_t*)fbuf, (fp->dataSector + fp->seekSector));
		}
		memcpy((uint8_t*)(buf + strn1), (uint8_t*)fbuf, strn2);
	} else {
		SDBlockRead((uint8_t*)fbuf, fp->dataSector + fp->seekSector);
		memcpy(buf, (uint8_t*)&fbuf[fp->seekBytes & 511], n);
	}

	my_fseek(fp, n, SEEK_CUR);

	return ( size == 1 ? n : (n / size) );
}


void changeDir(int id)
{
	int i, root_flag;
	static uint16_t dirLevel = 0, idxDirNameStack = 0;
	static uint8_t  dirCursorStack[32*2], // ディレクトリスタック(入れ子の深さ32)
	dirNameStack[sizeof(fat.currentDirName) * sizeof(dirCursorStack)]; // ディレクトリネームスタック
	uint32_t entryPointOffset, nextCluster;

	if(fat.currentDirEntry == fat.rootDirEntry){
		root_flag = 1;
	} else {
		root_flag = 0;
	}

	if(!setLFNname((uint8_t*)fat.currentDirName, id, LFN_WITHOUT_EXTENSION, sizeof(fat.currentDirName))){
		entryPointOffset = getListEntryPoint(id); // SFN to Multi byte arrange
		char fileName[8];

		memset(fileName, '\0', sizeof(fileName));
		strncpy((char*)fileName, (char*)&fbuf[entryPointOffset], 8); //　カレントディレクトリネーム取得
		for(i = 0;i < 8;i++){
			if(fileName[i] == '\0'){
				break;
			}
			*((uint16_t*)&fat.currentDirName[i * sizeof(uint16_t)]) = (uint16_t)fileName[i];
		}
		*((uint32_t*)&fat.currentDirName[i * sizeof(uint16_t)]) = 0x00000000;
	}

	entryPointOffset = getListEntryPoint(id); // リスト上にあるIDファイルエントリの先頭位置をセット
	if(!(fbuf[entryPointOffset + ATTRIBUTES] & _BV(ATTR_DIRECTORY))){
		return;
	}

	if(fat.fsType == FS_TYPE_FAT16){
		nextCluster = *((uint16_t*)&fbuf[entryPointOffset + CLUSTER_FAT16]); // クラスタ番号取得
	} else {
		nextCluster  = fbuf[entryPointOffset + CLUSTER_FAT32_LSB]; // フォントのクラスタ番号取得
		nextCluster |= (uint32_t)fbuf[entryPointOffset + CLUSTER_FAT32_LSB + 1] << 8;
		nextCluster |= (uint32_t)fbuf[entryPointOffset + CLUSTER_FAT32_MSB] << 16;
		nextCluster |= (uint32_t)fbuf[entryPointOffset + CLUSTER_FAT32_MSB + 1] << 24;
	}

	if(nextCluster != 0){
		fat.currentDirEntry = (uint32_t)(nextCluster - 2) * fat.sectorsPerCluster + fat.userDataSector; // カレントディレクトリエントリを更新
	} else {
		fat.currentDirEntry = fat.rootDirEntry; // 現在のディレクトリエントリをルートディレクトリエントリに
		memcpy((char*)fat.currentDirName, (char*)root_str, sizeof(root_str));
	}

	free(fat.pfileList); // ファイルリスト削除 メモリ解放

	makeFileList(); // 新しいディレクトリ内のファイルリストを作成

	if(!root_flag && id == 0){	// 移動先が親ディレクトリの場合
		dirLevel--;
		cursor.pageIdx = dirCursorStack[dirLevel * 2]; // スタックからカーソルページインデックスを取得
		cursor.pos = dirCursorStack[dirLevel * 2 + 1]; // スタックからカーソルポジションを取得

		if(nextCluster != 0){
			idxDirNameStack -= sizeof(fat.currentDirName);
			memcpy((uint8_t*)fat.currentDirName, (uint8_t*)&dirNameStack[idxDirNameStack - sizeof(fat.currentDirName)], sizeof(fat.currentDirName));
		}
	} else {			// 移動先が子ディレクトリの場合
		dirCursorStack[dirLevel * 2] = cursor.pageIdx; // スタックにカーソルページインデックスを待避
		dirCursorStack[dirLevel * 2 + 1] = cursor.pos; // スタックにカーソルポジションを待避
		dirLevel++;
		cursor.pageIdx = cursor.pos = 0;

		memcpy((uint8_t*)&dirNameStack[idxDirNameStack], (uint8_t*)fat.currentDirName, sizeof(fat.currentDirName));

		idxDirNameStack += sizeof(fat.currentDirName);
	}
}

