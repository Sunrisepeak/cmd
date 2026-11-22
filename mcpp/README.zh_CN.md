# 用 mcpp 构建 cmd：全部目标

[English](README.md)

mcpp（2026.11.22.1 或更新）在一台机器上为本机或下表中的任一目标构建 `cmd.exe` 与 `COMMAND.COM`：

```sh
mcpp build --release --target <目标>
mcpp run cmd.exe --release --target <目标> -- /c ver
```

## 级别

CI（`.github/workflows/mcpp.yml`）在每个能运行的目标上运行 `.github/mcpp/test.bat`，并按级别评判输出：

| 级别 | 系统提供的能力 | 解释器能做的事 |
|---|---|---|
| L0 | 控制台 | 内建命令、变量、`SET /A`、`IF`、`FOR`、`CALL :label` |
| L1 | L0 加文件 | 批处理文件、`MD`、`COPY`、`REN`、`TYPE`、`DIR`、`DEL`、重定向 |
| L2 | L1 加启动程序 | 外部命令、`%COMSPEC%`、它们的退出码 |

表中是每个目标的最低级别（`.github/mcpp/floor.txt`），低于它即失败。一次运行实际达到的级别写在运行摘要里；
系统补上某项能力后，不必改这里就能看到。低于 L2 的行注明原因。

| 系统 | 目标 | 级别 | CI 中的运行方式 |
|---|---|---|---|
| Linux（glibc） | `x86_64-linux-gnu`、`aarch64-linux-gnu` | L2，终端 | runner |
| Linux（musl，静态） | `x86_64-linux-musl`、`aarch64-linux-musl`、`i686-linux-musl` | L2 | runner |
| Windows（MinGW） | `x86_64-windows-gnu`、`aarch64-windows-gnu` | L2 | runner；Windows arm64 runner |
| Windows（MSVC） | `x86_64-windows-msvc`、`i686-windows-msvc` | L2 | runner（i686 经 WOW64） |
| Windows（musl，POSIX） | `x86_64-windows-musl` | L1 | runner ¹ |
| macOS | `aarch64-macos`、`aarch64-macos-musl`、`x86_64-macos-musl` | L2 | Apple 芯片与 Intel runner |
| PureDarwin（实验） | `x86_64-macos-musl` | L2 | QEMU ² |
| FreeBSD | `x86_64-freebsd`、`aarch64-freebsd` | L2 | FreeBSD 虚拟机 |
| FreeBSD（openkal-musl，静态） | `x86_64-freebsd-musl`、`aarch64-freebsd-musl` | L2 | FreeBSD 虚拟机 |
| NetBSD、OpenBSD | `x86_64-netbsd`、`aarch64-netbsd`、`x86_64-openbsd`、`aarch64-openbsd` | L2 | 该系统的虚拟机 ³ |
| GNU/Hurd、Haiku（preview） | `x86_64-hurd`、`x86_64-haiku` | L2 | 该系统的虚拟机 ³ |
| System V R4 i386（preview） | `i686-svr4-musl` | L2 | QEMU 中的 System V 内核 ⁴ |
| Android | `x86_64-linux-android` / `aarch64-linux-android` | L2 / 仅构建 | 模拟器 |
| openkal 映像 | `x86_64-openkal`、`aarch64-openkal` / `i686-openkal` | L2 / L1 | okrun；FreeBSD 中的自带运行器包 ⁷ |
| WebAssembly | `wasm32-emscripten` | L1（node：终端） | node；Chrome（[在线页面](https://sunrisepeak.github.io/cmd/)）⁵ |
| UEFI | `x86_64-uefi` | L1 | QEMU 与 OVMF ⁶ |
| MS-DOS | `i586-dos-djgpp`、`i686-dos-musl`（实验） | L1 | DOSBox ⁶ |

¹ Windows 上的 POSIX 构建把所启动程序的名字中的 `\` 写成 `/`，Windows 的 `cmd.exe` 会把它当作开关；
在 Windows 上启动程序由 MinGW 与 MSVC 行验证。
² 其上没有 macOS 的 xnu，由其项目的开发者预览映像启动（不作镜像）。
³ `mcpp.platforms` 的行：基于系统自身的基础库构建，在该系统的映像中运行。
⁴ AT&T UNIX System V/386 Release 4.0 Version 3.0（xim 的 `svr4-disk-image`；仅用于技术验证，该系统为专有软件）。
⁵ WebAssembly 没有进程。网页在会话内保留文件，用
`mcpp pack cmd.exe --profile web --target wasm32-emscripten --pkg-format web` 构建。
⁶ openkal-uefi 与 openkal-dos 尚不提供启动程序；DJGPP 既没有 `fork` 也没有 `posix_spawn`。openkal DOS
以大写保存文件名，因此比较输出时不区分大小写。
⁷ i386 映像能启动程序，但 i386 的 okrun 尚不能进入由映像启动的映像（mcpplibs/openkal-image#46）。

## 在虚拟机中运行

在 mcpp 首次使用时安装的虚拟机或模拟器（各虚拟机、QEMU、DOSBox）中运行的目标，需先启用其运行器：
在 `mcpp.toml` 或本机 `config.toml` 中写 `[target.x86_64-uefi] runner = { enabled = true }`，或对单次运行：

```sh
MCPP_TARGET_X86_64_NETBSD_RUNNER_ENABLED=true \
  mcpp run cmd.exe --release --target x86_64-netbsd --file test.bat -- /c test.bat
```

## 目标从哪里来

解释器只用 POSIX 接口（Windows 上用 Win32），不陈述任何目标。openkal 平台的各行在 `mcpp.toml` 中只写一次，
即 `openkal-platform` 0.31 系列；NetBSD、OpenBSD、GNU/Hurd、Haiku 与 DJGPP 行取包索引为它们指名的平台。
生态以后新增的行（mcpp-community/mcpp#1005：`riscv64-linux-musl`、`wasm32-wasi`、iOS 模拟器、裸机上的
openkal 行）在 CI 中只需加一行；平台补上的能力（UEFI 与 DOS 上启动程序）会自行提升该行的级别。
