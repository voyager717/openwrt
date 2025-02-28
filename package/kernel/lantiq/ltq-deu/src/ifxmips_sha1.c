/******************************************************************************
**
** FILE NAME    : ifxmips_sha1.c
** PROJECT      : IFX UEIP
** MODULES      : DEU Module for Danube
**
** DATE         : September 8, 2009
** AUTHOR       : Mohammad Firdaus
** DESCRIPTION  : Data Encryption Unit Driver
** COPYRIGHT    :       Copyright (c) 2009
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author             $Comment
** 08,Sept 2009 Mohammad Firdaus    Initial UEIP release
*******************************************************************************/
/*!
  \defgroup IFX_DEU IFX_DEU_DRIVERS
  \ingroup API
  \brief ifx deu driver module
*/

/*!
  \file	ifxmips_sha1.c
  \ingroup IFX_DEU
  \brief SHA1 encryption deu driver file
*/

/*!
  \defgroup IFX_SHA1_FUNCTIONS IFX_SHA1_FUNCTIONS
  \ingroup IFX_DEU
  \brief ifx deu sha1 functions
*/

<<<<<<< HEAD
=======

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
/* Project header */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/crypto.h>
<<<<<<< HEAD
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#endif
#include <crypto/hash.h>
=======
#include <linux/cryptohash.h>
#include <crypto/sha.h>
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
#include <crypto/internal/hash.h>
#include <linux/types.h>
#include <linux/scatterlist.h>
#include <asm/byteorder.h>

#if defined(CONFIG_DANUBE)
#include "ifxmips_deu_danube.h"
#elif defined(CONFIG_AR9)
#include "ifxmips_deu_ar9.h"
#elif defined(CONFIG_VR9) || defined(CONFIG_AR10)
#include "ifxmips_deu_vr9.h"
#else
#error "Plaform Unknwon!"
#endif

#define SHA1_DIGEST_SIZE    20
#define SHA1_HMAC_BLOCK_SIZE    64
#define HASH_START   IFX_HASH_CON

<<<<<<< HEAD
=======
static spinlock_t lock;
#define CRTCL_SECT_INIT        spin_lock_init(&lock)
#define CRTCL_SECT_START       spin_lock_irqsave(&lock, flag)
#define CRTCL_SECT_END         spin_unlock_irqrestore(&lock, flag)

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
//#define CRYPTO_DEBUG
#ifdef CRYPTO_DEBUG
extern char debug_level;
#define DPRINTF(level, format, args...) if (level < debug_level) printk(KERN_INFO "[%s %s %d]: " format, __FILE__, __func__, __LINE__, ##args);
#else
#define DPRINTF(level, format, args...)
#endif

/*
 * \brief SHA1 private structure
*/
struct sha1_ctx {
	int started;
        u64 count;
	u32 hash[5];
        u32 state[5];
        u8 buffer[64];
};

extern int disable_deudma;

<<<<<<< HEAD
/*! \fn static void sha1_transform1 (u32 *state, const u32 *in)
=======

/*! \fn static void sha1_transform (u32 *state, const u32 *in)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
 *  \ingroup IFX_SHA1_FUNCTIONS
 *  \brief main interface to sha1 hardware   
 *  \param state current state 
 *  \param in 64-byte block of input  
*/                                 
<<<<<<< HEAD
static void sha1_transform1 (struct sha1_ctx *sctx, u32 *state, const u32 *in)
=======
static void sha1_transform (struct sha1_ctx *sctx, u32 *state, const u32 *in)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
{
    int i = 0;
    volatile struct deu_hash_t *hashs = (struct deu_hash_t *) HASH_START;
    unsigned long flag;

<<<<<<< HEAD
    CRTCL_SECT_HASH_START;

    SHA_HASH_INIT;
=======
    CRTCL_SECT_START;
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

    /* For context switching purposes, the previous hash output
     * is loaded back into the output register 
    */
    if (sctx->started) {
        hashs->D1R = *((u32 *) sctx->hash + 0);
        hashs->D2R = *((u32 *) sctx->hash + 1);
        hashs->D3R = *((u32 *) sctx->hash + 2);
        hashs->D4R = *((u32 *) sctx->hash + 3);
        hashs->D5R = *((u32 *) sctx->hash + 4);
    }

    for (i = 0; i < 16; i++) {
        hashs->MR = in[i];
    };

    //wait for processing
    while (hashs->controlr.BSY) {
        // this will not take long
    }
   
    /* For context switching purposes, the output is saved into a 
     * context struct which can be used later on 
    */
    *((u32 *) sctx->hash + 0) = hashs->D1R;
    *((u32 *) sctx->hash + 1) = hashs->D2R;
    *((u32 *) sctx->hash + 2) = hashs->D3R;
    *((u32 *) sctx->hash + 3) = hashs->D4R;
    *((u32 *) sctx->hash + 4) = hashs->D5R;

    sctx->started = 1;

<<<<<<< HEAD
    CRTCL_SECT_HASH_END;
}

/*! \fn static void sha1_init1(struct crypto_tfm *tfm)
=======
    CRTCL_SECT_END;
}

/*! \fn static void sha1_init(struct crypto_tfm *tfm)
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
 *  \ingroup IFX_SHA1_FUNCTIONS
 *  \brief initialize sha1 hardware   
 *  \param tfm linux crypto algo transform  
*/                                 
<<<<<<< HEAD
static int sha1_init1(struct shash_desc *desc)
{
    struct sha1_ctx *sctx = shash_desc_ctx(desc);
    
=======
static int sha1_init(struct shash_desc *desc)
{
    struct sha1_ctx *sctx = shash_desc_ctx(desc);
    
    SHA_HASH_INIT;

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
    sctx->started = 0;
    sctx->count = 0;
    return 0;
}

/*! \fn static void sha1_update(struct crypto_tfm *tfm, const u8 *data, unsigned int len)
 *  \ingroup IFX_SHA1_FUNCTIONS
 *  \brief on-the-fly sha1 computation   
 *  \param tfm linux crypto algo transform  
 *  \param data input data  
 *  \param len size of input data  
*/                                 
static int sha1_update(struct shash_desc * desc, const u8 *data,
            unsigned int len)
{
    struct sha1_ctx *sctx = shash_desc_ctx(desc);
    unsigned int i, j;

    j = (sctx->count >> 3) & 0x3f;
    sctx->count += len << 3;

    if ((j + len) > 63) {
        memcpy (&sctx->buffer[j], data, (i = 64 - j));
<<<<<<< HEAD
        sha1_transform1 (sctx, sctx->state, (const u32 *)sctx->buffer);
        for (; i + 63 < len; i += 64) {
            sha1_transform1 (sctx, sctx->state, (const u32 *)&data[i]);
=======
        sha1_transform (sctx, sctx->state, (const u32 *)sctx->buffer);
        for (; i + 63 < len; i += 64) {
            sha1_transform (sctx, sctx->state, (const u32 *)&data[i]);
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
        }

        j = 0;
    }
    else
        i = 0;

    memcpy (&sctx->buffer[j], &data[i], len - i);
    return 0;
}

/*! \fn static void sha1_final(struct crypto_tfm *tfm, u8 *out)
 *  \ingroup IFX_SHA1_FUNCTIONS
 *  \brief compute final sha1 value   
 *  \param tfm linux crypto algo transform  
 *  \param out final md5 output value  
*/                                 
static int sha1_final(struct shash_desc *desc, u8 *out)
{
    struct sha1_ctx *sctx = shash_desc_ctx(desc);
    u32 index, padlen;
    u64 t;
    u8 bits[8] = { 0, };
    static const u8 padding[64] = { 0x80, };
<<<<<<< HEAD
    //volatile struct deu_hash_t *hashs = (struct deu_hash_t *) HASH_START;
    //unsigned long flag;
=======
    volatile struct deu_hash_t *hashs = (struct deu_hash_t *) HASH_START;
    unsigned long flag;
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

    t = sctx->count;
    bits[7] = 0xff & t;
    t >>= 8;
    bits[6] = 0xff & t;
    t >>= 8;
    bits[5] = 0xff & t;
    t >>= 8;
    bits[4] = 0xff & t;
    t >>= 8;
    bits[3] = 0xff & t;
    t >>= 8;
    bits[2] = 0xff & t;
    t >>= 8;
    bits[1] = 0xff & t;
    t >>= 8;
    bits[0] = 0xff & t;

    /* Pad out to 56 mod 64 */
    index = (sctx->count >> 3) & 0x3f;
    padlen = (index < 56) ? (56 - index) : ((64 + 56) - index);
    sha1_update (desc, padding, padlen);

    /* Append length */
    sha1_update (desc, bits, sizeof bits);

<<<<<<< HEAD
    memcpy(out, sctx->hash, SHA1_DIGEST_SIZE);
=======
    CRTCL_SECT_START;

    *((u32 *) out + 0) = hashs->D1R;
    *((u32 *) out + 1) = hashs->D2R;
    *((u32 *) out + 2) = hashs->D3R;
    *((u32 *) out + 3) = hashs->D4R;
    *((u32 *) out + 4) = hashs->D5R;

    CRTCL_SECT_END;
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)

    // Wipe context
    memset (sctx, 0, sizeof *sctx);

    return 0;
}

/* 
 * \brief SHA1 function mappings
*/
static struct shash_alg ifxdeu_sha1_alg = {
        .digestsize     =       SHA1_DIGEST_SIZE,
<<<<<<< HEAD
        .init           =       sha1_init1,
=======
        .init           =       sha1_init,
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
        .update         =       sha1_update,
        .final          =       sha1_final,
        .descsize       =       sizeof(struct sha1_ctx),
        .statesize      =       sizeof(struct sha1_state),
        .base           =       {
                .cra_name       =       "sha1",
                .cra_driver_name=       "ifxdeu-sha1",
                .cra_priority   =       300,
<<<<<<< HEAD
                .cra_flags      =       CRYPTO_ALG_TYPE_HASH | CRYPTO_ALG_KERN_DRIVER_ONLY,
=======
                .cra_flags      =       CRYPTO_ALG_TYPE_HASH,
>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
                .cra_blocksize  =       SHA1_HMAC_BLOCK_SIZE,
                .cra_module     =       THIS_MODULE,
        }
};


/*! \fn int ifxdeu_init_sha1 (void)
 *  \ingroup IFX_SHA1_FUNCTIONS
 *  \brief initialize sha1 driver    
*/                                 
int ifxdeu_init_sha1 (void)
{
    int ret = -ENOSYS;


    if ((ret = crypto_register_shash(&ifxdeu_sha1_alg)))
        goto sha1_err;

<<<<<<< HEAD
=======
    CRTCL_SECT_INIT;

>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
    printk (KERN_NOTICE "IFX DEU SHA1 initialized%s.\n", disable_deudma ? "" : " (DMA)");
    return ret;

sha1_err:
    printk(KERN_ERR "IFX DEU SHA1 initialization failed!\n");
    return ret;
}

/*! \fn void ifxdeu_fini_sha1 (void)
 *  \ingroup IFX_SHA1_FUNCTIONS
 *  \brief unregister sha1 driver   
*/                                 
void ifxdeu_fini_sha1 (void)
{
    crypto_unregister_shash(&ifxdeu_sha1_alg);


}
<<<<<<< HEAD
=======


>>>>>>> 712839d4c6 (Removed unwanted submodules from index)
