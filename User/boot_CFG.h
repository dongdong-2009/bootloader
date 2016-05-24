#ifndef __BOOT_CFG__H
#define __BOOT_CFG__H

#define update_master 0x0020
#define update_master_NO1 0x0010

#define update_slave 0x0002
#define update_slave_NO1 0x0001

#define packge_size 0x0001

//FLASH��ַ
#define STM32_FLASH_BASE            0x08000000 	            //STM32 FLASH����ʼ��
#define bootloaderStartAppAdress    0x08000000              //Ԥ��20K
#define appStartAdress              0x08006000              //Ԥ��80K
#define appBackStartAdress          0x0801C000              //Ԥ��80k
//д�����ĵ�ַ
#define paraAddress                 0x08032000              //number 101 page
#define test_addr                 0x08032003              //number 101 page

#define pBLOCK 128 
 //boot�������״̬��صĵ�ַ
#define bootUpdateIfoAddress        0x0803E000              //Number 102 page
#define appUpdateIfoAddress         0x0803F000              //Number 103 page

#define isbackup                    0x08037400              //Number 110 page


#define bootAppComIDAddress         0x0803E004              //��Ӫ��ID�ŵ�ַ
#define bootAppCharCodeAddress      0x0803E008              //���׮�����ַ
#define bootVersionAddress          0x0803E01E              //�Ѹ��³���İ汾
#define bootAppPakageNumAddress     0x0803E022              //Ҫ���µ��ܰ�����ַ
#define bootAppNumAddress           0x0803E026              //�������(����)����
#define bootAppUpdateStausAddress   0x0803E028              //�����Ƿ������ɱ�־��ַ
#define bootNewVerFlagAddress       0x0803E02A              //Boot���³���İ汾��Ч��־
#define bootVerByte_1_Add           0x0803E02C              //Boot���³���汾��һ�ֽ�
#define bootVerByte_2_Add           0x0803E02E              //Boot���³���汾�ڶ��ֽ�
				
				
#define appComIDAddress             0x0803F004              //��Ӫ��ID�ŵ�ַ
#define appCharCodeAddress          0x0803F008              //���׮�����ַ
#define appVersionAddress           0x0803F01E              //�Ѹ��³���İ汾
#define appPakageNumAddress         0x0803F022              //Ҫ���µ��ܰ�����ַ
#define appUpdateFlagAddress        0x0803F026              //������±�־��ַ
#define appNewVerFlagAddress        0x0803F028              //���°汾����Ч��־
#define appVerByte_1_Add            0x0803F02A              //���¸��°汾��һ���ֽ�          
#define appVerByte_2_Add            0x0803F02C              //���¸��°汾�ڶ����ֽ�

#endif
