# cmd

> cmd.exe, the command interpreter for Windows, one of the most widely used
> command-line shells in the world.
>
> But it is not available on Unix-like systems. Why not bring it to Unix to
> expand the market?
>
> -- ChenPi11

**A faithful reimplementation of the Windows `cmd.exe` command interpreter for Unix.**

Born on GNU/Linux, runs on **EVERY UNIX-LIKE SYSTEMS** and Windows.

**ONLY LIBC and POSIX API/Win32 API required.**

**[README: American English](README.md)** |
**[README: 简体中文](README.zh_CN.md)** |
**[README: 微软式中文](README.zh_MS.md)** |
**[README: 八股文](README.zh_WY.md)**

## Supported languages

- American English (en_US.UTF-8, default)
- Simplified Chinese (zh_CN.UTF-8)
- Microsoft translated Chinese (zh_MS.UTF-8)
- Classical Chinese (zh_WY.UTF-8)

By setting the `LANG`, `LC_MESSAGES`, or `LC_ALL` environment variable, you
can change the language of the `cmd` shell.

## Screenshots

`cmd` has been verified running on a wide range of operating systems —
even classic System V.

<table>
  <tr>
    <td align="center">
      <img src="ohaiku.png" alt="cmd running on Haiku" height="150" />
      <br/><sub><b>Haiku</b></sub>
    </td>
    <td align="center">
      <img src="oopenbsd.png" alt="cmd running on OpenBSD" height="150" />
      <br/><sub><b>OpenBSD</b></sub>
    </td>
    <td align="center">
      <img src="opuredarwin.png" alt="cmd running on PureDarwin" height="150" />
      <br/><sub><b>PureDarwin</b></sub>
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="onetbsd.png" alt="cmd running on NetBSD" height="150" />
      <br/><sub><b>NetBSD</b></sub>
    </td>
    <td align="center">
      <img src="ofreebsd.png" alt="cmd running on FreeBSD" height="150" />
      <br/><sub><b>FreeBSD</b></sub>
    </td>
    <td align="center">
      <img src="osystemv.png" alt="cmd running on System V" height="150" />
      <br/><sub><b>System V</b></sub>
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="ohurd.png" alt="cmd running on GNU/Hurd" height="150" />
      <br/><sub><b>GNU/Hurd</b></sub>
    </td>
    <td align="center">
      <img src="omacos.png" alt="cmd running on macOS" height="150" />
      <br/><sub><b>macOS</b></sub>
    </td>
    <td align="center">
      <img src="omsys2.png" alt="cmd running on MSYS2" height="150" />
      <br/><sub><b>MSYS2</b></sub>
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="owindows.png" alt="cmd running on Windows" height="150" />
      <br/><sub><b>Windows</b></sub>
    </td>
    <td align="center">
      <img src="olinux.png" alt="cmd running on Linux" height="150" />
      <br/><sub><b>Linux</b></sub>
    </td>
  </tr>
</table>

## Features

- **Batch scripting** — `CALL`, `GOTO`, `IF`, `FOR`, `SHIFT`,
  `%0`..`%9` expansion, `SETLOCAL`/`ENDLOCAL` scoping, and labels.
- **Full pipe & redirection support** — `|`, `<`, `>`, `>>`, `2>` with
  `cmd.exe` precedence rules.
- **40+ builtin commands** — `ASSOC`, `COPY`, `DIR`, `ECHO`, `FOR`, `IF`,
  `SET`, `START`, `TITLE`, `TYPE`, and more.
- **Windows-style environment semantics** — case-insensitive `%VAR%`
  expansion, `%CD%`, `%DATE%`, `%TIME%`, `%ERRORLEVEL%`, `%CMDCMDLINE%`,
  delayed expansion with `/v:on`.
- **Line editing** — bundled linenoise with Emacs bindings, persistent
  history (`~/.cmd_history`), and TAB file/directory completion.
- **AutoRun support** — site-wide/user init scripts from
  `$PREFIX/etc/cmd/AutoRun/`, plus `AUTOEXEC.BAT` for `COMMAND.COM`.
- **Localised UI** — English and Simplified Chinese, selected from the
  `LC_ALL` / `LC_MESSAGES` / `LANG` environment.
- **Two personalities** — `cmd.exe` for the standard interpreter and
  `COMMAND.COM` with classic MS-DOS AutoExec behaviour.
- **No external dependencies** — C89, a POSIX.1 libc, and nothing else.

## Building

### GNU/Linux (GNU Make)

```sh
make
make install PREFIX=/usr/local
```

### Generic Unix (including System V)

```sh
sh tbuild.sh                 # uses $CC, defaults to cc.
sh tbuild.sh CC=cc V=0       # quiet build with a specific compiler.
env SYSV=0 ./tbuild.sh       # Disable System V portability flags.
```

The source is STD C89 with no dependencies beyond a POSIX.1-1990 libc. On
`tbuild.sh` adds the portability flags (`-DLIBCMD_SYSV=1` and
`-DLIBCMD_NO_VSNPRINTF=1`) — see `lsysport.c`.
Use SYSV=0 can disable `-DLIBCMD_SYSV=1`.

### Supported Build Tools

#### GNU Make

```sh
make -j$(nproc)
# Or `gmake -j$(nproc)` on systems where GNU Make is invoked as `gmake`.
```

#### CMake

```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

#### MSBuild

```sh
msbuild cmd.sln /p:Configuration=Release
```

#### Gradle

```sh
./gradlew build
# Or even ./cmd.exe /c "gradlew.bat build", but need replace
# `bin\java.exe` to `bin\java`, and set `JAVA_HOME` (usually to /usr)
```

#### mcpp

mcpp builds for the host, or cross-builds for any target listed below.

```sh
mcpp build --release
```

An openkal image (`.oke`) runs on every system that has `okrun`:

```sh
mcpp build --release --target x86_64-openkal
mcpp run --target x86_64-openkal -- /c ver
```

In a browser, the web profile and mcpp's `dist-web` plugin make a page
([try it](https://sunrisepeak.github.io/cmd/)):

```sh
mcpp pack cmd.exe --profile web --target wasm32-emscripten --pkg-format web
```

<details>
<summary>More targets: Linux, Windows, macOS, FreeBSD, NetBSD, OpenBSD, Android, WebAssembly, openkal images, UEFI, DOS, System V, GNU/Hurd and Haiku</summary>

**runs**: a test batch script gives the same output as on Linux x86_64 (checked by `.github/workflows/mcpp.yml`); **starts**: the interpreter starts and exits; **builds**: not run. The script's part that starts a program (the interpreter itself, through `%COMSPEC%`) is left out where no program can be started: the browser and node (WebAssembly), an openkal image under okrun (it starts a program but not an image, mcpplibs/openkal-image#42), UEFI, both DOS rows, System V until its row claims `openkal.process` (openkal-svr4 0.3.0), `x86_64-windows-musl`, whose interpreter does not yet find a program named by a path with a drive letter, and PureDarwin, where openkal-macos reports no execute bit (mcpplibs/openkal-macos#53).

<table>
<tr><th>Target</th><th>Verified</th><th>Build command</th></tr>
<tr><td>Linux x86_64 (glibc)<br><code>x86_64-linux-gnu</code></td><td>runs</td><td>

```sh
mcpp build --release --target x86_64-linux-gnu
```

</td></tr>
<tr><td>Linux x86_64 (musl, static)<br><code>x86_64-linux-musl</code></td><td>runs</td><td>

```sh
mcpp build --release --target x86_64-linux-musl
```

</td></tr>
<tr><td>Linux aarch64 (glibc)<br><code>aarch64-linux-gnu</code></td><td>runs</td><td>

```sh
mcpp build --release --target aarch64-linux-gnu
```

</td></tr>
<tr><td>Linux aarch64 (musl, static)<br><code>aarch64-linux-musl</code></td><td>runs</td><td>

```sh
mcpp build --release --target aarch64-linux-musl
```

</td></tr>
<tr><td>Linux i686 (musl, static)<br><code>i686-linux-musl</code></td><td>runs</td><td>

```sh
mcpp build --release --target i686-linux-musl
```

</td></tr>
<tr><td>Windows x86_64 (MinGW)<br><code>x86_64-windows-gnu</code></td><td>runs</td><td>

```sh
mcpp build --release --target x86_64-windows-gnu
```

</td></tr>
<tr><td>Windows x86_64 (musl)<br><code>x86_64-windows-musl</code></td><td>runs</td><td>

```sh
mcpp build --release --target x86_64-windows-musl
```

</td></tr>
<tr><td>Windows aarch64 (MinGW)<br><code>aarch64-windows-gnu</code></td><td>runs</td><td>

```sh
mcpp build --release --target aarch64-windows-gnu
```

</td></tr>
<tr><td>macOS x86_64, and PureDarwin<br><code>x86_64-macos-musl</code></td><td>runs ¹</td><td>

```sh
mcpp build --release --target x86_64-macos-musl
```

</td></tr>
<tr><td>macOS aarch64<br><code>aarch64-macos-musl</code></td><td>runs</td><td>

```sh
mcpp build --release --target aarch64-macos-musl
```

</td></tr>
<tr><td>macOS aarch64 (Apple SDK, on a Mac)<br><code>aarch64-macos</code></td><td>runs</td><td>

```sh
mcpp build --release --target aarch64-macos
```

</td></tr>
<tr><td>FreeBSD x86_64<br><code>x86_64-freebsd</code></td><td>runs</td><td>

```sh
mcpp build --release --target x86_64-freebsd
```

</td></tr>
<tr><td>FreeBSD aarch64<br><code>aarch64-freebsd</code></td><td>runs</td><td>

```sh
mcpp build --release --target aarch64-freebsd
```

</td></tr>
<tr><td>NetBSD x86_64<br><code>x86_64-netbsd</code></td><td>runs</td><td>

```sh
mcpp build --release --target x86_64-netbsd
```

</td></tr>
<tr><td>NetBSD aarch64<br><code>aarch64-netbsd</code></td><td>builds</td><td>

```sh
mcpp build --release --target aarch64-netbsd
```

</td></tr>
<tr><td>OpenBSD x86_64<br><code>x86_64-openbsd</code></td><td>runs</td><td>

```sh
mcpp build --release --target x86_64-openbsd
```

</td></tr>
<tr><td>OpenBSD aarch64<br><code>aarch64-openbsd</code></td><td>builds</td><td>

```sh
mcpp build --release --target aarch64-openbsd
```

</td></tr>
<tr><td>Android x86_64<br><code>x86_64-linux-android</code></td><td>runs</td><td>

```sh
mcpp build --release --target x86_64-linux-android
```

</td></tr>
<tr><td>Android aarch64<br><code>aarch64-linux-android</code></td><td>builds</td><td>

```sh
mcpp build --release --target aarch64-linux-android
```

</td></tr>
<tr><td>WebAssembly (Node.js)<br><code>wasm32-emscripten</code></td><td>runs</td><td>

```sh
mcpp build --release --target wasm32-emscripten
```

</td></tr>
<tr><td>openkal image x86_64<br><code>x86_64-openkal</code></td><td>runs</td><td>

```sh
mcpp build --release --target x86_64-openkal
```

</td></tr>
<tr><td>openkal image aarch64<br><code>aarch64-openkal</code></td><td>runs</td><td>

```sh
mcpp build --release --target aarch64-openkal
```

</td></tr>
<tr><td>openkal image i386<br><code>i686-openkal</code></td><td>runs</td><td>

```sh
mcpp build --release --target i686-openkal
```

</td></tr>
<tr><td>UEFI x86_64<br><code>x86_64-uefi</code></td><td>runs</td><td>

```sh
mcpp build --release --target x86_64-uefi
```

</td></tr>
<tr><td>MS-DOS (DJGPP)<br><code>i586-dos-djgpp</code></td><td>runs</td><td>

```sh
mcpp build --release --target i586-dos-djgpp
```

</td></tr>
<tr><td>MS-DOS (openkal, experimental)<br><code>i686-dos-musl</code></td><td>runs ²</td><td>

```sh
mcpp build --release --target i686-dos-musl
```

</td></tr>
<tr><td>System V Release 4 i386 (openkal, preview)<br><code>i686-svr4-musl</code></td><td>runs ³</td><td>

```sh
mcpp build --release --target i686-svr4-musl
```

</td></tr>
<tr><td>GNU/Hurd x86_64 (preview)<br><code>x86_64-hurd</code></td><td>runs ⁴</td><td>

```sh
mcpp build --release --target x86_64-hurd
```

</td></tr>
<tr><td>Haiku x86_64 (preview)<br><code>x86_64-haiku</code></td><td>runs ⁴</td><td>

```sh
mcpp build --release --target x86_64-haiku
```

</td></tr>
</table>

¹ One Mach-O executable that links nothing but the system library. It is run on an Intel Mac,
and on Linux x86_64 in PureDarwin (xnu with no macOS above it), booted from the developer
preview its project publishes: an experiment, whose image is not mirrored.

² DOS without long names keeps a file's name in upper case (`F1.TXT` for `f1.txt`): there the
script's output is compared without regard to case.

³ A static ELF32 executable that links nothing of a system and enters the kernel through the
call gate. It is run on a System V kernel: AT&T UNIX System V/386 Release 4.0 Version 3.0 in QEMU
(xim's `svr4-disk-image`, technical verification only: the system is proprietary), booted by dirun
as this project's runner for the row. The platform's own runner is an emulation of the interface
(ibcs-us). The program starts no external command there (openkal-svr4 claims no
`openkal.process`).

⁴ A row of `mcpp.platforms` (mcpp-community/mcpp-platforms): built on Linux with the LLVM
toolchain over the system's own base (Debian GNU/Hurd's glibc, at the versions of the image it
runs in; Haiku's libroot), a sysroot package of the package index, and run in an image of the
system (Debian's; a Haiku nightly) booted by dirun.

The NetBSD, OpenBSD, GNU/Hurd and Haiku rows, and the openkal rows beyond mcpp's own, come
from the platform the package index names for them. The FreeBSD, NetBSD, OpenBSD, Android,
UEFI, DOS, System V, GNU/Hurd and Haiku rows, and the Darwin row on Linux, run
in a machine or an emulator that mcpp installs on first use, so their runner is enabled on
request: `[target.x86_64-uefi] runner = { enabled = true }` in `mcpp.toml` or this machine's
`config.toml`, or `MCPP_TARGET_X86_64_UEFI_RUNNER_ENABLED=true` for one `mcpp run`. A file the
program reads is given with the run:

```sh
MCPP_TARGET_X86_64_NETBSD_RUNNER_ENABLED=true \
  mcpp run cmd.exe --release --target x86_64-netbsd --file test.bat -- /c test.bat
```

</details>

#### Meson

```sh
meson setup build --buildtype=release
meson compile -C build
```

#### XMake

```sh
xmake build
```

#### Our build.sh

```sh
sh tbuild.sh
```

### Supported IDEs

- Visual Studio: `cmd.sln`, `COMMAND.sln`
- Dev-Cpp: `cmd.dev`, `COMMAND.dev`
- Code::Blocks: `cmd.cbp`, `COMMAND.cbp`

## Usage

```text
cmd [/c|/k] [/s] [/q] [/d] [/a|/u] [/t:{bf|f}]
    [/e:{on|off}] [/f:{on|off}] [/v:{on|off}] [string]
```

| Option | Description |
| :--- | :--- |
| `/c string` | Execute `string` and exit |
| `/k string` | Execute `string` and remain interactive |
| `/s` | Special parsing mode for `/c`/`/k` (strip outer quotes) |
| `/q` | Quiet mode; no banner or prompt |
| `/d` | Disable AutoRun scripts |
| `/a` | ANSI output (default) |
| `/u` | Unicode output |
| `/t:{bf\|f}` | Set foreground/background colour nibbles, e.g. `/t:0f` |
| `/e:{on\|off}` | Enable/disable command extensions |
| `/f:{on\|off}` | Enable/disable file-name completion |
| `/v:{on\|off}` | Enable/disable delayed expansion |

## Builtin Commands

```text
ASSOC        BREAK        CALL         CD / CHDIR   CLS
COLOR        COPY         DATE         DEL / ERASE  DIR
DOSKEY       ECHO         ENDLOCAL     EXIT         FOR
FTYPE        GOTO         IF           MD / MKDIR   MKLINK
MOVE         PATH         PAUSE        POPD         PROMPT
PUSHD        RD / RMDIR   REM          REN / RENAME SET
SETLOCAL     SHIFT        START        TIME         TITLE
TYPE         VER          VERIFY       VOL
```

## Batch Files

Batch files (`.bat` and `.cmd`) are fully supported, including `CALL`,
`GOTO`, `IF`, `FOR`, `SHIFT`, `%0`..`%9` argument expansion, and
`SETLOCAL`/`ENDLOCAL` scoping. Labels use the colon prefix:

```bat
@ECHO OFF
:again
ECHO Hello, %1
SHIFT
IF NOT "%~1"=="" GOTO again
```

## Pipes and Redirection

```text
|          Pipe stdout of the left command to stdin of the right
< file     Redirect stdin from file
> file     Redirect stdout to file (overwrite)
>> file    Redirect stdout to file (append)
2> file    Redirect stderr to file
```

Multiple redirections can be combined on a single command line.

## Files

| Path | Purpose |
| :--- | :--- |
| `~/.cmd_history` | Command history (plain text, one command per line) |
| `~/.cmd_assoc` | File extension associations (`ASSOC` / `FTYPE`) |
| `$PREFIX/etc/cmd/AutoRun/` | Scripts executed on startup |
| `AUTOEXEC.BAT` | Startup script for `COMMAND.COM` |

## Portability

`cmd` avoids GNU extensions, designated initialisers, and C99 types where
possible. Systems without `fnmatch(3)`, `settimeofday(2)`,
`setpriority(2)`, or `open_memstream(3)` are covered by the portability
layer in `lsysport.c`.

## License

GNU General Public License **version 3 or later** — see [LICENSE](LICENSE).

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for build, coding, and portability
rules, and the [Code of Conduct](CODE_OF_CONDUCT.md) for community standards.
