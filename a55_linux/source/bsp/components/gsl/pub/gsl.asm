
pub/gsl:     file format elf64-littleaarch64


Disassembly of section .text:

0000000004022e00 <_start>:
 4022e00:	14000002 	b	4022e08 <reset>
 4022e04:	d503201f 	nop

0000000004022e08 <reset>:
 4022e08:	58000e40 	ldr	x0, 4022fd0 <get_current_core+0x1c>
 4022e0c:	58000e21 	ldr	x1, 4022fd0 <get_current_core+0x1c>
 4022e10:	58000e02 	ldr	x2, 4022fd0 <get_current_core+0x1c>
 4022e14:	58000de3 	ldr	x3, 4022fd0 <get_current_core+0x1c>
 4022e18:	58000dc4 	ldr	x4, 4022fd0 <get_current_core+0x1c>
 4022e1c:	58000da5 	ldr	x5, 4022fd0 <get_current_core+0x1c>
 4022e20:	58000d86 	ldr	x6, 4022fd0 <get_current_core+0x1c>
 4022e24:	58000d67 	ldr	x7, 4022fd0 <get_current_core+0x1c>
 4022e28:	58000d48 	ldr	x8, 4022fd0 <get_current_core+0x1c>
 4022e2c:	58000d29 	ldr	x9, 4022fd0 <get_current_core+0x1c>
 4022e30:	58000d0a 	ldr	x10, 4022fd0 <get_current_core+0x1c>
 4022e34:	58000ceb 	ldr	x11, 4022fd0 <get_current_core+0x1c>
 4022e38:	58000ccc 	ldr	x12, 4022fd0 <get_current_core+0x1c>
 4022e3c:	58000cad 	ldr	x13, 4022fd0 <get_current_core+0x1c>
 4022e40:	58000c8e 	ldr	x14, 4022fd0 <get_current_core+0x1c>
 4022e44:	58000c6f 	ldr	x15, 4022fd0 <get_current_core+0x1c>
 4022e48:	58000c50 	ldr	x16, 4022fd0 <get_current_core+0x1c>
 4022e4c:	58000c31 	ldr	x17, 4022fd0 <get_current_core+0x1c>
 4022e50:	58000c12 	ldr	x18, 4022fd0 <get_current_core+0x1c>
 4022e54:	58000bf3 	ldr	x19, 4022fd0 <get_current_core+0x1c>
 4022e58:	58000bd4 	ldr	x20, 4022fd0 <get_current_core+0x1c>
 4022e5c:	58000bb5 	ldr	x21, 4022fd0 <get_current_core+0x1c>
 4022e60:	58000b96 	ldr	x22, 4022fd0 <get_current_core+0x1c>
 4022e64:	58000b77 	ldr	x23, 4022fd0 <get_current_core+0x1c>
 4022e68:	58000b58 	ldr	x24, 4022fd0 <get_current_core+0x1c>
 4022e6c:	58000b39 	ldr	x25, 4022fd0 <get_current_core+0x1c>
 4022e70:	58000b1a 	ldr	x26, 4022fd0 <get_current_core+0x1c>
 4022e74:	58000afb 	ldr	x27, 4022fd0 <get_current_core+0x1c>
 4022e78:	58000adc 	ldr	x28, 4022fd0 <get_current_core+0x1c>
 4022e7c:	58000abd 	ldr	x29, 4022fd0 <get_current_core+0x1c>
 4022e80:	58000a9e 	ldr	x30, 4022fd0 <get_current_core+0x1c>
 4022e84:	1005cbe0 	adr	x0, 402e800 <sync_exception_sp_el0>
 4022e88:	d51ec000 	msr	vbar_el3, x0
 4022e8c:	d53e1100 	mrs	x0, scr_el3
 4022e90:	b2400c00 	orr	x0, x0, #0xf
 4022e94:	d51e1100 	msr	scr_el3, x0
 4022e98:	d51e115f 	msr	cptr_el3, xzr
 4022e9c:	94000046 	bl	4022fb4 <get_current_core>
 4022ea0:	b5000200 	cbnz	x0, 4022ee0 <minor_core_boot>
 4022ea4:	580009a0 	ldr	x0, 4022fd8 <get_current_core+0x24>
 4022ea8:	d2800001 	mov	x1, #0x0                   	// #0
 4022eac:	580009a2 	ldr	x2, 4022fe0 <get_current_core+0x2c>
 4022eb0:	eb000042 	subs	x2, x2, x0
 4022eb4:	94002f94 	bl	402ed04 <clear_data>
 4022eb8:	58000940 	ldr	x0, 4022fe0 <get_current_core+0x2c>
 4022ebc:	927cec1f 	and	sp, x0, #0xfffffffffffffff0
 4022ec0:	58000940 	ldr	x0, 4022fe8 <get_current_core+0x34>
 4022ec4:	d2800001 	mov	x1, #0x0                   	// #0
 4022ec8:	58000942 	ldr	x2, 4022ff0 <get_current_core+0x3c>
 4022ecc:	eb000042 	subs	x2, x2, x0
 4022ed0:	94002f8d 	bl	402ed04 <clear_data>
 4022ed4:	94000456 	bl	402402c <stack_chk_guard_setup>
 4022ed8:	940004c7 	bl	40241f4 <sys_watchdog_enable>
 4022edc:	140004cb 	b	4024208 <main_entry>

0000000004022ee0 <minor_core_boot>:
 4022ee0:	58000401 	ldr	x1, 4022f60 <_jump_addr>
 4022ee4:	f9400037 	ldr	x23, [x1]
 4022ee8:	aa1703e0 	mov	x0, x23
 4022eec:	14000028 	b	4022f8c <switch_from_el3_to_el1>

0000000004022ef0 <non_tee_copy_begin>:
 4022ef0:	aa1503e0 	mov	x0, x21
 4022ef4:	aa1603e1 	mov	x1, x22
 4022ef8:	52800002 	mov	w2, #0x0                   	// #0
 4022efc:	d2800083 	mov	x3, #0x4                   	// #4

0000000004022f00 <do_code_clear_loop>:
 4022f00:	b9000002 	str	w2, [x0]
 4022f04:	8b030000 	add	x0, x0, x3
 4022f08:	eb01001f 	cmp	x0, x1
 4022f0c:	54ffffab 	b.lt	4022f00 <do_code_clear_loop>  // b.tstop
 4022f10:	14000002 	b	4022f18 <set_scs_finish>
 4022f14:	d503201f 	nop

0000000004022f18 <set_scs_finish>:
 4022f18:	58000203 	ldr	x3, 4022f58 <__REG_BASE_CA_MISC>
 4022f1c:	b9440061 	ldr	w1, [x3, #1024]
 4022f20:	121c6c21 	and	w1, w1, #0xfffffff0
 4022f24:	528000a2 	mov	w2, #0x5                   	// #5
 4022f28:	2a020021 	orr	w1, w1, w2
 4022f2c:	b9040061 	str	w1, [x3, #1024]

0000000004022f30 <excute_bootloader>:
 4022f30:	aa1703fe 	mov	x30, x23
 4022f34:	d65f03c0 	ret
 4022f38:	d503201f 	nop
 4022f3c:	d503201f 	nop
 4022f40:	d503201f 	nop
 4022f44:	d503201f 	nop
 4022f48:	d503201f 	nop
 4022f4c:	d503201f 	nop
 4022f50:	d503201f 	nop
 4022f54:	d503201f 	nop

0000000004022f58 <__REG_BASE_CA_MISC>:
 4022f58:	101e8000 	adr	x0, 405ff58 <_end+0x30758>
 4022f5c:	00000000 	udf	#0

0000000004022f60 <_jump_addr>:
 4022f60:	04021ab8 	.inst	0x04021ab8 ; undefined
 4022f64:	00000000 	udf	#0

0000000004022f68 <copy_code_to_sram>:
 4022f68:	8b020002 	add	x2, x0, x2

0000000004022f6c <copy_code_to_sram_loop>:
 4022f6c:	a8c12c0a 	ldp	x10, x11, [x0], #16
 4022f70:	a8812c2a 	stp	x10, x11, [x1], #16
 4022f74:	eb02001f 	cmp	x0, x2
 4022f78:	54ffffad 	b.le	4022f6c <copy_code_to_sram_loop>
 4022f7c:	d65f03c0 	ret

0000000004022f80 <_do_error>:
 4022f80:	580003c1 	ldr	x1, 4022ff8 <get_current_core+0x44>
 4022f84:	18000220 	ldr	w0, 4022fc8 <get_current_core+0x14>
 4022f88:	b9000420 	str	w0, [x1, #4]

0000000004022f8c <switch_from_el3_to_el1>:
 4022f8c:	580003a1 	ldr	x1, 4023000 <get_current_core+0x4c>
 4022f90:	d5181001 	msr	sctlr_el1, x1
 4022f94:	580003a1 	ldr	x1, 4023008 <get_current_core+0x54>
 4022f98:	d51c1101 	msr	hcr_el2, x1
 4022f9c:	580003a1 	ldr	x1, 4023010 <get_current_core+0x5c>
 4022fa0:	d51e1101 	msr	scr_el3, x1
 4022fa4:	580003a1 	ldr	x1, 4023018 <get_current_core+0x64>
 4022fa8:	d51e4001 	msr	spsr_el3, x1
 4022fac:	d51e4020 	msr	elr_el3, x0
 4022fb0:	d69f03e0 	eret

0000000004022fb4 <get_current_core>:
 4022fb4:	d53800a0 	mrs	x0, mpidr_el1
 4022fb8:	92401c01 	and	x1, x0, #0xff
 4022fbc:	92781c00 	and	x0, x0, #0xff00
 4022fc0:	8b402020 	add	x0, x1, x0, lsr #8
 4022fc4:	d65f03c0 	ret
 4022fc8:	00000001 	udf	#1
	...
 4022fd8:	04020000 	.inst	0x04020000 ; undefined
 4022fdc:	00000000 	udf	#0
 4022fe0:	04021000 	.inst	0x04021000 ; undefined
 4022fe4:	00000000 	udf	#0
 4022fe8:	04021a80 	.inst	0x04021a80 ; undefined
 4022fec:	00000000 	udf	#0
 4022ff0:	04021be0 	.inst	0x04021be0 ; undefined
 4022ff4:	00000000 	udf	#0
 4022ff8:	11020000 	add	w0, w0, #0x80
 4022ffc:	00000000 	udf	#0
 4023000:	30d00800 	adr	x0, 3fc3101 <hash_verify_buf-0x5e97f>
 4023004:	00000000 	udf	#0
 4023008:	80000000 	.inst	0x80000000 ; undefined
 402300c:	00000000 	udf	#0
 4023010:	00000401 	udf	#1025
 4023014:	00000000 	udf	#0
 4023018:	000003c5 	udf	#965
 402301c:	00000000 	udf	#0

0000000004023020 <clean_boot_img_addr_size>:
 4023020:	d2800280 	mov	x0, #0x14                  	// #20
 4023024:	f2a22240 	movk	x0, #0x1112, lsl #16
 4023028:	b900001f 	str	wzr, [x0]
 402302c:	b900041f 	str	wzr, [x0, #4]
 4023030:	d65f03c0 	ret

0000000004023034 <add_boot_img_size>:
 4023034:	d2800302 	mov	x2, #0x18                  	// #24
 4023038:	f2a22242 	movk	x2, #0x1112, lsl #16
 402303c:	b9400041 	ldr	w1, [x2]
 4023040:	0b000021 	add	w1, w1, w0
 4023044:	b9000041 	str	w1, [x2]
 4023048:	d65f03c0 	ret

000000000402304c <set_verify_backup_img_reg>:
 402304c:	d2800382 	mov	x2, #0x1c                  	// #28
 4023050:	f2a22242 	movk	x2, #0x1112, lsl #16
 4023054:	b9400041 	ldr	w1, [x2]
 4023058:	33180401 	bfi	w1, w0, #8, #2
 402305c:	b9000041 	str	w1, [x2]
 4023060:	d65f03c0 	ret

0000000004023064 <get_verify_backup_img_reg>:
 4023064:	d2800380 	mov	x0, #0x1c                  	// #28
 4023068:	f2a22240 	movk	x0, #0x1112, lsl #16
 402306c:	b9400000 	ldr	w0, [x0]
 4023070:	d3482400 	ubfx	x0, x0, #8, #2
 4023074:	d65f03c0 	ret

0000000004023078 <enable_lpds_lock>:
 4023078:	d2800580 	mov	x0, #0x2c                  	// #44
 402307c:	f2a22240 	movk	x0, #0x1112, lsl #16
 4023080:	b9400001 	ldr	w1, [x0]
 4023084:	32000021 	orr	w1, w1, #0x1
 4023088:	b9000001 	str	w1, [x0]
 402308c:	b9400000 	ldr	w0, [x0]
 4023090:	37000200 	tbnz	w0, #0, 40230d0 <enable_lpds_lock+0x58>
 4023094:	f81f0ffe 	str	x30, [sp, #-16]!
 4023098:	f0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 402309c:	913a5000 	add	x0, x0, #0xe94
 40230a0:	94001311 	bl	4027ce4 <log_serial_puts>
 40230a4:	d2800040 	mov	x0, #0x2                   	// #2
 40230a8:	9400132b 	bl	4027d54 <mdelay>
 40230ac:	9400131a 	bl	4027d14 <timer_deinit>
 40230b0:	d2800080 	mov	x0, #0x4                   	// #4
 40230b4:	52800021 	mov	w1, #0x1                   	// #1
 40230b8:	f2a22040 	movk	x0, #0x1102, lsl #16
 40230bc:	f84107fe 	ldr	x30, [sp], #16
 40230c0:	b9000001 	str	w1, [x0]
 40230c4:	b9000001 	str	w1, [x0]
 40230c8:	b9000001 	str	w1, [x0]
 40230cc:	d65f03c0 	ret
 40230d0:	d65f03c0 	ret

00000000040230d4 <set_low_power_mode>:
 40230d4:	d2800382 	mov	x2, #0x1c                  	// #28
 40230d8:	f2a22242 	movk	x2, #0x1112, lsl #16
 40230dc:	b9400041 	ldr	w1, [x2]
 40230e0:	33001c01 	bfxil	w1, w0, #0, #8
 40230e4:	b9000041 	str	w1, [x2]
 40230e8:	d65f03c0 	ret

00000000040230ec <get_low_power_mode>:
 40230ec:	d2800380 	mov	x0, #0x1c                  	// #28
 40230f0:	f2a22240 	movk	x0, #0x1112, lsl #16
 40230f4:	b9400000 	ldr	w0, [x0]
 40230f8:	12001c00 	and	w0, w0, #0xff
 40230fc:	d65f03c0 	ret

0000000004023100 <clear_lpds>:
 4023100:	d2800580 	mov	x0, #0x2c                  	// #44
 4023104:	f81f0ffe 	str	x30, [sp, #-16]!
 4023108:	f2a22240 	movk	x0, #0x1112, lsl #16
 402310c:	b9400000 	ldr	w0, [x0]
 4023110:	37000100 	tbnz	w0, #0, 4023130 <clear_lpds+0x30>
 4023114:	d2800600 	mov	x0, #0x30                  	// #48
 4023118:	d2802601 	mov	x1, #0x130                 	// #304
 402311c:	f2a22240 	movk	x0, #0x1112, lsl #16
 4023120:	f2a22241 	movk	x1, #0x1112, lsl #16
 4023124:	b800441f 	str	wzr, [x0], #4
 4023128:	eb01001f 	cmp	x0, x1
 402312c:	54ffffc1 	b.ne	4023124 <clear_lpds+0x24>  // b.any
 4023130:	52800000 	mov	w0, #0x0                   	// #0
 4023134:	97ffffe8 	bl	40230d4 <set_low_power_mode>
 4023138:	f84107fe 	ldr	x30, [sp], #16
 402313c:	f0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4023140:	913ab400 	add	x0, x0, #0xead
 4023144:	140012e8 	b	4027ce4 <log_serial_puts>

0000000004023148 <lpds_hash_get>:
 4023148:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402314c:	2a0203e3 	mov	w3, w2
 4023150:	2a0103f4 	mov	w20, w1
 4023154:	aa0003f3 	mov	x19, x0
 4023158:	d2800004 	mov	x4, #0x0                   	// #0
 402315c:	52800002 	mov	w2, #0x0                   	// #0
 4023160:	d2800401 	mov	x1, #0x20                  	// #32
 4023164:	f9000bfe 	str	x30, [sp, #16]
 4023168:	94001313 	bl	4027db4 <memset_ss>
 402316c:	7102941f 	cmp	w0, #0xa5
 4023170:	54000161 	b.ne	402319c <lpds_hash_get+0x54>  // b.any
 4023174:	d2800002 	mov	x2, #0x0                   	// #0
 4023178:	0b020281 	add	w1, w20, w2
 402317c:	b9400021 	ldr	w1, [x1]
 4023180:	b8226a61 	str	w1, [x19, x2]
 4023184:	91001042 	add	x2, x2, #0x4
 4023188:	f100805f 	cmp	x2, #0x20
 402318c:	54ffff61 	b.ne	4023178 <lpds_hash_get+0x30>  // b.any
 4023190:	f9400bfe 	ldr	x30, [sp, #16]
 4023194:	a8c253f3 	ldp	x19, x20, [sp], #32
 4023198:	d65f03c0 	ret
 402319c:	52800b40 	mov	w0, #0x5a                  	// #90
 40231a0:	17fffffc 	b	4023190 <lpds_hash_get+0x48>

00000000040231a4 <verify_all_lpds_data>:
 40231a4:	d2800380 	mov	x0, #0x1c                  	// #28
 40231a8:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 40231ac:	f2a22240 	movk	x0, #0x1112, lsl #16
 40231b0:	b9400013 	ldr	w19, [x0]
 40231b4:	12001e73 	and	w19, w19, #0xff
 40231b8:	71016a7f 	cmp	w19, #0x5a
 40231bc:	54000080 	b.eq	40231cc <verify_all_lpds_data+0x28>  // b.none
 40231c0:	528014a0 	mov	w0, #0xa5                  	// #165
 40231c4:	a8c17bf3 	ldp	x19, x30, [sp], #16
 40231c8:	d65f03c0 	ret
 40231cc:	d2800601 	mov	x1, #0x30                  	// #48
 40231d0:	52802200 	mov	w0, #0x110                 	// #272
 40231d4:	52801c02 	mov	w2, #0xe0                  	// #224
 40231d8:	f2a22241 	movk	x1, #0x1112, lsl #16
 40231dc:	72a22240 	movk	w0, #0x1112, lsl #16
 40231e0:	9400087e 	bl	40253d8 <lpds_hash_cmp>
 40231e4:	7102941f 	cmp	w0, #0xa5
 40231e8:	54fffec0 	b.eq	40231c0 <verify_all_lpds_data+0x1c>  // b.none
 40231ec:	52800d60 	mov	w0, #0x6b                  	// #107
 40231f0:	940003a2 	bl	4024078 <err_print>
 40231f4:	2a1303e0 	mov	w0, w19
 40231f8:	17fffff3 	b	40231c4 <verify_all_lpds_data+0x20>

00000000040231fc <calculate_all_lpds_data>:
 40231fc:	d2802202 	mov	x2, #0x110                 	// #272
 4023200:	52800600 	mov	w0, #0x30                  	// #48
 4023204:	52800004 	mov	w4, #0x0                   	// #0
 4023208:	52800403 	mov	w3, #0x20                  	// #32
 402320c:	f2a22242 	movk	x2, #0x1112, lsl #16
 4023210:	52801c01 	mov	w1, #0xe0                  	// #224
 4023214:	72a22240 	movk	w0, #0x1112, lsl #16
 4023218:	140000e2 	b	40235a0 <calc_hash>

000000000402321c <decrypt_data>:
 402321c:	d103c3ff 	sub	sp, sp, #0xf0
 4023220:	a90b53f3 	stp	x19, x20, [sp, #176]
 4023224:	aa0103f3 	mov	x19, x1
 4023228:	aa0303f4 	mov	x20, x3
 402322c:	a90c5bf5 	stp	x21, x22, [sp, #192]
 4023230:	2a0403f6 	mov	w22, w4
 4023234:	a90d63f7 	stp	x23, x24, [sp, #208]
 4023238:	aa0203f7 	mov	x23, x2
 402323c:	d2800002 	mov	x2, #0x0                   	// #0
 4023240:	a90e7bf9 	stp	x25, x30, [sp, #224]
 4023244:	2a0003f9 	mov	w25, w0
 4023248:	d0ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402324c:	f9455001 	ldr	x1, [x0, #2720]
 4023250:	f90057e1 	str	x1, [sp, #168]
 4023254:	d2800001 	mov	x1, #0x0                   	// #0
 4023258:	910023e0 	add	x0, sp, #0x8
 402325c:	910063e1 	add	x1, sp, #0x18
 4023260:	29037fff 	stp	wzr, wzr, [sp, #24]
 4023264:	940011dc 	bl	40279d4 <uapi_drv_cipher_create>
 4023268:	7102941f 	cmp	w0, #0xa5
 402326c:	54000160 	b.eq	4023298 <decrypt_data+0x7c>  // b.none
 4023270:	528008c0 	mov	w0, #0x46                  	// #70
 4023274:	52800b53 	mov	w19, #0x5a                  	// #90
 4023278:	94000380 	bl	4024078 <err_print>
 402327c:	d0ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4023280:	f94057e1 	ldr	x1, [sp, #168]
 4023284:	f9455002 	ldr	x2, [x0, #2720]
 4023288:	eb020021 	subs	x1, x1, x2
 402328c:	d2800002 	mov	x2, #0x0                   	// #0
 4023290:	540017a0 	b.eq	4023584 <decrypt_data+0x368>  // b.none
 4023294:	940003b6 	bl	402416c <__stack_chk_fail>
 4023298:	940011ff 	bl	4027a94 <uapi_drv_keyslot_init>
 402329c:	7102941f 	cmp	w0, #0xa5
 40232a0:	540000a0 	b.eq	40232b4 <decrypt_data+0x98>  // b.none
 40232a4:	f94007e0 	ldr	x0, [sp, #8]
 40232a8:	940011cf 	bl	40279e4 <uapi_drv_cipher_destroy>
 40232ac:	528008e0 	mov	w0, #0x47                  	// #71
 40232b0:	17fffff1 	b	4023274 <decrypt_data+0x58>
 40232b4:	910043e0 	add	x0, sp, #0x10
 40232b8:	52800001 	mov	w1, #0x0                   	// #0
 40232bc:	940011fe 	bl	4027ab4 <uapi_drv_keyslot_create>
 40232c0:	7102941f 	cmp	w0, #0xa5
 40232c4:	540000c0 	b.eq	40232dc <decrypt_data+0xc0>  // b.none
 40232c8:	940011f7 	bl	4027aa4 <uapi_drv_keyslot_deinit>
 40232cc:	f94007e0 	ldr	x0, [sp, #8]
 40232d0:	940011c5 	bl	40279e4 <uapi_drv_cipher_destroy>
 40232d4:	52800900 	mov	w0, #0x48                  	// #72
 40232d8:	17ffffe7 	b	4023274 <decrypt_data+0x58>
 40232dc:	a94087e0 	ldp	x0, x1, [sp, #8]
 40232e0:	d2800002 	mov	x2, #0x0                   	// #0
 40232e4:	940011cc 	bl	4027a14 <uapi_drv_cipher_attach>
 40232e8:	7102941f 	cmp	w0, #0xa5
 40232ec:	54000100 	b.eq	402330c <decrypt_data+0xf0>  // b.none
 40232f0:	940011ed 	bl	4027aa4 <uapi_drv_keyslot_deinit>
 40232f4:	f94007e0 	ldr	x0, [sp, #8]
 40232f8:	940011bb 	bl	40279e4 <uapi_drv_cipher_destroy>
 40232fc:	f9400be0 	ldr	x0, [sp, #16]
 4023300:	940011f1 	bl	4027ac4 <uapi_drv_keyslot_destroy>
 4023304:	52800920 	mov	w0, #0x49                  	// #73
 4023308:	17ffffdb 	b	4023274 <decrypt_data+0x58>
 402330c:	52800b40 	mov	w0, #0x5a                  	// #90
 4023310:	b90003e0 	str	w0, [sp]
 4023314:	d2a203c0 	mov	x0, #0x101e0000            	// #270401536
 4023318:	91009262 	add	x2, x19, #0x24
 402331c:	52800021 	mov	w1, #0x1                   	// #1
 4023320:	b9400000 	ldr	w0, [x0]
 4023324:	f94007f8 	ldr	x24, [sp, #8]
 4023328:	360003a0 	tbz	w0, #0, 402339c <decrypt_data+0x180>
 402332c:	52800040 	mov	w0, #0x2                   	// #2
 4023330:	b9005be0 	str	w0, [sp, #88]
 4023334:	9101c3e0 	add	x0, sp, #0x70
 4023338:	f90037e0 	str	x0, [sp, #104]
 402333c:	9101e3e0 	add	x0, sp, #0x78
 4023340:	290e07ff 	stp	wzr, w1, [sp, #112]
 4023344:	d2800203 	mov	x3, #0x10                  	// #16
 4023348:	d2800004 	mov	x4, #0x0                   	// #0
 402334c:	aa0303e1 	mov	x1, x3
 4023350:	94001295 	bl	4027da4 <memcpy_ss>
 4023354:	b90003e0 	str	w0, [sp]
 4023358:	b94003e0 	ldr	w0, [sp]
 402335c:	7102941f 	cmp	w0, #0xa5
 4023360:	540002e0 	b.eq	40233bc <decrypt_data+0x1a0>  // b.none
 4023364:	52800960 	mov	w0, #0x4b                  	// #75
 4023368:	52800b53 	mov	w19, #0x5a                  	// #90
 402336c:	94000343 	bl	4024078 <err_print>
 4023370:	528009a0 	mov	w0, #0x4d                  	// #77
 4023374:	94000341 	bl	4024078 <err_print>
 4023378:	a94087e0 	ldp	x0, x1, [sp, #8]
 402337c:	d2800002 	mov	x2, #0x0                   	// #0
 4023380:	940011a9 	bl	4027a24 <uapi_drv_cipher_detach>
 4023384:	f9400be0 	ldr	x0, [sp, #16]
 4023388:	940011cf 	bl	4027ac4 <uapi_drv_keyslot_destroy>
 402338c:	f94007e0 	ldr	x0, [sp, #8]
 4023390:	94001195 	bl	40279e4 <uapi_drv_cipher_destroy>
 4023394:	940011c4 	bl	4027aa4 <uapi_drv_keyslot_deinit>
 4023398:	17ffffb9 	b	402327c <decrypt_data+0x60>
 402339c:	52800060 	mov	w0, #0x3                   	// #3
 40233a0:	291207e0 	stp	w0, w1, [sp, #144]
 40233a4:	910223e0 	add	x0, sp, #0x88
 40233a8:	b9005be1 	str	w1, [sp, #88]
 40233ac:	f90037e0 	str	x0, [sp, #104]
 40233b0:	910263e0 	add	x0, sp, #0x98
 40233b4:	29117fe1 	stp	w1, wzr, [sp, #136]
 40233b8:	17ffffe3 	b	4023344 <decrypt_data+0x128>
 40233bc:	528000a0 	mov	w0, #0x5                   	// #5
 40233c0:	910163e1 	add	x1, sp, #0x58
 40233c4:	52800035 	mov	w21, #0x1                   	// #1
 40233c8:	d2800002 	mov	x2, #0x0                   	// #0
 40233cc:	290b83f5 	stp	w21, w0, [sp, #92]
 40233d0:	aa1803e0 	mov	x0, x24
 40233d4:	9400118c 	bl	4027a04 <uapi_drv_cipher_set_config>
 40233d8:	b90003e0 	str	w0, [sp]
 40233dc:	b94003e0 	ldr	w0, [sp]
 40233e0:	7102941f 	cmp	w0, #0xa5
 40233e4:	54000060 	b.eq	40233f0 <decrypt_data+0x1d4>  // b.none
 40233e8:	52800980 	mov	w0, #0x4c                  	// #76
 40233ec:	17ffffdf 	b	4023368 <decrypt_data+0x14c>
 40233f0:	52800b40 	mov	w0, #0x5a                  	// #90
 40233f4:	b90007e0 	str	w0, [sp, #4]
 40233f8:	d2a203c0 	mov	x0, #0x101e0000            	// #270401536
 40233fc:	52800402 	mov	w2, #0x20                  	// #32
 4023400:	52800041 	mov	w1, #0x2                   	// #2
 4023404:	b9400000 	ldr	w0, [x0]
 4023408:	790063f5 	strh	w21, [sp, #48]
 402340c:	72000000 	ands	w0, w0, #0x1
 4023410:	b90037f5 	str	w21, [sp, #52]
 4023414:	1a951021 	csel	w1, w1, w21, ne  // ne = any
 4023418:	52800a00 	mov	w0, #0x50                  	// #80
 402341c:	1a821000 	csel	w0, w0, w2, ne  // ne = any
 4023420:	91001262 	add	x2, x19, #0x4
 4023424:	290503f9 	stp	w25, w0, [sp, #40]
 4023428:	3200c3e0 	mov	w0, #0x1010101             	// #16843009
 402342c:	b9003be0 	str	w0, [sp, #56]
 4023430:	52800200 	mov	w0, #0x10                  	// #16
 4023434:	91005273 	add	x19, x19, #0x14
 4023438:	3900f3f5 	strb	w21, [sp, #60]
 402343c:	b90043e1 	str	w1, [sp, #64]
 4023440:	b90047e0 	str	w0, [sp, #68]
 4023444:	f90027f3 	str	x19, [sp, #72]
 4023448:	b90053ff 	str	wzr, [sp, #80]
 402344c:	290b07ff 	stp	wzr, w1, [sp, #88]
 4023450:	b90063e0 	str	w0, [sp, #96]
 4023454:	910083e0 	add	x0, sp, #0x20
 4023458:	f90037e2 	str	x2, [sp, #104]
 402345c:	f9400bf8 	ldr	x24, [sp, #16]
 4023460:	9400119d 	bl	4027ad4 <uapi_drv_klad_create>
 4023464:	7102941f 	cmp	w0, #0xa5
 4023468:	54000861 	b.ne	4023574 <decrypt_data+0x358>  // b.any
 402346c:	f94013e0 	ldr	x0, [sp, #32]
 4023470:	aa1803e2 	mov	x2, x24
 4023474:	d2800003 	mov	x3, #0x0                   	// #0
 4023478:	52800001 	mov	w1, #0x0                   	// #0
 402347c:	9400119e 	bl	4027af4 <uapi_drv_klad_attach>
 4023480:	7102941f 	cmp	w0, #0xa5
 4023484:	540003c1 	b.ne	40234fc <decrypt_data+0x2e0>  // b.any
 4023488:	f94013e0 	ldr	x0, [sp, #32]
 402348c:	9100a3e1 	add	x1, sp, #0x28
 4023490:	d2800002 	mov	x2, #0x0                   	// #0
 4023494:	940011a0 	bl	4027b14 <uapi_drv_klad_set_attr>
 4023498:	7102941f 	cmp	w0, #0xa5
 402349c:	540001e1 	b.ne	40234d8 <decrypt_data+0x2bc>  // b.any
 40234a0:	f94013e0 	ldr	x0, [sp, #32]
 40234a4:	910163e1 	add	x1, sp, #0x58
 40234a8:	d2800002 	mov	x2, #0x0                   	// #0
 40234ac:	9400119e 	bl	4027b24 <uapi_drv_klad_set_session_key>
 40234b0:	7102941f 	cmp	w0, #0xa5
 40234b4:	54000121 	b.ne	40234d8 <decrypt_data+0x2bc>  // b.any
 40234b8:	f94013e0 	ldr	x0, [sp, #32]
 40234bc:	910103e1 	add	x1, sp, #0x40
 40234c0:	d2800003 	mov	x3, #0x0                   	// #0
 40234c4:	52800002 	mov	w2, #0x0                   	// #0
 40234c8:	9400119b 	bl	4027b34 <uapi_drv_klad_set_content_key>
 40234cc:	7102941f 	cmp	w0, #0xa5
 40234d0:	54000041 	b.ne	40234d8 <decrypt_data+0x2bc>  // b.any
 40234d4:	b90007e0 	str	w0, [sp, #4]
 40234d8:	f94013e0 	ldr	x0, [sp, #32]
 40234dc:	aa1803e2 	mov	x2, x24
 40234e0:	d2800003 	mov	x3, #0x0                   	// #0
 40234e4:	52800001 	mov	w1, #0x0                   	// #0
 40234e8:	94001187 	bl	4027b04 <uapi_drv_klad_detach>
 40234ec:	7102941f 	cmp	w0, #0xa5
 40234f0:	54000060 	b.eq	40234fc <decrypt_data+0x2e0>  // b.none
 40234f4:	52800b40 	mov	w0, #0x5a                  	// #90
 40234f8:	b90007e0 	str	w0, [sp, #4]
 40234fc:	f94013e0 	ldr	x0, [sp, #32]
 4023500:	94001179 	bl	4027ae4 <uapi_drv_klad_destroy>
 4023504:	7102941f 	cmp	w0, #0xa5
 4023508:	54000361 	b.ne	4023574 <decrypt_data+0x358>  // b.any
 402350c:	b94007f3 	ldr	w19, [sp, #4]
 4023510:	7102967f 	cmp	w19, #0xa5
 4023514:	54000321 	b.ne	4023578 <decrypt_data+0x35c>  // b.any
 4023518:	52800020 	mov	w0, #0x1                   	// #1
 402351c:	b9004be0 	str	w0, [sp, #72]
 4023520:	b90063e0 	str	w0, [sp, #96]
 4023524:	92407e94 	and	x20, x20, #0xffffffff
 4023528:	f94007e0 	ldr	x0, [sp, #8]
 402352c:	2a1603e3 	mov	w3, w22
 4023530:	910163e2 	add	x2, sp, #0x58
 4023534:	910103e1 	add	x1, sp, #0x40
 4023538:	52800b55 	mov	w21, #0x5a                  	// #90
 402353c:	d2800005 	mov	x5, #0x0                   	// #0
 4023540:	52800004 	mov	w4, #0x0                   	// #0
 4023544:	b9002bf5 	str	w21, [sp, #40]
 4023548:	f90023f4 	str	x20, [sp, #64]
 402354c:	f9002ff7 	str	x23, [sp, #88]
 4023550:	94001139 	bl	4027a34 <uapi_drv_cipher_decrypt>
 4023554:	b9002be0 	str	w0, [sp, #40]
 4023558:	b9402bf3 	ldr	w19, [sp, #40]
 402355c:	7102967f 	cmp	w19, #0xa5
 4023560:	54fff0c0 	b.eq	4023378 <decrypt_data+0x15c>  // b.none
 4023564:	528009e0 	mov	w0, #0x4f                  	// #79
 4023568:	2a1503f3 	mov	w19, w21
 402356c:	940002c3 	bl	4024078 <err_print>
 4023570:	17ffff82 	b	4023378 <decrypt_data+0x15c>
 4023574:	52800b53 	mov	w19, #0x5a                  	// #90
 4023578:	528009c0 	mov	w0, #0x4e                  	// #78
 402357c:	940002bf 	bl	4024078 <err_print>
 4023580:	17ffff7e 	b	4023378 <decrypt_data+0x15c>
 4023584:	2a1303e0 	mov	w0, w19
 4023588:	a94b53f3 	ldp	x19, x20, [sp, #176]
 402358c:	a94c5bf5 	ldp	x21, x22, [sp, #192]
 4023590:	a94d63f7 	ldp	x23, x24, [sp, #208]
 4023594:	a94e7bf9 	ldp	x25, x30, [sp, #224]
 4023598:	9103c3ff 	add	sp, sp, #0xf0
 402359c:	d65f03c0 	ret

00000000040235a0 <calc_hash>:
 40235a0:	d10183ff 	sub	sp, sp, #0x60
 40235a4:	2a0003e0 	mov	w0, w0
 40235a8:	a90453f3 	stp	x19, x20, [sp, #64]
 40235ac:	2a0103f3 	mov	w19, w1
 40235b0:	d0ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 40235b4:	a9057bf5 	stp	x21, x30, [sp, #80]
 40235b8:	aa0203f5 	mov	x21, x2
 40235bc:	52800b54 	mov	w20, #0x5a                  	// #90
 40235c0:	f9455022 	ldr	x2, [x1, #2720]
 40235c4:	f9001fe2 	str	x2, [sp, #56]
 40235c8:	d2800002 	mov	x2, #0x0                   	// #0
 40235cc:	d2a203c1 	mov	x1, #0x101e0000            	// #270401536
 40235d0:	b90007f4 	str	w20, [sp, #4]
 40235d4:	52800042 	mov	w2, #0x2                   	// #2
 40235d8:	b9400021 	ldr	w1, [x1]
 40235dc:	a90203ff 	stp	xzr, x0, [sp, #32]
 40235e0:	52800020 	mov	w0, #0x1                   	// #1
 40235e4:	f240003f 	tst	x1, #0x1
 40235e8:	52800201 	mov	w1, #0x10                  	// #16
 40235ec:	1a821021 	csel	w1, w1, w2, ne  // ne = any
 40235f0:	b90033e0 	str	w0, [sp, #48]
 40235f4:	52800400 	mov	w0, #0x20                  	// #32
 40235f8:	b9000bf4 	str	w20, [sp, #8]
 40235fc:	b9000fe0 	str	w0, [sp, #12]
 4023600:	910043e0 	add	x0, sp, #0x10
 4023604:	b9001be1 	str	w1, [sp, #24]
 4023608:	910063e1 	add	x1, sp, #0x18
 402360c:	ca010002 	eor	x2, x0, x1
 4023610:	b9000bf4 	str	w20, [sp, #8]
 4023614:	9400110c 	bl	4027a44 <uapi_drv_cipher_hash_init>
 4023618:	b9000be0 	str	w0, [sp, #8]
 402361c:	b9400be0 	ldr	w0, [sp, #8]
 4023620:	7102941f 	cmp	w0, #0xa5
 4023624:	54000160 	b.eq	4023650 <calc_hash+0xb0>  // b.none
 4023628:	b9400be0 	ldr	w0, [sp, #8]
 402362c:	d0ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4023630:	b90007e0 	str	w0, [sp, #4]
 4023634:	b94007e0 	ldr	w0, [sp, #4]
 4023638:	f9401fe2 	ldr	x2, [sp, #56]
 402363c:	f9455023 	ldr	x3, [x1, #2720]
 4023640:	eb030042 	subs	x2, x2, x3
 4023644:	d2800003 	mov	x3, #0x0                   	// #0
 4023648:	54000240 	b.eq	4023690 <calc_hash+0xf0>  // b.none
 402364c:	940002c8 	bl	402416c <__stack_chk_fail>
 4023650:	f9400be0 	ldr	x0, [sp, #16]
 4023654:	9100a3e1 	add	x1, sp, #0x28
 4023658:	2a1303e2 	mov	w2, w19
 402365c:	52800003 	mov	w3, #0x0                   	// #0
 4023660:	ca000024 	eor	x4, x1, x0
 4023664:	b9000bf4 	str	w20, [sp, #8]
 4023668:	ca020084 	eor	x4, x4, x2
 402366c:	2a1303e2 	mov	w2, w19
 4023670:	940010f9 	bl	4027a54 <uapi_drv_cipher_hash_update>
 4023674:	b9000be0 	str	w0, [sp, #8]
 4023678:	f9400be0 	ldr	x0, [sp, #16]
 402367c:	910033e2 	add	x2, sp, #0xc
 4023680:	aa1503e1 	mov	x1, x21
 4023684:	d2800003 	mov	x3, #0x0                   	// #0
 4023688:	940010f7 	bl	4027a64 <uapi_drv_cipher_hash_final>
 402368c:	17ffffe7 	b	4023628 <calc_hash+0x88>
 4023690:	a94453f3 	ldp	x19, x20, [sp, #64]
 4023694:	a9457bf5 	ldp	x21, x30, [sp, #80]
 4023698:	910183ff 	add	sp, sp, #0x60
 402369c:	d65f03c0 	ret

00000000040236a0 <store_hash_to_lpds>:
 40236a0:	f81e0ffe 	str	x30, [sp, #-32]!
 40236a4:	52800b41 	mov	w1, #0x5a                  	// #90
 40236a8:	d2800403 	mov	x3, #0x20                  	// #32
 40236ac:	d2800004 	mov	x4, #0x0                   	// #0
 40236b0:	b9001fe1 	str	w1, [sp, #28]
 40236b4:	d0ffffe2 	adrp	x2, 4021000 <hash_verify_buf-0xa80>
 40236b8:	912a0042 	add	x2, x2, #0xa80
 40236bc:	b9001fe1 	str	w1, [sp, #28]
 40236c0:	aa0303e1 	mov	x1, x3
 40236c4:	940011b8 	bl	4027da4 <memcpy_ss>
 40236c8:	b9001fe0 	str	w0, [sp, #28]
 40236cc:	b9401fe0 	ldr	w0, [sp, #28]
 40236d0:	7102941f 	cmp	w0, #0xa5
 40236d4:	54000180 	b.eq	4023704 <store_hash_to_lpds+0x64>  // b.none
 40236d8:	52800ce0 	mov	w0, #0x67                  	// #103
 40236dc:	94000267 	bl	4024078 <err_print>
 40236e0:	d2800040 	mov	x0, #0x2                   	// #2
 40236e4:	9400119c 	bl	4027d54 <mdelay>
 40236e8:	9400118b 	bl	4027d14 <timer_deinit>
 40236ec:	d2800080 	mov	x0, #0x4                   	// #4
 40236f0:	52800021 	mov	w1, #0x1                   	// #1
 40236f4:	f2a22040 	movk	x0, #0x1102, lsl #16
 40236f8:	b9000001 	str	w1, [x0]
 40236fc:	b9000001 	str	w1, [x0]
 4023700:	b9000001 	str	w1, [x0]
 4023704:	f84207fe 	ldr	x30, [sp], #32
 4023708:	d65f03c0 	ret

000000000402370c <verify_signature>:
 402370c:	d10403ff 	sub	sp, sp, #0x100
 4023710:	a90c53f3 	stp	x19, x20, [sp, #192]
 4023714:	aa0003f3 	mov	x19, x0
 4023718:	d0ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402371c:	a90d5bf5 	stp	x21, x22, [sp, #208]
 4023720:	aa0103f6 	mov	x22, x1
 4023724:	a90e63f7 	stp	x23, x24, [sp, #224]
 4023728:	f9007bfe 	str	x30, [sp, #240]
 402372c:	f9455001 	ldr	x1, [x0, #2720]
 4023730:	f9005fe1 	str	x1, [sp, #184]
 4023734:	d2800001 	mov	x1, #0x0                   	// #0
 4023738:	b5000123 	cbnz	x3, 402375c <verify_signature+0x50>
 402373c:	52800b53 	mov	w19, #0x5a                  	// #90
 4023740:	d0ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4023744:	f9405fe1 	ldr	x1, [sp, #184]
 4023748:	f9455002 	ldr	x2, [x0, #2720]
 402374c:	eb020021 	subs	x1, x1, x2
 4023750:	d2800002 	mov	x2, #0x0                   	// #0
 4023754:	54000ca0 	b.eq	40238e8 <verify_signature+0x1dc>  // b.none
 4023758:	94000285 	bl	402416c <__stack_chk_fail>
 402375c:	f9400460 	ldr	x0, [x3, #8]
 4023760:	aa0303f4 	mov	x20, x3
 4023764:	b4fffec0 	cbz	x0, 402373c <verify_signature+0x30>
 4023768:	b9400075 	ldr	w21, [x3]
 402376c:	710082bf 	cmp	w21, #0x20
 4023770:	54fffe61 	b.ne	402373c <verify_signature+0x30>  // b.any
 4023774:	b4fffe53 	cbz	x19, 402373c <verify_signature+0x30>
 4023778:	9101e3f7 	add	x23, sp, #0x78
 402377c:	aa0203f8 	mov	x24, x2
 4023780:	d2800403 	mov	x3, #0x20                  	// #32
 4023784:	aa1703e0 	mov	x0, x23
 4023788:	aa0303e1 	mov	x1, x3
 402378c:	d2800004 	mov	x4, #0x0                   	// #0
 4023790:	52800b42 	mov	w2, #0x5a                  	// #90
 4023794:	94001188 	bl	4027db4 <memset_ss>
 4023798:	7102941f 	cmp	w0, #0xa5
 402379c:	54fffd01 	b.ne	402373c <verify_signature+0x30>  // b.any
 40237a0:	b9400283 	ldr	w3, [x20]
 40237a4:	d2800004 	mov	x4, #0x0                   	// #0
 40237a8:	f9400680 	ldr	x0, [x20, #8]
 40237ac:	aa0303e1 	mov	x1, x3
 40237b0:	12800b42 	mov	w2, #0xffffffa5            	// #-91
 40237b4:	94001180 	bl	4027db4 <memset_ss>
 40237b8:	7102941f 	cmp	w0, #0xa5
 40237bc:	54fffc01 	b.ne	402373c <verify_signature+0x30>  // b.any
 40237c0:	d2a203c0 	mov	x0, #0x101e0000            	// #270401536
 40237c4:	b90003f5 	str	w21, [sp]
 40237c8:	b9400000 	ldr	w0, [x0]
 40237cc:	360004a0 	tbz	w0, #0, 4023860 <verify_signature+0x154>
 40237d0:	910263e0 	add	x0, sp, #0x98
 40237d4:	f90007e0 	str	x0, [sp, #8]
 40237d8:	f0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 40237dc:	913af400 	add	x0, x0, #0xebd
 40237e0:	f9000fe0 	str	x0, [sp, #24]
 40237e4:	52800200 	mov	w0, #0x10                  	// #16
 40237e8:	b90013e0 	str	w0, [sp, #16]
 40237ec:	52800080 	mov	w0, #0x4                   	// #4
 40237f0:	b9400261 	ldr	w1, [x19]
 40237f4:	910003e3 	mov	x3, sp
 40237f8:	b90073e0 	str	w0, [sp, #112]
 40237fc:	52800020 	mov	w0, #0x1                   	// #1
 4023800:	f9400662 	ldr	x2, [x19, #8]
 4023804:	d2800004 	mov	x4, #0x0                   	// #0
 4023808:	b90023e1 	str	w1, [sp, #32]
 402380c:	aa1603e1 	mov	x1, x22
 4023810:	f90017e2 	str	x2, [sp, #40]
 4023814:	910083e2 	add	x2, sp, #0x20
 4023818:	b90033e0 	str	w0, [sp, #48]
 402381c:	910043e0 	add	x0, sp, #0x10
 4023820:	940010e9 	bl	4027bc4 <uapi_drv_pke_sm2_dsa_hash>
 4023824:	7102941f 	cmp	w0, #0xa5
 4023828:	54fff8a1 	b.ne	402373c <verify_signature+0x30>  // b.any
 402382c:	9100e3f5 	add	x21, sp, #0x38
 4023830:	b9400261 	ldr	w1, [x19]
 4023834:	aa1703e2 	mov	x2, x23
 4023838:	b9400a60 	ldr	w0, [x19, #8]
 402383c:	52800004 	mov	w4, #0x0                   	// #0
 4023840:	52800403 	mov	w3, #0x20                  	// #32
 4023844:	97ffff57 	bl	40235a0 <calc_hash>
 4023848:	2a0003f3 	mov	w19, w0
 402384c:	7102941f 	cmp	w0, #0xa5
 4023850:	54000100 	b.eq	4023870 <verify_signature+0x164>  // b.none
 4023854:	52800a40 	mov	w0, #0x52                  	// #82
 4023858:	94000208 	bl	4024078 <err_print>
 402385c:	17ffffb9 	b	4023740 <verify_signature+0x34>
 4023860:	f0000055 	adrp	x21, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4023864:	913542b5 	add	x21, x21, #0xd50
 4023868:	f90007f7 	str	x23, [sp, #8]
 402386c:	17fffff1 	b	4023830 <verify_signature+0x124>
 4023870:	d0fffff3 	adrp	x19, 4021000 <hash_verify_buf-0xa80>
 4023874:	912a0273 	add	x19, x19, #0xa80
 4023878:	d2800403 	mov	x3, #0x20                  	// #32
 402387c:	aa1303e0 	mov	x0, x19
 4023880:	aa0303e1 	mov	x1, x3
 4023884:	d2800004 	mov	x4, #0x0                   	// #0
 4023888:	52800002 	mov	w2, #0x0                   	// #0
 402388c:	9400114a 	bl	4027db4 <memset_ss>
 4023890:	7102941f 	cmp	w0, #0xa5
 4023894:	54fff541 	b.ne	402373c <verify_signature+0x30>  // b.any
 4023898:	d2800403 	mov	x3, #0x20                  	// #32
 402389c:	aa1703e2 	mov	x2, x23
 40238a0:	aa0303e1 	mov	x1, x3
 40238a4:	aa1303e0 	mov	x0, x19
 40238a8:	d2800004 	mov	x4, #0x0                   	// #0
 40238ac:	9400113e 	bl	4027da4 <memcpy_ss>
 40238b0:	7102941f 	cmp	w0, #0xa5
 40238b4:	54fff441 	b.ne	402373c <verify_signature+0x30>  // b.any
 40238b8:	f9400684 	ldr	x4, [x20, #8]
 40238bc:	aa1803e3 	mov	x3, x24
 40238c0:	910003e2 	mov	x2, sp
 40238c4:	aa1603e1 	mov	x1, x22
 40238c8:	aa1503e0 	mov	x0, x21
 40238cc:	d2800005 	mov	x5, #0x0                   	// #0
 40238d0:	940010b9 	bl	4027bb4 <uapi_drv_pke_ecdsa_verify>
 40238d4:	2a0003f3 	mov	w19, w0
 40238d8:	7102941f 	cmp	w0, #0xa5
 40238dc:	54fff320 	b.eq	4023740 <verify_signature+0x34>  // b.none
 40238e0:	52800a60 	mov	w0, #0x53                  	// #83
 40238e4:	17ffffdd 	b	4023858 <verify_signature+0x14c>
 40238e8:	2a1303e0 	mov	w0, w19
 40238ec:	a94c53f3 	ldp	x19, x20, [sp, #192]
 40238f0:	a94d5bf5 	ldp	x21, x22, [sp, #208]
 40238f4:	a94e63f7 	ldp	x23, x24, [sp, #224]
 40238f8:	f9407bfe 	ldr	x30, [sp, #240]
 40238fc:	910403ff 	add	sp, sp, #0x100
 4023900:	d65f03c0 	ret

0000000004023904 <dma_copy>:
 4023904:	d10103ff 	sub	sp, sp, #0x40
 4023908:	d0ffffe3 	adrp	x3, 4021000 <hash_verify_buf-0xa80>
 402390c:	f9001bfe 	str	x30, [sp, #48]
 4023910:	f9455064 	ldr	x4, [x3, #2720]
 4023914:	f90017e4 	str	x4, [sp, #40]
 4023918:	d2800004 	mov	x4, #0x0                   	// #0
 402391c:	f90007e0 	str	x0, [sp, #8]
 4023920:	52800020 	mov	w0, #0x1                   	// #1
 4023924:	d2800003 	mov	x3, #0x0                   	// #0
 4023928:	b90013e0 	str	w0, [sp, #16]
 402392c:	f9000fe2 	str	x2, [sp, #24]
 4023930:	2a0103e2 	mov	w2, w1
 4023934:	910063e1 	add	x1, sp, #0x18
 4023938:	b90023e0 	str	w0, [sp, #32]
 402393c:	910023e0 	add	x0, sp, #0x8
 4023940:	9400102d 	bl	40279f4 <uapi_drv_cipher_dma_copy>
 4023944:	d0ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4023948:	f94017e2 	ldr	x2, [sp, #40]
 402394c:	f9455023 	ldr	x3, [x1, #2720]
 4023950:	eb030042 	subs	x2, x2, x3
 4023954:	d2800003 	mov	x3, #0x0                   	// #0
 4023958:	54000040 	b.eq	4023960 <dma_copy+0x5c>  // b.none
 402395c:	94000204 	bl	402416c <__stack_chk_fail>
 4023960:	f9401bfe 	ldr	x30, [sp, #48]
 4023964:	910103ff 	add	sp, sp, #0x40
 4023968:	d65f03c0 	ret

000000000402396c <build_board_type_frame>:
 402396c:	f100003f 	cmp	x1, #0x0
 4023970:	7a4a1840 	ccmp	w2, #0xa, #0x0, ne  // ne = any
 4023974:	54000469 	b.ls	4023a00 <build_board_type_frame+0x94>  // b.plast
 4023978:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402397c:	aa0103f3 	mov	x19, x1
 4023980:	12800621 	mov	w1, #0xffffffce            	// #-50
 4023984:	f9000bfe 	str	x30, [sp, #16]
 4023988:	d2800014 	mov	x20, #0x0                   	// #0
 402398c:	39000261 	strb	w1, [x19]
 4023990:	52800021 	mov	w1, #0x1                   	// #1
 4023994:	39000661 	strb	w1, [x19, #1]
 4023998:	52800081 	mov	w1, #0x4                   	// #4
 402399c:	39000e61 	strb	w1, [x19, #3]
 40239a0:	53187c01 	lsr	w1, w0, #24
 40239a4:	39001261 	strb	w1, [x19, #4]
 40239a8:	53107c01 	lsr	w1, w0, #16
 40239ac:	39001661 	strb	w1, [x19, #5]
 40239b0:	53087c01 	lsr	w1, w0, #8
 40239b4:	39000a7f 	strb	wzr, [x19, #2]
 40239b8:	39001a61 	strb	w1, [x19, #6]
 40239bc:	52800001 	mov	w1, #0x0                   	// #0
 40239c0:	39001e60 	strb	w0, [x19, #7]
 40239c4:	38746a60 	ldrb	w0, [x19, x20]
 40239c8:	91000694 	add	x20, x20, #0x1
 40239cc:	940009eb 	bl	4026178 <cal_crc_perbyte>
 40239d0:	12003c01 	and	w1, w0, #0xffff
 40239d4:	f100229f 	cmp	x20, #0x8
 40239d8:	54ffff61 	b.ne	40239c4 <build_board_type_frame+0x58>  // b.any
 40239dc:	53087c20 	lsr	w0, w1, #8
 40239e0:	39002260 	strb	w0, [x19, #8]
 40239e4:	12800aa0 	mov	w0, #0xffffffaa            	// #-86
 40239e8:	39002661 	strb	w1, [x19, #9]
 40239ec:	39002a60 	strb	w0, [x19, #10]
 40239f0:	528014a0 	mov	w0, #0xa5                  	// #165
 40239f4:	f9400bfe 	ldr	x30, [sp, #16]
 40239f8:	a8c253f3 	ldp	x19, x20, [sp], #32
 40239fc:	d65f03c0 	ret
 4023a00:	52800b40 	mov	w0, #0x5a                  	// #90
 4023a04:	d65f03c0 	ret

0000000004023a08 <get_board_param_index>:
 4023a08:	d2800300 	mov	x0, #0x18                  	// #24
 4023a0c:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 4023a10:	f2a22040 	movk	x0, #0x1102, lsl #16
 4023a14:	b9400013 	ldr	w19, [x0]
 4023a18:	f0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4023a1c:	913b3800 	add	x0, x0, #0xece
 4023a20:	940010b1 	bl	4027ce4 <log_serial_puts>
 4023a24:	d35b7673 	ubfx	x19, x19, #27, #3
 4023a28:	2a1303e0 	mov	w0, w19
 4023a2c:	94000f08 	bl	402764c <serial_put_hex>
 4023a30:	2a1303e0 	mov	w0, w19
 4023a34:	a8c17bf3 	ldp	x19, x30, [sp], #16
 4023a38:	d65f03c0 	ret

0000000004023a3c <get_first_param_data_flash_offset>:
 4023a3c:	f81f0ffe 	str	x30, [sp, #-16]!
 4023a40:	9400059a 	bl	40250a8 <get_gsl_code_area_len>
 4023a44:	52830001 	mov	w1, #0x1800                	// #6144
 4023a48:	0b010000 	add	w0, w0, w1
 4023a4c:	f84107fe 	ldr	x30, [sp], #16
 4023a50:	d65f03c0 	ret

0000000004023a54 <get_uboot_info_flash_offset>:
 4023a54:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4023a58:	f9000bfe 	str	x30, [sp, #16]
 4023a5c:	940006f6 	bl	4025634 <get_ddr_param_len>
 4023a60:	2a0003f4 	mov	w20, w0
 4023a64:	94000705 	bl	4025678 <get_ddr_param_cnt>
 4023a68:	2a0003f3 	mov	w19, w0
 4023a6c:	97fffff4 	bl	4023a3c <get_first_param_data_flash_offset>
 4023a70:	f9400bfe 	ldr	x30, [sp, #16]
 4023a74:	1b130280 	madd	w0, w20, w19, w0
 4023a78:	a8c253f3 	ldp	x19, x20, [sp], #32
 4023a7c:	d65f03c0 	ret

0000000004023a80 <get_ree_key_and_paras_info_from_device>:
 4023a80:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 4023a84:	a9015bf5 	stp	x21, x22, [sp, #16]
 4023a88:	2a0103f5 	mov	w21, w1
 4023a8c:	aa0003f6 	mov	x22, x0
 4023a90:	f90013fe 	str	x30, [sp, #32]
 4023a94:	94000597 	bl	40250f0 <get_ree_key_area_addr>
 4023a98:	2a0003f3 	mov	w19, w0
 4023a9c:	94000583 	bl	40250a8 <get_gsl_code_area_len>
 4023aa0:	52828001 	mov	w1, #0x1400                	// #5120
 4023aa4:	0b010014 	add	w20, w0, w1
 4023aa8:	52896f00 	mov	w0, #0x4b78                	// #19320
 4023aac:	72b2d4a0 	movk	w0, #0x96a5, lsl #16
 4023ab0:	6b0002bf 	cmp	w21, w0
 4023ab4:	54000161 	b.ne	4023ae0 <get_ree_key_and_paras_info_from_device+0x60>  // b.any
 4023ab8:	b9400ac2 	ldr	w2, [x22, #8]
 4023abc:	52a1e000 	mov	w0, #0xf000000             	// #251658240
 4023ac0:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023ac4:	0b000042 	add	w2, w2, w0
 4023ac8:	0b140042 	add	w2, w2, w20
 4023acc:	2a1303e0 	mov	w0, w19
 4023ad0:	f94013fe 	ldr	x30, [sp, #32]
 4023ad4:	52808001 	mov	w1, #0x400                 	// #1024
 4023ad8:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023adc:	17ffff8a 	b	4023904 <dma_copy>
 4023ae0:	529690e0 	mov	w0, #0xb487                	// #46215
 4023ae4:	72ad2b40 	movk	w0, #0x695a, lsl #16
 4023ae8:	6b0002bf 	cmp	w21, w0
 4023aec:	54000141 	b.ne	4023b14 <get_ree_key_and_paras_info_from_device+0x94>  // b.any
 4023af0:	b9400ac1 	ldr	w1, [x22, #8]
 4023af4:	2a1303e0 	mov	w0, w19
 4023af8:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023afc:	0b010281 	add	w1, w20, w1
 4023b00:	f94013fe 	ldr	x30, [sp, #32]
 4023b04:	d2800023 	mov	x3, #0x1                   	// #1
 4023b08:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023b0c:	d2808002 	mov	x2, #0x400                 	// #1024
 4023b10:	14001039 	b	4027bf4 <mmc_read>
 4023b14:	528970e0 	mov	w0, #0x4b87                	// #19335
 4023b18:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 4023b1c:	6b0002bf 	cmp	w21, w0
 4023b20:	540000a1 	b.ne	4023b34 <get_ree_key_and_paras_info_from_device+0xb4>  // b.any
 4023b24:	940010b4 	bl	4027df4 <self_sdio_check>
 4023b28:	350000a0 	cbnz	w0, 4023b3c <get_ree_key_and_paras_info_from_device+0xbc>
 4023b2c:	52800800 	mov	w0, #0x40                  	// #64
 4023b30:	94000152 	bl	4024078 <err_print>
 4023b34:	52800b40 	mov	w0, #0x5a                  	// #90
 4023b38:	14000008 	b	4023b58 <get_ree_key_and_paras_info_from_device+0xd8>
 4023b3c:	94001046 	bl	4027c54 <enable_sdio_dma>
 4023b40:	2a1403e0 	mov	w0, w20
 4023b44:	94001040 	bl	4027c44 <set_sdio_pos>
 4023b48:	2a1303e0 	mov	w0, w19
 4023b4c:	d2808001 	mov	x1, #0x400                 	// #1024
 4023b50:	94001039 	bl	4027c34 <copy_from_sdio>
 4023b54:	528014a0 	mov	w0, #0xa5                  	// #165
 4023b58:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023b5c:	f94013fe 	ldr	x30, [sp, #32]
 4023b60:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023b64:	d65f03c0 	ret

0000000004023b68 <get_paras_data_from_flash>:
 4023b68:	a9b953f3 	stp	x19, x20, [sp, #-112]!
 4023b6c:	2a0203f4 	mov	w20, w2
 4023b70:	a9015bf5 	stp	x21, x22, [sp, #16]
 4023b74:	aa0003f5 	mov	x21, x0
 4023b78:	a90263f7 	stp	x23, x24, [sp, #32]
 4023b7c:	2a0103f7 	mov	w23, w1
 4023b80:	a9036bf9 	stp	x25, x26, [sp, #48]
 4023b84:	a90473fb 	stp	x27, x28, [sp, #64]
 4023b88:	f9002bfe 	str	x30, [sp, #80]
 4023b8c:	940006aa 	bl	4025634 <get_ddr_param_len>
 4023b90:	2a0003fb 	mov	w27, w0
 4023b94:	94000561 	bl	4025118 <get_ddr_param_data_addr>
 4023b98:	2a0003f8 	mov	w24, w0
 4023b9c:	940006a6 	bl	4025634 <get_ddr_param_len>
 4023ba0:	2a0003fc 	mov	w28, w0
 4023ba4:	94000558 	bl	4025104 <get_ddr_param_info_addr>
 4023ba8:	2a0003f6 	mov	w22, w0
 4023bac:	97ffffa4 	bl	4023a3c <get_first_param_data_flash_offset>
 4023bb0:	8b3642f6 	add	x22, x23, w22, uxtw
 4023bb4:	3944b2d3 	ldrb	w19, [x22, #300]
 4023bb8:	1b1c027c 	madd	w28, w19, w28, w0
 4023bbc:	52896f00 	mov	w0, #0x4b78                	// #19320
 4023bc0:	72b2d4a0 	movk	w0, #0x96a5, lsl #16
 4023bc4:	6b00029f 	cmp	w20, w0
 4023bc8:	540001c1 	b.ne	4023c00 <get_paras_data_from_flash+0x98>  // b.any
 4023bcc:	b9400aa2 	ldr	w2, [x21, #8]
 4023bd0:	52a1e000 	mov	w0, #0xf000000             	// #251658240
 4023bd4:	2a1b03e1 	mov	w1, w27
 4023bd8:	0b000042 	add	w2, w2, w0
 4023bdc:	2a1803e0 	mov	w0, w24
 4023be0:	0b1c0042 	add	w2, w2, w28
 4023be4:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023be8:	a94263f7 	ldp	x23, x24, [sp, #32]
 4023bec:	a9436bf9 	ldp	x25, x26, [sp, #48]
 4023bf0:	a94473fb 	ldp	x27, x28, [sp, #64]
 4023bf4:	f9402bfe 	ldr	x30, [sp, #80]
 4023bf8:	a8c753f3 	ldp	x19, x20, [sp], #112
 4023bfc:	17ffff42 	b	4023904 <dma_copy>
 4023c00:	529690e0 	mov	w0, #0xb487                	// #46215
 4023c04:	72ad2b40 	movk	w0, #0x695a, lsl #16
 4023c08:	6b00029f 	cmp	w20, w0
 4023c0c:	54000961 	b.ne	4023d38 <get_paras_data_from_flash+0x1d0>  // b.any
 4023c10:	b9400ab4 	ldr	w20, [x21, #8]
 4023c14:	97ffff8a 	bl	4023a3c <get_first_param_data_flash_offset>
 4023c18:	b9400aa2 	ldr	w2, [x21, #8]
 4023c1c:	2a0003f3 	mov	w19, w0
 4023c20:	b9006be2 	str	w2, [sp, #104]
 4023c24:	94000695 	bl	4025678 <get_ddr_param_cnt>
 4023c28:	2a0003f5 	mov	w21, w0
 4023c2c:	94000682 	bl	4025634 <get_ddr_param_len>
 4023c30:	2a0003f6 	mov	w22, w0
 4023c34:	94000534 	bl	4025104 <get_ddr_param_info_addr>
 4023c38:	0b160319 	add	w25, w24, w22
 4023c3c:	2a0003fa 	mov	w26, w0
 4023c40:	94000ff5 	bl	4027c14 <mmc_get_cur_mode>
 4023c44:	71016c1f 	cmp	w0, #0x5b
 4023c48:	54000420 	b.eq	4023ccc <get_paras_data_from_flash+0x164>  // b.none
 4023c4c:	b9406be2 	ldr	w2, [sp, #104]
 4023c50:	0b1c0053 	add	w19, w2, w28
 4023c54:	2a1803e0 	mov	w0, w24
 4023c58:	2a1b03e2 	mov	w2, w27
 4023c5c:	2a1303e1 	mov	w1, w19
 4023c60:	d2800023 	mov	x3, #0x1                   	// #1
 4023c64:	94000fe4 	bl	4027bf4 <mmc_read>
 4023c68:	2a0003f8 	mov	w24, w0
 4023c6c:	7102941f 	cmp	w0, #0xa5
 4023c70:	540004c1 	b.ne	4023d08 <get_paras_data_from_flash+0x1a0>  // b.any
 4023c74:	94000fe8 	bl	4027c14 <mmc_get_cur_mode>
 4023c78:	71016c1f 	cmp	w0, #0x5b
 4023c7c:	54000481 	b.ne	4023d0c <get_paras_data_from_flash+0x1a4>  // b.any
 4023c80:	8b170357 	add	x23, x26, x23
 4023c84:	510006b4 	sub	w20, w21, #0x1
 4023c88:	3944b2e0 	ldrb	w0, [x23, #300]
 4023c8c:	4b000294 	sub	w20, w20, w0
 4023c90:	7100229f 	cmp	w20, #0x8
 4023c94:	540003a8 	b.hi	4023d08 <get_paras_data_from_flash+0x1a0>  // b.pmore
 4023c98:	0b1302d3 	add	w19, w22, w19
 4023c9c:	2a1603f5 	mov	w21, w22
 4023ca0:	34000374 	cbz	w20, 4023d0c <get_paras_data_from_flash+0x1a4>
 4023ca4:	aa1503e2 	mov	x2, x21
 4023ca8:	2a1303e1 	mov	w1, w19
 4023cac:	aa1903e0 	mov	x0, x25
 4023cb0:	d2800023 	mov	x3, #0x1                   	// #1
 4023cb4:	94000fd0 	bl	4027bf4 <mmc_read>
 4023cb8:	7102941f 	cmp	w0, #0xa5
 4023cbc:	54000261 	b.ne	4023d08 <get_paras_data_from_flash+0x1a0>  // b.any
 4023cc0:	0b160273 	add	w19, w19, w22
 4023cc4:	51000694 	sub	w20, w20, #0x1
 4023cc8:	17fffff6 	b	4023ca0 <get_paras_data_from_flash+0x138>
 4023ccc:	8b170340 	add	x0, x26, x23
 4023cd0:	0b130293 	add	w19, w20, w19
 4023cd4:	2a1603fc 	mov	w28, w22
 4023cd8:	2a1903e4 	mov	w4, w25
 4023cdc:	3944b014 	ldrb	w20, [x0, #300]
 4023ce0:	2a1303e1 	mov	w1, w19
 4023ce4:	34fffb94 	cbz	w20, 4023c54 <get_paras_data_from_flash+0xec>
 4023ce8:	aa0403e0 	mov	x0, x4
 4023cec:	aa1c03e2 	mov	x2, x28
 4023cf0:	d2800023 	mov	x3, #0x1                   	// #1
 4023cf4:	f90037e4 	str	x4, [sp, #104]
 4023cf8:	94000fbf 	bl	4027bf4 <mmc_read>
 4023cfc:	7102941f 	cmp	w0, #0xa5
 4023d00:	f94037e4 	ldr	x4, [sp, #104]
 4023d04:	54000140 	b.eq	4023d2c <get_paras_data_from_flash+0x1c4>  // b.none
 4023d08:	52800b58 	mov	w24, #0x5a                  	// #90
 4023d0c:	2a1803e0 	mov	w0, w24
 4023d10:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023d14:	a94263f7 	ldp	x23, x24, [sp, #32]
 4023d18:	a9436bf9 	ldp	x25, x26, [sp, #48]
 4023d1c:	a94473fb 	ldp	x27, x28, [sp, #64]
 4023d20:	f9402bfe 	ldr	x30, [sp, #80]
 4023d24:	a8c753f3 	ldp	x19, x20, [sp], #112
 4023d28:	d65f03c0 	ret
 4023d2c:	0b160273 	add	w19, w19, w22
 4023d30:	51000694 	sub	w20, w20, #0x1
 4023d34:	17ffffeb 	b	4023ce0 <get_paras_data_from_flash+0x178>
 4023d38:	528970e0 	mov	w0, #0x4b87                	// #19335
 4023d3c:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 4023d40:	6b00029f 	cmp	w20, w0
 4023d44:	54fffe21 	b.ne	4023d08 <get_paras_data_from_flash+0x1a0>  // b.any
 4023d48:	94000fc3 	bl	4027c54 <enable_sdio_dma>
 4023d4c:	2a1c03e0 	mov	w0, w28
 4023d50:	94000fbd 	bl	4027c44 <set_sdio_pos>
 4023d54:	2a1b03e1 	mov	w1, w27
 4023d58:	2a1803e0 	mov	w0, w24
 4023d5c:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023d60:	a94263f7 	ldp	x23, x24, [sp, #32]
 4023d64:	a9436bf9 	ldp	x25, x26, [sp, #48]
 4023d68:	a94473fb 	ldp	x27, x28, [sp, #64]
 4023d6c:	f9402bfe 	ldr	x30, [sp, #80]
 4023d70:	a8c753f3 	ldp	x19, x20, [sp], #112
 4023d74:	14000fb0 	b	4027c34 <copy_from_sdio>

0000000004023d78 <get_uboot_info_from_flash>:
 4023d78:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 4023d7c:	a9015bf5 	stp	x21, x22, [sp, #16]
 4023d80:	2a0103f5 	mov	w21, w1
 4023d84:	aa0003f6 	mov	x22, x0
 4023d88:	f90013fe 	str	x30, [sp, #32]
 4023d8c:	940004e8 	bl	402512c <get_uboot_info_download_ddr_addr>
 4023d90:	aa0003f3 	mov	x19, x0
 4023d94:	97ffff30 	bl	4023a54 <get_uboot_info_flash_offset>
 4023d98:	2a0003f4 	mov	w20, w0
 4023d9c:	52896f00 	mov	w0, #0x4b78                	// #19320
 4023da0:	72b2d4a0 	movk	w0, #0x96a5, lsl #16
 4023da4:	6b0002bf 	cmp	w21, w0
 4023da8:	54000161 	b.ne	4023dd4 <get_uboot_info_from_flash+0x5c>  // b.any
 4023dac:	b9400ac2 	ldr	w2, [x22, #8]
 4023db0:	52a1e000 	mov	w0, #0xf000000             	// #251658240
 4023db4:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023db8:	0b000042 	add	w2, w2, w0
 4023dbc:	0b140042 	add	w2, w2, w20
 4023dc0:	2a1303e0 	mov	w0, w19
 4023dc4:	f94013fe 	ldr	x30, [sp, #32]
 4023dc8:	52804001 	mov	w1, #0x200                 	// #512
 4023dcc:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023dd0:	17fffecd 	b	4023904 <dma_copy>
 4023dd4:	529690e0 	mov	w0, #0xb487                	// #46215
 4023dd8:	72ad2b40 	movk	w0, #0x695a, lsl #16
 4023ddc:	6b0002bf 	cmp	w21, w0
 4023de0:	54000141 	b.ne	4023e08 <get_uboot_info_from_flash+0x90>  // b.any
 4023de4:	b9400ac1 	ldr	w1, [x22, #8]
 4023de8:	2a1303e0 	mov	w0, w19
 4023dec:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023df0:	0b010281 	add	w1, w20, w1
 4023df4:	f94013fe 	ldr	x30, [sp, #32]
 4023df8:	d2800023 	mov	x3, #0x1                   	// #1
 4023dfc:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023e00:	d2804002 	mov	x2, #0x200                 	// #512
 4023e04:	14000f7c 	b	4027bf4 <mmc_read>
 4023e08:	528970e0 	mov	w0, #0x4b87                	// #19335
 4023e0c:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 4023e10:	6b0002bf 	cmp	w21, w0
 4023e14:	54000141 	b.ne	4023e3c <get_uboot_info_from_flash+0xc4>  // b.any
 4023e18:	94000f8f 	bl	4027c54 <enable_sdio_dma>
 4023e1c:	2a1403e0 	mov	w0, w20
 4023e20:	94000f89 	bl	4027c44 <set_sdio_pos>
 4023e24:	2a1303e0 	mov	w0, w19
 4023e28:	d2804001 	mov	x1, #0x200                 	// #512
 4023e2c:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023e30:	f94013fe 	ldr	x30, [sp, #32]
 4023e34:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023e38:	14000f7f 	b	4027c34 <copy_from_sdio>
 4023e3c:	52800b40 	mov	w0, #0x5a                  	// #90
 4023e40:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023e44:	f94013fe 	ldr	x30, [sp, #32]
 4023e48:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023e4c:	d65f03c0 	ret

0000000004023e50 <get_uboot_code_from_flash>:
 4023e50:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 4023e54:	2a0103f4 	mov	w20, w1
 4023e58:	a9015bf5 	stp	x21, x22, [sp, #16]
 4023e5c:	aa0003f5 	mov	x21, x0
 4023e60:	a9027bf7 	stp	x23, x30, [sp, #32]
 4023e64:	940004b2 	bl	402512c <get_uboot_info_download_ddr_addr>
 4023e68:	aa0003f6 	mov	x22, x0
 4023e6c:	b9408817 	ldr	w23, [x0, #136]
 4023e70:	97fffef9 	bl	4023a54 <get_uboot_info_flash_offset>
 4023e74:	11080013 	add	w19, w0, #0x200
 4023e78:	52896f00 	mov	w0, #0x4b78                	// #19320
 4023e7c:	b94026d6 	ldr	w22, [x22, #36]
 4023e80:	72b2d4a0 	movk	w0, #0x96a5, lsl #16
 4023e84:	6b00029f 	cmp	w20, w0
 4023e88:	54000161 	b.ne	4023eb4 <get_uboot_code_from_flash+0x64>  // b.any
 4023e8c:	b9400aa2 	ldr	w2, [x21, #8]
 4023e90:	52a1e000 	mov	w0, #0xf000000             	// #251658240
 4023e94:	2a1603e1 	mov	w1, w22
 4023e98:	0b000042 	add	w2, w2, w0
 4023e9c:	2a1703e0 	mov	w0, w23
 4023ea0:	0b130042 	add	w2, w2, w19
 4023ea4:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023ea8:	a9427bf7 	ldp	x23, x30, [sp, #32]
 4023eac:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023eb0:	17fffe95 	b	4023904 <dma_copy>
 4023eb4:	529690e0 	mov	w0, #0xb487                	// #46215
 4023eb8:	72ad2b40 	movk	w0, #0x695a, lsl #16
 4023ebc:	6b00029f 	cmp	w20, w0
 4023ec0:	54000141 	b.ne	4023ee8 <get_uboot_code_from_flash+0x98>  // b.any
 4023ec4:	b9400aa1 	ldr	w1, [x21, #8]
 4023ec8:	2a1603e2 	mov	w2, w22
 4023ecc:	2a1703e0 	mov	w0, w23
 4023ed0:	d2800023 	mov	x3, #0x1                   	// #1
 4023ed4:	0b010261 	add	w1, w19, w1
 4023ed8:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023edc:	a9427bf7 	ldp	x23, x30, [sp, #32]
 4023ee0:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023ee4:	14000f44 	b	4027bf4 <mmc_read>
 4023ee8:	528970e0 	mov	w0, #0x4b87                	// #19335
 4023eec:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 4023ef0:	6b00029f 	cmp	w20, w0
 4023ef4:	54000141 	b.ne	4023f1c <get_uboot_code_from_flash+0xcc>  // b.any
 4023ef8:	94000f57 	bl	4027c54 <enable_sdio_dma>
 4023efc:	2a1303e0 	mov	w0, w19
 4023f00:	94000f51 	bl	4027c44 <set_sdio_pos>
 4023f04:	2a1603e1 	mov	w1, w22
 4023f08:	2a1703e0 	mov	w0, w23
 4023f0c:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023f10:	a9427bf7 	ldp	x23, x30, [sp, #32]
 4023f14:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023f18:	14000f47 	b	4027c34 <copy_from_sdio>
 4023f1c:	52800b40 	mov	w0, #0x5a                  	// #90
 4023f20:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4023f24:	a9427bf7 	ldp	x23, x30, [sp, #32]
 4023f28:	a8c353f3 	ldp	x19, x20, [sp], #48
 4023f2c:	d65f03c0 	ret

0000000004023f30 <get_random_num>:
 4023f30:	d29c2101 	mov	x1, #0xe108                	// #57608
 4023f34:	f2a203c1 	movk	x1, #0x101e, lsl #16
 4023f38:	b9400020 	ldr	w0, [x1]
 4023f3c:	f240101f 	tst	x0, #0x1f
 4023f40:	54ffffc0 	b.eq	4023f38 <get_random_num+0x8>  // b.none
 4023f44:	d29c2000 	mov	x0, #0xe100                	// #57600
 4023f48:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4023f4c:	b9400000 	ldr	w0, [x0]
 4023f50:	d65f03c0 	ret

0000000004023f54 <failure_process>:
 4023f54:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4023f58:	2a0003f4 	mov	w20, w0
 4023f5c:	f9000bfe 	str	x30, [sp, #16]
 4023f60:	97fffc63 	bl	40230ec <get_low_power_mode>
 4023f64:	7101681f 	cmp	w0, #0x5a
 4023f68:	54000081 	b.ne	4023f78 <failure_process+0x24>  // b.any
 4023f6c:	97fffc65 	bl	4023100 <clear_lpds>
 4023f70:	52800000 	mov	w0, #0x0                   	// #0
 4023f74:	1400000d 	b	4023fa8 <failure_process+0x54>
 4023f78:	d2800280 	mov	x0, #0x14                  	// #20
 4023f7c:	f2a22040 	movk	x0, #0x1102, lsl #16
 4023f80:	b9400000 	ldr	w0, [x0]
 4023f84:	72001c1f 	tst	w0, #0xff
 4023f88:	540003e1 	b.ne	4024004 <failure_process+0xb0>  // b.any
 4023f8c:	94000946 	bl	40264a4 <opt_get_boot_backup_enable>
 4023f90:	2a0003f3 	mov	w19, w0
 4023f94:	7100041f 	cmp	w0, #0x1
 4023f98:	54000361 	b.ne	4024004 <failure_process+0xb0>  // b.any
 4023f9c:	97fffc32 	bl	4023064 <get_verify_backup_img_reg>
 4023fa0:	35000320 	cbnz	w0, 4024004 <failure_process+0xb0>
 4023fa4:	2a1303e0 	mov	w0, w19
 4023fa8:	97fffc29 	bl	402304c <set_verify_backup_img_reg>
 4023fac:	97fffc1d 	bl	4023020 <clean_boot_img_addr_size>
 4023fb0:	f0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4023fb4:	913bec00 	add	x0, x0, #0xefb
 4023fb8:	94000f4b 	bl	4027ce4 <log_serial_puts>
 4023fbc:	94000919 	bl	4026420 <is_sec_dbg_enable>
 4023fc0:	5292cb41 	mov	w1, #0x965a                	// #38490
 4023fc4:	72a794a1 	movk	w1, #0x3ca5, lsl #16
 4023fc8:	6b01001f 	cmp	w0, w1
 4023fcc:	54000041 	b.ne	4023fd4 <failure_process+0x80>  // b.any
 4023fd0:	14000000 	b	4023fd0 <failure_process+0x7c>
 4023fd4:	d2800040 	mov	x0, #0x2                   	// #2
 4023fd8:	94000f5f 	bl	4027d54 <mdelay>
 4023fdc:	94000f4e 	bl	4027d14 <timer_deinit>
 4023fe0:	d2800080 	mov	x0, #0x4                   	// #4
 4023fe4:	52800021 	mov	w1, #0x1                   	// #1
 4023fe8:	f2a22040 	movk	x0, #0x1102, lsl #16
 4023fec:	f9400bfe 	ldr	x30, [sp, #16]
 4023ff0:	b9000001 	str	w1, [x0]
 4023ff4:	b9000001 	str	w1, [x0]
 4023ff8:	b9000001 	str	w1, [x0]
 4023ffc:	a8c253f3 	ldp	x19, x20, [sp], #32
 4024000:	d65f03c0 	ret
 4024004:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4024008:	913bac00 	add	x0, x0, #0xeeb
 402400c:	94000f36 	bl	4027ce4 <log_serial_puts>
 4024010:	d2902e02 	mov	x2, #0x8170                	// #33136
 4024014:	f2a203c2 	movk	x2, #0x101e, lsl #16
 4024018:	b9400041 	ldr	w1, [x2]
 402401c:	121c6c21 	and	w1, w1, #0xfffffff0
 4024020:	2a140020 	orr	w0, w1, w20
 4024024:	b9000040 	str	w0, [x2]
 4024028:	17ffffd2 	b	4023f70 <failure_process+0x1c>

000000000402402c <stack_chk_guard_setup>:
 402402c:	a9bd7bfd 	stp	x29, x30, [sp, #-48]!
 4024030:	910003fd 	mov	x29, sp
 4024034:	9100b3e0 	add	x0, sp, #0x2c
 4024038:	f9000bf3 	str	x19, [sp, #16]
 402403c:	b9002fff 	str	wzr, [sp, #44]
 4024040:	94000e8d 	bl	4027a74 <uapi_drv_cipher_trng_get_random>
 4024044:	b9402fe0 	ldr	w0, [sp, #44]
 4024048:	b0fffff3 	adrp	x19, 4021000 <hash_verify_buf-0xa80>
 402404c:	d3607c00 	lsl	x0, x0, #32
 4024050:	f9055260 	str	x0, [x19, #2720]
 4024054:	9100b3e0 	add	x0, sp, #0x2c
 4024058:	94000e87 	bl	4027a74 <uapi_drv_cipher_trng_get_random>
 402405c:	f9455261 	ldr	x1, [x19, #2720]
 4024060:	b9402fe0 	ldr	w0, [sp, #44]
 4024064:	aa010000 	orr	x0, x0, x1
 4024068:	f9055260 	str	x0, [x19, #2720]
 402406c:	f9400bf3 	ldr	x19, [sp, #16]
 4024070:	a8c37bfd 	ldp	x29, x30, [sp], #48
 4024074:	d65f03c0 	ret

0000000004024078 <err_print>:
 4024078:	d10083ff 	sub	sp, sp, #0x20
 402407c:	b0ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4024080:	a9017bf3 	stp	x19, x30, [sp, #16]
 4024084:	f9455022 	ldr	x2, [x1, #2720]
 4024088:	f90007e2 	str	x2, [sp, #8]
 402408c:	d2800002 	mov	x2, #0x0                   	// #0
 4024090:	7219001f 	tst	w0, #0x80
 4024094:	54000481 	b.ne	4024124 <err_print+0xac>  // b.any
 4024098:	12001c04 	and	w4, w0, #0xff
 402409c:	52800143 	mov	w3, #0xa                   	// #10
 40240a0:	52800c82 	mov	w2, #0x64                  	// #100
 40240a4:	39000fff 	strb	wzr, [sp, #3]
 40240a8:	1ac30880 	udiv	w0, w4, w3
 40240ac:	1ac20882 	udiv	w2, w4, w2
 40240b0:	1ac30801 	udiv	w1, w0, w3
 40240b4:	1100c042 	add	w2, w2, #0x30
 40240b8:	12001c42 	and	w2, w2, #0xff
 40240bc:	390003e2 	strb	w2, [sp]
 40240c0:	1b038021 	msub	w1, w1, w3, w0
 40240c4:	1b039000 	msub	w0, w0, w3, w4
 40240c8:	1100c021 	add	w1, w1, #0x30
 40240cc:	1100c000 	add	w0, w0, #0x30
 40240d0:	12001c21 	and	w1, w1, #0xff
 40240d4:	12001c00 	and	w0, w0, #0xff
 40240d8:	390007e1 	strb	w1, [sp, #1]
 40240dc:	39000be0 	strb	w0, [sp, #2]
 40240e0:	7100c05f 	cmp	w2, #0x30
 40240e4:	540002e1 	b.ne	4024140 <err_print+0xc8>  // b.any
 40240e8:	7100c03f 	cmp	w1, #0x30
 40240ec:	540002e1 	b.ne	4024148 <err_print+0xd0>  // b.any
 40240f0:	7100c01f 	cmp	w0, #0x30
 40240f4:	1a9f17f3 	cset	w19, eq  // eq = none
 40240f8:	11000a73 	add	w19, w19, #0x2
 40240fc:	52800141 	mov	w1, #0xa                   	// #10
 4024100:	52a22080 	mov	w0, #0x11040000            	// #285474816
 4024104:	94000d07 	bl	4027520 <serial_putc>
 4024108:	71000e7f 	cmp	w19, #0x3
 402410c:	54000221 	b.ne	4024150 <err_print+0xd8>  // b.any
 4024110:	52800601 	mov	w1, #0x30                  	// #48
 4024114:	52a22080 	mov	w0, #0x11040000            	// #285474816
 4024118:	94000d02 	bl	4027520 <serial_putc>
 402411c:	d2800140 	mov	x0, #0xa                   	// #10
 4024120:	94000f0d 	bl	4027d54 <mdelay>
 4024124:	b0ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4024128:	f94007e1 	ldr	x1, [sp, #8]
 402412c:	f9455002 	ldr	x2, [x0, #2720]
 4024130:	eb020021 	subs	x1, x1, x2
 4024134:	d2800002 	mov	x2, #0x0                   	// #0
 4024138:	54000140 	b.eq	4024160 <err_print+0xe8>  // b.none
 402413c:	9400000c 	bl	402416c <__stack_chk_fail>
 4024140:	52800013 	mov	w19, #0x0                   	// #0
 4024144:	17ffffee 	b	40240fc <err_print+0x84>
 4024148:	52800033 	mov	w19, #0x1                   	// #1
 402414c:	17ffffec 	b	40240fc <err_print+0x84>
 4024150:	92401e60 	and	x0, x19, #0xff
 4024154:	8b2063e0 	add	x0, sp, x0
 4024158:	94000ee3 	bl	4027ce4 <log_serial_puts>
 402415c:	17fffff0 	b	402411c <err_print+0xa4>
 4024160:	a9417bf3 	ldp	x19, x30, [sp, #16]
 4024164:	910083ff 	add	sp, sp, #0x20
 4024168:	d65f03c0 	ret

000000000402416c <__stack_chk_fail>:
 402416c:	a9bf7bfd 	stp	x29, x30, [sp, #-16]!
 4024170:	528007c0 	mov	w0, #0x3e                  	// #62
 4024174:	910003fd 	mov	x29, sp
 4024178:	97ffffc0 	bl	4024078 <err_print>
 402417c:	d2800040 	mov	x0, #0x2                   	// #2
 4024180:	94000ef5 	bl	4027d54 <mdelay>
 4024184:	94000ee4 	bl	4027d14 <timer_deinit>
 4024188:	d2800080 	mov	x0, #0x4                   	// #4
 402418c:	52800021 	mov	w1, #0x1                   	// #1
 4024190:	f2a22040 	movk	x0, #0x1102, lsl #16
 4024194:	b9000001 	str	w1, [x0]
 4024198:	b9000001 	str	w1, [x0]
 402419c:	b9000001 	str	w1, [x0]
 40241a0:	a8c17bfd 	ldp	x29, x30, [sp], #16
 40241a4:	d65f03c0 	ret

00000000040241a8 <first_recv_len_check>:
 40241a8:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 40241ac:	aa0003f3 	mov	x19, x0
 40241b0:	940003d5 	bl	4025104 <get_ddr_param_info_addr>
 40241b4:	2a0003e2 	mov	w2, w0
 40241b8:	29440041 	ldp	w1, w0, [x2, #32]
 40241bc:	0b000021 	add	w1, w1, w0
 40241c0:	110c0021 	add	w1, w1, #0x300
 40241c4:	eb21c27f 	cmp	x19, w1, sxtw
 40241c8:	540000c0 	b.eq	40241e0 <first_recv_len_check+0x38>  // b.none
 40241cc:	52800860 	mov	w0, #0x43                  	// #67
 40241d0:	97ffffaa 	bl	4024078 <err_print>
 40241d4:	52800b40 	mov	w0, #0x5a                  	// #90
 40241d8:	a8c17bf3 	ldp	x19, x30, [sp], #16
 40241dc:	d65f03c0 	ret
 40241e0:	528014a0 	mov	w0, #0xa5                  	// #165
 40241e4:	17fffffd 	b	40241d8 <first_recv_len_check+0x30>

00000000040241e8 <sram_to_npu_info>:
 40241e8:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 40241ec:	913c2000 	add	x0, x0, #0xf08
 40241f0:	14000ebd 	b	4027ce4 <log_serial_puts>

00000000040241f4 <sys_watchdog_enable>:
 40241f4:	5298d802 	mov	w2, #0xc6c0                	// #50880
 40241f8:	52807d01 	mov	w1, #0x3e8                 	// #1000
 40241fc:	72a005a2 	movk	w2, #0x2d, lsl #16
 4024200:	52800000 	mov	w0, #0x0                   	// #0
 4024204:	14000ed8 	b	4027d64 <watchdog_enable>

0000000004024208 <main_entry>:
 4024208:	d10503ff 	sub	sp, sp, #0x140
 402420c:	b0ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4024210:	a90f53f3 	stp	x19, x20, [sp, #240]
 4024214:	a9105bf5 	stp	x21, x22, [sp, #256]
 4024218:	a91163f7 	stp	x23, x24, [sp, #272]
 402421c:	a9126bf9 	stp	x25, x26, [sp, #288]
 4024220:	f9009bfe 	str	x30, [sp, #304]
 4024224:	f9455001 	ldr	x1, [x0, #2720]
 4024228:	f90077e1 	str	x1, [sp, #232]
 402422c:	d2800001 	mov	x1, #0x0                   	// #0
 4024230:	94000eed 	bl	4027de4 <save_cur_point_syscnt>
 4024234:	92a06001 	mov	x1, #0xfffffffffcffffff    	// #-50331649
 4024238:	52a06000 	mov	w0, #0x3000000             	// #50331648
 402423c:	94000e46 	bl	4027b54 <uapi_drv_klad_rootkey_disable>
 4024240:	92800061 	mov	x1, #0xfffffffffffffffc    	// #-4
 4024244:	52800060 	mov	w0, #0x3                   	// #3
 4024248:	f2bf9fe1 	movk	x1, #0xfcff, lsl #16
 402424c:	72a06000 	movk	w0, #0x300, lsl #16
 4024250:	94000e41 	bl	4027b54 <uapi_drv_klad_rootkey_disable>
 4024254:	94000399 	bl	40250b8 <get_gsl_heap_addr>
 4024258:	52900001 	mov	w1, #0x8000                	// #32768
 402425c:	940007d5 	bl	40261b0 <malloc_init>
 4024260:	94000ea9 	bl	4027d04 <timer_init>
 4024264:	a9037fff 	stp	xzr, xzr, [sp, #48]
 4024268:	d0000000 	adrp	x0, 4026000 <handle_atf_code+0x14c>
 402426c:	910a9000 	add	x0, x0, #0x2a4
 4024270:	f9000fe0 	str	x0, [sp, #24]
 4024274:	d0000000 	adrp	x0, 4026000 <handle_atf_code+0x14c>
 4024278:	910c8000 	add	x0, x0, #0x320
 402427c:	f90013e0 	str	x0, [sp, #32]
 4024280:	f0000000 	adrp	x0, 4027000 <start_svb+0x81c>
 4024284:	91148000 	add	x0, x0, #0x520
 4024288:	f90017e0 	str	x0, [sp, #40]
 402428c:	910063e0 	add	x0, sp, #0x18
 4024290:	f90023ff 	str	xzr, [sp, #64]
 4024294:	94000e50 	bl	4027bd4 <uapi_drv_register_func>
 4024298:	94000dc7 	bl	40279b4 <uapi_drv_cipher_init>
 402429c:	97fffddb 	bl	4023a08 <get_board_param_index>
 40242a0:	2a0003f4 	mov	w20, w0
 40242a4:	d2806380 	mov	x0, #0x31c                 	// #796
 40242a8:	f2a22040 	movk	x0, #0x1102, lsl #16
 40242ac:	b9400013 	ldr	w19, [x0]
 40242b0:	52968f00 	mov	w0, #0xb478                	// #46200
 40242b4:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 40242b8:	6b00027f 	cmp	w19, w0
 40242bc:	54000200 	b.eq	40242fc <main_entry+0xf4>  // b.none
 40242c0:	529690e0 	mov	w0, #0xb487                	// #46215
 40242c4:	540006e8 	b.hi	40243a0 <main_entry+0x198>  // b.pmore
 40242c8:	72ad2b40 	movk	w0, #0x695a, lsl #16
 40242cc:	6b00027f 	cmp	w19, w0
 40242d0:	54000160 	b.eq	40242fc <main_entry+0xf4>  // b.none
 40242d4:	528970e0 	mov	w0, #0x4b87                	// #19335
 40242d8:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 40242dc:	6b00027f 	cmp	w19, w0
 40242e0:	540000e0 	b.eq	40242fc <main_entry+0xf4>  // b.none
 40242e4:	52896f13 	mov	w19, #0x4b78                	// #19320
 40242e8:	528007e0 	mov	w0, #0x3f                  	// #63
 40242ec:	72b2cb53 	movk	w19, #0x965a, lsl #16
 40242f0:	97ffff62 	bl	4024078 <err_print>
 40242f4:	52800040 	mov	w0, #0x2                   	// #2
 40242f8:	97ffff17 	bl	4023f54 <failure_process>
 40242fc:	52968f17 	mov	w23, #0xb478                	// #46200
 4024300:	529690e0 	mov	w0, #0xb487                	// #46215
 4024304:	72ad34b7 	movk	w23, #0x69a5, lsl #16
 4024308:	6b17027f 	cmp	w19, w23
 402430c:	1a9f17f6 	cset	w22, eq  // eq = none
 4024310:	72b2cb40 	movk	w0, #0x965a, lsl #16
 4024314:	6b00027f 	cmp	w19, w0
 4024318:	1a9f17e0 	cset	w0, eq  // eq = none
 402431c:	2a0002d6 	orr	w22, w22, w0
 4024320:	52896f00 	mov	w0, #0x4b78                	// #19320
 4024324:	72b2d4a0 	movk	w0, #0x96a5, lsl #16
 4024328:	6b00027f 	cmp	w19, w0
 402432c:	529690e0 	mov	w0, #0xb487                	// #46215
 4024330:	1a9f17f5 	cset	w21, eq  // eq = none
 4024334:	72ad2b40 	movk	w0, #0x695a, lsl #16
 4024338:	6b00027f 	cmp	w19, w0
 402433c:	1a9f17e0 	cset	w0, eq  // eq = none
 4024340:	2a0002b5 	orr	w21, w21, w0
 4024344:	34002e76 	cbz	w22, 4024910 <main_entry+0x708>
 4024348:	52800000 	mov	w0, #0x0                   	// #0
 402434c:	94000e8a 	bl	4027d74 <watchdog_disable>
 4024350:	6b17027f 	cmp	w19, w23
 4024354:	54000321 	b.ne	40243b8 <main_entry+0x1b0>  // b.any
 4024358:	2a1403e0 	mov	w0, w20
 402435c:	94000c0c 	bl	402738c <send_board_type_to_uart>
 4024360:	7102941f 	cmp	w0, #0xa5
 4024364:	54000060 	b.eq	4024370 <main_entry+0x168>  // b.none
 4024368:	52800060 	mov	w0, #0x3                   	// #3
 402436c:	97fffefa 	bl	4023f54 <failure_process>
 4024370:	94000360 	bl	40250f0 <get_ree_key_area_addr>
 4024374:	2a0003e0 	mov	w0, w0
 4024378:	910063e1 	add	x1, sp, #0x18
 402437c:	52880002 	mov	w2, #0x4000                	// #16384
 4024380:	94000e5d 	bl	4027cf4 <copy_from_uart>
 4024384:	7102941f 	cmp	w0, #0xa5
 4024388:	54000a60 	b.eq	40244d4 <main_entry+0x2cc>  // b.none
 402438c:	52800820 	mov	w0, #0x41                  	// #65
 4024390:	97ffff3a 	bl	4024078 <err_print>
 4024394:	52800060 	mov	w0, #0x3                   	// #3
 4024398:	97fffeef 	bl	4023f54 <failure_process>
 402439c:	14000052 	b	40244e4 <main_entry+0x2dc>
 40243a0:	72b2cb40 	movk	w0, #0x965a, lsl #16
 40243a4:	6b00027f 	cmp	w19, w0
 40243a8:	54fffaa0 	b.eq	40242fc <main_entry+0xf4>  // b.none
 40243ac:	52896f00 	mov	w0, #0x4b78                	// #19320
 40243b0:	72b2d4a0 	movk	w0, #0x96a5, lsl #16
 40243b4:	17ffffca 	b	40242dc <main_entry+0xd4>
 40243b8:	52800b40 	mov	w0, #0x5a                  	// #90
 40243bc:	d2800203 	mov	x3, #0x10                  	// #16
 40243c0:	52800002 	mov	w2, #0x0                   	// #0
 40243c4:	aa0303e1 	mov	x1, x3
 40243c8:	b9000fe0 	str	w0, [sp, #12]
 40243cc:	910063e0 	add	x0, sp, #0x18
 40243d0:	f9000bff 	str	xzr, [sp, #16]
 40243d4:	94000e80 	bl	4027dd4 <memset_s>
 40243d8:	35000780 	cbnz	w0, 40244c8 <main_entry+0x2c0>
 40243dc:	910363f9 	add	x25, sp, #0xd8
 40243e0:	52a00038 	mov	w24, #0x10000               	// #65536
 40243e4:	94000e28 	bl	4027c84 <usb3_driver_init>
 40243e8:	910043e1 	add	x1, sp, #0x10
 40243ec:	aa1903e0 	mov	x0, x25
 40243f0:	528001c2 	mov	w2, #0xe                   	// #14
 40243f4:	94000e20 	bl	4027c74 <copy_from_usb>
 40243f8:	b9000fe0 	str	w0, [sp, #12]
 40243fc:	b9400fe0 	ldr	w0, [sp, #12]
 4024400:	7102941f 	cmp	w0, #0xa5
 4024404:	54000581 	b.ne	40244b4 <main_entry+0x2ac>  // b.any
 4024408:	f9400be0 	ldr	x0, [sp, #16]
 402440c:	f100381f 	cmp	x0, #0xe
 4024410:	54000521 	b.ne	40244b4 <main_entry+0x2ac>  // b.any
 4024414:	394363e0 	ldrb	w0, [sp, #216]
 4024418:	7103381f 	cmp	w0, #0xce
 402441c:	540004c1 	b.ne	40244b4 <main_entry+0x2ac>  // b.any
 4024420:	39436be0 	ldrb	w0, [sp, #218]
 4024424:	394367e1 	ldrb	w1, [sp, #217]
 4024428:	2a2003e0 	mvn	w0, w0
 402442c:	6b20003f 	cmp	w1, w0, uxtb
 4024430:	54000421 	b.ne	40244b4 <main_entry+0x2ac>  // b.any
 4024434:	7941cbf7 	ldrh	w23, [sp, #228]
 4024438:	d280001a 	mov	x26, #0x0                   	// #0
 402443c:	52800001 	mov	w1, #0x0                   	// #0
 4024440:	5ac006f7 	rev16	w23, w23
 4024444:	12003ef7 	and	w23, w23, #0xffff
 4024448:	387a6b20 	ldrb	w0, [x25, x26]
 402444c:	9100075a 	add	x26, x26, #0x1
 4024450:	9400074a 	bl	4026178 <cal_crc_perbyte>
 4024454:	12003c01 	and	w1, w0, #0xffff
 4024458:	f100335f 	cmp	x26, #0xc
 402445c:	54ffff61 	b.ne	4024448 <main_entry+0x240>  // b.any
 4024460:	6b0102ff 	cmp	w23, w1
 4024464:	54000281 	b.ne	40244b4 <main_entry+0x2ac>  // b.any
 4024468:	910323e1 	add	x1, sp, #0xc8
 402446c:	52800162 	mov	w2, #0xb                   	// #11
 4024470:	2a1403e0 	mov	w0, w20
 4024474:	97fffd3e 	bl	402396c <build_board_type_frame>
 4024478:	d2800161 	mov	x1, #0xb                   	// #11
 402447c:	910323e0 	add	x0, sp, #0xc8
 4024480:	94000e05 	bl	4027c94 <send_to_usb>
 4024484:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4024488:	913ca000 	add	x0, x0, #0xf28
 402448c:	94000e16 	bl	4027ce4 <log_serial_puts>
 4024490:	94000318 	bl	40250f0 <get_ree_key_area_addr>
 4024494:	2a0003e0 	mov	w0, w0
 4024498:	910063e1 	add	x1, sp, #0x18
 402449c:	52880002 	mov	w2, #0x4000                	// #16384
 40244a0:	94000df5 	bl	4027c74 <copy_from_usb>
 40244a4:	7102941f 	cmp	w0, #0xa5
 40244a8:	54000160 	b.eq	40244d4 <main_entry+0x2cc>  // b.none
 40244ac:	52800840 	mov	w0, #0x42                  	// #66
 40244b0:	17ffffb8 	b	4024390 <main_entry+0x188>
 40244b4:	71000718 	subs	w24, w24, #0x1
 40244b8:	54fff981 	b.ne	40243e8 <main_entry+0x1e0>  // b.any
 40244bc:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 40244c0:	913c5c00 	add	x0, x0, #0xf17
 40244c4:	94000e08 	bl	4027ce4 <log_serial_puts>
 40244c8:	52800060 	mov	w0, #0x3                   	// #3
 40244cc:	97fffea2 	bl	4023f54 <failure_process>
 40244d0:	17fffff0 	b	4024490 <main_entry+0x288>
 40244d4:	f9400fe0 	ldr	x0, [sp, #24]
 40244d8:	97ffff34 	bl	40241a8 <first_recv_len_check>
 40244dc:	7102941f 	cmp	w0, #0xa5
 40244e0:	54fff5a1 	b.ne	4024394 <main_entry+0x18c>  // b.any
 40244e4:	97ffff44 	bl	40241f4 <sys_watchdog_enable>
 40244e8:	940003f5 	bl	40254bc <handle_ree_key_area>
 40244ec:	7102941f 	cmp	w0, #0xa5
 40244f0:	54000120 	b.eq	4024514 <main_entry+0x30c>  // b.none
 40244f4:	52800d00 	mov	w0, #0x68                  	// #104
 40244f8:	97fffee0 	bl	4024078 <err_print>
 40244fc:	528000c0 	mov	w0, #0x6                   	// #6
 4024500:	97fffe95 	bl	4023f54 <failure_process>
 4024504:	52800d00 	mov	w0, #0x68                  	// #104
 4024508:	97fffedc 	bl	4024078 <err_print>
 402450c:	528000c0 	mov	w0, #0x6                   	// #6
 4024510:	97fffe91 	bl	4023f54 <failure_process>
 4024514:	2a1403e0 	mov	w0, w20
 4024518:	9400045e 	bl	4025690 <handle_ddr_param_info>
 402451c:	7102941f 	cmp	w0, #0xa5
 4024520:	54000120 	b.eq	4024544 <main_entry+0x33c>  // b.none
 4024524:	52800d20 	mov	w0, #0x69                  	// #105
 4024528:	97fffed4 	bl	4024078 <err_print>
 402452c:	528000e0 	mov	w0, #0x7                   	// #7
 4024530:	97fffe89 	bl	4023f54 <failure_process>
 4024534:	52800d20 	mov	w0, #0x69                  	// #105
 4024538:	97fffed0 	bl	4024078 <err_print>
 402453c:	528000e0 	mov	w0, #0x7                   	// #7
 4024540:	97fffe85 	bl	4023f54 <failure_process>
 4024544:	350000b5 	cbnz	w21, 4024558 <main_entry+0x350>
 4024548:	528970e0 	mov	w0, #0x4b87                	// #19335
 402454c:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 4024550:	6b00027f 	cmp	w19, w0
 4024554:	54000161 	b.ne	4024580 <main_entry+0x378>  // b.any
 4024558:	b0ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402455c:	912a8000 	add	x0, x0, #0xaa0
 4024560:	2a1303e2 	mov	w2, w19
 4024564:	2a1403e1 	mov	w1, w20
 4024568:	91002000 	add	x0, x0, #0x8
 402456c:	97fffd7f 	bl	4023b68 <get_paras_data_from_flash>
 4024570:	7102941f 	cmp	w0, #0xa5
 4024574:	54000060 	b.eq	4024580 <main_entry+0x378>  // b.none
 4024578:	528000e0 	mov	w0, #0x7                   	// #7
 402457c:	97fffe76 	bl	4023f54 <failure_process>
 4024580:	2a1403e0 	mov	w0, w20
 4024584:	940004cb 	bl	40258b0 <handle_ddr_param>
 4024588:	7102941f 	cmp	w0, #0xa5
 402458c:	54000120 	b.eq	40245b0 <main_entry+0x3a8>  // b.none
 4024590:	52800d40 	mov	w0, #0x6a                  	// #106
 4024594:	97fffeb9 	bl	4024078 <err_print>
 4024598:	528000e0 	mov	w0, #0x7                   	// #7
 402459c:	97fffe6e 	bl	4023f54 <failure_process>
 40245a0:	52800d40 	mov	w0, #0x6a                  	// #106
 40245a4:	97fffeb5 	bl	4024078 <err_print>
 40245a8:	528000e0 	mov	w0, #0x7                   	// #7
 40245ac:	97fffe6a 	bl	4023f54 <failure_process>
 40245b0:	2a1303e0 	mov	w0, w19
 40245b4:	94000e71 	bl	4027f78 <bsp_gpll_config>
 40245b8:	940002d8 	bl	4025118 <get_ddr_param_data_addr>
 40245bc:	2a0003f7 	mov	w23, w0
 40245c0:	d2918560 	mov	x0, #0x8c2b                	// #35883
 40245c4:	aa1703f4 	mov	x20, x23
 40245c8:	f2a34dc0 	movk	x0, #0x1a6e, lsl #16
 40245cc:	f94002e1 	ldr	x1, [x23]
 40245d0:	f2cdc340 	movk	x0, #0x6e1a, lsl #32
 40245d4:	f2e57180 	movk	x0, #0x2b8c, lsl #48
 40245d8:	eb00003f 	cmp	x1, x0
 40245dc:	54000221 	b.ne	4024620 <main_entry+0x418>  // b.any
 40245e0:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 40245e4:	913d0c00 	add	x0, x0, #0xf43
 40245e8:	94000dbf 	bl	4027ce4 <log_serial_puts>
 40245ec:	1102e2f4 	add	w20, w23, #0xb8
 40245f0:	910022e0 	add	x0, x23, #0x8
 40245f4:	94000dbc 	bl	4027ce4 <log_serial_puts>
 40245f8:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 40245fc:	913d7400 	add	x0, x0, #0xf5d
 4024600:	94000db9 	bl	4027ce4 <log_serial_puts>
 4024604:	910062e0 	add	x0, x23, #0x18
 4024608:	94000db7 	bl	4027ce4 <log_serial_puts>
 402460c:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4024610:	913ddc00 	add	x0, x0, #0xf77
 4024614:	94000db4 	bl	4027ce4 <log_serial_puts>
 4024618:	9100e2e0 	add	x0, x23, #0x38
 402461c:	94000db2 	bl	4027ce4 <log_serial_puts>
 4024620:	2a1403e0 	mov	w0, w20
 4024624:	52800001 	mov	w1, #0x0                   	// #0
 4024628:	94000ad6 	bl	4027180 <init_registers>
 402462c:	52800034 	mov	w20, #0x1                   	// #1
 4024630:	d2884400 	mov	x0, #0x4220                	// #16928
 4024634:	52800141 	mov	w1, #0xa                   	// #10
 4024638:	f2a22040 	movk	x0, #0x1102, lsl #16
 402463c:	b9000001 	str	w1, [x0]
 4024640:	b900041f 	str	wzr, [x0, #4]
 4024644:	b9000414 	str	w20, [x0, #4]
 4024648:	94000867 	bl	40267e4 <start_svb>
 402464c:	2a1403e2 	mov	w2, w20
 4024650:	12800001 	mov	w1, #0xffffffff            	// #-1
 4024654:	2a0103e0 	mov	w0, w1
 4024658:	94000b12 	bl	40272a0 <bsp_ddrtrn_training_item>
 402465c:	94000b10 	bl	402729c <bsp_ddrtrn_retrain_enable>
 4024660:	94000b4a 	bl	4027388 <bsp_ddrtrn_dmc_auto_power_down_cfg>
 4024664:	d2802d00 	mov	x0, #0x168                 	// #360
 4024668:	f2a22040 	movk	x0, #0x1102, lsl #16
 402466c:	b9400000 	ldr	w0, [x0]
 4024670:	12003c00 	and	w0, w0, #0xffff
 4024674:	7104001f 	cmp	w0, #0x100
 4024678:	540000e9 	b.ls	4024694 <main_entry+0x48c>  // b.plast
 402467c:	94000b2f 	bl	4027338 <bsp_ddrtrn_capat_adapt>
 4024680:	2a0003f4 	mov	w20, w0
 4024684:	35001b20 	cbnz	w0, 40249e8 <main_entry+0x7e0>
 4024688:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 402468c:	913e4400 	add	x0, x0, #0xf91
 4024690:	94000d95 	bl	4027ce4 <log_serial_puts>
 4024694:	94000e34 	bl	4027f64 <get_ddr_scrub_and_gpll_switch>
 4024698:	12001c00 	and	w0, w0, #0xff
 402469c:	7100041f 	cmp	w0, #0x1
 40246a0:	54001b40 	b.eq	4024a08 <main_entry+0x800>  // b.none
 40246a4:	b0fffff4 	adrp	x20, 4021000 <hash_verify_buf-0xa80>
 40246a8:	94000a6d 	bl	402705c <end_svb>
 40246ac:	912a8294 	add	x20, x20, #0xaa0
 40246b0:	34002616 	cbz	w22, 4024b70 <main_entry+0x968>
 40246b4:	52800000 	mov	w0, #0x0                   	// #0
 40246b8:	94000daf 	bl	4027d74 <watchdog_disable>
 40246bc:	52968f00 	mov	w0, #0xb478                	// #46200
 40246c0:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 40246c4:	6b00027f 	cmp	w19, w0
 40246c8:	54002321 	b.ne	4024b2c <main_entry+0x924>  // b.any
 40246cc:	f9000fff 	str	xzr, [sp, #24]
 40246d0:	94000297 	bl	402512c <get_uboot_info_download_ddr_addr>
 40246d4:	910063e1 	add	x1, sp, #0x18
 40246d8:	2a0003e0 	mov	w0, w0
 40246dc:	52a00202 	mov	w2, #0x100000              	// #1048576
 40246e0:	94000d85 	bl	4027cf4 <copy_from_uart>
 40246e4:	7102941f 	cmp	w0, #0xa5
 40246e8:	54002340 	b.eq	4024b50 <main_entry+0x948>  // b.none
 40246ec:	52800de0 	mov	w0, #0x6f                  	// #111
 40246f0:	97fffe62 	bl	4024078 <err_print>
 40246f4:	52800100 	mov	w0, #0x8                   	// #8
 40246f8:	97fffe17 	bl	4023f54 <failure_process>
 40246fc:	97fffebe 	bl	40241f4 <sys_watchdog_enable>
 4024700:	9400049f 	bl	402597c <handle_uboot_info>
 4024704:	7102941f 	cmp	w0, #0xa5
 4024708:	54000120 	b.eq	402472c <main_entry+0x524>  // b.none
 402470c:	52800e20 	mov	w0, #0x71                  	// #113
 4024710:	97fffe5a 	bl	4024078 <err_print>
 4024714:	52800100 	mov	w0, #0x8                   	// #8
 4024718:	97fffe0f 	bl	4023f54 <failure_process>
 402471c:	52800e20 	mov	w0, #0x71                  	// #113
 4024720:	97fffe56 	bl	4024078 <err_print>
 4024724:	52800100 	mov	w0, #0x8                   	// #8
 4024728:	97fffe0b 	bl	4023f54 <failure_process>
 402472c:	34000176 	cbz	w22, 4024758 <main_entry+0x550>
 4024730:	9400027f 	bl	402512c <get_uboot_info_download_ddr_addr>
 4024734:	92407c03 	and	x3, x0, #0xffffffff
 4024738:	11080002 	add	w2, w0, #0x200
 402473c:	b9402461 	ldr	w1, [x3, #36]
 4024740:	b9408860 	ldr	w0, [x3, #136]
 4024744:	97fffc70 	bl	4023904 <dma_copy>
 4024748:	7102941f 	cmp	w0, #0xa5
 402474c:	54000060 	b.eq	4024758 <main_entry+0x550>  // b.none
 4024750:	52800100 	mov	w0, #0x8                   	// #8
 4024754:	97fffe00 	bl	4023f54 <failure_process>
 4024758:	350000b5 	cbnz	w21, 402476c <main_entry+0x564>
 402475c:	528970e0 	mov	w0, #0x4b87                	// #19335
 4024760:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 4024764:	6b00027f 	cmp	w19, w0
 4024768:	54000101 	b.ne	4024788 <main_entry+0x580>  // b.any
 402476c:	2a1303e1 	mov	w1, w19
 4024770:	91002280 	add	x0, x20, #0x8
 4024774:	97fffdb7 	bl	4023e50 <get_uboot_code_from_flash>
 4024778:	7102941f 	cmp	w0, #0xa5
 402477c:	54000060 	b.eq	4024788 <main_entry+0x580>  // b.none
 4024780:	528000e0 	mov	w0, #0x7                   	// #7
 4024784:	97fffdf4 	bl	4023f54 <failure_process>
 4024788:	940004d7 	bl	4025ae4 <handle_uboot_code>
 402478c:	7102941f 	cmp	w0, #0xa5
 4024790:	54000120 	b.eq	40247b4 <main_entry+0x5ac>  // b.none
 4024794:	52800e40 	mov	w0, #0x72                  	// #114
 4024798:	97fffe38 	bl	4024078 <err_print>
 402479c:	52800100 	mov	w0, #0x8                   	// #8
 40247a0:	97fffded 	bl	4023f54 <failure_process>
 40247a4:	52800e40 	mov	w0, #0x72                  	// #114
 40247a8:	97fffe34 	bl	4024078 <err_print>
 40247ac:	52800100 	mov	w0, #0x8                   	// #8
 40247b0:	97fffde9 	bl	4023f54 <failure_process>
 40247b4:	9400025e 	bl	402512c <get_uboot_info_download_ddr_addr>
 40247b8:	2a0003f3 	mov	w19, w0
 40247bc:	d2804003 	mov	x3, #0x200                 	// #512
 40247c0:	aa1303e2 	mov	x2, x19
 40247c4:	aa0303e1 	mov	x1, x3
 40247c8:	b9408a60 	ldr	w0, [x19, #136]
 40247cc:	51080000 	sub	w0, w0, #0x200
 40247d0:	94000d7d 	bl	4027dc4 <memcpy_s>
 40247d4:	35000220 	cbnz	w0, 4024818 <main_entry+0x610>
 40247d8:	b9408a75 	ldr	w21, [x19, #136]
 40247dc:	94000245 	bl	40250f0 <get_ree_key_area_addr>
 40247e0:	d2802003 	mov	x3, #0x100                 	// #256
 40247e4:	2a0003e2 	mov	w2, w0
 40247e8:	510c02b5 	sub	w21, w21, #0x300
 40247ec:	aa0303e1 	mov	x1, x3
 40247f0:	aa1503e0 	mov	x0, x21
 40247f4:	94000d74 	bl	4027dc4 <memcpy_s>
 40247f8:	35000100 	cbnz	w0, 4024818 <main_entry+0x610>
 40247fc:	b9408a60 	ldr	w0, [x19, #136]
 4024800:	d2856002 	mov	x2, #0x2b00                	// #11008
 4024804:	d2802003 	mov	x3, #0x100                 	// #256
 4024808:	f2a08042 	movk	x2, #0x402, lsl #16
 402480c:	aa0303e1 	mov	x1, x3
 4024810:	51100000 	sub	w0, w0, #0x400
 4024814:	94000d6c 	bl	4027dc4 <memcpy_s>
 4024818:	d28a0501 	mov	x1, #0x5028                	// #20520
 402481c:	f2a22041 	movk	x1, #0x1102, lsl #16
 4024820:	b9400020 	ldr	w0, [x1]
 4024824:	12185c00 	and	w0, w0, #0xffffff00
 4024828:	32001800 	orr	w0, w0, #0x7f
 402482c:	b9000020 	str	w0, [x1]
 4024830:	b9400020 	ldr	w0, [x1]
 4024834:	12001c00 	and	w0, w0, #0xff
 4024838:	7101fc1f 	cmp	w0, #0x7f
 402483c:	540001a0 	b.eq	4024870 <main_entry+0x668>  // b.none
 4024840:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4024844:	913ed800 	add	x0, x0, #0xfb6
 4024848:	94000d27 	bl	4027ce4 <log_serial_puts>
 402484c:	d2800040 	mov	x0, #0x2                   	// #2
 4024850:	94000d41 	bl	4027d54 <mdelay>
 4024854:	94000d30 	bl	4027d14 <timer_deinit>
 4024858:	d2800080 	mov	x0, #0x4                   	// #4
 402485c:	52800021 	mov	w1, #0x1                   	// #1
 4024860:	f2a22040 	movk	x0, #0x1102, lsl #16
 4024864:	b9000001 	str	w1, [x0]
 4024868:	b9000001 	str	w1, [x0]
 402486c:	b9000001 	str	w1, [x0]
 4024870:	d28a0600 	mov	x0, #0x5030                	// #20528
 4024874:	52800021 	mov	w1, #0x1                   	// #1
 4024878:	f2a22040 	movk	x0, #0x1102, lsl #16
 402487c:	d0000053 	adrp	x19, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4024880:	913f3673 	add	x19, x19, #0xfcd
 4024884:	b9000001 	str	w1, [x0]
 4024888:	97fffa5d 	bl	40231fc <calculate_all_lpds_data>
 402488c:	97fff9fb 	bl	4023078 <enable_lpds_lock>
 4024890:	94000227 	bl	402512c <get_uboot_info_download_ddr_addr>
 4024894:	b9408800 	ldr	w0, [x0, #136]
 4024898:	d2802801 	mov	x1, #0x140                 	// #320
 402489c:	f2a22041 	movk	x1, #0x1102, lsl #16
 40248a0:	2a0003e0 	mov	w0, w0
 40248a4:	b900003f 	str	wzr, [x1]
 40248a8:	f9000e80 	str	x0, [x20, #24]
 40248ac:	52800000 	mov	w0, #0x0                   	// #0
 40248b0:	94000d31 	bl	4027d74 <watchdog_disable>
 40248b4:	528d34b4 	mov	w20, #0x69a5                	// #27045
 40248b8:	94000d4b 	bl	4027de4 <save_cur_point_syscnt>
 40248bc:	72b86b54 	movk	w20, #0xc35a, lsl #16
 40248c0:	d2884080 	mov	x0, #0x4204                	// #16900
 40248c4:	52917001 	mov	w1, #0x8b80                	// #35712
 40248c8:	f2a22040 	movk	x0, #0x1102, lsl #16
 40248cc:	72a02001 	movk	w1, #0x100, lsl #16
 40248d0:	b9000001 	str	w1, [x0]
 40248d4:	d2840981 	mov	x1, #0x204c                	// #8268
 40248d8:	f2a22021 	movk	x1, #0x1101, lsl #16
 40248dc:	b9400020 	ldr	w0, [x1]
 40248e0:	121e7400 	and	w0, w0, #0xfffffffc
 40248e4:	321c0000 	orr	w0, w0, #0x10
 40248e8:	b9000020 	str	w0, [x1]
 40248ec:	910123e1 	add	x1, sp, #0x48
 40248f0:	52800022 	mov	w2, #0x1                   	// #1
 40248f4:	52800000 	mov	w0, #0x0                   	// #0
 40248f8:	94000d64 	bl	4027e88 <ipc_recv_msg>
 40248fc:	7100001f 	cmp	w0, #0x0
 4024900:	5400166c 	b.gt	4024bcc <main_entry+0x9c4>
 4024904:	aa1303e0 	mov	x0, x19
 4024908:	94000cf7 	bl	4027ce4 <log_serial_puts>
 402490c:	17fffff8 	b	40248ec <main_entry+0x6e4>
 4024910:	350000b5 	cbnz	w21, 4024924 <main_entry+0x71c>
 4024914:	528970e0 	mov	w0, #0x4b87                	// #19335
 4024918:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 402491c:	6b00027f 	cmp	w19, w0
 4024920:	54000501 	b.ne	40249c0 <main_entry+0x7b8>  // b.any
 4024924:	97fffa20 	bl	40231a4 <verify_all_lpds_data>
 4024928:	7102941f 	cmp	w0, #0xa5
 402492c:	54000060 	b.eq	4024938 <main_entry+0x730>  // b.none
 4024930:	528000e0 	mov	w0, #0x7                   	// #7
 4024934:	97fffd88 	bl	4023f54 <failure_process>
 4024938:	d2800183 	mov	x3, #0xc                   	// #12
 402493c:	b0fffff7 	adrp	x23, 4021000 <hash_verify_buf-0xa80>
 4024940:	aa0303e1 	mov	x1, x3
 4024944:	912a82f7 	add	x23, x23, #0xaa0
 4024948:	52800002 	mov	w2, #0x0                   	// #0
 402494c:	910022f9 	add	x25, x23, #0x8
 4024950:	aa1903e0 	mov	x0, x25
 4024954:	94000d20 	bl	4027dd4 <memset_s>
 4024958:	97fff9c3 	bl	4023064 <get_verify_backup_img_reg>
 402495c:	2a0003f8 	mov	w24, w0
 4024960:	d2800280 	mov	x0, #0x14                  	// #20
 4024964:	f2a22040 	movk	x0, #0x1102, lsl #16
 4024968:	b9400000 	ldr	w0, [x0]
 402496c:	72001c1f 	tst	w0, #0xff
 4024970:	54000181 	b.ne	40249a0 <main_entry+0x798>  // b.any
 4024974:	940006cc 	bl	40264a4 <opt_get_boot_backup_enable>
 4024978:	7100041f 	cmp	w0, #0x1
 402497c:	54000121 	b.ne	40249a0 <main_entry+0x798>  // b.any
 4024980:	34000118 	cbz	w24, 40249a0 <main_entry+0x798>
 4024984:	d2800281 	mov	x1, #0x14                  	// #20
 4024988:	f2a22241 	movk	x1, #0x1112, lsl #16
 402498c:	b9400022 	ldr	w2, [x1]
 4024990:	34000082 	cbz	w2, 40249a0 <main_entry+0x798>
 4024994:	b9400021 	ldr	w1, [x1]
 4024998:	b9000ae0 	str	w0, [x23, #8]
 402499c:	b90012e1 	str	w1, [x23, #16]
 40249a0:	2a1303e1 	mov	w1, w19
 40249a4:	910022e0 	add	x0, x23, #0x8
 40249a8:	97fffc36 	bl	4023a80 <get_ree_key_and_paras_info_from_device>
 40249ac:	7102941f 	cmp	w0, #0xa5
 40249b0:	54ffd9c0 	b.eq	40244e8 <main_entry+0x2e0>  // b.none
 40249b4:	528000e0 	mov	w0, #0x7                   	// #7
 40249b8:	97fffd67 	bl	4023f54 <failure_process>
 40249bc:	17fffecb 	b	40244e8 <main_entry+0x2e0>
 40249c0:	d2800040 	mov	x0, #0x2                   	// #2
 40249c4:	94000ce4 	bl	4027d54 <mdelay>
 40249c8:	94000cd3 	bl	4027d14 <timer_deinit>
 40249cc:	d2800080 	mov	x0, #0x4                   	// #4
 40249d0:	52800021 	mov	w1, #0x1                   	// #1
 40249d4:	f2a22040 	movk	x0, #0x1102, lsl #16
 40249d8:	b9000001 	str	w1, [x0]
 40249dc:	b9000001 	str	w1, [x0]
 40249e0:	b9000001 	str	w1, [x0]
 40249e4:	17fffec1 	b	40244e8 <main_entry+0x2e0>
 40249e8:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 40249ec:	913e9400 	add	x0, x0, #0xfa5
 40249f0:	94000cbd 	bl	4027ce4 <log_serial_puts>
 40249f4:	2a1403e0 	mov	w0, w20
 40249f8:	94000b15 	bl	402764c <serial_put_hex>
 40249fc:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4024a00:	913ecc00 	add	x0, x0, #0xfb3
 4024a04:	17ffff23 	b	4024690 <main_entry+0x488>
 4024a08:	d2900a01 	mov	x1, #0x8050                	// #32848
 4024a0c:	f2a22281 	movk	x1, #0x1114, lsl #16
 4024a10:	b9400023 	ldr	w3, [x1]
 4024a14:	b9500022 	ldr	w2, [x1, #4096]
 4024a18:	72000c63 	ands	w3, w3, #0xf
 4024a1c:	12000c42 	and	w2, w2, #0xf
 4024a20:	54000080 	b.eq	4024a30 <main_entry+0x828>  // b.none
 4024a24:	d2900001 	mov	x1, #0x8000                	// #32768
 4024a28:	f2a22281 	movk	x1, #0x1114, lsl #16
 4024a2c:	b9000020 	str	w0, [x1]
 4024a30:	340000a2 	cbz	w2, 4024a44 <main_entry+0x83c>
 4024a34:	d2920000 	mov	x0, #0x9000                	// #36864
 4024a38:	52800021 	mov	w1, #0x1                   	// #1
 4024a3c:	f2a22280 	movk	x0, #0x1114, lsl #16
 4024a40:	b9000001 	str	w1, [x0]
 4024a44:	d2925284 	mov	x4, #0x9294                	// #37524
 4024a48:	d2905285 	mov	x5, #0x8294                	// #33428
 4024a4c:	f2a22284 	movk	x4, #0x1114, lsl #16
 4024a50:	f2a22285 	movk	x5, #0x1114, lsl #16
 4024a54:	340020c3 	cbz	w3, 4024e6c <main_entry+0xc64>
 4024a58:	b94000a0 	ldr	w0, [x5]
 4024a5c:	12000000 	and	w0, w0, #0x1
 4024a60:	340005e2 	cbz	w2, 4024b1c <main_entry+0x914>
 4024a64:	b9400081 	ldr	w1, [x4]
 4024a68:	12000021 	and	w1, w1, #0x1
 4024a6c:	6a00003f 	tst	w1, w0
 4024a70:	54ffff20 	b.eq	4024a54 <main_entry+0x84c>  // b.none
 4024a74:	97fffd2f 	bl	4023f30 <get_random_num>
 4024a78:	d2884301 	mov	x1, #0x4218                	// #16920
 4024a7c:	f2a22041 	movk	x1, #0x1102, lsl #16
 4024a80:	b9000020 	str	w0, [x1]
 4024a84:	97fffd2b 	bl	4023f30 <get_random_num>
 4024a88:	d2884381 	mov	x1, #0x421c                	// #16924
 4024a8c:	f2a22041 	movk	x1, #0x1102, lsl #16
 4024a90:	b9000020 	str	w0, [x1]
 4024a94:	d2884400 	mov	x0, #0x4220                	// #16928
 4024a98:	f2a22040 	movk	x0, #0x1102, lsl #16
 4024a9c:	528000a1 	mov	w1, #0x5                   	// #5
 4024aa0:	b9000001 	str	w1, [x0]
 4024aa4:	d2884481 	mov	x1, #0x4224                	// #16932
 4024aa8:	f2a22041 	movk	x1, #0x1102, lsl #16
 4024aac:	52800020 	mov	w0, #0x1                   	// #1
 4024ab0:	b900003f 	str	wzr, [x1]
 4024ab4:	b9000020 	str	w0, [x1]
 4024ab8:	b9000420 	str	w0, [x1, #4]
 4024abc:	97fffd1d 	bl	4023f30 <get_random_num>
 4024ac0:	340000a3 	cbz	w3, 4024ad4 <main_entry+0x8cc>
 4024ac4:	d2900000 	mov	x0, #0x8000                	// #32768
 4024ac8:	52800041 	mov	w1, #0x2                   	// #2
 4024acc:	f2a22280 	movk	x0, #0x1114, lsl #16
 4024ad0:	b9000001 	str	w1, [x0]
 4024ad4:	340000a2 	cbz	w2, 4024ae8 <main_entry+0x8e0>
 4024ad8:	d2920000 	mov	x0, #0x9000                	// #36864
 4024adc:	52800041 	mov	w1, #0x2                   	// #2
 4024ae0:	f2a22280 	movk	x0, #0x1114, lsl #16
 4024ae4:	b9000001 	str	w1, [x0]
 4024ae8:	d2925284 	mov	x4, #0x9294                	// #37524
 4024aec:	d2905285 	mov	x5, #0x8294                	// #33428
 4024af0:	f2a22284 	movk	x4, #0x1114, lsl #16
 4024af4:	f2a22285 	movk	x5, #0x1114, lsl #16
 4024af8:	34001b43 	cbz	w3, 4024e60 <main_entry+0xc58>
 4024afc:	b94000a0 	ldr	w0, [x5]
 4024b00:	12000001 	and	w1, w0, #0x1
 4024b04:	34000102 	cbz	w2, 4024b24 <main_entry+0x91c>
 4024b08:	b9400080 	ldr	w0, [x4]
 4024b0c:	12000000 	and	w0, w0, #0x1
 4024b10:	2a010000 	orr	w0, w0, w1
 4024b14:	35ffff20 	cbnz	w0, 4024af8 <main_entry+0x8f0>
 4024b18:	17fffee3 	b	40246a4 <main_entry+0x49c>
 4024b1c:	52800021 	mov	w1, #0x1                   	// #1
 4024b20:	17ffffd3 	b	4024a6c <main_entry+0x864>
 4024b24:	52800000 	mov	w0, #0x0                   	// #0
 4024b28:	17fffffa 	b	4024b10 <main_entry+0x908>
 4024b2c:	94000180 	bl	402512c <get_uboot_info_download_ddr_addr>
 4024b30:	2a0003e0 	mov	w0, w0
 4024b34:	910063e1 	add	x1, sp, #0x18
 4024b38:	52a00202 	mov	w2, #0x100000              	// #1048576
 4024b3c:	94000c4e 	bl	4027c74 <copy_from_usb>
 4024b40:	7102941f 	cmp	w0, #0xa5
 4024b44:	54000060 	b.eq	4024b50 <main_entry+0x948>  // b.none
 4024b48:	52800e00 	mov	w0, #0x70                  	// #112
 4024b4c:	17fffee9 	b	40246f0 <main_entry+0x4e8>
 4024b50:	f9400ff7 	ldr	x23, [sp, #24]
 4024b54:	94000176 	bl	402512c <get_uboot_info_download_ddr_addr>
 4024b58:	b9402400 	ldr	w0, [x0, #36]
 4024b5c:	11080000 	add	w0, w0, #0x200
 4024b60:	eb20c2ff 	cmp	x23, w0, sxtw
 4024b64:	54ffdcc0 	b.eq	40246fc <main_entry+0x4f4>  // b.none
 4024b68:	52800880 	mov	w0, #0x44                  	// #68
 4024b6c:	17fffee1 	b	40246f0 <main_entry+0x4e8>
 4024b70:	350000b5 	cbnz	w21, 4024b84 <main_entry+0x97c>
 4024b74:	528970e0 	mov	w0, #0x4b87                	// #19335
 4024b78:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 4024b7c:	6b00027f 	cmp	w19, w0
 4024b80:	54000121 	b.ne	4024ba4 <main_entry+0x99c>  // b.any
 4024b84:	2a1303e1 	mov	w1, w19
 4024b88:	91002280 	add	x0, x20, #0x8
 4024b8c:	97fffc7b 	bl	4023d78 <get_uboot_info_from_flash>
 4024b90:	7102941f 	cmp	w0, #0xa5
 4024b94:	54ffdb60 	b.eq	4024700 <main_entry+0x4f8>  // b.none
 4024b98:	52800100 	mov	w0, #0x8                   	// #8
 4024b9c:	97fffcee 	bl	4023f54 <failure_process>
 4024ba0:	17fffed8 	b	4024700 <main_entry+0x4f8>
 4024ba4:	d2800040 	mov	x0, #0x2                   	// #2
 4024ba8:	94000c6b 	bl	4027d54 <mdelay>
 4024bac:	94000c5a 	bl	4027d14 <timer_deinit>
 4024bb0:	d2800080 	mov	x0, #0x4                   	// #4
 4024bb4:	52800021 	mov	w1, #0x1                   	// #1
 4024bb8:	f2a22040 	movk	x0, #0x1102, lsl #16
 4024bbc:	b9000001 	str	w1, [x0]
 4024bc0:	b9000001 	str	w1, [x0]
 4024bc4:	b9000001 	str	w1, [x0]
 4024bc8:	17fffece 	b	4024700 <main_entry+0x4f8>
 4024bcc:	9400060c 	bl	40263fc <is_soc_tee_enable>
 4024bd0:	6b14001f 	cmp	w0, w20
 4024bd4:	b9404be0 	ldr	w0, [sp, #72]
 4024bd8:	54000121 	b.ne	4024bfc <main_entry+0x9f4>  // b.any
 4024bdc:	7102841f 	cmp	w0, #0xa1
 4024be0:	54000160 	b.eq	4024c0c <main_entry+0xa04>  // b.none
 4024be4:	52801620 	mov	w0, #0xb1                  	// #177
 4024be8:	910223e1 	add	x1, sp, #0x88
 4024bec:	29117fe0 	stp	w0, wzr, [sp, #136]
 4024bf0:	52800020 	mov	w0, #0x1                   	// #1
 4024bf4:	94000c83 	bl	4027e00 <ipc_send_msg>
 4024bf8:	17ffff3d 	b	40248ec <main_entry+0x6e4>
 4024bfc:	7102881f 	cmp	w0, #0xa2
 4024c00:	54000060 	b.eq	4024c0c <main_entry+0xa04>  // b.none
 4024c04:	52801640 	mov	w0, #0xb2                  	// #178
 4024c08:	17fffff8 	b	4024be8 <main_entry+0x9e0>
 4024c0c:	294a03f3 	ldp	w19, w0, [sp, #80]
 4024c10:	910223e1 	add	x1, sp, #0x88
 4024c14:	b0fffff4 	adrp	x20, 4021000 <hash_verify_buf-0xa80>
 4024c18:	aa138013 	orr	x19, x0, x19, lsl #32
 4024c1c:	52801400 	mov	w0, #0xa0                  	// #160
 4024c20:	29117fe0 	stp	w0, wzr, [sp, #136]
 4024c24:	52800020 	mov	w0, #0x1                   	// #1
 4024c28:	94000c76 	bl	4027e00 <ipc_send_msg>
 4024c2c:	940005f4 	bl	40263fc <is_soc_tee_enable>
 4024c30:	528d34a1 	mov	w1, #0x69a5                	// #27045
 4024c34:	72b86b41 	movk	w1, #0xc35a, lsl #16
 4024c38:	6b01001f 	cmp	w0, w1
 4024c3c:	54000501 	b.ne	4024cdc <main_entry+0xad4>  // b.any
 4024c40:	d2a80020 	mov	x0, #0x40010000            	// #1073807360
 4024c44:	eb00027f 	cmp	x19, x0
 4024c48:	54000240 	b.eq	4024c90 <main_entry+0xa88>  // b.none
 4024c4c:	aa1303e2 	mov	x2, x19
 4024c50:	52a00041 	mov	w1, #0x20000               	// #131072
 4024c54:	97fffb2c 	bl	4023904 <dma_copy>
 4024c58:	7102941f 	cmp	w0, #0xa5
 4024c5c:	540001a0 	b.eq	4024c90 <main_entry+0xa88>  // b.none
 4024c60:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4024c64:	913f8800 	add	x0, x0, #0xfe2
 4024c68:	94000c1f 	bl	4027ce4 <log_serial_puts>
 4024c6c:	d2800040 	mov	x0, #0x2                   	// #2
 4024c70:	94000c39 	bl	4027d54 <mdelay>
 4024c74:	94000c28 	bl	4027d14 <timer_deinit>
 4024c78:	d2800080 	mov	x0, #0x4                   	// #4
 4024c7c:	52800021 	mov	w1, #0x1                   	// #1
 4024c80:	f2a22040 	movk	x0, #0x1102, lsl #16
 4024c84:	b9000001 	str	w1, [x0]
 4024c88:	b9000001 	str	w1, [x0]
 4024c8c:	b9000001 	str	w1, [x0]
 4024c90:	d0000040 	adrp	x0, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4024c94:	913fdc00 	add	x0, x0, #0xff7
 4024c98:	94000c13 	bl	4027ce4 <log_serial_puts>
 4024c9c:	f94077e0 	ldr	x0, [sp, #232]
 4024ca0:	f9455281 	ldr	x1, [x20, #2720]
 4024ca4:	eb010000 	subs	x0, x0, x1
 4024ca8:	d2800001 	mov	x1, #0x0                   	// #0
 4024cac:	54000040 	b.eq	4024cb4 <main_entry+0xaac>  // b.none
 4024cb0:	97fffd2f 	bl	402416c <__stack_chk_fail>
 4024cb4:	d2800000 	mov	x0, #0x0                   	// #0
 4024cb8:	a94f53f3 	ldp	x19, x20, [sp, #240]
 4024cbc:	d2a80021 	mov	x1, #0x40010000            	// #1073807360
 4024cc0:	a9505bf5 	ldp	x21, x22, [sp, #256]
 4024cc4:	aa0103f0 	mov	x16, x1
 4024cc8:	a95163f7 	ldp	x23, x24, [sp, #272]
 4024ccc:	a9526bf9 	ldp	x25, x26, [sp, #288]
 4024cd0:	f9409bfe 	ldr	x30, [sp, #304]
 4024cd4:	910503ff 	add	sp, sp, #0x140
 4024cd8:	d61f0200 	br	x16
 4024cdc:	52800201 	mov	w1, #0x10                  	// #16
 4024ce0:	d2c00043 	mov	x3, #0x200000000           	// #8589934592
 4024ce4:	d2a80002 	mov	x2, #0x40000000            	// #1073741824
 4024ce8:	72aaaaa1 	movk	w1, #0x5555, lsl #16
 4024cec:	52800020 	mov	w0, #0x1                   	// #1
 4024cf0:	94000b25 	bl	4027984 <config_tzasc>
 4024cf4:	d2840003 	mov	x3, #0x2000                	// #8192
 4024cf8:	d29e0002 	mov	x2, #0xf000                	// #61440
 4024cfc:	52800201 	mov	w1, #0x10                  	// #16
 4024d00:	f2a80002 	movk	x2, #0x4000, lsl #16
 4024d04:	72aaa001 	movk	w1, #0x5500, lsl #16
 4024d08:	52800040 	mov	w0, #0x2                   	// #2
 4024d0c:	f2a02043 	movk	x3, #0x102, lsl #16
 4024d10:	94000b1d 	bl	4027984 <config_tzasc>
 4024d14:	d2840002 	mov	x2, #0x2000                	// #8192
 4024d18:	d29e0001 	mov	x1, #0xf000                	// #61440
 4024d1c:	f2a02042 	movk	x2, #0x102, lsl #16
 4024d20:	f2a80001 	movk	x1, #0x4000, lsl #16
 4024d24:	52800040 	mov	w0, #0x2                   	// #2
 4024d28:	94000ac2 	bl	4027830 <tzasc_sec_config_read_back>
 4024d2c:	7102941f 	cmp	w0, #0xa5
 4024d30:	54000201 	b.ne	4024d70 <main_entry+0xb68>  // b.any
 4024d34:	94000ade 	bl	40278ac <tzasc_bypass_disable>
 4024d38:	7102941f 	cmp	w0, #0xa5
 4024d3c:	540001a1 	b.ne	4024d70 <main_entry+0xb68>  // b.any
 4024d40:	aa1303e2 	mov	x2, x19
 4024d44:	52806001 	mov	w1, #0x300                 	// #768
 4024d48:	d2a82060 	mov	x0, #0x41030000            	// #1090715648
 4024d4c:	97fffaee 	bl	4023904 <dma_copy>
 4024d50:	7102941f 	cmp	w0, #0xa5
 4024d54:	54000121 	b.ne	4024d78 <main_entry+0xb70>  // b.any
 4024d58:	d2a82060 	mov	x0, #0x41030000            	// #1090715648
 4024d5c:	940003a1 	bl	4025be0 <handle_tee_key>
 4024d60:	7102941f 	cmp	w0, #0xa5
 4024d64:	54000480 	b.eq	4024df4 <main_entry+0xbec>  // b.none
 4024d68:	52800e60 	mov	w0, #0x73                  	// #115
 4024d6c:	14000002 	b	4024d74 <main_entry+0xb6c>
 4024d70:	52800dc0 	mov	w0, #0x6e                  	// #110
 4024d74:	97fffcc1 	bl	4024078 <err_print>
 4024d78:	d2902e01 	mov	x1, #0x8170                	// #33136
 4024d7c:	52800122 	mov	w2, #0x9                   	// #9
 4024d80:	f2a203c1 	movk	x1, #0x101e, lsl #16
 4024d84:	b9400020 	ldr	w0, [x1]
 4024d88:	121c6c00 	and	w0, w0, #0xfffffff0
 4024d8c:	2a020000 	orr	w0, w0, w2
 4024d90:	b9000020 	str	w0, [x1]
 4024d94:	d2800040 	mov	x0, #0x2                   	// #2
 4024d98:	94000bef 	bl	4027d54 <mdelay>
 4024d9c:	94000bde 	bl	4027d14 <timer_deinit>
 4024da0:	d2800080 	mov	x0, #0x4                   	// #4
 4024da4:	52800021 	mov	w1, #0x1                   	// #1
 4024da8:	f2a22040 	movk	x0, #0x1102, lsl #16
 4024dac:	b9000001 	str	w1, [x0]
 4024db0:	b9000001 	str	w1, [x0]
 4024db4:	b9000001 	str	w1, [x0]
 4024db8:	92800021 	mov	x1, #0xfffffffffffffffe    	// #-2
 4024dbc:	52800020 	mov	w0, #0x1                   	// #1
 4024dc0:	f2bf9fe1 	movk	x1, #0xfcff, lsl #16
 4024dc4:	72a06000 	movk	w0, #0x300, lsl #16
 4024dc8:	94000b63 	bl	4027b54 <uapi_drv_klad_rootkey_disable>
 4024dcc:	f0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4024dd0:	91003000 	add	x0, x0, #0xc
 4024dd4:	94000bc4 	bl	4027ce4 <log_serial_puts>
 4024dd8:	f94077e0 	ldr	x0, [sp, #232]
 4024ddc:	f9455281 	ldr	x1, [x20, #2720]
 4024de0:	eb010000 	subs	x0, x0, x1
 4024de4:	d2800001 	mov	x1, #0x0                   	// #0
 4024de8:	54fff641 	b.ne	4024cb0 <main_entry+0xaa8>  // b.any
 4024dec:	d2a80060 	mov	x0, #0x40030000            	// #1073938432
 4024df0:	17ffffb2 	b	4024cb8 <main_entry+0xab0>
 4024df4:	d2802001 	mov	x1, #0x100                 	// #256
 4024df8:	d2a82060 	mov	x0, #0x41030000            	// #1090715648
 4024dfc:	f2a82061 	movk	x1, #0x4103, lsl #16
 4024e00:	940003bc 	bl	4025cf0 <handle_tee_info>
 4024e04:	7102941f 	cmp	w0, #0xa5
 4024e08:	54000060 	b.eq	4024e14 <main_entry+0xc0c>  // b.none
 4024e0c:	52800e80 	mov	w0, #0x74                  	// #116
 4024e10:	17ffffd9 	b	4024d74 <main_entry+0xb6c>
 4024e14:	d2802001 	mov	x1, #0x100                 	// #256
 4024e18:	aa1303e0 	mov	x0, x19
 4024e1c:	d2a80022 	mov	x2, #0x40010000            	// #1073807360
 4024e20:	f2a82061 	movk	x1, #0x4103, lsl #16
 4024e24:	94000424 	bl	4025eb4 <handle_atf_code>
 4024e28:	7102941f 	cmp	w0, #0xa5
 4024e2c:	54000060 	b.eq	4024e38 <main_entry+0xc30>  // b.none
 4024e30:	52800ea0 	mov	w0, #0x75                  	// #117
 4024e34:	17ffffd0 	b	4024d74 <main_entry+0xb6c>
 4024e38:	d29ffc82 	mov	x2, #0xffe4                	// #65508
 4024e3c:	d2802001 	mov	x1, #0x100                 	// #256
 4024e40:	aa1303e0 	mov	x0, x19
 4024e44:	f2a80042 	movk	x2, #0x4002, lsl #16
 4024e48:	f2a82061 	movk	x1, #0x4103, lsl #16
 4024e4c:	94000473 	bl	4026018 <handle_tee_code>
 4024e50:	7102941f 	cmp	w0, #0xa5
 4024e54:	54fffb20 	b.eq	4024db8 <main_entry+0xbb0>  // b.none
 4024e58:	52800ec0 	mov	w0, #0x76                  	// #118
 4024e5c:	17ffffc6 	b	4024d74 <main_entry+0xb6c>
 4024e60:	34ffc222 	cbz	w2, 40246a4 <main_entry+0x49c>
 4024e64:	52800001 	mov	w1, #0x0                   	// #0
 4024e68:	17ffff28 	b	4024b08 <main_entry+0x900>
 4024e6c:	34ffe042 	cbz	w2, 4024a74 <main_entry+0x86c>
 4024e70:	52800020 	mov	w0, #0x1                   	// #1
 4024e74:	17fffefc 	b	4024a64 <main_entry+0x85c>

0000000004024e78 <check_msid_ext>:
 4024e78:	d100c3ff 	sub	sp, sp, #0x30
 4024e7c:	d2800002 	mov	x2, #0x0                   	// #0
 4024e80:	a90153f3 	stp	x19, x20, [sp, #16]
 4024e84:	2a0003f3 	mov	w19, w0
 4024e88:	2a0103f4 	mov	w20, w1
 4024e8c:	f90013fe 	str	x30, [sp, #32]
 4024e90:	b0ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4024e94:	f9455001 	ldr	x1, [x0, #2720]
 4024e98:	f90007e1 	str	x1, [sp, #8]
 4024e9c:	d2800001 	mov	x1, #0x0                   	// #0
 4024ea0:	52800600 	mov	w0, #0x30                  	// #48
 4024ea4:	910013e1 	add	x1, sp, #0x4
 4024ea8:	94000b2f 	bl	4027b64 <uapi_drv_otp_read_word>
 4024eac:	7102941f 	cmp	w0, #0xa5
 4024eb0:	54000160 	b.eq	4024edc <check_msid_ext+0x64>  // b.none
 4024eb4:	52800bc0 	mov	w0, #0x5e                  	// #94
 4024eb8:	97fffc70 	bl	4024078 <err_print>
 4024ebc:	52800b40 	mov	w0, #0x5a                  	// #90
 4024ec0:	b0ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4024ec4:	f94007e2 	ldr	x2, [sp, #8]
 4024ec8:	f9455023 	ldr	x3, [x1, #2720]
 4024ecc:	eb030042 	subs	x2, x2, x3
 4024ed0:	d2800003 	mov	x3, #0x0                   	// #0
 4024ed4:	54000100 	b.eq	4024ef4 <check_msid_ext+0x7c>  // b.none
 4024ed8:	97fffca5 	bl	402416c <__stack_chk_fail>
 4024edc:	b94007e1 	ldr	w1, [sp, #4]
 4024ee0:	4a010273 	eor	w19, w19, w1
 4024ee4:	6a14027f 	tst	w19, w20
 4024ee8:	54fffec0 	b.eq	4024ec0 <check_msid_ext+0x48>  // b.none
 4024eec:	52800c40 	mov	w0, #0x62                  	// #98
 4024ef0:	17fffff2 	b	4024eb8 <check_msid_ext+0x40>
 4024ef4:	a94153f3 	ldp	x19, x20, [sp, #16]
 4024ef8:	f94013fe 	ldr	x30, [sp, #32]
 4024efc:	9100c3ff 	add	sp, sp, #0x30
 4024f00:	d65f03c0 	ret

0000000004024f04 <die_id_compare>:
 4024f04:	d10103ff 	sub	sp, sp, #0x40
 4024f08:	d2800203 	mov	x3, #0x10                  	// #16
 4024f0c:	d2800004 	mov	x4, #0x0                   	// #0
 4024f10:	52800002 	mov	w2, #0x0                   	// #0
 4024f14:	a90253f3 	stp	x19, x20, [sp, #32]
 4024f18:	aa0003f4 	mov	x20, x0
 4024f1c:	b0ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4024f20:	f9001bfe 	str	x30, [sp, #48]
 4024f24:	52800b53 	mov	w19, #0x5a                  	// #90
 4024f28:	f9455001 	ldr	x1, [x0, #2720]
 4024f2c:	f9000fe1 	str	x1, [sp, #24]
 4024f30:	d2800001 	mov	x1, #0x0                   	// #0
 4024f34:	910023e0 	add	x0, sp, #0x8
 4024f38:	aa0303e1 	mov	x1, x3
 4024f3c:	b90007f3 	str	w19, [sp, #4]
 4024f40:	94000b9d 	bl	4027db4 <memset_ss>
 4024f44:	b90007e0 	str	w0, [sp, #4]
 4024f48:	b94007e0 	ldr	w0, [sp, #4]
 4024f4c:	7102941f 	cmp	w0, #0xa5
 4024f50:	54000160 	b.eq	4024f7c <die_id_compare+0x78>  // b.none
 4024f54:	52800a20 	mov	w0, #0x51                  	// #81
 4024f58:	97fffc48 	bl	4024078 <err_print>
 4024f5c:	2a1303e0 	mov	w0, w19
 4024f60:	b0ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4024f64:	f9400fe2 	ldr	x2, [sp, #24]
 4024f68:	f9455023 	ldr	x3, [x1, #2720]
 4024f6c:	eb030042 	subs	x2, x2, x3
 4024f70:	d2800003 	mov	x3, #0x0                   	// #0
 4024f74:	540002c0 	b.eq	4024fcc <die_id_compare+0xc8>  // b.none
 4024f78:	97fffc7d 	bl	402416c <__stack_chk_fail>
 4024f7c:	910023e0 	add	x0, sp, #0x8
 4024f80:	d2800002 	mov	x2, #0x0                   	// #0
 4024f84:	52800201 	mov	w1, #0x10                  	// #16
 4024f88:	94000aff 	bl	4027b84 <uapi_drv_otp_get_die_id>
 4024f8c:	b94007e0 	ldr	w0, [sp, #4]
 4024f90:	7102941f 	cmp	w0, #0xa5
 4024f94:	54000060 	b.eq	4024fa0 <die_id_compare+0x9c>  // b.none
 4024f98:	52800c60 	mov	w0, #0x63                  	// #99
 4024f9c:	17ffffef 	b	4024f58 <die_id_compare+0x54>
 4024fa0:	910023e1 	add	x1, sp, #0x8
 4024fa4:	aa1403e0 	mov	x0, x20
 4024fa8:	d2800003 	mov	x3, #0x0                   	// #0
 4024fac:	d2800202 	mov	x2, #0x10                  	// #16
 4024fb0:	94000b79 	bl	4027d94 <memcmp_ss>
 4024fb4:	b90007e0 	str	w0, [sp, #4]
 4024fb8:	b94007e0 	ldr	w0, [sp, #4]
 4024fbc:	7102941f 	cmp	w0, #0xa5
 4024fc0:	54fffd00 	b.eq	4024f60 <die_id_compare+0x5c>  // b.none
 4024fc4:	52800c80 	mov	w0, #0x64                  	// #100
 4024fc8:	17ffffe4 	b	4024f58 <die_id_compare+0x54>
 4024fcc:	a94253f3 	ldp	x19, x20, [sp, #32]
 4024fd0:	f9401bfe 	ldr	x30, [sp, #48]
 4024fd4:	910103ff 	add	sp, sp, #0x40
 4024fd8:	d65f03c0 	ret

0000000004024fdc <count_bit_set>:
 4024fdc:	1e270000 	fmov	s0, w0
 4024fe0:	0e205800 	cnt	v0.8b, v0.8b
 4024fe4:	0e31b800 	addv	b0, v0.8b
 4024fe8:	1e260000 	fmov	w0, s0
 4024fec:	d65f03c0 	ret

0000000004024ff0 <check_version>:
 4024ff0:	d100c3ff 	sub	sp, sp, #0x30
 4024ff4:	a90153f3 	stp	x19, x20, [sp, #16]
 4024ff8:	2a0103f4 	mov	w20, w1
 4024ffc:	b0ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4025000:	f90013fe 	str	x30, [sp, #32]
 4025004:	2a0203f3 	mov	w19, w2
 4025008:	f9455022 	ldr	x2, [x1, #2720]
 402500c:	f90007e2 	str	x2, [sp, #8]
 4025010:	d2800002 	mov	x2, #0x0                   	// #0
 4025014:	910013e1 	add	x1, sp, #0x4
 4025018:	b90007ff 	str	wzr, [sp, #4]
 402501c:	94000ad2 	bl	4027b64 <uapi_drv_otp_read_word>
 4025020:	7102941f 	cmp	w0, #0xa5
 4025024:	54000160 	b.eq	4025050 <check_version+0x60>  // b.none
 4025028:	52800bc0 	mov	w0, #0x5e                  	// #94
 402502c:	97fffc13 	bl	4024078 <err_print>
 4025030:	52800b43 	mov	w3, #0x5a                  	// #90
 4025034:	90ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4025038:	f94007e1 	ldr	x1, [sp, #8]
 402503c:	f9455002 	ldr	x2, [x0, #2720]
 4025040:	eb020021 	subs	x1, x1, x2
 4025044:	d2800002 	mov	x2, #0x0                   	// #0
 4025048:	540001a0 	b.eq	402507c <check_version+0x8c>  // b.none
 402504c:	97fffc48 	bl	402416c <__stack_chk_fail>
 4025050:	2a0003e3 	mov	w3, w0
 4025054:	0a130280 	and	w0, w20, w19
 4025058:	97ffffe1 	bl	4024fdc <count_bit_set>
 402505c:	2a0003e1 	mov	w1, w0
 4025060:	b94007e0 	ldr	w0, [sp, #4]
 4025064:	0a000260 	and	w0, w19, w0
 4025068:	97ffffdd 	bl	4024fdc <count_bit_set>
 402506c:	6b00003f 	cmp	w1, w0
 4025070:	54fffe22 	b.cs	4025034 <check_version+0x44>  // b.hs, b.nlast
 4025074:	52800be0 	mov	w0, #0x5f                  	// #95
 4025078:	17ffffed 	b	402502c <check_version+0x3c>
 402507c:	2a0303e0 	mov	w0, w3
 4025080:	a94153f3 	ldp	x19, x20, [sp, #16]
 4025084:	f94013fe 	ldr	x30, [sp, #32]
 4025088:	9100c3ff 	add	sp, sp, #0x30
 402508c:	d65f03c0 	ret

0000000004025090 <get_gsl_code_info_addr>:
 4025090:	52858000 	mov	w0, #0x2c00                	// #11264
 4025094:	72a08040 	movk	w0, #0x402, lsl #16
 4025098:	d65f03c0 	ret

000000000402509c <get_gsl_code_addr>:
 402509c:	5285c000 	mov	w0, #0x2e00                	// #11776
 40250a0:	72a08040 	movk	w0, #0x402, lsl #16
 40250a4:	d65f03c0 	ret

00000000040250a8 <get_gsl_code_area_len>:
 40250a8:	d2858000 	mov	x0, #0x2c00                	// #11264
 40250ac:	f2a08040 	movk	x0, #0x402, lsl #16
 40250b0:	b9402400 	ldr	w0, [x0, #36]
 40250b4:	d65f03c0 	ret

00000000040250b8 <get_gsl_heap_addr>:
 40250b8:	d2858000 	mov	x0, #0x2c00                	// #11264
 40250bc:	f2a08040 	movk	x0, #0x402, lsl #16
 40250c0:	b9402401 	ldr	w1, [x0, #36]
 40250c4:	11080000 	add	w0, w0, #0x200
 40250c8:	0b000020 	add	w0, w1, w0
 40250cc:	f240241f 	tst	x0, #0x3ff
 40250d0:	540000a0 	b.eq	40250e4 <get_gsl_heap_addr+0x2c>  // b.none
 40250d4:	52864000 	mov	w0, #0x3200                	// #12800
 40250d8:	72a08040 	movk	w0, #0x402, lsl #16
 40250dc:	0b000020 	add	w0, w1, w0
 40250e0:	12165400 	and	w0, w0, #0xfffffc00
 40250e4:	d65f03c0 	ret

00000000040250e8 <get_gsl_heap_len>:
 40250e8:	52900000 	mov	w0, #0x8000                	// #32768
 40250ec:	d65f03c0 	ret

00000000040250f0 <get_ree_key_area_addr>:
 40250f0:	f81f0ffe 	str	x30, [sp, #-16]!
 40250f4:	97fffff1 	bl	40250b8 <get_gsl_heap_addr>
 40250f8:	11402000 	add	w0, w0, #0x8, lsl #12
 40250fc:	f84107fe 	ldr	x30, [sp], #16
 4025100:	d65f03c0 	ret

0000000004025104 <get_ddr_param_info_addr>:
 4025104:	f81f0ffe 	str	x30, [sp, #-16]!
 4025108:	97fffffa 	bl	40250f0 <get_ree_key_area_addr>
 402510c:	11040000 	add	w0, w0, #0x100
 4025110:	f84107fe 	ldr	x30, [sp], #16
 4025114:	d65f03c0 	ret

0000000004025118 <get_ddr_param_data_addr>:
 4025118:	f81f0ffe 	str	x30, [sp, #-16]!
 402511c:	97fffffa 	bl	4025104 <get_ddr_param_info_addr>
 4025120:	110c0000 	add	w0, w0, #0x300
 4025124:	f84107fe 	ldr	x30, [sp], #16
 4025128:	d65f03c0 	ret

000000000402512c <get_uboot_info_download_ddr_addr>:
 402512c:	d2a82000 	mov	x0, #0x41000000            	// #1090519040
 4025130:	d65f03c0 	ret

0000000004025134 <get_uboot_code_ddr_addr>:
 4025134:	d2804000 	mov	x0, #0x200                 	// #512
 4025138:	f2a82000 	movk	x0, #0x4100, lsl #16
 402513c:	d65f03c0 	ret

0000000004025140 <get_uboot_code_size>:
 4025140:	d2a82000 	mov	x0, #0x41000000            	// #1090519040
 4025144:	b9402400 	ldr	w0, [x0, #36]
 4025148:	d65f03c0 	ret

000000000402514c <get_uboot_entrypoint_ddr_addr>:
 402514c:	d2a82000 	mov	x0, #0x41000000            	// #1090519040
 4025150:	b9408800 	ldr	w0, [x0, #136]
 4025154:	d65f03c0 	ret

0000000004025158 <hash_compare>:
 4025158:	d10183ff 	sub	sp, sp, #0x60
 402515c:	d2800403 	mov	x3, #0x20                  	// #32
 4025160:	d2800004 	mov	x4, #0x0                   	// #0
 4025164:	a90353f3 	stp	x19, x20, [sp, #48]
 4025168:	52800b53 	mov	w19, #0x5a                  	// #90
 402516c:	aa0203f4 	mov	x20, x2
 4025170:	a9045bf5 	stp	x21, x22, [sp, #64]
 4025174:	aa0003f5 	mov	x21, x0
 4025178:	2a0103f6 	mov	w22, w1
 402517c:	f9002bfe 	str	x30, [sp, #80]
 4025180:	90ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4025184:	2a1303e2 	mov	w2, w19
 4025188:	f9455001 	ldr	x1, [x0, #2720]
 402518c:	f90017e1 	str	x1, [sp, #40]
 4025190:	d2800001 	mov	x1, #0x0                   	// #0
 4025194:	910023e0 	add	x0, sp, #0x8
 4025198:	aa0303e1 	mov	x1, x3
 402519c:	b90007f3 	str	w19, [sp, #4]
 40251a0:	94000b05 	bl	4027db4 <memset_ss>
 40251a4:	b90007e0 	str	w0, [sp, #4]
 40251a8:	b94007e0 	ldr	w0, [sp, #4]
 40251ac:	7102941f 	cmp	w0, #0xa5
 40251b0:	540001c1 	b.ne	40251e8 <hash_compare+0x90>  // b.any
 40251b4:	910023e2 	add	x2, sp, #0x8
 40251b8:	2a1603e1 	mov	w1, w22
 40251bc:	2a1503e0 	mov	w0, w21
 40251c0:	52800004 	mov	w4, #0x0                   	// #0
 40251c4:	52800403 	mov	w3, #0x20                  	// #32
 40251c8:	b90007f3 	str	w19, [sp, #4]
 40251cc:	97fff8f5 	bl	40235a0 <calc_hash>
 40251d0:	b90007e0 	str	w0, [sp, #4]
 40251d4:	b94007e0 	ldr	w0, [sp, #4]
 40251d8:	7102941f 	cmp	w0, #0xa5
 40251dc:	54000140 	b.eq	4025204 <hash_compare+0xac>  // b.none
 40251e0:	52800a40 	mov	w0, #0x52                  	// #82
 40251e4:	97fffba5 	bl	4024078 <err_print>
 40251e8:	90ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 40251ec:	f94017e0 	ldr	x0, [sp, #40]
 40251f0:	f9455022 	ldr	x2, [x1, #2720]
 40251f4:	eb020000 	subs	x0, x0, x2
 40251f8:	d2800002 	mov	x2, #0x0                   	// #0
 40251fc:	54000200 	b.eq	402523c <hash_compare+0xe4>  // b.none
 4025200:	97fffbdb 	bl	402416c <__stack_chk_fail>
 4025204:	aa1403e1 	mov	x1, x20
 4025208:	910023e0 	add	x0, sp, #0x8
 402520c:	d2800003 	mov	x3, #0x0                   	// #0
 4025210:	d2800402 	mov	x2, #0x20                  	// #32
 4025214:	b90007f3 	str	w19, [sp, #4]
 4025218:	94000adf 	bl	4027d94 <memcmp_ss>
 402521c:	b90007e0 	str	w0, [sp, #4]
 4025220:	b94007e0 	ldr	w0, [sp, #4]
 4025224:	7102941f 	cmp	w0, #0xa5
 4025228:	54000060 	b.eq	4025234 <hash_compare+0xdc>  // b.none
 402522c:	52800aa0 	mov	w0, #0x55                  	// #85
 4025230:	17ffffed 	b	40251e4 <hash_compare+0x8c>
 4025234:	b94007f3 	ldr	w19, [sp, #4]
 4025238:	17ffffec 	b	40251e8 <hash_compare+0x90>
 402523c:	2a1303e0 	mov	w0, w19
 4025240:	a94353f3 	ldp	x19, x20, [sp, #48]
 4025244:	a9445bf5 	ldp	x21, x22, [sp, #64]
 4025248:	f9402bfe 	ldr	x30, [sp, #80]
 402524c:	910183ff 	add	sp, sp, #0x60
 4025250:	d65f03c0 	ret

0000000004025254 <secure_verify_area>:
 4025254:	d102c3ff 	sub	sp, sp, #0xb0
 4025258:	a90953f3 	stp	x19, x20, [sp, #144]
 402525c:	aa0003f4 	mov	x20, x0
 4025260:	90ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4025264:	a90a7bf5 	stp	x21, x30, [sp, #160]
 4025268:	52800b53 	mov	w19, #0x5a                  	// #90
 402526c:	52800415 	mov	w21, #0x20                  	// #32
 4025270:	f9455004 	ldr	x4, [x0, #2720]
 4025274:	f90047e4 	str	x4, [sp, #136]
 4025278:	d2800004 	mov	x4, #0x0                   	// #0
 402527c:	528f0b40 	mov	w0, #0x785a                	// #30810
 4025280:	b9000ff3 	str	w19, [sp, #12]
 4025284:	72b2d860 	movk	w0, #0x96c3, lsl #16
 4025288:	b9000ff3 	str	w19, [sp, #12]
 402528c:	b90013f3 	str	w19, [sp, #16]
 4025290:	b90017e0 	str	w0, [sp, #20]
 4025294:	91008060 	add	x0, x3, #0x20
 4025298:	b9001be2 	str	w2, [sp, #24]
 402529c:	f90013e1 	str	x1, [sp, #32]
 40252a0:	f9002be3 	str	x3, [sp, #80]
 40252a4:	f9002fe0 	str	x0, [sp, #88]
 40252a8:	b90063f5 	str	w21, [sp, #96]
 40252ac:	b40000f4 	cbz	x20, 40252c8 <secure_verify_area+0x74>
 40252b0:	b40000c3 	cbz	x3, 40252c8 <secure_verify_area+0x74>
 40252b4:	94000a38 	bl	4027b94 <uapi_drv_pke_init>
 40252b8:	7102941f 	cmp	w0, #0xa5
 40252bc:	54000240 	b.eq	4025304 <secure_verify_area+0xb0>  // b.none
 40252c0:	52800b20 	mov	w0, #0x59                  	// #89
 40252c4:	97fffb6d 	bl	4024078 <err_print>
 40252c8:	b9000ff3 	str	w19, [sp, #12]
 40252cc:	b9400fe0 	ldr	w0, [sp, #12]
 40252d0:	7102941f 	cmp	w0, #0xa5
 40252d4:	54000761 	b.ne	40253c0 <secure_verify_area+0x16c>  // b.any
 40252d8:	b9400fe0 	ldr	w0, [sp, #12]
 40252dc:	7102941f 	cmp	w0, #0xa5
 40252e0:	54000701 	b.ne	40253c0 <secure_verify_area+0x16c>  // b.any
 40252e4:	b9400fe0 	ldr	w0, [sp, #12]
 40252e8:	90ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 40252ec:	f94047e2 	ldr	x2, [sp, #136]
 40252f0:	f9455023 	ldr	x3, [x1, #2720]
 40252f4:	eb030042 	subs	x2, x2, x3
 40252f8:	d2800003 	mov	x3, #0x0                   	// #0
 40252fc:	54000660 	b.eq	40253c8 <secure_verify_area+0x174>  // b.none
 4025300:	97fffb9b 	bl	402416c <__stack_chk_fail>
 4025304:	9101a3f3 	add	x19, sp, #0x68
 4025308:	a90353f3 	stp	x19, x20, [sp, #48]
 402530c:	91008294 	add	x20, x20, #0x20
 4025310:	f90023f4 	str	x20, [sp, #64]
 4025314:	528f0bb4 	mov	w20, #0x785d                	// #30813
 4025318:	9100a3e3 	add	x3, sp, #0x28
 402531c:	910143e2 	add	x2, sp, #0x50
 4025320:	9100e3e1 	add	x1, sp, #0x38
 4025324:	910063e0 	add	x0, sp, #0x18
 4025328:	52800004 	mov	w4, #0x0                   	// #0
 402532c:	72b2d874 	movk	w20, #0x96c3, lsl #16
 4025330:	b9002bf5 	str	w21, [sp, #40]
 4025334:	b9004bf5 	str	w21, [sp, #72]
 4025338:	97fff8f5 	bl	402370c <verify_signature>
 402533c:	b90013e0 	str	w0, [sp, #16]
 4025340:	b94013e0 	ldr	w0, [sp, #16]
 4025344:	7102941f 	cmp	w0, #0xa5
 4025348:	54000261 	b.ne	4025394 <secure_verify_area+0x140>  // b.any
 402534c:	f9402be0 	ldr	x0, [sp, #80]
 4025350:	aa1303e1 	mov	x1, x19
 4025354:	d2800003 	mov	x3, #0x0                   	// #0
 4025358:	d2800402 	mov	x2, #0x20                  	// #32
 402535c:	94000a8e 	bl	4027d94 <memcmp_ss>
 4025360:	b90013e0 	str	w0, [sp, #16]
 4025364:	b94013e0 	ldr	w0, [sp, #16]
 4025368:	7102941f 	cmp	w0, #0xa5
 402536c:	54000180 	b.eq	402539c <secure_verify_area+0x148>  // b.none
 4025370:	52800a80 	mov	w0, #0x54                  	// #84
 4025374:	97fffb41 	bl	4024078 <err_print>
 4025378:	94000a0b 	bl	4027ba4 <uapi_drv_pke_deinit>
 402537c:	7102941f 	cmp	w0, #0xa5
 4025380:	540001c0 	b.eq	40253b8 <secure_verify_area+0x164>  // b.none
 4025384:	52800b60 	mov	w0, #0x5b                  	// #91
 4025388:	52800b53 	mov	w19, #0x5a                  	// #90
 402538c:	97fffb3b 	bl	4024078 <err_print>
 4025390:	17ffffce 	b	40252c8 <secure_verify_area+0x74>
 4025394:	52800b40 	mov	w0, #0x5a                  	// #90
 4025398:	17fffff7 	b	4025374 <secure_verify_area+0x120>
 402539c:	b94017e0 	ldr	w0, [sp, #20]
 40253a0:	11000400 	add	w0, w0, #0x1
 40253a4:	b90017e0 	str	w0, [sp, #20]
 40253a8:	b94017e0 	ldr	w0, [sp, #20]
 40253ac:	6b14001f 	cmp	w0, w20
 40253b0:	54fffc89 	b.ls	4025340 <secure_verify_area+0xec>  // b.plast
 40253b4:	17fffff1 	b	4025378 <secure_verify_area+0x124>
 40253b8:	b94013f3 	ldr	w19, [sp, #16]
 40253bc:	17ffffc3 	b	40252c8 <secure_verify_area+0x74>
 40253c0:	52800b40 	mov	w0, #0x5a                  	// #90
 40253c4:	17ffffc9 	b	40252e8 <secure_verify_area+0x94>
 40253c8:	a94953f3 	ldp	x19, x20, [sp, #144]
 40253cc:	a94a7bf5 	ldp	x21, x30, [sp, #160]
 40253d0:	9102c3ff 	add	sp, sp, #0xb0
 40253d4:	d65f03c0 	ret

00000000040253d8 <lpds_hash_cmp>:
 40253d8:	d10143ff 	sub	sp, sp, #0x50
 40253dc:	a90353f3 	stp	x19, x20, [sp, #48]
 40253e0:	aa0103f3 	mov	x19, x1
 40253e4:	2a0203f4 	mov	w20, w2
 40253e8:	f90023fe 	str	x30, [sp, #64]
 40253ec:	90ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 40253f0:	f9455022 	ldr	x2, [x1, #2720]
 40253f4:	f90017e2 	str	x2, [sp, #40]
 40253f8:	d2800002 	mov	x2, #0x0                   	// #0
 40253fc:	2a0003e1 	mov	w1, w0
 4025400:	52800402 	mov	w2, #0x20                  	// #32
 4025404:	910023e0 	add	x0, sp, #0x8
 4025408:	97fff750 	bl	4023148 <lpds_hash_get>
 402540c:	7102941f 	cmp	w0, #0xa5
 4025410:	54000141 	b.ne	4025438 <lpds_hash_cmp+0x60>  // b.any
 4025414:	910023e2 	add	x2, sp, #0x8
 4025418:	2a1403e1 	mov	w1, w20
 402541c:	aa1303e0 	mov	x0, x19
 4025420:	97ffff4e 	bl	4025158 <hash_compare>
 4025424:	7102941f 	cmp	w0, #0xa5
 4025428:	540000c0 	b.eq	4025440 <lpds_hash_cmp+0x68>  // b.none
 402542c:	d0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4025430:	91009c00 	add	x0, x0, #0x27
 4025434:	94000a2c 	bl	4027ce4 <log_serial_puts>
 4025438:	52800b40 	mov	w0, #0x5a                  	// #90
 402543c:	14000007 	b	4025458 <lpds_hash_cmp+0x80>
 4025440:	910023e2 	add	x2, sp, #0x8
 4025444:	2a1403e1 	mov	w1, w20
 4025448:	aa1303e0 	mov	x0, x19
 402544c:	97ffff43 	bl	4025158 <hash_compare>
 4025450:	7102941f 	cmp	w0, #0xa5
 4025454:	54fffec1 	b.ne	402542c <lpds_hash_cmp+0x54>  // b.any
 4025458:	90ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 402545c:	f94017e2 	ldr	x2, [sp, #40]
 4025460:	f9455023 	ldr	x3, [x1, #2720]
 4025464:	eb030042 	subs	x2, x2, x3
 4025468:	d2800003 	mov	x3, #0x0                   	// #0
 402546c:	54000040 	b.eq	4025474 <lpds_hash_cmp+0x9c>  // b.none
 4025470:	97fffb3f 	bl	402416c <__stack_chk_fail>
 4025474:	a94353f3 	ldp	x19, x20, [sp, #48]
 4025478:	f94023fe 	ldr	x30, [sp, #64]
 402547c:	910143ff 	add	sp, sp, #0x50
 4025480:	d65f03c0 	ret

0000000004025484 <get_ree_verify_enable_flag>:
 4025484:	d2902a80 	mov	x0, #0x8154                	// #33108
 4025488:	f2a203c0 	movk	x0, #0x101e, lsl #16
 402548c:	b9400000 	ldr	w0, [x0]
 4025490:	d3483c00 	ubfx	x0, x0, #8, #8
 4025494:	d65f03c0 	ret

0000000004025498 <get_tee_verify_enable_flag>:
 4025498:	d2902a80 	mov	x0, #0x8154                	// #33108
 402549c:	f2a203c0 	movk	x0, #0x101e, lsl #16
 40254a0:	b9400000 	ldr	w0, [x0]
 40254a4:	d65f03c0 	ret

00000000040254a8 <get_tp_verify_enable_flag>:
 40254a8:	d2902a80 	mov	x0, #0x8154                	// #33108
 40254ac:	f2a203c0 	movk	x0, #0x101e, lsl #16
 40254b0:	b9400000 	ldr	w0, [x0]
 40254b4:	d3505c00 	ubfx	x0, x0, #16, #8
 40254b8:	d65f03c0 	ret

00000000040254bc <handle_ree_key_area>:
 40254bc:	d2902a81 	mov	x1, #0x8154                	// #33108
 40254c0:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 40254c4:	f2a203c1 	movk	x1, #0x101e, lsl #16
 40254c8:	b9400020 	ldr	w0, [x1]
 40254cc:	f9000bfe 	str	x30, [sp, #16]
 40254d0:	d3483c00 	ubfx	x0, x0, #8, #8
 40254d4:	7101081f 	cmp	w0, #0x42
 40254d8:	540000e1 	b.ne	40254f4 <handle_ree_key_area+0x38>  // b.any
 40254dc:	b9400020 	ldr	w0, [x1]
 40254e0:	d3483c00 	ubfx	x0, x0, #8, #8
 40254e4:	7101081f 	cmp	w0, #0x42
 40254e8:	54000a20 	b.eq	402562c <handle_ree_key_area+0x170>  // b.none
 40254ec:	52800b53 	mov	w19, #0x5a                  	// #90
 40254f0:	1400002d 	b	40255a4 <handle_ree_key_area+0xe8>
 40254f4:	97fffeff 	bl	40250f0 <get_ree_key_area_addr>
 40254f8:	2a0003f4 	mov	w20, w0
 40254fc:	528783c0 	mov	w0, #0x3c1e                	// #15390
 4025500:	72a963c0 	movk	w0, #0x4b1e, lsl #16
 4025504:	b9400281 	ldr	w1, [x20]
 4025508:	6b00003f 	cmp	w1, w0
 402550c:	54000301 	b.ne	402556c <handle_ree_key_area+0xb0>  // b.any
 4025510:	97fff6f7 	bl	40230ec <get_low_power_mode>
 4025514:	7101681f 	cmp	w0, #0x5a
 4025518:	54000300 	b.eq	4025578 <handle_ree_key_area+0xbc>  // b.none
 402551c:	d2902a01 	mov	x1, #0x8150                	// #33104
 4025520:	f2a203c1 	movk	x1, #0x101e, lsl #16
 4025524:	b9400020 	ldr	w0, [x1]
 4025528:	12000c00 	and	w0, w0, #0xf
 402552c:	7100141f 	cmp	w0, #0x5
 4025530:	54000421 	b.ne	40255b4 <handle_ree_key_area+0xf8>  // b.any
 4025534:	b9400020 	ldr	w0, [x1]
 4025538:	12000c00 	and	w0, w0, #0xf
 402553c:	7100141f 	cmp	w0, #0x5
 4025540:	540003a1 	b.ne	40255b4 <handle_ree_key_area+0xf8>  // b.any
 4025544:	d2834800 	mov	x0, #0x1a40                	// #6720
 4025548:	91030283 	add	x3, x20, #0xc0
 402554c:	aa1403e1 	mov	x1, x20
 4025550:	52801802 	mov	w2, #0xc0                  	// #192
 4025554:	f2a08040 	movk	x0, #0x402, lsl #16
 4025558:	97ffff3f 	bl	4025254 <secure_verify_area>
 402555c:	7102941f 	cmp	w0, #0xa5
 4025560:	540002e0 	b.eq	40255bc <handle_ree_key_area+0x100>  // b.none
 4025564:	52800ba0 	mov	w0, #0x5d                  	// #93
 4025568:	14000002 	b	4025570 <handle_ree_key_area+0xb4>
 402556c:	528008a0 	mov	w0, #0x45                  	// #69
 4025570:	97fffac2 	bl	4024078 <err_print>
 4025574:	17ffffde 	b	40254ec <handle_ree_key_area+0x30>
 4025578:	52801600 	mov	w0, #0xb0                  	// #176
 402557c:	aa1403e1 	mov	x1, x20
 4025580:	52801802 	mov	w2, #0xc0                  	// #192
 4025584:	72a22240 	movk	w0, #0x1112, lsl #16
 4025588:	97ffff94 	bl	40253d8 <lpds_hash_cmp>
 402558c:	2a0003f3 	mov	w19, w0
 4025590:	7102941f 	cmp	w0, #0xa5
 4025594:	54fffac1 	b.ne	40254ec <handle_ree_key_area+0x30>  // b.any
 4025598:	d0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 402559c:	9100dc00 	add	x0, x0, #0x37
 40255a0:	940009d1 	bl	4027ce4 <log_serial_puts>
 40255a4:	2a1303e0 	mov	w0, w19
 40255a8:	f9400bfe 	ldr	x30, [sp, #16]
 40255ac:	a8c253f3 	ldp	x19, x20, [sp], #32
 40255b0:	d65f03c0 	ret
 40255b4:	52800ac0 	mov	w0, #0x56                  	// #86
 40255b8:	17ffffee 	b	4025570 <handle_ree_key_area+0xb4>
 40255bc:	29448a81 	ldp	w1, w2, [x20, #36]
 40255c0:	d2803a00 	mov	x0, #0x1d0                 	// #464
 40255c4:	97fffe8b 	bl	4024ff0 <check_version>
 40255c8:	2a0003f3 	mov	w19, w0
 40255cc:	7102941f 	cmp	w0, #0xa5
 40255d0:	54000201 	b.ne	4025610 <handle_ree_key_area+0x154>  // b.any
 40255d4:	29458680 	ldp	w0, w1, [x20, #44]
 40255d8:	97fffe28 	bl	4024e78 <check_msid_ext>
 40255dc:	7102941f 	cmp	w0, #0xa5
 40255e0:	54fff861 	b.ne	40254ec <handle_ree_key_area+0x30>  // b.any
 40255e4:	b9403681 	ldr	w1, [x20, #52]
 40255e8:	5292dc20 	mov	w0, #0x96e1                	// #38625
 40255ec:	72a78f00 	movk	w0, #0x3c78, lsl #16
 40255f0:	6b00003f 	cmp	w1, w0
 40255f4:	54000120 	b.eq	4025618 <handle_ree_key_area+0x15c>  // b.none
 40255f8:	d2801600 	mov	x0, #0xb0                  	// #176
 40255fc:	f2a22240 	movk	x0, #0x1112, lsl #16
 4025600:	97fff828 	bl	40236a0 <store_hash_to_lpds>
 4025604:	52802000 	mov	w0, #0x100                 	// #256
 4025608:	97fff68b 	bl	4023034 <add_boot_img_size>
 402560c:	17ffffe6 	b	40255a4 <handle_ree_key_area+0xe8>
 4025610:	52800c20 	mov	w0, #0x61                  	// #97
 4025614:	17ffffd7 	b	4025570 <handle_ree_key_area+0xb4>
 4025618:	9100e280 	add	x0, x20, #0x38
 402561c:	97fffe3a 	bl	4024f04 <die_id_compare>
 4025620:	7102941f 	cmp	w0, #0xa5
 4025624:	54fff641 	b.ne	40254ec <handle_ree_key_area+0x30>  // b.any
 4025628:	17fffff4 	b	40255f8 <handle_ree_key_area+0x13c>
 402562c:	528014b3 	mov	w19, #0xa5                  	// #165
 4025630:	17ffffdd 	b	40255a4 <handle_ree_key_area+0xe8>

0000000004025634 <get_ddr_param_len>:
 4025634:	f81f0ffe 	str	x30, [sp, #-16]!
 4025638:	97fffeb3 	bl	4025104 <get_ddr_param_info_addr>
 402563c:	2a0003e0 	mov	w0, w0
 4025640:	f84107fe 	ldr	x30, [sp], #16
 4025644:	b9402400 	ldr	w0, [x0, #36]
 4025648:	d65f03c0 	ret

000000000402564c <get_ddr_param_data_end_addr>:
 402564c:	f81f0ffe 	str	x30, [sp, #-16]!
 4025650:	97fffeb2 	bl	4025118 <get_ddr_param_data_addr>
 4025654:	2a0003e2 	mov	w2, w0
 4025658:	97fffff7 	bl	4025634 <get_ddr_param_len>
 402565c:	0b000040 	add	w0, w2, w0
 4025660:	f240241f 	tst	x0, #0x3ff
 4025664:	54000060 	b.eq	4025670 <get_ddr_param_data_end_addr+0x24>  // b.none
 4025668:	11100000 	add	w0, w0, #0x400
 402566c:	12165400 	and	w0, w0, #0xfffffc00
 4025670:	f84107fe 	ldr	x30, [sp], #16
 4025674:	d65f03c0 	ret

0000000004025678 <get_ddr_param_cnt>:
 4025678:	f81f0ffe 	str	x30, [sp, #-16]!
 402567c:	97fffea2 	bl	4025104 <get_ddr_param_info_addr>
 4025680:	2a0003e0 	mov	w0, w0
 4025684:	f84107fe 	ldr	x30, [sp], #16
 4025688:	b9402800 	ldr	w0, [x0, #40]
 402568c:	d65f03c0 	ret

0000000004025690 <handle_ddr_param_info>:
 4025690:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4025694:	2a0003f3 	mov	w19, w0
 4025698:	a9017bf5 	stp	x21, x30, [sp, #16]
 402569c:	97fffe9a 	bl	4025104 <get_ddr_param_info_addr>
 40256a0:	2a0003f4 	mov	w20, w0
 40256a4:	97fffff5 	bl	4025678 <get_ddr_param_cnt>
 40256a8:	7100201f 	cmp	w0, #0x8
 40256ac:	54000129 	b.ls	40256d0 <handle_ddr_param_info+0x40>  // b.plast
 40256b0:	d0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 40256b4:	91013400 	add	x0, x0, #0x4d
 40256b8:	9400098b 	bl	4027ce4 <log_serial_puts>
 40256bc:	52800100 	mov	w0, #0x8                   	// #8
 40256c0:	940007e3 	bl	402764c <serial_put_hex>
 40256c4:	52800ca0 	mov	w0, #0x65                  	// #101
 40256c8:	97fffa6c 	bl	4024078 <err_print>
 40256cc:	1400000b 	b	40256f8 <handle_ddr_param_info+0x68>
 40256d0:	8b334293 	add	x19, x20, w19, uxtw
 40256d4:	2a0003f5 	mov	w21, w0
 40256d8:	3944b260 	ldrb	w0, [x19, #300]
 40256dc:	7103fc1f 	cmp	w0, #0xff
 40256e0:	54000101 	b.ne	4025700 <handle_ddr_param_info+0x70>  // b.any
 40256e4:	d0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 40256e8:	91017c00 	add	x0, x0, #0x5f
 40256ec:	9400097e 	bl	4027ce4 <log_serial_puts>
 40256f0:	3944b260 	ldrb	w0, [x19, #300]
 40256f4:	940007d6 	bl	402764c <serial_put_hex>
 40256f8:	52800b53 	mov	w19, #0x5a                  	// #90
 40256fc:	14000036 	b	40257d4 <handle_ddr_param_info+0x144>
 4025700:	6b0002bf 	cmp	w21, w0
 4025704:	540000c8 	b.hi	402571c <handle_ddr_param_info+0x8c>  // b.pmore
 4025708:	d0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 402570c:	91022800 	add	x0, x0, #0x8a
 4025710:	94000975 	bl	4027ce4 <log_serial_puts>
 4025714:	2a1503e0 	mov	w0, w21
 4025718:	17ffffea 	b	40256c0 <handle_ddr_param_info+0x30>
 402571c:	52878013 	mov	w19, #0x3c00                	// #15360
 4025720:	97ffffc5 	bl	4025634 <get_ddr_param_len>
 4025724:	6b13001f 	cmp	w0, w19
 4025728:	54000109 	b.ls	4025748 <handle_ddr_param_info+0xb8>  // b.plast
 402572c:	d0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4025730:	91027000 	add	x0, x0, #0x9c
 4025734:	9400096c 	bl	4027ce4 <log_serial_puts>
 4025738:	2a1303e0 	mov	w0, w19
 402573c:	940007c4 	bl	402764c <serial_put_hex>
 4025740:	52800cc0 	mov	w0, #0x66                  	// #102
 4025744:	17ffffe1 	b	40256c8 <handle_ddr_param_info+0x38>
 4025748:	34ffffc0 	cbz	w0, 4025740 <handle_ddr_param_info+0xb0>
 402574c:	b9402280 	ldr	w0, [x20, #32]
 4025750:	7104001f 	cmp	w0, #0x100
 4025754:	540000a0 	b.eq	4025768 <handle_ddr_param_info+0xd8>  // b.none
 4025758:	d0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 402575c:	9102bc00 	add	x0, x0, #0xaf
 4025760:	94000961 	bl	4027ce4 <log_serial_puts>
 4025764:	17ffffe5 	b	40256f8 <handle_ddr_param_info+0x68>
 4025768:	b9400281 	ldr	w1, [x20]
 402576c:	5294a5a0 	mov	w0, #0xa52d                	// #42285
 4025770:	72a970e0 	movk	w0, #0x4b87, lsl #16
 4025774:	6b00003f 	cmp	w1, w0
 4025778:	54000361 	b.ne	40257e4 <handle_ddr_param_info+0x154>  // b.any
 402577c:	d2902a95 	mov	x21, #0x8154                	// #33108
 4025780:	f2a203d5 	movk	x21, #0x101e, lsl #16
 4025784:	b94002a0 	ldr	w0, [x21]
 4025788:	d3483c00 	ubfx	x0, x0, #8, #8
 402578c:	7101081f 	cmp	w0, #0x42
 4025790:	540002e0 	b.eq	40257ec <handle_ddr_param_info+0x15c>  // b.none
 4025794:	97fffe57 	bl	40250f0 <get_ree_key_area_addr>
 4025798:	2a0003f3 	mov	w19, w0
 402579c:	97fff654 	bl	40230ec <get_low_power_mode>
 40257a0:	7101681f 	cmp	w0, #0x5a
 40257a4:	54000301 	b.ne	4025804 <handle_ddr_param_info+0x174>  // b.any
 40257a8:	52801a00 	mov	w0, #0xd0                  	// #208
 40257ac:	aa1403e1 	mov	x1, x20
 40257b0:	52803002 	mov	w2, #0x180                 	// #384
 40257b4:	72a22240 	movk	w0, #0x1112, lsl #16
 40257b8:	97ffff08 	bl	40253d8 <lpds_hash_cmp>
 40257bc:	2a0003f3 	mov	w19, w0
 40257c0:	7102941f 	cmp	w0, #0xa5
 40257c4:	54fff9a1 	b.ne	40256f8 <handle_ddr_param_info+0x68>  // b.any
 40257c8:	d0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 40257cc:	91032400 	add	x0, x0, #0xc9
 40257d0:	94000945 	bl	4027ce4 <log_serial_puts>
 40257d4:	2a1303e0 	mov	w0, w19
 40257d8:	a9417bf5 	ldp	x21, x30, [sp, #16]
 40257dc:	a8c253f3 	ldp	x19, x20, [sp], #32
 40257e0:	d65f03c0 	ret
 40257e4:	528008a0 	mov	w0, #0x45                  	// #69
 40257e8:	17ffffb8 	b	40256c8 <handle_ddr_param_info+0x38>
 40257ec:	b94002a0 	ldr	w0, [x21]
 40257f0:	d3483c00 	ubfx	x0, x0, #8, #8
 40257f4:	7101081f 	cmp	w0, #0x42
 40257f8:	54fff801 	b.ne	40256f8 <handle_ddr_param_info+0x68>  // b.any
 40257fc:	528014b3 	mov	w19, #0xa5                  	// #165
 4025800:	17fffff5 	b	40257d4 <handle_ddr_param_info+0x144>
 4025804:	b94002a0 	ldr	w0, [x21]
 4025808:	d3505c00 	ubfx	x0, x0, #16, #8
 402580c:	7101081f 	cmp	w0, #0x42
 4025810:	54000160 	b.eq	402583c <handle_ddr_param_info+0x1ac>  // b.none
 4025814:	d2857000 	mov	x0, #0x2b80                	// #11136
 4025818:	91070283 	add	x3, x20, #0x1c0
 402581c:	aa1403e1 	mov	x1, x20
 4025820:	52803802 	mov	w2, #0x1c0                 	// #448
 4025824:	f2a08040 	movk	x0, #0x402, lsl #16
 4025828:	97fffe8b 	bl	4025254 <secure_verify_area>
 402582c:	7102941f 	cmp	w0, #0xa5
 4025830:	54000060 	b.eq	402583c <handle_ddr_param_info+0x1ac>  // b.none
 4025834:	52800b80 	mov	w0, #0x5c                  	// #92
 4025838:	17ffffa4 	b	40256c8 <handle_ddr_param_info+0x38>
 402583c:	91060283 	add	x3, x20, #0x180
 4025840:	aa1403e1 	mov	x1, x20
 4025844:	91020260 	add	x0, x19, #0x80
 4025848:	52803002 	mov	w2, #0x180                 	// #384
 402584c:	97fffe82 	bl	4025254 <secure_verify_area>
 4025850:	7102941f 	cmp	w0, #0xa5
 4025854:	54000261 	b.ne	40258a0 <handle_ddr_param_info+0x210>  // b.any
 4025858:	b9402a62 	ldr	w2, [x19, #40]
 402585c:	d2803980 	mov	x0, #0x1cc                 	// #460
 4025860:	b9401281 	ldr	w1, [x20, #16]
 4025864:	97fffde3 	bl	4024ff0 <check_version>
 4025868:	2a0003f3 	mov	w19, w0
 402586c:	7102941f 	cmp	w0, #0xa5
 4025870:	540001c1 	b.ne	40258a8 <handle_ddr_param_info+0x218>  // b.any
 4025874:	29430680 	ldp	w0, w1, [x20, #24]
 4025878:	97fffd80 	bl	4024e78 <check_msid_ext>
 402587c:	7102941f 	cmp	w0, #0xa5
 4025880:	54fff3c1 	b.ne	40256f8 <handle_ddr_param_info+0x68>  // b.any
 4025884:	d2801a00 	mov	x0, #0xd0                  	// #208
 4025888:	f2a22240 	movk	x0, #0x1112, lsl #16
 402588c:	97fff785 	bl	40236a0 <store_hash_to_lpds>
 4025890:	b9402280 	ldr	w0, [x20, #32]
 4025894:	11080000 	add	w0, w0, #0x200
 4025898:	97fff5e7 	bl	4023034 <add_boot_img_size>
 402589c:	17ffffce 	b	40257d4 <handle_ddr_param_info+0x144>
 40258a0:	52800ba0 	mov	w0, #0x5d                  	// #93
 40258a4:	17ffff89 	b	40256c8 <handle_ddr_param_info+0x38>
 40258a8:	52800c20 	mov	w0, #0x61                  	// #97
 40258ac:	17ffff87 	b	40256c8 <handle_ddr_param_info+0x38>

00000000040258b0 <handle_ddr_param>:
 40258b0:	d2902a81 	mov	x1, #0x8154                	// #33108
 40258b4:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 40258b8:	f2a203c1 	movk	x1, #0x101e, lsl #16
 40258bc:	2a0003f3 	mov	w19, w0
 40258c0:	a9017bf5 	stp	x21, x30, [sp, #16]
 40258c4:	b9400020 	ldr	w0, [x1]
 40258c8:	d3483c00 	ubfx	x0, x0, #8, #8
 40258cc:	7101081f 	cmp	w0, #0x42
 40258d0:	540000e1 	b.ne	40258ec <handle_ddr_param+0x3c>  // b.any
 40258d4:	b9400020 	ldr	w0, [x1]
 40258d8:	d3483c00 	ubfx	x0, x0, #8, #8
 40258dc:	7101081f 	cmp	w0, #0x42
 40258e0:	540004a0 	b.eq	4025974 <handle_ddr_param+0xc4>  // b.none
 40258e4:	52800b53 	mov	w19, #0x5a                  	// #90
 40258e8:	1400001c 	b	4025958 <handle_ddr_param+0xa8>
 40258ec:	97fffe06 	bl	4025104 <get_ddr_param_info_addr>
 40258f0:	2a0003f4 	mov	w20, w0
 40258f4:	8b204260 	add	x0, x19, w0, uxtw
 40258f8:	b9402a81 	ldr	w1, [x20, #40]
 40258fc:	3944b000 	ldrb	w0, [x0, #300]
 4025900:	6b00003f 	cmp	w1, w0
 4025904:	54000329 	b.ls	4025968 <handle_ddr_param+0xb8>  // b.plast
 4025908:	97fffe04 	bl	4025118 <get_ddr_param_data_addr>
 402590c:	8b131693 	add	x19, x20, x19, lsl #5
 4025910:	b9402681 	ldr	w1, [x20, #36]
 4025914:	2a0003f5 	mov	w21, w0
 4025918:	9100b273 	add	x19, x19, #0x2c
 402591c:	aa1503e0 	mov	x0, x21
 4025920:	aa1303e2 	mov	x2, x19
 4025924:	97fffe0d 	bl	4025158 <hash_compare>
 4025928:	7102941f 	cmp	w0, #0xa5
 402592c:	54fffdc1 	b.ne	40258e4 <handle_ddr_param+0x34>  // b.any
 4025930:	b9402681 	ldr	w1, [x20, #36]
 4025934:	aa1303e2 	mov	x2, x19
 4025938:	aa1503e0 	mov	x0, x21
 402593c:	97fffe07 	bl	4025158 <hash_compare>
 4025940:	2a0003f3 	mov	w19, w0
 4025944:	7102941f 	cmp	w0, #0xa5
 4025948:	54fffce1 	b.ne	40258e4 <handle_ddr_param+0x34>  // b.any
 402594c:	29448680 	ldp	w0, w1, [x20, #36]
 4025950:	1b007c20 	mul	w0, w1, w0
 4025954:	97fff5b8 	bl	4023034 <add_boot_img_size>
 4025958:	2a1303e0 	mov	w0, w19
 402595c:	a9417bf5 	ldp	x21, x30, [sp, #16]
 4025960:	a8c253f3 	ldp	x19, x20, [sp], #32
 4025964:	d65f03c0 	ret
 4025968:	52800ca0 	mov	w0, #0x65                  	// #101
 402596c:	97fff9c3 	bl	4024078 <err_print>
 4025970:	17ffffdd 	b	40258e4 <handle_ddr_param+0x34>
 4025974:	528014b3 	mov	w19, #0xa5                  	// #165
 4025978:	17fffff8 	b	4025958 <handle_ddr_param+0xa8>

000000000402597c <handle_uboot_info>:
 402597c:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4025980:	d2a82014 	mov	x20, #0x41000000            	// #1090519040
 4025984:	5283c5a0 	mov	w0, #0x1e2d                	// #7725
 4025988:	b9400281 	ldr	w1, [x20]
 402598c:	72a97e00 	movk	w0, #0x4bf0, lsl #16
 4025990:	a9017bf5 	stp	x21, x30, [sp, #16]
 4025994:	6b00003f 	cmp	w1, w0
 4025998:	54000361 	b.ne	4025a04 <handle_uboot_info+0x88>  // b.any
 402599c:	d2902a95 	mov	x21, #0x8154                	// #33108
 40259a0:	f2a203d5 	movk	x21, #0x101e, lsl #16
 40259a4:	b94002a0 	ldr	w0, [x21]
 40259a8:	d3483c00 	ubfx	x0, x0, #8, #8
 40259ac:	7101081f 	cmp	w0, #0x42
 40259b0:	54000300 	b.eq	4025a10 <handle_uboot_info+0x94>  // b.none
 40259b4:	97fffdcf 	bl	40250f0 <get_ree_key_area_addr>
 40259b8:	2a0003f3 	mov	w19, w0
 40259bc:	97fff5cc 	bl	40230ec <get_low_power_mode>
 40259c0:	7101681f 	cmp	w0, #0x5a
 40259c4:	54000321 	b.ne	4025a28 <handle_uboot_info+0xac>  // b.any
 40259c8:	52801e00 	mov	w0, #0xf0                  	// #240
 40259cc:	aa1403e1 	mov	x1, x20
 40259d0:	52803002 	mov	w2, #0x180                 	// #384
 40259d4:	72a22240 	movk	w0, #0x1112, lsl #16
 40259d8:	97fffe80 	bl	40253d8 <lpds_hash_cmp>
 40259dc:	2a0003f3 	mov	w19, w0
 40259e0:	7102941f 	cmp	w0, #0xa5
 40259e4:	540001e1 	b.ne	4025a20 <handle_uboot_info+0xa4>  // b.any
 40259e8:	d0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 40259ec:	91037400 	add	x0, x0, #0xdd
 40259f0:	940008bd 	bl	4027ce4 <log_serial_puts>
 40259f4:	2a1303e0 	mov	w0, w19
 40259f8:	a9417bf5 	ldp	x21, x30, [sp, #16]
 40259fc:	a8c253f3 	ldp	x19, x20, [sp], #32
 4025a00:	d65f03c0 	ret
 4025a04:	528008a0 	mov	w0, #0x45                  	// #69
 4025a08:	97fff99c 	bl	4024078 <err_print>
 4025a0c:	14000005 	b	4025a20 <handle_uboot_info+0xa4>
 4025a10:	b94002a0 	ldr	w0, [x21]
 4025a14:	d3483c00 	ubfx	x0, x0, #8, #8
 4025a18:	7101081f 	cmp	w0, #0x42
 4025a1c:	54000600 	b.eq	4025adc <handle_uboot_info+0x160>  // b.none
 4025a20:	52800b53 	mov	w19, #0x5a                  	// #90
 4025a24:	17fffff4 	b	40259f4 <handle_uboot_info+0x78>
 4025a28:	b94002a0 	ldr	w0, [x21]
 4025a2c:	d3505c00 	ubfx	x0, x0, #16, #8
 4025a30:	7101081f 	cmp	w0, #0x42
 4025a34:	54000180 	b.eq	4025a64 <handle_uboot_info+0xe8>  // b.none
 4025a38:	d2803803 	mov	x3, #0x1c0                 	// #448
 4025a3c:	d2857000 	mov	x0, #0x2b80                	// #11136
 4025a40:	aa1403e1 	mov	x1, x20
 4025a44:	f2a82003 	movk	x3, #0x4100, lsl #16
 4025a48:	52803802 	mov	w2, #0x1c0                 	// #448
 4025a4c:	f2a08040 	movk	x0, #0x402, lsl #16
 4025a50:	97fffe01 	bl	4025254 <secure_verify_area>
 4025a54:	7102941f 	cmp	w0, #0xa5
 4025a58:	54000060 	b.eq	4025a64 <handle_uboot_info+0xe8>  // b.none
 4025a5c:	52800b80 	mov	w0, #0x5c                  	// #92
 4025a60:	17ffffea 	b	4025a08 <handle_uboot_info+0x8c>
 4025a64:	2a1303e0 	mov	w0, w19
 4025a68:	d2803003 	mov	x3, #0x180                 	// #384
 4025a6c:	91020000 	add	x0, x0, #0x80
 4025a70:	f2a82003 	movk	x3, #0x4100, lsl #16
 4025a74:	52803002 	mov	w2, #0x180                 	// #384
 4025a78:	d2a82001 	mov	x1, #0x41000000            	// #1090519040
 4025a7c:	97fffdf6 	bl	4025254 <secure_verify_area>
 4025a80:	7102941f 	cmp	w0, #0xa5
 4025a84:	54000241 	b.ne	4025acc <handle_uboot_info+0x150>  // b.any
 4025a88:	d2a82014 	mov	x20, #0x41000000            	// #1090519040
 4025a8c:	d2803a00 	mov	x0, #0x1d0                 	// #464
 4025a90:	29420a81 	ldp	w1, w2, [x20, #16]
 4025a94:	97fffd57 	bl	4024ff0 <check_version>
 4025a98:	2a0003f3 	mov	w19, w0
 4025a9c:	7102941f 	cmp	w0, #0xa5
 4025aa0:	540001a1 	b.ne	4025ad4 <handle_uboot_info+0x158>  // b.any
 4025aa4:	29430680 	ldp	w0, w1, [x20, #24]
 4025aa8:	97fffcf4 	bl	4024e78 <check_msid_ext>
 4025aac:	7102941f 	cmp	w0, #0xa5
 4025ab0:	54fffb81 	b.ne	4025a20 <handle_uboot_info+0xa4>  // b.any
 4025ab4:	d2801e00 	mov	x0, #0xf0                  	// #240
 4025ab8:	f2a22240 	movk	x0, #0x1112, lsl #16
 4025abc:	97fff6f9 	bl	40236a0 <store_hash_to_lpds>
 4025ac0:	52804000 	mov	w0, #0x200                 	// #512
 4025ac4:	97fff55c 	bl	4023034 <add_boot_img_size>
 4025ac8:	17ffffcb 	b	40259f4 <handle_uboot_info+0x78>
 4025acc:	52800ba0 	mov	w0, #0x5d                  	// #93
 4025ad0:	17ffffce 	b	4025a08 <handle_uboot_info+0x8c>
 4025ad4:	52800c20 	mov	w0, #0x61                  	// #97
 4025ad8:	17ffffcc 	b	4025a08 <handle_uboot_info+0x8c>
 4025adc:	528014b3 	mov	w19, #0xa5                  	// #165
 4025ae0:	17ffffc5 	b	40259f4 <handle_uboot_info+0x78>

0000000004025ae4 <handle_uboot_code>:
 4025ae4:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4025ae8:	a9017bf5 	stp	x21, x30, [sp, #16]
 4025aec:	d2a82015 	mov	x21, #0x41000000            	// #1090519040
 4025af0:	b94026b4 	ldr	w20, [x21, #36]
 4025af4:	b9408ab3 	ldr	w19, [x21, #136]
 4025af8:	94000263 	bl	4026484 <is_ree_boot_dec_en_enable>
 4025afc:	528d34a1 	mov	w1, #0x69a5                	// #27045
 4025b00:	72b86b41 	movk	w1, #0xc35a, lsl #16
 4025b04:	6b01001f 	cmp	w0, w1
 4025b08:	540000c1 	b.ne	4025b20 <handle_uboot_code+0x3c>  // b.any
 4025b0c:	b9404aa1 	ldr	w1, [x21, #72]
 4025b10:	5292dc20 	mov	w0, #0x96e1                	// #38625
 4025b14:	72a78f00 	movk	w0, #0x3c78, lsl #16
 4025b18:	6b00003f 	cmp	w1, w0
 4025b1c:	54000240 	b.eq	4025b64 <handle_uboot_code+0x80>  // b.none
 4025b20:	d2800901 	mov	x1, #0x48                  	// #72
 4025b24:	52800080 	mov	w0, #0x4                   	// #4
 4025b28:	2a1403e4 	mov	w4, w20
 4025b2c:	aa1303e3 	mov	x3, x19
 4025b30:	aa1303e2 	mov	x2, x19
 4025b34:	f2a82001 	movk	x1, #0x4100, lsl #16
 4025b38:	72a06000 	movk	w0, #0x300, lsl #16
 4025b3c:	97fff5b8 	bl	402321c <decrypt_data>
 4025b40:	7102941f 	cmp	w0, #0xa5
 4025b44:	54000100 	b.eq	4025b64 <handle_uboot_code+0x80>  // b.none
 4025b48:	52800a00 	mov	w0, #0x50                  	// #80
 4025b4c:	97fff94b 	bl	4024078 <err_print>
 4025b50:	52800b53 	mov	w19, #0x5a                  	// #90
 4025b54:	2a1303e0 	mov	w0, w19
 4025b58:	a9417bf5 	ldp	x21, x30, [sp, #16]
 4025b5c:	a8c253f3 	ldp	x19, x20, [sp], #32
 4025b60:	d65f03c0 	ret
 4025b64:	d2902a81 	mov	x1, #0x8154                	// #33108
 4025b68:	f2a203c1 	movk	x1, #0x101e, lsl #16
 4025b6c:	b9400020 	ldr	w0, [x1]
 4025b70:	d3483c00 	ubfx	x0, x0, #8, #8
 4025b74:	7101081f 	cmp	w0, #0x42
 4025b78:	54000280 	b.eq	4025bc8 <handle_uboot_code+0xe4>  // b.none
 4025b7c:	d2800502 	mov	x2, #0x28                  	// #40
 4025b80:	2a1403e1 	mov	w1, w20
 4025b84:	aa1303e0 	mov	x0, x19
 4025b88:	f2a82002 	movk	x2, #0x4100, lsl #16
 4025b8c:	97fffd73 	bl	4025158 <hash_compare>
 4025b90:	7102941f 	cmp	w0, #0xa5
 4025b94:	54fffde1 	b.ne	4025b50 <handle_uboot_code+0x6c>  // b.any
 4025b98:	d2800502 	mov	x2, #0x28                  	// #40
 4025b9c:	aa1303e0 	mov	x0, x19
 4025ba0:	2a1403e1 	mov	w1, w20
 4025ba4:	f2a82002 	movk	x2, #0x4100, lsl #16
 4025ba8:	97fffd6c 	bl	4025158 <hash_compare>
 4025bac:	2a0003f3 	mov	w19, w0
 4025bb0:	7102941f 	cmp	w0, #0xa5
 4025bb4:	54fffce1 	b.ne	4025b50 <handle_uboot_code+0x6c>  // b.any
 4025bb8:	d2a82000 	mov	x0, #0x41000000            	// #1090519040
 4025bbc:	b9402400 	ldr	w0, [x0, #36]
 4025bc0:	97fff51d 	bl	4023034 <add_boot_img_size>
 4025bc4:	17ffffe4 	b	4025b54 <handle_uboot_code+0x70>
 4025bc8:	b9400020 	ldr	w0, [x1]
 4025bcc:	d3483c00 	ubfx	x0, x0, #8, #8
 4025bd0:	7101081f 	cmp	w0, #0x42
 4025bd4:	54fffbe1 	b.ne	4025b50 <handle_uboot_code+0x6c>  // b.any
 4025bd8:	528014b3 	mov	w19, #0xa5                  	// #165
 4025bdc:	17ffffde 	b	4025b54 <handle_uboot_code+0x70>

0000000004025be0 <handle_tee_key>:
 4025be0:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4025be4:	528d34b4 	mov	w20, #0x69a5                	// #27045
 4025be8:	72b86b54 	movk	w20, #0xc35a, lsl #16
 4025bec:	aa0003f3 	mov	x19, x0
 4025bf0:	f9000bfe 	str	x30, [sp, #16]
 4025bf4:	94000202 	bl	40263fc <is_soc_tee_enable>
 4025bf8:	6b14001f 	cmp	w0, w20
 4025bfc:	54000200 	b.eq	4025c3c <handle_tee_key+0x5c>  // b.none
 4025c00:	d2902a80 	mov	x0, #0x8154                	// #33108
 4025c04:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4025c08:	b9400000 	ldr	w0, [x0]
 4025c0c:	12001c00 	and	w0, w0, #0xff
 4025c10:	7101081f 	cmp	w0, #0x42
 4025c14:	54000140 	b.eq	4025c3c <handle_tee_key+0x5c>  // b.none
 4025c18:	b9400261 	ldr	w1, [x19]
 4025c1c:	5281e3c0 	mov	w0, #0xf1e                 	// #3870
 4025c20:	72a97c20 	movk	w0, #0x4be1, lsl #16
 4025c24:	6b00003f 	cmp	w1, w0
 4025c28:	54000260 	b.eq	4025c74 <handle_tee_key+0x94>  // b.none
 4025c2c:	528008a0 	mov	w0, #0x45                  	// #69
 4025c30:	97fff912 	bl	4024078 <err_print>
 4025c34:	52800b40 	mov	w0, #0x5a                  	// #90
 4025c38:	14000005 	b	4025c4c <handle_tee_key+0x6c>
 4025c3c:	940001f0 	bl	40263fc <is_soc_tee_enable>
 4025c40:	6b14001f 	cmp	w0, w20
 4025c44:	540000a1 	b.ne	4025c58 <handle_tee_key+0x78>  // b.any
 4025c48:	528014a0 	mov	w0, #0xa5                  	// #165
 4025c4c:	f9400bfe 	ldr	x30, [sp, #16]
 4025c50:	a8c253f3 	ldp	x19, x20, [sp], #32
 4025c54:	d65f03c0 	ret
 4025c58:	d2902a80 	mov	x0, #0x8154                	// #33108
 4025c5c:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4025c60:	b9400000 	ldr	w0, [x0]
 4025c64:	12001c00 	and	w0, w0, #0xff
 4025c68:	7101081f 	cmp	w0, #0x42
 4025c6c:	54fffd61 	b.ne	4025c18 <handle_tee_key+0x38>  // b.any
 4025c70:	17fffff6 	b	4025c48 <handle_tee_key+0x68>
 4025c74:	d2855000 	mov	x0, #0x2a80                	// #10880
 4025c78:	91030263 	add	x3, x19, #0xc0
 4025c7c:	aa1303e1 	mov	x1, x19
 4025c80:	52801802 	mov	w2, #0xc0                  	// #192
 4025c84:	f2a08040 	movk	x0, #0x402, lsl #16
 4025c88:	97fffd73 	bl	4025254 <secure_verify_area>
 4025c8c:	7102941f 	cmp	w0, #0xa5
 4025c90:	54000281 	b.ne	4025ce0 <handle_tee_key+0x100>  // b.any
 4025c94:	29448a61 	ldp	w1, w2, [x19, #36]
 4025c98:	d2803700 	mov	x0, #0x1b8                 	// #440
 4025c9c:	97fffcd5 	bl	4024ff0 <check_version>
 4025ca0:	7102941f 	cmp	w0, #0xa5
 4025ca4:	54000221 	b.ne	4025ce8 <handle_tee_key+0x108>  // b.any
 4025ca8:	29458660 	ldp	w0, w1, [x19, #44]
 4025cac:	97fffc73 	bl	4024e78 <check_msid_ext>
 4025cb0:	7102941f 	cmp	w0, #0xa5
 4025cb4:	54fffc01 	b.ne	4025c34 <handle_tee_key+0x54>  // b.any
 4025cb8:	b9403661 	ldr	w1, [x19, #52]
 4025cbc:	5292dc20 	mov	w0, #0x96e1                	// #38625
 4025cc0:	72a78f00 	movk	w0, #0x3c78, lsl #16
 4025cc4:	6b00003f 	cmp	w1, w0
 4025cc8:	54fffc01 	b.ne	4025c48 <handle_tee_key+0x68>  // b.any
 4025ccc:	9100e260 	add	x0, x19, #0x38
 4025cd0:	97fffc8d 	bl	4024f04 <die_id_compare>
 4025cd4:	7102941f 	cmp	w0, #0xa5
 4025cd8:	54fffae1 	b.ne	4025c34 <handle_tee_key+0x54>  // b.any
 4025cdc:	17ffffdb 	b	4025c48 <handle_tee_key+0x68>
 4025ce0:	52800ba0 	mov	w0, #0x5d                  	// #93
 4025ce4:	17ffffd3 	b	4025c30 <handle_tee_key+0x50>
 4025ce8:	52800c20 	mov	w0, #0x61                  	// #97
 4025cec:	17ffffd1 	b	4025c30 <handle_tee_key+0x50>

0000000004025cf0 <handle_tee_info>:
 4025cf0:	d100c3ff 	sub	sp, sp, #0x30
 4025cf4:	a90153f3 	stp	x19, x20, [sp, #16]
 4025cf8:	aa0103f3 	mov	x19, x1
 4025cfc:	aa0003f4 	mov	x20, x0
 4025d00:	a9027bf5 	stp	x21, x30, [sp, #32]
 4025d04:	90ffffe0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4025d08:	f9455001 	ldr	x1, [x0, #2720]
 4025d0c:	f90007e1 	str	x1, [sp, #8]
 4025d10:	d2800001 	mov	x1, #0x0                   	// #0
 4025d14:	5281e5a0 	mov	w0, #0xf2d                 	// #3885
 4025d18:	b9400261 	ldr	w1, [x19]
 4025d1c:	72a97c20 	movk	w0, #0x4be1, lsl #16
 4025d20:	6b00003f 	cmp	w1, w0
 4025d24:	54000221 	b.ne	4025d68 <handle_tee_info+0x78>  // b.any
 4025d28:	528d34b5 	mov	w21, #0x69a5                	// #27045
 4025d2c:	940001b4 	bl	40263fc <is_soc_tee_enable>
 4025d30:	72b86b55 	movk	w21, #0xc35a, lsl #16
 4025d34:	6b15001f 	cmp	w0, w21
 4025d38:	540001e1 	b.ne	4025d74 <handle_tee_info+0x84>  // b.any
 4025d3c:	940001b0 	bl	40263fc <is_soc_tee_enable>
 4025d40:	6b15001f 	cmp	w0, w21
 4025d44:	54000581 	b.ne	4025df4 <handle_tee_info+0x104>  // b.any
 4025d48:	528014a0 	mov	w0, #0xa5                  	// #165
 4025d4c:	90ffffe1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4025d50:	f94007e2 	ldr	x2, [sp, #8]
 4025d54:	f9455023 	ldr	x3, [x1, #2720]
 4025d58:	eb030042 	subs	x2, x2, x3
 4025d5c:	d2800003 	mov	x3, #0x0                   	// #0
 4025d60:	54000a20 	b.eq	4025ea4 <handle_tee_info+0x1b4>  // b.none
 4025d64:	97fff902 	bl	402416c <__stack_chk_fail>
 4025d68:	528008a0 	mov	w0, #0x45                  	// #69
 4025d6c:	97fff8c3 	bl	4024078 <err_print>
 4025d70:	1400004b 	b	4025e9c <handle_tee_info+0x1ac>
 4025d74:	d2902a80 	mov	x0, #0x8154                	// #33108
 4025d78:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4025d7c:	b9400000 	ldr	w0, [x0]
 4025d80:	12001c00 	and	w0, w0, #0xff
 4025d84:	7101081f 	cmp	w0, #0x42
 4025d88:	54fffda0 	b.eq	4025d3c <handle_tee_info+0x4c>  // b.none
 4025d8c:	d2902a80 	mov	x0, #0x8154                	// #33108
 4025d90:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4025d94:	b9400000 	ldr	w0, [x0]
 4025d98:	d3505c00 	ubfx	x0, x0, #16, #8
 4025d9c:	7101081f 	cmp	w0, #0x42
 4025da0:	54000381 	b.ne	4025e10 <handle_tee_info+0x120>  // b.any
 4025da4:	91060263 	add	x3, x19, #0x180
 4025da8:	aa1303e1 	mov	x1, x19
 4025dac:	91020280 	add	x0, x20, #0x80
 4025db0:	52803002 	mov	w2, #0x180                 	// #384
 4025db4:	97fffd28 	bl	4025254 <secure_verify_area>
 4025db8:	7102941f 	cmp	w0, #0xa5
 4025dbc:	540003e1 	b.ne	4025e38 <handle_tee_info+0x148>  // b.any
 4025dc0:	b9402a80 	ldr	w0, [x20, #40]
 4025dc4:	910013e1 	add	x1, sp, #0x4
 4025dc8:	b9401275 	ldr	w21, [x19, #16]
 4025dcc:	d2800002 	mov	x2, #0x0                   	// #0
 4025dd0:	2a2003f4 	mvn	w20, w0
 4025dd4:	52803700 	mov	w0, #0x1b8                 	// #440
 4025dd8:	94000763 	bl	4027b64 <uapi_drv_otp_read_word>
 4025ddc:	7102941f 	cmp	w0, #0xa5
 4025de0:	54000300 	b.eq	4025e40 <handle_tee_info+0x150>  // b.none
 4025de4:	52800bc0 	mov	w0, #0x5e                  	// #94
 4025de8:	97fff8a4 	bl	4024078 <err_print>
 4025dec:	52800c20 	mov	w0, #0x61                  	// #97
 4025df0:	17ffffdf 	b	4025d6c <handle_tee_info+0x7c>
 4025df4:	d2902a80 	mov	x0, #0x8154                	// #33108
 4025df8:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4025dfc:	b9400000 	ldr	w0, [x0]
 4025e00:	12001c00 	and	w0, w0, #0xff
 4025e04:	7101081f 	cmp	w0, #0x42
 4025e08:	54fffc21 	b.ne	4025d8c <handle_tee_info+0x9c>  // b.any
 4025e0c:	17ffffcf 	b	4025d48 <handle_tee_info+0x58>
 4025e10:	d2857000 	mov	x0, #0x2b80                	// #11136
 4025e14:	91070263 	add	x3, x19, #0x1c0
 4025e18:	aa1303e1 	mov	x1, x19
 4025e1c:	52803802 	mov	w2, #0x1c0                 	// #448
 4025e20:	f2a08040 	movk	x0, #0x402, lsl #16
 4025e24:	97fffd0c 	bl	4025254 <secure_verify_area>
 4025e28:	7102941f 	cmp	w0, #0xa5
 4025e2c:	54fffbc0 	b.eq	4025da4 <handle_tee_info+0xb4>  // b.none
 4025e30:	52800b80 	mov	w0, #0x5c                  	// #92
 4025e34:	17ffffce 	b	4025d6c <handle_tee_info+0x7c>
 4025e38:	52800ba0 	mov	w0, #0x5d                  	// #93
 4025e3c:	17ffffcc 	b	4025d6c <handle_tee_info+0x7c>
 4025e40:	b94007e0 	ldr	w0, [sp, #4]
 4025e44:	910013e1 	add	x1, sp, #0x4
 4025e48:	d2800002 	mov	x2, #0x0                   	// #0
 4025e4c:	0a000280 	and	w0, w20, w0
 4025e50:	b90007e0 	str	w0, [sp, #4]
 4025e54:	97fffc62 	bl	4024fdc <count_bit_set>
 4025e58:	2a0003f4 	mov	w20, w0
 4025e5c:	52803780 	mov	w0, #0x1bc                 	// #444
 4025e60:	94000741 	bl	4027b64 <uapi_drv_otp_read_word>
 4025e64:	7102941f 	cmp	w0, #0xa5
 4025e68:	54fffbe1 	b.ne	4025de4 <handle_tee_info+0xf4>  // b.any
 4025e6c:	b94007e0 	ldr	w0, [sp, #4]
 4025e70:	97fffc5b 	bl	4024fdc <count_bit_set>
 4025e74:	0b000294 	add	w20, w20, w0
 4025e78:	710102bf 	cmp	w21, #0x40
 4025e7c:	7a5492a0 	ccmp	w21, w20, #0x0, ls  // ls = plast
 4025e80:	54000062 	b.cs	4025e8c <handle_tee_info+0x19c>  // b.hs, b.nlast
 4025e84:	52800c00 	mov	w0, #0x60                  	// #96
 4025e88:	17ffffd8 	b	4025de8 <handle_tee_info+0xf8>
 4025e8c:	29430660 	ldp	w0, w1, [x19, #24]
 4025e90:	97fffbfa 	bl	4024e78 <check_msid_ext>
 4025e94:	7102941f 	cmp	w0, #0xa5
 4025e98:	54fff580 	b.eq	4025d48 <handle_tee_info+0x58>  // b.none
 4025e9c:	52800b40 	mov	w0, #0x5a                  	// #90
 4025ea0:	17ffffab 	b	4025d4c <handle_tee_info+0x5c>
 4025ea4:	a94153f3 	ldp	x19, x20, [sp, #16]
 4025ea8:	a9427bf5 	ldp	x21, x30, [sp, #32]
 4025eac:	9100c3ff 	add	sp, sp, #0x30
 4025eb0:	d65f03c0 	ret

0000000004025eb4 <handle_atf_code>:
 4025eb4:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 4025eb8:	aa0103f4 	mov	x20, x1
 4025ebc:	a9015bf5 	stp	x21, x22, [sp, #16]
 4025ec0:	aa0203f5 	mov	x21, x2
 4025ec4:	b940b836 	ldr	w22, [x1, #184]
 4025ec8:	a90263f7 	stp	x23, x24, [sp, #32]
 4025ecc:	12bfffc1 	mov	w1, #0x1ffff               	// #131071
 4025ed0:	510006c2 	sub	w2, w22, #0x1
 4025ed4:	f9001bfe 	str	x30, [sp, #48]
 4025ed8:	6b01005f 	cmp	w2, w1
 4025edc:	54000109 	b.ls	4025efc <handle_atf_code+0x48>  // b.plast
 4025ee0:	52800b53 	mov	w19, #0x5a                  	// #90
 4025ee4:	2a1303e0 	mov	w0, w19
 4025ee8:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4025eec:	a94263f7 	ldp	x23, x24, [sp, #32]
 4025ef0:	f9401bfe 	ldr	x30, [sp, #48]
 4025ef4:	a8c453f3 	ldp	x19, x20, [sp], #64
 4025ef8:	d65f03c0 	ret
 4025efc:	910c0013 	add	x19, x0, #0x300
 4025f00:	94000159 	bl	4026464 <is_tee_dec_en_enable>
 4025f04:	2a0003f7 	mov	w23, w0
 4025f08:	528d34a0 	mov	w0, #0x69a5                	// #27045
 4025f0c:	72b86b40 	movk	w0, #0xc35a, lsl #16
 4025f10:	6b0002ff 	cmp	w23, w0
 4025f14:	54000441 	b.ne	4025f9c <handle_atf_code+0xe8>  // b.any
 4025f18:	b9404a98 	ldr	w24, [x20, #72]
 4025f1c:	5292dc20 	mov	w0, #0x96e1                	// #38625
 4025f20:	72a78f00 	movk	w0, #0x3c78, lsl #16
 4025f24:	6b00031f 	cmp	w24, w0
 4025f28:	540003a1 	b.ne	4025f9c <handle_atf_code+0xe8>  // b.any
 4025f2c:	9400014e 	bl	4026464 <is_tee_dec_en_enable>
 4025f30:	6b17001f 	cmp	w0, w23
 4025f34:	54000341 	b.ne	4025f9c <handle_atf_code+0xe8>  // b.any
 4025f38:	b9404a80 	ldr	w0, [x20, #72]
 4025f3c:	6b18001f 	cmp	w0, w24
 4025f40:	540002e1 	b.ne	4025f9c <handle_atf_code+0xe8>  // b.any
 4025f44:	aa1303e2 	mov	x2, x19
 4025f48:	2a1603e1 	mov	w1, w22
 4025f4c:	d2a80020 	mov	x0, #0x40010000            	// #1073807360
 4025f50:	97fff66d 	bl	4023904 <dma_copy>
 4025f54:	2a0003f3 	mov	w19, w0
 4025f58:	7102941f 	cmp	w0, #0xa5
 4025f5c:	54000301 	b.ne	4025fbc <handle_atf_code+0x108>  // b.any
 4025f60:	528d34b6 	mov	w22, #0x69a5                	// #27045
 4025f64:	94000126 	bl	40263fc <is_soc_tee_enable>
 4025f68:	72b86b56 	movk	w22, #0xc35a, lsl #16
 4025f6c:	6b16001f 	cmp	w0, w22
 4025f70:	540002c1 	b.ne	4025fc8 <handle_atf_code+0x114>  // b.any
 4025f74:	94000122 	bl	40263fc <is_soc_tee_enable>
 4025f78:	6b16001f 	cmp	w0, w22
 4025f7c:	54fffb40 	b.eq	4025ee4 <handle_atf_code+0x30>  // b.none
 4025f80:	d2902a80 	mov	x0, #0x8154                	// #33108
 4025f84:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4025f88:	b9400000 	ldr	w0, [x0]
 4025f8c:	12001c00 	and	w0, w0, #0xff
 4025f90:	7101081f 	cmp	w0, #0x42
 4025f94:	54000261 	b.ne	4025fe0 <handle_atf_code+0x12c>  // b.any
 4025f98:	17ffffd3 	b	4025ee4 <handle_atf_code+0x30>
 4025f9c:	52800020 	mov	w0, #0x1                   	// #1
 4025fa0:	2a1603e4 	mov	w4, w22
 4025fa4:	aa1303e3 	mov	x3, x19
 4025fa8:	91012281 	add	x1, x20, #0x48
 4025fac:	d2a80022 	mov	x2, #0x40010000            	// #1073807360
 4025fb0:	72a06000 	movk	w0, #0x300, lsl #16
 4025fb4:	97fff49a 	bl	402321c <decrypt_data>
 4025fb8:	17ffffe7 	b	4025f54 <handle_atf_code+0xa0>
 4025fbc:	52800a00 	mov	w0, #0x50                  	// #80
 4025fc0:	97fff82e 	bl	4024078 <err_print>
 4025fc4:	17ffffc7 	b	4025ee0 <handle_atf_code+0x2c>
 4025fc8:	d2902a80 	mov	x0, #0x8154                	// #33108
 4025fcc:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4025fd0:	b9400000 	ldr	w0, [x0]
 4025fd4:	12001c00 	and	w0, w0, #0xff
 4025fd8:	7101081f 	cmp	w0, #0x42
 4025fdc:	54fffcc0 	b.eq	4025f74 <handle_atf_code+0xc0>  // b.none
 4025fe0:	b940ba81 	ldr	w1, [x20, #184]
 4025fe4:	91025296 	add	x22, x20, #0x94
 4025fe8:	aa1603e2 	mov	x2, x22
 4025fec:	aa1503e0 	mov	x0, x21
 4025ff0:	97fffc5a 	bl	4025158 <hash_compare>
 4025ff4:	7102941f 	cmp	w0, #0xa5
 4025ff8:	54fff741 	b.ne	4025ee0 <handle_atf_code+0x2c>  // b.any
 4025ffc:	b940ba81 	ldr	w1, [x20, #184]
 4026000:	aa1603e2 	mov	x2, x22
 4026004:	aa1503e0 	mov	x0, x21
 4026008:	97fffc54 	bl	4025158 <hash_compare>
 402600c:	7102941f 	cmp	w0, #0xa5
 4026010:	54fff6a0 	b.eq	4025ee4 <handle_atf_code+0x30>  // b.none
 4026014:	17ffffb3 	b	4025ee0 <handle_atf_code+0x2c>

0000000004026018 <handle_tee_code>:
 4026018:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402601c:	aa0103f4 	mov	x20, x1
 4026020:	a9015bf5 	stp	x21, x22, [sp, #16]
 4026024:	aa0203f5 	mov	x21, x2
 4026028:	b9402436 	ldr	w22, [x1, #36]
 402602c:	a90263f7 	stp	x23, x24, [sp, #32]
 4026030:	12bfe001 	mov	w1, #0xffffff              	// #16777215
 4026034:	510006c2 	sub	w2, w22, #0x1
 4026038:	f9001bfe 	str	x30, [sp, #48]
 402603c:	6b01005f 	cmp	w2, w1
 4026040:	54000069 	b.ls	402604c <handle_tee_code+0x34>  // b.plast
 4026044:	52800b53 	mov	w19, #0x5a                  	// #90
 4026048:	14000025 	b	40260dc <handle_tee_code+0xc4>
 402604c:	910c0000 	add	x0, x0, #0x300
 4026050:	b9409293 	ldr	w19, [x20, #144]
 4026054:	8b000273 	add	x19, x19, x0
 4026058:	94000103 	bl	4026464 <is_tee_dec_en_enable>
 402605c:	2a0003f7 	mov	w23, w0
 4026060:	528d34a0 	mov	w0, #0x69a5                	// #27045
 4026064:	72b86b40 	movk	w0, #0xc35a, lsl #16
 4026068:	6b0002ff 	cmp	w23, w0
 402606c:	54000441 	b.ne	40260f4 <handle_tee_code+0xdc>  // b.any
 4026070:	b9404a98 	ldr	w24, [x20, #72]
 4026074:	5292dc20 	mov	w0, #0x96e1                	// #38625
 4026078:	72a78f00 	movk	w0, #0x3c78, lsl #16
 402607c:	6b00031f 	cmp	w24, w0
 4026080:	540003a1 	b.ne	40260f4 <handle_tee_code+0xdc>  // b.any
 4026084:	940000f8 	bl	4026464 <is_tee_dec_en_enable>
 4026088:	6b17001f 	cmp	w0, w23
 402608c:	54000341 	b.ne	40260f4 <handle_tee_code+0xdc>  // b.any
 4026090:	b9404a80 	ldr	w0, [x20, #72]
 4026094:	6b18001f 	cmp	w0, w24
 4026098:	540002e1 	b.ne	40260f4 <handle_tee_code+0xdc>  // b.any
 402609c:	d29ffc80 	mov	x0, #0xffe4                	// #65508
 40260a0:	aa1303e2 	mov	x2, x19
 40260a4:	2a1603e1 	mov	w1, w22
 40260a8:	f2a80040 	movk	x0, #0x4002, lsl #16
 40260ac:	97fff616 	bl	4023904 <dma_copy>
 40260b0:	2a0003f3 	mov	w19, w0
 40260b4:	7102941f 	cmp	w0, #0xa5
 40260b8:	54000301 	b.ne	4026118 <handle_tee_code+0x100>  // b.any
 40260bc:	528d34b6 	mov	w22, #0x69a5                	// #27045
 40260c0:	940000cf 	bl	40263fc <is_soc_tee_enable>
 40260c4:	72b86b56 	movk	w22, #0xc35a, lsl #16
 40260c8:	6b16001f 	cmp	w0, w22
 40260cc:	540002c1 	b.ne	4026124 <handle_tee_code+0x10c>  // b.any
 40260d0:	940000cb 	bl	40263fc <is_soc_tee_enable>
 40260d4:	6b16001f 	cmp	w0, w22
 40260d8:	54000481 	b.ne	4026168 <handle_tee_code+0x150>  // b.any
 40260dc:	2a1303e0 	mov	w0, w19
 40260e0:	a9415bf5 	ldp	x21, x22, [sp, #16]
 40260e4:	a94263f7 	ldp	x23, x24, [sp, #32]
 40260e8:	f9401bfe 	ldr	x30, [sp, #48]
 40260ec:	a8c453f3 	ldp	x19, x20, [sp], #64
 40260f0:	d65f03c0 	ret
 40260f4:	d29ffc82 	mov	x2, #0xffe4                	// #65508
 40260f8:	52800020 	mov	w0, #0x1                   	// #1
 40260fc:	2a1603e4 	mov	w4, w22
 4026100:	aa1303e3 	mov	x3, x19
 4026104:	91012281 	add	x1, x20, #0x48
 4026108:	f2a80042 	movk	x2, #0x4002, lsl #16
 402610c:	72a06000 	movk	w0, #0x300, lsl #16
 4026110:	97fff443 	bl	402321c <decrypt_data>
 4026114:	17ffffe7 	b	40260b0 <handle_tee_code+0x98>
 4026118:	52800a00 	mov	w0, #0x50                  	// #80
 402611c:	97fff7d7 	bl	4024078 <err_print>
 4026120:	17ffffc9 	b	4026044 <handle_tee_code+0x2c>
 4026124:	940000ad 	bl	40263d8 <is_tee_verify_enable>
 4026128:	6b16001f 	cmp	w0, w22
 402612c:	54fffd20 	b.eq	40260d0 <handle_tee_code+0xb8>  // b.none
 4026130:	b9402681 	ldr	w1, [x20, #36]
 4026134:	9100a296 	add	x22, x20, #0x28
 4026138:	aa1603e2 	mov	x2, x22
 402613c:	aa1503e0 	mov	x0, x21
 4026140:	97fffc06 	bl	4025158 <hash_compare>
 4026144:	7102941f 	cmp	w0, #0xa5
 4026148:	54fff7e1 	b.ne	4026044 <handle_tee_code+0x2c>  // b.any
 402614c:	b9402681 	ldr	w1, [x20, #36]
 4026150:	aa1603e2 	mov	x2, x22
 4026154:	aa1503e0 	mov	x0, x21
 4026158:	97fffc00 	bl	4025158 <hash_compare>
 402615c:	7102941f 	cmp	w0, #0xa5
 4026160:	54fffbe0 	b.eq	40260dc <handle_tee_code+0xc4>  // b.none
 4026164:	17ffffb8 	b	4026044 <handle_tee_code+0x2c>
 4026168:	9400009c 	bl	40263d8 <is_tee_verify_enable>
 402616c:	6b16001f 	cmp	w0, w22
 4026170:	54fffe01 	b.ne	4026130 <handle_tee_code+0x118>  // b.any
 4026174:	17ffffda 	b	40260dc <handle_tee_code+0xc4>

0000000004026178 <cal_crc_perbyte>:
 4026178:	531c2c23 	ubfiz	w3, w1, #4, #12
 402617c:	53041c04 	ubfx	w4, w0, #4, #4
 4026180:	530c3c21 	ubfx	w1, w1, #12, #4
 4026184:	90000042 	adrp	x2, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4026188:	4a040021 	eor	w1, w1, w4
 402618c:	91394042 	add	x2, x2, #0xe50
 4026190:	12000c00 	and	w0, w0, #0xf
 4026194:	7861d841 	ldrh	w1, [x2, w1, sxtw #1]
 4026198:	4a030021 	eor	w1, w1, w3
 402619c:	531c2c23 	ubfiz	w3, w1, #4, #12
 40261a0:	4a413001 	eor	w1, w0, w1, lsr #12
 40261a4:	7861d840 	ldrh	w0, [x2, w1, sxtw #1]
 40261a8:	4a030000 	eor	w0, w0, w3
 40261ac:	d65f03c0 	ret

00000000040261b0 <malloc_init>:
 40261b0:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 40261b4:	f0ffffc1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 40261b8:	912b0033 	add	x19, x1, #0xac0
 40261bc:	2a0003f4 	mov	w20, w0
 40261c0:	d2802403 	mov	x3, #0x120                 	// #288
 40261c4:	aa1303e0 	mov	x0, x19
 40261c8:	aa0303e1 	mov	x1, x3
 40261cc:	52800002 	mov	w2, #0x0                   	// #0
 40261d0:	f9000bfe 	str	x30, [sp, #16]
 40261d4:	94000700 	bl	4027dd4 <memset_s>
 40261d8:	35000620 	cbnz	w0, 402629c <malloc_init+0xec>
 40261dc:	90000042 	adrp	x2, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 40261e0:	aa1303e1 	mov	x1, x19
 40261e4:	9139c042 	add	x2, x2, #0xe70
 40261e8:	79400440 	ldrh	w0, [x2, #2]
 40261ec:	35000120 	cbnz	w0, 4026210 <malloc_init+0x60>
 40261f0:	d2a04000 	mov	x0, #0x2000000             	// #33554432
 40261f4:	f9000020 	str	x0, [x1]
 40261f8:	aa1303e1 	mov	x1, x19
 40261fc:	b9400820 	ldr	w0, [x1, #8]
 4026200:	350001a0 	cbnz	w0, 4026234 <malloc_init+0x84>
 4026204:	f9400bfe 	ldr	x30, [sp, #16]
 4026208:	a8c253f3 	ldp	x19, x20, [sp], #32
 402620c:	d65f03c0 	ret
 4026210:	78404443 	ldrh	w3, [x2], #4
 4026214:	91008021 	add	x1, x1, #0x20
 4026218:	b81e8020 	stur	w0, [x1, #-24]
 402621c:	92403c64 	and	x4, x3, #0xffff
 4026220:	f81e0024 	stur	x4, [x1, #-32]
 4026224:	f81f0034 	stur	x20, [x1, #-16]
 4026228:	d503201f 	nop
 402622c:	9ba05074 	umaddl	x20, w3, w0, x20
 4026230:	17ffffee 	b	40261e8 <malloc_init+0x38>
 4026234:	91008022 	add	x2, x1, #0x20
 4026238:	aa0203e3 	mov	x3, x2
 402623c:	b9400864 	ldr	w4, [x3, #8]
 4026240:	35000224 	cbnz	w4, 4026284 <malloc_init+0xd4>
 4026244:	d1008043 	sub	x3, x2, #0x20
 4026248:	eb03003f 	cmp	x1, x3
 402624c:	54000180 	b.eq	402627c <malloc_init+0xcc>  // b.none
 4026250:	f9400825 	ldr	x5, [x1, #16]
 4026254:	f85f0044 	ldur	x4, [x2, #-16]
 4026258:	f81f0045 	stur	x5, [x2, #-16]
 402625c:	b9400825 	ldr	w5, [x1, #8]
 4026260:	b81e8045 	stur	w5, [x2, #-24]
 4026264:	f9400025 	ldr	x5, [x1]
 4026268:	f85e0043 	ldur	x3, [x2, #-32]
 402626c:	f81e0045 	stur	x5, [x2, #-32]
 4026270:	f9000023 	str	x3, [x1]
 4026274:	b9000820 	str	w0, [x1, #8]
 4026278:	f9000824 	str	x4, [x1, #16]
 402627c:	aa0203e1 	mov	x1, x2
 4026280:	17ffffdf 	b	40261fc <malloc_init+0x4c>
 4026284:	f9400024 	ldr	x4, [x1]
 4026288:	f9400065 	ldr	x5, [x3]
 402628c:	eb0400bf 	cmp	x5, x4
 4026290:	9a832021 	csel	x1, x1, x3, cs  // cs = hs, nlast
 4026294:	91008063 	add	x3, x3, #0x20
 4026298:	17ffffe9 	b	402623c <malloc_init+0x8c>
 402629c:	12800000 	mov	w0, #0xffffffff            	// #-1
 40262a0:	17ffffd9 	b	4026204 <malloc_init+0x54>

00000000040262a4 <malloc>:
 40262a4:	f0ffffc1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 40262a8:	912b0021 	add	x1, x1, #0xac0
 40262ac:	f9400022 	ldr	x2, [x1]
 40262b0:	eb00005f 	cmp	x2, x0
 40262b4:	54000163 	b.cc	40262e0 <malloc+0x3c>  // b.lo, b.ul, b.last
 40262b8:	f9400c25 	ldr	x5, [x1, #24]
 40262bc:	52800003 	mov	w3, #0x0                   	// #0
 40262c0:	b9400824 	ldr	w4, [x1, #8]
 40262c4:	aa0503e0 	mov	x0, x5
 40262c8:	6b03009f 	cmp	w4, w3
 40262cc:	540000e1 	b.ne	40262e8 <malloc+0x44>  // b.any
 40262d0:	91008021 	add	x1, x1, #0x20
 40262d4:	35ffff24 	cbnz	w4, 40262b8 <malloc+0x14>
 40262d8:	d2800000 	mov	x0, #0x0                   	// #0
 40262dc:	1400000d 	b	4026310 <malloc+0x6c>
 40262e0:	91008021 	add	x1, x1, #0x20
 40262e4:	17fffff2 	b	40262ac <malloc+0x8>
 40262e8:	37000160 	tbnz	w0, #0, 4026314 <malloc+0x70>
 40262ec:	f9400024 	ldr	x4, [x1]
 40262f0:	2a0303e0 	mov	w0, w3
 40262f4:	f9400822 	ldr	x2, [x1, #16]
 40262f8:	9b040800 	madd	x0, x0, x4, x2
 40262fc:	52800022 	mov	w2, #0x1                   	// #1
 4026300:	1ac32042 	lsl	w2, w2, w3
 4026304:	93407c42 	sxtw	x2, w2
 4026308:	aa050042 	orr	x2, x2, x5
 402630c:	f9000c22 	str	x2, [x1, #24]
 4026310:	d65f03c0 	ret
 4026314:	d341fc00 	lsr	x0, x0, #1
 4026318:	11000463 	add	w3, w3, #0x1
 402631c:	17ffffeb 	b	40262c8 <malloc+0x24>

0000000004026320 <free>:
 4026320:	f0ffffc1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4026324:	912b0021 	add	x1, x1, #0xac0
 4026328:	b9800822 	ldrsw	x2, [x1, #8]
 402632c:	f9400024 	ldr	x4, [x1]
 4026330:	f9400823 	ldr	x3, [x1, #16]
 4026334:	9b047c42 	mul	x2, x2, x4
 4026338:	cb030003 	sub	x3, x0, x3
 402633c:	eb02007f 	cmp	x3, x2
 4026340:	540001c2 	b.cs	4026378 <free+0x58>  // b.hs, b.nlast
 4026344:	52800002 	mov	w2, #0x0                   	// #0
 4026348:	2a0203e0 	mov	w0, w2
 402634c:	9b047c00 	mul	x0, x0, x4
 4026350:	eb00007f 	cmp	x3, x0
 4026354:	54000168 	b.hi	4026380 <free+0x60>  // b.pmore
 4026358:	52800020 	mov	w0, #0x1                   	// #1
 402635c:	1ac22000 	lsl	w0, w0, w2
 4026360:	2a2003e0 	mvn	w0, w0
 4026364:	f9400c22 	ldr	x2, [x1, #24]
 4026368:	93407c00 	sxtw	x0, w0
 402636c:	8a000040 	and	x0, x2, x0
 4026370:	f9000c20 	str	x0, [x1, #24]
 4026374:	d65f03c0 	ret
 4026378:	91008021 	add	x1, x1, #0x20
 402637c:	17ffffeb 	b	4026328 <free+0x8>
 4026380:	11000442 	add	w2, w2, #0x1
 4026384:	17fffff1 	b	4026348 <free+0x28>

0000000004026388 <timer_get_divider>:
 4026388:	d2817700 	mov	x0, #0xbb8                 	// #3000
 402638c:	d65f03c0 	ret

0000000004026390 <is_ree_verify_enable>:
 4026390:	d2800280 	mov	x0, #0x14                  	// #20
 4026394:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4026398:	b9400001 	ldr	w1, [x0]
 402639c:	528d34a0 	mov	w0, #0x69a5                	// #27045
 40263a0:	72b86b40 	movk	w0, #0xc35a, lsl #16
 40263a4:	d3505c21 	ubfx	x1, x1, #16, #8
 40263a8:	7101083f 	cmp	w1, #0x42
 40263ac:	5a800000 	cinv	w0, w0, ne  // ne = any
 40263b0:	d65f03c0 	ret

00000000040263b4 <is_tp_verify_enable>:
 40263b4:	d2800280 	mov	x0, #0x14                  	// #20
 40263b8:	f2a203c0 	movk	x0, #0x101e, lsl #16
 40263bc:	b9400001 	ldr	w1, [x0]
 40263c0:	528d34a0 	mov	w0, #0x69a5                	// #27045
 40263c4:	72b86b40 	movk	w0, #0xc35a, lsl #16
 40263c8:	d3483c21 	ubfx	x1, x1, #8, #8
 40263cc:	7101083f 	cmp	w1, #0x42
 40263d0:	5a800000 	cinv	w0, w0, ne  // ne = any
 40263d4:	d65f03c0 	ret

00000000040263d8 <is_tee_verify_enable>:
 40263d8:	d2800280 	mov	x0, #0x14                  	// #20
 40263dc:	f2a203c0 	movk	x0, #0x101e, lsl #16
 40263e0:	b9400001 	ldr	w1, [x0]
 40263e4:	528d34a0 	mov	w0, #0x69a5                	// #27045
 40263e8:	72b86b40 	movk	w0, #0xc35a, lsl #16
 40263ec:	53187c21 	lsr	w1, w1, #24
 40263f0:	7101083f 	cmp	w1, #0x42
 40263f4:	5a800000 	cinv	w0, w0, ne  // ne = any
 40263f8:	d65f03c0 	ret

00000000040263fc <is_soc_tee_enable>:
 40263fc:	d2800200 	mov	x0, #0x10                  	// #16
 4026400:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4026404:	b9400001 	ldr	w1, [x0]
 4026408:	528d34a0 	mov	w0, #0x69a5                	// #27045
 402640c:	72b86b40 	movk	w0, #0xc35a, lsl #16
 4026410:	d3505c21 	ubfx	x1, x1, #16, #8
 4026414:	7101083f 	cmp	w1, #0x42
 4026418:	5a800000 	cinv	w0, w0, ne  // ne = any
 402641c:	d65f03c0 	ret

0000000004026420 <is_sec_dbg_enable>:
 4026420:	d2803c00 	mov	x0, #0x1e0                 	// #480
 4026424:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4026428:	b9400001 	ldr	w1, [x0]
 402642c:	5292cb40 	mov	w0, #0x965a                	// #38490
 4026430:	72a794a0 	movk	w0, #0x3ca5, lsl #16
 4026434:	d3505c21 	ubfx	x1, x1, #16, #8
 4026438:	7101083f 	cmp	w1, #0x42
 402643c:	5a800000 	cinv	w0, w0, ne  // ne = any
 4026440:	d65f03c0 	ret

0000000004026444 <is_sec_dbg_lv_enable>:
 4026444:	d2803c00 	mov	x0, #0x1e0                 	// #480
 4026448:	f2a203c0 	movk	x0, #0x101e, lsl #16
 402644c:	b9400001 	ldr	w1, [x0]
 4026450:	5292cb40 	mov	w0, #0x965a                	// #38490
 4026454:	72a794a0 	movk	w0, #0x3ca5, lsl #16
 4026458:	7218043f 	tst	w1, #0x300
 402645c:	5a800000 	cinv	w0, w0, ne  // ne = any
 4026460:	d65f03c0 	ret

0000000004026464 <is_tee_dec_en_enable>:
 4026464:	d2800080 	mov	x0, #0x4                   	// #4
 4026468:	f2a203c0 	movk	x0, #0x101e, lsl #16
 402646c:	b9400001 	ldr	w1, [x0]
 4026470:	5292cb40 	mov	w0, #0x965a                	// #38490
 4026474:	72a794a0 	movk	w0, #0x3ca5, lsl #16
 4026478:	72100c3f 	tst	w1, #0xf0000
 402647c:	5a801000 	cinv	w0, w0, eq  // eq = none
 4026480:	d65f03c0 	ret

0000000004026484 <is_ree_boot_dec_en_enable>:
 4026484:	d2800080 	mov	x0, #0x4                   	// #4
 4026488:	f2a203c0 	movk	x0, #0x101e, lsl #16
 402648c:	b9400001 	ldr	w1, [x0]
 4026490:	5292cb40 	mov	w0, #0x965a                	// #38490
 4026494:	72a794a0 	movk	w0, #0x3ca5, lsl #16
 4026498:	720c0c3f 	tst	w1, #0xf00000
 402649c:	5a801000 	cinv	w0, w0, eq  // eq = none
 40264a0:	d65f03c0 	ret

00000000040264a4 <opt_get_boot_backup_enable>:
 40264a4:	d2a203c0 	mov	x0, #0x101e0000            	// #270401536
 40264a8:	b9400000 	ldr	w0, [x0]
 40264ac:	d3471c00 	ubfx	x0, x0, #7, #1
 40264b0:	d65f03c0 	ret

00000000040264b4 <check_volt_val>:
 40264b4:	b9400003 	ldr	w3, [x0]
 40264b8:	6b01007f 	cmp	w3, w1
 40264bc:	54000069 	b.ls	40264c8 <check_volt_val+0x14>  // b.plast
 40264c0:	b9000001 	str	w1, [x0]
 40264c4:	d65f03c0 	ret
 40264c8:	6b02007f 	cmp	w3, w2
 40264cc:	54ffffc2 	b.cs	40264c4 <check_volt_val+0x10>  // b.hs, b.nlast
 40264d0:	b9000002 	str	w2, [x0]
 40264d4:	17fffffc 	b	40264c4 <check_volt_val+0x10>

00000000040264d8 <svb_pwm_cfg>:
 40264d8:	2a0103e1 	mov	w1, w1
 40264dc:	53102400 	ubfiz	w0, w0, #16, #10
 40264e0:	52833ea2 	mov	w2, #0x19f5                	// #6645
 40264e4:	2a020000 	orr	w0, w0, w2
 40264e8:	b9000020 	str	w0, [x1]
 40264ec:	d65f03c0 	ret

00000000040264f0 <svb_error>:
 40264f0:	f81f0ffe 	str	x30, [sp, #-16]!
 40264f4:	b0000040 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 40264f8:	9103c400 	add	x0, x0, #0xf1
 40264fc:	940005fa 	bl	4027ce4 <log_serial_puts>
 4026500:	d2800040 	mov	x0, #0x2                   	// #2
 4026504:	94000614 	bl	4027d54 <mdelay>
 4026508:	94000603 	bl	4027d14 <timer_deinit>
 402650c:	d2800080 	mov	x0, #0x4                   	// #4
 4026510:	52800021 	mov	w1, #0x1                   	// #1
 4026514:	f2a22040 	movk	x0, #0x1102, lsl #16
 4026518:	f84107fe 	ldr	x30, [sp], #16
 402651c:	b9000001 	str	w1, [x0]
 4026520:	b9000001 	str	w1, [x0]
 4026524:	b9000001 	str	w1, [x0]
 4026528:	d65f03c0 	ret

000000000402652c <hpm_value_avg.constprop.0>:
 402652c:	d10043ff 	sub	sp, sp, #0x10
 4026530:	d2800001 	mov	x1, #0x0                   	// #0
 4026534:	b9000fff 	str	wzr, [sp, #12]
 4026538:	b8617803 	ldr	w3, [x0, x1, lsl #2]
 402653c:	91000421 	add	x1, x1, #0x1
 4026540:	b9400fe2 	ldr	w2, [sp, #12]
 4026544:	0b030042 	add	w2, w2, w3
 4026548:	b9000fe2 	str	w2, [sp, #12]
 402654c:	f100103f 	cmp	x1, #0x4
 4026550:	54ffff41 	b.ne	4026538 <hpm_value_avg.constprop.0+0xc>  // b.any
 4026554:	b9400fe0 	ldr	w0, [sp, #12]
 4026558:	910043ff 	add	sp, sp, #0x10
 402655c:	53057c00 	lsr	w0, w0, #5
 4026560:	d65f03c0 	ret

0000000004026564 <calc_volt_regval.constprop.0>:
 4026564:	d10043ff 	sub	sp, sp, #0x10
 4026568:	6b01001f 	cmp	w0, w1
 402656c:	540001c3 	b.cc	40265a4 <calc_volt_regval.constprop.0+0x40>  // b.lo, b.ul, b.last
 4026570:	51000420 	sub	w0, w1, #0x1
 4026574:	510a2022 	sub	w2, w1, #0x288
 4026578:	4b000020 	sub	w0, w1, w0
 402657c:	52803403 	mov	w3, #0x1a0                 	// #416
 4026580:	510a2421 	sub	w1, w1, #0x289
 4026584:	53017c42 	lsr	w2, w2, #1
 4026588:	1b030800 	madd	w0, w0, w3, w2
 402658c:	1ac10800 	udiv	w0, w0, w1
 4026590:	51000400 	sub	w0, w0, #0x1
 4026594:	b9000fe0 	str	w0, [sp, #12]
 4026598:	b9400fe0 	ldr	w0, [sp, #12]
 402659c:	910043ff 	add	sp, sp, #0x10
 40265a0:	d65f03c0 	ret
 40265a4:	710a281f 	cmp	w0, #0x28a
 40265a8:	52805142 	mov	w2, #0x28a                 	// #650
 40265ac:	1a822000 	csel	w0, w0, w2, cs  // cs = hs, nlast
 40265b0:	17fffff1 	b	4026574 <calc_volt_regval.constprop.0+0x10>

00000000040265b4 <set_volt>:
 40265b4:	d100c3ff 	sub	sp, sp, #0x30
 40265b8:	2a0103e7 	mov	w7, w1
 40265bc:	f0ffffc1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 40265c0:	2a0203e6 	mov	w6, w2
 40265c4:	aa0303e4 	mov	x4, x3
 40265c8:	52800023 	mov	w3, #0x1                   	// #1
 40265cc:	f90013fe 	str	x30, [sp, #32]
 40265d0:	5290d408 	mov	w8, #0x86a0                	// #34464
 40265d4:	72a00028 	movk	w8, #0x1, lsl #16
 40265d8:	f9455022 	ldr	x2, [x1, #2720]
 40265dc:	f9000fe2 	str	x2, [sp, #24]
 40265e0:	d2800002 	mov	x2, #0x0                   	// #0
 40265e4:	d2802181 	mov	x1, #0x10c                 	// #268
 40265e8:	f2a203c1 	movk	x1, #0x101e, lsl #16
 40265ec:	b9400021 	ldr	w1, [x1]
 40265f0:	b9000be1 	str	w1, [sp, #8]
 40265f4:	b9400be1 	ldr	w1, [sp, #8]
 40265f8:	b9400be2 	ldr	w2, [sp, #8]
 40265fc:	93401821 	sbfx	x1, x1, #0, #7
 4026600:	f279005f 	tst	x2, #0x80
 4026604:	5a83046a 	cneg	w10, w3, ne  // ne = any
 4026608:	1b017d4a 	mul	w10, w10, w1
 402660c:	d2802481 	mov	x1, #0x124                 	// #292
 4026610:	f2a203c1 	movk	x1, #0x101e, lsl #16
 4026614:	b9400021 	ldr	w1, [x1]
 4026618:	b90013e1 	str	w1, [sp, #16]
 402661c:	b94013e1 	ldr	w1, [sp, #16]
 4026620:	b94013e2 	ldr	w2, [sp, #16]
 4026624:	b94013e5 	ldr	w5, [sp, #16]
 4026628:	93505821 	sbfx	x1, x1, #16, #7
 402662c:	f269005f 	tst	x2, #0x800000
 4026630:	b9400082 	ldr	w2, [x4]
 4026634:	5a830469 	cneg	w9, w3, ne  // ne = any
 4026638:	935878a5 	sbfx	x5, x5, #24, #7
 402663c:	1b017d29 	mul	w9, w9, w1
 4026640:	b94013e1 	ldr	w1, [sp, #16]
 4026644:	7100003f 	cmp	w1, #0x0
 4026648:	b9400481 	ldr	w1, [x4, #4]
 402664c:	5a83a463 	cneg	w3, w3, lt  // lt = tstop
 4026650:	7104341f 	cmp	w0, #0x10d
 4026654:	1b028402 	msub	w2, w0, w2, w1
 4026658:	b9400c81 	ldr	w1, [x4, #12]
 402665c:	1b057c65 	mul	w5, w3, w5
 4026660:	1ac80842 	udiv	w2, w2, w8
 4026664:	1a818040 	csel	w0, w2, w1, hi  // hi = pmore
 4026668:	b9400882 	ldr	w2, [x4, #8]
 402666c:	b90017e0 	str	w0, [sp, #20]
 4026670:	910053e0 	add	x0, sp, #0x14
 4026674:	97ffff90 	bl	40264b4 <check_volt_val>
 4026678:	b94017e1 	ldr	w1, [sp, #20]
 402667c:	52806722 	mov	w2, #0x339                 	// #825
 4026680:	0b0a0021 	add	w1, w1, w10
 4026684:	b90017e1 	str	w1, [sp, #20]
 4026688:	52807e41 	mov	w1, #0x3f2                 	// #1010
 402668c:	97ffff8a 	bl	40264b4 <check_volt_val>
 4026690:	79402be1 	ldrh	w1, [sp, #20]
 4026694:	52800000 	mov	w0, #0x0                   	// #0
 4026698:	33003c20 	bfxil	w0, w1, #0, #16
 402669c:	33103c20 	bfi	w0, w1, #16, #16
 40266a0:	d2806881 	mov	x1, #0x344                 	// #836
 40266a4:	f2a22041 	movk	x1, #0x1102, lsl #16
 40266a8:	b9000020 	str	w0, [x1]
 40266ac:	528069a1 	mov	w1, #0x34d                 	// #845
 40266b0:	b94017e0 	ldr	w0, [sp, #20]
 40266b4:	5100a000 	sub	w0, w0, #0x28
 40266b8:	710d301f 	cmp	w0, #0x34c
 40266bc:	1a818000 	csel	w0, w0, w1, hi  // hi = pmore
 40266c0:	52808361 	mov	w1, #0x41b                 	// #1051
 40266c4:	b90017e0 	str	w0, [sp, #20]
 40266c8:	97ffffa7 	bl	4026564 <calc_volt_regval.constprop.0>
 40266cc:	52920001 	mov	w1, #0x9000                	// #36864
 40266d0:	72a22041 	movk	w1, #0x1102, lsl #16
 40266d4:	97ffff81 	bl	40264d8 <svb_pwm_cfg>
 40266d8:	29428881 	ldp	w1, w2, [x4, #20]
 40266dc:	910053e0 	add	x0, sp, #0x14
 40266e0:	1b0188e7 	msub	w7, w7, w1, w2
 40266e4:	29438482 	ldp	w2, w1, [x4, #28]
 40266e8:	1ac808e7 	udiv	w7, w7, w8
 40266ec:	b90017e7 	str	w7, [sp, #20]
 40266f0:	97ffff71 	bl	40264b4 <check_volt_val>
 40266f4:	b94017e1 	ldr	w1, [sp, #20]
 40266f8:	52806542 	mov	w2, #0x32a                 	// #810
 40266fc:	0b090021 	add	w1, w1, w9
 4026700:	b90017e1 	str	w1, [sp, #20]
 4026704:	52808321 	mov	w1, #0x419                 	// #1049
 4026708:	97ffff6b 	bl	40264b4 <check_volt_val>
 402670c:	79402be2 	ldrh	w2, [sp, #20]
 4026710:	52800000 	mov	w0, #0x0                   	// #0
 4026714:	33003c40 	bfxil	w0, w2, #0, #16
 4026718:	33103c40 	bfi	w0, w2, #16, #16
 402671c:	d2806902 	mov	x2, #0x348                 	// #840
 4026720:	f2a22042 	movk	x2, #0x1102, lsl #16
 4026724:	b9000040 	str	w0, [x2]
 4026728:	b94017e0 	ldr	w0, [sp, #20]
 402672c:	97ffff8e 	bl	4026564 <calc_volt_regval.constprop.0>
 4026730:	52920081 	mov	w1, #0x9004                	// #36868
 4026734:	72a22041 	movk	w1, #0x1102, lsl #16
 4026738:	97ffff68 	bl	40264d8 <svb_pwm_cfg>
 402673c:	b9403881 	ldr	w1, [x4, #56]
 4026740:	35000461 	cbnz	w1, 40267cc <set_volt+0x218>
 4026744:	29450482 	ldp	w2, w1, [x4, #40]
 4026748:	910053e0 	add	x0, sp, #0x14
 402674c:	1b0284c6 	msub	w6, w6, w2, w1
 4026750:	29460482 	ldp	w2, w1, [x4, #48]
 4026754:	1ac808c6 	udiv	w6, w6, w8
 4026758:	b90017e6 	str	w6, [sp, #20]
 402675c:	97ffff56 	bl	40264b4 <check_volt_val>
 4026760:	b94017e3 	ldr	w3, [sp, #20]
 4026764:	52808321 	mov	w1, #0x419                 	// #1049
 4026768:	52806542 	mov	w2, #0x32a                 	// #810
 402676c:	910053e0 	add	x0, sp, #0x14
 4026770:	0b050065 	add	w5, w3, w5
 4026774:	b90017e5 	str	w5, [sp, #20]
 4026778:	97ffff4f 	bl	40264b4 <check_volt_val>
 402677c:	79402be1 	ldrh	w1, [sp, #20]
 4026780:	52800000 	mov	w0, #0x0                   	// #0
 4026784:	33003c20 	bfxil	w0, w1, #0, #16
 4026788:	33103c20 	bfi	w0, w1, #16, #16
 402678c:	d2806981 	mov	x1, #0x34c                 	// #844
 4026790:	f2a22041 	movk	x1, #0x1102, lsl #16
 4026794:	b9000020 	str	w0, [x1]
 4026798:	52808361 	mov	w1, #0x41b                 	// #1051
 402679c:	b94017e0 	ldr	w0, [sp, #20]
 40267a0:	97ffff71 	bl	4026564 <calc_volt_regval.constprop.0>
 40267a4:	52920101 	mov	w1, #0x9008                	// #36872
 40267a8:	72a22041 	movk	w1, #0x1102, lsl #16
 40267ac:	97ffff4b 	bl	40264d8 <svb_pwm_cfg>
 40267b0:	f0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 40267b4:	f9400fe1 	ldr	x1, [sp, #24]
 40267b8:	f9455002 	ldr	x2, [x0, #2720]
 40267bc:	eb020021 	subs	x1, x1, x2
 40267c0:	d2800002 	mov	x2, #0x0                   	// #0
 40267c4:	54000080 	b.eq	40267d4 <set_volt+0x220>  // b.none
 40267c8:	97fff669 	bl	402416c <__stack_chk_fail>
 40267cc:	b90017e1 	str	w1, [sp, #20]
 40267d0:	17ffffe4 	b	4026760 <set_volt+0x1ac>
 40267d4:	f94013fe 	ldr	x30, [sp, #32]
 40267d8:	d2800280 	mov	x0, #0x14                  	// #20
 40267dc:	9100c3ff 	add	sp, sp, #0x30
 40267e0:	14000559 	b	4027d44 <udelay>

00000000040267e4 <start_svb>:
 40267e4:	d10283ff 	sub	sp, sp, #0xa0
 40267e8:	f0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 40267ec:	52800062 	mov	w2, #0x3                   	// #3
 40267f0:	d2963203 	mov	x3, #0xb190                	// #45456
 40267f4:	72ac0102 	movk	w2, #0x6008, lsl #16
 40267f8:	f2a22043 	movk	x3, #0x1102, lsl #16
 40267fc:	a90553f3 	stp	x19, x20, [sp, #80]
 4026800:	a9065bf5 	stp	x21, x22, [sp, #96]
 4026804:	a90763f7 	stp	x23, x24, [sp, #112]
 4026808:	a9086bf9 	stp	x25, x26, [sp, #128]
 402680c:	f9004bfe 	str	x30, [sp, #144]
 4026810:	f9455001 	ldr	x1, [x0, #2720]
 4026814:	f90027e1 	str	x1, [sp, #72]
 4026818:	d2800001 	mov	x1, #0x0                   	// #0
 402681c:	d2802d01 	mov	x1, #0x168                 	// #360
 4026820:	f2a22041 	movk	x1, #0x1102, lsl #16
 4026824:	b9400020 	ldr	w0, [x1]
 4026828:	72a02000 	movk	w0, #0x100, lsl #16
 402682c:	b9000020 	str	w0, [x1]
 4026830:	d2940000 	mov	x0, #0xa000                	// #40960
 4026834:	529f8001 	mov	w1, #0xfc00                	// #64512
 4026838:	f2a22040 	movk	x0, #0x1102, lsl #16
 402683c:	72b811e1 	movk	w1, #0xc08f, lsl #16
 4026840:	b9000001 	str	w1, [x0]
 4026844:	d2892f81 	mov	x1, #0x497c                	// #18812
 4026848:	f2a22021 	movk	x1, #0x1101, lsl #16
 402684c:	b900041f 	str	wzr, [x0, #4]
 4026850:	52800200 	mov	w0, #0x10                  	// #16
 4026854:	b9000020 	str	w0, [x1]
 4026858:	b9000420 	str	w0, [x1, #4]
 402685c:	b9000820 	str	w0, [x1, #8]
 4026860:	d2962000 	mov	x0, #0xb100                	// #45312
 4026864:	f2a22040 	movk	x0, #0x1102, lsl #16
 4026868:	52807fe1 	mov	w1, #0x3ff                 	// #1023
 402686c:	b9000002 	str	w2, [x0]
 4026870:	b9000401 	str	w1, [x0, #4]
 4026874:	b8100002 	stur	w2, [x0, #-256]
 4026878:	b8104001 	stur	w1, [x0, #-252]
 402687c:	91004000 	add	x0, x0, #0x10
 4026880:	b902f002 	str	w2, [x0, #752]
 4026884:	b902f401 	str	w1, [x0, #756]
 4026888:	eb03001f 	cmp	x0, x3
 402688c:	54ffff01 	b.ne	402686c <start_svb+0x88>  // b.any
 4026890:	52808361 	mov	w1, #0x41b                 	// #1051
 4026894:	52807bc0 	mov	w0, #0x3de                 	// #990
 4026898:	97ffff33 	bl	4026564 <calc_volt_regval.constprop.0>
 402689c:	52802239 	mov	w25, #0x111                 	// #273
 40268a0:	52920001 	mov	w1, #0x9000                	// #36864
 40268a4:	d288e093 	mov	x19, #0x4704                	// #18180
 40268a8:	72a22041 	movk	w1, #0x1102, lsl #16
 40268ac:	97ffff0b 	bl	40264d8 <svb_pwm_cfg>
 40268b0:	52808321 	mov	w1, #0x419                 	// #1049
 40268b4:	52807bc0 	mov	w0, #0x3de                 	// #990
 40268b8:	97ffff2b 	bl	4026564 <calc_volt_regval.constprop.0>
 40268bc:	f2a22053 	movk	x19, #0x1102, lsl #16
 40268c0:	52920081 	mov	w1, #0x9004                	// #36868
 40268c4:	52802218 	mov	w24, #0x110                 	// #272
 40268c8:	72a22041 	movk	w1, #0x1102, lsl #16
 40268cc:	97ffff03 	bl	40264d8 <svb_pwm_cfg>
 40268d0:	52808361 	mov	w1, #0x41b                 	// #1051
 40268d4:	52807bc0 	mov	w0, #0x3de                 	// #990
 40268d8:	97ffff23 	bl	4026564 <calc_volt_regval.constprop.0>
 40268dc:	52802357 	mov	w23, #0x11a                 	// #282
 40268e0:	52920101 	mov	w1, #0x9008                	// #36872
 40268e4:	52806636 	mov	w22, #0x331                 	// #817
 40268e8:	72a22041 	movk	w1, #0x1102, lsl #16
 40268ec:	97fffefb 	bl	40264d8 <svb_pwm_cfg>
 40268f0:	d2893200 	mov	x0, #0x4990                	// #18832
 40268f4:	52802755 	mov	w21, #0x13a                 	// #314
 40268f8:	f2a22020 	movk	x0, #0x1101, lsl #16
 40268fc:	72a00035 	movk	w21, #0x1, lsl #16
 4026900:	52802754 	mov	w20, #0x13a                 	// #314
 4026904:	b9000019 	str	w25, [x0]
 4026908:	72a02034 	movk	w20, #0x101, lsl #16
 402690c:	b9000018 	str	w24, [x0]
 4026910:	d288e100 	mov	x0, #0x4708                	// #18184
 4026914:	f2a22040 	movk	x0, #0x1102, lsl #16
 4026918:	b9000277 	str	w23, [x19]
 402691c:	b9000016 	str	w22, [x0]
 4026920:	d2800140 	mov	x0, #0xa                   	// #10
 4026924:	b9000275 	str	w21, [x19]
 4026928:	94000507 	bl	4027d44 <udelay>
 402692c:	b9000274 	str	w20, [x19]
 4026930:	d2800140 	mov	x0, #0xa                   	// #10
 4026934:	94000504 	bl	4027d44 <udelay>
 4026938:	d2893280 	mov	x0, #0x4994                	// #18836
 402693c:	f2a22020 	movk	x0, #0x1101, lsl #16
 4026940:	b9000019 	str	w25, [x0]
 4026944:	b9000018 	str	w24, [x0]
 4026948:	d288e300 	mov	x0, #0x4718                	// #18200
 402694c:	f2a22040 	movk	x0, #0x1102, lsl #16
 4026950:	b9001277 	str	w23, [x19, #16]
 4026954:	b9000016 	str	w22, [x0]
 4026958:	d2800140 	mov	x0, #0xa                   	// #10
 402695c:	b9001275 	str	w21, [x19, #16]
 4026960:	940004f9 	bl	4027d44 <udelay>
 4026964:	b9001274 	str	w20, [x19, #16]
 4026968:	d2800140 	mov	x0, #0xa                   	// #10
 402696c:	940004fa 	bl	4027d54 <mdelay>
 4026970:	29037fff 	stp	wzr, wzr, [sp, #24]
 4026974:	d2962508 	mov	x8, #0xb128                	// #45352
 4026978:	d2962587 	mov	x7, #0xb12c                	// #45356
 402697c:	d2960906 	mov	x6, #0xb048                	// #45128
 4026980:	d2960985 	mov	x5, #0xb04c                	// #45132
 4026984:	d2968b04 	mov	x4, #0xb458                	// #46168
 4026988:	d2968b83 	mov	x3, #0xb45c                	// #46172
 402698c:	52800101 	mov	w1, #0x8                   	// #8
 4026990:	f2a22048 	movk	x8, #0x1102, lsl #16
 4026994:	f2a22047 	movk	x7, #0x1102, lsl #16
 4026998:	f2a22046 	movk	x6, #0x1102, lsl #16
 402699c:	f2a22045 	movk	x5, #0x1102, lsl #16
 40269a0:	f2a22044 	movk	x4, #0x1102, lsl #16
 40269a4:	f2a22043 	movk	x3, #0x1102, lsl #16
 40269a8:	29047fff 	stp	wzr, wzr, [sp, #32]
 40269ac:	29057fff 	stp	wzr, wzr, [sp, #40]
 40269b0:	29067fff 	stp	wzr, wzr, [sp, #48]
 40269b4:	29077fff 	stp	wzr, wzr, [sp, #56]
 40269b8:	29087fff 	stp	wzr, wzr, [sp, #64]
 40269bc:	b9400100 	ldr	w0, [x8]
 40269c0:	71000421 	subs	w1, w1, #0x1
 40269c4:	b9401fe2 	ldr	w2, [sp, #28]
 40269c8:	d3506409 	ubfx	x9, x0, #16, #10
 40269cc:	12002400 	and	w0, w0, #0x3ff
 40269d0:	0b090042 	add	w2, w2, w9
 40269d4:	b9001fe2 	str	w2, [sp, #28]
 40269d8:	b9401be2 	ldr	w2, [sp, #24]
 40269dc:	0b000040 	add	w0, w2, w0
 40269e0:	b9001be0 	str	w0, [sp, #24]
 40269e4:	b94027e2 	ldr	w2, [sp, #36]
 40269e8:	b94000e0 	ldr	w0, [x7]
 40269ec:	d3506409 	ubfx	x9, x0, #16, #10
 40269f0:	12002400 	and	w0, w0, #0x3ff
 40269f4:	0b090042 	add	w2, w2, w9
 40269f8:	b90027e2 	str	w2, [sp, #36]
 40269fc:	b94023e2 	ldr	w2, [sp, #32]
 4026a00:	0b000040 	add	w0, w2, w0
 4026a04:	b90023e0 	str	w0, [sp, #32]
 4026a08:	b9402fe2 	ldr	w2, [sp, #44]
 4026a0c:	b94000c0 	ldr	w0, [x6]
 4026a10:	d3506409 	ubfx	x9, x0, #16, #10
 4026a14:	12002400 	and	w0, w0, #0x3ff
 4026a18:	0b090042 	add	w2, w2, w9
 4026a1c:	b9002fe2 	str	w2, [sp, #44]
 4026a20:	b9402be2 	ldr	w2, [sp, #40]
 4026a24:	0b000040 	add	w0, w2, w0
 4026a28:	b9002be0 	str	w0, [sp, #40]
 4026a2c:	b94037e2 	ldr	w2, [sp, #52]
 4026a30:	b94000a0 	ldr	w0, [x5]
 4026a34:	d3506409 	ubfx	x9, x0, #16, #10
 4026a38:	12002400 	and	w0, w0, #0x3ff
 4026a3c:	0b090042 	add	w2, w2, w9
 4026a40:	b90037e2 	str	w2, [sp, #52]
 4026a44:	b94033e2 	ldr	w2, [sp, #48]
 4026a48:	0b000040 	add	w0, w2, w0
 4026a4c:	b90033e0 	str	w0, [sp, #48]
 4026a50:	b9403fe2 	ldr	w2, [sp, #60]
 4026a54:	b9400080 	ldr	w0, [x4]
 4026a58:	d3506409 	ubfx	x9, x0, #16, #10
 4026a5c:	12002400 	and	w0, w0, #0x3ff
 4026a60:	0b090042 	add	w2, w2, w9
 4026a64:	b9003fe2 	str	w2, [sp, #60]
 4026a68:	b9403be2 	ldr	w2, [sp, #56]
 4026a6c:	0b000040 	add	w0, w2, w0
 4026a70:	b9003be0 	str	w0, [sp, #56]
 4026a74:	b94047e2 	ldr	w2, [sp, #68]
 4026a78:	b9400060 	ldr	w0, [x3]
 4026a7c:	d3506409 	ubfx	x9, x0, #16, #10
 4026a80:	12002400 	and	w0, w0, #0x3ff
 4026a84:	0b090042 	add	w2, w2, w9
 4026a88:	b90047e2 	str	w2, [sp, #68]
 4026a8c:	b94043e2 	ldr	w2, [sp, #64]
 4026a90:	0b000040 	add	w0, w2, w0
 4026a94:	b90043e0 	str	w0, [sp, #64]
 4026a98:	54fff921 	b.ne	40269bc <start_svb+0x1d8>  // b.any
 4026a9c:	910063e0 	add	x0, sp, #0x18
 4026aa0:	97fffea3 	bl	402652c <hpm_value_avg.constprop.0>
 4026aa4:	2a0003e5 	mov	w5, w0
 4026aa8:	9100a3e0 	add	x0, sp, #0x28
 4026aac:	97fffea0 	bl	402652c <hpm_value_avg.constprop.0>
 4026ab0:	2a0003e4 	mov	w4, w0
 4026ab4:	9100e3e0 	add	x0, sp, #0x38
 4026ab8:	97fffe9d 	bl	402652c <hpm_value_avg.constprop.0>
 4026abc:	2a0003e3 	mov	w3, w0
 4026ac0:	d288e180 	mov	x0, #0x470c                	// #18188
 4026ac4:	f2a22040 	movk	x0, #0x1102, lsl #16
 4026ac8:	b9400014 	ldr	w20, [x0]
 4026acc:	b9401015 	ldr	w21, [x0, #16]
 4026ad0:	d2940100 	mov	x0, #0xa008                	// #40968
 4026ad4:	f2a22040 	movk	x0, #0x1102, lsl #16
 4026ad8:	b90017ff 	str	wzr, [sp, #20]
 4026adc:	12002298 	and	w24, w20, #0x1ff
 4026ae0:	120022b9 	and	w25, w21, #0x1ff
 4026ae4:	b9400000 	ldr	w0, [x0]
 4026ae8:	d3506294 	ubfx	x20, x20, #16, #9
 4026aec:	b90017e0 	str	w0, [sp, #20]
 4026af0:	d35062b5 	ubfx	x21, x21, #16, #9
 4026af4:	4b140316 	sub	w22, w24, w20
 4026af8:	4b150337 	sub	w23, w25, w21
 4026afc:	b94017e0 	ldr	w0, [sp, #20]
 4026b00:	12002400 	and	w0, w0, #0x3ff
 4026b04:	b90017e0 	str	w0, [sp, #20]
 4026b08:	528014a0 	mov	w0, #0xa5                  	// #165
 4026b0c:	b94017f3 	ldr	w19, [sp, #20]
 4026b10:	51021273 	sub	w19, w19, #0x84
 4026b14:	1b007e73 	mul	w19, w19, w0
 4026b18:	52806500 	mov	w0, #0x328                 	// #808
 4026b1c:	1ac00e73 	sdiv	w19, w19, w0
 4026b20:	d2802500 	mov	x0, #0x128                 	// #296
 4026b24:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4026b28:	b9400002 	ldr	w2, [x0]
 4026b2c:	5100a27a 	sub	w26, w19, #0x28
 4026b30:	72002440 	ands	w0, w2, #0x3ff
 4026b34:	d34a4c41 	ubfx	x1, x2, #10, #10
 4026b38:	7a401824 	ccmp	w1, #0x0, #0x4, ne  // ne = any
 4026b3c:	54000060 	b.eq	4026b48 <start_svb+0x364>  // b.none
 4026b40:	d3547442 	ubfx	x2, x2, #20, #10
 4026b44:	35001562 	cbnz	w2, 4026df0 <start_svb+0x60c>
 4026b48:	71040cbf 	cmp	w5, #0x103
 4026b4c:	540008ed 	b.le	4026c68 <start_svb+0x484>
 4026b50:	7101b75f 	cmp	w26, #0x6d
 4026b54:	5400018d 	b.le	4026b84 <start_svb+0x3a0>
 4026b58:	528a3481 	mov	w1, #0x51a4                	// #20900
 4026b5c:	5280ae60 	mov	w0, #0x573                 	// #1395
 4026b60:	72bfff61 	movk	w1, #0xfffb, lsl #16
 4026b64:	1b0004a0 	madd	w0, w5, w0, w1
 4026b68:	5284e201 	mov	w1, #0x2710                	// #10000
 4026b6c:	1ac10800 	udiv	w0, w0, w1
 4026b70:	7104209f 	cmp	w4, #0x108
 4026b74:	540008ec 	b.gt	4026c90 <start_svb+0x4ac>
 4026b78:	7103f87f 	cmp	w3, #0xfe
 4026b7c:	54000ccc 	b.gt	4026d14 <start_svb+0x530>
 4026b80:	1400003f 	b	4026c7c <start_svb+0x498>
 4026b84:	7101675f 	cmp	w26, #0x59
 4026b88:	540002ad 	b.le	4026bdc <start_svb+0x3f8>
 4026b8c:	529ec381 	mov	w1, #0xf61c                	// #63004
 4026b90:	528072e0 	mov	w0, #0x397                 	// #919
 4026b94:	72bfff81 	movk	w1, #0xfffc, lsl #16
 4026b98:	1b0004a0 	madd	w0, w5, w0, w1
 4026b9c:	5284e201 	mov	w1, #0x2710                	// #10000
 4026ba0:	1ac10800 	udiv	w0, w0, w1
 4026ba4:	7104209f 	cmp	w4, #0x108
 4026ba8:	540024ad 	b.le	402703c <start_svb+0x858>
 4026bac:	52823242 	mov	w2, #0x1192                	// #4498
 4026bb0:	52806661 	mov	w1, #0x333                 	// #819
 4026bb4:	72bfffa2 	movk	w2, #0xfffd, lsl #16
 4026bb8:	1b010881 	madd	w1, w4, w1, w2
 4026bbc:	5284e202 	mov	w2, #0x2710                	// #10000
 4026bc0:	1ac20821 	udiv	w1, w1, w2
 4026bc4:	7103f87f 	cmp	w3, #0xfe
 4026bc8:	5400074d 	b.le	4026cb0 <start_svb+0x4cc>
 4026bcc:	52859846 	mov	w6, #0x2cc2                	// #11458
 4026bd0:	52806a62 	mov	w2, #0x353                 	// #851
 4026bd4:	72bfffa6 	movk	w6, #0xfffd, lsl #16
 4026bd8:	14000053 	b	4026d24 <start_svb+0x540>
 4026bdc:	7101175f 	cmp	w26, #0x45
 4026be0:	540002ad 	b.le	4026c34 <start_svb+0x450>
 4026be4:	52853101 	mov	w1, #0x2988                	// #10632
 4026be8:	528044c0 	mov	w0, #0x226                 	// #550
 4026bec:	72bfffc1 	movk	w1, #0xfffe, lsl #16
 4026bf0:	1b0004a0 	madd	w0, w5, w0, w1
 4026bf4:	5284e201 	mov	w1, #0x2710                	// #10000
 4026bf8:	1ac10800 	udiv	w0, w0, w1
 4026bfc:	7104209f 	cmp	w4, #0x108
 4026c00:	5400226d 	b.le	402704c <start_svb+0x868>
 4026c04:	528f66a2 	mov	w2, #0x7b35                	// #31541
 4026c08:	528034c1 	mov	w1, #0x1a6                 	// #422
 4026c0c:	72bfffc2 	movk	w2, #0xfffe, lsl #16
 4026c10:	1b010881 	madd	w1, w4, w1, w2
 4026c14:	5284e202 	mov	w2, #0x2710                	// #10000
 4026c18:	1ac20821 	udiv	w1, w1, w2
 4026c1c:	7103f87f 	cmp	w3, #0xfe
 4026c20:	5400048d 	b.le	4026cb0 <start_svb+0x4cc>
 4026c24:	52893846 	mov	w6, #0x49c2                	// #18882
 4026c28:	528040a2 	mov	w2, #0x205                 	// #517
 4026c2c:	72bfffc6 	movk	w6, #0xfffe, lsl #16
 4026c30:	1400003d 	b	4026d24 <start_svb+0x540>
 4026c34:	36f81f3a 	tbz	w26, #31, 4027018 <start_svb+0x834>
 4026c38:	528f0861 	mov	w1, #0x7843                	// #30787
 4026c3c:	12803ba0 	mov	w0, #0xfffffe22            	// #-478
 4026c40:	72a00021 	movk	w1, #0x1, lsl #16
 4026c44:	1b0004a0 	madd	w0, w5, w0, w1
 4026c48:	5284e201 	mov	w1, #0x2710                	// #10000
 4026c4c:	1ac10c00 	sdiv	w0, w0, w1
 4026c50:	7104209f 	cmp	w4, #0x108
 4026c54:	540003cc 	b.gt	4026ccc <start_svb+0x4e8>
 4026c58:	7103f87f 	cmp	w3, #0xfe
 4026c5c:	5400010d 	b.le	4026c7c <start_svb+0x498>
 4026c60:	52800001 	mov	w1, #0x0                   	// #0
 4026c64:	14000022 	b	4026cec <start_svb+0x508>
 4026c68:	7104209f 	cmp	w4, #0x108
 4026c6c:	540000cc 	b.gt	4026c84 <start_svb+0x4a0>
 4026c70:	7103f87f 	cmp	w3, #0xfe
 4026c74:	540004ac 	b.gt	4026d08 <start_svb+0x524>
 4026c78:	52800000 	mov	w0, #0x0                   	// #0
 4026c7c:	52800001 	mov	w1, #0x0                   	// #0
 4026c80:	1400000c 	b	4026cb0 <start_svb+0x4cc>
 4026c84:	52800000 	mov	w0, #0x0                   	// #0
 4026c88:	7101b75f 	cmp	w26, #0x6d
 4026c8c:	5400016d 	b.le	4026cb8 <start_svb+0x4d4>
 4026c90:	529a2d42 	mov	w2, #0xd16a                	// #53610
 4026c94:	52809601 	mov	w1, #0x4b0                 	// #1200
 4026c98:	72bfff62 	movk	w2, #0xfffb, lsl #16
 4026c9c:	1b010881 	madd	w1, w4, w1, w2
 4026ca0:	5284e202 	mov	w2, #0x2710                	// #10000
 4026ca4:	1ac20821 	udiv	w1, w1, w2
 4026ca8:	7103f87f 	cmp	w3, #0xfe
 4026cac:	5400036c 	b.gt	4026d18 <start_svb+0x534>
 4026cb0:	52800002 	mov	w2, #0x0                   	// #0
 4026cb4:	1400001f 	b	4026d30 <start_svb+0x54c>
 4026cb8:	7101675f 	cmp	w26, #0x59
 4026cbc:	54fff78c 	b.gt	4026bac <start_svb+0x3c8>
 4026cc0:	7101175f 	cmp	w26, #0x45
 4026cc4:	54fffa0c 	b.gt	4026c04 <start_svb+0x420>
 4026cc8:	36fffdba 	tbz	w26, #31, 4026c7c <start_svb+0x498>
 4026ccc:	5297c1c2 	mov	w2, #0xbe0e                	// #48654
 4026cd0:	12803781 	mov	w1, #0xfffffe43            	// #-445
 4026cd4:	72a00022 	movk	w2, #0x1, lsl #16
 4026cd8:	1b010881 	madd	w1, w4, w1, w2
 4026cdc:	5284e202 	mov	w2, #0x2710                	// #10000
 4026ce0:	1ac20c21 	sdiv	w1, w1, w2
 4026ce4:	7103f87f 	cmp	w3, #0xfe
 4026ce8:	54fffe4d 	b.le	4026cb0 <start_svb+0x4cc>
 4026cec:	5292ce46 	mov	w6, #0x9672                	// #38514
 4026cf0:	12803e22 	mov	w2, #0xfffffe0e            	// #-498
 4026cf4:	72a00026 	movk	w6, #0x1, lsl #16
 4026cf8:	1b021862 	madd	w2, w3, w2, w6
 4026cfc:	5284e206 	mov	w6, #0x2710                	// #10000
 4026d00:	1ac60c42 	sdiv	w2, w2, w6
 4026d04:	1400000b 	b	4026d30 <start_svb+0x54c>
 4026d08:	7101b75f 	cmp	w26, #0x6d
 4026d0c:	540018ed 	b.le	4027028 <start_svb+0x844>
 4026d10:	52800000 	mov	w0, #0x0                   	// #0
 4026d14:	52800001 	mov	w1, #0x0                   	// #0
 4026d18:	52882bc6 	mov	w6, #0x415e                	// #16734
 4026d1c:	52808f82 	mov	w2, #0x47c                 	// #1148
 4026d20:	72bfff86 	movk	w6, #0xfffc, lsl #16
 4026d24:	1b021862 	madd	w2, w3, w2, w6
 4026d28:	5284e206 	mov	w6, #0x2710                	// #10000
 4026d2c:	1ac60842 	udiv	w2, w2, w6
 4026d30:	0b050000 	add	w0, w0, w5
 4026d34:	0b020062 	add	w2, w3, w2
 4026d38:	0b010081 	add	w1, w4, w1
 4026d3c:	52800024 	mov	w4, #0x1                   	// #1
 4026d40:	12002405 	and	w5, w0, #0x3ff
 4026d44:	53162423 	ubfiz	w3, w1, #10, #10
 4026d48:	2a050063 	orr	w3, w3, w5
 4026d4c:	530c2445 	ubfiz	w5, w2, #20, #10
 4026d50:	2a050063 	orr	w3, w3, w5
 4026d54:	2a047863 	orr	w3, w3, w4, lsl #30
 4026d58:	d2806804 	mov	x4, #0x340                 	// #832
 4026d5c:	f2a22044 	movk	x4, #0x1102, lsl #16
 4026d60:	b9000083 	str	w3, [x4]
 4026d64:	d2802403 	mov	x3, #0x120                 	// #288
 4026d68:	f2a203c3 	movk	x3, #0x101e, lsl #16
 4026d6c:	b90013ff 	str	wzr, [sp, #16]
 4026d70:	b9400063 	ldr	w3, [x3]
 4026d74:	b90013e3 	str	w3, [sp, #16]
 4026d78:	d2802683 	mov	x3, #0x134                 	// #308
 4026d7c:	f2a22043 	movk	x3, #0x1102, lsl #16
 4026d80:	b9400063 	ldr	w3, [x3]
 4026d84:	b9000be3 	str	w3, [sp, #8]
 4026d88:	b9400be3 	ldr	w3, [sp, #8]
 4026d8c:	12000463 	and	w3, w3, #0x3
 4026d90:	7100047f 	cmp	w3, #0x1
 4026d94:	b9400be3 	ldr	w3, [sp, #8]
 4026d98:	54000a61 	b.ne	4026ee4 <start_svb+0x700>  // b.any
 4026d9c:	d3421463 	ubfx	x3, x3, #2, #4
 4026da0:	7100047f 	cmp	w3, #0x1
 4026da4:	540002a1 	b.ne	4026df8 <start_svb+0x614>  // b.any
 4026da8:	b94013e4 	ldr	w4, [sp, #16]
 4026dac:	52802023 	mov	w3, #0x101                 	// #257
 4026db0:	72a00443 	movk	w3, #0x22, lsl #16
 4026db4:	6b03009f 	cmp	w4, w3
 4026db8:	54000201 	b.ne	4026df8 <start_svb+0x614>  // b.any
 4026dbc:	b0000043 	adrp	x3, 402f000 <malloc_sizes_ro+0x190>
 4026dc0:	91064063 	add	x3, x3, #0x190
 4026dc4:	97fffdfc 	bl	40265b4 <set_volt>
 4026dc8:	14000027 	b	4026e64 <start_svb+0x680>
 4026dcc:	7101175f 	cmp	w26, #0x45
 4026dd0:	5400008d 	b.le	4026de0 <start_svb+0x5fc>
 4026dd4:	52800001 	mov	w1, #0x0                   	// #0
 4026dd8:	52800000 	mov	w0, #0x0                   	// #0
 4026ddc:	17ffff92 	b	4026c24 <start_svb+0x440>
 4026de0:	36fff4da 	tbz	w26, #31, 4026c78 <start_svb+0x494>
 4026de4:	52800001 	mov	w1, #0x0                   	// #0
 4026de8:	52800000 	mov	w0, #0x0                   	// #0
 4026dec:	17ffffc0 	b	4026cec <start_svb+0x508>
 4026df0:	52800004 	mov	w4, #0x0                   	// #0
 4026df4:	17ffffd3 	b	4026d40 <start_svb+0x55c>
 4026df8:	b9400be3 	ldr	w3, [sp, #8]
 4026dfc:	d3421463 	ubfx	x3, x3, #2, #4
 4026e00:	7100087f 	cmp	w3, #0x2
 4026e04:	54000141 	b.ne	4026e2c <start_svb+0x648>  // b.any
 4026e08:	b94013e4 	ldr	w4, [sp, #16]
 4026e0c:	52802023 	mov	w3, #0x101                 	// #257
 4026e10:	72a00443 	movk	w3, #0x22, lsl #16
 4026e14:	6b03009f 	cmp	w4, w3
 4026e18:	540000a1 	b.ne	4026e2c <start_svb+0x648>  // b.any
 4026e1c:	b0000043 	adrp	x3, 402f000 <malloc_sizes_ro+0x190>
 4026e20:	91064063 	add	x3, x3, #0x190
 4026e24:	9100f063 	add	x3, x3, #0x3c
 4026e28:	17ffffe7 	b	4026dc4 <start_svb+0x5e0>
 4026e2c:	b9400be3 	ldr	w3, [sp, #8]
 4026e30:	d3421463 	ubfx	x3, x3, #2, #4
 4026e34:	71000c7f 	cmp	w3, #0x3
 4026e38:	54000141 	b.ne	4026e60 <start_svb+0x67c>  // b.any
 4026e3c:	b94013e4 	ldr	w4, [sp, #16]
 4026e40:	52802043 	mov	w3, #0x102                 	// #258
 4026e44:	72a00443 	movk	w3, #0x22, lsl #16
 4026e48:	6b03009f 	cmp	w4, w3
 4026e4c:	540000a1 	b.ne	4026e60 <start_svb+0x67c>  // b.any
 4026e50:	b0000043 	adrp	x3, 402f000 <malloc_sizes_ro+0x190>
 4026e54:	91064063 	add	x3, x3, #0x190
 4026e58:	9101e063 	add	x3, x3, #0x78
 4026e5c:	17ffffda 	b	4026dc4 <start_svb+0x5e0>
 4026e60:	97fffda4 	bl	40264f0 <svb_error>
 4026e64:	d2806a80 	mov	x0, #0x354                 	// #852
 4026e68:	f2a22040 	movk	x0, #0x1102, lsl #16
 4026e6c:	b9400002 	ldr	w2, [x0]
 4026e70:	37f808da 	tbnz	w26, #31, 4026f88 <start_svb+0x7a4>
 4026e74:	12001f41 	and	w1, w26, #0xff
 4026e78:	52800000 	mov	w0, #0x0                   	// #0
 4026e7c:	33001c20 	bfxil	w0, w1, #0, #8
 4026e80:	33101c22 	bfi	w2, w1, #16, #8
 4026e84:	d2806a81 	mov	x1, #0x354                 	// #852
 4026e88:	f2a22041 	movk	x1, #0x1102, lsl #16
 4026e8c:	b9000022 	str	w2, [x1]
 4026e90:	3100535f 	cmn	w26, #0x14
 4026e94:	540008ea 	b.ge	4026fb0 <start_svb+0x7cc>  // b.tcont
 4026e98:	12106c00 	and	w0, w0, #0xffff0fff
 4026e9c:	36f80076 	tbz	w22, #31, 4026ea8 <start_svb+0x6c4>
 4026ea0:	32090000 	orr	w0, w0, #0x800000
 4026ea4:	4b180296 	sub	w22, w20, w24
 4026ea8:	33101ac0 	bfi	w0, w22, #16, #7
 4026eac:	36f80077 	tbz	w23, #31, 4026eb8 <start_svb+0x6d4>
 4026eb0:	32010000 	orr	w0, w0, #0x80000000
 4026eb4:	4b1902b7 	sub	w23, w21, w25
 4026eb8:	d2806a01 	mov	x1, #0x350                 	// #848
 4026ebc:	33081ae0 	bfi	w0, w23, #24, #7
 4026ec0:	f2a22041 	movk	x1, #0x1102, lsl #16
 4026ec4:	b9000020 	str	w0, [x1]
 4026ec8:	f0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4026ecc:	f94027e1 	ldr	x1, [sp, #72]
 4026ed0:	f9455002 	ldr	x2, [x0, #2720]
 4026ed4:	eb020021 	subs	x1, x1, x2
 4026ed8:	d2800002 	mov	x2, #0x0                   	// #0
 4026edc:	540008e0 	b.eq	4026ff8 <start_svb+0x814>  // b.none
 4026ee0:	97fff4a3 	bl	402416c <__stack_chk_fail>
 4026ee4:	f240047f 	tst	x3, #0x3
 4026ee8:	54fffbc1 	b.ne	4026e60 <start_svb+0x67c>  // b.any
 4026eec:	b9400be3 	ldr	w3, [sp, #8]
 4026ef0:	d3421463 	ubfx	x3, x3, #2, #4
 4026ef4:	7100047f 	cmp	w3, #0x1
 4026ef8:	54000141 	b.ne	4026f20 <start_svb+0x73c>  // b.any
 4026efc:	b94013e4 	ldr	w4, [sp, #16]
 4026f00:	52802023 	mov	w3, #0x101                 	// #257
 4026f04:	72a00443 	movk	w3, #0x22, lsl #16
 4026f08:	6b03009f 	cmp	w4, w3
 4026f0c:	540000a1 	b.ne	4026f20 <start_svb+0x73c>  // b.any
 4026f10:	b0000043 	adrp	x3, 402f000 <malloc_sizes_ro+0x190>
 4026f14:	91064063 	add	x3, x3, #0x190
 4026f18:	9102d063 	add	x3, x3, #0xb4
 4026f1c:	17ffffaa 	b	4026dc4 <start_svb+0x5e0>
 4026f20:	b9400be3 	ldr	w3, [sp, #8]
 4026f24:	d3421463 	ubfx	x3, x3, #2, #4
 4026f28:	7100087f 	cmp	w3, #0x2
 4026f2c:	54000141 	b.ne	4026f54 <start_svb+0x770>  // b.any
 4026f30:	b94013e4 	ldr	w4, [sp, #16]
 4026f34:	52802023 	mov	w3, #0x101                 	// #257
 4026f38:	72a00443 	movk	w3, #0x22, lsl #16
 4026f3c:	6b03009f 	cmp	w4, w3
 4026f40:	540000a1 	b.ne	4026f54 <start_svb+0x770>  // b.any
 4026f44:	b0000043 	adrp	x3, 402f000 <malloc_sizes_ro+0x190>
 4026f48:	91064063 	add	x3, x3, #0x190
 4026f4c:	9103c063 	add	x3, x3, #0xf0
 4026f50:	17ffff9d 	b	4026dc4 <start_svb+0x5e0>
 4026f54:	b9400be3 	ldr	w3, [sp, #8]
 4026f58:	d3421463 	ubfx	x3, x3, #2, #4
 4026f5c:	71000c7f 	cmp	w3, #0x3
 4026f60:	54fff801 	b.ne	4026e60 <start_svb+0x67c>  // b.any
 4026f64:	b94013e4 	ldr	w4, [sp, #16]
 4026f68:	52802043 	mov	w3, #0x102                 	// #258
 4026f6c:	72a00443 	movk	w3, #0x22, lsl #16
 4026f70:	6b03009f 	cmp	w4, w3
 4026f74:	54fff761 	b.ne	4026e60 <start_svb+0x67c>  // b.any
 4026f78:	b0000043 	adrp	x3, 402f000 <malloc_sizes_ro+0x190>
 4026f7c:	91064063 	add	x3, x3, #0x190
 4026f80:	9104b063 	add	x3, x3, #0x12c
 4026f84:	17ffff90 	b	4026dc4 <start_svb+0x5e0>
 4026f88:	52800021 	mov	w1, #0x1                   	// #1
 4026f8c:	52800000 	mov	w0, #0x0                   	// #0
 4026f90:	33180c20 	bfi	w0, w1, #8, #4
 4026f94:	52800501 	mov	w1, #0x28                  	// #40
 4026f98:	4b130033 	sub	w19, w1, w19
 4026f9c:	32080042 	orr	w2, w2, #0x1000000
 4026fa0:	12001e73 	and	w19, w19, #0xff
 4026fa4:	33001e60 	bfxil	w0, w19, #0, #8
 4026fa8:	33101e62 	bfi	w2, w19, #16, #8
 4026fac:	17ffffb6 	b	4026e84 <start_svb+0x6a0>
 4026fb0:	36f8009a 	tbz	w26, #31, 4026fc0 <start_svb+0x7dc>
 4026fb4:	52800021 	mov	w1, #0x1                   	// #1
 4026fb8:	33140c20 	bfi	w0, w1, #12, #4
 4026fbc:	17ffffb8 	b	4026e9c <start_svb+0x6b8>
 4026fc0:	71008b5f 	cmp	w26, #0x22
 4026fc4:	5400006c 	b.gt	4026fd0 <start_svb+0x7ec>
 4026fc8:	52800041 	mov	w1, #0x2                   	// #2
 4026fcc:	17fffffb 	b	4026fb8 <start_svb+0x7d4>
 4026fd0:	71012b5f 	cmp	w26, #0x4a
 4026fd4:	5400006c 	b.gt	4026fe0 <start_svb+0x7fc>
 4026fd8:	52800061 	mov	w1, #0x3                   	// #3
 4026fdc:	17fffff7 	b	4026fb8 <start_svb+0x7d4>
 4026fe0:	7101cb5f 	cmp	w26, #0x72
 4026fe4:	5400006c 	b.gt	4026ff0 <start_svb+0x80c>
 4026fe8:	52800081 	mov	w1, #0x4                   	// #4
 4026fec:	17fffff3 	b	4026fb8 <start_svb+0x7d4>
 4026ff0:	528000a1 	mov	w1, #0x5                   	// #5
 4026ff4:	17fffff1 	b	4026fb8 <start_svb+0x7d4>
 4026ff8:	a94553f3 	ldp	x19, x20, [sp, #80]
 4026ffc:	d28000c0 	mov	x0, #0x6                   	// #6
 4027000:	a9465bf5 	ldp	x21, x22, [sp, #96]
 4027004:	a94763f7 	ldp	x23, x24, [sp, #112]
 4027008:	a9486bf9 	ldp	x25, x26, [sp, #128]
 402700c:	f9404bfe 	ldr	x30, [sp, #144]
 4027010:	910283ff 	add	sp, sp, #0xa0
 4027014:	14000350 	b	4027d54 <mdelay>
 4027018:	7104209f 	cmp	w4, #0x108
 402701c:	54ffe2ec 	b.gt	4026c78 <start_svb+0x494>
 4027020:	7103f87f 	cmp	w3, #0xfe
 4027024:	54ffe2ad 	b.le	4026c78 <start_svb+0x494>
 4027028:	7101675f 	cmp	w26, #0x59
 402702c:	54ffed0d 	b.le	4026dcc <start_svb+0x5e8>
 4027030:	52800001 	mov	w1, #0x0                   	// #0
 4027034:	52800000 	mov	w0, #0x0                   	// #0
 4027038:	17fffee5 	b	4026bcc <start_svb+0x3e8>
 402703c:	7103f87f 	cmp	w3, #0xfe
 4027040:	54ffe1ed 	b.le	4026c7c <start_svb+0x498>
 4027044:	52800001 	mov	w1, #0x0                   	// #0
 4027048:	17fffee1 	b	4026bcc <start_svb+0x3e8>
 402704c:	7103f87f 	cmp	w3, #0xfe
 4027050:	54ffe16d 	b.le	4026c7c <start_svb+0x498>
 4027054:	52800001 	mov	w1, #0x0                   	// #0
 4027058:	17fffef3 	b	4026c24 <start_svb+0x440>

000000000402705c <end_svb>:
 402705c:	f81e0ffe 	str	x30, [sp, #-32]!
 4027060:	d2806881 	mov	x1, #0x344                 	// #836
 4027064:	f2a22041 	movk	x1, #0x1102, lsl #16
 4027068:	52807442 	mov	w2, #0x3a2                 	// #930
 402706c:	b9001bff 	str	wzr, [sp, #24]
 4027070:	b9001fff 	str	wzr, [sp, #28]
 4027074:	b9400020 	ldr	w0, [x1]
 4027078:	12003c00 	and	w0, w0, #0xffff
 402707c:	b9001be0 	str	w0, [sp, #24]
 4027080:	b9401be0 	ldr	w0, [sp, #24]
 4027084:	710e881f 	cmp	w0, #0x3a2
 4027088:	1a822000 	csel	w0, w0, w2, cs  // cs = hs, nlast
 402708c:	b9001be0 	str	w0, [sp, #24]
 4027090:	b9400020 	ldr	w0, [x1]
 4027094:	12103c00 	and	w0, w0, #0xffff0000
 4027098:	b9001fe0 	str	w0, [sp, #28]
 402709c:	b9401fe0 	ldr	w0, [sp, #28]
 40270a0:	b9401be2 	ldr	w2, [sp, #24]
 40270a4:	2a020000 	orr	w0, w0, w2
 40270a8:	b9001fe0 	str	w0, [sp, #28]
 40270ac:	b9401fe0 	ldr	w0, [sp, #28]
 40270b0:	b9000020 	str	w0, [x1]
 40270b4:	52808361 	mov	w1, #0x41b                 	// #1051
 40270b8:	b9401be0 	ldr	w0, [sp, #24]
 40270bc:	97fffd2a 	bl	4026564 <calc_volt_regval.constprop.0>
 40270c0:	b9001be0 	str	w0, [sp, #24]
 40270c4:	52920001 	mov	w1, #0x9000                	// #36864
 40270c8:	b9401be0 	ldr	w0, [sp, #24]
 40270cc:	72a22041 	movk	w1, #0x1102, lsl #16
 40270d0:	97fffd02 	bl	40264d8 <svb_pwm_cfg>
 40270d4:	f84207fe 	ldr	x30, [sp], #32
 40270d8:	d28000c0 	mov	x0, #0x6                   	// #6
 40270dc:	1400031e 	b	4027d54 <mdelay>

00000000040270e0 <reg_read>:
 40270e0:	b9400002 	ldr	w2, [x0]
 40270e4:	b9400c04 	ldr	w4, [x0, #12]
 40270e8:	b9400042 	ldr	w2, [x2]
 40270ec:	d3535c85 	ubfx	x5, x4, #19, #5
 40270f0:	110004a6 	add	w6, w5, #0x1
 40270f4:	71007cbf 	cmp	w5, #0x1f
 40270f8:	540000e0 	b.eq	4027114 <reg_read+0x34>  // b.none
 40270fc:	531b7c84 	lsr	w4, w4, #27
 4027100:	52800023 	mov	w3, #0x1                   	// #1
 4027104:	1ac62063 	lsl	w3, w3, w6
 4027108:	51000463 	sub	w3, w3, #0x1
 402710c:	1ac42442 	lsr	w2, w2, w4
 4027110:	0a020062 	and	w2, w3, w2
 4027114:	b9400400 	ldr	w0, [x0, #4]
 4027118:	6b02001f 	cmp	w0, w2
 402711c:	1a9f07e0 	cset	w0, ne  // ne = any
 4027120:	b9000020 	str	w0, [x1]
 4027124:	d65f03c0 	ret

0000000004027128 <reg_write>:
 4027128:	29400801 	ldp	w1, w2, [x0]
 402712c:	29410c04 	ldp	w4, w3, [x0, #8]
 4027130:	b9400026 	ldr	w6, [x1]
 4027134:	d3431c65 	ubfx	x5, x3, #3, #5
 4027138:	110004a7 	add	w7, w5, #0x1
 402713c:	71007cbf 	cmp	w5, #0x1f
 4027140:	54000120 	b.eq	4027164 <reg_write+0x3c>  // b.none
 4027144:	52800021 	mov	w1, #0x1                   	// #1
 4027148:	d34b3c63 	ubfx	x3, x3, #11, #5
 402714c:	1ac72021 	lsl	w1, w1, w7
 4027150:	51000421 	sub	w1, w1, #0x1
 4027154:	1ac32042 	lsl	w2, w2, w3
 4027158:	1ac32021 	lsl	w1, w1, w3
 402715c:	0a2100c1 	bic	w1, w6, w1
 4027160:	2a010042 	orr	w2, w2, w1
 4027164:	b9400000 	ldr	w0, [x0]
 4027168:	b9000002 	str	w2, [x0]
 402716c:	d503201f 	nop
 4027170:	2a0403e0 	mov	w0, w4
 4027174:	51000484 	sub	w4, w4, #0x1
 4027178:	35ffffa0 	cbnz	w0, 402716c <reg_write+0x44>
 402717c:	d65f03c0 	ret

0000000004027180 <init_registers>:
 4027180:	d10083ff 	sub	sp, sp, #0x20
 4027184:	2a0103eb 	mov	w11, w1
 4027188:	2a0003e9 	mov	w9, w0
 402718c:	d0ffffc1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4027190:	5280000a 	mov	w10, #0x0                   	// #0
 4027194:	f9000bfe 	str	x30, [sp, #16]
 4027198:	f9455022 	ldr	x2, [x1, #2720]
 402719c:	f90007e2 	str	x2, [sp, #8]
 40271a0:	d2800002 	mov	x2, #0x0                   	// #0
 40271a4:	d37c7d40 	ubfiz	x0, x10, #4, #32
 40271a8:	8b000128 	add	x8, x9, x0
 40271ac:	b8606920 	ldr	w0, [x9, x0]
 40271b0:	29410907 	ldp	w7, w2, [x8, #8]
 40271b4:	350001a0 	cbnz	w0, 40271e8 <init_registers+0x68>
 40271b8:	b9400500 	ldr	w0, [x8, #4]
 40271bc:	2a070043 	orr	w3, w2, w7
 40271c0:	2a000063 	orr	w3, w3, w0
 40271c4:	35000123 	cbnz	w3, 40271e8 <init_registers+0x68>
 40271c8:	d503201f 	nop
 40271cc:	d0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 40271d0:	f94007e1 	ldr	x1, [sp, #8]
 40271d4:	f9455002 	ldr	x2, [x0, #2720]
 40271d8:	eb020021 	subs	x1, x1, x2
 40271dc:	d2800002 	mov	x2, #0x0                   	// #0
 40271e0:	54000580 	b.eq	4027290 <init_registers+0x110>  // b.none
 40271e4:	97fff3e2 	bl	402416c <__stack_chk_fail>
 40271e8:	b90007ff 	str	wzr, [sp, #4]
 40271ec:	340002eb 	cbz	w11, 4027248 <init_registers+0xc8>
 40271f0:	36080082 	tbz	w2, #1, 4027200 <init_registers+0x80>
 40271f4:	aa0803e0 	mov	x0, x8
 40271f8:	97ffffcc 	bl	4027128 <reg_write>
 40271fc:	1400000c 	b	402722c <init_registers+0xac>
 4027200:	368801a2 	tbz	w2, #17, 4027234 <init_registers+0xb4>
 4027204:	910013e1 	add	x1, sp, #0x4
 4027208:	aa0803e0 	mov	x0, x8
 402720c:	97ffffb5 	bl	40270e0 <reg_read>
 4027210:	d503201f 	nop
 4027214:	b94007e0 	ldr	w0, [sp, #4]
 4027218:	35ffff60 	cbnz	w0, 4027204 <init_registers+0x84>
 402721c:	d503201f 	nop
 4027220:	2a0703e0 	mov	w0, w7
 4027224:	510004e7 	sub	w7, w7, #0x1
 4027228:	35ffffa0 	cbnz	w0, 402721c <init_registers+0x9c>
 402722c:	1100054a 	add	w10, w10, #0x1
 4027230:	17ffffdd 	b	40271a4 <init_registers+0x24>
 4027234:	d503201f 	nop
 4027238:	2a0703e0 	mov	w0, w7
 402723c:	510004e7 	sub	w7, w7, #0x1
 4027240:	35ffffa0 	cbnz	w0, 4027234 <init_registers+0xb4>
 4027244:	17fffffa 	b	402722c <init_registers+0xac>
 4027248:	3717fd62 	tbnz	w2, #2, 40271f4 <init_registers+0x74>
 402724c:	36900182 	tbz	w2, #18, 402727c <init_registers+0xfc>
 4027250:	910013e1 	add	x1, sp, #0x4
 4027254:	aa0803e0 	mov	x0, x8
 4027258:	97ffffa2 	bl	40270e0 <reg_read>
 402725c:	d503201f 	nop
 4027260:	b94007e0 	ldr	w0, [sp, #4]
 4027264:	35ffff60 	cbnz	w0, 4027250 <init_registers+0xd0>
 4027268:	d503201f 	nop
 402726c:	2a0703e0 	mov	w0, w7
 4027270:	510004e7 	sub	w7, w7, #0x1
 4027274:	35ffffa0 	cbnz	w0, 4027268 <init_registers+0xe8>
 4027278:	17ffffed 	b	402722c <init_registers+0xac>
 402727c:	d503201f 	nop
 4027280:	2a0703e0 	mov	w0, w7
 4027284:	510004e7 	sub	w7, w7, #0x1
 4027288:	35ffffa0 	cbnz	w0, 402727c <init_registers+0xfc>
 402728c:	17ffffe8 	b	402722c <init_registers+0xac>
 4027290:	f9400bfe 	ldr	x30, [sp, #16]
 4027294:	910083ff 	add	sp, sp, #0x20
 4027298:	d65f03c0 	ret

000000000402729c <bsp_ddrtrn_retrain_enable>:
 402729c:	1400129e 	b	402bd14 <ddrtrn_retrain_enable>

00000000040272a0 <bsp_ddrtrn_training_item>:
 40272a0:	d10283ff 	sub	sp, sp, #0xa0
 40272a4:	a90853f3 	stp	x19, x20, [sp, #128]
 40272a8:	2a0003f4 	mov	w20, w0
 40272ac:	2a0103f3 	mov	w19, w1
 40272b0:	a9097bf5 	stp	x21, x30, [sp, #144]
 40272b4:	d0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 40272b8:	2a0203f5 	mov	w21, w2
 40272bc:	f9455001 	ldr	x1, [x0, #2720]
 40272c0:	f9003fe1 	str	x1, [sp, #120]
 40272c4:	d2800001 	mov	x1, #0x0                   	// #0
 40272c8:	910003e0 	mov	x0, sp
 40272cc:	9100e3e1 	add	x1, sp, #0x38
 40272d0:	940004ce 	bl	4028608 <ddrtrn_hal_set_cfg_addr>
 40272d4:	2a1503e1 	mov	w1, w21
 40272d8:	2a1403e0 	mov	w0, w20
 40272dc:	9400055f 	bl	4028858 <ddrtrn_hw_training_init>
 40272e0:	3100041f 	cmn	w0, #0x1
 40272e4:	54000121 	b.ne	4027308 <bsp_ddrtrn_training_item+0x68>  // b.any
 40272e8:	12800000 	mov	w0, #0xffffffff            	// #-1
 40272ec:	d0ffffc1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 40272f0:	f9403fe2 	ldr	x2, [sp, #120]
 40272f4:	f9455023 	ldr	x3, [x1, #2720]
 40272f8:	eb030042 	subs	x2, x2, x3
 40272fc:	d2800003 	mov	x3, #0x0                   	// #0
 4027300:	54000140 	b.eq	4027328 <bsp_ddrtrn_training_item+0x88>  // b.none
 4027304:	97fff39a 	bl	402416c <__stack_chk_fail>
 4027308:	2a1303e0 	mov	w0, w19
 402730c:	940004ac 	bl	40285bc <ddrtrn_sw_training_if>
 4027310:	3100041f 	cmn	w0, #0x1
 4027314:	54fffea0 	b.eq	40272e8 <bsp_ddrtrn_training_item+0x48>  // b.none
 4027318:	9400146c 	bl	402c4c8 <bsp_ddrtrn_suspend>
 402731c:	3100041f 	cmn	w0, #0x1
 4027320:	5a9f13e0 	csetm	w0, eq  // eq = none
 4027324:	17fffff2 	b	40272ec <bsp_ddrtrn_training_item+0x4c>
 4027328:	a94853f3 	ldp	x19, x20, [sp, #128]
 402732c:	a9497bf5 	ldp	x21, x30, [sp, #144]
 4027330:	910283ff 	add	sp, sp, #0xa0
 4027334:	d65f03c0 	ret

0000000004027338 <bsp_ddrtrn_capat_adapt>:
 4027338:	d10243ff 	sub	sp, sp, #0x90
 402733c:	d0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4027340:	f90043fe 	str	x30, [sp, #128]
 4027344:	f9455001 	ldr	x1, [x0, #2720]
 4027348:	f9003fe1 	str	x1, [sp, #120]
 402734c:	d2800001 	mov	x1, #0x0                   	// #0
 4027350:	910003e0 	mov	x0, sp
 4027354:	9100e3e1 	add	x1, sp, #0x38
 4027358:	940004ac 	bl	4028608 <ddrtrn_hal_set_cfg_addr>
 402735c:	940005e2 	bl	4028ae4 <ddrtrn_capat_adapt_func>
 4027360:	d0ffffc1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4027364:	f9403fe2 	ldr	x2, [sp, #120]
 4027368:	f9455023 	ldr	x3, [x1, #2720]
 402736c:	eb030042 	subs	x2, x2, x3
 4027370:	d2800003 	mov	x3, #0x0                   	// #0
 4027374:	54000040 	b.eq	402737c <bsp_ddrtrn_capat_adapt+0x44>  // b.none
 4027378:	97fff37d 	bl	402416c <__stack_chk_fail>
 402737c:	f94043fe 	ldr	x30, [sp, #128]
 4027380:	910243ff 	add	sp, sp, #0x90
 4027384:	d65f03c0 	ret

0000000004027388 <bsp_ddrtrn_dmc_auto_power_down_cfg>:
 4027388:	1400125d 	b	402bcfc <ddrtrn_dmc_auto_power_down_cfg>

000000000402738c <send_board_type_to_uart>:
 402738c:	d10183ff 	sub	sp, sp, #0x60
 4027390:	d2800203 	mov	x3, #0x10                  	// #16
 4027394:	52800002 	mov	w2, #0x0                   	// #0
 4027398:	a90353f3 	stp	x19, x20, [sp, #48]
 402739c:	a9045bf5 	stp	x21, x22, [sp, #64]
 40273a0:	2a0003f5 	mov	w21, w0
 40273a4:	d0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 40273a8:	a9057bf7 	stp	x23, x30, [sp, #80]
 40273ac:	f9455001 	ldr	x1, [x0, #2720]
 40273b0:	f90017e1 	str	x1, [sp, #40]
 40273b4:	d2800001 	mov	x1, #0x0                   	// #0
 40273b8:	910023e0 	add	x0, sp, #0x8
 40273bc:	aa0303e1 	mov	x1, x3
 40273c0:	94000285 	bl	4027dd4 <memset_s>
 40273c4:	350005e0 	cbnz	w0, 4027480 <send_board_type_to_uart+0xf4>
 40273c8:	52800013 	mov	w19, #0x0                   	// #0
 40273cc:	52800156 	mov	w22, #0xa                   	// #10
 40273d0:	528001b7 	mov	w23, #0xd                   	// #13
 40273d4:	97fffbed 	bl	4026388 <timer_get_divider>
 40273d8:	1b007ed4 	mul	w20, w22, w0
 40273dc:	94000252 	bl	4027d24 <timer_start>
 40273e0:	52a22080 	mov	w0, #0x11040000            	// #285474816
 40273e4:	94000081 	bl	40275e8 <serial_tstc>
 40273e8:	34000460 	cbz	w0, 4027474 <send_board_type_to_uart+0xe8>
 40273ec:	91001fe1 	add	x1, sp, #0x7
 40273f0:	52a22080 	mov	w0, #0x11040000            	// #285474816
 40273f4:	94000083 	bl	4027600 <serial_getc>
 40273f8:	39401fe0 	ldrb	w0, [sp, #7]
 40273fc:	34000513 	cbz	w19, 402749c <send_board_type_to_uart+0x110>
 4027400:	7100067f 	cmp	w19, #0x1
 4027404:	54000540 	b.eq	40274ac <send_board_type_to_uart+0x120>  // b.none
 4027408:	71000a7f 	cmp	w19, #0x2
 402740c:	54000560 	b.eq	40274b8 <send_board_type_to_uart+0x12c>  // b.none
 4027410:	35000053 	cbnz	w19, 4027418 <send_board_type_to_uart+0x8c>
 4027414:	79001bf7 	strh	w23, [sp, #12]
 4027418:	79401be2 	ldrh	w2, [sp, #12]
 402741c:	51000841 	sub	w1, w2, #0x2
 4027420:	6b01027f 	cmp	w19, w1
 4027424:	54000548 	b.hi	40274cc <send_board_type_to_uart+0x140>  // b.pmore
 4027428:	794017e1 	ldrh	w1, [sp, #10]
 402742c:	97fffb53 	bl	4026178 <cal_crc_perbyte>
 4027430:	790017e0 	strh	w0, [sp, #10]
 4027434:	79401be0 	ldrh	w0, [sp, #12]
 4027438:	6b13001f 	cmp	w0, w19
 402743c:	54000621 	b.ne	4027500 <send_board_type_to_uart+0x174>  // b.any
 4027440:	d2800013 	mov	x19, #0x0                   	// #0
 4027444:	910063f4 	add	x20, sp, #0x18
 4027448:	aa1403e1 	mov	x1, x20
 402744c:	2a1503e0 	mov	w0, w21
 4027450:	52800162 	mov	w2, #0xb                   	// #11
 4027454:	97fff146 	bl	402396c <build_board_type_frame>
 4027458:	38736a81 	ldrb	w1, [x20, x19]
 402745c:	52a22080 	mov	w0, #0x11040000            	// #285474816
 4027460:	91000673 	add	x19, x19, #0x1
 4027464:	9400002f 	bl	4027520 <serial_putc>
 4027468:	f1002e7f 	cmp	x19, #0xb
 402746c:	54ffff61 	b.ne	4027458 <send_board_type_to_uart+0xcc>  // b.any
 4027470:	14000004 	b	4027480 <send_board_type_to_uart+0xf4>
 4027474:	94000230 	bl	4027d34 <timer_get_val>
 4027478:	eb14001f 	cmp	x0, x20
 402747c:	54fffb23 	b.cc	40273e0 <send_board_type_to_uart+0x54>  // b.lo, b.ul, b.last
 4027480:	d0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4027484:	f94017e1 	ldr	x1, [sp, #40]
 4027488:	f9455002 	ldr	x2, [x0, #2720]
 402748c:	eb020021 	subs	x1, x1, x2
 4027490:	d2800002 	mov	x2, #0x0                   	// #0
 4027494:	540003a0 	b.eq	4027508 <send_board_type_to_uart+0x17c>  // b.none
 4027498:	97fff335 	bl	402416c <__stack_chk_fail>
 402749c:	7103381f 	cmp	w0, #0xce
 40274a0:	54fffba0 	b.eq	4027414 <send_board_type_to_uart+0x88>  // b.none
 40274a4:	52800013 	mov	w19, #0x0                   	// #0
 40274a8:	17ffffcb 	b	40273d4 <send_board_type_to_uart+0x48>
 40274ac:	39003be0 	strb	w0, [sp, #14]
 40274b0:	34fffb40 	cbz	w0, 4027418 <send_board_type_to_uart+0x8c>
 40274b4:	17fffffc 	b	40274a4 <send_board_type_to_uart+0x118>
 40274b8:	39403be1 	ldrb	w1, [sp, #14]
 40274bc:	2a2103e1 	mvn	w1, w1
 40274c0:	6b21001f 	cmp	w0, w1, uxtb
 40274c4:	54fffaa0 	b.eq	4027418 <send_board_type_to_uart+0x8c>  // b.none
 40274c8:	17fffff7 	b	40274a4 <send_board_type_to_uart+0x118>
 40274cc:	4b130041 	sub	w1, w2, w19
 40274d0:	531d7021 	lsl	w1, w1, #3
 40274d4:	1ac12001 	lsl	w1, w0, w1
 40274d8:	794013e0 	ldrh	w0, [sp, #8]
 40274dc:	2a000021 	orr	w1, w1, w0
 40274e0:	12003c21 	and	w1, w1, #0xffff
 40274e4:	790013e1 	strh	w1, [sp, #8]
 40274e8:	6b02027f 	cmp	w19, w2
 40274ec:	540000a1 	b.ne	4027500 <send_board_type_to_uart+0x174>  // b.any
 40274f0:	794017e0 	ldrh	w0, [sp, #10]
 40274f4:	6b01001f 	cmp	w0, w1
 40274f8:	54fffa40 	b.eq	4027440 <send_board_type_to_uart+0xb4>  // b.none
 40274fc:	17ffffea 	b	40274a4 <send_board_type_to_uart+0x118>
 4027500:	11000673 	add	w19, w19, #0x1
 4027504:	17ffffb4 	b	40273d4 <send_board_type_to_uart+0x48>
 4027508:	528014a0 	mov	w0, #0xa5                  	// #165
 402750c:	a94353f3 	ldp	x19, x20, [sp, #48]
 4027510:	a9445bf5 	ldp	x21, x22, [sp, #64]
 4027514:	a9457bf7 	ldp	x23, x30, [sp, #80]
 4027518:	910183ff 	add	sp, sp, #0x60
 402751c:	d65f03c0 	ret

0000000004027520 <serial_putc>:
 4027520:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4027524:	13001c34 	sxtb	w20, w1
 4027528:	2a0003f3 	mov	w19, w0
 402752c:	f9000bfe 	str	x30, [sp, #16]
 4027530:	97fffbc5 	bl	4026444 <is_sec_dbg_lv_enable>
 4027534:	5292cb41 	mov	w1, #0x965a                	// #38490
 4027538:	72a794a1 	movk	w1, #0x3ca5, lsl #16
 402753c:	6b01001f 	cmp	w0, w1
 4027540:	54000201 	b.ne	4027580 <serial_putc+0x60>  // b.any
 4027544:	52a22080 	mov	w0, #0x11040000            	// #285474816
 4027548:	6b00027f 	cmp	w19, w0
 402754c:	540001a1 	b.ne	4027580 <serial_putc+0x60>  // b.any
 4027550:	71002a9f 	cmp	w20, #0xa
 4027554:	54000081 	b.ne	4027564 <serial_putc+0x44>  // b.any
 4027558:	2a1303e0 	mov	w0, w19
 402755c:	528001a1 	mov	w1, #0xd                   	// #13
 4027560:	940001d1 	bl	4027ca4 <pl011_putc>
 4027564:	52a22080 	mov	w0, #0x11040000            	// #285474816
 4027568:	2a1403e1 	mov	w1, w20
 402756c:	940001ce 	bl	4027ca4 <pl011_putc>
 4027570:	528014a0 	mov	w0, #0xa5                  	// #165
 4027574:	f9400bfe 	ldr	x30, [sp, #16]
 4027578:	a8c253f3 	ldp	x19, x20, [sp], #32
 402757c:	d65f03c0 	ret
 4027580:	52800b40 	mov	w0, #0x5a                  	// #90
 4027584:	17fffffc 	b	4027574 <serial_putc+0x54>

0000000004027588 <serial_puts>:
 4027588:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402758c:	aa0103f3 	mov	x19, x1
 4027590:	2a0003f4 	mov	w20, w0
 4027594:	f9000bfe 	str	x30, [sp, #16]
 4027598:	97fffbab 	bl	4026444 <is_sec_dbg_lv_enable>
 402759c:	5292cb41 	mov	w1, #0x965a                	// #38490
 40275a0:	72a794a1 	movk	w1, #0x3ca5, lsl #16
 40275a4:	6b01001f 	cmp	w0, w1
 40275a8:	540000a1 	b.ne	40275bc <serial_puts+0x34>  // b.any
 40275ac:	52a22080 	mov	w0, #0x11040000            	// #285474816
 40275b0:	6b00029f 	cmp	w20, w0
 40275b4:	54000041 	b.ne	40275bc <serial_puts+0x34>  // b.any
 40275b8:	b50000d3 	cbnz	x19, 40275d0 <serial_puts+0x48>
 40275bc:	52800b40 	mov	w0, #0x5a                  	// #90
 40275c0:	14000007 	b	40275dc <serial_puts+0x54>
 40275c4:	91000673 	add	x19, x19, #0x1
 40275c8:	52a22080 	mov	w0, #0x11040000            	// #285474816
 40275cc:	97ffffd5 	bl	4027520 <serial_putc>
 40275d0:	39c00261 	ldrsb	w1, [x19]
 40275d4:	35ffff81 	cbnz	w1, 40275c4 <serial_puts+0x3c>
 40275d8:	528014a0 	mov	w0, #0xa5                  	// #165
 40275dc:	f9400bfe 	ldr	x30, [sp, #16]
 40275e0:	a8c253f3 	ldp	x19, x20, [sp], #32
 40275e4:	d65f03c0 	ret

00000000040275e8 <serial_tstc>:
 40275e8:	52a22081 	mov	w1, #0x11040000            	// #285474816
 40275ec:	6b01001f 	cmp	w0, w1
 40275f0:	54000041 	b.ne	40275f8 <serial_tstc+0x10>  // b.any
 40275f4:	140001b4 	b	4027cc4 <pl011_tstc>
 40275f8:	12800000 	mov	w0, #0xffffffff            	// #-1
 40275fc:	d65f03c0 	ret

0000000004027600 <serial_getc>:
 4027600:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4027604:	aa0103f3 	mov	x19, x1
 4027608:	52a22081 	mov	w1, #0x11040000            	// #285474816
 402760c:	f9000bfe 	str	x30, [sp, #16]
 4027610:	6b01001f 	cmp	w0, w1
 4027614:	540000a0 	b.eq	4027628 <serial_getc+0x28>  // b.none
 4027618:	52800b40 	mov	w0, #0x5a                  	// #90
 402761c:	f9400bfe 	ldr	x30, [sp, #16]
 4027620:	a8c253f3 	ldp	x19, x20, [sp], #32
 4027624:	d65f03c0 	ret
 4027628:	b4ffff93 	cbz	x19, 4027618 <serial_getc+0x18>
 402762c:	2a0003f4 	mov	w20, w0
 4027630:	940001a5 	bl	4027cc4 <pl011_tstc>
 4027634:	34ffff20 	cbz	w0, 4027618 <serial_getc+0x18>
 4027638:	2a1403e0 	mov	w0, w20
 402763c:	9400019e 	bl	4027cb4 <pl011_getc>
 4027640:	39000260 	strb	w0, [x19]
 4027644:	528014a0 	mov	w0, #0xa5                  	// #165
 4027648:	17fffff5 	b	402761c <serial_getc+0x1c>

000000000402764c <serial_put_hex>:
 402764c:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4027650:	2a0003f4 	mov	w20, w0
 4027654:	f9000bfe 	str	x30, [sp, #16]
 4027658:	97fffb7b 	bl	4026444 <is_sec_dbg_lv_enable>
 402765c:	5292cb41 	mov	w1, #0x965a                	// #38490
 4027660:	72a794a1 	movk	w1, #0x3ca5, lsl #16
 4027664:	6b01001f 	cmp	w0, w1
 4027668:	540001a1 	b.ne	402769c <serial_put_hex+0x50>  // b.any
 402766c:	52800393 	mov	w19, #0x1c                  	// #28
 4027670:	1ad32682 	lsr	w2, w20, w19
 4027674:	12000c42 	and	w2, w2, #0xf
 4027678:	7100245f 	cmp	w2, #0x9
 402767c:	1100c040 	add	w0, w2, #0x30
 4027680:	1100dc41 	add	w1, w2, #0x37
 4027684:	51001273 	sub	w19, w19, #0x4
 4027688:	1a808021 	csel	w1, w1, w0, hi  // hi = pmore
 402768c:	52a22080 	mov	w0, #0x11040000            	// #285474816
 4027690:	97ffffa4 	bl	4027520 <serial_putc>
 4027694:	3100127f 	cmn	w19, #0x4
 4027698:	54fffec1 	b.ne	4027670 <serial_put_hex+0x24>  // b.any
 402769c:	f9400bfe 	ldr	x30, [sp, #16]
 40276a0:	a8c253f3 	ldp	x19, x20, [sp], #32
 40276a4:	d65f03c0 	ret

00000000040276a8 <serial_put_dec>:
 40276a8:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 40276ac:	2a0003f3 	mov	w19, w0
 40276b0:	a9015bf5 	stp	x21, x22, [sp, #16]
 40276b4:	f90013fe 	str	x30, [sp, #32]
 40276b8:	97fffb63 	bl	4026444 <is_sec_dbg_lv_enable>
 40276bc:	5292cb41 	mov	w1, #0x965a                	// #38490
 40276c0:	72a794a1 	movk	w1, #0x3ca5, lsl #16
 40276c4:	6b01001f 	cmp	w0, w1
 40276c8:	54000201 	b.ne	4027708 <serial_put_dec+0x60>  // b.any
 40276cc:	36f80273 	tbz	w19, #31, 4027718 <serial_put_dec+0x70>
 40276d0:	4b1303f5 	neg	w21, w19
 40276d4:	528005a1 	mov	w1, #0x2d                  	// #45
 40276d8:	52a22080 	mov	w0, #0x11040000            	// #285474816
 40276dc:	97ffff91 	bl	4027520 <serial_putc>
 40276e0:	52800034 	mov	w20, #0x1                   	// #1
 40276e4:	52800156 	mov	w22, #0xa                   	// #10
 40276e8:	1ad40aa1 	udiv	w1, w21, w20
 40276ec:	52a22080 	mov	w0, #0x11040000            	// #285474816
 40276f0:	1b14d421 	msub	w1, w1, w20, w21
 40276f4:	1b167e94 	mul	w20, w20, w22
 40276f8:	1100c021 	add	w1, w1, #0x30
 40276fc:	97ffff89 	bl	4027520 <serial_putc>
 4027700:	6b13029f 	cmp	w20, w19
 4027704:	54ffff23 	b.cc	40276e8 <serial_put_dec+0x40>  // b.lo, b.ul, b.last
 4027708:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402770c:	f94013fe 	ldr	x30, [sp, #32]
 4027710:	a8c353f3 	ldp	x19, x20, [sp], #48
 4027714:	d65f03c0 	ret
 4027718:	2a1303f5 	mov	w21, w19
 402771c:	17fffff1 	b	40276e0 <serial_put_dec+0x38>

0000000004027720 <debug_puts>:
 4027720:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4027724:	2a0103f4 	mov	w20, w1
 4027728:	aa0003f3 	mov	x19, x0
 402772c:	a9017bf5 	stp	x21, x30, [sp, #16]
 4027730:	97fffb45 	bl	4026444 <is_sec_dbg_lv_enable>
 4027734:	5292cb41 	mov	w1, #0x965a                	// #38490
 4027738:	72a794a1 	movk	w1, #0x3ca5, lsl #16
 402773c:	6b01001f 	cmp	w0, w1
 4027740:	540005a1 	b.ne	40277f4 <debug_puts+0xd4>  // b.any
 4027744:	f0000035 	adrp	x21, 402e000 <ddrtrn_hal_set_dq_type+0xc>
 4027748:	913ec2b5 	add	x21, x21, #0xfb0
 402774c:	39400261 	ldrb	w1, [x19]
 4027750:	35000121 	cbnz	w1, 4027774 <debug_puts+0x54>
 4027754:	528001a1 	mov	w1, #0xd                   	// #13
 4027758:	52800000 	mov	w0, #0x0                   	// #0
 402775c:	97ffff71 	bl	4027520 <serial_putc>
 4027760:	a9417bf5 	ldp	x21, x30, [sp, #16]
 4027764:	52800141 	mov	w1, #0xa                   	// #10
 4027768:	a8c253f3 	ldp	x19, x20, [sp], #32
 402776c:	52800000 	mov	w0, #0x0                   	// #0
 4027770:	17ffff6c 	b	4027520 <serial_putc>
 4027774:	7100943f 	cmp	w1, #0x25
 4027778:	540000a0 	b.eq	402778c <debug_puts+0x6c>  // b.none
 402777c:	52a22080 	mov	w0, #0x11040000            	// #285474816
 4027780:	97ffff68 	bl	4027520 <serial_putc>
 4027784:	91000673 	add	x19, x19, #0x1
 4027788:	17fffff1 	b	402774c <debug_puts+0x2c>
 402778c:	39400660 	ldrb	w0, [x19, #1]
 4027790:	71008c1f 	cmp	w0, #0x23
 4027794:	540001c0 	b.eq	40277cc <debug_puts+0xac>  // b.none
 4027798:	91000673 	add	x19, x19, #0x1
 402779c:	39400260 	ldrb	w0, [x19]
 40277a0:	7101901f 	cmp	w0, #0x64
 40277a4:	54000220 	b.eq	40277e8 <debug_puts+0xc8>  // b.none
 40277a8:	540001c8 	b.hi	40277e0 <debug_puts+0xc0>  // b.pmore
 40277ac:	7101101f 	cmp	w0, #0x44
 40277b0:	540001c0 	b.eq	40277e8 <debug_puts+0xc8>  // b.none
 40277b4:	7101601f 	cmp	w0, #0x58
 40277b8:	54fffe61 	b.ne	4027784 <debug_puts+0x64>  // b.any
 40277bc:	91000673 	add	x19, x19, #0x1
 40277c0:	2a1403e0 	mov	w0, w20
 40277c4:	97ffffa2 	bl	402764c <serial_put_hex>
 40277c8:	17ffffef 	b	4027784 <debug_puts+0x64>
 40277cc:	aa1503e1 	mov	x1, x21
 40277d0:	52a22080 	mov	w0, #0x11040000            	// #285474816
 40277d4:	91000a73 	add	x19, x19, #0x2
 40277d8:	97ffff6c 	bl	4027588 <serial_puts>
 40277dc:	17fffff0 	b	402779c <debug_puts+0x7c>
 40277e0:	7101e01f 	cmp	w0, #0x78
 40277e4:	17fffff5 	b	40277b8 <debug_puts+0x98>
 40277e8:	2a1403e0 	mov	w0, w20
 40277ec:	97ffffaf 	bl	40276a8 <serial_put_dec>
 40277f0:	17ffffe5 	b	4027784 <debug_puts+0x64>
 40277f4:	a9417bf5 	ldp	x21, x30, [sp, #16]
 40277f8:	a8c253f3 	ldp	x19, x20, [sp], #32
 40277fc:	d65f03c0 	ret

0000000004027800 <wait_uart_tx_busy_state>:
 4027800:	52909000 	mov	w0, #0x8480                	// #33920
 4027804:	d2800301 	mov	x1, #0x18                  	// #24
 4027808:	72a003c0 	movk	w0, #0x1e, lsl #16
 402780c:	f2a22081 	movk	x1, #0x1104, lsl #16
 4027810:	b9400022 	ldr	w2, [x1]
 4027814:	361800a2 	tbz	w2, #3, 4027828 <wait_uart_tx_busy_state+0x28>
 4027818:	71000400 	subs	w0, w0, #0x1
 402781c:	54ffffa1 	b.ne	4027810 <wait_uart_tx_busy_state+0x10>  // b.any
 4027820:	52800b40 	mov	w0, #0x5a                  	// #90
 4027824:	d65f03c0 	ret
 4027828:	528014a0 	mov	w0, #0xa5                  	// #165
 402782c:	17fffffe 	b	4027824 <wait_uart_tx_busy_state+0x24>

0000000004027830 <tzasc_sec_config_read_back>:
 4027830:	12001c00 	and	w0, w0, #0xff
 4027834:	71003c1f 	cmp	w0, #0xf
 4027838:	54000328 	b.hi	402789c <tzasc_sec_config_read_back+0x6c>  // b.pmore
 402783c:	52882203 	mov	w3, #0x4110                	// #16656
 4027840:	72a02223 	movk	w3, #0x111, lsl #16
 4027844:	0b030000 	add	w0, w0, w3
 4027848:	531c6c03 	lsl	w3, w0, #4
 402784c:	d37c6400 	ubfiz	x0, x0, #4, #26
 4027850:	11040063 	add	w3, w3, #0x100
 4027854:	93407c63 	sxtw	x3, w3
 4027858:	b9400004 	ldr	w4, [x0]
 402785c:	b9400060 	ldr	w0, [x3]
 4027860:	92405483 	and	x3, x4, #0x3fffff
 4027864:	d3405c00 	ubfx	x0, x0, #0, #24
 4027868:	eb41307f 	cmp	x3, x1, lsr #12
 402786c:	54000081 	b.ne	402787c <tzasc_sec_config_read_back+0x4c>  // b.any
 4027870:	2a0003e0 	mov	w0, w0
 4027874:	eb42301f 	cmp	x0, x2, lsr #12
 4027878:	540000e0 	b.eq	4027894 <tzasc_sec_config_read_back+0x64>  // b.none
 402787c:	f81f0ffe 	str	x30, [sp, #-16]!
 4027880:	52800d80 	mov	w0, #0x6c                  	// #108
 4027884:	97fff1fd 	bl	4024078 <err_print>
 4027888:	52800b40 	mov	w0, #0x5a                  	// #90
 402788c:	f84107fe 	ldr	x30, [sp], #16
 4027890:	d65f03c0 	ret
 4027894:	528014a0 	mov	w0, #0xa5                  	// #165
 4027898:	d65f03c0 	ret
 402789c:	d2800000 	mov	x0, #0x0                   	// #0
 40278a0:	b9400001 	ldr	w1, [x0]
 40278a4:	b9400000 	ldr	w0, [x0]
 40278a8:	d4207d00 	brk	#0x3e8

00000000040278ac <tzasc_bypass_disable>:
 40278ac:	d2820080 	mov	x0, #0x1004                	// #4100
 40278b0:	f2a22280 	movk	x0, #0x1114, lsl #16
 40278b4:	b900001f 	str	wzr, [x0]
 40278b8:	b9400000 	ldr	w0, [x0]
 40278bc:	340000e0 	cbz	w0, 40278d8 <tzasc_bypass_disable+0x2c>
 40278c0:	f81f0ffe 	str	x30, [sp, #-16]!
 40278c4:	52800da0 	mov	w0, #0x6d                  	// #109
 40278c8:	97fff1ec 	bl	4024078 <err_print>
 40278cc:	52800b40 	mov	w0, #0x5a                  	// #90
 40278d0:	f84107fe 	ldr	x30, [sp], #16
 40278d4:	d65f03c0 	ret
 40278d8:	528014a0 	mov	w0, #0xa5                  	// #165
 40278dc:	d65f03c0 	ret

00000000040278e0 <tzasc_rgn_enable>:
 40278e0:	12001c00 	and	w0, w0, #0xff
 40278e4:	71003c1f 	cmp	w0, #0xf
 40278e8:	54000108 	b.hi	4027908 <tzasc_rgn_enable+0x28>  // b.pmore
 40278ec:	52882201 	mov	w1, #0x4110                	// #16656
 40278f0:	72a02221 	movk	w1, #0x111, lsl #16
 40278f4:	0b010000 	add	w0, w0, w1
 40278f8:	d37c6400 	ubfiz	x0, x0, #4, #26
 40278fc:	b9400001 	ldr	w1, [x0]
 4027900:	32010021 	orr	w1, w1, #0x80000000
 4027904:	b9000001 	str	w1, [x0]
 4027908:	d65f03c0 	ret

000000000402790c <tzasc_set_rgn_map>:
 402790c:	12001c00 	and	w0, w0, #0xff
 4027910:	71003c1f 	cmp	w0, #0xf
 4027914:	54000208 	b.hi	4027954 <tzasc_set_rgn_map+0x48>  // b.pmore
 4027918:	52882203 	mov	w3, #0x4110                	// #16656
 402791c:	d34c8421 	ubfx	x1, x1, #12, #22
 4027920:	72a02223 	movk	w3, #0x111, lsl #16
 4027924:	0b030000 	add	w0, w0, w3
 4027928:	d34c8c42 	ubfx	x2, x2, #12, #24
 402792c:	531c6c03 	lsl	w3, w0, #4
 4027930:	d37c6400 	ubfiz	x0, x0, #4, #26
 4027934:	11040063 	add	w3, w3, #0x100
 4027938:	b9400004 	ldr	w4, [x0]
 402793c:	93407c63 	sxtw	x3, w3
 4027940:	33005424 	bfxil	w4, w1, #0, #22
 4027944:	b9000004 	str	w4, [x0]
 4027948:	b9400060 	ldr	w0, [x3]
 402794c:	33005c40 	bfxil	w0, w2, #0, #24
 4027950:	b9000060 	str	w0, [x3]
 4027954:	d65f03c0 	ret

0000000004027958 <tzasc_set_rgn_attr>:
 4027958:	12001c00 	and	w0, w0, #0xff
 402795c:	71003c1f 	cmp	w0, #0xf
 4027960:	54000108 	b.hi	4027980 <tzasc_set_rgn_attr+0x28>  // b.pmore
 4027964:	52882002 	mov	w2, #0x4100                	// #16640
 4027968:	72a02222 	movk	w2, #0x111, lsl #16
 402796c:	0b020000 	add	w0, w0, w2
 4027970:	531c6c00 	lsl	w0, w0, #4
 4027974:	11041000 	add	w0, w0, #0x104
 4027978:	93407c00 	sxtw	x0, w0
 402797c:	b9000001 	str	w1, [x0]
 4027980:	d65f03c0 	ret

0000000004027984 <config_tzasc>:
 4027984:	aa0203e4 	mov	x4, x2
 4027988:	f81f0ffe 	str	x30, [sp, #-16]!
 402798c:	12001c05 	and	w5, w0, #0xff
 4027990:	2a0503e0 	mov	w0, w5
 4027994:	97fffff1 	bl	4027958 <tzasc_set_rgn_attr>
 4027998:	2a0503e0 	mov	w0, w5
 402799c:	aa0303e2 	mov	x2, x3
 40279a0:	aa0403e1 	mov	x1, x4
 40279a4:	97ffffda 	bl	402790c <tzasc_set_rgn_map>
 40279a8:	f84107fe 	ldr	x30, [sp], #16
 40279ac:	2a0503e0 	mov	w0, w5
 40279b0:	17ffffcc 	b	40278e0 <tzasc_rgn_enable>

00000000040279b4 <uapi_drv_cipher_init>:
 40279b4:	d28a0000 	mov	x0, #0x5000                	// #20480
 40279b8:	b9400000 	ldr	w0, [x0]
 40279bc:	aa0003f0 	mov	x16, x0
 40279c0:	d61f0200 	br	x16

00000000040279c4 <uapi_drv_cipher_deinit>:
 40279c4:	d28a0080 	mov	x0, #0x5004                	// #20484
 40279c8:	b9400000 	ldr	w0, [x0]
 40279cc:	aa0003f0 	mov	x16, x0
 40279d0:	d61f0200 	br	x16

00000000040279d4 <uapi_drv_cipher_create>:
 40279d4:	d28a0103 	mov	x3, #0x5008                	// #20488
 40279d8:	b9400063 	ldr	w3, [x3]
 40279dc:	aa0303f0 	mov	x16, x3
 40279e0:	d61f0200 	br	x16

00000000040279e4 <uapi_drv_cipher_destroy>:
 40279e4:	d28a0181 	mov	x1, #0x500c                	// #20492
 40279e8:	b9400021 	ldr	w1, [x1]
 40279ec:	aa0103f0 	mov	x16, x1
 40279f0:	d61f0200 	br	x16

00000000040279f4 <uapi_drv_cipher_dma_copy>:
 40279f4:	d28a0204 	mov	x4, #0x5010                	// #20496
 40279f8:	b9400084 	ldr	w4, [x4]
 40279fc:	aa0403f0 	mov	x16, x4
 4027a00:	d61f0200 	br	x16

0000000004027a04 <uapi_drv_cipher_set_config>:
 4027a04:	d28a0283 	mov	x3, #0x5014                	// #20500
 4027a08:	b9400063 	ldr	w3, [x3]
 4027a0c:	aa0303f0 	mov	x16, x3
 4027a10:	d61f0200 	br	x16

0000000004027a14 <uapi_drv_cipher_attach>:
 4027a14:	d28a0303 	mov	x3, #0x5018                	// #20504
 4027a18:	b9400063 	ldr	w3, [x3]
 4027a1c:	aa0303f0 	mov	x16, x3
 4027a20:	d61f0200 	br	x16

0000000004027a24 <uapi_drv_cipher_detach>:
 4027a24:	d28a0383 	mov	x3, #0x501c                	// #20508
 4027a28:	b9400063 	ldr	w3, [x3]
 4027a2c:	aa0303f0 	mov	x16, x3
 4027a30:	d61f0200 	br	x16

0000000004027a34 <uapi_drv_cipher_decrypt>:
 4027a34:	d28a0406 	mov	x6, #0x5020                	// #20512
 4027a38:	b94000c6 	ldr	w6, [x6]
 4027a3c:	aa0603f0 	mov	x16, x6
 4027a40:	d61f0200 	br	x16

0000000004027a44 <uapi_drv_cipher_hash_init>:
 4027a44:	d28a0483 	mov	x3, #0x5024                	// #20516
 4027a48:	b9400063 	ldr	w3, [x3]
 4027a4c:	aa0303f0 	mov	x16, x3
 4027a50:	d61f0200 	br	x16

0000000004027a54 <uapi_drv_cipher_hash_update>:
 4027a54:	d28a0505 	mov	x5, #0x5028                	// #20520
 4027a58:	b94000a5 	ldr	w5, [x5]
 4027a5c:	aa0503f0 	mov	x16, x5
 4027a60:	d61f0200 	br	x16

0000000004027a64 <uapi_drv_cipher_hash_final>:
 4027a64:	d28a0584 	mov	x4, #0x502c                	// #20524
 4027a68:	b9400084 	ldr	w4, [x4]
 4027a6c:	aa0403f0 	mov	x16, x4
 4027a70:	d61f0200 	br	x16

0000000004027a74 <uapi_drv_cipher_trng_get_random>:
 4027a74:	d28a0601 	mov	x1, #0x5030                	// #20528
 4027a78:	b9400021 	ldr	w1, [x1]
 4027a7c:	aa0103f0 	mov	x16, x1
 4027a80:	d61f0200 	br	x16

0000000004027a84 <uapi_drv_cipher_register_cipher_wait_func>:
 4027a84:	d28a0684 	mov	x4, #0x5034                	// #20532
 4027a88:	b9400084 	ldr	w4, [x4]
 4027a8c:	aa0403f0 	mov	x16, x4
 4027a90:	d61f0200 	br	x16

0000000004027a94 <uapi_drv_keyslot_init>:
 4027a94:	d28a0700 	mov	x0, #0x5038                	// #20536
 4027a98:	b9400000 	ldr	w0, [x0]
 4027a9c:	aa0003f0 	mov	x16, x0
 4027aa0:	d61f0200 	br	x16

0000000004027aa4 <uapi_drv_keyslot_deinit>:
 4027aa4:	d28a0780 	mov	x0, #0x503c                	// #20540
 4027aa8:	b9400000 	ldr	w0, [x0]
 4027aac:	aa0003f0 	mov	x16, x0
 4027ab0:	d61f0200 	br	x16

0000000004027ab4 <uapi_drv_keyslot_create>:
 4027ab4:	d28a0802 	mov	x2, #0x5040                	// #20544
 4027ab8:	b9400042 	ldr	w2, [x2]
 4027abc:	aa0203f0 	mov	x16, x2
 4027ac0:	d61f0200 	br	x16

0000000004027ac4 <uapi_drv_keyslot_destroy>:
 4027ac4:	d28a0881 	mov	x1, #0x5044                	// #20548
 4027ac8:	b9400021 	ldr	w1, [x1]
 4027acc:	aa0103f0 	mov	x16, x1
 4027ad0:	d61f0200 	br	x16

0000000004027ad4 <uapi_drv_klad_create>:
 4027ad4:	d28a0901 	mov	x1, #0x5048                	// #20552
 4027ad8:	b9400021 	ldr	w1, [x1]
 4027adc:	aa0103f0 	mov	x16, x1
 4027ae0:	d61f0200 	br	x16

0000000004027ae4 <uapi_drv_klad_destroy>:
 4027ae4:	d28a0981 	mov	x1, #0x504c                	// #20556
 4027ae8:	b9400021 	ldr	w1, [x1]
 4027aec:	aa0103f0 	mov	x16, x1
 4027af0:	d61f0200 	br	x16

0000000004027af4 <uapi_drv_klad_attach>:
 4027af4:	d28a0a04 	mov	x4, #0x5050                	// #20560
 4027af8:	b9400084 	ldr	w4, [x4]
 4027afc:	aa0403f0 	mov	x16, x4
 4027b00:	d61f0200 	br	x16

0000000004027b04 <uapi_drv_klad_detach>:
 4027b04:	d28a0a84 	mov	x4, #0x5054                	// #20564
 4027b08:	b9400084 	ldr	w4, [x4]
 4027b0c:	aa0403f0 	mov	x16, x4
 4027b10:	d61f0200 	br	x16

0000000004027b14 <uapi_drv_klad_set_attr>:
 4027b14:	d28a0b03 	mov	x3, #0x5058                	// #20568
 4027b18:	b9400063 	ldr	w3, [x3]
 4027b1c:	aa0303f0 	mov	x16, x3
 4027b20:	d61f0200 	br	x16

0000000004027b24 <uapi_drv_klad_set_session_key>:
 4027b24:	d28a0b83 	mov	x3, #0x505c                	// #20572
 4027b28:	b9400063 	ldr	w3, [x3]
 4027b2c:	aa0303f0 	mov	x16, x3
 4027b30:	d61f0200 	br	x16

0000000004027b34 <uapi_drv_klad_set_content_key>:
 4027b34:	d28a0c04 	mov	x4, #0x5060                	// #20576
 4027b38:	b9400084 	ldr	w4, [x4]
 4027b3c:	aa0403f0 	mov	x16, x4
 4027b40:	d61f0200 	br	x16

0000000004027b44 <uapi_drv_klad_set_clear_key>:
 4027b44:	d28a2183 	mov	x3, #0x510c                	// #20748
 4027b48:	b9400063 	ldr	w3, [x3]
 4027b4c:	aa0303f0 	mov	x16, x3
 4027b50:	d61f0200 	br	x16

0000000004027b54 <uapi_drv_klad_rootkey_disable>:
 4027b54:	d28a0c82 	mov	x2, #0x5064                	// #20580
 4027b58:	b9400042 	ldr	w2, [x2]
 4027b5c:	aa0203f0 	mov	x16, x2
 4027b60:	d61f0200 	br	x16

0000000004027b64 <uapi_drv_otp_read_word>:
 4027b64:	d28a0d03 	mov	x3, #0x5068                	// #20584
 4027b68:	b9400063 	ldr	w3, [x3]
 4027b6c:	aa0303f0 	mov	x16, x3
 4027b70:	d61f0200 	br	x16

0000000004027b74 <uapi_drv_otp_read_byte>:
 4027b74:	d28a0d83 	mov	x3, #0x506c                	// #20588
 4027b78:	b9400063 	ldr	w3, [x3]
 4027b7c:	aa0303f0 	mov	x16, x3
 4027b80:	d61f0200 	br	x16

0000000004027b84 <uapi_drv_otp_get_die_id>:
 4027b84:	d28a0e03 	mov	x3, #0x5070                	// #20592
 4027b88:	b9400063 	ldr	w3, [x3]
 4027b8c:	aa0303f0 	mov	x16, x3
 4027b90:	d61f0200 	br	x16

0000000004027b94 <uapi_drv_pke_init>:
 4027b94:	d28a0e80 	mov	x0, #0x5074                	// #20596
 4027b98:	b9400000 	ldr	w0, [x0]
 4027b9c:	aa0003f0 	mov	x16, x0
 4027ba0:	d61f0200 	br	x16

0000000004027ba4 <uapi_drv_pke_deinit>:
 4027ba4:	d28a0f00 	mov	x0, #0x5078                	// #20600
 4027ba8:	b9400000 	ldr	w0, [x0]
 4027bac:	aa0003f0 	mov	x16, x0
 4027bb0:	d61f0200 	br	x16

0000000004027bb4 <uapi_drv_pke_ecdsa_verify>:
 4027bb4:	d28a0f86 	mov	x6, #0x507c                	// #20604
 4027bb8:	b94000c6 	ldr	w6, [x6]
 4027bbc:	aa0603f0 	mov	x16, x6
 4027bc0:	d61f0200 	br	x16

0000000004027bc4 <uapi_drv_pke_sm2_dsa_hash>:
 4027bc4:	d28a1005 	mov	x5, #0x5080                	// #20608
 4027bc8:	b94000a5 	ldr	w5, [x5]
 4027bcc:	aa0503f0 	mov	x16, x5
 4027bd0:	d61f0200 	br	x16

0000000004027bd4 <uapi_drv_register_func>:
 4027bd4:	d28a1081 	mov	x1, #0x5084                	// #20612
 4027bd8:	b9400021 	ldr	w1, [x1]
 4027bdc:	aa0103f0 	mov	x16, x1
 4027be0:	d61f0200 	br	x16

0000000004027be4 <mmc_init>:
 4027be4:	d28a1100 	mov	x0, #0x5088                	// #20616
 4027be8:	b9400000 	ldr	w0, [x0]
 4027bec:	aa0003f0 	mov	x16, x0
 4027bf0:	d61f0200 	br	x16

0000000004027bf4 <mmc_read>:
 4027bf4:	d28a1184 	mov	x4, #0x508c                	// #20620
 4027bf8:	b9400084 	ldr	w4, [x4]
 4027bfc:	aa0403f0 	mov	x16, x4
 4027c00:	d61f0200 	br	x16

0000000004027c04 <is_bootmode>:
 4027c04:	d28a1200 	mov	x0, #0x5090                	// #20624
 4027c08:	b9400000 	ldr	w0, [x0]
 4027c0c:	aa0003f0 	mov	x16, x0
 4027c10:	d61f0200 	br	x16

0000000004027c14 <mmc_get_cur_mode>:
 4027c14:	d28a1280 	mov	x0, #0x5094                	// #20628
 4027c18:	b9400000 	ldr	w0, [x0]
 4027c1c:	aa0003f0 	mov	x16, x0
 4027c20:	d61f0200 	br	x16

0000000004027c24 <mmc_set_bootup_mode>:
 4027c24:	d28a1301 	mov	x1, #0x5098                	// #20632
 4027c28:	b9400021 	ldr	w1, [x1]
 4027c2c:	aa0103f0 	mov	x16, x1
 4027c30:	d61f0200 	br	x16

0000000004027c34 <copy_from_sdio>:
 4027c34:	d28a1382 	mov	x2, #0x509c                	// #20636
 4027c38:	b9400042 	ldr	w2, [x2]
 4027c3c:	aa0203f0 	mov	x16, x2
 4027c40:	d61f0200 	br	x16

0000000004027c44 <set_sdio_pos>:
 4027c44:	d28a1401 	mov	x1, #0x50a0                	// #20640
 4027c48:	b9400021 	ldr	w1, [x1]
 4027c4c:	aa0103f0 	mov	x16, x1
 4027c50:	d61f0200 	br	x16

0000000004027c54 <enable_sdio_dma>:
 4027c54:	d28a1480 	mov	x0, #0x50a4                	// #20644
 4027c58:	b9400000 	ldr	w0, [x0]
 4027c5c:	aa0003f0 	mov	x16, x0
 4027c60:	d61f0200 	br	x16

0000000004027c64 <self_usb_check>:
 4027c64:	d28a1500 	mov	x0, #0x50a8                	// #20648
 4027c68:	b9400000 	ldr	w0, [x0]
 4027c6c:	aa0003f0 	mov	x16, x0
 4027c70:	d61f0200 	br	x16

0000000004027c74 <copy_from_usb>:
 4027c74:	d28a1583 	mov	x3, #0x50ac                	// #20652
 4027c78:	b9400063 	ldr	w3, [x3]
 4027c7c:	aa0303f0 	mov	x16, x3
 4027c80:	d61f0200 	br	x16

0000000004027c84 <usb3_driver_init>:
 4027c84:	d28a1600 	mov	x0, #0x50b0                	// #20656
 4027c88:	b9400000 	ldr	w0, [x0]
 4027c8c:	aa0003f0 	mov	x16, x0
 4027c90:	d61f0200 	br	x16

0000000004027c94 <send_to_usb>:
 4027c94:	d28a1682 	mov	x2, #0x50b4                	// #20660
 4027c98:	b9400042 	ldr	w2, [x2]
 4027c9c:	aa0203f0 	mov	x16, x2
 4027ca0:	d61f0200 	br	x16

0000000004027ca4 <pl011_putc>:
 4027ca4:	d28a1702 	mov	x2, #0x50b8                	// #20664
 4027ca8:	b9400042 	ldr	w2, [x2]
 4027cac:	aa0203f0 	mov	x16, x2
 4027cb0:	d61f0200 	br	x16

0000000004027cb4 <pl011_getc>:
 4027cb4:	d28a1781 	mov	x1, #0x50bc                	// #20668
 4027cb8:	b9400021 	ldr	w1, [x1]
 4027cbc:	aa0103f0 	mov	x16, x1
 4027cc0:	d61f0200 	br	x16

0000000004027cc4 <pl011_tstc>:
 4027cc4:	d28a1801 	mov	x1, #0x50c0                	// #20672
 4027cc8:	b9400021 	ldr	w1, [x1]
 4027ccc:	aa0103f0 	mov	x16, x1
 4027cd0:	d61f0200 	br	x16

0000000004027cd4 <serial_init>:
 4027cd4:	d28a1882 	mov	x2, #0x50c4                	// #20676
 4027cd8:	b9400042 	ldr	w2, [x2]
 4027cdc:	aa0203f0 	mov	x16, x2
 4027ce0:	d61f0200 	br	x16

0000000004027ce4 <log_serial_puts>:
 4027ce4:	d28a1901 	mov	x1, #0x50c8                	// #20680
 4027ce8:	b9400021 	ldr	w1, [x1]
 4027cec:	aa0103f0 	mov	x16, x1
 4027cf0:	d61f0200 	br	x16

0000000004027cf4 <copy_from_uart>:
 4027cf4:	d28a1983 	mov	x3, #0x50cc                	// #20684
 4027cf8:	b9400063 	ldr	w3, [x3]
 4027cfc:	aa0303f0 	mov	x16, x3
 4027d00:	d61f0200 	br	x16

0000000004027d04 <timer_init>:
 4027d04:	d28a1a00 	mov	x0, #0x50d0                	// #20688
 4027d08:	b9400000 	ldr	w0, [x0]
 4027d0c:	aa0003f0 	mov	x16, x0
 4027d10:	d61f0200 	br	x16

0000000004027d14 <timer_deinit>:
 4027d14:	d28a1a80 	mov	x0, #0x50d4                	// #20692
 4027d18:	b9400000 	ldr	w0, [x0]
 4027d1c:	aa0003f0 	mov	x16, x0
 4027d20:	d61f0200 	br	x16

0000000004027d24 <timer_start>:
 4027d24:	d28a1b00 	mov	x0, #0x50d8                	// #20696
 4027d28:	b9400000 	ldr	w0, [x0]
 4027d2c:	aa0003f0 	mov	x16, x0
 4027d30:	d61f0200 	br	x16

0000000004027d34 <timer_get_val>:
 4027d34:	d28a1b80 	mov	x0, #0x50dc                	// #20700
 4027d38:	b9400000 	ldr	w0, [x0]
 4027d3c:	aa0003f0 	mov	x16, x0
 4027d40:	d61f0200 	br	x16

0000000004027d44 <udelay>:
 4027d44:	d28a1c01 	mov	x1, #0x50e0                	// #20704
 4027d48:	b9400021 	ldr	w1, [x1]
 4027d4c:	aa0103f0 	mov	x16, x1
 4027d50:	d61f0200 	br	x16

0000000004027d54 <mdelay>:
 4027d54:	d28a1c81 	mov	x1, #0x50e4                	// #20708
 4027d58:	b9400021 	ldr	w1, [x1]
 4027d5c:	aa0103f0 	mov	x16, x1
 4027d60:	d61f0200 	br	x16

0000000004027d64 <watchdog_enable>:
 4027d64:	d28a1d03 	mov	x3, #0x50e8                	// #20712
 4027d68:	b9400063 	ldr	w3, [x3]
 4027d6c:	aa0303f0 	mov	x16, x3
 4027d70:	d61f0200 	br	x16

0000000004027d74 <watchdog_disable>:
 4027d74:	d28a1d81 	mov	x1, #0x50ec                	// #20716
 4027d78:	b9400021 	ldr	w1, [x1]
 4027d7c:	aa0103f0 	mov	x16, x1
 4027d80:	d61f0200 	br	x16

0000000004027d84 <watchdog_feed>:
 4027d84:	d28a1e02 	mov	x2, #0x50f0                	// #20720
 4027d88:	b9400042 	ldr	w2, [x2]
 4027d8c:	aa0203f0 	mov	x16, x2
 4027d90:	d61f0200 	br	x16

0000000004027d94 <memcmp_ss>:
 4027d94:	d28a1e84 	mov	x4, #0x50f4                	// #20724
 4027d98:	b9400084 	ldr	w4, [x4]
 4027d9c:	aa0403f0 	mov	x16, x4
 4027da0:	d61f0200 	br	x16

0000000004027da4 <memcpy_ss>:
 4027da4:	d28a1f05 	mov	x5, #0x50f8                	// #20728
 4027da8:	b94000a5 	ldr	w5, [x5]
 4027dac:	aa0503f0 	mov	x16, x5
 4027db0:	d61f0200 	br	x16

0000000004027db4 <memset_ss>:
 4027db4:	d28a1f85 	mov	x5, #0x50fc                	// #20732
 4027db8:	b94000a5 	ldr	w5, [x5]
 4027dbc:	aa0503f0 	mov	x16, x5
 4027dc0:	d61f0200 	br	x16

0000000004027dc4 <memcpy_s>:
 4027dc4:	d28a2004 	mov	x4, #0x5100                	// #20736
 4027dc8:	b9400084 	ldr	w4, [x4]
 4027dcc:	aa0403f0 	mov	x16, x4
 4027dd0:	d61f0200 	br	x16

0000000004027dd4 <memset_s>:
 4027dd4:	d28a2084 	mov	x4, #0x5104                	// #20740
 4027dd8:	b9400084 	ldr	w4, [x4]
 4027ddc:	aa0403f0 	mov	x16, x4
 4027de0:	d61f0200 	br	x16

0000000004027de4 <save_cur_point_syscnt>:
 4027de4:	d28a2100 	mov	x0, #0x5108                	// #20744
 4027de8:	b9400000 	ldr	w0, [x0]
 4027dec:	aa0003f0 	mov	x16, x0
 4027df0:	d61f0200 	br	x16

0000000004027df4 <self_sdio_check>:
 4027df4:	d2914d00 	mov	x0, #0x8a68                	// #35432
 4027df8:	aa0003f0 	mov	x16, x0
 4027dfc:	d61f0200 	br	x16

0000000004027e00 <ipc_send_msg>:
 4027e00:	7100201f 	cmp	w0, #0x8
 4027e04:	540003e8 	b.hi	4027e80 <ipc_send_msg+0x80>  // b.pmore
 4027e08:	b40003c1 	cbz	x1, 4027e80 <ipc_send_msg+0x80>
 4027e0c:	b9400422 	ldr	w2, [x1, #4]
 4027e10:	7100345f 	cmp	w2, #0xd
 4027e14:	54000368 	b.hi	4027e80 <ipc_send_msg+0x80>  // b.pmore
 4027e18:	d29c0405 	mov	x5, #0xe020                	// #57376
 4027e1c:	b9400022 	ldr	w2, [x1]
 4027e20:	f2a22065 	movk	x5, #0x1103, lsl #16
 4027e24:	d29c0484 	mov	x4, #0xe024                	// #57380
 4027e28:	f2a22064 	movk	x4, #0x1103, lsl #16
 4027e2c:	b90000a2 	str	w2, [x5]
 4027e30:	b9400423 	ldr	w3, [x1, #4]
 4027e34:	b9000083 	str	w3, [x4]
 4027e38:	4a030042 	eor	w2, w2, w3
 4027e3c:	d2800023 	mov	x3, #0x1                   	// #1
 4027e40:	91000463 	add	x3, x3, #0x1
 4027e44:	b9400426 	ldr	w6, [x1, #4]
 4027e48:	51000864 	sub	w4, w3, #0x2
 4027e4c:	6b0400df 	cmp	w6, w4
 4027e50:	540000e8 	b.hi	4027e6c <ipc_send_msg+0x6c>  // b.pmore
 4027e54:	d29c0b81 	mov	x1, #0xe05c                	// #57436
 4027e58:	f2a22061 	movk	x1, #0x1103, lsl #16
 4027e5c:	b9000022 	str	w2, [x1]
 4027e60:	b81a4020 	stur	w0, [x1, #-92]
 4027e64:	52800000 	mov	w0, #0x0                   	// #0
 4027e68:	d65f03c0 	ret
 4027e6c:	d37ef464 	lsl	x4, x3, #2
 4027e70:	b8637826 	ldr	w6, [x1, x3, lsl #2]
 4027e74:	4a060042 	eor	w2, w2, w6
 4027e78:	b82468a6 	str	w6, [x5, x4]
 4027e7c:	17fffff1 	b	4027e40 <ipc_send_msg+0x40>
 4027e80:	12800000 	mov	w0, #0xffffffff            	// #-1
 4027e84:	17fffff9 	b	4027e68 <ipc_send_msg+0x68>

0000000004027e88 <ipc_recv_msg>:
 4027e88:	b4000481 	cbz	x1, 4027f18 <ipc_recv_msg+0x90>
 4027e8c:	7100201f 	cmp	w0, #0x8
 4027e90:	54000448 	b.hi	4027f18 <ipc_recv_msg+0x90>  // b.pmore
 4027e94:	34000102 	cbz	w2, 4027eb4 <ipc_recv_msg+0x2c>
 4027e98:	52800022 	mov	w2, #0x1                   	// #1
 4027e9c:	d29c0104 	mov	x4, #0xe008                	// #57352
 4027ea0:	1ac02042 	lsl	w2, w2, w0
 4027ea4:	f2a22064 	movk	x4, #0x1103, lsl #16
 4027ea8:	b9400083 	ldr	w3, [x4]
 4027eac:	6a02007f 	tst	w3, w2
 4027eb0:	54ffffc0 	b.eq	4027ea8 <ipc_recv_msg+0x20>  // b.none
 4027eb4:	d29c0405 	mov	x5, #0xe020                	// #57376
 4027eb8:	d29c0483 	mov	x3, #0xe024                	// #57380
 4027ebc:	f2a22065 	movk	x5, #0x1103, lsl #16
 4027ec0:	f2a22063 	movk	x3, #0x1103, lsl #16
 4027ec4:	b94000a2 	ldr	w2, [x5]
 4027ec8:	b9000022 	str	w2, [x1]
 4027ecc:	b9400064 	ldr	w4, [x3]
 4027ed0:	b9000424 	str	w4, [x1, #4]
 4027ed4:	4a040042 	eor	w2, w2, w4
 4027ed8:	7100349f 	cmp	w4, #0xd
 4027edc:	54000308 	b.hi	4027f3c <ipc_recv_msg+0xb4>  // b.pmore
 4027ee0:	d2800023 	mov	x3, #0x1                   	// #1
 4027ee4:	91000463 	add	x3, x3, #0x1
 4027ee8:	51000866 	sub	w6, w3, #0x2
 4027eec:	6b0400df 	cmp	w6, w4
 4027ef0:	54000183 	b.cc	4027f20 <ipc_recv_msg+0x98>  // b.lo, b.ul, b.last
 4027ef4:	d29c0b83 	mov	x3, #0xe05c                	// #57436
 4027ef8:	f2a22063 	movk	x3, #0x1103, lsl #16
 4027efc:	b9400063 	ldr	w3, [x3]
 4027f00:	b9003c23 	str	w3, [x1, #60]
 4027f04:	6b03005f 	cmp	w2, w3
 4027f08:	d29c0082 	mov	x2, #0xe004                	// #57348
 4027f0c:	f2a22062 	movk	x2, #0x1103, lsl #16
 4027f10:	b9000040 	str	w0, [x2]
 4027f14:	54000100 	b.eq	4027f34 <ipc_recv_msg+0xac>  // b.none
 4027f18:	12800000 	mov	w0, #0xffffffff            	// #-1
 4027f1c:	14000007 	b	4027f38 <ipc_recv_msg+0xb0>
 4027f20:	d37ef466 	lsl	x6, x3, #2
 4027f24:	b86668a6 	ldr	w6, [x5, x6]
 4027f28:	b8237826 	str	w6, [x1, x3, lsl #2]
 4027f2c:	4a060042 	eor	w2, w2, w6
 4027f30:	17ffffed 	b	4027ee4 <ipc_recv_msg+0x5c>
 4027f34:	b9400420 	ldr	w0, [x1, #4]
 4027f38:	d65f03c0 	ret
 4027f3c:	d29c0081 	mov	x1, #0xe004                	// #57348
 4027f40:	f2a22061 	movk	x1, #0x1103, lsl #16
 4027f44:	b9000020 	str	w0, [x1]
 4027f48:	17fffff4 	b	4027f18 <ipc_recv_msg+0x90>

0000000004027f4c <delay_cl>:
 4027f4c:	53017c00 	lsr	w0, w0, #1
 4027f50:	35000040 	cbnz	w0, 4027f58 <delay_cl+0xc>
 4027f54:	d65f03c0 	ret
 4027f58:	d503201f 	nop
 4027f5c:	51000400 	sub	w0, w0, #0x1
 4027f60:	17fffffc 	b	4027f50 <delay_cl+0x4>

0000000004027f64 <get_ddr_scrub_and_gpll_switch>:
 4027f64:	d2802480 	mov	x0, #0x124                 	// #292
 4027f68:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4027f6c:	b9400000 	ldr	w0, [x0]
 4027f70:	d3451400 	ubfx	x0, x0, #5, #1
 4027f74:	d65f03c0 	ret

0000000004027f78 <bsp_gpll_config>:
 4027f78:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 4027f7c:	2a0003f3 	mov	w19, w0
 4027f80:	97fffff9 	bl	4027f64 <get_ddr_scrub_and_gpll_switch>
 4027f84:	12001c00 	and	w0, w0, #0xff
 4027f88:	7100041f 	cmp	w0, #0x1
 4027f8c:	54001341 	b.ne	40281f4 <bsp_gpll_config+0x27c>  // b.any
 4027f90:	d2800380 	mov	x0, #0x1c                  	// #28
 4027f94:	f2a203c0 	movk	x0, #0x101e, lsl #16
 4027f98:	b9400001 	ldr	w1, [x0]
 4027f9c:	d2840800 	mov	x0, #0x2040                	// #8256
 4027fa0:	f2a22020 	movk	x0, #0x1101, lsl #16
 4027fa4:	37c00261 	tbnz	w1, #24, 4027ff0 <bsp_gpll_config+0x78>
 4027fa8:	b9400001 	ldr	w1, [x0]
 4027fac:	12126421 	and	w1, w1, #0xffffc0ff
 4027fb0:	b9000001 	str	w1, [x0]
 4027fb4:	d2840001 	mov	x1, #0x2000                	// #8192
 4027fb8:	f2a22021 	movk	x1, #0x1101, lsl #16
 4027fbc:	b9400020 	ldr	w0, [x1]
 4027fc0:	12117000 	and	w0, w0, #0xffff8fff
 4027fc4:	32130400 	orr	w0, w0, #0x6000
 4027fc8:	b9000020 	str	w0, [x1]
 4027fcc:	52896f00 	mov	w0, #0x4b78                	// #19320
 4027fd0:	72b2d4a0 	movk	w0, #0x96a5, lsl #16
 4027fd4:	6b00027f 	cmp	w19, w0
 4027fd8:	54000141 	b.ne	4028000 <bsp_gpll_config+0x88>  // b.any
 4027fdc:	d287e801 	mov	x1, #0x3f40                	// #16192
 4027fe0:	f2a22021 	movk	x1, #0x1101, lsl #16
 4027fe4:	b9400020 	ldr	w0, [x1]
 4027fe8:	121b7800 	and	w0, w0, #0xffffffef
 4027fec:	14000057 	b	4028148 <bsp_gpll_config+0x1d0>
 4027ff0:	b9400001 	ldr	w1, [x0]
 4027ff4:	12126421 	and	w1, w1, #0xffffc0ff
 4027ff8:	b9000001 	str	w1, [x0]
 4027ffc:	17fffff4 	b	4027fcc <bsp_gpll_config+0x54>
 4028000:	529690e0 	mov	w0, #0xb487                	// #46215
 4028004:	72ad2b40 	movk	w0, #0x695a, lsl #16
 4028008:	6b00027f 	cmp	w19, w0
 402800c:	54000960 	b.eq	4028138 <bsp_gpll_config+0x1c0>  // b.none
 4028010:	52968f00 	mov	w0, #0xb478                	// #46200
 4028014:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 4028018:	6b00027f 	cmp	w19, w0
 402801c:	540009a0 	b.eq	4028150 <bsp_gpll_config+0x1d8>  // b.none
 4028020:	d2806081 	mov	x1, #0x304                 	// #772
 4028024:	d2806004 	mov	x4, #0x300                 	// #768
 4028028:	d2806705 	mov	x5, #0x338                 	// #824
 402802c:	52800062 	mov	w2, #0x3                   	// #3
 4028030:	f2a22021 	movk	x1, #0x1101, lsl #16
 4028034:	f2a22024 	movk	x4, #0x1101, lsl #16
 4028038:	52a25006 	mov	w6, #0x12800000            	// #310378496
 402803c:	f2a22025 	movk	x5, #0x1101, lsl #16
 4028040:	52800280 	mov	w0, #0x14                  	// #20
 4028044:	97ffffc2 	bl	4027f4c <delay_cl>
 4028048:	b9400020 	ldr	w0, [x1]
 402804c:	52800203 	mov	w3, #0x10                  	// #16
 4028050:	320c0000 	orr	w0, w0, #0x100000
 4028054:	b9000020 	str	w0, [x1]
 4028058:	b9000086 	str	w6, [x4]
 402805c:	52800020 	mov	w0, #0x1                   	// #1
 4028060:	97ffffbb 	bl	4027f4c <delay_cl>
 4028064:	b9400020 	ldr	w0, [x1]
 4028068:	120b7800 	and	w0, w0, #0xffefffff
 402806c:	b9000020 	str	w0, [x1]
 4028070:	52800280 	mov	w0, #0x14                  	// #20
 4028074:	97ffffb6 	bl	4027f4c <delay_cl>
 4028078:	b94000a0 	ldr	w0, [x5]
 402807c:	372002c0 	tbnz	w0, #4, 40280d4 <bsp_gpll_config+0x15c>
 4028080:	52800020 	mov	w0, #0x1                   	// #1
 4028084:	97ffffb2 	bl	4027f4c <delay_cl>
 4028088:	71000463 	subs	w3, w3, #0x1
 402808c:	54ffff61 	b.ne	4028078 <bsp_gpll_config+0x100>  // b.any
 4028090:	71000442 	subs	w2, w2, #0x1
 4028094:	54fffda1 	b.ne	4028048 <bsp_gpll_config+0xd0>  // b.any
 4028098:	52968f00 	mov	w0, #0xb478                	// #46200
 402809c:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 40280a0:	6b00027f 	cmp	w19, w0
 40280a4:	54000060 	b.eq	40280b0 <bsp_gpll_config+0x138>  // b.none
 40280a8:	52800b00 	mov	w0, #0x58                  	// #88
 40280ac:	97ffeff3 	bl	4024078 <err_print>
 40280b0:	d2800040 	mov	x0, #0x2                   	// #2
 40280b4:	97ffff28 	bl	4027d54 <mdelay>
 40280b8:	97ffff17 	bl	4027d14 <timer_deinit>
 40280bc:	d2800080 	mov	x0, #0x4                   	// #4
 40280c0:	52800021 	mov	w1, #0x1                   	// #1
 40280c4:	f2a22040 	movk	x0, #0x1102, lsl #16
 40280c8:	b9000001 	str	w1, [x0]
 40280cc:	b9000001 	str	w1, [x0]
 40280d0:	b9000001 	str	w1, [x0]
 40280d4:	d2800380 	mov	x0, #0x1c                  	// #28
 40280d8:	d2840801 	mov	x1, #0x2040                	// #8256
 40280dc:	f2a203c0 	movk	x0, #0x101e, lsl #16
 40280e0:	f2a22021 	movk	x1, #0x1101, lsl #16
 40280e4:	b9400000 	ldr	w0, [x0]
 40280e8:	37c00540 	tbnz	w0, #24, 4028190 <bsp_gpll_config+0x218>
 40280ec:	b9400020 	ldr	w0, [x1]
 40280f0:	52822002 	mov	w2, #0x1100                	// #4352
 40280f4:	12126400 	and	w0, w0, #0xffffc0ff
 40280f8:	2a020000 	orr	w0, w0, w2
 40280fc:	b9000020 	str	w0, [x1]
 4028100:	528a0002 	mov	w2, #0x5000                	// #20480
 4028104:	b85c0020 	ldur	w0, [x1, #-64]
 4028108:	12117000 	and	w0, w0, #0xffff8fff
 402810c:	2a020000 	orr	w0, w0, w2
 4028110:	b81c0020 	stur	w0, [x1, #-64]
 4028114:	52896f00 	mov	w0, #0x4b78                	// #19320
 4028118:	72b2d4a0 	movk	w0, #0x96a5, lsl #16
 402811c:	6b00027f 	cmp	w19, w0
 4028120:	54000441 	b.ne	40281a8 <bsp_gpll_config+0x230>  // b.any
 4028124:	d287e801 	mov	x1, #0x3f40                	// #16192
 4028128:	f2a22021 	movk	x1, #0x1101, lsl #16
 402812c:	b9400020 	ldr	w0, [x1]
 4028130:	321c0000 	orr	w0, w0, #0x10
 4028134:	1400002c 	b	40281e4 <bsp_gpll_config+0x26c>
 4028138:	d2869801 	mov	x1, #0x34c0                	// #13504
 402813c:	f2a22021 	movk	x1, #0x1101, lsl #16
 4028140:	b9400020 	ldr	w0, [x1]
 4028144:	121f7800 	and	w0, w0, #0xfffffffe
 4028148:	b9000020 	str	w0, [x1]
 402814c:	17ffffb5 	b	4028020 <bsp_gpll_config+0xa8>
 4028150:	97fffdac 	bl	4027800 <wait_uart_tx_busy_state>
 4028154:	7102941f 	cmp	w0, #0xa5
 4028158:	54000180 	b.eq	4028188 <bsp_gpll_config+0x210>  // b.none
 402815c:	52800ae0 	mov	w0, #0x57                  	// #87
 4028160:	97ffefc6 	bl	4024078 <err_print>
 4028164:	d2800040 	mov	x0, #0x2                   	// #2
 4028168:	97fffefb 	bl	4027d54 <mdelay>
 402816c:	97fffeea 	bl	4027d14 <timer_deinit>
 4028170:	d2800080 	mov	x0, #0x4                   	// #4
 4028174:	52800021 	mov	w1, #0x1                   	// #1
 4028178:	f2a22040 	movk	x0, #0x1102, lsl #16
 402817c:	b9000001 	str	w1, [x0]
 4028180:	b9000001 	str	w1, [x0]
 4028184:	b9000001 	str	w1, [x0]
 4028188:	d2883001 	mov	x1, #0x4180                	// #16768
 402818c:	17ffff95 	b	4027fe0 <bsp_gpll_config+0x68>
 4028190:	b9400020 	ldr	w0, [x1]
 4028194:	52828002 	mov	w2, #0x1400                	// #5120
 4028198:	12126400 	and	w0, w0, #0xffffc0ff
 402819c:	2a020000 	orr	w0, w0, w2
 40281a0:	b9000020 	str	w0, [x1]
 40281a4:	17ffffdc 	b	4028114 <bsp_gpll_config+0x19c>
 40281a8:	529690e0 	mov	w0, #0xb487                	// #46215
 40281ac:	72ad2b40 	movk	w0, #0x695a, lsl #16
 40281b0:	6b00027f 	cmp	w19, w0
 40281b4:	54000100 	b.eq	40281d4 <bsp_gpll_config+0x25c>  // b.none
 40281b8:	52968f00 	mov	w0, #0xb478                	// #46200
 40281bc:	72ad34a0 	movk	w0, #0x69a5, lsl #16
 40281c0:	6b00027f 	cmp	w19, w0
 40281c4:	54000140 	b.eq	40281ec <bsp_gpll_config+0x274>  // b.none
 40281c8:	a8c17bf3 	ldp	x19, x30, [sp], #16
 40281cc:	52800280 	mov	w0, #0x14                  	// #20
 40281d0:	17ffff5f 	b	4027f4c <delay_cl>
 40281d4:	d2869801 	mov	x1, #0x34c0                	// #13504
 40281d8:	f2a22021 	movk	x1, #0x1101, lsl #16
 40281dc:	b9400020 	ldr	w0, [x1]
 40281e0:	32000000 	orr	w0, w0, #0x1
 40281e4:	b9000020 	str	w0, [x1]
 40281e8:	17fffff8 	b	40281c8 <bsp_gpll_config+0x250>
 40281ec:	d2883001 	mov	x1, #0x4180                	// #16768
 40281f0:	17ffffce 	b	4028128 <bsp_gpll_config+0x1b0>
 40281f4:	a8c17bf3 	ldp	x19, x30, [sp], #16
 40281f8:	d65f03c0 	ret

00000000040281fc <ddrtrn_training_boot_func>:
 40281fc:	d10183ff 	sub	sp, sp, #0x60
 4028200:	b0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4028204:	a9057bf3 	stp	x19, x30, [sp, #80]
 4028208:	f9455001 	ldr	x1, [x0, #2720]
 402820c:	f90027e1 	str	x1, [sp, #72]
 4028210:	d2800001 	mov	x1, #0x0                   	// #0
 4028214:	940000f8 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028218:	b9400400 	ldr	w0, [x0, #4]
 402821c:	9400091a 	bl	402a684 <ddrtrn_hal_get_gt_status>
 4028220:	340000e0 	cbz	w0, 402823c <ddrtrn_training_boot_func+0x40>
 4028224:	940000f4 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028228:	b9400401 	ldr	w1, [x0, #4]
 402822c:	12800003 	mov	w3, #0xffffffff            	// #-1
 4028230:	52800040 	mov	w0, #0x2                   	// #2
 4028234:	2a0303e2 	mov	w2, w3
 4028238:	94000804 	bl	402a248 <ddrtrn_hal_training_stat>
 402823c:	940009a3 	bl	402a8c8 <ddrtrn_wl_func>
 4028240:	2a0003f3 	mov	w19, w0
 4028244:	9400107c 	bl	402c434 <ddrtrn_dataeye_training_func>
 4028248:	2b000273 	adds	w19, w19, w0
 402824c:	540001e0 	b.eq	4028288 <ddrtrn_training_boot_func+0x8c>  // b.none
 4028250:	52a00200 	mov	w0, #0x100000              	// #1048576
 4028254:	94000571 	bl	4029818 <ddrtrn_hal_check_bypass>
 4028258:	35000180 	cbnz	w0, 4028288 <ddrtrn_training_boot_func+0x8c>
 402825c:	910023e0 	add	x0, sp, #0x8
 4028260:	52a00201 	mov	w1, #0x100000              	// #1048576
 4028264:	9400039a 	bl	40290cc <ddrtrn_hal_save_reg>
 4028268:	94000a82 	bl	402ac70 <ddrtrn_hw_dataeye_read>
 402826c:	2a0003f3 	mov	w19, w0
 4028270:	910023e0 	add	x0, sp, #0x8
 4028274:	940003da 	bl	40291dc <ddrtrn_hal_restore_reg>
 4028278:	52800020 	mov	w0, #0x1                   	// #1
 402827c:	94000987 	bl	402a898 <ddrtrn_hal_set_adjust>
 4028280:	94001047 	bl	402c39c <ddrtrn_dataeye_training>
 4028284:	0b000273 	add	w19, w19, w0
 4028288:	940009dc 	bl	402a9f8 <ddrtrn_gating_func>
 402828c:	0b000273 	add	w19, w19, w0
 4028290:	94000e6e 	bl	402bc48 <ddrtrn_vref_training_func>
 4028294:	0b000260 	add	w0, w19, w0
 4028298:	b0ffffc1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 402829c:	f94027e2 	ldr	x2, [sp, #72]
 40282a0:	f9455023 	ldr	x3, [x1, #2720]
 40282a4:	eb030042 	subs	x2, x2, x3
 40282a8:	d2800003 	mov	x3, #0x0                   	// #0
 40282ac:	54000040 	b.eq	40282b4 <ddrtrn_training_boot_func+0xb8>  // b.none
 40282b0:	97ffefaf 	bl	402416c <__stack_chk_fail>
 40282b4:	a9457bf3 	ldp	x19, x30, [sp, #80]
 40282b8:	910183ff 	add	sp, sp, #0x60
 40282bc:	d65f03c0 	ret

00000000040282c0 <ddrtrn_training_by_dmc>:
 40282c0:	f81f0ffe 	str	x30, [sp, #-16]!
 40282c4:	940000cc 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40282c8:	b9402c00 	ldr	w0, [x0, #44]
 40282cc:	35000060 	cbnz	w0, 40282d8 <ddrtrn_training_by_dmc+0x18>
 40282d0:	f84107fe 	ldr	x30, [sp], #16
 40282d4:	17ffffca 	b	40281fc <ddrtrn_training_boot_func>
 40282d8:	52800000 	mov	w0, #0x0                   	// #0
 40282dc:	f84107fe 	ldr	x30, [sp], #16
 40282e0:	d65f03c0 	ret

00000000040282e4 <ddrtrn_training_by_rank>:
 40282e4:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 40282e8:	a9015bf5 	stp	x21, x22, [sp, #16]
 40282ec:	52800015 	mov	w21, #0x0                   	// #0
 40282f0:	d2800796 	mov	x22, #0x3c                  	// #60
 40282f4:	a90263f7 	stp	x23, x24, [sp, #32]
 40282f8:	d2800197 	mov	x23, #0xc                   	// #12
 40282fc:	f9001bfe 	str	x30, [sp, #48]
 4028300:	940000bd 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028304:	b9400413 	ldr	w19, [x0, #4]
 4028308:	940000bb 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402830c:	b9402401 	ldr	w1, [x0, #36]
 4028310:	2a1303e0 	mov	w0, w19
 4028314:	52800013 	mov	w19, #0x0                   	// #0
 4028318:	9400091e 	bl	402a790 <ddrtrn_hal_phy_switch_rank>
 402831c:	940000b6 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028320:	aa0003f4 	mov	x20, x0
 4028324:	940000af 	bl	40285e0 <ddrtrn_hal_get_phy>
 4028328:	b9402281 	ldr	w1, [x20, #32]
 402832c:	9b160021 	madd	x1, x1, x22, x0
 4028330:	b9400820 	ldr	w0, [x1, #8]
 4028334:	6b00027f 	cmp	w19, w0
 4028338:	540000e3 	b.cc	4028354 <ddrtrn_training_by_rank+0x70>  // b.lo, b.ul, b.last
 402833c:	2a1503e0 	mov	w0, w21
 4028340:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4028344:	a94263f7 	ldp	x23, x24, [sp, #32]
 4028348:	f9401bfe 	ldr	x30, [sp, #48]
 402834c:	a8c453f3 	ldp	x19, x20, [sp], #64
 4028350:	d65f03c0 	ret
 4028354:	940000a8 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028358:	b9002813 	str	w19, [x0, #40]
 402835c:	2a1303f4 	mov	w20, w19
 4028360:	11000673 	add	w19, w19, #0x1
 4028364:	940000a4 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028368:	aa0003f8 	mov	x24, x0
 402836c:	9400009d 	bl	40285e0 <ddrtrn_hal_get_phy>
 4028370:	9b177e94 	mul	x20, x20, x23
 4028374:	b9402301 	ldr	w1, [x24, #32]
 4028378:	9b165021 	madd	x1, x1, x22, x20
 402837c:	8b010000 	add	x0, x0, x1
 4028380:	b9401418 	ldr	w24, [x0, #20]
 4028384:	9400009c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028388:	b9000818 	str	w24, [x0, #8]
 402838c:	9400009a 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028390:	aa0003f8 	mov	x24, x0
 4028394:	94000093 	bl	40285e0 <ddrtrn_hal_get_phy>
 4028398:	b9402301 	ldr	w1, [x24, #32]
 402839c:	9b165034 	madd	x20, x1, x22, x20
 40283a0:	8b140000 	add	x0, x0, x20
 40283a4:	b9401c14 	ldr	w20, [x0, #28]
 40283a8:	94000093 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40283ac:	b9001014 	str	w20, [x0, #16]
 40283b0:	97ffffc4 	bl	40282c0 <ddrtrn_training_by_dmc>
 40283b4:	0b0002b5 	add	w21, w21, w0
 40283b8:	17ffffd9 	b	402831c <ddrtrn_training_by_rank+0x38>

00000000040283bc <ddrtrn_training_by_phy>:
 40283bc:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 40283c0:	52800014 	mov	w20, #0x0                   	// #0
 40283c4:	a9015bf5 	stp	x21, x22, [sp, #16]
 40283c8:	52800035 	mov	w21, #0x1                   	// #1
 40283cc:	d2800796 	mov	x22, #0x3c                  	// #60
 40283d0:	a90263f7 	stp	x23, x24, [sp, #32]
 40283d4:	f9001bfe 	str	x30, [sp, #48]
 40283d8:	94000087 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40283dc:	b9402000 	ldr	w0, [x0, #32]
 40283e0:	1ac022b5 	lsl	w21, w21, w0
 40283e4:	94000084 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40283e8:	aa0003f3 	mov	x19, x0
 40283ec:	9400007d 	bl	40285e0 <ddrtrn_hal_get_phy>
 40283f0:	b9402261 	ldr	w1, [x19, #32]
 40283f4:	52800013 	mov	w19, #0x0                   	// #0
 40283f8:	9b160021 	madd	x1, x1, x22, x0
 40283fc:	b9400c37 	ldr	w23, [x1, #12]
 4028400:	6b17027f 	cmp	w19, w23
 4028404:	540001e1 	b.ne	4028440 <ddrtrn_training_by_phy+0x84>  // b.any
 4028408:	71000a7f 	cmp	w19, #0x2
 402840c:	540000e1 	b.ne	4028428 <ddrtrn_training_by_phy+0x6c>  // b.any
 4028410:	9400060d 	bl	4029c44 <ddrtrn_hal_training_adjust_wdq>
 4028414:	940006be 	bl	4029f0c <ddrtrn_hal_training_adjust_wdqs>
 4028418:	94000077 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402841c:	b9400400 	ldr	w0, [x0, #4]
 4028420:	52800001 	mov	w1, #0x0                   	// #0
 4028424:	940008db 	bl	402a790 <ddrtrn_hal_phy_switch_rank>
 4028428:	2a1403e0 	mov	w0, w20
 402842c:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4028430:	a94263f7 	ldp	x23, x24, [sp, #32]
 4028434:	f9401bfe 	ldr	x30, [sp, #48]
 4028438:	a8c453f3 	ldp	x19, x20, [sp], #64
 402843c:	d65f03c0 	ret
 4028440:	9400006d 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028444:	b9002413 	str	w19, [x0, #36]
 4028448:	9400006b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402844c:	aa0003f8 	mov	x24, x0
 4028450:	94000064 	bl	40285e0 <ddrtrn_hal_get_phy>
 4028454:	d37d7e61 	ubfiz	x1, x19, #3, #32
 4028458:	b9402302 	ldr	w2, [x24, #32]
 402845c:	91008021 	add	x1, x1, #0x20
 4028460:	9b160441 	madd	x1, x2, x22, x1
 4028464:	8b010000 	add	x0, x0, x1
 4028468:	b9400c18 	ldr	w24, [x0, #12]
 402846c:	94000062 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028470:	b9000c18 	str	w24, [x0, #12]
 4028474:	2a1503e0 	mov	w0, w21
 4028478:	940004e8 	bl	4029818 <ddrtrn_hal_check_bypass>
 402847c:	35000060 	cbnz	w0, 4028488 <ddrtrn_training_by_phy+0xcc>
 4028480:	97ffff99 	bl	40282e4 <ddrtrn_training_by_rank>
 4028484:	0b000294 	add	w20, w20, w0
 4028488:	11000673 	add	w19, w19, #0x1
 402848c:	17ffffdd 	b	4028400 <ddrtrn_training_by_phy+0x44>

0000000004028490 <ddrtrn_training_all>:
 4028490:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 4028494:	52800013 	mov	w19, #0x0                   	// #0
 4028498:	52800014 	mov	w20, #0x0                   	// #0
 402849c:	a9015bf5 	stp	x21, x22, [sp, #16]
 40284a0:	d2800795 	mov	x21, #0x3c                  	// #60
 40284a4:	f90013fe 	str	x30, [sp, #32]
 40284a8:	94000053 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40284ac:	b9400000 	ldr	w0, [x0]
 40284b0:	6b00027f 	cmp	w19, w0
 40284b4:	540000c3 	b.cc	40284cc <ddrtrn_training_all+0x3c>  // b.lo, b.ul, b.last
 40284b8:	2a1403e0 	mov	w0, w20
 40284bc:	a9415bf5 	ldp	x21, x22, [sp, #16]
 40284c0:	f94013fe 	ldr	x30, [sp, #32]
 40284c4:	a8c353f3 	ldp	x19, x20, [sp], #48
 40284c8:	d65f03c0 	ret
 40284cc:	9400004a 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40284d0:	b9002013 	str	w19, [x0, #32]
 40284d4:	94000043 	bl	40285e0 <ddrtrn_hal_get_phy>
 40284d8:	2a1303e1 	mov	w1, w19
 40284dc:	11000673 	add	w19, w19, #0x1
 40284e0:	9b157c21 	mul	x1, x1, x21
 40284e4:	b8616816 	ldr	w22, [x0, x1]
 40284e8:	94000043 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40284ec:	b9000416 	str	w22, [x0, #4]
 40284f0:	97ffffb3 	bl	40283bc <ddrtrn_training_by_phy>
 40284f4:	0b000294 	add	w20, w20, w0
 40284f8:	17ffffec 	b	40284a8 <ddrtrn_training_all+0x18>

00000000040284fc <ddrtrn_sw_training_func>:
 40284fc:	d100c3ff 	sub	sp, sp, #0x30
 4028500:	b0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4028504:	a9027bf3 	stp	x19, x30, [sp, #32]
 4028508:	f9455001 	ldr	x1, [x0, #2720]
 402850c:	f9000fe1 	str	x1, [sp, #24]
 4028510:	d2800001 	mov	x1, #0x0                   	// #0
 4028514:	940008be 	bl	402a80c <ddrtrn_hal_version_flag>
 4028518:	940008c3 	bl	402a824 <ddrtrn_hal_check_sw_item>
 402851c:	2a0003f3 	mov	w19, w0
 4028520:	340002e0 	cbz	w0, 402857c <ddrtrn_sw_training_func+0x80>
 4028524:	94000ff4 	bl	402c4f4 <ddrtrn_training_start>
 4028528:	52800202 	mov	w2, #0x10                  	// #16
 402852c:	52800001 	mov	w1, #0x0                   	// #0
 4028530:	910023e0 	add	x0, sp, #0x8
 4028534:	940008f4 	bl	402a904 <ddrtrn_set_data>
 4028538:	910023e0 	add	x0, sp, #0x8
 402853c:	9400008e 	bl	4028774 <ddrtrn_hal_boot_cmd_save>
 4028540:	2a0003f3 	mov	w19, w0
 4028544:	350002e0 	cbnz	w0, 40285a0 <ddrtrn_sw_training_func+0xa4>
 4028548:	940008dc 	bl	402a8b8 <ddrtrn_hal_clear_sysctrl_stat_reg>
 402854c:	9400002a 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028550:	b9002c1f 	str	wzr, [x0, #44]
 4028554:	940008c1 	bl	402a858 <ddrtrn_hal_check_not_dcc_item>
 4028558:	35000060 	cbnz	w0, 4028564 <ddrtrn_sw_training_func+0x68>
 402855c:	97ffffcd 	bl	4028490 <ddrtrn_training_all>
 4028560:	2a0003f3 	mov	w19, w0
 4028564:	94000c7b 	bl	402b750 <ddrtrn_dcc_training_func>
 4028568:	2b130013 	adds	w19, w0, w19
 402856c:	54000161 	b.ne	4028598 <ddrtrn_sw_training_func+0x9c>  // b.any
 4028570:	94000fe0 	bl	402c4f0 <ddrtrn_training_success>
 4028574:	910023e0 	add	x0, sp, #0x8
 4028578:	940000a6 	bl	4028810 <ddrtrn_hal_boot_cmd_restore>
 402857c:	b0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4028580:	f9400fe1 	ldr	x1, [sp, #24]
 4028584:	f9455002 	ldr	x2, [x0, #2720]
 4028588:	eb020021 	subs	x1, x1, x2
 402858c:	d2800002 	mov	x2, #0x0                   	// #0
 4028590:	540000c0 	b.eq	40285a8 <ddrtrn_sw_training_func+0xac>  // b.none
 4028594:	97ffeef6 	bl	402416c <__stack_chk_fail>
 4028598:	940008d0 	bl	402a8d8 <ddrtrn_training_console_if>
 402859c:	17fffff6 	b	4028574 <ddrtrn_sw_training_func+0x78>
 40285a0:	12800013 	mov	w19, #0xffffffff            	// #-1
 40285a4:	17fffff6 	b	402857c <ddrtrn_sw_training_func+0x80>
 40285a8:	2a1303e0 	mov	w0, w19
 40285ac:	a9427bf3 	ldp	x19, x30, [sp, #32]
 40285b0:	9100c3ff 	add	sp, sp, #0x30
 40285b4:	d65f03c0 	ret

00000000040285b8 <ddrtrn_hw_training_func>:
 40285b8:	14000a6e 	b	402af70 <ddrtrn_hw_training>

00000000040285bc <ddrtrn_sw_training_if>:
 40285bc:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 40285c0:	2a0003f3 	mov	w19, w0
 40285c4:	94000018 	bl	4028624 <ddrtrn_hal_cfg_init>
 40285c8:	2a1303e0 	mov	w0, w19
 40285cc:	94000481 	bl	40297d0 <ddrtrn_hal_sw_item_cfg>
 40285d0:	a8c17bf3 	ldp	x19, x30, [sp], #16
 40285d4:	17ffffca 	b	40284fc <ddrtrn_sw_training_func>

00000000040285d8 <ddrtrn_hw_training_if>:
 40285d8:	14000a66 	b	402af70 <ddrtrn_hw_training>

00000000040285dc <ddrtrn_trace>:
 40285dc:	d65f03c0 	ret

00000000040285e0 <ddrtrn_hal_get_phy>:
 40285e0:	d2801780 	mov	x0, #0xbc                  	// #188
 40285e4:	f2a22040 	movk	x0, #0x1102, lsl #16
 40285e8:	b9400000 	ldr	w0, [x0]
 40285ec:	2a0003e0 	mov	w0, w0
 40285f0:	d65f03c0 	ret

00000000040285f4 <ddrtrn_hal_get_ctx>:
 40285f4:	d2801700 	mov	x0, #0xb8                  	// #184
 40285f8:	f2a22040 	movk	x0, #0x1102, lsl #16
 40285fc:	b9400000 	ldr	w0, [x0]
 4028600:	2a0003e0 	mov	w0, w0
 4028604:	d65f03c0 	ret

0000000004028608 <ddrtrn_hal_set_cfg_addr>:
 4028608:	d2801702 	mov	x2, #0xb8                  	// #184
 402860c:	f2a22042 	movk	x2, #0x1102, lsl #16
 4028610:	b9000040 	str	w0, [x2]
 4028614:	d2801780 	mov	x0, #0xbc                  	// #188
 4028618:	f2a22040 	movk	x0, #0x1102, lsl #16
 402861c:	b9000001 	str	w1, [x0]
 4028620:	d65f03c0 	ret

0000000004028624 <ddrtrn_hal_cfg_init>:
 4028624:	d2801700 	mov	x0, #0xb8                  	// #184
 4028628:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402862c:	f2a22040 	movk	x0, #0x1102, lsl #16
 4028630:	52800682 	mov	w2, #0x34                  	// #52
 4028634:	52800001 	mov	w1, #0x0                   	// #0
 4028638:	b9400014 	ldr	w20, [x0]
 402863c:	b9400413 	ldr	w19, [x0, #4]
 4028640:	2a1403f4 	mov	w20, w20
 4028644:	f9000bfe 	str	x30, [sp, #16]
 4028648:	2a1303f3 	mov	w19, w19
 402864c:	aa1403e0 	mov	x0, x20
 4028650:	940008ad 	bl	402a904 <ddrtrn_set_data>
 4028654:	52800782 	mov	w2, #0x3c                  	// #60
 4028658:	52800001 	mov	w1, #0x0                   	// #0
 402865c:	aa1303e0 	mov	x0, x19
 4028660:	940008a9 	bl	402a904 <ddrtrn_set_data>
 4028664:	52a222a0 	mov	w0, #0x11150000            	// #286588928
 4028668:	52800022 	mov	w2, #0x1                   	// #1
 402866c:	b9000282 	str	w2, [x20]
 4028670:	d2801501 	mov	x1, #0xa8                  	// #168
 4028674:	b9000260 	str	w0, [x19]
 4028678:	d2800580 	mov	x0, #0x2c                  	// #44
 402867c:	f2a222a0 	movk	x0, #0x1115, lsl #16
 4028680:	f2a22041 	movk	x1, #0x1102, lsl #16
 4028684:	b9400000 	ldr	w0, [x0]
 4028688:	12000c00 	and	w0, w0, #0xf
 402868c:	b9000660 	str	w0, [x19, #4]
 4028690:	7100181f 	cmp	w0, #0x6
 4028694:	52900000 	mov	w0, #0x8000                	// #32768
 4028698:	72a22280 	movk	w0, #0x1114, lsl #16
 402869c:	54000581 	b.ne	402874c <ddrtrn_hal_cfg_init+0x128>  // b.any
 40286a0:	52800042 	mov	w2, #0x2                   	// #2
 40286a4:	b9000a62 	str	w2, [x19, #8]
 40286a8:	b9400034 	ldr	w20, [x1]
 40286ac:	b9001660 	str	w0, [x19, #20]
 40286b0:	12003e81 	and	w1, w20, #0xffff
 40286b4:	b9001e61 	str	w1, [x19, #28]
 40286b8:	53107e94 	lsr	w20, w20, #16
 40286bc:	94000340 	bl	40293bc <ddrtrn_hal_phy_get_byte_num>
 40286c0:	b9001a60 	str	w0, [x19, #24]
 40286c4:	52920000 	mov	w0, #0x9000                	// #36864
 40286c8:	b9002a74 	str	w20, [x19, #40]
 40286cc:	72a22280 	movk	w0, #0x1114, lsl #16
 40286d0:	b9002260 	str	w0, [x19, #32]
 40286d4:	9400033a 	bl	40293bc <ddrtrn_hal_phy_get_byte_num>
 40286d8:	b9002660 	str	w0, [x19, #36]
 40286dc:	b9401a61 	ldr	w1, [x19, #24]
 40286e0:	0b000020 	add	w0, w1, w0
 40286e4:	52800021 	mov	w1, #0x1                   	// #1
 40286e8:	f9400bfe 	ldr	x30, [sp, #16]
 40286ec:	29018261 	stp	w1, w0, [x19, #12]
 40286f0:	d2801400 	mov	x0, #0xa0                  	// #160
 40286f4:	f2a22040 	movk	x0, #0x1102, lsl #16
 40286f8:	b9400000 	ldr	w0, [x0]
 40286fc:	b9002e60 	str	w0, [x19, #44]
 4028700:	d2801200 	mov	x0, #0x90                  	// #144
 4028704:	f2a22040 	movk	x0, #0x1102, lsl #16
 4028708:	b9400000 	ldr	w0, [x0]
 402870c:	b9003260 	str	w0, [x19, #48]
 4028710:	d2801480 	mov	x0, #0xa4                  	// #164
 4028714:	f2a22040 	movk	x0, #0x1102, lsl #16
 4028718:	b9400000 	ldr	w0, [x0]
 402871c:	b9003660 	str	w0, [x19, #52]
 4028720:	d2801280 	mov	x0, #0x94                  	// #148
 4028724:	f2a22040 	movk	x0, #0x1102, lsl #16
 4028728:	b9400002 	ldr	w2, [x0]
 402872c:	b9003a62 	str	w2, [x19, #56]
 4028730:	b9400000 	ldr	w0, [x0]
 4028734:	7100001f 	cmp	w0, #0x0
 4028738:	52800040 	mov	w0, #0x2                   	// #2
 402873c:	1a811000 	csel	w0, w0, w1, ne  // ne = any
 4028740:	b9000e60 	str	w0, [x19, #12]
 4028744:	a8c253f3 	ldp	x19, x20, [sp], #32
 4028748:	d65f03c0 	ret
 402874c:	b9000a62 	str	w2, [x19, #8]
 4028750:	b9001660 	str	w0, [x19, #20]
 4028754:	b9400021 	ldr	w1, [x1]
 4028758:	b9001e61 	str	w1, [x19, #28]
 402875c:	94000318 	bl	40293bc <ddrtrn_hal_phy_get_byte_num>
 4028760:	b9001a60 	str	w0, [x19, #24]
 4028764:	17ffffe0 	b	40286e4 <ddrtrn_hal_cfg_init+0xc0>

0000000004028768 <ddrtrn_hal_cmd_prepare_copy>:
 4028768:	d65f03c0 	ret

000000000402876c <ddrtrn_hal_cmd_site_save>:
 402876c:	d65f03c0 	ret

0000000004028770 <ddrtrn_hal_cmd_site_restore>:
 4028770:	d65f03c0 	ret

0000000004028774 <ddrtrn_hal_boot_cmd_save>:
 4028774:	b5000060 	cbnz	x0, 4028780 <ddrtrn_hal_boot_cmd_save+0xc>
 4028778:	12800000 	mov	w0, #0xffffffff            	// #-1
 402877c:	d65f03c0 	ret
 4028780:	d2884282 	mov	x2, #0x4214                	// #16916
 4028784:	f2a22042 	movk	x2, #0x1102, lsl #16
 4028788:	b9400041 	ldr	w1, [x2]
 402878c:	b9000c01 	str	w1, [x0, #12]
 4028790:	32000021 	orr	w1, w1, #0x1
 4028794:	b9000041 	str	w1, [x2]
 4028798:	d2845401 	mov	x1, #0x22a0                	// #8864
 402879c:	f2a22021 	movk	x1, #0x1101, lsl #16
 40287a0:	b9400022 	ldr	w2, [x1]
 40287a4:	b9000002 	str	w2, [x0]
 40287a8:	321c0042 	orr	w2, w2, #0x10
 40287ac:	b9000022 	str	w2, [x1]
 40287b0:	d503201f 	nop
 40287b4:	b9400022 	ldr	w2, [x1]
 40287b8:	121f7842 	and	w2, w2, #0xfffffffe
 40287bc:	b9000022 	str	w2, [x1]
 40287c0:	d2801602 	mov	x2, #0xb0                  	// #176
 40287c4:	f2a222a2 	movk	x2, #0x1115, lsl #16
 40287c8:	b9400041 	ldr	w1, [x2]
 40287cc:	b9000401 	str	w1, [x0, #4]
 40287d0:	12007821 	and	w1, w1, #0x7fffffff
 40287d4:	b9000041 	str	w1, [x2]
 40287d8:	9120c042 	add	x2, x2, #0x830
 40287dc:	b9400041 	ldr	w1, [x2]
 40287e0:	b9000801 	str	w1, [x0, #8]
 40287e4:	52800000 	mov	w0, #0x0                   	// #0
 40287e8:	121f7821 	and	w1, w1, #0xfffffffe
 40287ec:	b9000041 	str	w1, [x2]
 40287f0:	b9400041 	ldr	w1, [x2]
 40287f4:	11000400 	add	w0, w0, #0x1
 40287f8:	36000081 	tbz	w1, #0, 4028808 <ddrtrn_hal_boot_cmd_save+0x94>
 40287fc:	3100041f 	cmn	w0, #0x1
 4028800:	54ffff81 	b.ne	40287f0 <ddrtrn_hal_boot_cmd_save+0x7c>  // b.any
 4028804:	17ffffdd 	b	4028778 <ddrtrn_hal_boot_cmd_save+0x4>
 4028808:	52800000 	mov	w0, #0x0                   	// #0
 402880c:	17ffffdc 	b	402877c <ddrtrn_hal_boot_cmd_save+0x8>

0000000004028810 <ddrtrn_hal_boot_cmd_restore>:
 4028810:	b4000220 	cbz	x0, 4028854 <ddrtrn_hal_boot_cmd_restore+0x44>
 4028814:	d2884281 	mov	x1, #0x4214                	// #16916
 4028818:	b9400c02 	ldr	w2, [x0, #12]
 402881c:	f2a22041 	movk	x1, #0x1102, lsl #16
 4028820:	b9000022 	str	w2, [x1]
 4028824:	d2845401 	mov	x1, #0x22a0                	// #8864
 4028828:	f2a22021 	movk	x1, #0x1101, lsl #16
 402882c:	b9400002 	ldr	w2, [x0]
 4028830:	b9000022 	str	w2, [x1]
 4028834:	d2801601 	mov	x1, #0xb0                  	// #176
 4028838:	f2a222a1 	movk	x1, #0x1115, lsl #16
 402883c:	b9400402 	ldr	w2, [x0, #4]
 4028840:	b9000022 	str	w2, [x1]
 4028844:	b9400801 	ldr	w1, [x0, #8]
 4028848:	d2811c00 	mov	x0, #0x8e0                 	// #2272
 402884c:	f2a222a0 	movk	x0, #0x1115, lsl #16
 4028850:	b9000001 	str	w1, [x0]
 4028854:	d65f03c0 	ret

0000000004028858 <ddrtrn_hw_training_init>:
 4028858:	d10103ff 	sub	sp, sp, #0x40
 402885c:	52800202 	mov	w2, #0x10                  	// #16
 4028860:	a90253f3 	stp	x19, x20, [sp, #32]
 4028864:	2a0003f3 	mov	w19, w0
 4028868:	b0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402886c:	a9037bf5 	stp	x21, x30, [sp, #48]
 4028870:	2a0103f5 	mov	w21, w1
 4028874:	f9455001 	ldr	x1, [x0, #2720]
 4028878:	f9000fe1 	str	x1, [sp, #24]
 402887c:	d2800001 	mov	x1, #0x0                   	// #0
 4028880:	910023e0 	add	x0, sp, #0x8
 4028884:	94000820 	bl	402a904 <ddrtrn_set_data>
 4028888:	d2800580 	mov	x0, #0x2c                  	// #44
 402888c:	f2a222a0 	movk	x0, #0x1115, lsl #16
 4028890:	b9400014 	ldr	w20, [x0]
 4028894:	97ffff64 	bl	4028624 <ddrtrn_hal_cfg_init>
 4028898:	910023e0 	add	x0, sp, #0x8
 402889c:	97ffffb6 	bl	4028774 <ddrtrn_hal_boot_cmd_save>
 40288a0:	35000280 	cbnz	w0, 40288f0 <ddrtrn_hw_training_init+0x98>
 40288a4:	12000e94 	and	w20, w20, #0xf
 40288a8:	710006bf 	cmp	w21, #0x1
 40288ac:	7a460a80 	ccmp	w20, #0x6, #0x0, eq  // eq = none
 40288b0:	2a1303e0 	mov	w0, w19
 40288b4:	54000181 	b.ne	40288e4 <ddrtrn_hw_training_init+0x8c>  // b.any
 40288b8:	94000ff5 	bl	402c88c <ddrtrn_low_freq_start>
 40288bc:	2a0003e3 	mov	w3, w0
 40288c0:	910023e0 	add	x0, sp, #0x8
 40288c4:	97ffffd3 	bl	4028810 <ddrtrn_hal_boot_cmd_restore>
 40288c8:	b0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 40288cc:	f9400fe1 	ldr	x1, [sp, #24]
 40288d0:	f9455002 	ldr	x2, [x0, #2720]
 40288d4:	eb020021 	subs	x1, x1, x2
 40288d8:	d2800002 	mov	x2, #0x0                   	// #0
 40288dc:	540000e0 	b.eq	40288f8 <ddrtrn_hw_training_init+0xa0>  // b.none
 40288e0:	97ffee23 	bl	402416c <__stack_chk_fail>
 40288e4:	940003a9 	bl	4029788 <ddrtrn_hal_hw_item_cfg>
 40288e8:	97ffff3c 	bl	40285d8 <ddrtrn_hw_training_if>
 40288ec:	17fffff4 	b	40288bc <ddrtrn_hw_training_init+0x64>
 40288f0:	12800003 	mov	w3, #0xffffffff            	// #-1
 40288f4:	17fffff5 	b	40288c8 <ddrtrn_hw_training_init+0x70>
 40288f8:	2a0303e0 	mov	w0, w3
 40288fc:	a94253f3 	ldp	x19, x20, [sp, #32]
 4028900:	a9437bf5 	ldp	x21, x30, [sp, #48]
 4028904:	910103ff 	add	sp, sp, #0x40
 4028908:	d65f03c0 	ret

000000000402890c <ddrtrn_hal_get_cur_phy_rank_num>:
 402890c:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 4028910:	97ffff39 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028914:	aa0003f3 	mov	x19, x0
 4028918:	97ffff32 	bl	40285e0 <ddrtrn_hal_get_phy>
 402891c:	d2800782 	mov	x2, #0x3c                  	// #60
 4028920:	b9402261 	ldr	w1, [x19, #32]
 4028924:	a8c17bf3 	ldp	x19, x30, [sp], #16
 4028928:	d503201f 	nop
 402892c:	9b020021 	madd	x1, x1, x2, x0
 4028930:	b9400c20 	ldr	w0, [x1, #12]
 4028934:	d65f03c0 	ret

0000000004028938 <ddrtrn_hal_get_cur_phy_dram_type>:
 4028938:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402893c:	97ffff2e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028940:	aa0003f3 	mov	x19, x0
 4028944:	97ffff27 	bl	40285e0 <ddrtrn_hal_get_phy>
 4028948:	d2800782 	mov	x2, #0x3c                  	// #60
 402894c:	b9402261 	ldr	w1, [x19, #32]
 4028950:	a8c17bf3 	ldp	x19, x30, [sp], #16
 4028954:	d503201f 	nop
 4028958:	9b020021 	madd	x1, x1, x2, x0
 402895c:	b9400420 	ldr	w0, [x1, #4]
 4028960:	d65f03c0 	ret

0000000004028964 <ddrtrn_hal_get_cur_phy_dmc_num>:
 4028964:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 4028968:	97ffff23 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402896c:	aa0003f3 	mov	x19, x0
 4028970:	97ffff1c 	bl	40285e0 <ddrtrn_hal_get_phy>
 4028974:	d2800782 	mov	x2, #0x3c                  	// #60
 4028978:	b9402261 	ldr	w1, [x19, #32]
 402897c:	a8c17bf3 	ldp	x19, x30, [sp], #16
 4028980:	d503201f 	nop
 4028984:	9b020021 	madd	x1, x1, x2, x0
 4028988:	b9400820 	ldr	w0, [x1, #8]
 402898c:	d65f03c0 	ret

0000000004028990 <ddrtrn_hal_get_cur_dmc_addr>:
 4028990:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4028994:	2a0003f3 	mov	w19, w0
 4028998:	f9000bfe 	str	x30, [sp, #16]
 402899c:	97ffff16 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40289a0:	aa0003f4 	mov	x20, x0
 40289a4:	97ffff0f 	bl	40285e0 <ddrtrn_hal_get_phy>
 40289a8:	d2800782 	mov	x2, #0x3c                  	// #60
 40289ac:	f9400bfe 	ldr	x30, [sp, #16]
 40289b0:	b9402281 	ldr	w1, [x20, #32]
 40289b4:	9b027c21 	mul	x1, x1, x2
 40289b8:	d2800182 	mov	x2, #0xc                   	// #12
 40289bc:	9b020673 	madd	x19, x19, x2, x1
 40289c0:	8b130013 	add	x19, x0, x19
 40289c4:	b9401660 	ldr	w0, [x19, #20]
 40289c8:	a8c253f3 	ldp	x19, x20, [sp], #32
 40289cc:	d65f03c0 	ret

00000000040289d0 <ddrtrn_winding_identification>:
 40289d0:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 40289d4:	2a0103f4 	mov	w20, w1
 40289d8:	2a0003f3 	mov	w19, w0
 40289dc:	a9015bf5 	stp	x21, x22, [sp, #16]
 40289e0:	2a0203f5 	mov	w21, w2
 40289e4:	a90263f7 	stp	x23, x24, [sp, #32]
 40289e8:	52800017 	mov	w23, #0x0                   	// #0
 40289ec:	f9001bfe 	str	x30, [sp, #48]
 40289f0:	97ffffc7 	bl	402890c <ddrtrn_hal_get_cur_phy_rank_num>
 40289f4:	2a0003f6 	mov	w22, w0
 40289f8:	97ffffdb 	bl	4028964 <ddrtrn_hal_get_cur_phy_dmc_num>
 40289fc:	2a0003f8 	mov	w24, w0
 4028a00:	6b17031f 	cmp	w24, w23
 4028a04:	54000261 	b.ne	4028a50 <ddrtrn_winding_identification+0x80>  // b.any
 4028a08:	d2a80005 	mov	x5, #0x40000000            	// #1073741824
 4028a0c:	528acf07 	mov	w7, #0x5678                	// #22136
 4028a10:	529530e6 	mov	w6, #0xa987                	// #43399
 4028a14:	2a0503e8 	mov	w8, w5
 4028a18:	52800004 	mov	w4, #0x0                   	// #0
 4028a1c:	72a24687 	movk	w7, #0x1234, lsl #16
 4028a20:	72bdb966 	movk	w6, #0xedcb, lsl #16
 4028a24:	11000484 	add	w4, w4, #0x1
 4028a28:	b90000a7 	str	w7, [x5]
 4028a2c:	1b137c81 	mul	w1, w4, w19
 4028a30:	6b14003f 	cmp	w1, w20
 4028a34:	540002c3 	b.cc	4028a8c <ddrtrn_winding_identification+0xbc>  // b.lo, b.ul, b.last
 4028a38:	52a00800 	mov	w0, #0x400000              	// #4194304
 4028a3c:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4028a40:	a94263f7 	ldp	x23, x24, [sp, #32]
 4028a44:	f9401bfe 	ldr	x30, [sp, #48]
 4028a48:	a8c453f3 	ldp	x19, x20, [sp], #64
 4028a4c:	d65f03c0 	ret
 4028a50:	2a1703e0 	mov	w0, w23
 4028a54:	97ffffcf 	bl	4028990 <ddrtrn_hal_get_cur_dmc_addr>
 4028a58:	11014001 	add	w1, w0, #0x50
 4028a5c:	b9400023 	ldr	w3, [x1]
 4028a60:	710006df 	cmp	w22, #0x1
 4028a64:	540000a1 	b.ne	4028a78 <ddrtrn_winding_identification+0xa8>  // b.any
 4028a68:	120a7463 	and	w3, w3, #0xffcfffff
 4028a6c:	b9000023 	str	w3, [x1]
 4028a70:	110006f7 	add	w23, w23, #0x1
 4028a74:	17ffffe3 	b	4028a00 <ddrtrn_winding_identification+0x30>
 4028a78:	71000adf 	cmp	w22, #0x2
 4028a7c:	54ffffa1 	b.ne	4028a70 <ddrtrn_winding_identification+0xa0>  // b.any
 4028a80:	120a7463 	and	w3, w3, #0xffcfffff
 4028a84:	320c0063 	orr	w3, w3, #0x100000
 4028a88:	17fffff9 	b	4028a6c <ddrtrn_winding_identification+0x9c>
 4028a8c:	0b012902 	add	w2, w8, w1, lsl #10
 4028a90:	0b130023 	add	w3, w1, w19
 4028a94:	b9400040 	ldr	w0, [x2]
 4028a98:	4a070000 	eor	w0, w0, w7
 4028a9c:	6a15001f 	tst	w0, w21
 4028aa0:	54000080 	b.eq	4028ab0 <ddrtrn_winding_identification+0xe0>  // b.none
 4028aa4:	11000484 	add	w4, w4, #0x1
 4028aa8:	2a0303e1 	mov	w1, w3
 4028aac:	17ffffe1 	b	4028a30 <ddrtrn_winding_identification+0x60>
 4028ab0:	b90000a6 	str	w6, [x5]
 4028ab4:	b9400043 	ldr	w3, [x2]
 4028ab8:	4a060063 	eor	w3, w3, w6
 4028abc:	6a150060 	ands	w0, w3, w21
 4028ac0:	54000061 	b.ne	4028acc <ddrtrn_winding_identification+0xfc>  // b.any
 4028ac4:	7100049f 	cmp	w4, #0x1
 4028ac8:	54fffba0 	b.eq	4028a3c <ddrtrn_winding_identification+0x6c>  // b.none
 4028acc:	b9400040 	ldr	w0, [x2]
 4028ad0:	4a060000 	eor	w0, w0, w6
 4028ad4:	6a15001f 	tst	w0, w21
 4028ad8:	54fffa61 	b.ne	4028a24 <ddrtrn_winding_identification+0x54>  // b.any
 4028adc:	2a0103e0 	mov	w0, w1
 4028ae0:	17ffffd7 	b	4028a3c <ddrtrn_winding_identification+0x6c>

0000000004028ae4 <ddrtrn_capat_adapt_func>:
 4028ae4:	d10343ff 	sub	sp, sp, #0xd0
 4028ae8:	b0ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4028aec:	a90753f3 	stp	x19, x20, [sp, #112]
 4028af0:	d2800013 	mov	x19, #0x0                   	// #0
 4028af4:	a9085bf5 	stp	x21, x22, [sp, #128]
 4028af8:	9100c3f5 	add	x21, sp, #0x30
 4028afc:	a90963f7 	stp	x23, x24, [sp, #144]
 4028b00:	52974597 	mov	w23, #0xba2c                	// #47660
 4028b04:	72bddd77 	movk	w23, #0xeeeb, lsl #16
 4028b08:	a90a6bf9 	stp	x25, x26, [sp, #160]
 4028b0c:	1281eef8 	mov	w24, #0xfffff088            	// #-3960
 4028b10:	a90b73fb 	stp	x27, x28, [sp, #176]
 4028b14:	f90063fe 	str	x30, [sp, #192]
 4028b18:	f9455001 	ldr	x1, [x0, #2720]
 4028b1c:	f90037e1 	str	x1, [sp, #104]
 4028b20:	d2800001 	mov	x1, #0x0                   	// #0
 4028b24:	97fffec0 	bl	4028624 <ddrtrn_hal_cfg_init>
 4028b28:	97fffeb3 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028b2c:	b940001c 	ldr	w28, [x0]
 4028b30:	52800282 	mov	w2, #0x14                  	// #20
 4028b34:	9100c3e0 	add	x0, sp, #0x30
 4028b38:	52800001 	mov	w1, #0x0                   	// #0
 4028b3c:	94000772 	bl	402a904 <ddrtrn_set_data>
 4028b40:	6b13039f 	cmp	w28, w19
 4028b44:	54000268 	b.hi	4028b90 <ddrtrn_capat_adapt_func+0xac>  // b.pmore
 4028b48:	97ffff7c 	bl	4028938 <ddrtrn_hal_get_cur_phy_dram_type>
 4028b4c:	7100281f 	cmp	w0, #0xa
 4028b50:	54000061 	b.ne	4028b5c <ddrtrn_capat_adapt_func+0x78>  // b.any
 4028b54:	9100c3e0 	add	x0, sp, #0x30
 4028b58:	9400080d 	bl	402ab8c <ddrtrn_hal_timing8_trfc_ab_cfg>
 4028b5c:	97ffff77 	bl	4028938 <ddrtrn_hal_get_cur_phy_dram_type>
 4028b60:	7100181f 	cmp	w0, #0x6
 4028b64:	54001860 	b.eq	4028e70 <ddrtrn_capat_adapt_func+0x38c>  // b.none
 4028b68:	9100c3e0 	add	x0, sp, #0x30
 4028b6c:	940000f5 	bl	4028f40 <ddrtrn_capat_adpat_cfg_nonlpddr4>
 4028b70:	b0ffffc1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 4028b74:	b9403fe0 	ldr	w0, [sp, #60]
 4028b78:	f94037e2 	ldr	x2, [sp, #104]
 4028b7c:	f9455023 	ldr	x3, [x1, #2720]
 4028b80:	eb030042 	subs	x2, x2, x3
 4028b84:	d2800003 	mov	x3, #0x0                   	// #0
 4028b88:	54001800 	b.eq	4028e88 <ddrtrn_capat_adapt_func+0x3a4>  // b.none
 4028b8c:	97ffed78 	bl	402416c <__stack_chk_fail>
 4028b90:	97fffe99 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028b94:	b9002013 	str	w19, [x0, #32]
 4028b98:	97fffe92 	bl	40285e0 <ddrtrn_hal_get_phy>
 4028b9c:	2a1303e1 	mov	w1, w19
 4028ba0:	d2800782 	mov	x2, #0x3c                  	// #60
 4028ba4:	9b027c21 	mul	x1, x1, x2
 4028ba8:	b8616814 	ldr	w20, [x0, x1]
 4028bac:	97fffe92 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028bb0:	b9000414 	str	w20, [x0, #4]
 4028bb4:	97ffff56 	bl	402890c <ddrtrn_hal_get_cur_phy_rank_num>
 4028bb8:	9100c3e1 	add	x1, sp, #0x30
 4028bbc:	8b130821 	add	x1, x1, x19, lsl #2
 4028bc0:	b9001020 	str	w0, [x1, #16]
 4028bc4:	97fffe8c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028bc8:	b9402019 	ldr	w25, [x0, #32]
 4028bcc:	940000b7 	bl	4028ea8 <ddrtrn_chsel_remap_func>
 4028bd0:	d2a80000 	mov	x0, #0x40000000            	// #1073741824
 4028bd4:	b940001b 	ldr	w27, [x0]
 4028bd8:	97ffff58 	bl	4028938 <ddrtrn_hal_get_cur_phy_dram_type>
 4028bdc:	7100281f 	cmp	w0, #0xa
 4028be0:	54000741 	b.ne	4028cc8 <ddrtrn_capat_adapt_func+0x1e4>  // b.any
 4028be4:	97ffff60 	bl	4028964 <ddrtrn_hal_get_cur_phy_dmc_num>
 4028be8:	910123f4 	add	x20, sp, #0x48
 4028bec:	2a0003f6 	mov	w22, w0
 4028bf0:	97ffff47 	bl	402890c <ddrtrn_hal_get_cur_phy_rank_num>
 4028bf4:	5288c684 	mov	w4, #0x4634                	// #17972
 4028bf8:	2a0003e9 	mov	w9, w0
 4028bfc:	aa1403e8 	mov	x8, x20
 4028c00:	72a22284 	movk	w4, #0x1114, lsl #16
 4028c04:	5280001a 	mov	w26, #0x0                   	// #0
 4028c08:	6b1a013f 	cmp	w9, w26
 4028c0c:	54000500 	b.eq	4028cac <ddrtrn_capat_adapt_func+0x1c8>  // b.none
 4028c10:	0b17008b 	add	w11, w4, w23
 4028c14:	aa0803e5 	mov	x5, x8
 4028c18:	2a0403e7 	mov	w7, w4
 4028c1c:	52800003 	mov	w3, #0x0                   	// #0
 4028c20:	1400001d 	b	4028c94 <ddrtrn_capat_adapt_func+0x1b0>
 4028c24:	2a0303e0 	mov	w0, w3
 4028c28:	2901a7e3 	stp	w3, w9, [sp, #12]
 4028c2c:	b90017eb 	str	w11, [sp, #20]
 4028c30:	f9000fe5 	str	x5, [sp, #24]
 4028c34:	290413e7 	stp	w7, w4, [sp, #32]
 4028c38:	f90017e8 	str	x8, [sp, #40]
 4028c3c:	97ffff55 	bl	4028990 <ddrtrn_hal_get_cur_dmc_addr>
 4028c40:	294413e7 	ldp	w7, w4, [sp, #32]
 4028c44:	52814c4a 	mov	w10, #0xa62                 	// #2658
 4028c48:	b94017eb 	ldr	w11, [sp, #20]
 4028c4c:	f9400fe5 	ldr	x5, [sp, #24]
 4028c50:	0b0b0006 	add	w6, w0, w11
 4028c54:	2a0703e0 	mov	w0, w7
 4028c58:	110040e7 	add	w7, w7, #0x10
 4028c5c:	2941a7e3 	ldp	w3, w9, [sp, #12]
 4028c60:	910010a5 	add	x5, x5, #0x4
 4028c64:	b94000c2 	ldr	w2, [x6]
 4028c68:	b9400001 	ldr	w1, [x0]
 4028c6c:	11000463 	add	w3, w3, #0x1
 4028c70:	b81fc0a2 	stur	w2, [x5, #-4]
 4028c74:	0a180042 	and	w2, w2, w24
 4028c78:	f94017e8 	ldr	x8, [sp, #40]
 4028c7c:	2a0a0042 	orr	w2, w2, w10
 4028c80:	b90000c2 	str	w2, [x6]
 4028c84:	b90004a1 	str	w1, [x5, #4]
 4028c88:	0a180021 	and	w1, w1, w24
 4028c8c:	2a0a0021 	orr	w1, w1, w10
 4028c90:	b9000001 	str	w1, [x0]
 4028c94:	6b0302df 	cmp	w22, w3
 4028c98:	54fffc61 	b.ne	4028c24 <ddrtrn_capat_adapt_func+0x140>  // b.any
 4028c9c:	1100075a 	add	w26, w26, #0x1
 4028ca0:	11001084 	add	w4, w4, #0x4
 4028ca4:	91004108 	add	x8, x8, #0x10
 4028ca8:	17ffffd8 	b	4028c08 <ddrtrn_capat_adapt_func+0x124>
 4028cac:	12800002 	mov	w2, #0xffffffff            	// #-1
 4028cb0:	52800201 	mov	w1, #0x10                  	// #16
 4028cb4:	52800040 	mov	w0, #0x2                   	// #2
 4028cb8:	97ffff46 	bl	40289d0 <ddrtrn_winding_identification>
 4028cbc:	7100101f 	cmp	w0, #0x4
 4028cc0:	7a481804 	ccmp	w0, #0x8, #0x4, ne  // ne = any
 4028cc4:	54000600 	b.eq	4028d84 <ddrtrn_capat_adapt_func+0x2a0>  // b.none
 4028cc8:	97ffff27 	bl	4028964 <ddrtrn_hal_get_cur_phy_dmc_num>
 4028ccc:	b9000fe0 	str	w0, [sp, #12]
 4028cd0:	97ffff1a 	bl	4028938 <ddrtrn_hal_get_cur_phy_dram_type>
 4028cd4:	7100181f 	cmp	w0, #0x6
 4028cd8:	54000be1 	b.ne	4028e54 <ddrtrn_capat_adapt_func+0x370>  // b.any
 4028cdc:	52a00801 	mov	w1, #0x400000              	// #4194304
 4028ce0:	12800002 	mov	w2, #0xffffffff            	// #-1
 4028ce4:	52a00040 	mov	w0, #0x20000               	// #131072
 4028ce8:	97ffff3a 	bl	40289d0 <ddrtrn_winding_identification>
 4028cec:	2a1903f9 	mov	w25, w25
 4028cf0:	9101c3e1 	add	x1, sp, #0x70
 4028cf4:	d2800183 	mov	x3, #0xc                   	// #12
 4028cf8:	530a7c00 	lsr	w0, w0, #10
 4028cfc:	9b030739 	madd	x25, x25, x3, x1
 4028d00:	b81c8320 	stur	w0, [x25, #-56]
 4028d04:	14000042 	b	4028e0c <ddrtrn_capat_adapt_func+0x328>
 4028d08:	2a0103e0 	mov	w0, w1
 4028d0c:	290197e1 	stp	w1, w5, [sp, #12]
 4028d10:	b90017e6 	str	w6, [sp, #20]
 4028d14:	f9000fe2 	str	x2, [sp, #24]
 4028d18:	29040fe4 	stp	w4, w3, [sp, #32]
 4028d1c:	97ffff1d 	bl	4028990 <ddrtrn_hal_get_cur_dmc_addr>
 4028d20:	b94017e6 	ldr	w6, [sp, #20]
 4028d24:	f9400fe2 	ldr	x2, [sp, #24]
 4028d28:	0b060000 	add	w0, w0, w6
 4028d2c:	29440fe4 	ldp	w4, w3, [sp, #32]
 4028d30:	91001042 	add	x2, x2, #0x4
 4028d34:	b85fc047 	ldur	w7, [x2, #-4]
 4028d38:	294197e1 	ldp	w1, w5, [sp, #12]
 4028d3c:	b9000007 	str	w7, [x0]
 4028d40:	2a0403e0 	mov	w0, w4
 4028d44:	b9400447 	ldr	w7, [x2, #4]
 4028d48:	11000421 	add	w1, w1, #0x1
 4028d4c:	11004084 	add	w4, w4, #0x10
 4028d50:	b9000007 	str	w7, [x0]
 4028d54:	6b0102df 	cmp	w22, w1
 4028d58:	54fffd81 	b.ne	4028d08 <ddrtrn_capat_adapt_func+0x224>  // b.any
 4028d5c:	110004a5 	add	w5, w5, #0x1
 4028d60:	91004294 	add	x20, x20, #0x10
 4028d64:	11001063 	add	w3, w3, #0x4
 4028d68:	6b05035f 	cmp	w26, w5
 4028d6c:	54fffae0 	b.eq	4028cc8 <ddrtrn_capat_adapt_func+0x1e4>  // b.none
 4028d70:	0b170066 	add	w6, w3, w23
 4028d74:	2a0303e4 	mov	w4, w3
 4028d78:	aa1403e2 	mov	x2, x20
 4028d7c:	52800001 	mov	w1, #0x0                   	// #0
 4028d80:	17fffff5 	b	4028d54 <ddrtrn_capat_adapt_func+0x270>
 4028d84:	5288c683 	mov	w3, #0x4634                	// #17972
 4028d88:	52800005 	mov	w5, #0x0                   	// #0
 4028d8c:	72a22283 	movk	w3, #0x1114, lsl #16
 4028d90:	17fffff6 	b	4028d68 <ddrtrn_capat_adapt_func+0x284>
 4028d94:	2a1403e0 	mov	w0, w20
 4028d98:	97fffefe 	bl	4028990 <ddrtrn_hal_get_cur_dmc_addr>
 4028d9c:	b90013e0 	str	w0, [sp, #16]
 4028da0:	97fffe15 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028da4:	b94013e1 	ldr	w1, [sp, #16]
 4028da8:	b9000801 	str	w1, [x0, #8]
 4028dac:	94000227 	bl	4029648 <ddrtrn_hal_ddrt_get_mem_width>
 4028db0:	7100081f 	cmp	w0, #0x2
 4028db4:	540003e1 	b.ne	4028e30 <ddrtrn_capat_adapt_func+0x34c>  // b.any
 4028db8:	529fffe2 	mov	w2, #0xffff                	// #65535
 4028dbc:	52a00801 	mov	w1, #0x400000              	// #4194304
 4028dc0:	52a00040 	mov	w0, #0x20000               	// #131072
 4028dc4:	97ffff03 	bl	40289d0 <ddrtrn_winding_identification>
 4028dc8:	9100c3e1 	add	x1, sp, #0x30
 4028dcc:	530b7c00 	lsr	w0, w0, #11
 4028dd0:	52bfffe2 	mov	w2, #0xffff0000            	// #-65536
 4028dd4:	b8366820 	str	w0, [x1, x22]
 4028dd8:	52a00801 	mov	w1, #0x400000              	// #4194304
 4028ddc:	52a00040 	mov	w0, #0x20000               	// #131072
 4028de0:	97fffefc 	bl	40289d0 <ddrtrn_winding_identification>
 4028de4:	530b7c00 	lsr	w0, w0, #11
 4028de8:	9100c3e1 	add	x1, sp, #0x30
 4028dec:	b8366b20 	str	w0, [x25, x22]
 4028df0:	b8766821 	ldr	w1, [x1, x22]
 4028df4:	0b000020 	add	w0, w1, w0
 4028df8:	11000694 	add	w20, w20, #0x1
 4028dfc:	b8366b40 	str	w0, [x26, x22]
 4028e00:	b9400fe0 	ldr	w0, [sp, #12]
 4028e04:	6b14001f 	cmp	w0, w20
 4028e08:	54fffc61 	b.ne	4028d94 <ddrtrn_capat_adapt_func+0x2b0>  // b.any
 4028e0c:	d2a80000 	mov	x0, #0x40000000            	// #1073741824
 4028e10:	91000673 	add	x19, x19, #0x1
 4028e14:	910032b5 	add	x21, x21, #0xc
 4028e18:	b900001b 	str	w27, [x0]
 4028e1c:	b85fc2a1 	ldur	w1, [x21, #-4]
 4028e20:	b9403fe0 	ldr	w0, [sp, #60]
 4028e24:	0b010000 	add	w0, w0, w1
 4028e28:	b9003fe0 	str	w0, [sp, #60]
 4028e2c:	17ffff45 	b	4028b40 <ddrtrn_capat_adapt_func+0x5c>
 4028e30:	9100c3e0 	add	x0, sp, #0x30
 4028e34:	b8366b3f 	str	wzr, [x25, x22]
 4028e38:	12800002 	mov	w2, #0xffffffff            	// #-1
 4028e3c:	52a00801 	mov	w1, #0x400000              	// #4194304
 4028e40:	b836681f 	str	wzr, [x0, x22]
 4028e44:	52a00040 	mov	w0, #0x20000               	// #131072
 4028e48:	97fffee2 	bl	40289d0 <ddrtrn_winding_identification>
 4028e4c:	530a7c00 	lsr	w0, w0, #10
 4028e50:	17ffffea 	b	4028df8 <ddrtrn_capat_adapt_func+0x314>
 4028e54:	2a1903e3 	mov	w3, w25
 4028e58:	d2800180 	mov	x0, #0xc                   	// #12
 4028e5c:	9100d3f9 	add	x25, sp, #0x34
 4028e60:	9100e3fa 	add	x26, sp, #0x38
 4028e64:	52800014 	mov	w20, #0x0                   	// #0
 4028e68:	9b007c76 	mul	x22, x3, x0
 4028e6c:	17ffffe5 	b	4028e00 <ddrtrn_capat_adapt_func+0x31c>
 4028e70:	97fffeb2 	bl	4028938 <ddrtrn_hal_get_cur_phy_dram_type>
 4028e74:	7100181f 	cmp	w0, #0x6
 4028e78:	54ffe7c1 	b.ne	4028b70 <ddrtrn_capat_adapt_func+0x8c>  // b.any
 4028e7c:	9100c3e0 	add	x0, sp, #0x30
 4028e80:	94000052 	bl	4028fc8 <ddrtrn_capat_adpat_cfg_lpddr4>
 4028e84:	17ffff3b 	b	4028b70 <ddrtrn_capat_adapt_func+0x8c>
 4028e88:	a94753f3 	ldp	x19, x20, [sp, #112]
 4028e8c:	a9485bf5 	ldp	x21, x22, [sp, #128]
 4028e90:	a94963f7 	ldp	x23, x24, [sp, #144]
 4028e94:	a94a6bf9 	ldp	x25, x26, [sp, #160]
 4028e98:	a94b73fb 	ldp	x27, x28, [sp, #176]
 4028e9c:	f94063fe 	ldr	x30, [sp, #192]
 4028ea0:	910343ff 	add	sp, sp, #0xd0
 4028ea4:	d65f03c0 	ret

0000000004028ea8 <ddrtrn_chsel_remap_func>:
 4028ea8:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 4028eac:	97fffdd2 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028eb0:	b9402013 	ldr	w19, [x0, #32]
 4028eb4:	9400015d 	bl	4029428 <ddrtrn_hal_axi_special_intlv_en>
 4028eb8:	35000233 	cbnz	w19, 4028efc <ddrtrn_chsel_remap_func+0x54>
 4028ebc:	97fffdce 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028ec0:	aa0003f3 	mov	x19, x0
 4028ec4:	97fffdc7 	bl	40285e0 <ddrtrn_hal_get_phy>
 4028ec8:	d2800782 	mov	x2, #0x3c                  	// #60
 4028ecc:	b9402261 	ldr	w1, [x19, #32]
 4028ed0:	9b020021 	madd	x1, x1, x2, x0
 4028ed4:	f0000020 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4028ed8:	910be000 	add	x0, x0, #0x2f8
 4028edc:	b9400422 	ldr	w2, [x1, #4]
 4028ee0:	91014001 	add	x1, x0, #0x50
 4028ee4:	9100a000 	add	x0, x0, #0x28
 4028ee8:	7100185f 	cmp	w2, #0x6
 4028eec:	9a810000 	csel	x0, x0, x1, eq  // eq = none
 4028ef0:	a8c17bf3 	ldp	x19, x30, [sp], #16
 4028ef4:	528000a1 	mov	w1, #0x5                   	// #5
 4028ef8:	14000bb5 	b	402bdcc <ddrtrn_reg_config>
 4028efc:	7100067f 	cmp	w19, #0x1
 4028f00:	540001c1 	b.ne	4028f38 <ddrtrn_chsel_remap_func+0x90>  // b.any
 4028f04:	97fffdbc 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4028f08:	aa0003f3 	mov	x19, x0
 4028f0c:	97fffdb5 	bl	40285e0 <ddrtrn_hal_get_phy>
 4028f10:	d2800782 	mov	x2, #0x3c                  	// #60
 4028f14:	b9402261 	ldr	w1, [x19, #32]
 4028f18:	9b020021 	madd	x1, x1, x2, x0
 4028f1c:	f0000020 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4028f20:	b9400422 	ldr	w2, [x1, #4]
 4028f24:	910be001 	add	x1, x0, #0x2f8
 4028f28:	9101e020 	add	x0, x1, #0x78
 4028f2c:	7100185f 	cmp	w2, #0x6
 4028f30:	9a811000 	csel	x0, x0, x1, ne  // ne = any
 4028f34:	17ffffef 	b	4028ef0 <ddrtrn_chsel_remap_func+0x48>
 4028f38:	d2800000 	mov	x0, #0x0                   	// #0
 4028f3c:	17ffffed 	b	4028ef0 <ddrtrn_chsel_remap_func+0x48>

0000000004028f40 <ddrtrn_capat_adpat_cfg_nonlpddr4>:
 4028f40:	b9400401 	ldr	w1, [x0, #4]
 4028f44:	350000c1 	cbnz	w1, 4028f5c <ddrtrn_capat_adpat_cfg_nonlpddr4+0x1c>
 4028f48:	f0000020 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4028f4c:	910be000 	add	x0, x0, #0x2f8
 4028f50:	91028000 	add	x0, x0, #0xa0
 4028f54:	52800081 	mov	w1, #0x4                   	// #4
 4028f58:	14000b9d 	b	402bdcc <ddrtrn_reg_config>
 4028f5c:	b9400000 	ldr	w0, [x0]
 4028f60:	7110001f 	cmp	w0, #0x400
 4028f64:	54000101 	b.ne	4028f84 <ddrtrn_capat_adpat_cfg_nonlpddr4+0x44>  // b.any
 4028f68:	7108003f 	cmp	w1, #0x200
 4028f6c:	540001e1 	b.ne	4028fa8 <ddrtrn_capat_adpat_cfg_nonlpddr4+0x68>  // b.any
 4028f70:	f0000020 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4028f74:	910be000 	add	x0, x0, #0x2f8
 4028f78:	91030000 	add	x0, x0, #0xc0
 4028f7c:	52800121 	mov	w1, #0x9                   	// #9
 4028f80:	17fffff6 	b	4028f58 <ddrtrn_capat_adpat_cfg_nonlpddr4+0x18>
 4028f84:	7120001f 	cmp	w0, #0x800
 4028f88:	54000101 	b.ne	4028fa8 <ddrtrn_capat_adpat_cfg_nonlpddr4+0x68>  // b.any
 4028f8c:	7110003f 	cmp	w1, #0x400
 4028f90:	540000c1 	b.ne	4028fa8 <ddrtrn_capat_adpat_cfg_nonlpddr4+0x68>  // b.any
 4028f94:	f0000020 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4028f98:	910be000 	add	x0, x0, #0x2f8
 4028f9c:	91042000 	add	x0, x0, #0x108
 4028fa0:	52800121 	mov	w1, #0x9                   	// #9
 4028fa4:	17ffffed 	b	4028f58 <ddrtrn_capat_adpat_cfg_nonlpddr4+0x18>
 4028fa8:	6b00003f 	cmp	w1, w0
 4028fac:	540000c1 	b.ne	4028fc4 <ddrtrn_capat_adpat_cfg_nonlpddr4+0x84>  // b.any
 4028fb0:	f0000020 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4028fb4:	910be000 	add	x0, x0, #0x2f8
 4028fb8:	91054000 	add	x0, x0, #0x150
 4028fbc:	52800081 	mov	w1, #0x4                   	// #4
 4028fc0:	17ffffe6 	b	4028f58 <ddrtrn_capat_adpat_cfg_nonlpddr4+0x18>
 4028fc4:	d65f03c0 	ret

0000000004028fc8 <ddrtrn_capat_adpat_cfg_lpddr4>:
 4028fc8:	b9401001 	ldr	w1, [x0, #16]
 4028fcc:	7100043f 	cmp	w1, #0x1
 4028fd0:	540000c1 	b.ne	4028fe8 <ddrtrn_capat_adpat_cfg_lpddr4+0x20>  // b.any
 4028fd4:	f0000020 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4028fd8:	910be000 	add	x0, x0, #0x2f8
 4028fdc:	9105c000 	add	x0, x0, #0x170
 4028fe0:	52800101 	mov	w1, #0x8                   	// #8
 4028fe4:	14000b7a 	b	402bdcc <ddrtrn_reg_config>
 4028fe8:	7100083f 	cmp	w1, #0x2
 4028fec:	54000201 	b.ne	402902c <ddrtrn_capat_adpat_cfg_lpddr4+0x64>  // b.any
 4028ff0:	b9400800 	ldr	w0, [x0, #8]
 4028ff4:	7120001f 	cmp	w0, #0x800
 4028ff8:	540000c1 	b.ne	4029010 <ddrtrn_capat_adpat_cfg_lpddr4+0x48>  // b.any
 4028ffc:	f0000020 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 4029000:	910be000 	add	x0, x0, #0x2f8
 4029004:	9106c000 	add	x0, x0, #0x1b0
 4029008:	52800181 	mov	w1, #0xc                   	// #12
 402900c:	17fffff6 	b	4028fe4 <ddrtrn_capat_adpat_cfg_lpddr4+0x1c>
 4029010:	7140041f 	cmp	w0, #0x1, lsl #12
 4029014:	540000c1 	b.ne	402902c <ddrtrn_capat_adpat_cfg_lpddr4+0x64>  // b.any
 4029018:	d0000020 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 402901c:	910be000 	add	x0, x0, #0x2f8
 4029020:	91084000 	add	x0, x0, #0x210
 4029024:	52800181 	mov	w1, #0xc                   	// #12
 4029028:	17ffffef 	b	4028fe4 <ddrtrn_capat_adpat_cfg_lpddr4+0x1c>
 402902c:	d65f03c0 	ret

0000000004029030 <ddrtrn_hal_get_cur_dmc_addr>:
 4029030:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4029034:	2a0003f3 	mov	w19, w0
 4029038:	f9000bfe 	str	x30, [sp, #16]
 402903c:	97fffd6e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029040:	aa0003f4 	mov	x20, x0
 4029044:	97fffd67 	bl	40285e0 <ddrtrn_hal_get_phy>
 4029048:	d2800782 	mov	x2, #0x3c                  	// #60
 402904c:	f9400bfe 	ldr	x30, [sp, #16]
 4029050:	b9402281 	ldr	w1, [x20, #32]
 4029054:	9b027c21 	mul	x1, x1, x2
 4029058:	d2800182 	mov	x2, #0xc                   	// #12
 402905c:	9b020673 	madd	x19, x19, x2, x1
 4029060:	8b130013 	add	x19, x0, x19
 4029064:	b9401660 	ldr	w0, [x19, #20]
 4029068:	a8c253f3 	ldp	x19, x20, [sp], #32
 402906c:	d65f03c0 	ret

0000000004029070 <ddrtrn_hal_set_timing>:
 4029070:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4029074:	2a0003f3 	mov	w19, w0
 4029078:	11042273 	add	w19, w19, #0x108
 402907c:	2a0103f4 	mov	w20, w1
 4029080:	52807d00 	mov	w0, #0x3e8                 	// #1000
 4029084:	f9000bfe 	str	x30, [sp, #16]
 4029088:	9400046b 	bl	402a234 <ddrtrn_hal_training_delay>
 402908c:	f9400bfe 	ldr	x30, [sp, #16]
 4029090:	b9000274 	str	w20, [x19]
 4029094:	52807d00 	mov	w0, #0x3e8                 	// #1000
 4029098:	a8c253f3 	ldp	x19, x20, [sp], #32
 402909c:	14000466 	b	402a234 <ddrtrn_hal_training_delay>

00000000040290a0 <ddrtrn_hal_ddrc_get_bank_group>:
 40290a0:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 40290a4:	97fffd54 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40290a8:	b9400813 	ldr	w19, [x0, #8]
 40290ac:	97fffd52 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40290b0:	b9402400 	ldr	w0, [x0, #36]
 40290b4:	11018273 	add	w19, w19, #0x60
 40290b8:	0b000a60 	add	w0, w19, w0, lsl #2
 40290bc:	a8c17bf3 	ldp	x19, x30, [sp], #16
 40290c0:	b9400000 	ldr	w0, [x0]
 40290c4:	d34a2c00 	ubfx	x0, x0, #10, #2
 40290c8:	d65f03c0 	ret

00000000040290cc <ddrtrn_hal_save_reg>:
 40290cc:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 40290d0:	aa0003f3 	mov	x19, x0
 40290d4:	a9015bf5 	stp	x21, x22, [sp, #16]
 40290d8:	2a0103f5 	mov	w21, w1
 40290dc:	a90263f7 	stp	x23, x24, [sp, #32]
 40290e0:	a9037bf9 	stp	x25, x30, [sp, #48]
 40290e4:	97fffd44 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40290e8:	b9400816 	ldr	w22, [x0, #8]
 40290ec:	97fffd42 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40290f0:	b9400414 	ldr	w20, [x0, #4]
 40290f4:	110422c0 	add	w0, w22, #0x108
 40290f8:	52820c81 	mov	w1, #0x1064                	// #4196
 40290fc:	1100a2d9 	add	w25, w22, #0x28
 4029100:	0b010297 	add	w23, w20, w1
 4029104:	1101c298 	add	w24, w20, #0x70
 4029108:	b9400001 	ldr	w1, [x0]
 402910c:	b9000261 	str	w1, [x19]
 4029110:	b9400320 	ldr	w0, [x25]
 4029114:	b9000660 	str	w0, [x19, #4]
 4029118:	b9400300 	ldr	w0, [x24]
 402911c:	b9001260 	str	w0, [x19, #16]
 4029120:	b94002e0 	ldr	w0, [x23]
 4029124:	b9001660 	str	w0, [x19, #20]
 4029128:	b94002e0 	ldr	w0, [x23]
 402912c:	b9001660 	str	w0, [x19, #20]
 4029130:	714402bf 	cmp	w21, #0x100, lsl #12
 4029134:	540004a0 	b.eq	40291c8 <ddrtrn_hal_save_reg+0xfc>  // b.none
 4029138:	54000368 	b.hi	40291a4 <ddrtrn_hal_save_reg+0xd8>  // b.pmore
 402913c:	710042bf 	cmp	w21, #0x10
 4029140:	54000360 	b.eq	40291ac <ddrtrn_hal_save_reg+0xe0>  // b.none
 4029144:	710402bf 	cmp	w21, #0x100
 4029148:	540003a0 	b.eq	40291bc <ddrtrn_hal_save_reg+0xf0>  // b.none
 402914c:	b9400660 	ldr	w0, [x19, #4]
 4029150:	2a1403e1 	mov	w1, w20
 4029154:	121f7800 	and	w0, w0, #0xfffffffe
 4029158:	b9000320 	str	w0, [x25]
 402915c:	b9401260 	ldr	w0, [x19, #16]
 4029160:	120f7800 	and	w0, w0, #0xfffeffff
 4029164:	b9000300 	str	w0, [x24]
 4029168:	91006260 	add	x0, x19, #0x18
 402916c:	94000582 	bl	402a774 <ddrtrn_hal_dqsswap_save_func>
 4029170:	aa1303e0 	mov	x0, x19
 4029174:	940000c3 	bl	4029480 <ddrtrn_hal_axi_save_func>
 4029178:	2a1603e1 	mov	w1, w22
 402917c:	aa1303e0 	mov	x0, x19
 4029180:	94000114 	bl	40295d0 <ddrtrn_hal_rnkvol_save_func>
 4029184:	2a1403e0 	mov	w0, w20
 4029188:	940001b3 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 402918c:	d5033f9f 	dsb	sy
 4029190:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4029194:	a94263f7 	ldp	x23, x24, [sp, #32]
 4029198:	a9437bf9 	ldp	x25, x30, [sp, #48]
 402919c:	a8c453f3 	ldp	x19, x20, [sp], #64
 40291a0:	d65f03c0 	ret
 40291a4:	716002bf 	cmp	w21, #0x800, lsl #12
 40291a8:	54fffd21 	b.ne	402914c <ddrtrn_hal_save_reg+0x80>  // b.any
 40291ac:	12144c21 	and	w1, w1, #0xfffff000
 40291b0:	2a1603e0 	mov	w0, w22
 40291b4:	97ffffaf 	bl	4029070 <ddrtrn_hal_set_timing>
 40291b8:	17ffffe5 	b	402914c <ddrtrn_hal_save_reg+0x80>
 40291bc:	12144c21 	and	w1, w1, #0xfffff000
 40291c0:	2a1603e0 	mov	w0, w22
 40291c4:	97ffffab 	bl	4029070 <ddrtrn_hal_set_timing>
 40291c8:	1100b280 	add	w0, w20, #0x2c
 40291cc:	b9400000 	ldr	w0, [x0]
 40291d0:	372ffbe0 	tbnz	w0, #5, 402914c <ddrtrn_hal_save_reg+0x80>
 40291d4:	b90002ff 	str	wzr, [x23]
 40291d8:	17ffffdd 	b	402914c <ddrtrn_hal_save_reg+0x80>

00000000040291dc <ddrtrn_hal_restore_reg>:
 40291dc:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 40291e0:	aa0003f3 	mov	x19, x0
 40291e4:	a9017bf5 	stp	x21, x30, [sp, #16]
 40291e8:	97fffd03 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40291ec:	b9400815 	ldr	w21, [x0, #8]
 40291f0:	97fffd01 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40291f4:	b9400414 	ldr	w20, [x0, #4]
 40291f8:	b9400261 	ldr	w1, [x19]
 40291fc:	2a1503e0 	mov	w0, w21
 4029200:	97ffff9c 	bl	4029070 <ddrtrn_hal_set_timing>
 4029204:	1100a2a0 	add	w0, w21, #0x28
 4029208:	b9400661 	ldr	w1, [x19, #4]
 402920c:	b9000001 	str	w1, [x0]
 4029210:	1101c280 	add	w0, w20, #0x70
 4029214:	b9401261 	ldr	w1, [x19, #16]
 4029218:	b9000001 	str	w1, [x0]
 402921c:	1100b280 	add	w0, w20, #0x2c
 4029220:	b9400000 	ldr	w0, [x0]
 4029224:	372800a0 	tbnz	w0, #5, 4029238 <ddrtrn_hal_restore_reg+0x5c>
 4029228:	52820c80 	mov	w0, #0x1064                	// #4196
 402922c:	0b000280 	add	w0, w20, w0
 4029230:	b9401661 	ldr	w1, [x19, #20]
 4029234:	b9000001 	str	w1, [x0]
 4029238:	b9401a60 	ldr	w0, [x19, #24]
 402923c:	2a1403e1 	mov	w1, w20
 4029240:	94000552 	bl	402a788 <ddrtrn_hal_dqsswap_restore_func>
 4029244:	aa1303e0 	mov	x0, x19
 4029248:	9400009b 	bl	40294b4 <ddrtrn_hal_axi_restore_func>
 402924c:	2a1503e1 	mov	w1, w21
 4029250:	aa1303e0 	mov	x0, x19
 4029254:	940000e3 	bl	40295e0 <ddrtrn_hal_rnkvol_restore_func>
 4029258:	2a1403e0 	mov	w0, w20
 402925c:	9400017e 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 4029260:	d5033f9f 	dsb	sy
 4029264:	a9417bf5 	ldp	x21, x30, [sp, #16]
 4029268:	a8c253f3 	ldp	x19, x20, [sp], #32
 402926c:	d65f03c0 	ret

0000000004029270 <ddrtrn_hal_training_switch_axi>:
 4029270:	f81f0ffe 	str	x30, [sp, #-16]!
 4029274:	9400009b 	bl	40294e0 <ddrtrn_hal_axi_chsel_remap_func>
 4029278:	940000b4 	bl	4029548 <ddrtrn_hal_axi_switch_func>
 402927c:	f84107fe 	ldr	x30, [sp], #16
 4029280:	140000dc 	b	40295f0 <ddrtrn_hal_rnkvol_set_func>

0000000004029284 <ddrtrn_hal_ddrc_easr>:
 4029284:	7100083f 	cmp	w1, #0x2
 4029288:	540002a1 	b.ne	40292dc <ddrtrn_hal_ddrc_easr+0x58>  // b.any
 402928c:	2a0003e2 	mov	w2, w0
 4029290:	110a5000 	add	w0, w0, #0x294
 4029294:	b9000041 	str	w1, [x2]
 4029298:	52800001 	mov	w1, #0x0                   	// #0
 402929c:	b9400002 	ldr	w2, [x0]
 40292a0:	11000421 	add	w1, w1, #0x1
 40292a4:	37000062 	tbnz	w2, #0, 40292b0 <ddrtrn_hal_ddrc_easr+0x2c>
 40292a8:	52800000 	mov	w0, #0x0                   	// #0
 40292ac:	d65f03c0 	ret
 40292b0:	3100043f 	cmn	w1, #0x1
 40292b4:	54ffff41 	b.ne	402929c <ddrtrn_hal_ddrc_easr+0x18>  // b.any
 40292b8:	f81f0ffe 	str	x30, [sp, #-16]!
 40292bc:	12800003 	mov	w3, #0xffffffff            	// #-1
 40292c0:	52800200 	mov	w0, #0x10                  	// #16
 40292c4:	2a0303e2 	mov	w2, w3
 40292c8:	2a0303e1 	mov	w1, w3
 40292cc:	940003df 	bl	402a248 <ddrtrn_hal_training_stat>
 40292d0:	12800000 	mov	w0, #0xffffffff            	// #-1
 40292d4:	f84107fe 	ldr	x30, [sp], #16
 40292d8:	d65f03c0 	ret
 40292dc:	7100043f 	cmp	w1, #0x1
 40292e0:	54fffec1 	b.ne	40292b8 <ddrtrn_hal_ddrc_easr+0x34>  // b.any
 40292e4:	2a0003e2 	mov	w2, w0
 40292e8:	110a5000 	add	w0, w0, #0x294
 40292ec:	b9000041 	str	w1, [x2]
 40292f0:	52800001 	mov	w1, #0x0                   	// #0
 40292f4:	b9400002 	ldr	w2, [x0]
 40292f8:	11000421 	add	w1, w1, #0x1
 40292fc:	3707fd62 	tbnz	w2, #0, 40292a8 <ddrtrn_hal_ddrc_easr+0x24>
 4029300:	3100043f 	cmn	w1, #0x1
 4029304:	54ffff81 	b.ne	40292f4 <ddrtrn_hal_ddrc_easr+0x70>  // b.any
 4029308:	17ffffec 	b	40292b8 <ddrtrn_hal_ddrc_easr+0x34>

000000000402930c <ddrtrn_hal_save_timing>:
 402930c:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 4029310:	aa0003f4 	mov	x20, x0
 4029314:	52800013 	mov	w19, #0x0                   	// #0
 4029318:	a9015bf5 	stp	x21, x22, [sp, #16]
 402931c:	d2800796 	mov	x22, #0x3c                  	// #60
 4029320:	f90013fe 	str	x30, [sp, #32]
 4029324:	97fffcb4 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029328:	aa0003f5 	mov	x21, x0
 402932c:	97fffcad 	bl	40285e0 <ddrtrn_hal_get_phy>
 4029330:	b94022a1 	ldr	w1, [x21, #32]
 4029334:	9b160021 	madd	x1, x1, x22, x0
 4029338:	b9400820 	ldr	w0, [x1, #8]
 402933c:	6b00027f 	cmp	w19, w0
 4029340:	540000a3 	b.cc	4029354 <ddrtrn_hal_save_timing+0x48>  // b.lo, b.ul, b.last
 4029344:	a9415bf5 	ldp	x21, x22, [sp, #16]
 4029348:	f94013fe 	ldr	x30, [sp, #32]
 402934c:	a8c353f3 	ldp	x19, x20, [sp], #48
 4029350:	d65f03c0 	ret
 4029354:	2a1303e0 	mov	w0, w19
 4029358:	97ffff36 	bl	4029030 <ddrtrn_hal_get_cur_dmc_addr>
 402935c:	11042000 	add	w0, w0, #0x108
 4029360:	d37e7e75 	ubfiz	x21, x19, #2, #32
 4029364:	b9400000 	ldr	w0, [x0]
 4029368:	b8356a80 	str	w0, [x20, x21]
 402936c:	2a1303e0 	mov	w0, w19
 4029370:	11000673 	add	w19, w19, #0x1
 4029374:	97ffff2f 	bl	4029030 <ddrtrn_hal_get_cur_dmc_addr>
 4029378:	b8756a81 	ldr	w1, [x20, x21]
 402937c:	12144c21 	and	w1, w1, #0xfffff000
 4029380:	97ffff3c 	bl	4029070 <ddrtrn_hal_set_timing>
 4029384:	17ffffe8 	b	4029324 <ddrtrn_hal_save_timing+0x18>

0000000004029388 <ddrtrn_hal_dmc_get_sref_cfg>:
 4029388:	f81f0ffe 	str	x30, [sp, #-16]!
 402938c:	97ffff29 	bl	4029030 <ddrtrn_hal_get_cur_dmc_addr>
 4029390:	11008000 	add	w0, w0, #0x20
 4029394:	f84107fe 	ldr	x30, [sp], #16
 4029398:	b9400000 	ldr	w0, [x0]
 402939c:	d65f03c0 	ret

00000000040293a0 <ddrtrn_hal_dmc_set_sref_cfg>:
 40293a0:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 40293a4:	2a0103f3 	mov	w19, w1
 40293a8:	97ffff22 	bl	4029030 <ddrtrn_hal_get_cur_dmc_addr>
 40293ac:	11008000 	add	w0, w0, #0x20
 40293b0:	b9000013 	str	w19, [x0]
 40293b4:	a8c17bf3 	ldp	x19, x30, [sp], #16
 40293b8:	d65f03c0 	ret

00000000040293bc <ddrtrn_hal_phy_get_byte_num>:
 40293bc:	11014000 	add	w0, w0, #0x50
 40293c0:	52800081 	mov	w1, #0x4                   	// #4
 40293c4:	b9400000 	ldr	w0, [x0]
 40293c8:	53047c00 	lsr	w0, w0, #4
 40293cc:	531f0400 	ubfiz	w0, w0, #1, #2
 40293d0:	7100101f 	cmp	w0, #0x4
 40293d4:	1a819000 	csel	w0, w0, w1, ls  // ls = plast
 40293d8:	d65f03c0 	ret

00000000040293dc <ddrtrn_hal_get_rank_size>:
 40293dc:	f81f0ffe 	str	x30, [sp, #-16]!
 40293e0:	97fffc85 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40293e4:	b9400801 	ldr	w1, [x0, #8]
 40293e8:	f84107fe 	ldr	x30, [sp], #16
 40293ec:	11014020 	add	w0, w1, #0x50
 40293f0:	11018021 	add	w1, w1, #0x60
 40293f4:	b9400000 	ldr	w0, [x0]
 40293f8:	b9400021 	ldr	w1, [x1]
 40293fc:	d3441400 	ubfx	x0, x0, #4, #2
 4029400:	d3482422 	ubfx	x2, x1, #8, #2
 4029404:	0b020000 	add	w0, w0, w2
 4029408:	12000822 	and	w2, w1, #0x7
 402940c:	11005442 	add	w2, w2, #0x15
 4029410:	d3441821 	ubfx	x1, x1, #4, #3
 4029414:	0b020000 	add	w0, w0, w2
 4029418:	0b010000 	add	w0, w0, w1
 402941c:	d2800021 	mov	x1, #0x1                   	// #1
 4029420:	9ac02020 	lsl	x0, x1, x0
 4029424:	d65f03c0 	ret

0000000004029428 <ddrtrn_hal_axi_special_intlv_en>:
 4029428:	d2800401 	mov	x1, #0x20                  	// #32
 402942c:	f2a22281 	movk	x1, #0x1114, lsl #16
 4029430:	b9400020 	ldr	w0, [x1]
 4029434:	321d0000 	orr	w0, w0, #0x8
 4029438:	b9000020 	str	w0, [x1]
 402943c:	d65f03c0 	ret

0000000004029440 <ddrtrn_hal_timing8_trfc_ab_cfg_by_dmc>:
 4029440:	f81f0ffe 	str	x30, [sp, #-16]!
 4029444:	97fffc6c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029448:	b9400801 	ldr	w1, [x0, #8]
 402944c:	52802bc3 	mov	w3, #0x15e                 	// #350
 4029450:	11048021 	add	w1, w1, #0x120
 4029454:	f84107fe 	ldr	x30, [sp], #16
 4029458:	b9400022 	ldr	w2, [x1]
 402945c:	d3536c40 	ubfx	x0, x2, #19, #9
 4029460:	12045842 	and	w2, w2, #0xf007ffff
 4029464:	1b037c00 	mul	w0, w0, w3
 4029468:	528044c3 	mov	w3, #0x226                 	// #550
 402946c:	1ac30800 	udiv	w0, w0, w3
 4029470:	11000400 	add	w0, w0, #0x1
 4029474:	2a004c40 	orr	w0, w2, w0, lsl #19
 4029478:	b9000020 	str	w0, [x1]
 402947c:	d65f03c0 	ret

0000000004029480 <ddrtrn_hal_axi_save_func>:
 4029480:	d2802081 	mov	x1, #0x104                 	// #260
 4029484:	f2a22281 	movk	x1, #0x1114, lsl #16
 4029488:	b9400021 	ldr	w1, [x1]
 402948c:	b9002c01 	str	w1, [x0, #44]
 4029490:	d2802281 	mov	x1, #0x114                 	// #276
 4029494:	f2a22281 	movk	x1, #0x1114, lsl #16
 4029498:	b9400021 	ldr	w1, [x1]
 402949c:	b9003001 	str	w1, [x0, #48]
 40294a0:	d2800801 	mov	x1, #0x40                  	// #64
 40294a4:	f2a22281 	movk	x1, #0x1114, lsl #16
 40294a8:	b9400021 	ldr	w1, [x1]
 40294ac:	b9003801 	str	w1, [x0, #56]
 40294b0:	d65f03c0 	ret

00000000040294b4 <ddrtrn_hal_axi_restore_func>:
 40294b4:	d2802081 	mov	x1, #0x104                 	// #260
 40294b8:	b9402c02 	ldr	w2, [x0, #44]
 40294bc:	f2a22281 	movk	x1, #0x1114, lsl #16
 40294c0:	b9000022 	str	w2, [x1]
 40294c4:	b9403002 	ldr	w2, [x0, #48]
 40294c8:	b9001022 	str	w2, [x1, #16]
 40294cc:	b9403801 	ldr	w1, [x0, #56]
 40294d0:	d2800800 	mov	x0, #0x40                  	// #64
 40294d4:	f2a22280 	movk	x0, #0x1114, lsl #16
 40294d8:	b9000001 	str	w1, [x0]
 40294dc:	d65f03c0 	ret

00000000040294e0 <ddrtrn_hal_axi_chsel_remap_func>:
 40294e0:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 40294e4:	97fffc44 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40294e8:	aa0003f3 	mov	x19, x0
 40294ec:	97fffc3d 	bl	40285e0 <ddrtrn_hal_get_phy>
 40294f0:	d2800782 	mov	x2, #0x3c                  	// #60
 40294f4:	b9402261 	ldr	w1, [x19, #32]
 40294f8:	9b020021 	madd	x1, x1, x2, x0
 40294fc:	b9400420 	ldr	w0, [x1, #4]
 4029500:	7100181f 	cmp	w0, #0x6
 4029504:	d2800800 	mov	x0, #0x40                  	// #64
 4029508:	f2a22280 	movk	x0, #0x1114, lsl #16
 402950c:	540000c1 	b.ne	4029524 <ddrtrn_hal_axi_chsel_remap_func+0x44>  // b.any
 4029510:	52864201 	mov	w1, #0x3210                	// #12816
 4029514:	72aeca81 	movk	w1, #0x7654, lsl #16
 4029518:	b9000001 	str	w1, [x0]
 402951c:	a8c17bf3 	ldp	x19, x30, [sp], #16
 4029520:	d65f03c0 	ret
 4029524:	52828401 	mov	w1, #0x1420                	// #5152
 4029528:	72aeca61 	movk	w1, #0x7653, lsl #16
 402952c:	17fffffb 	b	4029518 <ddrtrn_hal_axi_chsel_remap_func+0x38>

0000000004029530 <ddrtrn_hal_dmc_sfc_cmd_write>:
 4029530:	32020000 	orr	w0, w0, #0x40000000
 4029534:	b9000020 	str	w0, [x1]
 4029538:	d65f03c0 	ret

000000000402953c <ddrtrn_hal_dmc_sfc_bank_write>:
 402953c:	32100000 	orr	w0, w0, #0x10000
 4029540:	b9000020 	str	w0, [x1]
 4029544:	d65f03c0 	ret

0000000004029548 <ddrtrn_hal_axi_switch_func>:
 4029548:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402954c:	f9000bfe 	str	x30, [sp, #16]
 4029550:	97fffc29 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029554:	b9402013 	ldr	w19, [x0, #32]
 4029558:	97fffc27 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402955c:	aa0003f4 	mov	x20, x0
 4029560:	97fffc20 	bl	40285e0 <ddrtrn_hal_get_phy>
 4029564:	d2800782 	mov	x2, #0x3c                  	// #60
 4029568:	b9402281 	ldr	w1, [x20, #32]
 402956c:	9b020021 	madd	x1, x1, x2, x0
 4029570:	b9400420 	ldr	w0, [x1, #4]
 4029574:	7100181f 	cmp	w0, #0x6
 4029578:	540000e1 	b.ne	4029594 <ddrtrn_hal_axi_switch_func+0x4c>  // b.any
 402957c:	97fffc1e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029580:	b9402013 	ldr	w19, [x0, #32]
 4029584:	97fffc1c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029588:	b9402800 	ldr	w0, [x0, #40]
 402958c:	531f7a73 	lsl	w19, w19, #1
 4029590:	0b000273 	add	w19, w19, w0
 4029594:	d2802081 	mov	x1, #0x104                 	// #260
 4029598:	f2a22281 	movk	x1, #0x1114, lsl #16
 402959c:	f9400bfe 	ldr	x30, [sp, #16]
 40295a0:	b9400020 	ldr	w0, [x1]
 40295a4:	121a6400 	and	w0, w0, #0xffffffc0
 40295a8:	2a130000 	orr	w0, w0, w19
 40295ac:	321d0000 	orr	w0, w0, #0x8
 40295b0:	b9000020 	str	w0, [x1]
 40295b4:	b9401020 	ldr	w0, [x1, #16]
 40295b8:	121a6400 	and	w0, w0, #0xffffffc0
 40295bc:	2a130000 	orr	w0, w0, w19
 40295c0:	321d0000 	orr	w0, w0, #0x8
 40295c4:	b9001020 	str	w0, [x1, #16]
 40295c8:	a8c253f3 	ldp	x19, x20, [sp], #32
 40295cc:	d65f03c0 	ret

00000000040295d0 <ddrtrn_hal_rnkvol_save_func>:
 40295d0:	11018021 	add	w1, w1, #0x60
 40295d4:	b9400021 	ldr	w1, [x1]
 40295d8:	b9003401 	str	w1, [x0, #52]
 40295dc:	d65f03c0 	ret

00000000040295e0 <ddrtrn_hal_rnkvol_restore_func>:
 40295e0:	11018021 	add	w1, w1, #0x60
 40295e4:	b9403400 	ldr	w0, [x0, #52]
 40295e8:	b9000020 	str	w0, [x1]
 40295ec:	d65f03c0 	ret

00000000040295f0 <ddrtrn_hal_rnkvol_set_func>:
 40295f0:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 40295f4:	97fffc00 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40295f8:	b9402400 	ldr	w0, [x0, #36]
 40295fc:	7100041f 	cmp	w0, #0x1
 4029600:	54000141 	b.ne	4029628 <ddrtrn_hal_rnkvol_set_func+0x38>  // b.any
 4029604:	97fffbfc 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029608:	b9400800 	ldr	w0, [x0, #8]
 402960c:	11018000 	add	w0, w0, #0x60
 4029610:	b9400013 	ldr	w19, [x0]
 4029614:	97fffbf8 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029618:	b9400800 	ldr	w0, [x0, #8]
 402961c:	121d7273 	and	w19, w19, #0xfffffff8
 4029620:	11018000 	add	w0, w0, #0x60
 4029624:	b9000013 	str	w19, [x0]
 4029628:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402962c:	d65f03c0 	ret

0000000004029630 <ddrt_reg_read>:
 4029630:	2a0003e0 	mov	w0, w0
 4029634:	b9400000 	ldr	w0, [x0]
 4029638:	d65f03c0 	ret

000000000402963c <ddrt_reg_write>:
 402963c:	2a0103e1 	mov	w1, w1
 4029640:	b9000020 	str	w0, [x1]
 4029644:	d65f03c0 	ret

0000000004029648 <ddrtrn_hal_ddrt_get_mem_width>:
 4029648:	f81f0ffe 	str	x30, [sp, #-16]!
 402964c:	97fffbea 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029650:	b9400800 	ldr	w0, [x0, #8]
 4029654:	f84107fe 	ldr	x30, [sp], #16
 4029658:	11014000 	add	w0, w0, #0x50
 402965c:	b9400000 	ldr	w0, [x0]
 4029660:	d3441400 	ubfx	x0, x0, #4, #2
 4029664:	d65f03c0 	ret

0000000004029668 <ddrtrn_hal_ddrt_get_addr>:
 4029668:	d2800400 	mov	x0, #0x20                  	// #32
 402966c:	f2a222c0 	movk	x0, #0x1116, lsl #16
 4029670:	b9400000 	ldr	w0, [x0]
 4029674:	d65f03c0 	ret

0000000004029678 <ddrtrn_hal_get_cur_byte>:
 4029678:	f81f0ffe 	str	x30, [sp, #-16]!
 402967c:	97fffbde 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029680:	b9401800 	ldr	w0, [x0, #24]
 4029684:	f84107fe 	ldr	x30, [sp], #16
 4029688:	d65f03c0 	ret

000000000402968c <ddrtrn_hal_get_rank_id>:
 402968c:	f81f0ffe 	str	x30, [sp, #-16]!
 4029690:	97fffbd9 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029694:	b9402400 	ldr	w0, [x0, #36]
 4029698:	f84107fe 	ldr	x30, [sp], #16
 402969c:	d65f03c0 	ret

00000000040296a0 <ddrtrn_hal_get_cur_mode>:
 40296a0:	f81f0ffe 	str	x30, [sp, #-16]!
 40296a4:	97fffbd4 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40296a8:	b9401400 	ldr	w0, [x0, #20]
 40296ac:	f84107fe 	ldr	x30, [sp], #16
 40296b0:	d65f03c0 	ret

00000000040296b4 <ddrtrn_hal_get_cur_phy>:
 40296b4:	f81f0ffe 	str	x30, [sp, #-16]!
 40296b8:	97fffbcf 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40296bc:	b9400400 	ldr	w0, [x0, #4]
 40296c0:	f84107fe 	ldr	x30, [sp], #16
 40296c4:	d65f03c0 	ret

00000000040296c8 <ddrtrn_hal_set_rank_id>:
 40296c8:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 40296cc:	2a0003f3 	mov	w19, w0
 40296d0:	97fffbc9 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40296d4:	b9002413 	str	w19, [x0, #36]
 40296d8:	a8c17bf3 	ldp	x19, x30, [sp], #16
 40296dc:	d65f03c0 	ret

00000000040296e0 <ddrtrn_hal_get_cur_phy_total_byte_num>:
 40296e0:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 40296e4:	97fffbc4 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40296e8:	aa0003f3 	mov	x19, x0
 40296ec:	97fffbbd 	bl	40285e0 <ddrtrn_hal_get_phy>
 40296f0:	d2800782 	mov	x2, #0x3c                  	// #60
 40296f4:	b9402261 	ldr	w1, [x19, #32]
 40296f8:	a8c17bf3 	ldp	x19, x30, [sp], #16
 40296fc:	d503201f 	nop
 4029700:	9b020021 	madd	x1, x1, x2, x0
 4029704:	b9401020 	ldr	w0, [x1, #16]
 4029708:	d65f03c0 	ret

000000000402970c <ddrtrn_hal_phy_get_dq_bdl>:
 402970c:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4029710:	a9017bf5 	stp	x21, x30, [sp, #16]
 4029714:	97ffffd9 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 4029718:	2a0003f3 	mov	w19, w0
 402971c:	97ffffdc 	bl	402968c <ddrtrn_hal_get_rank_id>
 4029720:	2a0003f4 	mov	w20, w0
 4029724:	97fffbb4 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029728:	b9401c15 	ldr	w21, [x0, #28]
 402972c:	97ffffdd 	bl	40296a0 <ddrtrn_hal_get_cur_mode>
 4029730:	53196273 	lsl	w19, w19, #7
 4029734:	7100081f 	cmp	w0, #0x2
 4029738:	53165680 	lsl	w0, w20, #10
 402973c:	121e02a1 	and	w1, w21, #0x4
 4029740:	0b000273 	add	w19, w19, w0
 4029744:	540001c1 	b.ne	402977c <ddrtrn_hal_phy_get_dq_bdl+0x70>  // b.any
 4029748:	11084260 	add	w0, w19, #0x210
 402974c:	11085273 	add	w19, w19, #0x214
 4029750:	7100003f 	cmp	w1, #0x0
 4029754:	531d06b5 	ubfiz	w21, w21, #3, #2
 4029758:	1a801273 	csel	w19, w19, w0, ne  // ne = any
 402975c:	97ffffd6 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029760:	0b130013 	add	w19, w0, w19
 4029764:	b9400260 	ldr	w0, [x19]
 4029768:	1ad52400 	lsr	w0, w0, w21
 402976c:	12001800 	and	w0, w0, #0x7f
 4029770:	a9417bf5 	ldp	x21, x30, [sp, #16]
 4029774:	a8c253f3 	ldp	x19, x20, [sp], #32
 4029778:	d65f03c0 	ret
 402977c:	11087260 	add	w0, w19, #0x21c
 4029780:	11088273 	add	w19, w19, #0x220
 4029784:	17fffff3 	b	4029750 <ddrtrn_hal_phy_get_dq_bdl+0x44>

0000000004029788 <ddrtrn_hal_hw_item_cfg>:
 4029788:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402978c:	2a0003f3 	mov	w19, w0
 4029790:	d2801200 	mov	x0, #0x90                  	// #144
 4029794:	f2a22040 	movk	x0, #0x1102, lsl #16
 4029798:	b9400014 	ldr	w20, [x0]
 402979c:	f9000bfe 	str	x30, [sp, #16]
 40297a0:	97fffb90 	bl	40285e0 <ddrtrn_hal_get_phy>
 40297a4:	0a140274 	and	w20, w19, w20
 40297a8:	b9003014 	str	w20, [x0, #48]
 40297ac:	d2801280 	mov	x0, #0x94                  	// #148
 40297b0:	f2a22040 	movk	x0, #0x1102, lsl #16
 40297b4:	b9400000 	ldr	w0, [x0]
 40297b8:	0a000273 	and	w19, w19, w0
 40297bc:	97fffb89 	bl	40285e0 <ddrtrn_hal_get_phy>
 40297c0:	f9400bfe 	ldr	x30, [sp, #16]
 40297c4:	b9003813 	str	w19, [x0, #56]
 40297c8:	a8c253f3 	ldp	x19, x20, [sp], #32
 40297cc:	d65f03c0 	ret

00000000040297d0 <ddrtrn_hal_sw_item_cfg>:
 40297d0:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 40297d4:	2a0003f3 	mov	w19, w0
 40297d8:	d2801400 	mov	x0, #0xa0                  	// #160
 40297dc:	f2a22040 	movk	x0, #0x1102, lsl #16
 40297e0:	b9400014 	ldr	w20, [x0]
 40297e4:	f9000bfe 	str	x30, [sp, #16]
 40297e8:	97fffb7e 	bl	40285e0 <ddrtrn_hal_get_phy>
 40297ec:	0a140274 	and	w20, w19, w20
 40297f0:	b9002c14 	str	w20, [x0, #44]
 40297f4:	d2801480 	mov	x0, #0xa4                  	// #164
 40297f8:	f2a22040 	movk	x0, #0x1102, lsl #16
 40297fc:	b9400000 	ldr	w0, [x0]
 4029800:	0a000273 	and	w19, w19, w0
 4029804:	97fffb77 	bl	40285e0 <ddrtrn_hal_get_phy>
 4029808:	f9400bfe 	ldr	x30, [sp, #16]
 402980c:	b9003413 	str	w19, [x0, #52]
 4029810:	a8c253f3 	ldp	x19, x20, [sp], #32
 4029814:	d65f03c0 	ret

0000000004029818 <ddrtrn_hal_check_bypass>:
 4029818:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402981c:	2a0003f3 	mov	w19, w0
 4029820:	97fffb75 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029824:	b9400c00 	ldr	w0, [x0, #12]
 4029828:	6a00027f 	tst	w19, w0
 402982c:	1a9f07e0 	cset	w0, ne  // ne = any
 4029830:	a8c17bf3 	ldp	x19, x30, [sp], #16
 4029834:	d65f03c0 	ret

0000000004029838 <ddrtrn_hal_phy_reset>:
 4029838:	11001000 	add	w0, w0, #0x4
 402983c:	b9400001 	ldr	w1, [x0]
 4029840:	32130022 	orr	w2, w1, #0x2000
 4029844:	b9000002 	str	w2, [x0]
 4029848:	12127821 	and	w1, w1, #0xffffdfff
 402984c:	b9000001 	str	w1, [x0]
 4029850:	d65f03c0 	ret

0000000004029854 <ddrtrn_hal_phy_cfg_update>:
 4029854:	1101c002 	add	w2, w0, #0x70
 4029858:	11001000 	add	w0, w0, #0x4
 402985c:	b9400041 	ldr	w1, [x2]
 4029860:	320d0023 	orr	w3, w1, #0x80000
 4029864:	b9000043 	str	w3, [x2]
 4029868:	120c7821 	and	w1, w1, #0xfff7ffff
 402986c:	b9000041 	str	w1, [x2]
 4029870:	b9400001 	ldr	w1, [x0]
 4029874:	32110022 	orr	w2, w1, #0x8000
 4029878:	b9000002 	str	w2, [x0]
 402987c:	12107821 	and	w1, w1, #0xffff7fff
 4029880:	b9000001 	str	w1, [x0]
 4029884:	d5033f9f 	dsb	sy
 4029888:	d65f03c0 	ret

000000000402988c <ddrtrn_hal_ck_cfg>:
 402988c:	52820e01 	mov	w1, #0x1070                	// #4208
 4029890:	0b010001 	add	w1, w0, w1
 4029894:	5281f802 	mov	w2, #0xfc0                 	// #4032
 4029898:	72a06002 	movk	w2, #0x300, lsl #16
 402989c:	b9400023 	ldr	w3, [x1]
 40298a0:	4a020062 	eor	w2, w3, w2
 40298a4:	b9000022 	str	w2, [x1]
 40298a8:	b9000023 	str	w3, [x1]
 40298ac:	17ffffea 	b	4029854 <ddrtrn_hal_phy_cfg_update>

00000000040298b0 <ddrtrn_hal_phy_set_dq_bdl>:
 40298b0:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 40298b4:	2a0003f4 	mov	w20, w0
 40298b8:	a9015bf5 	stp	x21, x22, [sp, #16]
 40298bc:	a9027bf7 	stp	x23, x30, [sp, #32]
 40298c0:	97ffff7d 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 40298c4:	2a0003f7 	mov	w23, w0
 40298c8:	97ffff6c 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 40298cc:	2a0003f5 	mov	w21, w0
 40298d0:	97ffff6f 	bl	402968c <ddrtrn_hal_get_rank_id>
 40298d4:	2a0003f3 	mov	w19, w0
 40298d8:	97fffb47 	bl	40285f4 <ddrtrn_hal_get_ctx>
 40298dc:	b9401c16 	ldr	w22, [x0, #28]
 40298e0:	97ffff70 	bl	40296a0 <ddrtrn_hal_get_cur_mode>
 40298e4:	7100081f 	cmp	w0, #0x2
 40298e8:	53165661 	lsl	w1, w19, #10
 40298ec:	531962a2 	lsl	w2, w21, #7
 40298f0:	121e02c0 	and	w0, w22, #0x4
 40298f4:	0b020021 	add	w1, w1, w2
 40298f8:	54000281 	b.ne	4029948 <ddrtrn_hal_phy_set_dq_bdl+0x98>  // b.any
 40298fc:	11084022 	add	w2, w1, #0x210
 4029900:	11085021 	add	w1, w1, #0x214
 4029904:	7100001f 	cmp	w0, #0x0
 4029908:	531d06d6 	ubfiz	w22, w22, #3, #2
 402990c:	1a821021 	csel	w1, w1, w2, ne  // ne = any
 4029910:	12001a94 	and	w20, w20, #0x7f
 4029914:	0b170021 	add	w1, w1, w23
 4029918:	52801fe2 	mov	w2, #0xff                  	// #255
 402991c:	1ad62042 	lsl	w2, w2, w22
 4029920:	1ad62294 	lsl	w20, w20, w22
 4029924:	b9400020 	ldr	w0, [x1]
 4029928:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402992c:	0a220002 	bic	w2, w0, w2
 4029930:	2a020294 	orr	w20, w20, w2
 4029934:	2a1703e0 	mov	w0, w23
 4029938:	a9427bf7 	ldp	x23, x30, [sp, #32]
 402993c:	b9000034 	str	w20, [x1]
 4029940:	a8c353f3 	ldp	x19, x20, [sp], #48
 4029944:	17ffffc4 	b	4029854 <ddrtrn_hal_phy_cfg_update>
 4029948:	11087022 	add	w2, w1, #0x21c
 402994c:	11088021 	add	w1, w1, #0x220
 4029950:	17ffffed 	b	4029904 <ddrtrn_hal_phy_set_dq_bdl+0x54>

0000000004029954 <ddrtrn_hal_switch_rank_all_phy>:
 4029954:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4029958:	2a0003f4 	mov	w20, w0
 402995c:	52800013 	mov	w19, #0x0                   	// #0
 4029960:	a9017bf5 	stp	x21, x30, [sp, #16]
 4029964:	d2800795 	mov	x21, #0x3c                  	// #60
 4029968:	97fffb23 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402996c:	b9400000 	ldr	w0, [x0]
 4029970:	6b00027f 	cmp	w19, w0
 4029974:	54000083 	b.cc	4029984 <ddrtrn_hal_switch_rank_all_phy+0x30>  // b.lo, b.ul, b.last
 4029978:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402997c:	a8c253f3 	ldp	x19, x20, [sp], #32
 4029980:	d65f03c0 	ret
 4029984:	97fffb17 	bl	40285e0 <ddrtrn_hal_get_phy>
 4029988:	2a1303e2 	mov	w2, w19
 402998c:	2a1403e1 	mov	w1, w20
 4029990:	11000673 	add	w19, w19, #0x1
 4029994:	9b157c42 	mul	x2, x2, x21
 4029998:	b8626800 	ldr	w0, [x0, x2]
 402999c:	9400037d 	bl	402a790 <ddrtrn_hal_phy_switch_rank>
 40299a0:	17fffff2 	b	4029968 <ddrtrn_hal_switch_rank_all_phy+0x14>

00000000040299a4 <ddrtrn_hal_adjust_get_val>:
 40299a4:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 40299a8:	f9000bfe 	str	x30, [sp, #16]
 40299ac:	97ffff3d 	bl	40296a0 <ddrtrn_hal_get_cur_mode>
 40299b0:	7100041f 	cmp	w0, #0x1
 40299b4:	54000161 	b.ne	40299e0 <ddrtrn_hal_adjust_get_val+0x3c>  // b.any
 40299b8:	97ffff3f 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 40299bc:	2a0003f3 	mov	w19, w0
 40299c0:	1108b273 	add	w19, w19, #0x22c
 40299c4:	97ffff2d 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 40299c8:	0b001e73 	add	w19, w19, w0, lsl #7
 40299cc:	b9400260 	ldr	w0, [x19]
 40299d0:	12002000 	and	w0, w0, #0x1ff
 40299d4:	f9400bfe 	ldr	x30, [sp, #16]
 40299d8:	a8c253f3 	ldp	x19, x20, [sp], #32
 40299dc:	d65f03c0 	ret
 40299e0:	97ffff35 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 40299e4:	2a0003f4 	mov	w20, w0
 40299e8:	97ffff29 	bl	402968c <ddrtrn_hal_get_rank_id>
 40299ec:	2a0003f3 	mov	w19, w0
 40299f0:	97ffff22 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 40299f4:	53196000 	lsl	w0, w0, #7
 40299f8:	0b132813 	add	w19, w0, w19, lsl #10
 40299fc:	1108d294 	add	w20, w20, #0x234
 4029a00:	0b140273 	add	w19, w19, w20
 4029a04:	b9400260 	ldr	w0, [x19]
 4029a08:	d3483400 	ubfx	x0, x0, #8, #6
 4029a0c:	17fffff2 	b	40299d4 <ddrtrn_hal_adjust_get_val+0x30>

0000000004029a10 <ddrtrn_hal_adjust_set_val>:
 4029a10:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 4029a14:	2a0003f3 	mov	w19, w0
 4029a18:	a9017bf5 	stp	x21, x30, [sp, #16]
 4029a1c:	97ffff21 	bl	40296a0 <ddrtrn_hal_get_cur_mode>
 4029a20:	7100041f 	cmp	w0, #0x1
 4029a24:	540002a1 	b.ne	4029a78 <ddrtrn_hal_adjust_set_val+0x68>  // b.any
 4029a28:	97ffff23 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029a2c:	2a0003f4 	mov	w20, w0
 4029a30:	1108b294 	add	w20, w20, #0x22c
 4029a34:	97ffff11 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 4029a38:	0b001e94 	add	w20, w20, w0, lsl #7
 4029a3c:	2a1303e0 	mov	w0, w19
 4029a40:	b9400294 	ldr	w20, [x20]
 4029a44:	9400034b 	bl	402a770 <ddrtrn_hal_phy_rdqs_sync_rdm>
 4029a48:	97ffff1b 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029a4c:	12175a94 	and	w20, w20, #0xfffffe00
 4029a50:	2a130293 	orr	w19, w20, w19
 4029a54:	2a0003f4 	mov	w20, w0
 4029a58:	1108b294 	add	w20, w20, #0x22c
 4029a5c:	97ffff07 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 4029a60:	0b001e94 	add	w20, w20, w0, lsl #7
 4029a64:	b9000293 	str	w19, [x20]
 4029a68:	97ffff13 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029a6c:	a9417bf5 	ldp	x21, x30, [sp, #16]
 4029a70:	a8c253f3 	ldp	x19, x20, [sp], #32
 4029a74:	17ffff78 	b	4029854 <ddrtrn_hal_phy_cfg_update>
 4029a78:	97ffff0f 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029a7c:	2a0003f5 	mov	w21, w0
 4029a80:	97ffff03 	bl	402968c <ddrtrn_hal_get_rank_id>
 4029a84:	2a0003f4 	mov	w20, w0
 4029a88:	97fffefc 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 4029a8c:	53196000 	lsl	w0, w0, #7
 4029a90:	0b142814 	add	w20, w0, w20, lsl #10
 4029a94:	1108d2b5 	add	w21, w21, #0x234
 4029a98:	0b150294 	add	w20, w20, w21
 4029a9c:	b9400294 	ldr	w20, [x20]
 4029aa0:	97ffff05 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029aa4:	2a0003f5 	mov	w21, w0
 4029aa8:	97fffef9 	bl	402968c <ddrtrn_hal_get_rank_id>
 4029aac:	12126694 	and	w20, w20, #0xffffc0ff
 4029ab0:	1108d2b5 	add	w21, w21, #0x234
 4029ab4:	2a132294 	orr	w20, w20, w19, lsl #8
 4029ab8:	2a0003f3 	mov	w19, w0
 4029abc:	97fffeef 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 4029ac0:	53196000 	lsl	w0, w0, #7
 4029ac4:	0b132813 	add	w19, w0, w19, lsl #10
 4029ac8:	0b150273 	add	w19, w19, w21
 4029acc:	b9000274 	str	w20, [x19]
 4029ad0:	17ffffe6 	b	4029a68 <ddrtrn_hal_adjust_set_val+0x58>

0000000004029ad4 <ddrtrn_hal_get_dly_value>:
 4029ad4:	0b031c23 	add	w3, w1, w3, lsl #7
 4029ad8:	0b022862 	add	w2, w3, w2, lsl #10
 4029adc:	1108c041 	add	w1, w2, #0x230
 4029ae0:	b9400021 	ldr	w1, [x1]
 4029ae4:	b9000001 	str	w1, [x0]
 4029ae8:	12001823 	and	w3, w1, #0x7f
 4029aec:	b9000803 	str	w3, [x0, #8]
 4029af0:	d3482c21 	ubfx	x1, x1, #8, #4
 4029af4:	b9000401 	str	w1, [x0, #4]
 4029af8:	1108d041 	add	w1, w2, #0x234
 4029afc:	1108e042 	add	w2, w2, #0x238
 4029b00:	b9400021 	ldr	w1, [x1]
 4029b04:	b9000c01 	str	w1, [x0, #12]
 4029b08:	d3483421 	ubfx	x1, x1, #8, #6
 4029b0c:	b9001001 	str	w1, [x0, #16]
 4029b10:	b9400041 	ldr	w1, [x2]
 4029b14:	b9001401 	str	w1, [x0, #20]
 4029b18:	d3504421 	ubfx	x1, x1, #16, #2
 4029b1c:	b9001801 	str	w1, [x0, #24]
 4029b20:	d65f03c0 	ret

0000000004029b24 <ddrtrn_hal_restore_dly_value>:
 4029b24:	0b031c23 	add	w3, w1, w3, lsl #7
 4029b28:	aa0003e4 	mov	x4, x0
 4029b2c:	0b022862 	add	w2, w3, w2, lsl #10
 4029b30:	2a0103e0 	mov	w0, w1
 4029b34:	1108c041 	add	w1, w2, #0x230
 4029b38:	b9400085 	ldr	w5, [x4]
 4029b3c:	b9000025 	str	w5, [x1]
 4029b40:	1108d041 	add	w1, w2, #0x234
 4029b44:	1108e042 	add	w2, w2, #0x238
 4029b48:	b9400c83 	ldr	w3, [x4, #12]
 4029b4c:	b9000023 	str	w3, [x1]
 4029b50:	b9401481 	ldr	w1, [x4, #20]
 4029b54:	b9000041 	str	w1, [x2]
 4029b58:	17ffff3f 	b	4029854 <ddrtrn_hal_phy_cfg_update>

0000000004029b5c <ddrtrn_hal_wdqs_bdl2phase>:
 4029b5c:	1108b003 	add	w3, w0, #0x22c
 4029b60:	0b012801 	add	w1, w0, w1, lsl #10
 4029b64:	0b021c63 	add	w3, w3, w2, lsl #7
 4029b68:	0b021c22 	add	w2, w1, w2, lsl #7
 4029b6c:	1108c04b 	add	w11, w2, #0x230
 4029b70:	1108d04a 	add	w10, w2, #0x234
 4029b74:	1108e042 	add	w2, w2, #0x238
 4029b78:	b9400063 	ldr	w3, [x3]
 4029b7c:	b9400164 	ldr	w4, [x11]
 4029b80:	b9400149 	ldr	w9, [x10]
 4029b84:	d3506063 	ubfx	x3, x3, #16, #9
 4029b88:	b9400045 	ldr	w5, [x2]
 4029b8c:	71002c7f 	cmp	w3, #0xb
 4029b90:	54000589 	b.ls	4029c40 <ddrtrn_hal_wdqs_bdl2phase+0xe4>  // b.plast
 4029b94:	52800181 	mov	w1, #0xc                   	// #12
 4029b98:	d3482c87 	ubfx	x7, x4, #8, #4
 4029b9c:	d3483526 	ubfx	x6, x9, #8, #6
 4029ba0:	1ac10861 	udiv	w1, w3, w1
 4029ba4:	d35044a3 	ubfx	x3, x5, #16, #2
 4029ba8:	12001885 	and	w5, w4, #0x7f
 4029bac:	4b0100a5 	sub	w5, w5, w1
 4029bb0:	0b050028 	add	w8, w1, w5
 4029bb4:	6b01011f 	cmp	w8, w1
 4029bb8:	540001a9 	b.ls	4029bec <ddrtrn_hal_wdqs_bdl2phase+0x90>  // b.plast
 4029bbc:	710034ff 	cmp	w7, #0xd
 4029bc0:	54000128 	b.hi	4029be4 <ddrtrn_hal_wdqs_bdl2phase+0x88>  // b.pmore
 4029bc4:	110004e8 	add	w8, w7, #0x1
 4029bc8:	1200050c 	and	w12, w8, #0x3
 4029bcc:	71000d9f 	cmp	w12, #0x3
 4029bd0:	54000041 	b.ne	4029bd8 <ddrtrn_hal_wdqs_bdl2phase+0x7c>  // b.any
 4029bd4:	110008e8 	add	w8, w7, #0x2
 4029bd8:	4b0100a5 	sub	w5, w5, w1
 4029bdc:	2a0803e7 	mov	w7, w8
 4029be0:	17fffff4 	b	4029bb0 <ddrtrn_hal_wdqs_bdl2phase+0x54>
 4029be4:	7100047f 	cmp	w3, #0x1
 4029be8:	540001c9 	b.ls	4029c20 <ddrtrn_hal_wdqs_bdl2phase+0xc4>  // b.plast
 4029bec:	12196084 	and	w4, w4, #0xffffff80
 4029bf0:	12126529 	and	w9, w9, #0xffffc0ff
 4029bf4:	2a080084 	orr	w4, w4, w8
 4029bf8:	2a062126 	orr	w6, w9, w6, lsl #8
 4029bfc:	12146c84 	and	w4, w4, #0xfffff0ff
 4029c00:	2a072084 	orr	w4, w4, w7, lsl #8
 4029c04:	b9000164 	str	w4, [x11]
 4029c08:	b9400041 	ldr	w1, [x2]
 4029c0c:	120e7421 	and	w1, w1, #0xfffcffff
 4029c10:	2a034023 	orr	w3, w1, w3, lsl #16
 4029c14:	b9000043 	str	w3, [x2]
 4029c18:	b9000146 	str	w6, [x10]
 4029c1c:	17ffff0e 	b	4029854 <ddrtrn_hal_phy_cfg_update>
 4029c20:	11000463 	add	w3, w3, #0x1
 4029c24:	71003cdf 	cmp	w6, #0xf
 4029c28:	54000089 	b.ls	4029c38 <ddrtrn_hal_wdqs_bdl2phase+0xdc>  // b.plast
 4029c2c:	510040c6 	sub	w6, w6, #0x10
 4029c30:	52800008 	mov	w8, #0x0                   	// #0
 4029c34:	17ffffe9 	b	4029bd8 <ddrtrn_hal_wdqs_bdl2phase+0x7c>
 4029c38:	52800006 	mov	w6, #0x0                   	// #0
 4029c3c:	17fffffd 	b	4029c30 <ddrtrn_hal_wdqs_bdl2phase+0xd4>
 4029c40:	d65f03c0 	ret

0000000004029c44 <ddrtrn_hal_training_adjust_wdq>:
 4029c44:	d10343ff 	sub	sp, sp, #0xd0
 4029c48:	90ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4029c4c:	a90753f3 	stp	x19, x20, [sp, #112]
 4029c50:	a9085bf5 	stp	x21, x22, [sp, #128]
 4029c54:	52800016 	mov	w22, #0x0                   	// #0
 4029c58:	a90963f7 	stp	x23, x24, [sp, #144]
 4029c5c:	52804698 	mov	w24, #0x234                 	// #564
 4029c60:	a90a6bf9 	stp	x25, x26, [sp, #160]
 4029c64:	5280019a 	mov	w26, #0xc                   	// #12
 4029c68:	a90b73fb 	stp	x27, x28, [sp, #176]
 4029c6c:	5280009b 	mov	w27, #0x4                   	// #4
 4029c70:	f90063fe 	str	x30, [sp, #192]
 4029c74:	f9455001 	ldr	x1, [x0, #2720]
 4029c78:	f90037e1 	str	x1, [sp, #104]
 4029c7c:	d2800001 	mov	x1, #0x0                   	// #0
 4029c80:	97fffe8d 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029c84:	2a0003f7 	mov	w23, w0
 4029c88:	97fffe81 	bl	402968c <ddrtrn_hal_get_rank_id>
 4029c8c:	2a0003f9 	mov	w25, w0
 4029c90:	97fffe94 	bl	40296e0 <ddrtrn_hal_get_cur_phy_total_byte_num>
 4029c94:	6b16001f 	cmp	w0, w22
 4029c98:	54000188 	b.hi	4029cc8 <ddrtrn_hal_training_adjust_wdq+0x84>  // b.pmore
 4029c9c:	2a1903e0 	mov	w0, w25
 4029ca0:	97fffe8a 	bl	40296c8 <ddrtrn_hal_set_rank_id>
 4029ca4:	2a1703e0 	mov	w0, w23
 4029ca8:	97fffeeb 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 4029cac:	90ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4029cb0:	f94037e1 	ldr	x1, [sp, #104]
 4029cb4:	f9455002 	ldr	x2, [x0, #2720]
 4029cb8:	eb020021 	subs	x1, x1, x2
 4029cbc:	d2800002 	mov	x2, #0x0                   	// #0
 4029cc0:	54001160 	b.eq	4029eec <ddrtrn_hal_training_adjust_wdq+0x2a8>  // b.none
 4029cc4:	97ffe92a 	bl	402416c <__stack_chk_fail>
 4029cc8:	97fffa4b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029ccc:	b9001816 	str	w22, [x0, #24]
 4029cd0:	2a1603e3 	mov	w3, w22
 4029cd4:	2a1703e1 	mov	w1, w23
 4029cd8:	910063e0 	add	x0, sp, #0x18
 4029cdc:	52800002 	mov	w2, #0x0                   	// #0
 4029ce0:	97ffff7d 	bl	4029ad4 <ddrtrn_hal_get_dly_value>
 4029ce4:	910103e0 	add	x0, sp, #0x40
 4029ce8:	2a1703e1 	mov	w1, w23
 4029cec:	2a1603e3 	mov	w3, w22
 4029cf0:	52800022 	mov	w2, #0x1                   	// #1
 4029cf4:	97ffff78 	bl	4029ad4 <ddrtrn_hal_get_dly_value>
 4029cf8:	97fffe6f 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029cfc:	b9402bf4 	ldr	w20, [sp, #40]
 4029d00:	b94053fc 	ldr	w28, [sp, #80]
 4029d04:	51002301 	sub	w1, w24, #0x8
 4029d08:	12000e94 	and	w20, w20, #0xf
 4029d0c:	0b000021 	add	w1, w1, w0
 4029d10:	11000693 	add	w19, w20, #0x1
 4029d14:	12000f9c 	and	w28, w28, #0xf
 4029d18:	b9400035 	ldr	w21, [x1]
 4029d1c:	1adb0e73 	sdiv	w19, w19, w27
 4029d20:	d35062b5 	ubfx	x21, x21, #16, #9
 4029d24:	4b130280 	sub	w0, w20, w19
 4029d28:	11000793 	add	w19, w28, #0x1
 4029d2c:	1ada0ab5 	udiv	w21, w21, w26
 4029d30:	1adb0e73 	sdiv	w19, w19, w27
 4029d34:	4b130393 	sub	w19, w28, w19
 4029d38:	6b13001f 	cmp	w0, w19
 4029d3c:	54000a8b 	b.lt	4029e8c <ddrtrn_hal_training_adjust_wdq+0x248>  // b.tstop
 4029d40:	4b130013 	sub	w19, w0, w19
 4029d44:	52800000 	mov	w0, #0x0                   	// #0
 4029d48:	97fffe60 	bl	40296c8 <ddrtrn_hal_set_rank_id>
 4029d4c:	71001a7f 	cmp	w19, #0x6
 4029d50:	5400012d 	b.le	4029d74 <ddrtrn_hal_training_adjust_wdq+0x130>
 4029d54:	b94053e0 	ldr	w0, [sp, #80]
 4029d58:	71003c1f 	cmp	w0, #0xf
 4029d5c:	540000c9 	b.ls	4029d74 <ddrtrn_hal_training_adjust_wdq+0x130>  // b.plast
 4029d60:	51004000 	sub	w0, w0, #0x10
 4029d64:	4b130353 	sub	w19, w26, w19
 4029d68:	b90053e0 	str	w0, [sp, #80]
 4029d6c:	52800020 	mov	w0, #0x1                   	// #1
 4029d70:	97fffe56 	bl	40296c8 <ddrtrn_hal_set_rank_id>
 4029d74:	97fffe46 	bl	402968c <ddrtrn_hal_get_rank_id>
 4029d78:	35000a40 	cbnz	w0, 4029ec0 <ddrtrn_hal_training_adjust_wdq+0x27c>
 4029d7c:	b9402be0 	ldr	w0, [sp, #40]
 4029d80:	0b00039c 	add	w28, w28, w0
 4029d84:	4b140394 	sub	w20, w28, w20
 4029d88:	b94027fc 	ldr	w28, [sp, #36]
 4029d8c:	b9002bf4 	str	w20, [sp, #40]
 4029d90:	1212679c 	and	w28, w28, #0xffffc0ff
 4029d94:	2a142394 	orr	w20, w28, w20, lsl #8
 4029d98:	97fffe47 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029d9c:	2a0003fc 	mov	w28, w0
 4029da0:	0b1c031c 	add	w28, w24, w28
 4029da4:	97fffe3a 	bl	402968c <ddrtrn_hal_get_rank_id>
 4029da8:	0b002b9c 	add	w28, w28, w0, lsl #10
 4029dac:	b9000394 	str	w20, [x28]
 4029db0:	97fffe3c 	bl	40296a0 <ddrtrn_hal_get_cur_mode>
 4029db4:	2a0003f4 	mov	w20, w0
 4029db8:	97fffa0f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029dbc:	1b137eb3 	mul	w19, w21, w19
 4029dc0:	52800041 	mov	w1, #0x2                   	// #2
 4029dc4:	b9001401 	str	w1, [x0, #20]
 4029dc8:	97fffe3b 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029dcc:	2a0003fc 	mov	w28, w0
 4029dd0:	97fffe2f 	bl	402968c <ddrtrn_hal_get_rank_id>
 4029dd4:	2a0003f5 	mov	w21, w0
 4029dd8:	97fffe28 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 4029ddc:	53196000 	lsl	w0, w0, #7
 4029de0:	0b152815 	add	w21, w0, w21, lsl #10
 4029de4:	1108639c 	add	w28, w28, #0x218
 4029de8:	0b1c02b5 	add	w21, w21, w28
 4029dec:	52800ffc 	mov	w28, #0x7f                  	// #127
 4029df0:	b94002a0 	ldr	w0, [x21]
 4029df4:	12001801 	and	w1, w0, #0x7f
 4029df8:	12196000 	and	w0, w0, #0xffffff80
 4029dfc:	0b010261 	add	w1, w19, w1
 4029e00:	6b1c003f 	cmp	w1, w28
 4029e04:	1a9cd021 	csel	w1, w1, w28, le
 4029e08:	7100003f 	cmp	w1, #0x0
 4029e0c:	1a9fa021 	csel	w1, w1, wzr, ge  // ge = tcont
 4029e10:	2a010001 	orr	w1, w0, w1
 4029e14:	b9000fe1 	str	w1, [sp, #12]
 4029e18:	97fffe27 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029e1c:	b9000be0 	str	w0, [sp, #8]
 4029e20:	97fffe1b 	bl	402968c <ddrtrn_hal_get_rank_id>
 4029e24:	2a0003f5 	mov	w21, w0
 4029e28:	97fffe14 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 4029e2c:	53196000 	lsl	w0, w0, #7
 4029e30:	294107e2 	ldp	w2, w1, [sp, #8]
 4029e34:	0b152815 	add	w21, w0, w21, lsl #10
 4029e38:	11086042 	add	w2, w2, #0x218
 4029e3c:	0b0202b5 	add	w21, w21, w2
 4029e40:	b90002a1 	str	w1, [x21]
 4029e44:	52800015 	mov	w21, #0x0                   	// #0
 4029e48:	97fff9eb 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029e4c:	b9001c15 	str	w21, [x0, #28]
 4029e50:	110006b5 	add	w21, w21, #0x1
 4029e54:	97fffe2e 	bl	402970c <ddrtrn_hal_phy_get_dq_bdl>
 4029e58:	0b000260 	add	w0, w19, w0
 4029e5c:	7101fc1f 	cmp	w0, #0x7f
 4029e60:	1a9cd000 	csel	w0, w0, w28, le
 4029e64:	7100001f 	cmp	w0, #0x0
 4029e68:	1a9fa000 	csel	w0, w0, wzr, ge  // ge = tcont
 4029e6c:	97fffe91 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 4029e70:	710022bf 	cmp	w21, #0x8
 4029e74:	54fffea1 	b.ne	4029e48 <ddrtrn_hal_training_adjust_wdq+0x204>  // b.any
 4029e78:	97fff9df 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029e7c:	110006d6 	add	w22, w22, #0x1
 4029e80:	11020318 	add	w24, w24, #0x80
 4029e84:	b9001414 	str	w20, [x0, #20]
 4029e88:	17ffff82 	b	4029c90 <ddrtrn_hal_training_adjust_wdq+0x4c>
 4029e8c:	4b000273 	sub	w19, w19, w0
 4029e90:	52800020 	mov	w0, #0x1                   	// #1
 4029e94:	97fffe0d 	bl	40296c8 <ddrtrn_hal_set_rank_id>
 4029e98:	71001a7f 	cmp	w19, #0x6
 4029e9c:	54fff6cd 	b.le	4029d74 <ddrtrn_hal_training_adjust_wdq+0x130>
 4029ea0:	b9402be0 	ldr	w0, [sp, #40]
 4029ea4:	71003c1f 	cmp	w0, #0xf
 4029ea8:	54fff669 	b.ls	4029d74 <ddrtrn_hal_training_adjust_wdq+0x130>  // b.plast
 4029eac:	51004000 	sub	w0, w0, #0x10
 4029eb0:	4b130353 	sub	w19, w26, w19
 4029eb4:	b9002be0 	str	w0, [sp, #40]
 4029eb8:	52800000 	mov	w0, #0x0                   	// #0
 4029ebc:	17ffffad 	b	4029d70 <ddrtrn_hal_training_adjust_wdq+0x12c>
 4029ec0:	97fffdf3 	bl	402968c <ddrtrn_hal_get_rank_id>
 4029ec4:	7100041f 	cmp	w0, #0x1
 4029ec8:	54fff741 	b.ne	4029db0 <ddrtrn_hal_training_adjust_wdq+0x16c>  // b.any
 4029ecc:	b94053e0 	ldr	w0, [sp, #80]
 4029ed0:	0b000294 	add	w20, w20, w0
 4029ed4:	b9404fe0 	ldr	w0, [sp, #76]
 4029ed8:	4b1c0294 	sub	w20, w20, w28
 4029edc:	b90053f4 	str	w20, [sp, #80]
 4029ee0:	12126400 	and	w0, w0, #0xffffc0ff
 4029ee4:	2a142014 	orr	w20, w0, w20, lsl #8
 4029ee8:	17ffffac 	b	4029d98 <ddrtrn_hal_training_adjust_wdq+0x154>
 4029eec:	a94753f3 	ldp	x19, x20, [sp, #112]
 4029ef0:	a9485bf5 	ldp	x21, x22, [sp, #128]
 4029ef4:	a94963f7 	ldp	x23, x24, [sp, #144]
 4029ef8:	a94a6bf9 	ldp	x25, x26, [sp, #160]
 4029efc:	a94b73fb 	ldp	x27, x28, [sp, #176]
 4029f00:	f94063fe 	ldr	x30, [sp, #192]
 4029f04:	910343ff 	add	sp, sp, #0xd0
 4029f08:	d65f03c0 	ret

0000000004029f0c <ddrtrn_hal_training_adjust_wdqs>:
 4029f0c:	d10343ff 	sub	sp, sp, #0xd0
 4029f10:	90ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4029f14:	a90753f3 	stp	x19, x20, [sp, #112]
 4029f18:	a9085bf5 	stp	x21, x22, [sp, #128]
 4029f1c:	52804616 	mov	w22, #0x230                 	// #560
 4029f20:	a90963f7 	stp	x23, x24, [sp, #144]
 4029f24:	52800017 	mov	w23, #0x0                   	// #0
 4029f28:	a90a6bf9 	stp	x25, x26, [sp, #160]
 4029f2c:	5280019a 	mov	w26, #0xc                   	// #12
 4029f30:	a90b73fb 	stp	x27, x28, [sp, #176]
 4029f34:	5280009b 	mov	w27, #0x4                   	// #4
 4029f38:	f90063fe 	str	x30, [sp, #192]
 4029f3c:	f9455001 	ldr	x1, [x0, #2720]
 4029f40:	f90037e1 	str	x1, [sp, #104]
 4029f44:	d2800001 	mov	x1, #0x0                   	// #0
 4029f48:	97fffddb 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029f4c:	2a0003f8 	mov	w24, w0
 4029f50:	97fffdcf 	bl	402968c <ddrtrn_hal_get_rank_id>
 4029f54:	2a0003f9 	mov	w25, w0
 4029f58:	97fffde2 	bl	40296e0 <ddrtrn_hal_get_cur_phy_total_byte_num>
 4029f5c:	6b17001f 	cmp	w0, w23
 4029f60:	54000188 	b.hi	4029f90 <ddrtrn_hal_training_adjust_wdqs+0x84>  // b.pmore
 4029f64:	2a1903e0 	mov	w0, w25
 4029f68:	97fffdd8 	bl	40296c8 <ddrtrn_hal_set_rank_id>
 4029f6c:	2a1803e0 	mov	w0, w24
 4029f70:	97fffe39 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 4029f74:	90ffffc0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 4029f78:	f94037e1 	ldr	x1, [sp, #104]
 4029f7c:	f9455002 	ldr	x2, [x0, #2720]
 4029f80:	eb020021 	subs	x1, x1, x2
 4029f84:	d2800002 	mov	x2, #0x0                   	// #0
 4029f88:	54001460 	b.eq	402a214 <ddrtrn_hal_training_adjust_wdqs+0x308>  // b.none
 4029f8c:	97ffe878 	bl	402416c <__stack_chk_fail>
 4029f90:	97fff999 	bl	40285f4 <ddrtrn_hal_get_ctx>
 4029f94:	b9001817 	str	w23, [x0, #24]
 4029f98:	2a1703e3 	mov	w3, w23
 4029f9c:	2a1803e1 	mov	w1, w24
 4029fa0:	910063e0 	add	x0, sp, #0x18
 4029fa4:	52800002 	mov	w2, #0x0                   	// #0
 4029fa8:	97fffecb 	bl	4029ad4 <ddrtrn_hal_get_dly_value>
 4029fac:	910103e0 	add	x0, sp, #0x40
 4029fb0:	2a1803e1 	mov	w1, w24
 4029fb4:	2a1703e3 	mov	w3, w23
 4029fb8:	52800022 	mov	w2, #0x1                   	// #1
 4029fbc:	97fffec6 	bl	4029ad4 <ddrtrn_hal_get_dly_value>
 4029fc0:	97fffdbd 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 4029fc4:	b9401ff4 	ldr	w20, [sp, #28]
 4029fc8:	b94047fc 	ldr	w28, [sp, #68]
 4029fcc:	510012c1 	sub	w1, w22, #0x4
 4029fd0:	12000e94 	and	w20, w20, #0xf
 4029fd4:	0b000021 	add	w1, w1, w0
 4029fd8:	11000693 	add	w19, w20, #0x1
 4029fdc:	12000f9c 	and	w28, w28, #0xf
 4029fe0:	b9400035 	ldr	w21, [x1]
 4029fe4:	1adb0e73 	sdiv	w19, w19, w27
 4029fe8:	d35062b5 	ubfx	x21, x21, #16, #9
 4029fec:	4b130280 	sub	w0, w20, w19
 4029ff0:	11000793 	add	w19, w28, #0x1
 4029ff4:	1ada0ab5 	udiv	w21, w21, w26
 4029ff8:	1adb0e73 	sdiv	w19, w19, w27
 4029ffc:	4b130393 	sub	w19, w28, w19
 402a000:	6b13001f 	cmp	w0, w19
 402a004:	54000aab 	b.lt	402a158 <ddrtrn_hal_training_adjust_wdqs+0x24c>  // b.tstop
 402a008:	4b130013 	sub	w19, w0, w19
 402a00c:	52800000 	mov	w0, #0x0                   	// #0
 402a010:	97fffdae 	bl	40296c8 <ddrtrn_hal_set_rank_id>
 402a014:	71001a7f 	cmp	w19, #0x6
 402a018:	5400040d 	b.le	402a098 <ddrtrn_hal_training_adjust_wdqs+0x18c>
 402a01c:	b9405be0 	ldr	w0, [sp, #88]
 402a020:	340003c0 	cbz	w0, 402a098 <ddrtrn_hal_training_adjust_wdqs+0x18c>
 402a024:	b94053e1 	ldr	w1, [sp, #80]
 402a028:	7100b83f 	cmp	w1, #0x2e
 402a02c:	54000368 	b.hi	402a098 <ddrtrn_hal_training_adjust_wdqs+0x18c>  // b.pmore
 402a030:	11004021 	add	w1, w1, #0x10
 402a034:	b90053e1 	str	w1, [sp, #80]
 402a038:	b94057e1 	ldr	w1, [sp, #84]
 402a03c:	51000400 	sub	w0, w0, #0x1
 402a040:	b9005be0 	str	w0, [sp, #88]
 402a044:	4b130353 	sub	w19, w26, w19
 402a048:	120e7421 	and	w1, w1, #0xfffcffff
 402a04c:	2a004021 	orr	w1, w1, w0, lsl #16
 402a050:	b9000fe1 	str	w1, [sp, #12]
 402a054:	97fffd98 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a058:	111022c2 	add	w2, w22, #0x408
 402a05c:	b9400fe1 	ldr	w1, [sp, #12]
 402a060:	0b000042 	add	w2, w2, w0
 402a064:	b9000041 	str	w1, [x2]
 402a068:	b9404fe2 	ldr	w2, [sp, #76]
 402a06c:	12126440 	and	w0, w2, #0xffffc0ff
 402a070:	b94053e2 	ldr	w2, [sp, #80]
 402a074:	2a022002 	orr	w2, w0, w2, lsl #8
 402a078:	b9000fe2 	str	w2, [sp, #12]
 402a07c:	97fffd8e 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a080:	111012c1 	add	w1, w22, #0x404
 402a084:	b9400fe2 	ldr	w2, [sp, #12]
 402a088:	0b000021 	add	w1, w1, w0
 402a08c:	52800020 	mov	w0, #0x1                   	// #1
 402a090:	b9000022 	str	w2, [x1]
 402a094:	97fffd8d 	bl	40296c8 <ddrtrn_hal_set_rank_id>
 402a098:	97fffd7d 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a09c:	35000a60 	cbnz	w0, 402a1e8 <ddrtrn_hal_training_adjust_wdqs+0x2dc>
 402a0a0:	b9401fe0 	ldr	w0, [sp, #28]
 402a0a4:	0b00039c 	add	w28, w28, w0
 402a0a8:	4b140394 	sub	w20, w28, w20
 402a0ac:	b9401bfc 	ldr	w28, [sp, #24]
 402a0b0:	b9001ff4 	str	w20, [sp, #28]
 402a0b4:	12146f9c 	and	w28, w28, #0xfffff0ff
 402a0b8:	2a142394 	orr	w20, w28, w20, lsl #8
 402a0bc:	97fffd7e 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a0c0:	2a0003fc 	mov	w28, w0
 402a0c4:	0b1c02dc 	add	w28, w22, w28
 402a0c8:	97fffd71 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a0cc:	0b002b9c 	add	w28, w28, w0, lsl #10
 402a0d0:	b9000394 	str	w20, [x28]
 402a0d4:	97fffd78 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a0d8:	1b137eb3 	mul	w19, w21, w19
 402a0dc:	2a0003f5 	mov	w21, w0
 402a0e0:	97fffd6b 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a0e4:	2a0003f4 	mov	w20, w0
 402a0e8:	97fffd64 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 402a0ec:	53196000 	lsl	w0, w0, #7
 402a0f0:	1108c2b5 	add	w21, w21, #0x230
 402a0f4:	0b142814 	add	w20, w0, w20, lsl #10
 402a0f8:	110006f7 	add	w23, w23, #0x1
 402a0fc:	0b150294 	add	w20, w20, w21
 402a100:	110202d6 	add	w22, w22, #0x80
 402a104:	b9400280 	ldr	w0, [x20]
 402a108:	12001801 	and	w1, w0, #0x7f
 402a10c:	12196000 	and	w0, w0, #0xffffff80
 402a110:	0b010273 	add	w19, w19, w1
 402a114:	52800fe1 	mov	w1, #0x7f                  	// #127
 402a118:	7101fe7f 	cmp	w19, #0x7f
 402a11c:	1a81d273 	csel	w19, w19, w1, le
 402a120:	7100027f 	cmp	w19, #0x0
 402a124:	1a9fa273 	csel	w19, w19, wzr, ge  // ge = tcont
 402a128:	2a130013 	orr	w19, w0, w19
 402a12c:	97fffd62 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a130:	2a0003f5 	mov	w21, w0
 402a134:	97fffd56 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a138:	2a0003f4 	mov	w20, w0
 402a13c:	97fffd4f 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 402a140:	53196000 	lsl	w0, w0, #7
 402a144:	0b142814 	add	w20, w0, w20, lsl #10
 402a148:	1108c2b5 	add	w21, w21, #0x230
 402a14c:	0b150294 	add	w20, w20, w21
 402a150:	b9000293 	str	w19, [x20]
 402a154:	17ffff81 	b	4029f58 <ddrtrn_hal_training_adjust_wdqs+0x4c>
 402a158:	4b000273 	sub	w19, w19, w0
 402a15c:	52800020 	mov	w0, #0x1                   	// #1
 402a160:	97fffd5a 	bl	40296c8 <ddrtrn_hal_set_rank_id>
 402a164:	71001a7f 	cmp	w19, #0x6
 402a168:	54fff98d 	b.le	402a098 <ddrtrn_hal_training_adjust_wdqs+0x18c>
 402a16c:	b94033e0 	ldr	w0, [sp, #48]
 402a170:	34fff940 	cbz	w0, 402a098 <ddrtrn_hal_training_adjust_wdqs+0x18c>
 402a174:	b9402be1 	ldr	w1, [sp, #40]
 402a178:	7100b83f 	cmp	w1, #0x2e
 402a17c:	54fff8e8 	b.hi	402a098 <ddrtrn_hal_training_adjust_wdqs+0x18c>  // b.pmore
 402a180:	11004021 	add	w1, w1, #0x10
 402a184:	b9002be1 	str	w1, [sp, #40]
 402a188:	b9402fe1 	ldr	w1, [sp, #44]
 402a18c:	51000400 	sub	w0, w0, #0x1
 402a190:	b90033e0 	str	w0, [sp, #48]
 402a194:	4b130353 	sub	w19, w26, w19
 402a198:	120e7421 	and	w1, w1, #0xfffcffff
 402a19c:	2a004021 	orr	w1, w1, w0, lsl #16
 402a1a0:	b9000fe1 	str	w1, [sp, #12]
 402a1a4:	97fffd44 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a1a8:	110022c2 	add	w2, w22, #0x8
 402a1ac:	b9400fe1 	ldr	w1, [sp, #12]
 402a1b0:	0b000042 	add	w2, w2, w0
 402a1b4:	b9000041 	str	w1, [x2]
 402a1b8:	b94027e2 	ldr	w2, [sp, #36]
 402a1bc:	12126440 	and	w0, w2, #0xffffc0ff
 402a1c0:	b9402be2 	ldr	w2, [sp, #40]
 402a1c4:	2a022002 	orr	w2, w0, w2, lsl #8
 402a1c8:	b9000fe2 	str	w2, [sp, #12]
 402a1cc:	97fffd3a 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a1d0:	110012c1 	add	w1, w22, #0x4
 402a1d4:	b9400fe2 	ldr	w2, [sp, #12]
 402a1d8:	0b000021 	add	w1, w1, w0
 402a1dc:	52800000 	mov	w0, #0x0                   	// #0
 402a1e0:	b9000022 	str	w2, [x1]
 402a1e4:	17ffffac 	b	402a094 <ddrtrn_hal_training_adjust_wdqs+0x188>
 402a1e8:	97fffd29 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a1ec:	7100041f 	cmp	w0, #0x1
 402a1f0:	54fff721 	b.ne	402a0d4 <ddrtrn_hal_training_adjust_wdqs+0x1c8>  // b.any
 402a1f4:	b94047e0 	ldr	w0, [sp, #68]
 402a1f8:	0b000294 	add	w20, w20, w0
 402a1fc:	b94043e0 	ldr	w0, [sp, #64]
 402a200:	4b1c0294 	sub	w20, w20, w28
 402a204:	b90047f4 	str	w20, [sp, #68]
 402a208:	12146c00 	and	w0, w0, #0xfffff0ff
 402a20c:	2a142014 	orr	w20, w0, w20, lsl #8
 402a210:	17ffffab 	b	402a0bc <ddrtrn_hal_training_adjust_wdqs+0x1b0>
 402a214:	a94753f3 	ldp	x19, x20, [sp, #112]
 402a218:	a9485bf5 	ldp	x21, x22, [sp, #128]
 402a21c:	a94963f7 	ldp	x23, x24, [sp, #144]
 402a220:	a94a6bf9 	ldp	x25, x26, [sp, #160]
 402a224:	a94b73fb 	ldp	x27, x28, [sp, #176]
 402a228:	f94063fe 	ldr	x30, [sp, #192]
 402a22c:	910343ff 	add	sp, sp, #0xd0
 402a230:	d65f03c0 	ret

000000000402a234 <ddrtrn_hal_training_delay>:
 402a234:	35000040 	cbnz	w0, 402a23c <ddrtrn_hal_training_delay+0x8>
 402a238:	d65f03c0 	ret
 402a23c:	d503201f 	nop
 402a240:	51000400 	sub	w0, w0, #0x1
 402a244:	17fffffc 	b	402a234 <ddrtrn_hal_training_delay>

000000000402a248 <ddrtrn_hal_training_stat>:
 402a248:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402a24c:	2a0003f3 	mov	w19, w0
 402a250:	2a0303f4 	mov	w20, w3
 402a254:	a9015bf5 	stp	x21, x22, [sp, #16]
 402a258:	2a0103f6 	mov	w22, w1
 402a25c:	2a0203f5 	mov	w21, w2
 402a260:	f90013fe 	str	x30, [sp, #32]
 402a264:	940008a2 	bl	402c4ec <ddrtrn_training_error>
 402a268:	d2801600 	mov	x0, #0xb0                  	// #176
 402a26c:	f2a22040 	movk	x0, #0x1102, lsl #16
 402a270:	b9400000 	ldr	w0, [x0]
 402a274:	35000220 	cbnz	w0, 402a2b8 <ddrtrn_hal_training_stat+0x70>
 402a278:	340000f6 	cbz	w22, 402a294 <ddrtrn_hal_training_stat+0x4c>
 402a27c:	52a222a0 	mov	w0, #0x11150000            	// #286588928
 402a280:	6b0002df 	cmp	w22, w0
 402a284:	52840001 	mov	w1, #0x2000                	// #8192
 402a288:	52820000 	mov	w0, #0x1000                	// #4096
 402a28c:	1a810000 	csel	w0, w0, w1, eq  // eq = none
 402a290:	2a000273 	orr	w19, w19, w0
 402a294:	310006bf 	cmn	w21, #0x1
 402a298:	54000040 	b.eq	402a2a0 <ddrtrn_hal_training_stat+0x58>  // b.none
 402a29c:	2a156273 	orr	w19, w19, w21, lsl #24
 402a2a0:	3100069f 	cmn	w20, #0x1
 402a2a4:	54000040 	b.eq	402a2ac <ddrtrn_hal_training_stat+0x64>  // b.none
 402a2a8:	2a145273 	orr	w19, w19, w20, lsl #20
 402a2ac:	d2801600 	mov	x0, #0xb0                  	// #176
 402a2b0:	f2a22040 	movk	x0, #0x1102, lsl #16
 402a2b4:	b9000013 	str	w19, [x0]
 402a2b8:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402a2bc:	f94013fe 	ldr	x30, [sp, #32]
 402a2c0:	a8c353f3 	ldp	x19, x20, [sp], #48
 402a2c4:	d65f03c0 	ret

000000000402a2c8 <ddrtrn_hal_rdqs_sync_rank_rdq>:
 402a2c8:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402a2cc:	2a0003f4 	mov	w20, w0
 402a2d0:	a9015bf5 	stp	x21, x22, [sp, #16]
 402a2d4:	a90263f7 	stp	x23, x24, [sp, #32]
 402a2d8:	f9001bfe 	str	x30, [sp, #48]
 402a2dc:	97fffcf1 	bl	40296a0 <ddrtrn_hal_get_cur_mode>
 402a2e0:	2a0003f5 	mov	w21, w0
 402a2e4:	97fff8c4 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a2e8:	52800021 	mov	w1, #0x1                   	// #1
 402a2ec:	b9001401 	str	w1, [x0, #20]
 402a2f0:	97fffcf1 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a2f4:	2a0003f6 	mov	w22, w0
 402a2f8:	97fffce5 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a2fc:	2a0003f3 	mov	w19, w0
 402a300:	97fffcde 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 402a304:	53196000 	lsl	w0, w0, #7
 402a308:	0b132813 	add	w19, w0, w19, lsl #10
 402a30c:	110892d6 	add	w22, w22, #0x224
 402a310:	0b160273 	add	w19, w19, w22
 402a314:	52800ff6 	mov	w22, #0x7f                  	// #127
 402a318:	b9400260 	ldr	w0, [x19]
 402a31c:	12001801 	and	w1, w0, #0x7f
 402a320:	12196000 	and	w0, w0, #0xffffff80
 402a324:	0b010281 	add	w1, w20, w1
 402a328:	6b16003f 	cmp	w1, w22
 402a32c:	1a96d021 	csel	w1, w1, w22, le
 402a330:	7100003f 	cmp	w1, #0x0
 402a334:	1a9fa021 	csel	w1, w1, wzr, ge  // ge = tcont
 402a338:	2a010017 	orr	w23, w0, w1
 402a33c:	97fffcde 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a340:	2a0003f8 	mov	w24, w0
 402a344:	97fffcd2 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a348:	2a0003f3 	mov	w19, w0
 402a34c:	97fffccb 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 402a350:	53196000 	lsl	w0, w0, #7
 402a354:	0b132813 	add	w19, w0, w19, lsl #10
 402a358:	11089318 	add	w24, w24, #0x224
 402a35c:	0b180273 	add	w19, w19, w24
 402a360:	b9000277 	str	w23, [x19]
 402a364:	52800013 	mov	w19, #0x0                   	// #0
 402a368:	97fff8a3 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a36c:	b9001c13 	str	w19, [x0, #28]
 402a370:	11000673 	add	w19, w19, #0x1
 402a374:	97fffce6 	bl	402970c <ddrtrn_hal_phy_get_dq_bdl>
 402a378:	0b000280 	add	w0, w20, w0
 402a37c:	7101fc1f 	cmp	w0, #0x7f
 402a380:	1a96d000 	csel	w0, w0, w22, le
 402a384:	7100001f 	cmp	w0, #0x0
 402a388:	1a9fa000 	csel	w0, w0, wzr, ge  // ge = tcont
 402a38c:	97fffd49 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 402a390:	7100227f 	cmp	w19, #0x8
 402a394:	54fffea1 	b.ne	402a368 <ddrtrn_hal_rdqs_sync_rank_rdq+0xa0>  // b.any
 402a398:	97fff897 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a39c:	a94263f7 	ldp	x23, x24, [sp, #32]
 402a3a0:	f9401bfe 	ldr	x30, [sp, #48]
 402a3a4:	b9001415 	str	w21, [x0, #20]
 402a3a8:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402a3ac:	a8c453f3 	ldp	x19, x20, [sp], #64
 402a3b0:	d65f03c0 	ret

000000000402a3b4 <ddrtrn_hal_rdqbdl_adj>:
 402a3b4:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402a3b8:	aa0003f3 	mov	x19, x0
 402a3bc:	a9015bf5 	stp	x21, x22, [sp, #16]
 402a3c0:	a90263f7 	stp	x23, x24, [sp, #32]
 402a3c4:	a9037bf9 	stp	x25, x30, [sp, #48]
 402a3c8:	97fffcb1 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a3cc:	2a0003f4 	mov	w20, w0
 402a3d0:	97fffcb9 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a3d4:	2a0003f6 	mov	w22, w0
 402a3d8:	97fffca8 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 402a3dc:	2a0003f5 	mov	w21, w0
 402a3e0:	97fffcb0 	bl	40296a0 <ddrtrn_hal_get_cur_mode>
 402a3e4:	2a0003f7 	mov	w23, w0
 402a3e8:	97fff883 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a3ec:	53165694 	lsl	w20, w20, #10
 402a3f0:	52800021 	mov	w1, #0x1                   	// #1
 402a3f4:	b9001401 	str	w1, [x0, #20]
 402a3f8:	110892c0 	add	w0, w22, #0x224
 402a3fc:	1108b2d6 	add	w22, w22, #0x22c
 402a400:	0b151e94 	add	w20, w20, w21, lsl #7
 402a404:	d2800019 	mov	x25, #0x0                   	// #0
 402a408:	0b000294 	add	w20, w20, w0
 402a40c:	0b151ed5 	add	w21, w22, w21, lsl #7
 402a410:	b9400298 	ldr	w24, [x20]
 402a414:	b94002b6 	ldr	w22, [x21]
 402a418:	97fff877 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a41c:	b9001c19 	str	w25, [x0, #28]
 402a420:	97fffcbb 	bl	402970c <ddrtrn_hal_phy_get_dq_bdl>
 402a424:	b8397a60 	str	w0, [x19, x25, lsl #2]
 402a428:	91000739 	add	x25, x25, #0x1
 402a42c:	f100233f 	cmp	x25, #0x8
 402a430:	54ffff41 	b.ne	402a418 <ddrtrn_hal_rdqbdl_adj+0x64>  // b.any
 402a434:	12001b00 	and	w0, w24, #0x7f
 402a438:	b9002260 	str	w0, [x19, #32]
 402a43c:	120022c0 	and	w0, w22, #0x1ff
 402a440:	12800001 	mov	w1, #0xffffffff            	// #-1
 402a444:	b9002660 	str	w0, [x19, #36]
 402a448:	d2800000 	mov	x0, #0x0                   	// #0
 402a44c:	b8607a62 	ldr	w2, [x19, x0, lsl #2]
 402a450:	91000400 	add	x0, x0, #0x1
 402a454:	6b02003f 	cmp	w1, w2
 402a458:	1a829021 	csel	w1, w1, w2, ls  // ls = plast
 402a45c:	f100281f 	cmp	x0, #0xa
 402a460:	54ffff61 	b.ne	402a44c <ddrtrn_hal_rdqbdl_adj+0x98>  // b.any
 402a464:	d2800000 	mov	x0, #0x0                   	// #0
 402a468:	b8607a62 	ldr	w2, [x19, x0, lsl #2]
 402a46c:	4b010042 	sub	w2, w2, w1
 402a470:	b8207a62 	str	w2, [x19, x0, lsl #2]
 402a474:	91000400 	add	x0, x0, #0x1
 402a478:	f100281f 	cmp	x0, #0xa
 402a47c:	54ffff61 	b.ne	402a468 <ddrtrn_hal_rdqbdl_adj+0xb4>  // b.any
 402a480:	d2800019 	mov	x25, #0x0                   	// #0
 402a484:	97fff85c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a488:	b9001c19 	str	w25, [x0, #28]
 402a48c:	b8797a60 	ldr	w0, [x19, x25, lsl #2]
 402a490:	91000739 	add	x25, x25, #0x1
 402a494:	97fffd07 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 402a498:	f100233f 	cmp	x25, #0x8
 402a49c:	54ffff41 	b.ne	402a484 <ddrtrn_hal_rdqbdl_adj+0xd0>  // b.any
 402a4a0:	b9402260 	ldr	w0, [x19, #32]
 402a4a4:	12196318 	and	w24, w24, #0xffffff80
 402a4a8:	12175ad6 	and	w22, w22, #0xfffffe00
 402a4ac:	2a000318 	orr	w24, w24, w0
 402a4b0:	b9402660 	ldr	w0, [x19, #36]
 402a4b4:	b9000298 	str	w24, [x20]
 402a4b8:	2a0002d6 	orr	w22, w22, w0
 402a4bc:	b90002b6 	str	w22, [x21]
 402a4c0:	97fff84d 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a4c4:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402a4c8:	a9437bf9 	ldp	x25, x30, [sp, #48]
 402a4cc:	b9001417 	str	w23, [x0, #20]
 402a4d0:	a94263f7 	ldp	x23, x24, [sp, #32]
 402a4d4:	a8c453f3 	ldp	x19, x20, [sp], #64
 402a4d8:	d65f03c0 	ret

000000000402a4dc <ddrtrn_hal_rdqs_sync>:
 402a4dc:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402a4e0:	2a0003f3 	mov	w19, w0
 402a4e4:	a9017bf5 	stp	x21, x30, [sp, #16]
 402a4e8:	97fffc69 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a4ec:	2a0003f5 	mov	w21, w0
 402a4f0:	97fffc71 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a4f4:	2a0003f4 	mov	w20, w0
 402a4f8:	1108b294 	add	w20, w20, #0x22c
 402a4fc:	97fffc5f 	bl	4029678 <ddrtrn_hal_get_cur_byte>
 402a500:	0b001e94 	add	w20, w20, w0, lsl #7
 402a504:	b9400280 	ldr	w0, [x20]
 402a508:	12002000 	and	w0, w0, #0x1ff
 402a50c:	4b000273 	sub	w19, w19, w0
 402a510:	2a1303e0 	mov	w0, w19
 402a514:	97ffff6d 	bl	402a2c8 <ddrtrn_hal_rdqs_sync_rank_rdq>
 402a518:	97fff837 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a51c:	aa0003f4 	mov	x20, x0
 402a520:	97fff830 	bl	40285e0 <ddrtrn_hal_get_phy>
 402a524:	d2800782 	mov	x2, #0x3c                  	// #60
 402a528:	b9402281 	ldr	w1, [x20, #32]
 402a52c:	9b020021 	madd	x1, x1, x2, x0
 402a530:	b9400c20 	ldr	w0, [x1, #12]
 402a534:	7100041f 	cmp	w0, #0x1
 402a538:	54000140 	b.eq	402a560 <ddrtrn_hal_rdqs_sync+0x84>  // b.none
 402a53c:	52800020 	mov	w0, #0x1                   	// #1
 402a540:	4b150000 	sub	w0, w0, w21
 402a544:	97fffc61 	bl	40296c8 <ddrtrn_hal_set_rank_id>
 402a548:	2a1303e0 	mov	w0, w19
 402a54c:	97ffff5f 	bl	402a2c8 <ddrtrn_hal_rdqs_sync_rank_rdq>
 402a550:	2a1503e0 	mov	w0, w21
 402a554:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402a558:	a8c253f3 	ldp	x19, x20, [sp], #32
 402a55c:	17fffc5b 	b	40296c8 <ddrtrn_hal_set_rank_id>
 402a560:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402a564:	a8c253f3 	ldp	x19, x20, [sp], #32
 402a568:	d65f03c0 	ret

000000000402a56c <ddrtrn_hal_save_rdqbdl>:
 402a56c:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402a570:	aa0003f3 	mov	x19, x0
 402a574:	a9017bf5 	stp	x21, x30, [sp, #16]
 402a578:	97fffc4f 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a57c:	2a0003f4 	mov	w20, w0
 402a580:	97fffc43 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a584:	2a0003f5 	mov	w21, w0
 402a588:	97fffc56 	bl	40296e0 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402a58c:	0b152a84 	add	w4, w20, w21, lsl #10
 402a590:	aa1303e1 	mov	x1, x19
 402a594:	11087083 	add	w3, w4, #0x21c
 402a598:	0b001c84 	add	w4, w4, w0, lsl #7
 402a59c:	1108b282 	add	w2, w20, #0x22c
 402a5a0:	11087084 	add	w4, w4, #0x21c
 402a5a4:	6b04007f 	cmp	w3, w4
 402a5a8:	54000081 	b.ne	402a5b8 <ddrtrn_hal_save_rdqbdl+0x4c>  // b.any
 402a5ac:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402a5b0:	a8c253f3 	ldp	x19, x20, [sp], #32
 402a5b4:	d65f03c0 	ret
 402a5b8:	2a0303e0 	mov	w0, w3
 402a5bc:	91001021 	add	x1, x1, #0x4
 402a5c0:	b9400000 	ldr	w0, [x0]
 402a5c4:	b81fc020 	stur	w0, [x1, #-4]
 402a5c8:	11001060 	add	w0, w3, #0x4
 402a5cc:	b9400000 	ldr	w0, [x0]
 402a5d0:	b9000c20 	str	w0, [x1, #12]
 402a5d4:	11002060 	add	w0, w3, #0x8
 402a5d8:	11020063 	add	w3, w3, #0x80
 402a5dc:	b9400000 	ldr	w0, [x0]
 402a5e0:	b9002c20 	str	w0, [x1, #44]
 402a5e4:	2a0203e0 	mov	w0, w2
 402a5e8:	11020042 	add	w2, w2, #0x80
 402a5ec:	b9400000 	ldr	w0, [x0]
 402a5f0:	b9001c20 	str	w0, [x1, #28]
 402a5f4:	17ffffec 	b	402a5a4 <ddrtrn_hal_save_rdqbdl+0x38>

000000000402a5f8 <ddrtrn_hal_restore_rdqbdl>:
 402a5f8:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402a5fc:	aa0003f3 	mov	x19, x0
 402a600:	a9017bf5 	stp	x21, x30, [sp, #16]
 402a604:	97fffc2c 	bl	40296b4 <ddrtrn_hal_get_cur_phy>
 402a608:	2a0003f4 	mov	w20, w0
 402a60c:	97fffc20 	bl	402968c <ddrtrn_hal_get_rank_id>
 402a610:	2a0003f5 	mov	w21, w0
 402a614:	97fffc33 	bl	40296e0 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402a618:	0b152a84 	add	w4, w20, w21, lsl #10
 402a61c:	aa1303e1 	mov	x1, x19
 402a620:	11087083 	add	w3, w4, #0x21c
 402a624:	0b001c84 	add	w4, w4, w0, lsl #7
 402a628:	1108b282 	add	w2, w20, #0x22c
 402a62c:	11087084 	add	w4, w4, #0x21c
 402a630:	6b04007f 	cmp	w3, w4
 402a634:	54000081 	b.ne	402a644 <ddrtrn_hal_restore_rdqbdl+0x4c>  // b.any
 402a638:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402a63c:	a8c253f3 	ldp	x19, x20, [sp], #32
 402a640:	d65f03c0 	ret
 402a644:	2a0303e0 	mov	w0, w3
 402a648:	b9400025 	ldr	w5, [x1]
 402a64c:	91001021 	add	x1, x1, #0x4
 402a650:	b9000005 	str	w5, [x0]
 402a654:	11001060 	add	w0, w3, #0x4
 402a658:	b9400c25 	ldr	w5, [x1, #12]
 402a65c:	b9000005 	str	w5, [x0]
 402a660:	11002060 	add	w0, w3, #0x8
 402a664:	11020063 	add	w3, w3, #0x80
 402a668:	b9402c25 	ldr	w5, [x1, #44]
 402a66c:	b9000005 	str	w5, [x0]
 402a670:	2a0203e0 	mov	w0, w2
 402a674:	11020042 	add	w2, w2, #0x80
 402a678:	b9401c25 	ldr	w5, [x1, #28]
 402a67c:	b9000005 	str	w5, [x0]
 402a680:	17ffffec 	b	402a630 <ddrtrn_hal_restore_rdqbdl+0x38>

000000000402a684 <ddrtrn_hal_get_gt_status>:
 402a684:	11002000 	add	w0, w0, #0x8
 402a688:	b9400000 	ldr	w0, [x0]
 402a68c:	121b0000 	and	w0, w0, #0x20
 402a690:	d65f03c0 	ret

000000000402a694 <ddrtrn_hal_get_trfc_ctrl_val>:
 402a694:	11238000 	add	w0, w0, #0x8e0
 402a698:	b9400000 	ldr	w0, [x0]
 402a69c:	d65f03c0 	ret

000000000402a6a0 <ddrtrn_hal_set_trfc_ctrl>:
 402a6a0:	11238021 	add	w1, w1, #0x8e0
 402a6a4:	b9000020 	str	w0, [x1]
 402a6a8:	d65f03c0 	ret

000000000402a6ac <ddrtrn_hal_get_misc_val>:
 402a6ac:	1101c000 	add	w0, w0, #0x70
 402a6b0:	b9400000 	ldr	w0, [x0]
 402a6b4:	d65f03c0 	ret

000000000402a6b8 <ddrtrn_hal_set_misc_val>:
 402a6b8:	1101c021 	add	w1, w1, #0x70
 402a6bc:	b9000020 	str	w0, [x1]
 402a6c0:	d65f03c0 	ret

000000000402a6c4 <ddrtrn_hal_get_trfc_threshold1_val>:
 402a6c4:	11234000 	add	w0, w0, #0x8d0
 402a6c8:	b9400000 	ldr	w0, [x0]
 402a6cc:	d65f03c0 	ret

000000000402a6d0 <ddrtrn_hal_get_dmsel>:
 402a6d0:	11021000 	add	w0, w0, #0x84
 402a6d4:	b9400000 	ldr	w0, [x0]
 402a6d8:	d65f03c0 	ret

000000000402a6dc <ddrtrn_hal_get_phyctrl0>:
 402a6dc:	1101e000 	add	w0, w0, #0x78
 402a6e0:	b9400000 	ldr	w0, [x0]
 402a6e4:	d65f03c0 	ret

000000000402a6e8 <ddrtrn_hal_read_repeatedly>:
 402a6e8:	0b010000 	add	w0, w0, w1
 402a6ec:	b9400001 	ldr	w1, [x0]
 402a6f0:	b9400001 	ldr	w1, [x0]
 402a6f4:	b9400000 	ldr	w0, [x0]
 402a6f8:	d65f03c0 	ret

000000000402a6fc <ddrtrn_hal_enable_rdqs_anti_aging>:
 402a6fc:	1102c000 	add	w0, w0, #0xb0
 402a700:	b9400001 	ldr	w1, [x0]
 402a704:	32010021 	orr	w1, w1, #0x80000000
 402a708:	b9000001 	str	w1, [x0]
 402a70c:	d65f03c0 	ret

000000000402a710 <ddrtrn_hal_vref_get_host_max>:
 402a710:	7100001f 	cmp	w0, #0x0
 402a714:	528001e2 	mov	w2, #0xf                   	// #15
 402a718:	52800fe0 	mov	w0, #0x7f                  	// #127
 402a71c:	1a820000 	csel	w0, w0, w2, eq  // eq = none
 402a720:	b9000020 	str	w0, [x1]
 402a724:	d65f03c0 	ret

000000000402a728 <ddrtrn_hal_vref_phy_host_get>:
 402a728:	53196042 	lsl	w2, w2, #7
 402a72c:	1109d042 	add	w2, w2, #0x274
 402a730:	350000c1 	cbnz	w1, 402a748 <ddrtrn_hal_vref_phy_host_get+0x20>
 402a734:	2a0203e1 	mov	w1, w2
 402a738:	b8606820 	ldr	w0, [x1, x0]
 402a73c:	12001800 	and	w0, w0, #0x7f
 402a740:	b9000060 	str	w0, [x3]
 402a744:	d65f03c0 	ret
 402a748:	0b012841 	add	w1, w2, w1, lsl #10
 402a74c:	b8606820 	ldr	w0, [x1, x0]
 402a750:	d3586c00 	ubfx	x0, x0, #24, #4
 402a754:	17fffffb 	b	402a740 <ddrtrn_hal_vref_phy_host_get+0x18>

000000000402a758 <ddrtrn_hal_vref_phy_dram_get>:
 402a758:	53196042 	lsl	w2, w2, #7
 402a75c:	1109c042 	add	w2, w2, #0x270
 402a760:	b8606840 	ldr	w0, [x2, x0]
 402a764:	12001400 	and	w0, w0, #0x3f
 402a768:	b9000020 	str	w0, [x1]
 402a76c:	d65f03c0 	ret

000000000402a770 <ddrtrn_hal_phy_rdqs_sync_rdm>:
 402a770:	17ffff5b 	b	402a4dc <ddrtrn_hal_rdqs_sync>

000000000402a774 <ddrtrn_hal_dqsswap_save_func>:
 402a774:	b9408422 	ldr	w2, [x1, #132]
 402a778:	b9000002 	str	w2, [x0]
 402a77c:	12057042 	and	w2, w2, #0xf8ffffff
 402a780:	b9008422 	str	w2, [x1, #132]
 402a784:	d65f03c0 	ret

000000000402a788 <ddrtrn_hal_dqsswap_restore_func>:
 402a788:	b9008420 	str	w0, [x1, #132]
 402a78c:	d65f03c0 	ret

000000000402a790 <ddrtrn_hal_phy_switch_rank>:
 402a790:	11012004 	add	w4, w0, #0x48
 402a794:	52826285 	mov	w5, #0x1314                	// #4884
 402a798:	0b050002 	add	w2, w0, w5
 402a79c:	b9400083 	ldr	w3, [x4]
 402a7a0:	121c6c63 	and	w3, w3, #0xfffffff0
 402a7a4:	2a010063 	orr	w3, w3, w1
 402a7a8:	b9000083 	str	w3, [x4]
 402a7ac:	11032004 	add	w4, w0, #0xc8
 402a7b0:	b9400083 	ldr	w3, [x4]
 402a7b4:	12007863 	and	w3, w3, #0x7fffffff
 402a7b8:	2a017c63 	orr	w3, w3, w1, lsl #31
 402a7bc:	b9000083 	str	w3, [x4]
 402a7c0:	52824283 	mov	w3, #0x1214                	// #4628
 402a7c4:	0b030004 	add	w4, w0, w3
 402a7c8:	b9400083 	ldr	w3, [x4]
 402a7cc:	12117863 	and	w3, w3, #0xffffbfff
 402a7d0:	2a013863 	orr	w3, w3, w1, lsl #14
 402a7d4:	b9000083 	str	w3, [x4]
 402a7d8:	b9400043 	ldr	w3, [x2]
 402a7dc:	12117863 	and	w3, w3, #0xffffbfff
 402a7e0:	2a013861 	orr	w1, w3, w1, lsl #14
 402a7e4:	b9000041 	str	w1, [x2]
 402a7e8:	17fffc1b 	b	4029854 <ddrtrn_hal_phy_cfg_update>

000000000402a7ec <ddrtrn_hal_get_sysctrl_cfg>:
 402a7ec:	52a22041 	mov	w1, #0x11020000            	// #285343744
 402a7f0:	0b010000 	add	w0, w0, w1
 402a7f4:	b9400000 	ldr	w0, [x0]
 402a7f8:	d65f03c0 	ret

000000000402a7fc <ddrtrn_hal_set_sysctrl_cfg>:
 402a7fc:	d2801401 	mov	x1, #0xa0                  	// #160
 402a800:	f2a22041 	movk	x1, #0x1102, lsl #16
 402a804:	b9000020 	str	w0, [x1]
 402a808:	d65f03c0 	ret

000000000402a80c <ddrtrn_hal_version_flag>:
 402a80c:	d2801681 	mov	x1, #0xb4                  	// #180
 402a810:	f2a22041 	movk	x1, #0x1102, lsl #16
 402a814:	b9400020 	ldr	w0, [x1]
 402a818:	72804460 	movk	w0, #0x223
 402a81c:	b9000020 	str	w0, [x1]
 402a820:	d65f03c0 	ret

000000000402a824 <ddrtrn_hal_check_sw_item>:
 402a824:	d2801400 	mov	x0, #0xa0                  	// #160
 402a828:	f2a22040 	movk	x0, #0x1102, lsl #16
 402a82c:	b9400000 	ldr	w0, [x0]
 402a830:	3100041f 	cmn	w0, #0x1
 402a834:	540000e1 	b.ne	402a850 <ddrtrn_hal_check_sw_item+0x2c>  // b.any
 402a838:	d2801480 	mov	x0, #0xa4                  	// #164
 402a83c:	f2a22040 	movk	x0, #0x1102, lsl #16
 402a840:	b9400000 	ldr	w0, [x0]
 402a844:	3100041f 	cmn	w0, #0x1
 402a848:	5a9f03e0 	csetm	w0, ne  // ne = any
 402a84c:	d65f03c0 	ret
 402a850:	12800000 	mov	w0, #0xffffffff            	// #-1
 402a854:	17fffffe 	b	402a84c <ddrtrn_hal_check_sw_item+0x28>

000000000402a858 <ddrtrn_hal_check_not_dcc_item>:
 402a858:	d2801400 	mov	x0, #0xa0                  	// #160
 402a85c:	f2a22040 	movk	x0, #0x1102, lsl #16
 402a860:	b9400001 	ldr	w1, [x0]
 402a864:	529fff00 	mov	w0, #0xfff8                	// #65528
 402a868:	72beffe0 	movk	w0, #0xf7ff, lsl #16
 402a86c:	0a000022 	and	w2, w1, w0
 402a870:	6a21001f 	bics	wzr, w0, w1
 402a874:	540000e1 	b.ne	402a890 <ddrtrn_hal_check_not_dcc_item+0x38>  // b.any
 402a878:	d2801480 	mov	x0, #0xa4                  	// #164
 402a87c:	f2a22040 	movk	x0, #0x1102, lsl #16
 402a880:	b9400000 	ldr	w0, [x0]
 402a884:	6a20005f 	bics	wzr, w2, w0
 402a888:	5a9f13e0 	csetm	w0, eq  // eq = none
 402a88c:	d65f03c0 	ret
 402a890:	52800000 	mov	w0, #0x0                   	// #0
 402a894:	17fffffe 	b	402a88c <ddrtrn_hal_check_not_dcc_item+0x34>

000000000402a898 <ddrtrn_hal_set_adjust>:
 402a898:	d2801801 	mov	x1, #0xc0                  	// #192
 402a89c:	f2a22041 	movk	x1, #0x1102, lsl #16
 402a8a0:	b9000020 	str	w0, [x1]
 402a8a4:	d65f03c0 	ret

000000000402a8a8 <ddrtrn_hal_get_adjust>:
 402a8a8:	d2801800 	mov	x0, #0xc0                  	// #192
 402a8ac:	f2a22040 	movk	x0, #0x1102, lsl #16
 402a8b0:	b9400000 	ldr	w0, [x0]
 402a8b4:	d65f03c0 	ret

000000000402a8b8 <ddrtrn_hal_clear_sysctrl_stat_reg>:
 402a8b8:	d2801600 	mov	x0, #0xb0                  	// #176
 402a8bc:	f2a22040 	movk	x0, #0x1102, lsl #16
 402a8c0:	b900001f 	str	wzr, [x0]
 402a8c4:	d65f03c0 	ret

000000000402a8c8 <ddrtrn_wl_func>:
 402a8c8:	52800000 	mov	w0, #0x0                   	// #0
 402a8cc:	d65f03c0 	ret

000000000402a8d0 <ddrtrn_training_console>:
 402a8d0:	52800000 	mov	w0, #0x0                   	// #0
 402a8d4:	d65f03c0 	ret

000000000402a8d8 <ddrtrn_training_console_if>:
 402a8d8:	52800000 	mov	w0, #0x0                   	// #0
 402a8dc:	d65f03c0 	ret

000000000402a8e0 <ddrtrn_copy_data>:
 402a8e0:	2a0203e2 	mov	w2, w2
 402a8e4:	d2800003 	mov	x3, #0x0                   	// #0
 402a8e8:	eb02007f 	cmp	x3, x2
 402a8ec:	54000041 	b.ne	402a8f4 <ddrtrn_copy_data+0x14>  // b.any
 402a8f0:	d65f03c0 	ret
 402a8f4:	38636824 	ldrb	w4, [x1, x3]
 402a8f8:	38236804 	strb	w4, [x0, x3]
 402a8fc:	91000463 	add	x3, x3, #0x1
 402a900:	17fffffa 	b	402a8e8 <ddrtrn_copy_data+0x8>

000000000402a904 <ddrtrn_set_data>:
 402a904:	2a0203e2 	mov	w2, w2
 402a908:	d2800003 	mov	x3, #0x0                   	// #0
 402a90c:	eb02007f 	cmp	x3, x2
 402a910:	54000041 	b.ne	402a918 <ddrtrn_set_data+0x14>  // b.any
 402a914:	d65f03c0 	ret
 402a918:	38236801 	strb	w1, [x0, x3]
 402a91c:	91000463 	add	x3, x3, #0x1
 402a920:	17fffffb 	b	402a90c <ddrtrn_set_data+0x8>

000000000402a924 <ddrtrn_rdq_offset_cfg_by_phy>:
 402a924:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402a928:	52800013 	mov	w19, #0x0                   	// #0
 402a92c:	a9015bf5 	stp	x21, x22, [sp, #16]
 402a930:	d2800796 	mov	x22, #0x3c                  	// #60
 402a934:	a90263f7 	stp	x23, x24, [sp, #32]
 402a938:	f9001bfe 	str	x30, [sp, #48]
 402a93c:	97fff72e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a940:	b9400000 	ldr	w0, [x0]
 402a944:	6b00027f 	cmp	w19, w0
 402a948:	540000c3 	b.cc	402a960 <ddrtrn_rdq_offset_cfg_by_phy+0x3c>  // b.lo, b.ul, b.last
 402a94c:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402a950:	a94263f7 	ldp	x23, x24, [sp, #32]
 402a954:	f9401bfe 	ldr	x30, [sp, #48]
 402a958:	a8c453f3 	ldp	x19, x20, [sp], #64
 402a95c:	d65f03c0 	ret
 402a960:	97fff725 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a964:	b9002013 	str	w19, [x0, #32]
 402a968:	97fff71e 	bl	40285e0 <ddrtrn_hal_get_phy>
 402a96c:	2a1303e1 	mov	w1, w19
 402a970:	9b167c21 	mul	x1, x1, x22
 402a974:	b8616814 	ldr	w20, [x0, x1]
 402a978:	97fff71f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a97c:	b9000414 	str	w20, [x0, #4]
 402a980:	97fff71d 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a984:	aa0003f4 	mov	x20, x0
 402a988:	97fff716 	bl	40285e0 <ddrtrn_hal_get_phy>
 402a98c:	b9402281 	ldr	w1, [x20, #32]
 402a990:	52800014 	mov	w20, #0x0                   	// #0
 402a994:	9b160021 	madd	x1, x1, x22, x0
 402a998:	b9400c37 	ldr	w23, [x1, #12]
 402a99c:	6b17029f 	cmp	w20, w23
 402a9a0:	54000061 	b.ne	402a9ac <ddrtrn_rdq_offset_cfg_by_phy+0x88>  // b.any
 402a9a4:	11000673 	add	w19, w19, #0x1
 402a9a8:	17ffffe5 	b	402a93c <ddrtrn_rdq_offset_cfg_by_phy+0x18>
 402a9ac:	97fff712 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a9b0:	b9002414 	str	w20, [x0, #36]
 402a9b4:	97fff710 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a9b8:	aa0003f5 	mov	x21, x0
 402a9bc:	97fff709 	bl	40285e0 <ddrtrn_hal_get_phy>
 402a9c0:	b94022a1 	ldr	w1, [x21, #32]
 402a9c4:	52800015 	mov	w21, #0x0                   	// #0
 402a9c8:	9b160021 	madd	x1, x1, x22, x0
 402a9cc:	b9401038 	ldr	w24, [x1, #16]
 402a9d0:	6b1802bf 	cmp	w21, w24
 402a9d4:	54000061 	b.ne	402a9e0 <ddrtrn_rdq_offset_cfg_by_phy+0xbc>  // b.any
 402a9d8:	11000694 	add	w20, w20, #0x1
 402a9dc:	17fffff0 	b	402a99c <ddrtrn_rdq_offset_cfg_by_phy+0x78>
 402a9e0:	97fff705 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402a9e4:	b9001815 	str	w21, [x0, #24]
 402a9e8:	52800060 	mov	w0, #0x3                   	// #3
 402a9ec:	110006b5 	add	w21, w21, #0x1
 402a9f0:	97fffe36 	bl	402a2c8 <ddrtrn_hal_rdqs_sync_rank_rdq>
 402a9f4:	17fffff7 	b	402a9d0 <ddrtrn_rdq_offset_cfg_by_phy+0xac>

000000000402a9f8 <ddrtrn_gating_func>:
 402a9f8:	52800000 	mov	w0, #0x0                   	// #0
 402a9fc:	d65f03c0 	ret

000000000402aa00 <ddrtrn_hal_get_cur_dmc_addr>:
 402aa00:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402aa04:	2a0003f3 	mov	w19, w0
 402aa08:	f9000bfe 	str	x30, [sp, #16]
 402aa0c:	97fff6fa 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402aa10:	aa0003f4 	mov	x20, x0
 402aa14:	97fff6f3 	bl	40285e0 <ddrtrn_hal_get_phy>
 402aa18:	d2800782 	mov	x2, #0x3c                  	// #60
 402aa1c:	f9400bfe 	ldr	x30, [sp, #16]
 402aa20:	b9402281 	ldr	w1, [x20, #32]
 402aa24:	9b027c21 	mul	x1, x1, x2
 402aa28:	d2800182 	mov	x2, #0xc                   	// #12
 402aa2c:	9b020673 	madd	x19, x19, x2, x1
 402aa30:	8b130013 	add	x19, x0, x19
 402aa34:	b9401660 	ldr	w0, [x19, #20]
 402aa38:	a8c253f3 	ldp	x19, x20, [sp], #32
 402aa3c:	d65f03c0 	ret

000000000402aa40 <ddrtrn_hal_get_cur_phy_dmc_num>:
 402aa40:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402aa44:	97fff6ec 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402aa48:	aa0003f3 	mov	x19, x0
 402aa4c:	97fff6e5 	bl	40285e0 <ddrtrn_hal_get_phy>
 402aa50:	d2800782 	mov	x2, #0x3c                  	// #60
 402aa54:	b9402261 	ldr	w1, [x19, #32]
 402aa58:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402aa5c:	d503201f 	nop
 402aa60:	9b020021 	madd	x1, x1, x2, x0
 402aa64:	b9400820 	ldr	w0, [x1, #8]
 402aa68:	d65f03c0 	ret

000000000402aa6c <ddrtrn_sref_cfg_save>:
 402aa6c:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402aa70:	aa0003f4 	mov	x20, x0
 402aa74:	52800013 	mov	w19, #0x0                   	// #0
 402aa78:	f9000bfe 	str	x30, [sp, #16]
 402aa7c:	97fffff1 	bl	402aa40 <ddrtrn_hal_get_cur_phy_dmc_num>
 402aa80:	6b13001f 	cmp	w0, w19
 402aa84:	54000088 	b.hi	402aa94 <ddrtrn_sref_cfg_save+0x28>  // b.pmore
 402aa88:	f9400bfe 	ldr	x30, [sp, #16]
 402aa8c:	a8c253f3 	ldp	x19, x20, [sp], #32
 402aa90:	d65f03c0 	ret
 402aa94:	2a1303e0 	mov	w0, w19
 402aa98:	97ffffda 	bl	402aa00 <ddrtrn_hal_get_cur_dmc_addr>
 402aa9c:	11008000 	add	w0, w0, #0x20
 402aaa0:	b9400000 	ldr	w0, [x0]
 402aaa4:	b8335a80 	str	w0, [x20, w19, uxtw #2]
 402aaa8:	11000673 	add	w19, w19, #0x1
 402aaac:	17fffff4 	b	402aa7c <ddrtrn_sref_cfg_save+0x10>

000000000402aab0 <ddrtrn_sref_cfg_restore>:
 402aab0:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402aab4:	aa0003f4 	mov	x20, x0
 402aab8:	52800013 	mov	w19, #0x0                   	// #0
 402aabc:	f9000bfe 	str	x30, [sp, #16]
 402aac0:	97ffffe0 	bl	402aa40 <ddrtrn_hal_get_cur_phy_dmc_num>
 402aac4:	6b13001f 	cmp	w0, w19
 402aac8:	54000088 	b.hi	402aad8 <ddrtrn_sref_cfg_restore+0x28>  // b.pmore
 402aacc:	f9400bfe 	ldr	x30, [sp, #16]
 402aad0:	a8c253f3 	ldp	x19, x20, [sp], #32
 402aad4:	d65f03c0 	ret
 402aad8:	b8735a81 	ldr	w1, [x20, w19, uxtw #2]
 402aadc:	2a1303e0 	mov	w0, w19
 402aae0:	11000673 	add	w19, w19, #0x1
 402aae4:	97fffa2f 	bl	40293a0 <ddrtrn_hal_dmc_set_sref_cfg>
 402aae8:	17fffff6 	b	402aac0 <ddrtrn_sref_cfg_restore+0x10>

000000000402aaec <ddrtrn_training_ctrl_easr>:
 402aaec:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402aaf0:	a9017bf5 	stp	x21, x30, [sp, #16]
 402aaf4:	2a0003f5 	mov	w21, w0
 402aaf8:	97ffffd2 	bl	402aa40 <ddrtrn_hal_get_cur_phy_dmc_num>
 402aafc:	7100081f 	cmp	w0, #0x2
 402ab00:	54000228 	b.hi	402ab44 <ddrtrn_training_ctrl_easr+0x58>  // b.pmore
 402ab04:	52800014 	mov	w20, #0x0                   	// #0
 402ab08:	52800013 	mov	w19, #0x0                   	// #0
 402ab0c:	97ffffcd 	bl	402aa40 <ddrtrn_hal_get_cur_phy_dmc_num>
 402ab10:	6b14001f 	cmp	w0, w20
 402ab14:	540000a8 	b.hi	402ab28 <ddrtrn_training_ctrl_easr+0x3c>  // b.pmore
 402ab18:	2a1303e0 	mov	w0, w19
 402ab1c:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402ab20:	a8c253f3 	ldp	x19, x20, [sp], #32
 402ab24:	d65f03c0 	ret
 402ab28:	2a1403e0 	mov	w0, w20
 402ab2c:	97ffffb5 	bl	402aa00 <ddrtrn_hal_get_cur_dmc_addr>
 402ab30:	2a1503e1 	mov	w1, w21
 402ab34:	11000694 	add	w20, w20, #0x1
 402ab38:	97fff9d3 	bl	4029284 <ddrtrn_hal_ddrc_easr>
 402ab3c:	0b000273 	add	w19, w19, w0
 402ab40:	17fffff3 	b	402ab0c <ddrtrn_training_ctrl_easr+0x20>
 402ab44:	12800013 	mov	w19, #0xffffffff            	// #-1
 402ab48:	17fffff4 	b	402ab18 <ddrtrn_training_ctrl_easr+0x2c>

000000000402ab4c <ddrtrn_training_restore_timing>:
 402ab4c:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402ab50:	aa0003f4 	mov	x20, x0
 402ab54:	52800013 	mov	w19, #0x0                   	// #0
 402ab58:	f9000bfe 	str	x30, [sp, #16]
 402ab5c:	97ffffb9 	bl	402aa40 <ddrtrn_hal_get_cur_phy_dmc_num>
 402ab60:	6b13001f 	cmp	w0, w19
 402ab64:	54000088 	b.hi	402ab74 <ddrtrn_training_restore_timing+0x28>  // b.pmore
 402ab68:	f9400bfe 	ldr	x30, [sp, #16]
 402ab6c:	a8c253f3 	ldp	x19, x20, [sp], #32
 402ab70:	d65f03c0 	ret
 402ab74:	2a1303e0 	mov	w0, w19
 402ab78:	97ffffa2 	bl	402aa00 <ddrtrn_hal_get_cur_dmc_addr>
 402ab7c:	b8735a81 	ldr	w1, [x20, w19, uxtw #2]
 402ab80:	11000673 	add	w19, w19, #0x1
 402ab84:	97fff93b 	bl	4029070 <ddrtrn_hal_set_timing>
 402ab88:	17fffff5 	b	402ab5c <ddrtrn_training_restore_timing+0x10>

000000000402ab8c <ddrtrn_hal_timing8_trfc_ab_cfg>:
 402ab8c:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402ab90:	aa0003f3 	mov	x19, x0
 402ab94:	a9015bf5 	stp	x21, x22, [sp, #16]
 402ab98:	52800015 	mov	w21, #0x0                   	// #0
 402ab9c:	a90263f7 	stp	x23, x24, [sp, #32]
 402aba0:	f9001bfe 	str	x30, [sp, #48]
 402aba4:	97fff694 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402aba8:	b9400018 	ldr	w24, [x0]
 402abac:	6b1802bf 	cmp	w21, w24
 402abb0:	540000c1 	b.ne	402abc8 <ddrtrn_hal_timing8_trfc_ab_cfg+0x3c>  // b.any
 402abb4:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402abb8:	a94263f7 	ldp	x23, x24, [sp, #32]
 402abbc:	f9401bfe 	ldr	x30, [sp, #48]
 402abc0:	a8c453f3 	ldp	x19, x20, [sp], #64
 402abc4:	d65f03c0 	ret
 402abc8:	97fff68b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402abcc:	b9002015 	str	w21, [x0, #32]
 402abd0:	52800014 	mov	w20, #0x0                   	// #0
 402abd4:	97ffff9b 	bl	402aa40 <ddrtrn_hal_get_cur_phy_dmc_num>
 402abd8:	2a0003f6 	mov	w22, w0
 402abdc:	6b1402df 	cmp	w22, w20
 402abe0:	540001c0 	b.eq	402ac18 <ddrtrn_hal_timing8_trfc_ab_cfg+0x8c>  // b.none
 402abe4:	97fff684 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402abe8:	b9002814 	str	w20, [x0, #40]
 402abec:	2a1403e0 	mov	w0, w20
 402abf0:	97ffff84 	bl	402aa00 <ddrtrn_hal_get_cur_dmc_addr>
 402abf4:	2a0003f7 	mov	w23, w0
 402abf8:	97fff67f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402abfc:	b9000817 	str	w23, [x0, #8]
 402ac00:	97fffa92 	bl	4029648 <ddrtrn_hal_ddrt_get_mem_width>
 402ac04:	7100081f 	cmp	w0, #0x2
 402ac08:	540001a1 	b.ne	402ac3c <ddrtrn_hal_timing8_trfc_ab_cfg+0xb0>  // b.any
 402ac0c:	b9400260 	ldr	w0, [x19]
 402ac10:	711ffc1f 	cmp	w0, #0x7ff
 402ac14:	54000089 	b.ls	402ac24 <ddrtrn_hal_timing8_trfc_ab_cfg+0x98>  // b.plast
 402ac18:	110006b5 	add	w21, w21, #0x1
 402ac1c:	91003273 	add	x19, x19, #0xc
 402ac20:	17ffffe3 	b	402abac <ddrtrn_hal_timing8_trfc_ab_cfg+0x20>
 402ac24:	b9400660 	ldr	w0, [x19, #4]
 402ac28:	711ffc1f 	cmp	w0, #0x7ff
 402ac2c:	54ffff68 	b.hi	402ac18 <ddrtrn_hal_timing8_trfc_ab_cfg+0x8c>  // b.pmore
 402ac30:	11000694 	add	w20, w20, #0x1
 402ac34:	97fffa03 	bl	4029440 <ddrtrn_hal_timing8_trfc_ab_cfg_by_dmc>
 402ac38:	17ffffe9 	b	402abdc <ddrtrn_hal_timing8_trfc_ab_cfg+0x50>
 402ac3c:	b9400a60 	ldr	w0, [x19, #8]
 402ac40:	17fffffa 	b	402ac28 <ddrtrn_hal_timing8_trfc_ab_cfg+0x9c>

000000000402ac44 <ddrtrn_hal_get_cur_phy_total_byte_num>:
 402ac44:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402ac48:	97fff66b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ac4c:	aa0003f3 	mov	x19, x0
 402ac50:	97fff664 	bl	40285e0 <ddrtrn_hal_get_phy>
 402ac54:	d2800782 	mov	x2, #0x3c                  	// #60
 402ac58:	b9402261 	ldr	w1, [x19, #32]
 402ac5c:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402ac60:	d503201f 	nop
 402ac64:	9b020021 	madd	x1, x1, x2, x0
 402ac68:	b9401020 	ldr	w0, [x1, #16]
 402ac6c:	d65f03c0 	ret

000000000402ac70 <ddrtrn_hw_dataeye_read>:
 402ac70:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402ac74:	52800013 	mov	w19, #0x0                   	// #0
 402ac78:	a9017bf5 	stp	x21, x30, [sp, #16]
 402ac7c:	97fff65e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ac80:	b9400415 	ldr	w21, [x0, #4]
 402ac84:	97fffff0 	bl	402ac44 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402ac88:	2a0003f4 	mov	w20, w0
 402ac8c:	97fff666 	bl	4028624 <ddrtrn_hal_cfg_init>
 402ac90:	6b14027f 	cmp	w19, w20
 402ac94:	54000161 	b.ne	402acc0 <ddrtrn_hw_dataeye_read+0x50>  // b.any
 402ac98:	2a1503e0 	mov	w0, w21
 402ac9c:	97fffaee 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 402aca0:	52802000 	mov	w0, #0x100                 	// #256
 402aca4:	94000ae4 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402aca8:	2a0003f3 	mov	w19, w0
 402acac:	94000aac 	bl	402d75c <ddrtrn_hal_hw_read_adj>
 402acb0:	2a1303e0 	mov	w0, w19
 402acb4:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402acb8:	a8c253f3 	ldp	x19, x20, [sp], #32
 402acbc:	d65f03c0 	ret
 402acc0:	2a1303e0 	mov	w0, w19
 402acc4:	11000673 	add	w19, w19, #0x1
 402acc8:	94000c27 	bl	402dd64 <ddrtrn_hal_hw_clear_rdq>
 402accc:	17fffff1 	b	402ac90 <ddrtrn_hw_dataeye_read+0x20>

000000000402acd0 <ddrtrn_hw_training_by_phy>:
 402acd0:	d10383ff 	sub	sp, sp, #0xe0
 402acd4:	f0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402acd8:	a90853f3 	stp	x19, x20, [sp, #128]
 402acdc:	52800014 	mov	w20, #0x0                   	// #0
 402ace0:	a9095bf5 	stp	x21, x22, [sp, #144]
 402ace4:	52800016 	mov	w22, #0x0                   	// #0
 402ace8:	a90a63f7 	stp	x23, x24, [sp, #160]
 402acec:	d2800798 	mov	x24, #0x3c                  	// #60
 402acf0:	9100a3f7 	add	x23, sp, #0x28
 402acf4:	a90b6bf9 	stp	x25, x26, [sp, #176]
 402acf8:	a90c73fb 	stp	x27, x28, [sp, #192]
 402acfc:	f9006bfe 	str	x30, [sp, #208]
 402ad00:	f9455001 	ldr	x1, [x0, #2720]
 402ad04:	f9003fe1 	str	x1, [sp, #120]
 402ad08:	d2800001 	mov	x1, #0x0                   	// #0
 402ad0c:	97fff63a 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ad10:	aa0003f3 	mov	x19, x0
 402ad14:	97fff633 	bl	40285e0 <ddrtrn_hal_get_phy>
 402ad18:	b9402261 	ldr	w1, [x19, #32]
 402ad1c:	9b180021 	madd	x1, x1, x24, x0
 402ad20:	910043e0 	add	x0, sp, #0x10
 402ad24:	b9400c39 	ldr	w25, [x1, #12]
 402ad28:	97fff979 	bl	402930c <ddrtrn_hal_save_timing>
 402ad2c:	6b19029f 	cmp	w20, w25
 402ad30:	54000401 	b.ne	402adb0 <ddrtrn_hw_training_by_phy+0xe0>  // b.any
 402ad34:	71000a9f 	cmp	w20, #0x2
 402ad38:	540002a1 	b.ne	402ad8c <ddrtrn_hw_training_by_phy+0xbc>  // b.any
 402ad3c:	97ffffc2 	bl	402ac44 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402ad40:	2a0003f5 	mov	w21, w0
 402ad44:	97fff62c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ad48:	b9402417 	ldr	w23, [x0, #36]
 402ad4c:	910063f4 	add	x20, sp, #0x18
 402ad50:	52800013 	mov	w19, #0x0                   	// #0
 402ad54:	91001294 	add	x20, x20, #0x4
 402ad58:	6b1302bf 	cmp	w21, w19
 402ad5c:	54000ea1 	b.ne	402af30 <ddrtrn_hw_training_by_phy+0x260>  // b.any
 402ad60:	97fff625 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ad64:	b9002417 	str	w23, [x0, #36]
 402ad68:	97fff623 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ad6c:	b9400400 	ldr	w0, [x0, #4]
 402ad70:	97fffab9 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 402ad74:	97fffbb4 	bl	4029c44 <ddrtrn_hal_training_adjust_wdq>
 402ad78:	97fffc65 	bl	4029f0c <ddrtrn_hal_training_adjust_wdqs>
 402ad7c:	97fff61e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ad80:	b9400400 	ldr	w0, [x0, #4]
 402ad84:	52800001 	mov	w1, #0x0                   	// #0
 402ad88:	97fffe82 	bl	402a790 <ddrtrn_hal_phy_switch_rank>
 402ad8c:	910043e0 	add	x0, sp, #0x10
 402ad90:	97ffff6f 	bl	402ab4c <ddrtrn_training_restore_timing>
 402ad94:	f0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402ad98:	f9403fe1 	ldr	x1, [sp, #120]
 402ad9c:	f9455002 	ldr	x2, [x0, #2720]
 402ada0:	eb020021 	subs	x1, x1, x2
 402ada4:	d2800002 	mov	x2, #0x0                   	// #0
 402ada8:	54000d20 	b.eq	402af4c <ddrtrn_hw_training_by_phy+0x27c>  // b.none
 402adac:	97ffe4f0 	bl	402416c <__stack_chk_fail>
 402adb0:	97fff611 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402adb4:	b9002414 	str	w20, [x0, #36]
 402adb8:	97fff60f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402adbc:	aa0003f3 	mov	x19, x0
 402adc0:	97fff608 	bl	40285e0 <ddrtrn_hal_get_phy>
 402adc4:	d37d7e81 	ubfiz	x1, x20, #3, #32
 402adc8:	b9402262 	ldr	w2, [x19, #32]
 402adcc:	91008021 	add	x1, x1, #0x20
 402add0:	9b180441 	madd	x1, x2, x24, x1
 402add4:	8b010000 	add	x0, x0, x1
 402add8:	b9401013 	ldr	w19, [x0, #16]
 402addc:	97fff606 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ade0:	b9000c13 	str	w19, [x0, #12]
 402ade4:	97fff604 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ade8:	b9400413 	ldr	w19, [x0, #4]
 402adec:	97fff602 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402adf0:	b9402401 	ldr	w1, [x0, #36]
 402adf4:	2a1303e0 	mov	w0, w19
 402adf8:	97fffe66 	bl	402a790 <ddrtrn_hal_phy_switch_rank>
 402adfc:	97fff5fe 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ae00:	b9400c1a 	ldr	w26, [x0, #12]
 402ae04:	97fff5fc 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ae08:	b9400415 	ldr	w21, [x0, #4]
 402ae0c:	97ffff8e 	bl	402ac44 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402ae10:	2a0003fc 	mov	w28, w0
 402ae14:	340008ba 	cbz	w26, 402af28 <ddrtrn_hw_training_by_phy+0x258>
 402ae18:	2a1503e0 	mov	w0, w21
 402ae1c:	97fffa8e 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 402ae20:	12110340 	and	w0, w26, #0x8000
 402ae24:	94000a84 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402ae28:	2a0003f3 	mov	w19, w0
 402ae2c:	121f0b40 	and	w0, w26, #0xe
 402ae30:	94000a81 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402ae34:	0b000273 	add	w19, w19, w0
 402ae38:	3608007a 	tbz	w26, #1, 402ae44 <ddrtrn_hw_training_by_phy+0x174>
 402ae3c:	2a1503e0 	mov	w0, w21
 402ae40:	97fffa93 	bl	402988c <ddrtrn_hal_ck_cfg>
 402ae44:	3618007a 	tbz	w26, #3, 402ae50 <ddrtrn_hw_training_by_phy+0x180>
 402ae48:	94000c0f 	bl	402de84 <ddrtrn_ac_oe_enable>
 402ae4c:	94000c2b 	bl	402def8 <ddrtrn_dummy_io_oe_enable>
 402ae50:	97fff5e9 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ae54:	b9400c00 	ldr	w0, [x0, #12]
 402ae58:	36100140 	tbz	w0, #2, 402ae80 <ddrtrn_hw_training_by_phy+0x1b0>
 402ae5c:	97fff5e6 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ae60:	aa0003fb 	mov	x27, x0
 402ae64:	97fff5df 	bl	40285e0 <ddrtrn_hal_get_phy>
 402ae68:	b9402361 	ldr	w1, [x27, #32]
 402ae6c:	9b180021 	madd	x1, x1, x24, x0
 402ae70:	2a1503e0 	mov	w0, w21
 402ae74:	b9400422 	ldr	w2, [x1, #4]
 402ae78:	2a1c03e1 	mov	w1, w28
 402ae7c:	94000a39 	bl	402d760 <ddrtrn_hal_hw_rdqs_offset_cfg>
 402ae80:	97fff5dd 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ae84:	b9402400 	ldr	w0, [x0, #36]
 402ae88:	35000080 	cbnz	w0, 402ae98 <ddrtrn_hw_training_by_phy+0x1c8>
 402ae8c:	910063e0 	add	x0, sp, #0x18
 402ae90:	94000a43 	bl	402d79c <ddrtrn_hal_training_get_rdqs>
 402ae94:	94000bec 	bl	402de44 <ddrtrn_hal_hw_save_rdqsbdl>
 402ae98:	5280001b 	mov	w27, #0x0                   	// #0
 402ae9c:	6b1b039f 	cmp	w28, w27
 402aea0:	54000381 	b.ne	402af10 <ddrtrn_hw_training_by_phy+0x240>  // b.any
 402aea4:	910033e0 	add	x0, sp, #0xc
 402aea8:	94000b4d 	bl	402dbdc <ddrtrn_hal_hw_dataeye_adapt>
 402aeac:	0b000273 	add	w19, w19, w0
 402aeb0:	12150340 	and	w0, w26, #0x800
 402aeb4:	94000a60 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402aeb8:	0b000273 	add	w19, w19, w0
 402aebc:	120a0340 	and	w0, w26, #0x400000
 402aec0:	94000a5d 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402aec4:	0b000273 	add	w19, w19, w0
 402aec8:	94000b86 	bl	402dce0 <ddrtrn_hal_hw_dataeye_vref_set>
 402aecc:	0b000273 	add	w19, w19, w0
 402aed0:	94000a84 	bl	402d8e0 <ddrtrn_hal_hw_training_normal_conf>
 402aed4:	0b000273 	add	w19, w19, w0
 402aed8:	2a1503e0 	mov	w0, w21
 402aedc:	97fffa5e 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 402aee0:	0b1302d6 	add	w22, w22, w19
 402aee4:	71000b3f 	cmp	w25, #0x2
 402aee8:	54fff521 	b.ne	402ad8c <ddrtrn_hw_training_by_phy+0xbc>  // b.any
 402aeec:	aa1703e0 	mov	x0, x23
 402aef0:	94000a2b 	bl	402d79c <ddrtrn_hal_training_get_rdqs>
 402aef4:	7100069f 	cmp	w20, #0x1
 402aef8:	54000060 	b.eq	402af04 <ddrtrn_hw_training_by_phy+0x234>  // b.none
 402aefc:	910063e0 	add	x0, sp, #0x18
 402af00:	94000a3a 	bl	402d7e8 <ddrtrn_hal_training_set_rdqs>
 402af04:	11000694 	add	w20, w20, #0x1
 402af08:	910042f7 	add	x23, x23, #0x10
 402af0c:	17ffff88 	b	402ad2c <ddrtrn_hw_training_by_phy+0x5c>
 402af10:	97fff5b9 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402af14:	b900181b 	str	w27, [x0, #24]
 402af18:	910123e0 	add	x0, sp, #0x48
 402af1c:	1100077b 	add	w27, w27, #0x1
 402af20:	97fffd25 	bl	402a3b4 <ddrtrn_hal_rdqbdl_adj>
 402af24:	17ffffde 	b	402ae9c <ddrtrn_hw_training_by_phy+0x1cc>
 402af28:	52800013 	mov	w19, #0x0                   	// #0
 402af2c:	17ffffed 	b	402aee0 <ddrtrn_hw_training_by_phy+0x210>
 402af30:	b9401e81 	ldr	w1, [x20, #28]
 402af34:	2a1303e2 	mov	w2, w19
 402af38:	b9400e80 	ldr	w0, [x20, #12]
 402af3c:	11000673 	add	w19, w19, #0x1
 402af40:	94000ba0 	bl	402ddc0 <ddrtrn_hal_hw_restore_rdqsbdl>
 402af44:	97fffce1 	bl	402a2c8 <ddrtrn_hal_rdqs_sync_rank_rdq>
 402af48:	17ffff83 	b	402ad54 <ddrtrn_hw_training_by_phy+0x84>
 402af4c:	2a1603e0 	mov	w0, w22
 402af50:	a94853f3 	ldp	x19, x20, [sp, #128]
 402af54:	a9495bf5 	ldp	x21, x22, [sp, #144]
 402af58:	a94a63f7 	ldp	x23, x24, [sp, #160]
 402af5c:	a94b6bf9 	ldp	x25, x26, [sp, #176]
 402af60:	a94c73fb 	ldp	x27, x28, [sp, #192]
 402af64:	f9406bfe 	ldr	x30, [sp, #208]
 402af68:	910383ff 	add	sp, sp, #0xe0
 402af6c:	d65f03c0 	ret

000000000402af70 <ddrtrn_hw_training>:
 402af70:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402af74:	52800013 	mov	w19, #0x0                   	// #0
 402af78:	52800014 	mov	w20, #0x0                   	// #0
 402af7c:	a9015bf5 	stp	x21, x22, [sp, #16]
 402af80:	d2800795 	mov	x21, #0x3c                  	// #60
 402af84:	f90013fe 	str	x30, [sp, #32]
 402af88:	97fff59b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402af8c:	b9400000 	ldr	w0, [x0]
 402af90:	6b00027f 	cmp	w19, w0
 402af94:	540000c3 	b.cc	402afac <ddrtrn_hw_training+0x3c>  // b.lo, b.ul, b.last
 402af98:	2a1403e0 	mov	w0, w20
 402af9c:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402afa0:	f94013fe 	ldr	x30, [sp, #32]
 402afa4:	a8c353f3 	ldp	x19, x20, [sp], #48
 402afa8:	d65f03c0 	ret
 402afac:	97fff592 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402afb0:	b9002013 	str	w19, [x0, #32]
 402afb4:	97fff58b 	bl	40285e0 <ddrtrn_hal_get_phy>
 402afb8:	2a1303e1 	mov	w1, w19
 402afbc:	11000673 	add	w19, w19, #0x1
 402afc0:	9b157c21 	mul	x1, x1, x21
 402afc4:	b8616816 	ldr	w22, [x0, x1]
 402afc8:	97fff58b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402afcc:	b9000416 	str	w22, [x0, #4]
 402afd0:	97ffff40 	bl	402acd0 <ddrtrn_hw_training_by_phy>
 402afd4:	0b000294 	add	w20, w20, w0
 402afd8:	17ffffec 	b	402af88 <ddrtrn_hw_training+0x18>

000000000402afdc <ddrtrn_hal_get_cur_phy_dram_type>:
 402afdc:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402afe0:	97fff585 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402afe4:	aa0003f3 	mov	x19, x0
 402afe8:	97fff57e 	bl	40285e0 <ddrtrn_hal_get_phy>
 402afec:	d2800782 	mov	x2, #0x3c                  	// #60
 402aff0:	b9402261 	ldr	w1, [x19, #32]
 402aff4:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402aff8:	d503201f 	nop
 402affc:	9b020021 	madd	x1, x1, x2, x0
 402b000:	b9400420 	ldr	w0, [x1, #4]
 402b004:	d65f03c0 	ret

000000000402b008 <ddrtrn_dcc_get_min_win>:
 402b008:	937a7c21 	sbfiz	x1, x1, #6, #32
 402b00c:	8b010000 	add	x0, x0, x1
 402b010:	29540001 	ldp	w1, w0, [x0, #160]
 402b014:	12002422 	and	w2, w1, #0x3ff
 402b018:	d3506421 	ubfx	x1, x1, #16, #10
 402b01c:	4b010042 	sub	w2, w2, w1
 402b020:	12002401 	and	w1, w0, #0x3ff
 402b024:	d3506400 	ubfx	x0, x0, #16, #10
 402b028:	4b000020 	sub	w0, w1, w0
 402b02c:	6b00005f 	cmp	w2, w0
 402b030:	1a809040 	csel	w0, w2, w0, ls  // ls = plast
 402b034:	d65f03c0 	ret

000000000402b038 <ddrtrn_sref_cfg>:
 402b038:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402b03c:	52800013 	mov	w19, #0x0                   	// #0
 402b040:	a9015bf5 	stp	x21, x22, [sp, #16]
 402b044:	aa0003f5 	mov	x21, x0
 402b048:	2a0103f6 	mov	w22, w1
 402b04c:	a9027bf7 	stp	x23, x30, [sp, #32]
 402b050:	d2800797 	mov	x23, #0x3c                  	// #60
 402b054:	97fff568 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b058:	aa0003f4 	mov	x20, x0
 402b05c:	97fff561 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b060:	b9402282 	ldr	w2, [x20, #32]
 402b064:	9b170042 	madd	x2, x2, x23, x0
 402b068:	b9400840 	ldr	w0, [x2, #8]
 402b06c:	6b00027f 	cmp	w19, w0
 402b070:	540000a3 	b.cc	402b084 <ddrtrn_sref_cfg+0x4c>  // b.lo, b.ul, b.last
 402b074:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402b078:	a9427bf7 	ldp	x23, x30, [sp, #32]
 402b07c:	a8c353f3 	ldp	x19, x20, [sp], #48
 402b080:	d65f03c0 	ret
 402b084:	2a1303e0 	mov	w0, w19
 402b088:	97fff8c0 	bl	4029388 <ddrtrn_hal_dmc_get_sref_cfg>
 402b08c:	b8335aa0 	str	w0, [x21, w19, uxtw #2]
 402b090:	121c7400 	and	w0, w0, #0xfffffff3
 402b094:	2a160001 	orr	w1, w0, w22
 402b098:	2a1303e0 	mov	w0, w19
 402b09c:	11000673 	add	w19, w19, #0x1
 402b0a0:	97fff8c0 	bl	40293a0 <ddrtrn_hal_dmc_set_sref_cfg>
 402b0a4:	17ffffec 	b	402b054 <ddrtrn_sref_cfg+0x1c>

000000000402b0a8 <ddrtrn_dcc_training>:
 402b0a8:	d10743ff 	sub	sp, sp, #0x1d0
 402b0ac:	d0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402b0b0:	a91753f3 	stp	x19, x20, [sp, #368]
 402b0b4:	52800a14 	mov	w20, #0x50                  	// #80
 402b0b8:	a9185bf5 	stp	x21, x22, [sp, #384]
 402b0bc:	a91963f7 	stp	x23, x24, [sp, #400]
 402b0c0:	a91a6bf9 	stp	x25, x26, [sp, #416]
 402b0c4:	a91b73fb 	stp	x27, x28, [sp, #432]
 402b0c8:	f900e3fe 	str	x30, [sp, #448]
 402b0cc:	f9455001 	ldr	x1, [x0, #2720]
 402b0d0:	f900b7e1 	str	x1, [sp, #360]
 402b0d4:	d2800001 	mov	x1, #0x0                   	// #0
 402b0d8:	97fff547 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b0dc:	aa0003f3 	mov	x19, x0
 402b0e0:	97fff540 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b0e4:	d2800782 	mov	x2, #0x3c                  	// #60
 402b0e8:	b9402261 	ldr	w1, [x19, #32]
 402b0ec:	52800013 	mov	w19, #0x0                   	// #0
 402b0f0:	9b020021 	madd	x1, x1, x2, x0
 402b0f4:	b9400c3b 	ldr	w27, [x1, #12]
 402b0f8:	6b1b027f 	cmp	w19, w27
 402b0fc:	540008a1 	b.ne	402b210 <ddrtrn_dcc_training+0x168>  // b.any
 402b100:	9100c3e0 	add	x0, sp, #0x30
 402b104:	97fff882 	bl	402930c <ddrtrn_hal_save_timing>
 402b108:	52800195 	mov	w21, #0xc                   	// #12
 402b10c:	97ffffb4 	bl	402afdc <ddrtrn_hal_get_cur_phy_dram_type>
 402b110:	7100181f 	cmp	w0, #0x6
 402b114:	910463e1 	add	x1, sp, #0x118
 402b118:	12800000 	mov	w0, #0xffffffff            	// #-1
 402b11c:	291cffe0 	stp	w0, wzr, [sp, #228]
 402b120:	1a9f17f6 	cset	w22, eq  // eq = none
 402b124:	b90103ff 	str	wzr, [sp, #256]
 402b128:	110006d6 	add	w22, w22, #0x1
 402b12c:	2901fc20 	stp	w0, wzr, [x1, #12]
 402b130:	52800300 	mov	w0, #0x18                  	// #24
 402b134:	b9010fe0 	str	w0, [sp, #268]
 402b138:	52800160 	mov	w0, #0xb                   	// #11
 402b13c:	b90113e0 	str	w0, [sp, #272]
 402b140:	52800100 	mov	w0, #0x8                   	// #8
 402b144:	b90117e0 	str	w0, [sp, #276]
 402b148:	52800320 	mov	w0, #0x19                  	// #25
 402b14c:	293dfc3f 	stp	wzr, wzr, [x1, #-20]
 402b150:	b9014fe0 	str	w0, [sp, #332]
 402b154:	528001e0 	mov	w0, #0xf                   	// #15
 402b158:	b90143ff 	str	wzr, [sp, #320]
 402b15c:	b90147ff 	str	wzr, [sp, #324]
 402b160:	b9014bff 	str	wzr, [sp, #328]
 402b164:	b90153e0 	str	w0, [sp, #336]
 402b168:	b90157f5 	str	w21, [sp, #340]
 402b16c:	94000bb7 	bl	402e048 <ddrtrn_hal_dcc_get_gated_bypass>
 402b170:	94000bb6 	bl	402e048 <ddrtrn_hal_dcc_get_gated_bypass>
 402b174:	2a0003f4 	mov	w20, w0
 402b178:	94000ba3 	bl	402e004 <ddrtrn_hal_dcc_get_ioctl21>
 402b17c:	94000ba2 	bl	402e004 <ddrtrn_hal_dcc_get_ioctl21>
 402b180:	2a0003f3 	mov	w19, w0
 402b184:	97ffff96 	bl	402afdc <ddrtrn_hal_get_cur_phy_dram_type>
 402b188:	7100181f 	cmp	w0, #0x6
 402b18c:	54000081 	b.ne	402b19c <ddrtrn_dcc_training+0xf4>  // b.any
 402b190:	2a1503e1 	mov	w1, w21
 402b194:	9100a3e0 	add	x0, sp, #0x28
 402b198:	97ffffa8 	bl	402b038 <ddrtrn_sref_cfg>
 402b19c:	52800020 	mov	w0, #0x1                   	// #1
 402b1a0:	97fffe53 	bl	402aaec <ddrtrn_training_ctrl_easr>
 402b1a4:	34000480 	cbz	w0, 402b234 <ddrtrn_dcc_training+0x18c>
 402b1a8:	12800015 	mov	w21, #0xffffffff            	// #-1
 402b1ac:	d2800013 	mov	x19, #0x0                   	// #0
 402b1b0:	d2800796 	mov	x22, #0x3c                  	// #60
 402b1b4:	52a00058 	mov	w24, #0x20000               	// #131072
 402b1b8:	6b13037f 	cmp	w27, w19
 402b1bc:	54001f88 	b.hi	402b5ac <ddrtrn_dcc_training+0x504>  // b.pmore
 402b1c0:	97fffec4 	bl	402acd0 <ddrtrn_hw_training_by_phy>
 402b1c4:	0b150014 	add	w20, w0, w21
 402b1c8:	d2800013 	mov	x19, #0x0                   	// #0
 402b1cc:	d2800796 	mov	x22, #0x3c                  	// #60
 402b1d0:	6b13037f 	cmp	w27, w19
 402b1d4:	540021a8 	b.hi	402b608 <ddrtrn_dcc_training+0x560>  // b.pmore
 402b1d8:	9100c3e0 	add	x0, sp, #0x30
 402b1dc:	97fffe5c 	bl	402ab4c <ddrtrn_training_restore_timing>
 402b1e0:	97ffff7f 	bl	402afdc <ddrtrn_hal_get_cur_phy_dram_type>
 402b1e4:	7100181f 	cmp	w0, #0x6
 402b1e8:	54000061 	b.ne	402b1f4 <ddrtrn_dcc_training+0x14c>  // b.any
 402b1ec:	9100a3e0 	add	x0, sp, #0x28
 402b1f0:	97fffe30 	bl	402aab0 <ddrtrn_sref_cfg_restore>
 402b1f4:	d0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402b1f8:	f940b7e1 	ldr	x1, [sp, #360]
 402b1fc:	f9455002 	ldr	x2, [x0, #2720]
 402b200:	eb020021 	subs	x1, x1, x2
 402b204:	d2800002 	mov	x2, #0x0                   	// #0
 402b208:	54002900 	b.eq	402b728 <ddrtrn_dcc_training+0x680>  // b.none
 402b20c:	97ffe3d8 	bl	402416c <__stack_chk_fail>
 402b210:	97fff4f9 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b214:	b9002413 	str	w19, [x0, #36]
 402b218:	11000673 	add	w19, w19, #0x1
 402b21c:	97fff4f6 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b220:	b9402400 	ldr	w0, [x0, #36]
 402b224:	9100e3e1 	add	x1, sp, #0x38
 402b228:	9bb40400 	umaddl	x0, w0, w20, x1
 402b22c:	97fffcd0 	bl	402a56c <ddrtrn_hal_save_rdqbdl>
 402b230:	17ffffb2 	b	402b0f8 <ddrtrn_dcc_training+0x50>
 402b234:	97ffff6a 	bl	402afdc <ddrtrn_hal_get_cur_phy_dram_type>
 402b238:	2a1403e0 	mov	w0, w20
 402b23c:	94000b84 	bl	402e04c <ddrtrn_hal_dcc_set_gated_bypass>
 402b240:	52800040 	mov	w0, #0x2                   	// #2
 402b244:	97fffe2a 	bl	402aaec <ddrtrn_training_ctrl_easr>
 402b248:	35fffb00 	cbnz	w0, 402b1a8 <ddrtrn_dcc_training+0x100>
 402b24c:	12910000 	mov	w0, #0xffff77ff            	// #-34817
 402b250:	52800019 	mov	w25, #0x0                   	// #0
 402b254:	0a000260 	and	w0, w19, w0
 402b258:	b9001be0 	str	w0, [sp, #24]
 402b25c:	12800000 	mov	w0, #0xffffffff            	// #-1
 402b260:	291dffe0 	stp	w0, wzr, [sp, #236]
 402b264:	71000adf 	cmp	w22, #0x2
 402b268:	54000061 	b.ne	402b274 <ddrtrn_dcc_training+0x1cc>  // b.any
 402b26c:	b9012fe0 	str	w0, [sp, #300]
 402b270:	b90133ff 	str	wzr, [sp, #304]
 402b274:	52800020 	mov	w0, #0x1                   	// #1
 402b278:	97fffe1d 	bl	402aaec <ddrtrn_training_ctrl_easr>
 402b27c:	35fff960 	cbnz	w0, 402b1a8 <ddrtrn_dcc_training+0x100>
 402b280:	97ffff57 	bl	402afdc <ddrtrn_hal_get_cur_phy_dram_type>
 402b284:	53155321 	lsl	w1, w25, #11
 402b288:	7100181f 	cmp	w0, #0x6
 402b28c:	540008e1 	b.ne	402b3a8 <ddrtrn_dcc_training+0x300>  // b.any
 402b290:	b9401be0 	ldr	w0, [sp, #24]
 402b294:	2a193c35 	orr	w21, w1, w25, lsl #15
 402b298:	2a0002b5 	orr	w21, w21, w0
 402b29c:	2a1503e0 	mov	w0, w21
 402b2a0:	94000b61 	bl	402e024 <ddrtrn_hal_dcc_set_ioctl21>
 402b2a4:	52800040 	mov	w0, #0x2                   	// #2
 402b2a8:	b90163f5 	str	w21, [sp, #352]
 402b2ac:	97fffe10 	bl	402aaec <ddrtrn_training_ctrl_easr>
 402b2b0:	35fff7c0 	cbnz	w0, 402b1a8 <ddrtrn_dcc_training+0x100>
 402b2b4:	52800017 	mov	w23, #0x0                   	// #0
 402b2b8:	d280079c 	mov	x28, #0x3c                  	// #60
 402b2bc:	12800000 	mov	w0, #0xffffffff            	// #-1
 402b2c0:	b900e3e0 	str	w0, [sp, #224]
 402b2c4:	71000adf 	cmp	w22, #0x2
 402b2c8:	54000041 	b.ne	402b2d0 <ddrtrn_dcc_training+0x228>  // b.any
 402b2cc:	b90123e0 	str	w0, [sp, #288]
 402b2d0:	52800020 	mov	w0, #0x1                   	// #1
 402b2d4:	97fffe06 	bl	402aaec <ddrtrn_training_ctrl_easr>
 402b2d8:	35fff680 	cbnz	w0, 402b1a8 <ddrtrn_dcc_training+0x100>
 402b2dc:	b94163f8 	ldr	w24, [sp, #352]
 402b2e0:	97ffff3f 	bl	402afdc <ddrtrn_hal_get_cur_phy_dram_type>
 402b2e4:	53185ee1 	lsl	w1, w23, #8
 402b2e8:	7100181f 	cmp	w0, #0x6
 402b2ec:	54000641 	b.ne	402b3b4 <ddrtrn_dcc_training+0x30c>  // b.any
 402b2f0:	128ee015 	mov	w21, #0xffff88ff            	// #-30465
 402b2f4:	2a173021 	orr	w1, w1, w23, lsl #12
 402b2f8:	0a150315 	and	w21, w24, w21
 402b2fc:	2a0102b5 	orr	w21, w21, w1
 402b300:	2a1503e0 	mov	w0, w21
 402b304:	94000b48 	bl	402e024 <ddrtrn_hal_dcc_set_ioctl21>
 402b308:	52800040 	mov	w0, #0x2                   	// #2
 402b30c:	b90163f5 	str	w21, [sp, #352]
 402b310:	97fffdf7 	bl	402aaec <ddrtrn_training_ctrl_easr>
 402b314:	2a0003f5 	mov	w21, w0
 402b318:	35fff480 	cbnz	w0, 402b1a8 <ddrtrn_dcc_training+0x100>
 402b31c:	97fff4b6 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b320:	aa0003f8 	mov	x24, x0
 402b324:	97fff4af 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b328:	5280001a 	mov	w26, #0x0                   	// #0
 402b32c:	b9402301 	ldr	w1, [x24, #32]
 402b330:	52800018 	mov	w24, #0x0                   	// #0
 402b334:	9b1c0021 	madd	x1, x1, x28, x0
 402b338:	b9400c20 	ldr	w0, [x1, #12]
 402b33c:	b9000fe0 	str	w0, [sp, #12]
 402b340:	b9400fe0 	ldr	w0, [sp, #12]
 402b344:	6b00035f 	cmp	w26, w0
 402b348:	540003a1 	b.ne	402b3bc <ddrtrn_dcc_training+0x314>  // b.any
 402b34c:	34000f38 	cbz	w24, 402b530 <ddrtrn_dcc_training+0x488>
 402b350:	9100e3f7 	add	x23, sp, #0x38
 402b354:	aa1703e0 	mov	x0, x23
 402b358:	b940ac02 	ldr	w2, [x0, #172]
 402b35c:	b940b401 	ldr	w1, [x0, #180]
 402b360:	6b02003f 	cmp	w1, w2
 402b364:	54000042 	b.cs	402b36c <ddrtrn_dcc_training+0x2c4>  // b.hs, b.nlast
 402b368:	b900ac01 	str	w1, [x0, #172]
 402b36c:	b940b002 	ldr	w2, [x0, #176]
 402b370:	b940b801 	ldr	w1, [x0, #184]
 402b374:	6b02003f 	cmp	w1, w2
 402b378:	540000a9 	b.ls	402b38c <ddrtrn_dcc_training+0x2e4>  // b.plast
 402b37c:	b900b001 	str	w1, [x0, #176]
 402b380:	b940c801 	ldr	w1, [x0, #200]
 402b384:	b900cc01 	str	w1, [x0, #204]
 402b388:	b900d019 	str	w25, [x0, #208]
 402b38c:	110006a1 	add	w1, w21, #0x1
 402b390:	91010000 	add	x0, x0, #0x40
 402b394:	6b0102df 	cmp	w22, w1
 402b398:	54000f21 	b.ne	402b57c <ddrtrn_dcc_training+0x4d4>  // b.any
 402b39c:	35001579 	cbnz	w25, 402b648 <ddrtrn_dcc_training+0x5a0>
 402b3a0:	52800039 	mov	w25, #0x1                   	// #1
 402b3a4:	17ffffae 	b	402b25c <ddrtrn_dcc_training+0x1b4>
 402b3a8:	12147a75 	and	w21, w19, #0xfffff7ff
 402b3ac:	2a0102b5 	orr	w21, w21, w1
 402b3b0:	17ffffbb 	b	402b29c <ddrtrn_dcc_training+0x1f4>
 402b3b4:	12157315 	and	w21, w24, #0xfffff8ff
 402b3b8:	17ffffd1 	b	402b2fc <ddrtrn_dcc_training+0x254>
 402b3bc:	97fff48e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b3c0:	b900241a 	str	w26, [x0, #36]
 402b3c4:	97fff48c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b3c8:	b9400402 	ldr	w2, [x0, #4]
 402b3cc:	b90007e2 	str	w2, [sp, #4]
 402b3d0:	97fff489 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b3d4:	b9402401 	ldr	w1, [x0, #36]
 402b3d8:	b94007e2 	ldr	w2, [sp, #4]
 402b3dc:	2a0203e0 	mov	w0, w2
 402b3e0:	97fffcec 	bl	402a790 <ddrtrn_hal_phy_switch_rank>
 402b3e4:	52802000 	mov	w0, #0x100                 	// #256
 402b3e8:	94000913 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402b3ec:	b9000be0 	str	w0, [sp, #8]
 402b3f0:	94000b18 	bl	402e050 <ddrtrn_hal_dcc_rdet_enable>
 402b3f4:	b9400be0 	ldr	w0, [sp, #8]
 402b3f8:	340001a0 	cbz	w0, 402b42c <ddrtrn_dcc_training+0x384>
 402b3fc:	b900e3ff 	str	wzr, [sp, #224]
 402b400:	b90123ff 	str	wzr, [sp, #288]
 402b404:	97fff47c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b408:	b9402400 	ldr	w0, [x0, #36]
 402b40c:	9100e3e2 	add	x2, sp, #0x38
 402b410:	52800a01 	mov	w1, #0x50                  	// #80
 402b414:	1100075a 	add	w26, w26, #0x1
 402b418:	9ba10800 	umaddl	x0, w0, w1, x2
 402b41c:	97fffc77 	bl	402a5f8 <ddrtrn_hal_restore_rdqbdl>
 402b420:	b9400be0 	ldr	w0, [sp, #8]
 402b424:	0b000318 	add	w24, w24, w0
 402b428:	17ffffc6 	b	402b340 <ddrtrn_dcc_training+0x298>
 402b42c:	b940e3e0 	ldr	w0, [sp, #224]
 402b430:	b90007e0 	str	w0, [sp, #4]
 402b434:	97fff470 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b438:	f9000be0 	str	x0, [sp, #16]
 402b43c:	97fff469 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b440:	f9400be2 	ldr	x2, [sp, #16]
 402b444:	b9402041 	ldr	w1, [x2, #32]
 402b448:	9b1c0021 	madd	x1, x1, x28, x0
 402b44c:	b9401022 	ldr	w2, [x1, #16]
 402b450:	d2800001 	mov	x1, #0x0                   	// #0
 402b454:	53017c42 	lsr	w2, w2, #1
 402b458:	6b01005f 	cmp	w2, w1
 402b45c:	540003e8 	b.hi	402b4d8 <ddrtrn_dcc_training+0x430>  // b.pmore
 402b460:	52800001 	mov	w1, #0x0                   	// #0
 402b464:	9100e3e0 	add	x0, sp, #0x38
 402b468:	97fffee8 	bl	402b008 <ddrtrn_dcc_get_min_win>
 402b46c:	b94007e1 	ldr	w1, [sp, #4]
 402b470:	6b01001f 	cmp	w0, w1
 402b474:	1a819000 	csel	w0, w0, w1, ls  // ls = plast
 402b478:	b900e3e0 	str	w0, [sp, #224]
 402b47c:	97fffed8 	bl	402afdc <ddrtrn_hal_get_cur_phy_dram_type>
 402b480:	7100181f 	cmp	w0, #0x6
 402b484:	54fffc01 	b.ne	402b404 <ddrtrn_dcc_training+0x35c>  // b.any
 402b488:	b94123e0 	ldr	w0, [sp, #288]
 402b48c:	b90007e0 	str	w0, [sp, #4]
 402b490:	97fff459 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b494:	f9000be0 	str	x0, [sp, #16]
 402b498:	97fff452 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b49c:	f9400be2 	ldr	x2, [sp, #16]
 402b4a0:	b9402041 	ldr	w1, [x2, #32]
 402b4a4:	9b1c0021 	madd	x1, x1, x28, x0
 402b4a8:	b9401022 	ldr	w2, [x1, #16]
 402b4ac:	d2800041 	mov	x1, #0x2                   	// #2
 402b4b0:	6b01005f 	cmp	w2, w1
 402b4b4:	54000288 	b.hi	402b504 <ddrtrn_dcc_training+0x45c>  // b.pmore
 402b4b8:	52800021 	mov	w1, #0x1                   	// #1
 402b4bc:	9100e3e0 	add	x0, sp, #0x38
 402b4c0:	97fffed2 	bl	402b008 <ddrtrn_dcc_get_min_win>
 402b4c4:	b94007e1 	ldr	w1, [sp, #4]
 402b4c8:	6b01001f 	cmp	w0, w1
 402b4cc:	1a819000 	csel	w0, w0, w1, ls  // ls = plast
 402b4d0:	b90123e0 	str	w0, [sp, #288]
 402b4d4:	17ffffcc 	b	402b404 <ddrtrn_dcc_training+0x35c>
 402b4d8:	2a0103e0 	mov	w0, w1
 402b4dc:	f9000be1 	str	x1, [sp, #16]
 402b4e0:	b9001fe2 	str	w2, [sp, #28]
 402b4e4:	94000ae3 	bl	402e070 <ddrtrn_hal_dcc_get_dxnrdbound>
 402b4e8:	f9400be1 	ldr	x1, [sp, #16]
 402b4ec:	9100e3e2 	add	x2, sp, #0x38
 402b4f0:	8b010843 	add	x3, x2, x1, lsl #2
 402b4f4:	b9401fe2 	ldr	w2, [sp, #28]
 402b4f8:	91000421 	add	x1, x1, #0x1
 402b4fc:	b900a060 	str	w0, [x3, #160]
 402b500:	17ffffd6 	b	402b458 <ddrtrn_dcc_training+0x3b0>
 402b504:	2a0103e0 	mov	w0, w1
 402b508:	f9000be1 	str	x1, [sp, #16]
 402b50c:	b9001fe2 	str	w2, [sp, #28]
 402b510:	94000ad8 	bl	402e070 <ddrtrn_hal_dcc_get_dxnrdbound>
 402b514:	f9400be1 	ldr	x1, [sp, #16]
 402b518:	9100e3e2 	add	x2, sp, #0x38
 402b51c:	8b010843 	add	x3, x2, x1, lsl #2
 402b520:	b9401fe2 	ldr	w2, [sp, #28]
 402b524:	91000421 	add	x1, x1, #0x1
 402b528:	b900d860 	str	w0, [x3, #216]
 402b52c:	17ffffe1 	b	402b4b0 <ddrtrn_dcc_training+0x408>
 402b530:	9100e3e0 	add	x0, sp, #0x38
 402b534:	b940a801 	ldr	w1, [x0, #168]
 402b538:	b940b402 	ldr	w2, [x0, #180]
 402b53c:	6b02003f 	cmp	w1, w2
 402b540:	54000042 	b.cs	402b548 <ddrtrn_dcc_training+0x4a0>  // b.hs, b.nlast
 402b544:	b900b401 	str	w1, [x0, #180]
 402b548:	b940b802 	ldr	w2, [x0, #184]
 402b54c:	6b02003f 	cmp	w1, w2
 402b550:	54000069 	b.ls	402b55c <ddrtrn_dcc_training+0x4b4>  // b.plast
 402b554:	b900b801 	str	w1, [x0, #184]
 402b558:	b900c817 	str	w23, [x0, #200]
 402b55c:	11000718 	add	w24, w24, #0x1
 402b560:	91010000 	add	x0, x0, #0x40
 402b564:	6b1802df 	cmp	w22, w24
 402b568:	54fffe61 	b.ne	402b534 <ddrtrn_dcc_training+0x48c>  // b.any
 402b56c:	110006f7 	add	w23, w23, #0x1
 402b570:	710012ff 	cmp	w23, #0x4
 402b574:	54ffea41 	b.ne	402b2bc <ddrtrn_dcc_training+0x214>  // b.any
 402b578:	17ffff76 	b	402b350 <ddrtrn_dcc_training+0x2a8>
 402b57c:	2a0103f5 	mov	w21, w1
 402b580:	17ffff76 	b	402b358 <ddrtrn_dcc_training+0x2b0>
 402b584:	1ac222c0 	lsl	w0, w22, w2
 402b588:	1ac12303 	lsl	w3, w24, w1
 402b58c:	2a030000 	orr	w0, w0, w3
 402b590:	0a200273 	bic	w19, w19, w0
 402b594:	b940d2e0 	ldr	w0, [x23, #208]
 402b598:	1ac22002 	lsl	w2, w0, w2
 402b59c:	b940cee0 	ldr	w0, [x23, #204]
 402b5a0:	1ac12001 	lsl	w1, w0, w1
 402b5a4:	2a010040 	orr	w0, w2, w1
 402b5a8:	14000055 	b	402b6fc <ddrtrn_dcc_training+0x654>
 402b5ac:	97fff412 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b5b0:	b9002413 	str	w19, [x0, #36]
 402b5b4:	d37d7e74 	ubfiz	x20, x19, #3, #32
 402b5b8:	97fff40f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b5bc:	aa0003f7 	mov	x23, x0
 402b5c0:	97fff408 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b5c4:	91008294 	add	x20, x20, #0x20
 402b5c8:	b94022e1 	ldr	w1, [x23, #32]
 402b5cc:	9b165021 	madd	x1, x1, x22, x20
 402b5d0:	8b010000 	add	x0, x0, x1
 402b5d4:	b9401001 	ldr	w1, [x0, #16]
 402b5d8:	9100e3e0 	add	x0, sp, #0x38
 402b5dc:	8b130800 	add	x0, x0, x19, lsl #2
 402b5e0:	91000673 	add	x19, x19, #0x1
 402b5e4:	b9012001 	str	w1, [x0, #288]
 402b5e8:	97fff403 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b5ec:	aa0003f7 	mov	x23, x0
 402b5f0:	97fff3fc 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b5f4:	b94022e1 	ldr	w1, [x23, #32]
 402b5f8:	9b165034 	madd	x20, x1, x22, x20
 402b5fc:	8b140000 	add	x0, x0, x20
 402b600:	b9001018 	str	w24, [x0, #16]
 402b604:	17fffeed 	b	402b1b8 <ddrtrn_dcc_training+0x110>
 402b608:	97fff3fb 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b60c:	b9002413 	str	w19, [x0, #36]
 402b610:	9100e3e0 	add	x0, sp, #0x38
 402b614:	8b130800 	add	x0, x0, x19, lsl #2
 402b618:	b9412017 	ldr	w23, [x0, #288]
 402b61c:	97fff3f6 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b620:	aa0003f5 	mov	x21, x0
 402b624:	97fff3ef 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b628:	d37d7e62 	ubfiz	x2, x19, #3, #32
 402b62c:	91000673 	add	x19, x19, #0x1
 402b630:	91008042 	add	x2, x2, #0x20
 402b634:	b94022a1 	ldr	w1, [x21, #32]
 402b638:	9b160821 	madd	x1, x1, x22, x2
 402b63c:	8b010000 	add	x0, x0, x1
 402b640:	b9001017 	str	w23, [x0, #16]
 402b644:	17fffee3 	b	402b1d0 <ddrtrn_dcc_training+0x128>
 402b648:	97fffe65 	bl	402afdc <ddrtrn_hal_get_cur_phy_dram_type>
 402b64c:	7100181f 	cmp	w0, #0x6
 402b650:	54000081 	b.ne	402b660 <ddrtrn_dcc_training+0x5b8>  // b.any
 402b654:	9100a3e0 	add	x0, sp, #0x28
 402b658:	52800181 	mov	w1, #0xc                   	// #12
 402b65c:	97fffe77 	bl	402b038 <ddrtrn_sref_cfg>
 402b660:	52800020 	mov	w0, #0x1                   	// #1
 402b664:	97fffd22 	bl	402aaec <ddrtrn_training_ctrl_easr>
 402b668:	35ffda00 	cbnz	w0, 402b1a8 <ddrtrn_dcc_training+0x100>
 402b66c:	9101e3e0 	add	x0, sp, #0x78
 402b670:	52800801 	mov	w1, #0x40                  	// #64
 402b674:	52800036 	mov	w22, #0x1                   	// #1
 402b678:	528000f8 	mov	w24, #0x7                   	// #7
 402b67c:	9ba102b5 	umaddl	x21, w21, w1, x0
 402b680:	295582e1 	ldp	w1, w0, [x23, #172]
 402b684:	b940dae2 	ldr	w2, [x23, #216]
 402b688:	4b010000 	sub	w0, w0, w1
 402b68c:	b940dee1 	ldr	w1, [x23, #220]
 402b690:	7100081f 	cmp	w0, #0x2
 402b694:	54fff788 	b.hi	402b584 <ddrtrn_dcc_training+0x4dc>  // b.pmore
 402b698:	b940d6e4 	ldr	w4, [x23, #212]
 402b69c:	1ac22662 	lsr	w2, w19, w2
 402b6a0:	1ac12661 	lsr	w1, w19, w1
 402b6a4:	12000042 	and	w2, w2, #0x1
 402b6a8:	12000821 	and	w1, w1, #0x7
 402b6ac:	b900c6e1 	str	w1, [x23, #196]
 402b6b0:	1ac42683 	lsr	w3, w20, w4
 402b6b4:	12000063 	and	w3, w3, #0x1
 402b6b8:	1ac422c0 	lsl	w0, w22, w4
 402b6bc:	0a200294 	bic	w20, w20, w0
 402b6c0:	29178ae3 	stp	w3, w2, [x23, #188]
 402b6c4:	1ac42063 	lsl	w3, w3, w4
 402b6c8:	2a140074 	orr	w20, w3, w20
 402b6cc:	2a1403e0 	mov	w0, w20
 402b6d0:	94000a5f 	bl	402e04c <ddrtrn_hal_dcc_set_gated_bypass>
 402b6d4:	295b0ae1 	ldp	w1, w2, [x23, #216]
 402b6d8:	1ac122c0 	lsl	w0, w22, w1
 402b6dc:	1ac22303 	lsl	w3, w24, w2
 402b6e0:	2a030000 	orr	w0, w0, w3
 402b6e4:	0a200273 	bic	w19, w19, w0
 402b6e8:	b940c2e0 	ldr	w0, [x23, #192]
 402b6ec:	1ac12000 	lsl	w0, w0, w1
 402b6f0:	b940c6e1 	ldr	w1, [x23, #196]
 402b6f4:	1ac22021 	lsl	w1, w1, w2
 402b6f8:	2a010000 	orr	w0, w0, w1
 402b6fc:	2a130013 	orr	w19, w0, w19
 402b700:	910102f7 	add	x23, x23, #0x40
 402b704:	2a1303e0 	mov	w0, w19
 402b708:	94000a47 	bl	402e024 <ddrtrn_hal_dcc_set_ioctl21>
 402b70c:	eb1502ff 	cmp	x23, x21
 402b710:	54fffb81 	b.ne	402b680 <ddrtrn_dcc_training+0x5d8>  // b.any
 402b714:	52800040 	mov	w0, #0x2                   	// #2
 402b718:	97fffcf5 	bl	402aaec <ddrtrn_training_ctrl_easr>
 402b71c:	7100001f 	cmp	w0, #0x0
 402b720:	5a9f03f5 	csetm	w21, ne  // ne = any
 402b724:	17fffea2 	b	402b1ac <ddrtrn_dcc_training+0x104>
 402b728:	2a1403e0 	mov	w0, w20
 402b72c:	a95753f3 	ldp	x19, x20, [sp, #368]
 402b730:	a9585bf5 	ldp	x21, x22, [sp, #384]
 402b734:	a95963f7 	ldp	x23, x24, [sp, #400]
 402b738:	a95a6bf9 	ldp	x25, x26, [sp, #416]
 402b73c:	a95b73fb 	ldp	x27, x28, [sp, #432]
 402b740:	f940e3fe 	ldr	x30, [sp, #448]
 402b744:	910743ff 	add	sp, sp, #0x1d0
 402b748:	d65f03c0 	ret

000000000402b74c <ddrtrn_training_break_point>:
 402b74c:	d65f03c0 	ret

000000000402b750 <ddrtrn_dcc_training_func>:
 402b750:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402b754:	52800013 	mov	w19, #0x0                   	// #0
 402b758:	a9015bf5 	stp	x21, x22, [sp, #16]
 402b75c:	52800015 	mov	w21, #0x0                   	// #0
 402b760:	d2800796 	mov	x22, #0x3c                  	// #60
 402b764:	a90263f7 	stp	x23, x24, [sp, #32]
 402b768:	52800037 	mov	w23, #0x1                   	// #1
 402b76c:	f9001bfe 	str	x30, [sp, #48]
 402b770:	97fff3a1 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b774:	b9400000 	ldr	w0, [x0]
 402b778:	6b00027f 	cmp	w19, w0
 402b77c:	540000e3 	b.cc	402b798 <ddrtrn_dcc_training_func+0x48>  // b.lo, b.ul, b.last
 402b780:	2a1503e0 	mov	w0, w21
 402b784:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402b788:	a94263f7 	ldp	x23, x24, [sp, #32]
 402b78c:	f9401bfe 	ldr	x30, [sp, #48]
 402b790:	a8c453f3 	ldp	x19, x20, [sp], #64
 402b794:	d65f03c0 	ret
 402b798:	2a1303f4 	mov	w20, w19
 402b79c:	97fff396 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b7a0:	b9002013 	str	w19, [x0, #32]
 402b7a4:	9b167e94 	mul	x20, x20, x22
 402b7a8:	97fff38e 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b7ac:	b8746818 	ldr	w24, [x0, x20]
 402b7b0:	97fff391 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b7b4:	b9000418 	str	w24, [x0, #4]
 402b7b8:	97fff38a 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b7bc:	8b140014 	add	x20, x0, x20
 402b7c0:	b9402e94 	ldr	w20, [x20, #44]
 402b7c4:	97fff38c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b7c8:	b9000c14 	str	w20, [x0, #12]
 402b7cc:	97fff38a 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b7d0:	b9402000 	ldr	w0, [x0, #32]
 402b7d4:	1ac022e0 	lsl	w0, w23, w0
 402b7d8:	97fff810 	bl	4029818 <ddrtrn_hal_check_bypass>
 402b7dc:	350000c0 	cbnz	w0, 402b7f4 <ddrtrn_dcc_training_func+0xa4>
 402b7e0:	52a10000 	mov	w0, #0x8000000             	// #134217728
 402b7e4:	97fff80d 	bl	4029818 <ddrtrn_hal_check_bypass>
 402b7e8:	35000060 	cbnz	w0, 402b7f4 <ddrtrn_dcc_training_func+0xa4>
 402b7ec:	97fffe2f 	bl	402b0a8 <ddrtrn_dcc_training>
 402b7f0:	0b0002b5 	add	w21, w21, w0
 402b7f4:	11000673 	add	w19, w19, #0x1
 402b7f8:	17ffffde 	b	402b770 <ddrtrn_dcc_training_func+0x20>

000000000402b7fc <ddrtrn_hal_get_byte_num>:
 402b7fc:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402b800:	97fff37d 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b804:	aa0003f3 	mov	x19, x0
 402b808:	97fff376 	bl	40285e0 <ddrtrn_hal_get_phy>
 402b80c:	d2800783 	mov	x3, #0x3c                  	// #60
 402b810:	b9402262 	ldr	w2, [x19, #32]
 402b814:	b9402a61 	ldr	w1, [x19, #40]
 402b818:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402b81c:	9b037c42 	mul	x2, x2, x3
 402b820:	d2800183 	mov	x3, #0xc                   	// #12
 402b824:	9b030821 	madd	x1, x1, x3, x2
 402b828:	8b010000 	add	x0, x0, x1
 402b82c:	b9401800 	ldr	w0, [x0, #24]
 402b830:	d65f03c0 	ret

000000000402b834 <ddrtrn_vref_get_win>:
 402b834:	d100c3ff 	sub	sp, sp, #0x30
 402b838:	a90153f3 	stp	x19, x20, [sp, #16]
 402b83c:	aa0003f4 	mov	x20, x0
 402b840:	2a0103f3 	mov	w19, w1
 402b844:	f90013fe 	str	x30, [sp, #32]
 402b848:	d0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402b84c:	f9455001 	ldr	x1, [x0, #2720]
 402b850:	f90007e1 	str	x1, [sp, #8]
 402b854:	d2800001 	mov	x1, #0x0                   	// #0
 402b858:	52800640 	mov	w0, #0x32                  	// #50
 402b85c:	b901029f 	str	wzr, [x20, #256]
 402b860:	b90007e0 	str	w0, [sp, #4]
 402b864:	97fff364 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b868:	b9401400 	ldr	w0, [x0, #20]
 402b86c:	7100041f 	cmp	w0, #0x1
 402b870:	540000a1 	b.ne	402b884 <ddrtrn_vref_get_win+0x50>  // b.any
 402b874:	97fff360 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b878:	b9402400 	ldr	w0, [x0, #36]
 402b87c:	910013e1 	add	x1, sp, #0x4
 402b880:	97fffba4 	bl	402a710 <ddrtrn_hal_vref_get_host_max>
 402b884:	37f801f3 	tbnz	w19, #31, 402b8c0 <ddrtrn_vref_get_win+0x8c>
 402b888:	b94007e0 	ldr	w0, [sp, #4]
 402b88c:	6b13001f 	cmp	w0, w19
 402b890:	1a80a260 	csel	w0, w19, w0, ge  // ge = tcont
 402b894:	94000740 	bl	402d594 <ddrtrn_hal_vref_status_set>
 402b898:	aa1403e0 	mov	x0, x20
 402b89c:	940001eb 	bl	402c048 <ddrtrn_dataeye_deskew>
 402b8a0:	d0ffffa1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 402b8a4:	b9410280 	ldr	w0, [x20, #256]
 402b8a8:	f94007e2 	ldr	x2, [sp, #8]
 402b8ac:	f9455023 	ldr	x3, [x1, #2720]
 402b8b0:	eb030042 	subs	x2, x2, x3
 402b8b4:	d2800003 	mov	x3, #0x0                   	// #0
 402b8b8:	54000080 	b.eq	402b8c8 <ddrtrn_vref_get_win+0x94>  // b.none
 402b8bc:	97ffe22c 	bl	402416c <__stack_chk_fail>
 402b8c0:	52800000 	mov	w0, #0x0                   	// #0
 402b8c4:	17fffff4 	b	402b894 <ddrtrn_vref_get_win+0x60>
 402b8c8:	a94153f3 	ldp	x19, x20, [sp, #16]
 402b8cc:	f94013fe 	ldr	x30, [sp, #32]
 402b8d0:	9100c3ff 	add	sp, sp, #0x30
 402b8d4:	d65f03c0 	ret

000000000402b8d8 <ddrtrn_vref_find_best>:
 402b8d8:	d10143ff 	sub	sp, sp, #0x50
 402b8dc:	a90153f3 	stp	x19, x20, [sp, #16]
 402b8e0:	2a0103f3 	mov	w19, w1
 402b8e4:	a9025bf5 	stp	x21, x22, [sp, #32]
 402b8e8:	2a0203f6 	mov	w22, w2
 402b8ec:	a90363f7 	stp	x23, x24, [sp, #48]
 402b8f0:	aa0003f7 	mov	x23, x0
 402b8f4:	d0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402b8f8:	f90023fe 	str	x30, [sp, #64]
 402b8fc:	f9455001 	ldr	x1, [x0, #2720]
 402b900:	f90007e1 	str	x1, [sp, #8]
 402b904:	d2800001 	mov	x1, #0x0                   	// #0
 402b908:	52800640 	mov	w0, #0x32                  	// #50
 402b90c:	b90007e0 	str	w0, [sp, #4]
 402b910:	97fff339 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b914:	b9401400 	ldr	w0, [x0, #20]
 402b918:	7100041f 	cmp	w0, #0x1
 402b91c:	540000a1 	b.ne	402b930 <ddrtrn_vref_find_best+0x58>  // b.any
 402b920:	97fff335 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b924:	b9402400 	ldr	w0, [x0, #36]
 402b928:	910013e1 	add	x1, sp, #0x4
 402b92c:	97fffb79 	bl	402a710 <ddrtrn_hal_vref_get_host_max>
 402b930:	0b160274 	add	w20, w19, w22
 402b934:	37f80333 	tbnz	w19, #31, 402b998 <ddrtrn_vref_find_best+0xc0>
 402b938:	b94007e1 	ldr	w1, [sp, #4]
 402b93c:	6b13003f 	cmp	w1, w19
 402b940:	1a81a273 	csel	w19, w19, w1, ge  // ge = tcont
 402b944:	52800015 	mov	w21, #0x0                   	// #0
 402b948:	52800018 	mov	w24, #0x0                   	// #0
 402b94c:	37f80194 	tbnz	w20, #31, 402b97c <ddrtrn_vref_find_best+0xa4>
 402b950:	b94007e0 	ldr	w0, [sp, #4]
 402b954:	6b14001f 	cmp	w0, w20
 402b958:	5400012b 	b.lt	402b97c <ddrtrn_vref_find_best+0xa4>  // b.tstop
 402b95c:	2a1403e1 	mov	w1, w20
 402b960:	aa1703e0 	mov	x0, x23
 402b964:	97ffffb4 	bl	402b834 <ddrtrn_vref_get_win>
 402b968:	6b00031f 	cmp	w24, w0
 402b96c:	540001a9 	b.ls	402b9a0 <ddrtrn_vref_find_best+0xc8>  // b.plast
 402b970:	110006b5 	add	w21, w21, #0x1
 402b974:	71000ebf 	cmp	w21, #0x3
 402b978:	540001a1 	b.ne	402b9ac <ddrtrn_vref_find_best+0xd4>  // b.any
 402b97c:	d0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402b980:	f94007e1 	ldr	x1, [sp, #8]
 402b984:	f9455002 	ldr	x2, [x0, #2720]
 402b988:	eb020021 	subs	x1, x1, x2
 402b98c:	d2800002 	mov	x2, #0x0                   	// #0
 402b990:	54000120 	b.eq	402b9b4 <ddrtrn_vref_find_best+0xdc>  // b.none
 402b994:	97ffe1f6 	bl	402416c <__stack_chk_fail>
 402b998:	52800013 	mov	w19, #0x0                   	// #0
 402b99c:	17ffffea 	b	402b944 <ddrtrn_vref_find_best+0x6c>
 402b9a0:	2a1403f3 	mov	w19, w20
 402b9a4:	2a0003f8 	mov	w24, w0
 402b9a8:	52800015 	mov	w21, #0x0                   	// #0
 402b9ac:	0b160294 	add	w20, w20, w22
 402b9b0:	17ffffe7 	b	402b94c <ddrtrn_vref_find_best+0x74>
 402b9b4:	2a1303e0 	mov	w0, w19
 402b9b8:	a94153f3 	ldp	x19, x20, [sp, #16]
 402b9bc:	a9425bf5 	ldp	x21, x22, [sp, #32]
 402b9c0:	a94363f7 	ldp	x23, x24, [sp, #48]
 402b9c4:	f94023fe 	ldr	x30, [sp, #64]
 402b9c8:	910143ff 	add	sp, sp, #0x50
 402b9cc:	d65f03c0 	ret

000000000402b9d0 <ddrtrn_vref_cal>:
 402b9d0:	d100c3ff 	sub	sp, sp, #0x30
 402b9d4:	a90153f3 	stp	x19, x20, [sp, #16]
 402b9d8:	aa0003f3 	mov	x19, x0
 402b9dc:	d0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402b9e0:	a9027bf5 	stp	x21, x30, [sp, #32]
 402b9e4:	f9455001 	ldr	x1, [x0, #2720]
 402b9e8:	f90007e1 	str	x1, [sp, #8]
 402b9ec:	d2800001 	mov	x1, #0x0                   	// #0
 402b9f0:	b90007ff 	str	wzr, [sp, #4]
 402b9f4:	97fff300 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402b9f8:	b9401400 	ldr	w0, [x0, #20]
 402b9fc:	7100041f 	cmp	w0, #0x1
 402ba00:	54000401 	b.ne	402ba80 <ddrtrn_vref_cal+0xb0>  // b.any
 402ba04:	97fff2fc 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ba08:	b9400414 	ldr	w20, [x0, #4]
 402ba0c:	97fff2fa 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ba10:	b9402415 	ldr	w21, [x0, #36]
 402ba14:	97fff2f8 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ba18:	b9401802 	ldr	w2, [x0, #24]
 402ba1c:	910013e3 	add	x3, sp, #0x4
 402ba20:	2a1503e1 	mov	w1, w21
 402ba24:	aa1403e0 	mov	x0, x20
 402ba28:	97fffb40 	bl	402a728 <ddrtrn_hal_vref_phy_host_get>
 402ba2c:	b94007f4 	ldr	w20, [sp, #4]
 402ba30:	aa1303e0 	mov	x0, x19
 402ba34:	51000e81 	sub	w1, w20, #0x3
 402ba38:	97ffff7f 	bl	402b834 <ddrtrn_vref_get_win>
 402ba3c:	2a0003f5 	mov	w21, w0
 402ba40:	11000e81 	add	w1, w20, #0x3
 402ba44:	aa1303e0 	mov	x0, x19
 402ba48:	97ffff7b 	bl	402b834 <ddrtrn_vref_get_win>
 402ba4c:	6b0002bf 	cmp	w21, w0
 402ba50:	54000282 	b.cs	402baa0 <ddrtrn_vref_cal+0xd0>  // b.hs, b.nlast
 402ba54:	52800022 	mov	w2, #0x1                   	// #1
 402ba58:	2a1403e1 	mov	w1, w20
 402ba5c:	aa1303e0 	mov	x0, x19
 402ba60:	97ffff9e 	bl	402b8d8 <ddrtrn_vref_find_best>
 402ba64:	d0ffffa1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 402ba68:	f94007e2 	ldr	x2, [sp, #8]
 402ba6c:	f9455023 	ldr	x3, [x1, #2720]
 402ba70:	eb030042 	subs	x2, x2, x3
 402ba74:	d2800003 	mov	x3, #0x0                   	// #0
 402ba78:	54000360 	b.eq	402bae4 <ddrtrn_vref_cal+0x114>  // b.none
 402ba7c:	97ffe1bc 	bl	402416c <__stack_chk_fail>
 402ba80:	97fff2dd 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ba84:	b9400414 	ldr	w20, [x0, #4]
 402ba88:	97fff2db 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ba8c:	b9401802 	ldr	w2, [x0, #24]
 402ba90:	910013e1 	add	x1, sp, #0x4
 402ba94:	aa1403e0 	mov	x0, x20
 402ba98:	97fffb30 	bl	402a758 <ddrtrn_hal_vref_phy_dram_get>
 402ba9c:	17ffffe4 	b	402ba2c <ddrtrn_vref_cal+0x5c>
 402baa0:	54000069 	b.ls	402baac <ddrtrn_vref_cal+0xdc>  // b.plast
 402baa4:	12800002 	mov	w2, #0xffffffff            	// #-1
 402baa8:	17ffffec 	b	402ba58 <ddrtrn_vref_cal+0x88>
 402baac:	52800640 	mov	w0, #0x32                  	// #50
 402bab0:	b90007e0 	str	w0, [sp, #4]
 402bab4:	97fff2d0 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bab8:	b9401400 	ldr	w0, [x0, #20]
 402babc:	7100041f 	cmp	w0, #0x1
 402bac0:	540000a1 	b.ne	402bad4 <ddrtrn_vref_cal+0x104>  // b.any
 402bac4:	97fff2cc 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bac8:	b9402400 	ldr	w0, [x0, #36]
 402bacc:	910013e1 	add	x1, sp, #0x4
 402bad0:	97fffb10 	bl	402a710 <ddrtrn_hal_vref_get_host_max>
 402bad4:	b94007e0 	ldr	w0, [sp, #4]
 402bad8:	6b40069f 	cmp	w20, w0, lsr #1
 402badc:	54fffe42 	b.cs	402baa4 <ddrtrn_vref_cal+0xd4>  // b.hs, b.nlast
 402bae0:	17ffffdd 	b	402ba54 <ddrtrn_vref_cal+0x84>
 402bae4:	a94153f3 	ldp	x19, x20, [sp, #16]
 402bae8:	a9427bf5 	ldp	x21, x30, [sp, #32]
 402baec:	9100c3ff 	add	sp, sp, #0x30
 402baf0:	140006a9 	b	402d594 <ddrtrn_hal_vref_status_set>

000000000402baf4 <ddrtrn_vref_training>:
 402baf4:	d10603ff 	sub	sp, sp, #0x180
 402baf8:	d0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402bafc:	a91653f3 	stp	x19, x20, [sp, #352]
 402bb00:	a9177bf5 	stp	x21, x30, [sp, #368]
 402bb04:	f9455001 	ldr	x1, [x0, #2720]
 402bb08:	f900afe1 	str	x1, [sp, #344]
 402bb0c:	d2800001 	mov	x1, #0x0                   	// #0
 402bb10:	910003e0 	mov	x0, sp
 402bb14:	94000647 	bl	402d430 <ddrtrn_hal_vref_save_bdl>
 402bb18:	52802082 	mov	w2, #0x104                 	// #260
 402bb1c:	52800001 	mov	w1, #0x0                   	// #0
 402bb20:	910143e0 	add	x0, sp, #0x50
 402bb24:	97fffb78 	bl	402a904 <ddrtrn_set_data>
 402bb28:	97fff2b3 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bb2c:	b9401400 	ldr	w0, [x0, #20]
 402bb30:	7100041f 	cmp	w0, #0x1
 402bb34:	540007c0 	b.eq	402bc2c <ddrtrn_vref_training+0x138>  // b.none
 402bb38:	97fff2af 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bb3c:	aa0003f3 	mov	x19, x0
 402bb40:	97fff2a8 	bl	40285e0 <ddrtrn_hal_get_phy>
 402bb44:	d2800782 	mov	x2, #0x3c                  	// #60
 402bb48:	b9402261 	ldr	w1, [x19, #32]
 402bb4c:	9b020021 	madd	x1, x1, x2, x0
 402bb50:	b9400433 	ldr	w19, [x1, #4]
 402bb54:	97fff553 	bl	40290a0 <ddrtrn_hal_ddrc_get_bank_group>
 402bb58:	71001a7f 	cmp	w19, #0x6
 402bb5c:	7a4a1a64 	ccmp	w19, #0xa, #0x4, ne  // ne = any
 402bb60:	54000581 	b.ne	402bc10 <ddrtrn_vref_training+0x11c>  // b.any
 402bb64:	71001a7f 	cmp	w19, #0x6
 402bb68:	52800013 	mov	w19, #0x0                   	// #0
 402bb6c:	1a9f1414 	csinc	w20, w0, wzr, ne  // ne = any
 402bb70:	97ffff23 	bl	402b7fc <ddrtrn_hal_get_byte_num>
 402bb74:	6b13001f 	cmp	w0, w19
 402bb78:	54000449 	b.ls	402bc00 <ddrtrn_vref_training+0x10c>  // b.plast
 402bb7c:	97fff29e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bb80:	b9402815 	ldr	w21, [x0, #40]
 402bb84:	97fff29c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bb88:	0b150675 	add	w21, w19, w21, lsl #1
 402bb8c:	b9001815 	str	w21, [x0, #24]
 402bb90:	7100069f 	cmp	w20, #0x1
 402bb94:	54000081 	b.ne	402bba4 <ddrtrn_vref_training+0xb0>  // b.any
 402bb98:	121e7a60 	and	w0, w19, #0xfffffffd
 402bb9c:	7100041f 	cmp	w0, #0x1
 402bba0:	54000060 	b.eq	402bbac <ddrtrn_vref_training+0xb8>  // b.none
 402bba4:	910143e0 	add	x0, sp, #0x50
 402bba8:	97ffff8a 	bl	402b9d0 <ddrtrn_vref_cal>
 402bbac:	11000673 	add	w19, w19, #0x1
 402bbb0:	17fffff0 	b	402bb70 <ddrtrn_vref_training+0x7c>
 402bbb4:	97fff290 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bbb8:	b9402814 	ldr	w20, [x0, #40]
 402bbbc:	97fff28e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bbc0:	0b140674 	add	w20, w19, w20, lsl #1
 402bbc4:	b9001814 	str	w20, [x0, #24]
 402bbc8:	97fff28b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bbcc:	b9401800 	ldr	w0, [x0, #24]
 402bbd0:	7100041f 	cmp	w0, #0x1
 402bbd4:	540000e0 	b.eq	402bbf0 <ddrtrn_vref_training+0xfc>  // b.none
 402bbd8:	97fff287 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bbdc:	b9401800 	ldr	w0, [x0, #24]
 402bbe0:	71000c1f 	cmp	w0, #0x3
 402bbe4:	54000060 	b.eq	402bbf0 <ddrtrn_vref_training+0xfc>  // b.none
 402bbe8:	910143e0 	add	x0, sp, #0x50
 402bbec:	97ffff79 	bl	402b9d0 <ddrtrn_vref_cal>
 402bbf0:	11000673 	add	w19, w19, #0x1
 402bbf4:	97ffff02 	bl	402b7fc <ddrtrn_hal_get_byte_num>
 402bbf8:	6b13001f 	cmp	w0, w19
 402bbfc:	54fffdc8 	b.hi	402bbb4 <ddrtrn_vref_training+0xc0>  // b.pmore
 402bc00:	910003e0 	mov	x0, sp
 402bc04:	94000639 	bl	402d4e8 <ddrtrn_hal_vref_restore_bdl>
 402bc08:	910143e0 	add	x0, sp, #0x50
 402bc0c:	94000234 	bl	402c4dc <ddrtrn_result_data_save>
 402bc10:	d0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402bc14:	f940afe1 	ldr	x1, [sp, #344]
 402bc18:	f9455002 	ldr	x2, [x0, #2720]
 402bc1c:	eb020021 	subs	x1, x1, x2
 402bc20:	d2800002 	mov	x2, #0x0                   	// #0
 402bc24:	54000080 	b.eq	402bc34 <ddrtrn_vref_training+0x140>  // b.none
 402bc28:	97ffe151 	bl	402416c <__stack_chk_fail>
 402bc2c:	52800013 	mov	w19, #0x0                   	// #0
 402bc30:	17fffff1 	b	402bbf4 <ddrtrn_vref_training+0x100>
 402bc34:	52800000 	mov	w0, #0x0                   	// #0
 402bc38:	a95653f3 	ldp	x19, x20, [sp, #352]
 402bc3c:	a9577bf5 	ldp	x21, x30, [sp, #368]
 402bc40:	910603ff 	add	sp, sp, #0x180
 402bc44:	d65f03c0 	ret

000000000402bc48 <ddrtrn_vref_training_func>:
 402bc48:	d10183ff 	sub	sp, sp, #0x60
 402bc4c:	d0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402bc50:	a9057bf3 	stp	x19, x30, [sp, #80]
 402bc54:	f9455001 	ldr	x1, [x0, #2720]
 402bc58:	f90027e1 	str	x1, [sp, #72]
 402bc5c:	d2800001 	mov	x1, #0x0                   	// #0
 402bc60:	52800020 	mov	w0, #0x1                   	// #1
 402bc64:	940008e4 	bl	402dff4 <ddrtrn_hal_set_dq_type>
 402bc68:	910023e0 	add	x0, sp, #0x8
 402bc6c:	52a02001 	mov	w1, #0x1000000             	// #16777216
 402bc70:	97fff517 	bl	40290cc <ddrtrn_hal_save_reg>
 402bc74:	97fff57f 	bl	4029270 <ddrtrn_hal_training_switch_axi>
 402bc78:	52800040 	mov	w0, #0x2                   	// #2
 402bc7c:	94000996 	bl	402e2d4 <ddrtrn_ddrt_init>
 402bc80:	52a02000 	mov	w0, #0x1000000             	// #16777216
 402bc84:	97fff6e5 	bl	4029818 <ddrtrn_hal_check_bypass>
 402bc88:	350002e0 	cbnz	w0, 402bce4 <ddrtrn_vref_training_func+0x9c>
 402bc8c:	97fff25a 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bc90:	52800021 	mov	w1, #0x1                   	// #1
 402bc94:	b9001401 	str	w1, [x0, #20]
 402bc98:	97ffff97 	bl	402baf4 <ddrtrn_vref_training>
 402bc9c:	2a0003f3 	mov	w19, w0
 402bca0:	52a04000 	mov	w0, #0x2000000             	// #33554432
 402bca4:	97fff6dd 	bl	4029818 <ddrtrn_hal_check_bypass>
 402bca8:	350000c0 	cbnz	w0, 402bcc0 <ddrtrn_vref_training_func+0x78>
 402bcac:	97fff252 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bcb0:	52800041 	mov	w1, #0x2                   	// #2
 402bcb4:	b9001401 	str	w1, [x0, #20]
 402bcb8:	97ffff8f 	bl	402baf4 <ddrtrn_vref_training>
 402bcbc:	0b000273 	add	w19, w19, w0
 402bcc0:	910023e0 	add	x0, sp, #0x8
 402bcc4:	97fff546 	bl	40291dc <ddrtrn_hal_restore_reg>
 402bcc8:	d0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402bccc:	f94027e1 	ldr	x1, [sp, #72]
 402bcd0:	f9455002 	ldr	x2, [x0, #2720]
 402bcd4:	eb020021 	subs	x1, x1, x2
 402bcd8:	d2800002 	mov	x2, #0x0                   	// #0
 402bcdc:	54000080 	b.eq	402bcec <ddrtrn_vref_training_func+0xa4>  // b.none
 402bce0:	97ffe123 	bl	402416c <__stack_chk_fail>
 402bce4:	52800013 	mov	w19, #0x0                   	// #0
 402bce8:	17ffffee 	b	402bca0 <ddrtrn_vref_training_func+0x58>
 402bcec:	2a1303e0 	mov	w0, w19
 402bcf0:	a9457bf3 	ldp	x19, x30, [sp, #80]
 402bcf4:	910183ff 	add	sp, sp, #0x60
 402bcf8:	d65f03c0 	ret

000000000402bcfc <ddrtrn_dmc_auto_power_down_cfg>:
 402bcfc:	52920002 	mov	w2, #0x9000                	// #36864
 402bd00:	52900001 	mov	w1, #0x8000                	// #32768
 402bd04:	72a22282 	movk	w2, #0x1114, lsl #16
 402bd08:	72a22281 	movk	w1, #0x1114, lsl #16
 402bd0c:	52a222a0 	mov	w0, #0x11150000            	// #286588928
 402bd10:	140004fc 	b	402d100 <ddrtrn_hal_dmc_auto_pd_by_phy>

000000000402bd14 <ddrtrn_retrain_enable>:
 402bd14:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402bd18:	52a222a0 	mov	w0, #0x11150000            	// #286588928
 402bd1c:	a9015bf5 	stp	x21, x22, [sp, #16]
 402bd20:	f90013fe 	str	x30, [sp, #32]
 402bd24:	97fffa76 	bl	402a6fc <ddrtrn_hal_enable_rdqs_anti_aging>
 402bd28:	52a222a0 	mov	w0, #0x11150000            	// #286588928
 402bd2c:	97fffa5a 	bl	402a694 <ddrtrn_hal_get_trfc_ctrl_val>
 402bd30:	34000460 	cbz	w0, 402bdbc <ddrtrn_retrain_enable+0xa8>
 402bd34:	2a0003f6 	mov	w22, w0
 402bd38:	52a222a0 	mov	w0, #0x11150000            	// #286588928
 402bd3c:	97fffa5c 	bl	402a6ac <ddrtrn_hal_get_misc_val>
 402bd40:	2a0003f4 	mov	w20, w0
 402bd44:	52a222a0 	mov	w0, #0x11150000            	// #286588928
 402bd48:	97fffa5f 	bl	402a6c4 <ddrtrn_hal_get_trfc_threshold1_val>
 402bd4c:	2a0003f3 	mov	w19, w0
 402bd50:	52a222a0 	mov	w0, #0x11150000            	// #286588928
 402bd54:	97fffa5f 	bl	402a6d0 <ddrtrn_hal_get_dmsel>
 402bd58:	2a0003f5 	mov	w21, w0
 402bd5c:	52a222a0 	mov	w0, #0x11150000            	// #286588928
 402bd60:	97fffa5f 	bl	402a6dc <ddrtrn_hal_get_phyctrl0>
 402bd64:	d3524a81 	ubfx	x1, x20, #18, #1
 402bd68:	d3410a73 	ubfx	x19, x19, #1, #2
 402bd6c:	11001024 	add	w4, w1, #0x4
 402bd70:	d34822a1 	ubfx	x1, x21, #8, #1
 402bd74:	12000400 	and	w0, w0, #0x3
 402bd78:	d3483e82 	ubfx	x2, x20, #8, #8
 402bd7c:	12001e83 	and	w3, w20, #0xff
 402bd80:	12103e94 	and	w20, w20, #0xffff0000
 402bd84:	1b019261 	msub	w1, w19, w1, w4
 402bd88:	1b007c21 	mul	w1, w1, w0
 402bd8c:	4b010463 	sub	w3, w3, w1, lsl #1
 402bd90:	4b010442 	sub	w2, w2, w1, lsl #1
 402bd94:	2a140074 	orr	w20, w3, w20
 402bd98:	52a222a1 	mov	w1, #0x11150000            	// #286588928
 402bd9c:	2a022280 	orr	w0, w20, w2, lsl #8
 402bda0:	97fffa46 	bl	402a6b8 <ddrtrn_hal_set_misc_val>
 402bda4:	320002c0 	orr	w0, w22, #0x1
 402bda8:	52a222a1 	mov	w1, #0x11150000            	// #286588928
 402bdac:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402bdb0:	f94013fe 	ldr	x30, [sp, #32]
 402bdb4:	a8c353f3 	ldp	x19, x20, [sp], #48
 402bdb8:	17fffa3a 	b	402a6a0 <ddrtrn_hal_set_trfc_ctrl>
 402bdbc:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402bdc0:	f94013fe 	ldr	x30, [sp, #32]
 402bdc4:	a8c353f3 	ldp	x19, x20, [sp], #48
 402bdc8:	d65f03c0 	ret

000000000402bdcc <ddrtrn_reg_config>:
 402bdcc:	52800002 	mov	w2, #0x0                   	// #0
 402bdd0:	6b01005f 	cmp	w2, w1
 402bdd4:	54000041 	b.ne	402bddc <ddrtrn_reg_config+0x10>  // b.any
 402bdd8:	d65f03c0 	ret
 402bddc:	34000042 	cbz	w2, 402bde4 <ddrtrn_reg_config+0x18>
 402bde0:	91002000 	add	x0, x0, #0x8
 402bde4:	29401003 	ldp	w3, w4, [x0]
 402bde8:	11000442 	add	w2, w2, #0x1
 402bdec:	b9000064 	str	w4, [x3]
 402bdf0:	17fffff8 	b	402bdd0 <ddrtrn_reg_config+0x4>

000000000402bdf4 <ddrtrn_save_rdqbdl_phy>:
 402bdf4:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402bdf8:	52800013 	mov	w19, #0x0                   	// #0
 402bdfc:	a9015bf5 	stp	x21, x22, [sp, #16]
 402be00:	aa0003f5 	mov	x21, x0
 402be04:	a90263f7 	stp	x23, x24, [sp, #32]
 402be08:	d2800797 	mov	x23, #0x3c                  	// #60
 402be0c:	52801418 	mov	w24, #0xa0                  	// #160
 402be10:	a9037bf9 	stp	x25, x30, [sp, #48]
 402be14:	97fff1f8 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402be18:	b9400000 	ldr	w0, [x0]
 402be1c:	6b00027f 	cmp	w19, w0
 402be20:	540000c3 	b.cc	402be38 <ddrtrn_save_rdqbdl_phy+0x44>  // b.lo, b.ul, b.last
 402be24:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402be28:	a94263f7 	ldp	x23, x24, [sp, #32]
 402be2c:	a9437bf9 	ldp	x25, x30, [sp, #48]
 402be30:	a8c453f3 	ldp	x19, x20, [sp], #64
 402be34:	d65f03c0 	ret
 402be38:	97fff1ef 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402be3c:	b9002013 	str	w19, [x0, #32]
 402be40:	d503201f 	nop
 402be44:	9bb85676 	umaddl	x22, w19, w24, x21
 402be48:	97fff1e6 	bl	40285e0 <ddrtrn_hal_get_phy>
 402be4c:	2a1303e1 	mov	w1, w19
 402be50:	9b177c21 	mul	x1, x1, x23
 402be54:	b8616814 	ldr	w20, [x0, x1]
 402be58:	97fff1e7 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402be5c:	b9000414 	str	w20, [x0, #4]
 402be60:	97fff1e5 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402be64:	aa0003f4 	mov	x20, x0
 402be68:	97fff1de 	bl	40285e0 <ddrtrn_hal_get_phy>
 402be6c:	b9402281 	ldr	w1, [x20, #32]
 402be70:	52800014 	mov	w20, #0x0                   	// #0
 402be74:	9b170021 	madd	x1, x1, x23, x0
 402be78:	b9400c39 	ldr	w25, [x1, #12]
 402be7c:	6b19029f 	cmp	w20, w25
 402be80:	54000061 	b.ne	402be8c <ddrtrn_save_rdqbdl_phy+0x98>  // b.any
 402be84:	11000673 	add	w19, w19, #0x1
 402be88:	17ffffe3 	b	402be14 <ddrtrn_save_rdqbdl_phy+0x20>
 402be8c:	97fff1da 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402be90:	b9002414 	str	w20, [x0, #36]
 402be94:	aa1603e0 	mov	x0, x22
 402be98:	11000694 	add	w20, w20, #0x1
 402be9c:	910142d6 	add	x22, x22, #0x50
 402bea0:	97fff9b3 	bl	402a56c <ddrtrn_hal_save_rdqbdl>
 402bea4:	17fffff6 	b	402be7c <ddrtrn_save_rdqbdl_phy+0x88>

000000000402bea8 <ddrtrn_restore_rdqbdl_phy>:
 402bea8:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402beac:	52800013 	mov	w19, #0x0                   	// #0
 402beb0:	a9015bf5 	stp	x21, x22, [sp, #16]
 402beb4:	aa0003f5 	mov	x21, x0
 402beb8:	a90263f7 	stp	x23, x24, [sp, #32]
 402bebc:	d2800797 	mov	x23, #0x3c                  	// #60
 402bec0:	52801418 	mov	w24, #0xa0                  	// #160
 402bec4:	a9037bf9 	stp	x25, x30, [sp, #48]
 402bec8:	97fff1cb 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402becc:	b9400000 	ldr	w0, [x0]
 402bed0:	6b00027f 	cmp	w19, w0
 402bed4:	540000c3 	b.cc	402beec <ddrtrn_restore_rdqbdl_phy+0x44>  // b.lo, b.ul, b.last
 402bed8:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402bedc:	a94263f7 	ldp	x23, x24, [sp, #32]
 402bee0:	a9437bf9 	ldp	x25, x30, [sp, #48]
 402bee4:	a8c453f3 	ldp	x19, x20, [sp], #64
 402bee8:	d65f03c0 	ret
 402beec:	97fff1c2 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bef0:	b9002013 	str	w19, [x0, #32]
 402bef4:	d503201f 	nop
 402bef8:	9bb85676 	umaddl	x22, w19, w24, x21
 402befc:	97fff1b9 	bl	40285e0 <ddrtrn_hal_get_phy>
 402bf00:	2a1303e1 	mov	w1, w19
 402bf04:	9b177c21 	mul	x1, x1, x23
 402bf08:	b8616814 	ldr	w20, [x0, x1]
 402bf0c:	97fff1ba 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bf10:	b9000414 	str	w20, [x0, #4]
 402bf14:	97fff1b8 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bf18:	aa0003f4 	mov	x20, x0
 402bf1c:	97fff1b1 	bl	40285e0 <ddrtrn_hal_get_phy>
 402bf20:	b9402281 	ldr	w1, [x20, #32]
 402bf24:	52800014 	mov	w20, #0x0                   	// #0
 402bf28:	9b170021 	madd	x1, x1, x23, x0
 402bf2c:	b9400c39 	ldr	w25, [x1, #12]
 402bf30:	6b19029f 	cmp	w20, w25
 402bf34:	54000061 	b.ne	402bf40 <ddrtrn_restore_rdqbdl_phy+0x98>  // b.any
 402bf38:	11000673 	add	w19, w19, #0x1
 402bf3c:	17ffffe3 	b	402bec8 <ddrtrn_restore_rdqbdl_phy+0x20>
 402bf40:	97fff1ad 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402bf44:	b9002414 	str	w20, [x0, #36]
 402bf48:	aa1603e0 	mov	x0, x22
 402bf4c:	11000694 	add	w20, w20, #0x1
 402bf50:	910142d6 	add	x22, x22, #0x50
 402bf54:	97fff9a9 	bl	402a5f8 <ddrtrn_hal_restore_rdqbdl>
 402bf58:	17fffff6 	b	402bf30 <ddrtrn_restore_rdqbdl_phy+0x88>

000000000402bf5c <ddrtrn_dataeye_check_dq>:
 402bf5c:	f81f0ffe 	str	x30, [sp, #-16]!
 402bf60:	94000821 	bl	402dfe4 <ddrtrn_hal_get_dq_type>
 402bf64:	7100041f 	cmp	w0, #0x1
 402bf68:	54000061 	b.ne	402bf74 <ddrtrn_dataeye_check_dq+0x18>  // b.any
 402bf6c:	f84107fe 	ldr	x30, [sp], #16
 402bf70:	14000474 	b	402d140 <ddrtrn_ddrt_check>
 402bf74:	52800000 	mov	w0, #0x0                   	// #0
 402bf78:	f84107fe 	ldr	x30, [sp], #16
 402bf7c:	d65f03c0 	ret

000000000402bf80 <ddrtrn_dataeye_search_dq>:
 402bf80:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402bf84:	2a0003f3 	mov	w19, w0
 402bf88:	2a0303f4 	mov	w20, w3
 402bf8c:	a9015bf5 	stp	x21, x22, [sp, #16]
 402bf90:	2a0103f5 	mov	w21, w1
 402bf94:	a90263f7 	stp	x23, x24, [sp, #32]
 402bf98:	aa0203f7 	mov	x23, x2
 402bf9c:	a9037bf9 	stp	x25, x30, [sp, #48]
 402bfa0:	528000d9 	mov	w25, #0x6                   	// #6
 402bfa4:	4b1302a0 	sub	w0, w21, w19
 402bfa8:	2a1303f8 	mov	w24, w19
 402bfac:	0b400673 	add	w19, w19, w0, lsr #1
 402bfb0:	2a1303e0 	mov	w0, w19
 402bfb4:	97fff63f 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 402bfb8:	97ffffe9 	bl	402bf5c <ddrtrn_dataeye_check_dq>
 402bfbc:	71000a9f 	cmp	w20, #0x2
 402bfc0:	54000180 	b.eq	402bff0 <ddrtrn_dataeye_search_dq+0x70>  // b.none
 402bfc4:	7100129f 	cmp	w20, #0x4
 402bfc8:	540001e0 	b.eq	402c004 <ddrtrn_dataeye_search_dq+0x84>  // b.none
 402bfcc:	7100069f 	cmp	w20, #0x1
 402bfd0:	1a9f0336 	csel	w22, w25, wzr, eq  // eq = none
 402bfd4:	35000200 	cbnz	w0, 402c014 <ddrtrn_dataeye_search_dq+0x94>
 402bfd8:	b90002f3 	str	w19, [x23]
 402bfdc:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402bfe0:	a94263f7 	ldp	x23, x24, [sp, #32]
 402bfe4:	a9437bf9 	ldp	x25, x30, [sp, #48]
 402bfe8:	a8c453f3 	ldp	x19, x20, [sp], #64
 402bfec:	d65f03c0 	ret
 402bff0:	35000100 	cbnz	w0, 402c010 <ddrtrn_dataeye_search_dq+0x90>
 402bff4:	2a1803e0 	mov	w0, w24
 402bff8:	97fff62e 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 402bffc:	97ffffd8 	bl	402bf5c <ddrtrn_dataeye_check_dq>
 402c000:	17fffff6 	b	402bfd8 <ddrtrn_dataeye_search_dq+0x58>
 402c004:	350001e0 	cbnz	w0, 402c040 <ddrtrn_dataeye_search_dq+0xc0>
 402c008:	2a1503e0 	mov	w0, w21
 402c00c:	17fffffb 	b	402bff8 <ddrtrn_dataeye_search_dq+0x78>
 402c010:	52800096 	mov	w22, #0x4                   	// #4
 402c014:	6b13031f 	cmp	w24, w19
 402c018:	7a5312a4 	ccmp	w21, w19, #0x4, ne  // ne = any
 402c01c:	54fffe00 	b.eq	402bfdc <ddrtrn_dataeye_search_dq+0x5c>  // b.none
 402c020:	360800d6 	tbz	w22, #1, 402c038 <ddrtrn_dataeye_search_dq+0xb8>
 402c024:	2a1403e3 	mov	w3, w20
 402c028:	aa1703e2 	mov	x2, x23
 402c02c:	2a1303e1 	mov	w1, w19
 402c030:	2a1803e0 	mov	w0, w24
 402c034:	97ffffd3 	bl	402bf80 <ddrtrn_dataeye_search_dq>
 402c038:	3617fd36 	tbz	w22, #2, 402bfdc <ddrtrn_dataeye_search_dq+0x5c>
 402c03c:	17ffffda 	b	402bfa4 <ddrtrn_dataeye_search_dq+0x24>
 402c040:	52800056 	mov	w22, #0x2                   	// #2
 402c044:	17fffff4 	b	402c014 <ddrtrn_dataeye_search_dq+0x94>

000000000402c048 <ddrtrn_dataeye_deskew>:
 402c048:	d10243ff 	sub	sp, sp, #0x90
 402c04c:	a90353f3 	stp	x19, x20, [sp, #48]
 402c050:	52800013 	mov	w19, #0x0                   	// #0
 402c054:	a9045bf5 	stp	x21, x22, [sp, #64]
 402c058:	aa0003f5 	mov	x21, x0
 402c05c:	b0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402c060:	a90563f7 	stp	x23, x24, [sp, #80]
 402c064:	52800016 	mov	w22, #0x0                   	// #0
 402c068:	52800017 	mov	w23, #0x0                   	// #0
 402c06c:	a9066bf9 	stp	x25, x26, [sp, #96]
 402c070:	5280001a 	mov	w26, #0x0                   	// #0
 402c074:	a90773fb 	stp	x27, x28, [sp, #112]
 402c078:	f90043fe 	str	x30, [sp, #128]
 402c07c:	f9455001 	ldr	x1, [x0, #2720]
 402c080:	f90017e1 	str	x1, [sp, #40]
 402c084:	d2800001 	mov	x1, #0x0                   	// #0
 402c088:	940007b4 	bl	402df58 <ddrtrn_hal_dataeye_get_dm>
 402c08c:	b90102bf 	str	wzr, [x21, #256]
 402c090:	b9000be0 	str	w0, [sp, #8]
 402c094:	97fff158 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c098:	b940181c 	ldr	w28, [x0, #24]
 402c09c:	531d7380 	lsl	w0, w28, #3
 402c0a0:	b9000fe0 	str	w0, [sp, #12]
 402c0a4:	97fff154 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c0a8:	b9001c16 	str	w22, [x0, #28]
 402c0ac:	b9400fe0 	ldr	w0, [sp, #12]
 402c0b0:	0b0002db 	add	w27, w22, w0
 402c0b4:	97fff596 	bl	402970c <ddrtrn_hal_phy_get_dq_bdl>
 402c0b8:	2a0003f9 	mov	w25, w0
 402c0bc:	0b0002f7 	add	w23, w23, w0
 402c0c0:	97fff14d 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c0c4:	b9401814 	ldr	w20, [x0, #24]
 402c0c8:	97fff14b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c0cc:	b9401c00 	ldr	w0, [x0, #28]
 402c0d0:	531d7294 	lsl	w20, w20, #3
 402c0d4:	0b000294 	add	w20, w20, w0
 402c0d8:	97fff58d 	bl	402970c <ddrtrn_hal_phy_get_dq_bdl>
 402c0dc:	2a0003f8 	mov	w24, w0
 402c0e0:	b9001fe0 	str	w0, [sp, #28]
 402c0e4:	97ffff9e 	bl	402bf5c <ddrtrn_dataeye_check_dq>
 402c0e8:	340006a0 	cbz	w0, 402c1bc <ddrtrn_dataeye_deskew+0x174>
 402c0ec:	12800000 	mov	w0, #0xffffffff            	// #-1
 402c0f0:	910073e2 	add	x2, sp, #0x1c
 402c0f4:	52800023 	mov	w3, #0x1                   	// #1
 402c0f8:	52800fe1 	mov	w1, #0x7f                  	// #127
 402c0fc:	b9001fe0 	str	w0, [sp, #28]
 402c100:	52800000 	mov	w0, #0x0                   	// #0
 402c104:	97ffff9f 	bl	402bf80 <ddrtrn_dataeye_search_dq>
 402c108:	b9401fe0 	ldr	w0, [sp, #28]
 402c10c:	3100041f 	cmn	w0, #0x1
 402c110:	54000561 	b.ne	402c1bc <ddrtrn_dataeye_deskew+0x174>  // b.any
 402c114:	d37e7e94 	ubfiz	x20, x20, #2, #32
 402c118:	8b1402a0 	add	x0, x21, x20
 402c11c:	b8346abf 	str	wzr, [x21, x20]
 402c120:	b900801f 	str	wzr, [x0, #128]
 402c124:	2a1803e0 	mov	w0, w24
 402c128:	97fff5e2 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 402c12c:	8b3b4abb 	add	x27, x21, w27, uxtw #2
 402c130:	b9408374 	ldr	w20, [x27, #128]
 402c134:	53107e98 	lsr	w24, w20, #16
 402c138:	71001f1f 	cmp	w24, #0x7
 402c13c:	54000b28 	b.hi	402c2a0 <ddrtrn_dataeye_deskew+0x258>  // b.pmore
 402c140:	350009da 	cbnz	w26, 402c278 <ddrtrn_dataeye_deskew+0x230>
 402c144:	510006d6 	sub	w22, w22, #0x1
 402c148:	5280003a 	mov	w26, #0x1                   	// #1
 402c14c:	110006d6 	add	w22, w22, #0x1
 402c150:	710022df 	cmp	w22, #0x8
 402c154:	54fffa81 	b.ne	402c0a4 <ddrtrn_dataeye_deskew+0x5c>  // b.any
 402c158:	b9400be0 	ldr	w0, [sp, #8]
 402c15c:	53037ef7 	lsr	w23, w23, #3
 402c160:	0b530c13 	add	w19, w0, w19, lsr #3
 402c164:	6b17027f 	cmp	w19, w23
 402c168:	54000ae3 	b.cc	402c2c4 <ddrtrn_dataeye_deskew+0x27c>  // b.lo, b.ul, b.last
 402c16c:	4b170273 	sub	w19, w19, w23
 402c170:	52800fe0 	mov	w0, #0x7f                  	// #127
 402c174:	7101fe7f 	cmp	w19, #0x7f
 402c178:	1a809273 	csel	w19, w19, w0, ls  // ls = plast
 402c17c:	97fff11e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c180:	b9401400 	ldr	w0, [x0, #20]
 402c184:	7100081f 	cmp	w0, #0x2
 402c188:	54000061 	b.ne	402c194 <ddrtrn_dataeye_deskew+0x14c>  // b.any
 402c18c:	2a1303e0 	mov	w0, w19
 402c190:	94000783 	bl	402df9c <ddrtrn_hal_dataeye_set_dq_sum>
 402c194:	97fff118 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c198:	b9400400 	ldr	w0, [x0, #4]
 402c19c:	97fff5ae 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 402c1a0:	b0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402c1a4:	f94017e1 	ldr	x1, [sp, #40]
 402c1a8:	f9455002 	ldr	x2, [x0, #2720]
 402c1ac:	eb020021 	subs	x1, x1, x2
 402c1b0:	d2800002 	mov	x2, #0x0                   	// #0
 402c1b4:	540008c0 	b.eq	402c2cc <ddrtrn_dataeye_deskew+0x284>  // b.none
 402c1b8:	97ffdfed 	bl	402416c <__stack_chk_fail>
 402c1bc:	b9401fe1 	ldr	w1, [sp, #28]
 402c1c0:	910083e2 	add	x2, sp, #0x20
 402c1c4:	52800043 	mov	w3, #0x2                   	// #2
 402c1c8:	52800000 	mov	w0, #0x0                   	// #0
 402c1cc:	b90023e1 	str	w1, [sp, #32]
 402c1d0:	97ffff6c 	bl	402bf80 <ddrtrn_dataeye_search_dq>
 402c1d4:	b94023e0 	ldr	w0, [sp, #32]
 402c1d8:	7100001f 	cmp	w0, #0x0
 402c1dc:	5400012d 	b.le	402c200 <ddrtrn_dataeye_deskew+0x1b8>
 402c1e0:	51000400 	sub	w0, w0, #0x1
 402c1e4:	b90023e0 	str	w0, [sp, #32]
 402c1e8:	97fff5b2 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 402c1ec:	97ffff5c 	bl	402bf5c <ddrtrn_dataeye_check_dq>
 402c1f0:	34ffff20 	cbz	w0, 402c1d4 <ddrtrn_dataeye_deskew+0x18c>
 402c1f4:	b94023e0 	ldr	w0, [sp, #32]
 402c1f8:	11000400 	add	w0, w0, #0x1
 402c1fc:	b90023e0 	str	w0, [sp, #32]
 402c200:	b9401fe0 	ldr	w0, [sp, #28]
 402c204:	910093e2 	add	x2, sp, #0x24
 402c208:	52800083 	mov	w3, #0x4                   	// #4
 402c20c:	52800fe1 	mov	w1, #0x7f                  	// #127
 402c210:	b90027e0 	str	w0, [sp, #36]
 402c214:	97ffff5b 	bl	402bf80 <ddrtrn_dataeye_search_dq>
 402c218:	b94027e0 	ldr	w0, [sp, #36]
 402c21c:	7101f81f 	cmp	w0, #0x7e
 402c220:	5400012c 	b.gt	402c244 <ddrtrn_dataeye_deskew+0x1fc>
 402c224:	11000400 	add	w0, w0, #0x1
 402c228:	b90027e0 	str	w0, [sp, #36]
 402c22c:	97fff5a1 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 402c230:	97ffff4b 	bl	402bf5c <ddrtrn_dataeye_check_dq>
 402c234:	34ffff20 	cbz	w0, 402c218 <ddrtrn_dataeye_deskew+0x1d0>
 402c238:	b94027e0 	ldr	w0, [sp, #36]
 402c23c:	51000400 	sub	w0, w0, #0x1
 402c240:	b90027e0 	str	w0, [sp, #36]
 402c244:	2a1803e0 	mov	w0, w24
 402c248:	97fff59a 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 402c24c:	29440be1 	ldp	w1, w2, [sp, #32]
 402c250:	d37e7e94 	ubfiz	x20, x20, #2, #32
 402c254:	8b1402a3 	add	x3, x21, x20
 402c258:	11000440 	add	w0, w2, #0x1
 402c25c:	4b010000 	sub	w0, w0, w1
 402c260:	2a014042 	orr	w2, w2, w1, lsl #16
 402c264:	b8346aa2 	str	w2, [x21, x20]
 402c268:	0b400421 	add	w1, w1, w0, lsr #1
 402c26c:	2a004020 	orr	w0, w1, w0, lsl #16
 402c270:	b9008060 	str	w0, [x3, #128]
 402c274:	17ffffae 	b	402c12c <ddrtrn_dataeye_deskew+0xe4>
 402c278:	35000158 	cbnz	w24, 402c2a0 <ddrtrn_dataeye_deskew+0x258>
 402c27c:	2a1903e0 	mov	w0, w25
 402c280:	97fff58c 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 402c284:	97fff0dc 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c288:	b9400401 	ldr	w1, [x0, #4]
 402c28c:	2a1603e3 	mov	w3, w22
 402c290:	2a1c03e2 	mov	w2, w28
 402c294:	52800800 	mov	w0, #0x40                  	// #64
 402c298:	97fff7ec 	bl	402a248 <ddrtrn_hal_training_stat>
 402c29c:	17ffffac 	b	402c14c <ddrtrn_dataeye_deskew+0x104>
 402c2a0:	12003e94 	and	w20, w20, #0xffff
 402c2a4:	5280001a 	mov	w26, #0x0                   	// #0
 402c2a8:	2a1403e0 	mov	w0, w20
 402c2ac:	97fff581 	bl	40298b0 <ddrtrn_hal_phy_set_dq_bdl>
 402c2b0:	b94102a0 	ldr	w0, [x21, #256]
 402c2b4:	0b140273 	add	w19, w19, w20
 402c2b8:	0b180000 	add	w0, w0, w24
 402c2bc:	b90102a0 	str	w0, [x21, #256]
 402c2c0:	17ffffa3 	b	402c14c <ddrtrn_dataeye_deskew+0x104>
 402c2c4:	52800013 	mov	w19, #0x0                   	// #0
 402c2c8:	17ffffad 	b	402c17c <ddrtrn_dataeye_deskew+0x134>
 402c2cc:	52800000 	mov	w0, #0x0                   	// #0
 402c2d0:	a94353f3 	ldp	x19, x20, [sp, #48]
 402c2d4:	a9445bf5 	ldp	x21, x22, [sp, #64]
 402c2d8:	a94563f7 	ldp	x23, x24, [sp, #80]
 402c2dc:	a9466bf9 	ldp	x25, x26, [sp, #96]
 402c2e0:	a94773fb 	ldp	x27, x28, [sp, #112]
 402c2e4:	f94043fe 	ldr	x30, [sp, #128]
 402c2e8:	910243ff 	add	sp, sp, #0x90
 402c2ec:	d65f03c0 	ret

000000000402c2f0 <ddrtrn_dataeye_process>:
 402c2f0:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402c2f4:	aa0003f4 	mov	x20, x0
 402c2f8:	52800013 	mov	w19, #0x0                   	// #0
 402c2fc:	a9015bf5 	stp	x21, x22, [sp, #16]
 402c300:	52800015 	mov	w21, #0x0                   	// #0
 402c304:	a90263f7 	stp	x23, x24, [sp, #32]
 402c308:	d2800197 	mov	x23, #0xc                   	// #12
 402c30c:	d2800798 	mov	x24, #0x3c                  	// #60
 402c310:	f9001bfe 	str	x30, [sp, #48]
 402c314:	97fff0b8 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c318:	aa0003f6 	mov	x22, x0
 402c31c:	97fff0b1 	bl	40285e0 <ddrtrn_hal_get_phy>
 402c320:	b94022c2 	ldr	w2, [x22, #32]
 402c324:	b9402ac1 	ldr	w1, [x22, #40]
 402c328:	9b187c42 	mul	x2, x2, x24
 402c32c:	9b170821 	madd	x1, x1, x23, x2
 402c330:	8b010000 	add	x0, x0, x1
 402c334:	b9401800 	ldr	w0, [x0, #24]
 402c338:	6b0002bf 	cmp	w21, w0
 402c33c:	54000183 	b.cc	402c36c <ddrtrn_dataeye_process+0x7c>  // b.lo, b.ul, b.last
 402c340:	350002b3 	cbnz	w19, 402c394 <ddrtrn_dataeye_process+0xa4>
 402c344:	aa1403e0 	mov	x0, x20
 402c348:	940007b6 	bl	402e220 <ddrtrn_adjust_dataeye>
 402c34c:	aa1403e0 	mov	x0, x20
 402c350:	94000063 	bl	402c4dc <ddrtrn_result_data_save>
 402c354:	2a1303e0 	mov	w0, w19
 402c358:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402c35c:	a94263f7 	ldp	x23, x24, [sp, #32]
 402c360:	f9401bfe 	ldr	x30, [sp, #48]
 402c364:	a8c453f3 	ldp	x19, x20, [sp], #64
 402c368:	d65f03c0 	ret
 402c36c:	97fff0a2 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c370:	b9402816 	ldr	w22, [x0, #40]
 402c374:	97fff0a0 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c378:	0b1606b6 	add	w22, w21, w22, lsl #1
 402c37c:	b9001816 	str	w22, [x0, #24]
 402c380:	110006b5 	add	w21, w21, #0x1
 402c384:	aa1403e0 	mov	x0, x20
 402c388:	97ffff30 	bl	402c048 <ddrtrn_dataeye_deskew>
 402c38c:	0b000273 	add	w19, w19, w0
 402c390:	17ffffe1 	b	402c314 <ddrtrn_dataeye_process+0x24>
 402c394:	12800013 	mov	w19, #0xffffffff            	// #-1
 402c398:	17ffffed 	b	402c34c <ddrtrn_dataeye_process+0x5c>

000000000402c39c <ddrtrn_dataeye_training>:
 402c39c:	d10483ff 	sub	sp, sp, #0x120
 402c3a0:	b0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402c3a4:	a9117bf3 	stp	x19, x30, [sp, #272]
 402c3a8:	f9455001 	ldr	x1, [x0, #2720]
 402c3ac:	f90087e1 	str	x1, [sp, #264]
 402c3b0:	d2800001 	mov	x1, #0x0                   	// #0
 402c3b4:	97fff090 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c3b8:	52800041 	mov	w1, #0x2                   	// #2
 402c3bc:	b9001401 	str	w1, [x0, #20]
 402c3c0:	52802082 	mov	w2, #0x104                 	// #260
 402c3c4:	52800001 	mov	w1, #0x0                   	// #0
 402c3c8:	910003e0 	mov	x0, sp
 402c3cc:	97fff94e 	bl	402a904 <ddrtrn_set_data>
 402c3d0:	910003e0 	mov	x0, sp
 402c3d4:	97ffffc7 	bl	402c2f0 <ddrtrn_dataeye_process>
 402c3d8:	2a0003f3 	mov	w19, w0
 402c3dc:	97fff086 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c3e0:	52800021 	mov	w1, #0x1                   	// #1
 402c3e4:	b9001401 	str	w1, [x0, #20]
 402c3e8:	52802082 	mov	w2, #0x104                 	// #260
 402c3ec:	52800001 	mov	w1, #0x0                   	// #0
 402c3f0:	910003e0 	mov	x0, sp
 402c3f4:	97fff944 	bl	402a904 <ddrtrn_set_data>
 402c3f8:	910003e0 	mov	x0, sp
 402c3fc:	97ffffbd 	bl	402c2f0 <ddrtrn_dataeye_process>
 402c400:	2a000273 	orr	w19, w19, w0
 402c404:	b0ffffa1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 402c408:	7100027f 	cmp	w19, #0x0
 402c40c:	5a9f03e0 	csetm	w0, ne  // ne = any
 402c410:	f94087e2 	ldr	x2, [sp, #264]
 402c414:	f9455023 	ldr	x3, [x1, #2720]
 402c418:	eb030042 	subs	x2, x2, x3
 402c41c:	d2800003 	mov	x3, #0x0                   	// #0
 402c420:	54000040 	b.eq	402c428 <ddrtrn_dataeye_training+0x8c>  // b.none
 402c424:	97ffdf52 	bl	402416c <__stack_chk_fail>
 402c428:	a9517bf3 	ldp	x19, x30, [sp, #272]
 402c42c:	910483ff 	add	sp, sp, #0x120
 402c430:	d65f03c0 	ret

000000000402c434 <ddrtrn_dataeye_training_func>:
 402c434:	d10183ff 	sub	sp, sp, #0x60
 402c438:	b0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402c43c:	a9057bf3 	stp	x19, x30, [sp, #80]
 402c440:	f9455001 	ldr	x1, [x0, #2720]
 402c444:	f90027e1 	str	x1, [sp, #72]
 402c448:	d2800001 	mov	x1, #0x0                   	// #0
 402c44c:	52800020 	mov	w0, #0x1                   	// #1
 402c450:	940006e9 	bl	402dff4 <ddrtrn_hal_set_dq_type>
 402c454:	52a00020 	mov	w0, #0x10000               	// #65536
 402c458:	97fff4f0 	bl	4029818 <ddrtrn_hal_check_bypass>
 402c45c:	35000280 	cbnz	w0, 402c4ac <ddrtrn_dataeye_training_func+0x78>
 402c460:	910023e0 	add	x0, sp, #0x8
 402c464:	52a00021 	mov	w1, #0x10000               	// #65536
 402c468:	97fff319 	bl	40290cc <ddrtrn_hal_save_reg>
 402c46c:	97fff381 	bl	4029270 <ddrtrn_hal_training_switch_axi>
 402c470:	52800040 	mov	w0, #0x2                   	// #2
 402c474:	94000798 	bl	402e2d4 <ddrtrn_ddrt_init>
 402c478:	52800020 	mov	w0, #0x1                   	// #1
 402c47c:	97fff907 	bl	402a898 <ddrtrn_hal_set_adjust>
 402c480:	97ffffc7 	bl	402c39c <ddrtrn_dataeye_training>
 402c484:	2a0003f3 	mov	w19, w0
 402c488:	910023e0 	add	x0, sp, #0x8
 402c48c:	97fff354 	bl	40291dc <ddrtrn_hal_restore_reg>
 402c490:	b0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402c494:	f94027e1 	ldr	x1, [sp, #72]
 402c498:	f9455002 	ldr	x2, [x0, #2720]
 402c49c:	eb020021 	subs	x1, x1, x2
 402c4a0:	d2800002 	mov	x2, #0x0                   	// #0
 402c4a4:	54000080 	b.eq	402c4b4 <ddrtrn_dataeye_training_func+0x80>  // b.none
 402c4a8:	97ffdf31 	bl	402416c <__stack_chk_fail>
 402c4ac:	52800013 	mov	w19, #0x0                   	// #0
 402c4b0:	17fffff8 	b	402c490 <ddrtrn_dataeye_training_func+0x5c>
 402c4b4:	2a1303e0 	mov	w0, w19
 402c4b8:	a9457bf3 	ldp	x19, x30, [sp, #80]
 402c4bc:	910183ff 	add	sp, sp, #0x60
 402c4c0:	d65f03c0 	ret

000000000402c4c4 <bsp_ddrtrn_resume>:
 402c4c4:	1400030d 	b	402d0f8 <ddrtrn_resume>

000000000402c4c8 <bsp_ddrtrn_suspend>:
 402c4c8:	f81f0ffe 	str	x30, [sp, #-16]!
 402c4cc:	9400030a 	bl	402d0f4 <ddrtrn_suspend_store_para>
 402c4d0:	52800000 	mov	w0, #0x0                   	// #0
 402c4d4:	f84107fe 	ldr	x30, [sp], #16
 402c4d8:	d65f03c0 	ret

000000000402c4dc <ddrtrn_result_data_save>:
 402c4dc:	d65f03c0 	ret

000000000402c4e0 <ddrtrn_lpca_data_save>:
 402c4e0:	d65f03c0 	ret

000000000402c4e4 <ddrtrn_ddrt_get_test_addr>:
 402c4e4:	52a80000 	mov	w0, #0x40000000            	// #1073741824
 402c4e8:	d65f03c0 	ret

000000000402c4ec <ddrtrn_training_error>:
 402c4ec:	d65f03c0 	ret

000000000402c4f0 <ddrtrn_training_success>:
 402c4f0:	d65f03c0 	ret

000000000402c4f4 <ddrtrn_training_start>:
 402c4f4:	d65f03c0 	ret

000000000402c4f8 <ddrtrn_hal_get_ck_phase>:
 402c4f8:	52820e02 	mov	w2, #0x1070                	// #4208
 402c4fc:	0b020000 	add	w0, w0, w2
 402c500:	b9400000 	ldr	w0, [x0]
 402c504:	53197c02 	lsr	w2, w0, #25
 402c508:	d3462003 	ubfx	x3, x0, #6, #3
 402c50c:	531d0042 	ubfiz	w2, w2, #3, #1
 402c510:	2a030042 	orr	w2, w2, w3
 402c514:	b9000022 	str	w2, [x1]
 402c518:	53187c02 	lsr	w2, w0, #24
 402c51c:	d3492c00 	ubfx	x0, x0, #9, #3
 402c520:	531d0042 	ubfiz	w2, w2, #3, #1
 402c524:	2a000040 	orr	w0, w2, w0
 402c528:	b9000420 	str	w0, [x1, #4]
 402c52c:	d65f03c0 	ret

000000000402c530 <ddrtrn_hal_low_freq_cfg_freq_process>:
 402c530:	a9bb53f3 	stp	x19, x20, [sp, #-80]!
 402c534:	d2840013 	mov	x19, #0x2000                	// #8192
 402c538:	f2a22033 	movk	x19, #0x1101, lsl #16
 402c53c:	a9015bf5 	stp	x21, x22, [sp, #16]
 402c540:	d2803215 	mov	x21, #0x190                 	// #400
 402c544:	f2a22035 	movk	x21, #0x1101, lsl #16
 402c548:	a90263f7 	stp	x23, x24, [sp, #32]
 402c54c:	2a0003f4 	mov	w20, w0
 402c550:	b9400277 	ldr	w23, [x19]
 402c554:	b9400260 	ldr	w0, [x19]
 402c558:	b94002b6 	ldr	w22, [x21]
 402c55c:	120d72e0 	and	w0, w23, #0xfff8ffff
 402c560:	a9036bf9 	stp	x25, x26, [sp, #48]
 402c564:	2a0103f9 	mov	w25, w1
 402c568:	f90023fe 	str	x30, [sp, #64]
 402c56c:	b9000260 	str	w0, [x19]
 402c570:	5289c400 	mov	w0, #0x4e20                	// #20000
 402c574:	97fff730 	bl	402a234 <ddrtrn_hal_training_delay>
 402c578:	b9400278 	ldr	w24, [x19]
 402c57c:	12117300 	and	w0, w24, #0xffff8fff
 402c580:	b9000260 	str	w0, [x19]
 402c584:	5289c400 	mov	w0, #0x4e20                	// #20000
 402c588:	97fff72b 	bl	402a234 <ddrtrn_hal_training_delay>
 402c58c:	d2840400 	mov	x0, #0x2020                	// #8224
 402c590:	f2a22020 	movk	x0, #0x1101, lsl #16
 402c594:	b9400000 	ldr	w0, [x0]
 402c598:	f274081f 	tst	x0, #0x7000
 402c59c:	540004e1 	b.ne	402c638 <ddrtrn_hal_low_freq_cfg_freq_process+0x108>  // b.any
 402c5a0:	528000a0 	mov	w0, #0x5                   	// #5
 402c5a4:	2a0002c0 	orr	w0, w22, w0
 402c5a8:	b90002a0 	str	w0, [x21]
 402c5ac:	5290d400 	mov	w0, #0x86a0                	// #34464
 402c5b0:	72a00020 	movk	w0, #0x1, lsl #16
 402c5b4:	d280309a 	mov	x26, #0x184                 	// #388
 402c5b8:	97fff71f 	bl	402a234 <ddrtrn_hal_training_delay>
 402c5bc:	f2a2203a 	movk	x26, #0x1101, lsl #16
 402c5c0:	128000a0 	mov	w0, #0xfffffffa            	// #-6
 402c5c4:	0a0002c0 	and	w0, w22, w0
 402c5c8:	321e0000 	orr	w0, w0, #0x4
 402c5cc:	b90002a0 	str	w0, [x21]
 402c5d0:	320c0280 	orr	w0, w20, #0x100000
 402c5d4:	b9000340 	str	w0, [x26]
 402c5d8:	d2803000 	mov	x0, #0x180                 	// #384
 402c5dc:	120b7a94 	and	w20, w20, #0xffefffff
 402c5e0:	f2a22020 	movk	x0, #0x1101, lsl #16
 402c5e4:	b9000019 	str	w25, [x0]
 402c5e8:	5280fa00 	mov	w0, #0x7d0                 	// #2000
 402c5ec:	97fff712 	bl	402a234 <ddrtrn_hal_training_delay>
 402c5f0:	b9000354 	str	w20, [x26]
 402c5f4:	5281a800 	mov	w0, #0xd40                 	// #3392
 402c5f8:	72a00060 	movk	w0, #0x3, lsl #16
 402c5fc:	97fff70e 	bl	402a234 <ddrtrn_hal_training_delay>
 402c600:	321e02c0 	orr	w0, w22, #0x4
 402c604:	b90002a0 	str	w0, [x21]
 402c608:	5280fa00 	mov	w0, #0x7d0                 	// #2000
 402c60c:	97fff70a 	bl	402a234 <ddrtrn_hal_training_delay>
 402c610:	a9436bf9 	ldp	x25, x26, [sp, #48]
 402c614:	5289c400 	mov	w0, #0x4e20                	// #20000
 402c618:	f94023fe 	ldr	x30, [sp, #64]
 402c61c:	b90002b6 	str	w22, [x21]
 402c620:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402c624:	b9000278 	str	w24, [x19]
 402c628:	b9000277 	str	w23, [x19]
 402c62c:	a94263f7 	ldp	x23, x24, [sp, #32]
 402c630:	a8c553f3 	ldp	x19, x20, [sp], #80
 402c634:	17fff700 	b	402a234 <ddrtrn_hal_training_delay>
 402c638:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402c63c:	a94263f7 	ldp	x23, x24, [sp, #32]
 402c640:	a9436bf9 	ldp	x25, x26, [sp, #48]
 402c644:	f94023fe 	ldr	x30, [sp, #64]
 402c648:	a8c553f3 	ldp	x19, x20, [sp], #80
 402c64c:	d65f03c0 	ret

000000000402c650 <ddrtrn_hal_get_phy_addr>:
 402c650:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402c654:	2a0003f3 	mov	w19, w0
 402c658:	97ffefe2 	bl	40285e0 <ddrtrn_hal_get_phy>
 402c65c:	d2800781 	mov	x1, #0x3c                  	// #60
 402c660:	9b017e73 	mul	x19, x19, x1
 402c664:	b8736800 	ldr	w0, [x0, x19]
 402c668:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402c66c:	d65f03c0 	ret

000000000402c670 <ddrtrn_hal_get_cur_phy_total_byte_num>:
 402c670:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402c674:	97ffefe0 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c678:	aa0003f3 	mov	x19, x0
 402c67c:	97ffefd9 	bl	40285e0 <ddrtrn_hal_get_phy>
 402c680:	d2800782 	mov	x2, #0x3c                  	// #60
 402c684:	b9402261 	ldr	w1, [x19, #32]
 402c688:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402c68c:	d503201f 	nop
 402c690:	9b020021 	madd	x1, x1, x2, x0
 402c694:	b9401020 	ldr	w0, [x1, #16]
 402c698:	d65f03c0 	ret

000000000402c69c <ddrtrn_hal_get_phy_num>:
 402c69c:	f81f0ffe 	str	x30, [sp, #-16]!
 402c6a0:	97ffefd5 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c6a4:	b9400000 	ldr	w0, [x0]
 402c6a8:	f84107fe 	ldr	x30, [sp], #16
 402c6ac:	d65f03c0 	ret

000000000402c6b0 <ddrtrn_hal_low_freq_pll_power>:
 402c6b0:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402c6b4:	2a0003f4 	mov	w20, w0
 402c6b8:	52800013 	mov	w19, #0x0                   	// #0
 402c6bc:	a9017bf5 	stp	x21, x30, [sp, #16]
 402c6c0:	d2800795 	mov	x21, #0x3c                  	// #60
 402c6c4:	97fffff6 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402c6c8:	6b13001f 	cmp	w0, w19
 402c6cc:	540000a8 	b.hi	402c6e0 <ddrtrn_hal_low_freq_pll_power+0x30>  // b.pmore
 402c6d0:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402c6d4:	5289c400 	mov	w0, #0x4e20                	// #20000
 402c6d8:	a8c253f3 	ldp	x19, x20, [sp], #32
 402c6dc:	17fff6d6 	b	402a234 <ddrtrn_hal_training_delay>
 402c6e0:	97ffefc0 	bl	40285e0 <ddrtrn_hal_get_phy>
 402c6e4:	2a1303e1 	mov	w1, w19
 402c6e8:	9b150021 	madd	x1, x1, x21, x0
 402c6ec:	b9403020 	ldr	w0, [x1, #48]
 402c6f0:	34000100 	cbz	w0, 402c710 <ddrtrn_hal_low_freq_pll_power+0x60>
 402c6f4:	2a1303e0 	mov	w0, w19
 402c6f8:	97ffffd6 	bl	402c650 <ddrtrn_hal_get_phy_addr>
 402c6fc:	11006002 	add	w2, w0, #0x18
 402c700:	b9400041 	ldr	w1, [x2]
 402c704:	121d7021 	and	w1, w1, #0xfffffff8
 402c708:	2a140021 	orr	w1, w1, w20
 402c70c:	b9000041 	str	w1, [x2]
 402c710:	11000673 	add	w19, w19, #0x1
 402c714:	17ffffec 	b	402c6c4 <ddrtrn_hal_low_freq_pll_power+0x14>

000000000402c718 <ddrtrn_pll_lock>:
 402c718:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402c71c:	52800013 	mov	w19, #0x0                   	// #0
 402c720:	52800034 	mov	w20, #0x1                   	// #1
 402c724:	f9000bfe 	str	x30, [sp, #16]
 402c728:	97ffffdd 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402c72c:	6b13001f 	cmp	w0, w19
 402c730:	540000a8 	b.hi	402c744 <ddrtrn_pll_lock+0x2c>  // b.pmore
 402c734:	2a1403e0 	mov	w0, w20
 402c738:	f9400bfe 	ldr	x30, [sp, #16]
 402c73c:	a8c253f3 	ldp	x19, x20, [sp], #32
 402c740:	d65f03c0 	ret
 402c744:	2a1303e0 	mov	w0, w19
 402c748:	97ffffc2 	bl	402c650 <ddrtrn_hal_get_phy_addr>
 402c74c:	11026000 	add	w0, w0, #0x98
 402c750:	11000673 	add	w19, w19, #0x1
 402c754:	b9400000 	ldr	w0, [x0]
 402c758:	0a407e94 	and	w20, w20, w0, lsr #31
 402c75c:	17fffff3 	b	402c728 <ddrtrn_pll_lock+0x10>

000000000402c760 <ddrtrn_hal_low_freq_phy_clk>:
 402c760:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402c764:	2a0003f4 	mov	w20, w0
 402c768:	52800013 	mov	w19, #0x0                   	// #0
 402c76c:	a9017bf5 	stp	x21, x30, [sp, #16]
 402c770:	d2800795 	mov	x21, #0x3c                  	// #60
 402c774:	97ffffca 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402c778:	6b13001f 	cmp	w0, w19
 402c77c:	54000088 	b.hi	402c78c <ddrtrn_hal_low_freq_phy_clk+0x2c>  // b.pmore
 402c780:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402c784:	a8c253f3 	ldp	x19, x20, [sp], #32
 402c788:	d65f03c0 	ret
 402c78c:	97ffef95 	bl	40285e0 <ddrtrn_hal_get_phy>
 402c790:	2a1303e1 	mov	w1, w19
 402c794:	9b150021 	madd	x1, x1, x21, x0
 402c798:	b9403020 	ldr	w0, [x1, #48]
 402c79c:	34000100 	cbz	w0, 402c7bc <ddrtrn_hal_low_freq_phy_clk+0x5c>
 402c7a0:	2a1303e0 	mov	w0, w19
 402c7a4:	97ffffab 	bl	402c650 <ddrtrn_hal_get_phy_addr>
 402c7a8:	11003002 	add	w2, w0, #0xc
 402c7ac:	b9400041 	ldr	w1, [x2]
 402c7b0:	12113c21 	and	w1, w1, #0x7fff8000
 402c7b4:	2a140021 	orr	w1, w1, w20
 402c7b8:	b9000041 	str	w1, [x2]
 402c7bc:	11000673 	add	w19, w19, #0x1
 402c7c0:	17ffffed 	b	402c774 <ddrtrn_hal_low_freq_phy_clk+0x14>

000000000402c7c4 <ddrtrn_hal_low_freq_ctrl_easr>:
 402c7c4:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402c7c8:	aa0003f3 	mov	x19, x0
 402c7cc:	a9015bf5 	stp	x21, x22, [sp, #16]
 402c7d0:	2a0103f5 	mov	w21, w1
 402c7d4:	a9027bf7 	stp	x23, x30, [sp, #32]
 402c7d8:	97ffffb1 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402c7dc:	7100041f 	cmp	w0, #0x1
 402c7e0:	54000108 	b.hi	402c800 <ddrtrn_hal_low_freq_ctrl_easr+0x3c>  // b.pmore
 402c7e4:	52800014 	mov	w20, #0x0                   	// #0
 402c7e8:	d2800797 	mov	x23, #0x3c                  	// #60
 402c7ec:	97ffffac 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402c7f0:	6b00029f 	cmp	w20, w0
 402c7f4:	54000103 	b.cc	402c814 <ddrtrn_hal_low_freq_ctrl_easr+0x50>  // b.lo, b.ul, b.last
 402c7f8:	52800000 	mov	w0, #0x0                   	// #0
 402c7fc:	14000002 	b	402c804 <ddrtrn_hal_low_freq_ctrl_easr+0x40>
 402c800:	12800000 	mov	w0, #0xffffffff            	// #-1
 402c804:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402c808:	a9427bf7 	ldp	x23, x30, [sp, #32]
 402c80c:	a8c353f3 	ldp	x19, x20, [sp], #48
 402c810:	d65f03c0 	ret
 402c814:	97ffef78 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c818:	b9002014 	str	w20, [x0, #32]
 402c81c:	97ffef76 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c820:	aa0003f6 	mov	x22, x0
 402c824:	97ffef6f 	bl	40285e0 <ddrtrn_hal_get_phy>
 402c828:	b94022c1 	ldr	w1, [x22, #32]
 402c82c:	9b170021 	madd	x1, x1, x23, x0
 402c830:	b9400420 	ldr	w0, [x1, #4]
 402c834:	7100181f 	cmp	w0, #0x6
 402c838:	540000c1 	b.ne	402c850 <ddrtrn_hal_low_freq_ctrl_easr+0x8c>  // b.any
 402c83c:	aa1303e0 	mov	x0, x19
 402c840:	97fff88b 	bl	402aa6c <ddrtrn_sref_cfg_save>
 402c844:	aa1303e0 	mov	x0, x19
 402c848:	52800181 	mov	w1, #0xc                   	// #12
 402c84c:	97fff9fb 	bl	402b038 <ddrtrn_sref_cfg>
 402c850:	2a1503e0 	mov	w0, w21
 402c854:	97fff8a6 	bl	402aaec <ddrtrn_training_ctrl_easr>
 402c858:	35fffd40 	cbnz	w0, 402c800 <ddrtrn_hal_low_freq_ctrl_easr+0x3c>
 402c85c:	97ffef66 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402c860:	aa0003f6 	mov	x22, x0
 402c864:	97ffef5f 	bl	40285e0 <ddrtrn_hal_get_phy>
 402c868:	b94022c1 	ldr	w1, [x22, #32]
 402c86c:	9b170021 	madd	x1, x1, x23, x0
 402c870:	b9400420 	ldr	w0, [x1, #4]
 402c874:	7100181f 	cmp	w0, #0x6
 402c878:	54000061 	b.ne	402c884 <ddrtrn_hal_low_freq_ctrl_easr+0xc0>  // b.any
 402c87c:	aa1303e0 	mov	x0, x19
 402c880:	97fff88c 	bl	402aab0 <ddrtrn_sref_cfg_restore>
 402c884:	11000694 	add	w20, w20, #0x1
 402c888:	17ffffd9 	b	402c7ec <ddrtrn_hal_low_freq_ctrl_easr+0x28>

000000000402c88c <ddrtrn_low_freq_start>:
 402c88c:	a9ba53f3 	stp	x19, x20, [sp, #-96]!
 402c890:	52802402 	mov	w2, #0x120                 	// #288
 402c894:	52807d14 	mov	w20, #0x3e8                 	// #1000
 402c898:	a9015bf5 	stp	x21, x22, [sp, #16]
 402c89c:	2a0003f6 	mov	w22, w0
 402c8a0:	b0ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402c8a4:	a90263f7 	stp	x23, x24, [sp, #32]
 402c8a8:	52955555 	mov	w21, #0xaaaa                	// #43690
 402c8ac:	52955557 	mov	w23, #0xaaaa                	// #43690
 402c8b0:	a9036bf9 	stp	x25, x26, [sp, #48]
 402c8b4:	52955558 	mov	w24, #0xaaaa                	// #43690
 402c8b8:	72a23555 	movk	w21, #0x11aa, lsl #16
 402c8bc:	a90473fb 	stp	x27, x28, [sp, #64]
 402c8c0:	72a29557 	movk	w23, #0x14aa, lsl #16
 402c8c4:	72a25558 	movk	w24, #0x12aa, lsl #16
 402c8c8:	f9002bfe 	str	x30, [sp, #80]
 402c8cc:	d10943ff 	sub	sp, sp, #0x250
 402c8d0:	f9455001 	ldr	x1, [x0, #2720]
 402c8d4:	f90127e1 	str	x1, [sp, #584]
 402c8d8:	d2800001 	mov	x1, #0x0                   	// #0
 402c8dc:	9104a3e0 	add	x0, sp, #0x128
 402c8e0:	97fff809 	bl	402a904 <ddrtrn_set_data>
 402c8e4:	910223e0 	add	x0, sp, #0x88
 402c8e8:	97fffd43 	bl	402bdf4 <ddrtrn_save_rdqbdl_phy>
 402c8ec:	d2803080 	mov	x0, #0x184                 	// #388
 402c8f0:	f2a22020 	movk	x0, #0x1101, lsl #16
 402c8f4:	b9400000 	ldr	w0, [x0]
 402c8f8:	b9000be0 	str	w0, [sp, #8]
 402c8fc:	d2803000 	mov	x0, #0x180                 	// #384
 402c900:	f2a22020 	movk	x0, #0x1101, lsl #16
 402c904:	b9400000 	ldr	w0, [x0]
 402c908:	b90013e0 	str	w0, [sp, #16]
 402c90c:	52800000 	mov	w0, #0x0                   	// #0
 402c910:	97ffff94 	bl	402c760 <ddrtrn_hal_low_freq_phy_clk>
 402c914:	528000e0 	mov	w0, #0x7                   	// #7
 402c918:	97ffff66 	bl	402c6b0 <ddrtrn_hal_low_freq_pll_power>
 402c91c:	97ffef31 	bl	40285e0 <ddrtrn_hal_get_phy>
 402c920:	b9400000 	ldr	w0, [x0]
 402c924:	1101e000 	add	w0, w0, #0x78
 402c928:	b9400000 	ldr	w0, [x0]
 402c92c:	72000400 	ands	w0, w0, #0x3
 402c930:	54000260 	b.eq	402c97c <ddrtrn_low_freq_start+0xf0>  // b.none
 402c934:	7100041f 	cmp	w0, #0x1
 402c938:	1a9812e1 	csel	w1, w23, w24, ne  // ne = any
 402c93c:	52800013 	mov	w19, #0x0                   	// #0
 402c940:	5284e219 	mov	w25, #0x2710                	// #10000
 402c944:	52820420 	mov	w0, #0x1021                	// #4129
 402c948:	97fffefa 	bl	402c530 <ddrtrn_hal_low_freq_cfg_freq_process>
 402c94c:	52800000 	mov	w0, #0x0                   	// #0
 402c950:	97ffff58 	bl	402c6b0 <ddrtrn_hal_low_freq_pll_power>
 402c954:	97ffff71 	bl	402c718 <ddrtrn_pll_lock>
 402c958:	6b19027f 	cmp	w19, w25
 402c95c:	350001c0 	cbnz	w0, 402c994 <ddrtrn_low_freq_start+0x108>
 402c960:	54000121 	b.ne	402c984 <ddrtrn_low_freq_start+0xf8>  // b.any
 402c964:	71000694 	subs	w20, w20, #0x1
 402c968:	54fffd61 	b.ne	402c914 <ddrtrn_low_freq_start+0x88>  // b.any
 402c96c:	f0000000 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 402c970:	91042800 	add	x0, x0, #0x10a
 402c974:	97ffecdc 	bl	4027ce4 <log_serial_puts>
 402c978:	14000008 	b	402c998 <ddrtrn_low_freq_start+0x10c>
 402c97c:	2a1503e1 	mov	w1, w21
 402c980:	17ffffef 	b	402c93c <ddrtrn_low_freq_start+0xb0>
 402c984:	5280fa00 	mov	w0, #0x7d0                 	// #2000
 402c988:	11000673 	add	w19, w19, #0x1
 402c98c:	97fff62a 	bl	402a234 <ddrtrn_hal_training_delay>
 402c990:	17fffff1 	b	402c954 <ddrtrn_low_freq_start+0xc8>
 402c994:	54fffe80 	b.eq	402c964 <ddrtrn_low_freq_start+0xd8>  // b.none
 402c998:	52800013 	mov	w19, #0x0                   	// #0
 402c99c:	32013fe0 	mov	w0, #0x80007fff            	// #-2147450881
 402c9a0:	97ffff70 	bl	402c760 <ddrtrn_hal_low_freq_phy_clk>
 402c9a4:	97ffff3e 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402c9a8:	6b13001f 	cmp	w0, w19
 402c9ac:	54000488 	b.hi	402ca3c <ddrtrn_low_freq_start+0x1b0>  // b.pmore
 402c9b0:	528281a0 	mov	w0, #0x140d                	// #5133
 402c9b4:	72a00480 	movk	w0, #0x24, lsl #16
 402c9b8:	0a0002c0 	and	w0, w22, w0
 402c9bc:	97fff373 	bl	4029788 <ddrtrn_hal_hw_item_cfg>
 402c9c0:	97fff96c 	bl	402af70 <ddrtrn_hw_training>
 402c9c4:	b9000fe0 	str	w0, [sp, #12]
 402c9c8:	97ffff35 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402c9cc:	7100041f 	cmp	w0, #0x1
 402c9d0:	54000108 	b.hi	402c9f0 <ddrtrn_low_freq_start+0x164>  // b.pmore
 402c9d4:	52800015 	mov	w21, #0x0                   	// #0
 402c9d8:	52800a17 	mov	w23, #0x50                  	// #80
 402c9dc:	d2800798 	mov	x24, #0x3c                  	// #60
 402c9e0:	52800199 	mov	w25, #0xc                   	// #12
 402c9e4:	97ffff2e 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402c9e8:	6b0002bf 	cmp	w21, w0
 402c9ec:	54000323 	b.cc	402ca50 <ddrtrn_low_freq_start+0x1c4>  // b.lo, b.ul, b.last
 402c9f0:	52800220 	mov	w0, #0x11                  	// #17
 402c9f4:	97fff365 	bl	4029788 <ddrtrn_hal_hw_item_cfg>
 402c9f8:	d280079a 	mov	x26, #0x3c                  	// #60
 402c9fc:	97fff95d 	bl	402af70 <ddrtrn_hw_training>
 402ca00:	97ffeefd 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ca04:	b900201f 	str	wzr, [x0, #32]
 402ca08:	97ffeefb 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ca0c:	aa0003f4 	mov	x20, x0
 402ca10:	97ffeef4 	bl	40285e0 <ddrtrn_hal_get_phy>
 402ca14:	b9402281 	ldr	w1, [x20, #32]
 402ca18:	9b1a7c21 	mul	x1, x1, x26
 402ca1c:	b8616814 	ldr	w20, [x0, x1]
 402ca20:	97ffeef5 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ca24:	b9000414 	str	w20, [x0, #4]
 402ca28:	97ffeeee 	bl	40285e0 <ddrtrn_hal_get_phy>
 402ca2c:	b9403000 	ldr	w0, [x0, #48]
 402ca30:	34000800 	cbz	w0, 402cb30 <ddrtrn_low_freq_start+0x2a4>
 402ca34:	52800015 	mov	w21, #0x0                   	// #0
 402ca38:	14000036 	b	402cb10 <ddrtrn_low_freq_start+0x284>
 402ca3c:	2a1303e0 	mov	w0, w19
 402ca40:	11000673 	add	w19, w19, #0x1
 402ca44:	97ffff03 	bl	402c650 <ddrtrn_hal_get_phy_addr>
 402ca48:	97fff391 	bl	402988c <ddrtrn_hal_ck_cfg>
 402ca4c:	17ffffd6 	b	402c9a4 <ddrtrn_low_freq_start+0x118>
 402ca50:	9bb77eba 	umull	x26, w21, w23
 402ca54:	2a1503e0 	mov	w0, w21
 402ca58:	97fffefe 	bl	402c650 <ddrtrn_hal_get_phy_addr>
 402ca5c:	2a0003f4 	mov	w20, w0
 402ca60:	9100e3e2 	add	x2, sp, #0x38
 402ca64:	91010341 	add	x1, x26, #0x40
 402ca68:	8b010041 	add	x1, x2, x1
 402ca6c:	97fffea3 	bl	402c4f8 <ddrtrn_hal_get_ck_phase>
 402ca70:	97ffeedc 	bl	40285e0 <ddrtrn_hal_get_phy>
 402ca74:	2a1503e1 	mov	w1, w21
 402ca78:	9b187c3b 	mul	x27, x1, x24
 402ca7c:	8b1b0000 	add	x0, x0, x27
 402ca80:	b9401000 	ldr	w0, [x0, #16]
 402ca84:	7100101f 	cmp	w0, #0x4
 402ca88:	54fffb48 	b.hi	402c9f0 <ddrtrn_low_freq_start+0x164>  // b.pmore
 402ca8c:	9100e3e0 	add	x0, sp, #0x38
 402ca90:	1108b294 	add	w20, w20, #0x22c
 402ca94:	8b1a001a 	add	x26, x0, x26
 402ca98:	52800013 	mov	w19, #0x0                   	// #0
 402ca9c:	97ffeed1 	bl	40285e0 <ddrtrn_hal_get_phy>
 402caa0:	8b1b0000 	add	x0, x0, x27
 402caa4:	b9401000 	ldr	w0, [x0, #16]
 402caa8:	6b00027f 	cmp	w19, w0
 402caac:	54000063 	b.cc	402cab8 <ddrtrn_low_freq_start+0x22c>  // b.lo, b.ul, b.last
 402cab0:	110006b5 	add	w21, w21, #0x1
 402cab4:	17ffffcc 	b	402c9e4 <ddrtrn_low_freq_start+0x158>
 402cab8:	2a1403e0 	mov	w0, w20
 402cabc:	11000673 	add	w19, w19, #0x1
 402cac0:	11020294 	add	w20, w20, #0x80
 402cac4:	9100135a 	add	x26, x26, #0x4
 402cac8:	b9400000 	ldr	w0, [x0]
 402cacc:	d3506000 	ubfx	x0, x0, #16, #9
 402cad0:	b81fc340 	stur	w0, [x26, #-4]
 402cad4:	1ad90800 	udiv	w0, w0, w25
 402cad8:	b9001f40 	str	w0, [x26, #28]
 402cadc:	17fffff0 	b	402ca9c <ddrtrn_low_freq_start+0x210>
 402cae0:	97ffeec5 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402cae4:	b9002415 	str	w21, [x0, #36]
 402cae8:	97ffeec3 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402caec:	b9402014 	ldr	w20, [x0, #32]
 402caf0:	97ffeec1 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402caf4:	b9402419 	ldr	w25, [x0, #36]
 402caf8:	97ffeebf 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402cafc:	b9400418 	ldr	w24, [x0, #4]
 402cb00:	97fffedc 	bl	402c670 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402cb04:	7100101f 	cmp	w0, #0x4
 402cb08:	540002c9 	b.ls	402cb60 <ddrtrn_low_freq_start+0x2d4>  // b.plast
 402cb0c:	110006b5 	add	w21, w21, #0x1
 402cb10:	97ffeeb9 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402cb14:	aa0003f4 	mov	x20, x0
 402cb18:	97ffeeb2 	bl	40285e0 <ddrtrn_hal_get_phy>
 402cb1c:	b9402281 	ldr	w1, [x20, #32]
 402cb20:	9b1a0021 	madd	x1, x1, x26, x0
 402cb24:	b9400c20 	ldr	w0, [x1, #12]
 402cb28:	6b0002bf 	cmp	w21, w0
 402cb2c:	54fffda3 	b.cc	402cae0 <ddrtrn_low_freq_start+0x254>  // b.lo, b.ul, b.last
 402cb30:	9100c3e0 	add	x0, sp, #0x30
 402cb34:	52800021 	mov	w1, #0x1                   	// #1
 402cb38:	97ffff23 	bl	402c7c4 <ddrtrn_hal_low_freq_ctrl_easr>
 402cb3c:	340008e0 	cbz	w0, 402cc58 <ddrtrn_low_freq_start+0x3cc>
 402cb40:	12800000 	mov	w0, #0xffffffff            	// #-1
 402cb44:	b0ffffa1 	adrp	x1, 4021000 <hash_verify_buf-0xa80>
 402cb48:	f94127e2 	ldr	x2, [sp, #584]
 402cb4c:	f9455023 	ldr	x3, [x1, #2720]
 402cb50:	eb030042 	subs	x2, x2, x3
 402cb54:	d2800003 	mov	x3, #0x0                   	// #0
 402cb58:	54002be0 	b.eq	402d0d4 <ddrtrn_low_freq_start+0x848>  // b.none
 402cb5c:	97ffdd84 	bl	402416c <__stack_chk_fail>
 402cb60:	52801200 	mov	w0, #0x90                  	// #144
 402cb64:	9100e3e1 	add	x1, sp, #0x38
 402cb68:	11084305 	add	w5, w24, #0x210
 402cb6c:	52800013 	mov	w19, #0x0                   	// #0
 402cb70:	9ba07f24 	umull	x4, w25, w0
 402cb74:	52802400 	mov	w0, #0x120                 	// #288
 402cb78:	0b1928bc 	add	w28, w5, w25, lsl #10
 402cb7c:	9ba01284 	umaddl	x4, w20, w0, x4
 402cb80:	9104a3e0 	add	x0, sp, #0x128
 402cb84:	8b04001b 	add	x27, x0, x4
 402cb88:	52800a00 	mov	w0, #0x50                  	// #80
 402cb8c:	aa1b03f7 	mov	x23, x27
 402cb90:	9ba00694 	umaddl	x20, w20, w0, x1
 402cb94:	97fffeb7 	bl	402c670 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402cb98:	6b00027f 	cmp	w19, w0
 402cb9c:	54fffb82 	b.cs	402cb0c <ddrtrn_low_freq_start+0x280>  // b.hs, b.nlast
 402cba0:	2a1903e2 	mov	w2, w25
 402cba4:	2a1803e1 	mov	w1, w24
 402cba8:	aa1703e0 	mov	x0, x23
 402cbac:	2a1303e3 	mov	w3, w19
 402cbb0:	97fff3c9 	bl	4029ad4 <ddrtrn_hal_get_dly_value>
 402cbb4:	b9400762 	ldr	w2, [x27, #4]
 402cbb8:	52800081 	mov	w1, #0x4                   	// #4
 402cbbc:	11000440 	add	w0, w2, #0x1
 402cbc0:	1ac10c00 	sdiv	w0, w0, w1
 402cbc4:	4b000042 	sub	w2, w2, w0
 402cbc8:	71003c5f 	cmp	w2, #0xf
 402cbcc:	54fffa0c 	b.gt	402cb0c <ddrtrn_low_freq_start+0x280>
 402cbd0:	b9401b61 	ldr	w1, [x27, #24]
 402cbd4:	71000c3f 	cmp	w1, #0x3
 402cbd8:	54fff9a8 	b.hi	402cb0c <ddrtrn_low_freq_start+0x280>  // b.pmore
 402cbdc:	b9400287 	ldr	w7, [x20]
 402cbe0:	7107fcff 	cmp	w7, #0x1ff
 402cbe4:	54fff948 	b.hi	402cb0c <ddrtrn_low_freq_start+0x280>  // b.pmore
 402cbe8:	b9402286 	ldr	w6, [x20, #32]
 402cbec:	9100937b 	add	x27, x27, #0x24
 402cbf0:	910092f7 	add	x23, x23, #0x24
 402cbf4:	91001294 	add	x20, x20, #0x4
 402cbf8:	7100a8df 	cmp	w6, #0x2a
 402cbfc:	54fff888 	b.hi	402cb0c <ddrtrn_low_freq_start+0x280>  // b.pmore
 402cc00:	b85e4360 	ldur	w0, [x27, #-28]
 402cc04:	11000673 	add	w19, w19, #0x1
 402cc08:	1b0200c2 	madd	w2, w6, w2, w0
 402cc0c:	51000420 	sub	w0, w1, #0x1
 402cc10:	b85f8361 	ldur	w1, [x27, #-8]
 402cc14:	1b070800 	madd	w0, w0, w7, w2
 402cc18:	0b010000 	add	w0, w0, w1
 402cc1c:	293f0360 	stp	w0, w0, [x27, #-8]
 402cc20:	2a1c03e0 	mov	w0, w28
 402cc24:	b900001f 	str	wzr, [x0]
 402cc28:	11001380 	add	w0, w28, #0x4
 402cc2c:	b900001f 	str	wzr, [x0]
 402cc30:	11002380 	add	w0, w28, #0x8
 402cc34:	b900001f 	str	wzr, [x0]
 402cc38:	11008380 	add	w0, w28, #0x20
 402cc3c:	b900001f 	str	wzr, [x0]
 402cc40:	11009380 	add	w0, w28, #0x24
 402cc44:	1102039c 	add	w28, w28, #0x80
 402cc48:	b900001f 	str	wzr, [x0]
 402cc4c:	2a1803e0 	mov	w0, w24
 402cc50:	97fff301 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 402cc54:	17ffffd0 	b	402cb94 <ddrtrn_low_freq_start+0x308>
 402cc58:	52807d15 	mov	w21, #0x3e8                 	// #1000
 402cc5c:	5284e217 	mov	w23, #0x2710                	// #10000
 402cc60:	97fffec0 	bl	402c760 <ddrtrn_hal_low_freq_phy_clk>
 402cc64:	528000e0 	mov	w0, #0x7                   	// #7
 402cc68:	97fffe92 	bl	402c6b0 <ddrtrn_hal_low_freq_pll_power>
 402cc6c:	b9400be0 	ldr	w0, [sp, #8]
 402cc70:	52800014 	mov	w20, #0x0                   	// #0
 402cc74:	b94013e1 	ldr	w1, [sp, #16]
 402cc78:	97fffe2e 	bl	402c530 <ddrtrn_hal_low_freq_cfg_freq_process>
 402cc7c:	52800000 	mov	w0, #0x0                   	// #0
 402cc80:	97fffe8c 	bl	402c6b0 <ddrtrn_hal_low_freq_pll_power>
 402cc84:	97fffea5 	bl	402c718 <ddrtrn_pll_lock>
 402cc88:	6b17029f 	cmp	w20, w23
 402cc8c:	35000180 	cbnz	w0, 402ccbc <ddrtrn_low_freq_start+0x430>
 402cc90:	540000e1 	b.ne	402ccac <ddrtrn_low_freq_start+0x420>  // b.any
 402cc94:	710006b5 	subs	w21, w21, #0x1
 402cc98:	54fffe61 	b.ne	402cc64 <ddrtrn_low_freq_start+0x3d8>  // b.any
 402cc9c:	f0000000 	adrp	x0, 402f000 <malloc_sizes_ro+0x190>
 402cca0:	91042800 	add	x0, x0, #0x10a
 402cca4:	97ffec10 	bl	4027ce4 <log_serial_puts>
 402cca8:	14000006 	b	402ccc0 <ddrtrn_low_freq_start+0x434>
 402ccac:	5280fa00 	mov	w0, #0x7d0                 	// #2000
 402ccb0:	11000694 	add	w20, w20, #0x1
 402ccb4:	97fff560 	bl	402a234 <ddrtrn_hal_training_delay>
 402ccb8:	17fffff3 	b	402cc84 <ddrtrn_low_freq_start+0x3f8>
 402ccbc:	54fffec0 	b.eq	402cc94 <ddrtrn_low_freq_start+0x408>  // b.none
 402ccc0:	52800014 	mov	w20, #0x0                   	// #0
 402ccc4:	32013fe0 	mov	w0, #0x80007fff            	// #-2147450881
 402ccc8:	97fffea6 	bl	402c760 <ddrtrn_hal_low_freq_phy_clk>
 402cccc:	97fffe74 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402ccd0:	6b14001f 	cmp	w0, w20
 402ccd4:	540003c8 	b.hi	402cd4c <ddrtrn_low_freq_start+0x4c0>  // b.pmore
 402ccd8:	9100c3e0 	add	x0, sp, #0x30
 402ccdc:	52800041 	mov	w1, #0x2                   	// #2
 402cce0:	97fffeb9 	bl	402c7c4 <ddrtrn_hal_low_freq_ctrl_easr>
 402cce4:	35fff2e0 	cbnz	w0, 402cb40 <ddrtrn_low_freq_start+0x2b4>
 402cce8:	528000a0 	mov	w0, #0x5                   	// #5
 402ccec:	0a0002c0 	and	w0, w22, w0
 402ccf0:	97fff2a6 	bl	4029788 <ddrtrn_hal_hw_item_cfg>
 402ccf4:	97fff89f 	bl	402af70 <ddrtrn_hw_training>
 402ccf8:	b9400fe1 	ldr	w1, [sp, #12]
 402ccfc:	0b000020 	add	w0, w1, w0
 402cd00:	b9001be0 	str	w0, [sp, #24]
 402cd04:	97fffe66 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402cd08:	7100041f 	cmp	w0, #0x1
 402cd0c:	54000789 	b.ls	402cdfc <ddrtrn_low_freq_start+0x570>  // b.plast
 402cd10:	52800015 	mov	w21, #0x0                   	// #0
 402cd14:	d280029a 	mov	x26, #0x14                  	// #20
 402cd18:	97fffe61 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402cd1c:	6b0002bf 	cmp	w21, w0
 402cd20:	54000783 	b.cc	402ce10 <ddrtrn_low_freq_start+0x584>  // b.lo, b.ul, b.last
 402cd24:	910223e0 	add	x0, sp, #0x88
 402cd28:	97fffc60 	bl	402bea8 <ddrtrn_restore_rdqbdl_phy>
 402cd2c:	529d7ea0 	mov	w0, #0xebf5                	// #60405
 402cd30:	72bffb60 	movk	w0, #0xffdb, lsl #16
 402cd34:	0a0002c0 	and	w0, w22, w0
 402cd38:	97fff294 	bl	4029788 <ddrtrn_hal_hw_item_cfg>
 402cd3c:	97fff88d 	bl	402af70 <ddrtrn_hw_training>
 402cd40:	b9401be1 	ldr	w1, [sp, #24]
 402cd44:	0b000020 	add	w0, w1, w0
 402cd48:	17ffff7f 	b	402cb44 <ddrtrn_low_freq_start+0x2b8>
 402cd4c:	2a1403e0 	mov	w0, w20
 402cd50:	11000694 	add	w20, w20, #0x1
 402cd54:	97fffe3f 	bl	402c650 <ddrtrn_hal_get_phy_addr>
 402cd58:	97fff2cd 	bl	402988c <ddrtrn_hal_ck_cfg>
 402cd5c:	17ffffdc 	b	402cccc <ddrtrn_low_freq_start+0x440>
 402cd60:	9bb77eb3 	umull	x19, w21, w23
 402cd64:	2a1503e0 	mov	w0, w21
 402cd68:	2a1503fa 	mov	w26, w21
 402cd6c:	97fffe39 	bl	402c650 <ddrtrn_hal_get_phy_addr>
 402cd70:	9100e3e2 	add	x2, sp, #0x38
 402cd74:	91012261 	add	x1, x19, #0x48
 402cd78:	8b010041 	add	x1, x2, x1
 402cd7c:	2a0003f4 	mov	w20, w0
 402cd80:	9b187f5a 	mul	x26, x26, x24
 402cd84:	97fffddd 	bl	402c4f8 <ddrtrn_hal_get_ck_phase>
 402cd88:	97ffee16 	bl	40285e0 <ddrtrn_hal_get_phy>
 402cd8c:	8b1a0000 	add	x0, x0, x26
 402cd90:	b9401000 	ldr	w0, [x0, #16]
 402cd94:	7100101f 	cmp	w0, #0x4
 402cd98:	54fffbc8 	b.hi	402cd10 <ddrtrn_low_freq_start+0x484>  // b.pmore
 402cd9c:	9100e3e0 	add	x0, sp, #0x38
 402cda0:	1108b294 	add	w20, w20, #0x22c
 402cda4:	8b130013 	add	x19, x0, x19
 402cda8:	5280001b 	mov	w27, #0x0                   	// #0
 402cdac:	97ffee0d 	bl	40285e0 <ddrtrn_hal_get_phy>
 402cdb0:	8b1a0000 	add	x0, x0, x26
 402cdb4:	91001273 	add	x19, x19, #0x4
 402cdb8:	b9401000 	ldr	w0, [x0, #16]
 402cdbc:	6b00037f 	cmp	w27, w0
 402cdc0:	540000c3 	b.cc	402cdd8 <ddrtrn_low_freq_start+0x54c>  // b.lo, b.ul, b.last
 402cdc4:	110006b5 	add	w21, w21, #0x1
 402cdc8:	97fffe35 	bl	402c69c <ddrtrn_hal_get_phy_num>
 402cdcc:	6b0002bf 	cmp	w21, w0
 402cdd0:	54fffc83 	b.cc	402cd60 <ddrtrn_low_freq_start+0x4d4>  // b.lo, b.ul, b.last
 402cdd4:	17ffffcf 	b	402cd10 <ddrtrn_low_freq_start+0x484>
 402cdd8:	2a1403e0 	mov	w0, w20
 402cddc:	1100077b 	add	w27, w27, #0x1
 402cde0:	11020294 	add	w20, w20, #0x80
 402cde4:	b9400000 	ldr	w0, [x0]
 402cde8:	d3506000 	ubfx	x0, x0, #16, #9
 402cdec:	b9000e60 	str	w0, [x19, #12]
 402cdf0:	1ad90800 	udiv	w0, w0, w25
 402cdf4:	b9002e60 	str	w0, [x19, #44]
 402cdf8:	17ffffed 	b	402cdac <ddrtrn_low_freq_start+0x520>
 402cdfc:	52800015 	mov	w21, #0x0                   	// #0
 402ce00:	52800a17 	mov	w23, #0x50                  	// #80
 402ce04:	d2800798 	mov	x24, #0x3c                  	// #60
 402ce08:	52800199 	mov	w25, #0xc                   	// #12
 402ce0c:	17ffffef 	b	402cdc8 <ddrtrn_low_freq_start+0x53c>
 402ce10:	97ffedf9 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ce14:	b9002015 	str	w21, [x0, #32]
 402ce18:	2a1503e0 	mov	w0, w21
 402ce1c:	97fffe0d 	bl	402c650 <ddrtrn_hal_get_phy_addr>
 402ce20:	2a0003f3 	mov	w19, w0
 402ce24:	97ffedf4 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ce28:	b9000413 	str	w19, [x0, #4]
 402ce2c:	97ffeded 	bl	40285e0 <ddrtrn_hal_get_phy>
 402ce30:	2a1503e1 	mov	w1, w21
 402ce34:	d2800782 	mov	x2, #0x3c                  	// #60
 402ce38:	9b020021 	madd	x1, x1, x2, x0
 402ce3c:	b9403020 	ldr	w0, [x1, #48]
 402ce40:	35001460 	cbnz	w0, 402d0cc <ddrtrn_low_freq_start+0x840>
 402ce44:	110006b5 	add	w21, w21, #0x1
 402ce48:	17ffffb4 	b	402cd18 <ddrtrn_low_freq_start+0x48c>
 402ce4c:	97ffedea 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ce50:	b9002419 	str	w25, [x0, #36]
 402ce54:	97ffede8 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ce58:	b940201b 	ldr	w27, [x0, #32]
 402ce5c:	97ffede6 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ce60:	b9402413 	ldr	w19, [x0, #36]
 402ce64:	97fffe03 	bl	402c670 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402ce68:	7100101f 	cmp	w0, #0x4
 402ce6c:	54000228 	b.hi	402ceb0 <ddrtrn_low_freq_start+0x624>  // b.pmore
 402ce70:	52801200 	mov	w0, #0x90                  	// #144
 402ce74:	9100e3f8 	add	x24, sp, #0x38
 402ce78:	52800a14 	mov	w20, #0x50                  	// #80
 402ce7c:	52800017 	mov	w23, #0x0                   	// #0
 402ce80:	9ba07e60 	umull	x0, w19, w0
 402ce84:	52802413 	mov	w19, #0x120                 	// #288
 402ce88:	9bb46374 	umaddl	x20, w27, w20, x24
 402ce8c:	9bb30373 	umaddl	x19, w27, w19, x0
 402ce90:	2a1b03fb 	mov	w27, w27
 402ce94:	9104a3e0 	add	x0, sp, #0x128
 402ce98:	8b130013 	add	x19, x0, x19
 402ce9c:	9b1a7f60 	mul	x0, x27, x26
 402cea0:	f90013e0 	str	x0, [sp, #32]
 402cea4:	97fffdf3 	bl	402c670 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402cea8:	6b0002ff 	cmp	w23, w0
 402ceac:	54000183 	b.cc	402cedc <ddrtrn_low_freq_start+0x650>  // b.lo, b.ul, b.last
 402ceb0:	11000739 	add	w25, w25, #0x1
 402ceb4:	97ffedd0 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402ceb8:	aa0003f3 	mov	x19, x0
 402cebc:	97ffedc9 	bl	40285e0 <ddrtrn_hal_get_phy>
 402cec0:	d2800782 	mov	x2, #0x3c                  	// #60
 402cec4:	b9402261 	ldr	w1, [x19, #32]
 402cec8:	9b020021 	madd	x1, x1, x2, x0
 402cecc:	b9400c20 	ldr	w0, [x1, #12]
 402ced0:	6b00033f 	cmp	w25, w0
 402ced4:	54fffbc3 	b.cc	402ce4c <ddrtrn_low_freq_start+0x5c0>  // b.lo, b.ul, b.last
 402ced8:	17ffffdb 	b	402ce44 <ddrtrn_low_freq_start+0x5b8>
 402cedc:	97ffedc6 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402cee0:	b9001817 	str	w23, [x0, #24]
 402cee4:	f94013e0 	ldr	x0, [sp, #32]
 402cee8:	710006ff 	cmp	w23, #0x1
 402ceec:	9a809401 	cinc	x1, x0, hi  // hi = pmore
 402cef0:	91004020 	add	x0, x1, #0x10
 402cef4:	91004821 	add	x1, x1, #0x12
 402cef8:	b8607b02 	ldr	w2, [x24, x0, lsl #2]
 402cefc:	b8617b01 	ldr	w1, [x24, x1, lsl #2]
 402cf00:	11000440 	add	w0, w2, #0x1
 402cf04:	4b400840 	sub	w0, w2, w0, lsr #2
 402cf08:	11000422 	add	w2, w1, #0x1
 402cf0c:	4b420822 	sub	w2, w1, w2, lsr #2
 402cf10:	71003c1f 	cmp	w0, #0xf
 402cf14:	54fffce8 	b.hi	402ceb0 <ddrtrn_low_freq_start+0x624>  // b.pmore
 402cf18:	71003c5f 	cmp	w2, #0xf
 402cf1c:	54fffca8 	b.hi	402ceb0 <ddrtrn_low_freq_start+0x624>  // b.pmore
 402cf20:	b9402283 	ldr	w3, [x20, #32]
 402cf24:	7100a87f 	cmp	w3, #0x2a
 402cf28:	54fffc48 	b.hi	402ceb0 <ddrtrn_low_freq_start+0x624>  // b.pmore
 402cf2c:	b9403281 	ldr	w1, [x20, #48]
 402cf30:	7100a83f 	cmp	w1, #0x2a
 402cf34:	54fffbe8 	b.hi	402ceb0 <ddrtrn_low_freq_start+0x624>  // b.pmore
 402cf38:	b9402264 	ldr	w4, [x19, #32]
 402cf3c:	5280003b 	mov	w27, #0x1                   	// #1
 402cf40:	1b011041 	madd	w1, w2, w1, w4
 402cf44:	1b038401 	msub	w1, w0, w3, w1
 402cf48:	b9401283 	ldr	w3, [x20, #16]
 402cf4c:	12800040 	mov	w0, #0xfffffffd            	// #-3
 402cf50:	1ac00c60 	sdiv	w0, w3, w0
 402cf54:	0b010000 	add	w0, w0, w1
 402cf58:	6a407f7f 	tst	w27, w0, lsr #31
 402cf5c:	540008c1 	b.ne	402d074 <ddrtrn_low_freq_start+0x7e8>  // b.any
 402cf60:	6b00007f 	cmp	w3, w0
 402cf64:	5400006c 	b.gt	402cf70 <ddrtrn_low_freq_start+0x6e4>
 402cf68:	71000b7f 	cmp	w27, #0x2
 402cf6c:	540008a1 	b.ne	402d080 <ddrtrn_low_freq_start+0x7f4>  // b.any
 402cf70:	7101fc1f 	cmp	w0, #0x7f
 402cf74:	52800fe1 	mov	w1, #0x7f                  	// #127
 402cf78:	1a81d01c 	csel	w28, w0, w1, le
 402cf7c:	97ffed9e 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402cf80:	b9400400 	ldr	w0, [x0, #4]
 402cf84:	b9000fe0 	str	w0, [sp, #12]
 402cf88:	97ffed9b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402cf8c:	b9402400 	ldr	w0, [x0, #36]
 402cf90:	b9002be0 	str	w0, [sp, #40]
 402cf94:	97ffed98 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402cf98:	b9401800 	ldr	w0, [x0, #24]
 402cf9c:	b9000be0 	str	w0, [sp, #8]
 402cfa0:	7100039f 	cmp	w28, #0x0
 402cfa4:	b9400260 	ldr	w0, [x19]
 402cfa8:	1a9fa381 	csel	w1, w28, wzr, ge  // ge = tcont
 402cfac:	b9001fe0 	str	w0, [sp, #28]
 402cfb0:	5280001c 	mov	w28, #0x0                   	// #0
 402cfb4:	b9400be0 	ldr	w0, [sp, #8]
 402cfb8:	f9000be0 	str	x0, [sp, #16]
 402cfbc:	b9002fe1 	str	w1, [sp, #44]
 402cfc0:	97ffed8d 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402cfc4:	b9402000 	ldr	w0, [x0, #32]
 402cfc8:	f9400be1 	ldr	x1, [sp, #16]
 402cfcc:	9b1a0400 	madd	x0, x0, x26, x1
 402cfd0:	b9402fe1 	ldr	w1, [sp, #44]
 402cfd4:	91003000 	add	x0, x0, #0xc
 402cfd8:	b8607b00 	ldr	w0, [x24, x0, lsl #2]
 402cfdc:	6b00003f 	cmp	w1, w0
 402cfe0:	54000069 	b.ls	402cfec <ddrtrn_low_freq_start+0x760>  // b.plast
 402cfe4:	7100179f 	cmp	w28, #0x5
 402cfe8:	54000529 	b.ls	402d08c <ddrtrn_low_freq_start+0x800>  // b.plast
 402cfec:	b9401fe0 	ldr	w0, [sp, #28]
 402cff0:	110006f7 	add	w23, w23, #0x1
 402cff4:	b9402be2 	ldr	w2, [sp, #40]
 402cff8:	91001294 	add	x20, x20, #0x4
 402cffc:	12196000 	and	w0, w0, #0xffffff80
 402d000:	91009273 	add	x19, x19, #0x24
 402d004:	2a000021 	orr	w1, w1, w0
 402d008:	b9400be0 	ldr	w0, [sp, #8]
 402d00c:	12146c21 	and	w1, w1, #0xfffff0ff
 402d010:	2a1c2021 	orr	w1, w1, w28, lsl #8
 402d014:	53196000 	lsl	w0, w0, #7
 402d018:	0b022800 	add	w0, w0, w2, lsl #10
 402d01c:	b9400fe2 	ldr	w2, [sp, #12]
 402d020:	0b020000 	add	w0, w0, w2
 402d024:	1108c003 	add	w3, w0, #0x230
 402d028:	b9000061 	str	w1, [x3]
 402d02c:	b85f0261 	ldur	w1, [x19, #-16]
 402d030:	120e7421 	and	w1, w1, #0xfffcffff
 402d034:	2a1b4022 	orr	w2, w1, w27, lsl #16
 402d038:	1108e001 	add	w1, w0, #0x238
 402d03c:	b9000022 	str	w2, [x1]
 402d040:	1108d002 	add	w2, w0, #0x234
 402d044:	b85e8261 	ldur	w1, [x19, #-24]
 402d048:	12126421 	and	w1, w1, #0xffffc0ff
 402d04c:	b9000041 	str	w1, [x2]
 402d050:	11084001 	add	w1, w0, #0x210
 402d054:	b900003f 	str	wzr, [x1]
 402d058:	11085001 	add	w1, w0, #0x214
 402d05c:	11086000 	add	w0, w0, #0x218
 402d060:	b900003f 	str	wzr, [x1]
 402d064:	b900001f 	str	wzr, [x0]
 402d068:	b9400fe0 	ldr	w0, [sp, #12]
 402d06c:	97fff1fa 	bl	4029854 <ddrtrn_hal_phy_cfg_update>
 402d070:	17ffff8d 	b	402cea4 <ddrtrn_low_freq_start+0x618>
 402d074:	0b030000 	add	w0, w0, w3
 402d078:	5280001b 	mov	w27, #0x0                   	// #0
 402d07c:	17ffffb7 	b	402cf58 <ddrtrn_low_freq_start+0x6cc>
 402d080:	4b030000 	sub	w0, w0, w3
 402d084:	1100077b 	add	w27, w27, #0x1
 402d088:	17ffffb6 	b	402cf60 <ddrtrn_low_freq_start+0x6d4>
 402d08c:	b9002fe1 	str	w1, [sp, #44]
 402d090:	97ffed59 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d094:	b9402000 	ldr	w0, [x0, #32]
 402d098:	f9400be1 	ldr	x1, [sp, #16]
 402d09c:	9b1a0400 	madd	x0, x0, x26, x1
 402d0a0:	b9402fe1 	ldr	w1, [sp, #44]
 402d0a4:	91003000 	add	x0, x0, #0xc
 402d0a8:	b8607b00 	ldr	w0, [x24, x0, lsl #2]
 402d0ac:	4b000021 	sub	w1, w1, w0
 402d0b0:	11000780 	add	w0, w28, #0x1
 402d0b4:	12000404 	and	w4, w0, #0x3
 402d0b8:	71000c9f 	cmp	w4, #0x3
 402d0bc:	54000041 	b.ne	402d0c4 <ddrtrn_low_freq_start+0x838>  // b.any
 402d0c0:	11000b80 	add	w0, w28, #0x2
 402d0c4:	2a0003fc 	mov	w28, w0
 402d0c8:	17ffffbd 	b	402cfbc <ddrtrn_low_freq_start+0x730>
 402d0cc:	52800019 	mov	w25, #0x0                   	// #0
 402d0d0:	17ffff79 	b	402ceb4 <ddrtrn_low_freq_start+0x628>
 402d0d4:	910943ff 	add	sp, sp, #0x250
 402d0d8:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402d0dc:	a94263f7 	ldp	x23, x24, [sp, #32]
 402d0e0:	a9436bf9 	ldp	x25, x26, [sp, #48]
 402d0e4:	a94473fb 	ldp	x27, x28, [sp, #64]
 402d0e8:	f9402bfe 	ldr	x30, [sp, #80]
 402d0ec:	a8c653f3 	ldp	x19, x20, [sp], #96
 402d0f0:	d65f03c0 	ret

000000000402d0f4 <ddrtrn_suspend_store_para>:
 402d0f4:	d65f03c0 	ret

000000000402d0f8 <ddrtrn_resume>:
 402d0f8:	52800000 	mov	w0, #0x0                   	// #0
 402d0fc:	d65f03c0 	ret

000000000402d100 <ddrtrn_hal_dmc_auto_pd_by_phy>:
 402d100:	1100a021 	add	w1, w1, #0x28
 402d104:	b9400023 	ldr	w3, [x1]
 402d108:	f27c2c7f 	tst	x3, #0xfff0
 402d10c:	54000180 	b.eq	402d13c <ddrtrn_hal_dmc_auto_pd_by_phy+0x3c>  // b.none
 402d110:	1100b000 	add	w0, w0, #0x2c
 402d114:	b9400004 	ldr	w4, [x0]
 402d118:	32000060 	orr	w0, w3, #0x1
 402d11c:	b9000020 	str	w0, [x1]
 402d120:	12000c83 	and	w3, w4, #0xf
 402d124:	7100187f 	cmp	w3, #0x6
 402d128:	540000a1 	b.ne	402d13c <ddrtrn_hal_dmc_auto_pd_by_phy+0x3c>  // b.any
 402d12c:	1100a042 	add	w2, w2, #0x28
 402d130:	b9400040 	ldr	w0, [x2]
 402d134:	32000000 	orr	w0, w0, #0x1
 402d138:	b9000040 	str	w0, [x2]
 402d13c:	d65f03c0 	ret

000000000402d140 <ddrtrn_ddrt_check>:
 402d140:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402d144:	a9017bf5 	stp	x21, x30, [sp, #16]
 402d148:	97ffed2b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d14c:	b9401813 	ldr	w19, [x0, #24]
 402d150:	97ffed29 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d154:	b9401c14 	ldr	w20, [x0, #28]
 402d158:	97ffed27 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d15c:	b9401815 	ldr	w21, [x0, #24]
 402d160:	97ffed25 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d164:	b9402800 	ldr	w0, [x0, #40]
 402d168:	6b0006bf 	cmp	w21, w0, lsl #1
 402d16c:	540000c3 	b.cc	402d184 <ddrtrn_ddrt_check+0x44>  // b.lo, b.ul, b.last
 402d170:	97ffed21 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d174:	b9401815 	ldr	w21, [x0, #24]
 402d178:	97ffed1f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d17c:	b9402813 	ldr	w19, [x0, #40]
 402d180:	4b1306b3 	sub	w19, w21, w19, lsl #1
 402d184:	52800801 	mov	w1, #0x40                  	// #64
 402d188:	528330c0 	mov	w0, #0x1986                	// #6534
 402d18c:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d190:	72a224a0 	movk	w0, #0x1125, lsl #16
 402d194:	97fff12a 	bl	402963c <ddrt_reg_write>
 402d198:	52800881 	mov	w1, #0x44                  	// #68
 402d19c:	528330c0 	mov	w0, #0x1986                	// #6534
 402d1a0:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d1a4:	72a224a0 	movk	w0, #0x1125, lsl #16
 402d1a8:	97fff125 	bl	402963c <ddrt_reg_write>
 402d1ac:	52800901 	mov	w1, #0x48                  	// #72
 402d1b0:	528330c0 	mov	w0, #0x1986                	// #6534
 402d1b4:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d1b8:	72a224a0 	movk	w0, #0x1125, lsl #16
 402d1bc:	97fff120 	bl	402963c <ddrt_reg_write>
 402d1c0:	52800981 	mov	w1, #0x4c                  	// #76
 402d1c4:	528330c0 	mov	w0, #0x1986                	// #6534
 402d1c8:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d1cc:	72a224a0 	movk	w0, #0x1125, lsl #16
 402d1d0:	97fff11b 	bl	402963c <ddrt_reg_write>
 402d1d4:	52800a01 	mov	w1, #0x50                  	// #80
 402d1d8:	528330c0 	mov	w0, #0x1986                	// #6534
 402d1dc:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d1e0:	72a224a0 	movk	w0, #0x1125, lsl #16
 402d1e4:	97fff116 	bl	402963c <ddrt_reg_write>
 402d1e8:	52800a81 	mov	w1, #0x54                  	// #84
 402d1ec:	528330c0 	mov	w0, #0x1986                	// #6534
 402d1f0:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d1f4:	72a224a0 	movk	w0, #0x1125, lsl #16
 402d1f8:	97fff111 	bl	402963c <ddrt_reg_write>
 402d1fc:	52800b01 	mov	w1, #0x58                  	// #88
 402d200:	528330c0 	mov	w0, #0x1986                	// #6534
 402d204:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d208:	72a224a0 	movk	w0, #0x1125, lsl #16
 402d20c:	97fff10c 	bl	402963c <ddrt_reg_write>
 402d210:	52800b81 	mov	w1, #0x5c                  	// #92
 402d214:	528330c0 	mov	w0, #0x1986                	// #6534
 402d218:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d21c:	72a224a0 	movk	w0, #0x1125, lsl #16
 402d220:	97fff107 	bl	402963c <ddrt_reg_write>
 402d224:	52800601 	mov	w1, #0x30                  	// #48
 402d228:	52800000 	mov	w0, #0x0                   	// #0
 402d22c:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d230:	97fff103 	bl	402963c <ddrt_reg_write>
 402d234:	2a1403e2 	mov	w2, w20
 402d238:	2a1303e1 	mov	w1, w19
 402d23c:	52800000 	mov	w0, #0x0                   	// #0
 402d240:	9400047c 	bl	402e430 <ddrtrn_ddrt_test>
 402d244:	340000a0 	cbz	w0, 402d258 <ddrtrn_ddrt_check+0x118>
 402d248:	12800000 	mov	w0, #0xffffffff            	// #-1
 402d24c:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402d250:	a8c253f3 	ldp	x19, x20, [sp], #32
 402d254:	d65f03c0 	ret
 402d258:	97ffece7 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d25c:	b9401000 	ldr	w0, [x0, #16]
 402d260:	52800601 	mov	w1, #0x30                  	// #48
 402d264:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d268:	97fff0f5 	bl	402963c <ddrt_reg_write>
 402d26c:	2a1403e2 	mov	w2, w20
 402d270:	2a1303e1 	mov	w1, w19
 402d274:	52800000 	mov	w0, #0x0                   	// #0
 402d278:	9400046e 	bl	402e430 <ddrtrn_ddrt_test>
 402d27c:	35fffe60 	cbnz	w0, 402d248 <ddrtrn_ddrt_check+0x108>
 402d280:	52800801 	mov	w1, #0x40                  	// #64
 402d284:	528330c0 	mov	w0, #0x1986                	// #6534
 402d288:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d28c:	72a224a0 	movk	w0, #0x1125, lsl #16
 402d290:	97fff0eb 	bl	402963c <ddrt_reg_write>
 402d294:	52800881 	mov	w1, #0x44                  	// #68
 402d298:	5284a320 	mov	w0, #0x2519                	// #9497
 402d29c:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d2a0:	72b6c220 	movk	w0, #0xb611, lsl #16
 402d2a4:	97fff0e6 	bl	402963c <ddrt_reg_write>
 402d2a8:	52800901 	mov	w1, #0x48                  	// #72
 402d2ac:	528224a0 	mov	w0, #0x1125                	// #4389
 402d2b0:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d2b4:	72b756c0 	movk	w0, #0xbab6, lsl #16
 402d2b8:	97fff0e1 	bl	402963c <ddrt_reg_write>
 402d2bc:	52800981 	mov	w1, #0x4c                  	// #76
 402d2c0:	5296c220 	mov	w0, #0xb611                	// #46609
 402d2c4:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d2c8:	72a03740 	movk	w0, #0x1ba, lsl #16
 402d2cc:	97fff0dc 	bl	402963c <ddrt_reg_write>
 402d2d0:	52800a01 	mov	w1, #0x50                  	// #80
 402d2d4:	529756c0 	mov	w0, #0xbab6                	// #47798
 402d2d8:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d2dc:	72ba6020 	movk	w0, #0xd301, lsl #16
 402d2e0:	97fff0d7 	bl	402963c <ddrt_reg_write>
 402d2e4:	52800a81 	mov	w1, #0x54                  	// #84
 402d2e8:	52803740 	mov	w0, #0x1ba                 	// #442
 402d2ec:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d2f0:	72bc1a60 	movk	w0, #0xe0d3, lsl #16
 402d2f4:	97fff0d2 	bl	402963c <ddrt_reg_write>
 402d2f8:	52800b01 	mov	w1, #0x58                  	// #88
 402d2fc:	529a6020 	mov	w0, #0xd301                	// #54017
 402d300:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d304:	72b1bc00 	movk	w0, #0x8de0, lsl #16
 402d308:	97fff0cd 	bl	402963c <ddrt_reg_write>
 402d30c:	52800b81 	mov	w1, #0x5c                  	// #92
 402d310:	529c1a60 	mov	w0, #0xe0d3                	// #57555
 402d314:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d318:	72ac31a0 	movk	w0, #0x618d, lsl #16
 402d31c:	97fff0c8 	bl	402963c <ddrt_reg_write>
 402d320:	52800601 	mov	w1, #0x30                  	// #48
 402d324:	52800000 	mov	w0, #0x0                   	// #0
 402d328:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d32c:	97fff0c4 	bl	402963c <ddrt_reg_write>
 402d330:	2a1403e2 	mov	w2, w20
 402d334:	2a1303e1 	mov	w1, w19
 402d338:	52800000 	mov	w0, #0x0                   	// #0
 402d33c:	9400043d 	bl	402e430 <ddrtrn_ddrt_test>
 402d340:	35fff840 	cbnz	w0, 402d248 <ddrtrn_ddrt_check+0x108>
 402d344:	97ffecac 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d348:	b9401000 	ldr	w0, [x0, #16]
 402d34c:	52800601 	mov	w1, #0x30                  	// #48
 402d350:	72a222c1 	movk	w1, #0x1116, lsl #16
 402d354:	97fff0ba 	bl	402963c <ddrt_reg_write>
 402d358:	2a1403e2 	mov	w2, w20
 402d35c:	2a1303e1 	mov	w1, w19
 402d360:	52800000 	mov	w0, #0x0                   	// #0
 402d364:	94000433 	bl	402e430 <ddrtrn_ddrt_test>
 402d368:	7100001f 	cmp	w0, #0x0
 402d36c:	5a9f03e0 	csetm	w0, ne  // ne = any
 402d370:	17ffffb7 	b	402d24c <ddrtrn_ddrt_check+0x10c>

000000000402d374 <ddrtrn_hal_get_cur_phy>:
 402d374:	f81f0ffe 	str	x30, [sp, #-16]!
 402d378:	97ffec9f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d37c:	b9400400 	ldr	w0, [x0, #4]
 402d380:	f84107fe 	ldr	x30, [sp], #16
 402d384:	d65f03c0 	ret

000000000402d388 <ddrtrn_hal_get_byte_num>:
 402d388:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402d38c:	97ffec9a 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d390:	aa0003f3 	mov	x19, x0
 402d394:	97ffec93 	bl	40285e0 <ddrtrn_hal_get_phy>
 402d398:	d2800783 	mov	x3, #0x3c                  	// #60
 402d39c:	b9402262 	ldr	w2, [x19, #32]
 402d3a0:	b9402a61 	ldr	w1, [x19, #40]
 402d3a4:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402d3a8:	9b037c42 	mul	x2, x2, x3
 402d3ac:	d2800183 	mov	x3, #0xc                   	// #12
 402d3b0:	9b030821 	madd	x1, x1, x3, x2
 402d3b4:	8b010000 	add	x0, x0, x1
 402d3b8:	b9401800 	ldr	w0, [x0, #24]
 402d3bc:	d65f03c0 	ret

000000000402d3c0 <ddrtrn_hal_vref_phy_dram_set.isra.0>:
 402d3c0:	0b021c05 	add	w5, w0, w2, lsl #7
 402d3c4:	11031006 	add	w6, w0, #0xc4
 402d3c8:	1109c0a5 	add	w5, w5, #0x270
 402d3cc:	b94000c4 	ldr	w4, [x6]
 402d3d0:	b94000a3 	ldr	w3, [x5]
 402d3d4:	32010087 	orr	w7, w4, #0x80000000
 402d3d8:	b90000c7 	str	w7, [x6]
 402d3dc:	121a6463 	and	w3, w3, #0xffffffc0
 402d3e0:	2a010063 	orr	w3, w3, w1
 402d3e4:	b90000a3 	str	w3, [x5]
 402d3e8:	11001003 	add	w3, w0, #0x4
 402d3ec:	52800021 	mov	w1, #0x1                   	// #1
 402d3f0:	72a00081 	movk	w1, #0x4, lsl #16
 402d3f4:	b9000061 	str	w1, [x3]
 402d3f8:	52800001 	mov	w1, #0x0                   	// #0
 402d3fc:	b9400065 	ldr	w5, [x3]
 402d400:	11000421 	add	w1, w1, #0x1
 402d404:	36000105 	tbz	w5, #0, 402d424 <ddrtrn_hal_vref_phy_dram_set.isra.0+0x64>
 402d408:	3100043f 	cmn	w1, #0x1
 402d40c:	54ffff81 	b.ne	402d3fc <ddrtrn_hal_vref_phy_dram_set.isra.0+0x3c>  // b.any
 402d410:	11002001 	add	w1, w0, #0x8
 402d414:	b9400023 	ldr	w3, [x1]
 402d418:	2a0003e1 	mov	w1, w0
 402d41c:	52800200 	mov	w0, #0x10                  	// #16
 402d420:	17fff38a 	b	402a248 <ddrtrn_hal_training_stat>
 402d424:	12007884 	and	w4, w4, #0x7fffffff
 402d428:	b90000c4 	str	w4, [x6]
 402d42c:	d65f03c0 	ret

000000000402d430 <ddrtrn_hal_vref_save_bdl>:
 402d430:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402d434:	aa0003f4 	mov	x20, x0
 402d438:	a9015bf5 	stp	x21, x22, [sp, #16]
 402d43c:	52800015 	mov	w21, #0x0                   	// #0
 402d440:	f90013fe 	str	x30, [sp, #32]
 402d444:	97ffffcc 	bl	402d374 <ddrtrn_hal_get_cur_phy>
 402d448:	2a0003f6 	mov	w22, w0
 402d44c:	97ffec6a 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d450:	b9402400 	ldr	w0, [x0, #36]
 402d454:	0b002ad6 	add	w22, w22, w0, lsl #10
 402d458:	97ffffcc 	bl	402d388 <ddrtrn_hal_get_byte_num>
 402d45c:	6b15001f 	cmp	w0, w21
 402d460:	540000a8 	b.hi	402d474 <ddrtrn_hal_vref_save_bdl+0x44>  // b.pmore
 402d464:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402d468:	f94013fe 	ldr	x30, [sp, #32]
 402d46c:	a8c353f3 	ldp	x19, x20, [sp], #48
 402d470:	d65f03c0 	ret
 402d474:	97ffec60 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d478:	b9402801 	ldr	w1, [x0, #40]
 402d47c:	0b0106b3 	add	w19, w21, w1, lsl #1
 402d480:	97ffec5d 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d484:	b9401400 	ldr	w0, [x0, #20]
 402d488:	0b131ec1 	add	w1, w22, w19, lsl #7
 402d48c:	7100081f 	cmp	w0, #0x2
 402d490:	2a1503e0 	mov	w0, w21
 402d494:	540001a1 	b.ne	402d4c8 <ddrtrn_hal_vref_save_bdl+0x98>  // b.any
 402d498:	11084022 	add	w2, w1, #0x210
 402d49c:	b9400043 	ldr	w3, [x2]
 402d4a0:	8b000a82 	add	x2, x20, x0, lsl #2
 402d4a4:	b8207a83 	str	w3, [x20, x0, lsl #2]
 402d4a8:	11085020 	add	w0, w1, #0x214
 402d4ac:	11086021 	add	w1, w1, #0x218
 402d4b0:	b9400000 	ldr	w0, [x0]
 402d4b4:	b9001040 	str	w0, [x2, #16]
 402d4b8:	b9400020 	ldr	w0, [x1]
 402d4bc:	b9004040 	str	w0, [x2, #64]
 402d4c0:	110006b5 	add	w21, w21, #0x1
 402d4c4:	17ffffe5 	b	402d458 <ddrtrn_hal_vref_save_bdl+0x28>
 402d4c8:	11087022 	add	w2, w1, #0x21c
 402d4cc:	11088021 	add	w1, w1, #0x220
 402d4d0:	b9400043 	ldr	w3, [x2]
 402d4d4:	8b000a82 	add	x2, x20, x0, lsl #2
 402d4d8:	b8207a83 	str	w3, [x20, x0, lsl #2]
 402d4dc:	b9400020 	ldr	w0, [x1]
 402d4e0:	b9001040 	str	w0, [x2, #16]
 402d4e4:	17fffff7 	b	402d4c0 <ddrtrn_hal_vref_save_bdl+0x90>

000000000402d4e8 <ddrtrn_hal_vref_restore_bdl>:
 402d4e8:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402d4ec:	aa0003f4 	mov	x20, x0
 402d4f0:	a9015bf5 	stp	x21, x22, [sp, #16]
 402d4f4:	52800015 	mov	w21, #0x0                   	// #0
 402d4f8:	f90013fe 	str	x30, [sp, #32]
 402d4fc:	97ffff9e 	bl	402d374 <ddrtrn_hal_get_cur_phy>
 402d500:	2a0003f6 	mov	w22, w0
 402d504:	97ffec3c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d508:	b9402400 	ldr	w0, [x0, #36]
 402d50c:	0b002ad6 	add	w22, w22, w0, lsl #10
 402d510:	97ffff9e 	bl	402d388 <ddrtrn_hal_get_byte_num>
 402d514:	6b15001f 	cmp	w0, w21
 402d518:	540000a8 	b.hi	402d52c <ddrtrn_hal_vref_restore_bdl+0x44>  // b.pmore
 402d51c:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402d520:	f94013fe 	ldr	x30, [sp, #32]
 402d524:	a8c353f3 	ldp	x19, x20, [sp], #48
 402d528:	d65f03c0 	ret
 402d52c:	97ffec32 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d530:	b9402801 	ldr	w1, [x0, #40]
 402d534:	0b0106b3 	add	w19, w21, w1, lsl #1
 402d538:	97ffec2f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d53c:	b9401400 	ldr	w0, [x0, #20]
 402d540:	0b131ec1 	add	w1, w22, w19, lsl #7
 402d544:	7100081f 	cmp	w0, #0x2
 402d548:	2a1503e0 	mov	w0, w21
 402d54c:	8b000a82 	add	x2, x20, x0, lsl #2
 402d550:	b8607a83 	ldr	w3, [x20, x0, lsl #2]
 402d554:	54000161 	b.ne	402d580 <ddrtrn_hal_vref_restore_bdl+0x98>  // b.any
 402d558:	11084020 	add	w0, w1, #0x210
 402d55c:	b9000003 	str	w3, [x0]
 402d560:	11085020 	add	w0, w1, #0x214
 402d564:	11086021 	add	w1, w1, #0x218
 402d568:	b9401043 	ldr	w3, [x2, #16]
 402d56c:	b9000003 	str	w3, [x0]
 402d570:	b9404040 	ldr	w0, [x2, #64]
 402d574:	110006b5 	add	w21, w21, #0x1
 402d578:	b9000020 	str	w0, [x1]
 402d57c:	17ffffe5 	b	402d510 <ddrtrn_hal_vref_restore_bdl+0x28>
 402d580:	11087020 	add	w0, w1, #0x21c
 402d584:	11088021 	add	w1, w1, #0x220
 402d588:	b9000003 	str	w3, [x0]
 402d58c:	b9401040 	ldr	w0, [x2, #16]
 402d590:	17fffff9 	b	402d574 <ddrtrn_hal_vref_restore_bdl+0x8c>

000000000402d594 <ddrtrn_hal_vref_status_set>:
 402d594:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402d598:	2a0003f3 	mov	w19, w0
 402d59c:	a9017bf5 	stp	x21, x30, [sp, #16]
 402d5a0:	97ffec15 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d5a4:	b9401400 	ldr	w0, [x0, #20]
 402d5a8:	7100041f 	cmp	w0, #0x1
 402d5ac:	54000361 	b.ne	402d618 <ddrtrn_hal_vref_status_set+0x84>  // b.any
 402d5b0:	97ffff71 	bl	402d374 <ddrtrn_hal_get_cur_phy>
 402d5b4:	2a0003f4 	mov	w20, w0
 402d5b8:	97ffec0f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d5bc:	b9402415 	ldr	w21, [x0, #36]
 402d5c0:	97ffec0d 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d5c4:	b9401802 	ldr	w2, [x0, #24]
 402d5c8:	1109d280 	add	w0, w20, #0x274
 402d5cc:	53196043 	lsl	w3, w2, #7
 402d5d0:	11000442 	add	w2, w2, #0x1
 402d5d4:	35000175 	cbnz	w21, 402d600 <ddrtrn_hal_vref_status_set+0x6c>
 402d5d8:	0b000063 	add	w3, w3, w0
 402d5dc:	b9400061 	ldr	w1, [x3]
 402d5e0:	12196021 	and	w1, w1, #0xffffff80
 402d5e4:	2a130033 	orr	w19, w1, w19
 402d5e8:	0b021c02 	add	w2, w0, w2, lsl #7
 402d5ec:	b9000073 	str	w19, [x3]
 402d5f0:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402d5f4:	b9000053 	str	w19, [x2]
 402d5f8:	a8c253f3 	ldp	x19, x20, [sp], #32
 402d5fc:	d65f03c0 	ret
 402d600:	0b152800 	add	w0, w0, w21, lsl #10
 402d604:	0b000063 	add	w3, w3, w0
 402d608:	b9400061 	ldr	w1, [x3]
 402d60c:	12046c21 	and	w1, w1, #0xf0ffffff
 402d610:	2a136033 	orr	w19, w1, w19, lsl #24
 402d614:	17fffff5 	b	402d5e8 <ddrtrn_hal_vref_status_set+0x54>
 402d618:	97ffebf7 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d61c:	b9400800 	ldr	w0, [x0, #8]
 402d620:	11042000 	add	w0, w0, #0x108
 402d624:	b9400015 	ldr	w21, [x0]
 402d628:	97ffebf3 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d62c:	b9400800 	ldr	w0, [x0, #8]
 402d630:	12144ea1 	and	w1, w21, #0xfffff000
 402d634:	97ffee8f 	bl	4029070 <ddrtrn_hal_set_timing>
 402d638:	97ffff4f 	bl	402d374 <ddrtrn_hal_get_cur_phy>
 402d63c:	11030000 	add	w0, w0, #0xc0
 402d640:	b9400014 	ldr	w20, [x0]
 402d644:	97ffff4c 	bl	402d374 <ddrtrn_hal_get_cur_phy>
 402d648:	11030000 	add	w0, w0, #0xc0
 402d64c:	32010294 	orr	w20, w20, #0x80000000
 402d650:	b9000014 	str	w20, [x0]
 402d654:	97ffff48 	bl	402d374 <ddrtrn_hal_get_cur_phy>
 402d658:	2a0003f4 	mov	w20, w0
 402d65c:	97ffebe6 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d660:	b9401802 	ldr	w2, [x0, #24]
 402d664:	2a1303e1 	mov	w1, w19
 402d668:	2a1403e0 	mov	w0, w20
 402d66c:	97ffff55 	bl	402d3c0 <ddrtrn_hal_vref_phy_dram_set.isra.0>
 402d670:	97ffff41 	bl	402d374 <ddrtrn_hal_get_cur_phy>
 402d674:	2a0003f4 	mov	w20, w0
 402d678:	97ffebdf 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d67c:	b9401802 	ldr	w2, [x0, #24]
 402d680:	2a1303e1 	mov	w1, w19
 402d684:	2a1403e0 	mov	w0, w20
 402d688:	97ffff4e 	bl	402d3c0 <ddrtrn_hal_vref_phy_dram_set.isra.0>
 402d68c:	97ffff3a 	bl	402d374 <ddrtrn_hal_get_cur_phy>
 402d690:	11030000 	add	w0, w0, #0xc0
 402d694:	b9400014 	ldr	w20, [x0]
 402d698:	97ffff37 	bl	402d374 <ddrtrn_hal_get_cur_phy>
 402d69c:	11030000 	add	w0, w0, #0xc0
 402d6a0:	12007a94 	and	w20, w20, #0x7fffffff
 402d6a4:	b9000014 	str	w20, [x0]
 402d6a8:	97ffff33 	bl	402d374 <ddrtrn_hal_get_cur_phy>
 402d6ac:	2a0003f4 	mov	w20, w0
 402d6b0:	97ffebd1 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d6b4:	b9401802 	ldr	w2, [x0, #24]
 402d6b8:	2a1303e1 	mov	w1, w19
 402d6bc:	2a1403e0 	mov	w0, w20
 402d6c0:	97ffff40 	bl	402d3c0 <ddrtrn_hal_vref_phy_dram_set.isra.0>
 402d6c4:	97ffebcc 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d6c8:	b9400800 	ldr	w0, [x0, #8]
 402d6cc:	2a1503e1 	mov	w1, w21
 402d6d0:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402d6d4:	a8c253f3 	ldp	x19, x20, [sp], #32
 402d6d8:	17ffee66 	b	4029070 <ddrtrn_hal_set_timing>

000000000402d6dc <ddrtrn_hal_get_cur_phy>:
 402d6dc:	f81f0ffe 	str	x30, [sp, #-16]!
 402d6e0:	97ffebc5 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d6e4:	b9400400 	ldr	w0, [x0, #4]
 402d6e8:	f84107fe 	ldr	x30, [sp], #16
 402d6ec:	d65f03c0 	ret

000000000402d6f0 <ddrtrn_hal_get_cur_phy_total_byte_num>:
 402d6f0:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402d6f4:	97ffebc0 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d6f8:	aa0003f3 	mov	x19, x0
 402d6fc:	97ffebb9 	bl	40285e0 <ddrtrn_hal_get_phy>
 402d700:	d2800782 	mov	x2, #0x3c                  	// #60
 402d704:	b9402261 	ldr	w1, [x19, #32]
 402d708:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402d70c:	d503201f 	nop
 402d710:	9b020021 	madd	x1, x1, x2, x0
 402d714:	b9401020 	ldr	w0, [x1, #16]
 402d718:	d65f03c0 	ret

000000000402d71c <ddrtrn_hal_set_rank1_wdq_to_rank0>:
 402d71c:	1118c002 	add	w2, w0, #0x630
 402d720:	52800003 	mov	w3, #0x0                   	// #0
 402d724:	6b01007f 	cmp	w3, w1
 402d728:	54000041 	b.ne	402d730 <ddrtrn_hal_set_rank1_wdq_to_rank0+0x14>  // b.any
 402d72c:	17fff04a 	b	4029854 <ddrtrn_hal_phy_cfg_update>
 402d730:	2a0203e4 	mov	w4, w2
 402d734:	11000463 	add	w3, w3, #0x1
 402d738:	b9400085 	ldr	w5, [x4]
 402d73c:	51100044 	sub	w4, w2, #0x400
 402d740:	b9000085 	str	w5, [x4]
 402d744:	11001044 	add	w4, w2, #0x4
 402d748:	b9400085 	ldr	w5, [x4]
 402d74c:	510ff044 	sub	w4, w2, #0x3fc
 402d750:	11020042 	add	w2, w2, #0x80
 402d754:	b9000085 	str	w5, [x4]
 402d758:	17fffff3 	b	402d724 <ddrtrn_hal_set_rank1_wdq_to_rank0+0x8>

000000000402d75c <ddrtrn_hal_hw_read_adj>:
 402d75c:	d65f03c0 	ret

000000000402d760 <ddrtrn_hal_hw_rdqs_offset_cfg>:
 402d760:	1108b000 	add	w0, w0, #0x22c
 402d764:	52800004 	mov	w4, #0x0                   	// #0
 402d768:	6b01009f 	cmp	w4, w1
 402d76c:	54000041 	b.ne	402d774 <ddrtrn_hal_hw_rdqs_offset_cfg+0x14>  // b.any
 402d770:	d65f03c0 	ret
 402d774:	2a0003e5 	mov	w5, w0
 402d778:	b94000a3 	ldr	w3, [x5]
 402d77c:	7100185f 	cmp	w2, #0x6
 402d780:	540000a1 	b.ne	402d794 <ddrtrn_hal_hw_rdqs_offset_cfg+0x34>  // b.any
 402d784:	11000484 	add	w4, w4, #0x1
 402d788:	11020000 	add	w0, w0, #0x80
 402d78c:	b90000a3 	str	w3, [x5]
 402d790:	17fffff6 	b	402d768 <ddrtrn_hal_hw_rdqs_offset_cfg+0x8>
 402d794:	11002863 	add	w3, w3, #0xa
 402d798:	17fffffb 	b	402d784 <ddrtrn_hal_hw_rdqs_offset_cfg+0x24>

000000000402d79c <ddrtrn_hal_training_get_rdqs>:
 402d79c:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402d7a0:	aa0003f4 	mov	x20, x0
 402d7a4:	f9000bfe 	str	x30, [sp, #16]
 402d7a8:	97ffffd2 	bl	402d6f0 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402d7ac:	2a0003f3 	mov	w19, w0
 402d7b0:	97ffffcb 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402d7b4:	1108b001 	add	w1, w0, #0x22c
 402d7b8:	d2800002 	mov	x2, #0x0                   	// #0
 402d7bc:	6b02027f 	cmp	w19, w2
 402d7c0:	54000088 	b.hi	402d7d0 <ddrtrn_hal_training_get_rdqs+0x34>  // b.pmore
 402d7c4:	f9400bfe 	ldr	x30, [sp, #16]
 402d7c8:	a8c253f3 	ldp	x19, x20, [sp], #32
 402d7cc:	d65f03c0 	ret
 402d7d0:	2a0103e0 	mov	w0, w1
 402d7d4:	11020021 	add	w1, w1, #0x80
 402d7d8:	b9400000 	ldr	w0, [x0]
 402d7dc:	b8227a80 	str	w0, [x20, x2, lsl #2]
 402d7e0:	91000442 	add	x2, x2, #0x1
 402d7e4:	17fffff6 	b	402d7bc <ddrtrn_hal_training_get_rdqs+0x20>

000000000402d7e8 <ddrtrn_hal_training_set_rdqs>:
 402d7e8:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402d7ec:	aa0003f4 	mov	x20, x0
 402d7f0:	f9000bfe 	str	x30, [sp, #16]
 402d7f4:	97ffffbf 	bl	402d6f0 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402d7f8:	2a0003f3 	mov	w19, w0
 402d7fc:	97ffffb8 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402d800:	1108b001 	add	w1, w0, #0x22c
 402d804:	d2800002 	mov	x2, #0x0                   	// #0
 402d808:	6b02027f 	cmp	w19, w2
 402d80c:	54000088 	b.hi	402d81c <ddrtrn_hal_training_set_rdqs+0x34>  // b.pmore
 402d810:	f9400bfe 	ldr	x30, [sp, #16]
 402d814:	a8c253f3 	ldp	x19, x20, [sp], #32
 402d818:	d65f03c0 	ret
 402d81c:	2a0103e0 	mov	w0, w1
 402d820:	b8627a83 	ldr	w3, [x20, x2, lsl #2]
 402d824:	11020021 	add	w1, w1, #0x80
 402d828:	91000442 	add	x2, x2, #0x1
 402d82c:	b9000003 	str	w3, [x0]
 402d830:	17fffff6 	b	402d808 <ddrtrn_hal_training_set_rdqs+0x20>

000000000402d834 <ddrtrn_hal_hw_training_process>:
 402d834:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402d838:	a9015bf5 	stp	x21, x22, [sp, #16]
 402d83c:	2a0003f5 	mov	w21, w0
 402d840:	f90013fe 	str	x30, [sp, #32]
 402d844:	97ffffa6 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402d848:	11001016 	add	w22, w0, #0x4
 402d84c:	2a0003f4 	mov	w20, w0
 402d850:	b94002c0 	ldr	w0, [x22]
 402d854:	350000d5 	cbnz	w21, 402d86c <ddrtrn_hal_hw_training_process+0x38>
 402d858:	52800000 	mov	w0, #0x0                   	// #0
 402d85c:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402d860:	f94013fe 	ldr	x30, [sp, #32]
 402d864:	a8c353f3 	ldp	x19, x20, [sp], #48
 402d868:	d65f03c0 	ret
 402d86c:	2a0002a0 	orr	w0, w21, w0
 402d870:	32000000 	orr	w0, w0, #0x1
 402d874:	b90002c0 	str	w0, [x22]
 402d878:	52828000 	mov	w0, #0x1400                	// #5120
 402d87c:	6a35001f 	bics	wzr, w0, w21
 402d880:	540001c0 	b.eq	402d8b8 <ddrtrn_hal_hw_training_process+0x84>  // b.none
 402d884:	52800013 	mov	w19, #0x0                   	// #0
 402d888:	b94002c0 	ldr	w0, [x22]
 402d88c:	11000673 	add	w19, w19, #0x1
 402d890:	360001e0 	tbz	w0, #0, 402d8cc <ddrtrn_hal_hw_training_process+0x98>
 402d894:	3100067f 	cmn	w19, #0x1
 402d898:	54ffff81 	b.ne	402d888 <ddrtrn_hal_hw_training_process+0x54>  // b.any
 402d89c:	11002280 	add	w0, w20, #0x8
 402d8a0:	b9400003 	ldr	w3, [x0]
 402d8a4:	2a1503e2 	mov	w2, w21
 402d8a8:	2a1403e1 	mov	w1, w20
 402d8ac:	52800200 	mov	w0, #0x10                  	// #16
 402d8b0:	97fff266 	bl	402a248 <ddrtrn_hal_training_stat>
 402d8b4:	14000004 	b	402d8c4 <ddrtrn_hal_hw_training_process+0x90>
 402d8b8:	52800040 	mov	w0, #0x2                   	// #2
 402d8bc:	97fff48c 	bl	402aaec <ddrtrn_training_ctrl_easr>
 402d8c0:	34fffe20 	cbz	w0, 402d884 <ddrtrn_hal_hw_training_process+0x50>
 402d8c4:	12800000 	mov	w0, #0xffffffff            	// #-1
 402d8c8:	17ffffe5 	b	402d85c <ddrtrn_hal_hw_training_process+0x28>
 402d8cc:	11002280 	add	w0, w20, #0x8
 402d8d0:	b9400001 	ldr	w1, [x0]
 402d8d4:	721c783f 	tst	w1, #0xfffffff7
 402d8d8:	54fffc00 	b.eq	402d858 <ddrtrn_hal_hw_training_process+0x24>  // b.none
 402d8dc:	17fffff1 	b	402d8a0 <ddrtrn_hal_hw_training_process+0x6c>

000000000402d8e0 <ddrtrn_hal_hw_training_normal_conf>:
 402d8e0:	d10383ff 	sub	sp, sp, #0xe0
 402d8e4:	90ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402d8e8:	a90a53f3 	stp	x19, x20, [sp, #160]
 402d8ec:	a90b5bf5 	stp	x21, x22, [sp, #176]
 402d8f0:	a90c63f7 	stp	x23, x24, [sp, #192]
 402d8f4:	f9006bfe 	str	x30, [sp, #208]
 402d8f8:	f9455001 	ldr	x1, [x0, #2720]
 402d8fc:	f9004fe1 	str	x1, [sp, #152]
 402d900:	d2800001 	mov	x1, #0x0                   	// #0
 402d904:	97ffeb3c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d908:	b9400c16 	ldr	w22, [x0, #12]
 402d90c:	52801202 	mov	w2, #0x90                  	// #144
 402d910:	52800001 	mov	w1, #0x0                   	// #0
 402d914:	910023e0 	add	x0, sp, #0x8
 402d918:	97fff3fb 	bl	402a904 <ddrtrn_set_data>
 402d91c:	97ffff70 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402d920:	2a0003f5 	mov	w21, w0
 402d924:	97ffff73 	bl	402d6f0 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402d928:	2a0003f4 	mov	w20, w0
 402d92c:	97ffeb32 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402d930:	b9402417 	ldr	w23, [x0, #36]
 402d934:	34000597 	cbz	w23, 402d9e4 <ddrtrn_hal_hw_training_normal_conf+0x104>
 402d938:	910023f7 	add	x23, sp, #0x8
 402d93c:	52800013 	mov	w19, #0x0                   	// #0
 402d940:	aa1703f8 	mov	x24, x23
 402d944:	6b14027f 	cmp	w19, w20
 402d948:	54000801 	b.ne	402da48 <ddrtrn_hal_hw_training_normal_conf+0x168>  // b.any
 402d94c:	121c02c0 	and	w0, w22, #0x10
 402d950:	97ffffb9 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402d954:	2a0003f3 	mov	w19, w0
 402d958:	52800018 	mov	w24, #0x0                   	// #0
 402d95c:	6b14031f 	cmp	w24, w20
 402d960:	54000841 	b.ne	402da68 <ddrtrn_hal_hw_training_normal_conf+0x188>  // b.any
 402d964:	2a1403e1 	mov	w1, w20
 402d968:	2a1503e0 	mov	w0, w21
 402d96c:	97ffff6c 	bl	402d71c <ddrtrn_hal_set_rank1_wdq_to_rank0>
 402d970:	121b12c0 	and	w0, w22, #0x3e0
 402d974:	97ffffb0 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402d978:	0b000273 	add	w19, w19, w0
 402d97c:	2a1403e1 	mov	w1, w20
 402d980:	2a1503e0 	mov	w0, w21
 402d984:	97ffff66 	bl	402d71c <ddrtrn_hal_set_rank1_wdq_to_rank0>
 402d988:	120f02c0 	and	w0, w22, #0x20000
 402d98c:	97ffffaa 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402d990:	0b000273 	add	w19, w19, w0
 402d994:	121002c0 	and	w0, w22, #0x10000
 402d998:	97ffffa7 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402d99c:	2a1403e1 	mov	w1, w20
 402d9a0:	0b000273 	add	w19, w19, w0
 402d9a4:	2a1503e0 	mov	w0, w21
 402d9a8:	97ffff5d 	bl	402d71c <ddrtrn_hal_set_rank1_wdq_to_rank0>
 402d9ac:	120402c0 	and	w0, w22, #0x10000000
 402d9b0:	52800016 	mov	w22, #0x0                   	// #0
 402d9b4:	97ffffa0 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402d9b8:	0b000273 	add	w19, w19, w0
 402d9bc:	6b1402df 	cmp	w22, w20
 402d9c0:	54000260 	b.eq	402da0c <ddrtrn_hal_hw_training_normal_conf+0x12c>  // b.none
 402d9c4:	2a1603e3 	mov	w3, w22
 402d9c8:	aa1703e0 	mov	x0, x23
 402d9cc:	2a1503e1 	mov	w1, w21
 402d9d0:	52800002 	mov	w2, #0x0                   	// #0
 402d9d4:	110006d6 	add	w22, w22, #0x1
 402d9d8:	910092f7 	add	x23, x23, #0x24
 402d9dc:	97fff052 	bl	4029b24 <ddrtrn_hal_restore_dly_value>
 402d9e0:	17fffff7 	b	402d9bc <ddrtrn_hal_hw_training_normal_conf+0xdc>
 402d9e4:	121c02c0 	and	w0, w22, #0x10
 402d9e8:	97ffff93 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402d9ec:	2a0003f3 	mov	w19, w0
 402d9f0:	6b1402ff 	cmp	w23, w20
 402d9f4:	540001a1 	b.ne	402da28 <ddrtrn_hal_hw_training_normal_conf+0x148>  // b.any
 402d9f8:	52807c00 	mov	w0, #0x3e0                 	// #992
 402d9fc:	72a20060 	movk	w0, #0x1003, lsl #16
 402da00:	0a0002c0 	and	w0, w22, w0
 402da04:	97ffff8c 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402da08:	0b000273 	add	w19, w19, w0
 402da0c:	90ffffa0 	adrp	x0, 4021000 <hash_verify_buf-0xa80>
 402da10:	f9404fe1 	ldr	x1, [sp, #152]
 402da14:	f9455002 	ldr	x2, [x0, #2720]
 402da18:	eb020021 	subs	x1, x1, x2
 402da1c:	d2800002 	mov	x2, #0x0                   	// #0
 402da20:	54000340 	b.eq	402da88 <ddrtrn_hal_hw_training_normal_conf+0x1a8>  // b.none
 402da24:	97ffd9d2 	bl	402416c <__stack_chk_fail>
 402da28:	97ffeaf3 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402da2c:	b9001817 	str	w23, [x0, #24]
 402da30:	2a1703e2 	mov	w2, w23
 402da34:	2a1503e0 	mov	w0, w21
 402da38:	52800001 	mov	w1, #0x0                   	// #0
 402da3c:	110006f7 	add	w23, w23, #0x1
 402da40:	97fff047 	bl	4029b5c <ddrtrn_hal_wdqs_bdl2phase>
 402da44:	17ffffeb 	b	402d9f0 <ddrtrn_hal_hw_training_normal_conf+0x110>
 402da48:	2a1303e3 	mov	w3, w19
 402da4c:	aa1803e0 	mov	x0, x24
 402da50:	2a1503e1 	mov	w1, w21
 402da54:	52800002 	mov	w2, #0x0                   	// #0
 402da58:	11000673 	add	w19, w19, #0x1
 402da5c:	91009318 	add	x24, x24, #0x24
 402da60:	97fff01d 	bl	4029ad4 <ddrtrn_hal_get_dly_value>
 402da64:	17ffffb8 	b	402d944 <ddrtrn_hal_hw_training_normal_conf+0x64>
 402da68:	97ffeae3 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402da6c:	b9001818 	str	w24, [x0, #24]
 402da70:	2a1803e2 	mov	w2, w24
 402da74:	2a1503e0 	mov	w0, w21
 402da78:	52800021 	mov	w1, #0x1                   	// #1
 402da7c:	11000718 	add	w24, w24, #0x1
 402da80:	97fff037 	bl	4029b5c <ddrtrn_hal_wdqs_bdl2phase>
 402da84:	17ffffb6 	b	402d95c <ddrtrn_hal_hw_training_normal_conf+0x7c>
 402da88:	2a1303e0 	mov	w0, w19
 402da8c:	a94a53f3 	ldp	x19, x20, [sp, #160]
 402da90:	a94b5bf5 	ldp	x21, x22, [sp, #176]
 402da94:	a94c63f7 	ldp	x23, x24, [sp, #192]
 402da98:	f9406bfe 	ldr	x30, [sp, #208]
 402da9c:	910383ff 	add	sp, sp, #0xe0
 402daa0:	d65f03c0 	ret

000000000402daa4 <ddrtrn_hal_hw_ca_vref_sync>:
 402daa4:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402daa8:	a9015bf5 	stp	x21, x22, [sp, #16]
 402daac:	a9027bf7 	stp	x23, x30, [sp, #32]
 402dab0:	97ffff0b 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402dab4:	2a0003f3 	mov	w19, w0
 402dab8:	11012273 	add	w19, w19, #0x48
 402dabc:	97ffeace 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dac0:	b9400c14 	ldr	w20, [x0, #12]
 402dac4:	b9400277 	ldr	w23, [x19]
 402dac8:	120b0294 	and	w20, w20, #0x200000
 402dacc:	2a1403e0 	mov	w0, w20
 402dad0:	121c6ef5 	and	w21, w23, #0xfffffff0
 402dad4:	b9000275 	str	w21, [x19]
 402dad8:	320002b5 	orr	w21, w21, #0x1
 402dadc:	97ffff56 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402dae0:	b9000275 	str	w21, [x19]
 402dae4:	2a0003f6 	mov	w22, w0
 402dae8:	2a1403e0 	mov	w0, w20
 402daec:	97ffff52 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402daf0:	0b0002c0 	add	w0, w22, w0
 402daf4:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402daf8:	b9000277 	str	w23, [x19]
 402dafc:	a9427bf7 	ldp	x23, x30, [sp, #32]
 402db00:	a8c353f3 	ldp	x19, x20, [sp], #48
 402db04:	d65f03c0 	ret

000000000402db08 <ddrtrn_hal_hw_dram_mr_init>:
 402db08:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402db0c:	a9015bf5 	stp	x21, x22, [sp, #16]
 402db10:	a90263f7 	stp	x23, x24, [sp, #32]
 402db14:	f9001bfe 	str	x30, [sp, #48]
 402db18:	97fffef1 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402db1c:	2a0003f3 	mov	w19, w0
 402db20:	97ffeab5 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402db24:	b9400c16 	ldr	w22, [x0, #12]
 402db28:	52821900 	mov	w0, #0x10c8                	// #4296
 402db2c:	0b000260 	add	w0, w19, w0
 402db30:	121602d6 	and	w22, w22, #0x400
 402db34:	b9400000 	ldr	w0, [x0]
 402db38:	f240041f 	tst	x0, #0x3
 402db3c:	540000e0 	b.eq	402db58 <ddrtrn_hal_hw_dram_mr_init+0x50>  // b.none
 402db40:	2a1603e0 	mov	w0, w22
 402db44:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402db48:	a94263f7 	ldp	x23, x24, [sp, #32]
 402db4c:	f9401bfe 	ldr	x30, [sp, #48]
 402db50:	a8c453f3 	ldp	x19, x20, [sp], #64
 402db54:	17ffff38 	b	402d834 <ddrtrn_hal_hw_training_process>
 402db58:	11013274 	add	w20, w19, #0x4c
 402db5c:	1101a273 	add	w19, w19, #0x68
 402db60:	b9400298 	ldr	w24, [x20]
 402db64:	2a1803e0 	mov	w0, w24
 402db68:	72800020 	movk	w0, #0x1
 402db6c:	b9000280 	str	w0, [x20]
 402db70:	b9400277 	ldr	w23, [x19]
 402db74:	121a72e0 	and	w0, w23, #0xffffffc7
 402db78:	b9000260 	str	w0, [x19]
 402db7c:	2a1603e0 	mov	w0, w22
 402db80:	97ffff2d 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402db84:	b9000298 	str	w24, [x20]
 402db88:	b9000277 	str	w23, [x19]
 402db8c:	2a0003f5 	mov	w21, w0
 402db90:	52800501 	mov	w1, #0x28                  	// #40
 402db94:	b9400298 	ldr	w24, [x20]
 402db98:	2a1803e0 	mov	w0, w24
 402db9c:	72800040 	movk	w0, #0x2
 402dba0:	b9000280 	str	w0, [x20]
 402dba4:	b9400277 	ldr	w23, [x19]
 402dba8:	121a72e0 	and	w0, w23, #0xffffffc7
 402dbac:	2a010000 	orr	w0, w0, w1
 402dbb0:	b9000260 	str	w0, [x19]
 402dbb4:	2a1603e0 	mov	w0, w22
 402dbb8:	97ffff1f 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402dbbc:	0b0002a0 	add	w0, w21, w0
 402dbc0:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402dbc4:	f9401bfe 	ldr	x30, [sp, #48]
 402dbc8:	b9000298 	str	w24, [x20]
 402dbcc:	b9000277 	str	w23, [x19]
 402dbd0:	a94263f7 	ldp	x23, x24, [sp, #32]
 402dbd4:	a8c453f3 	ldp	x19, x20, [sp], #64
 402dbd8:	d65f03c0 	ret

000000000402dbdc <ddrtrn_hal_hw_dataeye_adapt>:
 402dbdc:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402dbe0:	aa0003f3 	mov	x19, x0
 402dbe4:	a9015bf5 	stp	x21, x22, [sp, #16]
 402dbe8:	a90263f7 	stp	x23, x24, [sp, #32]
 402dbec:	f9001bfe 	str	x30, [sp, #48]
 402dbf0:	97fffebb 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402dbf4:	2a0003f5 	mov	w21, w0
 402dbf8:	97ffea7f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dbfc:	aa0003f4 	mov	x20, x0
 402dc00:	97ffea78 	bl	40285e0 <ddrtrn_hal_get_phy>
 402dc04:	d2800782 	mov	x2, #0x3c                  	// #60
 402dc08:	b9402281 	ldr	w1, [x20, #32]
 402dc0c:	9b020021 	madd	x1, x1, x2, x0
 402dc10:	b9400420 	ldr	w0, [x1, #4]
 402dc14:	7100181f 	cmp	w0, #0x6
 402dc18:	54000160 	b.eq	402dc44 <ddrtrn_hal_hw_dataeye_adapt+0x68>  // b.none
 402dc1c:	b900027f 	str	wzr, [x19]
 402dc20:	97ffea75 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dc24:	b9400c01 	ldr	w1, [x0, #12]
 402dc28:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402dc2c:	52828000 	mov	w0, #0x1400                	// #5120
 402dc30:	a94263f7 	ldp	x23, x24, [sp, #32]
 402dc34:	0a000020 	and	w0, w1, w0
 402dc38:	f9401bfe 	ldr	x30, [sp, #48]
 402dc3c:	a8c453f3 	ldp	x19, x20, [sp], #64
 402dc40:	17fffefd 	b	402d834 <ddrtrn_hal_hw_training_process>
 402dc44:	1100d2b6 	add	w22, w21, #0x34
 402dc48:	b94002d7 	ldr	w23, [x22]
 402dc4c:	12176ae0 	and	w0, w23, #0xfffffe0f
 402dc50:	b90002c0 	str	w0, [x22]
 402dc54:	97fffea2 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402dc58:	2a0003f4 	mov	w20, w0
 402dc5c:	11019294 	add	w20, w20, #0x64
 402dc60:	97ffea65 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dc64:	b9400c01 	ldr	w1, [x0, #12]
 402dc68:	b9400298 	ldr	w24, [x20]
 402dc6c:	12017300 	and	w0, w24, #0x8fffffff
 402dc70:	b9000280 	str	w0, [x20]
 402dc74:	52828000 	mov	w0, #0x1400                	// #5120
 402dc78:	0a000020 	and	w0, w1, w0
 402dc7c:	97fffeee 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402dc80:	b9000298 	str	w24, [x20]
 402dc84:	110392b4 	add	w20, w21, #0xe4
 402dc88:	2a0003f3 	mov	w19, w0
 402dc8c:	97ffff86 	bl	402daa4 <ddrtrn_hal_hw_ca_vref_sync>
 402dc90:	0b130013 	add	w19, w0, w19
 402dc94:	97ffff9d 	bl	402db08 <ddrtrn_hal_hw_dram_mr_init>
 402dc98:	0b000273 	add	w19, w19, w0
 402dc9c:	b9400295 	ldr	w21, [x20]
 402dca0:	321a02a0 	orr	w0, w21, #0x40
 402dca4:	b9000280 	str	w0, [x20]
 402dca8:	12197ab5 	and	w21, w21, #0xffffffbf
 402dcac:	97ffff97 	bl	402db08 <ddrtrn_hal_hw_dram_mr_init>
 402dcb0:	0b000273 	add	w19, w19, w0
 402dcb4:	97ffff7c 	bl	402daa4 <ddrtrn_hal_hw_ca_vref_sync>
 402dcb8:	b9000295 	str	w21, [x20]
 402dcbc:	0b000273 	add	w19, w19, w0
 402dcc0:	97ffff92 	bl	402db08 <ddrtrn_hal_hw_dram_mr_init>
 402dcc4:	0b000260 	add	w0, w19, w0
 402dcc8:	f9401bfe 	ldr	x30, [sp, #48]
 402dccc:	b90002d7 	str	w23, [x22]
 402dcd0:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402dcd4:	a94263f7 	ldp	x23, x24, [sp, #32]
 402dcd8:	a8c453f3 	ldp	x19, x20, [sp], #64
 402dcdc:	d65f03c0 	ret

000000000402dce0 <ddrtrn_hal_hw_dataeye_vref_set>:
 402dce0:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402dce4:	a9015bf5 	stp	x21, x22, [sp, #16]
 402dce8:	a9027bf7 	stp	x23, x30, [sp, #32]
 402dcec:	97fffe7c 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402dcf0:	2a0003f3 	mov	w19, w0
 402dcf4:	11031276 	add	w22, w19, #0xc4
 402dcf8:	97ffea3f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dcfc:	11030273 	add	w19, w19, #0xc0
 402dd00:	b9400c15 	ldr	w21, [x0, #12]
 402dd04:	b94002d7 	ldr	w23, [x22]
 402dd08:	120e02b5 	and	w21, w21, #0x40000
 402dd0c:	12007ae0 	and	w0, w23, #0x7fffffff
 402dd10:	b90002c0 	str	w0, [x22]
 402dd14:	b9400260 	ldr	w0, [x19]
 402dd18:	32010000 	orr	w0, w0, #0x80000000
 402dd1c:	b9000260 	str	w0, [x19]
 402dd20:	2a1503e0 	mov	w0, w21
 402dd24:	97fffec4 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402dd28:	2a0003f4 	mov	w20, w0
 402dd2c:	2a1503e0 	mov	w0, w21
 402dd30:	97fffec1 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402dd34:	0b000294 	add	w20, w20, w0
 402dd38:	b9400260 	ldr	w0, [x19]
 402dd3c:	12007800 	and	w0, w0, #0x7fffffff
 402dd40:	b9000260 	str	w0, [x19]
 402dd44:	2a1503e0 	mov	w0, w21
 402dd48:	97fffebb 	bl	402d834 <ddrtrn_hal_hw_training_process>
 402dd4c:	b90002d7 	str	w23, [x22]
 402dd50:	0b000280 	add	w0, w20, w0
 402dd54:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402dd58:	a9427bf7 	ldp	x23, x30, [sp, #32]
 402dd5c:	a8c353f3 	ldp	x19, x20, [sp], #48
 402dd60:	d65f03c0 	ret

000000000402dd64 <ddrtrn_hal_hw_clear_rdq>:
 402dd64:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402dd68:	53196013 	lsl	w19, w0, #7
 402dd6c:	f9000bfe 	str	x30, [sp, #16]
 402dd70:	97fffe5b 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402dd74:	2a0003f4 	mov	w20, w0
 402dd78:	97ffea1f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dd7c:	b9402400 	ldr	w0, [x0, #36]
 402dd80:	11087261 	add	w1, w19, #0x21c
 402dd84:	0b002a80 	add	w0, w20, w0, lsl #10
 402dd88:	0b010000 	add	w0, w0, w1
 402dd8c:	b900001f 	str	wzr, [x0]
 402dd90:	97ffea19 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dd94:	b9402400 	ldr	w0, [x0, #36]
 402dd98:	11088261 	add	w1, w19, #0x220
 402dd9c:	1108b273 	add	w19, w19, #0x22c
 402dda0:	0b140273 	add	w19, w19, w20
 402dda4:	0b002a80 	add	w0, w20, w0, lsl #10
 402dda8:	0b010000 	add	w0, w0, w1
 402ddac:	f9400bfe 	ldr	x30, [sp, #16]
 402ddb0:	b900001f 	str	wzr, [x0]
 402ddb4:	b900027f 	str	wzr, [x19]
 402ddb8:	a8c253f3 	ldp	x19, x20, [sp], #32
 402ddbc:	d65f03c0 	ret

000000000402ddc0 <ddrtrn_hal_hw_restore_rdqsbdl>:
 402ddc0:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402ddc4:	2a0203f3 	mov	w19, w2
 402ddc8:	12002014 	and	w20, w0, #0x1ff
 402ddcc:	a9015bf5 	stp	x21, x22, [sp, #16]
 402ddd0:	2a0103f6 	mov	w22, w1
 402ddd4:	12002035 	and	w21, w1, #0x1ff
 402ddd8:	a9027bf7 	stp	x23, x30, [sp, #32]
 402dddc:	2a0003f7 	mov	w23, w0
 402dde0:	97ffea05 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dde4:	b9001813 	str	w19, [x0, #24]
 402dde8:	53196273 	lsl	w19, w19, #7
 402ddec:	1108b273 	add	w19, w19, #0x22c
 402ddf0:	6b15029f 	cmp	w20, w21
 402ddf4:	540001a9 	b.ls	402de28 <ddrtrn_hal_hw_restore_rdqsbdl+0x68>  // b.plast
 402ddf8:	97fffe39 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402ddfc:	0b130013 	add	w19, w0, w19
 402de00:	4b150294 	sub	w20, w20, w21
 402de04:	b9000277 	str	w23, [x19]
 402de08:	97ffe9fb 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402de0c:	52800021 	mov	w1, #0x1                   	// #1
 402de10:	b9002401 	str	w1, [x0, #36]
 402de14:	2a1403e0 	mov	w0, w20
 402de18:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402de1c:	a9427bf7 	ldp	x23, x30, [sp, #32]
 402de20:	a8c353f3 	ldp	x19, x20, [sp], #48
 402de24:	d65f03c0 	ret
 402de28:	97fffe2d 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402de2c:	0b130013 	add	w19, w0, w19
 402de30:	4b1402b4 	sub	w20, w21, w20
 402de34:	b9000276 	str	w22, [x19]
 402de38:	97ffe9ef 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402de3c:	b900241f 	str	wzr, [x0, #36]
 402de40:	17fffff5 	b	402de14 <ddrtrn_hal_hw_restore_rdqsbdl+0x54>

000000000402de44 <ddrtrn_hal_hw_save_rdqsbdl>:
 402de44:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402de48:	f9000bfe 	str	x30, [sp, #16]
 402de4c:	97fffe24 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402de50:	1108b000 	add	w0, w0, #0x22c
 402de54:	b9400014 	ldr	w20, [x0]
 402de58:	97fffe21 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402de5c:	1107a000 	add	w0, w0, #0x1e8
 402de60:	53127e94 	lsr	w20, w20, #18
 402de64:	b9400013 	ldr	w19, [x0]
 402de68:	97fffe1d 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402de6c:	1107a000 	add	w0, w0, #0x1e8
 402de70:	f9400bfe 	ldr	x30, [sp, #16]
 402de74:	33161a93 	bfi	w19, w20, #10, #7
 402de78:	b9000013 	str	w19, [x0]
 402de7c:	a8c253f3 	ldp	x19, x20, [sp], #32
 402de80:	d65f03c0 	ret

000000000402de84 <ddrtrn_ac_oe_enable>:
 402de84:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402de88:	f9000bfe 	str	x30, [sp, #16]
 402de8c:	97fffe14 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402de90:	1101e001 	add	w1, w0, #0x78
 402de94:	2a0003f3 	mov	w19, w0
 402de98:	52820280 	mov	w0, #0x1014                	// #4116
 402de9c:	0b000273 	add	w19, w19, w0
 402dea0:	b9400020 	ldr	w0, [x1]
 402dea4:	32190000 	orr	w0, w0, #0x80
 402dea8:	b9000020 	str	w0, [x1]
 402deac:	97ffe9d2 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402deb0:	aa0003f4 	mov	x20, x0
 402deb4:	97ffe9cb 	bl	40285e0 <ddrtrn_hal_get_phy>
 402deb8:	d2800782 	mov	x2, #0x3c                  	// #60
 402debc:	b9402281 	ldr	w1, [x20, #32]
 402dec0:	9b020021 	madd	x1, x1, x2, x0
 402dec4:	b9400420 	ldr	w0, [x1, #4]
 402dec8:	7100181f 	cmp	w0, #0x6
 402decc:	b9400260 	ldr	w0, [x19]
 402ded0:	540000c1 	b.ne	402dee8 <ddrtrn_ac_oe_enable+0x64>  // b.any
 402ded4:	32002c00 	orr	w0, w0, #0xfff
 402ded8:	f9400bfe 	ldr	x30, [sp, #16]
 402dedc:	b9000260 	str	w0, [x19]
 402dee0:	a8c253f3 	ldp	x19, x20, [sp], #32
 402dee4:	d65f03c0 	ret
 402dee8:	12144c00 	and	w0, w0, #0xfffff000
 402deec:	5281fee1 	mov	w1, #0xff7                 	// #4087
 402def0:	2a010000 	orr	w0, w0, w1
 402def4:	17fffff9 	b	402ded8 <ddrtrn_ac_oe_enable+0x54>

000000000402def8 <ddrtrn_dummy_io_oe_enable>:
 402def8:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402defc:	97fffdfd 	bl	402d6f0 <ddrtrn_hal_get_cur_phy_total_byte_num>
 402df00:	2a0003f3 	mov	w19, w0
 402df04:	97fffdf6 	bl	402d6dc <ddrtrn_hal_get_cur_phy>
 402df08:	52825401 	mov	w1, #0x12a0                	// #4768
 402df0c:	52825382 	mov	w2, #0x129c                	// #4764
 402df10:	0b010004 	add	w4, w0, w1
 402df14:	0b020002 	add	w2, w0, w2
 402df18:	52800003 	mov	w3, #0x0                   	// #0
 402df1c:	6b13007f 	cmp	w3, w19
 402df20:	54000061 	b.ne	402df2c <ddrtrn_dummy_io_oe_enable+0x34>  // b.any
 402df24:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402df28:	d65f03c0 	ret
 402df2c:	53017c61 	lsr	w1, w3, #1
 402df30:	53185c21 	lsl	w1, w1, #8
 402df34:	370000e3 	tbnz	w3, #0, 402df50 <ddrtrn_dummy_io_oe_enable+0x58>
 402df38:	0b010041 	add	w1, w2, w1
 402df3c:	b9400020 	ldr	w0, [x1]
 402df40:	11000463 	add	w3, w3, #0x1
 402df44:	32010000 	orr	w0, w0, #0x80000000
 402df48:	b9000020 	str	w0, [x1]
 402df4c:	17fffff4 	b	402df1c <ddrtrn_dummy_io_oe_enable+0x24>
 402df50:	0b010081 	add	w1, w4, w1
 402df54:	17fffffa 	b	402df3c <ddrtrn_dummy_io_oe_enable+0x44>

000000000402df58 <ddrtrn_hal_dataeye_get_dm>:
 402df58:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402df5c:	f9000bfe 	str	x30, [sp, #16]
 402df60:	97ffe9a5 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402df64:	b9401814 	ldr	w20, [x0, #24]
 402df68:	97ffe9a3 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402df6c:	b9400400 	ldr	w0, [x0, #4]
 402df70:	11086013 	add	w19, w0, #0x218
 402df74:	53196294 	lsl	w20, w20, #7
 402df78:	97ffe99f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402df7c:	b9402400 	ldr	w0, [x0, #36]
 402df80:	f9400bfe 	ldr	x30, [sp, #16]
 402df84:	0b002a80 	add	w0, w20, w0, lsl #10
 402df88:	0b130000 	add	w0, w0, w19
 402df8c:	a8c253f3 	ldp	x19, x20, [sp], #32
 402df90:	b9400000 	ldr	w0, [x0]
 402df94:	12001800 	and	w0, w0, #0x7f
 402df98:	d65f03c0 	ret

000000000402df9c <ddrtrn_hal_dataeye_set_dq_sum>:
 402df9c:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402dfa0:	2a0003f3 	mov	w19, w0
 402dfa4:	12001a73 	and	w19, w19, #0x7f
 402dfa8:	a9017bf5 	stp	x21, x30, [sp, #16]
 402dfac:	97ffe992 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dfb0:	b9401815 	ldr	w21, [x0, #24]
 402dfb4:	97ffe990 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dfb8:	b9400400 	ldr	w0, [x0, #4]
 402dfbc:	11086014 	add	w20, w0, #0x218
 402dfc0:	531962b5 	lsl	w21, w21, #7
 402dfc4:	97ffe98c 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402dfc8:	b9402400 	ldr	w0, [x0, #36]
 402dfcc:	0b002aa0 	add	w0, w21, w0, lsl #10
 402dfd0:	0b140000 	add	w0, w0, w20
 402dfd4:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402dfd8:	b9000013 	str	w19, [x0]
 402dfdc:	a8c253f3 	ldp	x19, x20, [sp], #32
 402dfe0:	d65f03c0 	ret

000000000402dfe4 <ddrtrn_hal_get_dq_type>:
 402dfe4:	d2801880 	mov	x0, #0xc4                  	// #196
 402dfe8:	f2a22040 	movk	x0, #0x1102, lsl #16
 402dfec:	b9400000 	ldr	w0, [x0]
 402dff0:	d65f03c0 	ret

000000000402dff4 <ddrtrn_hal_set_dq_type>:
 402dff4:	d2801881 	mov	x1, #0xc4                  	// #196
 402dff8:	f2a22041 	movk	x1, #0x1102, lsl #16
 402dffc:	b9000020 	str	w0, [x1]
 402e000:	d65f03c0 	ret

000000000402e004 <ddrtrn_hal_dcc_get_ioctl21>:
 402e004:	f81f0ffe 	str	x30, [sp, #-16]!
 402e008:	97ffe97b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e00c:	b9400400 	ldr	w0, [x0, #4]
 402e010:	52821481 	mov	w1, #0x10a4                	// #4260
 402e014:	0b010000 	add	w0, w0, w1
 402e018:	f84107fe 	ldr	x30, [sp], #16
 402e01c:	b9400000 	ldr	w0, [x0]
 402e020:	d65f03c0 	ret

000000000402e024 <ddrtrn_hal_dcc_set_ioctl21>:
 402e024:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402e028:	2a0003f3 	mov	w19, w0
 402e02c:	97ffe972 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e030:	b9400401 	ldr	w1, [x0, #4]
 402e034:	52821480 	mov	w0, #0x10a4                	// #4260
 402e038:	0b000021 	add	w1, w1, w0
 402e03c:	b9000033 	str	w19, [x1]
 402e040:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402e044:	d65f03c0 	ret

000000000402e048 <ddrtrn_hal_dcc_get_gated_bypass>:
 402e048:	17ffffef 	b	402e004 <ddrtrn_hal_dcc_get_ioctl21>

000000000402e04c <ddrtrn_hal_dcc_set_gated_bypass>:
 402e04c:	17fffff6 	b	402e024 <ddrtrn_hal_dcc_set_ioctl21>

000000000402e050 <ddrtrn_hal_dcc_rdet_enable>:
 402e050:	f81f0ffe 	str	x30, [sp, #-16]!
 402e054:	97ffe968 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e058:	b9400400 	ldr	w0, [x0, #4]
 402e05c:	52802001 	mov	w1, #0x100                 	// #256
 402e060:	11002000 	add	w0, w0, #0x8
 402e064:	f84107fe 	ldr	x30, [sp], #16
 402e068:	b9000001 	str	w1, [x0]
 402e06c:	d65f03c0 	ret

000000000402e070 <ddrtrn_hal_dcc_get_dxnrdbound>:
 402e070:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402e074:	2a0003f3 	mov	w19, w0
 402e078:	97ffe95f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e07c:	53196273 	lsl	w19, w19, #7
 402e080:	b9400400 	ldr	w0, [x0, #4]
 402e084:	11094273 	add	w19, w19, #0x250
 402e088:	0b000273 	add	w19, w19, w0
 402e08c:	b9400260 	ldr	w0, [x19]
 402e090:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402e094:	d65f03c0 	ret

000000000402e098 <ddrtrn_adjust_move_win.constprop.0>:
 402e098:	a9b953f3 	stp	x19, x20, [sp, #-112]!
 402e09c:	2a0103f4 	mov	w20, w1
 402e0a0:	a9015bf5 	stp	x21, x22, [sp, #16]
 402e0a4:	aa0003f6 	mov	x22, x0
 402e0a8:	a90263f7 	stp	x23, x24, [sp, #32]
 402e0ac:	528007f8 	mov	w24, #0x3f                  	// #63
 402e0b0:	52800017 	mov	w23, #0x0                   	// #0
 402e0b4:	a9036bf9 	stp	x25, x26, [sp, #48]
 402e0b8:	52800039 	mov	w25, #0x1                   	// #1
 402e0bc:	5280081a 	mov	w26, #0x40                  	// #64
 402e0c0:	a90473fb 	stp	x27, x28, [sp, #64]
 402e0c4:	5280009b 	mov	w27, #0x4                   	// #4
 402e0c8:	f9002bfe 	str	x30, [sp, #80]
 402e0cc:	97ffe94a 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e0d0:	b9401400 	ldr	w0, [x0, #20]
 402e0d4:	7100081f 	cmp	w0, #0x2
 402e0d8:	52803fe0 	mov	w0, #0x1ff                 	// #511
 402e0dc:	1a800318 	csel	w24, w24, w0, eq  // eq = none
 402e0e0:	97ffee31 	bl	40299a4 <ddrtrn_hal_adjust_get_val>
 402e0e4:	2a0003f5 	mov	w21, w0
 402e0e8:	2a0003fc 	mov	w28, w0
 402e0ec:	97ffe942 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e0f0:	b9401401 	ldr	w1, [x0, #20]
 402e0f4:	7100083f 	cmp	w1, #0x2
 402e0f8:	54000161 	b.ne	402e124 <ddrtrn_adjust_move_win.constprop.0+0x8c>  // b.any
 402e0fc:	b9006fe1 	str	w1, [sp, #108]
 402e100:	94000128 	bl	402e5a0 <ddrtrn_hal_adjust_get_average>
 402e104:	b9406fe1 	ldr	w1, [sp, #108]
 402e108:	7100dc1f 	cmp	w0, #0x37
 402e10c:	52800033 	mov	w19, #0x1                   	// #1
 402e110:	54000169 	b.ls	402e13c <ddrtrn_adjust_move_win.constprop.0+0xa4>  // b.plast
 402e114:	7101201f 	cmp	w0, #0x48
 402e118:	540005e8 	b.hi	402e1d4 <ddrtrn_adjust_move_win.constprop.0+0x13c>  // b.pmore
 402e11c:	52800021 	mov	w1, #0x1                   	// #1
 402e120:	14000007 	b	402e13c <ddrtrn_adjust_move_win.constprop.0+0xa4>
 402e124:	9400011f 	bl	402e5a0 <ddrtrn_hal_adjust_get_average>
 402e128:	7101001f 	cmp	w0, #0x40
 402e12c:	54000489 	b.ls	402e1bc <ddrtrn_adjust_move_win.constprop.0+0x124>  // b.plast
 402e130:	7101201f 	cmp	w0, #0x48
 402e134:	51010013 	sub	w19, w0, #0x40
 402e138:	1a998361 	csel	w1, w27, w25, hi  // hi = pmore
 402e13c:	7100043f 	cmp	w1, #0x1
 402e140:	7a541024 	ccmp	w1, w20, #0x4, ne  // ne = any
 402e144:	540005c0 	b.eq	402e1fc <ddrtrn_adjust_move_win.constprop.0+0x164>  // b.none
 402e148:	97ffe92b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e14c:	b9401400 	ldr	w0, [x0, #20]
 402e150:	7100041f 	cmp	w0, #0x1
 402e154:	54000441 	b.ne	402e1dc <ddrtrn_adjust_move_win.constprop.0+0x144>  // b.any
 402e158:	0b130380 	add	w0, w28, w19
 402e15c:	4b130393 	sub	w19, w28, w19
 402e160:	7100129f 	cmp	w20, #0x4
 402e164:	1a80127c 	csel	w28, w19, w0, ne  // ne = any
 402e168:	97ffe923 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e16c:	b9401400 	ldr	w0, [x0, #20]
 402e170:	7100041f 	cmp	w0, #0x1
 402e174:	540003a1 	b.ne	402e1e8 <ddrtrn_adjust_move_win.constprop.0+0x150>  // b.any
 402e178:	7107ff9f 	cmp	w28, #0x1ff
 402e17c:	54000408 	b.hi	402e1fc <ddrtrn_adjust_move_win.constprop.0+0x164>  // b.pmore
 402e180:	2a1c03e0 	mov	w0, w28
 402e184:	97ffee23 	bl	4029a10 <ddrtrn_hal_adjust_set_val>
 402e188:	aa1603e0 	mov	x0, x22
 402e18c:	97fff7af 	bl	402c048 <ddrtrn_dataeye_deskew>
 402e190:	34000300 	cbz	w0, 402e1f0 <ddrtrn_adjust_move_win.constprop.0+0x158>
 402e194:	2a1503e0 	mov	w0, w21
 402e198:	97ffee1e 	bl	4029a10 <ddrtrn_hal_adjust_set_val>
 402e19c:	aa1603e0 	mov	x0, x22
 402e1a0:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402e1a4:	a94263f7 	ldp	x23, x24, [sp, #32]
 402e1a8:	a9436bf9 	ldp	x25, x26, [sp, #48]
 402e1ac:	a94473fb 	ldp	x27, x28, [sp, #64]
 402e1b0:	f9402bfe 	ldr	x30, [sp, #80]
 402e1b4:	a8c753f3 	ldp	x19, x20, [sp], #112
 402e1b8:	17fff7a4 	b	402c048 <ddrtrn_dataeye_deskew>
 402e1bc:	540002e0 	b.eq	402e218 <ddrtrn_adjust_move_win.constprop.0+0x180>  // b.none
 402e1c0:	4b000353 	sub	w19, w26, w0
 402e1c4:	7100dc1f 	cmp	w0, #0x37
 402e1c8:	54fffaa8 	b.hi	402e11c <ddrtrn_adjust_move_win.constprop.0+0x84>  // b.pmore
 402e1cc:	52800041 	mov	w1, #0x2                   	// #2
 402e1d0:	17ffffdb 	b	402e13c <ddrtrn_adjust_move_win.constprop.0+0xa4>
 402e1d4:	52800081 	mov	w1, #0x4                   	// #4
 402e1d8:	17ffffd9 	b	402e13c <ddrtrn_adjust_move_win.constprop.0+0xa4>
 402e1dc:	4b130380 	sub	w0, w28, w19
 402e1e0:	0b130393 	add	w19, w28, w19
 402e1e4:	17ffffdf 	b	402e160 <ddrtrn_adjust_move_win.constprop.0+0xc8>
 402e1e8:	7100ff9f 	cmp	w28, #0x3f
 402e1ec:	17ffffe4 	b	402e17c <ddrtrn_adjust_move_win.constprop.0+0xe4>
 402e1f0:	110006f7 	add	w23, w23, #0x1
 402e1f4:	6b1802ff 	cmp	w23, w24
 402e1f8:	54fff7a9 	b.ls	402e0ec <ddrtrn_adjust_move_win.constprop.0+0x54>  // b.plast
 402e1fc:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402e200:	a94263f7 	ldp	x23, x24, [sp, #32]
 402e204:	a9436bf9 	ldp	x25, x26, [sp, #48]
 402e208:	a94473fb 	ldp	x27, x28, [sp, #64]
 402e20c:	f9402bfe 	ldr	x30, [sp, #80]
 402e210:	a8c753f3 	ldp	x19, x20, [sp], #112
 402e214:	d65f03c0 	ret
 402e218:	52800033 	mov	w19, #0x1                   	// #1
 402e21c:	17ffffc0 	b	402e11c <ddrtrn_adjust_move_win.constprop.0+0x84>

000000000402e220 <ddrtrn_adjust_dataeye>:
 402e220:	a9bd53f3 	stp	x19, x20, [sp, #-48]!
 402e224:	aa0003f3 	mov	x19, x0
 402e228:	52a20000 	mov	w0, #0x10000000            	// #268435456
 402e22c:	a9015bf5 	stp	x21, x22, [sp, #16]
 402e230:	a9027bf7 	stp	x23, x30, [sp, #32]
 402e234:	97ffed79 	bl	4029818 <ddrtrn_hal_check_bypass>
 402e238:	35000220 	cbnz	w0, 402e27c <ddrtrn_adjust_dataeye+0x5c>
 402e23c:	97fff19b 	bl	402a8a8 <ddrtrn_hal_get_adjust>
 402e240:	340001e0 	cbz	w0, 402e27c <ddrtrn_adjust_dataeye+0x5c>
 402e244:	52800014 	mov	w20, #0x0                   	// #0
 402e248:	d2800196 	mov	x22, #0xc                   	// #12
 402e24c:	d2800797 	mov	x23, #0x3c                  	// #60
 402e250:	97ffe8e9 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e254:	aa0003f5 	mov	x21, x0
 402e258:	97ffe8e2 	bl	40285e0 <ddrtrn_hal_get_phy>
 402e25c:	b94022a2 	ldr	w2, [x21, #32]
 402e260:	b9402aa1 	ldr	w1, [x21, #40]
 402e264:	9b177c42 	mul	x2, x2, x23
 402e268:	9b160821 	madd	x1, x1, x22, x2
 402e26c:	8b010000 	add	x0, x0, x1
 402e270:	b9401800 	ldr	w0, [x0, #24]
 402e274:	6b00029f 	cmp	w20, w0
 402e278:	540000a3 	b.cc	402e28c <ddrtrn_adjust_dataeye+0x6c>  // b.lo, b.ul, b.last
 402e27c:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402e280:	a9427bf7 	ldp	x23, x30, [sp, #32]
 402e284:	a8c353f3 	ldp	x19, x20, [sp], #48
 402e288:	d65f03c0 	ret
 402e28c:	97ffe8da 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e290:	b9402815 	ldr	w21, [x0, #40]
 402e294:	97ffe8d8 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e298:	0b150695 	add	w21, w20, w21, lsl #1
 402e29c:	b9001815 	str	w21, [x0, #24]
 402e2a0:	940000c0 	bl	402e5a0 <ddrtrn_hal_adjust_get_average>
 402e2a4:	7100dc1f 	cmp	w0, #0x37
 402e2a8:	540000a8 	b.hi	402e2bc <ddrtrn_adjust_dataeye+0x9c>  // b.pmore
 402e2ac:	52800081 	mov	w1, #0x4                   	// #4
 402e2b0:	aa1303e0 	mov	x0, x19
 402e2b4:	97ffff79 	bl	402e098 <ddrtrn_adjust_move_win.constprop.0>
 402e2b8:	14000003 	b	402e2c4 <ddrtrn_adjust_dataeye+0xa4>
 402e2bc:	7101201f 	cmp	w0, #0x48
 402e2c0:	54000068 	b.hi	402e2cc <ddrtrn_adjust_dataeye+0xac>  // b.pmore
 402e2c4:	11000694 	add	w20, w20, #0x1
 402e2c8:	17ffffe2 	b	402e250 <ddrtrn_adjust_dataeye+0x30>
 402e2cc:	52800041 	mov	w1, #0x2                   	// #2
 402e2d0:	17fffff8 	b	402e2b0 <ddrtrn_adjust_dataeye+0x90>

000000000402e2d4 <ddrtrn_ddrt_init>:
 402e2d4:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402e2d8:	2a0003f4 	mov	w20, w0
 402e2dc:	f9000bfe 	str	x30, [sp, #16]
 402e2e0:	97ffe8c5 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e2e4:	b9402400 	ldr	w0, [x0, #36]
 402e2e8:	7100041f 	cmp	w0, #0x1
 402e2ec:	54000621 	b.ne	402e3b0 <ddrtrn_ddrt_init+0xdc>  // b.any
 402e2f0:	97ffec3b 	bl	40293dc <ddrtrn_hal_get_rank_size>
 402e2f4:	2a0003f3 	mov	w19, w0
 402e2f8:	97ffecd4 	bl	4029648 <ddrtrn_hal_ddrt_get_mem_width>
 402e2fc:	51000400 	sub	w0, w0, #0x1
 402e300:	52802a42 	mov	w2, #0x152                 	// #338
 402e304:	52800181 	mov	w1, #0xc                   	// #12
 402e308:	2a003040 	orr	w0, w2, w0, lsl #12
 402e30c:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e310:	97ffeccb 	bl	402963c <ddrt_reg_write>
 402e314:	52800381 	mov	w1, #0x1c                  	// #28
 402e318:	52a80000 	mov	w0, #0x40000000            	// #1073741824
 402e31c:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e320:	97ffecc7 	bl	402963c <ddrt_reg_write>
 402e324:	97fff870 	bl	402c4e4 <ddrtrn_ddrt_get_test_addr>
 402e328:	0b130000 	add	w0, w0, w19
 402e32c:	52800401 	mov	w1, #0x20                  	// #32
 402e330:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e334:	97ffecc2 	bl	402963c <ddrt_reg_write>
 402e338:	52800701 	mov	w1, #0x38                  	// #56
 402e33c:	528dada0 	mov	w0, #0x6d6d                	// #28013
 402e340:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e344:	72adada0 	movk	w0, #0x6d6d, lsl #16
 402e348:	97ffecbd 	bl	402963c <ddrt_reg_write>
 402e34c:	52800101 	mov	w1, #0x8                   	// #8
 402e350:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e354:	7100069f 	cmp	w20, #0x1
 402e358:	54000301 	b.ne	402e3b8 <ddrtrn_ddrt_init+0xe4>  // b.any
 402e35c:	52800860 	mov	w0, #0x43                  	// #67
 402e360:	97ffecb7 	bl	402963c <ddrt_reg_write>
 402e364:	52800201 	mov	w1, #0x10                  	// #16
 402e368:	52800000 	mov	w0, #0x0                   	// #0
 402e36c:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e370:	97ffecb3 	bl	402963c <ddrt_reg_write>
 402e374:	52800281 	mov	w1, #0x14                  	// #20
 402e378:	52800000 	mov	w0, #0x0                   	// #0
 402e37c:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e380:	97ffecaf 	bl	402963c <ddrt_reg_write>
 402e384:	52800301 	mov	w1, #0x18                  	// #24
 402e388:	52800000 	mov	w0, #0x0                   	// #0
 402e38c:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e390:	97ffecab 	bl	402963c <ddrt_reg_write>
 402e394:	52800601 	mov	w1, #0x30                  	// #48
 402e398:	528ab540 	mov	w0, #0x55aa                	// #21930
 402e39c:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e3a0:	72aab540 	movk	w0, #0x55aa, lsl #16
 402e3a4:	f9400bfe 	ldr	x30, [sp, #16]
 402e3a8:	a8c253f3 	ldp	x19, x20, [sp], #32
 402e3ac:	17ffeca4 	b	402963c <ddrt_reg_write>
 402e3b0:	52800013 	mov	w19, #0x0                   	// #0
 402e3b4:	17ffffd1 	b	402e2f8 <ddrtrn_ddrt_init+0x24>
 402e3b8:	528009e0 	mov	w0, #0x4f                  	// #79
 402e3bc:	97ffeca0 	bl	402963c <ddrt_reg_write>
 402e3c0:	97ffe88d 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e3c4:	b9402813 	ldr	w19, [x0, #40]
 402e3c8:	97ffe88b 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e3cc:	aa0003f4 	mov	x20, x0
 402e3d0:	97ffe884 	bl	40285e0 <ddrtrn_hal_get_phy>
 402e3d4:	2a1303f3 	mov	w19, w19
 402e3d8:	d2800782 	mov	x2, #0x3c                  	// #60
 402e3dc:	b9402281 	ldr	w1, [x20, #32]
 402e3e0:	9b027c21 	mul	x1, x1, x2
 402e3e4:	d2800182 	mov	x2, #0xc                   	// #12
 402e3e8:	9b020673 	madd	x19, x19, x2, x1
 402e3ec:	52800601 	mov	w1, #0x30                  	// #48
 402e3f0:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e3f4:	8b130013 	add	x19, x0, x19
 402e3f8:	b9401e60 	ldr	w0, [x19, #28]
 402e3fc:	97ffec90 	bl	402963c <ddrt_reg_write>
 402e400:	52800201 	mov	w1, #0x10                  	// #16
 402e404:	52800fe0 	mov	w0, #0x7f                  	// #127
 402e408:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e40c:	97ffec8c 	bl	402963c <ddrt_reg_write>
 402e410:	52800281 	mov	w1, #0x14                  	// #20
 402e414:	12800000 	mov	w0, #0xffffffff            	// #-1
 402e418:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e41c:	97ffec88 	bl	402963c <ddrt_reg_write>
 402e420:	52800301 	mov	w1, #0x18                  	// #24
 402e424:	52800000 	mov	w0, #0x0                   	// #0
 402e428:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e42c:	17ffffde 	b	402e3a4 <ddrtrn_ddrt_init+0xd0>

000000000402e430 <ddrtrn_ddrt_test>:
 402e430:	a9bc53f3 	stp	x19, x20, [sp, #-64]!
 402e434:	2a0203f3 	mov	w19, w2
 402e438:	2a0103f4 	mov	w20, w1
 402e43c:	52a222c1 	mov	w1, #0x11160000            	// #286654464
 402e440:	a9015bf5 	stp	x21, x22, [sp, #16]
 402e444:	2a0003f5 	mov	w21, w0
 402e448:	32000000 	orr	w0, w0, #0x1
 402e44c:	a90263f7 	stp	x23, x24, [sp, #32]
 402e450:	f9001bfe 	str	x30, [sp, #48]
 402e454:	97ffec7a 	bl	402963c <ddrt_reg_write>
 402e458:	52800081 	mov	w1, #0x4                   	// #4
 402e45c:	52800000 	mov	w0, #0x0                   	// #0
 402e460:	72a222c1 	movk	w1, #0x1116, lsl #16
 402e464:	97ffec76 	bl	402963c <ddrt_reg_write>
 402e468:	d5033f9f 	dsb	sy
 402e46c:	52800098 	mov	w24, #0x4                   	// #4
 402e470:	52884817 	mov	w23, #0x4240                	// #16960
 402e474:	52800016 	mov	w22, #0x0                   	// #0
 402e478:	72a222d8 	movk	w24, #0x1116, lsl #16
 402e47c:	72a001f7 	movk	w23, #0xf, lsl #16
 402e480:	2a1803e0 	mov	w0, w24
 402e484:	110006d6 	add	w22, w22, #0x1
 402e488:	97ffec6a 	bl	4029630 <ddrt_reg_read>
 402e48c:	370001c0 	tbnz	w0, #0, 402e4c4 <ddrtrn_ddrt_test+0x94>
 402e490:	6b1702df 	cmp	w22, w23
 402e494:	54ffff61 	b.ne	402e480 <ddrtrn_ddrt_test+0x50>  // b.any
 402e498:	12800003 	mov	w3, #0xffffffff            	// #-1
 402e49c:	52800001 	mov	w1, #0x0                   	// #0
 402e4a0:	2a0303e2 	mov	w2, w3
 402e4a4:	52800100 	mov	w0, #0x8                   	// #8
 402e4a8:	97ffef68 	bl	402a248 <ddrtrn_hal_training_stat>
 402e4ac:	12800000 	mov	w0, #0xffffffff            	// #-1
 402e4b0:	a9415bf5 	ldp	x21, x22, [sp, #16]
 402e4b4:	a94263f7 	ldp	x23, x24, [sp, #32]
 402e4b8:	f9401bfe 	ldr	x30, [sp, #48]
 402e4bc:	a8c453f3 	ldp	x19, x20, [sp], #64
 402e4c0:	d65f03c0 	ret
 402e4c4:	6b1702df 	cmp	w22, w23
 402e4c8:	54fffe80 	b.eq	402e498 <ddrtrn_ddrt_test+0x68>  // b.none
 402e4cc:	121806b5 	and	w21, w21, #0x300
 402e4d0:	710802bf 	cmp	w21, #0x200
 402e4d4:	54000620 	b.eq	402e598 <ddrtrn_ddrt_test+0x168>  // b.none
 402e4d8:	37080600 	tbnz	w0, #1, 402e598 <ddrtrn_ddrt_test+0x168>
 402e4dc:	3100067f 	cmn	w19, #0x1
 402e4e0:	54000320 	b.eq	402e544 <ddrtrn_ddrt_test+0x114>  // b.none
 402e4e4:	52801000 	mov	w0, #0x80                  	// #128
 402e4e8:	0b140e75 	add	w21, w19, w20, lsl #3
 402e4ec:	72a222c0 	movk	w0, #0x1116, lsl #16
 402e4f0:	97ffec50 	bl	4029630 <ddrt_reg_read>
 402e4f4:	52800021 	mov	w1, #0x1                   	// #1
 402e4f8:	1ad52021 	lsl	w1, w1, w21
 402e4fc:	6a00003f 	tst	w1, w0
 402e500:	54fffd61 	b.ne	402e4ac <ddrtrn_ddrt_test+0x7c>  // b.any
 402e504:	531d7261 	lsl	w1, w19, #3
 402e508:	7100127f 	cmp	w19, #0x4
 402e50c:	51001275 	sub	w21, w19, #0x4
 402e510:	53027e73 	lsr	w19, w19, #2
 402e514:	0b140673 	add	w19, w19, w20, lsl #1
 402e518:	52800c00 	mov	w0, #0x60                  	// #96
 402e51c:	531d72b5 	lsl	w21, w21, #3
 402e520:	72a222c0 	movk	w0, #0x1116, lsl #16
 402e524:	1a95b035 	csel	w21, w1, w21, lt  // lt = tstop
 402e528:	0b130800 	add	w0, w0, w19, lsl #2
 402e52c:	97ffec41 	bl	4029630 <ddrt_reg_read>
 402e530:	52801fe1 	mov	w1, #0xff                  	// #255
 402e534:	1ad52021 	lsl	w1, w1, w21
 402e538:	6a00003f 	tst	w1, w0
 402e53c:	5a9f03e0 	csetm	w0, ne  // ne = any
 402e540:	17ffffdc 	b	402e4b0 <ddrtrn_ddrt_test+0x80>
 402e544:	3100069f 	cmn	w20, #0x1
 402e548:	54000280 	b.eq	402e598 <ddrtrn_ddrt_test+0x168>  // b.none
 402e54c:	52801000 	mov	w0, #0x80                  	// #128
 402e550:	72a222c0 	movk	w0, #0x1116, lsl #16
 402e554:	97ffec37 	bl	4029630 <ddrt_reg_read>
 402e558:	531d7282 	lsl	w2, w20, #3
 402e55c:	52801fe1 	mov	w1, #0xff                  	// #255
 402e560:	1ac22021 	lsl	w1, w1, w2
 402e564:	6a00003f 	tst	w1, w0
 402e568:	54fffa21 	b.ne	402e4ac <ddrtrn_ddrt_test+0x7c>  // b.any
 402e56c:	52800c13 	mov	w19, #0x60                  	// #96
 402e570:	72a222d3 	movk	w19, #0x1116, lsl #16
 402e574:	0b130040 	add	w0, w2, w19
 402e578:	0b140e73 	add	w19, w19, w20, lsl #3
 402e57c:	97ffec2d 	bl	4029630 <ddrt_reg_read>
 402e580:	2a0003f5 	mov	w21, w0
 402e584:	11001260 	add	w0, w19, #0x4
 402e588:	97ffec2a 	bl	4029630 <ddrt_reg_read>
 402e58c:	2b0002bf 	cmn	w21, w0
 402e590:	5a9f03e0 	csetm	w0, ne  // ne = any
 402e594:	17ffffc7 	b	402e4b0 <ddrtrn_ddrt_test+0x80>
 402e598:	52800000 	mov	w0, #0x0                   	// #0
 402e59c:	17ffffc5 	b	402e4b0 <ddrtrn_ddrt_test+0x80>

000000000402e5a0 <ddrtrn_hal_adjust_get_average>:
 402e5a0:	a9be53f3 	stp	x19, x20, [sp, #-32]!
 402e5a4:	a9017bf5 	stp	x21, x30, [sp, #16]
 402e5a8:	97ffe813 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e5ac:	b9400415 	ldr	w21, [x0, #4]
 402e5b0:	97ffe811 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e5b4:	b9401814 	ldr	w20, [x0, #24]
 402e5b8:	97ffe80f 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e5bc:	b9402413 	ldr	w19, [x0, #36]
 402e5c0:	97ffe80d 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e5c4:	b9401400 	ldr	w0, [x0, #20]
 402e5c8:	53196281 	lsl	w1, w20, #7
 402e5cc:	7100081f 	cmp	w0, #0x2
 402e5d0:	53165660 	lsl	w0, w19, #10
 402e5d4:	0b010000 	add	w0, w0, w1
 402e5d8:	0b150000 	add	w0, w0, w21
 402e5dc:	54000301 	b.ne	402e63c <ddrtrn_hal_adjust_get_average+0x9c>  // b.any
 402e5e0:	11084001 	add	w1, w0, #0x210
 402e5e4:	11085000 	add	w0, w0, #0x214
 402e5e8:	b9400021 	ldr	w1, [x1]
 402e5ec:	b9400002 	ldr	w2, [x0]
 402e5f0:	d3505823 	ubfx	x3, x1, #16, #7
 402e5f4:	d3483820 	ubfx	x0, x1, #8, #7
 402e5f8:	12001844 	and	w4, w2, #0x7f
 402e5fc:	0b030000 	add	w0, w0, w3
 402e600:	12001823 	and	w3, w1, #0x7f
 402e604:	d3587821 	ubfx	x1, x1, #24, #7
 402e608:	0b040063 	add	w3, w3, w4
 402e60c:	0b030000 	add	w0, w0, w3
 402e610:	d3483843 	ubfx	x3, x2, #8, #7
 402e614:	0b030021 	add	w1, w1, w3
 402e618:	0b010000 	add	w0, w0, w1
 402e61c:	d3505841 	ubfx	x1, x2, #16, #7
 402e620:	d3587842 	ubfx	x2, x2, #24, #7
 402e624:	0b020022 	add	w2, w1, w2
 402e628:	0b020000 	add	w0, w0, w2
 402e62c:	a9417bf5 	ldp	x21, x30, [sp, #16]
 402e630:	53037c00 	lsr	w0, w0, #3
 402e634:	a8c253f3 	ldp	x19, x20, [sp], #32
 402e638:	d65f03c0 	ret
 402e63c:	11087001 	add	w1, w0, #0x21c
 402e640:	11088000 	add	w0, w0, #0x220
 402e644:	b9400021 	ldr	w1, [x1]
 402e648:	17ffffe9 	b	402e5ec <ddrtrn_hal_adjust_get_average+0x4c>

000000000402e64c <ddrtrn_hal_adjust_get_rdqs>:
 402e64c:	a9bf7bf3 	stp	x19, x30, [sp, #-16]!
 402e650:	97ffe7e9 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e654:	b9400413 	ldr	w19, [x0, #4]
 402e658:	97ffe7e7 	bl	40285f4 <ddrtrn_hal_get_ctx>
 402e65c:	b9401800 	ldr	w0, [x0, #24]
 402e660:	1108b273 	add	w19, w19, #0x22c
 402e664:	0b001e60 	add	w0, w19, w0, lsl #7
 402e668:	a8c17bf3 	ldp	x19, x30, [sp], #16
 402e66c:	b9400000 	ldr	w0, [x0]
 402e670:	12002000 	and	w0, w0, #0x1ff
 402e674:	d65f03c0 	ret

Disassembly of section .vectors:

000000000402e800 <sync_exception_sp_el0>:
	...

000000000402e880 <irq_sp_el0>:
	...

000000000402e900 <fiq_sp_el0>:
	...

000000000402e980 <serror_sp_el0>:
	...

000000000402ea00 <sync_exception_sp_elx>:
	...

000000000402ea80 <irq_sp_elx>:
	...

000000000402eb00 <fiq_sp_elx>:
	...

000000000402eb80 <serror_sp_elx>:
	...

000000000402ec00 <sync_exception_aarch64>:
 402ec00:	94000020 	bl	402ec80 <el1_to_el3_entry>
	...

000000000402ec80 <el1_to_el3_entry>:
 402ec80:	58000540 	ldr	x0, 402ed28 <power_down_sequence_end>
 402ec84:	927cec1f 	and	sp, x0, #0xfffffffffffffff0
 402ec88:	58000553 	ldr	x19, 402ed30 <power_down_sequence_end+0x8>
 402ec8c:	97ffda70 	bl	402564c <get_ddr_param_data_end_addr>
 402ec90:	cb130014 	sub	x20, x0, x19
 402ec94:	58000520 	ldr	x0, 402ed38 <power_down_sequence_end+0x10>
 402ec98:	58000541 	ldr	x1, 402ed40 <power_down_sequence_end+0x18>
 402ec9c:	cb000022 	sub	x2, x1, x0
 402eca0:	58000541 	ldr	x1, 402ed48 <power_down_sequence_end+0x20>
 402eca4:	97ffd0b1 	bl	4022f68 <copy_code_to_sram>
 402eca8:	97ffd550 	bl	40241e8 <sram_to_npu_info>
 402ecac:	580004fe 	ldr	x30, 402ed48 <power_down_sequence_end+0x20>
 402ecb0:	d65f03c0 	ret
 402ecb4:	d503201f 	nop

000000000402ecb8 <power_down_sequence_start>:
 402ecb8:	aa1303e0 	mov	x0, x19
 402ecbc:	d2800001 	mov	x1, #0x0                   	// #0
 402ecc0:	aa1403e2 	mov	x2, x20
 402ecc4:	94000010 	bl	402ed04 <clear_data>
 402ecc8:	d28a0481 	mov	x1, #0x5024                	// #20516
 402eccc:	f2a22041 	movk	x1, #0x1102, lsl #16
 402ecd0:	b9400020 	ldr	w0, [x1]
 402ecd4:	32000000 	orr	w0, w0, #0x1
 402ecd8:	b9000020 	str	w0, [x1]
 402ecdc:	58000223 	ldr	x3, 402ed20 <_MISC_REG_CPU_CTRL6>
 402ece0:	b9400061 	ldr	w1, [x3]
 402ece4:	321a0021 	orr	w1, w1, #0x40
 402ece8:	b9000061 	str	w1, [x3]
 402ecec:	d538f2e0 	mrs	x0, s3_0_c15_c2_7
 402ecf0:	b2400000 	orr	x0, x0, #0x1
 402ecf4:	d518f2e0 	msr	s3_0_c15_c2_7, x0
 402ecf8:	d5033fdf 	isb
 402ecfc:	d503207f 	wfi
 402ed00:	d65f03c0 	ret

000000000402ed04 <clear_data>:
 402ed04:	8b020002 	add	x2, x0, x2
 402ed08:	d2800103 	mov	x3, #0x8                   	// #8

000000000402ed0c <clear_data_loop>:
 402ed0c:	f9000001 	str	x1, [x0]
 402ed10:	8b030000 	add	x0, x0, x3
 402ed14:	eb02001f 	cmp	x0, x2
 402ed18:	54ffffab 	b.lt	402ed0c <clear_data_loop>  // b.tstop
 402ed1c:	d65f03c0 	ret

000000000402ed20 <_MISC_REG_CPU_CTRL6>:
 402ed20:	11024128 	add	w8, w9, #0x90
 402ed24:	00000000 	udf	#0

000000000402ed28 <power_down_sequence_end>:
 402ed28:	0403f000 	msb	z0.b, p4/m, z3.b, z0.b
 402ed2c:	00000000 	udf	#0
 402ed30:	04020000 	.inst	0x04020000 ; undefined
 402ed34:	00000000 	udf	#0
 402ed38:	0402ecb8 	msb	z24.b, p3/m, z2.b, z5.b
 402ed3c:	00000000 	udf	#0
 402ed40:	0402ed28 	msb	z8.b, p3/m, z2.b, z9.b
 402ed44:	00000000 	udf	#0
 402ed48:	4000f000 	.inst	0x4000f000 ; undefined
 402ed4c:	00000000 	udf	#0

Disassembly of section .rodata:

000000000402ed50 <g_rom_ecc>:
 402ed50:	0402ee30 	msb	z16.b, p3/m, z2.b, z17.b
 402ed54:	00000000 	udf	#0
 402ed58:	0402ee10 	msb	z16.b, p3/m, z2.b, z16.b
 402ed5c:	00000000 	udf	#0
 402ed60:	0402edf0 	msb	z16.b, p3/m, z2.b, z15.b
 402ed64:	00000000 	udf	#0
 402ed68:	0402edd0 	msb	z16.b, p3/m, z2.b, z14.b
 402ed6c:	00000000 	udf	#0
 402ed70:	0402edb0 	msb	z16.b, p3/m, z2.b, z13.b
 402ed74:	00000000 	udf	#0
 402ed78:	0402ed90 	msb	z16.b, p3/m, z2.b, z12.b
 402ed7c:	00000000 	udf	#0
 402ed80:	00000001 	udf	#1
 402ed84:	00000020 	udf	#32
	...

000000000402ed90 <rom_ecc_n>:
 402ed90:	db57fba9 	.inst	0xdb57fba9 ; undefined
 402ed94:	bca9eea1 	.inst	0xbca9eea1 ; undefined
 402ed98:	900a663e 	adrp	x30, 18cf2000 <_end+0x14cc2800>
 402ed9c:	718d839d 	.inst	0x718d839d ; undefined
 402eda0:	a37a398c 	.inst	0xa37a398c ; undefined
 402eda4:	f7a661b5 	.inst	0xf7a661b5 ; undefined
 402eda8:	820e1e90 	.inst	0x820e1e90 ; undefined
 402edac:	a7564897 	.inst	0xa7564897 ; undefined

000000000402edb0 <rom_ecc_gy>:
 402edb0:	35f87e54 	cbnz	w20, 401fd78 <hash_verify_buf-0x1d08>
 402edb4:	fdc4dac3 	.inst	0xfdc4dac3 ; undefined
 402edb8:	1a46f897 	.inst	0x1a46f897 ; undefined
 402edbc:	c91d6114 	.inst	0xc91d6114 ; undefined
 402edc0:	134577c2 	.inst	0x134577c2 ; undefined
 402edc4:	548eed2d 	b.le	3f4cb68 <hash_verify_buf-0xd4f18>
 402edc8:	c7541d5c 	.inst	0xc7541d5c ; undefined
 402edcc:	9769042f 	bl	1a6fe88 <hash_verify_buf-0x25b1bf8>

000000000402edd0 <rom_ecc_gx>:
 402edd0:	b9aed28b 	ldrsw	x11, [x20, #11984]
 402edd4:	cb577ecb 	sub	x11, x22, x23, lsr #31
 402edd8:	2f484b2c 	mls	v12.4h, v25.4h, v8.h[4]
 402eddc:	afb781fc 	.inst	0xafb781fc ; undefined
 402ede0:	e127deb9 	.inst	0xe127deb9 ; undefined
 402ede4:	c223bde3 	.inst	0xc223bde3 ; undefined
 402ede8:	bd53443a 	ldr	s26, [x1, #4932]
 402edec:	6232ce9a 	.inst	0x6232ce9a ; undefined

000000000402edf0 <rom_ecc_b>:
 402edf0:	6c5cdc26 	ldnp	d6, d23, [x1, #456]
 402edf4:	444b4ae9 	umlalb	z9.h, z23.b, z11.b
 402edf8:	d9b530f3 	.inst	0xd9b530f3 ; undefined
 402edfc:	bf7cd7bb 	.inst	0xbf7cd7bb ; undefined
 402ee00:	29168495 	stp	w21, w1, [x4, #180]
 402ee04:	cee1f75c 	.inst	0xcee1f75c ; undefined
 402ee08:	18dccc6b 	ldr	w11, 3fe8794 <hash_verify_buf-0x392ec>
 402ee0c:	b6078cff 	tbz	xzr, #32, 402dfa8 <ddrtrn_hal_dataeye_set_dq_sum+0xc>

000000000402ee10 <rom_ecc_a>:
 402ee10:	75095a7d 	.inst	0x75095a7d ; undefined
 402ee14:	57302cfc 	.inst	0x57302cfc ; undefined
 402ee18:	3075f6ee 	adr	x14, 411acf5 <_end+0xeb4f5>
 402ee1c:	e7ff7a41 	.inst	0xe7ff7a41 ; undefined
 402ee20:	c15580fb 	.inst	0xc15580fb ; undefined
 402ee24:	6c5cdc26 	ldnp	d6, d23, [x1, #456]
 402ee28:	444b4ae9 	umlalb	z9.h, z23.b, z11.b
 402ee2c:	d9b530f3 	.inst	0xd9b530f3 ; undefined

000000000402ee30 <rom_ecc_p>:
 402ee30:	db57fba9 	.inst	0xdb57fba9 ; undefined
 402ee34:	bca9eea1 	.inst	0xbca9eea1 ; undefined
 402ee38:	900a663e 	adrp	x30, 18cf2000 <_end+0x14cc2800>
 402ee3c:	728d839d 	movk	w29, #0x6c1c
 402ee40:	23f63b6e 	.inst	0x23f63b6e ; undefined
 402ee44:	282026d5 	stnp	w21, w9, [x22, #-256]
 402ee48:	1d481320 	.inst	0x1d481320 ; undefined
 402ee4c:	77536e1f 	.inst	0x77536e1f ; undefined

000000000402ee50 <g_crc_ta>:
 402ee50:	10210000 	adr	x0, 4070e50 <_end+0x41650>
 402ee54:	30632042 	adr	x2, 40f525d <_end+0xc5a5d>
 402ee58:	50a54084 	adr	x4, 3f7966a <hash_verify_buf-0xa8416>
 402ee5c:	70e760c6 	adr	x6, 3ffda77 <hash_verify_buf-0x24009>
 402ee60:	91298108 	add	x8, x8, #0xa60
 402ee64:	b16ba14a 	adds	x10, x10, #0xae8, lsl #12
 402ee68:	d1adc18c 	.inst	0xd1adc18c ; undefined
 402ee6c:	f1efe1ce 	.inst	0xf1efe1ce ; undefined

000000000402ee70 <malloc_sizes_ro>:
 402ee70:	00020050 	.inst	0x00020050 ; undefined
 402ee74:	00050100 	.inst	0x00050100 ; undefined
 402ee78:	00040200 	.inst	0x00040200 ; undefined
 402ee7c:	00010300 	.inst	0x00010300 ; undefined
 402ee80:	00010800 	.inst	0x00010800 ; undefined
 402ee84:	00010b00 	.inst	0x00010b00 ; undefined
 402ee88:	00010c00 	.inst	0x00010c00 ; undefined
 402ee8c:	00014000 	.inst	0x00014000 ; undefined
 402ee90:	00000000 	udf	#0

Disassembly of section .rodata.str1.1:

000000000402ee94 <.rodata.str1.1>:
 402ee94:	62616e65 	.inst	0x62616e65 ; undefined
 402ee98:	6c5f656c 	ldnp	d12, d25, [x11, #496]
 402ee9c:	5f736470 	.inst	0x5f736470 ; undefined
 402eea0:	6b636f6c 	.inst	0x6b636f6c ; undefined
 402eea4:	72726520 	.inst	0x72726520 ; undefined
 402eea8:	0a0d726f 	and	w15, w19, w13, lsl #28
 402eeac:	706c0a00 	adr	x0, 4106fef <_end+0xd77ef>
 402eeb0:	63207364 	.inst	0x63207364 ; undefined
 402eeb4:	7261656c 	.inst	0x7261656c ; undefined
 402eeb8:	0a6b6f20 	bic	w0, w25, w11, lsr #27
 402eebc:	33323100 	.inst	0x33323100 ; undefined
 402eec0:	37363534 	tbnz	w20, #6, 402b564 <ddrtrn_dcc_training+0x4bc>
 402eec4:	33323138 	.inst	0x33323138 ; undefined
 402eec8:	37363534 	tbnz	w20, #6, 402b56c <ddrtrn_dcc_training+0x4c4>
 402eecc:	0a0d0038 	and	w24, w1, w13
 402eed0:	746f6f62 	.inst	0x746f6f62 ; undefined
 402eed4:	62617420 	.inst	0x62617420 ; undefined
 402eed8:	6220656c 	.inst	0x6220656c ; undefined
 402eedc:	6472616f 	.inst	0x6472616f ; undefined
 402eee0:	69777320 	ldpsw	x0, x28, [x25, #-72]
 402eee4:	3a686374 	.inst	0x3a686374 ; undefined
 402eee8:	0a007830 	and	w16, w1, w0, lsl #30
 402eeec:	423a470d 	.inst	0x423a470d ; undefined
 402eef0:	20746f6f 	.inst	0x20746f6f ; undefined
 402eef4:	6c696166 	ldnp	d6, d24, [x11, #-368]
 402eef8:	0a006465 	and	w5, w3, w0, lsl #25
 402eefc:	733a470d 	.inst	0x733a470d ; undefined
 402ef00:	2074666f 	.inst	0x2074666f ; undefined
 402ef04:	00747372 	.inst	0x00747372 ; undefined
 402ef08:	6172730a 	.inst	0x6172730a ; undefined
 402ef0c:	6f74206d 	umlal2	v13.4s, v3.8h, v4.h[3]
 402ef10:	75706e20 	.inst	0x75706e20 ; undefined
 402ef14:	63000a21 	.inst	0x63000a21 ; undefined
 402ef18:	656e6e6f 	fnmls	z15.h, p3/m, z19.h, z14.h
 402ef1c:	74207463 	.inst	0x74207463 ; undefined
 402ef20:	6f656d69 	.inst	0x6f656d69 ; undefined
 402ef24:	000a7475 	.inst	0x000a7475 ; undefined
 402ef28:	646e6573 	.inst	0x646e6573 ; undefined
 402ef2c:	616f625f 	.inst	0x616f625f ; undefined
 402ef30:	745f6472 	.inst	0x745f6472 ; undefined
 402ef34:	5f657079 	sqdmlsl	s25, h3, v5.h[2]
 402ef38:	755f6f74 	.inst	0x755f6f74 ; undefined
 402ef3c:	6f206273 	.inst	0x6f206273 ; undefined
 402ef40:	0a000a6b 	and	w11, w19, w0, lsl #2
 402ef44:	746f6f62 	.inst	0x746f6f62 ; undefined
 402ef48:	62617420 	.inst	0x62617420 ; undefined
 402ef4c:	7620656c 	.inst	0x7620656c ; undefined
 402ef50:	69737265 	ldpsw	x5, x28, [x19, #-104]
 402ef54:	20206e6f 	.inst	0x20206e6f ; undefined
 402ef58:	3a202020 	.inst	0x3a202020 ; undefined
 402ef5c:	6f620a00 	mla	v0.8h, v16.8h, v2.h[6]
 402ef60:	7420746f 	.inst	0x7420746f ; undefined
 402ef64:	656c6261 	fnmls	z1.h, p0/m, z19.h, z12.h
 402ef68:	69756220 	ldpsw	x0, x24, [x17, #-88]
 402ef6c:	7420646c 	.inst	0x7420646c ; undefined
 402ef70:	20656d69 	.inst	0x20656d69 ; undefined
 402ef74:	0a003a20 	and	w0, w17, w0, lsl #14
 402ef78:	746f6f62 	.inst	0x746f6f62 ; undefined
 402ef7c:	62617420 	.inst	0x62617420 ; undefined
 402ef80:	6620656c 	.inst	0x6620656c ; undefined
 402ef84:	20656c69 	.inst	0x20656c69 ; undefined
 402ef88:	656d616e 	fnmls	z14.h, p0/m, z11.h, z13.h
 402ef8c:	3a202020 	.inst	0x3a202020 ; undefined
 402ef90:	65470a00 	fmul	z0.h, z16.h, z7.h
 402ef94:	44442074 	cmla	z20.h, z3.h, z4.h, #0
 402ef98:	69732052 	ldpsw	x18, x8, [x2, #-104]
 402ef9c:	6620657a 	.inst	0x6620657a ; undefined
 402efa0:	216c6961 	.inst	0x216c6961 ; undefined
 402efa4:	44440a00 	sqdmlalbt	z0.h, z16.b, z4.b
 402efa8:	69732052 	ldpsw	x18, x8, [x2, #-104]
 402efac:	203a657a 	.inst	0x203a657a ; undefined
 402efb0:	4d007830 	st3	{v16.h-v18.h}[7], [x1]
 402efb4:	65730042 	fmla	z2.h, p0/m, z2.h, z19.h
 402efb8:	65727563 	fnmls	z3.h, p5/m, z11.h, z18.h
 402efbc:	61727320 	.inst	0x61727320 ; undefined
 402efc0:	6573206d 	fmls	z13.h, p0/m, z3.h, z19.h
 402efc4:	72652074 	.inst	0x72652074 ; undefined
 402efc8:	0a726f72 	bic	w18, w27, w18, lsr #27
 402efcc:	726f6300 	.inst	0x726f6300 ; undefined
 402efd0:	67203065 	.inst	0x67203065 ; undefined
 402efd4:	6d20746f 	stp	d15, d29, [x3, #-512]
 402efd8:	65206773 	.inst	0x65206773 ; undefined
 402efdc:	726f7272 	.inst	0x726f7272 ; undefined
 402efe0:	6f63000a 	mla	v10.8h, v0.8h, v3.h[2]
 402efe4:	61207970 	.inst	0x61207970 ; undefined
 402efe8:	63206674 	.inst	0x63206674 ; undefined
 402efec:	2065646f 	.inst	0x2065646f ; undefined
 402eff0:	6f727265 	fcmla	v5.8h, v19.8h, v18.h[1], #270
 402eff4:	63000a72 	.inst	0x63000a72 ; undefined
 402eff8:	3065726f 	adr	x15, 40f9e45 <_end+0xca645>
 402effc:	61747320 	.inst	0x61747320 ; undefined
 402f000:	41207472 	.inst	0x41207472 ; undefined
 402f004:	2e204654 	ushl	v20.8b, v18.8b, v0.8b
 402f008:	000a2e2e 	.inst	0x000a2e2e ; undefined
 402f00c:	65726f63 	fnmls	z3.h, p3/m, z27.h, z18.h
 402f010:	74732030 	.inst	0x74732030 ; undefined
 402f014:	20747261 	.inst	0x20747261 ; undefined
 402f018:	20454554 	.inst	0x20454554 ; undefined
 402f01c:	67616d49 	.inst	0x67616d49 ; undefined
 402f020:	2e2e2065 	usubl	v5.8h, v3.8b, v14.8b
 402f024:	0a000a2e 	and	w14, w17, w0, lsl #2
 402f028:	7364706c 	.inst	0x7364706c ; undefined
 402f02c:	706d6320 	adr	x0, 4109c93 <_end+0xda493>
 402f030:	69616620 	ldpsw	x0, x25, [x17, #-248]
 402f034:	0a000a6c 	and	w12, w19, w0, lsl #2
 402f038:	7364706c 	.inst	0x7364706c ; undefined
 402f03c:	65657220 	fnmls	z0.h, p4/m, z17.h, z5.h
 402f040:	79656b5f 	ldrh	wzr, [x26, #4788]
 402f044:	6572615f 	fnmls	z31.h, p0/m, z10.h, z18.h
 402f048:	4b4f2061 	sub	w1, w3, w15, lsr #8
 402f04c:	650a0d00 	.inst	0x650a0d00 ; undefined
 402f050:	3a307272 	.inst	0x3a307272 ; undefined
 402f054:	2078616d 	.inst	0x2078616d ; undefined
 402f058:	206d756e 	.inst	0x206d756e ; undefined
 402f05c:	0d007830 	st3	{v16.h-v18.h}[3], [x1]
 402f060:	7272650a 	.inst	0x7272650a ; undefined
 402f064:	6e693a30 	.inst	0x6e693a30 ; undefined
 402f068:	696c6176 	ldpsw	x22, x24, [x11, #-160]
 402f06c:	61702064 	.inst	0x61702064 ; undefined
 402f070:	206d6172 	.inst	0x206d6172 ; undefined
 402f074:	6c626174 	ldnp	d20, d24, [x11, #-480]
 402f078:	6d692065 	ldp	d5, d8, [x3, #-368]
 402f07c:	20656761 	.inst	0x20656761 ; undefined
 402f080:	7366666f 	.inst	0x7366666f ; undefined
 402f084:	30207465 	adr	x5, 406ff11 <_end+0x40711>
 402f088:	0a0d0078 	and	w24, w3, w13
 402f08c:	31727265 	adds	w5, w19, #0xc9c, lsl #12
 402f090:	78616d3a 	.inst	0x78616d3a ; undefined
 402f094:	6d756e20 	ldp	d0, d27, [x17, #-176]
 402f098:	00783020 	.inst	0x00783020 ; undefined
 402f09c:	7272650a 	.inst	0x7272650a ; undefined
 402f0a0:	616d3a32 	.inst	0x616d3a32 ; undefined
 402f0a4:	656c2078 	fmls	z24.h, p0/m, z3.h, z12.h
 402f0a8:	2068746e 	.inst	0x2068746e ; undefined
 402f0ac:	0a007830 	and	w16, w1, w0, lsl #30
 402f0b0:	34727265 	cbz	w5, 4113efc <_end+0xe46fc>
 402f0b4:	6f6e203a 	umlal2	v26.4s, v1.8h, v14.h[2]
 402f0b8:	31352074 	adds	w20, w3, #0xd48
 402f0bc:	74796232 	.inst	0x74796232 ; undefined
 402f0c0:	6c612065 	ldnp	d5, d8, [x3, #-496]
 402f0c4:	216e6769 	.inst	0x216e6769 ; undefined
 402f0c8:	706c0a00 	adr	x0, 410720b <_end+0xd7a0b>
 402f0cc:	70207364 	adr	x4, 406ff3b <_end+0x4073b>
 402f0d0:	6d617261 	ldp	d1, d28, [x19, #-496]
 402f0d4:	666e695f 	.inst	0x666e695f ; undefined
 402f0d8:	4b4f206f 	sub	w15, w3, w15, lsr #8
 402f0dc:	706c0a00 	adr	x0, 410721f <_end+0xd7a1f>
 402f0e0:	75207364 	.inst	0x75207364 ; undefined
 402f0e4:	746f6f62 	.inst	0x746f6f62 ; undefined
 402f0e8:	666e695f 	.inst	0x666e695f ; undefined
 402f0ec:	4b4f206f 	sub	w15, w3, w15, lsr #8
 402f0f0:	65685400 	fnmla	z0.h, p5/m, z0.h, z8.h
 402f0f4:	62617420 	.inst	0x62617420 ; undefined
 402f0f8:	6920656c 	stgp	x12, x25, [x11, #-1024]
 402f0fc:	6e692073 	usubl2	v19.4s, v3.8h, v9.8h
 402f100:	72726f63 	.inst	0x72726f63 ; undefined
 402f104:	2e746365 	rsubhn	v5.4h, v27.4s, v20.4s
 402f108:	440a000a 	.inst	0x440a000a ; undefined
 402f10c:	50205244 	adr	x4, 406fb56 <_end+0x40356>
 402f110:	4c204c4c 	.inst	0x4c204c4c ; undefined
 402f114:	204b434f 	.inst	0x204b434f ; undefined
 402f118:	6c696146 	ldnp	d6, d24, [x10, #-368]
 402f11c:	Address 0x000000000402f11c is out of bounds.


Disassembly of section .eh_frame:

000000000402f120 <.eh_frame>:
 402f120:	00000010 	udf	#16
 402f124:	00000000 	udf	#0
 402f128:	00527a03 	.inst	0x00527a03 ; undefined
 402f12c:	011e7801 	.inst	0x011e7801 ; undefined
 402f130:	001f0c1b 	.inst	0x001f0c1b ; undefined
 402f134:	0000002c 	udf	#44
 402f138:	00000018 	udf	#24
 402f13c:	ffff4ef0 	.inst	0xffff4ef0 ; undefined
 402f140:	0000004c 	udf	#76
 402f144:	00040400 	.inst	0x00040400 ; undefined
 402f148:	300e0000 	adr	x0, 404b149 <_end+0x1b949>
 402f14c:	059e069d 	mov	z29.s, p14/z, #52
 402f150:	00000c04 	udf	#3076
 402f154:	04049300 	asrd	z0.h, p4/m, z0.h, #8
 402f158:	00000038 	udf	#56
 402f15c:	0ed3ddde 	.inst	0x0ed3ddde ; undefined
 402f160:	00000000 	udf	#0
 402f164:	00000024 	udf	#36
 402f168:	00000048 	udf	#72
 402f16c:	ffff5000 	.inst	0xffff5000 ; undefined
 402f170:	0000003c 	udf	#60
 402f174:	00040400 	.inst	0x00040400 ; undefined
 402f178:	100e0000 	adr	x0, 404b178 <_end+0x1b978>
 402f17c:	019e029d 	.inst	0x019e029d ; undefined
 402f180:	00003404 	udf	#13316
 402f184:	0eddde00 	.inst	0x0eddde00 ; undefined
 402f188:	00000000 	udf	#0

Disassembly of section .data:

000000000402f190 <g_svb_19d_battery>:
 402f190:	0002ab98 	.inst	0x0002ab98 ; undefined
 402f194:	087cda00 	.inst	0x087cda00 ; undefined
 402f198:	0000034e 	udf	#846
 402f19c:	000003d4 	udf	#980
 402f1a0:	00000000 	udf	#0
 402f1a4:	0002c251 	.inst	0x0002c251 ; undefined
 402f1a8:	08719bb5 	.inst	0x08719bb5 ; undefined
 402f1ac:	00000346 	udf	#838
 402f1b0:	0000040d 	udf	#1037
 402f1b4:	00000000 	udf	#0
 402f1b8:	00031045 	.inst	0x00031045 ; undefined
 402f1bc:	0938626f 	.inst	0x0938626f ; undefined
 402f1c0:	00000374 	udf	#884
 402f1c4:	00000405 	udf	#1029
 402f1c8:	00000000 	udf	#0

000000000402f1cc <g_svb_pt_battery>:
 402f1cc:	0002560c 	.inst	0x0002560c ; undefined
 402f1d0:	07f189f0 	.inst	0x07f189f0 ; undefined
 402f1d4:	00000343 	udf	#835
 402f1d8:	000003b6 	udf	#950
 402f1dc:	00000000 	udf	#0
 402f1e0:	00026e44 	.inst	0x00026e44 ; undefined
 402f1e4:	07d53059 	.inst	0x07d53059 ; undefined
 402f1e8:	0000032a 	udf	#810
 402f1ec:	00000403 	udf	#1027
 402f1f0:	00000000 	udf	#0
 402f1f4:	0002fa26 	.inst	0x0002fa26 ; undefined
 402f1f8:	088012c0 	stllrb	w0, [x22]
 402f1fc:	0000032a 	udf	#810
 402f200:	00000419 	udf	#1049
 402f204:	0000032a 	udf	#810

000000000402f208 <g_svb_16d_battery>:
 402f208:	0002560c 	.inst	0x0002560c ; undefined
 402f20c:	07f189f0 	.inst	0x07f189f0 ; undefined
 402f210:	00000343 	udf	#835
 402f214:	000003b6 	udf	#950
 402f218:	00000000 	udf	#0
 402f21c:	00026e44 	.inst	0x00026e44 ; undefined
 402f220:	07dcd179 	.inst	0x07dcd179 ; undefined
 402f224:	00000329 	udf	#809
 402f228:	00000408 	udf	#1032
 402f22c:	00000000 	udf	#0
 402f230:	0002fa26 	.inst	0x0002fa26 ; undefined
 402f234:	08bd1bc0 	.inst	0x08bd1bc0 ; undefined
 402f238:	00000336 	udf	#822
 402f23c:	00000403 	udf	#1027
 402f240:	00000000 	udf	#0

000000000402f244 <g_svb_19d>:
 402f244:	0002ab98 	.inst	0x0002ab98 ; undefined
 402f248:	089b5e80 	stllrb	w0, [x20]
 402f24c:	00000362 	udf	#866
 402f250:	000003e8 	udf	#1000
 402f254:	00000000 	udf	#0
 402f258:	0002c251 	.inst	0x0002c251 ; undefined
 402f25c:	08902035 	stllrb	w21, [x1]
 402f260:	0000035a 	udf	#858
 402f264:	00000419 	udf	#1049
 402f268:	00000000 	udf	#0
 402f26c:	00031045 	.inst	0x00031045 ; undefined
 402f270:	0956e6ef 	.inst	0x0956e6ef ; undefined
 402f274:	00000388 	udf	#904
 402f278:	00000419 	udf	#1049
 402f27c:	00000000 	udf	#0

000000000402f280 <g_svb_pt>:
 402f280:	0002560c 	.inst	0x0002560c ; undefined
 402f284:	08100e70 	stxrb	w16, w16, [x19]
 402f288:	0000034d 	udf	#845
 402f28c:	000003ca 	udf	#970
 402f290:	00000000 	udf	#0
 402f294:	00026e44 	.inst	0x00026e44 ; undefined
 402f298:	07f3b4d9 	.inst	0x07f3b4d9 ; undefined
 402f29c:	0000032a 	udf	#810
 402f2a0:	00000419 	udf	#1049
 402f2a4:	00000000 	udf	#0
 402f2a8:	0002fa26 	.inst	0x0002fa26 ; undefined
 402f2ac:	08add980 	.inst	0x08add980 ; undefined
 402f2b0:	0000032a 	udf	#810
 402f2b4:	00000419 	udf	#1049
 402f2b8:	0000033e 	udf	#830

000000000402f2bc <g_svb_16d>:
 402f2bc:	0002560c 	.inst	0x0002560c ; undefined
 402f2c0:	08100e70 	stxrb	w16, w16, [x19]
 402f2c4:	00000357 	udf	#855
 402f2c8:	000003ca 	udf	#970
 402f2cc:	00000000 	udf	#0
 402f2d0:	00026e44 	.inst	0x00026e44 ; undefined
 402f2d4:	07fb55f9 	.inst	0x07fb55f9 ; undefined
 402f2d8:	0000033d 	udf	#829
 402f2dc:	00000419 	udf	#1049
 402f2e0:	00000000 	udf	#0
 402f2e4:	0002fa26 	.inst	0x0002fa26 ; undefined
 402f2e8:	08dba040 	.inst	0x08dba040 ; undefined
 402f2ec:	0000034a 	udf	#842
 402f2f0:	00000417 	udf	#1047
 402f2f4:	00000000 	udf	#0

000000000402f2f8 <chsel_remap_reg_val_phy1_lpddr4>:
 402f2f8:	11140100 	add	w0, w8, #0x500
 402f2fc:	00001700 	udf	#5888
 402f300:	11140104 	add	w4, w8, #0x500
 402f304:	70050052 	adr	x18, 403930f <_end+0x9b0f>
 402f308:	11140804 	add	w4, w0, #0x502
 402f30c:	01ff0040 	.inst	0x01ff0040 ; undefined
 402f310:	11140808 	add	w8, w0, #0x502
 402f314:	00000000 	udf	#0
 402f318:	11140110 	add	w16, w8, #0x500
 402f31c:	00000000 	udf	#0

000000000402f320 <chsel_remap_reg_val_phy0_lpddr4>:
 402f320:	11140100 	add	w0, w8, #0x500
 402f324:	00001700 	udf	#5888
 402f328:	11140104 	add	w4, w8, #0x500
 402f32c:	70050050 	adr	x16, 4039337 <_end+0x9b37>
 402f330:	11140804 	add	w4, w0, #0x502
 402f334:	01ff0040 	.inst	0x01ff0040 ; undefined
 402f338:	11140808 	add	w8, w0, #0x502
 402f33c:	00000000 	udf	#0
 402f340:	11140110 	add	w16, w8, #0x500
 402f344:	00000000 	udf	#0

000000000402f348 <chsel_remap_reg_val_phy0_nonlpddr4>:
 402f348:	11140100 	add	w0, w8, #0x500
 402f34c:	00001700 	udf	#5888
 402f350:	11140104 	add	w4, w8, #0x500
 402f354:	70050088 	adr	x8, 4039367 <_end+0x9b67>
 402f358:	11140804 	add	w4, w0, #0x502
 402f35c:	01ff0040 	.inst	0x01ff0040 ; undefined
 402f360:	11140808 	add	w8, w0, #0x502
 402f364:	00000000 	udf	#0
 402f368:	11140110 	add	w16, w8, #0x500
 402f36c:	00000000 	udf	#0

000000000402f370 <chsel_remap_reg_val_phy1_nonlpddr4>:
 402f370:	11140100 	add	w0, w8, #0x500
 402f374:	00001700 	udf	#5888
 402f378:	11140104 	add	w4, w8, #0x500
 402f37c:	70050089 	adr	x9, 403938f <_end+0x9b8f>
 402f380:	11140804 	add	w4, w0, #0x502
 402f384:	01ff0040 	.inst	0x01ff0040 ; undefined
 402f388:	11140808 	add	w8, w0, #0x502
 402f38c:	00000000 	udf	#0
 402f390:	11140110 	add	w16, w8, #0x500
 402f394:	00000000 	udf	#0

000000000402f398 <ddr4_2g_table>:
 402f398:	11140100 	add	w0, w8, #0x500
 402f39c:	00001700 	udf	#5888
 402f3a0:	11140104 	add	w4, w8, #0x500
 402f3a4:	40050088 	.inst	0x40050088 ; undefined
 402f3a8:	11140804 	add	w4, w0, #0x502
 402f3ac:	00ff0040 	.inst	0x00ff0040 ; undefined
 402f3b0:	11140808 	add	w8, w0, #0x502
 402f3b4:	00000000 	udf	#0

000000000402f3b8 <ddr4_1p5g_table>:
 402f3b8:	11140100 	add	w0, w8, #0x500
 402f3bc:	00001700 	udf	#5888
 402f3c0:	11140104 	add	w4, w8, #0x500
 402f3c4:	30050088 	adr	x8, 40393d5 <_end+0x9bd5>
 402f3c8:	11140804 	add	w4, w0, #0x502
 402f3cc:	003f0040 	.inst	0x003f0040 ; NYI
 402f3d0:	11140808 	add	w8, w0, #0x502
 402f3d4:	00000000 	udf	#0
 402f3d8:	11140110 	add	w16, w8, #0x500
 402f3dc:	00001700 	udf	#5888
 402f3e0:	11140114 	add	w20, w8, #0x500
 402f3e4:	30040088 	adr	x8, 40373f5 <_end+0x7bf5>
 402f3e8:	11140814 	add	w20, w0, #0x502
 402f3ec:	001f0080 	.inst	0x001f0080 ; undefined
 402f3f0:	11140818 	add	w24, w0, #0x502
 402f3f4:	00000040 	udf	#64
 402f3f8:	11148600 	add	w0, w16, #0x521
 402f3fc:	0004480d 	.inst	0x0004480d ; undefined

000000000402f400 <ddr4_3g_table>:
 402f400:	11140100 	add	w0, w8, #0x500
 402f404:	00001700 	udf	#5888
 402f408:	11140104 	add	w4, w8, #0x500
 402f40c:	30050088 	adr	x8, 403941d <_end+0x9c1d>
 402f410:	11140804 	add	w4, w0, #0x502
 402f414:	007f0040 	.inst	0x007f0040 ; undefined
 402f418:	11140808 	add	w8, w0, #0x502
 402f41c:	00000000 	udf	#0
 402f420:	11140110 	add	w16, w8, #0x500
 402f424:	00001700 	udf	#5888
 402f428:	11140114 	add	w20, w8, #0x500
 402f42c:	30040088 	adr	x8, 403743d <_end+0x7c3d>
 402f430:	11140814 	add	w20, w0, #0x502
 402f434:	003f00c0 	.inst	0x003f00c0 ; NYI
 402f438:	11140818 	add	w24, w0, #0x502
 402f43c:	00000080 	udf	#128
 402f440:	11148600 	add	w0, w16, #0x521
 402f444:	0004480f 	.inst	0x0004480f ; undefined

000000000402f448 <ddr4_4g_table>:
 402f448:	11140100 	add	w0, w8, #0x500
 402f44c:	00001700 	udf	#5888
 402f450:	11140104 	add	w4, w8, #0x500
 402f454:	30050088 	adr	x8, 4039465 <_end+0x9c65>
 402f458:	11140804 	add	w4, w0, #0x502
 402f45c:	00ff0040 	.inst	0x00ff0040 ; undefined
 402f460:	11140808 	add	w8, w0, #0x502
 402f464:	00000000 	udf	#0

000000000402f468 <lpddr4_4g_1rank_table>:
 402f468:	11140100 	add	w0, w8, #0x500
 402f46c:	00001700 	udf	#5888
 402f470:	11140104 	add	w4, w8, #0x500
 402f474:	70050050 	adr	x16, 403947f <_end+0x9c7f>
 402f478:	11140804 	add	w4, w0, #0x502
 402f47c:	00ff0040 	.inst	0x00ff0040 ; undefined
 402f480:	11140808 	add	w8, w0, #0x502
 402f484:	00000000 	udf	#0
 402f488:	11148060 	add	w0, w3, #0x520
 402f48c:	00000162 	udf	#354
 402f490:	11149060 	add	w0, w3, #0x524
 402f494:	00000162 	udf	#354
 402f498:	11144634 	add	w20, w17, #0x511
 402f49c:	00000162 	udf	#354
 402f4a0:	11144644 	add	w4, w18, #0x511
 402f4a4:	00000162 	udf	#354

000000000402f4a8 <lpddr4_2g_2rank_table>:
 402f4a8:	11140100 	add	w0, w8, #0x500
 402f4ac:	00001700 	udf	#5888
 402f4b0:	11140104 	add	w4, w8, #0x500
 402f4b4:	70050050 	adr	x16, 40394bf <_end+0x9cbf>
 402f4b8:	11140804 	add	w4, w0, #0x502
 402f4bc:	00ff0040 	.inst	0x00ff0040 ; undefined
 402f4c0:	11140808 	add	w8, w0, #0x502
 402f4c4:	00000000 	udf	#0
 402f4c8:	11148060 	add	w0, w3, #0x520
 402f4cc:	00000142 	udf	#322
 402f4d0:	11149060 	add	w0, w3, #0x524
 402f4d4:	00000142 	udf	#322
 402f4d8:	11148064 	add	w4, w3, #0x520
 402f4dc:	00000142 	udf	#322
 402f4e0:	11149064 	add	w4, w3, #0x524
 402f4e4:	00000142 	udf	#322
 402f4e8:	11144634 	add	w20, w17, #0x511
 402f4ec:	00000142 	udf	#322
 402f4f0:	11144644 	add	w4, w18, #0x511
 402f4f4:	00000142 	udf	#322
 402f4f8:	11144638 	add	w24, w17, #0x511
 402f4fc:	00000142 	udf	#322
 402f500:	11144648 	add	w8, w18, #0x511
 402f504:	00000142 	udf	#322

000000000402f508 <lpddr4_4g_2rank_table>:
 402f508:	11140100 	add	w0, w8, #0x500
 402f50c:	00001700 	udf	#5888
 402f510:	11140104 	add	w4, w8, #0x500
 402f514:	70050050 	adr	x16, 403951f <_end+0x9d1f>
 402f518:	11140804 	add	w4, w0, #0x502
 402f51c:	00ff0040 	.inst	0x00ff0040 ; undefined
 402f520:	11140808 	add	w8, w0, #0x502
 402f524:	00000000 	udf	#0
 402f528:	11148060 	add	w0, w3, #0x520
 402f52c:	00000152 	udf	#338
 402f530:	11149060 	add	w0, w3, #0x524
 402f534:	00000152 	udf	#338
 402f538:	11148064 	add	w4, w3, #0x520
 402f53c:	00000152 	udf	#338
 402f540:	11149064 	add	w4, w3, #0x524
 402f544:	00000152 	udf	#338
 402f548:	11144634 	add	w20, w17, #0x511
 402f54c:	00000152 	udf	#338
 402f550:	11144644 	add	w4, w18, #0x511
 402f554:	00000152 	udf	#338
 402f558:	11144638 	add	w24, w17, #0x511
 402f55c:	00000152 	udf	#338
 402f560:	11144648 	add	w8, w18, #0x511
 402f564:	00000152 	udf	#338

Disassembly of section .bss:

0000000004021a80 <hash_verify_buf>:
	...

0000000004021aa0 <__stack_chk_guard>:
	...

0000000004021aa8 <g_backup_params>:
	...

0000000004021ab8 <jump_addr>:
	...

0000000004021ac0 <malloc_sizes>:
	...

Disassembly of section .comment:

0000000000000000 <.comment>:
   0:	3a434347 	ccmn	w26, w3, #0x7, mi  // mi = first
   4:	43472820 	.inst	0x43472820 ; undefined
   8:	30312d43 	adr	x3, 625b1 <hash_verify_buf-0x3fbf4cf>
   c:	302e332e 	adr	x14, 5c671 <hash_verify_buf-0x3fc540f>
  10:	696c6720 	ldpsw	x0, x25, [x25, #-160]
  14:	322d6362 	orr	w2, w27, #0xfff80fff
  18:	2034332e 	.inst	0x2034332e ; undefined
  1c:	31726576 	adds	w22, w11, #0xc99, lsl #12
  20:	312e3030 	adds	w16, w1, #0xb8c
  24:	32203034 	orr	w20, w1, #0x1fff
  28:	2d333230 	stp	s16, s12, [x17, #-104]
  2c:	322d3131 	orr	w17, w9, #0xfff80000
  30:	32312033 	orr	w19, w1, #0xff8000
  34:	3a30303a 	.inst	0x3a30303a ; undefined
  38:	20293030 	.inst	0x20293030 ; undefined
  3c:	332e3031 	.inst	0x332e3031 ; undefined
  40:	Address 0x0000000000000040 is out of bounds.


Disassembly of section .debug_frame:

0000000000000000 <.debug_frame>:
   0:	0000000c 	udf	#12
   4:	ffffffff 	.inst	0xffffffff ; undefined
   8:	78040001 	sturh	w1, [x0, #64]
   c:	001f0c1e 	.inst	0x001f0c1e ; undefined
  10:	00000014 	udf	#20
  14:	00000000 	udf	#0
  18:	0402e800 	msb	z0.b, p2/m, z2.b, z0.b
	...
  28:	00000014 	udf	#20
  2c:	00000000 	udf	#0
  30:	0402e880 	msb	z0.b, p2/m, z2.b, z4.b
	...
  40:	00000014 	udf	#20
  44:	00000000 	udf	#0
  48:	0402e900 	msb	z0.b, p2/m, z2.b, z8.b
	...
  58:	00000014 	udf	#20
  5c:	00000000 	udf	#0
  60:	0402e980 	msb	z0.b, p2/m, z2.b, z12.b
	...
  70:	00000014 	udf	#20
  74:	00000000 	udf	#0
  78:	0402ea00 	msb	z0.b, p2/m, z2.b, z16.b
	...
  88:	00000014 	udf	#20
  8c:	00000000 	udf	#0
  90:	0402ea80 	msb	z0.b, p2/m, z2.b, z20.b
	...
  a0:	00000014 	udf	#20
  a4:	00000000 	udf	#0
  a8:	0402eb00 	msb	z0.b, p2/m, z2.b, z24.b
	...
  b8:	00000014 	udf	#20
  bc:	00000000 	udf	#0
  c0:	0402eb80 	msb	z0.b, p2/m, z2.b, z28.b
	...
  d0:	00000014 	udf	#20
  d4:	00000000 	udf	#0
  d8:	0402ec00 	msb	z0.b, p3/m, z2.b, z0.b
  dc:	00000000 	udf	#0
  e0:	00000004 	udf	#4
  e4:	00000000 	udf	#0
