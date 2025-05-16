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
MAINTENANCE_MODE = CfgValue.maintenance_mode
SCS_SIMULATE = CfgValue.SCS_simulate

def init(cfg_file):
    scen = get_scen_from_user(OWNER.THIRD_PARTY)
    cfgtor = Configurator(TP_CFG_ITEM)
    cfgtor.dump_init(cfg_file, scen)
    log.info('Configuration is initialized in \'%s\'' % cfg_file)
    return

def load(cfg_file):
    cfgtor = Configurator(TP_CFG_ITEM)
    cfgtor.load(cfg_file)
    return cfgtor

TP_CFG_ITEM = CfgItem(
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
            Scen(sec_mode=SEC_MODE.SECURE, crypto_alg = ALG.SM, value=ALG.SM)
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
    CfgItem(key='root_key', scens=[Scen(sec_mode=SEC_MODE.SECURE)], sub_items=[
        CfgItem(
            key='TP_root_pub_key',
            typ=CfgValueType.PATH,
            usage='path of TP root pub key in PEM format',
        ),
        CfgItem(
            key='TP_root_private_key',
            typ=CfgValueType.PATH,
            usage='path of TP root private key in PEM format',
        ),
    ]),
    CfgItem(key='TP_info', scens=[Scen(sec_mode=SEC_MODE.SECURE)], sub_items=[
        CfgItem(
            key='TP_msid_ext',
            typ=CfgValueType.UINT32,
            usage='defined by TP',
            scens=[Scen(value='0x00000000')],
            spec=Spec(n_bytes=4)
        ),
        CfgItem(
            key='mask_TP_msid_ext',
            typ=CfgValueType.UINT32,
            usage='mask of the TP_MSID',
            scens=[Scen(value='0x00000000')],
            spec=Spec(n_bytes=4)
        ),
    ]),
    CfgItem(key='GSL', scens=[Scen(sec_mode=SEC_MODE.SECURE)], sub_items=[
        CfgItem(key='TP_key_area', sub_items=[
            CfgItem(
                key='TP_key_version_ext',
                typ=CfgValueType.UINT32,
                usage='the number of TP_external_public_key version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='mask_TP_key_version_ext',
                typ=CfgValueType.UINT32,
                usage='mask of the TP_external_public_key version',
                scens=[Scen(value='0x00000000')],
                spec=Spec(n_bytes=4)
            ),
            CfgItem(
                key='TP_external_public_key',
                typ=CfgValueType.PATH,
                usage='path of the tp external public key in PEM format',
                scens=[Scen(sec_mode=SEC_MODE.SECURE)],
            ),
            CfgItem(
                key='TP_external_private_key',
                typ=CfgValueType.PATH,
                usage='path of the tp external private key in PEM format',
                scens=[Scen(sec_mode=SEC_MODE.SECURE)],
            ),
        ]),
    ]),
    CfgItem(key='maintenance_mode', scens=[Scen(sec_mode=SEC_MODE.SECURE)], sub_items=[
        CfgItem(
            key='TP_key_maintenance_mode',
            typ=CfgValueType.UINT32,
            usage='Indicate whether TP enable the maintenace mode;0x3C7896E1->enable,others->close',
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
                key='TP_verify_simulate',
                typ=CfgValueType.UINT32,
                usage='Indicate whether the TP verification is enabled;enable value:0x2A13C812, disable value:0:',
                scens=[Scen(sec_mode=SEC_MODE.SECURE, value=SCS_SIMULATE.DISABLE)],
        ),
    ]),
]
)
