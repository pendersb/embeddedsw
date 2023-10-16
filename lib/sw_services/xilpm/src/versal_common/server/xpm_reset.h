/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_RESET_H_
#define XPM_RESET_H_

#include "xpm_node.h"
#include "xpm_common.h"
#include "xpm_subsystem.h"
#include "xpm_reset_plat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_RESET_PARENTS	(3U)

extern u32 UserAssertPsSrst;

/* All reset types */
typedef enum {
	XPM_RSTTYPE_POR,
	XPM_RSTTYPE_SYS,
	XPM_RSTTYPE_PERIPH,
	XPM_RSTTYPE_DBG,
}XPm_ResetType;


/* All reset Ops types */
typedef enum {
	XPM_RSTOPS_GENRERIC=1,
	XPM_RSTOPS_CUSTOM,
	XPM_RSTOPS_MAX,
}XPm_ResetOpsType;

typedef enum XPmResetActions XPm_ResetActions;
typedef struct XPm_ResetNode XPm_ResetNode;
typedef struct XPm_ResetHandle XPm_ResetHandle;

/**
 * xPmResetOps - Reset operations
 * @SetState	Assert or release reset line
 * @GetState	Get current status of reset line
 */
typedef struct XPmResetOps {
	XStatus (*const SetState)(XPm_ResetNode *Rst, const u32 Action);
	u32 (*const GetState)(const XPm_ResetNode *Rst);
} XPm_ResetOps;


/**
 * XPm_ResetHandle - This models reset/device pair.
 */
struct XPm_ResetHandle {
	XPm_ResetNode *Reset; /**< Reset used by device */
	struct XPm_DeviceNode *Device; /**< Device which uses the reset */
	XPm_ResetHandle *NextReset; /**< Next handle of same device */
	XPm_ResetHandle *NextDevice; /**< Next handle of same reset */
};

/**
 * The reset class.	 This is the base class for all the reset nodes.
 */
struct XPm_ResetNode {
	XPm_Node Node;
	u16 Parents[MAX_RESET_PARENTS]; /**< List of Parent Reset Index */
	uint8_t Shift;
	uint8_t Width;
	XPm_ResetOps *Ops;
	XPm_ResetHandle *RstHandles; /**< Pointer to the reset/device pairs */
	u32 AllowedSubsystems;
};

struct ResetCustomOps {
	u32 ResetIdx;
	XStatus (*const ActionAssert)(const XPm_ResetNode *Rst);
	XStatus (*const ActionRelease)(const XPm_ResetNode *Rst);
	XStatus (*const ActionPulse)(const XPm_ResetNode *Rst);
	u32 (*const GetStatus)(void);
};

#define MAX_RESETS	XPM_NODEIDX_RST_MAX

#define XPM_RST_STATE_DEASSERTED 0U
#define XPM_RST_STATE_ASSERTED 1U

#define RESET_PERM_SHIFT_NS	(0U)
#define RESET_PERM_SHIFT_S	(0U + MAX_NUM_SUBSYSTEMS)

/************************** Function Prototypes ******************************/

XStatus XPmReset_AddNode(u32 Id, u32 ControlReg, u8 Shift, u8 Width, u8 ResetType, u8 NumParents, const u32* Parents);
XPm_ResetNode* XPmReset_GetById(u32 ResetId);
XStatus XPmReset_AssertbyId(u32 ResetId, const u32 Action);
XStatus XPmReset_CheckPermissions(const XPm_Subsystem *Subsystem, u32 ResetId);
XStatus XPmReset_SystemReset(void);
XStatus XPmReset_IsPermissionReset(const u32 ResetId);
XStatus XPmReset_AddPermForGlobalResets(const XPm_Subsystem *Subsystem);
XStatus XPmReset_IsOperationAllowed(const u32 SubsystemId,
				    const XPm_ResetNode *Rst,
				    const u32 CmdType);
XStatus XPmReset_AddPermission(XPm_ResetNode *Rst,
			       const XPm_Subsystem *Subsystem,
			       const u32 Operations);
void XPmReset_MakeCpmPorResetCustom(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_RESET_H_ */
