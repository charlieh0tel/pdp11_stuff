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

proc fix_file {path user group mode} {
    shell_cmd "chown $user \"${path}\""
    shell_cmd "chgrp $group \"${path}\""
    shell_cmd "chmod $mode \"${path}\""
}

proc read_local_file {path} {
    set fd [open $path r]
    set contents [read $fd]
    close $fd
    return $contents
}

proc write_file {path contents user group mode} {
    shell_cmd "\[ -e  \"$path\" \] && cp -p \"$path\" \"$path.orig\""
    expect "# "
    send "cat >\"$path\" <<EOF\r"
    expect "> "
    send "$contents"
    send "EOF\r"
    expect "# "
    send "echo \$?\r"
    expect "0"
    fix_file $path $user $group $mode
}

proc copy_local_file {local_path remote_path user group mode} {
    set uuencoded [exec /usr/bin/uuencode $local_path /dev/stdout]
    expect "# "
    send "uudecode > \"${remote_path}\"\r"
    send "$uuencoded\r"
    fix_file $remote_path $user $group $mode
}

