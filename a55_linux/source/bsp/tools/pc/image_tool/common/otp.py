import os
import sys
import shutil

SCRIPT_DIR = os.path.dirname(__file__)
sys.path.append(SCRIPT_DIR)

import common.logger as log
import common.util as utl
import common.check as chk
import common.config as cfg
import binascii

class OemOtpBurnCmd:
    def __init__(self, scen, otp_dir):
        self._otp_cmd_dir = otp_dir
        self._scen = scen

    def merge_all_sub_otp_cmd(self):
        filename = os.listdir(self._otp_cmd_dir)
        cmm_tmp = ''
        txt_tmp = ''
        for file in filename:
            if file.split('.')[1] == 'cmm':
                cmm_tmp += utl.read_file(self._otp_cmd_dir + file, binary=False)
            elif file.split('.')[1] == 'txt':
                txt_tmp += utl.read_file(self._otp_cmd_dir + file, binary=False)

        utl.write_file(self._otp_cmd_dir + 'all_merge_trance32_otp_cmd.cmm', cmm_tmp, 'merge all trance32 subcmd')
        utl.write_file(self._otp_cmd_dir + 'all_merge_uboot_otp_cmd.txt', txt_tmp, 'merge all uboot subcmd')
    #uboot otp burn cmd
    def otp_uboot_write_read_cmd(self, hash_val, start_addr, len_byte):
        text = ''
        text += ';OTP write cmd:\n\n'
        for i in range(len_byte):  # HASH is composed of 16 * 2 bytes
            text += '''mw 0x101e1038  0x1\n'''
            text += '''mw 0x101e1004  '''
            text += '0x%x\n' %(start_addr + i)
            val_str = utl.hex2str(hash_val[i:i+1])
            text += '''mw 0x101e1008  '''
            text += '0x%s\n' %(val_str)
            text += '''mw 0x101e1000  0x5\n'''
            text += '''\n'''
        text += '''mw 0x101e1038  0x3\n'''

        text += '\n\n'
        text += ';OTP read cmd:\n\n'
        if  len_byte < 4:
            len_byte = 1
        else:
            len_byte = int(len_byte/4)
        for i in range(len_byte):  # read unit is  8 * 4 bytes
            text += '''mw 0x101e1038  0x1\n'''
            text += '''mw 0x101e1004  '''
            text += '0x%x\n' %(start_addr + i * 4)
            text += '''mw 0x101e1000  0x3\n'''
            text += '''md 0x101e100C  0x1\n'''
            text += '''\n'''
        text += '''mw 0x101e1038  0x3\n'''
        return text
    #trance32 otp burn cmd
    def otp_trance32_write_read_cmd(self, hash_val, start_addr, len_byte):
        #text = 'OTP write cmd:\n\n'
        text = ''
        text += ';OTP write cmd:\n\n'
        for i in range(len_byte):  # HASH is composed of 16 * 2 bytes
            text += '''Data.Set EAXI:0x101e1038 %Long 0x1\n'''
            text += '''Data.Set EAXI:0x101e1004 %Long '''
            text += '0x%x\n' %(start_addr + i)
            val_str = utl.hex2str(hash_val[i:i+1])
            text += '''Data.Set EAXI:0x101e1008 %Long '''
            text += '0x%s\n' %(val_str)
            text += '''Data.Set EAXI:0x101e1000 %Long 0x5\n'''
            text += '''wait 10ms\n\n'''
        text += '''Data.Set EAXI:0x101e1038 %Long 0x3\n'''

        text += '\n\n'
        text += ';OTP read cmd:\n\n'
        if  len_byte < 4:
            len_byte = 1
        else:
            len_byte = int(len_byte/4)
        for i in range(len_byte):  # read unit is  8 * 4 bytes
            text += '''Data.Set EAXI:0x101e1038 %Long 0x1\n'''
            text += '''Data.Set EAXI:0x101e1004 %Long '''
            text += '0x%x\n' %(start_addr + i * 4)
            text += '''Data.Set EAXI:0x101e1000 %Long 0x3\n'''
            text += '''Data.In EAXI:0x101e100C /Long\n'''
            text += '''wait 10ms\n\n'''
        text += '''Data.Set EAXI:0x101e1038 %Long 0x3\n'''
        text += 'area'
        return text

    def build_otp_burn_cmd(self, data, addr, len, filename):
        filesion = filename.split(".")[1]
        log.info('build_otp_burn_cmd_file: ' + filename)
        if (filesion == 'txt'):
            cmd_text = self.otp_uboot_write_read_cmd(data, addr, len)
            utl.write_file(filename, cmd_text, filename)
        elif (filesion == 'cmm'):
            cmd_text = self.otp_trance32_write_read_cmd(data, addr, len)
            utl.write_file(filename, cmd_text, filename)

    def build_oem_tee_owner_burn_cmd(self, otp_cmd_dir):
        data = utl.str2hex("2")
        self.build_otp_burn_cmd(data, 0, 1, otp_cmd_dir + "oem_tee_owner_burn_write_cmd.cmm")
        self.build_otp_burn_cmd(data, 0, 1, otp_cmd_dir + "oem_tee_owner_burn_write_cmd.txt")

    def build_soc_tee_enable_burn_cmd(self, otp_cmd_dir, enable = True):
        data = utl.str2hex("FF") if (enable) else utl.str2hex("42")
        self.build_otp_burn_cmd(data, 0x12, 1, otp_cmd_dir + "soc_tee_enable_write_cmd.cmm")
        self.build_otp_burn_cmd(data, 0x12, 1, otp_cmd_dir + "soc_tee_enable_write_cmd.txt")

    def build_ree_hash_otp_burn_cmd(self, otp_cmd_dir, chksum_file):
        data =  utl.read_file(chksum_file, binary=False)
        data =  utl.str2hex(data)
        self.build_otp_burn_cmd(data, 0x60, 32, otp_cmd_dir + "ree_root_public_key_area_image_sha256_to_otp_wr_cmd.cmm")
        self.build_otp_burn_cmd(data, 0x60, 32, otp_cmd_dir + "ree_root_public_key_area_image_sha256_to_otp_wr_cmd.txt")

    def build_tee_hash_otp_burn_cmd(self, otp_cmd_dir, chksum_file):
        data =  utl.read_file(chksum_file, binary=False)
        data =  utl.str2hex(data)
        self.build_otp_burn_cmd(data, 0x40, 32, otp_cmd_dir + "tee_root_public_key_area_image_sha256_to_otp_wr_cmd.cmm")
        self.build_otp_burn_cmd(data, 0x40, 32, otp_cmd_dir + "tee_root_public_key_area_image_sha256_to_otp_wr_cmd.txt")

    def build_tp_hash_otp_burn_cmd(self, otp_cmd_dir, chksum_file):
        data =  utl.read_file(chksum_file, binary=False)
        data =  utl.str2hex(data)
        self.build_otp_burn_cmd(data, 0x80, 32, otp_cmd_dir + "tp_root_public_key_area_image_sha256_to_otp_wr_cmd.cmm")
        self.build_otp_burn_cmd(data, 0x80, 32, otp_cmd_dir + "tp_root_public_key_area_image_sha256_to_otp_wr_cmd.txt")

    def build_ree_verify_enable_otp_burn_cmd(self, otp_cmd_dir, enable = True):
        data = utl.str2hex("FF") if (enable) else utl.str2hex("42")
        if (enable):
            self.build_otp_burn_cmd(data, 0x16, 1, otp_cmd_dir + "ree_verify_enable_write_cmd.cmm")
            self.build_otp_burn_cmd(data, 0x16, 1, otp_cmd_dir + "ree_verify_enable_write_cmd.txt")
        else:
            self.build_otp_burn_cmd(data, 0x16, 1, otp_cmd_dir + "ree_verify_not_enable_write_cmd.cmm")
            self.build_otp_burn_cmd(data, 0x16, 1, otp_cmd_dir + "ree_verify_not_enable_write_cmd.txt")

    def build_tee_verify_enable_otp_burn_cmd(self, otp_cmd_dir, enable = True):
        data = utl.str2hex("FF") if (enable) else utl.str2hex("42")
        if (enable):
            self.build_otp_burn_cmd(data, 0x17, 1, otp_cmd_dir + "tee_verify_enable_write_cmd.cmm")
            self.build_otp_burn_cmd(data, 0x17, 1, otp_cmd_dir + "tee_verify_enable_write_cmd.txt")
        else:
            self.build_otp_burn_cmd(data, 0x17, 1, otp_cmd_dir + "tee_verify_not_enable_write_cmd.cmm")
            self.build_otp_burn_cmd(data, 0x17, 1, otp_cmd_dir + "tee_verify_not_enable_write_cmd.txt")

    def build_tp_verify_enable_otp_burn_cmd(self, otp_cmd_dir, enable = True):
        data = utl.str2hex("FF") if (enable) else utl.str2hex("42")
        if (enable):
            self.build_otp_burn_cmd(data, 0x15, 1, otp_cmd_dir + "tp_verify_enable_write_cmd.cmm")
            self.build_otp_burn_cmd(data, 0x15, 1, otp_cmd_dir + "tp_verify_enable_write_cmd.txt")
        else:
            self.build_otp_burn_cmd(data, 0x15, 1, otp_cmd_dir + "tp_verify_not_enable_write_cmd.cmm")
            self.build_otp_burn_cmd(data, 0x15, 1, otp_cmd_dir + "tp_verify_not_enable_write_cmd.txt")

    def build_sm_argorithm_burn_cmd(self, otp_cmd_dir):
        data = utl.str2hex("5")
        self.build_otp_burn_cmd(data, 0x0, 1, otp_cmd_dir + "sm2_sm3_sm4_argorithm_and_rpk_DEOB_sm4_write_cmd.cmm")
        self.build_otp_burn_cmd(data, 0x0, 1, otp_cmd_dir + "sm2_sm3_sm4_argorithm_and_rpk_DEOB_sm4_write_cmd.txt")

    # atuo build OTP cmd by oem config
    def build_otp_cmd(self, oem_ree_root_pub_key_area_chksum_file, oem_tee_root_pub_key_area_chksum_file):
        # atuo build OTP cmd by oem config
        if not os.path.exists(self._otp_cmd_dir):
            os.mkdir(self._otp_cmd_dir)
        else:
            shutil.rmtree(self._otp_cmd_dir)
            os.mkdir(self._otp_cmd_dir)
        if self._scen.is_sec_boot_enable():
            if self._scen.is_tee_enbale():
                self.build_ree_verify_enable_otp_burn_cmd(self._otp_cmd_dir, enable=True)
                self.build_tee_verify_enable_otp_burn_cmd(self._otp_cmd_dir, enable=True)
                self.build_soc_tee_enable_burn_cmd(self._otp_cmd_dir, enable=True)
                self.build_ree_hash_otp_burn_cmd(self._otp_cmd_dir, oem_ree_root_pub_key_area_chksum_file)
                self.build_tee_hash_otp_burn_cmd(self._otp_cmd_dir, oem_tee_root_pub_key_area_chksum_file)
            else:
                self.build_ree_verify_enable_otp_burn_cmd(self._otp_cmd_dir, enable=True)
                self.build_ree_hash_otp_burn_cmd(self._otp_cmd_dir, oem_ree_root_pub_key_area_chksum_file)
        else:
            self.build_soc_tee_enable_burn_cmd(self._otp_cmd_dir, enable=False)
            self.build_ree_verify_enable_otp_burn_cmd(self._otp_cmd_dir, enable=False)
            self.build_tee_verify_enable_otp_burn_cmd(self._otp_cmd_dir, enable=False)

        if self._scen.tee_owner_is_oem():
            self.build_oem_tee_owner_burn_cmd(self._otp_cmd_dir)
        if self._scen.is_crypto_alg_sm():
            self.build_sm_argorithm_burn_cmd(self._otp_cmd_dir)

        # merge all sub otp cmd
        self.merge_all_sub_otp_cmd()

    def build_otp_cmd_tp(self, tp_root_pub_key_area_chksum_file):
         # atuo build OTP cmd by oem config
        if not os.path.exists(self._otp_cmd_dir):
            os.makedirs(self._otp_cmd_dir)
        else:
            shutil.rmtree(self._otp_cmd_dir)
            os.makedirs(self._otp_cmd_dir)

        if self._scen.is_sec_boot_enable():
            self.build_tp_verify_enable_otp_burn_cmd(self._otp_cmd_dir, enable=True)
            self.build_tp_hash_otp_burn_cmd(self._otp_cmd_dir, tp_root_pub_key_area_chksum_file)
