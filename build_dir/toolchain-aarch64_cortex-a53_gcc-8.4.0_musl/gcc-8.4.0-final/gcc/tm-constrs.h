/* Generated automatically by the program 'build/genpreds'
   from the machine description file '/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/aarch64.md'.  */

#ifndef GCC_TM_CONSTRS_H
#define GCC_TM_CONSTRS_H

static inline bool
satisfies_constraint_m (rtx op)
{
  return (GET_CODE (op) == MEM) && (
#line 26 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(memory_address_addr_space_p (GET_MODE (op), XEXP (op, 0),
						 MEM_ADDR_SPACE (op))));
}
static inline bool
satisfies_constraint_o (rtx op)
{
  return (GET_CODE (op) == MEM) && (
#line 32 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(offsettable_nonstrict_memref_p (op)));
}
static inline bool
satisfies_constraint_V (rtx op)
{
  return (GET_CODE (op) == MEM) && ((
#line 41 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(memory_address_addr_space_p (GET_MODE (op), XEXP (op, 0),
						 MEM_ADDR_SPACE (op)))) && (!(
#line 43 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(offsettable_nonstrict_memref_p (op)))));
}
static inline bool
satisfies_constraint__l (rtx op)
{
  return (GET_CODE (op) == MEM) && ((
#line 50 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(GET_CODE (XEXP (op, 0)) == PRE_DEC)) || (
#line 51 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(GET_CODE (XEXP (op, 0)) == POST_DEC)));
}
static inline bool
satisfies_constraint__g (rtx op)
{
  return (GET_CODE (op) == MEM) && ((
#line 57 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(GET_CODE (XEXP (op, 0)) == PRE_INC)) || (
#line 58 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(GET_CODE (XEXP (op, 0)) == POST_INC)));
}
static inline bool
satisfies_constraint_p (rtx ARG_UNUSED (op))
{
  return 
#line 62 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(address_operand (op, VOIDmode));
}
static inline bool
satisfies_constraint_i (rtx op)
{
  return (
#line 66 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(CONSTANT_P (op))) && (
#line 67 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(!flag_pic || LEGITIMATE_PIC_OPERAND_P (op)));
}
static inline bool
satisfies_constraint_s (rtx op)
{
  return (
#line 71 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(CONSTANT_P (op))) && ((
#line 72 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(!CONST_SCALAR_INT_P (op))) && (
#line 73 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(!flag_pic || LEGITIMATE_PIC_OPERAND_P (op))));
}
static inline bool
satisfies_constraint_n (rtx op)
{
  return (
#line 77 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(CONST_SCALAR_INT_P (op))) && (
#line 78 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(!flag_pic || LEGITIMATE_PIC_OPERAND_P (op)));
}
static inline bool
satisfies_constraint_E (rtx op)
{
  return (
#line 82 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(CONST_DOUBLE_AS_FLOAT_P (op))) || (
#line 83 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(GET_CODE (op) == CONST_VECTOR
		    && GET_MODE_CLASS (GET_MODE (op)) == MODE_VECTOR_FLOAT));
}
static inline bool
satisfies_constraint_F (rtx op)
{
  return (
#line 89 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(CONST_DOUBLE_AS_FLOAT_P (op))) || (
#line 90 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(GET_CODE (op) == CONST_VECTOR
		    && GET_MODE_CLASS (GET_MODE (op)) == MODE_VECTOR_FLOAT));
}
static inline bool
satisfies_constraint_X (rtx ARG_UNUSED (op))
{
  return 
#line 95 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/common.md"
(true);
}
static inline bool
satisfies_constraint_I (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 42 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_uimm12_shift (ival)));
}
static inline bool
satisfies_constraint_Uaa (rtx op)
{
  return (GET_CODE (op) == CONST_INT) && (
#line 47 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_pluslong_strict_immedate (op, VOIDmode)));
}
static inline bool
satisfies_constraint_Uav (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_addvl_addpl_immediate (op, mode);
}
static inline bool
satisfies_constraint_Uat (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_split_add_offset_immediate (op, mode);
}
static inline bool
satisfies_constraint_J (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 64 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_uimm12_shift (-ival)));
}
static inline bool
satisfies_constraint_K (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 73 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_bitmask_imm (ival, SImode)));
}
static inline bool
satisfies_constraint_L (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 78 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_bitmask_imm (ival, DImode)));
}
static inline bool
satisfies_constraint_M (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 83 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_move_imm (ival, SImode)));
}
static inline bool
satisfies_constraint_N (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 88 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_move_imm (ival, DImode)));
}
static inline bool
satisfies_constraint_Uti (rtx op)
{
  return ((GET_CODE (op) == CONST_INT) || (GET_CODE (op) == CONST_WIDE_INT)) && (
#line 94 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_mov128_immediate (op)));
}
static inline bool
satisfies_constraint_UsO (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 99 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_and_bitmask_imm (ival, SImode)));
}
static inline bool
satisfies_constraint_UsP (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 104 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_and_bitmask_imm (ival, DImode)));
}
static inline bool
satisfies_constraint_S (rtx op)
{
  switch (GET_CODE (op))
    {
    case CONST:
    case SYMBOL_REF:
    case LABEL_REF:
      break;
    default:
      return false;
    }
  return 
#line 109 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_symbolic_address_p (op));
}
static inline bool
satisfies_constraint_Y (rtx op)
{
  return (GET_CODE (op) == CONST_DOUBLE) && (
#line 114 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_float_const_zero_rtx_p (op)));
}
static inline bool
satisfies_constraint_Z (rtx op)
{
  return 
#line 118 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(op == const0_rtx);
}
static inline bool
satisfies_constraint_Ush (rtx op)
{
  return (GET_CODE (op) == HIGH) && (
#line 123 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_valid_symref (XEXP (op, 0), GET_MODE (XEXP (op, 0)))));
}
static inline bool
satisfies_constraint_Usa (rtx op)
{
  switch (GET_CODE (op))
    {
    case CONST:
    case SYMBOL_REF:
    case LABEL_REF:
      break;
    default:
      return false;
    }
  return (
#line 130 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_symbolic_address_p (op))) && (
#line 131 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_mov_operand_p (op, GET_MODE (op))));
}
static inline bool
satisfies_constraint_Uss (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 137 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
((unsigned HOST_WIDE_INT) ival < 32));
}
static inline bool
satisfies_constraint_Usn (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 142 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(IN_RANGE (ival, -31, 0)));
}
static inline bool
satisfies_constraint_Usd (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 148 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
((unsigned HOST_WIDE_INT) ival < 64));
}
static inline bool
satisfies_constraint_Usf (rtx op)
{
  return (GET_CODE (op) == SYMBOL_REF) && (
#line 153 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(!(aarch64_is_noplt_call_p (op)
		      || aarch64_is_long_call_p (op))));
}
static inline bool
satisfies_constraint_Usg (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 161 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(IN_RANGE (ival, 1, 31)));
}
static inline bool
satisfies_constraint_Usj (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 168 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(IN_RANGE (ival, 1, 63)));
}
static inline bool
satisfies_constraint_UsM (rtx op)
{
  return 
#line 173 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(op == constm1_rtx);
}
static inline bool
satisfies_constraint_Usv (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_cnt_immediate (op, mode);
}
static inline bool
satisfies_constraint_Usi (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_index_immediate (op, mode);
}
static inline bool
satisfies_constraint_Ui1 (rtx op)
{
  return 
#line 190 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(op == const1_rtx);
}
static inline bool
satisfies_constraint_Ui2 (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 196 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
((unsigned HOST_WIDE_INT) ival <= 3));
}
static inline bool
satisfies_constraint_Ui3 (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 202 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
((unsigned HOST_WIDE_INT) ival <= 4));
}
static inline bool
satisfies_constraint_Ui7 (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 208 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
((unsigned HOST_WIDE_INT) ival <= 7));
}
static inline bool
satisfies_constraint_Up3 (rtx op)
{
  HOST_WIDE_INT ival = 0;
  if (CONST_INT_P (op))
    ival = INTVAL (op);
  return (GET_CODE (op) == CONST_INT) && (
#line 214 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
((unsigned) exact_log2 (ival) <= 4));
}
static inline bool
satisfies_constraint_Q (rtx op)
{
  return (GET_CODE (op) == MEM) && (
#line 219 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(REG_P (XEXP (op, 0))));
}
static inline bool
satisfies_constraint_Umq (rtx op)
{
  return (GET_CODE (op) == MEM) && (
#line 226 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_legitimate_address_p (DImode, XEXP (op, 0), false,
						   ADDR_QUERY_LDP_STP)));
}
static inline bool
satisfies_constraint_Ump (rtx op)
{
  return (GET_CODE (op) == MEM) && (
#line 233 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_legitimate_address_p (GET_MODE (op), XEXP (op, 0),
						  true, ADDR_QUERY_LDP_STP)));
}
static inline bool
satisfies_constraint_Uml (rtx op)
{
  return (GET_CODE (op) == MEM) && (
#line 242 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_legitimate_address_p (DFmode, XEXP (op, 0), 1,
						  ADDR_QUERY_LDP_STP)));
}
static inline bool
satisfies_constraint_Utr (rtx op)
{
  return (GET_CODE (op) == MEM) && (
#line 250 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_sve_ldr_operand_p (op)));
}
static inline bool
satisfies_constraint_Utv (rtx op)
{
  return (GET_CODE (op) == MEM) && (
#line 257 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_simd_mem_operand_p (op)));
}
static inline bool
satisfies_constraint_Utq (rtx op)
{
  return (GET_CODE (op) == MEM) && (
#line 263 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_legitimate_address_p (V2DImode,
						  XEXP (op, 0), 1)));
}
static inline bool
satisfies_constraint_Uty (rtx op)
{
  return (GET_CODE (op) == MEM) && (
#line 270 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_sve_ld1r_operand_p (op)));
}
static inline bool
satisfies_constraint_Utx (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_struct_memory_operand (op, mode);
}
static inline bool
satisfies_constraint_Ufc (rtx op)
{
  return (GET_CODE (op) == CONST_DOUBLE) && (
#line 282 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_float_const_representable_p (op)));
}
static inline bool
satisfies_constraint_Uvi (rtx op)
{
  return (GET_CODE (op) == CONST_DOUBLE) && (
#line 288 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_can_const_movi_rtx_p (op, GET_MODE (op))));
}
static inline bool
satisfies_constraint_Do (rtx op)
{
  return (GET_CODE (op) == CONST_VECTOR) && (
#line 294 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_simd_valid_immediate (op, NULL,
						 AARCH64_CHECK_ORR)));
}
static inline bool
satisfies_constraint_Db (rtx op)
{
  return (GET_CODE (op) == CONST_VECTOR) && (
#line 301 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_simd_valid_immediate (op, NULL,
						 AARCH64_CHECK_BIC)));
}
static inline bool
satisfies_constraint_Dn (rtx op)
{
  switch (GET_CODE (op))
    {
    case CONST:
    case CONST_VECTOR:
      break;
    default:
      return false;
    }
  return 
#line 308 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_simd_valid_immediate (op, NULL));
}
static inline bool
satisfies_constraint_Dh (rtx op)
{
  return (GET_CODE (op) == CONST_INT) && (
#line 315 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_simd_scalar_immediate_valid_for_move (op,
						 HImode)));
}
static inline bool
satisfies_constraint_Dq (rtx op)
{
  return (GET_CODE (op) == CONST_INT) && (
#line 323 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_simd_scalar_immediate_valid_for_move (op,
						 QImode)));
}
static inline bool
satisfies_constraint_Dl (rtx op)
{
  switch (GET_CODE (op))
    {
    case CONST:
    case CONST_VECTOR:
      break;
    default:
      return false;
    }
  return 
#line 330 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_simd_shift_imm_p (op, GET_MODE (op),
						 true));
}
static inline bool
satisfies_constraint_Dr (rtx op)
{
  switch (GET_CODE (op))
    {
    case CONST:
    case CONST_VECTOR:
      break;
    default:
      return false;
    }
  return 
#line 337 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_simd_shift_imm_p (op, GET_MODE (op),
						 false));
}
static inline bool
satisfies_constraint_Dz (rtx op)
{
  switch (GET_CODE (op))
    {
    case CONST:
    case CONST_VECTOR:
      break;
    default:
      return false;
    }
  return 
#line 343 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(op == CONST0_RTX (GET_MODE (op)));
}
static inline bool
satisfies_constraint_Dm (rtx op)
{
  switch (GET_CODE (op))
    {
    case CONST:
    case CONST_VECTOR:
      break;
    default:
      return false;
    }
  return 
#line 349 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(op == CONST1_RTX (GET_MODE (op)));
}
static inline bool
satisfies_constraint_Dd (rtx op)
{
  return (GET_CODE (op) == CONST_INT) && (
#line 356 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_can_const_movi_rtx_p (op, DImode)));
}
static inline bool
satisfies_constraint_Ds (rtx op)
{
  return (GET_CODE (op) == CONST_INT) && (
#line 363 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_can_const_movi_rtx_p (op, SImode)));
}
static inline bool
satisfies_constraint_Dp (rtx op)
{
  return 
#line 368 "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/gcc-8.4.0/gcc/config/aarch64/constraints.md"
(aarch64_address_valid_for_prefetch_p (op, true));
}
static inline bool
satisfies_constraint_vsa (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_arith_immediate (op, mode);
}
static inline bool
satisfies_constraint_vsc (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_cmp_vsc_immediate (op, mode);
}
static inline bool
satisfies_constraint_vsd (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_cmp_vsd_immediate (op, mode);
}
static inline bool
satisfies_constraint_vsi (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_inc_dec_immediate (op, mode);
}
static inline bool
satisfies_constraint_vsn (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_sub_arith_immediate (op, mode);
}
static inline bool
satisfies_constraint_vsl (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_logical_immediate (op, mode);
}
static inline bool
satisfies_constraint_vsm (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_mul_immediate (op, mode);
}
static inline bool
satisfies_constraint_vsA (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_float_arith_immediate (op, mode);
}
static inline bool
satisfies_constraint_vsM (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_float_mul_immediate (op, mode);
}
static inline bool
satisfies_constraint_vsN (rtx op)
{
  machine_mode mode = GET_MODE (op);
  return aarch64_sve_float_arith_with_sub_immediate (op, mode);
}
#endif /* tm-constrs.h */
