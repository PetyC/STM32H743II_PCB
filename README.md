<!--
 * @Description: 2022-1-14
 * @Autor: Pi
 * @Date: 2021-12-28 15:04:41
 * @LastEditTime: 2022-04-13 17:58:21
-->
应用与水曰等一系列交互装置的核心板
主控使用STM32H743   
存在多个BUG 待修改

2022年1月13号
1、第一版USB、BOOT1、LED、下载电路验证完毕 暂未发现问题
2、第一版发现严重BUG BTB连接器 原设计的5V GND 触点 公座的41~44触点 母座并无对应 导致无法使用
3、第一版发现严重BUG W25Q128 FLASH芯片 封装使用错误 应使 SOIC-8W 误用成SOIC-8 导致无法焊接芯片

目前修改
1、修改BTB电气连接方式 去掉了PE0、PE1、PE5、PE6这四个引脚引出 更换为5V与GND属性
2、修正了W25Q128 FLASH芯片封装 目前为正确封装SOIC-8W
3、微调了丝印

2022年1月14号
1、完成第一版扩展版LCD屏幕设计并投稿打样
2、思考Core PCB 的BTB连接件是否更换为44PIN的更为合适

2022年2月18号
1、Core PCB 的BTB连接件改为44PIN，背面添加了一个0.5间距14PIN PFC，用于连接LCD扩展版，已打样完成，暂未进行功能验证
2、LCD扩展版第一版到手，严重时问题较多：（1、Type-C接口位置与Core PCB的W25Q芯片存在干涉  2、ESP8266 无法连接WIFI 但是AP功能正常 暂未找到原因所在  3、三档滑动开关切换串口部分电路错误 无法实现自由切换串口（USART无法连接到CP210X）
3、LCD扩展板目前准备大幅度修改


2022年2月20日
经过测试发现LCD扩展板问题如下
1、无法连接wifi 实测应该是π型天线阻抗未匹配导致的，L1 C2空焊导致无法正常连接到WIFI，后测试直接去掉π型电路 2PIN直接接陶瓷天线正常连接wifi信号


2022年2月25号
1、修改了LCD显示固件 优化了显示速度 但目前仍然使用阻塞方式发送数据，未能成功使用DMA、IT等方式实现驱动
2、发现BUG：显示器背光无法关闭 目前寻找原因中


2022年3月23号
1、调通DMA SPI发送 大大提高了刷屏速率
问题点：
使用环境如下
HAL库版本 V1.9.1
MDK-ARM版本 V5.32
STM32Cubmex版本 V6.30
在该环境使用Cubmex直接配置SPI MDA时，无法正确发出SPI信号，具体现象为使用HAL_SPI_Transmit_DMA发送数据,使用逻辑分析仪抓取无任何数据，再次使用HAL_SPI_Transmit_DMA时返回DMA busy
后在初始化dma函数前即 HAL_DMA_Init  重新使能DMA时钟(__HAL_RCC_DMA1_CLK_ENABLE())才恢复正常 
DMA 内存端应设为自递增 不然数据线只会输出0 


2022年3月24号
1、解决显示屏背光无法控制BUG,原设计PA7 TIM14作为背光控制引脚，实际上硬件使用了PA8 TIM1进行背光控制
2、加入FREERTOS工程测试

2022年3月25号
1、QSPI W25Q128 FLASH 硬件电路测试通过 
QSPI注意：初始化QSPI相关引脚时，需要将GPIO速度手动调整至最高 不然通信会有问题


2022年3月29号
1、修改了LCD扩展板 
(1 使用了信号复用芯片实现串口信号切换功能
(2 增加了一个拨动按钮
(3 增加了2个状态指示灯


2022年4月1号
1、验证QSPI FLASH 读写程序

!!!! 板载焊接的W25Q128 不确定是JV还是FV型号


2022年4月6号
1、FatFs系统大体移植完成，剩余一些文件操作功能未编写
2、添加W25Q128模拟U盘功能，可直接通过电脑读写
3、新的LCD扩展版依然存在一些BUG


2022年4月13号
1、LCD扩展版重新设计完毕，待焊接验证
2、更换了FLASH 确认目前型号为W25Q128JVSSIO
3、完善了USB和FatFS功能(W25Q128) 目前使用DMA 写入355kb/s 读出理论24MB/s
4、Nand FLASH可正常读写 但坏块管理、ECC校验等功能均未实现