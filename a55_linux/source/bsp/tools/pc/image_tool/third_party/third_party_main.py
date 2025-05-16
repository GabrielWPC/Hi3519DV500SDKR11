import os
import sys
import shutil

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/..' % SCRIPT_DIR)

sys.path.append(WORK_DIR)
sys.path.append('%s/third_party/tool/' % WORK_DIR)

import common.logger as log

def init_config(cfg_file):
    import third_party_config
    third_party_config.init(cfg_file)

def build(cfg_file):
    from third_party_build import ThirdPartyAreaBuilder, ThirdPartyDoubleSigner
    import third_party_config
    from common.otp import OemOtpBurnCmd

    # get configuration and scenario
    cfgtor = third_party_config.load(cfg_file)
    scen = cfgtor.scenario()
    scen.log()
    if scen.is_sec_boot_enable() == False:
        log.info("Non-Secure mode,no need build tp image")
        log.info("Done.")
        return

    # determine the paths of input and ouput file
    tmp_dir = 'third_party/tmp'
    out_dir = 'image/third_party'


    tp_root_pub_key_area_file           = tmp_dir + '/third_party_root_public_key_area.bin'
    tp_root_pub_key_area_chksum_file    = tmp_dir + '/third_party_root_public_key_area_checksum.txt'
    tp_key_area_file                    = tmp_dir + '/gsl_third_party_key_area.bin'

    if not os.path.exists(tmp_dir):
        os.makedirs(tmp_dir)

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    # build third party areas
    tp_ab = ThirdPartyAreaBuilder(cfgtor)
    tp_ab.build_tp_flash_root_pub_key_area(tp_root_pub_key_area_file, tp_root_pub_key_area_chksum_file)
    tp_ab.build_tp_key_area(tp_key_area_file)

    #otp debug
    if scen.is_sec_boot_enable():
        otp_sub_cmd_dir = tmp_dir + '/otp_cmd/'
        oem_otp=OemOtpBurnCmd(scen, otp_sub_cmd_dir)
        oem_otp.build_otp_cmd_tp(tp_root_pub_key_area_chksum_file)

    tp_ds = ThirdPartyDoubleSigner(cfgtor)
    tp_ds.sign_boot_image(
        'image/oem/boot_image.bin',
        tp_root_pub_key_area_file,
        tp_key_area_file,
        out_file=(out_dir + '/boot_image.bin')
    )
    if scen.is_tee_enbale():
        tp_ds.sign_tee_image(
            'image/oem/tee_image.bin',
            out_file=(out_dir + '/tee_image.bin')
        )

    log.info("Done.")
    return

def clean():
    import common.clean as cln
    cln.clean_output('third_party')

def tips():
    print('\nUsage: $ python3 thidr_party/thidr_party_main.py OPTION' )
    print('OPTION:')
    print('  gencfg    Genearte a configuration file.')
    print('  build     Build images based on the input configuartion file.')
    print('Examples:')
    print('  $ python3 thidr_party/thidr_party_main.py gencfg thidr_party_config.json' )
    print('  $ python3 thidr_party/thidr_party_main.py build thidr_party_config.json' )
    print('')

def main(argv):
    if len(argv) < 2:
        tips()
        sys.exit(1)

    if argv[1] == 'gencfg':
        init_config(argv[2])
    elif argv[1] == 'build':
        build(argv[2])
    else:
        tips()
        sys.exit(1)
    return

if __name__ == "__main__":
    os.chdir(WORK_DIR)
    main(sys.argv)
    sys.exit(0)
