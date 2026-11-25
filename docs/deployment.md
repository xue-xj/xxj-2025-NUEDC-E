# 构建与部署

> 公开版未提供 OLED 绘图及字模文件。完整 MCU 构建前先按 [外部依赖说明](external-dependencies.md) 补齐；本文完整构建结果对应本地依赖齐全的工程。

## MCU

CCS导入mspm0目录，工程名gimbal_controller。安装MSPM0 SDK 2.10.00.04、SysConfig 1.26.2、TI ArmClang 4.0.4 LTS或兼容版本。
根目录C文件由CCS托管构建发现；sources.json提供命令行源清单，不加入tests或外部build目录。
执行Clean/Rebuild并确认gimbal_display、gimbal_control、gimbal_protocol等文件参与编译。

命令行也可完整生成配置与固件：

```powershell
$env:MSPM0_SDK = "C:\TI\mspm0_sdk_2_10_00_04"
$env:SYSCONFIG_ROOT = "C:\TI\sysconfig_1.26.2"
$env:TI_ARMCLANG = "E:\ti\ccs2050\ccs\tools\compiler\ti-cgt-armllvm_4.0.4.LTS\bin\tiarmclang.exe"
python tools\build.py
```

输出build/gimbal.out、gimbal.bin、gimbal.hex及gimbal.map；BIN起始地址0。
导出BIN／HEX只包含主Flash，不改BCR／BSL配置。优先通过CCS使用OUT并核对下载配置，勿随意改安全配置。
向上PWM启用CC0_UP中断（下降沿），不要将其改为LOAD后仍沿用本预算停止逻辑。

## K230

使用支持media.sensor、ST7701 Display、TOUCH和UART2的CanMV固件。摄像头为QVGA RGB565。
将k230/main.py和整个k230/gimbal目录同时复制到板端同一脚本根目录，运行main.py。
更改硬件时核对FPIOA映射与屏幕驱动，不只替换开发板名称。
触摸阈值应用仅保存在RAM，离线指无需电脑操作，不代表断电保存。

## 自动化测试

Python 3.9+，测试工具不作为板端依赖，可安装到本目录.tools：

```powershell
python -m pip install --target .tools ziglang==0.13.0 unicorn pyelftools
$env:ZIG = (Resolve-Path .tools\ziglang\zig.exe).Path
python tools\build.py
python tools\test.py
```

9项Python逻辑测试，5组ARM核心／协议／控制／电机寄存器契约／显示状态机测试。
后两组采用模拟内存寄存器，不模拟真实定时器波形或I2C电气行为。

官方API：[UART](https://www.kendryte.com/k230_canmv/en/v1.3/api/machine/K230_CanMV_UART_API_Manual.html)、[TOUCH](https://www.kendryte.com/k230_canmv/en/main/api/machine/k230_canmv_touch_api_manual.html)。
软件接口及硬件时序仍需与实际CanMV固件核对。
