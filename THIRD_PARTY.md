# 第三方组件与许可范围

根目录MIT仅覆盖薛嘉兴有权授权的代码和文档，不替代以下组件许可。

| 部分 | 使用范围 | 许可处理 |
| --- | --- | --- |
| TI DriverLib、启动及SysConfig输出 | MCU构建从外部MSPM0 SDK取得；生成文件在build | 保留TI BSD三条款声明，副本见LICENSES/TI-BSD-3-Clause.txt |
| ARM CMSIS Core | 由外部TI SDK提供并参与编译 | Apache-2.0，见LICENSES/Apache-2.0.txt及ARM-CMSIS-NOTICE.txt |
| TI编译器运行库 | 编译链接使用外部工具安装包 | 依工具链RTS的实际组件条款，附RTS SPDX清单供核对，不重新授予MIT |
| OLED驱动、字模及位图 | 不分发：mspm0/oled.c、oled.h、oledfont.h、bmp.h | 再分发许可未确认，公开版排除；使用者须合法取得兼容实现，根目录MIT不覆盖 |
| CanMV媒体与machine运行库 | K230板端外部固件，仓库仅调用API | 按实际固件及组件许可证，不把完整SDK或板端固件重新授权为MIT |
| Python、Zig/LLD、Unicorn、pyelftools | 外部构建或验证工具 | 安装包不随仓库分发，不是MCU或K230应用代码的统一许可 |

TI SDK整体还受其发行包许可和组件清单约束，LICENSES/MSPM0-SDK-notices.txt为对应安装包附带文本，不表示全部SDK都为BSD。
SDK、编译器、CanMV固件和工具安装包均未复制进本项目源码目录。

对第三方部分的修改和应用集成不转移其版权。现有版权、作者、许可和免责声明须保留。
公开版已排除 OLED 绘图驱动、字模和位图文件；所需接口与恢复路径见 docs/external-dependencies.md。单独添加 MIT 不补足外部依赖的再分发许可。
