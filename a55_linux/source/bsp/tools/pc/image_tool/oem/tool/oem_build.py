import os
import sys

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/../..' % SCRIPT_DIR)
sys.path.append(WORK_DIR)

import common.logger as log
import common.util as utl
import common.check as chk
import common.config as cfg
import binascii
from common.config import CfgValue
from common.area_tool import PublicKey, AreaTool
from common.area_tool import AreaCfg

class OemAreaBuilder:
    def __init__(self, cfgtor):
        self.inputdir = dir
        self.cfgtor = cfgtor    # Configuartor
        self.area_tool = AreaTool(self.cfgtor.scenario().crypto_alg())
        self.boot_area_file = None

        if self.cfgtor.scenario().is_sec_boot_enable():
            self.cfgtor.reset_path()
        if self.cfgtor.scenario().crypto_alg() == CfgValue.CryptoAlg.ECC:
            self._crypto_alg        = utl.str2le("0x2A13C812", group_size=4)
        elif self.cfgtor.scenario().crypto_alg() == CfgValue.CryptoAlg.SM:
            self._crypto_alg        = utl.str2le("0x2A13C823", group_size=4)

        self.cfgtor.reset_path('OEM_info')
        self.msid                   = self.cfgtor.get_value('oem_msid_ext',         utl.str2hex('0x0') * 4)
        self.mask_msid              = self.cfgtor.get_value('mask_oem_msid_ext',    utl.str2hex('0x0') * 4)
        self.structure_version      = utl.str2le("0x00010000", group_size=4)
        self.key_owner_id           = utl.str2le("0x0"       , group_size=4)
        self.key_id                 = utl.str2le("0x0"       , group_size=4)

    def build_ree_flash_root_pub_key_area(self, area_out_file, area_out_file_chksum_file):
        if self.cfgtor.scenario().is_sec_boot_enable():
            log.debug('------------- Build ree_flash_root_pub_key_area -------------')
            image_id            = utl.str2le("0x4BA5C31E", group_size=4)    # REE Flash Root Public Key
            structure_lenth     = utl.str2le("0x80"      , group_size=4)    # ECC256/SM2
            key_alg             = self._crypto_alg
            ecc_cure_type       = self._crypto_alg
            key_length          = utl.str2le("0x40"      , group_size=4)    # 64
            verify_enable_ext   = utl.str2le("0x0"      , group_size=4)    # 64
            reserved            = utl.str2hex('0x0') * 0x1c                  # len = 0x80-0x24-0x40 = 0x1c'

            self.cfgtor.reset_path('root_key')
            root_pub_key_pem_file = self.cfgtor.get_value('REE_root_pub_key')
            root_pub_key = self.area_tool.import_pub_key(root_pub_key_pem_file, "REE_root_pub_key")
            area = utl.combine_bytes(
                image_id,
                self.structure_version,
                structure_lenth,
                self.key_owner_id,
                self.key_id,
                key_alg,
                ecc_cure_type,
                key_length,
                verify_enable_ext,
                reserved,
                root_pub_key
            )
            chksum_hex = self.area_tool.digest(area)
            chksum_str = '0x' + utl.hex2str(chksum_hex)
            utl.write_file(area_out_file_chksum_file, chksum_str, 'ree_root_public_key_area_image Area Checksum')
            self.cfgtor.reset_path('SCS_simulate')
            verify_enable_ext = self.cfgtor.get_value('REE_verify_simulate')
            if verify_enable_ext == utl.str2le("0x2A13C812", group_size=4):
                verify_enable_ext_offset = 32
                area = utl.replace_bytes(
                    old_bytes=area,
                    pos=verify_enable_ext_offset,
                    new_bytes=verify_enable_ext
                )
        else:
            log.debug('------------- Build ree_flash_root_pub_key_area no-sec image -------------')
            area = self.area_tool.gen_empty_area(size=0x80, fill=0x00)

        utl.write_file(area_out_file, area, 'ree_root_public_key_area_image')
        log.info('Done.\n\n')

    def build_tee_flash_root_pub_key_area(self, area_out_file, area_out_file_chksum_file):

        if self.cfgtor.scenario().is_sec_boot_enable():
            if self.cfgtor.scenario().is_tee_enbale():
                log.debug('------------- Build tee_flash_root_pub_key_area -------------')
                image_id            = utl.str2le("0x4B96B41E", group_size=4)    # REE Flash Root Public Key
                structure_lenth     = utl.str2le("0x80"      , group_size=4)    # ECC256/SM2
                key_alg             = self._crypto_alg
                ecc_cure_type       = self._crypto_alg
                key_length          = utl.str2le("0x40"      , group_size=4)    # 64
                verify_enable_ext   = utl.str2le("0x0"      , group_size=4)    # 64
                reserved            = utl.str2hex('0x0') * 0x1c                  # len = 0x80-0x24-0x40 = 0x1c'
                self.cfgtor.reset_path('root_key')
                root_pub_key_pem_file = self.cfgtor.get_value('TEE_root_pub_key')
                root_pub_key = self.area_tool.import_pub_key(root_pub_key_pem_file, "TEE_root_pub_key")
                area = utl.combine_bytes(
                    image_id,
                    self.structure_version,
                    structure_lenth,
                    self.key_owner_id,
                    self.key_id,
                    key_alg,
                    ecc_cure_type,
                    key_length,
                    verify_enable_ext,
                    reserved,
                    root_pub_key
                )
                chksum_hex = self.area_tool.digest(area)
                chksum_str = '0x' + utl.hex2str(chksum_hex)
                utl.write_file(area_out_file_chksum_file, chksum_str, 'tee_root_public_key_area_image Area Checksum')
                self.cfgtor.reset_path('SCS_simulate')
                verify_enable_ext = self.cfgtor.get_value('TEE_verify_simulate')
                if verify_enable_ext == utl.str2le("0x2A13C812", group_size=4):
                    verify_enable_ext_offset = 32
                    area = utl.replace_bytes(
                        old_bytes=area,
                        pos=verify_enable_ext_offset,
                        new_bytes=verify_enable_ext
                    )
            else:
                log.debug('------------- Build tee_flash_root_pub_key_area no-tee image -------------')
                area = self.area_tool.gen_empty_area(size=0x80, fill=0x00)
        else:
            log.debug('------------- Build tee_flash_root_pub_key_area no-sec image -------------')
            area = self.area_tool.gen_empty_area(size=0x80, fill=0x00)

        utl.write_file(area_out_file, area, 'tee_root_public_key_area_image')
        log.info('Done.\n\n')

    def build_cfct_area(self, cfct_area_file):
        log.debug('------------- Build cfct_area_file -------------')

        if self.cfgtor.scenario().crypto_alg() == CfgValue.CryptoAlg.ECC:
            area = utl.read_file('input/cfct_table_ecc.bin')
            print("ecc:", self.cfgtor.scenario().crypto_alg())
        elif self.cfgtor.scenario().crypto_alg() == CfgValue.CryptoAlg.SM:
            area = utl.read_file('input/cfct_table_sm.bin')
            print("sm:", self.cfgtor.scenario().crypto_alg())
        else:
            log.info(" \033[1;35m err: CryptoAlg parameter err\033[0m")
            exit(1)
        if self.cfgtor.scenario().is_cfct_data_enc():
            cfct_key_table_area         = area

            CFCT_protection_key_l1       = self.cfgtor.get_value('CFCT_protection_key_l1', utl.str2hex('0x0') * 4)
            CFCT_protection_key_l2       = self.cfgtor.get_value('CFCT_protection_key_l2', utl.str2hex('0x0') * 4)
            CFCT_iv                     = self.cfgtor.get_value('CFCT_iv',               utl.str2hex('0x0') * 4)

            cfct_table_offset = 0x100
            cfct_data_len_offset = 128
            cfct_data_len_hex           = cfct_key_table_area[cfct_table_offset + cfct_data_len_offset : cfct_table_offset + cfct_data_len_offset + 4]
            cfct_data_len_int           = utl.le2int(cfct_data_len_hex, group_size=4)

            cfct_data_offset = 132
            cfct_data                   = cfct_key_table_area[cfct_table_offset + cfct_data_offset : cfct_table_offset + cfct_data_offset + cfct_data_len_int]

            Symm_RootKey_Type           = self.cfgtor.get_value('CFCT_data_enc_flag')
            effective_rk                 = self.cfgtor.get_value('CFCT_kdf_abrk0_file')
            CFCT_ProtectionKey_L1_dec   = self.area_tool.enc_ecb(CFCT_protection_key_l1, effective_rk, data_name='cfct L1 dec', decrypt=True)
            CFCT_ProtectionKey_L2_dec   = self.area_tool.enc_ecb(CFCT_protection_key_l2, CFCT_ProtectionKey_L1_dec, data_name='cfct L2 dec', decrypt=True)
            content_key                 = CFCT_ProtectionKey_L2_dec
            print("cfct_enc_content_key:", utl.hex2str(content_key))
            cfct_data_enc               = self.area_tool.enc_ctr(cfct_data, content_key, CFCT_iv, data_name='cfct data')

            # fill Symm_RootKey_Type to cfct table
            Symm_RootKey_Type_offset = cfct_table_offset + 16
            area = utl.replace_bytes(
                old_bytes=area,
                pos=Symm_RootKey_Type_offset,
                new_bytes=Symm_RootKey_Type
            )

            # fill L1/L2/IV to cfct table
            CFCT_ProtectionKey_L1_offset = cfct_table_offset + 80
            protect_key_area = utl.combine_bytes(
                CFCT_protection_key_l1,
                CFCT_protection_key_l2,
                CFCT_iv
            )
            area = utl.replace_bytes(
                old_bytes=area,
                pos=CFCT_ProtectionKey_L1_offset,
                new_bytes=protect_key_area
            )
            #fill cfct enc data area to cfct table
            cfct_data_enc_offset = cfct_table_offset + cfct_data_offset
            area = utl.replace_bytes(
                old_bytes=area,
                pos=cfct_data_enc_offset,
                new_bytes=cfct_data_enc
            )

        utl.write_file(cfct_area_file, area, 'cfct_area image')
        log.info('Done.\n\n')

    def build_gsl_key_area(self, gsl_key_area_file):
        if self.cfgtor.scenario().is_sec_boot_enable():
            log.debug('------------- Build gsl_key_area -------------')
            image_id                    = utl.str2le("0x4BB4D21E", group_size=4)    #
            structure_lenth             = utl.str2le("0x100"     , group_size=4)    # ECC256/SM2
            signature_length            = utl.str2le("0x40"      , group_size=4)    # ECC256/SM2 key lenth
            key_alg                     = self._crypto_alg
            ecc_cure_type               = self._crypto_alg
            key_length                  = utl.str2le("0x40"      , group_size=4)    # 64bytes

            self.cfgtor.reset_path('GSL', 'GSL_key_area')
            mrot_key_version_ext        = self.cfgtor.get_value('GSL_key_version_ext', utl.str2hex('0x0') * 4)
            mask_mrot_key_version_ext   = self.cfgtor.get_value('mask_GSL_key_version_ext', utl.str2hex('0x0') * 4)
            mrot_external_public_key_pem_file =  self.cfgtor.get_value('GSL_external_public_key')
            mrot_external_public_key = self.area_tool.import_pub_key(mrot_external_public_key_pem_file, "OME_GSL_External_Public_Key")

            self.cfgtor.reset_path('maintenance_mode')
            maintenance_mode            = self.cfgtor.get_value('GSL_key_maintenance_mode', utl.str2hex('0x0') * 4)
            if maintenance_mode == utl.str2le("0x3C7896E1"      , group_size=4):
                die_id                      = self.cfgtor.get_value('DIE_ID')
                if die_id == utl.str2hex("0x0") * 0x10:
                    log.info(" \033[1;35m warning: DIE_ID should not all zero\033[0m")
            else:
                die_id                      = utl.str2hex("0x0") * 0x10

            code_info_addr              = utl.str2le("0x0"       , group_size=4)
            reserved                    = utl.str2hex("0x0") * 0x34                # Structure_Length-76-Key_Length-Signature_Length = 0x100 - 76 - 0x40 *2 = 52 = 0x34

            area = utl.combine_bytes(
                image_id,
                self.structure_version,
                structure_lenth,
                signature_length,
                self.key_owner_id,
                self.key_id,
                key_alg,
                ecc_cure_type,
                key_length,
                mrot_key_version_ext,
                mask_mrot_key_version_ext,
                self.msid,
                self.mask_msid,
                maintenance_mode,
                die_id,
                code_info_addr,
                reserved,
                mrot_external_public_key
            )
            if self.cfgtor.scenario().is_tee_enbale():
                self.cfgtor.reset_path('root_key')
                tee_root_private_key_file =  self.cfgtor.get_value('TEE_root_private_key')
            else:
                self.cfgtor.reset_path('root_key')
                tee_root_private_key_file =  self.cfgtor.get_value('REE_root_private_key')

            sig_tee_boot_key_area = self.area_tool.sign(data = area, priv_key_file=tee_root_private_key_file, data_name="gsl key area sign")
            print("sig_tee_boot_key_area", len(sig_tee_boot_key_area))
            area = utl.combine_bytes(
                area,
                sig_tee_boot_key_area
            )
        else:
            log.debug('------------- Build gsl_key_area no-sec image-------------')
            area = self.area_tool.gen_empty_area(size=0x100, fill=0x00)
        utl.write_file(gsl_key_area_file, area, 'gsl key image')
        log.info('Done.\n\n')

    def get_fmc_data(self):
        fmc_cfg_data_len            = utl.str2le("0x12"      , group_size=4)
        fmc_clk_sel                 = utl.str2le("0x0"       , group_size=1)

        rd_delay                    = utl.str2le("0x0"       , group_size=1)
        rd_cmd                      = utl.str2le("0x3"       , group_size=1)
        oen_mult_en                 = utl.str2le("0x1"       , group_size=1)
        dummy_num                   = utl.str2le("0x1"       , group_size=1)
        reserved0                   = utl.str2le("0x0"       , group_size=1)
        fmc_drv_sfc_clk             = utl.str2le("0x1140"       , group_size=2)
        fmc_drv_sfc_cs0n            = utl.str2le("0x1300"       , group_size=2)
        fmc_drv_sfc_mosi_io_0       = utl.str2le("0x1130"       , group_size=2)
        fmc_drv_sfc_miso_io_1       = utl.str2le("0x1130"       , group_size=2)
        fmc_drv_sfc_wp_io_2         = utl.str2le("0x1160"       , group_size=2)
        fmc_drv_sfc_hold_io_3       = utl.str2le("0x1330"       , group_size=2)

        self.cfgtor.reset_path('GSL', 'GSL_Info_area')
        fmc_clk_flag                = self.cfgtor.get_value('GSL_code_FlashReadClk')

        if fmc_clk_flag != utl.str2le("0x0", group_size=1):
            fmc_clk_sel             = utl.str2le("0x2"       , group_size=1)
            rd_cmd                  = utl.str2le("0xb"       , group_size=1)

        area = utl.combine_bytes(
            fmc_cfg_data_len,
            fmc_clk_sel,
            rd_delay,
            rd_cmd,
            oen_mult_en,
            dummy_num,
            reserved0,
            fmc_drv_sfc_clk,
            fmc_drv_sfc_cs0n,
            fmc_drv_sfc_mosi_io_0,
            fmc_drv_sfc_miso_io_1,
            fmc_drv_sfc_wp_io_2,
            fmc_drv_sfc_hold_io_3
        )
        return area

    def build_gsl_code_info_area(self, gsl_code_and_info_area_file):
        log.info('------------- build_gsl_code_info -------------')
        image_id                    = utl.str2le("0x4BB4D22D" , group_size=4)    # TEE Flash Root Public Key
        structure_lenth             = utl.str2le("0x200"     , group_size=4)    # ECC256/SM2
        signature_length            = utl.str2le("0x40"      , group_size=4)    # ECC256/SM2 key lenth
        self.cfgtor.reset_path('GSL', 'GSL_Info_area')
        gsl_code_version_ext        = self.cfgtor.get_value('GSL_code_version_ext', utl.str2hex('00') * 4)
        mask_gsl_code_version_ext   = self.cfgtor.get_value('mask_GSL_code_version_ext', utl.str2hex('00') * 4)

        gsl_code_area_addr          = utl.str2le("0x0"       , group_size=4)    # 0 means followed GSL Code info.
        self.cfgtor.reset_path('GSL', 'GSL_code_area')
        gsl_code                    = utl.align(self.cfgtor.get_value('GSL_code_file'), n_align_bytes=0x200)
        gsl_code_area_len           = utl.int2le(int_data = len(gsl_code), group_size=4)
        if self.cfgtor.scenario().is_sec_boot_enable():
            gsl_code_area_hash          = self.area_tool.digest(data=gsl_code, data_name="gsl code hash calculate")
        else:
            gsl_code_area_hash          = utl.str2hex('00' * 0x20)

        self.cfgtor.reset_path('GSL', 'GSL_Info_area')
        gsl_enc_flag  = self.cfgtor.get_value('GSL_code_enc_flag', utl.str2hex('0x0') * 4)
        protectionkey_gsl_l1_enc    = utl.str2hex('00' * 0x10)     # ?  16byte
        protectionKey_gsl_l2_enc    = utl.str2hex('00' * 0x10)     # ?  16byte
        iv                          = utl.str2hex('00' * 0x10)     # ?  16byte
        if self.cfgtor.scenario().is_sec_boot_enable() and self.cfgtor.scenario().is_gsl_enc():
            protectionkey_gsl_l1_enc    = self.cfgtor.get_value('GSL_protection_key_l1', utl.str2hex('0x0') * 4)     # ?  16byte
            protectionKey_gsl_l2_enc    = self.cfgtor.get_value('GSL_protection_key_l2', utl.str2hex('0x0') * 4)     # ?  16byte
            iv                          = self.cfgtor.get_value('GSL_iv',               utl.str2hex('0x0') * 4)     # ?  16byte

            if self.cfgtor.scenario().is_tee_enbale():
                effective_rk = self.cfgtor.get_value('GSL_kdf_sbrk0_file')
            else:
                effective_rk = self.cfgtor.get_value('GSL_kdf_abrk0_file')
            protectionkey_gsl_l1_dec = self.area_tool.enc_ecb(protectionkey_gsl_l1_enc, effective_rk, data_name='gsl L1 dec', decrypt=True)
            protectionkey_gsl_l2_dec = self.area_tool.enc_ecb(protectionKey_gsl_l2_enc, protectionkey_gsl_l1_dec, data_name='gsl L2 dec', decrypt=True)
            content_key = protectionkey_gsl_l2_dec
            print("gsl_enc_content_key:", utl.hex2str(content_key))
            gsl_code = self.area_tool.enc_cbc(gsl_code, content_key, iv, data_name='gsl code data')

        gsl_compress_flag           = utl.str2le("0x0"        , group_size=4) #0x3C7896E1: The GSL Code Area is compressed; Other value: The GSL Code Area is uncompressed
        gsl_uncompressed_len        = utl.int2le(int_data = len(gsl_code), group_size=4)
        text_segment_size           = utl.str2hex("0x0") * 0x4                 # reserved 4byte
        svb_cfg_enable              = utl.str2le("0x0"       , group_size=4)    # resrved
        fmc_data                    = self.get_fmc_data()
        reserved                    = utl.str2hex("0x0") * 0xde                # 0x200 - 144 - 18(fmc data) - 0x40 * 2 = 222 = 0xde
        area = utl.combine_bytes(
            image_id,
            self.structure_version,
            structure_lenth,
            signature_length,
            gsl_code_version_ext,
            mask_gsl_code_version_ext,
            self.msid,
            self.mask_msid,
            gsl_code_area_addr,
            gsl_code_area_len,
            gsl_code_area_hash,
            gsl_enc_flag,
            protectionkey_gsl_l1_enc,
            protectionKey_gsl_l2_enc,
            iv,
            gsl_compress_flag,
            gsl_uncompressed_len,
            text_segment_size,
            svb_cfg_enable,
            fmc_data,
            reserved
            #sig_gsl_info
            #sig_gsl_info_ext
        )
        if self.cfgtor.scenario().is_sec_boot_enable():
            self.cfgtor.reset_path('GSL', 'GSL_key_area')
            mrot_external_private_key_file  = self.cfgtor.get_value('GSL_external_private_key')
            sig_gsl_info                    = self.area_tool.sign(data = area, priv_key_file=mrot_external_private_key_file, data_name="gsl code area sign")
            sig_gsl_info_ext                = utl.str2hex("0x0") * 0x40
        else:
            sig_gsl_info                    = utl.str2hex("0x0") * 0x40
            sig_gsl_info_ext                = utl.str2hex("0x0") * 0x40

        area = utl.combine_bytes(
            area,
            sig_gsl_info,
            sig_gsl_info_ext,
            gsl_code
        )
        utl.write_file(gsl_code_and_info_area_file, area, 'gsl_code_and_info_image')
        log.info('Done.\n\n')

    def build_ree_boot_key_area(self, ree_boot_key_area_file):
        if self.cfgtor.scenario().is_sec_boot_enable():
            log.info('------------- build_ree_boot_key_area -------------')
            image_id                    = utl.str2le("0x4B1E3C1E", group_size=4)
            structure_lenth             = utl.str2le("0x100"     , group_size=4)    # ECC256/SM2
            signature_length            = utl.str2le("0x40"      , group_size=4)    # ECC256/SM2 key lenth

            key_alg             = self._crypto_alg
            ecc_cure_type       = self._crypto_alg
            key_length                  = utl.str2le("0x40"      , group_size=4)    # 64bytes
            self.cfgtor.reset_path('REE_boot','uboot_key_area')
            ree_key_version_ext         = self.cfgtor.get_value('uboot_key_version_ext', utl.str2hex('0x0') * 4)
            mask_ree_key_version_ext    = self.cfgtor.get_value('mask_uboot_key_version_ext', utl.str2hex('0x0') * 4)
            ree_boot_external_public_key_epm_file =  self.cfgtor.get_value('uboot_external_public_key')
            ree_boot_external_public_key = self.area_tool.import_pub_key(ree_boot_external_public_key_epm_file, "OME_ree_boot_external_public_key")

            self.cfgtor.reset_path('maintenance_mode')
            maintenance_mode            = self.cfgtor.get_value('uboot_key_maintenance_mode', utl.str2hex('0x0') * 4)
            if maintenance_mode == utl.str2le("0x3C7896E1"      , group_size=4):
                die_id                      = self.cfgtor.get_value('DIE_ID')
                if die_id == utl.str2hex("0x0") * 0x10:
                    log.info(" \033[1;35m warning: DIE_ID should not all zero\033[0m")
            else:
                die_id                      = utl.str2hex("0x0") * 0x10

            reserved2                   = utl.str2hex("0x0") * 0x38                # Structure_Length-72-Key_Length-Signature_Length = 0x100 - 72 - 0x40*2 = 56 = 0x38

            area = utl.combine_bytes(
                image_id,
                self.structure_version,
                structure_lenth,
                signature_length,
                self.key_owner_id,
                self.key_id,
                key_alg,
                ecc_cure_type,
                key_length,
                ree_key_version_ext,
                mask_ree_key_version_ext,
                self.msid,
                self.mask_msid,
                maintenance_mode,
                die_id,
                reserved2,
                ree_boot_external_public_key
                #sig_ree_key_area
            )

            self.cfgtor.reset_path('root_key')
            ree_root_private_key_file =  self.cfgtor.get_value('REE_root_private_key')

            sig_ree_key_area = self.area_tool.sign(data = area, priv_key_file=ree_root_private_key_file, data_name="ree boot key area sign")
            area = utl.combine_bytes(
                area,
                sig_ree_key_area
            )
        else:
            log.debug('------------- Build build_ree_boot_key_area no-sec image -------------')
            area = self.area_tool.gen_empty_area(size=0x100, fill=0x00)

        utl.write_file(ree_boot_key_area_file, area, 'ree_boot_key_area_image')
        log.info('Done.\n\n')

    def build_boot_params_info_area(self, boot_params_and_info_area_file):
        log.info('------------- build_params_area_info -------------')
        image_id                    = utl.str2le("0x4B87A52D", group_size=4)
        structure_length            = utl.str2le("0x200", group_size=4)     # ECC256/SM2 is 0x200
        signature_length            = utl.str2le("0x40", group_size=4)

        self.cfgtor.reset_path('REE_boot', 'boot_params_Info')
        params_version_ext         = self.cfgtor.get_value('boot_param_version_ext', utl.str2hex('0x0') * 4)
        mask_params_version_ext    = self.cfgtor.get_value('mask_boot_param_version_ext', utl.str2hex('0x0') * 4)

        params_area_addr            = utl.str2le("0x100", group_size=4)       #? 0 means followed Params Area info(emmc params_area_addr need 512bytes align)
        params_area_Len             = self.cfgtor.get_value('single_param_size')
        params_area_num             = self.cfgtor.get_value('boot_param_total_num')
        max_nums = 8
        params_area_num_int = utl.le2int(params_area_num, group_size=4)
        if params_area_num_int <= 0:
            log.err('params_area_num ERR, the total nums should more than 0')
            exit(1)
        if params_area_num_int > max_nums:
            log.err('params_area_num ERR, the total nums should less than 8')
            exit(1)
        alignbytes = utl.le2int(params_area_Len, group_size=4)
        print('params_area_hash', alignbytes)
        print('params_area_num', params_area_num_int)
        params_code = binascii.unhexlify('')
        params_area_hash = binascii.unhexlify('')
        board_index_hash_table = binascii.unhexlify('')
        invalid_params_cnt = 0
        valid_params_cnt = 0
        for board_index in range(params_area_num_int):
            cfg_param_num = 'boot_param_file' + str(board_index)
            cur_param_file = self.cfgtor.get_value(cfg_param_num)
            if cur_param_file == None:
                print(cfg_param_num + " is empty")
                if invalid_params_cnt >= params_area_num_int - 1:
                    log.err('params file config ERR: all is empty file')
                    exit(1)
                single_params_area_hash     = utl.str2hex(str(board_index)) * 0x20
                invalid_params_cnt = invalid_params_cnt + 1
                board_index_hash_table += utl.str2hex("0xff")
            else:
                cur_param_file = utl.align(cur_param_file, n_align_bytes=alignbytes)
                single_params_area_hash     = self.area_tool.digest(data=cur_param_file, data_name='boot_param_file' + str(board_index) + ' hash calculate')
                params_code                 = utl.combine_bytes(params_code, cur_param_file)
                board_index_hash_table += utl.str2hex(str(valid_params_cnt))
                valid_params_cnt += 1
            params_area_hash            = utl.combine_bytes(params_area_hash, single_params_area_hash)
        if params_area_num_int < max_nums:
            patch_hash = utl.str2hex("0x0") * 0x20 * (max_nums - params_area_num_int)
            params_area_hash                = utl.combine_bytes(params_area_hash, patch_hash)
            board_index_hash_table += utl.str2hex("0xff") * (max_nums - params_area_num_int)

        print("board_index_hash_table", board_index_hash_table)

        params_area_num = utl.int2le(int_data = valid_params_cnt, group_size=4)

        reserved                        = utl.str2hex('0xBB') * (0x200 - 0x2c - 0x20 * max_nums - max_nums - 0x40 * 2)# 0x200 - 44 - 32 * 8 - 8(board_index_hash_table_size) - 64 * 2  = 76 = 0x4c

        area = utl.combine_bytes(
            image_id,
            self.structure_version,
            structure_length,
            signature_length,
            params_version_ext,
            mask_params_version_ext,
            self.msid,
            self.mask_msid,
            params_area_addr,
            params_area_Len,
            params_area_num,
            params_area_hash,
            board_index_hash_table,
            reserved
            #sig_params_info,
            #sig_params_info_ext
        )
        if self.cfgtor.scenario().is_sec_boot_enable():
            self.cfgtor.reset_path('REE_boot', 'uboot_key_area')
            ree_boot_external_private_key_file  = self.cfgtor.get_value('uboot_external_private_key')
            sig_params_info                     = self.area_tool.sign(data = area, priv_key_file=ree_boot_external_private_key_file, data_name="parms info area sign")
            sig_params_info_ext                 = utl.str2hex("0x0") * 0x40
        else:
            sig_params_info                     = utl.str2hex("0x0") * 0x40
            sig_params_info_ext                 = utl.str2hex("0x0") * 0x40

        patch_256bytes_for_params_data_512bytes_align = utl.str2hex('0x0') * utl.le2int(params_area_addr, group_size=4)
        area = utl.combine_bytes(
            area,
            sig_params_info,
            sig_params_info_ext,
            patch_256bytes_for_params_data_512bytes_align,
            params_code   #512byte align for emmc/sd image
        )

        utl.write_file(boot_params_and_info_area_file, area, 'params_code_and_info_image')
        log.info('Done.\n\n')


    def build_ree_boot_code_info_area(self, ree_boot_code_and_info_area_file):
        log.info('------------- build_ree_boot_code_info_area -------------')
        image_id                            = utl.str2le("0x4BF01E2D" , group_size=4)   # TEE Flash Root Public Key
        structure_lenth                     = utl.str2le("0x200"      , group_size=4)   # ECC256/SM2
        signature_length                    = utl.str2le("0x40"       , group_size=4)   # ECC256/SM2 key lenth

        self.cfgtor.reset_path('REE_boot', 'uboot_info_area')
        uboot_version_ext                   = self.cfgtor.get_value('uboot_version_ext', utl.str2hex('00') * 4)
        mask_uboot_version_ext              = self.cfgtor.get_value('mask_uboot_version_ext', utl.str2hex('00') * 4)
        uboot_code_area_addr                = utl.str2le("0x0"        , group_size=4)   # 0 means followed UBoot Code info.
        self.cfgtor.reset_path('REE_boot', 'uboot_area')
        uboot_code                          = utl.align(self.cfgtor.get_value('uboot_code_file'), n_align_bytes=0x200)
        uboot_code_area_len                 = utl.int2le(int_data = len(uboot_code), group_size=4)

        if self.cfgtor.scenario().is_sec_boot_enable():
            uboot_code_area_hash                = self.area_tool.digest(data=uboot_code, data_name="uboot code hash calculate")
        else:
            uboot_code_area_hash                = utl.str2hex('00' * 0x20)

        self.cfgtor.reset_path('REE_boot', 'uboot_info_area')
        uboot_enc_flag  = self.cfgtor.get_value('uboot_code_enc_flag', utl.str2hex('0x0') * 4)
        protectionkey_uboot_l1_enc          = utl.str2hex('00' * 0x10)
        protectionkey_uboot_l2_enc          = utl.str2hex('00' * 0x10)
        iv                                  = utl.str2hex('00' * 0x10)
        if self.cfgtor.scenario().is_sec_boot_enable() and self.cfgtor.scenario().is_boot_enc():
            protectionkey_uboot_l1_enc      = self.cfgtor.get_value('uboot_protection_key_l1')     # ?  16byte
            protectionkey_uboot_l2_enc      = self.cfgtor.get_value('uboot_protection_key_l2')     # ?  16byte
            iv                              = self.cfgtor.get_value('uboot_iv')     # ?  16byte
            effective_rk = self.cfgtor.get_value('uboot_kdf_abrk1_file')
            protectionkey_uboot_l1_dec = self.area_tool.enc_ecb(protectionkey_uboot_l1_enc, effective_rk, data_name='uboot L1 dec', decrypt=True)
            protectionkey_uboot_l2_dec = self.area_tool.enc_ecb(protectionkey_uboot_l2_enc, protectionkey_uboot_l1_dec, data_name='uboot L2 dec', decrypt=True)
            content_key = protectionkey_uboot_l2_dec
            print("u-boot_enc_content_key:", utl.hex2str(content_key))
            uboot_code = self.area_tool.enc_cbc(uboot_code, content_key, iv, data_name='uboot code data')

        uboot_compress_flag                 = utl.str2le("0x0" , group_size=4)
        uboot_uncompressed_len              = utl.int2le(int_data = len(uboot_code), group_size=4)
        text_segment_size                   = utl.str2hex('0xff') * 0x4

        uboot_entry_point                   = self.cfgtor.get_value('uboot_entry_point')
        reserved                            = utl.str2hex('0x00') * 0xf4                # 0x200 - 140 - 0x40 * 2 = 244 = 0xf4
        #sig_uboot_info
        #sig_uboot_info_ext
        area = utl.combine_bytes(
            image_id,
            self.structure_version,
            structure_lenth,
            signature_length,
            uboot_version_ext,
            mask_uboot_version_ext,
            self.msid,
            self.mask_msid,
            uboot_code_area_addr,
            uboot_code_area_len,
            uboot_code_area_hash,
            uboot_enc_flag,
            protectionkey_uboot_l1_enc,
            protectionkey_uboot_l2_enc,
            iv,
            uboot_compress_flag,
            uboot_uncompressed_len,
            text_segment_size,
            uboot_entry_point,
            reserved
            #sig_uboot_info,
            #sig_uboot_info_ext
        )
        if self.cfgtor.scenario().is_sec_boot_enable():
            self.cfgtor.reset_path('REE_boot', 'uboot_key_area')
            ree_boot_external_private_key_file  = self.cfgtor.get_value('uboot_external_private_key')
            sig_uboot_info                     = self.area_tool.sign(data = area, priv_key_file=ree_boot_external_private_key_file, data_name="parms info area sign")
            sig_uboot_info_ext                 = utl.str2hex("0x0") * 0x40
        else:
            sig_uboot_info                     = utl.str2hex("0x0") * 0x40
            sig_uboot_info_ext                 = utl.str2hex("0x0") * 0x40

        area = utl.combine_bytes(
            area,
            sig_uboot_info,
            sig_uboot_info_ext,
            uboot_code
        )
        utl.write_file(ree_boot_code_and_info_area_file, area, 'ree_boot_code_and_info_area_file')
        log.info('Done.\n\n')

    def build_teeos_key_area(self, tee_key_area_file):
        if self.cfgtor.scenario().is_sec_boot_enable:
            if self.cfgtor.scenario().is_tee_enbale():
                log.info('------------- build_tee_key_area -------------')
                image_id            = utl.str2le("0x4BE10F1E", group_size=4)
                structure_lenth     = utl.str2le("0x100"     , group_size=4)    # ECC256/SM2
                signature_length    = utl.str2le("0x40"      , group_size=4)    # ECC256/SM2 key lenth
                key_alg             = self._crypto_alg
                ecc_cure_type       = self._crypto_alg

                key_length          = utl.str2le("0x40"      , group_size=4)    # 64
                self.cfgtor.reset_path('TEE', 'TEEOS_key_area')
                rrot_key_version_ext         = self.cfgtor.get_value('TEEOS_key_version_ext', utl.str2hex('0x0') * 4)
                mask_rrot_key_version_ext    = self.cfgtor.get_value('mask_TEEOS_key_version_ext', utl.str2hex('0x0') * 4)
                rrot_external_public_key_epm_file =  self.cfgtor.get_value('TEEOS_external_public_key')
                rrot_external_public_key = self.area_tool.import_pub_key(rrot_external_public_key_epm_file, "OME_RRoT_External_Public_Key")

                self.cfgtor.reset_path('maintenance_mode')
                maintenance_mode            = self.cfgtor.get_value('TEEOS_key_maintenance_mode', utl.str2hex('0x0') * 4)
                if maintenance_mode == utl.str2le("0x3C7896E1"      , group_size=4):
                    die_id                      = self.cfgtor.get_value('DIE_ID')
                    if die_id == utl.str2hex("0x0") * 0x10:
                        log.info(" \033[1;35m warning: DIE_ID should not all zero\033[0m")
                else:
                    die_id                      = utl.str2hex("0x0") * 0x10
                reserved2                   = utl.str2hex("0x0") * 0x38                # Structure_Length-72-Key_Length-Signature_Length = 0x100 - 72 - 0x40*2 = 56 = 0x38

                #sig_ree_key_area
                area = utl.combine_bytes(
                    image_id,
                    self.structure_version,
                    structure_lenth,
                    signature_length,
                    self.key_owner_id,
                    self.key_id,
                    key_alg,
                    ecc_cure_type,
                    key_length,
                    rrot_key_version_ext,
                    mask_rrot_key_version_ext,
                    self.msid,
                    self.mask_msid,
                    maintenance_mode,
                    die_id,
                    reserved2,
                    rrot_external_public_key
                    #sig_tee_key_area
                )
                self.cfgtor.reset_path('GSL', 'GSL_key_area')
                tee_private_key_file =  self.cfgtor.get_value('GSL_external_private_key')

                sig_tee_key_area = self.area_tool.sign(data = area, priv_key_file=tee_private_key_file, data_name="tee key area sign")
                area = utl.combine_bytes(
                    area,
                    sig_tee_key_area
                )
            else:
                log.info('------------- build_tee_key_area no-tee image -------------')
                area = self.area_tool.gen_empty_area(size=0x100, fill=0x00)
        else:
            log.info('------------- build_tee_key_area no-sec image -------------')
            area = self.area_tool.gen_empty_area(size=0x100, fill=0x00)

        utl.write_file(tee_key_area_file, area, 'tee_key_area_image')
        log.info('Done.\n\n')

    def build_teeos_atf_code_info_area(self, teeos_atf_code_and_info_area_file):
        if self.cfgtor.scenario().is_sec_boot_enable():
            if self.cfgtor.scenario().is_tee_enbale():
                log.info('------------- tee_code_info_area -------------')
                image_id                    = utl.str2le("0x4BE10F2D" , group_size=4)   # TEE Flash Root Public Key
                structure_lenth             = utl.str2le("0x200"      , group_size=4)   # ECC256/SM2
                signature_length            = utl.str2le("0x40"       , group_size=4)   # ECC256/SM2 key lenth
                self.cfgtor.reset_path('TEE', 'TEEOS_info_area')
                tee_code_version_ext        = self.cfgtor.get_value('TEEOS_version_ext', utl.str2hex('00') * 4)
                mask_tee_code_ext           = self.cfgtor.get_value('mask_TEEOS_version_ext', utl.str2hex('00') * 4)

                self.cfgtor.reset_path('TEE', 'TEEOS_code_area')
                tee_code_area_addr              = utl.str2le("0x0"        , group_size=4)   # 0 means followed UBoot Code info.
                tee_code                        = utl.align(self.cfgtor.get_value('TEEOS_code_file'), n_align_bytes=0x10, fill=0xFF)
                tee_code_area_len               = utl.int2le(int_data = len(tee_code), group_size=4)
                tee_code_area_hash              = self.area_tool.digest(data=tee_code, data_name="tee code hash calculate")

                tee_compress_flag               = utl.str2le("0x0" , group_size=4)
                tee_uncompressed_len            = utl.int2le(int_data = len(tee_code), group_size=4)
                tee_text_segment_size           = utl.str2hex('0xff') * 0x4
                self.cfgtor.reset_path('TEE', 'TEEOS_info_area')
                tee_secure_ddr_size             = utl.str2le("0x0", group_size=4)

                self.cfgtor.reset_path('TEE', 'ATF_code_area')
                atf_code_area_addr              = utl.str2le("0x0"        , group_size=4)   # 0 means followed tee Code info.
                atf_code                        = utl.align(self.cfgtor.get_value('ATF_code_file'), n_align_bytes=0x10, fill=0xFF)
                atf_code_area_len               = utl.int2le(int_data = len(atf_code), group_size=4)
                atf_code_area_hash              = self.area_tool.digest(data=atf_code, data_name="atf code hash calculate")
                atf_compress_flag               = utl.str2le("0" , group_size=4)
                atf_uncompressed_len            = utl.int2le(int_data = len(atf_code), group_size=4)
                atf_text_segment_size           = utl.str2hex('0xff') * 0x4
                reserved                        = utl.str2hex('0x0') * 0xc0                # 0x200 - 140 - 0x40 * 2 = 244 = 0xf4

                self.cfgtor.reset_path('TEE', 'TEEOS_info_area')
                tee_enc_flag                = self.cfgtor.get_value('TEEOS_ATF_enc_flag')
                protectionkey_tee_l1_enc        = utl.str2hex('00' * 0x10)     # ?  16byte
                protectionKey_tee_l2_enc        = utl.str2hex('00' * 0x10)     # ?  16byte
                iv                              = utl.str2hex('00' * 0x10)     # ?  16byte

                if self.cfgtor.scenario().is_atf_tee_enc():
                    protectionkey_tee_l1_enc    = self.cfgtor.get_value('TEEOS_ATF_protection_key_l1')     # ?  16byte
                    protectionKey_tee_l2_enc    = self.cfgtor.get_value('TEEOS_ATF_protection_key_l2')     # ?  16byte
                    iv                          = self.cfgtor.get_value('TEEOS_ATF_iv')     # ?  16byte
                    effective_rk                 = self.cfgtor.get_value('TEEOS_ATF_kdf_sbrk1_file')
                    protectionkey_tee_l1_dec    = self.area_tool.enc_ecb(protectionkey_tee_l1_enc, effective_rk, data_name='tee L1 dec', decrypt=True)
                    protectionKey_tee_l2_dec    = self.area_tool.enc_ecb(protectionKey_tee_l2_enc, protectionkey_tee_l1_dec, data_name='tee L2 dec', decrypt=True)
                    content_key                 = protectionKey_tee_l2_dec
                    print("tee_enc_content_key:", utl.hex2str(content_key))
                    atf_code = self.area_tool.enc_cbc(atf_code, content_key, iv, data_name='atf code data enc')
                    tee_code = self.area_tool.enc_cbc(tee_code, content_key, iv, data_name='tee code data enc')

                area = utl.combine_bytes(
                    image_id,
                    self.structure_version,
                    structure_lenth,
                    signature_length,
                    tee_code_version_ext,
                    mask_tee_code_ext,
                    self.msid,
                    self.mask_msid,
                    tee_code_area_addr,
                    tee_code_area_len,
                    tee_code_area_hash,
                    tee_enc_flag,
                    protectionkey_tee_l1_enc,
                    protectionKey_tee_l2_enc,
                    iv,
                    tee_compress_flag,
                    tee_uncompressed_len,
                    tee_text_segment_size,
                    tee_secure_ddr_size,
                    atf_code_area_addr,
                    atf_code_area_len,
                    atf_code_area_hash,
                    atf_compress_flag,
                    atf_uncompressed_len,
                    atf_text_segment_size,
                    reserved
                    #sig_tee_info,
                    #sig_tee_info_ext
                )
                self.cfgtor.reset_path('TEE', 'TEEOS_key_area')
                rrot_external_private_key_file  = self.cfgtor.get_value('TEEOS_external_private_key')
                sig_tee_info                     = self.area_tool.sign(data = area, priv_key_file=rrot_external_private_key_file, data_name="tee info area sign")
                sig_tee_info_ext                 = utl.str2hex("0x0") * 0x40
                area = utl.combine_bytes(
                    area,
                    sig_tee_info,
                    sig_tee_info_ext,
                    atf_code,
                    tee_code
                )
            else:
                log.info('------------- build_tee_info_area no-tee image -------------')
                area = self.area_tool.gen_empty_area(size=0x200, fill=0x00)
                area = utl.combine_bytes(
                    area,
                    atf_code,
                    tee_code
                )
        else:
            log.info('------------- build_tee_info_area no-sec image -------------')
            area = self.area_tool.gen_empty_area(size=0x200, fill=0x00)
            area = utl.combine_bytes(
                area,
                atf_code,
                tee_code
            )

        utl.write_file(teeos_atf_code_and_info_area_file, area, 'teeos_atf_code_and_info_area_file')
        log.info('Done.\n\n')


class OemImageBuilder:
    def __init__(self, cfgtor):
        self.cfgtor = cfgtor
        self.area_tool = AreaTool(self.cfgtor.scenario().crypto_alg())

        self.ree_flash_root_pub_key_area_file    =  None
        self.tee_flash_root_pub_key_area_file    =  None
        self.cfct_area_file                =  None
        self.gsl_key_area_file             =  None
        self.gsl_code_and_info_area_file       =  None

        self.ree_boot_key_area_file        =  None
        self.boot_params_and_info_area_file    =  None
        self.boot_params_area_file         =  None
        self.ree_boot_code_and_info_area_file  =  None

        self.tee_os_key_area_file             =  None
        self.teeos_atf_code_and_info_area_file       =  None


    def build_images(self, boot_out_file, tee_out_file=None):
        self._build_boot_image(boot_out_file)
        if self.cfgtor.scenario().is_tee_enbale():
            self._build_tee_image(tee_out_file)
        return

    def _build_boot_image(self, out_file):
        log.debug('============================================================')
        log.debug('                     Build Boot Image')
        log.debug('------------------------------------------------------------')
        log.debug('        Input                |          File')
        log.debug('-----------------------------+------------------------------')
        log.debug('ree_root_pub_key Area        | %s' % self.ree_flash_root_pub_key_area_file)
        log.debug('tee_root_pub_key Area        | %s' % self.tee_flash_root_pub_key_area_file)
        log.debug('cfct Area                    | %s' % self.cfct_area_file)
        log.debug('gsl_key Area                 | %s' % self.gsl_key_area_file)
        log.debug('gsl_code_info Area           | %s' % self.gsl_code_and_info_area_file)
        log.debug('ree_boot_key Area            | %s' % self.ree_boot_key_area_file)
        log.debug('boot_params_info Area        | %s' % self.boot_params_and_info_area_file)
        log.debug('ree_boot_code_info Area      | %s' % self.ree_boot_code_and_info_area_file)
        log.debug('----------------------------------------------------------')

        chk.check_empty(self.ree_flash_root_pub_key_area_file, 'ree_root_pub_key Area')
        chk.check_empty(self.tee_flash_root_pub_key_area_file, 'tee_root_pub_key Area')
        chk.check_empty(self.cfct_area_file, 'cfct Area')
        chk.check_empty(self.gsl_key_area_file, 'gsl_key Area')
        chk.check_empty(self.gsl_code_and_info_area_file, 'gsl_code_info Area')
        chk.check_empty(self.ree_boot_key_area_file, 'ree_boot_key Area')
        chk.check_empty(self.boot_params_and_info_area_file, 'boot_params_info Area')
        chk.check_empty(self.ree_boot_code_and_info_area_file, 'ree_boot_code_info Area')

        ree_flash_root_pub_key_area_file    =  utl.read_file(self.ree_flash_root_pub_key_area_file)
        tee_flash_root_pub_key_area_file    =  utl.read_file(self.tee_flash_root_pub_key_area_file)
        tp_root_pub_key_area_fill           =  self.area_tool.gen_empty_area(size=0x80, fill=0x00)
        cfct_area_file                      =  utl.read_file(self.cfct_area_file)
        gsl_key_area_4K_align_fill          =  self.area_tool.gen_empty_area(size=0x800 - 0x180, fill=0x00)

        gsl_key_area_file                   =  utl.read_file(self.gsl_key_area_file)
        tp_key_area_file                    =  self.area_tool.gen_empty_area(size=0x100, fill=0x00)
        gsl_code_and_info_area_file         =  utl.read_file(self.gsl_code_and_info_area_file)
        ree_boot_key_area_file              =  utl.read_file(self.ree_boot_key_area_file)
        boot_params_and_info_area_file      =  utl.read_file(self.boot_params_and_info_area_file)
        ree_boot_code_and_info_area_file    =  utl.read_file(self.ree_boot_code_and_info_area_file)

        image = utl.combine_bytes(
            ree_flash_root_pub_key_area_file,
            tee_flash_root_pub_key_area_file,
            tp_root_pub_key_area_fill,
            cfct_area_file,
            gsl_key_area_4K_align_fill,

            gsl_key_area_file,
            tp_key_area_file,
            gsl_code_and_info_area_file,

            ree_boot_key_area_file,
            boot_params_and_info_area_file,
            ree_boot_code_and_info_area_file
        )

        utl.write_file(out_file, image, 'Boot Image')
        log.debug('Done')

        return

    def _build_tee_image(self, out_file):
        log.debug('=============================================================')
        log.debug('                       Build TEE Image')
        log.debug('-------------------------------------------------------------')
        log.debug('          Input              |          File')
        log.debug('-----------------------------+-------------------------------')
        log.debug('teeos_key Area               | %s' % self.tee_os_key_area_file)
        log.debug('teeos_atf_code_info Area     | %s' % self.teeos_atf_code_and_info_area_file)
        log.debug('------------------------------------------------------------')

        chk.check_empty(self.tee_os_key_area_file, 'teeos_key Area')
        chk.check_empty(self.teeos_atf_code_and_info_area_file, 'teeos_atf_code_info Area')

        tee_key_area  = utl.read_file(self.tee_os_key_area_file)
        atf_area      = utl.read_file(self.teeos_atf_code_and_info_area_file)

        image = utl.combine_bytes(
           tee_key_area,
           atf_area,
        )

        utl.write_file(out_file, image, 'TEE Image')
        log.debug('Done')

        return

class OEM_Checker:
    def __init__():
        pass
