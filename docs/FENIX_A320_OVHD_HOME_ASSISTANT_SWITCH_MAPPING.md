# FENIX A320 OVHD V1_0_2 开关与 Home Assistant 映射

> 本文档由 `ControlMap.generated.h` 和旧版 `automations.yaml` 生成。推荐使用
> `home_assistant/node_red_flow.json`，修改源文件后运行
> `uv run --with pyyaml python tools/generate_mapping_doc.py` 更新本文档。

## 1. 总览

- 数字输入共 **154** 路，其中 **74** 路绑定家居动作，**80** 路仅监控。
- 全部数字输入都会发布 retained `state`（`ON/OFF`）和非 retained `event`（`PRESS/RELEASE`）。
- 家居动作只监听 `PRESS`。`仅监控` 表示仍有 MQTT 与 HA Discovery 实体，但不会操作家电。
- CoreS3 不保存家居设备信息；推荐在 Node-RED 中把 MQTT 事件连接到 Home Assistant Action 或米家节点。
- 表中的家居动作来自旧版 YAML，仅用于说明原有意图，不代表固件内置设备绑定。

通用主题：

```text
mobiflight/a320_ovhd/control/<object_id>/state
mobiflight/a320_ovhd/control/<object_id>/event
```

## 2. 模拟旋钮

| 索引 | 面板功能 | MQTT/HA 映射 |
| ---: | --- | --- |
| 0 | OVHD LIGHT | 顶灯亮度 0-100%，`home/light/brightness_pct` |
| 1 | LDG ELEV | 仅监控百分比和原始值 |
| 2 | COCKPIT TEMP | Node-RED 空调目标温度 16-30°C，`home/climate/target_c` |
| 3 | FWD CABIN | 仅监控百分比和原始值 |
| 4 | AFT CABIN | 仅监控百分比和原始值 |
| 5 | CARGO AFT | 仅监控百分比和原始值 |

## 3. 全部数字开关

### ADIRS

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| ADIRS ADR1 | Shifter LEFT / 1 (移位输入，按压/释放) | `adirs_adr1_shifter_left_1` | 仅监控 |
| ADIRS IR2 | Shifter LEFT / 2 (移位输入，按压/释放) | `adirs_ir2_shifter_left_2` | 仅监控 |
| ADIRS IR3 | Shifter LEFT / 3 (移位输入，按压/释放) | `adirs_ir3_shifter_left_3` | 仅监控 |
| ADIRS IR1 | Shifter LEFT / 8 (移位输入，按压/释放) | `adirs_ir1_shifter_left_8` | 仅监控 |
| ADIRS IR1 OFF | Shifter LEFT / 9 (移位输入，位置事件) | `adirs_ir1_off_shifter_left_9` | 仅监控 |
| ADIRS ADR2 | Shifter LEFT / 10 (移位输入，按压/释放) | `adirs_adr2_shifter_left_10` | 仅监控 |
| ADIRS ADR3 | Shifter LEFT / 11 (移位输入，按压/释放) | `adirs_adr3_shifter_left_11` | 仅监控 |
| ADIRS IR2 NAV | Shifter LEFT / 16 (移位输入，位置事件) | `adirs_ir2_nav_shifter_left_16` | 仅监控 |
| ADIRS IR2 OFF | Shifter LEFT / 17 (移位输入，位置事件) | `adirs_ir2_off_shifter_left_17` | 仅监控 |
| ADIRS IR2 ATT | Shifter LEFT / 18 (移位输入，位置事件) | `adirs_ir2_att_shifter_left_18` | 仅监控 |
| ADIRS IR1 NAV | Shifter LEFT / 19 (移位输入，位置事件) | `adirs_ir1_nav_shifter_left_19` | 仅监控 |
| ADIRS IR1 ATT | Shifter LEFT / 24 (移位输入，位置事件) | `adirs_ir1_att_shifter_left_24` | 仅监控 |
| ADIRS IR3 NAV | Shifter LEFT / 25 (移位输入，位置事件) | `adirs_ir3_nav_shifter_left_25` | 仅监控 |
| ADIRS IR3 OFF | Shifter LEFT / 26 (移位输入，位置事件) | `adirs_ir3_off_shifter_left_26` | 仅监控 |
| ADIRS IR3 ATT | Shifter LEFT / 27 (移位输入，位置事件) | `adirs_ir3_att_shifter_left_27` | 仅监控 |

### APU/场景

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| APU START | Shifter APU 2 / 13 (移位输入，按压/释放) | `apu_start_shifter_apu_2_13` | 欢迎场景 |
| APU MASTER | Shifter APU 2 / 27 (移位输入，按压/释放) | `apu_master_shifter_apu_2_27` | 欢迎准备场景 |

### GPWS

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| GPWS GSMODE | Shifter LEFT / 0 (移位输入，按压/释放) | `gpws_gsmode_shifter_left_0` | 仅监控 |
| GPWS FLAPMODE | Shifter LEFT / 4 (移位输入，按压/释放) | `gpws_flapmode_shifter_left_4` | 仅监控 |
| GPWS LDGFLAP3 | Shifter LEFT / 15 (移位输入，按压/释放) | `gpws_ldgflap3_shifter_left_15` | 仅监控 |
| GPWS TERR | Shifter LEFT / 28 (移位输入，按压/释放) | `gpws_terr_shifter_left_28` | 仅监控 |
| GPWS SYS | Shifter LEFT / 29 (移位输入，按压/释放) | `gpws_sys_shifter_left_29` | 仅监控 |

### 其他航空控件

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| MODESEL | Shifter APU 2 / 4 (移位输入，按压/释放) | `modesel_shifter_apu_2_4` | 仅监控 |
| DITCHING | Shifter APU 2 / 19 (移位输入，按压/释放) | `ditching_shifter_apu_2_19` | 仅监控 |
| MANVS UP | Shifter APU 2 / 24 (移位输入，按压/释放) | `manvs_up_shifter_apu_2_24` | 仅监控 |
| MANVS UP | Shifter APU 2 / 25 (移位输入，按压/释放) | `manvs_up_shifter_apu_2_25` | 仅监控 |
| FO ENG MANSTART 1 | Shifter RIGHT / 17 (移位输入，按压/释放) | `fo_eng_manstart_1_shifter_right_17` | 仅监控 |
| FO ENG MANSTART 2 | Shifter RIGHT / 18 (移位输入，按压/释放) | `fo_eng_manstart_2_shifter_right_18` | 仅监控 |

### 内部灯光

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| INTLT STBYCOMPASS OFF | Shifter APU 2 / 5 (移位输入，位置事件) | `intlt_stbycompass_off_shifter_apu_2_5` | 夜灯关 |
| INTLT STBYCOMPASS ON | Shifter APU 2 / 6 (移位输入，位置事件) | `intlt_stbycompass_on_shifter_apu_2_6` | 夜灯开 |
| INTLT DOME BRT | Shifter APU 2 / 14 (移位输入，按压/释放) | `intlt_dome_brt_shifter_apu_2_14` | 明亮顶灯场景 |
| INTLT DOME OFF | Shifter APU 2 / 15 (移位输入，按压/释放) | `intlt_dome_off_shifter_apu_2_15` | 顶灯关 |
| INTLT ANNLT TEST | Shifter APU 2 / 28 (移位输入，按压/释放) | `intlt_annlt_test_shifter_apu_2_28` | 全灯测试 5 秒后恢复 |
| INTLT ANNLT DIM | Shifter APU 2 / 29 (移位输入，按压/释放) | `intlt_annlt_dim_shifter_apu_2_29` | 暗光场景 |

### 加热

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| ANTICE ENG1 | Shifter APU / 7 (移位输入，按压/释放) | `antice_eng1_shifter_apu_7` | 左加热切换 |
| ANTICE WING | Shifter APU / 25 (移位输入，按压/释放) | `antice_wing_shifter_apu_25` | 地暖切换 |
| ANTICE ENG2 | Shifter APU / 28 (移位输入，按压/释放) | `antice_eng2_shifter_apu_28` | 右加热切换 |
| WINDOWS HEAT | Shifter APU 2 / 7 (移位输入，按压/释放) | `windows_heat_shifter_apu_2_7` | 除湿/除雾切换 |

### 呼叫

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| CALLS MECH | Multiplexer 1 / 0 (MUX 输入，按压/释放) | `calls_mech_multiplexer_1_0` | 维护通知 |
| CALLS ALL | Multiplexer 1 / 12 (MUX 输入，按压/释放) | `calls_all_multiplexer_1_12` | 全屋通知 |
| CALLS FWD | Multiplexer 1 / 13 (MUX 输入，按压/释放) | `calls_fwd_multiplexer_1_13` | 前舱通知 |
| CALLS AFT | Multiplexer 1 / 14 (MUX 输入，按压/释放) | `calls_aft_multiplexer_1_14` | 后舱通知 |
| CALLS EMER | Multiplexer 1 / 15 (MUX 输入，按压/释放) | `calls_emer_multiplexer_1_15` | 紧急通知 |

### 外部灯光

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| EXT LT RWTTURNOFF OFF | Shifter APU / 4 (移位输入，位置事件) | `ext_lt_rwtturnoff_off_shifter_apu_4` | 走廊灯关 |
| EXT LT STROBE ON | Shifter APU / 5 (移位输入，按压/释放) | `ext_lt_strobe_on_shifter_apu_5` | 仅监控 |
| EXT LT STROBE OFF | Shifter APU / 6 (移位输入，按压/释放) | `ext_lt_strobe_off_shifter_apu_6` | 仅监控 |
| EXT LT WING OFF | Shifter APU / 12 (移位输入，位置事件) | `ext_lt_wing_off_shifter_apu_12` | 辅助灯关 |
| EXT LT LAND L ON | Shifter APU / 13 (移位输入，按压/释放) | `ext_lt_land_l_on_shifter_apu_13` | 左灯区开 |
| EXT LT LAND L RETRACT | Shifter APU / 14 (移位输入，按压/释放) | `ext_lt_land_l_retract_shifter_apu_14` | 左灯区关 |
| EXT LT RWTTURNOFF ON | Shifter APU / 15 (移位输入，位置事件) | `ext_lt_rwtturnoff_on_shifter_apu_15` | 走廊灯开 |
| EXT LT NOSE TO | Shifter APU / 20 (移位输入，按压/释放) | `ext_lt_nose_to_shifter_apu_20` | 入口灯开 |
| EXT LT LAND R ON | Shifter APU / 21 (移位输入，按压/释放) | `ext_lt_land_r_on_shifter_apu_21` | 右灯区开 |
| EXT LT LAND R RETRACT | Shifter APU / 22 (移位输入，按压/释放) | `ext_lt_land_r_retract_shifter_apu_22` | 右灯区关 |
| EXT LT WING ON | Shifter APU / 23 (移位输入，位置事件) | `ext_lt_wing_on_shifter_apu_23` | 辅助灯开 |
| EXT LT BEACON OFF | Shifter APU / 26 (移位输入，位置事件) | `ext_lt_beacon_off_shifter_apu_26` | 主顶灯关 |
| EXT LT BEACON ON | Shifter APU / 27 (移位输入，位置事件) | `ext_lt_beacon_on_shifter_apu_27` | 主顶灯开 |
| EXT LT NAV&LOGO NAV | Shifter APU / 29 (移位输入，按压/释放) | `ext_lt_nav_logo_nav_shifter_apu_29` | 装饰灯开 |
| EXT LT NAV&LOGO OFF | Shifter APU / 30 (移位输入，按压/释放) | `ext_lt_nav_logo_off_shifter_apu_30` | 装饰灯关 |
| EXT LT NOSE OFF | Shifter APU / 31 (移位输入，按压/释放) | `ext_lt_nose_off_shifter_apu_31` | 入口灯关 |

### 应急电源

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| EMERELECPWR GEN1LINE | Shifter LEFT / 13 (移位输入，按压/释放) | `emerelecpwr_gen1line_shifter_left_13` | 仅监控 |
| EMERELECPWR EMERGENTEST | Shifter LEFT / 14 (移位输入，按压/释放) | `emerelecpwr_emergentest_shifter_left_14` | 仅监控 |
| EMERELECPWR EMERGEN | Shifter LEFT / 23 (移位输入，按压/释放) | `emerelecpwr_emergen_shifter_left_23` | 仅监控 |

### 撤离/通知

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| EVAC HORNSHUT | Shifter LEFT / 20 (移位输入，按压/释放) | `evac_hornshut_shifter_left_20` | 静音通知 |
| EVAC CAPT | Shifter LEFT / 21 (移位输入，按压/释放) | `evac_capt_shifter_left_21` | CAPTAIN 通知 |
| EVAC PURS PURS | Shifter LEFT / 22 (移位输入，位置事件) | `evac_purs_purs_shifter_left_22` | PURSER 通知 |
| EVAC COMMAND | Shifter LEFT / 31 (移位输入，按压/释放) | `evac_command_shifter_left_31` | 紧急通知 |

### 标志/模式

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| SIGNS SEARBELTS OFF | Shifter APU 2 / 12 (移位输入，位置事件) | `signs_searbelts_off_shifter_apu_2_12` | 安静模式关 |
| SIGNS NO SMOKING ON | Shifter APU 2 / 21 (移位输入，按压/释放) | `signs_no_smoking_on_shifter_apu_2_21` | 净化器开 |
| SIGNS NO SMOKING OFF | Shifter APU 2 / 22 (移位输入，按压/释放) | `signs_no_smoking_off_shifter_apu_2_22` | 净化器关 |
| SIGNS SEARBELTS ON | Shifter APU 2 / 23 (移位输入，位置事件) | `signs_searbelts_on_shifter_apu_2_23` | 安静模式开 |
| SIGNS ON | Shifter APU 2 / 30 (移位输入，按压/释放) | `signs_on_shifter_apu_2_30` | 回家场景 |
| SIGNS OFF | Shifter APU 2 / 31 (移位输入，按压/释放) | `signs_off_shifter_apu_2_31` | 离家场景 |

### 氧气

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| OXYGEN CREW | Multiplexer 1 / 8 (MUX 输入，按压/释放) | `oxygen_crew_multiplexer_1_8` | 仅监控 |
| OXYGEN MASK | Multiplexer 1 / 10 (MUX 输入，按压/释放) | `oxygen_mask_multiplexer_1_10` | 仅监控 |
| OXYGEN HIGH ALT | Multiplexer 1 / 11 (MUX 输入，按压/释放) | `oxygen_high_alt_multiplexer_1_11` | 仅监控 |

### 液压

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| HYD PTU | InputShifter / 4 (移位输入，按压/释放) | `hyd_ptu_inputshifter_4` | 仅监控 |
| HYD BLUEELEC | InputShifter / 5 (移位输入，按压/释放) | `hyd_blueelec_inputshifter_5` | 仅监控 |
| HYD RATMAN | InputShifter / 7 (移位输入，按压/释放) | `hyd_ratman_inputshifter_7` | 仅监控 |
| HYD YELLOPUMP | InputShifter / 13 (移位输入，按压/释放) | `hyd_yellopump_inputshifter_13` | 仅监控 |
| HYD YELLOWENG2 | InputShifter / 14 (移位输入，按压/释放) | `hyd_yelloweng2_inputshifter_14` | 仅监控 |
| HYD GREENENG1 | Shifter APU / 1 (移位输入，按压/释放) | `hyd_greeneng1_shifter_apu_1` | 仅监控 |

### 火警

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| FIRE ENG1 AGENT1 | Multiplexer / 0 (MUX 输入，按压/释放) | `fire_eng1_agent1_multiplexer_0` | 仅监控 |
| FIRETEST ENG1 | Multiplexer / 1 (MUX 输入，按压/释放) | `firetest_eng1_multiplexer_1` | 仅监控 |
| FIRE FIRE1 PUSH | Multiplexer / 2 (MUX 输入，按压/释放) | `fire_fire1_push_multiplexer_2` | 仅监控 |
| FIRE APU AGENT | Multiplexer / 8 (MUX 输入，按压/释放) | `fire_apu_agent_multiplexer_8` | 仅监控 |
| FIRETEST APU | Multiplexer / 9 (MUX 输入，按压/释放) | `firetest_apu_multiplexer_9` | 仅监控 |
| FIRE APU PUSH | Multiplexer / 10 (MUX 输入，按压/释放) | `fire_apu_push_multiplexer_10` | 仅监控 |
| FIRE FIRE2 PUSH | Multiplexer / 11 (MUX 输入，按压/释放) | `fire_fire2_push_multiplexer_11` | 仅监控 |
| FIRETEST ENG2 | Multiplexer / 12 (MUX 输入，按压/释放) | `firetest_eng2_multiplexer_12` | 仅监控 |
| FIRE ENG2 AGENT2 | Multiplexer / 13 (MUX 输入，按压/释放) | `fire_eng2_agent2_multiplexer_13` | 仅监控 |
| FIRE ENG2 AGENT1 | Multiplexer / 14 (MUX 输入，按压/释放) | `fire_eng2_agent1_multiplexer_14` | 仅监控 |
| FIRE ENG1 AGENT2 | Multiplexer / 15 (MUX 输入，按压/释放) | `fire_eng1_agent2_multiplexer_15` | 仅监控 |

### 燃油

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| FUEL LTK2 | InputShifter / 0 (移位输入，按压/释放) | `fuel_ltk2_inputshifter_0` | 仅监控 |
| FUEL CTR1 | InputShifter / 1 (移位输入，按压/释放) | `fuel_ctr1_inputshifter_1` | 仅监控 |
| FUEL CTRMODESEL | InputShifter / 2 (移位输入，按压/释放) | `fuel_ctrmodesel_inputshifter_2` | 仅监控 |
| FUEL CTR2 | InputShifter / 3 (移位输入，按压/释放) | `fuel_ctr2_inputshifter_3` | 仅监控 |
| FUEL XFEED | InputShifter / 6 (移位输入，按压/释放) | `fuel_xfeed_inputshifter_6` | 仅监控 |
| FUEL RTK2 | InputShifter / 8 (移位输入，按压/释放) | `fuel_rtk2_inputshifter_8` | 仅监控 |
| FUEL RTK1 | InputShifter / 10 (移位输入，按压/释放) | `fuel_rtk1_inputshifter_10` | 仅监控 |
| FUEL LTK1 | Shifter APU / 2 (移位输入，按压/释放) | `fuel_ltk1_shifter_apu_2` | 仅监控 |

### 电气/场景

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| ELEC GEN1 | InputShifter / 9 (移位输入，按压/释放) | `elec_gen1_inputshifter_9` | 左区域场景 |
| ELEC GEN2 | InputShifter / 12 (移位输入，按压/释放) | `elec_gen2_inputshifter_12` | 右区域场景 |
| ELEC BAT1 | InputShifter / 20 (移位输入，按压/释放) | `elec_bat1_inputshifter_20` | 左房间场景 |
| ELEC ACESS | InputShifter / 21 (移位输入，按压/释放) | `elec_acess_inputshifter_21` | 仅监控 |
| ELEC EXRPWR | InputShifter / 22 (移位输入，按压/释放) | `elec_exrpwr_inputshifter_22` | 离家场景 |
| ELEC IDG2 | InputShifter / 23 (移位输入，按压/释放) | `elec_idg2_inputshifter_23` | 仅监控 |
| ELEC APUGEN | InputShifter / 27 (移位输入，按压/释放) | `elec_apugen_inputshifter_27` | 仅监控 |
| ELEC BUSTIE | InputShifter / 28 (移位输入，按压/释放) | `elec_bustie_inputshifter_28` | 全屋场景 |
| ELEC BAT2 | InputShifter / 29 (移位输入，按压/释放) | `elec_bat2_inputshifter_29` | 右房间场景 |
| ELEC COMMERCIAL | Shifter APU / 3 (移位输入，按压/释放) | `elec_commercial_shifter_apu_3` | 回家场景 |
| ELEC IDG | Shifter APU / 10 (移位输入，按压/释放) | `elec_idg_shifter_apu_10` | 仅监控 |
| ELEC GALY | Shifter APU / 11 (移位输入，按压/释放) | `elec_galy_shifter_apu_11` | 厨房场景 |

### 空调/新风

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| AIRCOND PACK FLOW LO | Shifter APU / 8 (移位输入，位置事件) | `aircond_pack_flow_lo_shifter_apu_8` | 空调风速低 |
| AIRCOND PACK FLOW NORM | Shifter APU / 9 (移位输入，位置事件) | `aircond_pack_flow_norm_shifter_apu_9` | 空调风速中 |
| AIRCOND PACK FLOW HI | Shifter APU / 16 (移位输入，位置事件) | `aircond_pack_flow_hi_shifter_apu_16` | 空调风速高 |
| AIRCOND PACK1 | Shifter APU / 17 (移位输入，按压/释放) | `aircond_pack1_shifter_apu_17` | 空调开机 |
| AIRCOND ENG1BLEED | Shifter APU / 18 (移位输入，按压/释放) | `aircond_eng1bleed_shifter_apu_18` | 左空调区切换 |
| AIRCOND RAMAIR | Shifter APU / 19 (移位输入，按压/释放) | `aircond_ramair_shifter_apu_19` | 新风切换 |
| AIRCOND APUBLEED | Shifter APU / 24 (移位输入，按压/释放) | `aircond_apubleed_shifter_apu_24` | 通风切换 |
| AIRCOND XBLEED SHUT | Shifter APU 2 / 0 (移位输入，按压/释放) | `aircond_xbleed_shut_shifter_apu_2_0` | 新风关 |
| AIRCOND XBLEED AUTO | Shifter APU 2 / 1 (移位输入，按压/释放) | `aircond_xbleed_auto_shifter_apu_2_1` | 新风自动 |
| AIRCOND XBLEED OPEN | Shifter APU 2 / 2 (移位输入，按压/释放) | `aircond_xbleed_open_shifter_apu_2_2` | 新风 100% |
| AIRCOND ENG2BLEED | Shifter APU 2 / 16 (移位输入，按压/释放) | `aircond_eng2bleed_shifter_apu_2_16` | 右空调区切换 |
| AIRCOND HOTAIR | Shifter APU 2 / 17 (移位输入，按压/释放) | `aircond_hotair_shifter_apu_2_17` | 空调制热 |
| AIRCOND PACK2 | Shifter APU 2 / 18 (移位输入，按压/释放) | `aircond_pack2_shifter_apu_2_18` | 空调开机 |

### 记录器

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| RCDR GNDCTL | Multiplexer 1 / 5 (MUX 输入，按压/释放) | `rcdr_gndctl_multiplexer_1_5` | 仅监控 |
| RCDR CVRERASE | Multiplexer 1 / 6 (MUX 输入，按压/释放) | `rcdr_cvrerase_multiplexer_1_6` | 仅监控 |
| RCDR CVRTEST | Multiplexer 1 / 7 (MUX 输入，按压/释放) | `rcdr_cvrtest_multiplexer_1_7` | 仅监控 |

### 货舱烟雾

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| FO CARGOSMOKE TEST | Shifter RIGHT / 20 (移位输入，按压/释放) | `fo_cargosmoke_test_shifter_right_20` | 仅监控 |
| FO CARGOSMOKE AGENT2 R | Shifter RIGHT / 26 (移位输入，按压/释放) | `fo_cargosmoke_agent2_r_shifter_right_26` | 仅监控 |
| FO CARGOSMOKE AGENT1 R | Shifter RIGHT / 27 (移位输入，按压/释放) | `fo_cargosmoke_agent1_r_shifter_right_27` | 仅监控 |
| FO CARGOSMOKE AGENT2 L | Shifter RIGHT / 29 (移位输入，按压/释放) | `fo_cargosmoke_agent2_l_shifter_right_29` | 仅监控 |
| FO CARGOSMOKE AGENT1 L | Shifter RIGHT / 30 (移位输入，按压/释放) | `fo_cargosmoke_agent1_l_shifter_right_30` | 仅监控 |

### 通风

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| FO VENTILATION BLOWER | Shifter RIGHT / 7 (移位输入，位置事件) | `fo_ventilation_blower_shifter_right_7` | 风扇 33% |
| VENTILATION | Shifter RIGHT / 19 (移位输入，按压/释放) | `ventilation_shifter_right_19` | 家用风扇切换 |
| FO VENTILATION EXTRACT | Shifter RIGHT / 24 (移位输入，按压/释放) | `fo_ventilation_extract_shifter_right_24` | 风扇 66% |
| FO VENTILATION CABFANS | Shifter RIGHT / 25 (移位输入，按压/释放) | `fo_ventilation_cabfans_shifter_right_25` | 风扇 100% |

### 雨刷/清洁

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| RAINRPLNT | Multiplexer 1 / 1 (MUX 输入，按压/释放) | `rainrplnt_multiplexer_1_1` | 窗户清洁场景 |
| WIPER OFF | Multiplexer 1 / 2 (MUX 输入，位置事件) | `wiper_off_multiplexer_1_2` | 扫地机回充 |
| WIPER SLOW | Multiplexer 1 / 3 (MUX 输入，位置事件) | `wiper_slow_multiplexer_1_3` | 扫地机启动 |
| WIPER FAST | Multiplexer 1 / 4 (MUX 输入，位置事件) | `wiper_fast_multiplexer_1_4` | 扫地机强力启动 |
| FO WIPER SLOW | Shifter RIGHT / 9 (移位输入，位置事件) | `fo_wiper_slow_shifter_right_9` | 窗帘关闭 |
| FO WIPER SLOW | Shifter RIGHT / 10 (移位输入，位置事件) | `fo_wiper_slow_shifter_right_10` | 窗帘打开 |
| FO WIPER OFF | Shifter RIGHT / 11 (移位输入，位置事件) | `fo_wiper_off_shifter_right_11` | 窗帘停止 |
| FO RAIN RPLNT | Shifter RIGHT / 16 (移位输入，按压/释放) | `fo_rain_rplnt_shifter_right_16` | 扫地机启动 |

### 飞控

| 开关 | 硬件地址 | Object ID | 当前映射 |
| --- | --- | --- | --- |
| FLTCTL ELAC1 | Shifter LEFT / 5 (移位输入，按压/释放) | `fltctl_elac1_shifter_left_5` | 仅监控 |
| FLTCTL SEC1 | Shifter LEFT / 6 (移位输入，按压/释放) | `fltctl_sec1_shifter_left_6` | 仅监控 |
| FLTCTL FAC1 | Shifter LEFT / 7 (移位输入，按压/释放) | `fltctl_fac1_shifter_left_7` | 仅监控 |
| FO FLTCTL ELAC2 | Shifter RIGHT / 0 (移位输入，按压/释放) | `fo_fltctl_elac2_shifter_right_0` | 仅监控 |
| FO FLTCTL SEC2 | Shifter RIGHT / 1 (移位输入，按压/释放) | `fo_fltctl_sec2_shifter_right_1` | 仅监控 |
| FO FLTCTL SEC3 | Shifter RIGHT / 2 (移位输入，按压/释放) | `fo_fltctl_sec3_shifter_right_2` | 仅监控 |
| FO FLTCTL FAC2 | Shifter RIGHT / 3 (移位输入，按压/释放) | `fo_fltctl_fac2_shifter_right_3` | 仅监控 |
| FO FLTCTL CARGOHEAT ISOL | Shifter RIGHT / 22 (移位输入，按压/释放) | `fo_fltctl_cargoheat_isol_shifter_right_22` | 仅监控 |
| FO FLTCTL CARGOHEAT HOT | Shifter RIGHT / 23 (移位输入，按压/释放) | `fo_fltctl_cargoheat_hot_shifter_right_23` | 仅监控 |

## 4. 使用注意事项

1. `.mfproj` 不包含 Arduino 模块注册索引。必须先核对 `AppConfig.h` 中的 `INPUT_SHIFTER_INDEX_*` 和 `INPUT_MUX_INDEX_*`。
2. `FO WIPER SLOW` 在配置中重复两次（通道 9、10），当前分别映射为窗帘关闭和打开，需实机确认位置。
3. `MANVS UP` 在配置中重复两次（通道 24、25），目前均仅监控。
4. 火警、氧气、飞控、燃油、液压、ADIRS、GPWS、记录器、货舱烟雾等控件故意不绑定家居动作。
5. CoreS3 重启后协议不会自动发送所有输入快照；未再次变化的开关状态应视为未知。
6. 加热、空调和大功率设备应在 Home Assistant 端增加温度、在线状态和最长运行时间保护。
