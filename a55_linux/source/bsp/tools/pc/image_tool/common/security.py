import os
import sys
from sys import version_info

SCRIPT_DIR = os.path.dirname(__file__)
sys.path.append(SCRIPT_DIR)

import logger as log
import check as chk
import util as utl

def get_rs_in_signed_file(openssl_sign):
    sig_r_s = utl.hex2str(openssl_sign)
    #log.info("%s" %sig_r_s)

    r_head_off = 2 * 2
    r_len = 32 * 2
    s_len = 32 * 2
    r_data_off = 0
    s_head_off = 0
    s_data_off = 0
    if sig_r_s[r_head_off:r_head_off + 2 * 2] == "0221":
        r_data_off = r_head_off + 2 * 3
        s_head_off = r_data_off + r_len
    if sig_r_s[r_head_off:r_head_off + 2 * 2] == "0220":
        r_data_off = r_head_off + 2 * 2
        s_head_off = r_data_off + r_len

    sig_r = sig_r_s[r_data_off:s_head_off]
    #print("sig_r %s r_data_off %d r_len %d\n" %(sig_r, r_data_off, r_len))
    if sig_r_s[s_head_off:s_head_off + 2 * 2] == "0221":
        s_data_off = s_head_off +  2 * 3
    if sig_r_s[s_head_off:s_head_off + 2 * 2] == "0220":
        s_data_off = s_head_off +  2 * 2

    sig_s = sig_r_s[s_data_off:s_data_off + s_len]
    #print("sig_s %s r_data_off %d r_len %d\n" %(sig_s, s_data_off, s_len))

    sig_r_s = sig_r + sig_s
    #log.info("sig_r_s %s\n" %sig_r_s)
    return utl.str2hex(sig_r_s)

def cmd_failure_process(notice, stderr):
    stderr = stderr.decode("utf-8") if version_info.major > 2 else stderr
    stderr = stderr.strip()
    log.error('%s\n----stderr----\n%s\n--------------' % (notice, stderr))
    sys.exit(1)

def gen_rsa_priv_key(priv_key_file):
    cmd = ['openssl', 'genrsa', '-f4', '-out', priv_key_file, '4096']
    stdout, stderr, return_code = utl.run_cmd(cmd)

    if return_code != 0:
        cmd_failure_process('Failed to generate RSA public key', stderr)

    return stdout

def gen_rsa_pub_key(priv_key_file, pub_key_file):
    chk.check_file_exist(priv_key_file, 'RSA Private Key')

    cmd = ['openssl', 'rsa', '-pubout', '-in', priv_key_file,
           '-out', pub_key_file]
    stdout, stderr, return_code = utl.run_cmd(cmd)

    if return_code != 0:
        cmd_failure_process('Failed to generate RSA public key', stderr)

    return stdout

def is_rsa_priv_key(key_file):
    chk.check_file_exist(key_file)
    cmd = 'openssl rsa -check -noout -in %s' % key_file
    stdout, stderr, return_code = utl.run_cmd(cmd)
    return (return_code == 0)

def get_rsa_modulus(key_file):
    chk.check_file_exist(key_file, 'RSA Key')

    options = '-modulus -noout -in %s' % key_file
    if not is_rsa_priv_key(key_file):
        options +=' -pubin'
    cmd = 'openssl rsa %s' % options
    stdout, stderr, return_code = utl.run_cmd(cmd)

    if return_code != 0:
        cmd_failure_process('Failed to get RSA modulus', stderr)

    stdout = stdout.decode("utf-8") if version_info.major > 2 else stdout
    modulus = stdout.replace('Modulus=', '').replace('\n', '')
    modulus = utl.str2hex(modulus)
    return modulus


def get_rsa_exponent(key_file):
    chk.check_file_exist(key_file, 'RSA Key')

    options = '-text -noout -in %s' % key_file
    if not is_rsa_priv_key(key_file):
        options +=' -pubin'
    cmd = 'openssl rsa %s' % options
    stdout, stderr, return_code = utl.run_cmd(cmd)

    if return_code != 0:
        cmd_failure_process('Failed to get RSA exponent', stderr)

    stdout = stdout.decode("utf-8") if version_info.major > 2 else stdout
    exponent = '%08x' % int(stdout.split('Exponent:')[1].split('(')[0][:-1])
    exponent = utl.str2hex(exponent)
    return exponent

def enc_aes_128_ecb(data, key, decrypt=False):
    chk.check_bit_len(key, 128, notice='AES-128 Key')

    mode = '-d' if (decrypt) else '-e'
    cmd = ['openssl', 'enc', '-aes-128-ecb', '-nopad',
           mode, '-K', utl.hex2str(key)]
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('AES-128-ECB: Enc/Dec Error', stderr)

    return stdout

def enc_sm4_ecb(data, key, decrypt=False):
    chk.check_bit_len(key, 128, notice='SM4-ECB Key')

    mode = '-d' if (decrypt) else '-e'
    cmd = ['openssl', 'enc', '-sm4-ecb', '-nopad',
           mode, '-K', utl.hex2str(key)]
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('SM4-ECB: Enc/Dec Error', stderr)

    return stdout

def enc_aes_128_cbc(data, key, iv, decrypt=False):
    chk.check_bit_len(key, 128, notice='AES-128 Key')
    chk.check_bit_len(iv, 128, notice='AES-128 IV')

    mode = '-d' if (decrypt) else '-e'
    cmd = ['openssl', 'enc', '-aes-128-cbc', '-nopad', mode,
           '-K', utl.hex2str(key), '-iv', utl.hex2str(iv)]
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('AES-128-CBC: Enc/Dec Error', stderr)

    return stdout

def enc_aes_256_cbc(data, key, iv, decrypt=False):
    chk.check_bit_len(key, 256, notice='AES-256 Key')
    chk.check_bit_len(iv, 128, notice='AES-256 IV')

    mode = '-d' if (decrypt) else '-e'
    cmd = ['openssl', 'enc', '-aes-256-cbc', '-nopad', mode,
           '-K', utl.hex2str(key), '-iv', utl.hex2str(iv)]
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('AES-256-CBC: Enc/Dec Error', stderr)

    return stdout

def enc_aes_128_ctr(data, key, iv, decrypt=False):
    chk.check_bit_len(key, 128, notice='AES-128 Key')
    chk.check_bit_len(iv, 128, notice='AES-128 IV')

    mode = '-d' if (decrypt) else '-e'
    cmd = ['openssl', 'enc', '-aes-128-ctr', mode,
           '-K', utl.hex2str(key), '-iv', utl.hex2str(iv)]
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('AES-128-CTR: Enc/Dec Error', stderr)

    return stdout

def enc_sm4_cbc(data, key, iv, decrypt=False):
    chk.check_bit_len(key, 128, notice='sm4 Key')
    chk.check_bit_len(iv, 128, notice='sm4 IV')

    mode = '-d' if (decrypt) else '-e'
    cmd = ['openssl', 'enc', '-sm4-cbc', '-nopad', mode,
           '-K', utl.hex2str(key), '-iv', utl.hex2str(iv)]
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('SM4-CBC: Enc/Dec Error', stderr)

    return stdout

def enc_sm4_ctr(data, key, iv, decrypt=False):
    chk.check_bit_len(key, 128, notice='sm4 Key')
    chk.check_bit_len(iv, 128, notice='sm4 IV')

    mode = '-d' if (decrypt) else '-e'
    cmd = ['openssl', 'enc', '-sm4-ctr', '-nopad', mode,
           '-K', utl.hex2str(key), '-iv', utl.hex2str(iv)]
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('SM4-CTR: Enc Error', stderr)

    return stdout

def dgst_sha256(data):
    cmd = ['openssl', 'dgst', '-sha256', '-binary', '-r']
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('SHA256: Digest Error', stderr)

    return stdout

def dgst_sm3(data):
    cmd = ['openssl', 'dgst', '-SM3', '-binary', '-r']
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('SM3: Digest Error', stderr)

    return stdout

def sign_rsa_sha256(data, priv_key_file):
    chk.check_file_exist(priv_key_file, 'RSA Private Key')

    cmd = ['openssl', 'dgst', '-sha256', '-sign', priv_key_file,
           '-sigopt', 'rsa_padding_mode:pss', '-sigopt', 'rsa_pss_saltlen:-1']
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('RSA_SHA256: Signature Error', stderr)

    return stdout

def sign_ecc_sha256(data, priv_key_file):
    chk.check_file_exist(priv_key_file, 'ECC Private Key')

    cmd = ['openssl', 'dgst', '-sha256', '-sign', priv_key_file]
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('ECC_SHA256: Signature Error', stderr)

    return get_rs_in_signed_file(stdout)

def sign_sm2_sm3(data, priv_key_file):
    chk.check_file_exist(priv_key_file, 'SM2 Private Key')

    cmd = ['openssl', 'pkeyutl', '-sign', '-inkey', priv_key_file,
            '-rawin', '-digest', 'sm3', '-pkeyopt', 'distid:1234567812345678']
    stdout, stderr, return_code = utl.run_cmd(cmd, data)

    if return_code != 0:
        cmd_failure_process('SM2_SM3: Signature Error', stderr)

    return get_rs_in_signed_file(stdout)

def verify_rsa_sha256(data, sign, pub_key_file):
    sign_file = 'rsa_sha256_sign.tmp'
    chk.check_file_exist(pub_key_file, 'RSA Public Key')
    utl.write_file(sign_file, sign)

    cmd = 'openssl dgst -sha256'\
        ' -sigopt rsa_padding_mode:pss' \
        ' -sigopt rsa_pss_saltlen:-1'\
        ' -verify %s -signature %s' % (pub_key_file, sign_file)

    stdout, stderr, return_code = utl.run_cmd(cmd, data)
    if return_code == 0:
        os.remove(sign_file)
        return True

    if len(stderr) != 0:
        cmd_failure_process('RSA_SHA256: Verification Error', stderr)

    return False

def verify_ecc_sha256(data, sign, pub_key):
    sign_file = 'sign.tmp'
    pub_key_file = 'pubkey.tmp'

    pub_key_head = bytearray([0x30, 0x5a, 0x30, 0x14, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x09, 0x2b, 0x24, 0x03, 0x03, 0x02, 0x08, 0x01, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04])
    pub_key_der = pub_key_head + pub_key

    r_bytes = sign[:32]
    s_bytes = sign[32:]

    asn1_head = bytearray([0x02, 0x20]) if (type(r_bytes[0]) == str and ord(r_bytes[0]) & 0b10000000 == 0x0) \
        or (type(r_bytes[0]) == int and r_bytes[0] & 0b10000000 == 0x0) else bytearray([0x02, 0x21, 0x00])
    r_der =  asn1_head + r_bytes
    asn1_head = bytearray([0x02, 0x20]) if (type(s_bytes[0]) == str and ord(s_bytes[0]) & 0b10000000 == 0x0) \
        or (type(s_bytes[0]) == int and s_bytes[0] & 0b10000000 == 0x0) else bytearray([0x02, 0x21, 0x00])
    s_der = asn1_head + s_bytes
    sign_der = bytearray([0x30, len(r_der) + len(s_der)]) + r_der + s_der

    utl.write_file(sign_file, sign_der)
    utl.write_file(pub_key_file, pub_key_der)
    cmd = ['openssl', 'dgst', '-sha256', '-verify', pub_key_file,
           '-signature', sign_file]
    stdout, stderr, return_code = utl.run_cmd(cmd, data)
    os.remove(sign_file)
    os.remove(pub_key_file)

    if return_code == 0:
        return True

    if len(stderr) != 0:
        cmd_failure_process('ECC_SHA256: Verification Error', stderr)

    return False

def verify_sm2_sm3(data, sign, pub_key):
    sign_file = 'sign.tmp'
    pub_key_file = 'pubkey.tmp'

    pub_key_head = bytearray([0x30, 0x5a, 0x30, 0x14, 0x06, 0x08, 0x2a, 0x81, 0x1c, 0xcf, 0x55, 0x01, 0x82, 0x2d, 0x06, 0x08, 0x2a, 0x81, 0x1c, 0xcf, 0x55, 0x01, 0x82, 0x2d, 0x03, 0x42, 0x00, 0x04])

    pub_key_der = pub_key_head + pub_key

    r_bytes = sign[:32]
    s_bytes = sign[32:]

    asn1_head = bytearray([0x02, 0x20]) if r_bytes[0] & 0b10000000 == 0x0 else bytearray([0x02, 0x21, 0x00])
    r_der =  asn1_head + r_bytes
    asn1_head = bytearray([0x02, 0x20]) if s_bytes[0] & 0b10000000 == 0x0 else bytearray([0x02, 0x21, 0x00])
    s_der = asn1_head + s_bytes
    sign_der = bytearray([0x30, len(r_der) + len(s_der)]) + r_der + s_der

    utl.write_file(sign_file, sign_der)
    utl.write_file(pub_key_file, pub_key_der)
    # utl.write_file('data.tmp', data)

    cmd = ['openssl', 'pkeyutl', '-verify', '-pubin', '-inkey', pub_key_file,
            '-rawin', '-digest', 'sm3', '-sigfile', sign_file,
           '-pkeyopt', 'distid:1234567812345678']

    stdout, stderr, return_code = utl.run_cmd(cmd, data)
    os.remove(sign_file)
    os.remove(pub_key_file)

    if return_code == 0:
        return True

    if len(stderr) != 0:
        cmd_failure_process('Verification Error', stderr)

    return False

from Crypto.PublicKey import RSA
from Crypto.Util import number as crypto_num
def export_rsa_pub_key(modulus, exponent, out_pem):
    n = crypto_num.bytes_to_long(modulus)
    e = crypto_num.bytes_to_long(exponent)
    pub_key = RSA.construct((n, e))
    utl.write_file(out_pem, pub_key.exportKey('PEM'))

# openssl ec -in ec_bp256_pubkey.pem -pubin -text
def get_publickey_from_pem_file(public_Key_file):
    chk.check_file_exist(public_Key_file, 'ecc sha256 public Key')

    cmd = ['openssl', 'ec', '-in', public_Key_file, '-pubin', '-text']
    stdout, stderr, return_code = utl.run_cmd(cmd, None)

    if return_code != 0:
        cmd_failure_process('ECC_SHA256: Signature Error', stderr)

    utl.write_file("publickey_text.txt", stdout, 'publickey_text')
    line_count = 0
    file = open("publickey_text.txt")
    publickey = ""
    while 1:
        line = file.readline()
        #print("%s\n" %line)
        if not line:
            break
        if (line_count >= 2 and line_count <= 6):
            line = line.strip()
            line = line.replace(':','')
            publickey = publickey + line
        line_count += 1
    os.remove("publickey_text.txt")
    publickey = publickey[2:]
    print("get publickey:%s\n" %publickey)
    stdout = utl.str2hex(publickey)
    return stdout
