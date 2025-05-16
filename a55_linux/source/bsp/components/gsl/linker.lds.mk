/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)
SECTIONS
{
	. = TEXT_BASE;
	. = ALIGN(8);
	.text	:
	{
	  boot/start.o  (.text)
	  *(.text)
	}
	. = ALIGN(8);
	.rodata :
	{
	  *(.rodata)
	  *(.version)
	}
	. = ALIGN(8);
	.data : { *(.data) }
	. = ALIGN(1024);
	_end = .;
	. = RAM_BASE+0x1a00+0x80;
	__bss_start = .;
	.bss : { *(.bss) }
	. = ALIGN(8);
	__bss_end = .;
}
