from bsdbuild.customize import USER_HOME, USER_NAME, user_entry

MASTER_PASSWD_FIELDS = 10


def test_user_entry_is_in_master_passwd_format() -> None:
    fields = user_entry("HASH").split(":")
    assert len(fields) == MASTER_PASSWD_FIELDS
    assert fields[0] == USER_NAME
    assert fields[1] == "HASH"
    assert fields[8] == USER_HOME


def test_user_name_can_salt_crypt() -> None:
    assert len(USER_NAME) == 2
