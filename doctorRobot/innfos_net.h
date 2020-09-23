/*
    相关通讯报文
    EE 00 44 00 00 ED           必须先与ECB握手
    EE 00 02 00 00 ED           查询连接的设备
    EE 00 C1 00 01 04 BE 83 ED  与中间板通信的协议指令（必须！！）
    EE 02 2A 00 01 01 7E 80 ED  开启（02）设备指令
    EE 02 07 00 01 06 3F 42 ED  设置（02）设备为模式06的指令
    EE 02 2A 00 01 00 BF 40 ED  关闭（02）设备指令
*/

#ifndef INNFOS_NET_H
#define INNFOS_NET_H

/* --浮点数转换为定点数-- */
#define IQ24(A)     ((int)(A*(1<<24)))
#define IQ8(A)      ((int)(A*(1<<8)))

/* --定点数转换为浮点数-- */
#define IQR24(A)    ((qreal)(A / 16777216.0))
#define IQR8(A)     ((qreal)(A / 256.0))

#define ENABLE              (0x01)
#define DISABLE             (0x00)

/* --模式-- */
#define CURRENT_MODE        (0x01)
#define VELOCITY_MODE       (0x02)
#define POSITION_MODE       (0x03)
#define POSCURVE_MODE       (0x06)
#define VELCURVE_MODE       (0x07)
#define HOME_MODE           (0x08)

/* --异常-- */
#define OVERVOLTAGE         (0x0001)
#define UNDERVOLTAGE        (0x0002)
#define BLOCKING            (0x0004)
#define OVERHEATING         (0x0008)
#define READPARAERROR       (0x0010)
#define COUNTERROR          (0x0020)
#define TEMPSENSOR          (0x0040)
#define COMUNICATION        (0x0080)
#define DRVPROTECT          (0x0400)

/* --ECB相关-- */
#define GET_VERSION         (0x01)
#define GET_ADDRESS         (0x02)
#define ECB_HANDSHAKE       (0x44)
#define ECB_START           (0xC1)

/* --读命令-- */
#define HANDSHAKE           (0x00)
#define GET_MODE            (0x55)
#define GET_LAST_STATE      (0xB0)
#define GET_CURRT_STATE     (0x71)
#define GET_VELCY_STATE     (0x75)
#define GET_POSITN_STATE    (0x79)
#define GET_DEVICE_STATE    (0x2B)

#define GET_CURRT_BANDWT    (0x73)
#define GET_VELCY_BANDWT    (0x77)
#define GET_POSITN_BANDWT   (0x7B)
#define GET_DEVICE_TEMPR    (0x5F)
#define GET_IVERTR_TEMPR    (0x60)
#define GET_ALARM_INFO      (0xFF)

#define GET_CURRT_C         (0x04)
#define GET_CURRT_V         (0x05)
#define GET_CURRT_P         (0x06)

#define GET_POSITN_UPBAND   (0x85)
#define GET_POSITN_DNBAND   (0x86)

/* --写命令-- */
#define SET_MODE            (0x07)
#define SET_CURRT_FILTER    (0x70)
#define SET_VELCY_FILTER    (0x74)
#define SET_POSITN_FILTER   (0x78)
#define SET_DEVICE_STATE    (0x2A)

#define SET_CURRT_C         (0x08)
#define SET_CURRT_V         (0x09)
#define SET_CURRT_P         (0x0A)

#define SET_POSCURVE_MAX_V  (0x1F)
#define SET_POSCURVE_ACERT  (0x20)
#define SET_POSCURVE_DCERT  (0x21)

#define SET_POSITN_UPBADN   (0x32)
#define SET_POSITN_DNBADN   (0x33)

#define SET_DEVICE_POS_UP   (0x83)
#define SET_DEVICE_POS_DN   (0x84)
#define SET_HOME            (0x87)

#define CLEAR_ALARM         (0xFE)
#define CLEAR_HOME          (0x88)
#define SAVE_TO_EEPROM      (0x0D)

#endif // INNFOS_NET_H
