"""prompt.py PROGRAM...: types three lines into PROGRAM on a terminal and checks
that the interpreter runs each one, as a person at the prompt would."""
import os, pty, select, sys, time

pid, fd = pty.fork()
if pid == 0:
    os.execvp(sys.argv[1], sys.argv[1:])

def read(seconds):
    data, end = b"", time.time() + seconds
    while time.time() < end:
        if select.select([fd], [], [], 0.1)[0]:
            try:
                data += os.read(fd, 65536)
            except OSError:
                break
    return data

seen = read(5)
for line in (b"echo first\r", b"echo second\r", b"exit\r"):
    os.write(fd, line)
    seen += read(3)
text = seen.decode(errors="replace").replace("\r", "")
lines = [l.strip() for l in text.split("\n")]
ok = "first" in lines and "second" in lines
print(text)
print("prompt: the interpreter ran each line" if ok else "::error::prompt: a typed line did not run")
sys.exit(0 if ok else 1)
