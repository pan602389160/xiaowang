<a name="1.9.1"></a>
## [1.9.1](//compare/v1.9.0...v1.9.1) (2017-03-17)


### Bug Fixes

* **BSA:** fix when hs ring, disconnect bt connect, but ring thread is not return dc9a08c
* **mozart_app:** start the vr when wifi only in sta mode 117d237
* **mozart_cloude_music:** fix clound and bt sharamemory bug 6634b7b

### Features

* **tv_audio:** add new main app for tv audio ba0c3b5
* **vr:** support aispeech asr sds. 71f6fe5

### Performance Improvements

* **BSA:** perf bt_dg handle ac3fd3f



<a name="1.9.0"></a>
# 1.9.0 (2017-02-17)


### Bug Fixes

* **airplay:** remove a compile warning 66c7d41
* **ALSA:** strip alsa-lib/smixer/*.so 495d309
* **app-config:** fix compile error and mask app-config 4b0f980
* **app-config:** fix switch network bug abbc583
* **appserver:** fix compiler warning 5ad498f
* **appserver:** remove use of client_command_info b1139bd
* **appserver:** rename client_info to appserver_cli_info 5ebda1c
* **appserver:** start appserver after network connected and stop it when switch network 5f73659
* **asr:** fix the error analysis music domain f426924
* **BSA:** add #if (SUPPORT_BSA_BLE == 1) to mozart.c 7553aff
* **BSA:** add bluetooth_mode_pthread function macro expansion 8c89d56
* **BSA:** modify BLE device name display "No Services" problem 1622bed
* **BSA:** modify bsa aec resample parameter set d5da062
* **BSA:** modify bsa cancel auto_reconnect_thread logic d15b874
* **BSA:** modify bsa define bug e953fd9
* **BSA:** modify mozart_bluetooth_ble_set_visibility invocation point 8f28f4c
* **BSA:** modify mozart_bluetooth_dg_uipc_cback print type e8aaf17
* **BSA:** resolve BT display "Broadcom Bluetooth device" name when BT init cd1c377
* **bt:** fix a compile error 224d9a2
* **cgi:** fix compile error because of alarm refactor 75848ed
* **configuration:** remove useless configuration. 32c2217
* **event:** fix key is invalid when quick release. c775a31
* **ingenicplayer:** clean current musiclist when receive play_shortcut command c9bb130
* **ingenicplayer:** fix deadlock when play shortcut a32d7c1
* **ingenicplayer:** fix set_seek and play_queue 4af7fbd
* **ingenicplayer:** send a empty array to prevent ios APP error. 8e6d989
* **ingenicplayer:** stop musicplayer playback when musiclist is empty ae1bcab
* **lapsule:** will not start lapsule service if not enabled 6d6173a
* **LAPSULE:** add SUPPORT_LAPSULE Macro expansion to lapsule code a3949a5
* **libappserver:** use client_info instand of client_fd in struct cmd_info_c to transport client in a99e7bd
* **linein:** fix the bug linein will not be closed fb53a72
* **LOCALPLAYER:** add LOCAPLAYER domain_status judge when mozart_localplayer_stop_playback() ab03941
* **localstream_test:** switch to oss before exit 5a5b75d
* **locaplayer:** add SUPPORT_LOCALPLAYER in such code 24df786
* **main:** fix compile error 230b93a
* **Makefile:** fix halley2 config bug 88612b7
* **Makefile:** modify canna_v1.0_ap6212a_cramfs_config 2c380a4
* **module:** don't switch to cloud play mode when wifi is AP 21690f1
* **module:** snd source is LAPSULE when VR is active. 4cd60e0
* **modules:** fix vr-iflytek bug bbe15e4
* **mozart:** delete s in mozart 0379495
* **mozart:** fix compile error because of ingenicplayer add 2ff378d
* **mozart:** ingenicplayer play local music in tfcard_scan_done_callback 326bddc
* **mozart:** modify alarm register interface 9048b40
* **mtd-utils:** fix make mtd-utils bug 36eb215
* **multiroom:** fix multiroom send error caused by ntp time change ee91c53
* **multiroom:** fix sound not sync at system start cause by high cpu use. 7675603
* **musicplayer:** default volume is 0 bc87ee9
* **musicplayer:** do not wait module stop before play music when musicplayer is in use 22f6524
* **musicplayer:** Don't call mozart_module_stop, when snd_source don't change. 281f6a5
* **musicplayer:** fix compile error 945d4ea
* **musicplayer:** set MUSICPLAYER_DOMAIN to STATUS_PLAYING when receive PLAYER_TRANSPORT status 6f8be90
* **musicplayer:** when music start, check if musicplayer is acitve. ce7b8fc
* **net:** stop the player when config or switch wifi. be3b65b
* **network:** initialize new_mode in mozart_wifi_mode() acc8ee1
* **nvgen:** fix compile error 4897d12
* **nvgen:** fix nvgen bug 7fb07ab
* **nvgen:** modify old method name: update_block --> update_times 3feb8d7
* **nvgen:** support start_magic & end_magic 543c840
* **rootfs:** add flash_erase on spinand f15e718
* **rootfs:** decrease erase block count 1d61098
* **rootfs:** fix "unknown operand" S00system_init.sh 39076db
* **rootfs:** fix canot mount usrdata bug on cramfs. f965e6a
* **test:** fix compile error because of libs update ff1c917
* **test-musiclist:** change some func name 48f2ec3
* **test-network-manager:** ingore STA_CONNECT_STARTING cf2f6b7
* **updater:** complete updater code. 7b57606
* **updater:** fix compile error 9f822dd
* **updater:** fix deadlock bug caused by forking on a thread. fdf57f3
* **updater:** fix download error bug on some webserver. 6a8e098
* **updater:** fix download error on some http server. 4d6e44b
* **updater:** fix download size calc error 3c34bf9
* **updater:** fix memoey leak 83f0a00
* **updater:** fix some bug in updater 7e03b28
* **updater:** fix write flash bug 32fca09
* **updater:** link libplayer.so libsharememory.so 7d90eca
* **updater:** modify the position of check update version 6c6782f
* **updater:** modify update process follow libnvrw update. e4c8f2a
* **volume:** set default volume domain is MUSIC_VOLUME, not BT_MUSIC_VOLUME on volumedown. 7e8306d
* **vr:** fix build error because of make -j 531e7ec
* **vr:** fix can not play music bug if not in musicplayer mode. c65aafd
* **vr:** fix player resume failed, when asr failed. b473c64
* **vr:** fix vr build error 30ce20e
* **vr-iflytek:** modify compile err be6ae9c
* **vr-speech:** fixed a search song bug while the song has a same name with film 1e7e95b
* **ximalaya:** play music when system starting 2300c52
* can't mount usrdata. 9fac35e
* delete tmp_config cdddef3
* fix a error config format 2bb0c13

### Features

* **airplay:** add new interfaces in airplay module fbd8f1b
* **alarm:** add alarm config 2c261b9
* **alarm:** set rtc time and start-up alarm 3eb99e7
* **alarm:** support alarm settings by app dfb919e
* **amazon:** add amazon support 2593f57
* **appserver:** add device scope 893cfaf
* **appserver:** add ingenicplayer config and appserver callback func d17ece0
* **appserver:** add new command : get_device_name / set_device_name 6631a8b
* **bsa:** merge bt_ble event logic 9bc1a4f
* **BSA:** add A2DP Source profile, add the corresponding function acafb21
* **BSA:** add ap6255 bt firmware d6f5455
* **BSA:** add AP6255 BT support 713adf6
* **BSA:** add app_hh cback 77e4fc3
* **BSA:** add app_hh event c044c04
* **BSA:** add avk_set_abs_vol event handle 53fa466
* **BSA:** add ble client register notification function and get notification data code han 41b1df2
* **BSA:** add BLE server\client event 0520b07
* **BSA:** add ble_common_profile_cback, delete bt_ble_client_notify_cback 7c96c3a
* **BSA:** add ble_disc_new and ble_disc_complete event c61e0ee
* **BSA:** add bluetooth auto reconnect logic 41d6fbb
* **BSA:** add bluetooth ring evt code logic e4a8273
* **BSA:** add bluetooth sec_link_down misc type 0608660
* **BSA:** add bsa app_av logic handle 0999701
* **BSA:** add BSA avk volume sync function 99103b9
* **BSA:** add bsa ble hid support 50ad929
* **BSA:** add BSA connect\disconnect tts 0734403
* **BSA:** add bsa OPS OPC SUPPORT 0bea84b
* **BSA:** add bt AV event handle 343ff75
* **BSA:** add bt discovery info print afeed83
* **BSA:** add bt fimware from BCM4343A0_001.001.034.0056.0204.hcd to BCM4343A0_001.001.034 e025751
* **BSA:** add bt ops such event handle d6e3679
* **BSA:** add bt_ble_client_notify_cback() and bt_ble_server_write_cback() ae71c9e
* **BSA:** add dialog device support 6f77e9f
* **BSA:** add disc_new event handler c639b40
* **BSA:** add gets the current music play information logic processing addc7fb
* **BSA:** add hs ring start\hang up EVT 772a91d
* **BSA:** add hs_vgs logic 1485e8f
* **BSA:** add is_part_service, this parameter can set ble active boradcast type 6d8dcdd
* **BSA:** add Listen for notifications function 0cfdc43
* **BSA:** add mozart_bluetooth_ble_set_adv_param() interface f557319
* **BSA:** add mozart_bluetooth_parse_eir_manuf_specific function handle 265a5dd
* **BSA:** add pbc handle dd4b585
* **BSA:** add play_pos event, modify pause\stoped\TRACK_CHANGE event to paused\stopped\tra bcf9c6d
* **BSA:** add play_status event 39d16b0
* **BSA:** add resample_time in struct bt_aec_resample_init_data cf0145c
* **BSA:** add SPP function handle b7da799
* **BSA:** modify BT_VOLUME to BT_CALL_VOLUME, add BT_MUSIC_VOLUME d6c1bf9
* **BSA:** support alsa audio when set_abs_vol 99b7c9b
* **BSA:** update AP6212a/BCM43438A1 fimware 2c60280
* **cloud_music:** add cloud music module, include ximalaya and jushang. 43e398f
* **cloud_music:** support aispeech music provider 35798eb
* **configs:** add wireless-tools to updater 4100bd5
* **configs:** Using official wpa_supplicant and hostapd tools for realtek module 2e621da
* **event:** KEY_MODE, KEY_VOLUMEUP, KEY_VOLUMEDOWN, KEY_PLAYPAUSE support long press. 0170491
* **event_manager:** auto mount(umount) tf card to(from) pc when usb plug in(pull out). 8b04538
* **gen-updatepkg:** support 'make update` 2fc0496
* **ieee8021as:** add bcm6255 support 8021as firmware option f312f87
* **ingenicplayer:** add new interface for playing local tracks 159c078
* **ingenicplayer:** add play shortcut func cd548be
* **ingenicplayer:** add play vr tracks interface and tracks change event d1ee206
* **ingenicplayer:** call mozart_module_ingenicplayer_start in ingenicplayer_play_queue f4c1cbf
* **ingenicplayer:** change the interface of play tracks 310b72b
* **jushang:** support jushang a368e8b
* **libtts_jietong:** add pansy libtts_jietong support c4a4805
* **liburl:** compile liburl to file system b691bdf
* **localstream:** add localstream test program 1610345
* **main:** add check new version thread 6592a24
* **Makefile:** add halley2 sfc nand oss and alsa config 7e8b40f
* **Makefile:** add halley2 spi nand 43438 a1 config 2bd7366
* **minigui:** transplant minigui to device-tools 1619c8b
* **modules:** add camera function b10a3e8
* **mozart:** add jietong asr support 4052e76
* **mozart:** add mozart music player f2c02ac
* **mozart:** add multiroom control support 2f8d404
* **mozart:** adjust the directory structure of mozart 953873f
* **mozart:** can be controllered by ingenicplayer app 45ea241
* **mozart:** fix "Cannot allocate memory" 14ab8c4
* **mozart:** support alsa on 16MB spi norflash. 89f7c08
* **mozart:** support new module preemption method fa56ec7
* **mozartplayer:** move ingenicplayer to mozartplayer 83568c8
* **mtd-utils:** add flash_erase command 6ebaf5c
* **mtd-utils:** compile ubi-tools only on needed. 1450099
* **mtd-utils:** compile ubi-utils when compile mtd-utils package. 26fd72d
* **multiroom:** move audio output change from mulroom_interface to modules 1ec0f3d
* **network:** replace the network_manager with a newer version. d385566
* **network-manager:** add realtek airkiss test example 34c1950
* **network-manager:** add realtek simple config test example 68e4e3e
* **network-manager-v2:** add test case for network config b7c0f18
* **nvgen:** add nvgen tool a6af6b6
* **nvgen:** pad nv.img to 2 * erasesize d534820
* **nvgen:** support nvinfo->magic 626c31a
* **ota:** add usrdata partition on emmc e902340
* **ota:** add usrdata partition on spinand 7a5159b
* **player:** add support two mplayer channels simultaneously function de03f2f
* **render:** support next/previous song by qplay a1d7629
* **rootfs:** add ntp server ec81678
* **rootfs:** alias ps='ps wwl' 3e093df
* **rootfs:** cancel automatically mount /dev/mmcblk when boot ext4 f09cedf
* **rootfs:** modify S00system_init.sh 60e3385
* **rootfs:** modify S00system_init.sh when make ext4 and ubifs 9255c61
* **rootfs:** support multi storage on mkfs & mkotafs 6f16dfe
* **rootfs:** support some cmd completion on C-r at startup 03d5eb7
* **rootfs:** support updaterfs + apfs on ubifs df01e02
* **test:** add aec delay time test 529fa4c
* **test-misc:** get more nvinfo 082d55f
* **test-misc:** move tools/device-tools/test to tools/device-tools/test/misc f0173c8
* **test-musiclist:** add music list test 9e70227
* **test-network-manager:** add AP_STATUS message test 4909f8c
* **test-network-manager:** add AP-STA-DISCONNECTED test ed0c376
* **test-network-manager:** add CHECKMODE test 802ab6c
* **test-network-manager:** add cooee and atalk test 7946033
* **test-network-manager:** add eth test 219e7eb
* **test-network-manager:** add ip change test example 24a5941
* **test-network-manager:** add max wifi info count test 5193df3
* **test-network-manager:** add sta disconnect, auto reconnect test and update checkpatch 40ac665
* **test-network-manager:** add STA test 0a7ffd3
* **test-network-manager:** add SW_AP test d9055fc
* **test-network-manager:** add SW_NETCFG test 10cb477
* **test-network-manager:** add update_wifi_priority test d49d634
* **test-network-manager:** check WIFI_MODE message 7566c7f
* **tts:** canna_v1.0_ap6212a_cramfs_config use aispeech tts 41dd4ab
* **ui:** add libsmartui d91fa23
* **updater:** add emmc ota support 10c07b4
* **updater:** add sound control in updater e1c6874
* **updater:** add spinand updater func 27a2adc
* **updater:** check if nv_info is null e9ca34e
* **updater:** check md5 after download and write flash 9def292
* **updater:** complete md5 check func: after download & after update 1761786
* **updater:** split ota partition to (ota_updater + ota_kernel) on spinand and emmc c789831
* **updater:** support update uboot ee90bf8
* **vr:** add asr control led on and off 216ba5a
* **vr:** add jietong wakeup support 232f3db
* **vr:** re-write vr logic on press KEY_RECORD 3c7931d
* **vr:** support choose song supplyer on vr 21e7d7b
* add ALSA external codec AEC interface eb98655
* **vr:** support play joke bf25851
* **vr:** support search resouce with random interface 322e3bc
* Add AUXIN siwtch function for X1000 ALSA platform. M150 hasnot modify. 67e8af8
* enable webrtc in canna_v1.0_ap6212a_config bc13e54
* enable ximalaya, vr_speech in trunk_v1.1_ap6212 and enable webrtc in canna_v2.0_ 06d3d16
* support canna_v1.0_ap6212a_ubifs_config. 1f0c92d
* ximalaya instead of lapsule in mozart_snd_source_switch 5210a7c
* **vr:** update speech vr interface 79cbe5b
* **ximalaya:** add ximalaya_get_keyword() func ddfa24e
* **ximalaya:** support ximalaya OpenAPI 5af0dc5

### Performance Improvements

* **BSA:** add bt hs call evt code logic 31ba470
* **BSA:** add mozart_bluetooth_hs_get_call_state return value judge 4f7346c
* **BSA:** add mozart_bluetooth_set_visibility(0, 0) when bt connected, add mozart_bluetoot 873adfa
* **BSA:** modify dialog mode e423fbd
* **BSA:** modify hh_client_num position af7ee21
* **BSA:** modify manu_data.company_id print format 004bb0d
* **BSA:** modify mozart_bluetooth_auto_reconnect() define, modify auto_reconnect number of cf6c579
* **BSA:** modify mozart_bluetooth_ble_configure_adv_data() struct 7a1aadd
* **BSA:** optimised disc_complete and disc_new event code 2a24769
* **BSA:** optimize mozart_bluetooth_auto_reconnect return value judge 32a78ae
* **BSA:** Organize and optimize BSA interface 0476a49
* **BSA:** perf auto-reconnect_pthread logical 70da36e
* **BSA:** perf avk resample and channel change logic handle bc7eaf4
* **BSA:** perf when vr_speech is voice wakeup, hs call not open record route 1c04356
* **mplayer:** perf usleep time when check dsp is open or not 69a230e
* **updater:** change block read buffersize: 4KB to 128KB 61db288



<a name="1.8.0"></a>
# 1.8.0 (2016-08-22)


### Bug Fixes

* **airplay:** remove a compile warning 66c7d41
* **ALSA:** strip alsa-lib/smixer/*.so 495d309
* **app-config:** fix compile error and mask app-config 4b0f980
* **app-config:** fix switch network bug abbc583
* **appserver:** fix compiler warning 5ad498f
* **appserver:** remove use of client_command_info b1139bd
* **BSA:** add #if (SUPPORT_BSA_BLE == 1) to mozart.c 7553aff
* **BSA:** add bluetooth_mode_pthread function macro expansion 8c89d56
* **BSA:** modify BLE device name display "No Services" problem 1622bed
* **BSA:** modify bsa aec resample parameter set d5da062
* **BSA:** modify bsa cancel auto_reconnect_thread logic d15b874
* **BSA:** modify mozart_bluetooth_ble_set_visibility invocation point 8f28f4c
* **BSA:** resolve BT display "Broadcom Bluetooth device" name when BT init cd1c377
* **cgi:** fix compile error because of alarm refactor 75848ed
* **configuration:** remove useless configuration. 32c2217
* **event:** fix key is invalid when quick release. c775a31
* **ingenicplayer:** fix set_seek and play_queue 4af7fbd
* **ingenicplayer:** stop musicplayer playback when musiclist is empty ae1bcab
* **lapsule:** will not start lapsule service if not enabled 6d6173a
* **LAPSULE:** add SUPPORT_LAPSULE Macro expansion to lapsule code a3949a5
* **linein:** fix the bug linein will not be closed fb53a72
* **localplayer:** add LOCAPLAYER domain_status judge when mozart_localplayer_stop_playback() ab03941
* **locaplayer:** add SUPPORT_LOCALPLAYER in such code 24df786
* **module:** don't switch to cloud play mode when wifi is AP 21690f1
* **module:** snd source is LAPSULE when VR is active. 4cd60e0
* **mozart:** delete s in mozart 0379495
* **mozart:** fix compile error because of ingenicplayer add 2ff378d
* **mozart:** ingenicplayer play local music in tfcard_scan_done_callback 326bddc
* **mtd-utils:** fix make mtd-utils bug 36eb215
* **musicplayer:** default volume is 0 bc87ee9
* **musicplayer:** Don't call mozart_module_stop, when snd_source don't change. 281f6a5
* **musicplayer:** when music start, check if musicplayer is acitve. ce7b8fc
* **network:** stop the player when config or switch wifi. be3b65b
* **network:** initialize new_mode in mozart_wifi_mode() acc8ee1
* **nvgen:** fix compile error 4897d12
* **nvgen:** fix nvgen bug 7fb07ab
* **nvgen:** modify old method name: update_block --> update_times 3feb8d7
* **rootfs:** decrease erase block count 1d61098
* **rootfs:** fix canot mount usrdata bug on cramfs. f965e6a
* **test:** fix compile error because of libs update ff1c917
* **test-musiclist:** change some func name 48f2ec3
* **test-network-manager:** ingore STA_CONNECT_STARTING cf2f6b7
* **updater:** complete updater code. 7b57606
* **updater:** fix compile error 9f822dd
* **updater:** fix download error on some http server. 4d6e44b
* **updater:** fix download size calc error 3c34bf9
* **updater:** fix memoey leak 83f0a00
* **updater:** fix some bug in updater 7e03b28
* **updater:** fix write flash bug 32fca09
* delete tmp_config cdddef3
* **updater:** link libplayer.so libsharememory.so 7d90eca
* **updater:** modify the position of check update version 6c6782f
* **updater:** modify update process follow libnvrw update. e4c8f2a
* **volume:** set default volume domain is MUSIC_VOLUME, not BT_MUSIC_VOLUME on volumedown. 7e8306d
* can't mount usrdata. 9fac35e
* **vr:** fix player resume failed, when asr failed. b473c64
* **vr-iflytek:** modify compile err be6ae9c
* **vr-speech:** fixed a search song bug while the song has a same name with film 1e7e95b
* **ximalaya:** play music when system starting 2300c52

### Features

* **airplay:** add new interfaces in airplay module fbd8f1b
* **alarm:** add alarm config 2c261b9
* **alarm:** set rtc time and start-up alarm 3eb99e7
* **alarm:** support alarm settings by app dfb919e
* **amazon:** add amazon support 2593f57
* **appserver:** add device scope 893cfaf
* **appserver:** add ingenicplayer config and appserver callback func d17ece0
* **appserver:** add new command : get_device_name / set_device_name 6631a8b
* **bsa:** merge bt_ble event logic 9bc1a4f
* **BSA:** add avk_set_abs_vol event handle 53fa466
* **BSA:** add ble_disc_new and ble_disc_complete event c61e0ee
* **BSA:** add bluetooth auto reconnect logic 41d6fbb
* **BSA:** add bluetooth ring evt code logic e4a8273
* **BSA:** add bluetooth sec_link_down misc type 0608660
* **BSA:** add BSA avk volume sync function 99103b9
* **BSA:** add bsa ble hid support 50ad929
* **BSA:** add BSA connect\disconnect tts 0734403
* **BSA:** add bt discovery info print afeed83
* **BSA:** add disc_new event handler c639b40
* **BSA:** add gets the current music play information logic processing addc7fb
* **BSA:** add hs ring start\hang up EVT 772a91d
* **BSA:** add hs_vgs logic 1485e8f
* **BSA:** add Listen for notifications function 0cfdc43
* **BSA:** add mozart_bluetooth_parse_eir_manuf_specific function handle 265a5dd
* **BSA:** modify BT_VOLUME to BT_CALL_VOLUME, add BT_MUSIC_VOLUME d6c1bf9
* **cloud_music:** add cloud music module, include ximalaya and jushang. 43e398f
* **configs:** Using official wpa_supplicant and hostapd tools for realtek module 2e621da
* **event:** KEY_MODE, KEY_VOLUMEUP, KEY_VOLUMEDOWN, KEY_PLAYPAUSE support long press. 0170491
* **gen-updatepkg:** support 'make update` 2fc0496
* **ingenicplayer:** add new interface for playing local tracks 159c078
* **ingenicplayer:** add play shortcut func cd548be
* **ingenicplayer:** add play vr tracks interface and tracks change event d1ee206
* **ingenicplayer:** call mozart_module_ingenicplayer_start in ingenicplayer_play_queue f4c1cbf
* **ingenicplayer:** change the interface of play tracks 310b72b
* **jushang:** support jushang a368e8b
* **libtts_jietong:** add pansy libtts_jietong support c4a4805
* **liburl:** compile liburl to file system b691bdf
* **main:** add check new version thread 6592a24
* **Makefile:** add halley2 sfc nand oss and alsa config 7e8b40f
* **minigui:** transplant minigui to device-tools 1619c8b
* **mozart:** add mozart music player f2c02ac
* **mozart:** adjust the directory structure of mozart 953873f
* **mozart:** can be controllered by ingenicplayer app 45ea241
* **mozart:** support alsa on 16MB spi norflash. fcb75b7
* **mozart:** support new module preemption method fa56ec7
* **mozartplayer:** move ingenicplayer to mozartplayer 83568c8
* **mtd-utils:** add flash_erase command 6ebaf5c
* **mtd-utils:** compile ubi-tools only on needed. 1450099
* **mtd-utils:** compile ubi-utils when compile mtd-utils package. 26fd72d
* **network:** replace the network_manager with a newer version. d385566
* **network-manager:** add realtek airkiss test example 34c1950
* **network-manager:** add realtek simple config test example 68e4e3e
* **nvgen:** add nvgen tool a6af6b6
* **nvgen:** pad nv.img to 2 * erasesize d534820
* **nvgen:** support nvinfo->magic 626c31a
* **player:** add support two mplayer channels simultaneously function de03f2f
* **render:** support next/previous song by qplay a1d7629
* **rootfs:** alias ps='ps wwl' 3e093df
* **rootfs:** cancel automatically mount /dev/mmcblk when boot ext4 f09cedf
* **rootfs:** modify S00system_init.sh 60e3385
* **rootfs:** modify S00system_init.sh when make ext4 and ubifs 9255c61
* **rootfs:** support multi storage on mkfs & mkotafs 6f16dfe
* **rootfs:** support some cmd completion on C-r at startup 03d5eb7
* **rootfs:** support updaterfs + apfs on ubifs df01e02
* **test-misc:** get more nvinfo 082d55f
* **test-misc:** move tools/device-tools/test to tools/device-tools/test/misc f0173c8
* **test-musiclist:** add music list test 9e70227
* **test-network-manager:** add AP_STATUS message test 4909f8c
* **test-network-manager:** add AP-STA-DISCONNECTED test ed0c376
* **test-network-manager:** add CHECKMODE test 802ab6c
* **test-network-manager:** add cooee and atalk test 7946033
* **test-network-manager:** add eth test 219e7eb
* enable ximalaya, vr_speech in trunk_v1.1_ap6212 and enable webrtc in canna_v2.0_ 06d3d16
* **test-network-manager:** add ip change test example 24a5941
* **test-network-manager:** add max wifi info count test 5193df3
* **test-network-manager:** add sta disconnect, auto reconnect test and update checkpatch 40ac665
* **test-network-manager:** add STA test 0a7ffd3
* **test-network-manager:** add SW_AP test d9055fc
* **test-network-manager:** add SW_NETCFG test 10cb477
* **test-network-manager:** add update_wifi_priority test d49d634
* enable webrtc in canna_v1.0_ap6212a_config bc13e54
* support canna_v1.0_ap6212a_ubifs_config. 1f0c92d
* **test-network-manager:** check WIFI_MODE message 7566c7f
* **ui:** add libsmartui d91fa23
* **updater:** add emmc ota support 10c07b4
* **updater:** add sound control in updater e1c6874
* ximalaya instead of lapsule in mozart_snd_source_switch 5210a7c
* **updater:** add spinand updater func 27a2adc
* **updater:** check if nv_info is null e9ca34e
* **updater:** check md5 after download and write flash 9def292
* **updater:** complete md5 check func: after download & after update 1761786
* **vr:** add asr control led on and off 216ba5a
* **ximalaya:** add ximalaya_get_keyword() func ddfa24e
* **ximalaya:** support ximalaya OpenAPI 5af0dc5

### Performance Improvements

* **BSA:** add bt hs call evt code logic 31ba470
* **BSA:** add mozart_bluetooth_hs_get_call_state return value judge 4f7346c
* **BSA:** add mozart_bluetooth_set_visibility(0, 0) when bt connected, add mozart_bluetoot 873adfa
* **BSA:** modify hh_client_num position af7ee21
* **BSA:** modify manu_data.company_id print format 004bb0d
* **BSA:** modify mozart_bluetooth_auto_reconnect() define, modify auto_reconnect number of cf6c579
* **BSA:** modify mozart_bluetooth_ble_configure_adv_data() struct 7a1aadd
* **BSA:** optimised disc_complete and disc_new event code 2a24769
* **BSA:** optimize mozart_bluetooth_auto_reconnect return value judge 32a78ae
* **BSA:** Organize and optimize BSA interface 0476a49
* **updater:** change block read buffersize: 4KB to 128KB 61db288



<a name="1.7.0"></a>
