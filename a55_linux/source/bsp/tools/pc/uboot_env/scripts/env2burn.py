#!/usr/bin/python3
# -*- coding:utf-8 -*-

import sys
import os
import re
from xml.etree.ElementTree import Element
from xml.etree.ElementTree import SubElement
from xml.etree.ElementTree import ElementTree

class LOG_LEVEL:
	DEBUG = 0
	INFO  = 1
	ERROR = 2

class PartInfo:
	def __init__(self, name, start, size):
		self.name = name
		self.start = start
		self.size = size
	def __str__(self):
		return "%s(%#x, %#x)" % (self.name, self.start, self.size)
	def __repr__(self):
		return self.__str__()

CURR_LEVEL = LOG_LEVEL.ERROR
PLAT = None
TEE_ENABLE = False

def global_setup_plat(palt):
	global PLAT
	PLAT = palt

def global_get_plat():
	return PLAT

def global_setup_tee_enable(enable):
	global TEE_ENABLE
	TEE_ENABLE = enable

def global_is_tee_enable():
	return TEE_ENABLE

def log_debug(debug_msg):
	if CURR_LEVEL > LOG_LEVEL.DEBUG:
		return
	print('[DEBUG] %s' % debug_msg)

def log_info(info_msg):
	if CURR_LEVEL > LOG_LEVEL.INFO:
		return
	print('[INFO] %s' % info_msg)

def log_error(error_msg):
	if CURR_LEVEL > LOG_LEVEL.ERROR:
		return
	print('[ERROR] %s' % error_msg)

def err_process(err_msg):
	log_error('%s (%s)' % (err_msg, ENV_FILE))
	sys.exit(1)

def read_file(filename):
	if not os.path.exists(filename):
		err_process('%s : No such file.' % filename)

	with open(filename, 'r') as f:
		return f.read()
	return None

def select_boot(name):
	return 'boot_image.bin'

def select_env(flash_type):
	tee_infix = '_tee' if global_is_tee_enable() else ''
	env_dict = {
		'nand' : 'nand%s_env.bin' % tee_infix,
		'spi'  : 'nor%s_env.bin' % tee_infix,
		'emmc' : 'emmc%s_env.bin' % tee_infix,
	}
	if flash_type not in env_dict:
		return ''
	return env_dict[flash_type]

def select_kernel(has_fdt=True):
	if has_fdt:
		return 'uImage-fdt'
	return 'uImage'

def select_fw(name):
	fw_dict = {
		'bl31': 'bl31.bin',
		'teeimg': 'tee_image.bin'
	}
	return fw_dict[name]

def select_rootfs(flash_type, size):
	defualt = ''
	plat = global_get_plat()
	if plat == None:
		return defualt
	if flash_type == 'spi':
		return 'rootfs_%s_64k.jffs2' % plat
	if flash_type == 'emmc':
		return 'rootfs_%s_96M.ext4' % plat
	if flash_type == 'nand':
		if size == 0x2000000: #32M
			return 'rootfs_%s_2k_128k_32M.ubifs' % plat
		if size == 0x3200000: #50M
			return 'rootfs_%s_4k_256k_50M.ubifs' % plat

	return defualt

def select_file(flash_type, part):
	if part.name == 'boot':
		return select_boot(part.name)
	if part.name == 'env':
		return select_env(flash_type)
	if part.name in ['bl31', 'teeimg']:
		return select_fw(part.name)
	if part.name == 'kernel':
		return select_kernel()
	if part.name == 'rootfs':
		return select_rootfs(flash_type, part.size)
	return ''

def create_part_attr(flash_type, part):
	sel_file = select_file(flash_type, part)
	return {
		'Sel': '1',
		'PartitionName': part.name,
		'FlashType': flash_type,
		'FileSystem': 'none',
		'Start': hex(part.start),
		'Length': hex(part.size),
		'SelectFile': sel_file,
	}

def get_env_value(env_text, key, fail_exit=False):
	res = re.search(r'(?<=%s=).*\n?' % key, env_text)
	if res != None:
		return res[0].strip()
	if fail_exit:
		err_process('env: Failed to get "%s"' % key)
	return None

def get_bootargs_value(bootargs, key, fail_exit=True):
	res = re.search(r'(?<=%s=).*' % key, bootargs)
	if res == None:
		err_process('bootargs: Failed to get "%s"' % key)
	return res[0].strip().split(' ')[0]

def str2bytes(size_str):
	multiples = {
		'K': pow(1024, 1), 'M': pow(1024, 2),
		'G': pow(1024, 3), 'T': pow(1024, 4),
		'P': pow(1024, 5), 'E': pow(1024, 6),
	}
	val = size_str[:-1]
	unit = size_str[-1]
	if unit not in multiples:
		err_process('bootargs: wrong parition size "%s"' % size_str)
	return int(val) * multiples[unit]

def str2part(part_str):
	res_size = re.search(r'^[0-9]+[KMGTPE](?=\(.*\)$)', part_str)
	if res_size == None:
		err_process('bootargs: wrong parition size "%s"' % part_str)
	size = str2bytes(res_size[0])

	res_name = re.search(r'(?<=\().*(?=\))', part_str)
	if res_size == None:
		err_process('bootargs: wrong parition name "%s"' % part_str)
	name = res_name[0]

	# "start" will be set latter
	return PartInfo(name=name, start=None, size=size)

def get_bootargs_parts(bootargs, part_key):
	res = re.search(r'(?<=\s%s:)([0-9]+[KMGTPE]*\(.*\)\,)?[0-9]+[KMGTPE]*\(.*\)' % part_key, bootargs)
	if res == None:
		err_process('bootargs: parameter "%s:..." is missing' % part_key)
	partcfgstr = res[0]
	return partcfgstr

def part_cfgstr_to_list(partcfgstr):
	part_list = list()
	offset = 0
	for part_str in partcfgstr.split(','):
		part = str2part(part_str)
		part.start = offset
		part_list.append(part)
		offset += part.size
	return part_list

def check_tee_enable(part_list):
	for part in part_list:
		if part.name != 'teeimg':
			continue
		global_setup_tee_enable(True);
		break

def save_partition_tempalte(flash_type, part_list, out_xml_file):
	check_tee_enable(part_list)
	ele_part_info = Element('Partition_Info')
	for part in part_list:
		attr = create_part_attr(flash_type, part)
		log_debug(attr)
		ele_part = SubElement(ele_part_info, 'Part', attr)
	ele_tree = ElementTree(ele_part_info)
	ele_tree.write(out_xml_file, encoding = 'utf-8')

def mk_template(env_file, out_xml_file):
	env_text = read_file(env_file)

	plat = get_env_value(env_text, 'soc')
	log_debug("platform: %s" % plat)
	global_setup_plat(plat)

	# Step 1. get bootargs from env
	bootargs = get_env_value(env_text, 'bootargs', fail_exit=True)
	log_debug('bootargs: %s' % bootargs)

	# Step 2. get rootfs type from bootargs
	rootfs_type = get_bootargs_value(bootargs, 'rootfstype', fail_exit=True)
	log_debug('rootfs_type: %s' % rootfs_type)

	# Step 3. get partition configuration string from bootargs basing on rootfs type
	part_key_dict = {
		'ubifs': 'mtdparts=nand',
		'jffs2': 'mtdparts=sfc',
		'ext4' : 'blkdevparts=mmcblk0',
	}
	if rootfs_type not in part_key_dict:
		err_process('%s: rootfs type is not supported' % rootfs_type)
	part_key = part_key_dict[rootfs_type]
	partcfgstr = get_bootargs_parts(bootargs, part_key)
	log_debug("part cfg: %s" % partcfgstr)

	# Step 4. convert partition configuration string to PartInfo list
	part_list = part_cfgstr_to_list(partcfgstr)
	log_debug("part list: %s" % part_list)

	# Step 5. create template basing on PartInfo list
	flash_type_dict = {
		'ubifs': 'nand',
		'jffs2': 'spi',
		'ext4' : 'emmc'}
	if rootfs_type not in flash_type_dict:
		err_process('%s: rootfs type is not supported' % rootfs_type)
	flash_type = flash_type_dict[rootfs_type]
	save_partition_tempalte(flash_type, part_list, out_xml_file)

def print_usage():
	print('Usage:')
	print('python3 %s <env_dir> <out_xml_file>' % sys.argv[0])

if __name__=="__main__":
	if (len(sys.argv) < 3):
		print_usage()
		sys.exit(1)

	ENV_FILE = os.path.abspath(sys.argv[1])
	XML_FILE = os.path.abspath(sys.argv[2])
	log_info('Input: %s' % ENV_FILE)
	mk_template(ENV_FILE, XML_FILE)
	log_info('Output: %s' % XML_FILE)

