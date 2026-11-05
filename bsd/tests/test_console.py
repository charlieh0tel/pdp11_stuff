import binascii
import re
from collections.abc import Iterator
from unittest.mock import Mock

import pexpect
import pytest

from bsdbuild.console import (
    STATUS_COMMAND,
    STATUS_PATTERN,
    UU_LINE_BYTES,
    Console,
    ConsoleError,
    uuencode,
)

FAST_TIMEOUT_SECONDS = 0.5
TEST_TIMEOUT_SECONDS = 10.0


@pytest.fixture
def console() -> Iterator[Console]:
    """A Console attached to a host shell that prompts like the guest."""
    child = pexpect.spawn(
        "/bin/sh",
        ["-i"],
        encoding="ascii",
        codec_errors="backslashreplace",
        env={"PS1": "# ", "PATH": "/usr/bin:/bin"},
    )
    test_console = Console(child)
    test_console.expect("# ", TEST_TIMEOUT_SECONDS)
    yield test_console
    child.close(force=True)


def _decode(lines: list[str]) -> bytes:
    return b"".join(binascii.a2b_uu(line + "\n") for line in lines[1:-2])


@pytest.mark.parametrize(
    "data",
    [
        b"",
        b"x",
        b"\x00" * 3,
        b"a" * (UU_LINE_BYTES - 1),
        b"a" * UU_LINE_BYTES,
        b"a" * (UU_LINE_BYTES + 1),
        bytes(range(256)),
    ],
)
def test_uuencode_round_trips(data: bytes) -> None:
    assert _decode(uuencode(data, "/etc/f")) == data


def test_uuencode_frame() -> None:
    lines = uuencode(b"x", "/etc/f")
    assert lines[0] == "begin 644 /etc/f"
    assert lines[-2:] == ["`", "end"]


def test_uuencode_never_ends_a_line_with_space() -> None:
    assert not any(line.endswith(" ") for line in uuencode(b"\x00" * 90, "f"))


@pytest.mark.parametrize(("sync", "command"), [(True, "halt\r"), (False, "halt -n\r")])
def test_halt_syncs_only_when_asked(sync: bool, command: str) -> None:
    child = Mock()
    Console(child).halt(sync=sync)
    child.send.assert_called_once_with(command)


def test_status_pattern_ignores_the_typed_command() -> None:
    assert STATUS_PATTERN.search(STATUS_COMMAND) is None


def test_shell_returns_zero(console: Console) -> None:
    assert console.shell("true", timeout=TEST_TIMEOUT_SECONDS) == 0


def test_shell_raises_on_failure(console: Console) -> None:
    with pytest.raises(ConsoleError, match="status 1"):
        console.shell("false", timeout=TEST_TIMEOUT_SECONDS)


def test_status_ten_is_not_zero(console: Console) -> None:
    with pytest.raises(ConsoleError, match="status 10"):
        console.shell("sh -c 'exit 10'", timeout=TEST_TIMEOUT_SECONDS)


def test_shell_can_ignore_failure(console: Console) -> None:
    assert console.shell("false", check=False, timeout=TEST_TIMEOUT_SECONDS) == 1


def test_shell_times_out_instead_of_hanging(console: Console) -> None:
    with pytest.raises(ConsoleError, match=re.escape("sleep 5")):
        console.shell("sleep 5", timeout=FAST_TIMEOUT_SECONDS)


def test_shell_answers_prompts(console: Console) -> None:
    command = "printf 'NAME: '; read answer; test \"$answer\" = reply"
    status = console.shell(
        command, replies=[("NAME: ", "reply")], timeout=TEST_TIMEOUT_SECONDS
    )
    assert status == 0
