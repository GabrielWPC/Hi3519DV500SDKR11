/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef TA_SECURITY_SUBSYS_H
#define TA_SECURITY_SUBSYS_H

#define TA_SECURITY_SUBSYS_UUID \
    {                                                      \
        0x8aabf300, 0x2450, 0x11e4,                        \
        {                                                  \
            0xab, 0xe2, 0x9f, 0x02, 0xa5, 0xd5, 0xbc, 0x2f \
        }                                                  \
    }

/* The function IDs implemented in this TA */
enum {
    TEST_SYMC_CLEARKEY = 1,
    TEST_SYMC_ROOTKEY,
    TEST_SYMC_MAC,
    TEST_HASH,
    TEST_PBKDF2,
    TEST_TRNG,
    TEST_RSA,
    TEST_ECC,
    TEST_SM4,
    TEST_SM3,
    TEST_SM2,
    TA_CIPHER_SAMPLE_CMD_COUNT
};

#endif /* TA_SECURITY_SUBSYS_H */
