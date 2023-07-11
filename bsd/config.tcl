set label "MURGATROID"
set hostname "pdp1153.murgatroid.com"

# RA92
set disk_image "2.11BSD_patch457-root.ra92"
set ncyl 3099
set sec_per_cyl 949;		# 73 * 13
set disk_size_bytes [expr $ncyl * $sec_per_cyl * 512]

set root_disk_type "ra"
set root_disk_no 0
set root_disk "$root_disk_type$root_disk_no"

set root_part "a"
set root_dev "$root_disk$root_part"
set root_start 0
set root_ncyl 12
set root_size [expr $root_ncyl * $sec_per_cyl]

set swap_part "b"
set swap_dev "$root_disk$swap_part"
set swap_start $root_size
set swap_ncyl 25
set swap_size [expr $swap_ncyl * $sec_per_cyl]

set usr_part "d"
set usr_dev "$root_disk$usr_part"
set usr_start [expr $root_size + $swap_size]
set usr_ncyl [expr $ncyl - $root_ncyl - $swap_ncyl]
set usr_size [expr $usr_ncyl * $sec_per_cyl]

set whole_part "c"
set whole_dev "$root_disk$whole_part"
set whole_start 0
set whole_size [expr $ncyl * $sec_per_cyl]
