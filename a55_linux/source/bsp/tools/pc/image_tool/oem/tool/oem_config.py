import os
import sys
from collections import OrderedDict

SCRIPT_DIR = os.path.dirname(__file__)
WORK_DIR = os.path.abspath('%s/../..' % SCRIPT_DIR)
sys.path.append(WORK_DIR)

from common.config import CfgItem, Spec, Configurator, CfgValueType, CfgValue
from common.config import Scenario as Scen
from common.config import get_scen_from_user
import common.logger as log

SEC_MODE=CfgValue.SecMode
ALG = CfgValue.CryptoAlg
START_FLOW=CfgValue.start_Flow
OWNER = CfgValue.Owner
ENC_FLAG = CfgValue.EncFlag
CFCT_ENC_FLAG = CfgValue.CFCTEncFlag
MAINTENANCE_MODE = CfgValue.maintenance_mode
SCS_SIMULATE = CfgValue.SCS_simulate

def init(cfg_file):
    scen = get_scen_from_user(OWNER.OEM)
    cfgtor = Configurator(OEM_CFG_ITEM)
    cfgtor.dump_init(cfg_file, scen)
    log.info('Configuration is initialized in \'%s\'' % cfg_file)
    return

def load(cfg_file):
    cfgtor = Configurator(OEM_CFG_ITEM)
    cfgtor.load(cfg_file)
    return cfgtor

OEM_CFG_ITEM = CfgItem(
sub_items= [
    CfgItem(
        key='security_mode',
        typ=CfgValueType.STRING,
        usage='%s or %s' % (SEC_MODE.NON_SECURE, SEC_MODE.SECURE),
        scens=[
            Scen(sec_mode=SEC_MODE.NON_SECURE, value=SEC_MODE.NON_SECURE),
            Scen(sec_mode=SEC_MODE.SECURE, value=SEC_MODE.SECURE),
        ],
        spec=Spec(options=[SEC_MODE.NON_SECURE, SEC_MODE.SECURE])
    ),
    CfgItem(
        key='algorithm',
        typ=CfgValueType.STRING,
        usage='ECC+SHA256+AES: %s or SM2+SM3+SM4: %s' % (ALG.ECC, ALG.SM),
        scens=[
            Scen(sec_mode=SEC_MODE.SECURE, crypto_alg = ALG.ECC, value=ALG.ECC),
            Scen(sec_mode=SEC_MODE.SECURE, crypto_alg = ALG.SM, value=ALG.SM),
            Scen(sec_mode=SEC_MODE.NON_SECURE, crypto_alg = ALG.ECC, value=ALG.ECC),
            Scen(sec_mode=SEC_MODE.NON_SECURE, crypto_alg = ALG.SM, value=ALG.SM)
        ],
        spec=Spec(options=[ALG.ECC, ALG.SM])
    ),
    CfgItem(
        key='start_Flow',
        typ=CfgValueType.STRING,
        usage='%s or %s' % (START_FLOW.NON_TEE, START_FLOW.TEE),
        scens=[
            Scen(sec_mode=SEC_MODE.SECURE, start_flow=START_FLOW.NON_TEE, value=START_FLOW.NON_TEE),
            Scen(sec_mode=SEC_MODE.SECURE, start_flow=START_FLOW.TEE, value=START_FLOW.TEE),
        ],
        spec=Spec(options=[START_FLOW.NON_TEE, START_FLOW.TEE])
    ),
    CfgItem(
         key='TEE_owner',
         typ=CfgValueType.STRING,
         usage='must be %s' % (OWNER.OEM),
         scens=[
             Scen(sec_mode=SEC_MODE.SECURE, start_flow=START_FLOW.TEE, tee_owner=OWNER.OEM, value=OWNER.OEM),
         ],
         spec=Spec(options=[OWNER.OEM])
    ),
    CfgItem(key='OEM_info', scens=[Scen(sec_mode=SEC_MODE.SECURE)], sub_items=[
        CfgItem(
            key='oem_msid_ext',
            typ=CfgValueType.UINT32,
            usage='defined by OEM',
            scens=[Scen(value='0x00000000')],
            spec=Spec(n_bytes=4)
        ),
        CfgItem(
            key='mask_oem_msid_ext',
            typ=CfgValueType.UINT32,
            usage='mask of the OEM_MSID',
            scens=[Scen(value='0x00000000')],
            spec=Spec(n_bytes=4)
        ),
    ]),
    CfgItem(key='root_key', scens=[Scen(sec_mode=SEC_MODE.SECURE)], sub_items=[
        CfgItem(
            key='REE_root_pub_key',
            typ=CfgValueType.PATH,
            usage='path of REE root pub key in PEM format',
            scens=[Scen(sec_mode=SEC_MODE.SECURE)]
        ),
        CfgItem(
            key='REE_root_private_key',
            typ=CfgValueType.PATH,
            usage='path of REE root private key in PEM format',
            scens=[Scen(sec_mode=SEC_MODE.SECURE)]
        ),
        CfgItem(
            key='TEE_root_pub_key',
            typ=CfgValueType.PATH,
            usage='path of REE root pub key in PEM format',
            scens=[Scen(sec_mode=SEC_MODE.SECURE, start_flow=START_FLOW.TEE)]
        ),
        CfgItem(
            key='TEE_root_private_key',
            typ=CfgValueType.PATH,
            usage='path of REE root private key in PEM format',
            scens=[Scen(sec_mode=SEC_MODE.SECURE, start_flow=START_FLOW.TEE)]
        ),
    ]),
    CfgItem(key='CFCT', sub_items=[
        CfgItem(
            key='CFCT_protection_key_l1',
            typ=CfgValueType.HEX,
            usage='an 128-bit hexadecimal value with leading \'0x\'',
            scens=[Scen(cfct_data_enc=CFCT_ENC_FLAG.ENC)],
            spec=Spec(n_bits=128)
        ),
        CfgItem(
            key='CFCT_protection_key_l2',
            typ=CfgValueType.HEX,
            usage='an 128-bit hexadecimal value with leading \'0x\'',
            scens=[Scen(cfct_data_enc=CFCT_ENC_FLAG.ENC)],
            spec=Spec(n_bits=128)
        ),
        CfgItem(
            key='CFCT_iv',
            typ=CfgValueType.HEX,
            usage='an 128-bit hexadecimal value with leading \'0x\'',
            scens=[Scen(cfct_data_enc=CFCT_ENC_FLAG.ENC)],
            spec=Spec(n_bits=128)
        ),
        CfgItem(
            key='CFCT_kdf_abrk0_file',
            typ=CfgValueType.IN_FILE,
            usage='path of an 128-bit hexadecimal binary file',
            scens=[Scen(cfct_data_enc=CFCT_ENC_FLAG.ENC)],
        ),
        CfgItem(
            key='CFCT_data_enc_flag',
            typ=CfgValueType.HEX,
            usage='%s means encrypted' % CFCT_ENC_FLAG.ENC,
            scens=[
                Scen(cfct_data_enc=CFCT_ENC_FLAG.ENC, value=CFCT_ENC_FLAG.ENC),
                Scen(cfct_data_enc=CFCT_ENC_FLAG.NON_ENC, value=CFCT_ENC_FLAG.NON_ENC)
            ],
            spec=Spec(n_bytes=1)
        ),
    ]),
    CfgItem(key='GSL', scens=[Scen(tee_owner=OWNER.OEM)], sub_items=[
        CfgItem(key='GSL_key_area', sub_items=[
            CfgItem(
                key='GSL_key_version_ext',
                typ=CfgValueType.UINT32,
                usage='the number of MRoT_External_Public_Key version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='mask_GSL_key_version_ext',
                typ=CfgValueType.UINT32,
                usage='mask of the MRoT_External_Public_Key version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='GSL_external_public_key',
                typ=CfgValueType.PATH,
                usage='path of the gsl external public key in PEM format',
                scens=[Scen(sec_mode=SEC_MODE.SECURE)],
            ),
            CfgItem(
                key='GSL_external_private_key',
                typ=CfgValueType.PATH,
                usage='path of the gsl external private key in PEM format',
                scens=[Scen(sec_mode=SEC_MODE.SECURE)],
            ),
	    ]),
        CfgItem(key='GSL_Info_area', sub_items=[
            CfgItem(
                key='GSL_code_version_ext',
                typ=CfgValueType.UINT32,
                usage='the number of GSL code version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='mask_GSL_code_version_ext',
                typ=CfgValueType.UINT32,
                usage='mask of the GSL code version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='GSL_protection_key_l1',
                typ=CfgValueType.HEX,
                usage='an 128-bit hexadecimal value with leading \'0x\'',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, gsl_enc=ENC_FLAG.ENC)],
                spec=Spec(n_bits=128)
            ),
            CfgItem(
                key='GSL_protection_key_l2',
                typ=CfgValueType.HEX,
                usage='an 128-bit hexadecimal value with leading \'0x\'',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, gsl_enc=ENC_FLAG.ENC)],
                spec=Spec(n_bits=128)
            ),
            CfgItem(
                key='GSL_iv',
                typ=CfgValueType.HEX,
                usage='an 128-bit hexadecimal value with leading \'0x\'',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, gsl_enc=ENC_FLAG.ENC)],
                spec=Spec(n_bits=128)
            ),
            CfgItem(
                key='GSL_kdf_sbrk0_file',
                typ=CfgValueType.IN_FILE,
                usage='path of an 128-bit hexadecimal binary file',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, start_flow=START_FLOW.TEE, gsl_enc=ENC_FLAG.ENC)],
            ),
            CfgItem(
                key='GSL_kdf_abrk0_file',
                typ=CfgValueType.IN_FILE,
                usage='path of an 128-bit hexadecimal binary file',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, start_flow=START_FLOW.NON_TEE, gsl_enc=ENC_FLAG.ENC)],
            ),
            CfgItem(
                key='GSL_code_enc_flag',
                typ=CfgValueType.UINT32,
                usage='%s means non-encrypted' % ENC_FLAG.NON_ENC,
                scens=[
                    Scen(gsl_enc=ENC_FLAG.ENC, value=ENC_FLAG.ENC),
                    Scen(gsl_enc=ENC_FLAG.NON_ENC, value=ENC_FLAG.NON_ENC)
                ],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='GSL_code_FlashReadClk',
                typ=CfgValueType.HEX,
                usage='zero means fmc clk default to 12Mhz, others means fmc clk will be improved to 75Mhz',
                scens=[Scen(value='0x0')]
            ),
        ]),
        CfgItem(key='GSL_code_area', sub_items=[
            CfgItem(
                key='GSL_code_file',
                typ=CfgValueType.IN_FILE,
                usage='path of the binary GSL code '
            ),
        ]),
    ]),
    CfgItem(key='REE_boot', sub_items=[
	    CfgItem(key='uboot_key_area', sub_items=[
            CfgItem(
                key='uboot_key_version_ext',
                typ=CfgValueType.UINT32,
                usage='the number of uboot_external_public_key version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='mask_uboot_key_version_ext',
                typ=CfgValueType.UINT32,
                usage='mask of the uboot_external_public_key version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='uboot_external_public_key',
                typ=CfgValueType.PATH,
                usage='path of the REE boot external public key in PEM format',
                scens=[Scen(sec_mode=SEC_MODE.SECURE)],
            ),
            CfgItem(
                key='uboot_external_private_key',
                typ=CfgValueType.PATH,
                usage='path of the REE boot external private key in PEM format',
                scens=[Scen(sec_mode=SEC_MODE.SECURE)],
            ),
        ]),
        CfgItem(key='boot_params_Info', sub_items=[
            CfgItem(
                key='boot_param_version_ext',
                typ=CfgValueType.UINT32,
                usage='the number of params area version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='mask_boot_param_version_ext',
                typ=CfgValueType.UINT32,
                usage='mask of the params area version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='single_param_size',
                typ=CfgValueType.UINT32,
                usage='length of each params area, should align with 512bytes for emmc',
                scens=[Scen(value='0x3000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='boot_param_total_num',
                typ=CfgValueType.UINT32,
                usage='total num of the Params, the max num is 0x8',
                scens=[Scen(value='0x8')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='boot_param_file0',
                typ=CfgValueType.IN_FILE_OPTIONAL,
                usage='path of param0 file used for building muty boot table image'
            ),
            CfgItem(
                key='boot_param_file1',
                typ=CfgValueType.IN_FILE_OPTIONAL,
                usage='path of param1 file used for building muty boot table image'
            ),
            CfgItem(
                key='boot_param_file2',
                typ=CfgValueType.IN_FILE_OPTIONAL,
                usage='path of param2 file used for building muty boot table image'
            ),
            CfgItem(
                key='boot_param_file3',
                typ=CfgValueType.IN_FILE_OPTIONAL,
                usage='path of param3 file used for building muty boot table image'
            ),
            CfgItem(
                key='boot_param_file4',
                typ=CfgValueType.IN_FILE_OPTIONAL,
                usage='path of param4 file used for building muty boot table image'
            ),
            CfgItem(
                key='boot_param_file5',
                typ=CfgValueType.IN_FILE_OPTIONAL,
                usage='path of param5 file used for building muty boot table image'
            ),
            CfgItem(
                key='boot_param_file6',
                typ=CfgValueType.IN_FILE_OPTIONAL,
                usage='path of param6 file used for building muty boot table image'
            ),
            CfgItem(
                key='boot_param_file7',
                typ=CfgValueType.IN_FILE_OPTIONAL,
                usage='path of param7 file used for building muty boot table image'
            ),
        ]),
	    CfgItem(key='uboot_info_area', sub_items=[
            CfgItem(
                key='uboot_version_ext',
                typ=CfgValueType.UINT32,
                usage='the number of ree uboot code version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='mask_uboot_version_ext',
                typ=CfgValueType.UINT32,
                usage='mask of the ree boot code version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='uboot_protection_key_l1',
                typ=CfgValueType.HEX,
                usage='an 128-bit binary key file path',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, boot_enc=ENC_FLAG.ENC)],
                spec=Spec(n_bits=128)
            ),
            CfgItem(
                key='uboot_protection_key_l2',
                typ=CfgValueType.HEX,
                usage='a 128-bit binary key file path',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, boot_enc=ENC_FLAG.ENC)],
                spec=Spec(n_bits=128)
            ),
            CfgItem(
                key='uboot_iv',
                typ=CfgValueType.HEX,
                usage='an 128-bit hexadecimal value with leading \'0x\'',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, boot_enc=ENC_FLAG.ENC)],
                spec=Spec(n_bits=128)
            ),
            CfgItem(
                key='uboot_kdf_abrk1_file',
                typ=CfgValueType.IN_FILE,
                usage='path of an 128-bit hexadecimal binary file',
                scens=[Scen(gsl_enc=CFCT_ENC_FLAG.ENC)],
            ),
            CfgItem(
                key='uboot_code_enc_flag',
                typ=CfgValueType.UINT32,
                usage='%s means non-encrypted' % ENC_FLAG.NON_ENC,
                scens=[
                    Scen(boot_enc=ENC_FLAG.ENC, value=ENC_FLAG.ENC),
                    Scen(boot_enc=ENC_FLAG.NON_ENC, value=ENC_FLAG.NON_ENC)
                ],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='uboot_entry_point',
                typ=CfgValueType.UINT32,
                usage='address of the Boot startup in memory',
                scens=[Scen(value='0x48700000')],
                spec=Spec(n_bits=32)
            ),
        ]),

        CfgItem(key='uboot_area', sub_items=[
            CfgItem(
                key='uboot_code_file',
                typ=CfgValueType.IN_FILE,
                usage='path of the binary U-Boot code'
            ),
        ])
    ]),
    CfgItem(key='TEE', scens=[Scen(sec_mode=SEC_MODE.SECURE, start_flow=START_FLOW.TEE, tee_owner=OWNER.OEM)], sub_items=[
         CfgItem(key='TEEOS_key_area', sub_items=[
            CfgItem(
                key='TEEOS_key_version_ext',
                typ=CfgValueType.UINT32,
                usage='the number of TEEOS_external_public_key version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='mask_TEEOS_key_version_ext',
                typ=CfgValueType.UINT32,
                usage='mask of the TEEOS_external_public_key version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='TEEOS_external_public_key',
                typ=CfgValueType.PATH,
                usage='path of the TEEOS_external_public_key in PEM format',
                scens=[Scen(sec_mode=SEC_MODE.SECURE)],
            ),
            CfgItem(
                key='TEEOS_external_private_key',
                typ=CfgValueType.PATH,
                usage='path of the TEEOS_external_private_key in PEM format',
                scens=[Scen(sec_mode=SEC_MODE.SECURE)],
            ),
        ]),
	    CfgItem(key='TEEOS_info_area', sub_items=[
            CfgItem(
                key='TEEOS_version_ext',
                typ=CfgValueType.UINT32,
                usage='the number of tee code version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='mask_TEEOS_version_ext',
                typ=CfgValueType.UINT32,
                usage='mask of the tee code version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='TEEOS_ATF_protection_key_l1',
                typ=CfgValueType.HEX,
                usage='an 128-bit binary key file path',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, atf_tee_enc=ENC_FLAG.ENC)],
                spec=Spec(n_bits=128)
            ),
            CfgItem(
                key='TEEOS_ATF_protection_key_l2',
                typ=CfgValueType.HEX,
                usage='a 128-bit binary key file path',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, atf_tee_enc=ENC_FLAG.ENC)],
                spec=Spec(n_bits=128)
            ),
            CfgItem(
                key='TEEOS_ATF_iv',
                typ=CfgValueType.HEX,
                usage='an 128-bit hexadecimal value with leading \'0x\'',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, atf_tee_enc=ENC_FLAG.ENC)],
                spec=Spec(n_bits=128)
            ),
            CfgItem(
                key='TEEOS_ATF_kdf_sbrk1_file',
                typ=CfgValueType.IN_FILE,
                usage='path of an 128-bit hexadecimal binary file',
                scens=[Scen(atf_tee_enc=ENC_FLAG.ENC)],
            ),
            CfgItem(
                key='TEEOS_ATF_enc_flag',
                typ=CfgValueType.UINT32,
                usage='%s means non-encrypted' % ENC_FLAG.NON_ENC,
                scens=[
                    Scen(atf_tee_enc=ENC_FLAG.ENC, value=ENC_FLAG.ENC),
                    Scen(atf_tee_enc=ENC_FLAG.NON_ENC, value=ENC_FLAG.NON_ENC)
                ],
                spec=Spec(n_bytes=4)
            ),
        ]),
        CfgItem(key='ATF_code_area', sub_items=[
            CfgItem(
                key='ATF_code_file',
                typ=CfgValueType.IN_FILE,
                usage='path of the binary ATF code'
            ),
        ]),
        CfgItem(key='TEEOS_code_area', sub_items=[
            CfgItem(
                key='TEEOS_code_file',
                typ=CfgValueType.IN_FILE,
                usage='path of the binary TEE code'
            ),
        ])
    ]),
    CfgItem(key='maintenance_mode', scens=[Scen(sec_mode=SEC_MODE.SECURE)], sub_items=[
        CfgItem(
            key='GSL_key_maintenance_mode',
            typ=CfgValueType.UINT32,
            usage='Indicate whether OEM enable the maintenace mode;0x3C7896E1->enable,others->close',
            scens=[Scen(sec_mode=SEC_MODE.SECURE, value=MAINTENANCE_MODE.DISABLE)],
        ),
        CfgItem(
                key='uboot_key_maintenance_mode',
                typ=CfgValueType.UINT32,
                usage='Indicate whether OEM enable the maintenace mode;0x3C7896E1->enable,others->close',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, value=MAINTENANCE_MODE.DISABLE)],
            ),
        CfgItem(
            key='TEEOS_key_maintenance_mode',
            typ=CfgValueType.UINT32,
            usage='Indicate whether OEM enable the maintenace mode;0x3C7896E1->enable,others->close',
            scens=[Scen(sec_mode=SEC_MODE.SECURE, value=MAINTENANCE_MODE.DISABLE)],
        ),
        CfgItem(
            key='DIE_ID',
            typ=CfgValueType.HEX,
            usage='an 128-bit hexadecimal value with leading \'0x\',DIE_ID use when maintenance mode is enable',
            scens=[Scen(sec_mode=SEC_MODE.SECURE, value='0x00000000000000000000000000000000')],
            spec=Spec(n_bits=128)
        ),
    ]),
    CfgItem(key='SCS_simulate', scens=[Scen(sec_mode=SEC_MODE.SECURE)], sub_items=[
        CfgItem(
                key='REE_verify_simulate',
                typ=CfgValueType.UINT32,
                usage='Indicate whether the REE verification is enabled;enable value:0x2A13C812, disable value:0:',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, value=SCS_SIMULATE.DISABLE)],
        ),
        CfgItem(
            key='TEE_verify_simulate',
            typ=CfgValueType.UINT32,
            usage='Indicate whether the TEE verification is enabled;0x2A13C812->enable, others->disable',
            scens=[Scen(sec_mode=SEC_MODE.SECURE, value=SCS_SIMULATE.DISABLE)],
        ),
    ]),
]
)
