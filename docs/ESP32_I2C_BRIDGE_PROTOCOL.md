# Arduino-ESP32 I2C 镜像协议 V2

## 1. 总线参数

Arduino 是 I2C 主机，ESP32 是从机。默认 7 位从机地址为 `0x42`，总线频率为
400 kHz，方向为 Arduino 到 ESP32。Arduino 的原串口通信和本地硬件控制不变。

Arduino Mega、Uno、Nano 等 5 V 板必须使用双向 I2C 电平转换器，或者确保 SDA、
SCL 只上拉到 3.3 V。两端必须共地。地址 `0x42` 不能与 LCD 或其他设备冲突。

## 2. 通用帧

每个 I2C 写事务包含一个完整帧，最大 32 字节：

| 偏移 | 字段 | 长度 | 说明 |
| --- | --- | ---: | --- |
| 0 | Magic | 1 | 固定 `0xA5` |
| 1 | Version | 1 | 固定 `0x02` |
| 2 | Type | 1 | 事件类型 |
| 3 | Length | 1 | Payload 字节数，范围 0-27 |
| 4 | Payload | Length | 类型相关载荷 |
| 4 + Length | Checksum | 1 | 前面所有字节的 XOR |

接收端将整帧所有字节异或，结果为 `0` 表示校验通过。实际事务长度必须等于
`Length + 5`。

所有 16 位整数均采用小端序，即低字节在前。模块编号是设备在 Arduino 固件配置中的
注册顺序，从 0 开始；配置加载顺序改变时编号也可能改变。

## 3. 事件类型

### 3.1 普通按钮 `0x01`

```text
Payload: [arduinoGpio, state]
state: 1=按下，0=释放
```

### 3.2 普通输出 `0x02`

```text
Payload: [arduinoGpio, state]
state: 1=电脑下发非零值，0=电脑下发零值
```

### 3.3 输入移位寄存器 `0x03`

```text
Payload: [module, channel, state]
channel: 整条移位寄存器链的通道号
state: 1=按下，0=释放
```

V1_0_2 使用 5 个模块和 128 个已配置通道。

### 3.4 数字输入 MUX `0x04`

```text
Payload: [module, channel, state]
channel: 0-15
state: 1=按下，0=释放
```

V1_0_2 使用 2 个模块和 26 个已配置通道。

### 3.5 模拟输入 `0x05`

```text
Payload: [module, arduinoAnalogPin, valueLow, valueHigh]
value = valueLow | (valueHigh << 8)
```

`module` 是模拟设备注册索引。数值是 Arduino 经过平均和灵敏度判断后上报给电脑的
同一数值。V1_0_2 使用 6 路，索引 0-5 对应 Analog 1-6。

### 3.6 输出移位寄存器 `0x06`

```text
Payload: [module, value, byteCount, masks...]
```

- `value` 非零时，将每个 `masks[i]` 对应的位设置为 1；为零时清零；
- `byteCount` 是后续 mask 字节数；
- ESP32 应维护每个模块的完整字节数组；
- 更新规则与 Arduino 一致：

```cpp
if (value != 0)
    state[module][i] |= masks[i];
else
    state[module][i] &= ~masks[i];
```

V1_0_2 使用 5 个模块和 151 个已配置输出通道。

### 3.7 数码管显示 `0x07`

该类型可能分片：

```text
Payload: [module, offset, totalLength, chunkLength,
          subModule, points, mask, textChunk...]
```

ESP32 按 `module + subModule` 保存组包缓冲。当 `offset + chunkLength == totalLength`
时显示完整文本。`points` 是小数点掩码，`mask` 是需要更新的数字位掩码。

V1_0_2 的 BAT DISPLAY 是一个 6 位模块：BAT2 使用 0-2 位，BAT1 使用 3-5 位。

### 3.8 数码管亮度 `0x08`

```text
Payload: [module, subModule, brightness]
```

### 3.9 数码管单段控制 `0x09`

```text
Payload: [module, subModule, segment, state]
```

### 3.10 字符 LCD `0x0A`

该类型也使用分片：

```text
Payload: [lcdModule, offset, totalLength, chunkLength, textChunk...]
```

V1_0_2 当前没有字符 LCD，此类型用于通用固件兼容。

## 4. ESP32 接收框架

I2C 回调只应复制帧到队列。校验、组包、日志、网络发送和 GPIO 操作应在 `loop()`
或独立任务中执行。

```cpp
#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t I2C_ADDRESS = 0x42;
constexpr size_t MAX_FRAME = 32;
constexpr size_t QUEUE_SIZE = 16;
uint8_t outputShifterState[5][4] = {};

struct RxFrame {
    uint8_t length;
    uint8_t data[MAX_FRAME];
};

volatile uint8_t queueHead = 0;
volatile uint8_t queueTail = 0;
volatile uint32_t queueDrops = 0;
RxFrame rxQueue[QUEUE_SIZE];

void onI2cReceive(int count)
{
    const uint8_t next = (queueHead + 1) % QUEUE_SIZE;
    if (count <= 0 || count > MAX_FRAME || next == queueTail) {
        while (Wire.available()) Wire.read();
        ++queueDrops;
        return;
    }

    RxFrame &slot = rxQueue[queueHead];
    slot.length = 0;
    while (Wire.available() && slot.length < MAX_FRAME)
        slot.data[slot.length++] = static_cast<uint8_t>(Wire.read());
    queueHead = next;
}

bool validate(const RxFrame &frame)
{
    if (frame.length < 5 || frame.data[0] != 0xA5 || frame.data[1] != 0x02)
        return false;
    if (frame.length != static_cast<uint8_t>(frame.data[3] + 5))
        return false;

    uint8_t checksum = 0;
    for (uint8_t i = 0; i < frame.length; ++i)
        checksum ^= frame.data[i];
    return checksum == 0;
}

void processFrame(const RxFrame &frame)
{
    const uint8_t type = frame.data[2];
    const uint8_t length = frame.data[3];
    const uint8_t *payload = &frame.data[4];

    switch (type) {
    case 0x01: // Button
    case 0x02: // Output
        if (length == 2) {
            const uint8_t gpio = payload[0];
            const bool state = payload[1] != 0;
            // 使用 type + gpio 更新 ESP32 映射。
        }
        break;

    case 0x03: // InputShifter
    case 0x04: // InputMux
        if (length == 3) {
            const uint8_t module = payload[0];
            const uint8_t channel = payload[1];
            const bool state = payload[2] != 0;
            // 使用 type + module + channel 更新状态。
        }
        break;

    case 0x05: // Analog
        if (length == 4) {
            const uint8_t module = payload[0];
            const uint8_t pin = payload[1];
            const uint16_t value = payload[2] | (uint16_t(payload[3]) << 8);
            // 更新模拟量。
        }
        break;

    case 0x06: // OutputShifter
        if (length >= 3 && length == uint8_t(payload[2] + 3)) {
            const uint8_t module = payload[0];
            const bool setBits = payload[1] != 0;
            const uint8_t byteCount = payload[2];
            for (uint8_t i = 0; i < byteCount; ++i) {
                if (setBits) outputShifterState[module][i] |= payload[3 + i];
                else outputShifterState[module][i] &= ~payload[3 + i];
            }
        }
        break;

    case 0x07: // Segment display: 按 offset 组包
    case 0x08: // Segment brightness
    case 0x09: // Segment single
    case 0x0A: // LCD text: 按 offset 组包
        // 按第 3 节的载荷定义处理。
        break;
    }
}

void setup()
{
    Wire.setPins(21, 22);
    Wire.onReceive(onI2cReceive);
    Wire.begin(I2C_ADDRESS);
}

void loop()
{
    while (queueTail != queueHead) {
        noInterrupts();
        RxFrame frame = rxQueue[queueTail];
        queueTail = (queueTail + 1) % QUEUE_SIZE;
        interrupts();

        if (validate(frame))
            processFrame(frame);
    }
}
```

示例中的 `outputShifterState` 已按 V1_0_2 的 5 个四字节模块声明；用于其他配置时
需要按实际模块数和级联字节数调整。

不同 Arduino-ESP32 Core 版本的 I2C 从机初始化 API 可能不同。如 `Wire.begin(address)`
不可用，应按所用 Core 的从机示例调整初始化，协议解析不变。

## 5. 状态与可靠性边界

协议是事件镜像，没有 ACK、序号、重发、心跳或启动快照：

- ESP32 重启后，应将所有状态标记为未知，直到收到对应事件；
- Arduino 的输入重触发命令会重新发送输入状态，但输出没有自动快照；
- I2C 发送失败不会影响 Arduino 原有串口通信和实体输出；
- 长时间无事件不代表离线；
- 正式 ESP32 程序应统计长度错误、校验错误、未知类型和队列溢出。

若业务要求可靠恢复，需要在后续协议中增加序号、状态快照和确认重传。
