/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef SOC_PKE_API_H
#define SOC_PKE_API_H

#include "td_type.h"
#include "soc_cipher_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* __cplusplus */

#define EXT_DRV_PKE_SM2_LEN_IN_BYTES 32

typedef enum {
	EXT_DRV_PKE_LEN_256 = 32,
	EXT_DRV_PKE_LEN_384 = 48,
	EXT_DRV_PKE_LEN_512 = 64,
	EXT_DRV_PKE_LEN_521 = 68,
	EXT_DRV_PKE_LEN_2048 = 256,
	EXT_DRV_PKE_LEN_3072 = 384,
	EXT_DRV_PKE_LEN_4096 = 512,
	EXT_DRV_PKE_LEN_MAX,
	EXT_DRV_PKE_LEN_INVALID = 0xffffffff,
} ext_drv_pke_len;

typedef enum {
	EXT_DRV_PKE_ECC_TYPE_RFC5639,        /* RFC 5639 - Brainpool P256/384/512 */
	EXT_DRV_PKE_ECC_TYPE_FIPS,           /* NIST FIPS 186-4 P256/384/521, suggest not to use */
	EXT_DRV_PKE_ECC_TYPE_RFC7748,        /* RFC 7748 - Curve25519 */
	EXT_DRV_PKE_ECC_TYPE_RFC8032,        /* RFC 8032 - ED25519 */
	EXT_DRV_PKE_ECC_TYPE_SM2,            /* GMT 0003.2-2012 */
	EXT_DRV_PKE_ECC_TYPE_MAX,
	EXT_DRV_PKE_ECC_TYPE_INVALID = 0xffffffff,
} ext_drv_pke_ecc_type;

typedef enum {
	EXT_DRV_PKE_HASH_TYPE_SHA1 = 0x00,   /* Suggest Not to use */
	EXT_DRV_PKE_HASH_TYPE_SHA224,
	EXT_DRV_PKE_HASH_TYPE_SHA256,
	EXT_DRV_PKE_HASH_TYPE_SHA384,
	EXT_DRV_PKE_HASH_TYPE_SHA512,
	EXT_DRV_PKE_HASH_TYPE_SM3 = 0x10,
	EXT_DRV_PKE_HASH_TYPE_MAX,
	EXT_DRV_PKE_HASH_TYPE_INVALID = 0xffffffff,
} ext_drv_pke_hash_type;

/* * struct of ecc curves parameters. */
typedef struct {
	const td_u8 *p;   /* prime specifying the base field. It is p (RFC5639), p (FIPS), p (RFC7748). */
	const td_u8 *a;   /* Curve parameter a. It is A (RFC5639), c (FIPS), A24 (RFC7748). */
	const td_u8 *b;   /* Curve parameter b. It is B (RFC5639), b (FIPS), N/A (RFC7748). */
	const td_u8 *gx;  /* X coordinates of G which is a base point on the curve.
                         It is x (RFC5639), Gx (FIPS), U(P) (RFC7748). */
	const td_u8 *gy;  /* Y coordinates of G which is a base point on the curve.
                         It is y (RFC5639), Gy (FIPS), N/A (RFC7748). */
	const td_u8 *n;   /* Prime which is the order of G point. It is q (RFC5639), n (FIPS), N/A (RFC7748). */
	td_u32 h;         /* Cofactor, which is the order of the elliptic curve divided by the order of the point G.
                         It is h (RFC5639), h (FIPS), Cofactor (RFC7748). */
	ext_drv_pke_len ksize;         /* Ecc key size in bytes. It corresponds to the size in bytes of the prime. */
	ext_drv_pke_ecc_type ecc_type; /* Type of ECC curve */
} ext_drv_pke_ecc_curve;

/* * struct of ecc point */
typedef struct {
	td_u8 *x;   /* X coordinates of the generated public key, the caller ensures it is padded with leading
                   zeros if the effective size of this key is smaller than ecc key size. */
	td_u8 *y;   /* Y coordinates of the generated public key, the caller ensures it is padded with leading
                   zeros if the effective size of this key is smaller than ecc key size. */
	td_u32 length;
} ext_drv_pke_ecc_point;

/* * struct of ecc signature */
typedef struct {
	td_u8 *r;   /* r component of the signature. */
	td_u8 *s;   /* s component of the signature. */
	td_u32 length;
} ext_drv_pke_ecc_sig;

typedef enum {
	EXT_DRV_PKE_RSA_SCHEME_PKCS1_V15 = 0x00,  /* PKCS#1 V15 */
	EXT_DRV_PKE_RSA_SCHEME_PKCS1_V21,         /* PKCS#1 V21, PSS for signning, OAEP for encryption */
	EXT_DRV_PKE_RSA_SCHEME_MAX,
	EXT_DRV_PKE_RSA_SCHEME_INVALID = 0xffffffff,
} ext_drv_pke_rsa_scheme;

/* * struct of RSA public key */
typedef struct {
	td_u8  *n;            /* point to public modulus */
	td_u8  *e;            /* point to public exponent */
	td_u16 len;           /* length of public modulus, max value is 512Byte */
} ext_drv_pke_rsa_pub_key;

typedef struct {
	td_u32  length;
	td_u8  *data;
} ext_drv_pke_data;

typedef enum {
	EXT_DRV_PKE_BUF_NONSECURE = 0x00,
	EXT_DRV_PKE_BUF_SECURE,
	EXT_DRV_PKE_BUF_INVALID = 0xffffffff,
} ext_drv_pke_buffer_secure;

typedef struct {
	td_u32  length;
	td_u8  *data;
	ext_drv_pke_buffer_secure buf_sec;
} ext_drv_pke_msg;


/* * RSA private key struct */
typedef struct {
	td_u8 *n;          /* *< public modulus */
	td_u8 *e;          /* *< public exponent */
	td_u8 *d;          /* *< private exponent */
	td_u8 *p;          /* *< 1st prime factor */
	td_u8 *q;          /* *< 2nd prime factor */
	td_u8 *dp;         /* *< D % (P - 1) */
	td_u8 *dq;         /* *< D % (Q - 1) */
	td_u8 *qp;         /* *< 1 / (Q % P) */
	td_u16 n_len;      /* *< length of public modulus */
	td_u16 e_len;      /* *< length of public exponent */
	td_u16 d_len;      /* *< length of private exponent */
	td_u16 p_len;      /* *< length of 1st prime factor,should be half of u16NLen */
	td_u16 q_len;      /* *< length of 2nd prime factor,should be half of u16NLen */
	td_u16 dp_len;     /* *< length of D % (P - 1),should be half of u16NLen */
	td_u16 dq_len;     /* *< length of D % (Q - 1),should be half of u16NLen */
	td_u16 qp_len;     /* *< length of 1 / (Q % P),should be half of u16NLen */
} ext_drv_pke_rsa_priv_key;

/* PKE LPC configuration */
typedef enum {
	EXT_DRV_PKE_LPC_CLOCK = 0x00,        /* Clock of PKE module */
	EXT_DRV_PKE_LPC_CLOCKGATE = 0x01,    /* Low power feature of PKE module */
	EXT_DRV_PKE_LPC_INVALID = 0xffffffff,
} ext_drv_pke_lpc;

/**
\brief PKE initialization.
\attention \n
The PKE clock will be opened by this API.

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_init(void);

/**
\brief PKE deinitialization.
\attention \n
The PKE clock will be closed by this API.

\retval ::EXT_SEC_SUCCESS             Call this API succussful.
\retval ::EXT_SEC_FAILURE             Call this API fails.
\retval ::EXT_ERR_PKE_ENV_NOT_READY   PKE not initialized.

\see \n
N/A
*/
td_s32 uapi_drv_pke_deinit(void);

/**
\brief PKE lock and enable noise, need to be called before calling calc API.
\attention \n
N/A

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_lock_noise(void);

/**
\brief PKE unlock and disable noise, need to be called after calling calc API.
\attention \n
N/A

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_unlock_noise(void);

/**
\brief Generate ECC or SM2 public/private key pair.
\attention \n
Need to call uapi_drv_pke_init first.

\param[in] ecc:  Elliptic curve domain parameters. The caller is in charge of padding each buffer with leading zeros
    if the effective size of the domain parameter conveyed is smaller than ecc->ksize.
\param[in] input_priv_key: Buffer containing an optional input private key from which the public has to be generated.
    The caller ensures it is padded with leading zeros if the effective size of this key is smaller than the ecc->ksize.
    If no private key is provided as input, function generates a random private key and stores it in output_priv_key
    buffer.
\param[out] output_priv_key: Buffer where to write the generated private key,
    in case no private key is provided as input.
\param[out] output_pub_key: Buffer where to write the X and Y coordinates of the generated public key.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS                    Call this API succussful.
\retval ::EXT_SEC_FAILURE                    Call this API fails.
\retval ::EXT_ERR_PKE_ENV_NOT_READY          PKE not initialized.
\retval ::EXT_ERR_PKE_INVALID_PARAMETER      The parameters are invalid.
\retval ::EXT_ERR_PKE_TIMEOUT                Generate ECC private key timeout.

\see \n
N/A
*/
td_s32 uapi_drv_pke_ecc_gen_key(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_data *input_priv_key,
								const ext_drv_pke_data *output_priv_key, const ext_drv_pke_ecc_point *output_pub_key,
								const uintptr_t check_word);

/**
\brief Sign a message with ECDSA or SM2 algorithm and returns an uncompressed signature made of the pair (r, s).
\attention \n
1. Need to call uapi_drv_pke_init first.
2. Only support RFC5639, FIPS, RFC8032 and SM2 curves. Do not support RFC7748 curve.
3. Hash length should NOT shorter than ecc ksize.

\param[in] ecc:  Elliptic curve domain parameters. The caller is in charge of padding each buffer with leading zeros
    if the effective size of the domain parameter conveyed is smaller than ecc->ksize.
\param[in] priv_key: The ECC private key.
\param[in] hash: The hash result of message.
\param[out] sig: Buffer where to write the uncompressed r and s component of the resulting signature.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS                    Call this API succussful.
\retval ::EXT_SEC_FAILURE                    Call this API fails.
\retval ::EXT_ERR_PKE_ENV_NOT_READY          PKE not initialized.
\retval ::EXT_ERR_PKE_INVALID_PARAMETER      The parameters are invalid.
\retval ::EXT_ERR_PKE_TIMEOUT                Generate ECC private key timeout.

\see \n
N/A
*/
td_s32 uapi_drv_pke_ecdsa_sign(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_data *priv_key,
							   const ext_drv_pke_data *hash, const ext_drv_pke_ecc_sig *sig, const uintptr_t check_word);

/**
\brief Verify an ECDSA or SM2 signature.
\attention \n
1. Need to call uapi_drv_pke_init first.
2. Only support RFC5639, FIPS, RFC8032 and SM2 curves. Do not support RFC7748 curve.
3. Hash length should NOT shorter than ecc ksize.

\param[in] ecc:  Elliptic curve domain parameters. The caller is in charge of padding each buffer with leading zeros
    if the effective size of the domain parameter conveyed is smaller than ecc->ksize.
\param[in] pub_key: The ECC public key.
\param[in] hash: The hash value.
\param[in] sig: Buffer where to store the uncompressed r and s component of the resulting signature.
\param[out] v: Buffer where to store the uncompressed v value, which can be used for redundant check v=r.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS                    Call this API succussful.
\retval ::EXT_SEC_FAILURE                    Call this API fails.
\retval ::EXT_ERR_PKE_ENV_NOT_READY          PKE not initialized.
\retval ::EXT_ERR_PKE_INVALID_PARAMETER      The parameters are invalid.
\retval ::EXT_ERR_PKE_TIMEOUT                Generate ECC private key timeout.

\see \n
N/A
*/
td_s32 uapi_drv_pke_ecdsa_verify(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_ecc_point *pub_key,
								 const ext_drv_pke_data *hash, const ext_drv_pke_ecc_sig *sig, td_u8 *v, const uintptr_t check_word);

/**
\brief Sign a message with EDDSA algorithm and returns a signature made of the pair (r, s).
\attention \n
1. Need to call uapi_drv_pke_init first.
2. Only support RFC8032 curve.

\param[in] ecc:  Elliptic curve domain parameters. The caller is in charge of padding each buffer with leading zeros
    if the effective size of the domain parameter conveyed is smaller than ecc->ksize.
\param[in] priv_key: The ECC private key.
\param[in] msg: The message for signning or verification.
\param[out] sig: Buffer where to write the r and s component of the resulting signature.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS                    Call this API succussful.
\retval ::EXT_SEC_FAILURE                    Call this API fails.
\retval ::EXT_ERR_PKE_ENV_NOT_READY          PKE not initialized.
\retval ::EXT_ERR_PKE_INVALID_PARAMETER      The parameters are invalid.
\retval ::EXT_ERR_PKE_TIMEOUT                Generate ECC private key timeout.

\see \n
N/A
*/
td_s32 uapi_drv_pke_eddsa_sign(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_data *priv_key,
							   const ext_drv_pke_msg *msg, const ext_drv_pke_ecc_sig *sig, const uintptr_t check_word);

/**
\brief Verify an EDDSA signature.
\attention \n
1. Need to call uapi_drv_pke_init first.
2. Only support RFC8032 curve.

\param[in] ecc:  Elliptic curve domain parameters. The caller is in charge of padding each buffer with leading zeros
    if the effective size of the domain parameter conveyed is smaller than ecc->ksize.
\param[in] pub_key: The ECC public key.
\param[in] msg: The message for signning or verification.
\param[in] sig: Buffer where to store the r and s component of the resulting signature.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS                    Call this API succussful.
\retval ::EXT_SEC_FAILURE                    Call this API fails.
\retval ::EXT_ERR_PKE_ENV_NOT_READY          PKE not initialized.
\retval ::EXT_ERR_PKE_INVALID_PARAMETER      The parameters are invalid.
\retval ::EXT_ERR_PKE_TIMEOUT                Generate ECC private key timeout.

\see \n
N/A
*/
td_s32 uapi_drv_pke_eddsa_verify(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_ecc_point *pub_key,
								 const ext_drv_pke_msg *msg, const ext_drv_pke_ecc_sig *sig, const uintptr_t check_word);

/**
\brief Generate ECDH or SM2 shared key from a public key and a private key
\attention \n
1. Need to call uapi_drv_pke_init first.
2. Do not support RFC8032 curve.

\param[in] ecc:  Elliptic curve domain parameters. The caller is in charge of padding each buffer with leading zeros
    if the effective size of the domain parameter conveyed is smaller than ecc->ksize.
\param[in] input_pub_key: The input ECC public key.
\param[in] input_priv_key: The input ECC private key.
\param[out] output_shared_key: The output shared key.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS                    Call this API succussful.
\retval ::EXT_SEC_FAILURE                    Call this API fails.
\retval ::EXT_ERR_PKE_ENV_NOT_READY          PKE not initialized.
\retval ::EXT_ERR_PKE_INVALID_PARAMETER      The parameters are invalid.
\retval ::EXT_ERR_PKE_TIMEOUT                Generate ECC private key timeout.

\see \n
N/A
*/
td_s32 uapi_drv_pke_ecc_gen_ecdh_key(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_ecc_point *input_pub_key,
									 const ext_drv_pke_data *input_priv_key, const ext_drv_pke_data *output_shared_key,
									 const uintptr_t check_word);

/**
\brief Check whether the dot is on the curve or not
\attention \n
1. Need to call uapi_drv_pke_init first.
2. Only support RFC5639, FIPS, SM2 curves. Do not support RFC7748, RFC8032 curve.

\param[in] ecc:  Elliptic curve domain parameters. The caller is in charge of padding each buffer with leading zeros
    if the effective size of the domain parameter conveyed is smaller than ecc->ksize.
\param[in] pub_key: The ECC public key to specify the dot.
\param[out] is_on_curve: The result of the check.
    TD_TRUE: The dot is on the curve. TD_FALSE: The dot is not on the curve.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS                    Call this API succussful.
\retval ::EXT_SEC_FAILURE                    Call this API fails.
\retval ::EXT_ERR_PKE_ENV_NOT_READY          PKE not initialized.
\retval ::EXT_ERR_PKE_INVALID_PARAMETER      The parameters are invalid.
\retval ::EXT_ERR_PKE_TIMEOUT                Generate ECC private key timeout.

\see \n
N/A
*/
td_s32 uapi_drv_pke_check_dot_on_curve(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_ecc_point *pub_key,
									   td_bool *is_on_curve, const uintptr_t check_word);

/* *
\brief Generate the hash result of pre-process step2 for SM2 signning and verification.
\attention \n
Please refer to section 8.2 of GMT 0009-2012
Z = SM3(ENTL || ID || a || b || XG || YG || XA || YA)
H = SM3(Z || M);

\param[in] sm2_id:  USER ID.
\param[in] pub_key:  SM2 public key.
\param[in] msg:  the message for signning or verification.
\param[out] hash:  the hash result of pre-process step2 for SM2 signning.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_PKE_INVALID_PARAMETER  The parameter is invalid.
\see \n
N/A
*/
td_s32 uapi_drv_pke_sm2_dsa_hash(const ext_drv_pke_data *sm2_id, const ext_drv_pke_ecc_point *pub_key,
								 const ext_drv_pke_msg *msg, ext_drv_pke_data *hash, const uintptr_t check_word);

/* *
\brief SM2 encryption a plaintext with a SM2 public key.
\attention \n
GMT 0003.4-2012
N/A
\param[in] pub_key: SM2 public key.
\param[in] plain_text: data to be encrypted
\param[out] cipher_text：output encrypted data
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_CIPHER_INVALID_PARA  The parameter is invalid.
\see \n
N/A
*/
td_s32 uapi_drv_pke_sm2_public_encrypt(const ext_drv_pke_ecc_point *pub_key, const ext_drv_pke_data *plain_text,
									   const ext_drv_pke_data *cipher_text, const uintptr_t check_word);

/* *
\brief SM2 decryption a ciphertext with a SM2 private key.
\attention \n
GMT 0003.4-2012
N/A
\param[in] priv_key: SM2 private key.
\param[in] cipher_text：data to be decrypted
\param[out] plain_text: output decrypted data
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_PKE_INVALID_PARAMETER  The parameter is invalid.
\see \n
N/A
*/
td_s32 uapi_drv_pke_sm2_private_decrypt(const ext_drv_pke_data *priv_key, const ext_drv_pke_data *cipher_text,
										const ext_drv_pke_data *plain_text, const uintptr_t check_word);

/**
\brief RSA signature verification a context with appendix, where a RSA public key is used.
\attention \n
1. Need to call uapi_drv_pke_init first.
2. Please note that the input_hash will be changed by the driver code when doing RSASSA-PSS verification.

\param[in] pub_key: RSA public key.
\param[in] scheme: RSA signature scheme.
\param[in] input_hash: The input SHA-256 value.
\param[in] sign: message of signature.
\param[out] output_hash: The output SHA-256 value of the signature.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS                    Call this API succussful.
\retval ::EXT_SEC_FAILURE                    Call this API fails.
\retval ::EXT_ERR_PKE_ENV_NOT_READY          PKE not initialized.
\retval ::EXT_ERR_PKE_INVALID_PARAMETER      The parameters are invalid.
\retval ::EXT_ERR_PKE_TIMEOUT                Generate ECC private key timeout.

\see \n
N/A
*/
td_s32 uapi_drv_pke_rsa_verify(const ext_drv_pke_rsa_pub_key *pub_key, ext_drv_pke_rsa_scheme scheme,
							   const ext_drv_pke_hash_type hash_type, const ext_drv_pke_data *input_hash, const ext_drv_pke_data *sign,
							   const ext_drv_pke_data *output_hash, const uintptr_t check_word);

/* *
\brief RSA signature a context with appendix, where a signer’s RSA private key is used.
\attention \n
N/A
\param[in] priv_key: The RSA private key.
\param[in] scheme: The RSA schema used, V21/V15 in common.
\param[in] hash_type: Type of hash that may used in sign.
\param[in] input_hash: The hash result of message to sign.
\param[out] sign: output signature
\retval ::EXT_SEC_SUCCESS  Call this API successful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_CIPHER_INVALID_PARA  The parameter is invalid.
\see \n
N/A
*/
td_s32 uapi_drv_pke_rsa_sign(const ext_drv_pke_rsa_priv_key *priv_key, const ext_drv_pke_rsa_scheme scheme,
							 const ext_drv_pke_hash_type hash_type, const ext_drv_pke_data *input_hash,
							 const ext_drv_pke_data *sign, const uintptr_t check_word);

/* *
\brief RSA encryption, encrypt a plaintext with a RSA public key.
\attention \n
N/A
\param[in] scheme: RSA encryption scheme
\param[in] hash_type: Hash Type
\param[in] pub_key: RSA public key
\param[in] input: input data to be encrypted
\param[in] label: The buffer holding the custom label to use, it's optional and only for OAEP.
\param[out] output: output encrypted data, its buffer length must not less than the width of RSA Key N.
\retval ::EXT_SEC_SUCCESS  Call this API successful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_CIPHER_INVALID_PARA  The parameter is invalid.
\see \n
N/A
*/
td_s32 uapi_drv_pke_rsa_public_encrypt(const ext_drv_pke_rsa_scheme scheme, const ext_drv_pke_hash_type hash_type,
									   const ext_drv_pke_rsa_pub_key *pub_key, const ext_drv_pke_data *input, const ext_drv_pke_data *label,
									   const ext_drv_pke_data *output, const uintptr_t check_word);

/* *
\brief RSA decryption, decrypt a ciphertext with a RSA private key.
\attention \n
N/A
\param[in] scheme: RSA encryption scheme
\param[in] hash_type: Hash Type
\param[in] priv_key: RSA private key
\param[in] input: input data to be encrypted
\param[in] label: The buffer holding the custom label to use, it's optional and only for OAEP.
\param[out] output: output data after decryption
\retval ::EXT_SEC_SUCCESS  Call this API successful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_CIPHER_INVALID_PARA  The parameter is invalid.
\see \n
N/A
*/
td_s32 uapi_drv_pke_rsa_private_decrypt(const ext_drv_pke_rsa_scheme scheme, const ext_drv_pke_hash_type hash_type,
										const ext_drv_pke_rsa_priv_key *priv_key, const ext_drv_pke_data *input, const ext_drv_pke_data *label,
										ext_drv_pke_data *output, const uintptr_t check_word);

/* *
\brief RSA encryption, encrypt a plaintext with a RSA private key.
\attention \n
N/A
\param[in] scheme: RSA encryption scheme
\param[in] hash_type: Hash Type
\param[in] priv_key: RSA private key
\param[in] input: input data to be encryption
\param[in] label: The buffer holding the custom label to use, it's optional and only for OAEP.
\param[out] output： output data to be encryption, its buffer length must not less than the width of RSA Key N.
\retval ::EXT_SEC_SUCCESS  Call this API successful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_CIPHER_INVALID_PARA  The parameter is invalid.
\see \n
N/A
*/
td_s32 uapi_drv_pke_rsa_private_encrypt(const ext_drv_pke_rsa_scheme scheme, const ext_drv_pke_hash_type hash_type,
										const ext_drv_pke_rsa_priv_key *priv_key, const ext_drv_pke_data *input, const ext_drv_pke_data *label,
										const ext_drv_pke_data *output, const uintptr_t check_word);

/* *
\brief RSA decryption, decrypt a ciphertext with a RSA public key.
\attention \n
N/A
\param[in] scheme: RSA encryption scheme
\param[in] hash_type: Hash Type
\param[in] pub_key: RSA public key
\param[in] input: input data to be encryption
\param[in] label: The buffer holding the custom label to use, it's optional and only for OAEP.
\param[out] output： output buffer to storage decryption data, its length must not less than the width of RSA Key N
\retval ::EXT_SEC_SUCCESS  Call this API successful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_CIPHER_INVALID_PARA  The parameter is invalid.
\see \n
N/A
*/
td_s32 uapi_drv_pke_rsa_public_decrypt(const ext_drv_pke_rsa_scheme scheme, const ext_drv_pke_hash_type hash_type,
									   const ext_drv_pke_rsa_pub_key *pub_key, const ext_drv_pke_data *input, const ext_drv_pke_data *label,
									   ext_drv_pke_data *output, const uintptr_t check_word);

/**
\brief pke low power contrl.
\attention \n
N/A

\param[in] lpc: clock or clockgate to set.
\param[in] enable: disable or enable. TD_FALSE: disable, TD_TRUE: enable.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\see \n
N/A
*/
td_s32 uapi_drv_pke_low_power_set(ext_drv_pke_lpc lpc, td_bool enable, const uintptr_t check_word);

/**
\brief pke low power register status get.
\attention \n
N/A

\param[in] lpc: clock or clockgate register status want to get.
\param[out] enable: status get.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_low_power_get(ext_drv_pke_lpc lpc, td_bool *enable, const uintptr_t check_word);

/**
\brief c = a + b mod p.
\attention \n
N/A

\param[in] a: input parameter.
\param[in] b: input parameter.
\param[in] p: input parameter.
\param[out] c: output parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_add_mod(const ext_drv_pke_data *a, const ext_drv_pke_data *b, const ext_drv_pke_data *p,
							const ext_drv_pke_data *c, const uintptr_t check_word);

/**
\brief c = a - b mod p.
\attention \n
N/A

\param[in] a: input parameter.
\param[in] b: input parameter.
\param[in] p: input parameter.
\param[out] c: output parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_sub_mod(const ext_drv_pke_data *a, const ext_drv_pke_data *b, const ext_drv_pke_data *p,
							const ext_drv_pke_data *c, const uintptr_t check_word);

/**
\brief c = a * b mod p.
\attention \n
N/A

\param[in] a: input parameter.
\param[in] b: input parameter.
\param[in] p: input parameter.
\param[out] c: output parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_mul_mod(const ext_drv_pke_data *a, const ext_drv_pke_data *b, const ext_drv_pke_data *p,
							const ext_drv_pke_data *c, const uintptr_t check_word);

/**
\brief c = ~a mod p.
\attention \n
N/A

\param[in] a: input parameter.
\param[in] p: input parameter.
\param[out] c: output parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_inv_mod(const ext_drv_pke_data *a, const ext_drv_pke_data *p, const ext_drv_pke_data *c,
							const uintptr_t check_word);

/**
\brief c = a mod p.
\attention \n
N/A

\param[in] a: input parameter.
\param[in] p: input parameter.
\param[out] c: output parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_mod(const ext_drv_pke_data *a, const ext_drv_pke_data *p, const ext_drv_pke_data *c,
						const uintptr_t check_word);

/**
\brief c = a * b.
\attention \n
N/A

\param[in] a: input parameter.
\param[in] b: input parameter.
\param[out] c: output parameter. The size of c need to be (2 * upward aligning of max(a_len, klen)).
                The upward aligning size can be 32 bytes, 48 bytes, 68 bytes or 256 bytes.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_mul(const ext_drv_pke_data *a, const ext_drv_pke_data *b, const ext_drv_pke_data *c,
						const uintptr_t check_word);

/**
\brief R = k * U(x).
\attention \n
N/A

\param[in] k: input parameter.
\param[in] U: input U(x).
\param[in] p: input parameter p.
\param[in] a24: input parameter a24.
\param[out] rx: output parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_mg_mul_dot(const ext_drv_pke_data *k, const ext_drv_pke_data *U, const ext_drv_pke_data *p,
							   const ext_drv_pke_data *a24, const ext_drv_pke_data *rx, const uintptr_t check_word);

/**
\brief R = k * U(x,y).
\attention \n
1. Only support RFC8032 curve.
N/A

\param[in] k: input parameter.
\param[in] U: input U(x,y).
\param[in] p: input parameter p.
\param[in] pd: input parameter pd.
\param[out] r: output r(x,y).
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_ed_mul_dot(const ext_drv_pke_data *k, const ext_drv_pke_ecc_point *U, const ext_drv_pke_data *p,
							   const ext_drv_pke_data *pd, const ext_drv_pke_ecc_point *r, const uintptr_t check_word);

/**
\brief R = k * P(x,y).
\attention \n
N/A

\param[in] k: input parameter.
\param[in] px: input P(x).
\param[in] py: input P(y).
\param[out] rx: output parameter R(x).
\param[out] ry: output parameter R(y).
\param[in] ecc: the ecc curve parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_ecc_mul_dot(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_data *k,
								const ext_drv_pke_ecc_point *p, const ext_drv_pke_ecc_point *r, const uintptr_t check_word);

/**
\brief C = S(x,y) + R(x,y) for Ed25519 curve.
\attention \n
N/A

\param[in] sx: input S(x).
\param[in] sy: input S(y).
\param[in] rx: input R(x).
\param[in] ry: input R(y).
\param[out] cx: output parameter C(x).
\param[out] cy: output parameter C(y).
\param[in] ecc: the ecc curve parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_ed_add_dot(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_ecc_point *s,
							   const ext_drv_pke_ecc_point *r, const ext_drv_pke_ecc_point *c, const uintptr_t check_word);

/**
\brief C = S(x,y) + R(x,y).
\attention \n
N/A

\param[in] sx: input S(x).
\param[in] sy: input S(y).
\param[in] rx: input R(x).
\param[in] ry: input R(y).
\param[out] cx: output parameter C(x).
\param[out] cy: output parameter C(y).
\param[in] ecc: the ecc curve parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_add_dot(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_ecc_point *s,
							const ext_drv_pke_ecc_point *r, const ext_drv_pke_ecc_point *c, const uintptr_t check_word);

/**
\brief C = S(x,y) + S(x,y).
\attention \n
N/A

\param[in] sx: input S(x).
\param[in] sy: input S(y).
\param[out] cx: output parameter C(x).
\param[out] cy: output parameter C(y).
\param[in] ecc: the ecc curve parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_times_dot(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_ecc_point *s,
							  const ext_drv_pke_ecc_point *c, const uintptr_t check_word);

/**
\brief out = in ^ k mod n.
\attention \n
N/A

\param[in] n: input parameter.
\param[in] k: input parameter.
\param[in] in: input parameter.
\param[out] out: output parameter.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_exp_mod(const ext_drv_pke_data *n, const ext_drv_pke_data *k, const ext_drv_pke_data *in,
							const ext_drv_pke_data *out, const uintptr_t check_word);

/**
\brief Clear PKE ram.
\attention \n
N/A

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_clean_ram(void);

/**
\brief Montgomery the data before sending to MG_SUB_MOD, MG_MUL_MOD, MG_ADD_MOD.
\attention \n
This API is for ecc(ed25519) use to reduce size.

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_montgomery_data(td_u8 *output, const td_u8 *input, const td_u8 *mod_p, td_u32 klen,
									const uintptr_t check_word);

/**
\brief Calc hash of given data in arr.
\attention \n
This API is for ecc(ed25519) use to reduce size.

\param[in] arr: data array.
\param[in] len: length of data array.
\param[in] hash_type: hash type.
\param[out] hash: output hash.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_calc_hash(const ext_drv_pke_data *arr, const td_u32 len, const ext_drv_cipher_hash_type hash_type,
							  ext_drv_pke_data *hash);

/**
\brief Calc point_compress(point_mul()).
\attention \n
This API is for ecc(ed25519) use to reduce size.

\param[in] ecc: ecc curve.
\param[in] k: data size.
\param[in] len: ksize.
\param[out] out: output data.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_point_mul_compress(const ext_drv_pke_ecc_curve *ecc, const td_u8 *k, td_u8 *out, td_u32 len,
									   const uintptr_t check_word);

/**
\brief Montgomery(2d) the data before sending to MG_SUB_MOD, MG_MUL_MOD, MG_ADD_MOD.
\attention \n
This API is for ecc(ed25519) use to reduce size.

\param[out] pd: output result.
\param[in] d: input param.
\param[in] p: input param.
\param[in] klen: data size.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_montgomery_2d(td_u8 *pd, const td_u8 *d, const td_u8 *p, td_u32 klen, const uintptr_t check_word);

/**
\brief Expand secret.
\attention \n
This API is for ecc(ed25519) use to reduce size.

\param[in] secret: input secret data.
\param[out] a: output param.
\param[out] prefix: output param.
\param[in] len: data size.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_secret_expand(const td_u8 *secret, td_u8 *a, td_u8 *prefix, const td_u32 len);

/**
\brief Type of wait condition function provided by ROM
\attention \n
N/A

\param[in] param: params of wait condition func.
\retval ::TD_TRUE  wait condition reached.
\retval ::TD_FALSE  wait condition not reached.

\see \n
N/A
*/
typedef td_bool(*uapi_drv_pke_wait_condition_func)(const td_void *param);

/**
\brief Type of wait function provided by OS for ROM to call
\attention \n
N/A

\param[in] wait: wait queue.
\param[in] func: wait condition function.
\param[in] param: params of func.
\param[in] timeout_ms: timeout val in ms.
\retval ::TD_TRUE  event happened.
\retval ::TD_FALSE  no event happened.

\see \n
N/A
*/
typedef td_bool(*osal_pke_wait_timeout_interruptible)(const td_void *wait, uapi_drv_pke_wait_condition_func func,
													  const td_void *param, const td_u32 timeout_ms);

/**
\brief Register wait_func callback.
\attention \n
This API is provided by ROM to register wait_func callback for service.

\param[in] wait: wait event queue.
\param[in] wait_func: callback function.
\param[in] timeout_ms: timeout val in ms.
\param[in] check_word: checksum of params.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_pke_register_wait_func(const td_void *wait,
									   osal_pke_wait_timeout_interruptible wait_func, td_u32 timeout_ms, uintptr_t check_word);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
