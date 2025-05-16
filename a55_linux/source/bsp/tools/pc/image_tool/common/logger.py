from __future__ import print_function
import traceback
import sys

class LOG_LEVEL:
    DEBUG = 1
    INFO  = 2
    WARN  = 3
    ERROR = 4

# defualt
CURR_LEVEL = LOG_LEVEL.DEBUG

def print_to_stderr(msg):
    print(msg, file=sys.stderr)

def print_to_stdout(msg):
    print(msg, file=sys.stdout)

def error(err_msg):
    if CURR_LEVEL > LOG_LEVEL.ERROR:
        return
    print_to_stderr('ERROR: %s' % err_msg)
    traceback.print_stack()

def warn(warn_msg):
    if CURR_LEVEL > LOG_LEVEL.WARN:
        return
    print_to_stderr('WARNING: %s' % warn_msg)

def info(info_msg):
    if CURR_LEVEL > LOG_LEVEL.INFO:
        return
    print_to_stdout('INFO : %s' % info_msg)

def debug(info_msg):
    if CURR_LEVEL > LOG_LEVEL.DEBUG:
        return
    print_to_stdout('DEBUG: %s' % info_msg)