
APP_ID_AMP = 0
APP_ID_IMP = 1

AMPIMP_PKT_HEAD = 0xF09F91BF # b"\xF0\x9F\x91\xBF"
AMPIMP_PKT_TAIL = 0x0D0A #b"\x0D\x0A"
AMPIMP_STATUS = 0x242B #"$+@+" b"\x24\x2B\x40\x2B"

app_stat_keys = ['$+@+', 'status']
app_amp_keys = ['time', 'app_id', 'index', 'i_ua']
app_imp_keys = ['time', 'app_id', 'index', 'freq', 'mag', 'phase']
