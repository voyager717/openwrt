/* -*- buffer-read-only: t -*- */
/* Generated automatically by geniterators.sh from iterators.md.  */
#ifndef GCC_AARCH64_ITERATORS_H
#define GCC_AARCH64_ITERATORS_H
#define BUILTIN_GPI(T, N, MAP) \
  VAR2 (T, N, MAP, si, di)
#define BUILTIN_GPI_I16(T, N, MAP) \
  VAR3 (T, N, MAP, hi, si, di)
#define BUILTIN_SHORT(T, N, MAP) \
  VAR2 (T, N, MAP, qi, hi)
#define BUILTIN_ALLI(T, N, MAP) \
  VAR4 (T, N, MAP, qi, hi, si, di)
#define BUILTIN_ALLX(T, N, MAP) \
  VAR3 (T, N, MAP, qi, hi, si)
#define BUILTIN_GPF(T, N, MAP) \
  VAR2 (T, N, MAP, sf, df)
#define BUILTIN_GPF_F16(T, N, MAP) \
  VAR3 (T, N, MAP, hf, sf, df)
#define BUILTIN_GPF_HF(T, N, MAP) \
  VAR3 (T, N, MAP, hf, sf, df)
#define BUILTIN_GPF_TF_F16(T, N, MAP) \
  VAR4 (T, N, MAP, hf, sf, df, tf)
#define BUILTIN_VDF(T, N, MAP) \
  VAR2 (T, N, MAP, v2sf, v4hf)
#define BUILTIN_GPF_TF(T, N, MAP) \
  VAR3 (T, N, MAP, sf, df, tf)
#define BUILTIN_VDQ_I(T, N, MAP) \
  VAR7 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v2di)
#define BUILTIN_VSDQ_I(T, N, MAP) \
  VAR11 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v2di, qi, hi, si, di)
#define BUILTIN_VSDQ_I_DI(T, N, MAP) \
  VAR8 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v2di, di)
#define BUILTIN_VD(T, N, MAP) \
  VAR5 (T, N, MAP, v8qi, v4hi, v4hf, v2si, v2sf)
#define BUILTIN_VD_BHSI(T, N, MAP) \
  VAR3 (T, N, MAP, v8qi, v4hi, v2si)
#define BUILTIN_VDQ_BHSI(T, N, MAP) \
  VAR6 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si)
#define BUILTIN_VQ(T, N, MAP) \
  VAR7 (T, N, MAP, v16qi, v8hi, v4si, v2di, v8hf, v4sf, v2df)
#define BUILTIN_VQ_NO2E(T, N, MAP) \
  VAR5 (T, N, MAP, v16qi, v8hi, v4si, v8hf, v4sf)
#define BUILTIN_VQ_2E(T, N, MAP) \
  VAR2 (T, N, MAP, v2di, v2df)
#define BUILTIN_P(T, N, MAP) \
  VAR2 (T, N, MAP, si, di)
#define BUILTIN_PTR(T, N, MAP) \
  VAR2 (T, N, MAP, si, di)
#define BUILTIN_VDQF_F16(T, N, MAP) \
  VAR5 (T, N, MAP, v4hf, v8hf, v2sf, v4sf, v2df)
#define BUILTIN_VDQF(T, N, MAP) \
  VAR3 (T, N, MAP, v2sf, v4sf, v2df)
#define BUILTIN_VHSDF(T, N, MAP) \
  VAR5 (T, N, MAP, v4hf, v8hf, v2sf, v4sf, v2df)
#define BUILTIN_VHSDF_DF(T, N, MAP) \
  VAR6 (T, N, MAP, v4hf, v8hf, v2sf, v4sf, v2df, df)
#define BUILTIN_VHSDF_HSDF(T, N, MAP) \
  VAR8 (T, N, MAP, v4hf, v8hf, v2sf, v4sf, v2df, hf, sf, df)
#define BUILTIN_VDQSF(T, N, MAP) \
  VAR2 (T, N, MAP, v2sf, v4sf)
#define BUILTIN_VQ_HSF(T, N, MAP) \
  VAR2 (T, N, MAP, v8hf, v4sf)
#define BUILTIN_VDQF_COND(T, N, MAP) \
  VAR6 (T, N, MAP, v2sf, v2si, v4sf, v4si, v2df, v2di)
#define BUILTIN_VALLF(T, N, MAP) \
  VAR5 (T, N, MAP, v2sf, v4sf, v2df, sf, df)
#define BUILTIN_V2F(T, N, MAP) \
  VAR2 (T, N, MAP, v2sf, v2df)
#define BUILTIN_VALL(T, N, MAP) \
  VAR10 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v2di, v2sf, v4sf, v2df)
#define BUILTIN_VALL_F16(T, N, MAP) \
  VAR12 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v2di, v4hf, v8hf, v2sf, v4sf, v2df)
#define BUILTIN_VALL_F16_NO_V2Q(T, N, MAP) \
  VAR10 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v4hf, v8hf, v2sf, v4sf)
#define BUILTIN_VALLDI(T, N, MAP) \
  VAR11 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v2di, v2sf, v4sf, v2df, di)
#define BUILTIN_VALLDI_F16(T, N, MAP) \
  VAR13 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v2di, v4hf, v8hf, v2sf, v4sf, v2df, di)
#define BUILTIN_VALLDIF(T, N, MAP) \
  VAR14 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v2di, v4hf, v8hf, v2sf, v4sf, v2df, di, df)
#define BUILTIN_VDQV(T, N, MAP) \
  VAR6 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v4si, v2di)
#define BUILTIN_VDQV_S(T, N, MAP) \
  VAR5 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v4si)
#define BUILTIN_VDN(T, N, MAP) \
  VAR3 (T, N, MAP, v4hi, v2si, di)
#define BUILTIN_VQN(T, N, MAP) \
  VAR3 (T, N, MAP, v8hi, v4si, v2di)
#define BUILTIN_VSQN_HSDI(T, N, MAP) \
  VAR6 (T, N, MAP, v8hi, v4si, v2di, hi, si, di)
#define BUILTIN_VQW(T, N, MAP) \
  VAR3 (T, N, MAP, v16qi, v8hi, v4si)
#define BUILTIN_VDC(T, N, MAP) \
  VAR7 (T, N, MAP, v8qi, v4hi, v4hf, v2si, v2sf, di, df)
#define BUILTIN_VDQIF(T, N, MAP) \
  VAR9 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v2sf, v4sf, v2df)
#define BUILTIN_VDQIF_F16(T, N, MAP) \
  VAR11 (T, N, MAP, v8qi, v16qi, v4hi, v8hi, v2si, v4si, v4hf, v8hf, v2sf, v4sf, v2df)
#define BUILTIN_VDQ_SI(T, N, MAP) \
  VAR2 (T, N, MAP, v2si, v4si)
#define BUILTIN_VDQ_SDI(T, N, MAP) \
  VAR3 (T, N, MAP, v2si, v4si, v2di)
#define BUILTIN_VDQ_HSDI(T, N, MAP) \
  VAR5 (T, N, MAP, v4hi, v8hi, v2si, v4si, v2di)
#define BUILTIN_VSDQ_SDI(T, N, MAP) \
  VAR5 (T, N, MAP, v2si, v4si, v2di, si, di)
#define BUILTIN_VSDQ_HSDI(T, N, MAP) \
  VAR8 (T, N, MAP, v4hi, v8hi, v2si, v4si, v2di, hi, si, di)
#define BUILTIN_VDQQH(T, N, MAP) \
  VAR4 (T, N, MAP, v8qi, v16qi, v4hi, v8hi)
#define BUILTIN_VDQHS(T, N, MAP) \
  VAR4 (T, N, MAP, v4hi, v8hi, v2si, v4si)
#define BUILTIN_VDQHSD(T, N, MAP) \
  VAR5 (T, N, MAP, v4hi, v8hi, v2si, v4si, v2di)
#define BUILTIN_VSDQ_HSI(T, N, MAP) \
  VAR6 (T, N, MAP, v4hi, v8hi, v2si, v4si, hi, si)
#define BUILTIN_VSD_HSI(T, N, MAP) \
  VAR4 (T, N, MAP, v4hi, v2si, hi, si)
#define BUILTIN_VD_HSI(T, N, MAP) \
  VAR2 (T, N, MAP, v4hi, v2si)
#define BUILTIN_SD_HSI(T, N, MAP) \
  VAR2 (T, N, MAP, hi, si)
#define BUILTIN_VQ_HSI(T, N, MAP) \
  VAR2 (T, N, MAP, v8hi, v4si)
#define BUILTIN_VB(T, N, MAP) \
  VAR2 (T, N, MAP, v8qi, v16qi)
#define BUILTIN_VS(T, N, MAP) \
  VAR2 (T, N, MAP, v2si, v4si)
#define BUILTIN_TX(T, N, MAP) \
  VAR2 (T, N, MAP, ti, tf)
#define BUILTIN_VSTRUCT(T, N, MAP) \
  VAR3 (T, N, MAP, oi, ci, xi)
#define BUILTIN_DX(T, N, MAP) \
  VAR2 (T, N, MAP, di, df)
#define BUILTIN_VMUL(T, N, MAP) \
  VAR9 (T, N, MAP, v4hi, v8hi, v2si, v4si, v4hf, v8hf, v2sf, v4sf, v2df)
#define BUILTIN_VMUL_CHANGE_NLANES(T, N, MAP) \
  VAR6 (T, N, MAP, v4hi, v8hi, v2si, v4si, v2sf, v4sf)
#endif /* GCC_AARCH64_ITERATORS_H  */
