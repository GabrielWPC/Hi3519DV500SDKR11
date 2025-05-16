import sys
import os

SCRIPT_DIR = os.path.dirname(__file__)
sys.path.append(SCRIPT_DIR)

import common.logger as log
import common.util as utl
import common.check as chk

class KdfTool:
    def __init__(self, otp):
        self.__obfu_mrk = otp.obfu_mrk1()
        self.__alg = otp.scs_alg_sel()
        self.__kdf_exec = './KDF_Linux_SecureBoot'
        self.__param_file = './parameter.bin'
        self.__hash_tee_flash_root_public_key = otp.tee_hash_flash_root_key()
        self.__tee_enable = otp.soc_tee_enable_val()

    def derive_keys(self):
        chk.check_file_exist(self.__kdf_exec, 'KDFTool')
        chk.check_file_exist(self.__param_file, 'param_file_path')

        tmp_kdf_ini = 'tmp_kdf.ini'
        tmp_kdf_out = 'tmp_kdf_out.bin'
        key_names = ['abrk0', 'abrk1', 'abrk2', 'sbrk0', 'sbrk1', 'sbrk2']
        key_file_names = ['tmp_kdf_out_' + key_name + '.bin' for key_name in key_names]

        ini_items = [
                'function=0',
                'param_file_path=./parameter.bin',
                'Tee_owner_sel=1',
                'output_file_path=%s' % tmp_kdf_out,
                'encryption=%s' % ('1' if self.__alg == 'SM2+SM3+SM4' else '0'),
                'Obfu_MRK1=%s' % (utl.hex2str(self.__obfu_mrk)).zfill(32),
                'Tee_enable={:2x}'.format(self.__tee_enable),
                'Hash_TEE_Flash_Root_Public_Key=%s' % utl.hex2str(self.__hash_tee_flash_root_public_key).zfill(64),
                ]

        ini_text = "\n".join(ini_items)
        utl.write_file(tmp_kdf_ini, ini_text)

        cmd = '%s %s' % (self.__kdf_exec, tmp_kdf_ini)
        stdout, _, _= utl.run_cmd(cmd)
        stdout = stdout.decode("utf-8")
        if stdout.find('Processed successfully !') == -1:
            log.error('KDF Failed!\nKDF: %s' % stdout)
            exit(1)

        keys = dict(zip(key_names,
                        map(lambda key_file: utl.read_file(key_file), key_file_names)))

        for file_name in key_file_names:
            utl.rm_file(file_name)
        utl.rm_file(tmp_kdf_ini)

        return keys
