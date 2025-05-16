/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2023. All rights reserved.
 */

/** @defgroup Transport Transport
 *  @brief    Abstraction layer to access device's registers
 *  @{
 */

/** @file  inv_imu_transport.h */

#ifndef _INV_IMU_TRANSPORT_H_
#define _INV_IMU_TRANSPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* forward declaration */
struct inv_imu_device;

/** Available serial interface type. */
typedef enum {
	UI_I2C, /**< Selects I2C interface. */
	UI_SPI4, /**< Selects 4-wire SPI interface. */
	UI_SPI3 /**< Selects 3-wire SPI interface. */
} SERIAL_IF_TYPE_t;

/** Serial interface definition */
struct inv_imu_serif {
	void *context;
	int (*read_reg)(struct inv_imu_serif *serif, uint8_t reg, uint8_t *buf, uint32_t len);
	int (*write_reg)(struct inv_imu_serif *serif, uint8_t reg, const uint8_t *buf, uint32_t len);
	uint32_t         max_read;
	uint32_t         max_write;
	SERIAL_IF_TYPE_t serif_type;
};

/** Transport interface definition. */
struct inv_imu_transport {
	/** Serial interface object.
	 *  @warning Must be the first object in this structure.
	 */
	struct inv_imu_serif serif;

	/** Contains mirrored values of some IP registers. */
	struct register_cache {
		uint8_t pwr_mgmt0_reg;
		uint8_t gyro_config0_reg;
		uint8_t accel_config0_reg;
		uint8_t tmst_config1_reg;
	} register_cache;

	/** Internal counter for MCLK requests. */
	uint8_t need_mclk_cnt;
};

/** @brief Init cache variable.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_init_transport(struct inv_imu_device *s);

/** @brief Reads data from a register on IMU.
 *  @param[in] s     Pointer to device.
 *  @param[in] reg   Register address to be read.
 *  @param[in] len   Number of byte to be read.
 *  @param[out] buf  Output data from the register.
 *  @return          0 on success, negative value on error.
 */
int inv_imu_read_reg(struct inv_imu_device *s, uint32_t reg, uint32_t len, uint8_t *buf);

/** @brief Writes data to a register on IMU.
 *  @param[in] s    Pointer to device.
 *  @param[in] reg  Register address to be written.
 *  @param[in] len  Number of byte to be written.
 *  @param[in] buf  Input data to write.
 *  @return         0 on success, negative value on error.
 */
int inv_imu_write_reg(struct inv_imu_device *s, uint32_t reg, uint32_t len, const uint8_t *buf);

/** @brief Enable MCLK.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_switch_on_mclk(struct inv_imu_device *s);

/** @brief Disable MCLK.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_switch_off_mclk(struct inv_imu_device *s);

#ifdef __cplusplus
}
#endif

#endif /* _INV_IMU_TRANSPORT_H_ */

/** @} */
