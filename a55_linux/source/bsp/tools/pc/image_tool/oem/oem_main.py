import os
import sys
import shutil

SCRIPT_DIR = os.path.dirname(__file__)
SCRIPT_DIR = SCRIPT_DIR if len(SCRIPT_DIR) != 0 else '.'
WORK_DIR = os.path.abspath('%s/..' % SCRIPT_DIR)
sys.path.append(WORK_DIR)
sys.path.append('%s/oem/tool/' % WORK_DIR)

def init_config(cfg_file):
    import oem_config
    oem_config.init(cfg_file)

def build(cfg_file):
    from oem_build import OemAreaBuilder, OemImageBuilder
    from common.otp import OemOtpBurnCmd
    import oem_config


    # get configuration and scenario
    cfgtor = oem_config.load(cfg_file)
    scen = cfgtor.scenario()
    scen.log()

    # determine the paths of input and ouput file
    out_dir = 'image/oem'
    tmp_dir = 'oem/tmp'

    tee_path = {'vendor': 'vendor', 'oem': tmp_dir}
    boot_image_file = '%s/boot_image.bin' % out_dir
    tee_image_file  = '%s/tee_image.bin' % out_dir

    oem_ree_root_pub_key_area_chksum_file   ='%s/oem_ree_root_public_key_area_checksum.txt' % tmp_dir
    oem_tee_root_pub_key_area_chksum_file   ='%s/oem_tee_root_public_key_area_checksum.txt' % tmp_dir
    oem_ib = OemImageBuilder(cfgtor)
    oem_ib.ree_flash_root_pub_key_area_file    =  '%s/ree_root_pub_key_area.bin' % tmp_dir
    oem_ib.tee_flash_root_pub_key_area_file    =  '%s/tee_root_pub_key_area.bin' % tmp_dir
    oem_ib.cfct_area_file                =  '%s/cfct_area.bin' % tmp_dir
    oem_ib.gsl_key_area_file             =  '%s/gsl_key_area.bin' % tmp_dir
    oem_ib.gsl_code_and_info_area_file       =  '%s/gsl_code_and_info_area.bin' % tmp_dir
    oem_ib.ree_boot_key_area_file        =  '%s/ree_boot_key_area.bin' % tmp_dir
    oem_ib.boot_params_and_info_area_file    =  '%s/boot_params_and_info_area.bin' % tmp_dir
    oem_ib.ree_boot_code_and_info_area_file  =  '%s/ree_boot_code_and_info_area.bin' % tmp_dir
    oem_ib.tee_os_key_area_file             =  '%s/teeos_key_area.bin' % tmp_dir
    oem_ib.teeos_atf_code_and_info_area_file       =  '%s/teeos_atf_code_and_info_area.bin' % tmp_dir


    # build sub area image
    if not os.path.exists(tmp_dir):
        os.mkdir(tmp_dir)
    else:
        shutil.rmtree(tmp_dir)
        os.mkdir(tmp_dir)
    oem_ab = OemAreaBuilder(cfgtor)
    oem_ab.build_ree_flash_root_pub_key_area(oem_ib.ree_flash_root_pub_key_area_file, oem_ree_root_pub_key_area_chksum_file)
    oem_ab.build_tee_flash_root_pub_key_area(oem_ib.tee_flash_root_pub_key_area_file, oem_tee_root_pub_key_area_chksum_file)

    oem_ab.build_cfct_area(oem_ib.cfct_area_file)

    oem_ab.build_gsl_key_area(oem_ib.gsl_key_area_file)
    oem_ab.build_gsl_code_info_area(oem_ib.gsl_code_and_info_area_file)

    oem_ab.build_ree_boot_key_area(oem_ib.ree_boot_key_area_file)
    oem_ab.build_boot_params_info_area(oem_ib.boot_params_and_info_area_file)
    oem_ab.build_ree_boot_code_info_area(oem_ib.ree_boot_code_and_info_area_file)

    if scen.is_tee_enbale():
        oem_ab.build_teeos_key_area(oem_ib.tee_os_key_area_file)
        oem_ab.build_teeos_atf_code_info_area(oem_ib.teeos_atf_code_and_info_area_file)

    #otp debug
    if scen.is_sec_boot_enable():
        otp_sub_cmd_dir = tmp_dir + '/otp_cmd/'
        oem_otp=OemOtpBurnCmd(scen, otp_sub_cmd_dir)
        oem_otp.build_otp_cmd(oem_ree_root_pub_key_area_chksum_file, oem_tee_root_pub_key_area_chksum_file)

    # build boot_image and tee_image
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)
    else:
        shutil.rmtree(out_dir)
        os.mkdir(out_dir)
    oem_ib.build_images(boot_image_file, tee_image_file)
    print('Done.')
    return

def clean():
    import common.clean as cln
    cln.clean_output('oem')

def tips(script):
    print('\nUsage: $ python3 %s OPTION' % script)
    print('OPTION:')
    print('  gencfg    Genearte a configuration file.')
    print('  build     Build images based on the input configuartion file.')
    print('  check     Check images using the input OTP values.')
    print('Examples:')
    print('  $ python3 %s gencfg oem_config.json' % script)
    print('  $ python3 %s build oem_config.json' % script)
    print('  $ python3 %s check oem/otp_check.json image/oem/boot_image.bin image/oem/tee_image.bin' % script)
    print('')

def check(otp_file, boot_image, tee_image=None):
    from oem_check import OemChecker
    checker = OemChecker(otp_file)
    checker.check(boot_image, tee_image)
    print('Done')
    return

def main(argv):
    if len(argv) > 2 and argv[1] == 'gencfg':
        init_config(cfg_file=argv[2])
    elif len(argv) > 2 and argv[1] == 'build':
        build(cfg_file=argv[2])
    elif len(argv) > 3 and argv[1] == 'check':
        tee_image = None
        if len(argv) > 4:
            tee_image = argv[4]
        check(otp_file=argv[2], boot_image=argv[3], tee_image=tee_image)
    else:
        tips(argv[0])
        sys.exit(1)
    return

if __name__ == "__main__":
    os.chdir(WORK_DIR)
    main(sys.argv)
    sys.exit(0)

