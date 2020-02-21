/*! *****************************************************************************
 * @file:    adi_pkstor.c
 * @brief:   PKSTOR source file (part of CRYPTO device driver).
 * @details: This is the Crypto PKSTOR driver implementation file.
 -----------------------------------------------------------------------------
Copyright (c) 2010-2016 Analog Devices, Inc.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Modified versions of the software must be conspicuously marked as such.
  - This software is licensed solely and exclusively for use with processors
    manufactured by or for Analog Devices, Inc.
  - This software may not be combined or merged with other code in any manner
    that would cause the software to become subject to terms and conditions
    which differ from those listed here.
  - Neither the name of Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.
  - The use of this software may or may not infringe the patent rights of one
    or more patent holders.  This license does not release you from the
    requirement that you obtain separate licenses from these patent holders
    to use this software.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-
INFRINGEMENT, TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF
CLAIMS OF INTELLECTUAL PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/

#if (defined (__ADUCM4x50__) && (1 == ADI_CRYPTO_ENABLE_PKSTOR_SUPPORT))


/** @addtogroup Crypto_Driver_PKSTOR PKSTOR
 *  @ingroup Crypto_Driver
 *  @{
 *
 *  @brief <b>Crypto Driver PKSTOR Extensions</b>
 *
 * @details
 *
 * Protected Key Storage (PKSTOR) is an extension of the Crypto Driver and
 * provides safe storage and management of secure keys and using Crypto AES
 * encryption.
 *
 * PKSTOR flash storage space consists of two reserved 2KB pages of internal
 * flash memory that provides protected (encrypted) key storage of up to 51
 * user keys per page of flash memory (102 keys in total).  Each key record spans
 * 5 64-bit DWORDS (320 bits per record) and supports storage of either 128-bit
 * or 256-bit keys (512-bit key storage is not supported).  In addition, each key
 * record also provisions for a dedicated 64-bit verification string that allows
 * validation of recovered keys that have been decrypted (the verification strings
 * are encrypted and decrypted with the rest of each key record).
 *
 * Each 320-bit key record is identified by a Page and Index value.  Key indexing
 * uses a 7-bit addressing scheme, of which the MSB acts as page selector, and the
 * lower 6-bits act as a (5-DWORD) key record index, e.g. index value 0x47 would
 * designate the key record stored in PKSTOR flash memory page 1, index 7.
 *
 * - PKSTOR features are activated by static configuration macro:
 * #ADI_CRYPTO_ENABLE_PKSTOR_SUPPORT, which must be set to use PKSTOR extensions.
 *
 * - PKSTOR operations require PKSTOR extensions to be dynamically enabled and
 * disabled (#adi_crypto_pk_EnablePKSTOR()) for every sequence of PKSTOR operations,
 * during which time normal Crypto operations are suspended.  The PKSTOR extensions
 * must be disabled again before resuming normal Crypto operations.
 *
 * - PKSTOR operations are blocking and return upon completion.
 *
 * - PKSTOR hardware shares the Cortex Advanced Peripheral Bus (APB) with the
 * Flash Controller peripheral for access to the dedicated PKSTOR flash pages,
 * and so these two peripherals may not be used concurrently.
 */


/*========  I N C L U D E  ========*/

/* This source file is meant to be included by adi_crypto.c CRYPTO device driver.
   It is not meant to be compiled stand-alone.  It depends on includes provided
   by the CRYPTO device driver source file.
*/


/* The entire content of this file is dedicated to supporting PKSTOR functionality,
   and so it is all bracketed by a single ADI_CRYPTO_ENABLE_PKSTOR_SUPPORT macro.
*/

/*! \cond PRIVATE */

/* common PKSTOR command dispatcher */
static ADI_CRYPTO_RESULT pkCmd (ADI_CRYPTO_HANDLE const hDevice, ADI_CRYPTO_PK_CMD const cmd, uint8_t const index);

/*! \endcond */



/**
 * @brief    Enable/Disable PKSTOR.  Enabling PKSTOR is required to access all other PKSTOR APIs.
 *
 * @param [in]  hDevice   Handle to the CRYPTO driver device instance.
 * @param [in]  bEnable   'true' to enable and 'false' to disable PKSTOR.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_ALREADY_ENABLED       Error: PKSTOR mode already enabled.
 *           - #ADI_CRYPTO_PK_ALREADY_DISABLED      Error: PKSTOR mode already disabled.
 *
 * Enabling PKSTOR places the CRYPTO driver into a special mode during which PKSTOR operations
 * are enabled and normal Crypto operations are suspended.  Such operations typically consist of
 * encrypting user keys and programming them into reserved internal flash pages dedicated to PKSTOR.
 * The PKSTOR mode must also be enabled for fetching encrypted keys and decrypting them.
 *
 * User meta-data supplies key indexes, key sizes, expected key verification strings, etc.,
 * all of which must be configured (using the balance of the PKSTOR API) for each sequence of
 * PKSTOR operations.
 *
 * Normal CRYPTO mode is resumed when PKSTOR mode is disabled.
 */
ADI_CRYPTO_RESULT adi_crypto_pk_EnablePKSTOR (ADI_CRYPTO_HANDLE const hDevice, bool const bEnable)
{
    static uint32_t savedConfig;
    static bool bEnableState = false;

#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
#endif /* ADI_DEBUG */

    /* reconfigure Crypto Config Register for PKSTOR operation */
    if (bEnable) {
        if (true == bEnableState) {
            return ADI_CRYPTO_PK_ALREADY_ENABLED;
        } else {
            savedConfig = hDevice->pDev->CFG;
            hDevice->pDev->CFG = PK_CONFIG_BITS;
            bEnableState = true;
        }
    } else {
        if (true == bEnableState) {
            hDevice->pDev->CFG = savedConfig;
            bEnableState = false;
        } else {
            return ADI_CRYPTO_PK_ALREADY_DISABLED;
        }
    }

    return ADI_CRYPTO_SUCCESS;
}


/**
 * @brief    Store KUW key verification string prior to "wrapping" a key.
 *
 * @param [in]  hDevice   Handle to the CRYPTO driver device instance.
 * @param [in]  pValStr   Pointer to an array of eight uint8_t string values.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *
 * The process of wrapping keys is invisible to the user as both the AES and KUW register
 * sets within the CRYPTO device are "write-only" with no visible evidence of actual key
 * values (encrypted or otherwise).
 *
 * The validation strings are encrypted and decrypted in parallel with any/all key encryption
 * and decryption operations and provide the ability to verify that key encryption/decryption
 * (and key store/retrieve from PKSTOR flash) is being performed correctly even though the key
 * registers themselves are never directly observable.  The validation string registers should
 * match perfectly through all such transformations.
 *
 * The validation string registers are also normally "write-only", but are momentarily readable
 * following a key unwrap operation.  Subsequent PKSTOR operations clear the string registers.
 *
 * @note String validation may be applied to back-to-back wrap and unwrap operations without
 * actually storing the wrapped key at all.  This may be helpful in debugging PKSTOR operations.
 *
 * @sa adi_crypto_pk_GetValString().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_SetValString (ADI_CRYPTO_HANDLE const hDevice, uint8_t * const pValStr)
{
    uint32_t volatile *pReg;
    uint8_t *pData8;

#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* store string into validation registers */
    pReg = &hDevice->pDev->KUWVALSTR1;
    pData8 = pValStr;
    for (uint32_t count = 0u; count < NUM_PKSTOR_VAL_STRING_WORDS; count++, pReg++) {
        *pReg = u32FromU8p(pData8);
        pData8 += sizeof(uint32_t);
    }

    return ADI_CRYPTO_SUCCESS;
}


/**
 * @brief    Retrieve a KUW key verification string after "unwrapping" a key.
 *
 * @param [in]  hDevice   Handle to the CRYPTO driver device instance.
 * @param [out] pValStr   Pointer to an array of eight uint8_t string values.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *
 * The inverse operation of #adi_crypto_pk_SetValString().  This API is only appropriate
 * following a adi_crypto_pk_UnwrapKuwReg() call, when the validation string registers
 * are readable.
 *
 * @sa adi_crypto_pk_SetValString().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_GetValString (ADI_CRYPTO_HANDLE const hDevice, uint8_t * const pValStr)
{
    uint32_t volatile *pReg;
    uint32_t data32;
    uint8_t *pData8;

#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* copy string out of validation registers */
    pData8 = pValStr;
    pReg = &hDevice->pDev->KUWVALSTR1;
    for (uint32_t count = 0u; count < NUM_PKSTOR_VAL_STRING_WORDS; count++, pReg++) {
        data32 = *pReg;
        *pData8 = (uint8_t)((data32 & 0x000000ff) >>  0);    pData8++;
        *pData8 = (uint8_t)((data32 & 0x0000ff00) >>  8);    pData8++;
        *pData8 = (uint8_t)((data32 & 0x00ff0000) >> 16);    pData8++;
        *pData8 = (uint8_t)((data32 & 0xff000000) >> 24);    pData8++;
    }

    return ADI_CRYPTO_SUCCESS;
}


/**
 * @brief    Sets the KUW key width for a sequence of PKSTOR operations.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 * @param [in]  kuwDataLen KUW key width.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: KUW length is invalid.
 *
 * Most PKSTOR operations act on keys that pass through the KUW register set.  All such operations
 * require the KUW length to be designated once, ahead of time.
 *
 * @note Setting the KUW length between KUW operations is not recommended as this resets the KUW
 * registers.
 *
 * @sa adi_crypto_pk_SetValString().
 * @sa adi_crypto_pk_SetKuwReg().
 * @sa adi_crypto_pk_WrapKuwReg().
 * @sa adi_crypto_pk_StoreKey().
 * @sa adi_crypto_pk_RetrieveKey().
 * @sa adi_crypto_pk_UnwrapKuwReg().
 * @sa adi_crypto_pk_GetValString().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_SetKuwLen (ADI_CRYPTO_HANDLE const hDevice, ADI_CRYPTO_PK_KUW_LEN const kuwDataLen)
{
#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result = ADI_CRYPTO_SUCCESS;
    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
   /* Note: 512-bit keys are not storable in flash... it is just here for HMAC support, so most other PKSTOR command exempt it... */
    if ((ADI_PK_KUW_LEN_128 != kuwDataLen) && (ADI_PK_KUW_LEN_256 != kuwDataLen) && (ADI_PK_KUW_LEN_512 != kuwDataLen)) {
        return ADI_CRYPTO_PK_INVALID_KUWLEN;
    }
#endif

    /* set KUW key length register field */
    CLR_BITS(hDevice->pDev->CFG, BITM_CRYPT_CFG_KUWKEYLEN);
    SET_BITS(hDevice->pDev->CFG, (uint32_t)kuwDataLen);  /* pre-shifted */

    return ADI_CRYPTO_SUCCESS;
}


/**
 * @brief    Sets the KUW key register set with (user key) data.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 * @param [in]  pKuwData   KUW key data.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: Specified KUW length is invalid.
 *
 * Transfers user data (typically a key) into the KUW register set for subsequent "wrapping"
 * and/or storing into PKSTOR flash memory.
 *
 * Requires KUW key length to have been previously specified with #adi_crypto_pk_SetKuwLen().
 *
 * @sa adi_crypto_pk_SetKuwLen().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_SetKuwReg (ADI_CRYPTO_HANDLE const hDevice, uint8_t * const pKuwData)
{
    ADI_CRYPTO_RESULT result = ADI_CRYPTO_SUCCESS;
    uint32_t volatile *pReg;
    uint8_t *pData8;
    uint32_t num32;
    uint32_t count;

#ifdef ADI_DEBUG
    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* set number of words to write to the 32-bit KUW registers
       according to currently active KUW length value */
    switch (hDevice->pDev->CFG & BITM_CRYPT_CFG_KUWKEYLEN) {
        case ADI_PK_KUW_LEN_128:
            num32 = 4u;
            break;
        case ADI_PK_KUW_LEN_256:
            num32 = 8u;
            break;
        case ADI_PK_KUW_LEN_512:
            num32 = 16u;
            break;
        default:
#ifdef ADI_DEBUG
            return ADI_CRYPTO_PK_INVALID_KUWLEN;
#else
            num32 = 0u;
            break;
#endif /* ADI_DEBUG */
    }

    /* write the specified KUW register set according to kuwDataLen */
    pReg = &hDevice->pDev->KUW0;
    pData8 = pKuwData;
    for (count = 0u; count < num32; count++, pReg++) {
        *pReg = u32FromU8p(pData8);
        pData8 += sizeof(uint32_t);
    }

    /* zero out unused KUW registers... so entire KUW register set is written */
    for (; count < 16; count++, pReg++) {
        *pReg = 0ul;
    }

    return result;
}


/**
 * @brief    Wrap (encrypt) the KUW register set using current AES key.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: Specified KUW length is invalid.
 *           - #ADI_CRYPTO_PK_CMD_FAULT             Error: PKSTOR command produced failure status.
 *           - #ADI_CRYPTO_PK_CMD_ECC_FAULT         Error: PKSTOR command produced ECC bit-error status.
 *
 * Wraps (encrypts) current content of KUW register set and validation string using current
 * AES key and AES and KUW key length settings.  Results are invisible, as the AES, KUW, and
 * KUW validation string registers remain "write-only".
 *
 * Requires KUW key length to have been previously specified with #adi_crypto_pk_SetKuwLen().
 *
 * Wrapping the KUW data (key) is done by the currently-active AES key.  The PKSTOR module
 * auto-loads the AES key with either the current KUW register (assuming an unwrapped key is
 * present in KUW), or the default device key.
 *
 * The current KUW (unwrapped) key may be auto-loaded into AES with the
 * #adi_crypto_pk_UseDecryptedKey() API.  In this case, the AES key length must be specified
 * manually from user meta-data because KUW size info is not maintained by PKSTOR when "using"
 * a decrypted key (presumably, following a retrieve and unwrap key sequence).
 *
 * The default device key may be auto-loaded into AES with the #adi_crypto_pk_LoadDeviceKey() API.
 * When loading the device key, the AES key length register is set automatically to the
 * fixed device key 256-bit size.
 *
 * This API is typically used to encrypt a key prior to storing it in PKSTOR flash.
 *
 * @note The "Device Key" is a reserved secret hardware key that is never seen.  Device keys are
 * unique to each product, but not unique across multiple devices of the same product.
 * The device key has a fixed-width of 256-bits.
 *
 * @sa adi_crypto_pk_UnwrapKuwReg().
 * @sa adi_crypto_pk_UseDecryptedKey().
 * @sa adi_crypto_pk_LoadDeviceKey().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_WrapKuwReg (ADI_CRYPTO_HANDLE const hDevice)
{
#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* issue the wrap command */
    return pkCmd(hDevice, ADI_PK_CMD_WRAP_KUW, 0);
}


/**
 * @brief    Unwrap (decrypt) the KUW register set using current AES key.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: Specified KUW length is invalid.
 *           - #ADI_CRYPTO_PK_CMD_FAULT             Error: PKSTOR command produced failure status.
 *           - #ADI_CRYPTO_PK_CMD_ECC_FAULT         Error: PKSTOR command produced ECC bit-error status.
 *
 * Unwraps (decrypts) current content of KUW register set and validation string using current
 * AES key and AES and KUW key length settings.  AES and KUW results are invisible, as the AES
 * and KUW registers remain "write-only".  However, the KUW validation string registers contain
 * the unwrapped validation string which may be used to verify the unwrap process.  But the
 * validation string registers will revert to "write-only" state following any other PKSTOR
 * operation.
 *
 * Requires KUW key length to have been previously specified with #adi_crypto_pk_SetKuwLen().
 *
 * Unwrapping the KUW data (key) is done by the currently-active AES key.  The PKSTOR module
 * auto-loads the AES key with either the current KUW register (assuming an unwrapped key is
 * present in KUW), or the default device key.
 *
 * The unwrapped KUW key may be auto-loaded into AES with the #adi_crypto_pk_UseDecryptedKey()
 * API.  In this case, the AES key length must be specified manually from user meta-data because
 * KUW size info is not maintained by PKSTOR when "using" a decrypted key (presumably, following
 * a retrieve and unwrap key sequence).
 *
 * The default device key may be auto-loaded into AES with the #adi_crypto_pk_LoadDeviceKey() API.
 * When loading the device key, the AES key length register is set automatically to the
 * fixed device key 256-bit size.
 *
 * This API is typically used to decrypt a key after retrieving it from PKSTOR flash.
 *
 * @note The "Device Key" is a reserved secret hardware key that is never seen.  Device keys are
 * unique to each product, but not unique across multiple devices of the same product.
 * The device key has a fixed-width of 256-bits.
 *
 * @sa adi_crypto_pk_UnwrapKuwReg().
 * @sa adi_crypto_pk_UseDecryptedKey().
 * @sa adi_crypto_pk_LoadDeviceKey().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_UnwrapKuwReg (ADI_CRYPTO_HANDLE const hDevice)
{
#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* issue the unwrap command */
    return pkCmd(hDevice, ADI_PK_CMD_UNWRAP_KUW, 0);
}


/**
 * @brief    Clear the KUW register set.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: Specified KUW length is invalid.
 *           - #ADI_CRYPTO_PK_CMD_FAULT             Error: PKSTOR command produced failure status.
 *           - #ADI_CRYPTO_PK_CMD_ECC_FAULT         Error: PKSTOR command produced ECC bit-error status.
 *
 * Clears (resets) the entire KUW register set.
 *
 * This API is typically used to disable a key to prevent subsequent use.  Of course, the key
 * may be recovered again if it is stored and the correct AES key is used to decrypt it.
 */
ADI_CRYPTO_RESULT adi_crypto_pk_ResetKuwReg (ADI_CRYPTO_HANDLE const hDevice)
{
#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* issue the reset KUW command */
    return pkCmd(hDevice, ADI_PK_CMD_RESET_KUW, 0);
}


/**
 * @brief    "Use" the current KUW key by transferring it to the AES register set.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: Specified KUW length is invalid.
 *           - #ADI_CRYPTO_PK_CMD_FAULT             Error: PKSTOR command produced failure status.
 *           - #ADI_CRYPTO_PK_CMD_ECC_FAULT         Error: PKSTOR command produced ECC bit-error status.
 *
 * "Use" a decrypted KUW key by transferring the KUW register set to the AES register set.
 *
 * The AES key length must be specified manually from user meta-data because KUW size info is
 * not maintained by PKSTOR when "using" a decrypted key (presumably, following a retrieve and
 * unwrap key sequence).
 *
 * This API is typically used to activate a key for subsequent PKSTOR or AES CRYPTO operations,
 * following a sequence of PKSTOR retrieve and unwrap operations.
 *
 * @note It is assumed the KUW key is properly unwrapped (decrypted) prior to "using".
 *
 * @sa adi_crypto_pk_LoadDeviceKey().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_UseDecryptedKey (ADI_CRYPTO_HANDLE const hDevice)
{
#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* issue the use decrypted key command */
    return pkCmd(hDevice, ADI_PK_CMD_USE_KEY, 0);
}


/**
 * @brief    Load the AES register set with the default device key.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: Specified KUW length is invalid.
 *           - #ADI_CRYPTO_PK_CMD_FAULT             Error: PKSTOR command produced failure status.
 *           - #ADI_CRYPTO_PK_CMD_ECC_FAULT         Error: PKSTOR command produced ECC bit-error status.
 *
 * Load the AES register set with the default device key.
 *
 * When loading the device key, the AES key length register is set automatically to the
 * fixed device key 256-bit size.
 *
 * This API is typically used to activate the default device key for subsequent PKSTOR or
 * AES CRYPTO operations.  The device key is not encrypted and need not be unwrapped.
 * *
 * @sa adi_crypto_pk_UseDecryptedKey().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_LoadDeviceKey (ADI_CRYPTO_HANDLE const hDevice)
{
#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* NOTE: "load" device key size is fixed at 256-bit key and
       AESKEYLEN should reflect this after command completes */

    /* issue the use default device key command */
    return pkCmd(hDevice, ADI_PK_CMD_USE_DEV_KEY, 0);
}


/**
 * @brief    Retrieve a key stored in PKSTOR flash.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 * @param [in]  index      PKSTOR flash key index.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: Specified KUW length is invalid.
 *           - #ADI_CRYPTO_PK_CMD_FAULT             Error: PKSTOR command produced failure status.
 *           - #ADI_CRYPTO_PK_CMD_ECC_FAULT         Error: PKSTOR command produced ECC bit-error status.
 *
 * Fetches an indexed key stored within PKSTOR flash memory into the KUW register set.
 *
 * The key may be wrapped or not, depending on what was stored.
 *
 * Requires KUW key length to have been previously specified with #adi_crypto_pk_SetKuwLen().
 *
 * This API is typically used to retrieve an indexed key from PKSTOR flash memory that was previously
 * stored.  The key will typically have been wrapped prior to storage, and will require unwrapping
 * prior to use.
 *
 * @sa adi_crypto_pk_StoreKey().
 * @sa adi_crypto_pk_SetKuwLen().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_RetrieveKey (ADI_CRYPTO_HANDLE const hDevice, uint8_t const index)
{
#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* issue the retrieve key command */
    return pkCmd(hDevice, ADI_PK_CMD_RETRIEVE_KEY, index);
}


/**
 * @brief    Program a key into PKSTOR flash at an indexed location.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 * @param [in]  index      PKSTOR flash key index.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: Specified KUW length is invalid.
 *           - #ADI_CRYPTO_PK_CMD_FAULT             Error: PKSTOR command produced failure status.
 *           - #ADI_CRYPTO_PK_CMD_ECC_FAULT         Error: PKSTOR command produced ECC bit-error status.
 *
 * Stores the current KUW key into PKSTOR flash memory at an indexed location.
 *
 * The key may be wrapped or not, depending on need.
 *
 * Requires KUW key length to have been previously specified with #adi_crypto_pk_SetKuwLen().
 *
 * This API is typically used to store a key at an indexed location within PKSTOR flash memory.
 * The key will typically have been wrapped prior to storage, and will require unwrapping
 * following retrieval.
 *
 * @sa adi_crypto_pk_RetrieveKey().
 * @sa adi_crypto_pk_SetKuwLen().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_StoreKey (ADI_CRYPTO_HANDLE const hDevice, uint8_t const index)
{
#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* issue the store key command */
    return pkCmd(hDevice, ADI_PK_CMD_STORE_KEY, index);
}


/**
 * @brief    Destroy a key within PKSTOR flash.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 * @param [in]  index      PKSTOR flash key index.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: Specified KUW length is invalid.
 *           - #ADI_CRYPTO_PK_CMD_FAULT             Error: PKSTOR command produced failure status.
 *           - #ADI_CRYPTO_PK_CMD_ECC_FAULT         Error: PKSTOR command produced ECC bit-error status.
 *
 * Permanently destroys the indexed key within PKSTOR flash memory.
 *
 * The key and key location may not be used anymore.  The entire containing flash page
 * (including all other keys sharing that page) must be erased before the destroyed key
 * location may be reused.
 *
 * This API is typically used to permanently destroy a key and prevent its further use.
 * If a key has been compromised or otherwise "retired", it is appropriate to destroy it.
 *
 * @note Once destroyed, the key location may not be used or even referenced again.  Keys
 * may not be stored to "destroyed" key locations until the entire containing page is erased
 * (which will erase all other keys within that flash page).
 *
 * @note There is a known HARDWARE ANOMALY that results in the PKSTOR "retrieve" command not
 * completing if referencing a destroyed key.  This will appear to hang the PKSTOR command
 * processor since it is typically awaiting the PKSTOR command "DONE" status.  Do not do this.
 *
 * @sa adi_crypto_pk_RetrieveKey().
 * @sa adi_crypto_pk_ErasePage().
 * @sa adi_crypto_pk_SetKuwLen().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_DestroyKey (ADI_CRYPTO_HANDLE const hDevice, uint8_t const index)
{
#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* issue the erase key command */
    return pkCmd(hDevice, ADI_PK_CMD_ERASE_KEY, index);
}


/**
 * @brief    Erase a PKSTOR flash page.
 *
 * @param [in]  hDevice    Handle to the CRYPTO driver device instance.
 * @param [in]  index      PKSTOR flash key index.
 *
 * @return   Status
 *           - #ADI_CRYPTO_SUCCESS                  API was successful.
 *           - #ADI_CRYPTO_ERR_BAD_DEV_HANDLE   [D] Error: Handle Passed is invalid.
 *           - #ADI_CRYPTO_PK_NOT_ENABLED       [D] Error: PKSTOR mode is not enabled.
 *           - #ADI_CRYPTO_PK_INVALID_KUWLEN    [D] Error: Specified KUW length is invalid.
 *           - #ADI_CRYPTO_PK_CMD_FAULT             Error: PKSTOR command produced failure status.
 *           - #ADI_CRYPTO_PK_CMD_ECC_FAULT         Error: PKSTOR command produced ECC bit-error status.
 *
 * Erase an entire PKSTOR flash page, including all 51 key records contained therein.
 *
 * In the case of erasing an entire page, only the MSB of the index value is considered.
 *
 * This API is typically used to erase an entire page of PKSTOR flash memory key storage.
 *
 * @note Page erase is the ONLY mechanism by which a “destroyed” key location may be reused.
 *
 * @sa adi_crypto_pk_DestroyKey().
 */
ADI_CRYPTO_RESULT adi_crypto_pk_ErasePage (ADI_CRYPTO_HANDLE const hDevice, uint8_t const index)
{
#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
#endif /* ADI_DEBUG */

    /* issue the erase page command */
    return pkCmd(hDevice, ADI_PK_CMD_ERASE_PAGE, index);
}


/*! \cond PRIVATE */


/* common dispatcher for simple PKSTOR commands */
static ADI_CRYPTO_RESULT pkCmd(ADI_CRYPTO_HANDLE const hDevice, ADI_CRYPTO_PK_CMD const cmd, uint8_t const index)
{
    volatile uint32_t status;

#ifdef ADI_DEBUG
    ADI_CRYPTO_RESULT result;

    if ((result = ValidateHandle(hDevice)) != ADI_CRYPTO_SUCCESS) {
        return result;
    }
    if (PK_CONFIG_BITS != (PK_CONFIG_BITS & hDevice->pDev->CFG)) {
        return ADI_CRYPTO_PK_NOT_ENABLED;
    }
    if (~BITM_CRYPT_PRKSTORCFG_KEY_INDEX & index) {
        return ADI_CRYPTO_PK_INVALID_KUWLEN;
    }
#endif /* ADI_DEBUG */

    /* clear status */
    hDevice->pDev->STAT = 0xffffffff;

    /* issue command (cmd is pre-shifted) */
    hDevice->pDev->PRKSTORCFG = cmd | (index << BITP_CRYPT_PRKSTORCFG_KEY_INDEX);

    /* wait for PKSTOR command DONE status */
    while (1) {
        status = hDevice->pDev->STAT;
        if (status & BITM_CRYPT_STAT_PRKSTOR_CMD_DONE)
            break;

        /* fault checks... */
        if (status & BITM_CRYPT_STAT_PRKSTOR_CMD_FAIL)
            return ADI_CRYPTO_PK_CMD_FAULT;

		if (status & BITM_CRYPT_STAT_PRKSTOR_RET_STATUS)
            return ADI_CRYPTO_PK_CMD_ECC_FAULT;
    }

    return ADI_CRYPTO_SUCCESS;
}


/*! \endcond */

#endif  /* ADI_CRYPTO_ENABLE_PKSTOR_SUPPORT */

/*@}*/
