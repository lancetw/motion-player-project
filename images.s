.section ".rodata"

.balign 2
.global internal_flash_pcf_font
internal_flash_pcf_font:
.incbin "mplus_1c_bold_basic_latin.pcf"
.global _sizeof_internal_flash_pcf_font
.set _sizeof_internal_flash_pcf_font, . - internal_flash_pcf_font

.balign 2
.global video_info_board_170x170
video_info_board_170x170:
.incbin "video_info_board_170x170.bin"
.global _sizeof_video_info_board_170x170
.set _sizeof_video_info_board_170x170, . - video_info_board_170x170

.balign 2
.global video_info_board_170x170_alpha
video_info_board_170x170_alpha:
.incbin "video_info_board_170x170_alpha.bin"
.global _sizeof_video_info_board_170x170_alpha
.set _sizeof_video_info_board_170x170_alpha, . - video_info_board_170x170_alpha


.balign 2
.global video_info_26x24
video_info_26x24:
.incbin "video_info_26x24.bin"
.global _sizeof_video_info_26x24
.set _sizeof_video_info_26x24, . - video_info_26x24

.balign 2
.global video_info_26x24_alpha
video_info_26x24_alpha:
.incbin "video_info_26x24_alpha.bin"
.global _sizeof_video_info_26x24_alpha
.set _sizeof_video_info_26x24_alpha, . - video_info_26x24_alpha

.balign 2
.global music_underbar_320x80
music_underbar_320x80:
.incbin "music_underbar_320x80.bin"
.global _sizeof_music_underbar_320x80
.set _sizeof_music_underbar_320x80, . - music_underbar_320x80

.balign 2
.global music_underbar_320x80_alpha
music_underbar_320x80_alpha:
.incbin "music_underbar_320x80_alpha.bin"
.global _sizeof_music_underbar_320x80_alpha
.set _sizeof_music_underbar_320x80_alpha, . - music_underbar_320x80_alpha

.balign 2
.global music_art_default_74x74
music_art_default_74x74:
.incbin "music_art_default_74x74.bin"
.global _sizeof_music_art_default_74x74
.set _sizeof_music_art_default_74x74, . - music_art_default_74x74

.balign 2
.global seek_circle_16x16
seek_circle_16x16:
.incbin "seek_circle_16x16.bin"
.global _sizeof_seek_circle_16x16
.set _sizeof_seek_circle_16x16, . - seek_circle_16x16

.balign 2
.global seek_circle_16x16_alpha
seek_circle_16x16_alpha:
.incbin "seek_circle_16x16_alpha.bin"
.global _sizeof_seek_circle_16x16_alpha
.set _sizeof_seek_circle_16x16_alpha, . - seek_circle_16x16_alpha

.balign 2
.global play_icon_40x40
play_icon_40x40:
.incbin "play_icon_40x40.bin"
.global _sizeof_play_icon_40x40
.set _sizeof_play_icon_40x40, . - play_icon_40x40


.balign 2
.global play_icon_40x40_alpha
play_icon_40x40_alpha:
.incbin "play_icon_40x40_alpha.bin"
.global _sizeof_play_icon_40x40_alpha
.set _sizeof_play_icon_40x40_alpha, . - play_icon_40x40_alpha

.balign 2
.global abort_icon_40x40
abort_icon_40x40:
.incbin "abort_icon_40x40.bin"
.global _sizeof_abort_icon_40x40
.set _sizeof_abort_icon_40x40, . - abort_icon_40x40

.balign 2
.global next_right_32x17
next_right_32x17:
.incbin "next_right_32x17.bin"
.global _sizeof_next_right_32x17
.set _sizeof_next_right_32x17, . - next_right_32x17

.balign 2
.global next_right_32x17_alpha
next_right_32x17_alpha:
.incbin "next_right_32x17_alpha.bin"
.global _sizeof_next_right_32x17_alpha
.set _sizeof_next_right_32x17_alpha, . -next_right_32x17_alpha

.balign 2
.global next_left_32x17
next_left_32x17:
.incbin "next_left_32x17.bin"
.global _sizeof_next_left_32x17
.set _sizeof_next_left_32x17, . -next_left_32x17

.balign 2
.global next_left_32x17_alpha
next_left_32x17_alpha:
.incbin "next_left_32x17_alpha.bin"
.global _sizeof_next_left_32x17_alpha
.set _sizeof_next_left_32x17_alpha, . -next_left_32x17_alpha

.balign 2
.global exit_play_20x13
exit_play_20x13:
.incbin "exit_play_20x13.bin"
.global _sizeof_exit_play_20x13
.set _sizeof_exit_play_20x13, . -exit_play_20x13

.balign 2
.global exit_play_20x13_alpha
exit_play_20x13_alpha:
.incbin "exit_play_20x13_alpha.bin"
.global _sizeof_exit_play_20x13_alpha
.set _sizeof_exit_play_20x13_alpha, . -exit_play_20x13_alpha

.balign 2
.global menubar_320x22
menubar_320x22:
.incbin "menubar_320x22.bin"
.global _sizeof_menubar_320x22
.set _sizeof_menubar_320x22, . -menubar_320x22

.balign 2
.global menubar_320x22_alpha
menubar_320x22_alpha:
.incbin "menubar_320x22_alpha.bin"
.global _sizeof_menubar_320x22_alpha
.set _sizeof_menubar_320x22_alpha, . -menubar_320x22_alpha

.balign 2
.global pic_right_arrow_30x30
pic_right_arrow_30x30:
.incbin "pic_right_arrow_30x30.bin"
.global _sizeof_pic_right_arrow_30x30
.set _sizeof_pic_right_arrow_30x30, . -pic_right_arrow_30x30

.balign 2
.global pic_right_arrow_30x30_alpha
pic_right_arrow_30x30_alpha:
.incbin "pic_right_arrow_30x30_alpha.bin"
.global _sizeof_pic_right_arrow_30x30_alpha
.set _sizeof_pic_right_arrow_30x30_alpha, . -pic_right_arrow_30x30_alpha

.balign 2
.global pic_left_arrow_30x30
pic_left_arrow_30x30:
.incbin "pic_left_arrow_30x30.bin"
.global _sizeof_pic_left_arrow_30x30
.set _sizeof_pic_left_arrow_30x30, . -pic_left_arrow_30x30

.balign 2
.global pic_left_arrow_30x30_alpha
pic_left_arrow_30x30_alpha:
.incbin "pic_left_arrow_30x30_alpha.bin"
.global _sizeof_pic_left_arrow_30x30_alpha
.set _sizeof_pic_left_arrow_30x30_alpha, . -pic_left_arrow_30x30_alpha


.balign 2
.global bass_base_24x18
bass_base_24x18:
.incbin "bass_base_24x18.bin"
.global _sizeof_bass_base_24x18
.set _sizeof_bass_base_24x18, . -bass_base_24x18

.balign 2
.global bass_base_24x18_alpha
bass_base_24x18_alpha:
.incbin "bass_base_24x18_alpha.bin"
.global _sizeof_bass_base_24x18_alpha
.set _sizeof_bass_base_24x18_alpha, . -bass_base_24x18_alpha

.balign 2
.global bass_level1_24x18
bass_level1_24x18:
.incbin "bass_level1_24x18.bin"
.global _sizeof_bass_level1_24x18
.set _sizeof_bass_level1_24x18, . -bass_level1_24x18

.balign 2
.global bass_level1_24x18_alpha
bass_level1_24x18_alpha:
.incbin "bass_level1_24x18_alpha.bin"
.global _sizeof_bass_level1_24x18_alpha
.set _sizeof_bass_level1_24x18_alpha, . -bass_level1_24x18_alpha

.balign 2
.global bass_level2_24x18
bass_level2_24x18:
.incbin "bass_level2_24x18.bin"
.global _sizeof_bass_level2_24x18
.set _sizeof_bass_level2_24x18, . -bass_level2_24x18

.balign 2
.global bass_level2_24x18_alpha
bass_level2_24x18_alpha:
.incbin "bass_level2_24x18_alpha.bin"
.global _sizeof_bass_level2_24x18_alpha
.set _sizeof_bass_level2_24x18_alpha, . -bass_level2_24x18_alpha

.balign 2
.global bass_level3_24x18
bass_level3_24x18:
.incbin "bass_level3_24x18.bin"
.global _sizeof_bass_level3_24x18
.set _sizeof_bass_level3_24x18, . -bass_level3_24x18


.balign 2
.global bass_level3_24x18_alpha
bass_level3_24x18_alpha:
.incbin "bass_level3_24x18_alpha.bin"
.global _sizeof_bass_level3_24x18_alpha
.set _sizeof_bass_level3_24x18_alpha, . -bass_level3_24x18_alpha

.balign 2
.global reverb_base_24x18
reverb_base_24x18:
.incbin "reverb_base_24x18.bin"
.global _sizeof_reverb_base_24x18
.set _sizeof_reverb_base_24x18, . -reverb_base_24x18

.balign 2
.global reverb_base_24x18_alpha
reverb_base_24x18_alpha:
.incbin "reverb_base_24x18_alpha.bin"
.global _sizeof_reverb_base_24x18_alpha
.set _sizeof_reverb_base_24x18_alpha, . -reverb_base_24x18_alpha

.balign 2
.global reverb_level1_24x18
reverb_level1_24x18:
.incbin "reverb_level1_24x18.bin"
.global _sizeof_reverb_level1_24x18
.set _sizeof_reverb_level1_24x18, . -reverb_level1_24x18

.balign 2
.global reverb_level1_24x18_alpha
reverb_level1_24x18_alpha:
.incbin "reverb_level1_24x18_alpha.bin"
.global _sizeof_reverb_level1_24x18_alpha
.set _sizeof_reverb_level1_24x18_alpha, . -reverb_level1_24x18_alpha

.balign 2
.global reverb_level2_24x18
reverb_level2_24x18:
.incbin "reverb_level2_24x18.bin"
.global _sizeof_reverb_level2_24x18
.set _sizeof_reverb_level2_24x18, . -reverb_level2_24x18

.balign 2
.global reverb_level2_24x18_alpha
reverb_level2_24x18_alpha:
.incbin "reverb_level2_24x18_alpha.bin"
.global _sizeof_reverb_level2_24x18_alpha
.set _sizeof_reverb_level2_24x18_alpha, . -reverb_level2_24x18_alpha

.balign 2
.global reverb_level3_24x18
reverb_level3_24x18:
.incbin "reverb_level3_24x18.bin"
.global _sizeof_reverb_level3_24x18
.set _sizeof_reverb_level3_24x18, . -reverb_level3_24x18

.balign 2
.global reverb_level3_24x18_alpha
reverb_level3_24x18_alpha:
.incbin "reverb_level3_24x18_alpha.bin"
.global _sizeof_reverb_level3_24x18_alpha
.set _sizeof_reverb_level3_24x18_alpha, . -reverb_level3_24x18_alpha

.balign 2
.global vocal_base_24x18
vocal_base_24x18:
.incbin "vocal_base_24x18.bin"
.global _sizeof_vocal_base_24x18
.set _sizeof_vocal_base_24x18, . -vocal_base_24x18

.balign 2
.global vocal_base_24x18_alpha
vocal_base_24x18_alpha:
.incbin "vocal_base_24x18_alpha.bin"
.global _sizeof_vocal_base_24x18_alpha
.set _sizeof_vocal_base_24x18_alpha, . -vocal_base_24x18_alpha

.balign 2
.global vocal_canceled_24x18
vocal_canceled_24x18:
.incbin "vocal_canceled_24x18.bin"
.global _sizeof_vocal_canceled_24x18
.set _sizeof_vocal_canceled_24x18, . -vocal_canceled_24x18

.balign 2
.global vocal_canceled_24x18_alpha
vocal_canceled_24x18_alpha:
.incbin "vocal_canceled_24x18_alpha.bin"
.global _sizeof_vocal_canceled_24x18_alpha
.set _sizeof_vocal_canceled_24x18_alpha, . -vocal_canceled_24x18_alpha

.balign 2
.global radiobutton_checked_22x22
radiobutton_checked_22x22:
.incbin "radiobutton_checked_22x22.bin"
.global _sizeof_radiobutton_checked_22x22
.set _sizeof_radiobutton_checked_22x22, . -radiobutton_checked_22x22

.balign 2
.global radiobutton_unchecked_22x22
radiobutton_unchecked_22x22:
.incbin "radiobutton_unchecked_22x22.bin"
.global _sizeof_radiobutton_unchecked_22x22
.set _sizeof_radiobutton_unchecked_22x22, . -radiobutton_unchecked_22x22

.balign 2
.global radiobutton_22x22_alpha
radiobutton_22x22_alpha:
.incbin "radiobutton_22x22_alpha.bin"
.global _sizeof_radiobutton_22x22_alpha
.set _sizeof_radiobutton_22x22_alpha, . -radiobutton_22x22_alpha

.balign 2
.global card_22x22
card_22x22:
.incbin "card_22x22.bin"
.global _sizeof_card_22x22
.set _sizeof_card_22x22, . -card_22x22

.balign 2
.global card_22x22_alpha
card_22x22_alpha:
.incbin "card_22x22_alpha.bin"
.global _sizeof_card_22x22_alpha
.set _sizeof_card_22x22_alpha, . -card_22x22_alpha

.balign 2
.global cpu_22x22
cpu_22x22:
.incbin "cpu_22x22.bin"
.global _sizeof_cpu_22x22
.set _sizeof_cpu_22x22, . -cpu_22x22

.balign 2
.global cpu_22x22_alpha
cpu_22x22_alpha:
.incbin "cpu_22x22_alpha.bin"
.global _sizeof_cpu_22x22_alpha
.set _sizeof_cpu_22x22_alpha, . -cpu_22x22_alpha

.balign 2
.global display_22x22
display_22x22:
.incbin "display_22x22.bin"
.global _sizeof_display_22x22
.set _sizeof_display_22x22, . -display_22x22

.balign 2
.global display_22x22_alpha
display_22x22_alpha:
.incbin "display_22x22_alpha.bin"
.global _sizeof_display_22x22_alpha
.set _sizeof_display_22x22_alpha, . -display_22x22_alpha

.balign 2
.global debug_22x22
debug_22x22:
.incbin "debug_22x22.bin"
.global _sizeof_debug_22x22
.set _sizeof_debug_22x22, . -debug_22x22

.balign 2
.global debug_22x22_alpha
debug_22x22_alpha:
.incbin "debug_22x22_alpha.bin"
.global _sizeof_debug_22x22_alpha
.set _sizeof_debug_22x22_alpha, . -debug_22x22_alpha

.balign 2
.global info_22x22
info_22x22:
.incbin "info_22x22.bin"
.global _sizeof_info_22x22
.set _sizeof_info_22x22, . -info_22x22

.balign 2
.global info_22x22_alpha
info_22x22_alpha:
.incbin "info_22x22_alpha.bin"
.global _sizeof_info_22x22_alpha
.set _sizeof_info_22x22_alpha, . -info_22x22_alpha

.balign 2
.global parent_arrow_22x22
parent_arrow_22x22:
.incbin "parent_arrow_22x22.bin"
.global _sizeof_parent_arrow_22x22
.set _sizeof_parent_arrow_22x22, . -parent_arrow_22x22

.balign 2
.global parent_arrow_22x22_alpha
parent_arrow_22x22_alpha:
.incbin "parent_arrow_22x22_alpha.bin"
.global _sizeof_parent_arrow_22x22_alpha
.set _sizeof_parent_arrow_22x22_alpha, . -parent_arrow_22x22_alpha

.balign 2
.global select_22x22
select_22x22:
.incbin "select_22x22.bin"
.global _sizeof_select_22x22
.set _sizeof_select_22x22, . -select_22x22

.balign 2
.global select_22x22_alpha
select_22x22_alpha:
.incbin "select_22x22_alpha.bin"
.global _sizeof_select_22x22_alpha
.set _sizeof_select_22x22_alpha, . -select_22x22_alpha

.balign 2
.global usb_22x22
usb_22x22:
.incbin "usb_22x22.bin"
.global _sizeof_usb_22x22
.set _sizeof_usb_22x22, . -usb_22x22

.balign 2
.global usb_22x22_alpha
usb_22x22_alpha:
.incbin "usb_22x22_alpha.bin"
.global _sizeof_usb_22x22_alpha
.set _sizeof_usb_22x22_alpha, . -usb_22x22_alpha


.balign 2
.global connect_22x22
connect_22x22:
.incbin "connect_22x22.bin"
.global _sizeof_connect_22x22
.set _sizeof_connect_22x22, . -connect_22x22

.balign 2
.global connect_22x22_alpha
connect_22x22_alpha:
.incbin "connect_22x22_alpha.bin"
.global _sizeof_connect_22x22_alpha
.set _sizeof_connect_22x22_alpha, . -connect_22x22_alpha


.balign 2
.global jpeg_22x22
jpeg_22x22:
.incbin "jpeg_22x22.bin"
.global _sizeof_jpeg_22x22
.set _sizeof_jpeg_22x22, . -jpeg_22x22

.balign 2
.global jpeg_22x22_alpha
jpeg_22x22_alpha:
.incbin "jpeg_22x22_alpha.bin"
.global _sizeof_jpeg_22x22_alpha
.set _sizeof_jpeg_22x22_alpha, . -jpeg_22x22_alpha


.balign 2
.global settings_22x22
settings_22x22:
.incbin "settings_22x22.bin"
.global _sizeof_settings_22x22
.set _sizeof_settings_22x22, . -settings_22x22

.balign 2
.global settings_22x22_alpha
settings_22x22_alpha:
.incbin "settings_22x22_alpha.bin"
.global _sizeof_settings_22x22_alpha
.set _sizeof_settings_22x22_alpha, . -settings_22x22_alpha


.balign 2
.global folder_22x22
folder_22x22:
.incbin "folder_22x22.bin"
.global _sizeof_folder_22x22
.set _sizeof_folder_22x22, . -folder_22x22

.balign 2
.global folder_22x22_alpha
folder_22x22_alpha:
.incbin "folder_22x22_alpha.bin"
.global _sizeof_folder_22x22_alpha
.set _sizeof_folder_22x22_alpha, . -folder_22x22_alpha

.balign 2
.global onpu_22x22
onpu_22x22:
.incbin "onpu_22x22.bin"
.global _sizeof_onpu_22x22
.set _sizeof_onpu_22x22, . -onpu_22x22

.balign 2
.global onpu_22x22_alpha
onpu_22x22_alpha:
.incbin "onpu_22x22_alpha.bin"
.global _sizeof_onpu_22x22_alpha
.set _sizeof_onpu_22x22_alpha, . -onpu_22x22_alpha


.balign 2
.global movie_22x22
movie_22x22:
.incbin "movie_22x22.bin"
.global _sizeof_movie_22x22
.set _sizeof_movie_22x22, . -movie_22x22

.balign 2
.global movie_22x22_alpha
movie_22x22_alpha:
.incbin "movie_22x22_alpha.bin"
.global _sizeof_movie_22x22_alpha
.set _sizeof_movie_22x22_alpha, . -movie_22x22_alpha


.balign 2
.global font_22x22
font_22x22:
.incbin "font_22x22.bin"
.global _sizeof_font_22x22
.set _sizeof_font_22x22, . -font_22x22

.balign 2
.global font_22x22_alpha
font_22x22_alpha:
.incbin "font_22x22_alpha.bin"
.global _sizeof_font_22x22_alpha
.set _sizeof_font_22x22_alpha, . -font_22x22_alpha


.balign 2
.global archive_22x22
archive_22x22:
.incbin "archive_22x22.bin"
.global _sizeof_archive_22x22
.set _sizeof_archive_22x22, . -archive_22x22

.balign 2
.global archive_22x22_alpha
archive_22x22_alpha:
.incbin "archive_22x22_alpha.bin"
.global _sizeof_archive_22x22_alpha
.set _sizeof_archive_22x22_alpha, . -archive_22x22_alpha


.balign 2
.global scrollbar_top_6x7
scrollbar_top_6x7:
.incbin "scrollbar_top_6x7.bin"
.global _sizeof_scrollbar_top_6x7
.set _sizeof_scrollbar_top_6x7, . -scrollbar_top_6x7

.balign 2
.global scrollbar_top_6x7_alpha
scrollbar_top_6x7_alpha:
.incbin "scrollbar_top_6x7_alpha.bin"
.global _sizeof_scrollbar_top_6x7_alpha
.set _sizeof_scrollbar_top_6x7_alpha, . -scrollbar_top_6x7_alpha


.balign 2
.global scrollbar_6x204
scrollbar_6x204:
.incbin "scrollbar_6x204.bin"
.global _sizeof_scrollbar_6x204
.set _sizeof_scrollbar_6x204, . -scrollbar_6x204

.balign 2
.global scrollbar_6x204_alpha
scrollbar_6x204_alpha:
.incbin "scrollbar_6x204_alpha.bin"
.global _sizeof_scrollbar_6x204_alpha
.set _sizeof_scrollbar_6x204_alpha, . -scrollbar_6x204_alpha


.balign 2
.global scrollbar_bottom_6x7
scrollbar_bottom_6x7:
.incbin "scrollbar_bottom_6x7.bin"
.global _sizeof_scrollbar_bottom_6x7
.set _sizeof_scrollbar_bottom_6x7, . -scrollbar_bottom_6x7

.balign 2
.global scrollbar_bottom_6x7_alpha
scrollbar_bottom_6x7_alpha:
.incbin "scrollbar_bottom_6x7_alpha.bin"
.global _sizeof_scrollbar_bottom_6x7_alpha
.set _sizeof_scrollbar_bottom_6x7_alpha, . -scrollbar_bottom_6x7_alpha


.balign 2
.global scrollbar_hline_6x1
scrollbar_hline_6x1:
.incbin "scrollbar_hline_6x1.bin"
.global _sizeof_scrollbar_hline_6x1
.set _sizeof_scrollbar_hline_6x1, . -scrollbar_hline_6x1

.balign 2
.global scrollbar_hline_6x1_alpha
scrollbar_hline_6x1_alpha:
.incbin "scrollbar_hline_6x1_alpha.bin"
.global _sizeof_scrollbar_hline_6x1_alpha
.set _sizeof_scrollbar_hline_6x1_alpha, . -scrollbar_hline_6x1_alpha


.balign 2
.global pic_pref_30x30
pic_pref_30x30:
.incbin "pic_pref_30x30.bin"
.global _sizeof_pic_pref_30x30
.set _sizeof_pic_pref_30x30, . -pic_pref_30x30

.balign 2
.global pic_pref_30x30_alpha
pic_pref_30x30_alpha:
.incbin "pic_pref_30x30_alpha.bin"
.global _sizeof_pic_pref_30x30_alpha
.set _sizeof_pic_pref_30x30_alpha, . -pic_pref_30x30_alpha


.balign 2
.global copy_image_to_100x24
copy_image_to_100x24:
.incbin "copy_image_to_100x24.bin"
.global _sizeof_copy_image_to_100x24
.set _sizeof_copy_image_to_100x24, . -copy_image_to_100x24

.balign 2
.global copy_image_to_100x24_alpha
copy_image_to_100x24_alpha:
.incbin "copy_image_to_100x24_alpha.bin"
.global _sizeof_copy_image_to_100x24_alpha
.set _sizeof_copy_image_to_100x24_alpha, . -copy_image_to_100x24_alpha


.balign 2
.global copy_image_to_music_100x24
copy_image_to_music_100x24:
.incbin "copy_image_to_music_100x24.bin"
.global _sizeof_copy_image_to_music_100x24
.set _sizeof_copy_image_to_music_100x24, . -copy_image_to_music_100x24

.balign 2
.global copy_image_to_filer_100x24
copy_image_to_filer_100x24:
.incbin "copy_image_to_filer_100x24.bin"
.global _sizeof_copy_image_to_filer_100x24
.set _sizeof_copy_image_to_filer_100x24, . -copy_image_to_filer_100x24



.balign 2
.global navigation_playing_patch_32x32
navigation_playing_patch_32x32:
.incbin "navigation_playing_patch_32x32.bin"
.global _sizeof_navigation_playing_patch_32x32
.set _sizeof_navigation_playing_patch_32x32, . -navigation_playing_patch_32x32

.balign 2
.global navigation_playing_patch_32x32_alpha
navigation_playing_patch_32x32_alpha:
.incbin "navigation_playing_patch_32x32_alpha.bin"
.global _sizeof_navigation_playing_patch_32x32_alpha
.set _sizeof_navigation_playing_patch_32x32_alpha, . -navigation_playing_patch_32x32_alpha


.balign 2
.global navigation_pause_patch_32x32
navigation_pause_patch_32x32:
.incbin "navigation_pause_patch_32x32.bin"
.global _sizeof_navigation_pause_patch_32x32
.set _sizeof_navigation_pause_patch_32x32, . -navigation_pause_patch_32x32

.balign 2
.global navigation_pause_patch_32x32_alpha
navigation_pause_patch_32x32_alpha:
.incbin "navigation_pause_patch_32x32_alpha.bin"
.global _sizeof_navigation_pause_patch_32x32_alpha
.set _sizeof_navigation_pause_patch_32x32_alpha, . -navigation_pause_patch_32x32_alpha


.balign 2
.global navigation_bar_24x18
navigation_bar_24x18:
.incbin "navigation_bar_24x18.bin"
.global _sizeof_navigation_bar_24x18
.set _sizeof_navigation_bar_24x18, . -navigation_bar_24x18

.balign 2
.global navigation_bar_24x18_alpha
navigation_bar_24x18_alpha:
.incbin "navigation_bar_24x18_alpha.bin"
.global _sizeof_navigation_bar_24x18_alpha
.set _sizeof_navigation_bar_24x18_alpha, . -navigation_bar_24x18_alpha


.balign 2
.global navigation_entire_loop_24x18
navigation_entire_loop_24x18:
.incbin "navigation_entire_loop_24x18.bin"
.global _sizeof_navigation_entire_loop_24x18
.set _sizeof_navigation_entire_loop_24x18, . -navigation_entire_loop_24x18

.balign 2
.global navigation_entire_loop_24x18_alpha
navigation_entire_loop_24x18_alpha:
.incbin "navigation_entire_loop_24x18_alpha.bin"
.global _sizeof_navigation_entire_loop_24x18_alpha
.set _sizeof_navigation_entire_loop_24x18_alpha, . -navigation_entire_loop_24x18_alpha


.balign 2
.global navigation_infinite_entire_loop_24x18
navigation_infinite_entire_loop_24x18:
.incbin "navigation_infinite_entire_loop_24x18.bin"
.global _sizeof_navigation_infinite_entire_loop_24x18
.set _sizeof_navigation_infinite_entire_loop_24x18, . -navigation_infinite_entire_loop_24x18

.balign 2
.global navigation_infinite_entire_loop_24x18_alpha
navigation_infinite_entire_loop_24x18_alpha:
.incbin "navigation_infinite_entire_loop_24x18_alpha.bin"
.global _sizeof_navigation_infinite_entire_loop_24x18_alpha
.set _sizeof_navigation_infinite_entire_loop_24x18_alpha, . -navigation_infinite_entire_loop_24x18_alpha


.balign 2
.global navigation_one_loop_24x18
navigation_one_loop_24x18:
.incbin "navigation_one_loop_24x18.bin"
.global _sizeof_navigation_one_loop_24x18
.set _sizeof_navigation_one_loop_24x18, . -navigation_one_loop_24x18

.balign 2
.global navigation_one_loop_24x18_alpha
navigation_one_loop_24x18_alpha:
.incbin "navigation_one_loop_24x18_alpha.bin"
.global _sizeof_navigation_one_loop_24x18_alpha
.set _sizeof_navigation_one_loop_24x18_alpha, . -navigation_one_loop_24x18_alpha


.balign 2
.global navigation_shuffle_24x18
navigation_shuffle_24x18:
.incbin "navigation_shuffle_24x18.bin"
.global _sizeof_navigation_shuffle_24x18
.set _sizeof_navigation_shuffle_24x18, . -navigation_shuffle_24x18


.balign 2
.global navigation_shuffle_24x18_alpha
navigation_shuffle_24x18_alpha:
.incbin "navigation_shuffle_24x18_alpha.bin"
.global _sizeof_navigation_shuffle_24x18_alpha
.set _sizeof_navigation_shuffle_24x18_alpha, . -navigation_shuffle_24x18_alpha



.section ".text"
