/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "svb.h"
#include "share_drivers.h"
#include "common.h"

static svb_cfg g_svb_pt = {
	.core_curve = {CORE_CURVE_A19_PT, CORE_CURVE_B19_PT, CORE_CURVE_VOLT_MIN19_PT, CORE_CURVE_VOLT_MAX19_PT, 0},
	.cpu_curve = {CPU_CURVE_A19_PT, CPU_CURVE_B19_PT, CPU_CURVE_VOLT_MIN19_PT, CPU_CURVE_VOLT_MAX19_PT, 0},
	.npu_curve = {NPU_CURVE_A19_PT, NPU_CURVE_B19_PT, NPU_CURVE_VOLT_MIN19_PT, NPU_CURVE_VOLT_MAX19_PT, \
					NPU_FIX_VOLT_VAL_PT},
};

static svb_cfg g_svb_19d = {
	.core_curve = {CORE_CURVE_A19, CORE_CURVE_B19, CORE_CURVE_VOLT_MIN19, CORE_CURVE_VOLT_MAX19, 0},
	.cpu_curve = {CPU_CURVE_A19, CPU_CURVE_B19, CPU_CURVE_VOLT_MIN19, CPU_CURVE_VOLT_MAX19, 0},
	.npu_curve = {NPU_CURVE_A19, NPU_CURVE_B19, NPU_CURVE_VOLT_MIN19, NPU_CURVE_VOLT_MAX19, 0},
};

static svb_cfg g_svb_16d = {
	.core_curve = {CORE_CURVE_A16, CORE_CURVE_B16, CORE_CURVE_VOLT_MIN16, CORE_CURVE_VOLT_MAX16, 0},
	.cpu_curve = {CPU_CURVE_A16, CPU_CURVE_B16, CPU_CURVE_VOLT_MIN16, CPU_CURVE_VOLT_MAX16, 0},
	.npu_curve = {NPU_CURVE_A16, NPU_CURVE_B16, NPU_CURVE_VOLT_MIN16, NPU_CURVE_VOLT_MAX16, 0},
};

static svb_cfg g_svb_pt_battery = {
	.core_curve = {CORE_CURVE_A19_PT_BATTERY, CORE_CURVE_B19_PT_BATTERY, CORE_CURVE_VOLT_MIN19_PT_BATTERY, \
					CORE_CURVE_VOLT_MAX19_PT_BATTERY, 0},
	.cpu_curve = {CPU_CURVE_A19_PT_BATTERY, CPU_CURVE_B19_PT_BATTERY, CPU_CURVE_VOLT_MIN19_PT_BATTERY, \
					CPU_CURVE_VOLT_MAX19_PT_BATTERY, 0},
	.npu_curve = {NPU_CURVE_A19_PT_BATTERY, NPU_CURVE_B19_PT_BATTERY, NPU_CURVE_VOLT_MIN19_PT_BATTERY, \
					NPU_CURVE_VOLT_MAX19_PT_BATTERY, NPU_FIX_VOLT_VAL_PT_BATTERY},
};

static svb_cfg g_svb_19d_battery = {
	.core_curve = {CORE_CURVE_A19_BATTERY, CORE_CURVE_B19_BATTERY, CORE_CURVE_VOLT_MIN19_BATTERY, \
					CORE_CURVE_VOLT_MAX19_BATTERY, 0},
	.cpu_curve = {CPU_CURVE_A19_BATTERY, CPU_CURVE_B19_BATTERY, CPU_CURVE_VOLT_MIN19_BATTERY, \
					CPU_CURVE_VOLT_MAX19_BATTERY, 0},
	.npu_curve = {NPU_CURVE_A19_BATTERY, NPU_CURVE_B19_BATTERY, NPU_CURVE_VOLT_MIN19_BATTERY, \
					NPU_CURVE_VOLT_MAX19_BATTERY, 0},
};

static svb_cfg g_svb_16d_battery = {
	.core_curve = {CORE_CURVE_A16_BATTERY, CORE_CURVE_B16_BATTERY, CORE_CURVE_VOLT_MIN16_BATTERY, \
					CORE_CURVE_VOLT_MAX16_BATTERY, 0},
	.cpu_curve = {CPU_CURVE_A16_BATTERY, CPU_CURVE_B16_BATTERY, CPU_CURVE_VOLT_MIN16_BATTERY, \
					CPU_CURVE_VOLT_MAX16_BATTERY, 0},
	.npu_curve = {NPU_CURVE_A16_BATTERY, NPU_CURVE_B16_BATTERY, NPU_CURVE_VOLT_MIN16_BATTERY, \
					NPU_CURVE_VOLT_MAX16_BATTERY, 0},
};

static inline void dwb(void) /* drain write buffer */
{
}

static inline unsigned int readl(unsigned addr)
{
	unsigned int val;
	val = (*(volatile unsigned int *)(long)(addr));
	return val;
}

static inline void writel(unsigned val, unsigned addr)
{
	dwb();
	(*(volatile unsigned *)(long)(addr)) = (val);
	dwb();
}

static void check_volt_val(unsigned int *volt_val, unsigned int curve_volt_max, unsigned int curve_volt_min)
{
	if (*volt_val > curve_volt_max) {
		*volt_val = curve_volt_max;
	} else if (*volt_val < curve_volt_min) {
		*volt_val = curve_volt_min;
	}
}

static inline int temperature_formula(int val)
{
	return (((((val) - TEMPERATURE_A) * TEMPERATURE_B) / TEMPERATURE_C) - TEMPERATURE_D);
}

static inline unsigned int duty_formula(unsigned int val, \
							unsigned int max, unsigned int min)
{
	return ((((max) - (val)) * PWM_PERIOD + ((max) - (min) + DUTY_ONE) / DUTY_TWO) / ((max) - (min)) - DUTY_ONE);
}

static inline int hpm_delta_formula(int val, svb_coef svb_coef_val)
{
	return ((((val) * svb_coef_val.coef_k) - svb_coef_val.coef_b) / DELTA_FORMULA_MULTIPLES);
}

static inline unsigned int core_max(unsigned int a, unsigned int b)
{
	return (((a) > (b)) ? (a) : (b));
}

static void svb_pwm_cfg(unsigned int reg_val, unsigned int addr)
{
	u_svb_pwm_ctrl svb_pwm_ctrl;
	svb_pwm_ctrl.bits.svb_pwm_enable = 1;
	svb_pwm_ctrl.bits.svb_pwm_inv = 0;
	svb_pwm_ctrl.bits.svb_pwm_load = 1;
	svb_pwm_ctrl.bits.svb_pwm_period = (PWM_PERIOD - 1);
	svb_pwm_ctrl.bits.svb_pwm_duty = reg_val & 0x3ff;
	writel(svb_pwm_ctrl.u32, addr);
}

static int core_hpm_temperature_calibration(int in_hpm_core_rvt40, int temperature)
{
	int core_hpm_delta = 0;
	svb_coef core_svb_coef[TEMPERATURE_ZONE_BUTT] = {
		{1395, 306780},
		{919, 199140},
		{550, 120440},
		{-478, -96323}
	};

	if (in_hpm_core_rvt40 >= CORE_HPM_TEM_COM_BOUNDED) {
		if ((temperature >= TEMPERATURE_110))
			core_hpm_delta = hpm_delta_formula(in_hpm_core_rvt40, core_svb_coef[TEMPERATURE_ZONE_110]);
		else if ((temperature >= TEMPERATURE_90))
			core_hpm_delta = hpm_delta_formula(in_hpm_core_rvt40, core_svb_coef[TEMPERATURE_ZONE_90]);
		else if ((temperature >= TEMPERATURE_70))
			core_hpm_delta = hpm_delta_formula(in_hpm_core_rvt40, core_svb_coef[TEMPERATURE_ZONE_70]);
		else if (temperature >= TEMPERATURE_0)
			core_hpm_delta = 0;
		else
			core_hpm_delta = hpm_delta_formula(in_hpm_core_rvt40, core_svb_coef[TEMPERATURE_ZONE_NEG20]);
	} else {
		core_hpm_delta = 0;
	}

	return core_hpm_delta;
}

static int cpu_hpm_temperature_calibration(int in_hpm_cpu_hvt35, int temperature)
{
	int cpu_hpm_delta = 0;
	svb_coef cpu_svb_coef[TEMPERATURE_ZONE_BUTT] = {
		{1200, 274070},
		{819, 192110},
		{422, 99531},
		{-445, -114190}
	};

	if (in_hpm_cpu_hvt35 >= CPU_HPM_TEM_COM_BOUNDED) {
		if ((temperature >= TEMPERATURE_110))
			cpu_hpm_delta = hpm_delta_formula(in_hpm_cpu_hvt35, cpu_svb_coef[TEMPERATURE_ZONE_110]);
		else if ((temperature >= TEMPERATURE_90))
			cpu_hpm_delta = hpm_delta_formula(in_hpm_cpu_hvt35, cpu_svb_coef[TEMPERATURE_ZONE_90]);
		else if ((temperature >= TEMPERATURE_70))
			cpu_hpm_delta = hpm_delta_formula(in_hpm_cpu_hvt35, cpu_svb_coef[TEMPERATURE_ZONE_70]);
		else if (temperature >= TEMPERATURE_0)
			cpu_hpm_delta = 0;
		else
			cpu_hpm_delta = hpm_delta_formula(in_hpm_cpu_hvt35, cpu_svb_coef[TEMPERATURE_ZONE_NEG20]);
	} else {
		cpu_hpm_delta = 0;
	}

	return cpu_hpm_delta;
}

static int npu_hpm_temperature_calibration(int in_hpm_npu_rvt40, int temperature)
{
	int npu_hpm_delta = 0;
	svb_coef npu_svb_coef[TEMPERATURE_ZONE_BUTT] = {
		{1148, 245410},
		{851, 185150},
		{517, 112190},
		{-498, -104050}
	};

	if (in_hpm_npu_rvt40 >= NPU_HPM_TEM_COM_BOUNDED) {
		if ((temperature >= TEMPERATURE_110))
			npu_hpm_delta = hpm_delta_formula(in_hpm_npu_rvt40, npu_svb_coef[TEMPERATURE_ZONE_110]);
		else if ((temperature >= TEMPERATURE_90))
			npu_hpm_delta = hpm_delta_formula(in_hpm_npu_rvt40, npu_svb_coef[TEMPERATURE_ZONE_90]);
		else if ((temperature >= TEMPERATURE_70))
			npu_hpm_delta = hpm_delta_formula(in_hpm_npu_rvt40, npu_svb_coef[TEMPERATURE_ZONE_70]);
		else if (temperature >= TEMPERATURE_0)
			npu_hpm_delta = 0;
		else
			npu_hpm_delta = hpm_delta_formula(in_hpm_npu_rvt40, npu_svb_coef[TEMPERATURE_ZONE_NEG20]);
	} else {
		npu_hpm_delta = 0;
	}

	return npu_hpm_delta;
}


static void adjust_hpm(unsigned int *hpm_core_rvt40, unsigned int *hpm_cpu_hvt35,
					   unsigned int *hpm_npu_rvt40, int temperature, int *use_board_hpm)
{
	unsigned int otp_hpm_core_rvt40 = 0;
	unsigned int otp_hpm_cpu_hvt35  = 0;
	unsigned int otp_hpm_npu_rvt40  = 0;
	unsigned int hpm_storage = 0;
	int in_hpm_core_rvt40 = 0;
	int in_hpm_cpu_hvt35 = 0;
	int in_hpm_npu_rvt40 = 0;
	int core_hpm_delta = 0;
	int cpu_hpm_delta  = 0;
	int npu_hpm_delta  = 0;

	hpm_storage = readl(OTP_BASE_REG + OTP_HPM_OFFSET);
	otp_hpm_core_rvt40 = (hpm_storage & 0x3ff);
	otp_hpm_cpu_hvt35 = (hpm_storage >> OTP_CPU_OFFSETS) & 0x3ff;
	otp_hpm_npu_rvt40 = (hpm_storage >> OTP_NPU_OFFSETS) & 0x3ff;
	in_hpm_core_rvt40 = (int)(*hpm_core_rvt40);
	in_hpm_cpu_hvt35  = (int)(*hpm_cpu_hvt35);
	in_hpm_npu_rvt40  = (int)(*hpm_npu_rvt40);
	if (otp_hpm_core_rvt40 && otp_hpm_cpu_hvt35 && otp_hpm_npu_rvt40) {
		*hpm_core_rvt40 = otp_hpm_core_rvt40;
		*hpm_cpu_hvt35 = otp_hpm_cpu_hvt35;
		*hpm_npu_rvt40 = otp_hpm_npu_rvt40;
	} else {
		core_hpm_delta = core_hpm_temperature_calibration(in_hpm_core_rvt40, temperature);
		cpu_hpm_delta = cpu_hpm_temperature_calibration(in_hpm_cpu_hvt35, temperature);
		npu_hpm_delta = npu_hpm_temperature_calibration(in_hpm_npu_rvt40, temperature);
		*use_board_hpm = 1;
		in_hpm_core_rvt40 += core_hpm_delta;
		*hpm_core_rvt40 = (unsigned int)in_hpm_core_rvt40;
		in_hpm_cpu_hvt35 += cpu_hpm_delta;
		*hpm_cpu_hvt35 = (unsigned int)in_hpm_cpu_hvt35;
		in_hpm_npu_rvt40 += npu_hpm_delta;
		*hpm_npu_rvt40 = (unsigned int)in_hpm_npu_rvt40;
	}
}

static void init_temperature(void)
{
	writel(TSENSOR_CTRL0_CFG0, UBOOT_REG_TSENSOR_CTRL + TSENSOR0_CTRL0);
	writel(TSENSOR_CTRL1_CFG0, UBOOT_REG_TSENSOR_CTRL + TSENSOR0_CTRL1);
}

static void get_temperature(int *temperature)
{
	volatile int tmp = 0;

	tmp = readl(UBOOT_REG_TSENSOR_CTRL + TSENSOR_STATUS0);
	tmp = tmp & 0x3ff;
	*temperature = temperature_formula(tmp);
}

static void init_hpm(void)
{
	int i = 0;

	/* Enable the HPM clock. */
	writel(HPM_CLK_CFG0, HPM_NPU_RVT40_CLK_REG);
	writel(HPM_CLK_CFG0, HPM_CORE_RVT40_CLK_REG);
	writel(HPM_CLK_CFG0, HPM_CPU_HVT35_CLK_REG);

	/* Deassert reset */
	for (i = 0; i < HPM_NUM; i++) {
		/* CORE */
		writel(HPM_CTRL_VALUE, HPM_BASE_ADDR + CORE_HPM_CTRL0 + 0x10 * i);
		writel(HPM_LIMIT, HPM_BASE_ADDR + CORE_HPM_CTRL1 + 0x10 * i);
		/* CPU */
		writel(HPM_CTRL_VALUE, HPM_BASE_ADDR + CPU_HPM_CTRL0 + 0x10 * i);
		writel(HPM_LIMIT, HPM_BASE_ADDR + CPU_HPM_CTRL1 + 0x10 * i);
		/* NPU */
		writel(HPM_CTRL_VALUE, HPM_BASE_ADDR + NPU_HPM_CTRL0 + 0x10 * i);
		writel(HPM_LIMIT, HPM_BASE_ADDR + NPU_HPM_CTRL1 + 0x10 * i);
	}
}

static unsigned int  hpm_value_avg(unsigned int *val, unsigned int num)
{
	unsigned int i;
	volatile unsigned int tmp = 0;

	for (i = 0; i < num; i++) /* 4: Cycle */
		tmp += val[i];
	return ((tmp / CYCLE_NUM) / num);
}

static void start_hpm(unsigned int *hpm_core_rvt40, unsigned int *hpm_cpu_hvt35,
					  unsigned int *hpm_npu_rvt40)
{
	int i = 0;
	u_hpm_reg hpm_reg;
	unsigned int core_value[HPM_RECORD_BUTT] = {0, 0, 0, 0};
	unsigned int cpu_value[HPM_RECORD_BUTT] = {0, 0, 0, 0};
	unsigned int npu_value[HPM_RECORD_BUTT] = {0, 0, 0, 0};

	for (i = 0; i < CYCLE_NUM; i++) {
		/* core */
		hpm_reg.u32 = readl(SYSCTRL_REG + HPM_CORE_RVT40_REG0);
		core_value[HPM_RECORD1] += hpm_reg.bits.hpm1;
		core_value[HPM_RECORD0] += hpm_reg.bits.hpm0;
		hpm_reg.u32 = readl(SYSCTRL_REG + HPM_CORE_RVT40_REG1);
		core_value[HPM_RECORD3] += hpm_reg.bits.hpm1;
		core_value[HPM_RECORD2] += hpm_reg.bits.hpm0;
		/* cpu */
		hpm_reg.u32 = readl(SYSCTRL_REG + HPM_CPU_HVT35_REG0);
		cpu_value[HPM_RECORD1] += hpm_reg.bits.hpm1;
		cpu_value[HPM_RECORD0] += hpm_reg.bits.hpm0;
		hpm_reg.u32 = readl(SYSCTRL_REG + HPM_CPU_HVT35_REG1);
		cpu_value[HPM_RECORD3] += hpm_reg.bits.hpm1;
		cpu_value[HPM_RECORD2] += hpm_reg.bits.hpm0;
		/* npu */
		hpm_reg.u32 = readl(SYSCTRL_REG + HPM_NPU_RVT40_REG0);
		npu_value[HPM_RECORD1] += hpm_reg.bits.hpm1;
		npu_value[HPM_RECORD0] += hpm_reg.bits.hpm0;
		hpm_reg.u32 = readl(SYSCTRL_REG + HPM_NPU_RVT40_REG1);
		npu_value[HPM_RECORD3] += hpm_reg.bits.hpm1;
		npu_value[HPM_RECORD2] += hpm_reg.bits.hpm0;
	}

	*hpm_core_rvt40 = hpm_value_avg(core_value, HPM_RECORD_BUTT); /* 4 : Array size */
	*hpm_cpu_hvt35 = hpm_value_avg(cpu_value, HPM_RECORD_BUTT); /* 4 : Array size */
	*hpm_npu_rvt40 = hpm_value_avg(npu_value, HPM_RECORD_BUTT); /* 4 : Array size */
}

static void get_delta_v(int *core_delta_v, int *cpu_delta_v,  int *npu_delta_v)
{
	unsigned int value = 0;
	int flag = 0;
	volatile u_core_delta_v_reg core_delta_v_reg;
	volatile u_cpu_npu_delta_v_reg cpu_npu_delta_v_reg;

	/* core:bit 6-0,
	 * bit7 equal to 1 means negative, equal to 0 means positive,
	 * bit 6-0 is the  absolute delta_v
	 */
	core_delta_v_reg.u32 = readl(OTP_BASE_REG + OTP_DELTA_CORE_OFFSET);
	value = core_delta_v_reg.bits.core_delta_v;
	flag  = (core_delta_v_reg.bits.core_flag) ? -1 : 1;
	*core_delta_v = flag * value;

	cpu_npu_delta_v_reg.u32 = readl(OTP_BASE_REG + OTP_DELTA_CPU_NPU_OFFSET);
	value = cpu_npu_delta_v_reg.bits.cpu_delta_v;
	flag = (cpu_npu_delta_v_reg.bits.cpu_flag) ? -1 : 1;
	*cpu_delta_v = flag * value;

	value = cpu_npu_delta_v_reg.bits.npu_delta_v;
	flag = (cpu_npu_delta_v_reg.bits.npu_flag) ? -1 : 1;
	*npu_delta_v = flag * value;
}

static unsigned int calc_volt_regval(unsigned int volt_val, unsigned int volt_max,
									 unsigned int volt_min)
{
	volatile unsigned int duty;

	if ((volt_val >= volt_max))
		volt_val = volt_max - 1;
	if ((volt_val <= volt_min))
		volt_val = volt_min + 1;
	duty =  duty_formula(volt_val, volt_max, volt_min);

	return duty;
}

static void set_hpm_core_rvt40_volt(unsigned int hpm_core_rvt40_value, int delta_v, const svb_curve* core_curve)
{
	unsigned int volt_val = 0;
	unsigned int reg_val = 0;
	u_storage_reg core_storage_reg;

	/* Calculate the required voltage value */
	volt_val = (core_curve->curve_b - core_curve->curve_a * hpm_core_rvt40_value) / FORMULA_MULTIPLES;
	if (hpm_core_rvt40_value < CORE_HPM_BOUND) {
		volt_val = core_curve->volt_max;
	}
	check_volt_val(&volt_val, core_curve->volt_max, core_curve->volt_min);

	/* Saves the core voltage */
	volt_val = (unsigned int)((int)volt_val + delta_v);
	check_volt_val(&volt_val, CORE_SVB_MAX0, CORE_SVB_MIN0);
	core_storage_reg.bits.work_v = volt_val & 0xffff;
	core_storage_reg.bits.cacl_v = volt_val & 0xffff;
	writel(core_storage_reg.u32, SYSCTRL_BASE_REG + CORE_V_STORAGE_REG);

	/* Reduced by 40 mv training */
	volt_val = volt_val - CORE_TRAINING;
	if (volt_val < CORE_TRAINING_LOW_VOLT) {
			volt_val = CORE_TRAINING_LOW_VOLT;
	}

	/* Configure the voltage */
	reg_val = calc_volt_regval(volt_val, CORE_VOLT_MAX0, CORE_VOLT_MIN0);
	svb_pwm_cfg(reg_val, PWM_CORE_RVT40_VOL_REG);
}

static void set_hpm_cpu_hvt35_volt(unsigned int hpm_cpu_hvt35_value, int delta_v, const svb_curve* cpu_curve)
{
	unsigned int volt_val = 0;
	unsigned int reg_val = 0;
	u_storage_reg cpu_storage_reg;

	/* Calculate the required voltage value */
	volt_val = (cpu_curve->curve_b - cpu_curve->curve_a * hpm_cpu_hvt35_value) / FORMULA_MULTIPLES;
	check_volt_val(&volt_val, cpu_curve->volt_max, cpu_curve->volt_min);

	/* Saves the core voltage */
	volt_val = (unsigned int)((int)volt_val + delta_v);
	check_volt_val(&volt_val, CPU_SVB_MAX0, CPU_SVB_MIN0);
	cpu_storage_reg.bits.work_v = volt_val & 0xffff;
	cpu_storage_reg.bits.cacl_v = volt_val & 0xffff;
	writel(cpu_storage_reg.u32, SYSCTRL_BASE_REG + CPU_V_STORAGE_REG);

	/* Configure the voltage */
	reg_val = calc_volt_regval(volt_val, CPU_VOLT_MAX0, CPU_VOLT_MIN0);
	svb_pwm_cfg(reg_val, PWM_CPU_HVT35_VOL_REG);
}

static void set_hpm_npu_rvt40_volt(unsigned int hpm_npu_rvt40_value, int delta_v, const svb_curve* npu_curve)
{
	unsigned int volt_val = 0;
	unsigned int reg_val = 0;
	u_storage_reg npu_storage_reg;

	/* Calculate the required voltage value */
	if (npu_curve->volt_fix == 0) {
		volt_val = (npu_curve->curve_b - npu_curve->curve_a * hpm_npu_rvt40_value) / FORMULA_MULTIPLES;
		check_volt_val(&volt_val, npu_curve->volt_max, npu_curve->volt_min);
	} else {
		volt_val = npu_curve->volt_fix;
	}

	/* Saves the core voltage */
	volt_val = (unsigned int)((int)volt_val + delta_v);
	check_volt_val(&volt_val, NPU_SVB_MAX0, NPU_SVB_MIN0);
	npu_storage_reg.bits.work_v = volt_val & 0xffff;
	npu_storage_reg.bits.cacl_v = volt_val & 0xffff;
	writel(npu_storage_reg.u32, SYSCTRL_BASE_REG + NPU_V_STORAGE_REG);

	/* Configure the voltage */
	reg_val = calc_volt_regval(volt_val, NPU_VOLT_MAX0, NPU_VOLT_MIN0);
	svb_pwm_cfg(reg_val, PWM_NPU_RVT40_VOL_REG);
}

static void set_volt(unsigned int hpm_core_rvt40, unsigned int hpm_cpu_hvt35,
					 unsigned int hpm_npu_rvt40, const svb_cfg* svb)
{
	int  core_delta_v = 0;
	int  npu_delta_v = 0;
	int  cpu_delta_v = 0;

	/* Obtaining Voltage Compensation */
	get_delta_v(&core_delta_v, &cpu_delta_v, &npu_delta_v);

	/* Calculate and configure voltage */
	set_hpm_core_rvt40_volt(hpm_core_rvt40, core_delta_v, &svb->core_curve);
	set_hpm_cpu_hvt35_volt(hpm_cpu_hvt35, cpu_delta_v,  &svb->cpu_curve);
	set_hpm_npu_rvt40_volt(hpm_npu_rvt40, npu_delta_v, &svb->npu_curve);

	udelay(WAITING_20_US);
}

static void  set_voltage_val(unsigned int volt_val)
{
	unsigned int core_volt_val = 0;
	unsigned int cpu_volt_val = 0;
	unsigned int npu_volt_val = 0;

	core_volt_val = calc_volt_regval(volt_val, CORE_VOLT_MAX0, CORE_VOLT_MIN0);
	svb_pwm_cfg(core_volt_val, PWM_CORE_RVT40_VOL_REG);
	cpu_volt_val = calc_volt_regval(volt_val, CPU_VOLT_MAX0, CPU_VOLT_MIN0);
	svb_pwm_cfg(cpu_volt_val, PWM_CPU_HVT35_VOL_REG);
	npu_volt_val = calc_volt_regval(volt_val, NPU_VOLT_MAX0, NPU_VOLT_MIN0);
	svb_pwm_cfg(npu_volt_val, PWM_NPU_RVT40_VOL_REG);
}

static void  save_temperature_pasensor(int temperature, int cpu_pasensor, int npu_pasensor)
{
	unsigned int tsensor_abs_val = 0;
	u_temp_pa_storage_reg temp_pa_storage_reg;
	u_dyn_comp_falg_storage_reg dyn_comp_falg_storage_reg;

	dyn_comp_falg_storage_reg.u32 = readl(SYSCTRL_REG + DYN_COMP_FALG_STORAGE_REG);

	/* Save the current temperature.bit0-7 */
	if (temperature >= 0) {
		temp_pa_storage_reg.bits.temp_value = (unsigned int)temperature & 0xff;
		dyn_comp_falg_storage_reg.bits.start_up_temp = (unsigned int)temperature & 0xff;
	} else {
		tsensor_abs_val = temperature * NEG_1;
		temp_pa_storage_reg.bits.temp_negative = 1;
		temp_pa_storage_reg.bits.temp_value = ((unsigned int)tsensor_abs_val & 0xff);
		dyn_comp_falg_storage_reg.bits.temp_negative = 1;
		dyn_comp_falg_storage_reg.bits.start_up_temp = ((unsigned int)tsensor_abs_val & 0xff);
	}
	writel(dyn_comp_falg_storage_reg.u32, SYSCTRL_REG + DYN_COMP_FALG_STORAGE_REG);

	/* Obtaining the Temperature Range bit15-12 */
	if (temperature < TEMPERATURE_NEG20)
		temp_pa_storage_reg.bits.temp_range = TEMPERATURE_INTERVAL_0;
	else if (temperature < TEMPERATURE_0)
		temp_pa_storage_reg.bits.temp_range = TEMPERATURE_INTERVAL_1;
	else if (temperature < TEMPERATURE_35)
		temp_pa_storage_reg.bits.temp_range = TEMPERATURE_INTERVAL_2;
	else if (temperature < TEMPERATURE_75)
		temp_pa_storage_reg.bits.temp_range = TEMPERATURE_INTERVAL_3;
	else if (temperature < TEMPERATURE_115)
		temp_pa_storage_reg.bits.temp_range = TEMPERATURE_INTERVAL_4;
	else
		temp_pa_storage_reg.bits.temp_range = TEMPERATURE_INTERVAL_5;

	/* Obtaining the PASENSOR */
	if (cpu_pasensor < 0) {
		temp_pa_storage_reg.bits.cpu_pa_negative = PA_IS_NEGATIVE;
		cpu_pasensor = cpu_pasensor * NEG_1;
	}
	temp_pa_storage_reg.bits.cpu_pa_value = cpu_pasensor & 0x7f;
	if (npu_pasensor < 0) {
		temp_pa_storage_reg.bits.npu_pa_negative = PA_IS_NEGATIVE;
		npu_pasensor = npu_pasensor * NEG_1;
	}
	temp_pa_storage_reg.bits.npu_pa_value = npu_pasensor & 0x7f;
	writel(temp_pa_storage_reg.u32, SYSCTRL_REG + TEMP_PA_STORAGE_REG);
}

static void get_pasensor(int *cpu_pasensor, int *npu_pasensor)
{
	int cpu_pasensor_a_data = 0, cpu_pasensor_o_data = 0;
	int npu_pasensor_a_data = 0, npu_pasensor_o_data = 0;
	u_pasensor_out pasensor_out;

	pasensor_out.u32 = readl(SYSCTRL_BASE_REG + CPU_PASENSOR_OUT0);
	cpu_pasensor_o_data = pasensor_out.bits.pasensor_nbti_o_data & 0x1ff;
	cpu_pasensor_a_data = pasensor_out.bits.pasensor_nbti_a_data & 0x1ff;
	*cpu_pasensor = cpu_pasensor_o_data - cpu_pasensor_a_data;

	pasensor_out.u32 = readl(SYSCTRL_BASE_REG + NPU_PASENSOR_OUT0);
	npu_pasensor_o_data = pasensor_out.bits.pasensor_nbti_o_data & 0x1ff;
	npu_pasensor_a_data = pasensor_out.bits.pasensor_nbti_a_data & 0x1ff;
	*npu_pasensor = npu_pasensor_o_data - npu_pasensor_a_data;
}

static void init_pasensor(void)
{
	/* Initializing the CPU pasensor */
	writel(CRG_PASENSOR_CLK_EN, CRG_BASE_ADDR + CRG_CPU_PASENSOR_CK_RST_CTRL);
	writel(CRG_PASENSOR_SRST_REQ, CRG_BASE_ADDR + CRG_CPU_PASENSOR_CK_RST_CTRL);
	writel(PASENSOR_CFG0_VAL0, MISC_BASE_ADDR + CPU_PASENSOR_CFG0);
	writel(PASENSOR_CFG1_VAL, MISC_BASE_ADDR + CPU_PASENSOR_CFG1);
	writel(PASENSOR_CFG0_VAL1, MISC_BASE_ADDR + CPU_PASENSOR_CFG0);
	udelay(WAITING_10_US);
	writel(PASENSOR_CFG0_VAL2, MISC_BASE_ADDR + CPU_PASENSOR_CFG0);
	udelay(WAITING_10_US);

	/* Initializing the NPU pasensor */
	writel(CRG_PASENSOR_CLK_EN, CRG_BASE_ADDR + CRG_NPU_PASENSOR_CK_RST_CTRL);
	writel(CRG_PASENSOR_SRST_REQ, CRG_BASE_ADDR + CRG_NPU_PASENSOR_CK_RST_CTRL);
	writel(PASENSOR_CFG0_VAL0, MISC_BASE_ADDR + NPU_PASENSOR_CFG0);
	writel(PASENSOR_CFG1_VAL, MISC_BASE_ADDR + NPU_PASENSOR_CFG1);
	writel(PASENSOR_CFG0_VAL1, MISC_BASE_ADDR + NPU_PASENSOR_CFG0);
	udelay(WAITING_10_US);
	writel(PASENSOR_CFG0_VAL2, MISC_BASE_ADDR + NPU_PASENSOR_CFG0);
}
static void save_hpm(unsigned int hpm_core_rvt40, unsigned int hpm_cpu_hvt35, \
					unsigned int hpm_npu_rvt40, int use_board_hpm)
{
	u_hpm_storage_reg hpm_storage_reg;
	hpm_storage_reg.bits.core_hpm_value = hpm_core_rvt40 & 0x3ff;
	hpm_storage_reg.bits.cpu_hpm_value = hpm_cpu_hvt35 & 0x3ff;
	hpm_storage_reg.bits.npu_hpm_value = hpm_npu_rvt40 & 0x3ff;
	hpm_storage_reg.bits.use_board_hpm = use_board_hpm & 0x1;
	writel(hpm_storage_reg.u32, SYSCTRL_REG + HPM_STORAGE_REG);
}

static void svb_error(void)
{
	log_serial_puts((const s8 *)"The table is incorrect.\n");
	call_reset();
}

static void set_svb_volt(unsigned int hpm_core_rvt40, unsigned int hpm_cpu_hvt35, unsigned int hpm_npu_rvt40)
{
	volatile unsigned int version_id = 0;
	volatile u_svb_version_reg svb_version;

	version_id = readl(OTP_BASE_REG + OTP_VERSION_ID_REG);
	svb_version.u32 = readl(SYSCTRL_REG + SVB_VERSION_ADDR);

	if (svb_version.bits.battery_type == YES_BATTERIES) {
		if ((svb_version.bits.svb_type == SVB_19DV5) && (version_id == OTP_19D_VERSION_ID)) {
			set_volt(hpm_core_rvt40, hpm_cpu_hvt35, hpm_npu_rvt40, &g_svb_19d_battery);
		} else if ((svb_version.bits.svb_type == SVB_19DV5_PT) && (version_id == OTP_19D_VERSION_ID)) {
			set_volt(hpm_core_rvt40, hpm_cpu_hvt35, hpm_npu_rvt40, &g_svb_pt_battery);
		} else if ((svb_version.bits.svb_type == SVB_16DV5) && (version_id == OTP_16D_VERSION_ID)) {
			set_volt(hpm_core_rvt40, hpm_cpu_hvt35, hpm_npu_rvt40, &g_svb_16d_battery);
		} else {
			svb_error();
		}
	} else if (svb_version.bits.battery_type == NO_BATTERIES) {
		if ((svb_version.bits.svb_type == SVB_19DV5) && (version_id == OTP_19D_VERSION_ID)) {
			set_volt(hpm_core_rvt40, hpm_cpu_hvt35, hpm_npu_rvt40, &g_svb_19d);
		} else if ((svb_version.bits.svb_type == SVB_19DV5_PT) && (version_id == OTP_19D_VERSION_ID)) {
			set_volt(hpm_core_rvt40, hpm_cpu_hvt35, hpm_npu_rvt40, &g_svb_pt);
		} else if ((svb_version.bits.svb_type == SVB_16DV5) && (version_id == OTP_16D_VERSION_ID)) {
			set_volt(hpm_core_rvt40, hpm_cpu_hvt35, hpm_npu_rvt40, &g_svb_16d);
		} else {
			svb_error();
		}
	} else {
		svb_error();
	}
}

void start_svb(void)
{
	unsigned int hpm_core_rvt40 = 0;
	unsigned int hpm_cpu_hvt35 = 0;
	unsigned int hpm_npu_rvt40 = 0;
	unsigned int svb_ver = 0;
	int cpu_pasensor = 0;
	int npu_pasensor = 0;
	int temperature = 0;
	int use_board_hpm = 0;

	/* add SVB VER */
	svb_ver = readl(SVB_VER_REG);
	svb_ver = svb_ver & 0xffff;
	svb_ver = svb_ver | SVB_VER_VAL;
	writel(svb_ver, SVB_VER_REG);

	/* init temperature and hpm and pasensor */
	init_temperature();
	init_hpm();

	/* Configuration 0.99V */
	set_voltage_val(HPM_STD_VOLT);
	init_pasensor();

	/* Waiting for 10 ms */
	mdelay(WAITING_10_MS);
	start_hpm(&hpm_core_rvt40, &hpm_cpu_hvt35, &hpm_npu_rvt40);
	get_pasensor(&cpu_pasensor, &npu_pasensor);
	get_temperature(&temperature);
	adjust_hpm(&hpm_core_rvt40, &hpm_cpu_hvt35, &hpm_npu_rvt40, temperature, &use_board_hpm);
	save_hpm(hpm_core_rvt40, hpm_cpu_hvt35, hpm_npu_rvt40, use_board_hpm);
	set_svb_volt(hpm_core_rvt40, hpm_cpu_hvt35, hpm_npu_rvt40);
	save_temperature_pasensor(temperature, cpu_pasensor, npu_pasensor);

	mdelay(WAITING_6_MS);
}

void end_svb(void)
{
	volatile unsigned int core_volt_val = 0;
	volatile unsigned int reg_val = 0;

	core_volt_val = readl(SYSCTRL_REG + CORE_V_STORAGE_REG) & 0xffff;
	core_volt_val = core_max(core_volt_val, LOW_TEMPERATURE_V);
	reg_val = readl(SYSCTRL_REG + CORE_V_STORAGE_REG) & 0xffff0000;
	reg_val = reg_val | core_volt_val;
	writel(reg_val, SYSCTRL_REG + CORE_V_STORAGE_REG);

	core_volt_val = calc_volt_regval(core_volt_val, CORE_VOLT_MAX0, CORE_VOLT_MIN0);
	svb_pwm_cfg(core_volt_val, PWM_CORE_RVT40_VOL_REG);
	mdelay(WAITING_6_MS);
}
