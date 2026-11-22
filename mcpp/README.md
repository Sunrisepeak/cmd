# cmd with mcpp: every target

[简体中文](README.zh_CN.md)

mcpp (2026.11.22.1 or later) builds `cmd.exe` and `COMMAND.COM` for the host, or for any
target below from one machine:

```sh
mcpp build --release --target <target>
mcpp run cmd.exe --release --target <target> -- /c ver
```

## Levels

CI (`.github/workflows/mcpp.yml`) runs `.github/mcpp/test.bat` on every target it can run, and
grades the output by level:

| Level | What the system offers | What the interpreter does there |
|---|---|---|
| L0 | a console | builtins, variables, `SET /A`, `IF`, `FOR`, `CALL :label` |
| L1 | L0 and files | batch files, `MD`, `COPY`, `REN`, `TYPE`, `DIR`, `DEL`, redirection |
| L2 | L1 and starting a program | external commands, `%COMSPEC%`, their exit status |

The table gives each target's floor (`.github/mcpp/floor.txt`): a run below it fails. The
level a run reaches is written in the run's summary, so a target whose system gains a
capability shows it without a change here. A row below L2 says why.

| System | Target | Level | Run in CI |
|---|---|---|---|
| Linux (glibc) | `x86_64-linux-gnu`, `aarch64-linux-gnu` | L2, a terminal | the runner |
| Linux (musl, static) | `x86_64-linux-musl`, `aarch64-linux-musl`, `i686-linux-musl` | L2 | the runner |
| Windows (MinGW) | `x86_64-windows-gnu`, `aarch64-windows-gnu` | L2 | the runner; a Windows arm64 runner |
| Windows (MSVC) | `x86_64-windows-msvc`, `i686-windows-msvc` | L2 | the runner (i686 by WOW64) |
| Windows (musl, POSIX) | `x86_64-windows-musl` | L1 | the runner ¹ |
| macOS | `aarch64-macos`, `aarch64-macos-musl`, `x86_64-macos-musl` | L2 | the runners (Apple silicon, Intel) |
| PureDarwin (experimental) | `x86_64-macos-musl` | L2 | QEMU ² |
| FreeBSD | `x86_64-freebsd`, `aarch64-freebsd` | L2 | a FreeBSD VM |
| FreeBSD (openkal-musl, static) | `x86_64-freebsd-musl`, `aarch64-freebsd-musl` | L2 | a FreeBSD VM |
| NetBSD, OpenBSD | `x86_64-netbsd`, `aarch64-netbsd`, `x86_64-openbsd`, `aarch64-openbsd` | L2 | the system's VM ³ |
| GNU/Hurd, Haiku (preview) | `x86_64-hurd`, `x86_64-haiku` | L2 | the system's VM ³ |
| System V R4 i386 (preview) | `i686-svr4-musl` | L2 | the System V kernel in QEMU ⁴ |
| Android | `x86_64-linux-android` / `aarch64-linux-android` | L2 / built | the emulator |
| openkal image | `x86_64-openkal`, `aarch64-openkal` / `i686-openkal` | L2 / L1 | okrun; the self-contained package in FreeBSD ⁷ |
| WebAssembly | `wasm32-emscripten` | L1 (node: a terminal) | node; Chrome ([the page](https://sunrisepeak.github.io/cmd/)) ⁵ |
| UEFI | `x86_64-uefi` | L1 | QEMU and OVMF ⁶ |
| MS-DOS | `i586-dos-djgpp`, `i686-dos-musl` (experimental) | L1 | DOSBox ⁶ |

¹ The POSIX build on Windows hands a program it starts its name with `/` for `\`, and Windows'
`cmd.exe` reads that as switches; the MinGW and MSVC rows start programs on Windows.
² xnu with no macOS above it, booted from its project's developer preview (not mirrored).
³ Rows of `mcpp.platforms`, built over the system's own base and run in an image of it.
⁴ AT&T UNIX System V/386 Release 4.0 Version 3.0 (xim's `svr4-disk-image`; technical
verification only, the system is proprietary).
⁵ WebAssembly has no processes. The page keeps its files for the session; it is built with
`mcpp pack cmd.exe --profile web --target wasm32-emscripten --pkg-format web`.
⁶ openkal-uefi and openkal-dos offer no program start yet; DJGPP has neither `fork` nor
`posix_spawn`. Under openkal DOS a name is kept in upper case, so its output is compared
without regard to case.
⁷ An i386 image starts a program, but okrun for i386 does not yet enter an image an image
starts (mcpplibs/openkal-image#46).

## Running a target in a machine

A target that runs in a machine or an emulator mcpp installs on first use (the VMs, QEMU,
DOSBox) runs once its runner is enabled: `[target.x86_64-uefi] runner = { enabled = true }`
in `mcpp.toml` or this machine's `config.toml`, or for one run:

```sh
MCPP_TARGET_X86_64_NETBSD_RUNNER_ENABLED=true \
  mcpp run cmd.exe --release --target x86_64-netbsd --file test.bat -- /c test.bat
```

## Where the targets come from

The interpreter uses POSIX interfaces (and Win32 on Windows) and states no target. The rows
of the openkal platform are named once in `mcpp.toml`, as the `openkal-platform` 0.31 series;
the NetBSD, OpenBSD, GNU/Hurd, Haiku and DJGPP rows take the platform the package index names.
A row the ecosystem adds later (mcpp-community/mcpp#1005: `riscv64-linux-musl`, `wasm32-wasi`,
the iOS simulator, an openkal row on bare metal) is one line in CI, and a capability a platform
gains (a program start under UEFI or DOS) raises its level by itself.
