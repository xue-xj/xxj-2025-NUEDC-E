# 外部依赖与公开版构建

## OLED 依赖

以下文件因尚未确认再分发授权而不随公开仓库提供：

- `mspm0/oled.c`
- `mspm0/oled.h`
- `mspm0/oledfont.h`
- `mspm0/bmp.h`

自行从合法渠道取得相应授权，或实现兼容的绘图接口。接口调用以 `mspm0/gimbal_display.c`、`gimbal_app.c` 和 `main.c` 等源码为准；接线见 `docs/wiring.md`。
保留本仓库 `gimbal_display.c/.h` 的显示调度与非阻塞 I²C 发送逻辑，补齐文件须与该逻辑兼容，不能用任意同名库直接替换而不核对。
`mspm0/sources.json` 和 CCS 工程保留 OLED 源路径；缺少依赖时，MCU 完整构建会失败，不会静默省略显示功能。

## 外部工具与运行库

- MSPM0 SDK 2.10.00.04，SysConfig 1.26.2，TI ArmClang 4.0.4 LTS 或核实兼容的版本。
- K230 使用支持 `media.sensor`、ST7701 显示、TOUCH 和 UART2 的 CanMV 固件。
- 自动化测试使用 Python、Zig/LLD、Unicorn、pyelftools。SDK、工具和板端固件安装包不分发。

补齐后按 `docs/deployment.md` 构建与测试。公开版可独立运行不依赖板端库的 Python 逻辑测试：

```powershell
$env:PYTHONPATH = (Resolve-Path k230).Path
python -m unittest discover -s tests -p 'test_*.py' -v
```

完整本地工程的 ARM 编译和模拟回归记录不代替摄像头、触摸、STEP 波形及机械跟踪的实板验收。
