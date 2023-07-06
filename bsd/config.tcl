set label "MURGATROID"
set hostname "pdp1153"

set ncyl 3000
set disk_size [expr $ncyl * 512]

set root_disk_type "ra"
set root_disk_no 0
set root_disk "$root_disk_type$root_disk_no"

set root_part "a"
set root_dev "$root_disk$root_part"
set root_start 0
set root_size 204800

set swap_part "b"
set swap_dev "$root_disk$swap_part"
set swap_start $root_size
set swap_size 32768

set whole_part "c"
set whole_dev "$root_disk$whole_part"
set whole_start 0
set whole_size $disk_size

set usr_part "g"
set usr_dev "$root_disk$usr_part"
set usr_start [expr $root_size + $swap_size]
set usr_size [expr $disk_size - $usr_start]
