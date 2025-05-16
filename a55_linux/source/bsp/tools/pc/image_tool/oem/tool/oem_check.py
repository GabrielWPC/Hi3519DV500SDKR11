import os
import sys
import common.logger as log
import common.util as utl

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/../..' % SCRIPT_DIR)
sys.path.append(WORK_DIR)

import common.util as util
import common.logger as log
from common.area_tool import AreaTool
from kdf import KdfTool

# The values in OTP and the values in the mirror may conflict, and they are verified according to the following criteria.
# The security algorithm is based on the configuration in OTP. If the value in the mirror is inconsistent with the configuration in OTP, an error will be reported;
# For the signature verification enablement:
#   - It is determined by the values in the mirror and OTP together;
#   - If OTP is enabled but not in the mirror, an error will be reported;
#   - If OTP is not enabled but enabled in the mirror, verification will be performed;
#   - If neither OTP nor the mirror is enabled, the verification will be skipped;
# The encryption enablement is determined by the values in the mirror and OTP together, and the check policy same as the verification enable configurations.

class OneTimeProgrammable:
    def __init__(self, file = None):
        self.__soc_tee_enable = None
        self.__ree_verify_enable = None
        self.__ree_hash_flash_root_key = None
        self.__msid = None
        self.__params_ver = None
        self.__ree_boot_ver = None
        self.__tee_verify_enable = None
        self.__tee_hash_flash_root_key = None
        self.__tee_boot_ver = None
        self.__tee_os_ver = None
        self.__scs_alg_sel = None
        self.__ree_dec_enable = None
        self.__obfu_mrk1 = None
        self.__tee_dec_enable = None
        self.load(file)

    def load(self, file):
        import json
        with open(file, 'r') as f:
            obj = json.load(f)
            self.__soc_tee_enable = int(obj['soc_tee_enable'], 16)
            self.__ree_verify_enable = int(obj['ree_verify_enable'], 16)
            self.__ree_hash_flash_root_key = util.str2hex(obj["ree_hash_flash_root_key"])
            self.__msid = int(obj['msid'], 16)
            self.__params_ver = int(obj['params_ver'], 16)
            self.__ree_boot_ver = int(obj['ree_boot_ver'], 16)

            self.__tee_verify_enable = int(obj['tee_verify_enable'], 16)
            self.__tee_hash_flash_root_key = util.str2hex(obj["tee_hash_flash_root_key"])
            self.__tee_boot_ver = int(obj['tee_boot_ver'], 16)
            self.__tee_os_ver = int(obj['tee_os_ver'], 16)

            self.__scs_alg_sel = int(obj['scs_alg_sel'], 16)
            self.__ree_dec_enable = int(obj['ree_dec_enable'], 16)
            self.__tee_dec_enable = int(obj['tee_dec_enable'], 16)
            self.__obfu_mrk1 = util.str2hex(obj['obfu_mrk1'])

    def ree_verify_enable(self):
        return self.__ree_verify_enable != 0x42

    def tee_verify_enable(self):
        return self.__tee_verify_enable != 0x42

    def soc_tee_enable(self):
        return self.__soc_tee_enable != 0x42

    def soc_tee_enable_val(self):
        return self.__soc_tee_enable

    def tee_dec_enable(self):
        return self.__tee_dec_enable != 0x0

    def ree_dec_enable(self):
        return self.__ree_dec_enable != 0x0

    def params_ver(self, mask):
        return bin(self.__params_ver & mask).count('1')

    def ree_boot_ver(self, mask):
        return bin(self.__ree_boot_ver & mask).count('1')

    def tee_boot_ver(self, mask):
        return bin(self.__tee_boot_ver & mask).count('1')

    def tee_os_ver(self, mask):
        return bin(self.__tee_os_ver & mask).count('1')

    def msid(self, mask):
        return self.__msid & mask

    def ree_hash_flash_root_key(self):
        return self.__ree_hash_flash_root_key

    def tee_hash_flash_root_key(self):
        return self.__tee_hash_flash_root_key

    def obfu_mrk1(self):
        return self.__obfu_mrk1

    def scs_alg_sel(self):
        return "ECC+SHA256+AES" if self.__scs_alg_sel == 0 else "SM2+SM3+SM4"

class Field(object):
    def __init__(self, name, width = 0, val = None, left_pad = 0, right_pad = 0):
        self.offset = 0
        self.width = width

        self.__name = name
        self.__left_pad = left_pad
        self.__right_pad = right_pad
        self.__val = val

    def name(self):
        return self.__name

    def val(self):
        return self.__val

    def val2hex(self):
        if self.__val != None:
            return self.__val
        return utl.str2hex('0x0') * self.width

    def hex2val(self, raw):
        return raw

    def populate_image(self):
        return utl.str2hex('0x0') * self.__left_pad + \
                self.val2hex() + \
                utl.str2hex('0x0') * self.__right_pad

    def extract_val(self, raw):
        _hex = raw
        if self.__left_pad != 0:
            _hex = _hex[self.__left_pad:]
        if self.__right_pad != 0:
            _hex = _hex[:-self.__right_pad]
        self.__val = self.hex2val(_hex)
        return self.__val

    def __len__(self):
        return self.__left_pad + self.width + self.__right_pad

    def replace(self, image, new):
        return utl.replace_bytes(
                old_bytes=image,
                pos=self.offset,
                new_bytes=new
                )

class ImageArea(object):
    def __init__(self, name, field_list = [], static_info = {}):
        self.__name = name
        self.__fields = field_list
        self.__area_info = {}
        self.__static_info = static_info

    def parse(self, raw):
        offset = 0
        area_info = {}
        for field in self.__fields:
            field.offset = offset
            field.val = field.extract_val(raw[offset: offset + len(field)])
            area_info[field.name()] = field
            offset += len(field)
        area_info["length"] = offset
        self.__area_info = area_info
        return area_info

    def area_info(self):
        return self.__area_info

    def set_area_info(self, area_info):
        self.__area_info = area_info

    def check(self, raw):
        for field_name, val in self.__static_info.items():
            if val is not None and self.__area_info[field_name].val != val:
                log.error("{}: {} check fail: {:#x} expect {:#x}".format(self.name(), field_name, self.__area_info[field_name].val, val))
                return False
            log.info("{}: {} check pass. ".format(self.name(), field_name))
        return True

    def build(self):
        area = utl.combine_bytes(*list(
                self.__fields.map(
                    lambda field: field.populate_image()
                    )))
        return area

    def fields(self):
        return self.__fields

    def name(self):
        return self.__name

    def dump(self):
        log.info("========== {} ==========".format(self.__name))
        for k, v in self.area_info().items():
            if k == "length":
                log.info("area length: {:#x}".format(v))
            elif type(v.val) == int:
                log.info("{}, {:#x}".format(k, v.val))
            else:
                log.info("key:{}, val:{}".format(k, v.val))
        log.info("=========== end =========")
        log.info("")

class FieldLe(Field):
    def __init__(self, name, width=4, val=0):
        super(FieldLe, self).__init__(name, width=width, val=val)

    def val2hex(self):
        return util.str2le(str(self.__val), group_size=self.width) 

    def hex2val(self, raw):
        return util.le2int(raw, self.width)

class FieldRes(Field):
    def __init__(self, name, width):
        super(FieldRes, self).__init__(name, width=width, val=None)

    def hex2val(self, raw):
        return raw

class RootPublicKeysArea(ImageArea):
    def __init__(self, key_name, otp, skip_check = False):
        structure_length = 0x80
        key_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("KeyOwnerID"),
                FieldLe("KeyID"),
                FieldLe("Key_Alg"),
                FieldLe("ECC_Curve_Type"),
                FieldLe("Key_Length"),
                FieldLe("Verify_Enable_Ext"),
                FieldRes("Reserved", structure_length - 36 - key_length),
                Field("Flash_Root_Public_Key", key_length),
                ]

        static_info = {
                "Image_ID" : 0x4BA5C31E if key_name == "Ree" else 0x4B96B41E if key_name == "Tee" else 0x4B2D4B1E,
                "Structure_Version": 0x00010000,
                "Structure_Length": structure_length,
                "KeyOwnerID": 0x0,
                "KeyID": 0x0,
                "Key_Alg": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "ECC_Curve_Type": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "Key_Length": key_length,
                # "Verify_Enable_Ext": 0x2A13C812 if otp.ree_verify_enable() else None,
                }

        self.__skip_check = skip_check
        super(RootPublicKeysArea, self).__init__(key_name + "RootPublicKeysArea", fields, static_info)

    def check(self, raw, area_tool, hash_val):
        if self.__skip_check:
            log.info("skip check")
            return True

        if super(RootPublicKeysArea, self).check(raw) != True:
            return False

        field = self.area_info()["Verify_Enable_Ext"]
        length = self.area_info()["length"]
        raw = field.replace(raw, utl.str2hex('00' * field.width))
        digest = area_tool.digest(data=raw[:length], data_name="check {} hash val".format(self.name()))
        if digest != hash_val:
            log.error("{}: {} check fail: {} expect {}".format(self.name(), "hash_val", utl.hex2str(digest), utl.hex2str(hash_val)))
            return False
        return True

class KeyAreaOfCFCT(ImageArea):
    def __init__(self, otp, skip_check = True):
        structure_length = 0x100
        key_length = 0x40
        signature_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("Signature_Length"),
                FieldLe("KeyOwnerID"),
                FieldLe("KeyID"),
                FieldLe("Key_Alg"),
                FieldLe("ECC_Curve_Type"),
                FieldLe("Key_Length"),
                FieldLe("CFCT_Key_Version_Ext"),
                FieldLe("Mask_CFCT_Key_Version_Ext"),
                FieldLe("MSID_Ext"),
                FieldLe("Mask_MSID_Ext"),
                FieldLe("MaintenanceMode"),
                Field("DIE_ID", 16),
                FieldLe("CFCT_Addr"),
                FieldLe("Rom_Root_Pub_Key_Sel_Ext", 1),
                FieldRes("Reserved2", structure_length - 77 - key_length - signature_length),
                Field("CFCT_External_Public_Key", key_length),
                Field("Sig_CFCT_Key_Area", signature_length)
                ]

        static_info = {
                "Image_ID" : 0x4BD2F01E,
                "Structure_Version": 0x00010000,
                "Structure_Length": 0x100,
                "Signature_Length": 0x40,
                "KeyOwnerID": 0x0,
                # "KeyID": 0x0,
                # CFCT: The choice of security algorithm in the CFCT region is a topic worthy of discussion.
                # "Key_Alg": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                # "ECC_Curve_Type": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "Key_Length": 0x40,
                "CFCT_Addr": 0x0,
                }

        self.__skip_check = skip_check
        super(KeyAreaOfCFCT, self).__init__("KeyAreaOfCFCT", fields, static_info)

class CFCTArea(ImageArea):
    def __init__(self, otp = None, skip_check = False):
        data_length = 0 # dummy
        structure_length = 0x700
        signature_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("Signature_Length"),
                FieldLe("Symm_RootKey_Type", 1),
                FieldRes("Reserved1", 11),
                FieldLe("AuthorizationMode"),
                Field("Auth_ID", 16),
                FieldLe("OEM_ID"),
                Field("OEM_Name", 28),
                Field("ProtectionKey_CFCT_L1_enc", 16),
                Field("ProtectionKey_CFCT_L2_enc", 16),
                Field("IV", 16),
                FieldLe("Data_Length"),
                Field("Data", data_length), # dummy
                FieldRes("Reserved2", structure_length - 132 - signature_length - data_length),
                Field("Sig_ChipFeatures_Table", signature_length)
                ]

        static_info = {
                "Image_ID" : 0x4BD2F02D,
                "Structure_Version": 0x00010000,
                "Structure_Length": 0x700,
                "Signature_Length": 0x40,
                "Symm_RootKey_Type": 0x0,
                }

        self.__skip_check = skip_check
        super(CFCTArea, self).__init__("CFCTArea", fields, static_info)

    def __set_data_length(self, data_length):
        fields = self.fields()
        next(item for item in fields if item.name() == "Data").width = data_length
        next(item for item in fields if item.name() == "Reserved2").width -= data_length

    def parse(self, raw):
        offset = 0
        data_length = 0
        area_info = {}
        fields = self.fields()
        for field in fields:
            field.offset = offset
            field.val = field.extract_val(raw[offset: offset + len(field)])
            area_info[field.name()] = field
            if field.name() == "Data_Length":
                data_length = field.val
                self.__set_data_length(data_length)
            offset += len(field)
        area_info["length"] = offset
        self.set_area_info(area_info)
        return area_info

    def check(self, raw):
        if self.__skip_check:
            log.info("skip check")
            return True

        if super(CFCTArea, self).check(raw) != True:
            return False
        field_name = "Data_Length"
        if self.area_info()[field_name].val > 1596:
                log.error("{}: {} check fail: {:#x}, which is not more than 1596 bytes".format(self.__name, field_name, self.__area_info[field_name].val))

class GSLKeyArea(ImageArea):
    def __init__(self, otp, skip_check = False):
        structure_length = 0x100
        key_length = 0x40
        signature_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("Signature_Length"),
                FieldLe("KeyOwnerID"),
                FieldLe("KeyID"),
                FieldLe("Key_Alg"),
                FieldLe("ECC_Curve_Type"),
                FieldLe("Key_Length"),
                FieldLe("MRoT_Key_Version_Ext"),
                FieldLe("Mask_MRoT_Key_Version_Ext"),
                FieldLe("MSID_Ext"),
                FieldLe("Mask_MSID_Ext"),
                FieldLe("MaintenanceMode"),
                Field("DIE_ID", width=16),
                FieldLe("Dummy_Field"),
                FieldRes("Reserved2", structure_length - 76 - key_length - signature_length),
                Field("MRoT_External_Public_Key", key_length),
                Field("Sig_TEE_Boot_Key_Area", signature_length),
                ]

        static_info = {
                "Image_ID" : 0x4BB4D21E,
                "Structure_Version": 0x00010000,
                "Structure_Length": 0x100,
                "Signature_Length": 0x40,
                "KeyOwnerID": 0x0,
                "KeyID": 0x0,
                "Key_Alg": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "ECC_Curve_Type": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "Key_Length": 0x40,
                }

        self.__skip_check = skip_check
        super(GSLKeyArea, self).__init__("GSLKeyArea", fields, static_info)

    def check(self, raw, area_tool, pub_key):
        if self.__skip_check:
            log.info("skip check")
            return True

        if super(GSLKeyArea, self).check(raw) != True:
            return False

        field = self.area_info()["Sig_TEE_Boot_Key_Area"]
        length = len(field)
        if not area_tool.verify(pub_key, raw[:field.offset], raw[field.offset:field.offset + length], data_name=None):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True

class TPExternalKeyArea(ImageArea):
    def __init__(self, otp, skip_check):
        structure_length = 0x100
        key_length = 0x40
        signature_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("Signature_Length"),
                FieldLe("KeyOwnerID"),
                FieldLe("KeyID"),
                FieldLe("KeyAlg"),
                FieldLe("ECC_Curve_Type"),
                FieldLe("Key_Length"),
                FieldLe("MRoT_Key_Version_Ext"),
                FieldLe("Mask_MRoT_Key_Version_Ext"),
                FieldLe("MSID_Ext"),
                FieldLe("Mask_MSID_Ext"),
                FieldLe("MaintenanceMode"),
                Field("DIE_ID", width=16),
                FieldLe("Dummy_Field"),
                FieldRes("Reserved2", structure_length - 76 - key_length - signature_length),
                Field("MRoT_External_Public_Key", key_length),
                Field("Sig_TEE_Boot_Key_Area", signature_length),
                ]

        static_info = {
                "Image_ID" : 0x4BB4D21E,
                "Structure_Version": 0x00010000,
                "Structure_Length": 0x100,
                "Signature_Length": 0x40,
                "KeyOwnerID": 0x0,
                "KeyID": 0x0,
                "Key_Alg": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "ECC_Curve_Type": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "Key_Length": 0x40,
                }

        self.__skip_check = skip_check
        super(TPExternalKeyArea, self).__init__("TPExternalKeyArea", fields, static_info)

    def check(self, raw):
        if self.__skip_check:
            log.info("skip check")
            return True

        if super(TPExternalKeyArea, self).check(raw) != True:
            return False

        return True

class GSLCodeInfo(ImageArea):
    def __init__(self, otp, skip_check = False):
        fmc_cfg_data_len = 0 # dummy
        structure_length = 0x200
        signature_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("Signature_Length"),
                FieldLe("GSL_Code_Version_Ext"),
                FieldLe("Mask_GSL_Code_Version_Ext"),
                FieldLe("MSID_Ext"),
                FieldLe("Mask_MSID_Ext"),
                FieldLe("GSL_Code_Area_Addr"),
                FieldLe("GSL_Code_Area_Len"),
                Field("GSL_Code_Area_Hash", 32),
                FieldLe("GSL_Enc_Flag"),
                Field("ProtectionKey_GSL_L1_enc", 16),
                Field("ProtectionKey_GSL_L2_enc", 16),
                Field("IV", 16),
                FieldLe("GSL_Compress_Flag"),
                FieldLe("GSL_Uncompressed_Len"),
                FieldLe("Text_Segment_Size"),
                FieldLe("svb_cfg_enable"),
                FieldLe("fmc_cfg_data_len"),
                Field("fmc_cfg_data", fmc_cfg_data_len),
                FieldRes("Reserved", structure_length - 144 - fmc_cfg_data_len - signature_length * 2),
                Field("Sig_GSL_Info", signature_length),
                Field("Sig_GSL_Info_Ext", signature_length),
                ]

        static_info = {
                "Image_ID" : 0x4BB4D22D,
                "Structure_Version": 0x00010000,
                "Structure_Length": 0x200,
                "Signature_Length": 0x40,
                "GSL_Code_Area_Addr": 0x0,
                }

        self.__dec_enable = otp.tee_dec_enable() if otp.soc_tee_enable() else otp.ree_dec_enable()
        self.__skip_check = skip_check
        super(GSLCodeInfo, self).__init__("GSLCodeInfo", fields, static_info)

    def check(self, raw, area_tool, pub_key):
        if self.__skip_check:
            log.info("skip check")
            return True

        if super(GSLCodeInfo, self).check(raw) != True:
            return False

        if self.__dec_enable:
            if self.area_info()["GSL_Enc_Flag"].val == 0x3C7896E1:
                log.error("{}: Enc_Flag check fail: dec_enable otp has burned, but it is a plaintext image.".format(self.name()))

        field = self.area_info()["Sig_GSL_Info"]
        length = len(field)
        if not area_tool.verify(pub_key, raw[:field.offset], raw[field.offset:field.offset + length], data_name=None):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True

    def parse(self, raw):
        offset = 0
        data_length = 0
        area_info = {}
        fields = self.fields()
        for field in fields:
            field.offset = offset
            field.val = field.extract_val(raw[offset: offset + len(field)])
            area_info[field.name()] = field
            if field.name() == "fmc_cfg_data_len":
                data_length = field.val
                self.__set_data_length(data_length)
            offset += len(field)
        area_info["length"] = offset
        self.set_area_info(area_info)
        return area_info

    def __set_data_length(self, data_length):
        fields = self.fields()
        next(item for item in fields if item.name() == "fmc_cfg_data").width = data_length
        next(item for item in fields if item.name() == "Reserved").width -= data_length

class REEBootKeyArea(ImageArea):
    def __init__(self, otp, skip_check = False):
        structure_length = 0x100
        key_length = 0x40
        signature_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("Signature_Length"),
                FieldLe("KeyOwnerID"),
                FieldLe("KeyID"),
                FieldLe("Key_Alg"),
                FieldLe("ECC_Curve_Type"),
                FieldLe("Key_Length"),
                FieldLe("REE_Key_Version_Ext"),
                FieldLe("Mask_REE_Key_Version_Ext"),
                FieldLe("MSID_Ext"),
                FieldLe("Mask_MSID_Ext"),
                FieldLe("MaintenanceMode"),
                Field("DIE_ID", 16),
                FieldRes("Reserved2", structure_length - 72 - key_length - signature_length),
                Field("REE_Boot_External_Public_Key", key_length),
                Field("Sig_REE_Key_Area", signature_length)
                ]

        static_info = {
                "Image_ID" : 0x4B1E3C1E,
                "Structure_Version": 0x00010000,
                "Structure_Length": 0x100,
                "Signature_Length": 0x40,
                "KeyOwnerID": 0x0,
                "KeyID": 0x0,
                "Key_Alg": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "ECC_Curve_Type": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "Key_Length": 0x40,
                }

        self.__skip_check = skip_check
        super(REEBootKeyArea, self).__init__("REEBootKeyArea", fields, static_info)

    def check(self, raw, area_tool, pub_key):
        if self.__skip_check:
            log.info("skip check")
            return True

        if super(REEBootKeyArea, self).check(raw) != True:
            return False

        field = self.area_info()["Sig_REE_Key_Area"]
        length = len(field)
        if not area_tool.verify(pub_key, raw[:field.offset], raw[field.offset:field.offset + length], data_name=None):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True

class ParamsAreaInfo(ImageArea):
    def __init__(self, otp, skip_check = False):
        signature_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("Signature_Length"),
                FieldLe("Params_Version_Ext"),
                FieldLe("Mask_Params_Version_Ext"),
                FieldLe("MSID_Ext"),
                FieldLe("Mask_MSID_Ext"),
                FieldLe("Params_Area_Addr"),
                FieldLe("Params_Area_Len"),
                FieldLe("Params_Area_Num")
                ] \
                + [Field("Params_Area_Hash" + str(i), 32) for i in range(8)] \
                + [FieldLe("Board_Index_Hash_Table" + str(i), 1) for i in range(8)] \
                + [ FieldRes("Reserved", 76), Field("Sig_Params_Info", signature_length), Field("Sig_Params_Info_Ext", signature_length)]

        static_info = {
                "Image_ID" : 0x4B87A52D,
                "Structure_Version": 0x00010000,
                "Structure_Length": 0x200,
                "Signature_Length": 0x40,
                "Params_Area_Addr": 0x100,
                }
        self.__skip_check = skip_check
        super(ParamsAreaInfo, self).__init__("ParamsAreaInfo", fields, static_info)

    def check(self, raw, area_tool, pub_key):
        if self.__skip_check:
            log.info("skip check")
            return True

        if super(ParamsAreaInfo, self).check(raw) != True:
            return False

        field = self.area_info()["Sig_Params_Info"]
        length = len(field)
        if not area_tool.verify(pub_key, raw[:field.offset], raw[field.offset:field.offset + length], data_name=None):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True

class UBootCodeInfo(ImageArea):
    def __init__(self, otp, skip_check = False):
        structure_length = 0x200
        signature_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("Signature_Length"),
                FieldLe("UBoot_Version_Ext"),
                FieldLe("Mask_UBoot_Version_Ext"),
                FieldLe("MSID_Ext"),
                FieldLe("Mask_MSID_Ext"),
                FieldLe("UBoot_Code_Area_Addr"),
                FieldLe("UBoot_Code_Area_Len"),
                Field("UBoot_Code_Area_Hash", 32),
                FieldLe("UBoot_Enc_Flag"),
                Field("ProtectionKey_UBoot_L1_enc", 16),
                Field("ProtectionKey_UBoot_L2_enc", 16),
                Field("IV", 16),
                FieldLe("UBoot_Compress_Flag"),
                FieldLe("UBoot_Uncompressed_Len"),
                FieldLe("Text_Segment_Size"),
                FieldLe("UBoot_Entry_Point"),
                Field("Reserved", structure_length - 140 - signature_length * 2),
                Field("Sig_UBoot_Info", signature_length),
                Field("Sig_UBoot_Info_Ext", signature_length),
                ]

        static_info = {
                "Image_ID" : 0x4BF01E2D,
                "Structure_Version": 0x00010000,
                "Structure_Length": 0x200,
                "Signature_Length": 0x40,
                "UBoot_Code_Area_Addr": 0x0,
                "UBoot_Compress_Flag": 0x0,
                }

        self.__dec_enable = otp.ree_dec_enable()
        self.__skip_check = skip_check
        super(UBootCodeInfo, self).__init__("UBootCodeInfo", fields, static_info)

    def check(self, raw, area_tool, pub_key):
        if self.__skip_check:
            log.info("skip check")
            return True

        if super(UBootCodeInfo, self).check(raw) != True:
            return False

        if self.__dec_enable:
            if self.area_info()["UBoot_Enc_Flag"].val == 0x3C7896E1:
                log.error("{}: Enc_Flag check fail: dec_enable otp has burned, but it is a plaintext image.".format(self.name()))

        field = self.area_info()["Sig_UBoot_Info"]
        length = len(field)
        if not area_tool.verify(pub_key, raw[:field.offset], raw[field.offset:field.offset + length], data_name=None):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True

class TEEKeyArea(ImageArea):
    def __init__(self, otp, skip_check = False):
        structure_length = 0x100
        key_length = 0x40
        signature_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("Signature_Length"),
                FieldLe("KeyOwnerID"),
                FieldLe("KeyID"),
                FieldLe("Key_Alg"),
                FieldLe("ECC_Curve_Type"),
                FieldLe("KeyLength"),
                FieldLe("RRoT_Key_Version_Ext"),
                FieldLe("Mask_RRoT_Key_Version_Ext"),
                FieldLe("MSID_Ext"),
                FieldLe("Mask_MSID_Ext"),
                FieldLe("MaintenanceMode"),
                Field("DIE_ID", 16),
                FieldLe("Debug_RUSK_Enable"),
                FieldRes("Reserved2", structure_length - 76 - key_length - signature_length),
                Field("RRoT_External_Public_Key", key_length),
                Field("Sig_TEE_Key_Area", signature_length)
                ]

        static_info = {
                "Image_ID" : 0x4BE10F1E,
                "Structure_Version": 0x00010000,
                "Structure_Length": 0x100,
                "Signature_Length": 0x40,
                "KeyOwnerID": 0x0,
                "KeyID": 0x0,
                "Key_Alg": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "ECC_Curve_Type": 0x2A13C812 if otp.scs_alg_sel() == "ECC+SHA256+AES" else 0x2A13C823,
                "KeyLength": 0x40,
                }
        self.__skip_check = skip_check
        super(TEEKeyArea, self).__init__("TEEKeyArea", fields, static_info)

    def check(self, raw, area_tool, pub_key):
        if self.__skip_check:
            log.info("skip check")
            return True

        if super(TEEKeyArea, self).check(raw) != True:
            return False

        field = self.area_info()["Sig_TEE_Key_Area"]
        length = len(field)
        if not area_tool.verify(pub_key, raw[:field.offset], raw[field.offset:field.offset + length], data_name=None):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True

class TEECodeInfo(ImageArea):
    def __init__(self, otp, skip_check = False):
        structure_length = 0x200
        signature_length = 0x40
        fields = [
                FieldLe("Image_ID"),
                FieldLe("Structure_Version"),
                FieldLe("Structure_Length"),
                FieldLe("Signature_Length"),
                FieldLe("TEE_Code_Version_Ext"),
                FieldLe("Mask_TEE_Code_Version_Ext"),
                FieldLe("MSID_Ext"),
                FieldLe("Mask_MSID_Ext"),
                FieldLe("TEE_Code_Area_Addr"),
                FieldLe("TEE_Code_Area_Len"),
                Field("TEE_Code_Area_Hash", 32),
                FieldLe("TEE_Enc_Flag"),
                Field("ProtectionKey_TEE_L1_enc", 16),
                Field("ProtectionKey_TEE_L2_enc", 16),
                Field("IV", 16),
                FieldLe("TEE_Compress_Flag"),
                FieldLe("TEE_Uncompressed_Len"),
                FieldLe("TEE_Text_Segment_Size"),
                FieldLe("TEE_Secure_DDR_Size"),
                FieldLe("ATF_Code_Area_Addr"),
                FieldLe("ATF_Code_Area_Len"),
                Field("ATF_Code_Area_Hash", 32),
                FieldLe("ATF_Compress_Flag"),
                FieldLe("ATF_Uncompressed_Len"),
                FieldLe("ATF_Text_Segment_Size"),
                FieldRes("Reserved", structure_length - 192 - signature_length * 2),
                Field("Sig_TEE_Code_Info", signature_length),
                Field("Sig_TEE_Code_Info_Ext", signature_length)
                ]

        static_info = {
                "Image_ID" : 0x4BE10F2D,
                "Structure_Version": 0x00010000,
                "Structure_Length": 0x200,
                "Signature_Length": 0x40,
                "TEE_Code_Area_Addr": 0x0,
                "TEE_Compress_Flag": 0x0,
                "ATF_Compress_Flag": 0x0,
                }

        self.__dec_enable = otp.tee_dec_enable()
        self.__skip_check = skip_check
        super(TEECodeInfo, self).__init__("TEECodeInfo", fields, static_info)

    def check(self, raw, area_tool, pub_key):
        if self.__skip_check:
            log.info("skip check")
            return True

        if super(TEECodeInfo, self).check(raw) != True:
            return False

        if self.__dec_enable:
            if self.area_info()["TEE_Enc_Flag"].val == 0x3C7896E1:
                log.error("{}: Enc_Flag check fail: dec_enable otp has burned, but it is a plaintext image.".format(self.name()))

        field = self.area_info()["Sig_TEE_Code_Info"]
        length = len(field)
        if not area_tool.verify(pub_key, raw[:field.offset], raw[field.offset:field.offset + length], data_name=None):
            log.error("{}: verify error!!!!".format(self.name()))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True

class CodeArea(ImageArea):
    def __init__(self, length, name, skip_check, dec_en):
        fields = [
                Field("Code", length),
                ]

        self.__skip_check = skip_check
        self.__dec_en = dec_en
        super(CodeArea, self).__init__(name, fields)

    def check(self, raw, area_tool, hash_val, dec_tool):
        if self.__skip_check:
            return True

        if super(CodeArea, self).check(raw) != True:
            return False

        code = self.area_info()["Code"]

        if self.__dec_en:
            code.val = dec_tool.dec(code.val)

        digest = area_tool.digest(data=code.val, data_name="check {} hash val".format(self.name()))

        if digest != hash_val:
            log.error("{}: {} check fail: {} expect {}".format(self.name(), "hash_val", utl.hex2str(digest), utl.hex2str(hash_val)))
            return False

        log.info("{}: verify pass!!!!".format(self.name()))
        return True

    def dump(self):
        log.info("========== {} ==========".format(self.name()))
        for k, v in self.area_info().items():
            if k == "length":
                log.info("area length: {:#x}".format(v))
        log.info("=========== end =========")
        log.info("")

class DecTool:
    def __init__(self, kdf, area_tool):
        self.area_tool = area_tool
        self.keys = kdf.derive_keys()

    def update(self, key_sel, protectionkey_l1_enc, protectionkey_l2_enc, iv):
        self.__key_sel = key_sel
        self.__protectionkey_l1_enc = protectionkey_l1_enc
        self.__protectionkey_l2_enc = protectionkey_l2_enc
        self.__iv = iv

    def dec(self, data):
        effective_rk = self.keys[self.__key_sel]
        protectionkey_l1_dec = self.area_tool.enc_ecb(self.__protectionkey_l1_enc, effective_rk, data_name='L1 dec', decrypt=True)
        protectionkey_l2_dec = self.area_tool.enc_ecb(self.__protectionkey_l2_enc, protectionkey_l1_dec, data_name='L2 dec', decrypt=True)
        return self.area_tool.enc_cbc(data, protectionkey_l2_dec, self.__iv, data_name='code data', decrypt=True)

class OemChecker:
    def __init__(self, otp_file):
        self.__otp = OneTimeProgrammable(otp_file)
        self.__kdf = KdfTool(self.__otp)
        self.__area_tool = AreaTool(self.__otp.scs_alg_sel())
    

    def check(self, boot_image, tee_image=None):
        image_bytes = util.read_file(boot_image)
        dec_tool = None

        offset = 0
        log.info("offset: {:#x}".format(offset))
        ree_flash_root_pub_key_area = RootPublicKeysArea(key_name = "Ree", otp = self.__otp)
        self.__check_flash_root_pub_key_area(ree_flash_root_pub_key_area, image_bytes, self.__otp.ree_hash_flash_root_key())
        offset += ree_flash_root_pub_key_area.area_info()["length"]

        log.info("offset: {:#x}".format(offset))
        tee_flash_root_pub_key_area = RootPublicKeysArea(key_name = "Tee", otp = self.__otp, skip_check=not self.__otp.soc_tee_enable())
        self.__check_flash_root_pub_key_area(tee_flash_root_pub_key_area, image_bytes[offset:], self.__otp.tee_hash_flash_root_key())
        offset += tee_flash_root_pub_key_area.area_info()["length"]

        log.info("offset: {:#x}".format(offset))
        tp_flash_root_pub_key_area = RootPublicKeysArea(key_name = "TP", otp = self.__otp, skip_check = True)
        self.__check_flash_root_pub_key_area(tp_flash_root_pub_key_area, image_bytes[offset:], None)
        offset += tp_flash_root_pub_key_area.area_info()["length"]

        log.info("offset: {:#x}".format(offset))
        cfct_key = KeyAreaOfCFCT(self.__otp)
        self.__check_area(cfct_key, image_bytes[offset:])
        offset += cfct_key.area_info()["length"]

        log.info("offset: {:#x}".format(offset))
        cfct_area = CFCTArea()
        self.__check_area(cfct_area, image_bytes[offset:])

        offset = 0x1000
        log.info("offset: {:#x}".format(offset))

        if self.__otp.soc_tee_enable() and not self.__otp.tee_verify_enable():
                log.error("Enabling tee without verification is an unacceptable error!!!!!!")

        # Considering the scs simulation mode, the actual verification and encryption flag should be based on fields in the mirror.
        ree_verify_enable = ree_flash_root_pub_key_area.area_info()["Verify_Enable_Ext"].val == 0x2A13C812 or self.__otp.ree_verify_enable()
        tee_verify_enable = tee_flash_root_pub_key_area.area_info()["Verify_Enable_Ext"].val == 0x2A13C812 or self.__otp.tee_verify_enable()

        # Considering the network security factor, if tee verification is enabled, then ree verification should also be enabled.
        # However, no check will be performed in this script for now.
        gsl_key_area = GSLKeyArea(self.__otp)

        if not ree_verify_enable and not tee_verify_enable:
            return

        pub_key = tee_flash_root_pub_key_area.area_info()["Flash_Root_Public_Key"].val if self.__otp.soc_tee_enable() \
                else ree_flash_root_pub_key_area.area_info()["Flash_Root_Public_Key"].val

        self.__checkout_key_area(gsl_key_area, image_bytes[offset:], pub_key)
        offset += gsl_key_area.area_info()["length"]

        dummy = TPExternalKeyArea(self.__otp, skip_check = False)
        dummy.parse(image_bytes[offset:])
        offset += dummy.area_info()["length"]

        gsl_info_area = GSLCodeInfo(self.__otp)
        pub_key = gsl_key_area.area_info()["MRoT_External_Public_Key"].val
        self.__checkout_code_info_area(gsl_info_area, image_bytes[offset:], pub_key)
        offset += gsl_info_area.area_info()["length"]

        gsl_code_area_len = gsl_info_area.area_info()["GSL_Code_Area_Len"].val
        hash_val = gsl_info_area.area_info()["GSL_Code_Area_Hash"].val
        gsl_code_area = CodeArea(gsl_code_area_len,
                                 'gsl_code_area',
                                 skip_check=not tee_verify_enable if self.__otp.soc_tee_enable() else not ree_verify_enable
                                 , dec_en=gsl_info_area.area_info()["GSL_Enc_Flag"].val != 0x3C7896E1)

        if gsl_info_area.area_info()["GSL_Enc_Flag"].val != 0x3C7896E1:
            if dec_tool == None:
                dec_tool = DecTool(self.__kdf, self.__area_tool)
            dec_tool.update('sbrk0' if self.__otp.soc_tee_enable() else 'abrk0',
                            gsl_info_area.area_info()["ProtectionKey_GSL_L1_enc"].val,
                            gsl_info_area.area_info()["ProtectionKey_GSL_L2_enc"].val,
                            gsl_info_area.area_info()["IV"].val)

        self.__checkout_code_area(gsl_code_area, image_bytes[offset:], hash_val, dec_tool)
        offset += gsl_code_area.area_info()["length"]

        ree_boot_key_area = REEBootKeyArea(self.__otp, not ree_verify_enable)
        pub_key = ree_flash_root_pub_key_area.area_info()["Flash_Root_Public_Key"].val
        self.__checkout_key_area(ree_boot_key_area, image_bytes[offset:], pub_key)
        offset += ree_boot_key_area.area_info()["length"]

        para_info_area = ParamsAreaInfo(self.__otp, not ree_verify_enable)
        pub_key = ree_boot_key_area.area_info()["REE_Boot_External_Public_Key"].val
        self.__checkout_code_info_area(para_info_area, image_bytes[offset:], pub_key)
        offset += para_info_area.area_info()["length"]

        offset += para_info_area.area_info()["Params_Area_Addr"].val
        para_area_len = para_info_area.area_info()["Params_Area_Len"].val
        params_area_num = para_info_area.area_info()["Params_Area_Num"].val
        for i in range(params_area_num):
            board_num = next(iter(
                filter(lambda ind: para_info_area.area_info()["Board_Index_Hash_Table" + str(ind)].val == i, range(8))), None)
            if board_num == None:
                log.error("index {} in Board_Index_Hash_Table is invalid!!!!Please check the parameter area configuration.".format(i))

            para_area = CodeArea(para_area_len, 'para_area' + '_ind' + str(i) + '_board' + str(board_num), skip_check=not ree_verify_enable, dec_en=False)
            hash_val = para_info_area.area_info()["Params_Area_Hash" + str(board_num)].val
            self.__checkout_code_area(para_area, image_bytes[offset:], hash_val)
            offset += para_area.area_info()["length"]

        uboot_info_area = UBootCodeInfo(self.__otp, not ree_verify_enable)
        pub_key = ree_boot_key_area.area_info()["REE_Boot_External_Public_Key"].val
        self.__checkout_code_info_area(uboot_info_area, image_bytes[offset:], pub_key)
        offset += uboot_info_area.area_info()["length"]

        uboot_area_len = uboot_info_area.area_info()["UBoot_Code_Area_Len"].val
        hash_val = uboot_info_area.area_info()["UBoot_Code_Area_Hash"].val

        if uboot_info_area.area_info()["UBoot_Enc_Flag"].val != 0x3C7896E1:
            if dec_tool == None:
                dec_tool = DecTool(self.__kdf, self.__area_tool)
            dec_tool.update('abrk1',
                            uboot_info_area.area_info()["ProtectionKey_UBoot_L1_enc"].val,
                            uboot_info_area.area_info()["ProtectionKey_UBoot_L2_enc"].val,
                            uboot_info_area.area_info()["IV"].val)

        uboot_area = CodeArea(uboot_area_len, 'uboot_area', skip_check=not ree_verify_enable, dec_en=uboot_info_area.area_info()["UBoot_Enc_Flag"].val != 0x3C7896E1)
        self.__checkout_code_area(uboot_area, image_bytes[offset:], hash_val, dec_tool)
        offset += uboot_area.area_info()["length"]

        if tee_image == None:
            return

        image_bytes = util.read_file(tee_image)
        offset = 0
        tee_os_key_area = TEEKeyArea(self.__otp, skip_check=not tee_verify_enable)
        pub_key = gsl_key_area.area_info()["MRoT_External_Public_Key"].val
        self.__checkout_key_area(tee_os_key_area, image_bytes[offset:], pub_key)
        offset += tee_os_key_area.area_info()["length"]

        tee_code_info = TEECodeInfo(self.__otp, skip_check=not tee_verify_enable)
        pub_key = tee_os_key_area.area_info()["RRoT_External_Public_Key"].val
        self.__checkout_code_info_area(tee_code_info, image_bytes[offset:], pub_key)
        offset += tee_code_info.area_info()["length"]

        atf_code_len = tee_code_info.area_info()["ATF_Uncompressed_Len"].val
        tee_code_len = tee_code_info.area_info()["TEE_Uncompressed_Len"].val
        atf_hash_val = tee_code_info.area_info()["ATF_Code_Area_Hash"].val
        tee_hash_val = tee_code_info.area_info()["TEE_Code_Area_Hash"].val
        atf_area = CodeArea(atf_code_len, 'atf_area', not tee_verify_enable, tee_code_info.area_info()["TEE_Enc_Flag"].val != 0x3C7896E1)
        tee_area = CodeArea(tee_code_len, 'tee_area', not tee_verify_enable, tee_code_info.area_info()["TEE_Enc_Flag"].val != 0x3C7896E1)

        if tee_code_info.area_info()["TEE_Enc_Flag"].val != 0x3C7896E1:
            if dec_tool == None:
                dec_tool = DecTool(self.__kdf, self.__area_tool)
            dec_tool.update('sbrk1',
                            tee_code_info.area_info()["ProtectionKey_TEE_L1_enc"].val,
                            tee_code_info.area_info()["ProtectionKey_TEE_L2_enc"].val,
                            tee_code_info.area_info()["IV"].val)
        self.__checkout_code_area(atf_area, image_bytes[offset:], atf_hash_val, dec_tool)

        offset += atf_area.area_info()["length"]
        self.__checkout_code_area(tee_area, image_bytes[offset:], tee_hash_val, dec_tool)
        offset += atf_area.area_info()["length"]

    def __check_flash_root_pub_key_area(self, flash_root_pub_key_area, image_bytes, hash_val):
        flash_root_pub_key_area.parse(image_bytes)
        # flash_root_pub_key_area.dump()
        log.info("")
        log.info("========== check start ==========")
        flash_root_pub_key_area.check(image_bytes, self.__area_tool, hash_val)
        log.info("========== check end ==========")
        log.info("")

    def __check_area(self, area, image_bytes):
        area.parse(image_bytes)
        # area.dump()
        log.info("")
        log.info("========== check start ==========")
        area.check(image_bytes)
        log.info("========== check end ==========")
        log.info("")

    def __checkout_key_area(self, key_area, image_bytes, pub_key = None):
        key_area.parse(image_bytes)
        # key_area.dump()
        log.info("")
        log.info("========== check start ==========")
        key_area.check(image_bytes, self.__area_tool, pub_key)
        log.info("========== check end ==========")
        log.info("")

    def __checkout_code_info_area(self, info_area, image_bytes, pub_key = None):
        info_area.parse(image_bytes)
        # info_area.dump()
        log.info("")
        log.info("========== check start ==========")
        info_area.check(image_bytes, self.__area_tool, pub_key)
        log.info("========== check end ==========")
        log.info("")

    def __checkout_code_area(self, code_area, image_bytes, hash_val = None, dec_tool = None):
        code_area.parse(image_bytes)
        # code_area.dump()
        log.info("")
        log.info("========== check start ==========")
        code_area.check(image_bytes, self.__area_tool, hash_val, dec_tool)
        log.info("#### code area: {} check pass. ".format(code_area.name()))
        log.info("========== check end ==========")
        log.info("")

def tips(script):
    print('\nUsage: $ python3 %s OPTION' % script)
    print('OPTION:')
    print('  check     Check images using the input OTP values.')
    print('Examples:')
    print('  $ python3 %s check oem/otp_check.json image/oem/boot_image.bin image/oem/tee_image.bin' % script)
    print('')

if __name__ == "__main__":
    if len(sys.argv) == 3:
        otp_file, boot_image = sys.argv[1:]
        tee_image = None
    elif len(sys.argv) == 4:
        otp_file, boot_image, tee_image = sys.argv[1:]
    else:
        tips(sys.argv[0])
        sys.exit(1)
    otp_file, boot_image, tee_image = [sys.argv[1:], None] if len(sys.argv) < 3 else sys.argv[1:4]
    checker = OemChecker(otp_file)
    checker.check(boot_image, tee_image)
    log.info('Done')
