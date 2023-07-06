proc boot {from to} {
    expect "sim> "
    send "boot $from\r"
    expect ": "
    send "$to\r"
}

proc halt {} {
    expect "# "
    send "halt\r"

    expect "sim> "
}

proc shell_cmd {cmd} {
    expect "# "
    send "$cmd\r"
    expect "# "
    send "echo \$?\r"
    expect "0"
}

proc write_file {path contents} {
    expect "# "
    send "cat >\"$path\" <<EOF\r"
    expect "> "
    send "$contents\r"
    send "EOF\r"
    expect "# "
    send "echo \$?\r"
    expect "0"
}
