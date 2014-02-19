/*
 * pcf_font.c
 *
 *  Created on: 2012/03/12
 *      Author: Tonsuke
 */

#include <stdlib.h>
#include <string.h>

#include "pcf_font.h"
#include "lcd.h"
#include "fat.h"
#include "cfile.h"
#include "xpt2046.h"
#include "settings.h"
#include "usart.h"
#include "mpool.h"

uint32_t conv_b2l(void* val, size_t t){
	uint32_t ret = 0;
	size_t tc = t;
	uint8_t *pval = (uint8_t*)val;

	for(;t > 0;t--){
		ret |= pval[tc - t] << 8 * (t - 1);
	}

	return ret;
}


int PCFFontInit(int id)
{
	int i, type_idx;
	toc_entry toc;

	uint8_t buf[512];

	MY_FILE *fp = '\0';

	fp = my_fopen(id);

	if(!fp || id == -1){
		my_fclose((void*)fp);
		return -1;
	}

	my_fread(buf, 1, 4, fp);

	if(strncmp((char*)buf, "\1fcp" , 4) != 0){
		debug.printf("\r\nNot PCF File");
		my_fclose((void*)fp);
		return -1;
	}

	static char cluster_gap_flag = 0;

	if(cluster_gap_flag){
		free((void*)pcf_font.enc_tbl.fp.cache.p_cluster_gap);
		pcf_font.enc_tbl.fp.cache.p_cluster_gap = '\0';
	}

	my_fread(buf, 1, sizeof(uint32_t), fp);
	memcpy((void*)&pcf_font.table_count, buf, sizeof(uint32_t));

//	debug.printf("\r\ntable_count:%d", pcf_font.table_count);

	for(i = 0;i < pcf_font.table_count;i++){
		my_fread(buf, 1, sizeof(toc_entry), fp);
		memcpy((void*)&toc, buf, sizeof(toc_entry));
//		debug.printf("\r\n\nEntry#%d", i);
		type_idx = 0;
		do{
			if((toc.type >> type_idx) & 1){
//				debug.printf("\r\n%s", (char*)&type[type_idx][0]);
				break;
			}
		}while(++type_idx < 9);

//		debug.printf("\r\ntype:%d", toc.type);
//		debug.printf("\r\nformat:%d", toc.format);
//		debug.printf("\r\nsize:%d", toc.size);
//		debug.printf("\r\noffset:%d", toc.offset);

		switch(toc.type){
		case PCF_METRICS:
			pcf_font.metrics_tbl.size = toc.size;
			pcf_font.metrics_tbl.offset = toc.offset;
			break;
		case PCF_BITMAPS:
			pcf_font.bitmap_tbl.size = toc.size;
			pcf_font.bitmap_tbl.offset = toc.offset;
			break;
		case PCF_BDF_ENCODINGS:
			pcf_font.enc_tbl.size = toc.size;
			pcf_font.enc_tbl.offset = toc.offset;
			break;
		default:
			break;
		}

	}

//	debug.printf("\r\npcf_font.metrics_tbl.size:%d", pcf_font.metrics_tbl.size);
//	debug.printf("\r\npcf_font.metrics.offset:%d", pcf_font.metrics_tbl.offset);

//	debug.printf("\r\npcf_font.bitmap_tbl.size:%d", pcf_font.bitmap_tbl.size);
//	debug.printf("\r\npcf_font.bitmap_tbl.offset:%d", pcf_font.bitmap_tbl.offset);

//	debug.printf("\r\npcf_font.enc_tbl.size:%d", pcf_font.enc_tbl.size);
//	debug.printf("\r\npcf_font.enc_tbl.offset:%d", pcf_font.enc_tbl.offset);

	// Collect Metrics Information
	my_fseek(fp, pcf_font.metrics_tbl.offset + 6, SEEK_SET); // jump to metrics table. skip format(4bytes), metrics_count(2bytes)
	memcpy((void*)&pcf_font.metrics_tbl.fp, (void*)fp, sizeof(MY_FILE)); // copy file pointer to the metrics table.

	// Collect Encoding Information
	my_fseek(fp, pcf_font.enc_tbl.offset + 4, SEEK_SET); // jump to encoding table. skip format(4bytes)

	my_fread(buf, 1, sizeof(uint16_t), fp);
	pcf_font.enc_info.min_char_or_byte2 = conv_b2l((void*)buf, sizeof(uint16_t));

	my_fread(buf, 1, sizeof(uint16_t), fp);
	pcf_font.enc_info.max_char_or_byte2 = conv_b2l((void*)buf, sizeof(uint16_t));

	my_fread(buf, 1, sizeof(uint16_t), fp);
	pcf_font.enc_info.min_byte1 = conv_b2l((void*)buf, sizeof(uint16_t));

	my_fread(buf, 1, sizeof(uint16_t), fp);
	pcf_font.enc_info.max_byte1 = conv_b2l((void*)buf, sizeof(uint16_t));

	my_fread(buf, 1, sizeof(uint16_t), fp);
	pcf_font.enc_info.default_char = conv_b2l((void*)buf, sizeof(uint16_t));

//	debug.printf("\r\nmin_char_or_byte2:%d", pcf_font.enc_info.min_char_or_byte2);
//	debug.printf("\r\nmax_char_or_byte2:%d", pcf_font.enc_info.max_char_or_byte2);
//	debug.printf("\r\nmin_byte1:%d", pcf_font.enc_info.min_byte1);
//	debug.printf("\r\nmax_byte1:%d", pcf_font.enc_info.max_byte1);
//	debug.printf("\r\ndefault_char:%d", pcf_font.enc_info.default_char);

	pcf_font.enc_tbl.glyphindeces = (pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) * (pcf_font.enc_info.max_byte1 - pcf_font.enc_info.min_byte1 + 1);

//	debug.printf("\r\nglyphindeces:%d", pcf_font.enc_tbl.glyphindeces);

	memcpy((void*)&pcf_font.enc_tbl.fp, (void*)fp, sizeof(MY_FILE)); // copy file pointer to the encode table.

	// Collect Bitmap information
	my_fseek(fp, pcf_font.bitmap_tbl.offset, SEEK_SET); // jump to bitmap_table, skip format(4bytes), glyph_count(4bytes)
	my_fread(buf, 1, sizeof(uint32_t), fp);
//	debug.printf("\r\nformat***%08x", conv_b2l((void*)buf, sizeof(uint32_t)));
	my_fread(buf, 1, sizeof(uint32_t), fp);


	memcpy((void*)&pcf_font.bitmap_tbl.fp_offset, (void*)fp, sizeof(MY_FILE)); // copy file pointer to the bitmap offset.

	uint32_t glyph_count;
	my_fseek(fp, pcf_font.bitmap_tbl.offset + 4, SEEK_SET); // jump to bitmap_table, skip format(4bytes)
	my_fread(buf, 1, sizeof(uint32_t), fp);
	glyph_count = conv_b2l((void*)buf, sizeof(uint32_t));
	my_fseek(fp, glyph_count * sizeof(uint32_t) + sizeof(uint32_t) * 4, SEEK_CUR); // skip glyph_count * 4, bitmapSize(uint32_t * 4)
	memcpy((void*)&pcf_font.bitmap_tbl.fp_bitmap, (void*)fp, sizeof(MY_FILE)); // copy file pointer to the bitmap data.

//	debug.printf("\r\nglyph_count:%d", glyph_count);

	if(fp->cache.fragCnt > 0){
		void *new;
		new = malloc(fp->cache.fragCnt * sizeof(frag_cluster));
		memcpy(new, (void*)fp->cache.p_cluster_gap, fp->cache.fragCnt * sizeof(frag_cluster));
		pcf_font.enc_tbl.fp.cache.p_cluster_gap = new;
		pcf_font.bitmap_tbl.fp_bitmap.cache.p_cluster_gap = new;
		pcf_font.bitmap_tbl.fp_offset.cache.p_cluster_gap = new;
		pcf_font.metrics_tbl.fp.cache.p_cluster_gap = new;
		cluster_gap_flag = 1;
	}

	my_fclose(fp);

	pcf_font.c_loaded = 0;
	pcf_font.metrics.hSpacing = PCF_METRICS_DEFAULT_HSPACING;

	return 0;
}

void PCFPutChar(uint16_t code, colors color)
{
	int i, j, k, misc;
	int double_size = 0;
	uint32_t tmp, *p_u32;
	uint64_t bitmap_data_64;
	uint16_t bg_ram[13][13];

	uint8_t buf[512], enc1, enc2;
	uint8_t glyph_samples[48][12];

	MY_FILE fpTmp;

	pixel_fmt_typedef pixel_fg, pixel_bg;
	float alpha_ratio;

	// Get the glyph_index from UTF16 code.
	uint16_t glyph_index;
	enc1 = (uint8_t)(code >> 8);
	enc2 = (uint8_t)code;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return;
	}

	// グリフの位置を計算
	tmp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
			(pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
			enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(MY_FILE));
	my_fseek(&fpTmp, tmp, SEEK_CUR);
	my_fread(buf, 1, sizeof(uint16_t), &fpTmp);
//	glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

	if(glyph_index == 0xFFFF) {
		return;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(MY_FILE)); //
	my_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	my_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	// xor 0x80 each metric(subtract 0x80)
	*((uint32_t*)&metric.left_sided_bearing) ^= 0x80808080;
//	metric.left_sided_bearing  ^= (1 << 7);
//	metric.right_sided_bearing ^= (1 << 7);
//	metric.character_width     ^= (1 << 7);
//	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);

	// Get the bitmap data offset
	uint32_t bitmap_offset;
	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_offset, sizeof(MY_FILE));
	my_fseek(&fpTmp, glyph_index * sizeof(int32_t), SEEK_CUR); // グリフインデックスが指すビットマップオフセットまでファイルポインタを移動
	my_fread(buf, 1, sizeof(int32_t), &fpTmp);
//	bitmap_offset = conv_b2l((void*)buf, sizeof(int32_t)); // ビットマップオフセット
	bitmap_offset = __REV(*(uint32_t*)buf);

	if(metric.right_sided_bearing - metric.left_sided_bearing > 32){ // 右ベアリング - 左ベアリングが32pxを超えていたらダブルサイズ有効
		double_size = 1;
	}

	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_bitmap, sizeof(MY_FILE)); // ビットマップデータまでファイルポインタを移動
	my_fseek(&fpTmp, bitmap_offset, SEEK_CUR);

	// バッファにビットマップデータをキャッシュする
	my_fread((void*)buf, 1, ((metric.character_ascent + metric.character_descent) * \
			sizeof(uint32_t)) * (double_size == 0 ? 1 : 2), &fpTmp); 	// ビットマップデータが32bitを超える場合は2倍

	memset((void*)glyph_samples, 0, sizeof(glyph_samples));

	p_u32 = (uint32_t*)buf;
	for(i = 0;i < metric.character_ascent + metric.character_descent;i++){
		bitmap_data_64 = __REV(*p_u32++);
		/*
		tmp = *((uint32_t*)&buf[misc]);
		misc += 4;
		bitmap_data = tmp;
		bitmap_data <<= 8;
		bitmap_data |= (uint8_t)(tmp >> 8);
		bitmap_data <<= 8;
		bitmap_data |= (uint8_t)(tmp >> 16);
		bitmap_data <<= 8;
		bitmap_data |= (uint8_t)(tmp >> 24);
		bitmap_data_64 = bitmap_data;
		*/

		if(double_size){
			/*
			tmp = *((uint32_t*)&buf[misc]);
			misc += 4;
			bitmap_data = tmp;
			bitmap_data <<= 8;
			bitmap_data |= (uint8_t)(tmp >> 8);
			bitmap_data <<= 8;
			bitmap_data |= (uint8_t)(tmp >> 16);
			bitmap_data <<= 8;
			bitmap_data |= (uint8_t)(tmp >> 24);
			*/
			bitmap_data_64 <<= 32;
			bitmap_data_64 |= __REV(*p_u32++);;
			bitmap_data_64 >>= 16; // 64bit - 48bit = 16bit  余白を右に詰める
		} else {
			bitmap_data_64 <<= 16; // 48bit - 32bit = 16bit
		}

		for(j = 1;j <= 12;j++){
			glyph_samples[i][12 - j] = bit_count_table[bitmap_data_64 & 0xf]; // 4bitずつサンプルする
			bitmap_data_64 >>= 4;
		}
	}

	misc = 11 - (int)((float)(metric.character_ascent * 0.25f) + 0.5f); // グリフの高さ揃えパラメータ
	/*
	fypos = 11.0f - (float)(metric.character_ascent / 4.0f); // グリフの高さ揃えパラメータ
	misc = (int)fypos;
	ratioX0 = 1.0f;
	ratioX1 = 1.0f - ratioX0;
	ratioY0 = 1.0f - (fypos - misc);
	ratioY1 = 1.0f - ratioY0;

	debug.printf("\r\n\nratioY0:%f ratioY1:%f", ratioY0, ratioY1);
	debug.printf("\r\nftypos:%f misc:%d", fypos, misc);
*/
	for(k = 0;k < 12;k++){ // 背景データをRAMに格納
		tmp = k + cly + misc;
		LCDSetGramAddr(clx, tmp < LCD_HEIGHT ? tmp : LCD_HEIGHT - 1);
		LCDPutCmd(0x0022);
		LCD->RAM; // dummy read
		for(j = 0;j < 13;j++){
			bg_ram[k][j] = LCD->RAM;
		}
	}


	for(k = 0;k < 12;k++){ // オーバーサンプリングしたデータと背景データを合成して描画
		tmp = k + cly + misc;
		LCDSetGramAddr(clx, tmp < LCD_HEIGHT ? tmp : LCD_HEIGHT - 1);
		LCDPutCmd(0x0022);
		for(j = 0;j < 12;j++){
			// サブピクセルのドット合計を求める
			alpha_ratio = color_tone_table_4bit[glyph_samples[k * 4 + 0][j] + \
			                                    glyph_samples[k * 4 + 1][j] + \
			                                    glyph_samples[k * 4 + 2][j] + \
			                                    glyph_samples[k * 4 + 3][j]]; // アルファ率を取得

			// Foreground Image
			pixel_fg.color.d16 = colorc[color];
			pixel_fg.color.R *= alpha_ratio;
			pixel_fg.color.G *= alpha_ratio;
			pixel_fg.color.B *= alpha_ratio;
/*
			pixel_fgBox[0][0].color.d16 = colorc[color];
			pixel_fgBox[1][0].color.d16 = colorc[color];
			pixel_fgBox[0][1].color.d16 = colorc[color];
			pixel_fgBox[1][1].color.d16 = colorc[color];

			pixel_fgBox[0][0].color.R *= alpha_ratio;
			pixel_fgBox[0][0].color.G *= alpha_ratio;
			pixel_fgBox[0][0].color.B *= alpha_ratio;

			pixel_fgBox[1][0].color.R *= alpha_ratio;
			pixel_fgBox[1][0].color.G *= alpha_ratio;
			pixel_fgBox[1][0].color.B *= alpha_ratio;

			pixel_fgBox[0][1].color.R *= alpha_ratio;
			pixel_fgBox[0][1].color.G *= alpha_ratio;
			pixel_fgBox[0][1].color.B *= alpha_ratio;

			pixel_fgBox[1][1].color.R *= alpha_ratio;
			pixel_fgBox[1][1].color.G *= alpha_ratio;
			pixel_fgBox[1][1].color.B *= alpha_ratio;
*/

			// Background Image
			pixel_bg.color.d16 = bg_ram[k][j];
			pixel_bg.color.R *= (1.0f - alpha_ratio);
			pixel_bg.color.G *= (1.0f - alpha_ratio);
			pixel_bg.color.B *= (1.0f - alpha_ratio);
			/*
			pixel_bgBox[0][0].color.d16 = bg_ram[k + 0][j + 0];
			pixel_bgBox[1][0].color.d16 = bg_ram[k + 1][j + 0];
			pixel_bgBox[0][1].color.d16 = bg_ram[k + 0][j + 1];
			pixel_bgBox[1][1].color.d16 = bg_ram[k + 1][j + 1];

			pixel_bgBox[0][0].color.R *= (1.0f - alpha_ratio);
			pixel_bgBox[0][0].color.G *= (1.0f - alpha_ratio);
			pixel_bgBox[0][0].color.B *= (1.0f - alpha_ratio);

			pixel_bgBox[1][0].color.R *= (1.0f - alpha_ratio);
			pixel_bgBox[1][0].color.G *= (1.0f - alpha_ratio);
			pixel_bgBox[1][0].color.B *= (1.0f - alpha_ratio);

			pixel_bgBox[0][1].color.R *= (1.0f - alpha_ratio);
			pixel_bgBox[0][1].color.G *= (1.0f - alpha_ratio);
			pixel_bgBox[0][1].color.B *= (1.0f - alpha_ratio);

			pixel_bgBox[1][0].color.R *= (1.0f - alpha_ratio);
			pixel_bgBox[1][0].color.G *= (1.0f - alpha_ratio);
			pixel_bgBox[1][0].color.B *= (1.0f - alpha_ratio);
*/

			// Add colors
			pixel_fg.color.R += pixel_bg.color.R;
			pixel_fg.color.G += pixel_bg.color.G;
			pixel_fg.color.B += pixel_bg.color.B;
/*
			pixel_fgBox[0][0].color.R += pixel_bgBox[0][0].color.R;
			pixel_fgBox[0][0].color.G += pixel_bgBox[0][0].color.G;
			pixel_fgBox[0][0].color.B += pixel_bgBox[0][0].color.B;

			pixel_fgBox[1][0].color.R += pixel_bgBox[1][0].color.R;
			pixel_fgBox[1][0].color.G += pixel_bgBox[1][0].color.G;
			pixel_fgBox[1][0].color.B += pixel_bgBox[1][0].color.B;

			pixel_fgBox[0][1].color.R += pixel_bgBox[0][1].color.R;
			pixel_fgBox[0][1].color.G += pixel_bgBox[0][1].color.G;
			pixel_fgBox[0][1].color.B += pixel_bgBox[0][1].color.B;

			pixel_fgBox[1][1].color.R += pixel_bgBox[1][1].color.R;
			pixel_fgBox[1][1].color.G += pixel_bgBox[1][1].color.G;
			pixel_fgBox[1][1].color.B += pixel_bgBox[1][1].color.B;


			pixelV1.color.R = __USAT(pixel_fgBox[0][0].color.R * ratioX0 + pixel_fgBox[1][0].color.R * ratioX1, 5);
			pixelV1.color.G = __USAT(pixel_fgBox[0][0].color.G * ratioX0 + pixel_fgBox[1][0].color.G * ratioX1, 6);
			pixelV1.color.B = __USAT(pixel_fgBox[0][0].color.B * ratioX0 + pixel_fgBox[1][0].color.B * ratioX1, 5);

			pixelV2.color.R = __USAT(pixel_fgBox[0][1].color.R * ratioX0 + pixel_fgBox[1][1].color.R * ratioX1, 5);
			pixelV2.color.G = __USAT(pixel_fgBox[0][1].color.G * ratioX0 + pixel_fgBox[1][1].color.G * ratioX1, 6);
			pixelV2.color.B = __USAT(pixel_fgBox[0][1].color.B * ratioX0 + pixel_fgBox[1][1].color.B * ratioX1, 5);

			pixel.color.R = __USAT(pixelV1.color.R * ratioY0 + pixelV2.color.R * ratioY1, 5);
			pixel.color.G = __USAT(pixelV1.color.G * ratioY0 + pixelV2.color.G * ratioY1, 6);
			pixel.color.B = __USAT(pixelV1.color.B * ratioY0 + pixelV2.color.B * ratioY1, 5);
			LCDPutData(pixel.color.d16);
*/

			LCDPutData(pixel_fg.color.d16);
		}
	}
	clx += ((metric.right_sided_bearing - metric.left_sided_bearing) >> 2) + pcf_font.metrics.hSpacing;
	if(code == 0x20 || code == 0x3000){
		clx += 2;
	}
}

void PCFPutChar16px(uint16_t code, colors color)
{
	int i, j, k, misc;
	int double_size = 0;
	uint32_t tmp, *p_u32;
	uint64_t bitmap_data_64;
	uint16_t bg_ram[16][16];

	uint8_t buf[512], enc1, enc2;
	uint8_t glyph_samples[48][16];
	MY_FILE fpTmp;

	pixel_fmt_typedef pixel_fg, pixel_bg;
	float alpha_ratio;

	// Get the glyph_index from UTF16 code.
	uint16_t glyph_index;
	enc1 = (uint8_t)(code >> 8);
	enc2 = (uint8_t)code;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return;
	}

	// グリフの位置を計算
	tmp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
			 (pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
		      enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(MY_FILE));
	my_fseek(&fpTmp, tmp, SEEK_CUR);
	my_fread(buf, 1, sizeof(uint16_t), &fpTmp);
//	glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

	if(glyph_index == 0xFFFF) {
		return;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(MY_FILE)); //
	my_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	my_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	// xor 0x80 each metric(subtract 0x80)
	*(uint32_t*)&metric.left_sided_bearing ^= 0x80808080;
//	metric.left_sided_bearing  ^= (1 << 7);
//	metric.right_sided_bearing ^= (1 << 7);
//	metric.character_width     ^= (1 << 7);
//	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);

	// Get the bitmap data offset
	uint32_t bitmap_offset;
	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_offset, sizeof(MY_FILE));
	my_fseek(&fpTmp, glyph_index * sizeof(int32_t), SEEK_CUR); // グリフインデックスが指すビットマップオフセットまでファイルポインタを移動
	my_fread(buf, 1, sizeof(int32_t), &fpTmp);
//	bitmap_offset = conv_b2l((void*)buf, sizeof(int32_t)); // ビットマップオフセット
	bitmap_offset = __REV(*(uint32_t*)buf);

	if(metric.right_sided_bearing - metric.left_sided_bearing > 32){ // 右ベアリング - 左ベアリングが32pxを超えていたらダブルサイズ有効
		double_size = 1;
	}

	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_bitmap, sizeof(MY_FILE)); // ビットマップデータまでファイルポインタを移動
	my_fseek(&fpTmp, bitmap_offset, SEEK_CUR);

	// バッファにビットマップデータをキャッシュする
	my_fread((void*)buf, 1, ((metric.character_ascent + metric.character_descent) * \
			sizeof(uint32_t)) * (double_size == 0 ? 1 : 2), &fpTmp); 	// ビットマップデータが32bitを超える場合は2倍

	memset((void*)glyph_samples, 0, sizeof(glyph_samples));

	p_u32 = (uint32_t*)buf;
	for(i = 0;i < metric.character_ascent + metric.character_descent;i++){
		bitmap_data_64 = __REV(*p_u32++);
		if(double_size){
			bitmap_data_64 <<= 32;
			bitmap_data_64 |= __REV(*p_u32++);;
			bitmap_data_64 >>= 16; // 64bit - 48bit = 16bit  余白を右に詰める
		} else {
			bitmap_data_64 <<= 16; // 48bit - 32bit = 16bit
		}

		for(j = 1;j <= 16;j++){
			glyph_samples[i][16 - j] = bit_count_table[bitmap_data_64 & 0x7]; // 3bitずつサンプルする
			bitmap_data_64 >>= 3;
		}
	}

	misc = 15 - (int)((float)metric.character_ascent * 0.333f + 0.5f); // グリフの高さ揃えパラメータ

	for(k = 0;k < 16;k++){ // 背景データをRAMに格納
		tmp = k + cly + misc;
		LCDSetGramAddr(clx, tmp < LCD_HEIGHT ? tmp : LCD_HEIGHT - 1);
		LCDPutCmd(0x0022);
		LCD->RAM; // dummy read
		for(j = 0;j < 16;j++){
			bg_ram[k][j] = LCD->RAM;
		}
	}

	for(k = 0;k < 16;k++){ // オーバーサンプリングしたデータと背景データを合成して描画
		tmp = k + cly + misc;
		LCDSetGramAddr(clx, tmp < LCD_HEIGHT ? tmp : LCD_HEIGHT - 1);
		LCDPutCmd(0x0022);
		for(j = 0;j < 16;j++){
			// サブピクセルのドット合計を求める
			alpha_ratio = color_tone_table_3bit[glyph_samples[k * 3 + 0][j] + \
			                                    glyph_samples[k * 3 + 1][j] + \
			                                    glyph_samples[k * 3 + 2][j]]; // アルファ率を取得

			// Foreground Image
			pixel_fg.color.d16 = colorc[color];
			pixel_fg.color.R *= alpha_ratio;
			pixel_fg.color.G *= alpha_ratio;
			pixel_fg.color.B *= alpha_ratio;

			// Background Image
			pixel_bg.color.d16 = bg_ram[k][j];
			pixel_bg.color.R *= (1.0f - alpha_ratio);
			pixel_bg.color.G *= (1.0f - alpha_ratio);
			pixel_bg.color.B *= (1.0f - alpha_ratio);

			// Add colors
			pixel_fg.color.R += pixel_bg.color.R;
			pixel_fg.color.G += pixel_bg.color.G;
			pixel_fg.color.B += pixel_bg.color.B;

			LCDPutData(pixel_fg.color.d16);
		}
	}
	clx += ((metric.right_sided_bearing - metric.left_sided_bearing) * 0.333f + pcf_font.metrics.hSpacing);
	if(code == 0x20 || code == 0x3000){
		clx += 5;
	}
}

void PCFSetGlyphCacheStartAddress(void *addr){
	pcf_font.cache.start_address = addr;
	pcf_font.cache.glyph_count = 0;
}

void PCFCachePlayTimeGlyphs(uint8_t px){
	unsigned char c;

	for(c = '0';c <= '9';c++){
		PCFCacheGlyph(c, px);
	}
	PCFCacheGlyph('-', px);
	PCFCacheGlyph(':', px);
}

void PCFCacheGlyph(uint16_t code, uint16_t font_width)
{
	int i, j, k;
	int double_size = 0;
	uint32_t tmp, *p_u32;
	uint64_t bitmap_data_64;

	uint8_t buf[512], enc1, enc2;
	uint8_t glyph_samples[48][16];

	MY_FILE fpTmp;

	// Get the glyph_index from UTF16 code.
	uint16_t glyph_index;
	enc1 = (uint8_t)(code >> 8);
	enc2 = (uint8_t)code;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return;
	}

	// グリフの位置を計算
	tmp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
			(pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
			enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(MY_FILE));
	my_fseek(&fpTmp, tmp, SEEK_CUR);
	my_fread(buf, 1, sizeof(uint16_t), &fpTmp);
//	glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

	if(glyph_index == 0xFFFF) {
		return;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(MY_FILE)); //
	my_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	my_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	// xor 0x80 each metric(subtract 0x80)
	*(uint32_t*)&metric.left_sided_bearing ^= 0x80808080;
//	metric.left_sided_bearing  ^= (1 << 7);
//	metric.right_sided_bearing ^= (1 << 7);
//	metric.character_width     ^= (1 << 7);
//	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);

	// Get the bitmap data offset
	uint32_t bitmap_offset;
	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_offset, sizeof(MY_FILE));
	my_fseek(&fpTmp, glyph_index * sizeof(int32_t), SEEK_CUR); // グリフインデックスが指すビットマップオフセットまでファイルポインタを移動
	my_fread(buf, 1, sizeof(int32_t), &fpTmp);
//	bitmap_offset = conv_b2l((void*)buf, sizeof(int32_t)); // ビットマップオフセット
	bitmap_offset = __REV(*(uint32_t*)buf);

	if(metric.right_sided_bearing - metric.left_sided_bearing > 32){ // 右ベアリング - 左ベアリングが32pxを超えていたらダブルサイズ有効
		double_size = 1;
	}

	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_bitmap, sizeof(MY_FILE)); // ビットマップデータまでファイルポインタを移動
	my_fseek(&fpTmp, bitmap_offset, SEEK_CUR);

	// バッファにビットマップデータをキャッシュする
	my_fread((void*)buf, 1, ((metric.character_ascent + metric.character_descent) * \
			sizeof(uint32_t)) * (double_size == 0 ? 1 : 2), &fpTmp); 	// ビットマップデータが32bitを超える場合は2倍

	memset((void*)glyph_samples, 0, sizeof(glyph_samples));

	p_u32 = (uint32_t*)buf;
	for(i = 0;i < metric.character_ascent + metric.character_descent;i++){
		bitmap_data_64 = __REV(*p_u32++);
		if(double_size){
			bitmap_data_64 <<= 32;
			bitmap_data_64 |= __REV(*p_u32++);
			bitmap_data_64 >>= 16; // 64bit - 48bit = 16bit  余白を右に詰める
		} else {
			bitmap_data_64 <<= 16; // 48bit - 32bit = 16bit
		}

		for(j = 1;j <= (font_width < 16 ? 12 : 16);j++){
			glyph_samples[i][(font_width < 16 ? 12 : 16) - j] = bit_count_table[bitmap_data_64 & (font_width < 16 ? 0xf : 0x3)]; // 4bitずつサンプルする
			bitmap_data_64 >>= (font_width < 16 ? 4 : 3);
		}
	}

	pcf_glyph_cache_head_typedef glyph_cache_head;
	void *cache_addr = pcf_font.cache.start_address;
	for(i = 0;i < pcf_font.cache.glyph_count;i++){
		memcpy(&glyph_cache_head, cache_addr, sizeof(pcf_glyph_cache_head_typedef));
		cache_addr += (glyph_cache_head.size + sizeof(glyph_cache_head));
	}
	glyph_cache_head.code = code;
	glyph_cache_head.width = font_width;
	glyph_cache_head.size = (font_width < 16 ? 12 * 12 : 16 * 16) * sizeof(float);

	memcpy(&glyph_cache_head.metric, &metric, sizeof(metric_data_typedef));
	memcpy(cache_addr, &glyph_cache_head, sizeof(pcf_glyph_cache_head_typedef));

	float *glyph_alpha = (float*)(cache_addr + sizeof(glyph_cache_head));

	for(k = 0;k < (font_width < 16 ? 12 : 16);k++){ // オーバーサンプリングしたデータと背景データを合成して描画
		for(j = 0;j < (font_width < 16 ? 12 : 16);j++){
			// サブピクセルのドット合計を求める
			if(font_width < 16){
				*glyph_alpha++ = color_tone_table_4bit[glyph_samples[k * 4 + 0][j] + \
				                                       glyph_samples[k * 4 + 1][j] + \
				                                       glyph_samples[k * 4 + 2][j] + \
				                                       glyph_samples[k * 4 + 3][j]]; // アルファ率を取得
			} else {
				*glyph_alpha++ = color_tone_table_3bit[glyph_samples[k * 3 + 0][j] + \
				                                       glyph_samples[k * 3 + 1][j] + \
				                                       glyph_samples[k * 3 + 2][j]]; // アルファ率を取得
			}
		}
	}
	pcf_font.cache.glyph_count++;
}

void PCFPutCharCache(uint16_t code, colors color)
{
	int i, j, k, misc;
	float alpha_ratio;

	uint8_t enc1, enc2;
	uint16_t bg_ram[16][16], font_width;

	pixel_fmt_typedef pixel_fg, pixel_bg;

	uint32_t tmp;

	// Get the glyph_index from UTF16 code.
	enc1 = (code >> 8) & 0xff;
	enc2 = code & 0xff;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;

	pcf_glyph_cache_head_typedef glyph_cache_head;
	void *cache_addr = pcf_font.cache.start_address;
	for(i = 0;i < pcf_font.cache.glyph_count;i++){
		memcpy(&glyph_cache_head, cache_addr, sizeof(pcf_glyph_cache_head_typedef));
		if(glyph_cache_head.code == code){
			break;
		}
		cache_addr += (glyph_cache_head.size + sizeof(glyph_cache_head));
	}

	font_width = glyph_cache_head.width;
	memcpy(&metric, &glyph_cache_head.metric, sizeof(metric_data_typedef));
	float *glyph_alpha = (float*)(cache_addr + sizeof(glyph_cache_head));

	misc = 11 - (int)((float)metric.character_ascent * (font_width < 16 ? 0.25f : 0.333f) + 0.5f); // グリフの高さ揃えパラメータ

	for(k = 0;k < (font_width < 16 ? 12 : 16);k++){ // 背景データをRAMに格納
		tmp = k + cly + misc;
		LCDSetGramAddr(clx, tmp < LCD_HEIGHT ? tmp : LCD_HEIGHT - 1);
		LCDPutCmd(0x0022);
		LCD->RAM; // dummy read
		for(j = 0;j < (font_width < 16 ? 12 : 16);j++){
			bg_ram[k][j] = LCD->RAM;
		}
	}

	for(k = 0;k < (font_width < 16 ? 12 : 16);k++){ // オーバーサンプリングしたデータと背景データを合成して描画
		tmp = k + cly + misc;
		LCDSetGramAddr(clx, tmp < LCD_HEIGHT ? tmp : LCD_HEIGHT - 1);
		LCDPutCmd(0x0022);
		for(j = 0;j < (font_width < 16 ? 12 : 16);j++){
			alpha_ratio = *glyph_alpha++;

			// Foreground Image
			pixel_fg.color.d16 = colorc[color];
			pixel_fg.color.R *= alpha_ratio;
			pixel_fg.color.G *= alpha_ratio;
			pixel_fg.color.B *= alpha_ratio;

			// Background Image
			pixel_bg.color.d16 = bg_ram[k][j];
			pixel_bg.color.R *= (1.0f - alpha_ratio);
			pixel_bg.color.G *= (1.0f - alpha_ratio);
			pixel_bg.color.B *= (1.0f - alpha_ratio);

			// Add colors
			pixel_fg.color.R += pixel_bg.color.R;
			pixel_fg.color.G += pixel_bg.color.G;
			pixel_fg.color.B += pixel_bg.color.B;

			LCDPutData(pixel_fg.color.d16);
		}
	}
	clx += (metric.right_sided_bearing - metric.left_sided_bearing) * (font_width < 16 ? 0.25f : 0.333f) + pcf_font.metrics.hSpacing;
}

void PCFPutString(const uint16_t *uni_str, int n, colors color)
{
//	int offset = 0;
	while(n-- > 0){
		PCFPutChar(*uni_str++, color);
	}
}


uint16_t PCFGetCharPixelLength(uint16_t code, uint16_t font_width)
{
	uint32_t tmp;
	uint16_t len;
	uint8_t buf[512], enc1, enc2;

	MY_FILE fpTmp;

	// Get the glyph_index from UTF16 code.
	uint16_t glyph_index;
	enc1 = (uint8_t)(code >> 8);
	enc2 = (uint8_t)code;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return 0;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return 0;
	}

	// グリフの位置を計算
	tmp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
			(pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
			enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(MY_FILE));
	my_fseek(&fpTmp, tmp, SEEK_CUR);
	my_fread(buf, 1, sizeof(uint16_t), &fpTmp);
//	glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

	if(glyph_index == 0xFFFF) {
		return 0;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(MY_FILE)); //
	my_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	my_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	// xor 0x80 each metric(subtract 0x80)
	*((uint32_t*)&metric.left_sided_bearing) ^= 0x80808080;
//	metric.left_sided_bearing  ^= (1 << 7);
//	metric.right_sided_bearing ^= (1 << 7);
//	metric.character_width     ^= (1 << 7);
//	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);

	len = (metric.right_sided_bearing - metric.left_sided_bearing) * (font_width < 16 ? 0.25f : 0.333f) + pcf_font.metrics.hSpacing;

	if(code == 0x20 || code == 0x3000){
		len += font_width < 16 ? 2 : 5;
	}

	return len;
}



int C_PCFFontInit(uint32_t fileAddr, size_t fileSize)
{
	uint8_t buf[512];
	C_FILE *fp = '\0';
	toc_entry toc;
	int i, type_idx;

	fp = c_fopen(fileAddr, fileSize);

	c_fread(buf, 1, 4, fp);

	if(strncmp(buf, "\1fcp" , 4) != 0){
		debug.printf("\r\nNot PCF File");
		c_fclose((void*)fp);
		return -1;
	}

	c_fread(buf, 1, sizeof(uint32_t), fp);
	memcpy((void*)&pcf_font.table_count, buf, sizeof(uint32_t));

	debug.printf("\r\ntable_count:%d", pcf_font.table_count);

	for(i = 0;i < pcf_font.table_count;i++){
		c_fread(buf, 1, sizeof(toc_entry), fp);
		memcpy((void*)&toc, buf, sizeof(toc_entry));
		debug.printf("\r\n\nEntry#%d", i);
		type_idx = 0;
		do{
			if((toc.type >> type_idx) & 1){
				debug.printf("\r\n%s", (char*)&type[type_idx][0]);
				break;
			}
		}while(++type_idx < 9);

		debug.printf("\r\ntype:%d", toc.type);
		debug.printf("\r\nformat:%d", toc.format);
		debug.printf("\r\nsize:%d", toc.size);
		debug.printf("\r\noffset:%d", toc.offset);

		switch(toc.type){
            case PCF_METRICS:
                pcf_font.metrics_tbl.size = toc.size;
                pcf_font.metrics_tbl.offset = toc.offset;
                break;
            case PCF_BITMAPS:
                pcf_font.bitmap_tbl.size = toc.size;
                pcf_font.bitmap_tbl.offset = toc.offset;
                break;
            case PCF_BDF_ENCODINGS:
                pcf_font.enc_tbl.size = toc.size;
                pcf_font.enc_tbl.offset = toc.offset;
                break;
            default:
                break;
		}

	}

	debug.printf("\r\npcf_font.metrics_tbl.size:%d", pcf_font.metrics_tbl.size);
	debug.printf("\r\npcf_font.metrics.offset:%d", pcf_font.metrics_tbl.offset);

	debug.printf("\r\npcf_font.bitmap_tbl.size:%d", pcf_font.bitmap_tbl.size);
	debug.printf("\r\npcf_font.bitmap_tbl.offset:%d", pcf_font.bitmap_tbl.offset);

	debug.printf("\r\npcf_font.enc_tbl.size:%d", pcf_font.enc_tbl.size);
	debug.printf("\r\npcf_font.enc_tbl.offset:%d", pcf_font.enc_tbl.offset);

	// Collect Metrics Information
	c_fseek(fp, pcf_font.metrics_tbl.offset + 6, SEEK_SET); // jump to metrics table. skip format(4bytes), metrics_count(2bytes)
	memcpy((void*)&pcf_font.metrics_tbl.fp, (void*)fp, sizeof(C_FILE)); // copy file pointer to the metrics table.

	// Collect Encoding Information
	c_fseek(fp, pcf_font.enc_tbl.offset + 4, SEEK_SET); // jump to encoding table. skip format(4bytes)

	c_fread(buf, 1, sizeof(uint16_t), fp);
    //	pcf_font.enc_info.min_char_or_byte2 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.min_char_or_byte2 = __REV16(*(uint16_t*)buf);


	c_fread(buf, 1, sizeof(uint16_t), fp);
    //	pcf_font.enc_info.max_char_or_byte2 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.max_char_or_byte2 = __REV16(*(uint16_t*)buf);

	c_fread(buf, 1, sizeof(uint16_t), fp);
    //	pcf_font.enc_info.min_byte1 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.min_byte1 = __REV16(*(uint16_t*)buf);

	c_fread(buf, 1, sizeof(uint16_t), fp);
    //	pcf_font.enc_info.max_byte1 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.max_byte1 = __REV16(*(uint16_t*)buf);

	c_fread(buf, 1, sizeof(uint16_t), fp);
    //	pcf_font.enc_info.default_char = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.default_char = __REV16(*(uint16_t*)buf);

	debug.printf("\r\nmin_char_or_byte2:%d", pcf_font.enc_info.min_char_or_byte2);
	debug.printf("\r\nmax_char_or_byte2:%d", pcf_font.enc_info.max_char_or_byte2);
	debug.printf("\r\nmin_byte1:%d", pcf_font.enc_info.min_byte1);
	debug.printf("\r\nmax_byte1:%d", pcf_font.enc_info.max_byte1);
	debug.printf("\r\ndefault_char:%d", pcf_font.enc_info.default_char);

	pcf_font.enc_tbl.glyphindeces = (pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) * (pcf_font.enc_info.max_byte1 - pcf_font.enc_info.min_byte1 + 1);

	debug.printf("\r\nglyphindeces:%d", pcf_font.enc_tbl.glyphindeces);

	memcpy((void*)&pcf_font.enc_tbl.fp, (void*)fp, sizeof(C_FILE)); // copy file pointer to the encode table.

	// Collect Bitmap information
    //	c_fseek(fp, pcf_font.bitmap_tbl.offset + 8, SEEK_SET); // jump to bitmap_table, skip format(4bytes), glyph_count(4bytes)
	c_fseek(fp, pcf_font.bitmap_tbl.offset, SEEK_SET); // jump to bitmap_table, skip format(4bytes), glyph_count(4bytes)
	c_fread(buf, 1, sizeof(uint32_t), fp);
    //	debug.printf("\r\nformat***%08x", conv_b2l((void*)buf, sizeof(uint32_t)));
	debug.printf("\r\nformat***%08x", __REV(*(uint32_t*)buf));
	c_fread(buf, 1, sizeof(uint32_t), fp);


	memcpy((void*)&pcf_font.bitmap_tbl.fp_offset, (void*)fp, sizeof(C_FILE)); // copy file pointer to the bitmap offset.

	uint32_t glyph_count;
	c_fseek(fp, pcf_font.bitmap_tbl.offset + 4, SEEK_SET); // jump to bitmap_table, skip format(4bytes)
	c_fread(buf, 1, sizeof(uint32_t), fp);
    //	glyph_count = conv_b2l((void*)buf, sizeof(uint32_t));
	glyph_count = __REV(*(uint32_t*)buf);
	c_fseek(fp, glyph_count * sizeof(uint32_t) + sizeof(uint32_t) * 4, SEEK_CUR); // skip glyph_count * 4, bitmapSize(uint32_t * 4)
	memcpy((void*)&pcf_font.bitmap_tbl.fp_bitmap, (void*)fp, sizeof(C_FILE)); // copy file pointer to the bitmap data.

	debug.printf("\r\nglyph_count:%d", glyph_count);

	c_fclose(fp);
	pcf_font.c_loaded = 1;
	pcf_font.metrics.hSpacing = PCF_METRICS_DEFAULT_HSPACING;

	return 0;
}



void C_PCFPutChar(uint16_t code, colors color)
{
	uint64_t bitmap_data_64;
	uint32_t bitmap_data, tmp, *p_u32;
	int i, j, k, misc, double_size = 0;
	uint16_t *p_u16, glyph_index;
	uint16_t bg_ram[13][13];

	uint8_t buf[512], glyph_samples[48][12];
	uint8_t enc1, enc2;

	C_FILE fpTmp;

	pixel_fmt_typedef pixel_fg, pixel_bg;
	float alpha_ratio;

	// Get the glyph_index from UTF16 code.
	enc1 = (code >> 8) & 0xff;
	enc2 = code & 0xff;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return;
	}

	// グリフの位置を計算
	tmp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
             (pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
             enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(C_FILE));
	c_fseek(&fpTmp, tmp, SEEK_CUR);
	c_fread(buf, 1, sizeof(uint16_t), &fpTmp);
	//glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

    //	debug.printf("\r\n\nGlyph Index");
    //	debug.printf("\r\nglyph index:0x%02x", glyph_index);
	if(glyph_index == 0xFFFF) {
		return;
        //		code = 0x25A1; // 0x25A1;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(C_FILE)); //
	c_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	c_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	*(uint32_t*)&metric.left_sided_bearing ^= 0x80808080;
    //	metric.left_sided_bearing  ^= (1 << 7);
    //	metric.right_sided_bearing ^= (1 << 7);
    //	metric.character_width     ^= (1 << 7);
    //	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);
    /*
     debug.printf("\r\n\nMetric data code:%04x", code);
     debug.printf("\r\nmetric.left_sided_bearing:%d", metric.left_sided_bearing);
     debug.printf("\r\nmetric.right_sided_bearing:%d", metric.right_sided_bearing);
     debug.printf("\r\nmetric.character_width:%d", metric.character_width);
     debug.printf("\r\nmetric.character_ascent:%d", metric.character_ascent);
     debug.printf("\r\nmetric.character_descent:%d", metric.character_descent);
     */
	// Get the bitmap data offset
	uint32_t bitmap_offset;
	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_offset, sizeof(C_FILE));
	c_fseek(&fpTmp, glyph_index * sizeof(int32_t), SEEK_CUR); // グリフインデックスが指すビットマップオフセットまでファイルポインタを移動
	c_fread(buf, 1, sizeof(int32_t), &fpTmp);
    //	bitmap_offset = conv_b2l((void*)buf, sizeof(int32_t)); // ビットマップオフセット
	bitmap_offset = __REV(*(uint32_t*)buf);

    //	debug.printf("\r\n\nBitmap data offset");
    //	debug.printf("\r\nglyph offset:0x%04x", bitmap_offset);

	if(metric.right_sided_bearing - metric.left_sided_bearing > 32){ // 右ベアリング - 左ベアリングが32pxを超えていたらダブルサイズ有効
		double_size = 1;
	}

	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_bitmap, sizeof(C_FILE)); // ビットマップデータまでファイルポインタを移動
	c_fseek(&fpTmp, bitmap_offset, SEEK_CUR);

	// バッファにビットマップデータをキャッシュする
	c_fread((void*)buf, 1, ((metric.character_ascent + metric.character_descent) * \
                             sizeof(uint32_t)) * (double_size == 0 ? 1 : 2), &fpTmp); 	// ビットマップデータが32bitを超える場合は2倍

	memset((void*)glyph_samples, 0, sizeof(glyph_samples));

	p_u32 = (uint32_t*)buf;
	for(i = 0;i < metric.character_ascent + metric.character_descent;i++){
		bitmap_data_64 = __REV(*p_u32++);
		if(double_size){
			bitmap_data_64 <<= 32;
			bitmap_data_64 |= __REV(*p_u32++);;
			bitmap_data_64 >>= 16; // 64bit - 48bit = 16bit  余白を右に詰める
		} else {
			bitmap_data_64 <<= 16; // 48bit - 32bit = 16bit
		}
		for(j = 1;j <= 12;j++){
			glyph_samples[i][12 - j] = bit_count_table[bitmap_data_64 & 0xf]; // 4bitずつサンプルする
			bitmap_data_64 >>= 4;
		}
	}

	misc = 11 - (int)(((float)metric.character_ascent * 0.25f) + 0.5f); // グリフの高さ揃えパラメータ

	for(k = 0;k < 12;k++){ // 背景データをRAMに格納
		tmp = k + cly + misc;
		LCDSetGramAddr(clx, tmp < LCD_HEIGHT ? tmp : LCD_HEIGHT - 1);
		LCDPutCmd(0x0022);
		LCD->RAM; // dummy read
		for(j = 0;j < 13;j++){
			bg_ram[k][j] = LCD->RAM;
		}
	}


	for(k = 0;k < 12;k++){ // オーバーサンプリングしたデータと背景データを合成して描画
		tmp = k + cly + misc;
		LCDSetGramAddr(clx, tmp < LCD_HEIGHT ? tmp : LCD_HEIGHT - 1);
		LCDPutCmd(0x0022);
		for(j = 0;j < 12;j++){
			// サブピクセルのドット合計を求める
			alpha_ratio = color_tone_table_4bit[glyph_samples[k * 4 + 0][j] + \
			                                    glyph_samples[k * 4 + 1][j] + \
			                                    glyph_samples[k * 4 + 2][j] + \
			                                    glyph_samples[k * 4 + 3][j]]; // アルファ率を取得

			// Foreground Image
			pixel_fg.color.d16 = colorc[color];
			pixel_fg.color.R *= alpha_ratio;
			pixel_fg.color.G *= alpha_ratio;
			pixel_fg.color.B *= alpha_ratio;

			// Background Image
			pixel_bg.color.d16 = bg_ram[k][j];
			pixel_bg.color.R *= (1.0f - alpha_ratio);
			pixel_bg.color.G *= (1.0f - alpha_ratio);
			pixel_bg.color.B *= (1.0f - alpha_ratio);

			// Add colors
			pixel_fg.color.R += pixel_bg.color.R;
			pixel_fg.color.G += pixel_bg.color.G;
			pixel_fg.color.B += pixel_bg.color.B;

			LCDPutData(pixel_fg.color.d16);
		}
	}

	clx += (metric.right_sided_bearing - metric.left_sided_bearing) * 0.25f + pcf_font.metrics.hSpacing;
	if(code == 0x20 || code == 0x3000){
		clx += 2;
	}
}



void C_PCFPutChar16px(uint16_t code, colors color)
{
	uint64_t bitmap_data_64;
	uint32_t tmp, *p_u32;
	int i, j, k, misc, double_size = 0;
	uint16_t *p_u16, glyph_index;
	uint16_t bg_ram[16][16];

	uint8_t buf[512], glyph_samples[48][16];
	uint8_t enc1, enc2;

	C_FILE fpTmp;

	pixel_fmt_typedef pixel_fg, pixel_bg;
	float alpha_ratio;

	// Get the glyph_index from UTF16 code.
	enc1 = (uint8_t)(code >> 8);
	enc2 = (uint8_t)code;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return;
	}

	// グリフの位置を計算
	tmp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
			 (pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
             enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(C_FILE));
	c_fseek(&fpTmp, tmp, SEEK_CUR);
	c_fread(buf, 1, sizeof(uint16_t), &fpTmp);
    //	glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

	if(glyph_index == 0xFFFF) {
		return;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(C_FILE)); //
	c_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	c_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	// xor 0x80 each metric(subtract 0x80)
	*(uint32_t*)&metric.left_sided_bearing ^= 0x80808080;
    //	metric.left_sided_bearing  ^= (1 << 7);
    //	metric.right_sided_bearing ^= (1 << 7);
    //	metric.character_width     ^= (1 << 7);
    //	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);

	// Get the bitmap data offset
	uint32_t bitmap_offset;
	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_offset, sizeof(C_FILE));
	c_fseek(&fpTmp, glyph_index * sizeof(int32_t), SEEK_CUR); // グリフインデックスが指すビットマップオフセットまでファイルポインタを移動
	c_fread(buf, 1, sizeof(int32_t), &fpTmp);
    //	bitmap_offset = conv_b2l((void*)buf, sizeof(int32_t)); // ビットマップオフセット
	bitmap_offset = __REV(*(uint32_t*)buf);

	if(metric.right_sided_bearing - metric.left_sided_bearing > 32){ // 右ベアリング - 左ベアリングが32pxを超えていたらダブルサイズ有効
		double_size = 1;
	}

	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_bitmap, sizeof(C_FILE)); // ビットマップデータまでファイルポインタを移動
	c_fseek(&fpTmp, bitmap_offset, SEEK_CUR);

	// バッファにビットマップデータをキャッシュする
	c_fread((void*)buf, 1, ((metric.character_ascent + metric.character_descent) * \
                             sizeof(uint32_t)) * (double_size == 0 ? 1 : 2), &fpTmp); 	// ビットマップデータが32bitを超える場合は2倍

	memset((void*)glyph_samples, 0, sizeof(glyph_samples));

	p_u32 = (uint32_t*)buf;
	for(i = 0;i < metric.character_ascent + metric.character_descent;i++){
		bitmap_data_64 = __REV(*p_u32++);
		if(double_size){
			bitmap_data_64 <<= 32;
			bitmap_data_64 |= __REV(*p_u32++);;
			bitmap_data_64 >>= 16; // 64bit - 48bit = 16bit  余白を右に詰める
		} else {
			bitmap_data_64 <<= 16; // 48bit - 32bit = 16bit
		}

		for(j = 1;j <= 16;j++){
			glyph_samples[i][16 - j] = bit_count_table[bitmap_data_64 & 0x7]; // 3bitずつサンプルする
			bitmap_data_64 >>= 3;
		}
	}
	misc = 15 - (int)((float)metric.character_ascent * 0.333f + 0.5f); // グリフの高さ揃えパラメータ

	for(k = 0;k < 16;k++){ // 背景データをRAMに格納
		tmp = k + cly + misc;
		LCDSetGramAddr(clx, tmp < LCD_HEIGHT ? tmp : LCD_HEIGHT - 1);
		LCDPutCmd(0x0022);
		LCD->RAM; // dummy read
		for(j = 0;j < 16;j++){
			bg_ram[k][j] = LCD->RAM;
		}
	}

	for(k = 0;k < 16;k++){ // オーバーサンプリングしたデータと背景データを合成して描画
		tmp = k + cly + misc;
		LCDSetGramAddr(clx, tmp < LCD_HEIGHT ? tmp : LCD_HEIGHT - 1);
		LCDPutCmd(0x0022);
		for(j = 0;j < 16;j++){
			// サブピクセルのドット合計を求める
			alpha_ratio = color_tone_table_3bit[glyph_samples[k * 3 + 0][j] + \
			                                    glyph_samples[k * 3 + 1][j] + \
			                                    glyph_samples[k * 3 + 2][j]]; // アルファ率を取得

			// Foreground Image
			pixel_fg.color.d16 = colorc[color];
			pixel_fg.color.R *= alpha_ratio;
			pixel_fg.color.G *= alpha_ratio;
			pixel_fg.color.B *= alpha_ratio;

			// Background Image
			pixel_bg.color.d16 = bg_ram[k][j];
			pixel_bg.color.R *= (1.0f - alpha_ratio);
			pixel_bg.color.G *= (1.0f - alpha_ratio);
			pixel_bg.color.B *= (1.0f - alpha_ratio);

			// Add colors
			pixel_fg.color.R += pixel_bg.color.R;
			pixel_fg.color.G += pixel_bg.color.G;
			pixel_fg.color.B += pixel_bg.color.B;

			LCDPutData(pixel_fg.color.d16);
		}
	}

	clx += ((metric.right_sided_bearing - metric.left_sided_bearing) * 0.333f + pcf_font.metrics.hSpacing);
	if(code == 0x20 || code == 0x3000){
		clx += 5;
	}
}

void C_PCFPutString(const uint16_t *uni_str, int n, colors color)
{
    //	int offset = 0;
	while(n-- > 0){
		C_PCFPutChar(*uni_str++, color);
	}
}

void C_PCFPutString16px(const uint16_t *uni_str, int n, colors color)
{
    //	int offset = 0;
	while(n-- > 0){
		C_PCFPutChar16px(*uni_str++, color);
	}
}

uint16_t C_PCFGetCharPixelLength(uint16_t code, uint16_t font_width)
{
	uint32_t tmp;
	uint16_t len;
	uint8_t buf[512], enc1, enc2;

	C_FILE fpTmp;

	// Get the glyph_index from UTF16 code.
	uint16_t glyph_index;
	enc1 = (uint8_t)(code >> 8);
	enc2 = (uint8_t)code;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return 0;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return 0;
	}

	// グリフの位置を計算
	tmp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
			(pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
			enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(C_FILE));
	c_fseek(&fpTmp, tmp, SEEK_CUR);
	c_fread(buf, 1, sizeof(uint16_t), &fpTmp);
//	glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

	if(glyph_index == 0xFFFF) {
		return 0;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(C_FILE)); //
	c_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	c_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	// xor 0x80 each metric(subtract 0x80)
	*((uint32_t*)&metric.left_sided_bearing) ^= 0x80808080;
//	metric.left_sided_bearing  ^= (1 << 7);
//	metric.right_sided_bearing ^= (1 << 7);
//	metric.character_width     ^= (1 << 7);
//	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);

	len = (metric.right_sided_bearing - metric.left_sided_bearing) * (font_width < 16 ? 0.25f : 0.333f) + pcf_font.metrics.hSpacing;

	if(code == 0x20 || code == 0x3000){
		len += font_width < 16 ? 2 : 5;
	}

	return len;
}
