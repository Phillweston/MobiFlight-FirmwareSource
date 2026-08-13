# M5Stack CoreS3 SE I2C Monitor

该工程把 M5Stack CoreS3 SE 配置为 I2C 地址 `0x42` 的 ESP32-S3 从机，接收
MobiFlight Arduino 固件发送的 V2 镜像帧，并在 320x240 屏幕上显示 V1_0_2 的
6 路模拟旋钮状态。

接收端还会通过 MQTT 把状态转发到 Home Assistant，并使用 MQTT Discovery 自动创建
12 个传感器：每路旋钮各一个百分比实体和一个原始值实体。

## 接线

| Arduino | CoreS3 SE 外部 Port A | 说明 |
| --- | --- | --- |
| SDA | SDA，默认 GPIO2 | 数据 |
| SCL | SCL，默认 GPIO1 | 时钟 |
| GND | GND | 必须共地 |

默认引脚在 `include/AppConfig.h` 中设置。不同硬件批次或扩展底座可能改变可用引脚，
接线前必须核对 CoreS3 SE 机身丝印或对应原理图。

Arduino Mega、Uno、Nano 等 5 V 板不能把 I2C 线路上拉到 5 V。建议使用双向 I2C
电平转换器，Arduino 侧上拉到 5 V，CoreS3 侧上拉到 3.3 V。不要连接两块设备的
5 V/3.3 V 电源输出；各自供电时只连接 SDA、SCL 和 GND。

## 屏幕内容

模拟设备索引按 Arduino 固件配置注册顺序映射：

| 索引 | V1_0_2 设备 | 屏幕名称 |
| ---: | --- | --- |
| 0 | Analog 1 | OVHD LIGHT |
| 1 | Analog 2 | LDG ELEV |
| 2 | Analog 3 | COCKPIT TEMP |
| 3 | Analog 4 | FWD CABIN |
| 4 | Analog 5 | AFT CABIN |
| 5 | Analog 6 | CARGO AFT |

每项显示原始 ADC 值和百分比。默认 `ANALOG_RAW_MAX=1023`，适用于 AVR Arduino
的 10 位 ADC；如主控使用其他 ADC 范围，需要修改 `AppConfig.h`。

顶部 `I2C` 为绿色表示最近 5 秒收到过有效协议帧；`WAIT` 表示等待或链路静默。
由于当前协议没有心跳，面板长时间没有事件也会显示 `WAIT`，这不一定代表物理断线。

底部显示：

- `RX`：已进入接收队列的事务数；
- `DROP`：长度错误或队列满导致的丢弃数；
- `BAD`：协议或校验错误数；
- `OTHER`：收到的按钮、灯光等非模拟量有效帧数。

## 构建

烧录前修改 `include/AppConfig.h`：

```cpp
constexpr char WIFI_SSID[] = "你的 Wi-Fi";
constexpr char WIFI_PASSWORD[] = "Wi-Fi 密码";
constexpr char MQTT_HOST[] = "Home Assistant 或 MQTT Broker IP";
constexpr char MQTT_USERNAME[] = "MQTT 用户名";
constexpr char MQTT_PASSWORD[] = "MQTT 密码";
```

Home Assistant 必须已经配置 MQTT Integration，并与 CoreS3 使用同一个 Broker。
固件默认使用 1883 端口的非 TLS MQTT。真实凭据不要提交到 Git。

### MQTT 主题

| 主题 | Retain | 内容 |
| --- | --- | --- |
| `mobiflight/a320_ovhd/status` | 是 | `online` 或遗嘱 `offline` |
| `mobiflight/a320_ovhd/analog/<id>/percent` | 是 | 0-100 |
| `mobiflight/a320_ovhd/analog/<id>/raw` | 是 | Arduino ADC 原始值 |
| `mobiflight/a320_ovhd/event` | 否 | 所有 I2C 事件的 JSON |

事件 JSON 示例：

```json
{"type":"input_shifter","type_id":3,"length":3,"payload_hex":"001101"}
```

Home Assistant Discovery 配置发布在
`homeassistant/sensor/m5stack-cores3-a320-ovhd/.../config`。设备重连 MQTT 时会重新发布
Discovery，并重新发布内存中已有的 6 路旋钮状态。

I2C 接收和屏幕刷新不依赖 Wi-Fi/MQTT 在线。MQTT 离线时事件进入 32 条内存队列；
队列满后增加屏幕底部 `MQD` 计数，旧消息不会写入 Flash。

## 构建和烧录

在本目录执行：

```powershell
platformio run
platformio run --target upload
platformio device monitor
```

依赖为 Arduino-ESP32 和 M5Unified。代码使用 `Wire1` 承载外部 I2C 从机，避免与
M5Unified 管理的屏幕、触摸和内部设备总线互相重新初始化。

完整协议见 [`../../docs/ESP32_I2C_BRIDGE_PROTOCOL.md`](../../docs/ESP32_I2C_BRIDGE_PROTOCOL.md)。
