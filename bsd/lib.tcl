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

proc shell_cmd {cmd {check_error true}} {
    expect "# "
    send "$cmd\r"
    if {$check_error == true} {
	expect "# "
	send "echo \$?\r"
	expect "0\r"
    }
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

proc maybe_backup_file {path} {
    shell_cmd "\[ -e  \"$path\" -a \! -e \"$path.orig\" \] && cp -p \"$path\" \"$path.orig\" || :;"
}

proc write_file {path contents user group mode} {
    maybe_backup_file $path
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

proc append_line {path line user group mode} {
    maybe_backup_file $path
    shell_cmd "touch \"$path\""
    shell_cmd("echo \'$line\' >> \"$path\"")
    fix_file $path $user $group $mode
}

proc copy_local_file {local_path remote_path user group mode} {
    maybe_backup_file $remote_path
    set uuencoded [exec /usr/bin/uuencode $local_path /dev/stdout]
    expect "# "
    send "uudecode > \"${remote_path}\"\r"
    send "$uuencoded\r"

    fix_file $remote_path $user $group $mode
}

proc boot_to_multiuser {} {
    boot "rq0" ""

    expect "# "
    send "\004"

    expect "login: "
    send "root\r"
}


proc set_clock {{check_error true}} {
    set yymmddhhmm [clock format [clock seconds] -format "%y%m%d%H%M" -gmt true]
    shell_cmd "date -n -u $yymmddhhmm" $check_error
}

proc set_hostname {} {
    upvar hostname my_hostname
    shell_cmd "hostname $my_hostname"
}
