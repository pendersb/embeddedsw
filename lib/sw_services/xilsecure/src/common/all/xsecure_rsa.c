/******************************************************************************
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_rsa.c
*
* This file contains the implementation of the interface functions for RSA
* driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   ba   10/13/14 Initial release
* 1.1   ba   12/11/15 Added support for NIST approved SHA-3 in 2.0 silicon
* 2.0   vns  03/15/17 Fixed compilation warning, and corrected SHA2 padding
*                     verification for silicon version other than 1.0
* 2.2   vns  07/06/17 Added doxygen tags
*       vns  17/08/17 Added APIs XSecure_RsaPublicEncrypt and
*                     XSecure_RsaPrivateDecrypt.As per functionality
*                     XSecure_RsaPublicEncrypt is same as XSecure_RsaDecrypt.
* 3.1   vns  11/04/18 Added support for 512, 576, 704, 768, 992, 1024, 1152,
*                     1408, 1536, 1984, 3072 key sizes, where previous verision
*                     has support only 2048 and 4096 key sizes.
* 3.2   vns  04/30/18 Added check for private RSA key decryption, such that only
*                     data to be decrypted should always be lesser than modulus
* 4.0 	arc  18/12/18 Fixed MISRA-C violations.
*       vns  21/12/18 Added RSA key zeroization after RSA operation
*       arc  03/06/19 Added input validations
*       vns  03/12/19 Modified as part of XilSecure code re-arch.
*       psl  03/26/19 Fixed MISRA-C violation
* 4.1   kal  05/20/19 Updated doxygen tags
* 4.2   kpt  01/07/20 Resolved CR-1049149,1049107,1049116,1049115,1049118
*                     and Replaced Magic Numbers with Macros
*       har  04/19/20 Removed platform specific code
* 4.3   har  06/17/20 Renamed ErrorCode as Status in XSecure_RsaPublicEncrypt
*                     and initialized with XST_FAILURE
*       har  06/17/20 Removed references to unused algorithms
*       rpo  09/01/20 Asserts are not compiled by default for secure libraries
*       rpo  09/10/20 Input validations are added
*       rpo  09/21/20 New error code added for crypto state mismatch
*       am   09/24/20 Resolved MISRA C violations
*       har  10/12/20 Addressed security review comments
* 4.5   am   11/24/20 Resolved MISRA C violations
* 4.6   gm   07/16/21 Added support for 64-bit address
*       am   09/17/21 Resolved compiler warnings
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_rsa.h"
#include "xsecure_utils.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSECURE_RSA_PUBLIC_EXPO_SIZE	(4U)	/**< Size of public key expo */

/************************** Function Prototypes ******************************/
static int XSecure_IsNonZeroBuffer(u8 *Data, const u32 Size);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function initializes a a XSecure_Rsa structure with the
 * 		default values located at a 64-bit address required for
 * 		operating the RSA cryptographic engine
 *
 * @param	InstancePtr - Pointer to the XSecure_Rsa instance
 * @param	Mod	    - Address of the key Modulus of key size
 * @param	ModExt	    - Address of the pre-calculated exponential
 *			      (R^2 Mod N) value
 *			    - 0 - if user doesn't have pre-calculated
 *				     R^2 Mod N value, control will take care
 *				     of this calculation internally
 * @param	ModExpo	    - Address of the buffer which contains key exponent
 *
 * @return
 *	-	XST_SUCCESS - If initialization was successful
 *	-	XSECURE_RSA_INVALID_PARAM - On invalid arguments
 *
 * @note	`Modulus`, `ModExt` and `ModExpo` are part of partition signature
 * 		 when authenticated boot image is generated by bootgen, else the
 *		 all of them should be extracted from the key
 *
 ******************************************************************************/
int XSecure_RsaInitialize_64Bit(XSecure_Rsa *InstancePtr, u64 Mod, u64 ModExt,
				u64 ModExpo)
{
	int Status = XST_FAILURE;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (Mod == 0x00U) || (ModExpo == 0x00U)) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	Status = (int)XSecure_RsaCfgInitialize(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	InstancePtr->SizeInWords = XSECURE_RSA_4096_SIZE_WORDS;
	InstancePtr->RsaState = XSECURE_RSA_INITIALIZED;

#ifdef versal
	InstancePtr->ModAddr = Mod;
	InstancePtr->ModExtAddr = ModExt;
	InstancePtr->ModExpoAddr = ModExpo;
#else
	InstancePtr->Mod = (u8 *)(UINTPTR)Mod;
	InstancePtr->ModExt = (u8 *)(UINTPTR)ModExt;
	InstancePtr->ModExpo = (u8 *)(UINTPTR)ModExpo;
#endif

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes a a XSecure_Rsa structure with the
 * 		default values required for operating the RSA cryptographic engine
 *
 * @param	InstancePtr - Pointer to the XSecure_Rsa instance
 * @param	Mod	    - A character Pointer which contains the key
 *			      Modulus of key size
 * @param	ModExt	    - A Pointer to the pre-calculated exponential
 *			      (R^2 Mod N) value
 *			    - NULL - if user doesn't have pre-calculated
 *				     R^2 Mod N value, control will take care
 *				     of this calculation internally
 * @param	ModExpo	    - Pointer to the buffer which contains key exponent
 *
 * @return
 *	-	XST_SUCCESS - If initialization was successful
 *	-	XSECURE_RSA_INVALID_PARAM - On invalid arguments
 *
 * @note	`Modulus`, `ModExt` and `ModExpo` are part of partition signature
 * 		 when authenticated boot image is generated by bootgen, else the
 *		 all of them should be extracted from the key
 *
 ******************************************************************************/
int XSecure_RsaInitialize(XSecure_Rsa *InstancePtr, u8 *Mod, u8 *ModExt,
				u8 *ModExpo)
{
	return XSecure_RsaInitialize_64Bit(InstancePtr, (u64)(UINTPTR)Mod,
			(u64)(UINTPTR)ModExt, (u64)(UINTPTR)ModExpo);
}

/*****************************************************************************/
/**
 * @brief	This function verifies the RSA decrypted data located at a
 *  		64-bit address provided is either matching with the provided
 *  		expected hash by taking care of PKCS padding
 *
 * @param	Signature - Address of the buffer which holds the decrypted
 *			    RSA signature
 * @param	Hash	  - Address of the buffer which has the hash calculated
 *		 	    on the data to be authenticated
 * @param	HashLen	  - Length of Hash used
 *			  - For SHA3 it should be 48 bytes
 *
 * @return
 *	-	XST_SUCCESS - If decryption was successful
 *	-	XSECURE_RSA_INVALID_PARAM - On invalid arguments
 *	-	XST_FAILURE - In case of mismatch
 *
 ******************************************************************************/
int XSecure_RsaSignVerification_64Bit(const u64 Signature, const u64 Hash,
	u32 HashLen)
{
	int Status = XST_FAILURE;
	const u8 *Tpadding = (u8 *)XNULL;
	u32 PadLength;
	volatile u32 SignIndex;
	u64 PadIndex = Signature;

	/* Validate the input arguments */
	if ((Signature == 0x00U) || (Hash == 0x00U) ||
		(HashLen != XSECURE_HASH_TYPE_SHA3)) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto ENDF;
	}
	PadLength = XSECURE_FSBL_SIG_SIZE - XSECURE_RSA_BYTE_PAD_LENGTH
				- XSECURE_RSA_T_PAD_LENGTH - HashLen;

	Tpadding = XSecure_RsaGetTPadding();

	/*
	 * Re-Create PKCS#1v1.5 Padding
	* MSB  ------------------------------------------------------------LSB
	* 0x0 || 0x1 || 0xFF(for 202 bytes) || 0x0 || T_padding || SHA384 Hash
	*/

	if (XSECURE_RSA_BYTE_PAD1 != XSecure_InByte64(PadIndex)) {
		goto ENDF;
	}
	PadIndex++;

	if (XSECURE_RSA_BYTE_PAD2 != XSecure_InByte64(PadIndex)) {
		goto ENDF;
	}
	PadIndex++;

	for (SignIndex = 0U; SignIndex < PadLength; SignIndex++) {
		if (XSECURE_RSA_BYTE_PAD3 != XSecure_InByte64(PadIndex)) {
			goto ENDF;
		}
		PadIndex++;
	}

	if (XSECURE_RSA_BYTE_PAD1 != XSecure_InByte64(PadIndex)) {
		goto ENDF;
	}
	PadIndex++;

	for (SignIndex = 0U; SignIndex < XSECURE_RSA_T_PAD_LENGTH; SignIndex++) {
		if (XSecure_InByte64(PadIndex) != Tpadding[SignIndex]) {
			goto ENDF;
		}
		PadIndex++;
	}

	for (SignIndex = 0U; SignIndex < HashLen; SignIndex++) {
		if (XSecure_InByte64(PadIndex) != XSecure_InByte64((Hash +SignIndex))) {
			goto ENDF;
		}
		PadIndex++;
	}

	if (SignIndex == HashLen) {
		Status = XST_SUCCESS;
	}

ENDF:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function verifies the RSA decrypted data provided is either
 *  		matching with the provided expected hash by taking care of PKCS
 *		padding
 *
 * @param	Signature - Pointer to the buffer which holds the decrypted
 *			    RSA signature
 * @param	Hash	  - Pointer to the buffer which has the hash calculated
 *		 	    on the data to be authenticated
 * @param	HashLen	  - Length of Hash used
 *			  - For SHA3 it should be 48 bytes
 *
 * @return
 *	-	XST_SUCCESS - If decryption was successful
 *	-	XSECURE_RSA_INVALID_PARAM - On invalid arguments
 *	-	XST_FAILURE - In case of mismatch
 *
 ******************************************************************************/
int XSecure_RsaSignVerification(const u8 *Signature, const u8 *Hash,
	u32 HashLen)
{
	return XSecure_RsaSignVerification_64Bit((u64)(UINTPTR)Signature,
			(u64)(UINTPTR)Hash, HashLen);
}

/*****************************************************************************/
/**
 * @brief	This function handles the RSA encryption for data available at
 *  		64-bit address with the public key components provided when
 *  		initializing the RSA cryptographic core with the
 *  		XSecure_RsaInitialize function
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance
 * @param	Input		Address of the buffer which contains the input
 *				  data to be encrypted
 * @param	Size		Key size in bytes, Input size also should be
 * 				  same as Key size mentioned. Inputs supported are
 * 				- XSECURE_RSA_4096_KEY_SIZE
 * 				- XSECURE_RSA_2048_KEY_SIZE
 * 				- XSECURE_RSA_3072_KEY_SIZE
 * @param	Result		Address of buffer where resultant decrypted
 *				  data to be stored
 *
 * @return
 *	-	XST_SUCCESS - If encryption was successful
 *	-	XSECURE_RSA_INVALID_PARAM - On invalid arguments
 *	-	XSECURE_RSA_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *
 * @note	The Size passed here needs to match the key size used in the
 * 		XSecure_RsaInitialize function
 *
******************************************************************************/
int XSecure_RsaPublicEncrypt_64Bit(XSecure_Rsa *InstancePtr, u64 Input,
	u32 Size, u64 Result)
{
	int Status = XST_FAILURE;
	u64 ModExpoAddr;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (Result == 0x00U) || (Input == 0x00U) ||
		(Size == 0x00U)) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->RsaState != XSECURE_RSA_INITIALIZED) {
		Status = (int)XSECURE_RSA_STATE_MISMATCH_ERROR;
		goto END;
	}

#ifdef versal
	ModExpoAddr = InstancePtr->ModExpoAddr;
	Status = XSecure_IsNonZeroBuffer((u8 *)(UINTPTR)ModExpoAddr, XSECURE_RSA_PUBLIC_EXPO_SIZE);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}
	Status = XSecure_RsaOperation(InstancePtr, Input, Result,
			XSECURE_RSA_SIGN_ENC, Size);
#else
	ModExpoAddr = (u64)(UINTPTR)(InstancePtr->ModExpo);
	Status = XSecure_IsNonZeroBuffer((u8 *)(UINTPTR)ModExpoAddr, XSECURE_RSA_PUBLIC_EXPO_SIZE);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}
	Status = (int)XSecure_RsaOperation(InstancePtr, (u8 *)(UINTPTR)Input,
			(u8 *)(UINTPTR)Result, XSECURE_RSA_SIGN_ENC, Size);
#endif


END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handles the RSA encryption with the public key
 * 		components provided when initializing the RSA cryptographic core
 * 		with the XSecure_RsaInitialize function
 *
 * @param	InstancePtr	Pointer to the XSecure_Rsa instance
 * @param	Input		Pointer to the buffer which contains the input
 *				  data to be encrypted
 * @param	Size		Key size in bytes, Input size also should be
 * 				  same as Key size mentioned. Inputs supported are
 * 				- XSECURE_RSA_4096_KEY_SIZE
 * 				- XSECURE_RSA_2048_KEY_SIZE
 * 				- XSECURE_RSA_3072_KEY_SIZE
 * @param	Result		Pointer to the buffer where resultant decrypted
 *				  data to be stored
 *
 * @return
 *	-	XST_SUCCESS - If encryption was successful
 *	-	XSECURE_RSA_INVALID_PARAM - On invalid arguments
 *	-	XSECURE_RSA_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *
 * @note	The Size passed here needs to match the key size used in the
 * 		XSecure_RsaInitialize function
 *
******************************************************************************/
int XSecure_RsaPublicEncrypt(XSecure_Rsa *InstancePtr, u8 *Input,
	u32 Size, u8 *Result)
{
	return XSecure_RsaPublicEncrypt_64Bit(InstancePtr, (u64)(UINTPTR)Input,
			Size, (u64)(UINTPTR)Result);
}

/*****************************************************************************/
/**
 * @brief	This function handles the RSA decryption for data available at
 *  		64-bit address with the private key components provided when
 *  		initializing the RSA cryptographic core with the
 *  		XSecure_RsaInitialize function
 *
 * @param	InstancePtr	- Pointer to the XSecure_Rsa instance
 * @param	Input		- Address of the buffer which contains the input
 *				  data to be decrypted
 * @param	Size		- Key size in bytes, Input size also should be same as
 * 				  Key size mentioned. Inputs supported are
 * 				- XSECURE_RSA_4096_KEY_SIZE,
 * 				- XSECURE_RSA_2048_KEY_SIZE
 * 				- XSECURE_RSA_3072_KEY_SIZE
 * @param	Result		- Address of buffer where resultant decrypted
 *				  data to be stored
 *
 * @return
 *	-	XST_SUCCESS - If decryption was successful
 *	-	XSECURE_RSA_DATA_VALUE_ERROR - If input data is greater than modulus
 *	-	XSECURE_RSA_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *	-	XST_FAILURE - On RSA operation failure
 *
 * @note	The Size passed here needs to match the key size used in the
 *  		XSecure_RsaInitialize function
 *
******************************************************************************/
int XSecure_RsaPrivateDecrypt_64Bit(XSecure_Rsa *InstancePtr, u64 Input,
	u32 Size, u64 Result)
{
	int Status = (int)XSECURE_RSA_DATA_VALUE_ERROR;
	u32 Idx;
	u32 InputData;
	u32 ModData;
	u64 ModAddr;
	u64 ModExpoAddr;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (Result == 0x00U) || (Input == 0x00U) ||
		(Size == 0x00U)) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->RsaState != XSECURE_RSA_INITIALIZED) {
		Status = (int)XSECURE_RSA_STATE_MISMATCH_ERROR;
		goto END;
	}

#ifdef versal
	ModAddr = InstancePtr->ModAddr;
	ModExpoAddr = InstancePtr->ModExpoAddr;
#else
	ModAddr = (u64)(UINTPTR)(InstancePtr->Mod);
	ModExpoAddr = (u64)(UINTPTR)(InstancePtr->ModExpo);
#endif

	Status = XSecure_IsNonZeroBuffer((u8 *)(UINTPTR)ModExpoAddr, Size);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}
	/*
	 * Input data should always be smaller than modulus
	 * One byte is being checked at a time to make sure the input data
	 * is smaller than the modulus
	 */
	for (Idx = 0U; Idx < Size; Idx++) {
		ModData = XSecure_InByte64((ModAddr + Idx));
		InputData = XSecure_InByte64((Input + Idx));
		if (ModData > InputData) {
#ifdef versal
			Status = XSecure_RsaOperation(InstancePtr, Input,
					Result, XSECURE_RSA_SIGN_DEC, Size);
#else
			Status = (int)XSecure_RsaOperation(InstancePtr,
					(u8 *)(UINTPTR)Input,
					(u8 *)(UINTPTR)Result,
					XSECURE_RSA_SIGN_DEC,
					Size);
#endif
			break;
		}

		if (ModData < InputData) {
			break;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handles the RSA decryption with the private key components
 * 		provided when initializing the RSA cryptographic core with the
 * 		XSecure_RsaInitialize function
 *
 * @param	InstancePtr	- Pointer to the XSecure_Rsa instance
 * @param	Input		- Pointer to the buffer which contains the input
 *				  data to be decrypted
 * @param	Size		- Key size in bytes, Input size also should be same as
 * 				  Key size mentioned. Inputs supported are
 * 				- XSECURE_RSA_4096_KEY_SIZE,
 * 				- XSECURE_RSA_2048_KEY_SIZE
 * 				- XSECURE_RSA_3072_KEY_SIZE
 * @param	Result		- Pointer to the buffer where resultant decrypted
 *				  data to be stored
 *
 * @return
 *	-	XST_SUCCESS - If decryption was successful
 *	-	XSECURE_RSA_DATA_VALUE_ERROR - If input data is greater than modulus
 *	-	XSECURE_RSA_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *	-	XST_FAILURE - On RSA operation failure
 *
 * @note	The Size passed here needs to match the key size used in the
 *  		XSecure_RsaInitialize function
 *
******************************************************************************/
int XSecure_RsaPrivateDecrypt(XSecure_Rsa *InstancePtr, u8 *Input,
	u32 Size, u8 *Result)
{
	return XSecure_RsaPrivateDecrypt_64Bit(InstancePtr, (u64)(UINTPTR)Input,
			Size, (u64)(UINTPTR)Result);
}

/*****************************************************************************/
/**
 * @brief	This function checks if the data in the provided buffer is non-zero
 *
 * @param	Pointer to the buffer
 * @param	Size of the buffer
 *
 * @return
 *		 - XST_SUCCESS  In case of non-zero buffer
 *		 - XST_FAILURE  In case the buffer is all zeroes
 *
 *****************************************************************************/
static int XSecure_IsNonZeroBuffer(u8 *Data, const u32 Size)
{
	int Status = XST_FAILURE;
	u32 Index;

	for (Index = 0U; Index < Size; Index++) {
		if (Data[Index] != 0x00U) {
			Status = XST_SUCCESS;
			break;
		}
	}

	return Status;
}
#endif
