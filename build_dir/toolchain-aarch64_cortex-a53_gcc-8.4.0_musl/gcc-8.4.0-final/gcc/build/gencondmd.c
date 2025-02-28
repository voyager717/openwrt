/* Generated automatically by the program `genconditions' from the target
   machine description file.  */

#define IN_TARGET_CODE 1
#include "bconfig.h"
#define INCLUDE_STRING
#include "system.h"

/* It is necessary, but not entirely safe, to include the headers below
   in a generator program.  As a defensive measure, don't do so when the
   table isn't going to have anything in it.  */
#if GCC_VERSION >= 3001

/* Do not allow checking to confuse the issue.  */
#undef CHECKING_P
#define CHECKING_P 0
#undef ENABLE_TREE_CHECKING
#undef ENABLE_RTL_CHECKING
#undef ENABLE_RTL_FLAG_CHECKING
#undef ENABLE_GC_CHECKING
#undef ENABLE_GC_ALWAYS_COLLECT
#define USE_ENUM_MODES

#include "coretypes.h"
#include "tm.h"
#include "insn-constants.h"
#include "rtl.h"
#include "memmodel.h"
#include "tm_p.h"
#include "hard-reg-set.h"
#include "function.h"
#include "emit-rtl.h"

/* Fake - insn-config.h doesn't exist yet.  */
#define MAX_RECOG_OPERANDS 10
#define MAX_DUP_OPERANDS 10
#define MAX_INSNS_PER_SPLIT 5

#include "regs.h"
#include "recog.h"
#include "output.h"
#include "flags.h"
#include "hard-reg-set.h"
#include "predict.h"
#include "basic-block.h"
#include "bitmap.h"
#include "df.h"
#include "resource.h"
#include "diagnostic-core.h"
#include "reload.h"
#include "tm-constrs.h"

#include "except.h"

/* Dummy external declarations.  */
extern rtx_insn *insn;
extern rtx ins1;
extern rtx operands[];

#endif /* gcc >= 3.0.1 */

/* Structure definition duplicated from gensupport.h rather than
   drag in that file and its dependencies.  */
struct c_test
{
  const char *expr;
  int value;
};

/* This table lists each condition found in the machine description.
   Each condition is mapped to its truth value (0 or 1), or -1 if that
   cannot be calculated at compile time.
   If we don't have __builtin_constant_p, or it's not acceptable in array
   initializers, fall back to assuming that all conditions potentially
   vary at run time.  It works in 3.0.1 and later; 3.0 only when not
   optimizing.  */

#if GCC_VERSION >= 3001
static const struct c_test insn_conditions[] = {

#line 4964 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT\n\
   && IN_RANGE (aarch64_fpconst_pow_of_2 (operands[2]), 1,\n\
		GET_MODE_BITSIZE (DImode))",
    __builtin_constant_p 
#line 4964 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT
   && IN_RANGE (aarch64_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (DImode)))
    ? (int) 
#line 4964 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT
   && IN_RANGE (aarch64_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (DImode)))
    : -1 },
#line 328 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_adjust_ok_for_ldpstp (operands, false, SImode)",
    __builtin_constant_p 
#line 328 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, false, SImode))
    ? (int) 
#line 328 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, false, SImode))
    : -1 },
#line 1879 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_zero_extend_const_eq (DImode, operands[3],\n\
                                 SImode, operands[2])",
    __builtin_constant_p 
#line 1879 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_zero_extend_const_eq (DImode, operands[3],
                                 SImode, operands[2]))
    ? (int) 
#line 1879 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_zero_extend_const_eq (DImode, operands[3],
                                 SImode, operands[2]))
    : -1 },
#line 2483 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!reg_overlap_mentioned_p (operands[0], operands[1])\n\
   && INTVAL (operands[3]) == -INTVAL (operands[2])",
    __builtin_constant_p 
#line 2483 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!reg_overlap_mentioned_p (operands[0], operands[1])
   && INTVAL (operands[3]) == -INTVAL (operands[2]))
    ? (int) 
#line 2483 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!reg_overlap_mentioned_p (operands[0], operands[1])
   && INTVAL (operands[3]) == -INTVAL (operands[2]))
    : -1 },
  { "((!aarch64_move_imm (INTVAL (operands[1]), SImode)\n\
   && !aarch64_plus_operand (operands[1], SImode)\n\
   && !reload_completed) && (ptr_mode == SImode || Pmode == SImode)) && ( true)",
    __builtin_constant_p ((
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode)) && 
#line 500 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) ((
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode)) && 
#line 500 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
  { "(INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)) && ( true)",
    __builtin_constant_p (
#line 4363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)) && 
#line 4365 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) (
#line 4363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)) && 
#line 4365 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 5026 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!HONOR_SIGNED_ZEROS (DFmode) && TARGET_FLOAT",
    __builtin_constant_p 
#line 5026 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!HONOR_SIGNED_ZEROS (DFmode) && TARGET_FLOAT)
    ? (int) 
#line 5026 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!HONOR_SIGNED_ZEROS (DFmode) && TARGET_FLOAT)
    : -1 },
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 16, 63)",
    __builtin_constant_p 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 16, 63))
    ? (int) 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 16, 63))
    : -1 },
#line 1983 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,\n\
		GET_MODE_BITSIZE (GET_MODE_INNER (V2DFmode)))",
    __builtin_constant_p 
#line 1983 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (GET_MODE_INNER (V2DFmode))))
    ? (int) 
#line 1983 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (GET_MODE_INNER (V2DFmode))))
    : -1 },
#line 4610 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[3]) < GET_MODE_BITSIZE (SImode)\n\
   && (UINTVAL (operands[3]) + UINTVAL (operands[4])\n\
       == GET_MODE_BITSIZE (SImode))",
    __builtin_constant_p 
#line 4610 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[3]) < GET_MODE_BITSIZE (SImode)
   && (UINTVAL (operands[3]) + UINTVAL (operands[4])
       == GET_MODE_BITSIZE (SImode)))
    ? (int) 
#line 4610 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[3]) < GET_MODE_BITSIZE (SImode)
   && (UINTVAL (operands[3]) + UINTVAL (operands[4])
       == GET_MODE_BITSIZE (SImode)))
    : -1 },
#line 3847 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "reload_completed && FP_REGNUM_P (REGNO (operands[0]))",
    __builtin_constant_p 
#line 3847 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(reload_completed && FP_REGNUM_P (REGNO (operands[0])))
    ? (int) 
#line 3847 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(reload_completed && FP_REGNUM_P (REGNO (operands[0])))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx16SFmode)\n\
       || register_operand (operands[2], VNx16SFmode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16SFmode)
       || register_operand (operands[2], VNx16SFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16SFmode)
       || register_operand (operands[2], VNx16SFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx16SImode)\n\
       || register_operand (operands[2], VNx16SImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16SImode)
       || register_operand (operands[2], VNx16SImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16SImode)
       || register_operand (operands[2], VNx16SImode)))
    : -1 },
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx2DImode)\n\
       || register_operand (operands[2], VNx2DImode))",
    __builtin_constant_p 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx2DImode)
       || register_operand (operands[2], VNx2DImode)))
    ? (int) 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx2DImode)
       || register_operand (operands[2], VNx2DImode)))
    : -1 },
#line 4863 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_mask_and_shift_for_ubfiz_p (DImode, operands[3], operands[2])\n\
  && (INTVAL (operands[2]) + popcount_hwi (INTVAL (operands[3])))\n\
      == GET_MODE_BITSIZE (SImode)",
    __builtin_constant_p 
#line 4863 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_mask_and_shift_for_ubfiz_p (DImode, operands[3], operands[2])
  && (INTVAL (operands[2]) + popcount_hwi (INTVAL (operands[3])))
      == GET_MODE_BITSIZE (SImode))
    ? (int) 
#line 4863 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_mask_and_shift_for_ubfiz_p (DImode, operands[3], operands[2])
  && (INTVAL (operands[2]) + popcount_hwi (INTVAL (operands[3])))
      == GET_MODE_BITSIZE (SImode))
    : -1 },
  { "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DFmode)) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 1501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DFmode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 1501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DFmode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
  { "(INTVAL (operands[5]) == GET_MODE_SIZE (DFmode)) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 1467 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (DFmode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 1467 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (DFmode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx4DImode)\n\
       || register_operand (operands[2], VNx4DImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4DImode)
       || register_operand (operands[2], VNx4DImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4DImode)
       || register_operand (operands[2], VNx4DImode)))
    : -1 },
#line 1849 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[2], VNx4SFmode)\n\
       || register_operand (operands[3], VNx4SFmode))",
    __builtin_constant_p 
#line 1849 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[2], VNx4SFmode)
       || register_operand (operands[3], VNx4SFmode)))
    ? (int) 
#line 1849 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[2], VNx4SFmode)
       || register_operand (operands[3], VNx4SFmode)))
    : -1 },
#line 1232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "can_create_pseudo_p ()\n\
   && !aarch64_can_const_movi_rtx_p (operands[1], DFmode)\n\
   && !aarch64_float_const_representable_p (operands[1])\n\
   &&  aarch64_float_const_rtx_p (operands[1])",
    __builtin_constant_p 
#line 1232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(can_create_pseudo_p ()
   && !aarch64_can_const_movi_rtx_p (operands[1], DFmode)
   && !aarch64_float_const_representable_p (operands[1])
   &&  aarch64_float_const_rtx_p (operands[1]))
    ? (int) 
#line 1232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(can_create_pseudo_p ()
   && !aarch64_can_const_movi_rtx_p (operands[1], DFmode)
   && !aarch64_float_const_representable_p (operands[1])
   &&  aarch64_float_const_rtx_p (operands[1]))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx16HImode)\n\
       || register_operand (operands[2], VNx16HImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16HImode)
       || register_operand (operands[2], VNx16HImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16HImode)
       || register_operand (operands[2], VNx16HImode)))
    : -1 },
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (SImode) >= 64",
    __builtin_constant_p 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (SImode) >= 64)
    ? (int) 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (SImode) >= 64)
    : -1 },
  { "(INTVAL (operands[5]) == GET_MODE_SIZE (SFmode)) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 1467 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (SFmode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 1467 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (SFmode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
#line 2630 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_is_extend_from_extract (SImode, operands[2], operands[3])",
    __builtin_constant_p 
#line 2630 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_is_extend_from_extract (SImode, operands[2], operands[3]))
    ? (int) 
#line 2630 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_is_extend_from_extract (SImode, operands[2], operands[3]))
    : -1 },
  { "(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)) && ( true)",
    __builtin_constant_p (
#line 4293 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)) && 
#line 4295 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) (
#line 4293 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)) && 
#line 4295 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx8DFmode)\n\
       || register_operand (operands[2], VNx8DFmode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8DFmode)
       || register_operand (operands[2], VNx8DFmode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8DFmode)
       || register_operand (operands[2], VNx8DFmode)))
    : -1 },
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V4HImode)",
    __builtin_constant_p 
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V4HImode))
    ? (int) 
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V4HImode))
    : -1 },
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 16, 63)",
    __builtin_constant_p 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 16, 63))
    ? (int) 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 16, 63))
    : -1 },
  { "(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)) && ( true)",
    __builtin_constant_p (
#line 4293 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)) && 
#line 4295 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) (
#line 4293 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)) && 
#line 4295 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 4750 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]), 1,\n\
	     GET_MODE_BITSIZE (DImode) - 1)\n\
   && (INTVAL (operands[2]) + INTVAL (operands[3]))\n\
       == GET_MODE_BITSIZE (SImode)",
    __builtin_constant_p 
#line 4750 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]), 1,
	     GET_MODE_BITSIZE (DImode) - 1)
   && (INTVAL (operands[2]) + INTVAL (operands[3]))
       == GET_MODE_BITSIZE (SImode))
    ? (int) 
#line 4750 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]), 1,
	     GET_MODE_BITSIZE (DImode) - 1)
   && (INTVAL (operands[2]) + INTVAL (operands[3]))
       == GET_MODE_BITSIZE (SImode))
    : -1 },
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V2SImode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V2SImode))",
    __builtin_constant_p 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V2SImode)
       || aarch64_simd_reg_or_zero (operands[1], V2SImode)))
    ? (int) 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V2SImode)
       || aarch64_simd_reg_or_zero (operands[1], V2SImode)))
    : -1 },
#line 152 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && BYTES_BIG_ENDIAN",
    __builtin_constant_p 
#line 152 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && BYTES_BIG_ENDIAN)
    ? (int) 
#line 152 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && BYTES_BIG_ENDIAN)
    : -1 },
  { "(TARGET_SIMD) && (TARGET_SIMD_F16INST)",
    __builtin_constant_p (
#line 4716 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD) && 
#line 103 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(TARGET_SIMD_F16INST))
    ? (int) (
#line 4716 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD) && 
#line 103 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(TARGET_SIMD_F16INST))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx24HFmode)\n\
       || register_operand (operands[2], VNx24HFmode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx24HFmode)
       || register_operand (operands[2], VNx24HFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx24HFmode)
       || register_operand (operands[2], VNx24HFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx8SFmode)\n\
       || register_operand (operands[2], VNx8SFmode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8SFmode)
       || register_operand (operands[2], VNx8SFmode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8SFmode)
       || register_operand (operands[2], VNx8SFmode)))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx48QImode)\n\
       || register_operand (operands[2], VNx48QImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx48QImode)
       || register_operand (operands[2], VNx48QImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx48QImode)
       || register_operand (operands[2], VNx48QImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 934 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "(register_operand (operands[0], QImode)\n\
    || aarch64_reg_or_zero (operands[1], QImode))",
    __builtin_constant_p 
#line 934 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((register_operand (operands[0], QImode)
    || aarch64_reg_or_zero (operands[1], QImode)))
    ? (int) 
#line 934 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((register_operand (operands[0], QImode)
    || aarch64_reg_or_zero (operands[1], QImode)))
    : -1 },
#line 4833 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[2]) < 64",
    __builtin_constant_p 
#line 4833 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < 64)
    ? (int) 
#line 4833 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < 64)
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx12SFmode)\n\
       || register_operand (operands[2], VNx12SFmode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx12SFmode)
       || register_operand (operands[2], VNx12SFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx12SFmode)
       || register_operand (operands[2], VNx12SFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
  { "ptr_mode == DImode",
    __builtin_constant_p 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode)
    ? (int) 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode)
    : -1 },
#line 3709 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD_RDMA",
    __builtin_constant_p 
#line 3709 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD_RDMA)
    ? (int) 
#line 3709 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD_RDMA)
    : -1 },
#line 4293 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)",
    __builtin_constant_p 
#line 4293 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0))
    ? (int) 
#line 4293 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0))
    : -1 },
  { "(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)\n\
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)) && ( true)",
    __builtin_constant_p (
#line 4318 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)) && 
#line 4321 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) (
#line 4318 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)) && 
#line 4321 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (DImode) >= 64",
    __builtin_constant_p 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (DImode) >= 64)
    ? (int) 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (DImode) >= 64)
    : -1 },
#line 1208 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT && (register_operand (operands[0], DFmode)\n\
    || aarch64_reg_or_fp_zero (operands[1], DFmode))",
    __builtin_constant_p 
#line 1208 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (register_operand (operands[0], DFmode)
    || aarch64_reg_or_fp_zero (operands[1], DFmode)))
    ? (int) 
#line 1208 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (register_operand (operands[0], DFmode)
    || aarch64_reg_or_fp_zero (operands[1], DFmode)))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx8DFmode)\n\
       || register_operand (operands[2], VNx8DFmode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8DFmode)
       || register_operand (operands[2], VNx8DFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8DFmode)
       || register_operand (operands[2], VNx8DFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && ENDIAN_LANE_N (8, INTVAL (operands[2])) == 0",
    __builtin_constant_p 
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && ENDIAN_LANE_N (8, INTVAL (operands[2])) == 0)
    ? (int) 
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && ENDIAN_LANE_N (8, INTVAL (operands[2])) == 0)
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx8SFmode)\n\
       || register_operand (operands[2], VNx8SFmode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8SFmode)
       || register_operand (operands[2], VNx8SFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8SFmode)
       || register_operand (operands[2], VNx8SFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 4318 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)\n\
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)",
    __builtin_constant_p 
#line 4318 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode))
    ? (int) 
#line 4318 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode))
    : -1 },
#line 5602 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT && (reload_completed || reload_in_progress)",
    __builtin_constant_p 
#line 5602 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (reload_completed || reload_in_progress))
    ? (int) 
#line 5602 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (reload_completed || reload_in_progress))
    : -1 },
#line 4596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[3]) < GET_MODE_BITSIZE (DImode) &&\n\
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == GET_MODE_BITSIZE (DImode))",
    __builtin_constant_p 
#line 4596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[3]) < GET_MODE_BITSIZE (DImode) &&
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == GET_MODE_BITSIZE (DImode)))
    ? (int) 
#line 4596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[3]) < GET_MODE_BITSIZE (DImode) &&
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == GET_MODE_BITSIZE (DImode)))
    : -1 },
#line 360 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && !BYTES_BIG_ENDIAN",
    __builtin_constant_p 
#line 360 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && !BYTES_BIG_ENDIAN)
    ? (int) 
#line 360 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && !BYTES_BIG_ENDIAN)
    : -1 },
  { "(TARGET_TLS_DESC && TARGET_SVE) && (ptr_mode == SImode)",
    __builtin_constant_p (
#line 5873 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC && TARGET_SVE) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode))
    ? (int) (
#line 5873 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC && TARGET_SVE) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode))
    : -1 },
#line 727 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && aarch64_check_zero_based_sve_index_immediate (operands[2])",
    __builtin_constant_p 
#line 727 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && aarch64_check_zero_based_sve_index_immediate (operands[2]))
    ? (int) 
#line 727 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && aarch64_check_zero_based_sve_index_immediate (operands[2]))
    : -1 },
#line 211 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && reload_completed\n\
   && GP_REGNUM_P (REGNO (operands[0]))\n\
   && GP_REGNUM_P (REGNO (operands[1]))",
    __builtin_constant_p 
#line 211 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && reload_completed
   && GP_REGNUM_P (REGNO (operands[0]))
   && GP_REGNUM_P (REGNO (operands[1])))
    ? (int) 
#line 211 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && reload_completed
   && GP_REGNUM_P (REGNO (operands[0]))
   && GP_REGNUM_P (REGNO (operands[1])))
    : -1 },
#line 4701 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[2]) < GET_MODE_BITSIZE (HImode)",
    __builtin_constant_p 
#line 4701 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < GET_MODE_BITSIZE (HImode))
    ? (int) 
#line 4701 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < GET_MODE_BITSIZE (HImode))
    : -1 },
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V8QImode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V8QImode))",
    __builtin_constant_p 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V8QImode)
       || aarch64_simd_reg_or_zero (operands[1], V8QImode)))
    ? (int) 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V8QImode)
       || aarch64_simd_reg_or_zero (operands[1], V8QImode)))
    : -1 },
#line 1057 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "(CONST_INT_P (operands[1]) && !aarch64_move_imm (INTVAL (operands[1]), DImode))\n\
    && REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0]))",
    __builtin_constant_p 
#line 1057 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((CONST_INT_P (operands[1]) && !aarch64_move_imm (INTVAL (operands[1]), DImode))
    && REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0])))
    ? (int) 
#line 1057 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((CONST_INT_P (operands[1]) && !aarch64_move_imm (INTVAL (operands[1]), DImode))
    && REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0])))
    : -1 },
#line 5841 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && TARGET_SHA2 && !BYTES_BIG_ENDIAN",
    __builtin_constant_p 
#line 5841 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_SHA2 && !BYTES_BIG_ENDIAN)
    ? (int) 
#line 5841 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_SHA2 && !BYTES_BIG_ENDIAN)
    : -1 },
  { "(TARGET_SIMD) && ( REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0])))",
    __builtin_constant_p (
#line 2522 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD) && 
#line 2528 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
( REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0]))))
    ? (int) (
#line 2522 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD) && 
#line 2528 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
( REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0]))))
    : -1 },
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && GET_MODE_NUNITS (VNx8HFmode).is_constant ()",
    __builtin_constant_p 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx8HFmode).is_constant ())
    ? (int) 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx8HFmode).is_constant ())
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx16SFmode)\n\
       || register_operand (operands[2], VNx16SFmode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16SFmode)
       || register_operand (operands[2], VNx16SFmode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16SFmode)
       || register_operand (operands[2], VNx16SFmode)))
    : -1 },
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !STRICT_ALIGNMENT\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (V2SImode)))",
    __builtin_constant_p 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SImode))))
    ? (int) 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SImode))))
    : -1 },
#line 1429 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "rtx_equal_p (XEXP (operands[2], 0),\n\
		plus_constant (Pmode,\n\
			       XEXP (operands[0], 0),\n\
			       GET_MODE_SIZE (DFmode)))",
    __builtin_constant_p 
#line 1429 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (DFmode))))
    ? (int) 
#line 1429 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (DFmode))))
    : -1 },
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V8HImode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V8HImode))",
    __builtin_constant_p 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V8HImode)
       || aarch64_simd_reg_or_zero (operands[1], V8HImode)))
    ? (int) 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V8HImode)
       || aarch64_simd_reg_or_zero (operands[1], V8HImode)))
    : -1 },
#line 1107 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "(register_operand (operands[0], TImode)\n\
    || aarch64_reg_or_zero (operands[1], TImode))",
    __builtin_constant_p 
#line 1107 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((register_operand (operands[0], TImode)
    || aarch64_reg_or_zero (operands[1], TImode)))
    ? (int) 
#line 1107 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((register_operand (operands[0], TImode)
    || aarch64_reg_or_zero (operands[1], TImode)))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx24HImode)\n\
       || register_operand (operands[2], VNx24HImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx24HImode)
       || register_operand (operands[2], VNx24HImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx24HImode)
       || register_operand (operands[2], VNx24HImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
  { "(TARGET_SVE && operands[0] != stack_pointer_rtx) && ( epilogue_completed\n\
   && !reg_overlap_mentioned_p (operands[0], operands[1])\n\
   && aarch64_split_add_offset_immediate (operands[2], DImode))",
    __builtin_constant_p (
#line 1746 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_SVE && operands[0] != stack_pointer_rtx) && 
#line 1754 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( epilogue_completed
   && !reg_overlap_mentioned_p (operands[0], operands[1])
   && aarch64_split_add_offset_immediate (operands[2], DImode)))
    ? (int) (
#line 1746 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_SVE && operands[0] != stack_pointer_rtx) && 
#line 1754 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( epilogue_completed
   && !reg_overlap_mentioned_p (operands[0], operands[1])
   && aarch64_split_add_offset_immediate (operands[2], DImode)))
    : -1 },
#line 1078 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[1]) < GET_MODE_BITSIZE (SImode)\n\
   && UINTVAL (operands[1]) % 16 == 0",
    __builtin_constant_p 
#line 1078 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[1]) < GET_MODE_BITSIZE (SImode)
   && UINTVAL (operands[1]) % 16 == 0)
    ? (int) 
#line 1078 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[1]) < GET_MODE_BITSIZE (SImode)
   && UINTVAL (operands[1]) % 16 == 0)
    : -1 },
#line 1326 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "rtx_equal_p (XEXP (operands[3], 0),\n\
		plus_constant (Pmode,\n\
			       XEXP (operands[1], 0),\n\
			       GET_MODE_SIZE (DImode)))",
    __builtin_constant_p 
#line 1326 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (DImode))))
    ? (int) 
#line 1326 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (DImode))))
    : -1 },
#line 5068 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !BYTES_BIG_ENDIAN\n\
   && (register_operand (operands[0], OImode)\n\
       || register_operand (operands[1], OImode))",
    __builtin_constant_p 
#line 5068 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !BYTES_BIG_ENDIAN
   && (register_operand (operands[0], OImode)
       || register_operand (operands[1], OImode)))
    ? (int) 
#line 5068 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !BYTES_BIG_ENDIAN
   && (register_operand (operands[0], OImode)
       || register_operand (operands[1], OImode)))
    : -1 },
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
  { "ptr_mode == SImode",
    __builtin_constant_p 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode)
    ? (int) 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode)
    : -1 },
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V2SFmode)",
    __builtin_constant_p 
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V2SFmode))
    ? (int) 
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V2SFmode))
    : -1 },
#line 4809 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[1]) <= 32",
    __builtin_constant_p 
#line 4809 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[1]) <= 32)
    ? (int) 
#line 4809 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[1]) <= 32)
    : -1 },
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[0], 0),\n\
				  GET_MODE_SIZE (V2SFmode)))",
    __builtin_constant_p 
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V2SFmode))))
    ? (int) 
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V2SFmode))))
    : -1 },
#line 4363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)",
    __builtin_constant_p 
#line 4363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode))
    ? (int) 
#line 4363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode))
    : -1 },
#line 1554 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "rtx_equal_p (XEXP (operands[3], 0),\n\
		plus_constant (Pmode,\n\
			       XEXP (operands[1], 0),\n\
			       GET_MODE_SIZE (SImode)))",
    __builtin_constant_p 
#line 1554 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (SImode))))
    ? (int) 
#line 1554 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (SImode))))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx32HFmode)\n\
       || register_operand (operands[2], VNx32HFmode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32HFmode)
       || register_operand (operands[2], VNx32HFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32HFmode)
       || register_operand (operands[2], VNx32HFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx6DFmode)\n\
       || register_operand (operands[2], VNx6DFmode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx6DFmode)
       || register_operand (operands[2], VNx6DFmode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx6DFmode)
       || register_operand (operands[2], VNx6DFmode)))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx12SImode)\n\
       || register_operand (operands[2], VNx12SImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx12SImode)
       || register_operand (operands[2], VNx12SImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx12SImode)
       || register_operand (operands[2], VNx12SImode)))
    : -1 },
  { "(TARGET_SVE && operands[0] != stack_pointer_rtx) && ( epilogue_completed\n\
   && !reg_overlap_mentioned_p (operands[0], operands[1])\n\
   && aarch64_split_add_offset_immediate (operands[2], SImode))",
    __builtin_constant_p (
#line 1746 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_SVE && operands[0] != stack_pointer_rtx) && 
#line 1754 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( epilogue_completed
   && !reg_overlap_mentioned_p (operands[0], operands[1])
   && aarch64_split_add_offset_immediate (operands[2], SImode)))
    ? (int) (
#line 1746 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_SVE && operands[0] != stack_pointer_rtx) && 
#line 1754 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( epilogue_completed
   && !reg_overlap_mentioned_p (operands[0], operands[1])
   && aarch64_split_add_offset_immediate (operands[2], SImode)))
    : -1 },
#line 934 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "(register_operand (operands[0], HImode)\n\
    || aarch64_reg_or_zero (operands[1], HImode))",
    __builtin_constant_p 
#line 934 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((register_operand (operands[0], HImode)
    || aarch64_reg_or_zero (operands[1], HImode)))
    ? (int) 
#line 934 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((register_operand (operands[0], HImode)
    || aarch64_reg_or_zero (operands[1], HImode)))
    : -1 },
  { "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SFmode)) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 1501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SFmode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 1501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SFmode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
#line 4809 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[1]) <= 16",
    __builtin_constant_p 
#line 4809 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[1]) <= 16)
    ? (int) 
#line 4809 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[1]) <= 16)
    : -1 },
#line 5415 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT\n\
   && ((GET_MODE_BITSIZE (DFmode) <= LONG_TYPE_SIZE)\n\
   || !flag_trapping_math || flag_fp_int_builtin_inexact)",
    __builtin_constant_p 
#line 5415 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT
   && ((GET_MODE_BITSIZE (DFmode) <= LONG_TYPE_SIZE)
   || !flag_trapping_math || flag_fp_int_builtin_inexact))
    ? (int) 
#line 5415 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT
   && ((GET_MODE_BITSIZE (DFmode) <= LONG_TYPE_SIZE)
   || !flag_trapping_math || flag_fp_int_builtin_inexact))
    : -1 },
#line 4964 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT\n\
   && IN_RANGE (aarch64_fpconst_pow_of_2 (operands[2]), 1,\n\
		GET_MODE_BITSIZE (SImode))",
    __builtin_constant_p 
#line 4964 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT
   && IN_RANGE (aarch64_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (SImode)))
    ? (int) 
#line 4964 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT
   && IN_RANGE (aarch64_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (SImode)))
    : -1 },
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx8HImode)\n\
       || register_operand (operands[2], VNx8HImode))",
    __builtin_constant_p 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8HImode)
       || register_operand (operands[2], VNx8HImode)))
    ? (int) 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8HImode)
       || register_operand (operands[2], VNx8HImode)))
    : -1 },
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, true, V4HImode)",
    __builtin_constant_p 
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, V4HImode))
    ? (int) 
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, V4HImode))
    : -1 },
#line 1232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "can_create_pseudo_p ()\n\
   && !aarch64_can_const_movi_rtx_p (operands[1], HFmode)\n\
   && !aarch64_float_const_representable_p (operands[1])\n\
   &&  aarch64_float_const_rtx_p (operands[1])",
    __builtin_constant_p 
#line 1232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(can_create_pseudo_p ()
   && !aarch64_can_const_movi_rtx_p (operands[1], HFmode)
   && !aarch64_float_const_representable_p (operands[1])
   &&  aarch64_float_const_rtx_p (operands[1]))
    ? (int) 
#line 1232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(can_create_pseudo_p ()
   && !aarch64_can_const_movi_rtx_p (operands[1], HFmode)
   && !aarch64_float_const_representable_p (operands[1])
   &&  aarch64_float_const_rtx_p (operands[1]))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx24HImode)\n\
       || register_operand (operands[2], VNx24HImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx24HImode)
       || register_operand (operands[2], VNx24HImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx24HImode)
       || register_operand (operands[2], VNx24HImode)))
    : -1 },
#line 1847 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_zero_extend_const_eq (DImode, operands[2],\n\
				 SImode, operands[1])",
    __builtin_constant_p 
#line 1847 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_zero_extend_const_eq (DImode, operands[2],
				 SImode, operands[1]))
    ? (int) 
#line 1847 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_zero_extend_const_eq (DImode, operands[2],
				 SImode, operands[1]))
    : -1 },
#line 1690 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_move_imm (INTVAL (operands[2]), DImode)",
    __builtin_constant_p 
#line 1690 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_move_imm (INTVAL (operands[2]), DImode))
    ? (int) 
#line 1690 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_move_imm (INTVAL (operands[2]), DImode))
    : -1 },
#line 4850 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_mask_and_shift_for_ubfiz_p (SImode, operands[3], operands[2])",
    __builtin_constant_p 
#line 4850 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_mask_and_shift_for_ubfiz_p (SImode, operands[3], operands[2]))
    ? (int) 
#line 4850 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_mask_and_shift_for_ubfiz_p (SImode, operands[3], operands[2]))
    : -1 },
#line 5679 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_ILP32",
    __builtin_constant_p 
#line 5679 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_ILP32)
    ? (int) 
#line 5679 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_ILP32)
    : -1 },
  { "(TARGET_TLS_DESC && !TARGET_SVE) && (ptr_mode == SImode)",
    __builtin_constant_p (
#line 5833 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC && !TARGET_SVE) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode))
    ? (int) (
#line 5833 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC && !TARGET_SVE) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode))
    : -1 },
  { "(TARGET_FLOAT) && (AARCH64_ISA_F16)",
    __builtin_constant_p (
#line 5355 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT) && 
#line 45 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(AARCH64_ISA_F16))
    ? (int) (
#line 5355 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT) && 
#line 45 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(AARCH64_ISA_F16))
    : -1 },
  { "(TARGET_SIMD) && ( reload_completed)",
    __builtin_constant_p (
#line 5506 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD) && 
#line 5508 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
( reload_completed))
    ? (int) (
#line 5506 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD) && 
#line 5508 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
( reload_completed))
    : -1 },
  { "(TARGET_TLS_DESC && !TARGET_SVE) && (ptr_mode == DImode)",
    __builtin_constant_p (
#line 5833 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC && !TARGET_SVE) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode))
    ? (int) (
#line 5833 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC && !TARGET_SVE) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx4DFmode)\n\
       || register_operand (operands[2], VNx4DFmode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4DFmode)
       || register_operand (operands[2], VNx4DFmode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4DFmode)
       || register_operand (operands[2], VNx4DFmode)))
    : -1 },
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 0, 63)",
    __builtin_constant_p 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 0, 63))
    ? (int) 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 0, 63))
    : -1 },
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (SFmode) >= 64",
    __builtin_constant_p 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (SFmode) >= 64)
    ? (int) 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (SFmode) >= 64)
    : -1 },
#line 4797 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!(UINTVAL (operands[1]) == 0\n\
     || (UINTVAL (operands[2]) + UINTVAL (operands[1])\n\
	 > GET_MODE_BITSIZE (SImode)))",
    __builtin_constant_p 
#line 4797 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[2]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (SImode))))
    ? (int) 
#line 4797 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[2]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (SImode))))
    : -1 },
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
  { "ptr_mode == SImode || Pmode == SImode",
    __builtin_constant_p 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode)
    ? (int) 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode)
    : -1 },
  { "(TARGET_TLS_DESC && TARGET_SVE) && (ptr_mode == DImode)",
    __builtin_constant_p (
#line 5873 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC && TARGET_SVE) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode))
    ? (int) (
#line 5873 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC && TARGET_SVE) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode))
    : -1 },
#line 1379 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "rtx_equal_p (XEXP (operands[3], 0),\n\
		plus_constant (Pmode,\n\
			       XEXP (operands[1], 0),\n\
			       GET_MODE_SIZE (SFmode)))",
    __builtin_constant_p 
#line 1379 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (SFmode))))
    ? (int) 
#line 1379 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (SFmode))))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx32HImode)\n\
       || register_operand (operands[2], VNx32HImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32HImode)
       || register_operand (operands[2], VNx32HImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32HImode)
       || register_operand (operands[2], VNx32HImode)))
    : -1 },
#line 4281 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "(~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0",
    __builtin_constant_p 
#line 4281 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)
    ? (int) 
#line 4281 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)
    : -1 },
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V4HFmode)",
    __builtin_constant_p 
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V4HFmode))
    ? (int) 
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V4HFmode))
    : -1 },
#line 4135 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "INTVAL (operands[1]) > 0\n\
   && ((INTVAL (operands[1]) + INTVAL (operands[2]))\n\
	<= GET_MODE_BITSIZE (SImode))\n\
   && aarch64_bitmask_imm (\n\
	UINTVAL (aarch64_mask_from_zextract_ops (operands[1],\n\
						 operands[2])),\n\
	SImode)",
    __builtin_constant_p 
#line 4135 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[1]) > 0
   && ((INTVAL (operands[1]) + INTVAL (operands[2]))
	<= GET_MODE_BITSIZE (SImode))
   && aarch64_bitmask_imm (
	UINTVAL (aarch64_mask_from_zextract_ops (operands[1],
						 operands[2])),
	SImode))
    ? (int) 
#line 4135 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[1]) > 0
   && ((INTVAL (operands[1]) + INTVAL (operands[2]))
	<= GET_MODE_BITSIZE (SImode))
   && aarch64_bitmask_imm (
	UINTVAL (aarch64_mask_from_zextract_ops (operands[1],
						 operands[2])),
	SImode))
    : -1 },
#line 5415 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT\n\
   && ((GET_MODE_BITSIZE (SFmode) <= LONG_TYPE_SIZE)\n\
   || !flag_trapping_math || flag_fp_int_builtin_inexact)",
    __builtin_constant_p 
#line 5415 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT
   && ((GET_MODE_BITSIZE (SFmode) <= LONG_TYPE_SIZE)
   || !flag_trapping_math || flag_fp_int_builtin_inexact))
    ? (int) 
#line 5415 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT
   && ((GET_MODE_BITSIZE (SFmode) <= LONG_TYPE_SIZE)
   || !flag_trapping_math || flag_fp_int_builtin_inexact))
    : -1 },
#line 1277 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "reload_completed && aarch64_split_128bit_move_p (operands[0], operands[1])",
    __builtin_constant_p 
#line 1277 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(reload_completed && aarch64_split_128bit_move_p (operands[0], operands[1]))
    ? (int) 
#line 1277 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(reload_completed && aarch64_split_128bit_move_p (operands[0], operands[1]))
    : -1 },
#line 2615 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_is_extend_from_extract (DImode, operands[2], operands[3])",
    __builtin_constant_p 
#line 2615 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_is_extend_from_extract (DImode, operands[2], operands[3]))
    ? (int) 
#line 2615 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_is_extend_from_extract (DImode, operands[2], operands[3]))
    : -1 },
#line 1849 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[2], VNx2DFmode)\n\
       || register_operand (operands[3], VNx2DFmode))",
    __builtin_constant_p 
#line 1849 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[2], VNx2DFmode)
       || register_operand (operands[3], VNx2DFmode)))
    ? (int) 
#line 1849 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[2], VNx2DFmode)
       || register_operand (operands[3], VNx2DFmode)))
    : -1 },
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V2DFmode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V2DFmode))",
    __builtin_constant_p 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V2DFmode)
       || aarch64_simd_reg_or_zero (operands[1], V2DFmode)))
    ? (int) 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V2DFmode)
       || aarch64_simd_reg_or_zero (operands[1], V2DFmode)))
    : -1 },
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
  { "ptr_mode == DImode || Pmode == DImode",
    __builtin_constant_p 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode)
    ? (int) 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode)
    : -1 },
  { "(TARGET_SVE) && ( reload_completed\n\
   && REG_P (operands[0])\n\
   && REGNO (operands[0]) == REGNO (operands[1]))",
    __builtin_constant_p (
#line 521 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE) && 
#line 536 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed
   && REG_P (operands[0])
   && REGNO (operands[0]) == REGNO (operands[1])))
    ? (int) (
#line 521 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE) && 
#line 536 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed
   && REG_P (operands[0])
   && REGNO (operands[0]) == REGNO (operands[1])))
    : -1 },
#line 6656 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && TARGET_AES",
    __builtin_constant_p 
#line 6656 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_AES)
    ? (int) 
#line 6656 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_AES)
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx16SImode)\n\
       || register_operand (operands[2], VNx16SImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16SImode)
       || register_operand (operands[2], VNx16SImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16SImode)
       || register_operand (operands[2], VNx16SImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
  { "(TARGET_SVE) && ( MEM_P (operands[1]))",
    __builtin_constant_p (
#line 645 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE) && 
#line 650 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( MEM_P (operands[1])))
    ? (int) (
#line 645 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE) && 
#line 650 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( MEM_P (operands[1])))
    : -1 },
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 1, 15)",
    __builtin_constant_p 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 1, 15))
    ? (int) 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 1, 15))
    : -1 },
#line 1849 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[2], VNx8HFmode)\n\
       || register_operand (operands[3], VNx8HFmode))",
    __builtin_constant_p 
#line 1849 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[2], VNx8HFmode)
       || register_operand (operands[3], VNx8HFmode)))
    ? (int) 
#line 1849 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[2], VNx8HFmode)
       || register_operand (operands[3], VNx8HFmode)))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx8SImode)\n\
       || register_operand (operands[2], VNx8SImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8SImode)
       || register_operand (operands[2], VNx8SImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8SImode)
       || register_operand (operands[2], VNx8SImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !STRICT_ALIGNMENT\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (V4HFmode)))",
    __builtin_constant_p 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HFmode))))
    ? (int) 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HFmode))))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx32HFmode)\n\
       || register_operand (operands[2], VNx32HFmode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32HFmode)
       || register_operand (operands[2], VNx32HFmode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32HFmode)
       || register_operand (operands[2], VNx32HFmode)))
    : -1 },
#line 5102 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && BYTES_BIG_ENDIAN\n\
   && (register_operand (operands[0], OImode)\n\
       || register_operand (operands[1], OImode))",
    __builtin_constant_p 
#line 5102 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && BYTES_BIG_ENDIAN
   && (register_operand (operands[0], OImode)
       || register_operand (operands[1], OImode)))
    ? (int) 
#line 5102 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && BYTES_BIG_ENDIAN
   && (register_operand (operands[0], OImode)
       || register_operand (operands[1], OImode)))
    : -1 },
#line 4701 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[2]) < GET_MODE_BITSIZE (QImode)",
    __builtin_constant_p 
#line 4701 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < GET_MODE_BITSIZE (QImode))
    ? (int) 
#line 4701 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < GET_MODE_BITSIZE (QImode))
    : -1 },
#line 2445 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!reg_overlap_mentioned_p (operands[0], operands[1])\n\
   && !reg_overlap_mentioned_p (operands[0], operands[2])",
    __builtin_constant_p 
#line 2445 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!reg_overlap_mentioned_p (operands[0], operands[1])
   && !reg_overlap_mentioned_p (operands[0], operands[2]))
    ? (int) 
#line 2445 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!reg_overlap_mentioned_p (operands[0], operands[1])
   && !reg_overlap_mentioned_p (operands[0], operands[2]))
    : -1 },
  { "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DFmode)) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 1501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DFmode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 1501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DFmode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
  { "((!aarch64_move_imm (INTVAL (operands[1]), SImode)\n\
   && !aarch64_plus_operand (operands[1], SImode)\n\
   && !reload_completed) && (ptr_mode == DImode || Pmode == DImode)) && ( true)",
    __builtin_constant_p ((
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode)) && 
#line 500 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) ((
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode)) && 
#line 500 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx2DFmode)\n\
       || register_operand (operands[2], VNx2DFmode))",
    __builtin_constant_p 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx2DFmode)
       || register_operand (operands[2], VNx2DFmode)))
    ? (int) 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx2DFmode)
       || register_operand (operands[2], VNx2DFmode)))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx16HFmode)\n\
       || register_operand (operands[2], VNx16HFmode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16HFmode)
       || register_operand (operands[2], VNx16HFmode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16HFmode)
       || register_operand (operands[2], VNx16HFmode)))
    : -1 },
#line 5851 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && TARGET_SHA2 && BYTES_BIG_ENDIAN",
    __builtin_constant_p 
#line 5851 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_SHA2 && BYTES_BIG_ENDIAN)
    ? (int) 
#line 5851 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_SHA2 && BYTES_BIG_ENDIAN)
    : -1 },
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V4SFmode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V4SFmode))",
    __builtin_constant_p 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V4SFmode)
       || aarch64_simd_reg_or_zero (operands[1], V4SFmode)))
    ? (int) 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V4SFmode)
       || aarch64_simd_reg_or_zero (operands[1], V4SFmode)))
    : -1 },
  { "(TARGET_SVE) && ( !CONSTANT_P (operands[1]))",
    __builtin_constant_p (
#line 1281 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE) && 
#line 1285 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( !CONSTANT_P (operands[1])))
    ? (int) (
#line 1281 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE) && 
#line 1285 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( !CONSTANT_P (operands[1])))
    : -1 },
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx4BImode)\n\
       || register_operand (operands[1], VNx4BImode))",
    __builtin_constant_p 
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4BImode)
       || register_operand (operands[1], VNx4BImode)))
    ? (int) 
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4BImode)
       || register_operand (operands[1], VNx4BImode)))
    : -1 },
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && GET_MODE_NUNITS (VNx8HImode).is_constant ()",
    __builtin_constant_p 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx8HImode).is_constant ())
    ? (int) 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx8HImode).is_constant ())
    : -1 },
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 0, 63)",
    __builtin_constant_p 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 0, 63))
    ? (int) 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 0, 63))
    : -1 },
  { "(INTVAL (operands[5]) == GET_MODE_SIZE (SFmode)) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 1467 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (SFmode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 1467 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (SFmode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 1, 15)",
    __builtin_constant_p 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 1, 15))
    ? (int) 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 1, 15))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx4DImode)\n\
       || register_operand (operands[2], VNx4DImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4DImode)
       || register_operand (operands[2], VNx4DImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4DImode)
       || register_operand (operands[2], VNx4DImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 0, 63)",
    __builtin_constant_p 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 0, 63))
    ? (int) 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 0, 63))
    : -1 },
#line 5544 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT",
    __builtin_constant_p 
#line 5544 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT)
    ? (int) 
#line 5544 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT)
    : -1 },
#line 5127 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && BYTES_BIG_ENDIAN\n\
   && (register_operand (operands[0], XImode)\n\
       || register_operand (operands[1], XImode))",
    __builtin_constant_p 
#line 5127 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && BYTES_BIG_ENDIAN
   && (register_operand (operands[0], XImode)
       || register_operand (operands[1], XImode)))
    ? (int) 
#line 5127 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && BYTES_BIG_ENDIAN
   && (register_operand (operands[0], XImode)
       || register_operand (operands[1], XImode)))
    : -1 },
  { "(TARGET_SIMD) && (AARCH64_ISA_F16)",
    __builtin_constant_p (
#line 5754 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD) && 
#line 45 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(AARCH64_ISA_F16))
    ? (int) (
#line 5754 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD) && 
#line 45 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(AARCH64_ISA_F16))
    : -1 },
#line 1037 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "(register_operand (operands[0], DImode)\n\
    || aarch64_reg_or_zero (operands[1], DImode))",
    __builtin_constant_p 
#line 1037 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((register_operand (operands[0], DImode)
    || aarch64_reg_or_zero (operands[1], DImode)))
    ? (int) 
#line 1037 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((register_operand (operands[0], DImode)
    || aarch64_reg_or_zero (operands[1], DImode)))
    : -1 },
#line 328 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_adjust_ok_for_ldpstp (operands, false, DImode)",
    __builtin_constant_p 
#line 328 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, false, DImode))
    ? (int) 
#line 328 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, false, DImode))
    : -1 },
#line 3087 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !BYTES_BIG_ENDIAN",
    __builtin_constant_p 
#line 3087 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !BYTES_BIG_ENDIAN)
    ? (int) 
#line 3087 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !BYTES_BIG_ENDIAN)
    : -1 },
#line 4809 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[1]) <= 8",
    __builtin_constant_p 
#line 4809 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[1]) <= 8)
    ? (int) 
#line 4809 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[1]) <= 8)
    : -1 },
#line 5184 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT && (TARGET_FP_F16INST || TARGET_SIMD)",
    __builtin_constant_p 
#line 5184 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (TARGET_FP_F16INST || TARGET_SIMD))
    ? (int) 
#line 5184 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (TARGET_FP_F16INST || TARGET_SIMD))
    : -1 },
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !STRICT_ALIGNMENT\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (DImode)))",
    __builtin_constant_p 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (DImode))))
    ? (int) 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (DImode))))
    : -1 },
#line 360 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_adjust_ok_for_ldpstp (operands, false, SFmode)",
    __builtin_constant_p 
#line 360 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, false, SFmode))
    ? (int) 
#line 360 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, false, SFmode))
    : -1 },
#line 3501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_CRC32",
    __builtin_constant_p 
#line 3501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_CRC32)
    ? (int) 
#line 3501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_CRC32)
    : -1 },
#line 2391 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE",
    __builtin_constant_p 
#line 2391 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE)
    ? (int) 
#line 2391 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE)
    : -1 },
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx8BImode)\n\
       || register_operand (operands[1], VNx8BImode))",
    __builtin_constant_p 
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8BImode)
       || register_operand (operands[1], VNx8BImode)))
    ? (int) 
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8BImode)
       || register_operand (operands[1], VNx8BImode)))
    : -1 },
#line 4735 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]),\n\
	     1, GET_MODE_BITSIZE (SImode) - 1)",
    __builtin_constant_p 
#line 4735 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]),
	     1, GET_MODE_BITSIZE (SImode) - 1))
    ? (int) 
#line 4735 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]),
	     1, GET_MODE_BITSIZE (SImode) - 1))
    : -1 },
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !STRICT_ALIGNMENT\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (V4HImode)))",
    __builtin_constant_p 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HImode))))
    ? (int) 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HImode))))
    : -1 },
#line 1000 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "(register_operand (operands[0], SImode)\n\
    || aarch64_reg_or_zero (operands[1], SImode))",
    __builtin_constant_p 
#line 1000 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((register_operand (operands[0], SImode)
    || aarch64_reg_or_zero (operands[1], SImode)))
    ? (int) 
#line 1000 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((register_operand (operands[0], SImode)
    || aarch64_reg_or_zero (operands[1], SImode)))
    : -1 },
#line 169 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, true, SImode)",
    __builtin_constant_p 
#line 169 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, SImode))
    ? (int) 
#line 169 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, SImode))
    : -1 },
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && ENDIAN_LANE_N (4, INTVAL (operands[2])) == 0",
    __builtin_constant_p 
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && ENDIAN_LANE_N (4, INTVAL (operands[2])) == 0)
    ? (int) 
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && ENDIAN_LANE_N (4, INTVAL (operands[2])) == 0)
    : -1 },
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, true, V2SImode)",
    __builtin_constant_p 
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, V2SImode))
    ? (int) 
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, V2SImode))
    : -1 },
  { "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SImode)) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 1485 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SImode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 1485 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SImode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx48QImode)\n\
       || register_operand (operands[2], VNx48QImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx48QImode)
       || register_operand (operands[2], VNx48QImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx48QImode)
       || register_operand (operands[2], VNx48QImode)))
    : -1 },
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && GET_MODE_NUNITS (VNx2DImode).is_constant ()",
    __builtin_constant_p 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx2DImode).is_constant ())
    ? (int) 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx2DImode).is_constant ())
    : -1 },
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !STRICT_ALIGNMENT\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (V2SFmode)))",
    __builtin_constant_p 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SFmode))))
    ? (int) 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SFmode))))
    : -1 },
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 16, 63)",
    __builtin_constant_p 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 16, 63))
    ? (int) 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 16, 63))
    : -1 },
#line 3268 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!aarch64_move_imm (INTVAL (operands[2]), SImode)\n\
   && !aarch64_plus_operand (operands[2], SImode)\n\
   && !reload_completed",
    __builtin_constant_p 
#line 3268 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[2]), SImode)
   && !aarch64_plus_operand (operands[2], SImode)
   && !reload_completed)
    ? (int) 
#line 3268 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[2]), SImode)
   && !aarch64_plus_operand (operands[2], SImode)
   && !reload_completed)
    : -1 },
#line 5784 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD",
    __builtin_constant_p 
#line 5784 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD)
    ? (int) 
#line 5784 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD)
    : -1 },
#line 1847 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_zero_extend_const_eq (TImode, operands[2],\n\
				 DImode, operands[1])",
    __builtin_constant_p 
#line 1847 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_zero_extend_const_eq (TImode, operands[2],
				 DImode, operands[1]))
    ? (int) 
#line 1847 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_zero_extend_const_eq (TImode, operands[2],
				 DImode, operands[1]))
    : -1 },
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && !BYTES_BIG_ENDIAN\n\
   && ((lra_in_progress || reload_completed)\n\
       || (register_operand (operands[0], VNx2DFmode)\n\
	   && nonmemory_operand (operands[1], VNx2DFmode)))",
    __builtin_constant_p 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx2DFmode)
	   && nonmemory_operand (operands[1], VNx2DFmode))))
    ? (int) 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx2DFmode)
	   && nonmemory_operand (operands[1], VNx2DFmode))))
    : -1 },
#line 5244 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FP_F16INST",
    __builtin_constant_p 
#line 5244 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FP_F16INST)
    ? (int) 
#line 5244 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FP_F16INST)
    : -1 },
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[0], 0),\n\
				  GET_MODE_SIZE (V2SImode)))",
    __builtin_constant_p 
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V2SImode))))
    ? (int) 
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V2SImode))))
    : -1 },
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 0, 63)",
    __builtin_constant_p 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 0, 63))
    ? (int) 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 0, 63))
    : -1 },
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (HImode) >= 64",
    __builtin_constant_p 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (HImode) >= 64)
    ? (int) 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (HImode) >= 64)
    : -1 },
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V4SImode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V4SImode))",
    __builtin_constant_p 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V4SImode)
       || aarch64_simd_reg_or_zero (operands[1], V4SImode)))
    ? (int) 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V4SImode)
       || aarch64_simd_reg_or_zero (operands[1], V4SImode)))
    : -1 },
#line 86 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, false, SFmode)",
    __builtin_constant_p 
#line 86 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, false, SFmode))
    ? (int) 
#line 86 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, false, SFmode))
    : -1 },
#line 4069 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "reload_completed",
    __builtin_constant_p 
#line 4069 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(reload_completed)
    ? (int) 
#line 4069 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(reload_completed)
    : -1 },
#line 4648 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[2]) < GET_MODE_BITSIZE (DImode)",
    __builtin_constant_p 
#line 4648 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < GET_MODE_BITSIZE (DImode))
    ? (int) 
#line 4648 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < GET_MODE_BITSIZE (DImode))
    : -1 },
#line 6552 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_F16FML",
    __builtin_constant_p 
#line 6552 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_F16FML)
    ? (int) 
#line 6552 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_F16FML)
    : -1 },
#line 5176 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && reload_completed",
    __builtin_constant_p 
#line 5176 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && reload_completed)
    ? (int) 
#line 5176 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && reload_completed)
    : -1 },
#line 4915 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch_rev16_shleft_mask_imm_p (operands[3], DImode)\n\
   && aarch_rev16_shright_mask_imm_p (operands[2], DImode)",
    __builtin_constant_p 
#line 4915 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch_rev16_shleft_mask_imm_p (operands[3], DImode)
   && aarch_rev16_shright_mask_imm_p (operands[2], DImode))
    ? (int) 
#line 4915 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch_rev16_shleft_mask_imm_p (operands[3], DImode)
   && aarch_rev16_shright_mask_imm_p (operands[2], DImode))
    : -1 },
#line 1773 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "epilogue_completed",
    __builtin_constant_p 
#line 1773 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(epilogue_completed)
    ? (int) 
#line 1773 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(epilogue_completed)
    : -1 },
#line 4318 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)\n\
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)",
    __builtin_constant_p 
#line 4318 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode))
    ? (int) 
#line 4318 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode))
    : -1 },
#line 381 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && reload_completed",
    __builtin_constant_p 
#line 381 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && reload_completed)
    ? (int) 
#line 381 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && reload_completed)
    : -1 },
#line 4735 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]),\n\
	     1, GET_MODE_BITSIZE (DImode) - 1)",
    __builtin_constant_p 
#line 4735 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]),
	     1, GET_MODE_BITSIZE (DImode) - 1))
    ? (int) 
#line 4735 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]),
	     1, GET_MODE_BITSIZE (DImode) - 1))
    : -1 },
#line 4135 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "INTVAL (operands[1]) > 0\n\
   && ((INTVAL (operands[1]) + INTVAL (operands[2]))\n\
	<= GET_MODE_BITSIZE (DImode))\n\
   && aarch64_bitmask_imm (\n\
	UINTVAL (aarch64_mask_from_zextract_ops (operands[1],\n\
						 operands[2])),\n\
	DImode)",
    __builtin_constant_p 
#line 4135 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[1]) > 0
   && ((INTVAL (operands[1]) + INTVAL (operands[2]))
	<= GET_MODE_BITSIZE (DImode))
   && aarch64_bitmask_imm (
	UINTVAL (aarch64_mask_from_zextract_ops (operands[1],
						 operands[2])),
	DImode))
    ? (int) 
#line 4135 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[1]) > 0
   && ((INTVAL (operands[1]) + INTVAL (operands[2]))
	<= GET_MODE_BITSIZE (DImode))
   && aarch64_bitmask_imm (
	UINTVAL (aarch64_mask_from_zextract_ops (operands[1],
						 operands[2])),
	DImode))
    : -1 },
  { "(TARGET_SVE && BYTES_BIG_ENDIAN) && ( reload_completed)",
    __builtin_constant_p (
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && BYTES_BIG_ENDIAN) && 
#line 108 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && BYTES_BIG_ENDIAN) && 
#line 108 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 432 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_DOTPROD",
    __builtin_constant_p 
#line 432 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_DOTPROD)
    ? (int) 
#line 432 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_DOTPROD)
    : -1 },
#line 1746 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_SVE && operands[0] != stack_pointer_rtx",
    __builtin_constant_p 
#line 1746 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_SVE && operands[0] != stack_pointer_rtx)
    ? (int) 
#line 1746 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_SVE && operands[0] != stack_pointer_rtx)
    : -1 },
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (HFmode) >= 64",
    __builtin_constant_p 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (HFmode) >= 64)
    ? (int) 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (HFmode) >= 64)
    : -1 },
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !STRICT_ALIGNMENT\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (DFmode)))",
    __builtin_constant_p 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (DFmode))))
    ? (int) 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (DFmode))))
    : -1 },
#line 1254 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT && (register_operand (operands[0], TFmode)\n\
    || aarch64_reg_or_fp_zero (operands[1], TFmode))",
    __builtin_constant_p 
#line 1254 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (register_operand (operands[0], TFmode)
    || aarch64_reg_or_fp_zero (operands[1], TFmode)))
    ? (int) 
#line 1254 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (register_operand (operands[0], TFmode)
    || aarch64_reg_or_fp_zero (operands[1], TFmode)))
    : -1 },
  { "(TARGET_LSE) && ( reload_completed)",
    __builtin_constant_p (
#line 432 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
(TARGET_LSE) && 
#line 434 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
( reload_completed))
    ? (int) (
#line 432 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
(TARGET_LSE) && 
#line 434 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
( reload_completed))
    : -1 },
#line 1413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "rtx_equal_p (XEXP (operands[2], 0),\n\
		plus_constant (Pmode,\n\
			       XEXP (operands[0], 0),\n\
			       GET_MODE_SIZE (SFmode)))",
    __builtin_constant_p 
#line 1413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (SFmode))))
    ? (int) 
#line 1413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (SFmode))))
    : -1 },
#line 360 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_adjust_ok_for_ldpstp (operands, false, DFmode)",
    __builtin_constant_p 
#line 360 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, false, DFmode))
    ? (int) 
#line 360 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, false, DFmode))
    : -1 },
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && !BYTES_BIG_ENDIAN\n\
   && ((lra_in_progress || reload_completed)\n\
       || (register_operand (operands[0], VNx4SFmode)\n\
	   && nonmemory_operand (operands[1], VNx4SFmode)))",
    __builtin_constant_p 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx4SFmode)
	   && nonmemory_operand (operands[1], VNx4SFmode))))
    ? (int) 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx4SFmode)
	   && nonmemory_operand (operands[1], VNx4SFmode))))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx8SImode)\n\
       || register_operand (operands[2], VNx8SImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8SImode)
       || register_operand (operands[2], VNx8SImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8SImode)
       || register_operand (operands[2], VNx8SImode)))
    : -1 },
#line 6060 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && TARGET_SM4",
    __builtin_constant_p 
#line 6060 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_SM4)
    ? (int) 
#line 6060 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_SM4)
    : -1 },
  { "(TARGET_FLOAT) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 5531 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 5531 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V2SFmode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V2SFmode))",
    __builtin_constant_p 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V2SFmode)
       || aarch64_simd_reg_or_zero (operands[1], V2SFmode)))
    ? (int) 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V2SFmode)
       || aarch64_simd_reg_or_zero (operands[1], V2SFmode)))
    : -1 },
#line 5917 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && TARGET_SHA2",
    __builtin_constant_p 
#line 5917 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_SHA2)
    ? (int) 
#line 5917 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_SHA2)
    : -1 },
  { "(INTVAL (operands[5]) == GET_MODE_SIZE (DImode)) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 1452 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (DImode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 1452 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (DImode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && ENDIAN_LANE_N (16, INTVAL (operands[2])) == 0",
    __builtin_constant_p 
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && ENDIAN_LANE_N (16, INTVAL (operands[2])) == 0)
    ? (int) 
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && ENDIAN_LANE_N (16, INTVAL (operands[2])) == 0)
    : -1 },
#line 4596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[3]) < GET_MODE_BITSIZE (SImode) &&\n\
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == GET_MODE_BITSIZE (SImode))",
    __builtin_constant_p 
#line 4596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[3]) < GET_MODE_BITSIZE (SImode) &&
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == GET_MODE_BITSIZE (SImode)))
    ? (int) 
#line 4596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[3]) < GET_MODE_BITSIZE (SImode) &&
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == GET_MODE_BITSIZE (SImode)))
    : -1 },
#line 4850 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_mask_and_shift_for_ubfiz_p (DImode, operands[3], operands[2])",
    __builtin_constant_p 
#line 4850 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_mask_and_shift_for_ubfiz_p (DImode, operands[3], operands[2]))
    ? (int) 
#line 4850 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_mask_and_shift_for_ubfiz_p (DImode, operands[3], operands[2]))
    : -1 },
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (HImode), 0, 255)",
    __builtin_constant_p 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (HImode), 0, 255))
    ? (int) 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (HImode), 0, 255))
    : -1 },
#line 1983 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,\n\
		GET_MODE_BITSIZE (GET_MODE_INNER (V2SFmode)))",
    __builtin_constant_p 
#line 1983 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (GET_MODE_INNER (V2SFmode))))
    ? (int) 
#line 1983 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (GET_MODE_INNER (V2SFmode))))
    : -1 },
#line 477 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_FLOAT && TARGET_SIMD",
    __builtin_constant_p 
#line 477 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_FLOAT && TARGET_SIMD)
    ? (int) 
#line 477 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_FLOAT && TARGET_SIMD)
    : -1 },
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[0], 0),\n\
				  GET_MODE_SIZE (V4HFmode)))",
    __builtin_constant_p 
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V4HFmode))))
    ? (int) 
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V4HFmode))))
    : -1 },
#line 877 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "SIBLING_CALL_P (insn)",
    __builtin_constant_p 
#line 877 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(SIBLING_CALL_P (insn))
    ? (int) 
#line 877 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(SIBLING_CALL_P (insn))
    : -1 },
#line 1361 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "rtx_equal_p (XEXP (operands[2], 0),\n\
		plus_constant (Pmode,\n\
			       XEXP (operands[0], 0),\n\
			       GET_MODE_SIZE (DImode)))",
    __builtin_constant_p 
#line 1361 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (DImode))))
    ? (int) 
#line 1361 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (DImode))))
    : -1 },
  { "(TARGET_FLOAT) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 5531 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 5531 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (SFmode), 0, 255)",
    __builtin_constant_p 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (SFmode), 0, 255))
    ? (int) 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (SFmode), 0, 255))
    : -1 },
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 0, 63)",
    __builtin_constant_p 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 0, 63))
    ? (int) 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 0, 63))
    : -1 },
#line 2504 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "INTVAL (operands[3]) == -INTVAL (operands[2])",
    __builtin_constant_p 
#line 2504 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[3]) == -INTVAL (operands[2]))
    ? (int) 
#line 2504 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[3]) == -INTVAL (operands[2]))
    : -1 },
  { "(!aarch64_move_imm (INTVAL (operands[1]), SImode)\n\
   && !aarch64_plus_operand (operands[1], SImode)\n\
   && !reload_completed) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V8QImode)",
    __builtin_constant_p 
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V8QImode))
    ? (int) 
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V8QImode))
    : -1 },
#line 232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_adjust_ok_for_ldpstp (operands, true, SFmode)",
    __builtin_constant_p 
#line 232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, true, SFmode))
    ? (int) 
#line 232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, true, SFmode))
    : -1 },
  { "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DImode)) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 1485 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DImode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 1485 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DImode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V2SImode)",
    __builtin_constant_p 
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V2SImode))
    ? (int) 
#line 126 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V2SImode))
    : -1 },
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && GET_MODE_NUNITS (VNx16QImode).is_constant ()",
    __builtin_constant_p 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx16QImode).is_constant ())
    ? (int) 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx16QImode).is_constant ())
    : -1 },
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 16, 63)",
    __builtin_constant_p 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 16, 63))
    ? (int) 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 16, 63))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx16HFmode)\n\
       || register_operand (operands[2], VNx16HFmode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16HFmode)
       || register_operand (operands[2], VNx16HFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16HFmode)
       || register_operand (operands[2], VNx16HFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 26 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, true, DImode)",
    __builtin_constant_p 
#line 26 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, DImode))
    ? (int) 
#line 26 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, DImode))
    : -1 },
  { "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SFmode)) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 1501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SFmode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 1501 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SFmode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && rtx_equal_p (XEXP (operands[3], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (V4HImode)))",
    __builtin_constant_p 
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HImode))))
    ? (int) 
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HImode))))
    : -1 },
  { "(INTVAL (operands[5]) == GET_MODE_SIZE (DFmode)) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 1467 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (DFmode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 1467 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (DFmode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
#line 2784 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_uxt_size (exact_log2 (INTVAL (operands[2])),INTVAL (operands[3])) != 0",
    __builtin_constant_p 
#line 2784 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_uxt_size (exact_log2 (INTVAL (operands[2])),INTVAL (operands[3])) != 0)
    ? (int) 
#line 2784 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_uxt_size (exact_log2 (INTVAL (operands[2])),INTVAL (operands[3])) != 0)
    : -1 },
  { "(INTVAL (operands[5]) == GET_MODE_SIZE (DImode)) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 1452 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (DImode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 1452 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (DImode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx16HImode)\n\
       || register_operand (operands[2], VNx16HImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16HImode)
       || register_operand (operands[2], VNx16HImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16HImode)
       || register_operand (operands[2], VNx16HImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_adjust_ok_for_ldpstp (operands, true, DFmode)",
    __builtin_constant_p 
#line 232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, true, DFmode))
    ? (int) 
#line 232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, true, DFmode))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx32QImode)\n\
       || register_operand (operands[2], VNx32QImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32QImode)
       || register_operand (operands[2], VNx32QImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32QImode)
       || register_operand (operands[2], VNx32QImode)))
    : -1 },
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (QImode) >= 64",
    __builtin_constant_p 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (QImode) >= 64)
    ? (int) 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (QImode) >= 64)
    : -1 },
#line 458 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
  { " epilogue_completed",
    __builtin_constant_p 
#line 458 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
( epilogue_completed)
    ? (int) 
#line 458 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
( epilogue_completed)
    : -1 },
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V4HFmode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V4HFmode))",
    __builtin_constant_p 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V4HFmode)
       || aarch64_simd_reg_or_zero (operands[1], V4HFmode)))
    ? (int) 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V4HFmode)
       || aarch64_simd_reg_or_zero (operands[1], V4HFmode)))
    : -1 },
#line 4821 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!(UINTVAL (operands[1]) == 0\n\
     || (UINTVAL (operands[3]) + UINTVAL (operands[1])\n\
	 > GET_MODE_BITSIZE (SImode)))",
    __builtin_constant_p 
#line 4821 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[3]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (SImode))))
    ? (int) 
#line 4821 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[3]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (SImode))))
    : -1 },
  { "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DImode)) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 1485 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DImode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 1485 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DImode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
#line 5068 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !BYTES_BIG_ENDIAN\n\
   && (register_operand (operands[0], CImode)\n\
       || register_operand (operands[1], CImode))",
    __builtin_constant_p 
#line 5068 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !BYTES_BIG_ENDIAN
   && (register_operand (operands[0], CImode)
       || register_operand (operands[1], CImode)))
    ? (int) 
#line 5068 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !BYTES_BIG_ENDIAN
   && (register_operand (operands[0], CImode)
       || register_operand (operands[1], CImode)))
    : -1 },
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 0, 63)",
    __builtin_constant_p 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 0, 63))
    ? (int) 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 0, 63))
    : -1 },
  { "(INTVAL (operands[5]) == GET_MODE_SIZE (SImode)) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 1452 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (SImode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 1452 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (SImode)) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx32HImode)\n\
       || register_operand (operands[2], VNx32HImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32HImode)
       || register_operand (operands[2], VNx32HImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32HImode)
       || register_operand (operands[2], VNx32HImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 1879 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_zero_extend_const_eq (TImode, operands[3],\n\
                                 DImode, operands[2])",
    __builtin_constant_p 
#line 1879 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_zero_extend_const_eq (TImode, operands[3],
                                 DImode, operands[2]))
    ? (int) 
#line 1879 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_zero_extend_const_eq (TImode, operands[3],
                                 DImode, operands[2]))
    : -1 },
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 16, 63)",
    __builtin_constant_p 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 16, 63))
    ? (int) 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 16, 63))
    : -1 },
#line 666 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_use_return_insn_p ()",
    __builtin_constant_p 
#line 666 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_use_return_insn_p ())
    ? (int) 
#line 666 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_use_return_insn_p ())
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx32QImode)\n\
       || register_operand (operands[2], VNx32QImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32QImode)
       || register_operand (operands[2], VNx32QImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx32QImode)
       || register_operand (operands[2], VNx32QImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 0, 63)",
    __builtin_constant_p 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 0, 63))
    ? (int) 
#line 875 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 0, 63))
    : -1 },
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && !BYTES_BIG_ENDIAN\n\
   && ((lra_in_progress || reload_completed)\n\
       || (register_operand (operands[0], VNx2DImode)\n\
	   && nonmemory_operand (operands[1], VNx2DImode)))",
    __builtin_constant_p 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx2DImode)
	   && nonmemory_operand (operands[1], VNx2DImode))))
    ? (int) 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx2DImode)
	   && nonmemory_operand (operands[1], VNx2DImode))))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx12SImode)\n\
       || register_operand (operands[2], VNx12SImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx12SImode)
       || register_operand (operands[2], VNx12SImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx12SImode)
       || register_operand (operands[2], VNx12SImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 46 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, false, DImode)",
    __builtin_constant_p 
#line 46 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, false, DImode))
    ? (int) 
#line 46 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, false, DImode))
    : -1 },
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx2BImode)\n\
       || register_operand (operands[1], VNx2BImode))",
    __builtin_constant_p 
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx2BImode)
       || register_operand (operands[1], VNx2BImode)))
    ? (int) 
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx2BImode)
       || register_operand (operands[1], VNx2BImode)))
    : -1 },
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V4HImode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V4HImode))",
    __builtin_constant_p 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V4HImode)
       || aarch64_simd_reg_or_zero (operands[1], V4HImode)))
    ? (int) 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V4HImode)
       || aarch64_simd_reg_or_zero (operands[1], V4HImode)))
    : -1 },
#line 66 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, true, SFmode)",
    __builtin_constant_p 
#line 66 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, SFmode))
    ? (int) 
#line 66 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, SFmode))
    : -1 },
  { "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SImode)) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 1485 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SImode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 1485 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SImode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
#line 1184 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT && (register_operand (operands[0], SFmode)\n\
    || aarch64_reg_or_fp_zero (operands[1], SFmode))",
    __builtin_constant_p 
#line 1184 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (register_operand (operands[0], SFmode)
    || aarch64_reg_or_fp_zero (operands[1], SFmode)))
    ? (int) 
#line 1184 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (register_operand (operands[0], SFmode)
    || aarch64_reg_or_fp_zero (operands[1], SFmode)))
    : -1 },
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V2DImode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V2DImode))",
    __builtin_constant_p 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V2DImode)
       || aarch64_simd_reg_or_zero (operands[1], V2DImode)))
    ? (int) 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V2DImode)
       || aarch64_simd_reg_or_zero (operands[1], V2DImode)))
    : -1 },
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 16, 63)",
    __builtin_constant_p 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 16, 63))
    ? (int) 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 16, 63))
    : -1 },
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (HFmode), 0, 255)",
    __builtin_constant_p 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (HFmode), 0, 255))
    ? (int) 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (HFmode), 0, 255))
    : -1 },
#line 296 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_adjust_ok_for_ldpstp (operands, true, SImode)",
    __builtin_constant_p 
#line 296 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, true, SImode))
    ? (int) 
#line 296 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, true, SImode))
    : -1 },
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (SImode), 0, 255)",
    __builtin_constant_p 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (SImode), 0, 255))
    ? (int) 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (SImode), 0, 255))
    : -1 },
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 1, 15)",
    __builtin_constant_p 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 1, 15))
    ? (int) 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 1, 15))
    : -1 },
  { "(!aarch64_move_imm (INTVAL (operands[1]), SImode)\n\
   && !aarch64_plus_operand (operands[1], SImode)\n\
   && !reload_completed) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
#line 1395 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "rtx_equal_p (XEXP (operands[3], 0),\n\
		plus_constant (Pmode,\n\
			       XEXP (operands[1], 0),\n\
			       GET_MODE_SIZE (DFmode)))",
    __builtin_constant_p 
#line 1395 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (DFmode))))
    ? (int) 
#line 1395 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (DFmode))))
    : -1 },
#line 6001 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && TARGET_SHA3",
    __builtin_constant_p 
#line 6001 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_SHA3)
    ? (int) 
#line 6001 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && TARGET_SHA3)
    : -1 },
#line 4345 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)",
    __builtin_constant_p 
#line 4345 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0))
    ? (int) 
#line 4345 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx8DImode)\n\
       || register_operand (operands[2], VNx8DImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8DImode)
       || register_operand (operands[2], VNx8DImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8DImode)
       || register_operand (operands[2], VNx8DImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[0], 0),\n\
				  GET_MODE_SIZE (V4HImode)))",
    __builtin_constant_p 
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V4HImode))))
    ? (int) 
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V4HImode))))
    : -1 },
#line 3102 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && BYTES_BIG_ENDIAN",
    __builtin_constant_p 
#line 3102 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && BYTES_BIG_ENDIAN)
    ? (int) 
#line 3102 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && BYTES_BIG_ENDIAN)
    : -1 },
  { "(INTVAL (operands[5]) == GET_MODE_SIZE (SImode)) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 1452 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (SImode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 1452 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[5]) == GET_MODE_SIZE (SImode)) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V16QImode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V16QImode))",
    __builtin_constant_p 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V16QImode)
       || aarch64_simd_reg_or_zero (operands[1], V16QImode)))
    ? (int) 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V16QImode)
       || aarch64_simd_reg_or_zero (operands[1], V16QImode)))
    : -1 },
  { "(!aarch64_move_imm (INTVAL (operands[2]), DImode)\n\
   && !aarch64_plus_operand (operands[2], DImode)\n\
   && !reload_completed) && ( true)",
    __builtin_constant_p (
#line 3268 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[2]), DImode)
   && !aarch64_plus_operand (operands[2], DImode)
   && !reload_completed) && 
#line 3272 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) (
#line 3268 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[2]), DImode)
   && !aarch64_plus_operand (operands[2], DImode)
   && !reload_completed) && 
#line 3272 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && ENDIAN_LANE_N (2, INTVAL (operands[2])) == 0",
    __builtin_constant_p 
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && ENDIAN_LANE_N (2, INTVAL (operands[2])) == 0)
    ? (int) 
#line 174 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && ENDIAN_LANE_N (2, INTVAL (operands[2])) == 0)
    : -1 },
  { "(TARGET_TLS_DESC) && (ptr_mode == DImode)",
    __builtin_constant_p (
#line 5814 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode))
    ? (int) (
#line 5814 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx12SFmode)\n\
       || register_operand (operands[2], VNx12SFmode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx12SFmode)
       || register_operand (operands[2], VNx12SFmode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx12SFmode)
       || register_operand (operands[2], VNx12SFmode)))
    : -1 },
#line 4821 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!(UINTVAL (operands[1]) == 0\n\
     || (UINTVAL (operands[3]) + UINTVAL (operands[1])\n\
	 > GET_MODE_BITSIZE (DImode)))",
    __builtin_constant_p 
#line 4821 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[3]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (DImode))))
    ? (int) 
#line 4821 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[3]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (DImode))))
    : -1 },
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && !BYTES_BIG_ENDIAN\n\
   && ((lra_in_progress || reload_completed)\n\
       || (register_operand (operands[0], VNx8HImode)\n\
	   && nonmemory_operand (operands[1], VNx8HImode)))",
    __builtin_constant_p 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx8HImode)
	   && nonmemory_operand (operands[1], VNx8HImode))))
    ? (int) 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx8HImode)
	   && nonmemory_operand (operands[1], VNx8HImode))))
    : -1 },
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && !BYTES_BIG_ENDIAN\n\
   && ((lra_in_progress || reload_completed)\n\
       || (register_operand (operands[0], VNx8HFmode)\n\
	   && nonmemory_operand (operands[1], VNx8HFmode)))",
    __builtin_constant_p 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx8HFmode)
	   && nonmemory_operand (operands[1], VNx8HFmode))))
    ? (int) 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx8HFmode)
	   && nonmemory_operand (operands[1], VNx8HFmode))))
    : -1 },
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, true, V4HFmode)",
    __builtin_constant_p 
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, V4HFmode))
    ? (int) 
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, V4HFmode))
    : -1 },
  { "(TARGET_FLOAT && TARGET_SIMD) && (TARGET_SIMD_F16INST)",
    __builtin_constant_p (
#line 477 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_FLOAT && TARGET_SIMD) && 
#line 103 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(TARGET_SIMD_F16INST))
    ? (int) (
#line 477 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_FLOAT && TARGET_SIMD) && 
#line 103 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(TARGET_SIMD_F16INST))
    : -1 },
#line 86 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, false, DFmode)",
    __builtin_constant_p 
#line 86 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, false, DFmode))
    ? (int) 
#line 86 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, false, DFmode))
    : -1 },
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (QImode), 0, 255)",
    __builtin_constant_p 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (QImode), 0, 255))
    ? (int) 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (QImode), 0, 255))
    : -1 },
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (DImode), 0, 255)",
    __builtin_constant_p 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (DImode), 0, 255))
    ? (int) 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (DImode), 0, 255))
    : -1 },
#line 4281 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "(~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0",
    __builtin_constant_p 
#line 4281 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)
    ? (int) 
#line 4281 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)
    : -1 },
#line 3381 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!((operands[3] == const1_rtx && operands[4] == constm1_rtx)\n\
     || (operands[3] == constm1_rtx && operands[4] == const1_rtx))",
    __builtin_constant_p 
#line 3381 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!((operands[3] == const1_rtx && operands[4] == constm1_rtx)
     || (operands[3] == constm1_rtx && operands[4] == const1_rtx)))
    ? (int) 
#line 3381 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!((operands[3] == const1_rtx && operands[4] == constm1_rtx)
     || (operands[3] == constm1_rtx && operands[4] == const1_rtx)))
    : -1 },
#line 223 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && reload_completed\n\
   && ((FP_REGNUM_P (REGNO (operands[0])) && GP_REGNUM_P (REGNO (operands[1])))\n\
       || (GP_REGNUM_P (REGNO (operands[0])) && FP_REGNUM_P (REGNO (operands[1]))))",
    __builtin_constant_p 
#line 223 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && reload_completed
   && ((FP_REGNUM_P (REGNO (operands[0])) && GP_REGNUM_P (REGNO (operands[1])))
       || (GP_REGNUM_P (REGNO (operands[0])) && FP_REGNUM_P (REGNO (operands[1])))))
    ? (int) 
#line 223 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && reload_completed
   && ((FP_REGNUM_P (REGNO (operands[0])) && GP_REGNUM_P (REGNO (operands[1])))
       || (GP_REGNUM_P (REGNO (operands[0])) && FP_REGNUM_P (REGNO (operands[1])))))
    : -1 },
#line 4797 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!(UINTVAL (operands[1]) == 0\n\
     || (UINTVAL (operands[2]) + UINTVAL (operands[1])\n\
	 > GET_MODE_BITSIZE (DImode)))",
    __builtin_constant_p 
#line 4797 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[2]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (DImode))))
    ? (int) 
#line 4797 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[2]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (DImode))))
    : -1 },
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && GET_MODE_NUNITS (VNx2DFmode).is_constant ()",
    __builtin_constant_p 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx2DFmode).is_constant ())
    ? (int) 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx2DFmode).is_constant ())
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx24HFmode)\n\
       || register_operand (operands[2], VNx24HFmode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx24HFmode)
       || register_operand (operands[2], VNx24HFmode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx24HFmode)
       || register_operand (operands[2], VNx24HFmode)))
    : -1 },
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 16, 63)",
    __builtin_constant_p 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 16, 63))
    ? (int) 
#line 581 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 16, 63))
    : -1 },
  { "(TARGET_TLS_DESC) && (ptr_mode == SImode)",
    __builtin_constant_p (
#line 5814 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode))
    ? (int) (
#line 5814 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_TLS_DESC) && 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode))
    : -1 },
#line 4915 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch_rev16_shleft_mask_imm_p (operands[3], SImode)\n\
   && aarch_rev16_shright_mask_imm_p (operands[2], SImode)",
    __builtin_constant_p 
#line 4915 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch_rev16_shleft_mask_imm_p (operands[3], SImode)
   && aarch_rev16_shright_mask_imm_p (operands[2], SImode))
    ? (int) 
#line 4915 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch_rev16_shleft_mask_imm_p (operands[3], SImode)
   && aarch_rev16_shright_mask_imm_p (operands[2], SImode))
    : -1 },
#line 6052 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "optimize > 0 && flag_modulo_sched",
    __builtin_constant_p 
#line 6052 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(optimize > 0 && flag_modulo_sched)
    ? (int) 
#line 6052 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(optimize > 0 && flag_modulo_sched)
    : -1 },
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx4SFmode)\n\
       || register_operand (operands[2], VNx4SFmode))",
    __builtin_constant_p 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4SFmode)
       || register_operand (operands[2], VNx4SFmode)))
    ? (int) 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4SFmode)
       || register_operand (operands[2], VNx4SFmode)))
    : -1 },
#line 4363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)",
    __builtin_constant_p 
#line 4363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode))
    ? (int) 
#line 4363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode))
    : -1 },
  { "(!aarch64_move_imm (INTVAL (operands[1]), DImode)\n\
   && !aarch64_plus_operand (operands[1], DImode)\n\
   && !reload_completed) && (ptr_mode == SImode || Pmode == SImode)",
    __builtin_constant_p (
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    ? (int) (
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx6DImode)\n\
       || register_operand (operands[2], VNx6DImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx6DImode)
       || register_operand (operands[2], VNx6DImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx6DImode)
       || register_operand (operands[2], VNx6DImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && rtx_equal_p (XEXP (operands[3], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (V2SImode)))",
    __builtin_constant_p 
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SImode))))
    ? (int) 
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SImode))))
    : -1 },
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && GET_MODE_NUNITS (VNx4SFmode).is_constant ()",
    __builtin_constant_p 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx4SFmode).is_constant ())
    ? (int) 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx4SFmode).is_constant ())
    : -1 },
#line 1019 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "CONST_INT_P (operands[1]) && !aarch64_move_imm (INTVAL (operands[1]), SImode)\n\
    && REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0]))",
    __builtin_constant_p 
#line 1019 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(CONST_INT_P (operands[1]) && !aarch64_move_imm (INTVAL (operands[1]), SImode)
    && REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0])))
    ? (int) 
#line 1019 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(CONST_INT_P (operands[1]) && !aarch64_move_imm (INTVAL (operands[1]), SImode)
    && REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0])))
    : -1 },
  { "((!aarch64_move_imm (INTVAL (operands[1]), DImode)\n\
   && !aarch64_plus_operand (operands[1], DImode)\n\
   && !reload_completed) && (ptr_mode == DImode || Pmode == DImode)) && ( true)",
    __builtin_constant_p ((
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode)) && 
#line 500 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) ((
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode)) && 
#line 500 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 66 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, true, DFmode)",
    __builtin_constant_p 
#line 66 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, DFmode))
    ? (int) 
#line 66 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, DFmode))
    : -1 },
#line 3634 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "true",
    __builtin_constant_p 
#line 3634 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(true)
    ? (int) 
#line 3634 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(true)
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx8DImode)\n\
       || register_operand (operands[2], VNx8DImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8DImode)
       || register_operand (operands[2], VNx8DImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8DImode)
       || register_operand (operands[2], VNx8DImode)))
    : -1 },
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[0], 0),\n\
				  GET_MODE_SIZE (V8QImode)))",
    __builtin_constant_p 
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V8QImode))))
    ? (int) 
#line 199 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V8QImode))))
    : -1 },
#line 5068 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !BYTES_BIG_ENDIAN\n\
   && (register_operand (operands[0], XImode)\n\
       || register_operand (operands[1], XImode))",
    __builtin_constant_p 
#line 5068 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !BYTES_BIG_ENDIAN
   && (register_operand (operands[0], XImode)
       || register_operand (operands[1], XImode)))
    ? (int) 
#line 5068 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !BYTES_BIG_ENDIAN
   && (register_operand (operands[0], XImode)
       || register_operand (operands[1], XImode)))
    : -1 },
#line 432 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
  { "TARGET_LSE",
    __builtin_constant_p 
#line 432 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
(TARGET_LSE)
    ? (int) 
#line 432 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
(TARGET_LSE)
    : -1 },
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, true, V8QImode)",
    __builtin_constant_p 
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, V8QImode))
    ? (int) 
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, V8QImode))
    : -1 },
#line 670 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
  { "TARGET_LSE && reload_completed",
    __builtin_constant_p 
#line 670 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
(TARGET_LSE && reload_completed)
    ? (int) 
#line 670 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/atomics.md"
(TARGET_LSE && reload_completed)
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx64QImode)\n\
       || register_operand (operands[2], VNx64QImode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx64QImode)
       || register_operand (operands[2], VNx64QImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx64QImode)
       || register_operand (operands[2], VNx64QImode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && rtx_equal_p (XEXP (operands[3], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (V8QImode)))",
    __builtin_constant_p 
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V8QImode))))
    ? (int) 
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V8QImode))))
    : -1 },
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx4SImode)\n\
       || register_operand (operands[2], VNx4SImode))",
    __builtin_constant_p 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4SImode)
       || register_operand (operands[2], VNx4SImode)))
    ? (int) 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4SImode)
       || register_operand (operands[2], VNx4SImode)))
    : -1 },
#line 5116 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && BYTES_BIG_ENDIAN\n\
   && (register_operand (operands[0], CImode)\n\
       || register_operand (operands[1], CImode))",
    __builtin_constant_p 
#line 5116 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && BYTES_BIG_ENDIAN
   && (register_operand (operands[0], CImode)
       || register_operand (operands[1], CImode)))
    ? (int) 
#line 5116 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && BYTES_BIG_ENDIAN
   && (register_operand (operands[0], CImode)
       || register_operand (operands[1], CImode)))
    : -1 },
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && !BYTES_BIG_ENDIAN\n\
   && ((lra_in_progress || reload_completed)\n\
       || (register_operand (operands[0], VNx4SImode)\n\
	   && nonmemory_operand (operands[1], VNx4SImode)))",
    __builtin_constant_p 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx4SImode)
	   && nonmemory_operand (operands[1], VNx4SImode))))
    ? (int) 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx4SImode)
	   && nonmemory_operand (operands[1], VNx4SImode))))
    : -1 },
#line 1295 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!STRICT_ALIGNMENT",
    __builtin_constant_p 
#line 1295 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!STRICT_ALIGNMENT)
    ? (int) 
#line 1295 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!STRICT_ALIGNMENT)
    : -1 },
#line 2328 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_uxt_size (exact_log2 (INTVAL (operands[2])), INTVAL (operands[3])) != 0",
    __builtin_constant_p 
#line 2328 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_uxt_size (exact_log2 (INTVAL (operands[2])), INTVAL (operands[3])) != 0)
    ? (int) 
#line 2328 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_uxt_size (exact_log2 (INTVAL (operands[2])), INTVAL (operands[3])) != 0)
    : -1 },
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (DFmode) >= 64",
    __builtin_constant_p 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (DFmode) >= 64)
    ? (int) 
#line 596 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (DFmode) >= 64)
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx6DFmode)\n\
       || register_operand (operands[2], VNx6DFmode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx6DFmode)
       || register_operand (operands[2], VNx6DFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx6DFmode)
       || register_operand (operands[2], VNx6DFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 46 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, false, SImode)",
    __builtin_constant_p 
#line 46 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, false, SImode))
    ? (int) 
#line 46 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, false, SImode))
    : -1 },
  { "(!aarch64_move_imm (INTVAL (operands[1]), DImode)\n\
   && !aarch64_plus_operand (operands[1], DImode)\n\
   && !reload_completed) && (ptr_mode == DImode || Pmode == DImode)",
    __builtin_constant_p (
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    ? (int) (
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && 
#line 91 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == DImode || Pmode == DImode))
    : -1 },
#line 1232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "can_create_pseudo_p ()\n\
   && !aarch64_can_const_movi_rtx_p (operands[1], SFmode)\n\
   && !aarch64_float_const_representable_p (operands[1])\n\
   &&  aarch64_float_const_rtx_p (operands[1])",
    __builtin_constant_p 
#line 1232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(can_create_pseudo_p ()
   && !aarch64_can_const_movi_rtx_p (operands[1], SFmode)
   && !aarch64_float_const_representable_p (operands[1])
   &&  aarch64_float_const_rtx_p (operands[1]))
    ? (int) 
#line 1232 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(can_create_pseudo_p ()
   && !aarch64_can_const_movi_rtx_p (operands[1], SFmode)
   && !aarch64_float_const_representable_p (operands[1])
   &&  aarch64_float_const_rtx_p (operands[1]))
    : -1 },
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_ok_for_ldpstp (operands, true, V2SFmode)",
    __builtin_constant_p 
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, V2SFmode))
    ? (int) 
#line 106 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_ok_for_ldpstp (operands, true, V2SFmode))
    : -1 },
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx16QImode)\n\
       || register_operand (operands[2], VNx16QImode))",
    __builtin_constant_p 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16QImode)
       || register_operand (operands[2], VNx16QImode)))
    ? (int) 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16QImode)
       || register_operand (operands[2], VNx16QImode)))
    : -1 },
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (DFmode), 0, 255)",
    __builtin_constant_p 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (DFmode), 0, 255))
    ? (int) 
#line 888 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (DFmode), 0, 255))
    : -1 },
  { "(!aarch64_move_imm (INTVAL (operands[2]), SImode)\n\
   && !aarch64_plus_operand (operands[2], SImode)\n\
   && !reload_completed) && ( true)",
    __builtin_constant_p (
#line 3268 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[2]), SImode)
   && !aarch64_plus_operand (operands[2], SImode)
   && !reload_completed) && 
#line 3272 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) (
#line 3268 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[2]), SImode)
   && !aarch64_plus_operand (operands[2], SImode)
   && !reload_completed) && 
#line 3272 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 1, 15)",
    __builtin_constant_p 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 1, 15))
    ? (int) 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 1, 15))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx64QImode)\n\
       || register_operand (operands[2], VNx64QImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx64QImode)
       || register_operand (operands[2], VNx64QImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx64QImode)
       || register_operand (operands[2], VNx64QImode)))
    : -1 },
#line 4610 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[3]) < GET_MODE_BITSIZE (DImode)\n\
   && (UINTVAL (operands[3]) + UINTVAL (operands[4])\n\
       == GET_MODE_BITSIZE (DImode))",
    __builtin_constant_p 
#line 4610 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[3]) < GET_MODE_BITSIZE (DImode)
   && (UINTVAL (operands[3]) + UINTVAL (operands[4])
       == GET_MODE_BITSIZE (DImode)))
    ? (int) 
#line 4610 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[3]) < GET_MODE_BITSIZE (DImode)
   && (UINTVAL (operands[3]) + UINTVAL (operands[4])
       == GET_MODE_BITSIZE (DImode)))
    : -1 },
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx8HFmode)\n\
       || register_operand (operands[2], VNx8HFmode))",
    __builtin_constant_p 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8HFmode)
       || register_operand (operands[2], VNx8HFmode)))
    ? (int) 
#line 179 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx8HFmode)
       || register_operand (operands[2], VNx8HFmode)))
    : -1 },
#line 1078 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[1]) < GET_MODE_BITSIZE (DImode)\n\
   && UINTVAL (operands[1]) % 16 == 0",
    __builtin_constant_p 
#line 1078 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[1]) < GET_MODE_BITSIZE (DImode)
   && UINTVAL (operands[1]) % 16 == 0)
    ? (int) 
#line 1078 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[1]) < GET_MODE_BITSIZE (DImode)
   && UINTVAL (operands[1]) % 16 == 0)
    : -1 },
#line 4648 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[2]) < GET_MODE_BITSIZE (SImode)",
    __builtin_constant_p 
#line 4648 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < GET_MODE_BITSIZE (SImode))
    ? (int) 
#line 4648 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < GET_MODE_BITSIZE (SImode))
    : -1 },
#line 4833 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[2]) < 32",
    __builtin_constant_p 
#line 4833 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < 32)
    ? (int) 
#line 4833 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[2]) < 32)
    : -1 },
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && rtx_equal_p (XEXP (operands[3], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (V4HFmode)))",
    __builtin_constant_p 
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HFmode))))
    ? (int) 
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HFmode))))
    : -1 },
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 1, 15)",
    __builtin_constant_p 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 1, 15))
    ? (int) 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 1, 15))
    : -1 },
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE && GET_MODE_NUNITS (VNx4SImode).is_constant ()",
    __builtin_constant_p 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx4SImode).is_constant ())
    ? (int) 
#line 793 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE && GET_MODE_NUNITS (VNx4SImode).is_constant ())
    : -1 },
#line 200 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
  { "aarch64_operands_adjust_ok_for_ldpstp (operands, true, DImode)",
    __builtin_constant_p 
#line 200 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, true, DImode))
    ? (int) 
#line 200 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-ldpstp.md"
(aarch64_operands_adjust_ok_for_ldpstp (operands, true, DImode))
    : -1 },
#line 2296 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_uxt_size (INTVAL (operands[2]), INTVAL (operands[3])) != 0",
    __builtin_constant_p 
#line 2296 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_uxt_size (INTVAL (operands[2]), INTVAL (operands[3])) != 0)
    ? (int) 
#line 2296 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_uxt_size (INTVAL (operands[2]), INTVAL (operands[3])) != 0)
    : -1 },
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && !BYTES_BIG_ENDIAN\n\
   && ((lra_in_progress || reload_completed)\n\
       || (register_operand (operands[0], VNx16QImode)\n\
	   && nonmemory_operand (operands[1], VNx16QImode)))",
    __builtin_constant_p 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx16QImode)
	   && nonmemory_operand (operands[1], VNx16QImode))))
    ? (int) 
#line 122 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx16QImode)
	   && nonmemory_operand (operands[1], VNx16QImode))))
    : -1 },
  { "(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)\n\
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)) && ( true)",
    __builtin_constant_p (
#line 4318 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)) && 
#line 4321 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) (
#line 4318 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)) && 
#line 4321 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 1161 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT && (register_operand (operands[0], HFmode)\n\
    || aarch64_reg_or_fp_zero (operands[1], HFmode))",
    __builtin_constant_p 
#line 1161 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (register_operand (operands[0], HFmode)
    || aarch64_reg_or_fp_zero (operands[1], HFmode)))
    ? (int) 
#line 1161 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && (register_operand (operands[0], HFmode)
    || aarch64_reg_or_fp_zero (operands[1], HFmode)))
    : -1 },
#line 1345 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "rtx_equal_p (XEXP (operands[2], 0),\n\
		plus_constant (Pmode,\n\
			       XEXP (operands[0], 0),\n\
			       GET_MODE_SIZE (SImode)))",
    __builtin_constant_p 
#line 1345 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (SImode))))
    ? (int) 
#line 1345 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (SImode))))
    : -1 },
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx6DImode)\n\
       || register_operand (operands[2], VNx6DImode))",
    __builtin_constant_p 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx6DImode)
       || register_operand (operands[2], VNx6DImode)))
    ? (int) 
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx6DImode)
       || register_operand (operands[2], VNx6DImode)))
    : -1 },
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 1, 15)",
    __builtin_constant_p 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 1, 15))
    ? (int) 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 1, 15))
    : -1 },
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD && !STRICT_ALIGNMENT\n\
   && rtx_equal_p (XEXP (operands[2], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (V8QImode)))",
    __builtin_constant_p 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V8QImode))))
    ? (int) 
#line 3058 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V8QImode))))
    : -1 },
#line 5308 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "TARGET_FLOAT && !flag_rounding_math",
    __builtin_constant_p 
#line 5308 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && !flag_rounding_math)
    ? (int) 
#line 5308 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(TARGET_FLOAT && !flag_rounding_math)
    : -1 },
  { "((!aarch64_move_imm (INTVAL (operands[1]), DImode)\n\
   && !aarch64_plus_operand (operands[1], DImode)\n\
   && !reload_completed) && (ptr_mode == SImode || Pmode == SImode)) && ( true)",
    __builtin_constant_p ((
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode)) && 
#line 500 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) ((
#line 496 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && 
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/iterators.md"
(ptr_mode == SImode || Pmode == SImode)) && 
#line 500 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 1, 15)",
    __builtin_constant_p 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 1, 15))
    ? (int) 
#line 555 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 1, 15))
    : -1 },
#line 1983 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,\n\
		GET_MODE_BITSIZE (GET_MODE_INNER (V4SFmode)))",
    __builtin_constant_p 
#line 1983 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (GET_MODE_INNER (V4SFmode))))
    ? (int) 
#line 1983 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (GET_MODE_INNER (V4SFmode))))
    : -1 },
#line 5026 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!HONOR_SIGNED_ZEROS (SFmode) && TARGET_FLOAT",
    __builtin_constant_p 
#line 5026 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!HONOR_SIGNED_ZEROS (SFmode) && TARGET_FLOAT)
    ? (int) 
#line 5026 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!HONOR_SIGNED_ZEROS (SFmode) && TARGET_FLOAT)
    : -1 },
  { "(INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)) && ( true)",
    __builtin_constant_p (
#line 4363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)) && 
#line 4365 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    ? (int) (
#line 4363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)) && 
#line 4365 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
( true))
    : -1 },
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && (register_operand (operands[0], V8HFmode)\n\
       || aarch64_simd_reg_or_zero (operands[1], V8HFmode))",
    __builtin_constant_p 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V8HFmode)
       || aarch64_simd_reg_or_zero (operands[1], V8HFmode)))
    ? (int) 
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && (register_operand (operands[0], V8HFmode)
       || aarch64_simd_reg_or_zero (operands[1], V8HFmode)))
    : -1 },
  { "(TARGET_SVE\n\
   && (register_operand (operands[0], VNx4DFmode)\n\
       || register_operand (operands[2], VNx4DFmode))) && ( reload_completed)",
    __builtin_constant_p (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4DFmode)
       || register_operand (operands[2], VNx4DFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    ? (int) (
#line 409 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx4DFmode)
       || register_operand (operands[2], VNx4DFmode))) && 
#line 413 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
( reload_completed))
    : -1 },
#line 2752 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_uxt_size (INTVAL (operands[2]),INTVAL (operands[3])) != 0",
    __builtin_constant_p 
#line 2752 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_uxt_size (INTVAL (operands[2]),INTVAL (operands[3])) != 0)
    ? (int) 
#line 2752 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_uxt_size (INTVAL (operands[2]),INTVAL (operands[3])) != 0)
    : -1 },
#line 1971 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD_F16INST",
    __builtin_constant_p 
#line 1971 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD_F16INST)
    ? (int) 
#line 1971 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD_F16INST)
    : -1 },
#line 4638 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "UINTVAL (operands[3]) < 32 &&\n\
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == 32)",
    __builtin_constant_p 
#line 4638 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[3]) < 32 &&
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == 32))
    ? (int) 
#line 4638 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(UINTVAL (operands[3]) < 32 &&
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == 32))
    : -1 },
#line 1702 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "aarch64_move_imm (INTVAL (operands[2]), SImode)",
    __builtin_constant_p 
#line 1702 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_move_imm (INTVAL (operands[2]), SImode))
    ? (int) 
#line 1702 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(aarch64_move_imm (INTVAL (operands[2]), SImode))
    : -1 },
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
  { "TARGET_SIMD\n\
   && rtx_equal_p (XEXP (operands[3], 0),\n\
		   plus_constant (Pmode,\n\
				  XEXP (operands[1], 0),\n\
				  GET_MODE_SIZE (V2SFmode)))",
    __builtin_constant_p 
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SFmode))))
    ? (int) 
#line 185 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-simd.md"
(TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SFmode))))
    : -1 },
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
  { "TARGET_SVE\n\
   && (register_operand (operands[0], VNx16BImode)\n\
       || register_operand (operands[1], VNx16BImode))",
    __builtin_constant_p 
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16BImode)
       || register_operand (operands[1], VNx16BImode)))
    ? (int) 
#line 444 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64-sve.md"
(TARGET_SVE
   && (register_operand (operands[0], VNx16BImode)
       || register_operand (operands[1], VNx16BImode)))
    : -1 },
#line 3268 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
  { "!aarch64_move_imm (INTVAL (operands[2]), DImode)\n\
   && !aarch64_plus_operand (operands[2], DImode)\n\
   && !reload_completed",
    __builtin_constant_p 
#line 3268 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[2]), DImode)
   && !aarch64_plus_operand (operands[2], DImode)
   && !reload_completed)
    ? (int) 
#line 3268 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md"
(!aarch64_move_imm (INTVAL (operands[2]), DImode)
   && !aarch64_plus_operand (operands[2], DImode)
   && !reload_completed)
    : -1 },

};
#endif /* gcc >= 3.0.1 */

int
main(void)
{
  unsigned int i;
  const char *p;
  puts ("(define_conditions [");
#if GCC_VERSION >= 3001
  for (i = 0; i < ARRAY_SIZE (insn_conditions); i++)
    {
      printf ("  (%d \"", insn_conditions[i].value);
      for (p = insn_conditions[i].expr; *p; p++)
        {
          switch (*p)
	     {
	     case '\\':
	     case '\"': putchar ('\\'); break;
	     default: break;
	     }
          putchar (*p);
        }
      puts ("\")");
    }
#endif /* gcc >= 3.0.1 */
  puts ("])");
  fflush (stdout);
return ferror (stdout) != 0 ? FATAL_EXIT_CODE : SUCCESS_EXIT_CODE;
}
