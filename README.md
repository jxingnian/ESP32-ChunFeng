<!--
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-24 14:53:16
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-29 20:47:07
 * @FilePath: \ESP32-ChunFeng\README.md
 * @Description: 
-->
# ESP32 春风 - 遇事不决，可问春风

By.星年

功能框图：https://drive.google.com/file/d/1-uiZKS8jdsUj1PGumI2GhHzRNVBK2e_1/view?usp=sharing

使用说明：

编译
idf.py build

WiFi配网
连接设备热点 "ChunFeng"
访问： "192.168.4.1:8080" 

烧录 (COM8修改为实际端口号)
idf.py -p COM8 flash monitor 





•	春风功能框图
o	状态机
	初始化
	联网
	运行
	异常
o	显示
	系统状态
	网络状态
	音频状态
	coze状态
o	网络
	初始化
	反初始化
	状态上报
----	初始化
----	正在联网
----	WiFi已连接
----	4G已连接
----	连接失败
o	音频
	初始化
	反初始化
	mic数据获取
	音频播放
	音频参数
o	coze
	初始化
	反初始化
	音频输入
	音频输出
	音频参数
	对话参数
o	后台管理
	当前状态
	网络设置
	音频设置
	对话设置
	系统设置


