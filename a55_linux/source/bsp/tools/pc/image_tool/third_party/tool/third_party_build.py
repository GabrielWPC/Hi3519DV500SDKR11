import os
import sys

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/../..' % SCRIPT_DIR)
sys.path.append(WORK_DIR)

import common.config as cfg
from common.config import CfgValue
from common.area_tool import AreaTool, AreaCfg
import common.logger as log
import common.util as utl
import common.check as chk

class ThirdPartyAreaBuilder:
    def __init__(self, cfgtor):
        self.cfgtor = cfgtor    # Configuartor
        self.area_tool = AreaTool(self.cfgtor.scenario().crypto_alg())
        if self.cfgtor.scenario().crypto_alg() == CfgValue.CryptoAlg.ECC:
            self._crypto_alg        = utl.str2le("0x2A13C812", group_size=4)
        elif self.cfgtor.scenario().crypto_alg() == CfgValue.CryptoAlg.SM:
            self._crypto_alg        = utl.str2le("0x2A13C823", group_size=4)
        self.cfgtor.reset_path('TP_info')
        self.msid                   = self.cfgtor.get_value('TP_msid_ext',         utl.str2hex('0x0') * 4)
        self.mask_msid              = self.cfgtor.get_value('mask_TP_msid_ext',    utl.str2hex('0x0') * 4)
        self.structure_version      = utl.str2le("0x00010000", group_size=4)
        self.key_owner_id           = utl.str2le("0x0"       , group_size=4)
        self.key_id                 = utl.str2le("0x0"       , group_size=4)

    def build_tp_flash_root_pub_key_area(self, area_out_file, area_out_file_chksum_file):
        if self.cfgtor.scenario().is_sec_boot_enable():
            log.debug('------------- Build tp_flash_root_pub_key_area -------------')
            image_id            = utl.str2le("0x4B2D4B1E", group_size=4)    # REE Flash root_key Public Key
            structure_lenth     = utl.str2le("0x80"      , group_size=4)    # ECC256/SM2
            key_alg             = self._crypto_alg
            ecc_cure_type       = self._crypto_alg
            key_length          = utl.str2le("0x40"      , group_size=4)    # 64
            verify_enable_ext   = utl.str2le("0x0"      , group_size=4)    # 64
            reserved            = utl.str2hex('0x0') * 0x1c                  # len = 0x80-0x24-0x40 = 0x1c'

            self.cfgtor.reset_path('root_key')
            root_pub_key_pem_file = self.cfgtor.get_value('TP_root_pub_key')
            root_pub_key = self.area_tool.import_pub_key(root_pub_key_pem_file, "TP_root_pub_key")
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
            utl.write_file(area_out_file_chksum_file, chksum_str, 'tp_root_public_key_area_image Area Checksum')

            self.cfgtor.reset_path('SCS_simulate')
            verify_enable_ext = self.cfgtor.get_value('TP_verify_simulate', utl.str2hex('0x0') * 4)
            if verify_enable_ext == utl.str2le("0x2A13C812", group_size=4):
                verify_enable_ext_offset = 32
                area = utl.replace_bytes(
                    old_bytes=area,
                    pos=verify_enable_ext_offset,
                    new_bytes=verify_enable_ext
                )
        else:
            log.debug('------------- Build tp_flash_root_pub_key_area no-sec image -------------')
            area = self.area_tool.gen_empty_area(size=0x80, fill=0x00)

        utl.write_file(area_out_file, area, 'tp_root_public_key_area_image')
        log.info('Done.\n\n')

    def build_tp_key_area(self, tp_key_area_file):
        if self.cfgtor.scenario().is_sec_boot_enable():
            log.debug('------------- Build tp_key_area -------------')
            image_id                    = utl.str2le("0x4B3C5A3C", group_size=4)    #
            structure_lenth             = utl.str2le("0x100"     , group_size=4)    # ECC256/SM2
            signature_length            = utl.str2le("0x40"      , group_size=4)    # ECC256/SM2 key lenth
            key_alg                     = self._crypto_alg
            ecc_cure_type               = self._crypto_alg
            key_length                  = utl.str2le("0x40"      , group_size=4)    # 64bytes

            self.cfgtor.reset_path('GSL', 'TP_key_area')
            mrot_key_version_ext        = self.cfgtor.get_value('TP_key_version_ext', utl.str2hex('0x0') * 4)
            mask_mrot_key_version_ext   = self.cfgtor.get_value('mask_TP_key_version_ext', utl.str2hex('0x0') * 4)
            mrot_external_public_key_pem_file =  self.cfgtor.get_value('TP_external_public_key')
            mrot_external_public_key = self.area_tool.import_pub_key(mrot_external_public_key_pem_file, "TP_external_public_key")

            self.cfgtor.reset_path('maintenance_mode')
            maintenance_mode            = self.cfgtor.get_value('TP_key_maintenance_mode', utl.str2hex('0x0') * 4)
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
            self.cfgtor.reset_path('root_key')
            tee_root_private_key_file =  self.cfgtor.get_value('TP_root_private_key')

            sig_tee_boot_key_area = self.area_tool.sign(data = area, priv_key_file=tee_root_private_key_file, data_name="tp key area sign")
            area = utl.combine_bytes(
                area,
                sig_tee_boot_key_area
            )
        else:
            log.debug('------------- Build tp_key_area no-sec image-------------')
            area = self.area_tool.gen_empty_area(size=0x100, fill=0x00)
        utl.write_file(tp_key_area_file, area, 'tp key area image')

class ThirdPartyDoubleSigner:
    def __init__(self, cfgtor):
        self.cfgtor = cfgtor    # Configuartor
        self.area_tool = AreaTool(self.cfgtor.scenario().crypto_alg())

    def sign_boot_image(self, oem_boot_image_file, tp_root_pub_key_area_file, tp_key_area_file, out_file):
        len_width = 4
        sign_len = AreaCfg.SIGNATURE_SIZE

        log.debug('=================== Sign Boot Image =====================')
        chk.check_empty(tp_root_pub_key_area_file, 'Third_party_Root_Public_Key Area')
        chk.check_empty(tp_key_area_file, 'Third_party_Key Area')
        chk.check_empty(oem_boot_image_file, 'Boot Image')

        self.cfgtor.reset_path('GSL', 'TP_key_area')
        tp_ext_priv_key_file = self.cfgtor.get_value('TP_external_private_key')

        boot_image = utl.read_file(oem_boot_image_file, 'Boot Image')

        tp_root_pub_key_area = utl.read_file(tp_root_pub_key_area_file, 'Third_party_Root_Public_Key Area')
        root_pub_key_area_size = 0x80
        tp_root_pub_key_area_offset = root_pub_key_area_size * 2 # ree + tee

        #fill tp root pub key area to boot_image
        boot_image = utl.replace_bytes(
            old_bytes=boot_image,
            pos=tp_root_pub_key_area_offset,
            new_bytes=tp_root_pub_key_area
        )

        ctct_fill_size = 0x680
        cfct_area_size = 0x800
        cfct_area_and_fill_size = cfct_area_size + ctct_fill_size
        key_area_size = 0x100
        tp_key_area_offset = root_pub_key_area_size * 3 + cfct_area_and_fill_size + key_area_size    # ree + tee + cfct + fill + gsl key
        tp_key_area_data = utl.read_file(tp_key_area_file, 'Third_party_Key')

        #fill tp key area to boot_image
        boot_image = utl.replace_bytes(
            old_bytes=boot_image,
            pos=tp_key_area_offset,
            new_bytes=tp_key_area_data
        )

        info_area_size = 0x200
        gsl_info_area_offset = tp_key_area_offset + key_area_size # tp key
        gsl_code_area_len_offset = gsl_info_area_offset + 36
        gsl_code_area_len =  self._get_area_len(boot_image, gsl_code_area_len_offset)
        print("gsl_code_area_len ", gsl_code_area_len)
        #tp sign gsl info area
        tp_sign_len = 0x40
        gsl_info_area_data = boot_image[gsl_info_area_offset: gsl_info_area_offset + info_area_size - tp_sign_len]
        sig_gsl_info_area_tp = self.area_tool.sign(
            data=gsl_info_area_data,
            priv_key_file = tp_ext_priv_key_file,
            data_name='GSL_info Area'
        )
        # fill tp signed data to boot_image
        gsl_info_tp_sign_offset = gsl_info_area_offset + info_area_size - tp_sign_len
        boot_image = utl.replace_bytes(
            old_bytes=boot_image,
            pos=gsl_info_tp_sign_offset,
            new_bytes=sig_gsl_info_area_tp
        )

        ree_boot_key_area_offset = gsl_info_area_offset + info_area_size + gsl_code_area_len
        param_info_area_offset = ree_boot_key_area_offset + key_area_size
        param_info_fill_size = 0x100
        param_code_offset = param_info_area_offset + info_area_size + param_info_fill_size
        single_param_code_len_offset = param_info_area_offset + 36
        single_param_code_area_len =  self._get_area_len(boot_image, single_param_code_len_offset)
        print("single_param_code_area_len ",single_param_code_area_len)
        single_param_num_offset = param_info_area_offset + 40
        single_param_num =  self._get_area_len(boot_image, single_param_num_offset)
        print("single_param_num ", single_param_num)
        #tp sign params info area
        tp_sign_len = 0x40
        param_info_area_data = boot_image[param_info_area_offset: param_info_area_offset + info_area_size - tp_sign_len]
        sig_param_info_area_tp = self.area_tool.sign(
            data=param_info_area_data,
            priv_key_file = tp_ext_priv_key_file,
            data_name='param_info Area'
        )
        # fill tp signed data to boot_image
        param_info_tp_sign_offset = param_info_area_offset + info_area_size - tp_sign_len
        boot_image = utl.replace_bytes(
            old_bytes=boot_image,
            pos=param_info_tp_sign_offset,
            new_bytes=sig_param_info_area_tp
        )

        uboot_info_area_offset = param_code_offset + single_param_num * single_param_code_area_len
        uboot_code_len_offset = uboot_info_area_offset + 36
        uboot_code_len = self._get_area_len(boot_image, uboot_code_len_offset)
        print("uboot_code_len ", uboot_code_len)

        #tp sign uboot info area
        tp_sign_len = 0x40
        uboot_info_area_data = boot_image[uboot_info_area_offset: uboot_info_area_offset + info_area_size - tp_sign_len]
        sig_uboot_info_area_tp = self.area_tool.sign(
            data=uboot_info_area_data,
            priv_key_file = tp_ext_priv_key_file,
            data_name='uboot_info Area'
        )
        # fill tp signed data to boot_image
        uboot_info_tp_sign_offset = uboot_info_area_offset + info_area_size - tp_sign_len
        boot_image = utl.replace_bytes(
            old_bytes=boot_image,
            pos=uboot_info_tp_sign_offset,
            new_bytes=sig_uboot_info_area_tp
        )
        utl.write_file(out_file, boot_image, 'Double Signed Boot Image')
        log.debug('Done.')
        return

    def sign_tee_image(self, oem_tee_image_file, out_file):
        sign_len = AreaCfg.SIGNATURE_SIZE
        log.debug('=================== Sign TEE Image =====================')
        chk.check_empty(oem_tee_image_file, 'TEE Image')

        self.cfgtor.reset_path('GSL', 'TP_key_area')
        tp_ext_priv_key_file = self.cfgtor.get_value('TP_external_private_key')

        tee_image = utl.read_file(oem_tee_image_file, 'TEE Image')
        teeosKeyArea_offset = 0
        key_area_size = 0x100
        tee_info_area_offset = teeosKeyArea_offset + key_area_size
        info_area_size = 0x200

        #tp sign tee info area
        tp_sign_len = 0x40
        tee_info_area_data = tee_image[tee_info_area_offset: tee_info_area_offset + info_area_size - tp_sign_len]
        sig_tee_info_area_tp = self.area_tool.sign(
            data=tee_info_area_data,
            priv_key_file = tp_ext_priv_key_file,
            data_name='tee_info Area'
        )
        # fill tp signed data to tee_image
        tee_info_tp_sign_offset = tee_info_area_offset + info_area_size - tp_sign_len
        tee_image = utl.replace_bytes(
            old_bytes=tee_image,
            pos=tee_info_tp_sign_offset,
            new_bytes=sig_tee_info_area_tp
        )
        utl.write_file(out_file, tee_image, 'Double Signed TEE Image')
        log.debug('Done.')
        return

    def _get_area_len(self, image, offset):
        len_width = 4
        len_bytes = image[offset:(offset+len_width)]
        len_bytes = utl.reverse_endian(len_bytes, group_size=4)
        return int(utl.hex2str(len_bytes), 16)

