/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "sample_utils.h"
#include "ss_mpi_sys_mem.h"
#include "ss_mpi_cipher.h"

#define BYTES_IN_WORD   4

td_void cipher_free(const crypto_buf_attr *buf_attr, const void *virt_addr)
{
    if (buf_attr->phys_addr != TD_NULL && virt_addr != TD_NULL) {
        ss_mpi_sys_mmz_free(buf_attr->phys_addr, virt_addr);
    }
}

td_s32 cipher_alloc(crypto_buf_attr *buf_attr, void **virt_addr, unsigned int size)
{
    td_s32 ret;
    td_phys_addr_t phys_addr;
    ret = ss_mpi_sys_mmz_alloc(&phys_addr, virt_addr, NULL, NULL, size);
    if (ret != TD_SUCCESS) {
        sample_err("ss_mpi_sys_mmz_alloc failed\n");
        return TD_FAILURE;
    }
    buf_attr->phys_addr = (unsigned long) phys_addr;
    return TD_SUCCESS;
}

td_s32 get_random_data(td_u8 *buffer, td_u32 size)
{
    td_s32 ret;
    if (buffer == TD_NULL) {
        sample_err("invalid buffer!\n");
        return TD_FAILURE;
    }
    ret = ss_mpi_cipher_trng_get_multi_random(size, buffer);
    return ret;
}

static td_s32 get_rsa3072_key(drv_pke_rsa_pub_key *pub_key, drv_pke_rsa_priv_key *priv_key)
{
    to_be_processed(pub_key);
    to_be_processed(priv_key);
    sample_err("Please implement this function to get rsa3072 key!!!\n");
    return TD_FAILURE;
}

static td_s32 get_rsa4096_key(drv_pke_rsa_pub_key *pub_key, drv_pke_rsa_priv_key *priv_key)
{
    to_be_processed(pub_key);
    to_be_processed(priv_key);
    sample_err("Please implement this function to get rsa4096 key!!!\n");
    return TD_FAILURE;
}

td_s32 get_rsa_key(drv_pke_rsa_pub_key *pub_key, drv_pke_rsa_priv_key *priv_key, td_u32 key_len)
{
    td_s32 ret = TD_SUCCESS;
    if (pub_key == TD_NULL || priv_key == TD_NULL) {
        return TD_FAILURE;
    }
    switch (key_len) {
        case RSA_3072_KEY_LEN:
            sample_chk_expr_return(get_rsa3072_key(pub_key, priv_key), TD_SUCCESS);
            break;
        case RSA_4096_KEY_LEN:
            sample_chk_expr_return(get_rsa4096_key(pub_key, priv_key), TD_SUCCESS);
            break;
        default:
            sample_err("Unsupported key length!!!\n");
            return TD_FAILURE;
    }
    priv_key->n_len = key_len;
    priv_key->d_len = key_len;
    pub_key->len = key_len;
    return ret;
}

td_void destroy_rsa_key(drv_pke_rsa_pub_key *pub_key, drv_pke_rsa_priv_key *priv_key, td_u32 key_len)
{
    to_be_processed(pub_key);
    to_be_processed(priv_key);
    to_be_processed(key_len);
    sample_err("Please ensure that the key is cleared after use!!!\n");
}