import os
import oem_main

import public_and_private_keys

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/..' % SCRIPT_DIR)

key_list = [
        ["REE_root", "REE_root_private_key", "REE_root_pub_key"],
        ["GSL_external", "GSL_external_private_key", "GSL_external_public_key"],
        ["uboot_external", "uboot_external_private_key", "uboot_external_public_key"],
        ]

config_file = "./oem/quick_build_secure_boot_config.json"
keys_dir = "pub_private_key"

from public_and_private_keys import pair_of_public_and_private_keys

def find_or_gen_tee_key():
    for key_name, priv_key, pub_key in key_list:
        if pair_of_public_and_private_keys(key_name, 
                                           priv_key, 
                                           pub_key, 
                                           config_file).exist_or_gen() != True:
            return False

if __name__ == "__main__":
    os.chdir(WORK_DIR)
    if not os.path.exists(keys_dir):
        os.mkdir(keys_dir)
    find_or_gen_tee_key()
    oem_main.main(['', 'build', config_file])
