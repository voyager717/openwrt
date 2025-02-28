(define_conditions [
  (-1 "TARGET_FLOAT
   && IN_RANGE (aarch64_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (DImode))")
  (-1 "aarch64_operands_adjust_ok_for_ldpstp (operands, false, SImode)")
  (-1 "aarch64_zero_extend_const_eq (DImode, operands[3],
                                 SImode, operands[2])")
  (-1 "!reg_overlap_mentioned_p (operands[0], operands[1])
   && INTVAL (operands[3]) == -INTVAL (operands[2])")
  (-1 "((!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && (ptr_mode == SImode || Pmode == SImode)) && ( true)")
  (-1 "(INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)) && ( true)")
  (-1 "!HONOR_SIGNED_ZEROS (DFmode) && TARGET_FLOAT")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 16, 63)")
  (-1 "TARGET_SIMD
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (GET_MODE_INNER (V2DFmode)))")
  (-1 "UINTVAL (operands[3]) < GET_MODE_BITSIZE (SImode)
   && (UINTVAL (operands[3]) + UINTVAL (operands[4])
       == GET_MODE_BITSIZE (SImode))")
  (-1 "reload_completed && FP_REGNUM_P (REGNO (operands[0]))")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx16SFmode)
       || register_operand (operands[2], VNx16SFmode))) && ( reload_completed)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx16SImode)
       || register_operand (operands[2], VNx16SImode))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx2DImode)
       || register_operand (operands[2], VNx2DImode))")
  (-1 "aarch64_mask_and_shift_for_ubfiz_p (DImode, operands[3], operands[2])
  && (INTVAL (operands[2]) + popcount_hwi (INTVAL (operands[3])))
      == GET_MODE_BITSIZE (SImode)")
  (-1 "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DFmode)) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "(INTVAL (operands[5]) == GET_MODE_SIZE (DFmode)) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx4DImode)
       || register_operand (operands[2], VNx4DImode))")
  (-1 "TARGET_SVE
   && (register_operand (operands[2], VNx4SFmode)
       || register_operand (operands[3], VNx4SFmode))")
  (-1 "can_create_pseudo_p ()
   && !aarch64_can_const_movi_rtx_p (operands[1], DFmode)
   && !aarch64_float_const_representable_p (operands[1])
   &&  aarch64_float_const_rtx_p (operands[1])")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx16HImode)
       || register_operand (operands[2], VNx16HImode))")
  (-1 "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (SImode) >= 64")
  (-1 "(INTVAL (operands[5]) == GET_MODE_SIZE (SFmode)) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "aarch64_is_extend_from_extract (SImode, operands[2], operands[3])")
  (-1 "(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)) && ( true)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx8DFmode)
       || register_operand (operands[2], VNx8DFmode))")
  (-1 "TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V4HImode)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 16, 63)")
  (-1 "(((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)) && ( true)")
  (-1 "IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]), 1,
	     GET_MODE_BITSIZE (DImode) - 1)
   && (INTVAL (operands[2]) + INTVAL (operands[3]))
       == GET_MODE_BITSIZE (SImode)")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V2SImode)
       || aarch64_simd_reg_or_zero (operands[1], V2SImode))")
  (-1 "TARGET_SVE && BYTES_BIG_ENDIAN")
  (-1 "(TARGET_SIMD) && (TARGET_SIMD_F16INST)")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx24HFmode)
       || register_operand (operands[2], VNx24HFmode))) && ( reload_completed)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx8SFmode)
       || register_operand (operands[2], VNx8SFmode))")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx48QImode)
       || register_operand (operands[2], VNx48QImode))) && ( reload_completed)")
  (-1 "(register_operand (operands[0], QImode)
    || aarch64_reg_or_zero (operands[1], QImode))")
  (-1 "UINTVAL (operands[2]) < 64")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx12SFmode)
       || register_operand (operands[2], VNx12SFmode))) && ( reload_completed)")
  (-1 "ptr_mode == DImode")
  (-1 "TARGET_SIMD_RDMA")
  (-1 "((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)")
  (-1 "(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)) && ( true)")
  (-1 "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (DImode) >= 64")
  (-1 "TARGET_FLOAT && (register_operand (operands[0], DFmode)
    || aarch64_reg_or_fp_zero (operands[1], DFmode))")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx8DFmode)
       || register_operand (operands[2], VNx8DFmode))) && ( reload_completed)")
  (-1 "TARGET_SIMD
   && ENDIAN_LANE_N (8, INTVAL (operands[2])) == 0")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx8SFmode)
       || register_operand (operands[2], VNx8SFmode))) && ( reload_completed)")
  (-1 "((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)")
  (-1 "TARGET_FLOAT && (reload_completed || reload_in_progress)")
  (-1 "UINTVAL (operands[3]) < GET_MODE_BITSIZE (DImode) &&
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == GET_MODE_BITSIZE (DImode))")
  (-1 "TARGET_SVE && !BYTES_BIG_ENDIAN")
  (-1 "(TARGET_TLS_DESC && TARGET_SVE) && (ptr_mode == SImode)")
  (-1 "TARGET_SVE && aarch64_check_zero_based_sve_index_immediate (operands[2])")
  (-1 "TARGET_SIMD && reload_completed
   && GP_REGNUM_P (REGNO (operands[0]))
   && GP_REGNUM_P (REGNO (operands[1]))")
  (-1 "UINTVAL (operands[2]) < GET_MODE_BITSIZE (HImode)")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V8QImode)
       || aarch64_simd_reg_or_zero (operands[1], V8QImode))")
  (-1 "(CONST_INT_P (operands[1]) && !aarch64_move_imm (INTVAL (operands[1]), DImode))
    && REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0]))")
  (-1 "TARGET_SIMD && TARGET_SHA2 && !BYTES_BIG_ENDIAN")
  (-1 "(TARGET_SIMD) && ( REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0])))")
  (-1 "TARGET_SVE && GET_MODE_NUNITS (VNx8HFmode).is_constant ()")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx16SFmode)
       || register_operand (operands[2], VNx16SFmode))")
  (-1 "TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SImode)))")
  (-1 "rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (DFmode)))")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V8HImode)
       || aarch64_simd_reg_or_zero (operands[1], V8HImode))")
  (-1 "(register_operand (operands[0], TImode)
    || aarch64_reg_or_zero (operands[1], TImode))")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx24HImode)
       || register_operand (operands[2], VNx24HImode))) && ( reload_completed)")
  (-1 "(TARGET_SVE && operands[0] != stack_pointer_rtx) && ( epilogue_completed
   && !reg_overlap_mentioned_p (operands[0], operands[1])
   && aarch64_split_add_offset_immediate (operands[2], DImode))")
  (-1 "UINTVAL (operands[1]) < GET_MODE_BITSIZE (SImode)
   && UINTVAL (operands[1]) % 16 == 0")
  (-1 "rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (DImode)))")
  (-1 "TARGET_SIMD && !BYTES_BIG_ENDIAN
   && (register_operand (operands[0], OImode)
       || register_operand (operands[1], OImode))")
  (-1 "ptr_mode == SImode")
  (-1 "TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V2SFmode)")
  (-1 "UINTVAL (operands[1]) <= 32")
  (-1 "TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V2SFmode)))")
  (-1 "INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)")
  (-1 "rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (SImode)))")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx32HFmode)
       || register_operand (operands[2], VNx32HFmode))) && ( reload_completed)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx6DFmode)
       || register_operand (operands[2], VNx6DFmode))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx12SImode)
       || register_operand (operands[2], VNx12SImode))")
  (-1 "(TARGET_SVE && operands[0] != stack_pointer_rtx) && ( epilogue_completed
   && !reg_overlap_mentioned_p (operands[0], operands[1])
   && aarch64_split_add_offset_immediate (operands[2], SImode))")
  (-1 "(register_operand (operands[0], HImode)
    || aarch64_reg_or_zero (operands[1], HImode))")
  (-1 "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SFmode)) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "UINTVAL (operands[1]) <= 16")
  (-1 "TARGET_FLOAT
   && ((GET_MODE_BITSIZE (DFmode) <= LONG_TYPE_SIZE)
   || !flag_trapping_math || flag_fp_int_builtin_inexact)")
  (-1 "TARGET_FLOAT
   && IN_RANGE (aarch64_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (SImode))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx8HImode)
       || register_operand (operands[2], VNx8HImode))")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, true, V4HImode)")
  (-1 "can_create_pseudo_p ()
   && !aarch64_can_const_movi_rtx_p (operands[1], HFmode)
   && !aarch64_float_const_representable_p (operands[1])
   &&  aarch64_float_const_rtx_p (operands[1])")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx24HImode)
       || register_operand (operands[2], VNx24HImode))")
  (-1 "aarch64_zero_extend_const_eq (DImode, operands[2],
				 SImode, operands[1])")
  (-1 "aarch64_move_imm (INTVAL (operands[2]), DImode)")
  (-1 "aarch64_mask_and_shift_for_ubfiz_p (SImode, operands[3], operands[2])")
  (-1 "TARGET_ILP32")
  (-1 "(TARGET_TLS_DESC && !TARGET_SVE) && (ptr_mode == SImode)")
  (-1 "(TARGET_FLOAT) && (AARCH64_ISA_F16)")
  (-1 "(TARGET_SIMD) && ( reload_completed)")
  (-1 "(TARGET_TLS_DESC && !TARGET_SVE) && (ptr_mode == DImode)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx4DFmode)
       || register_operand (operands[2], VNx4DFmode))")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 0, 63)")
  (-1 "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (SFmode) >= 64")
  (-1 "!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[2]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (SImode)))")
  (-1 "ptr_mode == SImode || Pmode == SImode")
  (-1 "(TARGET_TLS_DESC && TARGET_SVE) && (ptr_mode == DImode)")
  (-1 "rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (SFmode)))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx32HImode)
       || register_operand (operands[2], VNx32HImode))")
  (-1 "(~INTVAL (operands[3]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0")
  (-1 "TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V4HFmode)")
  (-1 "INTVAL (operands[1]) > 0
   && ((INTVAL (operands[1]) + INTVAL (operands[2]))
	<= GET_MODE_BITSIZE (SImode))
   && aarch64_bitmask_imm (
	UINTVAL (aarch64_mask_from_zextract_ops (operands[1],
						 operands[2])),
	SImode)")
  (-1 "TARGET_FLOAT
   && ((GET_MODE_BITSIZE (SFmode) <= LONG_TYPE_SIZE)
   || !flag_trapping_math || flag_fp_int_builtin_inexact)")
  (-1 "reload_completed && aarch64_split_128bit_move_p (operands[0], operands[1])")
  (-1 "aarch64_is_extend_from_extract (DImode, operands[2], operands[3])")
  (-1 "TARGET_SVE
   && (register_operand (operands[2], VNx2DFmode)
       || register_operand (operands[3], VNx2DFmode))")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V2DFmode)
       || aarch64_simd_reg_or_zero (operands[1], V2DFmode))")
  (-1 "ptr_mode == DImode || Pmode == DImode")
  (-1 "(TARGET_SVE) && ( reload_completed
   && REG_P (operands[0])
   && REGNO (operands[0]) == REGNO (operands[1]))")
  (-1 "TARGET_SIMD && TARGET_AES")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx16SImode)
       || register_operand (operands[2], VNx16SImode))) && ( reload_completed)")
  (-1 "(TARGET_SVE) && ( MEM_P (operands[1]))")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SImode), 1, 15)")
  (-1 "TARGET_SVE
   && (register_operand (operands[2], VNx8HFmode)
       || register_operand (operands[3], VNx8HFmode))")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx8SImode)
       || register_operand (operands[2], VNx8SImode))) && ( reload_completed)")
  (-1 "TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HFmode)))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx32HFmode)
       || register_operand (operands[2], VNx32HFmode))")
  (-1 "TARGET_SIMD && BYTES_BIG_ENDIAN
   && (register_operand (operands[0], OImode)
       || register_operand (operands[1], OImode))")
  (-1 "UINTVAL (operands[2]) < GET_MODE_BITSIZE (QImode)")
  (-1 "!reg_overlap_mentioned_p (operands[0], operands[1])
   && !reg_overlap_mentioned_p (operands[0], operands[2])")
  (-1 "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DFmode)) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "((!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && (ptr_mode == DImode || Pmode == DImode)) && ( true)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx2DFmode)
       || register_operand (operands[2], VNx2DFmode))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx16HFmode)
       || register_operand (operands[2], VNx16HFmode))")
  (-1 "TARGET_SIMD && TARGET_SHA2 && BYTES_BIG_ENDIAN")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V4SFmode)
       || aarch64_simd_reg_or_zero (operands[1], V4SFmode))")
  (-1 "(TARGET_SVE) && ( !CONSTANT_P (operands[1]))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx4BImode)
       || register_operand (operands[1], VNx4BImode))")
  (-1 "TARGET_SVE && GET_MODE_NUNITS (VNx8HImode).is_constant ()")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 0, 63)")
  (-1 "(INTVAL (operands[5]) == GET_MODE_SIZE (SFmode)) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 1, 15)")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx4DImode)
       || register_operand (operands[2], VNx4DImode))) && ( reload_completed)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 0, 63)")
  (-1 "TARGET_FLOAT")
  (-1 "TARGET_SIMD && BYTES_BIG_ENDIAN
   && (register_operand (operands[0], XImode)
       || register_operand (operands[1], XImode))")
  (-1 "(TARGET_SIMD) && (AARCH64_ISA_F16)")
  (-1 "(register_operand (operands[0], DImode)
    || aarch64_reg_or_zero (operands[1], DImode))")
  (-1 "aarch64_operands_adjust_ok_for_ldpstp (operands, false, DImode)")
  (-1 "TARGET_SIMD && !BYTES_BIG_ENDIAN")
  (-1 "UINTVAL (operands[1]) <= 8")
  (-1 "TARGET_FLOAT && (TARGET_FP_F16INST || TARGET_SIMD)")
  (-1 "TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (DImode)))")
  (-1 "aarch64_operands_adjust_ok_for_ldpstp (operands, false, SFmode)")
  (-1 "TARGET_CRC32")
  (-1 "TARGET_SVE")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx8BImode)
       || register_operand (operands[1], VNx8BImode))")
  (-1 "IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]),
	     1, GET_MODE_BITSIZE (SImode) - 1)")
  (-1 "TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HImode)))")
  (-1 "(register_operand (operands[0], SImode)
    || aarch64_reg_or_zero (operands[1], SImode))")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, true, SImode)")
  (-1 "TARGET_SIMD
   && ENDIAN_LANE_N (4, INTVAL (operands[2])) == 0")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, true, V2SImode)")
  (-1 "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SImode)) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx48QImode)
       || register_operand (operands[2], VNx48QImode))")
  (-1 "TARGET_SVE && GET_MODE_NUNITS (VNx2DImode).is_constant ()")
  (-1 "TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SFmode)))")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 16, 63)")
  (-1 "!aarch64_move_imm (INTVAL (operands[2]), SImode)
   && !aarch64_plus_operand (operands[2], SImode)
   && !reload_completed")
  (-1 "TARGET_SIMD")
  (-1 "aarch64_zero_extend_const_eq (TImode, operands[2],
				 DImode, operands[1])")
  (-1 "TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx2DFmode)
	   && nonmemory_operand (operands[1], VNx2DFmode)))")
  (-1 "TARGET_FP_F16INST")
  (-1 "TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V2SImode)))")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 0, 63)")
  (-1 "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (HImode) >= 64")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V4SImode)
       || aarch64_simd_reg_or_zero (operands[1], V4SImode))")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, false, SFmode)")
  (-1 "reload_completed")
  (-1 "UINTVAL (operands[2]) < GET_MODE_BITSIZE (DImode)")
  (-1 "TARGET_F16FML")
  (-1 "TARGET_SIMD && reload_completed")
  (-1 "aarch_rev16_shleft_mask_imm_p (operands[3], DImode)
   && aarch_rev16_shright_mask_imm_p (operands[2], DImode)")
  (-1 "epilogue_completed")
  (-1 "((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (DImode)")
  (-1 "TARGET_SVE && reload_completed")
  (-1 "IN_RANGE (INTVAL (operands[2]) + INTVAL (operands[3]),
	     1, GET_MODE_BITSIZE (DImode) - 1)")
  (-1 "INTVAL (operands[1]) > 0
   && ((INTVAL (operands[1]) + INTVAL (operands[2]))
	<= GET_MODE_BITSIZE (DImode))
   && aarch64_bitmask_imm (
	UINTVAL (aarch64_mask_from_zextract_ops (operands[1],
						 operands[2])),
	DImode)")
  (-1 "(TARGET_SVE && BYTES_BIG_ENDIAN) && ( reload_completed)")
  (-1 "TARGET_DOTPROD")
  (-1 "TARGET_SVE && operands[0] != stack_pointer_rtx")
  (-1 "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (HFmode) >= 64")
  (-1 "TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (DFmode)))")
  (-1 "TARGET_FLOAT && (register_operand (operands[0], TFmode)
    || aarch64_reg_or_fp_zero (operands[1], TFmode))")
  (-1 "(TARGET_LSE) && ( reload_completed)")
  (-1 "rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (SFmode)))")
  (-1 "aarch64_operands_adjust_ok_for_ldpstp (operands, false, DFmode)")
  (-1 "TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx4SFmode)
	   && nonmemory_operand (operands[1], VNx4SFmode)))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx8SImode)
       || register_operand (operands[2], VNx8SImode))")
  (-1 "TARGET_SIMD && TARGET_SM4")
  (-1 "(TARGET_FLOAT) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V2SFmode)
       || aarch64_simd_reg_or_zero (operands[1], V2SFmode))")
  (-1 "TARGET_SIMD && TARGET_SHA2")
  (-1 "(INTVAL (operands[5]) == GET_MODE_SIZE (DImode)) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "TARGET_SIMD
   && ENDIAN_LANE_N (16, INTVAL (operands[2])) == 0")
  (-1 "UINTVAL (operands[3]) < GET_MODE_BITSIZE (SImode) &&
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == GET_MODE_BITSIZE (SImode))")
  (-1 "aarch64_mask_and_shift_for_ubfiz_p (DImode, operands[3], operands[2])")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (HImode), 0, 255)")
  (-1 "TARGET_SIMD
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (GET_MODE_INNER (V2SFmode)))")
  (-1 "TARGET_FLOAT && TARGET_SIMD")
  (-1 "TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V4HFmode)))")
  (-1 "SIBLING_CALL_P (insn)")
  (-1 "rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (DImode)))")
  (-1 "(TARGET_FLOAT) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (SFmode), 0, 255)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 0, 63)")
  (-1 "INTVAL (operands[3]) == -INTVAL (operands[2])")
  (-1 "(!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V8QImode)")
  (-1 "aarch64_operands_adjust_ok_for_ldpstp (operands, true, SFmode)")
  (-1 "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DImode)) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "TARGET_SIMD && aarch64_operands_ok_for_ldpstp (operands, false, V2SImode)")
  (-1 "TARGET_SVE && GET_MODE_NUNITS (VNx16QImode).is_constant ()")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 16, 63)")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx16HFmode)
       || register_operand (operands[2], VNx16HFmode))) && ( reload_completed)")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, true, DImode)")
  (-1 "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SFmode)) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HImode)))")
  (-1 "(INTVAL (operands[5]) == GET_MODE_SIZE (DFmode)) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "aarch64_uxt_size (exact_log2 (INTVAL (operands[2])),INTVAL (operands[3])) != 0")
  (-1 "(INTVAL (operands[5]) == GET_MODE_SIZE (DImode)) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx16HImode)
       || register_operand (operands[2], VNx16HImode))) && ( reload_completed)")
  (-1 "aarch64_operands_adjust_ok_for_ldpstp (operands, true, DFmode)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx32QImode)
       || register_operand (operands[2], VNx32QImode))")
  (-1 "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (QImode) >= 64")
  (-1 " epilogue_completed")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V4HFmode)
       || aarch64_simd_reg_or_zero (operands[1], V4HFmode))")
  (-1 "!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[3]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (SImode)))")
  (-1 "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (DImode)) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "TARGET_SIMD && !BYTES_BIG_ENDIAN
   && (register_operand (operands[0], CImode)
       || register_operand (operands[1], CImode))")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 0, 63)")
  (-1 "(INTVAL (operands[5]) == GET_MODE_SIZE (SImode)) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx32HImode)
       || register_operand (operands[2], VNx32HImode))) && ( reload_completed)")
  (-1 "aarch64_zero_extend_const_eq (TImode, operands[3],
                                 DImode, operands[2])")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (SFmode), 16, 63)")
  (-1 "aarch64_use_return_insn_p ()")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx32QImode)
       || register_operand (operands[2], VNx32QImode))) && ( reload_completed)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 0, 63)")
  (-1 "TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx2DImode)
	   && nonmemory_operand (operands[1], VNx2DImode)))")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx12SImode)
       || register_operand (operands[2], VNx12SImode))) && ( reload_completed)")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, false, DImode)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx2BImode)
       || register_operand (operands[1], VNx2BImode))")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V4HImode)
       || aarch64_simd_reg_or_zero (operands[1], V4HImode))")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, true, SFmode)")
  (-1 "(INTVAL (operands[5]) == INTVAL (operands[4]) + GET_MODE_SIZE (SImode)) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "TARGET_FLOAT && (register_operand (operands[0], SFmode)
    || aarch64_reg_or_fp_zero (operands[1], SFmode))")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V2DImode)
       || aarch64_simd_reg_or_zero (operands[1], V2DImode))")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 16, 63)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (HFmode), 0, 255)")
  (-1 "aarch64_operands_adjust_ok_for_ldpstp (operands, true, SImode)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (SImode), 0, 255)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HFmode), 1, 15)")
  (-1 "(!aarch64_move_imm (INTVAL (operands[1]), SImode)
   && !aarch64_plus_operand (operands[1], SImode)
   && !reload_completed) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "rtx_equal_p (XEXP (operands[3], 0),
		plus_constant (Pmode,
			       XEXP (operands[1], 0),
			       GET_MODE_SIZE (DFmode)))")
  (-1 "TARGET_SIMD && TARGET_SHA3")
  (-1 "((~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0)")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx8DImode)
       || register_operand (operands[2], VNx8DImode))) && ( reload_completed)")
  (-1 "TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V4HImode)))")
  (-1 "TARGET_SIMD && BYTES_BIG_ENDIAN")
  (-1 "(INTVAL (operands[5]) == GET_MODE_SIZE (SImode)) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V16QImode)
       || aarch64_simd_reg_or_zero (operands[1], V16QImode))")
  (-1 "(!aarch64_move_imm (INTVAL (operands[2]), DImode)
   && !aarch64_plus_operand (operands[2], DImode)
   && !reload_completed) && ( true)")
  (-1 "TARGET_SIMD
   && ENDIAN_LANE_N (2, INTVAL (operands[2])) == 0")
  (-1 "(TARGET_TLS_DESC) && (ptr_mode == DImode)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx12SFmode)
       || register_operand (operands[2], VNx12SFmode))")
  (-1 "!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[3]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (DImode)))")
  (-1 "TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx8HImode)
	   && nonmemory_operand (operands[1], VNx8HImode)))")
  (-1 "TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx8HFmode)
	   && nonmemory_operand (operands[1], VNx8HFmode)))")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, true, V4HFmode)")
  (-1 "(TARGET_FLOAT && TARGET_SIMD) && (TARGET_SIMD_F16INST)")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, false, DFmode)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (QImode), 0, 255)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (DImode), 0, 255)")
  (-1 "(~INTVAL (operands[3]) & (GET_MODE_BITSIZE (DImode) - 1)) == 0")
  (-1 "!((operands[3] == const1_rtx && operands[4] == constm1_rtx)
     || (operands[3] == constm1_rtx && operands[4] == const1_rtx))")
  (-1 "TARGET_SIMD && reload_completed
   && ((FP_REGNUM_P (REGNO (operands[0])) && GP_REGNUM_P (REGNO (operands[1])))
       || (GP_REGNUM_P (REGNO (operands[0])) && FP_REGNUM_P (REGNO (operands[1]))))")
  (-1 "!(UINTVAL (operands[1]) == 0
     || (UINTVAL (operands[2]) + UINTVAL (operands[1])
	 > GET_MODE_BITSIZE (DImode)))")
  (-1 "TARGET_SVE && GET_MODE_NUNITS (VNx2DFmode).is_constant ()")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx24HFmode)
       || register_operand (operands[2], VNx24HFmode))")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 16, 63)")
  (-1 "(TARGET_TLS_DESC) && (ptr_mode == SImode)")
  (-1 "aarch_rev16_shleft_mask_imm_p (operands[3], SImode)
   && aarch_rev16_shright_mask_imm_p (operands[2], SImode)")
  (-1 "optimize > 0 && flag_modulo_sched")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx4SFmode)
       || register_operand (operands[2], VNx4SFmode))")
  (-1 "INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)")
  (-1 "(!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && (ptr_mode == SImode || Pmode == SImode)")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx6DImode)
       || register_operand (operands[2], VNx6DImode))) && ( reload_completed)")
  (-1 "TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SImode)))")
  (-1 "TARGET_SVE && GET_MODE_NUNITS (VNx4SFmode).is_constant ()")
  (-1 "CONST_INT_P (operands[1]) && !aarch64_move_imm (INTVAL (operands[1]), SImode)
    && REG_P (operands[0]) && GP_REGNUM_P (REGNO (operands[0]))")
  (-1 "((!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && (ptr_mode == DImode || Pmode == DImode)) && ( true)")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, true, DFmode)")
  (1 "true")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx8DImode)
       || register_operand (operands[2], VNx8DImode))")
  (-1 "TARGET_SIMD
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[0], 0),
				  GET_MODE_SIZE (V8QImode)))")
  (-1 "TARGET_SIMD && !BYTES_BIG_ENDIAN
   && (register_operand (operands[0], XImode)
       || register_operand (operands[1], XImode))")
  (-1 "TARGET_LSE")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, true, V8QImode)")
  (-1 "TARGET_LSE && reload_completed")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx64QImode)
       || register_operand (operands[2], VNx64QImode))) && ( reload_completed)")
  (-1 "TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V8QImode)))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx4SImode)
       || register_operand (operands[2], VNx4SImode))")
  (-1 "TARGET_SIMD && BYTES_BIG_ENDIAN
   && (register_operand (operands[0], CImode)
       || register_operand (operands[1], CImode))")
  (-1 "TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx4SImode)
	   && nonmemory_operand (operands[1], VNx4SImode)))")
  (-1 "!STRICT_ALIGNMENT")
  (-1 "aarch64_uxt_size (exact_log2 (INTVAL (operands[2])), INTVAL (operands[3])) != 0")
  (-1 "TARGET_SVE && INTVAL (operands[2]) * GET_MODE_SIZE (DFmode) >= 64")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx6DFmode)
       || register_operand (operands[2], VNx6DFmode))) && ( reload_completed)")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, false, SImode)")
  (-1 "(!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && (ptr_mode == DImode || Pmode == DImode)")
  (-1 "can_create_pseudo_p ()
   && !aarch64_can_const_movi_rtx_p (operands[1], SFmode)
   && !aarch64_float_const_representable_p (operands[1])
   &&  aarch64_float_const_rtx_p (operands[1])")
  (-1 "aarch64_operands_ok_for_ldpstp (operands, true, V2SFmode)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx16QImode)
       || register_operand (operands[2], VNx16QImode))")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[3]) * GET_MODE_SIZE (DFmode), 0, 255)")
  (-1 "(!aarch64_move_imm (INTVAL (operands[2]), SImode)
   && !aarch64_plus_operand (operands[2], SImode)
   && !reload_completed) && ( true)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DFmode), 1, 15)")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx64QImode)
       || register_operand (operands[2], VNx64QImode))")
  (-1 "UINTVAL (operands[3]) < GET_MODE_BITSIZE (DImode)
   && (UINTVAL (operands[3]) + UINTVAL (operands[4])
       == GET_MODE_BITSIZE (DImode))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx8HFmode)
       || register_operand (operands[2], VNx8HFmode))")
  (-1 "UINTVAL (operands[1]) < GET_MODE_BITSIZE (DImode)
   && UINTVAL (operands[1]) % 16 == 0")
  (-1 "UINTVAL (operands[2]) < GET_MODE_BITSIZE (SImode)")
  (-1 "UINTVAL (operands[2]) < 32")
  (-1 "TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V4HFmode)))")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (DImode), 1, 15)")
  (-1 "TARGET_SVE && GET_MODE_NUNITS (VNx4SImode).is_constant ()")
  (-1 "aarch64_operands_adjust_ok_for_ldpstp (operands, true, DImode)")
  (-1 "aarch64_uxt_size (INTVAL (operands[2]), INTVAL (operands[3])) != 0")
  (-1 "TARGET_SVE
   && !BYTES_BIG_ENDIAN
   && ((lra_in_progress || reload_completed)
       || (register_operand (operands[0], VNx16QImode)
	   && nonmemory_operand (operands[1], VNx16QImode)))")
  (-1 "(((~INTVAL (operands[4]) & (GET_MODE_BITSIZE (SImode) - 1)) == 0)
   && INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)) && ( true)")
  (-1 "TARGET_FLOAT && (register_operand (operands[0], HFmode)
    || aarch64_reg_or_fp_zero (operands[1], HFmode))")
  (-1 "rtx_equal_p (XEXP (operands[2], 0),
		plus_constant (Pmode,
			       XEXP (operands[0], 0),
			       GET_MODE_SIZE (SImode)))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx6DImode)
       || register_operand (operands[2], VNx6DImode))")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (QImode), 1, 15)")
  (-1 "TARGET_SIMD && !STRICT_ALIGNMENT
   && rtx_equal_p (XEXP (operands[2], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V8QImode)))")
  (-1 "TARGET_FLOAT && !flag_rounding_math")
  (-1 "((!aarch64_move_imm (INTVAL (operands[1]), DImode)
   && !aarch64_plus_operand (operands[1], DImode)
   && !reload_completed) && (ptr_mode == SImode || Pmode == SImode)) && ( true)")
  (-1 "TARGET_SVE
   && IN_RANGE (INTVAL (operands[2]) * GET_MODE_SIZE (HImode), 1, 15)")
  (-1 "TARGET_SIMD
   && IN_RANGE (aarch64_vec_fpconst_pow_of_2 (operands[2]), 1,
		GET_MODE_BITSIZE (GET_MODE_INNER (V4SFmode)))")
  (-1 "!HONOR_SIGNED_ZEROS (SFmode) && TARGET_FLOAT")
  (-1 "(INTVAL (operands[2]) == GET_MODE_BITSIZE (SImode)) && ( true)")
  (-1 "TARGET_SIMD
   && (register_operand (operands[0], V8HFmode)
       || aarch64_simd_reg_or_zero (operands[1], V8HFmode))")
  (-1 "(TARGET_SVE
   && (register_operand (operands[0], VNx4DFmode)
       || register_operand (operands[2], VNx4DFmode))) && ( reload_completed)")
  (-1 "aarch64_uxt_size (INTVAL (operands[2]),INTVAL (operands[3])) != 0")
  (-1 "TARGET_SIMD_F16INST")
  (-1 "UINTVAL (operands[3]) < 32 &&
   (UINTVAL (operands[3]) + UINTVAL (operands[4]) == 32)")
  (-1 "aarch64_move_imm (INTVAL (operands[2]), SImode)")
  (-1 "TARGET_SIMD
   && rtx_equal_p (XEXP (operands[3], 0),
		   plus_constant (Pmode,
				  XEXP (operands[1], 0),
				  GET_MODE_SIZE (V2SFmode)))")
  (-1 "TARGET_SVE
   && (register_operand (operands[0], VNx16BImode)
       || register_operand (operands[1], VNx16BImode))")
  (-1 "!aarch64_move_imm (INTVAL (operands[2]), DImode)
   && !aarch64_plus_operand (operands[2], DImode)
   && !reload_completed")
])
