"""Drives a simulated 2.11BSD machine through its console."""

from __future__ import annotations

import binascii
import contextlib
import re
import sys
import time
from collections.abc import Iterable, Iterator, Sequence
from pathlib import Path

import pexpect

SIMULATOR = "pdp11"
SIMH_CONFIG = Path(__file__).parent.parent / "simh.ini"
CUSTOM_DIR = Path(__file__).parent.parent / "custom"

SHELL_PROMPT = "# "
SIM_PROMPT = "sim> "
BOOT_PROMPT = ": "
LOGIN_PROMPT = "login: "
CONTROL_D = "\x04"
CONTROL_E = "\x05"

DEFAULT_TIMEOUT_SECONDS = 600.0
LONG_TIMEOUT_SECONDS = 3600.0

# The typed command shows "$?" here, so only the shell's answer has digits.
STATUS_COMMAND = "echo STATUS=$?=END"
STATUS_PATTERN = re.compile(r"STATUS=(\d+)=END")

UU_LINE_BYTES = 45
UU_FILE_MODE = "644"


class ConsoleError(RuntimeError):
    """The guest failed a command or stopped answering."""


def uuencode(data: bytes, remote_path: str) -> list[str]:
    """Returns the uuencoded lines that make uudecode write remote_path."""
    chunks = (
        data[start : start + UU_LINE_BYTES]
        for start in range(0, len(data), UU_LINE_BYTES)
    )
    body = (
        binascii.b2a_uu(chunk, backtick=True).decode("ascii").rstrip("\n")
        for chunk in chunks
    )
    return [f"begin {UU_FILE_MODE} {remote_path}", *body, "`", "end"]


class Console:
    """A running simulator, from its "sim>" prompt through guest shells."""

    def __init__(self, child: pexpect.spawn[str]) -> None:
        self._child = child

    def _expect(
        self, pattern: str | re.Pattern[str], description: str, timeout: float
    ) -> int:
        try:
            if isinstance(pattern, str):
                return self._child.expect_exact(pattern, timeout=timeout)
            return self._child.expect(pattern, timeout=timeout)
        except (pexpect.TIMEOUT, pexpect.EOF) as error:
            raise ConsoleError(
                f"gave up waiting for {description!r} after {timeout} seconds "
                f"({type(error).__name__}); console showed: "
                f"{self._child.before!r}"
            ) from error

    def expect(self, text: str, timeout: float = DEFAULT_TIMEOUT_SECONDS) -> None:
        self._expect(text, text, timeout)

    def send(self, text: str) -> None:
        self._child.send(text)

    def respond(
        self, prompt: str, reply: str, timeout: float = DEFAULT_TIMEOUT_SECONDS
    ) -> None:
        """Waits for prompt, then sends reply without adding a line ending."""
        self.expect(prompt, timeout)
        self.send(reply)

    def answer(
        self, prompt: str, reply: str, timeout: float = DEFAULT_TIMEOUT_SECONDS
    ) -> None:
        """Waits for prompt, then sends reply as a line."""
        self.respond(prompt, f"{reply}\r", timeout)

    def boot(self, device: str, program: str) -> None:
        self.answer(SIM_PROMPT, f"boot {device}")
        self.answer(BOOT_PROMPT, program)

    def boot_to_single_user(self, device: str, program: str = "") -> None:
        self.boot(device, program)
        self.expect(SHELL_PROMPT)

    def boot_to_multiuser(self) -> None:
        self.boot_to_single_user("rq0")
        self.send(CONTROL_D)
        self.answer(LOGIN_PROMPT, "root", LONG_TIMEOUT_SECONDS)
        self.expect(SHELL_PROMPT)

    def _wait_for_status(self, command: str, timeout: float) -> int:
        self._expect(STATUS_PATTERN, command, timeout)
        match = self._child.match
        if not isinstance(match, re.Match):
            raise ConsoleError(f"no exit status for {command!r}")
        status = int(match.group(1))
        self.expect(SHELL_PROMPT)
        return status

    def _raise_unless_zero(self, command: str, status: int) -> None:
        if status != 0:
            raise ConsoleError(
                f"{command!r} exited with status {status}; "
                f"console showed: {self._child.before!r}"
            )

    def shell(
        self,
        command: str,
        *,
        check: bool = True,
        replies: Sequence[tuple[str, str]] = (),
        timeout: float = DEFAULT_TIMEOUT_SECONDS,
    ) -> int:
        """Runs command at the guest shell and returns its exit status.

        Each entry of replies is a (prompt, line) answer the command needs.
        """
        self.send(f"{command}; {STATUS_COMMAND}\r")
        for prompt, reply in replies:
            self.answer(prompt, reply, timeout)
        status = self._wait_for_status(command, timeout)
        if check:
            self._raise_unless_zero(command, status)
        return status

    def _backup_once(self, path: str) -> None:
        self.shell(
            f'[ -e "{path}" -a ! -e "{path}.orig" ] && '
            f'cp -p "{path}" "{path}.orig" || :'
        )

    def _set_ownership(self, path: str, owner: str, group: str, mode: str) -> None:
        self.shell(f'chown {owner} "{path}"')
        self.shell(f'chgrp {group} "{path}"')
        self.shell(f'chmod {mode} "{path}"')

    def _send_lines(self, lines: Iterable[str], timeout: float) -> None:
        # Waiting for each echo paces the input and catches line editing
        # changing what was typed.
        for line in lines:
            self.send(f"{line}\r")
            self.expect(line, timeout)

    def write_file(
        self, path: str, data: bytes, owner: str, group: str, mode: str
    ) -> None:
        """Writes data to path on the guest and verifies its size."""
        self._backup_once(path)
        command = "uudecode"
        self.send(f"{command}; {STATUS_COMMAND}\r")
        self._send_lines(uuencode(data, path), DEFAULT_TIMEOUT_SECONDS)
        self._raise_unless_zero(
            command, self._wait_for_status(command, DEFAULT_TIMEOUT_SECONDS)
        )
        self.shell(f'test `wc -c < "{path}"` -eq {len(data)}')
        self._set_ownership(path, owner, group, mode)

    def copy_custom_file(
        self, name: str, path: str, owner: str, group: str, mode: str
    ) -> None:
        """Copies a file from the custom directory to path on the guest."""
        self.write_file(path, (CUSTOM_DIR / name).read_bytes(), owner, group, mode)

    def set_clock(self, *, check: bool = True) -> None:
        now = time.strftime("%y%m%d%H%M", time.gmtime())
        self.shell(f"date -n -u {now}", check=check)

    def halt(self, *, sync: bool = True) -> None:
        self.send("halt\r" if sync else "halt -n\r")
        self.expect(SIM_PROMPT)

    def interrupt(self) -> None:
        """Returns from the guest to the simulator's prompt."""
        self.send(CONTROL_E)
        self.expect(SIM_PROMPT)

    def quit(self) -> None:
        """Exits the simulator so it flushes and closes its disk."""
        self.send("quit\r")
        try:
            self._child.expect(pexpect.EOF, timeout=DEFAULT_TIMEOUT_SECONDS)
        except pexpect.TIMEOUT as error:
            raise ConsoleError("simulator did not exit") from error


@contextlib.contextmanager
def simulator(tape: Path, disk: Path) -> Iterator[Console]:
    """Starts SIMH with tape and disk attached, echoing its console to stdout."""
    child = pexpect.spawn(
        SIMULATOR,
        [str(SIMH_CONFIG), str(tape), str(disk)],
        encoding="ascii",
        codec_errors="backslashreplace",
    )
    child.logfile_read = sys.stdout
    try:
        yield Console(child)
    finally:
        child.close(force=True)
