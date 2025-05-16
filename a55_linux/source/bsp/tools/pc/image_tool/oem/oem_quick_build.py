import os
import oem_main

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/..' % SCRIPT_DIR)

os.chdir(WORK_DIR)
oem_main.main(['', 'build', 'oem/quick_build_config.json'])
