import os

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/..' % SCRIPT_DIR)

os.chdir(WORK_DIR)

import common.logger as log
import common.util as utl
import common.check as chk

def err_log(msg):
    log.error(msg)
    
def exist(file_name):
    return os.path.isfile(file_name) 

def gen_ecc_priv_key(priv_key_file):
    cmd = ['openssl', 'ecparam', '-genkey', '-name', 'brainpoolP256r1', '-out', priv_key_file]
    stdout, stderr, return_code = utl.run_cmd(cmd)

    if return_code != 0:
        err_log('Failed to generate ECC public key: %s' % stderr)

    return stdout

def gen_ecc_pub_key(priv_key_file, pub_key_file):
    chk.check_file_exist(priv_key_file, 'ECC Private Key')

    cmd = ['openssl', 'ec', '-pubout', '-in', priv_key_file,
           '-out', pub_key_file]
    stdout, stderr, return_code = utl.run_cmd(cmd)

    if return_code != 0:
        err_log('Failed to generate ECC public key %s' % stderr)

    return stdout

from contextlib import contextmanager
import sys, os

@contextmanager
def suppress_stdout():
    with open(os.devnull, "w") as devnull:
        old_stdout = sys.stdout
        sys.stdout = devnull
        try:  
            yield old_stdout
        finally:
            sys.stdout = old_stdout

def wait_user_input_or_exit_with_error_log(wait_user_input, error_log):
    with suppress_stdout() as out:
        if wait_user_input(out):
            return True
        err_log(error_log)
        exit(1)

def wait_user_input_for_gen_key_pair(out, priv_key_file, pub_key_file):
    out.write("can't find key pair {:s} or {:s}.\n\renter y to generate a key pair, otherwise, exit.\n".format(priv_key_file, pub_key_file))
    if sys.stdin.readline().strip() == "y":
        return True
    return False

def gen_ecc_key_pair(priv_key_file, pub_key_file):
    error_log = "can't find {:s} or {:s}, error exit!!!!!!\n".format(priv_key_file, pub_key_file)
    wait_user_input_or_exit_with_error_log(
            lambda out: wait_user_input_for_gen_key_pair(out, priv_key_file, pub_key_file), error_log)
    gen_ecc_priv_key(priv_key_file)
    gen_ecc_pub_key(priv_key_file, pub_key_file)

def get_attr_from_config_file(attr, cfg):
    import json
    with open(cfg) as f:
        raw = f.read()
        json_obj = json.loads(raw)
        def find_attr(attr, obj):
            ans = list()
            if type(obj) is dict:
                for k, v in obj.items():
                    if k == attr:
                        return v
                    else:
                        ans.append(find_attr(attr, v))
            if type(obj) is list:
                for item in obj:
                    ans.append(find_attr(attr, item))
            for i in ans:
                if i is not None:
                    return i
            return None
        return find_attr(attr, json_obj)

class pair_of_public_and_private_keys:
    def __init__(self, name, pri_key, pub_key, config_file):
        self.__name = name
        self.__priv_key = pri_key
        self.__pub_key = pub_key
        self.__cfg = config_file

    def exist_or_gen(self):
        if not exist(self.__cfg):
            err_log("Config %s file not exist!!!!" % self.__cfg)
            return False
        pub_key_file_name = get_attr_from_config_file(self.__pub_key, self.__cfg)
        priv_key_file_name = get_attr_from_config_file(self.__priv_key, self.__cfg)
        if not exist(pub_key_file_name) or not exist(priv_key_file_name):
            gen_ecc_key_pair(priv_key_file_name, pub_key_file_name)
        return True

