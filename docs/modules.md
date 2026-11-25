# 模块与阅读顺序

## MCU

main → gimbal_app → gimbal_control → step_motor。

- gimbal_control只计算动作，不访问寄存器；模式、扫描路径和手动步进在此维护。
- gimbal_protocol只在12字节全部有效时发布命令；uart在ISR内写最新完整命令，主循环短临界区复制。
- motion_math区分毫度、度每秒与脉冲数；跟踪不再进行角度换算。
- step_motor独占两路定时器，CC0_UP为当前向上PWM的下降沿；停止后ODIS强制STEP为低。
- 定时器使能保持预设计数（NO_CHANGE），输出压低时预运行1μs清除锁存，再从LOAD起步，避免重新使能带来额外边沿。
- gimbal_display每轮最多启动一笔不超过8字节的I2C事务，等待在后续轮询完成，错误或超时仅停显示。
- oled绘图函数仅操作显存，应用不调用其阻塞式OLED_Init／OLED_Clear／OLED_Refresh。
- key中断只置事件，主循环消抖和切换；SysTick提供1ms应用时基。

## K230

main → app → detection／tracking → protocol；threshold_ui负责触摸编辑。

| 模块 | 职责 |
| --- | --- |
| config | 目标点、识别过滤、PI、步数上限和串口映射 |
| geometry | 夹角和对角线交点 |
| detection | 矩形筛选与最小面积目标选择 |
| tracking | 连续帧确认、短时保持、PI和方向计算 |
| pi | 按帧更新的增量PI，无微分项 |
| protocol | 编码、字段校验与发送完成检查 |
| thresholds | 0～255阈值上下界约束 |
| threshold_ui | 二值预览、触摸操作及应用／返回 |
| preview | 轮廓、中心、十字和状态叠加 |
| app | 板端SDK初始化、循环与退出清理 |

硬件SDK在app.run中导入，纯算法模块可直接在CPython中测试。

## 数据流和时序

图像 → 几何筛选 → 帧确认 → PI → 12字节方向／步数帧 → MCU原子接收 → 控制动作 → PWM。
视觉有效命令优先于扫描下一段；相邻命令使用最新完整帧，不保证每一帧都排队执行。
手动轮询间隔20ms，模式消抖30ms，显示最短刷新间隔100ms；实际显示帧率由I2C传输速度决定。
参数见gimbal_config.h和k230/gimbal/config.py。步进为开环脉冲驱动，没有编码器或机械限位反馈。
