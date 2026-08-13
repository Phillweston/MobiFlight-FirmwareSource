# FENIX A320 OVHD 自锁版 V1_0_2 I2C 映射分析

## 1. 结论

配置文件共有 322 个启用配置项。除 2 个没有绑定设备的占位项外，需要通过 I2C
镜像的有效配置项为 320 个。

当前固件只镜像普通 Button 和普通 Output。该配置没有使用普通 Button，因而当前
实现实际上只能覆盖 7 个普通 Output，不能覆盖面板上的 128 个移位寄存器按钮、
26 个 MUX 按钮、6 个模拟旋钮、151 个移位寄存器灯和 2 个电池数码显示区域。

必须新增以下五类协议映射：

1. InputShiftRegister 输入事件；
2. InputMultiplexer 输入事件；
3. AnalogInput 数值事件；
4. ShiftRegister 输出事件；
5. Display Module 数码管显示事件。

V1_0_2 没有配置字符 LCD。`BAT DISPLAY` 是一个 6 位 LED 数码管模块，其中 BAT2
使用数字位 0、1、2，BAT1 使用数字位 3、4、5。

## 2. 配置统计

| 方向 | MobiFlight 类型 | 配置项数 | 当前 I2C 是否覆盖 | 要求 |
| --- | --- | ---: | --- | --- |
| 输入 | InputShiftRegister | 128 | 否 | 必须新增 |
| 输入 | InputMultiplexer | 26 | 否 | 必须新增 |
| 输入 | AnalogInput | 6 | 否 | 必须新增 |
| 输出 | ShiftRegister | 151 | 否 | 必须新增 |
| 输出 | Output | 7 | 是 | 保留 |
| 输出 | Display Module | 2 | 否 | 必须新增 |
| 输入 | 未绑定设备 | 1 | 不适用 | 不发送 |
| 输出 | 未绑定设备 | 1 | 不适用 | 不发送 |

MCC 中的每一项是电脑端业务配置，不等同于一个 Arduino 设备对象。I2C 不应发送
FENIX 模拟器变量或 GUID，而应发送固件能够稳定获得的硬件标识和最终值。

## 3. 输入映射

### 3.1 输入移位寄存器

共 5 个逻辑模块、128 个已用通道：

| 模块名 | 已用数 | 已用范围 | 未使用通道 |
| --- | ---: | --- | --- |
| InputShifter | 21 | 0-29 | 11、15-19、24-26、30、31 |
| Shifter APU | 31 | 1-31 | 0 |
| Shifter APU 2 | 25 | 0-31 | 3、8-11、20、26 |
| Shifter LEFT | 30 | 0-31 | 12、30 |
| Shifter RIGHT | 21 | 0-30 | 4-6、8、12-15、21、28、31 |

每次变化应发送：

```text
类型 = INPUT_SHIFTER
模块标识 = 固件配置中的 InputShifter 模块索引
通道 = 0..31
状态 = 1 按下 / 0 释放
```

只发送通道号不足以区分 5 个模块。例如 `Shifter APU/通道 1` 和
`Shifter RIGHT/通道 1` 是不同开关。

### 3.2 数字输入 MUX

共 2 个逻辑模块、26 个已用通道：

| 模块名 | 已用数 | 已用通道 |
| --- | ---: | --- |
| Multiplexer | 11 | 0、1、2、8、9、10、11、12、13、14、15 |
| Multiplexer 1 | 15 | 0-8、10-15 |

每次变化应发送 MUX 模块索引、通道和按下/释放状态。不能使用 `DataPin` 代替模块
标识，因为 DataPin 是通道字段在 MCC 中的业务表示，不是唯一设备 ID。

### 3.3 模拟输入

共 6 路：

| 设备 | 配置用途 |
| --- | --- |
| Analog 1 | LIGHT |
| Analog 2 | LDGELEV |
| Analog 3 | AIRCOND_COCKPIT |
| Analog 4 | AIRCOND_FWDCABIN |
| Analog 5 | AIRCOND_AFTCABIN |
| Analog 6 | CARGOHEAT_AFT |

模拟量不能复用二值 `state`。协议必须发送至少 16 位无符号原始值，并携带模拟输入
模块索引或 Arduino 模拟引脚。建议发送固件上报电脑前的同一个最终整数值，ESP32
不要重复实现 MobiFlight 的灵敏度滤波。

## 4. 输出映射

### 4.1 输出移位寄存器

共 5 个逻辑模块、151 个已用通道：

| 模块名 | 已用数 | 未使用通道 |
| --- | ---: | --- |
| Shift APU | 32 | 无 |
| Shift CENTER | 32 | 无 |
| Shift FIRE | 30 | 0、1 |
| Shift LEFT | 32 | 无 |
| Shift RIGHT | 25 | 24-29、31 |

电脑下发给固件的是一个模块更新命令，其中包含模块索引、子模块数量、位掩码值以及
各子模块字节。I2C 应镜像最终完整位图，而不是为 MCC 中 151 条灯光规则硬编码名称。

建议 ESP32 按以下方式维护状态：

```text
outputShifter[moduleIndex][byteIndex] = value
```

随后以 `channel = byteIndex * 8 + bitIndex` 取得单灯状态。必须确认现有固件
`MFOutputShifter::setPins()` 的位序后再固定 ESP32 位序，不能仅依据 MCC 的
`Output N` 字符串猜测。

### 4.2 普通输出

共 7 个：BACKLIGHT、DOME LIGHT、BAT V、STROBE、LOGO、RWY 和 NOSE。当前协议
已按 Arduino GPIO 与开关状态镜像，但 ESP32 必须使用独立映射表，不应把 Arduino
GPIO 直接当作 ESP32 GPIO。

### 4.3 BAT DISPLAY 数码管

配置只有一个名为 `BAT DISPLAY` 的 6 位 LED 数码管模块：

| 配置项 | 使用位 | 数据来源 |
| --- | --- | --- |
| BAT2_DISPLAY | 0、1、2 | BAT 2 电压，保留 1 位小数 |
| BAT1_DISPLAY | 3、4、5 | BAT 1 电压，保留 1 位小数 |

I2C 应镜像 MobiFlight 下发给 `LedSegment::OnSetModule()` 的最终显示命令，至少包含：

- 数码管模块索引；
- 子模块索引；
- 显示字符串；
- 小数点位掩码 `points`；
- 数字位掩码 `mask`。

还应映射亮度命令和单段控制命令，否则 ESP32 端不能完整复现显示模块行为。

## 5. 建议的协议事件类型

现有固定 6 字节 V1 帧只适合二值 GPIO，无法完整表示本配置。建议升级为带长度的 V2
帧，并保留以下事件类型：

| 类型 | 方向 | 载荷 |
| --- | --- | --- |
| `BUTTON` | Arduino -> ESP32 | GPIO、状态 |
| `INPUT_SHIFTER` | Arduino -> ESP32 | 模块、通道、状态 |
| `INPUT_MUX` | Arduino -> ESP32 | 模块、通道、状态 |
| `ANALOG` | Arduino -> ESP32 | 模块或引脚、16 位值 |
| `OUTPUT` | Arduino -> ESP32 | GPIO、状态 |
| `OUTPUT_SHIFTER` | Arduino -> ESP32 | 模块、起始字节、位图数据 |
| `SEGMENT_DISPLAY` | Arduino -> ESP32 | 模块、子模块、文本、points、mask |
| `SEGMENT_BRIGHTNESS` | Arduino -> ESP32 | 模块、子模块、亮度 |
| `SEGMENT_SINGLE` | Arduino -> ESP32 | 模块、子模块、段号、状态 |
| `LCD_TEXT` | Arduino -> ESP32 | LCD 模块、文本 |

`LCD_TEXT` 不属于 V1_0_2 的实际需求，但可作为通用固件扩展。舵机、步进电机、编码器
等类型在本 MCC 中未使用，不需要为了该面板加入映射。

## 6. 标识规则

推荐以固件配置中的模块索引作为总线标识，并由 ESP32 保存一份面板映射表。不要把
MCC 的显示名称写死到 Arduino 固件，原因如下：

- 名称只存在于电脑配置，固件不一定能获得；
- 名称可被用户修改；
- 传输字符串会占用 AVR RAM 和 I2C 带宽；
- I2C 单次事务缓冲通常只有 32 字节。

ESP32 映射键应至少包含 `eventType + moduleIndex + channel`。普通 GPIO 类型可使用
`eventType + gpio`。

## 7. 配置异常与边界

以下两项虽然标记为 Active，但没有绑定设备，不能由 Arduino 映射：

- 输出 `FENIX_A320_OVHD_BUS_ACC`；
- 输入 `FENIX_A320_OVHD_BUTTONRESET`。

它们不会形成发往固件的有效硬件命令或硬件事件。如需同步到 ESP32，必须先在
MobiFlight 中给它们绑定实际设备。

另外，配置顶层名称是 V1_0_2，但嵌入 ConfigFile 的 Label 和文件名仍为 V1_0_1。
这不影响上述硬件统计，但版本发布前应确认附件确实是期望的最终配置。

## 8. 完整覆盖判定

只有满足以下条件，才能称为 V1_0_2 全控件映射：

1. 128 路 InputShiftRegister 的按下和释放均可区分模块与通道；
2. 26 路 InputMultiplexer 的按下和释放均可区分模块与通道；
3. 6 路 AnalogInput 可发送完整数值；
4. 151 路 ShiftRegister 输出可从模块位图还原；
5. 7 路普通 Output 可按 Arduino GPIO 还原；
6. BAT DISPLAY 的 6 位内容、小数点、掩码和亮度可还原；
7. ESP32 对启动时未知状态、丢帧和重启后的状态重建有明确处理。

当前 V2 实现已覆盖以上 1-6 项，并提供通用字符 LCD 文本映射。第 7 项中的输入状态
可借助 MobiFlight 重触发恢复，但输出启动快照、事件序号和失败重传仍不属于当前协议，
ESP32 重启后需要将尚未重新收到的输出状态标记为未知。
