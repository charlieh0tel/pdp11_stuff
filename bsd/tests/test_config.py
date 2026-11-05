from bsdbuild import config


def test_disk_matches_simh_ra92_capacity() -> None:
    assert config.DISK_SECTORS == 2940951


def test_partitions_are_contiguous_and_fill_the_disk() -> None:
    assert config.ROOT.start == 0
    assert config.SWAP.start == config.ROOT.size
    assert config.USR.start == config.ROOT.size + config.SWAP.size
    assert config.USR.start + config.USR.size == config.DISK_SECTORS
    assert config.WHOLE.size == config.DISK_SECTORS


def test_device_names() -> None:
    assert config.ROOT.device == "ra0a"
    assert config.USR.device == "ra0d"
