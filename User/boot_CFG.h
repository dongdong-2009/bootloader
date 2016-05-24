#ifndef __BOOT_CFG__H
#define __BOOT_CFG__H

#define update_master 0x0020
#define update_master_NO1 0x0010

#define update_slave 0x0002
#define update_slave_NO1 0x0001

#define packge_size 0x0001

//FLASH地址
#define STM32_FLASH_BASE            0x08000000 	            //STM32 FLASH的起始地
#define bootloaderStartAppAdress    0x08000000              //预留20K
#define appStartAdress              0x08006000              //预留80K
#define appBackStartAdress          0x0801C000              //预留80k
//写参数的地址
#define paraAddress                 0x08032000              //number 101 page
#define test_addr                 0x08032003              //number 101 page

#define pBLOCK 128 
 //boot程序更新状态相关的地址
#define bootUpdateIfoAddress        0x0803E000              //Number 102 page
#define appUpdateIfoAddress         0x0803F000              //Number 103 page

#define isbackup                    0x08037400              //Number 110 page


#define bootAppComIDAddress         0x0803E004              //运营商ID号地址
#define bootAppCharCodeAddress      0x0803E008              //充电桩编码地址
#define bootVersionAddress          0x0803E01E              //已更新程序的版本
#define bootAppPakageNumAddress     0x0803E022              //要更新的总包数地址
#define bootAppNumAddress           0x0803E026              //程序更新(重启)次数
#define bootAppUpdateStausAddress   0x0803E028              //程序是否更新完成标志地址
#define bootNewVerFlagAddress       0x0803E02A              //Boot最新程序的版本有效标志
#define bootVerByte_1_Add           0x0803E02C              //Boot最新程序版本第一字节
#define bootVerByte_2_Add           0x0803E02E              //Boot最新程序版本第二字节
				
				
#define appComIDAddress             0x0803F004              //运营商ID号地址
#define appCharCodeAddress          0x0803F008              //充电桩编码地址
#define appVersionAddress           0x0803F01E              //已更新程序的版本
#define appPakageNumAddress         0x0803F022              //要更新的总包数地址
#define appUpdateFlagAddress        0x0803F026              //程序更新标志地址
#define appNewVerFlagAddress        0x0803F028              //最新版本的有效标志
#define appVerByte_1_Add            0x0803F02A              //最新更新版本第一个字节          
#define appVerByte_2_Add            0x0803F02C              //最新更新版本第二个字节

#endif
