/* Generated automatically by the program `genopinit'
   from the machine description file `md'.  */

#define IN_TARGET_CODE 1
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "predict.h"
#include "tree.h"
#include "rtl.h"
#include "alias.h"
#include "varasm.h"
#include "stor-layout.h"
#include "calls.h"
#include "memmodel.h"
#include "tm_p.h"
#include "flags.h"
#include "insn-config.h"
#include "expmed.h"
#include "dojump.h"
#include "explow.h"
#include "emit-rtl.h"
#include "stmt.h"
#include "expr.h"
#include "insn-codes.h"
#include "optabs.h"

struct optab_pat {
  unsigned scode;
  enum insn_code icode;
};

static const struct optab_pat pats[NUM_OPTAB_PATTERNS] = {
  { 0x010a0b, CODE_FOR_extendqihi2 },
  { 0x010a0c, CODE_FOR_extendqisi2 },
  { 0x010a0d, CODE_FOR_extendqidi2 },
  { 0x010b0c, CODE_FOR_extendhisi2 },
  { 0x010b0d, CODE_FOR_extendhidi2 },
  { 0x010c0d, CODE_FOR_extendsidi2 },
  { 0x012425, CODE_FOR_extendhfsf2 },
  { 0x012426, CODE_FOR_extendhfdf2 },
  { 0x012526, CODE_FOR_extendsfdf2 },
  { 0x022524, CODE_FOR_truncsfhf2 },
  { 0x022624, CODE_FOR_truncdfhf2 },
  { 0x022625, CODE_FOR_truncdfsf2 },
  { 0x030a0b, CODE_FOR_zero_extendqihi2 },
  { 0x030a0c, CODE_FOR_zero_extendqisi2 },
  { 0x030a0d, CODE_FOR_zero_extendqidi2 },
  { 0x030b0c, CODE_FOR_zero_extendhisi2 },
  { 0x030b0d, CODE_FOR_zero_extendhidi2 },
  { 0x030c0d, CODE_FOR_zero_extendsidi2 },
  { 0x04573c, CODE_FOR_fixv4hfv4hi2 },
  { 0x04583d, CODE_FOR_fixv2sfv2si2 },
  { 0x045a40, CODE_FOR_fixv8hfv8hi2 },
  { 0x045c42, CODE_FOR_fixv4sfv4si2 },
  { 0x045e44, CODE_FOR_fixv2dfv2di2 },
  { 0x05573c, CODE_FOR_fixunsv4hfv4hi2 },
  { 0x05583d, CODE_FOR_fixunsv2sfv2si2 },
  { 0x055a40, CODE_FOR_fixunsv8hfv8hi2 },
  { 0x055c42, CODE_FOR_fixunsv4sfv4si2 },
  { 0x055e44, CODE_FOR_fixunsv2dfv2di2 },
  { 0x060b24, CODE_FOR_floathihf2 },
  { 0x060c24, CODE_FOR_floatsihf2 },
  { 0x060c25, CODE_FOR_floatsisf2 },
  { 0x060c26, CODE_FOR_floatsidf2 },
  { 0x060d24, CODE_FOR_floatdihf2 },
  { 0x060d25, CODE_FOR_floatdisf2 },
  { 0x060d26, CODE_FOR_floatdidf2 },
  { 0x063c57, CODE_FOR_floatv4hiv4hf2 },
  { 0x063d58, CODE_FOR_floatv2siv2sf2 },
  { 0x06405a, CODE_FOR_floatv8hiv8hf2 },
  { 0x06415b, CODE_FOR_floatvnx8hivnx8hf2 },
  { 0x06425c, CODE_FOR_floatv4siv4sf2 },
  { 0x06435d, CODE_FOR_floatvnx4sivnx4sf2 },
  { 0x06445e, CODE_FOR_floatv2div2df2 },
  { 0x06455f, CODE_FOR_floatvnx2divnx2df2 },
  { 0x070b24, CODE_FOR_floatunshihf2 },
  { 0x070c24, CODE_FOR_floatunssihf2 },
  { 0x070c25, CODE_FOR_floatunssisf2 },
  { 0x070c26, CODE_FOR_floatunssidf2 },
  { 0x070d24, CODE_FOR_floatunsdihf2 },
  { 0x070d25, CODE_FOR_floatunsdisf2 },
  { 0x070d26, CODE_FOR_floatunsdidf2 },
  { 0x073c57, CODE_FOR_floatunsv4hiv4hf2 },
  { 0x073d58, CODE_FOR_floatunsv2siv2sf2 },
  { 0x07405a, CODE_FOR_floatunsv8hiv8hf2 },
  { 0x07415b, CODE_FOR_floatunsvnx8hivnx8hf2 },
  { 0x07425c, CODE_FOR_floatunsv4siv4sf2 },
  { 0x07435d, CODE_FOR_floatunsvnx4sivnx4sf2 },
  { 0x07445e, CODE_FOR_floatunsv2div2df2 },
  { 0x07455f, CODE_FOR_floatunsvnx2divnx2df2 },
  { 0x08250c, CODE_FOR_lrintsfsi2 },
  { 0x08250d, CODE_FOR_lrintsfdi2 },
  { 0x08260c, CODE_FOR_lrintdfsi2 },
  { 0x08260d, CODE_FOR_lrintdfdi2 },
  { 0x09240b, CODE_FOR_lroundhfhi2 },
  { 0x09240c, CODE_FOR_lroundhfsi2 },
  { 0x09240d, CODE_FOR_lroundhfdi2 },
  { 0x09250c, CODE_FOR_lroundsfsi2 },
  { 0x09250d, CODE_FOR_lroundsfdi2 },
  { 0x09260c, CODE_FOR_lrounddfsi2 },
  { 0x09260d, CODE_FOR_lrounddfdi2 },
  { 0x09573c, CODE_FOR_lroundv4hfv4hi2 },
  { 0x09583d, CODE_FOR_lroundv2sfv2si2 },
  { 0x095a40, CODE_FOR_lroundv8hfv8hi2 },
  { 0x095c42, CODE_FOR_lroundv4sfv4si2 },
  { 0x095e44, CODE_FOR_lroundv2dfv2di2 },
  { 0x0a240b, CODE_FOR_lfloorhfhi2 },
  { 0x0a240c, CODE_FOR_lfloorhfsi2 },
  { 0x0a240d, CODE_FOR_lfloorhfdi2 },
  { 0x0a250c, CODE_FOR_lfloorsfsi2 },
  { 0x0a250d, CODE_FOR_lfloorsfdi2 },
  { 0x0a260c, CODE_FOR_lfloordfsi2 },
  { 0x0a260d, CODE_FOR_lfloordfdi2 },
  { 0x0a573c, CODE_FOR_lfloorv4hfv4hi2 },
  { 0x0a583d, CODE_FOR_lfloorv2sfv2si2 },
  { 0x0a5a40, CODE_FOR_lfloorv8hfv8hi2 },
  { 0x0a5c42, CODE_FOR_lfloorv4sfv4si2 },
  { 0x0a5e44, CODE_FOR_lfloorv2dfv2di2 },
  { 0x0b240b, CODE_FOR_lceilhfhi2 },
  { 0x0b240c, CODE_FOR_lceilhfsi2 },
  { 0x0b240d, CODE_FOR_lceilhfdi2 },
  { 0x0b250c, CODE_FOR_lceilsfsi2 },
  { 0x0b250d, CODE_FOR_lceilsfdi2 },
  { 0x0b260c, CODE_FOR_lceildfsi2 },
  { 0x0b260d, CODE_FOR_lceildfdi2 },
  { 0x0b573c, CODE_FOR_lceilv4hfv4hi2 },
  { 0x0b583d, CODE_FOR_lceilv2sfv2si2 },
  { 0x0b5a40, CODE_FOR_lceilv8hfv8hi2 },
  { 0x0b5c42, CODE_FOR_lceilv4sfv4si2 },
  { 0x0b5e44, CODE_FOR_lceilv2dfv2di2 },
  { 0x10240b, CODE_FOR_fix_trunchfhi2 },
  { 0x10240c, CODE_FOR_fix_trunchfsi2 },
  { 0x10240d, CODE_FOR_fix_trunchfdi2 },
  { 0x10250c, CODE_FOR_fix_truncsfsi2 },
  { 0x10250d, CODE_FOR_fix_truncsfdi2 },
  { 0x10260c, CODE_FOR_fix_truncdfsi2 },
  { 0x10260d, CODE_FOR_fix_truncdfdi2 },
  { 0x10573c, CODE_FOR_fix_truncv4hfv4hi2 },
  { 0x10583d, CODE_FOR_fix_truncv2sfv2si2 },
  { 0x105a40, CODE_FOR_fix_truncv8hfv8hi2 },
  { 0x105b41, CODE_FOR_fix_truncvnx8hfvnx8hi2 },
  { 0x105c42, CODE_FOR_fix_truncv4sfv4si2 },
  { 0x105d43, CODE_FOR_fix_truncvnx4sfvnx4si2 },
  { 0x105e44, CODE_FOR_fix_truncv2dfv2di2 },
  { 0x105f45, CODE_FOR_fix_truncvnx2dfvnx2di2 },
  { 0x11240b, CODE_FOR_fixuns_trunchfhi2 },
  { 0x11240c, CODE_FOR_fixuns_trunchfsi2 },
  { 0x11240d, CODE_FOR_fixuns_trunchfdi2 },
  { 0x11250c, CODE_FOR_fixuns_truncsfsi2 },
  { 0x11250d, CODE_FOR_fixuns_truncsfdi2 },
  { 0x11260c, CODE_FOR_fixuns_truncdfsi2 },
  { 0x11260d, CODE_FOR_fixuns_truncdfdi2 },
  { 0x11573c, CODE_FOR_fixuns_truncv4hfv4hi2 },
  { 0x11583d, CODE_FOR_fixuns_truncv2sfv2si2 },
  { 0x115a40, CODE_FOR_fixuns_truncv8hfv8hi2 },
  { 0x115b41, CODE_FOR_fixuns_truncvnx8hfvnx8hi2 },
  { 0x115c42, CODE_FOR_fixuns_truncv4sfv4si2 },
  { 0x115d43, CODE_FOR_fixuns_truncvnx4sfvnx4si2 },
  { 0x115e44, CODE_FOR_fixuns_truncv2dfv2di2 },
  { 0x115f45, CODE_FOR_fixuns_truncvnx2dfvnx2di2 },
  { 0x120c0d, CODE_FOR_mulsidi3 },
  { 0x120d0e, CODE_FOR_mulditi3 },
  { 0x130c0d, CODE_FOR_umulsidi3 },
  { 0x130d0e, CODE_FOR_umulditi3 },
  { 0x150c0d, CODE_FOR_maddsidi4 },
  { 0x160c0d, CODE_FOR_umaddsidi4 },
  { 0x190c0d, CODE_FOR_msubsidi4 },
  { 0x1a0c0d, CODE_FOR_umsubsidi4 },
  { 0x1d3e0f, CODE_FOR_vec_load_lanesoiv16qi },
  { 0x1d3e10, CODE_FOR_vec_load_lanesciv16qi },
  { 0x1d3e11, CODE_FOR_vec_load_lanesxiv16qi },
  { 0x1d3f46, CODE_FOR_vec_load_lanesvnx32qivnx16qi },
  { 0x1d3f4b, CODE_FOR_vec_load_lanesvnx48qivnx16qi },
  { 0x1d3f50, CODE_FOR_vec_load_lanesvnx64qivnx16qi },
  { 0x1d400f, CODE_FOR_vec_load_lanesoiv8hi },
  { 0x1d4010, CODE_FOR_vec_load_lanesciv8hi },
  { 0x1d4011, CODE_FOR_vec_load_lanesxiv8hi },
  { 0x1d4147, CODE_FOR_vec_load_lanesvnx16hivnx8hi },
  { 0x1d414c, CODE_FOR_vec_load_lanesvnx24hivnx8hi },
  { 0x1d4151, CODE_FOR_vec_load_lanesvnx32hivnx8hi },
  { 0x1d420f, CODE_FOR_vec_load_lanesoiv4si },
  { 0x1d4210, CODE_FOR_vec_load_lanesciv4si },
  { 0x1d4211, CODE_FOR_vec_load_lanesxiv4si },
  { 0x1d4348, CODE_FOR_vec_load_lanesvnx8sivnx4si },
  { 0x1d434d, CODE_FOR_vec_load_lanesvnx12sivnx4si },
  { 0x1d4352, CODE_FOR_vec_load_lanesvnx16sivnx4si },
  { 0x1d440f, CODE_FOR_vec_load_lanesoiv2di },
  { 0x1d4410, CODE_FOR_vec_load_lanesciv2di },
  { 0x1d4411, CODE_FOR_vec_load_lanesxiv2di },
  { 0x1d4549, CODE_FOR_vec_load_lanesvnx4divnx2di },
  { 0x1d454e, CODE_FOR_vec_load_lanesvnx6divnx2di },
  { 0x1d4553, CODE_FOR_vec_load_lanesvnx8divnx2di },
  { 0x1d5a0f, CODE_FOR_vec_load_lanesoiv8hf },
  { 0x1d5a10, CODE_FOR_vec_load_lanesciv8hf },
  { 0x1d5a11, CODE_FOR_vec_load_lanesxiv8hf },
  { 0x1d5b60, CODE_FOR_vec_load_lanesvnx16hfvnx8hf },
  { 0x1d5b63, CODE_FOR_vec_load_lanesvnx24hfvnx8hf },
  { 0x1d5b66, CODE_FOR_vec_load_lanesvnx32hfvnx8hf },
  { 0x1d5c0f, CODE_FOR_vec_load_lanesoiv4sf },
  { 0x1d5c10, CODE_FOR_vec_load_lanesciv4sf },
  { 0x1d5c11, CODE_FOR_vec_load_lanesxiv4sf },
  { 0x1d5d61, CODE_FOR_vec_load_lanesvnx8sfvnx4sf },
  { 0x1d5d64, CODE_FOR_vec_load_lanesvnx12sfvnx4sf },
  { 0x1d5d67, CODE_FOR_vec_load_lanesvnx16sfvnx4sf },
  { 0x1d5e0f, CODE_FOR_vec_load_lanesoiv2df },
  { 0x1d5e10, CODE_FOR_vec_load_lanesciv2df },
  { 0x1d5e11, CODE_FOR_vec_load_lanesxiv2df },
  { 0x1d5f62, CODE_FOR_vec_load_lanesvnx4dfvnx2df },
  { 0x1d5f65, CODE_FOR_vec_load_lanesvnx6dfvnx2df },
  { 0x1d5f68, CODE_FOR_vec_load_lanesvnx8dfvnx2df },
  { 0x1e3e0f, CODE_FOR_vec_store_lanesoiv16qi },
  { 0x1e3e10, CODE_FOR_vec_store_lanesciv16qi },
  { 0x1e3e11, CODE_FOR_vec_store_lanesxiv16qi },
  { 0x1e3f46, CODE_FOR_vec_store_lanesvnx32qivnx16qi },
  { 0x1e3f4b, CODE_FOR_vec_store_lanesvnx48qivnx16qi },
  { 0x1e3f50, CODE_FOR_vec_store_lanesvnx64qivnx16qi },
  { 0x1e400f, CODE_FOR_vec_store_lanesoiv8hi },
  { 0x1e4010, CODE_FOR_vec_store_lanesciv8hi },
  { 0x1e4011, CODE_FOR_vec_store_lanesxiv8hi },
  { 0x1e4147, CODE_FOR_vec_store_lanesvnx16hivnx8hi },
  { 0x1e414c, CODE_FOR_vec_store_lanesvnx24hivnx8hi },
  { 0x1e4151, CODE_FOR_vec_store_lanesvnx32hivnx8hi },
  { 0x1e420f, CODE_FOR_vec_store_lanesoiv4si },
  { 0x1e4210, CODE_FOR_vec_store_lanesciv4si },
  { 0x1e4211, CODE_FOR_vec_store_lanesxiv4si },
  { 0x1e4348, CODE_FOR_vec_store_lanesvnx8sivnx4si },
  { 0x1e434d, CODE_FOR_vec_store_lanesvnx12sivnx4si },
  { 0x1e4352, CODE_FOR_vec_store_lanesvnx16sivnx4si },
  { 0x1e440f, CODE_FOR_vec_store_lanesoiv2di },
  { 0x1e4410, CODE_FOR_vec_store_lanesciv2di },
  { 0x1e4411, CODE_FOR_vec_store_lanesxiv2di },
  { 0x1e4549, CODE_FOR_vec_store_lanesvnx4divnx2di },
  { 0x1e454e, CODE_FOR_vec_store_lanesvnx6divnx2di },
  { 0x1e4553, CODE_FOR_vec_store_lanesvnx8divnx2di },
  { 0x1e5a0f, CODE_FOR_vec_store_lanesoiv8hf },
  { 0x1e5a10, CODE_FOR_vec_store_lanesciv8hf },
  { 0x1e5a11, CODE_FOR_vec_store_lanesxiv8hf },
  { 0x1e5b60, CODE_FOR_vec_store_lanesvnx16hfvnx8hf },
  { 0x1e5b63, CODE_FOR_vec_store_lanesvnx24hfvnx8hf },
  { 0x1e5b66, CODE_FOR_vec_store_lanesvnx32hfvnx8hf },
  { 0x1e5c0f, CODE_FOR_vec_store_lanesoiv4sf },
  { 0x1e5c10, CODE_FOR_vec_store_lanesciv4sf },
  { 0x1e5c11, CODE_FOR_vec_store_lanesxiv4sf },
  { 0x1e5d61, CODE_FOR_vec_store_lanesvnx8sfvnx4sf },
  { 0x1e5d64, CODE_FOR_vec_store_lanesvnx12sfvnx4sf },
  { 0x1e5d67, CODE_FOR_vec_store_lanesvnx16sfvnx4sf },
  { 0x1e5e0f, CODE_FOR_vec_store_lanesoiv2df },
  { 0x1e5e10, CODE_FOR_vec_store_lanesciv2df },
  { 0x1e5e11, CODE_FOR_vec_store_lanesxiv2df },
  { 0x1e5f62, CODE_FOR_vec_store_lanesvnx4dfvnx2df },
  { 0x1e5f65, CODE_FOR_vec_store_lanesvnx6dfvnx2df },
  { 0x1e5f68, CODE_FOR_vec_store_lanesvnx8dfvnx2df },
  { 0x1f3f46, CODE_FOR_vec_mask_load_lanesvnx32qivnx16qi },
  { 0x1f3f4b, CODE_FOR_vec_mask_load_lanesvnx48qivnx16qi },
  { 0x1f3f50, CODE_FOR_vec_mask_load_lanesvnx64qivnx16qi },
  { 0x1f4147, CODE_FOR_vec_mask_load_lanesvnx16hivnx8hi },
  { 0x1f414c, CODE_FOR_vec_mask_load_lanesvnx24hivnx8hi },
  { 0x1f4151, CODE_FOR_vec_mask_load_lanesvnx32hivnx8hi },
  { 0x1f4348, CODE_FOR_vec_mask_load_lanesvnx8sivnx4si },
  { 0x1f434d, CODE_FOR_vec_mask_load_lanesvnx12sivnx4si },
  { 0x1f4352, CODE_FOR_vec_mask_load_lanesvnx16sivnx4si },
  { 0x1f4549, CODE_FOR_vec_mask_load_lanesvnx4divnx2di },
  { 0x1f454e, CODE_FOR_vec_mask_load_lanesvnx6divnx2di },
  { 0x1f4553, CODE_FOR_vec_mask_load_lanesvnx8divnx2di },
  { 0x1f5b60, CODE_FOR_vec_mask_load_lanesvnx16hfvnx8hf },
  { 0x1f5b63, CODE_FOR_vec_mask_load_lanesvnx24hfvnx8hf },
  { 0x1f5b66, CODE_FOR_vec_mask_load_lanesvnx32hfvnx8hf },
  { 0x1f5d61, CODE_FOR_vec_mask_load_lanesvnx8sfvnx4sf },
  { 0x1f5d64, CODE_FOR_vec_mask_load_lanesvnx12sfvnx4sf },
  { 0x1f5d67, CODE_FOR_vec_mask_load_lanesvnx16sfvnx4sf },
  { 0x1f5f62, CODE_FOR_vec_mask_load_lanesvnx4dfvnx2df },
  { 0x1f5f65, CODE_FOR_vec_mask_load_lanesvnx6dfvnx2df },
  { 0x1f5f68, CODE_FOR_vec_mask_load_lanesvnx8dfvnx2df },
  { 0x203f46, CODE_FOR_vec_mask_store_lanesvnx32qivnx16qi },
  { 0x203f4b, CODE_FOR_vec_mask_store_lanesvnx48qivnx16qi },
  { 0x203f50, CODE_FOR_vec_mask_store_lanesvnx64qivnx16qi },
  { 0x204147, CODE_FOR_vec_mask_store_lanesvnx16hivnx8hi },
  { 0x20414c, CODE_FOR_vec_mask_store_lanesvnx24hivnx8hi },
  { 0x204151, CODE_FOR_vec_mask_store_lanesvnx32hivnx8hi },
  { 0x204348, CODE_FOR_vec_mask_store_lanesvnx8sivnx4si },
  { 0x20434d, CODE_FOR_vec_mask_store_lanesvnx12sivnx4si },
  { 0x204352, CODE_FOR_vec_mask_store_lanesvnx16sivnx4si },
  { 0x204549, CODE_FOR_vec_mask_store_lanesvnx4divnx2di },
  { 0x20454e, CODE_FOR_vec_mask_store_lanesvnx6divnx2di },
  { 0x204553, CODE_FOR_vec_mask_store_lanesvnx8divnx2di },
  { 0x205b60, CODE_FOR_vec_mask_store_lanesvnx16hfvnx8hf },
  { 0x205b63, CODE_FOR_vec_mask_store_lanesvnx24hfvnx8hf },
  { 0x205b66, CODE_FOR_vec_mask_store_lanesvnx32hfvnx8hf },
  { 0x205d61, CODE_FOR_vec_mask_store_lanesvnx8sfvnx4sf },
  { 0x205d64, CODE_FOR_vec_mask_store_lanesvnx12sfvnx4sf },
  { 0x205d67, CODE_FOR_vec_mask_store_lanesvnx16sfvnx4sf },
  { 0x205f62, CODE_FOR_vec_mask_store_lanesvnx4dfvnx2df },
  { 0x205f65, CODE_FOR_vec_mask_store_lanesvnx6dfvnx2df },
  { 0x205f68, CODE_FOR_vec_mask_store_lanesvnx8dfvnx2df },
  { 0x210d0d, CODE_FOR_vconddidi },
  { 0x213b3b, CODE_FOR_vcondv8qiv8qi },
  { 0x213c3c, CODE_FOR_vcondv4hiv4hi },
  { 0x213d3d, CODE_FOR_vcondv2siv2si },
  { 0x213d58, CODE_FOR_vcondv2sfv2si },
  { 0x213e3e, CODE_FOR_vcondv16qiv16qi },
  { 0x213f3f, CODE_FOR_vcondvnx16qivnx16qi },
  { 0x214040, CODE_FOR_vcondv8hiv8hi },
  { 0x214141, CODE_FOR_vcondvnx8hivnx8hi },
  { 0x21415b, CODE_FOR_vcondvnx8hfvnx8hi },
  { 0x214242, CODE_FOR_vcondv4siv4si },
  { 0x21425c, CODE_FOR_vcondv4sfv4si },
  { 0x214343, CODE_FOR_vcondvnx4sivnx4si },
  { 0x21435d, CODE_FOR_vcondvnx4sfvnx4si },
  { 0x214444, CODE_FOR_vcondv2div2di },
  { 0x21445e, CODE_FOR_vcondv2dfv2di },
  { 0x214545, CODE_FOR_vcondvnx2divnx2di },
  { 0x21455f, CODE_FOR_vcondvnx2dfvnx2di },
  { 0x21583d, CODE_FOR_vcondv2siv2sf },
  { 0x215858, CODE_FOR_vcondv2sfv2sf },
  { 0x215c42, CODE_FOR_vcondv4siv4sf },
  { 0x215c5c, CODE_FOR_vcondv4sfv4sf },
  { 0x215d43, CODE_FOR_vcondvnx4sivnx4sf },
  { 0x215d5d, CODE_FOR_vcondvnx4sfvnx4sf },
  { 0x215e44, CODE_FOR_vcondv2div2df },
  { 0x215e5e, CODE_FOR_vcondv2dfv2df },
  { 0x215f45, CODE_FOR_vcondvnx2divnx2df },
  { 0x215f5f, CODE_FOR_vcondvnx2dfvnx2df },
  { 0x220d0d, CODE_FOR_vcondudidi },
  { 0x223b3b, CODE_FOR_vconduv8qiv8qi },
  { 0x223c3c, CODE_FOR_vconduv4hiv4hi },
  { 0x223d3d, CODE_FOR_vconduv2siv2si },
  { 0x223d58, CODE_FOR_vconduv2sfv2si },
  { 0x223e3e, CODE_FOR_vconduv16qiv16qi },
  { 0x223f3f, CODE_FOR_vconduvnx16qivnx16qi },
  { 0x224040, CODE_FOR_vconduv8hiv8hi },
  { 0x224141, CODE_FOR_vconduvnx8hivnx8hi },
  { 0x22415b, CODE_FOR_vconduvnx8hfvnx8hi },
  { 0x224242, CODE_FOR_vconduv4siv4si },
  { 0x22425c, CODE_FOR_vconduv4sfv4si },
  { 0x224343, CODE_FOR_vconduvnx4sivnx4si },
  { 0x22435d, CODE_FOR_vconduvnx4sfvnx4si },
  { 0x224444, CODE_FOR_vconduv2div2di },
  { 0x22445e, CODE_FOR_vconduv2dfv2di },
  { 0x224545, CODE_FOR_vconduvnx2divnx2di },
  { 0x22455f, CODE_FOR_vconduvnx2dfvnx2di },
  { 0x240d0d, CODE_FOR_vcond_mask_didi },
  { 0x24373f, CODE_FOR_vcond_mask_vnx16qivnx16bi },
  { 0x243841, CODE_FOR_vcond_mask_vnx8hivnx8bi },
  { 0x24385b, CODE_FOR_vcond_mask_vnx8hfvnx8bi },
  { 0x243943, CODE_FOR_vcond_mask_vnx4sivnx4bi },
  { 0x24395d, CODE_FOR_vcond_mask_vnx4sfvnx4bi },
  { 0x243a45, CODE_FOR_vcond_mask_vnx2divnx2bi },
  { 0x243a5f, CODE_FOR_vcond_mask_vnx2dfvnx2bi },
  { 0x243b3b, CODE_FOR_vcond_mask_v8qiv8qi },
  { 0x243c3c, CODE_FOR_vcond_mask_v4hiv4hi },
  { 0x243d3d, CODE_FOR_vcond_mask_v2siv2si },
  { 0x243d58, CODE_FOR_vcond_mask_v2sfv2si },
  { 0x243e3e, CODE_FOR_vcond_mask_v16qiv16qi },
  { 0x244040, CODE_FOR_vcond_mask_v8hiv8hi },
  { 0x244242, CODE_FOR_vcond_mask_v4siv4si },
  { 0x24425c, CODE_FOR_vcond_mask_v4sfv4si },
  { 0x244444, CODE_FOR_vcond_mask_v2div2di },
  { 0x24445e, CODE_FOR_vcond_mask_v2dfv2di },
  { 0x250d0d, CODE_FOR_vec_cmpdidi },
  { 0x25373f, CODE_FOR_vec_cmpvnx16qivnx16bi },
  { 0x253841, CODE_FOR_vec_cmpvnx8hivnx8bi },
  { 0x25385b, CODE_FOR_vec_cmpvnx8hfvnx8bi },
  { 0x253943, CODE_FOR_vec_cmpvnx4sivnx4bi },
  { 0x25395d, CODE_FOR_vec_cmpvnx4sfvnx4bi },
  { 0x253a45, CODE_FOR_vec_cmpvnx2divnx2bi },
  { 0x253a5f, CODE_FOR_vec_cmpvnx2dfvnx2bi },
  { 0x253b3b, CODE_FOR_vec_cmpv8qiv8qi },
  { 0x253c3c, CODE_FOR_vec_cmpv4hiv4hi },
  { 0x253d3d, CODE_FOR_vec_cmpv2siv2si },
  { 0x253d58, CODE_FOR_vec_cmpv2sfv2si },
  { 0x253e3e, CODE_FOR_vec_cmpv16qiv16qi },
  { 0x254040, CODE_FOR_vec_cmpv8hiv8hi },
  { 0x254242, CODE_FOR_vec_cmpv4siv4si },
  { 0x25425c, CODE_FOR_vec_cmpv4sfv4si },
  { 0x254444, CODE_FOR_vec_cmpv2div2di },
  { 0x25445e, CODE_FOR_vec_cmpv2dfv2di },
  { 0x260d0d, CODE_FOR_vec_cmpudidi },
  { 0x26373f, CODE_FOR_vec_cmpuvnx16qivnx16bi },
  { 0x263841, CODE_FOR_vec_cmpuvnx8hivnx8bi },
  { 0x263943, CODE_FOR_vec_cmpuvnx4sivnx4bi },
  { 0x263a45, CODE_FOR_vec_cmpuvnx2divnx2bi },
  { 0x263b3b, CODE_FOR_vec_cmpuv8qiv8qi },
  { 0x263c3c, CODE_FOR_vec_cmpuv4hiv4hi },
  { 0x263d3d, CODE_FOR_vec_cmpuv2siv2si },
  { 0x263e3e, CODE_FOR_vec_cmpuv16qiv16qi },
  { 0x264040, CODE_FOR_vec_cmpuv8hiv8hi },
  { 0x264242, CODE_FOR_vec_cmpuv4siv4si },
  { 0x264444, CODE_FOR_vec_cmpuv2div2di },
  { 0x28373f, CODE_FOR_maskloadvnx16qivnx16bi },
  { 0x283841, CODE_FOR_maskloadvnx8hivnx8bi },
  { 0x28385b, CODE_FOR_maskloadvnx8hfvnx8bi },
  { 0x283943, CODE_FOR_maskloadvnx4sivnx4bi },
  { 0x28395d, CODE_FOR_maskloadvnx4sfvnx4bi },
  { 0x283a45, CODE_FOR_maskloadvnx2divnx2bi },
  { 0x283a5f, CODE_FOR_maskloadvnx2dfvnx2bi },
  { 0x29373f, CODE_FOR_maskstorevnx16qivnx16bi },
  { 0x293841, CODE_FOR_maskstorevnx8hivnx8bi },
  { 0x29385b, CODE_FOR_maskstorevnx8hfvnx8bi },
  { 0x293943, CODE_FOR_maskstorevnx4sivnx4bi },
  { 0x29395d, CODE_FOR_maskstorevnx4sfvnx4bi },
  { 0x293a45, CODE_FOR_maskstorevnx2divnx2bi },
  { 0x293a5f, CODE_FOR_maskstorevnx2dfvnx2bi },
  { 0x2a0a37, CODE_FOR_vec_extractvnx16biqi },
  { 0x2a0a3b, CODE_FOR_vec_extractv8qiqi },
  { 0x2a0a3e, CODE_FOR_vec_extractv16qiqi },
  { 0x2a0a3f, CODE_FOR_vec_extractvnx16qiqi },
  { 0x2a0b38, CODE_FOR_vec_extractvnx8bihi },
  { 0x2a0b3c, CODE_FOR_vec_extractv4hihi },
  { 0x2a0b40, CODE_FOR_vec_extractv8hihi },
  { 0x2a0b41, CODE_FOR_vec_extractvnx8hihi },
  { 0x2a0c39, CODE_FOR_vec_extractvnx4bisi },
  { 0x2a0c3d, CODE_FOR_vec_extractv2sisi },
  { 0x2a0c42, CODE_FOR_vec_extractv4sisi },
  { 0x2a0c43, CODE_FOR_vec_extractvnx4sisi },
  { 0x2a0d3a, CODE_FOR_vec_extractvnx2bidi },
  { 0x2a0d44, CODE_FOR_vec_extractv2didi },
  { 0x2a0d45, CODE_FOR_vec_extractvnx2didi },
  { 0x2a2457, CODE_FOR_vec_extractv4hfhf },
  { 0x2a245a, CODE_FOR_vec_extractv8hfhf },
  { 0x2a245b, CODE_FOR_vec_extractvnx8hfhf },
  { 0x2a2558, CODE_FOR_vec_extractv2sfsf },
  { 0x2a255c, CODE_FOR_vec_extractv4sfsf },
  { 0x2a255d, CODE_FOR_vec_extractvnx4sfsf },
  { 0x2a265e, CODE_FOR_vec_extractv2dfdf },
  { 0x2a265f, CODE_FOR_vec_extractvnx2dfdf },
  { 0x2b0a3b, CODE_FOR_vec_initv8qiqi },
  { 0x2b0a3e, CODE_FOR_vec_initv16qiqi },
  { 0x2b0b3c, CODE_FOR_vec_initv4hihi },
  { 0x2b0b40, CODE_FOR_vec_initv8hihi },
  { 0x2b0c3d, CODE_FOR_vec_initv2sisi },
  { 0x2b0c42, CODE_FOR_vec_initv4sisi },
  { 0x2b0d44, CODE_FOR_vec_initv2didi },
  { 0x2b2457, CODE_FOR_vec_initv4hfhf },
  { 0x2b245a, CODE_FOR_vec_initv8hfhf },
  { 0x2b2558, CODE_FOR_vec_initv2sfsf },
  { 0x2b255c, CODE_FOR_vec_initv4sfsf },
  { 0x2b265e, CODE_FOR_vec_initv2dfdf },
  { 0x2c370c, CODE_FOR_while_ultsivnx16bi },
  { 0x2c370d, CODE_FOR_while_ultdivnx16bi },
  { 0x2c380c, CODE_FOR_while_ultsivnx8bi },
  { 0x2c380d, CODE_FOR_while_ultdivnx8bi },
  { 0x2c390c, CODE_FOR_while_ultsivnx4bi },
  { 0x2c390d, CODE_FOR_while_ultdivnx4bi },
  { 0x2c3a0c, CODE_FOR_while_ultsivnx2bi },
  { 0x2c3a0d, CODE_FOR_while_ultdivnx2bi },
  { 0x2d000c, CODE_FOR_addsi3 },
  { 0x2d000d, CODE_FOR_adddi3 },
  { 0x2d000e, CODE_FOR_addti3 },
  { 0x2d0024, CODE_FOR_addhf3 },
  { 0x2d0025, CODE_FOR_addsf3 },
  { 0x2d0026, CODE_FOR_adddf3 },
  { 0x2d003b, CODE_FOR_addv8qi3 },
  { 0x2d003c, CODE_FOR_addv4hi3 },
  { 0x2d003d, CODE_FOR_addv2si3 },
  { 0x2d003e, CODE_FOR_addv16qi3 },
  { 0x2d003f, CODE_FOR_addvnx16qi3 },
  { 0x2d0040, CODE_FOR_addv8hi3 },
  { 0x2d0041, CODE_FOR_addvnx8hi3 },
  { 0x2d0042, CODE_FOR_addv4si3 },
  { 0x2d0043, CODE_FOR_addvnx4si3 },
  { 0x2d0044, CODE_FOR_addv2di3 },
  { 0x2d0045, CODE_FOR_addvnx2di3 },
  { 0x2d0057, CODE_FOR_addv4hf3 },
  { 0x2d0058, CODE_FOR_addv2sf3 },
  { 0x2d005a, CODE_FOR_addv8hf3 },
  { 0x2d005b, CODE_FOR_addvnx8hf3 },
  { 0x2d005c, CODE_FOR_addv4sf3 },
  { 0x2d005d, CODE_FOR_addvnx4sf3 },
  { 0x2d005e, CODE_FOR_addv2df3 },
  { 0x2d005f, CODE_FOR_addvnx2df3 },
  { 0x31000c, CODE_FOR_subsi3 },
  { 0x31000d, CODE_FOR_subdi3 },
  { 0x31000e, CODE_FOR_subti3 },
  { 0x310024, CODE_FOR_subhf3 },
  { 0x310025, CODE_FOR_subsf3 },
  { 0x310026, CODE_FOR_subdf3 },
  { 0x31003b, CODE_FOR_subv8qi3 },
  { 0x31003c, CODE_FOR_subv4hi3 },
  { 0x31003d, CODE_FOR_subv2si3 },
  { 0x31003e, CODE_FOR_subv16qi3 },
  { 0x31003f, CODE_FOR_subvnx16qi3 },
  { 0x310040, CODE_FOR_subv8hi3 },
  { 0x310041, CODE_FOR_subvnx8hi3 },
  { 0x310042, CODE_FOR_subv4si3 },
  { 0x310043, CODE_FOR_subvnx4si3 },
  { 0x310044, CODE_FOR_subv2di3 },
  { 0x310045, CODE_FOR_subvnx2di3 },
  { 0x310057, CODE_FOR_subv4hf3 },
  { 0x310058, CODE_FOR_subv2sf3 },
  { 0x31005a, CODE_FOR_subv8hf3 },
  { 0x31005b, CODE_FOR_subvnx8hf3 },
  { 0x31005c, CODE_FOR_subv4sf3 },
  { 0x31005d, CODE_FOR_subvnx4sf3 },
  { 0x31005e, CODE_FOR_subv2df3 },
  { 0x31005f, CODE_FOR_subvnx2df3 },
  { 0x35000c, CODE_FOR_mulsi3 },
  { 0x35000d, CODE_FOR_muldi3 },
  { 0x35000e, CODE_FOR_multi3 },
  { 0x350024, CODE_FOR_mulhf3 },
  { 0x350025, CODE_FOR_mulsf3 },
  { 0x350026, CODE_FOR_muldf3 },
  { 0x35003b, CODE_FOR_mulv8qi3 },
  { 0x35003c, CODE_FOR_mulv4hi3 },
  { 0x35003d, CODE_FOR_mulv2si3 },
  { 0x35003e, CODE_FOR_mulv16qi3 },
  { 0x35003f, CODE_FOR_mulvnx16qi3 },
  { 0x350040, CODE_FOR_mulv8hi3 },
  { 0x350041, CODE_FOR_mulvnx8hi3 },
  { 0x350042, CODE_FOR_mulv4si3 },
  { 0x350043, CODE_FOR_mulvnx4si3 },
  { 0x350045, CODE_FOR_mulvnx2di3 },
  { 0x350057, CODE_FOR_mulv4hf3 },
  { 0x350058, CODE_FOR_mulv2sf3 },
  { 0x35005a, CODE_FOR_mulv8hf3 },
  { 0x35005b, CODE_FOR_mulvnx8hf3 },
  { 0x35005c, CODE_FOR_mulv4sf3 },
  { 0x35005d, CODE_FOR_mulvnx4sf3 },
  { 0x35005e, CODE_FOR_mulv2df3 },
  { 0x35005f, CODE_FOR_mulvnx2df3 },
  { 0x39000c, CODE_FOR_divsi3 },
  { 0x39000d, CODE_FOR_divdi3 },
  { 0x390024, CODE_FOR_divhf3 },
  { 0x390025, CODE_FOR_divsf3 },
  { 0x390026, CODE_FOR_divdf3 },
  { 0x390057, CODE_FOR_divv4hf3 },
  { 0x390058, CODE_FOR_divv2sf3 },
  { 0x39005a, CODE_FOR_divv8hf3 },
  { 0x39005b, CODE_FOR_divvnx8hf3 },
  { 0x39005c, CODE_FOR_divv4sf3 },
  { 0x39005d, CODE_FOR_divvnx4sf3 },
  { 0x39005e, CODE_FOR_divv2df3 },
  { 0x39005f, CODE_FOR_divvnx2df3 },
  { 0x3c000c, CODE_FOR_udivsi3 },
  { 0x3c000d, CODE_FOR_udivdi3 },
  { 0x40000c, CODE_FOR_modsi3 },
  { 0x40000d, CODE_FOR_moddi3 },
  { 0x420057, CODE_FOR_ftruncv4hf2 },
  { 0x420058, CODE_FOR_ftruncv2sf2 },
  { 0x42005a, CODE_FOR_ftruncv8hf2 },
  { 0x42005c, CODE_FOR_ftruncv4sf2 },
  { 0x42005e, CODE_FOR_ftruncv2df2 },
  { 0x43000c, CODE_FOR_andsi3 },
  { 0x43000d, CODE_FOR_anddi3 },
  { 0x430037, CODE_FOR_andvnx16bi3 },
  { 0x430038, CODE_FOR_andvnx8bi3 },
  { 0x430039, CODE_FOR_andvnx4bi3 },
  { 0x43003a, CODE_FOR_andvnx2bi3 },
  { 0x43003b, CODE_FOR_andv8qi3 },
  { 0x43003c, CODE_FOR_andv4hi3 },
  { 0x43003d, CODE_FOR_andv2si3 },
  { 0x43003e, CODE_FOR_andv16qi3 },
  { 0x43003f, CODE_FOR_andvnx16qi3 },
  { 0x430040, CODE_FOR_andv8hi3 },
  { 0x430041, CODE_FOR_andvnx8hi3 },
  { 0x430042, CODE_FOR_andv4si3 },
  { 0x430043, CODE_FOR_andvnx4si3 },
  { 0x430044, CODE_FOR_andv2di3 },
  { 0x430045, CODE_FOR_andvnx2di3 },
  { 0x44000c, CODE_FOR_iorsi3 },
  { 0x44000d, CODE_FOR_iordi3 },
  { 0x440037, CODE_FOR_iorvnx16bi3 },
  { 0x440038, CODE_FOR_iorvnx8bi3 },
  { 0x440039, CODE_FOR_iorvnx4bi3 },
  { 0x44003a, CODE_FOR_iorvnx2bi3 },
  { 0x44003b, CODE_FOR_iorv8qi3 },
  { 0x44003c, CODE_FOR_iorv4hi3 },
  { 0x44003d, CODE_FOR_iorv2si3 },
  { 0x44003e, CODE_FOR_iorv16qi3 },
  { 0x44003f, CODE_FOR_iorvnx16qi3 },
  { 0x440040, CODE_FOR_iorv8hi3 },
  { 0x440041, CODE_FOR_iorvnx8hi3 },
  { 0x440042, CODE_FOR_iorv4si3 },
  { 0x440043, CODE_FOR_iorvnx4si3 },
  { 0x440044, CODE_FOR_iorv2di3 },
  { 0x440045, CODE_FOR_iorvnx2di3 },
  { 0x45000c, CODE_FOR_xorsi3 },
  { 0x45000d, CODE_FOR_xordi3 },
  { 0x450037, CODE_FOR_xorvnx16bi3 },
  { 0x450038, CODE_FOR_xorvnx8bi3 },
  { 0x450039, CODE_FOR_xorvnx4bi3 },
  { 0x45003a, CODE_FOR_xorvnx2bi3 },
  { 0x45003b, CODE_FOR_xorv8qi3 },
  { 0x45003c, CODE_FOR_xorv4hi3 },
  { 0x45003d, CODE_FOR_xorv2si3 },
  { 0x45003e, CODE_FOR_xorv16qi3 },
  { 0x45003f, CODE_FOR_xorvnx16qi3 },
  { 0x450040, CODE_FOR_xorv8hi3 },
  { 0x450041, CODE_FOR_xorvnx8hi3 },
  { 0x450042, CODE_FOR_xorv4si3 },
  { 0x450043, CODE_FOR_xorvnx4si3 },
  { 0x450044, CODE_FOR_xorv2di3 },
  { 0x450045, CODE_FOR_xorvnx2di3 },
  { 0x46000a, CODE_FOR_ashlqi3 },
  { 0x46000b, CODE_FOR_ashlhi3 },
  { 0x46000c, CODE_FOR_ashlsi3 },
  { 0x46000d, CODE_FOR_ashldi3 },
  { 0x46003b, CODE_FOR_ashlv8qi3 },
  { 0x46003c, CODE_FOR_ashlv4hi3 },
  { 0x46003d, CODE_FOR_ashlv2si3 },
  { 0x46003e, CODE_FOR_ashlv16qi3 },
  { 0x46003f, CODE_FOR_ashlvnx16qi3 },
  { 0x460040, CODE_FOR_ashlv8hi3 },
  { 0x460041, CODE_FOR_ashlvnx8hi3 },
  { 0x460042, CODE_FOR_ashlv4si3 },
  { 0x460043, CODE_FOR_ashlvnx4si3 },
  { 0x460044, CODE_FOR_ashlv2di3 },
  { 0x460045, CODE_FOR_ashlvnx2di3 },
  { 0x49000c, CODE_FOR_ashrsi3 },
  { 0x49000d, CODE_FOR_ashrdi3 },
  { 0x49003b, CODE_FOR_ashrv8qi3 },
  { 0x49003c, CODE_FOR_ashrv4hi3 },
  { 0x49003d, CODE_FOR_ashrv2si3 },
  { 0x49003e, CODE_FOR_ashrv16qi3 },
  { 0x49003f, CODE_FOR_ashrvnx16qi3 },
  { 0x490040, CODE_FOR_ashrv8hi3 },
  { 0x490041, CODE_FOR_ashrvnx8hi3 },
  { 0x490042, CODE_FOR_ashrv4si3 },
  { 0x490043, CODE_FOR_ashrvnx4si3 },
  { 0x490044, CODE_FOR_ashrv2di3 },
  { 0x490045, CODE_FOR_ashrvnx2di3 },
  { 0x4a000c, CODE_FOR_lshrsi3 },
  { 0x4a000d, CODE_FOR_lshrdi3 },
  { 0x4a003b, CODE_FOR_lshrv8qi3 },
  { 0x4a003c, CODE_FOR_lshrv4hi3 },
  { 0x4a003d, CODE_FOR_lshrv2si3 },
  { 0x4a003e, CODE_FOR_lshrv16qi3 },
  { 0x4a003f, CODE_FOR_lshrvnx16qi3 },
  { 0x4a0040, CODE_FOR_lshrv8hi3 },
  { 0x4a0041, CODE_FOR_lshrvnx8hi3 },
  { 0x4a0042, CODE_FOR_lshrv4si3 },
  { 0x4a0043, CODE_FOR_lshrvnx4si3 },
  { 0x4a0044, CODE_FOR_lshrv2di3 },
  { 0x4a0045, CODE_FOR_lshrvnx2di3 },
  { 0x4b000c, CODE_FOR_rotlsi3 },
  { 0x4b000d, CODE_FOR_rotldi3 },
  { 0x4c000c, CODE_FOR_rotrsi3 },
  { 0x4c000d, CODE_FOR_rotrdi3 },
  { 0x4d003b, CODE_FOR_vashlv8qi3 },
  { 0x4d003c, CODE_FOR_vashlv4hi3 },
  { 0x4d003d, CODE_FOR_vashlv2si3 },
  { 0x4d003e, CODE_FOR_vashlv16qi3 },
  { 0x4d003f, CODE_FOR_vashlvnx16qi3 },
  { 0x4d0040, CODE_FOR_vashlv8hi3 },
  { 0x4d0041, CODE_FOR_vashlvnx8hi3 },
  { 0x4d0042, CODE_FOR_vashlv4si3 },
  { 0x4d0043, CODE_FOR_vashlvnx4si3 },
  { 0x4d0044, CODE_FOR_vashlv2di3 },
  { 0x4d0045, CODE_FOR_vashlvnx2di3 },
  { 0x4e003b, CODE_FOR_vashrv8qi3 },
  { 0x4e003c, CODE_FOR_vashrv4hi3 },
  { 0x4e003d, CODE_FOR_vashrv2si3 },
  { 0x4e003e, CODE_FOR_vashrv16qi3 },
  { 0x4e003f, CODE_FOR_vashrvnx16qi3 },
  { 0x4e0040, CODE_FOR_vashrv8hi3 },
  { 0x4e0041, CODE_FOR_vashrvnx8hi3 },
  { 0x4e0042, CODE_FOR_vashrv4si3 },
  { 0x4e0043, CODE_FOR_vashrvnx4si3 },
  { 0x4e0045, CODE_FOR_vashrvnx2di3 },
  { 0x4f003b, CODE_FOR_vlshrv8qi3 },
  { 0x4f003c, CODE_FOR_vlshrv4hi3 },
  { 0x4f003d, CODE_FOR_vlshrv2si3 },
  { 0x4f003e, CODE_FOR_vlshrv16qi3 },
  { 0x4f003f, CODE_FOR_vlshrvnx16qi3 },
  { 0x4f0040, CODE_FOR_vlshrv8hi3 },
  { 0x4f0041, CODE_FOR_vlshrvnx8hi3 },
  { 0x4f0042, CODE_FOR_vlshrv4si3 },
  { 0x4f0043, CODE_FOR_vlshrvnx4si3 },
  { 0x4f0045, CODE_FOR_vlshrvnx2di3 },
  { 0x520025, CODE_FOR_sminsf3 },
  { 0x520026, CODE_FOR_smindf3 },
  { 0x52003b, CODE_FOR_sminv8qi3 },
  { 0x52003c, CODE_FOR_sminv4hi3 },
  { 0x52003d, CODE_FOR_sminv2si3 },
  { 0x52003e, CODE_FOR_sminv16qi3 },
  { 0x52003f, CODE_FOR_sminvnx16qi3 },
  { 0x520040, CODE_FOR_sminv8hi3 },
  { 0x520041, CODE_FOR_sminvnx8hi3 },
  { 0x520042, CODE_FOR_sminv4si3 },
  { 0x520043, CODE_FOR_sminvnx4si3 },
  { 0x520044, CODE_FOR_sminv2di3 },
  { 0x520045, CODE_FOR_sminvnx2di3 },
  { 0x520057, CODE_FOR_sminv4hf3 },
  { 0x520058, CODE_FOR_sminv2sf3 },
  { 0x52005a, CODE_FOR_sminv8hf3 },
  { 0x52005b, CODE_FOR_sminvnx8hf3 },
  { 0x52005c, CODE_FOR_sminv4sf3 },
  { 0x52005d, CODE_FOR_sminvnx4sf3 },
  { 0x52005e, CODE_FOR_sminv2df3 },
  { 0x52005f, CODE_FOR_sminvnx2df3 },
  { 0x530025, CODE_FOR_smaxsf3 },
  { 0x530026, CODE_FOR_smaxdf3 },
  { 0x53003b, CODE_FOR_smaxv8qi3 },
  { 0x53003c, CODE_FOR_smaxv4hi3 },
  { 0x53003d, CODE_FOR_smaxv2si3 },
  { 0x53003e, CODE_FOR_smaxv16qi3 },
  { 0x53003f, CODE_FOR_smaxvnx16qi3 },
  { 0x530040, CODE_FOR_smaxv8hi3 },
  { 0x530041, CODE_FOR_smaxvnx8hi3 },
  { 0x530042, CODE_FOR_smaxv4si3 },
  { 0x530043, CODE_FOR_smaxvnx4si3 },
  { 0x530044, CODE_FOR_smaxv2di3 },
  { 0x530045, CODE_FOR_smaxvnx2di3 },
  { 0x530057, CODE_FOR_smaxv4hf3 },
  { 0x530058, CODE_FOR_smaxv2sf3 },
  { 0x53005a, CODE_FOR_smaxv8hf3 },
  { 0x53005b, CODE_FOR_smaxvnx8hf3 },
  { 0x53005c, CODE_FOR_smaxv4sf3 },
  { 0x53005d, CODE_FOR_smaxvnx4sf3 },
  { 0x53005e, CODE_FOR_smaxv2df3 },
  { 0x53005f, CODE_FOR_smaxvnx2df3 },
  { 0x54003b, CODE_FOR_uminv8qi3 },
  { 0x54003c, CODE_FOR_uminv4hi3 },
  { 0x54003d, CODE_FOR_uminv2si3 },
  { 0x54003e, CODE_FOR_uminv16qi3 },
  { 0x54003f, CODE_FOR_uminvnx16qi3 },
  { 0x540040, CODE_FOR_uminv8hi3 },
  { 0x540041, CODE_FOR_uminvnx8hi3 },
  { 0x540042, CODE_FOR_uminv4si3 },
  { 0x540043, CODE_FOR_uminvnx4si3 },
  { 0x540044, CODE_FOR_uminv2di3 },
  { 0x540045, CODE_FOR_uminvnx2di3 },
  { 0x55000c, CODE_FOR_umaxsi3 },
  { 0x55000d, CODE_FOR_umaxdi3 },
  { 0x55003b, CODE_FOR_umaxv8qi3 },
  { 0x55003c, CODE_FOR_umaxv4hi3 },
  { 0x55003d, CODE_FOR_umaxv2si3 },
  { 0x55003e, CODE_FOR_umaxv16qi3 },
  { 0x55003f, CODE_FOR_umaxvnx16qi3 },
  { 0x550040, CODE_FOR_umaxv8hi3 },
  { 0x550041, CODE_FOR_umaxvnx8hi3 },
  { 0x550042, CODE_FOR_umaxv4si3 },
  { 0x550043, CODE_FOR_umaxvnx4si3 },
  { 0x550044, CODE_FOR_umaxv2di3 },
  { 0x550045, CODE_FOR_umaxvnx2di3 },
  { 0x56000c, CODE_FOR_negsi2 },
  { 0x56000d, CODE_FOR_negdi2 },
  { 0x560024, CODE_FOR_neghf2 },
  { 0x560025, CODE_FOR_negsf2 },
  { 0x560026, CODE_FOR_negdf2 },
  { 0x56003b, CODE_FOR_negv8qi2 },
  { 0x56003c, CODE_FOR_negv4hi2 },
  { 0x56003d, CODE_FOR_negv2si2 },
  { 0x56003e, CODE_FOR_negv16qi2 },
  { 0x56003f, CODE_FOR_negvnx16qi2 },
  { 0x560040, CODE_FOR_negv8hi2 },
  { 0x560041, CODE_FOR_negvnx8hi2 },
  { 0x560042, CODE_FOR_negv4si2 },
  { 0x560043, CODE_FOR_negvnx4si2 },
  { 0x560044, CODE_FOR_negv2di2 },
  { 0x560045, CODE_FOR_negvnx2di2 },
  { 0x560057, CODE_FOR_negv4hf2 },
  { 0x560058, CODE_FOR_negv2sf2 },
  { 0x56005a, CODE_FOR_negv8hf2 },
  { 0x56005b, CODE_FOR_negvnx8hf2 },
  { 0x56005c, CODE_FOR_negv4sf2 },
  { 0x56005d, CODE_FOR_negvnx4sf2 },
  { 0x56005e, CODE_FOR_negv2df2 },
  { 0x56005f, CODE_FOR_negvnx2df2 },
  { 0x5a000c, CODE_FOR_abssi2 },
  { 0x5a000d, CODE_FOR_absdi2 },
  { 0x5a0024, CODE_FOR_abshf2 },
  { 0x5a0025, CODE_FOR_abssf2 },
  { 0x5a0026, CODE_FOR_absdf2 },
  { 0x5a003b, CODE_FOR_absv8qi2 },
  { 0x5a003c, CODE_FOR_absv4hi2 },
  { 0x5a003d, CODE_FOR_absv2si2 },
  { 0x5a003e, CODE_FOR_absv16qi2 },
  { 0x5a0040, CODE_FOR_absv8hi2 },
  { 0x5a0042, CODE_FOR_absv4si2 },
  { 0x5a0044, CODE_FOR_absv2di2 },
  { 0x5a0057, CODE_FOR_absv4hf2 },
  { 0x5a0058, CODE_FOR_absv2sf2 },
  { 0x5a005a, CODE_FOR_absv8hf2 },
  { 0x5a005b, CODE_FOR_absvnx8hf2 },
  { 0x5a005c, CODE_FOR_absv4sf2 },
  { 0x5a005d, CODE_FOR_absvnx4sf2 },
  { 0x5a005e, CODE_FOR_absv2df2 },
  { 0x5a005f, CODE_FOR_absvnx2df2 },
  { 0x5c000c, CODE_FOR_one_cmplsi2 },
  { 0x5c000d, CODE_FOR_one_cmpldi2 },
  { 0x5c0037, CODE_FOR_one_cmplvnx16bi2 },
  { 0x5c0038, CODE_FOR_one_cmplvnx8bi2 },
  { 0x5c0039, CODE_FOR_one_cmplvnx4bi2 },
  { 0x5c003a, CODE_FOR_one_cmplvnx2bi2 },
  { 0x5c003b, CODE_FOR_one_cmplv8qi2 },
  { 0x5c003c, CODE_FOR_one_cmplv4hi2 },
  { 0x5c003d, CODE_FOR_one_cmplv2si2 },
  { 0x5c003e, CODE_FOR_one_cmplv16qi2 },
  { 0x5c003f, CODE_FOR_one_cmplvnx16qi2 },
  { 0x5c0040, CODE_FOR_one_cmplv8hi2 },
  { 0x5c0041, CODE_FOR_one_cmplvnx8hi2 },
  { 0x5c0042, CODE_FOR_one_cmplv4si2 },
  { 0x5c0043, CODE_FOR_one_cmplvnx4si2 },
  { 0x5c0044, CODE_FOR_one_cmplv2di2 },
  { 0x5c0045, CODE_FOR_one_cmplvnx2di2 },
  { 0x5d000b, CODE_FOR_bswaphi2 },
  { 0x5d000c, CODE_FOR_bswapsi2 },
  { 0x5d000d, CODE_FOR_bswapdi2 },
  { 0x5d003c, CODE_FOR_bswapv4hi2 },
  { 0x5d003d, CODE_FOR_bswapv2si2 },
  { 0x5d0040, CODE_FOR_bswapv8hi2 },
  { 0x5d0042, CODE_FOR_bswapv4si2 },
  { 0x5d0044, CODE_FOR_bswapv2di2 },
  { 0x5e000c, CODE_FOR_ffssi2 },
  { 0x5e000d, CODE_FOR_ffsdi2 },
  { 0x5f000c, CODE_FOR_clzsi2 },
  { 0x5f000d, CODE_FOR_clzdi2 },
  { 0x5f003b, CODE_FOR_clzv8qi2 },
  { 0x5f003c, CODE_FOR_clzv4hi2 },
  { 0x5f003d, CODE_FOR_clzv2si2 },
  { 0x5f003e, CODE_FOR_clzv16qi2 },
  { 0x5f0040, CODE_FOR_clzv8hi2 },
  { 0x5f0042, CODE_FOR_clzv4si2 },
  { 0x60000c, CODE_FOR_ctzsi2 },
  { 0x60000d, CODE_FOR_ctzdi2 },
  { 0x60003d, CODE_FOR_ctzv2si2 },
  { 0x600042, CODE_FOR_ctzv4si2 },
  { 0x61000c, CODE_FOR_clrsbsi2 },
  { 0x61000d, CODE_FOR_clrsbdi2 },
  { 0x61003b, CODE_FOR_clrsbv8qi2 },
  { 0x61003c, CODE_FOR_clrsbv4hi2 },
  { 0x61003d, CODE_FOR_clrsbv2si2 },
  { 0x61003e, CODE_FOR_clrsbv16qi2 },
  { 0x610040, CODE_FOR_clrsbv8hi2 },
  { 0x610042, CODE_FOR_clrsbv4si2 },
  { 0x62000c, CODE_FOR_popcountsi2 },
  { 0x62000d, CODE_FOR_popcountdi2 },
  { 0x62003b, CODE_FOR_popcountv8qi2 },
  { 0x62003e, CODE_FOR_popcountv16qi2 },
  { 0x62003f, CODE_FOR_popcountvnx16qi2 },
  { 0x620041, CODE_FOR_popcountvnx8hi2 },
  { 0x620043, CODE_FOR_popcountvnx4si2 },
  { 0x620045, CODE_FOR_popcountvnx2di2 },
  { 0x6e0024, CODE_FOR_sqrthf2 },
  { 0x6e0025, CODE_FOR_sqrtsf2 },
  { 0x6e0026, CODE_FOR_sqrtdf2 },
  { 0x6e0057, CODE_FOR_sqrtv4hf2 },
  { 0x6e0058, CODE_FOR_sqrtv2sf2 },
  { 0x6e005a, CODE_FOR_sqrtv8hf2 },
  { 0x6e005b, CODE_FOR_sqrtvnx8hf2 },
  { 0x6e005c, CODE_FOR_sqrtv4sf2 },
  { 0x6e005d, CODE_FOR_sqrtvnx4sf2 },
  { 0x6e005e, CODE_FOR_sqrtv2df2 },
  { 0x6e005f, CODE_FOR_sqrtvnx2df2 },
  { 0x7d000a, CODE_FOR_movqi },
  { 0x7d000b, CODE_FOR_movhi },
  { 0x7d000c, CODE_FOR_movsi },
  { 0x7d000d, CODE_FOR_movdi },
  { 0x7d000e, CODE_FOR_movti },
  { 0x7d000f, CODE_FOR_movoi },
  { 0x7d0010, CODE_FOR_movci },
  { 0x7d0011, CODE_FOR_movxi },
  { 0x7d0024, CODE_FOR_movhf },
  { 0x7d0025, CODE_FOR_movsf },
  { 0x7d0026, CODE_FOR_movdf },
  { 0x7d0027, CODE_FOR_movtf },
  { 0x7d0037, CODE_FOR_movvnx16bi },
  { 0x7d0038, CODE_FOR_movvnx8bi },
  { 0x7d0039, CODE_FOR_movvnx4bi },
  { 0x7d003a, CODE_FOR_movvnx2bi },
  { 0x7d003b, CODE_FOR_movv8qi },
  { 0x7d003c, CODE_FOR_movv4hi },
  { 0x7d003d, CODE_FOR_movv2si },
  { 0x7d003e, CODE_FOR_movv16qi },
  { 0x7d003f, CODE_FOR_movvnx16qi },
  { 0x7d0040, CODE_FOR_movv8hi },
  { 0x7d0041, CODE_FOR_movvnx8hi },
  { 0x7d0042, CODE_FOR_movv4si },
  { 0x7d0043, CODE_FOR_movvnx4si },
  { 0x7d0044, CODE_FOR_movv2di },
  { 0x7d0045, CODE_FOR_movvnx2di },
  { 0x7d0046, CODE_FOR_movvnx32qi },
  { 0x7d0047, CODE_FOR_movvnx16hi },
  { 0x7d0048, CODE_FOR_movvnx8si },
  { 0x7d0049, CODE_FOR_movvnx4di },
  { 0x7d004b, CODE_FOR_movvnx48qi },
  { 0x7d004c, CODE_FOR_movvnx24hi },
  { 0x7d004d, CODE_FOR_movvnx12si },
  { 0x7d004e, CODE_FOR_movvnx6di },
  { 0x7d0050, CODE_FOR_movvnx64qi },
  { 0x7d0051, CODE_FOR_movvnx32hi },
  { 0x7d0052, CODE_FOR_movvnx16si },
  { 0x7d0053, CODE_FOR_movvnx8di },
  { 0x7d0057, CODE_FOR_movv4hf },
  { 0x7d0058, CODE_FOR_movv2sf },
  { 0x7d005a, CODE_FOR_movv8hf },
  { 0x7d005b, CODE_FOR_movvnx8hf },
  { 0x7d005c, CODE_FOR_movv4sf },
  { 0x7d005d, CODE_FOR_movvnx4sf },
  { 0x7d005e, CODE_FOR_movv2df },
  { 0x7d005f, CODE_FOR_movvnx2df },
  { 0x7d0060, CODE_FOR_movvnx16hf },
  { 0x7d0061, CODE_FOR_movvnx8sf },
  { 0x7d0062, CODE_FOR_movvnx4df },
  { 0x7d0063, CODE_FOR_movvnx24hf },
  { 0x7d0064, CODE_FOR_movvnx12sf },
  { 0x7d0065, CODE_FOR_movvnx6df },
  { 0x7d0066, CODE_FOR_movvnx32hf },
  { 0x7d0067, CODE_FOR_movvnx16sf },
  { 0x7d0068, CODE_FOR_movvnx8df },
  { 0x7f003b, CODE_FOR_movmisalignv8qi },
  { 0x7f003c, CODE_FOR_movmisalignv4hi },
  { 0x7f003d, CODE_FOR_movmisalignv2si },
  { 0x7f003e, CODE_FOR_movmisalignv16qi },
  { 0x7f003f, CODE_FOR_movmisalignvnx16qi },
  { 0x7f0040, CODE_FOR_movmisalignv8hi },
  { 0x7f0041, CODE_FOR_movmisalignvnx8hi },
  { 0x7f0042, CODE_FOR_movmisalignv4si },
  { 0x7f0043, CODE_FOR_movmisalignvnx4si },
  { 0x7f0044, CODE_FOR_movmisalignv2di },
  { 0x7f0045, CODE_FOR_movmisalignvnx2di },
  { 0x7f0058, CODE_FOR_movmisalignv2sf },
  { 0x7f005b, CODE_FOR_movmisalignvnx8hf },
  { 0x7f005c, CODE_FOR_movmisalignv4sf },
  { 0x7f005d, CODE_FOR_movmisalignvnx4sf },
  { 0x7f005e, CODE_FOR_movmisalignv2df },
  { 0x7f005f, CODE_FOR_movmisalignvnx2df },
  { 0x81000c, CODE_FOR_insvsi },
  { 0x81000d, CODE_FOR_insvdi },
  { 0x8a0002, CODE_FOR_cbranchcc4 },
  { 0x8a000c, CODE_FOR_cbranchsi4 },
  { 0x8a000d, CODE_FOR_cbranchdi4 },
  { 0x8a0025, CODE_FOR_cbranchsf4 },
  { 0x8a0026, CODE_FOR_cbranchdf4 },
  { 0x8a0037, CODE_FOR_cbranchvnx16bi4 },
  { 0x8a0038, CODE_FOR_cbranchvnx8bi4 },
  { 0x8a0039, CODE_FOR_cbranchvnx4bi4 },
  { 0x8a003a, CODE_FOR_cbranchvnx2bi4 },
  { 0x8c000c, CODE_FOR_negsicc },
  { 0x8c000d, CODE_FOR_negdicc },
  { 0x8d000c, CODE_FOR_notsicc },
  { 0x8d000d, CODE_FOR_notdicc },
  { 0x8e000a, CODE_FOR_movqicc },
  { 0x8e000b, CODE_FOR_movhicc },
  { 0x8e000c, CODE_FOR_movsicc },
  { 0x8e000d, CODE_FOR_movdicc },
  { 0x8e0025, CODE_FOR_movsfcc },
  { 0x8e0026, CODE_FOR_movdfcc },
  { 0x8f003f, CODE_FOR_cond_addvnx16qi },
  { 0x8f0041, CODE_FOR_cond_addvnx8hi },
  { 0x8f0043, CODE_FOR_cond_addvnx4si },
  { 0x8f0045, CODE_FOR_cond_addvnx2di },
  { 0x8f005b, CODE_FOR_cond_addvnx8hf },
  { 0x8f005d, CODE_FOR_cond_addvnx4sf },
  { 0x8f005f, CODE_FOR_cond_addvnx2df },
  { 0x90003f, CODE_FOR_cond_subvnx16qi },
  { 0x900041, CODE_FOR_cond_subvnx8hi },
  { 0x900043, CODE_FOR_cond_subvnx4si },
  { 0x900045, CODE_FOR_cond_subvnx2di },
  { 0x90005b, CODE_FOR_cond_subvnx8hf },
  { 0x90005d, CODE_FOR_cond_subvnx4sf },
  { 0x90005f, CODE_FOR_cond_subvnx2df },
  { 0x91003f, CODE_FOR_cond_andvnx16qi },
  { 0x910041, CODE_FOR_cond_andvnx8hi },
  { 0x910043, CODE_FOR_cond_andvnx4si },
  { 0x910045, CODE_FOR_cond_andvnx2di },
  { 0x92003f, CODE_FOR_cond_iorvnx16qi },
  { 0x920041, CODE_FOR_cond_iorvnx8hi },
  { 0x920043, CODE_FOR_cond_iorvnx4si },
  { 0x920045, CODE_FOR_cond_iorvnx2di },
  { 0x93003f, CODE_FOR_cond_xorvnx16qi },
  { 0x930041, CODE_FOR_cond_xorvnx8hi },
  { 0x930043, CODE_FOR_cond_xorvnx4si },
  { 0x930045, CODE_FOR_cond_xorvnx2di },
  { 0x94003f, CODE_FOR_cond_sminvnx16qi },
  { 0x940041, CODE_FOR_cond_sminvnx8hi },
  { 0x940043, CODE_FOR_cond_sminvnx4si },
  { 0x940045, CODE_FOR_cond_sminvnx2di },
  { 0x95003f, CODE_FOR_cond_smaxvnx16qi },
  { 0x950041, CODE_FOR_cond_smaxvnx8hi },
  { 0x950043, CODE_FOR_cond_smaxvnx4si },
  { 0x950045, CODE_FOR_cond_smaxvnx2di },
  { 0x96003f, CODE_FOR_cond_uminvnx16qi },
  { 0x960041, CODE_FOR_cond_uminvnx8hi },
  { 0x960043, CODE_FOR_cond_uminvnx4si },
  { 0x960045, CODE_FOR_cond_uminvnx2di },
  { 0x97003f, CODE_FOR_cond_umaxvnx16qi },
  { 0x970041, CODE_FOR_cond_umaxvnx8hi },
  { 0x970043, CODE_FOR_cond_umaxvnx4si },
  { 0x970045, CODE_FOR_cond_umaxvnx2di },
  { 0x98000c, CODE_FOR_cmovsi6 },
  { 0x98000d, CODE_FOR_cmovdi6 },
  { 0x980025, CODE_FOR_cmovsf6 },
  { 0x980026, CODE_FOR_cmovdf6 },
  { 0x990002, CODE_FOR_cstorecc4 },
  { 0x99000c, CODE_FOR_cstoresi4 },
  { 0x99000d, CODE_FOR_cstoredi4 },
  { 0x990025, CODE_FOR_cstoresf4 },
  { 0x990026, CODE_FOR_cstoredf4 },
  { 0xa3000d, CODE_FOR_smuldi3_highpart },
  { 0xa3003f, CODE_FOR_smulvnx16qi3_highpart },
  { 0xa30041, CODE_FOR_smulvnx8hi3_highpart },
  { 0xa30043, CODE_FOR_smulvnx4si3_highpart },
  { 0xa30045, CODE_FOR_smulvnx2di3_highpart },
  { 0xa4000d, CODE_FOR_umuldi3_highpart },
  { 0xa4003f, CODE_FOR_umulvnx16qi3_highpart },
  { 0xa40041, CODE_FOR_umulvnx8hi3_highpart },
  { 0xa40043, CODE_FOR_umulvnx4si3_highpart },
  { 0xa40045, CODE_FOR_umulvnx2di3_highpart },
  { 0xa8000d, CODE_FOR_movmemdi },
  { 0xab0024, CODE_FOR_fmahf4 },
  { 0xab0025, CODE_FOR_fmasf4 },
  { 0xab0026, CODE_FOR_fmadf4 },
  { 0xab0057, CODE_FOR_fmav4hf4 },
  { 0xab0058, CODE_FOR_fmav2sf4 },
  { 0xab005a, CODE_FOR_fmav8hf4 },
  { 0xab005b, CODE_FOR_fmavnx8hf4 },
  { 0xab005c, CODE_FOR_fmav4sf4 },
  { 0xab005d, CODE_FOR_fmavnx4sf4 },
  { 0xab005e, CODE_FOR_fmav2df4 },
  { 0xab005f, CODE_FOR_fmavnx2df4 },
  { 0xac0025, CODE_FOR_fmssf4 },
  { 0xac0026, CODE_FOR_fmsdf4 },
  { 0xac005b, CODE_FOR_fmsvnx8hf4 },
  { 0xac005d, CODE_FOR_fmsvnx4sf4 },
  { 0xac005f, CODE_FOR_fmsvnx2df4 },
  { 0xad0024, CODE_FOR_fnmahf4 },
  { 0xad0025, CODE_FOR_fnmasf4 },
  { 0xad0026, CODE_FOR_fnmadf4 },
  { 0xad0057, CODE_FOR_fnmav4hf4 },
  { 0xad0058, CODE_FOR_fnmav2sf4 },
  { 0xad005a, CODE_FOR_fnmav8hf4 },
  { 0xad005b, CODE_FOR_fnmavnx8hf4 },
  { 0xad005c, CODE_FOR_fnmav4sf4 },
  { 0xad005d, CODE_FOR_fnmavnx4sf4 },
  { 0xad005e, CODE_FOR_fnmav2df4 },
  { 0xad005f, CODE_FOR_fnmavnx2df4 },
  { 0xae0025, CODE_FOR_fnmssf4 },
  { 0xae0026, CODE_FOR_fnmsdf4 },
  { 0xae005b, CODE_FOR_fnmsvnx8hf4 },
  { 0xae005d, CODE_FOR_fnmsvnx4sf4 },
  { 0xae005f, CODE_FOR_fnmsvnx2df4 },
  { 0xaf0024, CODE_FOR_rinthf2 },
  { 0xaf0025, CODE_FOR_rintsf2 },
  { 0xaf0026, CODE_FOR_rintdf2 },
  { 0xaf0057, CODE_FOR_rintv4hf2 },
  { 0xaf0058, CODE_FOR_rintv2sf2 },
  { 0xaf005a, CODE_FOR_rintv8hf2 },
  { 0xaf005b, CODE_FOR_rintvnx8hf2 },
  { 0xaf005c, CODE_FOR_rintv4sf2 },
  { 0xaf005d, CODE_FOR_rintvnx4sf2 },
  { 0xaf005e, CODE_FOR_rintv2df2 },
  { 0xaf005f, CODE_FOR_rintvnx2df2 },
  { 0xb00024, CODE_FOR_roundhf2 },
  { 0xb00025, CODE_FOR_roundsf2 },
  { 0xb00026, CODE_FOR_rounddf2 },
  { 0xb00057, CODE_FOR_roundv4hf2 },
  { 0xb00058, CODE_FOR_roundv2sf2 },
  { 0xb0005a, CODE_FOR_roundv8hf2 },
  { 0xb0005b, CODE_FOR_roundvnx8hf2 },
  { 0xb0005c, CODE_FOR_roundv4sf2 },
  { 0xb0005d, CODE_FOR_roundvnx4sf2 },
  { 0xb0005e, CODE_FOR_roundv2df2 },
  { 0xb0005f, CODE_FOR_roundvnx2df2 },
  { 0xb10024, CODE_FOR_floorhf2 },
  { 0xb10025, CODE_FOR_floorsf2 },
  { 0xb10026, CODE_FOR_floordf2 },
  { 0xb10057, CODE_FOR_floorv4hf2 },
  { 0xb10058, CODE_FOR_floorv2sf2 },
  { 0xb1005a, CODE_FOR_floorv8hf2 },
  { 0xb1005b, CODE_FOR_floorvnx8hf2 },
  { 0xb1005c, CODE_FOR_floorv4sf2 },
  { 0xb1005d, CODE_FOR_floorvnx4sf2 },
  { 0xb1005e, CODE_FOR_floorv2df2 },
  { 0xb1005f, CODE_FOR_floorvnx2df2 },
  { 0xb20024, CODE_FOR_ceilhf2 },
  { 0xb20025, CODE_FOR_ceilsf2 },
  { 0xb20026, CODE_FOR_ceildf2 },
  { 0xb20057, CODE_FOR_ceilv4hf2 },
  { 0xb20058, CODE_FOR_ceilv2sf2 },
  { 0xb2005a, CODE_FOR_ceilv8hf2 },
  { 0xb2005b, CODE_FOR_ceilvnx8hf2 },
  { 0xb2005c, CODE_FOR_ceilv4sf2 },
  { 0xb2005d, CODE_FOR_ceilvnx4sf2 },
  { 0xb2005e, CODE_FOR_ceilv2df2 },
  { 0xb2005f, CODE_FOR_ceilvnx2df2 },
  { 0xb30024, CODE_FOR_btrunchf2 },
  { 0xb30025, CODE_FOR_btruncsf2 },
  { 0xb30026, CODE_FOR_btruncdf2 },
  { 0xb30057, CODE_FOR_btruncv4hf2 },
  { 0xb30058, CODE_FOR_btruncv2sf2 },
  { 0xb3005a, CODE_FOR_btruncv8hf2 },
  { 0xb3005b, CODE_FOR_btruncvnx8hf2 },
  { 0xb3005c, CODE_FOR_btruncv4sf2 },
  { 0xb3005d, CODE_FOR_btruncvnx4sf2 },
  { 0xb3005e, CODE_FOR_btruncv2df2 },
  { 0xb3005f, CODE_FOR_btruncvnx2df2 },
  { 0xb40024, CODE_FOR_nearbyinthf2 },
  { 0xb40025, CODE_FOR_nearbyintsf2 },
  { 0xb40026, CODE_FOR_nearbyintdf2 },
  { 0xb40057, CODE_FOR_nearbyintv4hf2 },
  { 0xb40058, CODE_FOR_nearbyintv2sf2 },
  { 0xb4005a, CODE_FOR_nearbyintv8hf2 },
  { 0xb4005b, CODE_FOR_nearbyintvnx8hf2 },
  { 0xb4005c, CODE_FOR_nearbyintv4sf2 },
  { 0xb4005d, CODE_FOR_nearbyintvnx4sf2 },
  { 0xb4005e, CODE_FOR_nearbyintv2df2 },
  { 0xb4005f, CODE_FOR_nearbyintvnx2df2 },
  { 0xb90025, CODE_FOR_copysignsf3 },
  { 0xb90026, CODE_FOR_copysigndf3 },
  { 0xb90057, CODE_FOR_copysignv4hf3 },
  { 0xb90058, CODE_FOR_copysignv2sf3 },
  { 0xb9005a, CODE_FOR_copysignv8hf3 },
  { 0xb9005c, CODE_FOR_copysignv4sf3 },
  { 0xb9005e, CODE_FOR_copysignv2df3 },
  { 0xba0025, CODE_FOR_xorsignsf3 },
  { 0xba0026, CODE_FOR_xorsigndf3 },
  { 0xba0057, CODE_FOR_xorsignv4hf3 },
  { 0xba0058, CODE_FOR_xorsignv2sf3 },
  { 0xba005a, CODE_FOR_xorsignv8hf3 },
  { 0xba005c, CODE_FOR_xorsignv4sf3 },
  { 0xba005e, CODE_FOR_xorsignv2df3 },
  { 0xcb0025, CODE_FOR_rsqrtsf2 },
  { 0xcb0026, CODE_FOR_rsqrtdf2 },
  { 0xcb0058, CODE_FOR_rsqrtv2sf2 },
  { 0xcb005c, CODE_FOR_rsqrtv4sf2 },
  { 0xcb005e, CODE_FOR_rsqrtv2df2 },
  { 0xd20024, CODE_FOR_fmaxhf3 },
  { 0xd20025, CODE_FOR_fmaxsf3 },
  { 0xd20026, CODE_FOR_fmaxdf3 },
  { 0xd20057, CODE_FOR_fmaxv4hf3 },
  { 0xd20058, CODE_FOR_fmaxv2sf3 },
  { 0xd2005a, CODE_FOR_fmaxv8hf3 },
  { 0xd2005b, CODE_FOR_fmaxvnx8hf3 },
  { 0xd2005c, CODE_FOR_fmaxv4sf3 },
  { 0xd2005d, CODE_FOR_fmaxvnx4sf3 },
  { 0xd2005e, CODE_FOR_fmaxv2df3 },
  { 0xd2005f, CODE_FOR_fmaxvnx2df3 },
  { 0xd30024, CODE_FOR_fminhf3 },
  { 0xd30025, CODE_FOR_fminsf3 },
  { 0xd30026, CODE_FOR_fmindf3 },
  { 0xd30057, CODE_FOR_fminv4hf3 },
  { 0xd30058, CODE_FOR_fminv2sf3 },
  { 0xd3005a, CODE_FOR_fminv8hf3 },
  { 0xd3005b, CODE_FOR_fminvnx8hf3 },
  { 0xd3005c, CODE_FOR_fminv4sf3 },
  { 0xd3005d, CODE_FOR_fminvnx4sf3 },
  { 0xd3005e, CODE_FOR_fminv2df3 },
  { 0xd3005f, CODE_FOR_fminvnx2df3 },
  { 0xd4003b, CODE_FOR_reduc_smax_scal_v8qi },
  { 0xd4003c, CODE_FOR_reduc_smax_scal_v4hi },
  { 0xd4003d, CODE_FOR_reduc_smax_scal_v2si },
  { 0xd4003e, CODE_FOR_reduc_smax_scal_v16qi },
  { 0xd4003f, CODE_FOR_reduc_smax_scal_vnx16qi },
  { 0xd40040, CODE_FOR_reduc_smax_scal_v8hi },
  { 0xd40041, CODE_FOR_reduc_smax_scal_vnx8hi },
  { 0xd40042, CODE_FOR_reduc_smax_scal_v4si },
  { 0xd40043, CODE_FOR_reduc_smax_scal_vnx4si },
  { 0xd40045, CODE_FOR_reduc_smax_scal_vnx2di },
  { 0xd40057, CODE_FOR_reduc_smax_scal_v4hf },
  { 0xd40058, CODE_FOR_reduc_smax_scal_v2sf },
  { 0xd4005a, CODE_FOR_reduc_smax_scal_v8hf },
  { 0xd4005b, CODE_FOR_reduc_smax_scal_vnx8hf },
  { 0xd4005c, CODE_FOR_reduc_smax_scal_v4sf },
  { 0xd4005d, CODE_FOR_reduc_smax_scal_vnx4sf },
  { 0xd4005e, CODE_FOR_reduc_smax_scal_v2df },
  { 0xd4005f, CODE_FOR_reduc_smax_scal_vnx2df },
  { 0xd5003b, CODE_FOR_reduc_smin_scal_v8qi },
  { 0xd5003c, CODE_FOR_reduc_smin_scal_v4hi },
  { 0xd5003d, CODE_FOR_reduc_smin_scal_v2si },
  { 0xd5003e, CODE_FOR_reduc_smin_scal_v16qi },
  { 0xd5003f, CODE_FOR_reduc_smin_scal_vnx16qi },
  { 0xd50040, CODE_FOR_reduc_smin_scal_v8hi },
  { 0xd50041, CODE_FOR_reduc_smin_scal_vnx8hi },
  { 0xd50042, CODE_FOR_reduc_smin_scal_v4si },
  { 0xd50043, CODE_FOR_reduc_smin_scal_vnx4si },
  { 0xd50045, CODE_FOR_reduc_smin_scal_vnx2di },
  { 0xd50057, CODE_FOR_reduc_smin_scal_v4hf },
  { 0xd50058, CODE_FOR_reduc_smin_scal_v2sf },
  { 0xd5005a, CODE_FOR_reduc_smin_scal_v8hf },
  { 0xd5005b, CODE_FOR_reduc_smin_scal_vnx8hf },
  { 0xd5005c, CODE_FOR_reduc_smin_scal_v4sf },
  { 0xd5005d, CODE_FOR_reduc_smin_scal_vnx4sf },
  { 0xd5005e, CODE_FOR_reduc_smin_scal_v2df },
  { 0xd5005f, CODE_FOR_reduc_smin_scal_vnx2df },
  { 0xd6003b, CODE_FOR_reduc_plus_scal_v8qi },
  { 0xd6003c, CODE_FOR_reduc_plus_scal_v4hi },
  { 0xd6003d, CODE_FOR_reduc_plus_scal_v2si },
  { 0xd6003e, CODE_FOR_reduc_plus_scal_v16qi },
  { 0xd6003f, CODE_FOR_reduc_plus_scal_vnx16qi },
  { 0xd60040, CODE_FOR_reduc_plus_scal_v8hi },
  { 0xd60041, CODE_FOR_reduc_plus_scal_vnx8hi },
  { 0xd60042, CODE_FOR_reduc_plus_scal_v4si },
  { 0xd60043, CODE_FOR_reduc_plus_scal_vnx4si },
  { 0xd60044, CODE_FOR_reduc_plus_scal_v2di },
  { 0xd60045, CODE_FOR_reduc_plus_scal_vnx2di },
  { 0xd60058, CODE_FOR_reduc_plus_scal_v2sf },
  { 0xd6005b, CODE_FOR_reduc_plus_scal_vnx8hf },
  { 0xd6005c, CODE_FOR_reduc_plus_scal_v4sf },
  { 0xd6005d, CODE_FOR_reduc_plus_scal_vnx4sf },
  { 0xd6005e, CODE_FOR_reduc_plus_scal_v2df },
  { 0xd6005f, CODE_FOR_reduc_plus_scal_vnx2df },
  { 0xd7003b, CODE_FOR_reduc_umax_scal_v8qi },
  { 0xd7003c, CODE_FOR_reduc_umax_scal_v4hi },
  { 0xd7003d, CODE_FOR_reduc_umax_scal_v2si },
  { 0xd7003e, CODE_FOR_reduc_umax_scal_v16qi },
  { 0xd7003f, CODE_FOR_reduc_umax_scal_vnx16qi },
  { 0xd70040, CODE_FOR_reduc_umax_scal_v8hi },
  { 0xd70041, CODE_FOR_reduc_umax_scal_vnx8hi },
  { 0xd70042, CODE_FOR_reduc_umax_scal_v4si },
  { 0xd70043, CODE_FOR_reduc_umax_scal_vnx4si },
  { 0xd70045, CODE_FOR_reduc_umax_scal_vnx2di },
  { 0xd8003b, CODE_FOR_reduc_umin_scal_v8qi },
  { 0xd8003c, CODE_FOR_reduc_umin_scal_v4hi },
  { 0xd8003d, CODE_FOR_reduc_umin_scal_v2si },
  { 0xd8003e, CODE_FOR_reduc_umin_scal_v16qi },
  { 0xd8003f, CODE_FOR_reduc_umin_scal_vnx16qi },
  { 0xd80040, CODE_FOR_reduc_umin_scal_v8hi },
  { 0xd80041, CODE_FOR_reduc_umin_scal_vnx8hi },
  { 0xd80042, CODE_FOR_reduc_umin_scal_v4si },
  { 0xd80043, CODE_FOR_reduc_umin_scal_vnx4si },
  { 0xd80045, CODE_FOR_reduc_umin_scal_vnx2di },
  { 0xd9003f, CODE_FOR_reduc_and_scal_vnx16qi },
  { 0xd90041, CODE_FOR_reduc_and_scal_vnx8hi },
  { 0xd90043, CODE_FOR_reduc_and_scal_vnx4si },
  { 0xd90045, CODE_FOR_reduc_and_scal_vnx2di },
  { 0xda003f, CODE_FOR_reduc_ior_scal_vnx16qi },
  { 0xda0041, CODE_FOR_reduc_ior_scal_vnx8hi },
  { 0xda0043, CODE_FOR_reduc_ior_scal_vnx4si },
  { 0xda0045, CODE_FOR_reduc_ior_scal_vnx2di },
  { 0xdb003f, CODE_FOR_reduc_xor_scal_vnx16qi },
  { 0xdb0041, CODE_FOR_reduc_xor_scal_vnx8hi },
  { 0xdb0043, CODE_FOR_reduc_xor_scal_vnx4si },
  { 0xdb0045, CODE_FOR_reduc_xor_scal_vnx2di },
  { 0xdc005b, CODE_FOR_fold_left_plus_vnx8hf },
  { 0xdc005d, CODE_FOR_fold_left_plus_vnx4sf },
  { 0xdc005f, CODE_FOR_fold_left_plus_vnx2df },
  { 0xdd003f, CODE_FOR_extract_last_vnx16qi },
  { 0xdd0041, CODE_FOR_extract_last_vnx8hi },
  { 0xdd0043, CODE_FOR_extract_last_vnx4si },
  { 0xdd0045, CODE_FOR_extract_last_vnx2di },
  { 0xdd005b, CODE_FOR_extract_last_vnx8hf },
  { 0xdd005d, CODE_FOR_extract_last_vnx4sf },
  { 0xdd005f, CODE_FOR_extract_last_vnx2df },
  { 0xde003f, CODE_FOR_fold_extract_last_vnx16qi },
  { 0xde0041, CODE_FOR_fold_extract_last_vnx8hi },
  { 0xde0043, CODE_FOR_fold_extract_last_vnx4si },
  { 0xde0045, CODE_FOR_fold_extract_last_vnx2di },
  { 0xde005b, CODE_FOR_fold_extract_last_vnx8hf },
  { 0xde005d, CODE_FOR_fold_extract_last_vnx4sf },
  { 0xde005f, CODE_FOR_fold_extract_last_vnx2df },
  { 0xdf003b, CODE_FOR_sdot_prodv8qi },
  { 0xdf003e, CODE_FOR_sdot_prodv16qi },
  { 0xe0003b, CODE_FOR_widen_ssumv8qi3 },
  { 0xe0003c, CODE_FOR_widen_ssumv4hi3 },
  { 0xe0003d, CODE_FOR_widen_ssumv2si3 },
  { 0xe0003e, CODE_FOR_widen_ssumv16qi3 },
  { 0xe00040, CODE_FOR_widen_ssumv8hi3 },
  { 0xe00042, CODE_FOR_widen_ssumv4si3 },
  { 0xe1003b, CODE_FOR_udot_prodv8qi },
  { 0xe1003e, CODE_FOR_udot_prodv16qi },
  { 0xe2003b, CODE_FOR_widen_usumv8qi3 },
  { 0xe2003c, CODE_FOR_widen_usumv4hi3 },
  { 0xe2003d, CODE_FOR_widen_usumv2si3 },
  { 0xe2003e, CODE_FOR_widen_usumv16qi3 },
  { 0xe20040, CODE_FOR_widen_usumv8hi3 },
  { 0xe20042, CODE_FOR_widen_usumv4si3 },
  { 0xe5005f, CODE_FOR_vec_pack_sfix_trunc_vnx2df },
  { 0xe7000d, CODE_FOR_vec_pack_trunc_di },
  { 0xe70026, CODE_FOR_vec_pack_trunc_df },
  { 0xe70038, CODE_FOR_vec_pack_trunc_vnx8bi },
  { 0xe70039, CODE_FOR_vec_pack_trunc_vnx4bi },
  { 0xe7003a, CODE_FOR_vec_pack_trunc_vnx2bi },
  { 0xe7003c, CODE_FOR_vec_pack_trunc_v4hi },
  { 0xe7003d, CODE_FOR_vec_pack_trunc_v2si },
  { 0xe70040, CODE_FOR_vec_pack_trunc_v8hi },
  { 0xe70041, CODE_FOR_vec_pack_trunc_vnx8hi },
  { 0xe70042, CODE_FOR_vec_pack_trunc_v4si },
  { 0xe70043, CODE_FOR_vec_pack_trunc_vnx4si },
  { 0xe70044, CODE_FOR_vec_pack_trunc_v2di },
  { 0xe70045, CODE_FOR_vec_pack_trunc_vnx2di },
  { 0xe7005d, CODE_FOR_vec_pack_trunc_vnx4sf },
  { 0xe7005e, CODE_FOR_vec_pack_trunc_v2df },
  { 0xe7005f, CODE_FOR_vec_pack_trunc_vnx2df },
  { 0xe8005f, CODE_FOR_vec_pack_ufix_trunc_vnx2df },
  { 0xea003b, CODE_FOR_vec_permv8qi },
  { 0xea003e, CODE_FOR_vec_permv16qi },
  { 0xea003f, CODE_FOR_vec_permvnx16qi },
  { 0xea0041, CODE_FOR_vec_permvnx8hi },
  { 0xea0043, CODE_FOR_vec_permvnx4si },
  { 0xea0045, CODE_FOR_vec_permvnx2di },
  { 0xea005b, CODE_FOR_vec_permvnx8hf },
  { 0xea005d, CODE_FOR_vec_permvnx4sf },
  { 0xea005f, CODE_FOR_vec_permvnx2df },
  { 0xec003b, CODE_FOR_vec_setv8qi },
  { 0xec003c, CODE_FOR_vec_setv4hi },
  { 0xec003d, CODE_FOR_vec_setv2si },
  { 0xec003e, CODE_FOR_vec_setv16qi },
  { 0xec0040, CODE_FOR_vec_setv8hi },
  { 0xec0042, CODE_FOR_vec_setv4si },
  { 0xec0044, CODE_FOR_vec_setv2di },
  { 0xec0057, CODE_FOR_vec_setv4hf },
  { 0xec0058, CODE_FOR_vec_setv2sf },
  { 0xec005a, CODE_FOR_vec_setv8hf },
  { 0xec005c, CODE_FOR_vec_setv4sf },
  { 0xec005e, CODE_FOR_vec_setv2df },
  { 0xed003b, CODE_FOR_vec_shr_v8qi },
  { 0xed003c, CODE_FOR_vec_shr_v4hi },
  { 0xed003d, CODE_FOR_vec_shr_v2si },
  { 0xed0057, CODE_FOR_vec_shr_v4hf },
  { 0xed0058, CODE_FOR_vec_shr_v2sf },
  { 0xee0043, CODE_FOR_vec_unpacks_float_hi_vnx4si },
  { 0xef0043, CODE_FOR_vec_unpacks_float_lo_vnx4si },
  { 0xf00037, CODE_FOR_vec_unpacks_hi_vnx16bi },
  { 0xf00038, CODE_FOR_vec_unpacks_hi_vnx8bi },
  { 0xf00039, CODE_FOR_vec_unpacks_hi_vnx4bi },
  { 0xf0003e, CODE_FOR_vec_unpacks_hi_v16qi },
  { 0xf0003f, CODE_FOR_vec_unpacks_hi_vnx16qi },
  { 0xf00040, CODE_FOR_vec_unpacks_hi_v8hi },
  { 0xf00041, CODE_FOR_vec_unpacks_hi_vnx8hi },
  { 0xf00042, CODE_FOR_vec_unpacks_hi_v4si },
  { 0xf00043, CODE_FOR_vec_unpacks_hi_vnx4si },
  { 0xf0005a, CODE_FOR_vec_unpacks_hi_v8hf },
  { 0xf0005b, CODE_FOR_vec_unpacks_hi_vnx8hf },
  { 0xf0005c, CODE_FOR_vec_unpacks_hi_v4sf },
  { 0xf0005d, CODE_FOR_vec_unpacks_hi_vnx4sf },
  { 0xf10037, CODE_FOR_vec_unpacks_lo_vnx16bi },
  { 0xf10038, CODE_FOR_vec_unpacks_lo_vnx8bi },
  { 0xf10039, CODE_FOR_vec_unpacks_lo_vnx4bi },
  { 0xf1003e, CODE_FOR_vec_unpacks_lo_v16qi },
  { 0xf1003f, CODE_FOR_vec_unpacks_lo_vnx16qi },
  { 0xf10040, CODE_FOR_vec_unpacks_lo_v8hi },
  { 0xf10041, CODE_FOR_vec_unpacks_lo_vnx8hi },
  { 0xf10042, CODE_FOR_vec_unpacks_lo_v4si },
  { 0xf10043, CODE_FOR_vec_unpacks_lo_vnx4si },
  { 0xf1005a, CODE_FOR_vec_unpacks_lo_v8hf },
  { 0xf1005b, CODE_FOR_vec_unpacks_lo_vnx8hf },
  { 0xf1005c, CODE_FOR_vec_unpacks_lo_v4sf },
  { 0xf1005d, CODE_FOR_vec_unpacks_lo_vnx4sf },
  { 0xf20043, CODE_FOR_vec_unpacku_float_hi_vnx4si },
  { 0xf30043, CODE_FOR_vec_unpacku_float_lo_vnx4si },
  { 0xf40037, CODE_FOR_vec_unpacku_hi_vnx16bi },
  { 0xf40038, CODE_FOR_vec_unpacku_hi_vnx8bi },
  { 0xf40039, CODE_FOR_vec_unpacku_hi_vnx4bi },
  { 0xf4003e, CODE_FOR_vec_unpacku_hi_v16qi },
  { 0xf4003f, CODE_FOR_vec_unpacku_hi_vnx16qi },
  { 0xf40040, CODE_FOR_vec_unpacku_hi_v8hi },
  { 0xf40041, CODE_FOR_vec_unpacku_hi_vnx8hi },
  { 0xf40042, CODE_FOR_vec_unpacku_hi_v4si },
  { 0xf40043, CODE_FOR_vec_unpacku_hi_vnx4si },
  { 0xf50037, CODE_FOR_vec_unpacku_lo_vnx16bi },
  { 0xf50038, CODE_FOR_vec_unpacku_lo_vnx8bi },
  { 0xf50039, CODE_FOR_vec_unpacku_lo_vnx4bi },
  { 0xf5003e, CODE_FOR_vec_unpacku_lo_v16qi },
  { 0xf5003f, CODE_FOR_vec_unpacku_lo_vnx16qi },
  { 0xf50040, CODE_FOR_vec_unpacku_lo_v8hi },
  { 0xf50041, CODE_FOR_vec_unpacku_lo_vnx8hi },
  { 0xf50042, CODE_FOR_vec_unpacku_lo_v4si },
  { 0xf50043, CODE_FOR_vec_unpacku_lo_vnx4si },
  { 0xf7003e, CODE_FOR_vec_widen_smult_hi_v16qi },
  { 0xf70040, CODE_FOR_vec_widen_smult_hi_v8hi },
  { 0xf70042, CODE_FOR_vec_widen_smult_hi_v4si },
  { 0xf8003e, CODE_FOR_vec_widen_smult_lo_v16qi },
  { 0xf80040, CODE_FOR_vec_widen_smult_lo_v8hi },
  { 0xf80042, CODE_FOR_vec_widen_smult_lo_v4si },
  { 0xfd003e, CODE_FOR_vec_widen_umult_hi_v16qi },
  { 0xfd0040, CODE_FOR_vec_widen_umult_hi_v8hi },
  { 0xfd0042, CODE_FOR_vec_widen_umult_hi_v4si },
  { 0xfe003e, CODE_FOR_vec_widen_umult_lo_v16qi },
  { 0xfe0040, CODE_FOR_vec_widen_umult_lo_v8hi },
  { 0xfe0042, CODE_FOR_vec_widen_umult_lo_v4si },
  { 0x109000a, CODE_FOR_atomic_add_fetchqi },
  { 0x109000b, CODE_FOR_atomic_add_fetchhi },
  { 0x109000c, CODE_FOR_atomic_add_fetchsi },
  { 0x109000d, CODE_FOR_atomic_add_fetchdi },
  { 0x10a000a, CODE_FOR_atomic_addqi },
  { 0x10a000b, CODE_FOR_atomic_addhi },
  { 0x10a000c, CODE_FOR_atomic_addsi },
  { 0x10a000d, CODE_FOR_atomic_adddi },
  { 0x10b000a, CODE_FOR_atomic_and_fetchqi },
  { 0x10b000b, CODE_FOR_atomic_and_fetchhi },
  { 0x10b000c, CODE_FOR_atomic_and_fetchsi },
  { 0x10b000d, CODE_FOR_atomic_and_fetchdi },
  { 0x10c000a, CODE_FOR_atomic_andqi },
  { 0x10c000b, CODE_FOR_atomic_andhi },
  { 0x10c000c, CODE_FOR_atomic_andsi },
  { 0x10c000d, CODE_FOR_atomic_anddi },
  { 0x110000a, CODE_FOR_atomic_compare_and_swapqi },
  { 0x110000b, CODE_FOR_atomic_compare_and_swaphi },
  { 0x110000c, CODE_FOR_atomic_compare_and_swapsi },
  { 0x110000d, CODE_FOR_atomic_compare_and_swapdi },
  { 0x111000a, CODE_FOR_atomic_exchangeqi },
  { 0x111000b, CODE_FOR_atomic_exchangehi },
  { 0x111000c, CODE_FOR_atomic_exchangesi },
  { 0x111000d, CODE_FOR_atomic_exchangedi },
  { 0x112000a, CODE_FOR_atomic_fetch_addqi },
  { 0x112000b, CODE_FOR_atomic_fetch_addhi },
  { 0x112000c, CODE_FOR_atomic_fetch_addsi },
  { 0x112000d, CODE_FOR_atomic_fetch_adddi },
  { 0x113000a, CODE_FOR_atomic_fetch_andqi },
  { 0x113000b, CODE_FOR_atomic_fetch_andhi },
  { 0x113000c, CODE_FOR_atomic_fetch_andsi },
  { 0x113000d, CODE_FOR_atomic_fetch_anddi },
  { 0x114000a, CODE_FOR_atomic_fetch_nandqi },
  { 0x114000b, CODE_FOR_atomic_fetch_nandhi },
  { 0x114000c, CODE_FOR_atomic_fetch_nandsi },
  { 0x114000d, CODE_FOR_atomic_fetch_nanddi },
  { 0x115000a, CODE_FOR_atomic_fetch_orqi },
  { 0x115000b, CODE_FOR_atomic_fetch_orhi },
  { 0x115000c, CODE_FOR_atomic_fetch_orsi },
  { 0x115000d, CODE_FOR_atomic_fetch_ordi },
  { 0x116000a, CODE_FOR_atomic_fetch_subqi },
  { 0x116000b, CODE_FOR_atomic_fetch_subhi },
  { 0x116000c, CODE_FOR_atomic_fetch_subsi },
  { 0x116000d, CODE_FOR_atomic_fetch_subdi },
  { 0x117000a, CODE_FOR_atomic_fetch_xorqi },
  { 0x117000b, CODE_FOR_atomic_fetch_xorhi },
  { 0x117000c, CODE_FOR_atomic_fetch_xorsi },
  { 0x117000d, CODE_FOR_atomic_fetch_xordi },
  { 0x118000a, CODE_FOR_atomic_loadqi },
  { 0x118000b, CODE_FOR_atomic_loadhi },
  { 0x118000c, CODE_FOR_atomic_loadsi },
  { 0x118000d, CODE_FOR_atomic_loaddi },
  { 0x119000a, CODE_FOR_atomic_nand_fetchqi },
  { 0x119000b, CODE_FOR_atomic_nand_fetchhi },
  { 0x119000c, CODE_FOR_atomic_nand_fetchsi },
  { 0x119000d, CODE_FOR_atomic_nand_fetchdi },
  { 0x11a000a, CODE_FOR_atomic_nandqi },
  { 0x11a000b, CODE_FOR_atomic_nandhi },
  { 0x11a000c, CODE_FOR_atomic_nandsi },
  { 0x11a000d, CODE_FOR_atomic_nanddi },
  { 0x11b000a, CODE_FOR_atomic_or_fetchqi },
  { 0x11b000b, CODE_FOR_atomic_or_fetchhi },
  { 0x11b000c, CODE_FOR_atomic_or_fetchsi },
  { 0x11b000d, CODE_FOR_atomic_or_fetchdi },
  { 0x11c000a, CODE_FOR_atomic_orqi },
  { 0x11c000b, CODE_FOR_atomic_orhi },
  { 0x11c000c, CODE_FOR_atomic_orsi },
  { 0x11c000d, CODE_FOR_atomic_ordi },
  { 0x11d000a, CODE_FOR_atomic_storeqi },
  { 0x11d000b, CODE_FOR_atomic_storehi },
  { 0x11d000c, CODE_FOR_atomic_storesi },
  { 0x11d000d, CODE_FOR_atomic_storedi },
  { 0x11e000a, CODE_FOR_atomic_sub_fetchqi },
  { 0x11e000b, CODE_FOR_atomic_sub_fetchhi },
  { 0x11e000c, CODE_FOR_atomic_sub_fetchsi },
  { 0x11e000d, CODE_FOR_atomic_sub_fetchdi },
  { 0x11f000a, CODE_FOR_atomic_subqi },
  { 0x11f000b, CODE_FOR_atomic_subhi },
  { 0x11f000c, CODE_FOR_atomic_subsi },
  { 0x11f000d, CODE_FOR_atomic_subdi },
  { 0x120000a, CODE_FOR_atomic_xor_fetchqi },
  { 0x120000b, CODE_FOR_atomic_xor_fetchhi },
  { 0x120000c, CODE_FOR_atomic_xor_fetchsi },
  { 0x120000d, CODE_FOR_atomic_xor_fetchdi },
  { 0x121000a, CODE_FOR_atomic_xorqi },
  { 0x121000b, CODE_FOR_atomic_xorhi },
  { 0x121000c, CODE_FOR_atomic_xorsi },
  { 0x121000d, CODE_FOR_atomic_xordi },
  { 0x122000d, CODE_FOR_get_thread_pointerdi },
  { 0x1240043, CODE_FOR_gather_loadvnx4si },
  { 0x1240045, CODE_FOR_gather_loadvnx2di },
  { 0x124005d, CODE_FOR_gather_loadvnx4sf },
  { 0x124005f, CODE_FOR_gather_loadvnx2df },
  { 0x1250043, CODE_FOR_mask_gather_loadvnx4si },
  { 0x1250045, CODE_FOR_mask_gather_loadvnx2di },
  { 0x125005d, CODE_FOR_mask_gather_loadvnx4sf },
  { 0x125005f, CODE_FOR_mask_gather_loadvnx2df },
  { 0x1260043, CODE_FOR_scatter_storevnx4si },
  { 0x1260045, CODE_FOR_scatter_storevnx2di },
  { 0x126005d, CODE_FOR_scatter_storevnx4sf },
  { 0x126005f, CODE_FOR_scatter_storevnx2df },
  { 0x1270043, CODE_FOR_mask_scatter_storevnx4si },
  { 0x1270045, CODE_FOR_mask_scatter_storevnx2di },
  { 0x127005d, CODE_FOR_mask_scatter_storevnx4sf },
  { 0x127005f, CODE_FOR_mask_scatter_storevnx2df },
  { 0x1280037, CODE_FOR_vec_duplicatevnx16bi },
  { 0x1280038, CODE_FOR_vec_duplicatevnx8bi },
  { 0x1280039, CODE_FOR_vec_duplicatevnx4bi },
  { 0x128003a, CODE_FOR_vec_duplicatevnx2bi },
  { 0x128003f, CODE_FOR_vec_duplicatevnx16qi },
  { 0x1280041, CODE_FOR_vec_duplicatevnx8hi },
  { 0x1280043, CODE_FOR_vec_duplicatevnx4si },
  { 0x1280045, CODE_FOR_vec_duplicatevnx2di },
  { 0x128005b, CODE_FOR_vec_duplicatevnx8hf },
  { 0x128005d, CODE_FOR_vec_duplicatevnx4sf },
  { 0x128005f, CODE_FOR_vec_duplicatevnx2df },
  { 0x129003f, CODE_FOR_vec_seriesvnx16qi },
  { 0x1290041, CODE_FOR_vec_seriesvnx8hi },
  { 0x1290043, CODE_FOR_vec_seriesvnx4si },
  { 0x1290045, CODE_FOR_vec_seriesvnx2di },
  { 0x12a003f, CODE_FOR_vec_shl_insert_vnx16qi },
  { 0x12a0041, CODE_FOR_vec_shl_insert_vnx8hi },
  { 0x12a0043, CODE_FOR_vec_shl_insert_vnx4si },
  { 0x12a0045, CODE_FOR_vec_shl_insert_vnx2di },
  { 0x12a005b, CODE_FOR_vec_shl_insert_vnx8hf },
  { 0x12a005d, CODE_FOR_vec_shl_insert_vnx4sf },
  { 0x12a005f, CODE_FOR_vec_shl_insert_vnx2df },
};

void
init_all_optabs (struct target_optabs *optabs)
{
  bool *ena = optabs->pat_enable;
  ena[0] = HAVE_extendqihi2;
  ena[1] = HAVE_extendqisi2;
  ena[2] = HAVE_extendqidi2;
  ena[3] = HAVE_extendhisi2;
  ena[4] = HAVE_extendhidi2;
  ena[5] = HAVE_extendsidi2;
  ena[6] = HAVE_extendhfsf2;
  ena[7] = HAVE_extendhfdf2;
  ena[8] = HAVE_extendsfdf2;
  ena[9] = HAVE_truncsfhf2;
  ena[10] = HAVE_truncdfhf2;
  ena[11] = HAVE_truncdfsf2;
  ena[12] = HAVE_zero_extendqihi2;
  ena[13] = HAVE_zero_extendqisi2;
  ena[14] = HAVE_zero_extendqidi2;
  ena[15] = HAVE_zero_extendhisi2;
  ena[16] = HAVE_zero_extendhidi2;
  ena[17] = HAVE_zero_extendsidi2;
  ena[18] = HAVE_fixv4hfv4hi2;
  ena[19] = HAVE_fixv2sfv2si2;
  ena[20] = HAVE_fixv8hfv8hi2;
  ena[21] = HAVE_fixv4sfv4si2;
  ena[22] = HAVE_fixv2dfv2di2;
  ena[23] = HAVE_fixunsv4hfv4hi2;
  ena[24] = HAVE_fixunsv2sfv2si2;
  ena[25] = HAVE_fixunsv8hfv8hi2;
  ena[26] = HAVE_fixunsv4sfv4si2;
  ena[27] = HAVE_fixunsv2dfv2di2;
  ena[28] = HAVE_floathihf2;
  ena[29] = HAVE_floatsihf2;
  ena[30] = HAVE_floatsisf2;
  ena[31] = HAVE_floatsidf2;
  ena[32] = HAVE_floatdihf2;
  ena[33] = HAVE_floatdisf2;
  ena[34] = HAVE_floatdidf2;
  ena[35] = HAVE_floatv4hiv4hf2;
  ena[36] = HAVE_floatv2siv2sf2;
  ena[37] = HAVE_floatv8hiv8hf2;
  ena[38] = HAVE_floatvnx8hivnx8hf2;
  ena[39] = HAVE_floatv4siv4sf2;
  ena[40] = HAVE_floatvnx4sivnx4sf2;
  ena[41] = HAVE_floatv2div2df2;
  ena[42] = HAVE_floatvnx2divnx2df2;
  ena[43] = HAVE_floatunshihf2;
  ena[44] = HAVE_floatunssihf2;
  ena[45] = HAVE_floatunssisf2;
  ena[46] = HAVE_floatunssidf2;
  ena[47] = HAVE_floatunsdihf2;
  ena[48] = HAVE_floatunsdisf2;
  ena[49] = HAVE_floatunsdidf2;
  ena[50] = HAVE_floatunsv4hiv4hf2;
  ena[51] = HAVE_floatunsv2siv2sf2;
  ena[52] = HAVE_floatunsv8hiv8hf2;
  ena[53] = HAVE_floatunsvnx8hivnx8hf2;
  ena[54] = HAVE_floatunsv4siv4sf2;
  ena[55] = HAVE_floatunsvnx4sivnx4sf2;
  ena[56] = HAVE_floatunsv2div2df2;
  ena[57] = HAVE_floatunsvnx2divnx2df2;
  ena[58] = HAVE_lrintsfsi2;
  ena[59] = HAVE_lrintsfdi2;
  ena[60] = HAVE_lrintdfsi2;
  ena[61] = HAVE_lrintdfdi2;
  ena[62] = HAVE_lroundhfhi2;
  ena[63] = HAVE_lroundhfsi2;
  ena[64] = HAVE_lroundhfdi2;
  ena[65] = HAVE_lroundsfsi2;
  ena[66] = HAVE_lroundsfdi2;
  ena[67] = HAVE_lrounddfsi2;
  ena[68] = HAVE_lrounddfdi2;
  ena[69] = HAVE_lroundv4hfv4hi2;
  ena[70] = HAVE_lroundv2sfv2si2;
  ena[71] = HAVE_lroundv8hfv8hi2;
  ena[72] = HAVE_lroundv4sfv4si2;
  ena[73] = HAVE_lroundv2dfv2di2;
  ena[74] = HAVE_lfloorhfhi2;
  ena[75] = HAVE_lfloorhfsi2;
  ena[76] = HAVE_lfloorhfdi2;
  ena[77] = HAVE_lfloorsfsi2;
  ena[78] = HAVE_lfloorsfdi2;
  ena[79] = HAVE_lfloordfsi2;
  ena[80] = HAVE_lfloordfdi2;
  ena[81] = HAVE_lfloorv4hfv4hi2;
  ena[82] = HAVE_lfloorv2sfv2si2;
  ena[83] = HAVE_lfloorv8hfv8hi2;
  ena[84] = HAVE_lfloorv4sfv4si2;
  ena[85] = HAVE_lfloorv2dfv2di2;
  ena[86] = HAVE_lceilhfhi2;
  ena[87] = HAVE_lceilhfsi2;
  ena[88] = HAVE_lceilhfdi2;
  ena[89] = HAVE_lceilsfsi2;
  ena[90] = HAVE_lceilsfdi2;
  ena[91] = HAVE_lceildfsi2;
  ena[92] = HAVE_lceildfdi2;
  ena[93] = HAVE_lceilv4hfv4hi2;
  ena[94] = HAVE_lceilv2sfv2si2;
  ena[95] = HAVE_lceilv8hfv8hi2;
  ena[96] = HAVE_lceilv4sfv4si2;
  ena[97] = HAVE_lceilv2dfv2di2;
  ena[98] = HAVE_fix_trunchfhi2;
  ena[99] = HAVE_fix_trunchfsi2;
  ena[100] = HAVE_fix_trunchfdi2;
  ena[101] = HAVE_fix_truncsfsi2;
  ena[102] = HAVE_fix_truncsfdi2;
  ena[103] = HAVE_fix_truncdfsi2;
  ena[104] = HAVE_fix_truncdfdi2;
  ena[105] = HAVE_fix_truncv4hfv4hi2;
  ena[106] = HAVE_fix_truncv2sfv2si2;
  ena[107] = HAVE_fix_truncv8hfv8hi2;
  ena[108] = HAVE_fix_truncvnx8hfvnx8hi2;
  ena[109] = HAVE_fix_truncv4sfv4si2;
  ena[110] = HAVE_fix_truncvnx4sfvnx4si2;
  ena[111] = HAVE_fix_truncv2dfv2di2;
  ena[112] = HAVE_fix_truncvnx2dfvnx2di2;
  ena[113] = HAVE_fixuns_trunchfhi2;
  ena[114] = HAVE_fixuns_trunchfsi2;
  ena[115] = HAVE_fixuns_trunchfdi2;
  ena[116] = HAVE_fixuns_truncsfsi2;
  ena[117] = HAVE_fixuns_truncsfdi2;
  ena[118] = HAVE_fixuns_truncdfsi2;
  ena[119] = HAVE_fixuns_truncdfdi2;
  ena[120] = HAVE_fixuns_truncv4hfv4hi2;
  ena[121] = HAVE_fixuns_truncv2sfv2si2;
  ena[122] = HAVE_fixuns_truncv8hfv8hi2;
  ena[123] = HAVE_fixuns_truncvnx8hfvnx8hi2;
  ena[124] = HAVE_fixuns_truncv4sfv4si2;
  ena[125] = HAVE_fixuns_truncvnx4sfvnx4si2;
  ena[126] = HAVE_fixuns_truncv2dfv2di2;
  ena[127] = HAVE_fixuns_truncvnx2dfvnx2di2;
  ena[128] = HAVE_mulsidi3;
  ena[129] = HAVE_mulditi3;
  ena[130] = HAVE_umulsidi3;
  ena[131] = HAVE_umulditi3;
  ena[132] = HAVE_maddsidi4;
  ena[133] = HAVE_umaddsidi4;
  ena[134] = HAVE_msubsidi4;
  ena[135] = HAVE_umsubsidi4;
  ena[136] = HAVE_vec_load_lanesoiv16qi;
  ena[137] = HAVE_vec_load_lanesciv16qi;
  ena[138] = HAVE_vec_load_lanesxiv16qi;
  ena[139] = HAVE_vec_load_lanesvnx32qivnx16qi;
  ena[140] = HAVE_vec_load_lanesvnx48qivnx16qi;
  ena[141] = HAVE_vec_load_lanesvnx64qivnx16qi;
  ena[142] = HAVE_vec_load_lanesoiv8hi;
  ena[143] = HAVE_vec_load_lanesciv8hi;
  ena[144] = HAVE_vec_load_lanesxiv8hi;
  ena[145] = HAVE_vec_load_lanesvnx16hivnx8hi;
  ena[146] = HAVE_vec_load_lanesvnx24hivnx8hi;
  ena[147] = HAVE_vec_load_lanesvnx32hivnx8hi;
  ena[148] = HAVE_vec_load_lanesoiv4si;
  ena[149] = HAVE_vec_load_lanesciv4si;
  ena[150] = HAVE_vec_load_lanesxiv4si;
  ena[151] = HAVE_vec_load_lanesvnx8sivnx4si;
  ena[152] = HAVE_vec_load_lanesvnx12sivnx4si;
  ena[153] = HAVE_vec_load_lanesvnx16sivnx4si;
  ena[154] = HAVE_vec_load_lanesoiv2di;
  ena[155] = HAVE_vec_load_lanesciv2di;
  ena[156] = HAVE_vec_load_lanesxiv2di;
  ena[157] = HAVE_vec_load_lanesvnx4divnx2di;
  ena[158] = HAVE_vec_load_lanesvnx6divnx2di;
  ena[159] = HAVE_vec_load_lanesvnx8divnx2di;
  ena[160] = HAVE_vec_load_lanesoiv8hf;
  ena[161] = HAVE_vec_load_lanesciv8hf;
  ena[162] = HAVE_vec_load_lanesxiv8hf;
  ena[163] = HAVE_vec_load_lanesvnx16hfvnx8hf;
  ena[164] = HAVE_vec_load_lanesvnx24hfvnx8hf;
  ena[165] = HAVE_vec_load_lanesvnx32hfvnx8hf;
  ena[166] = HAVE_vec_load_lanesoiv4sf;
  ena[167] = HAVE_vec_load_lanesciv4sf;
  ena[168] = HAVE_vec_load_lanesxiv4sf;
  ena[169] = HAVE_vec_load_lanesvnx8sfvnx4sf;
  ena[170] = HAVE_vec_load_lanesvnx12sfvnx4sf;
  ena[171] = HAVE_vec_load_lanesvnx16sfvnx4sf;
  ena[172] = HAVE_vec_load_lanesoiv2df;
  ena[173] = HAVE_vec_load_lanesciv2df;
  ena[174] = HAVE_vec_load_lanesxiv2df;
  ena[175] = HAVE_vec_load_lanesvnx4dfvnx2df;
  ena[176] = HAVE_vec_load_lanesvnx6dfvnx2df;
  ena[177] = HAVE_vec_load_lanesvnx8dfvnx2df;
  ena[178] = HAVE_vec_store_lanesoiv16qi;
  ena[179] = HAVE_vec_store_lanesciv16qi;
  ena[180] = HAVE_vec_store_lanesxiv16qi;
  ena[181] = HAVE_vec_store_lanesvnx32qivnx16qi;
  ena[182] = HAVE_vec_store_lanesvnx48qivnx16qi;
  ena[183] = HAVE_vec_store_lanesvnx64qivnx16qi;
  ena[184] = HAVE_vec_store_lanesoiv8hi;
  ena[185] = HAVE_vec_store_lanesciv8hi;
  ena[186] = HAVE_vec_store_lanesxiv8hi;
  ena[187] = HAVE_vec_store_lanesvnx16hivnx8hi;
  ena[188] = HAVE_vec_store_lanesvnx24hivnx8hi;
  ena[189] = HAVE_vec_store_lanesvnx32hivnx8hi;
  ena[190] = HAVE_vec_store_lanesoiv4si;
  ena[191] = HAVE_vec_store_lanesciv4si;
  ena[192] = HAVE_vec_store_lanesxiv4si;
  ena[193] = HAVE_vec_store_lanesvnx8sivnx4si;
  ena[194] = HAVE_vec_store_lanesvnx12sivnx4si;
  ena[195] = HAVE_vec_store_lanesvnx16sivnx4si;
  ena[196] = HAVE_vec_store_lanesoiv2di;
  ena[197] = HAVE_vec_store_lanesciv2di;
  ena[198] = HAVE_vec_store_lanesxiv2di;
  ena[199] = HAVE_vec_store_lanesvnx4divnx2di;
  ena[200] = HAVE_vec_store_lanesvnx6divnx2di;
  ena[201] = HAVE_vec_store_lanesvnx8divnx2di;
  ena[202] = HAVE_vec_store_lanesoiv8hf;
  ena[203] = HAVE_vec_store_lanesciv8hf;
  ena[204] = HAVE_vec_store_lanesxiv8hf;
  ena[205] = HAVE_vec_store_lanesvnx16hfvnx8hf;
  ena[206] = HAVE_vec_store_lanesvnx24hfvnx8hf;
  ena[207] = HAVE_vec_store_lanesvnx32hfvnx8hf;
  ena[208] = HAVE_vec_store_lanesoiv4sf;
  ena[209] = HAVE_vec_store_lanesciv4sf;
  ena[210] = HAVE_vec_store_lanesxiv4sf;
  ena[211] = HAVE_vec_store_lanesvnx8sfvnx4sf;
  ena[212] = HAVE_vec_store_lanesvnx12sfvnx4sf;
  ena[213] = HAVE_vec_store_lanesvnx16sfvnx4sf;
  ena[214] = HAVE_vec_store_lanesoiv2df;
  ena[215] = HAVE_vec_store_lanesciv2df;
  ena[216] = HAVE_vec_store_lanesxiv2df;
  ena[217] = HAVE_vec_store_lanesvnx4dfvnx2df;
  ena[218] = HAVE_vec_store_lanesvnx6dfvnx2df;
  ena[219] = HAVE_vec_store_lanesvnx8dfvnx2df;
  ena[220] = HAVE_vec_mask_load_lanesvnx32qivnx16qi;
  ena[221] = HAVE_vec_mask_load_lanesvnx48qivnx16qi;
  ena[222] = HAVE_vec_mask_load_lanesvnx64qivnx16qi;
  ena[223] = HAVE_vec_mask_load_lanesvnx16hivnx8hi;
  ena[224] = HAVE_vec_mask_load_lanesvnx24hivnx8hi;
  ena[225] = HAVE_vec_mask_load_lanesvnx32hivnx8hi;
  ena[226] = HAVE_vec_mask_load_lanesvnx8sivnx4si;
  ena[227] = HAVE_vec_mask_load_lanesvnx12sivnx4si;
  ena[228] = HAVE_vec_mask_load_lanesvnx16sivnx4si;
  ena[229] = HAVE_vec_mask_load_lanesvnx4divnx2di;
  ena[230] = HAVE_vec_mask_load_lanesvnx6divnx2di;
  ena[231] = HAVE_vec_mask_load_lanesvnx8divnx2di;
  ena[232] = HAVE_vec_mask_load_lanesvnx16hfvnx8hf;
  ena[233] = HAVE_vec_mask_load_lanesvnx24hfvnx8hf;
  ena[234] = HAVE_vec_mask_load_lanesvnx32hfvnx8hf;
  ena[235] = HAVE_vec_mask_load_lanesvnx8sfvnx4sf;
  ena[236] = HAVE_vec_mask_load_lanesvnx12sfvnx4sf;
  ena[237] = HAVE_vec_mask_load_lanesvnx16sfvnx4sf;
  ena[238] = HAVE_vec_mask_load_lanesvnx4dfvnx2df;
  ena[239] = HAVE_vec_mask_load_lanesvnx6dfvnx2df;
  ena[240] = HAVE_vec_mask_load_lanesvnx8dfvnx2df;
  ena[241] = HAVE_vec_mask_store_lanesvnx32qivnx16qi;
  ena[242] = HAVE_vec_mask_store_lanesvnx48qivnx16qi;
  ena[243] = HAVE_vec_mask_store_lanesvnx64qivnx16qi;
  ena[244] = HAVE_vec_mask_store_lanesvnx16hivnx8hi;
  ena[245] = HAVE_vec_mask_store_lanesvnx24hivnx8hi;
  ena[246] = HAVE_vec_mask_store_lanesvnx32hivnx8hi;
  ena[247] = HAVE_vec_mask_store_lanesvnx8sivnx4si;
  ena[248] = HAVE_vec_mask_store_lanesvnx12sivnx4si;
  ena[249] = HAVE_vec_mask_store_lanesvnx16sivnx4si;
  ena[250] = HAVE_vec_mask_store_lanesvnx4divnx2di;
  ena[251] = HAVE_vec_mask_store_lanesvnx6divnx2di;
  ena[252] = HAVE_vec_mask_store_lanesvnx8divnx2di;
  ena[253] = HAVE_vec_mask_store_lanesvnx16hfvnx8hf;
  ena[254] = HAVE_vec_mask_store_lanesvnx24hfvnx8hf;
  ena[255] = HAVE_vec_mask_store_lanesvnx32hfvnx8hf;
  ena[256] = HAVE_vec_mask_store_lanesvnx8sfvnx4sf;
  ena[257] = HAVE_vec_mask_store_lanesvnx12sfvnx4sf;
  ena[258] = HAVE_vec_mask_store_lanesvnx16sfvnx4sf;
  ena[259] = HAVE_vec_mask_store_lanesvnx4dfvnx2df;
  ena[260] = HAVE_vec_mask_store_lanesvnx6dfvnx2df;
  ena[261] = HAVE_vec_mask_store_lanesvnx8dfvnx2df;
  ena[262] = HAVE_vconddidi;
  ena[263] = HAVE_vcondv8qiv8qi;
  ena[264] = HAVE_vcondv4hiv4hi;
  ena[265] = HAVE_vcondv2siv2si;
  ena[266] = HAVE_vcondv2sfv2si;
  ena[267] = HAVE_vcondv16qiv16qi;
  ena[268] = HAVE_vcondvnx16qivnx16qi;
  ena[269] = HAVE_vcondv8hiv8hi;
  ena[270] = HAVE_vcondvnx8hivnx8hi;
  ena[271] = HAVE_vcondvnx8hfvnx8hi;
  ena[272] = HAVE_vcondv4siv4si;
  ena[273] = HAVE_vcondv4sfv4si;
  ena[274] = HAVE_vcondvnx4sivnx4si;
  ena[275] = HAVE_vcondvnx4sfvnx4si;
  ena[276] = HAVE_vcondv2div2di;
  ena[277] = HAVE_vcondv2dfv2di;
  ena[278] = HAVE_vcondvnx2divnx2di;
  ena[279] = HAVE_vcondvnx2dfvnx2di;
  ena[280] = HAVE_vcondv2siv2sf;
  ena[281] = HAVE_vcondv2sfv2sf;
  ena[282] = HAVE_vcondv4siv4sf;
  ena[283] = HAVE_vcondv4sfv4sf;
  ena[284] = HAVE_vcondvnx4sivnx4sf;
  ena[285] = HAVE_vcondvnx4sfvnx4sf;
  ena[286] = HAVE_vcondv2div2df;
  ena[287] = HAVE_vcondv2dfv2df;
  ena[288] = HAVE_vcondvnx2divnx2df;
  ena[289] = HAVE_vcondvnx2dfvnx2df;
  ena[290] = HAVE_vcondudidi;
  ena[291] = HAVE_vconduv8qiv8qi;
  ena[292] = HAVE_vconduv4hiv4hi;
  ena[293] = HAVE_vconduv2siv2si;
  ena[294] = HAVE_vconduv2sfv2si;
  ena[295] = HAVE_vconduv16qiv16qi;
  ena[296] = HAVE_vconduvnx16qivnx16qi;
  ena[297] = HAVE_vconduv8hiv8hi;
  ena[298] = HAVE_vconduvnx8hivnx8hi;
  ena[299] = HAVE_vconduvnx8hfvnx8hi;
  ena[300] = HAVE_vconduv4siv4si;
  ena[301] = HAVE_vconduv4sfv4si;
  ena[302] = HAVE_vconduvnx4sivnx4si;
  ena[303] = HAVE_vconduvnx4sfvnx4si;
  ena[304] = HAVE_vconduv2div2di;
  ena[305] = HAVE_vconduv2dfv2di;
  ena[306] = HAVE_vconduvnx2divnx2di;
  ena[307] = HAVE_vconduvnx2dfvnx2di;
  ena[308] = HAVE_vcond_mask_didi;
  ena[309] = HAVE_vcond_mask_vnx16qivnx16bi;
  ena[310] = HAVE_vcond_mask_vnx8hivnx8bi;
  ena[311] = HAVE_vcond_mask_vnx8hfvnx8bi;
  ena[312] = HAVE_vcond_mask_vnx4sivnx4bi;
  ena[313] = HAVE_vcond_mask_vnx4sfvnx4bi;
  ena[314] = HAVE_vcond_mask_vnx2divnx2bi;
  ena[315] = HAVE_vcond_mask_vnx2dfvnx2bi;
  ena[316] = HAVE_vcond_mask_v8qiv8qi;
  ena[317] = HAVE_vcond_mask_v4hiv4hi;
  ena[318] = HAVE_vcond_mask_v2siv2si;
  ena[319] = HAVE_vcond_mask_v2sfv2si;
  ena[320] = HAVE_vcond_mask_v16qiv16qi;
  ena[321] = HAVE_vcond_mask_v8hiv8hi;
  ena[322] = HAVE_vcond_mask_v4siv4si;
  ena[323] = HAVE_vcond_mask_v4sfv4si;
  ena[324] = HAVE_vcond_mask_v2div2di;
  ena[325] = HAVE_vcond_mask_v2dfv2di;
  ena[326] = HAVE_vec_cmpdidi;
  ena[327] = HAVE_vec_cmpvnx16qivnx16bi;
  ena[328] = HAVE_vec_cmpvnx8hivnx8bi;
  ena[329] = HAVE_vec_cmpvnx8hfvnx8bi;
  ena[330] = HAVE_vec_cmpvnx4sivnx4bi;
  ena[331] = HAVE_vec_cmpvnx4sfvnx4bi;
  ena[332] = HAVE_vec_cmpvnx2divnx2bi;
  ena[333] = HAVE_vec_cmpvnx2dfvnx2bi;
  ena[334] = HAVE_vec_cmpv8qiv8qi;
  ena[335] = HAVE_vec_cmpv4hiv4hi;
  ena[336] = HAVE_vec_cmpv2siv2si;
  ena[337] = HAVE_vec_cmpv2sfv2si;
  ena[338] = HAVE_vec_cmpv16qiv16qi;
  ena[339] = HAVE_vec_cmpv8hiv8hi;
  ena[340] = HAVE_vec_cmpv4siv4si;
  ena[341] = HAVE_vec_cmpv4sfv4si;
  ena[342] = HAVE_vec_cmpv2div2di;
  ena[343] = HAVE_vec_cmpv2dfv2di;
  ena[344] = HAVE_vec_cmpudidi;
  ena[345] = HAVE_vec_cmpuvnx16qivnx16bi;
  ena[346] = HAVE_vec_cmpuvnx8hivnx8bi;
  ena[347] = HAVE_vec_cmpuvnx4sivnx4bi;
  ena[348] = HAVE_vec_cmpuvnx2divnx2bi;
  ena[349] = HAVE_vec_cmpuv8qiv8qi;
  ena[350] = HAVE_vec_cmpuv4hiv4hi;
  ena[351] = HAVE_vec_cmpuv2siv2si;
  ena[352] = HAVE_vec_cmpuv16qiv16qi;
  ena[353] = HAVE_vec_cmpuv8hiv8hi;
  ena[354] = HAVE_vec_cmpuv4siv4si;
  ena[355] = HAVE_vec_cmpuv2div2di;
  ena[356] = HAVE_maskloadvnx16qivnx16bi;
  ena[357] = HAVE_maskloadvnx8hivnx8bi;
  ena[358] = HAVE_maskloadvnx8hfvnx8bi;
  ena[359] = HAVE_maskloadvnx4sivnx4bi;
  ena[360] = HAVE_maskloadvnx4sfvnx4bi;
  ena[361] = HAVE_maskloadvnx2divnx2bi;
  ena[362] = HAVE_maskloadvnx2dfvnx2bi;
  ena[363] = HAVE_maskstorevnx16qivnx16bi;
  ena[364] = HAVE_maskstorevnx8hivnx8bi;
  ena[365] = HAVE_maskstorevnx8hfvnx8bi;
  ena[366] = HAVE_maskstorevnx4sivnx4bi;
  ena[367] = HAVE_maskstorevnx4sfvnx4bi;
  ena[368] = HAVE_maskstorevnx2divnx2bi;
  ena[369] = HAVE_maskstorevnx2dfvnx2bi;
  ena[370] = HAVE_vec_extractvnx16biqi;
  ena[371] = HAVE_vec_extractv8qiqi;
  ena[372] = HAVE_vec_extractv16qiqi;
  ena[373] = HAVE_vec_extractvnx16qiqi;
  ena[374] = HAVE_vec_extractvnx8bihi;
  ena[375] = HAVE_vec_extractv4hihi;
  ena[376] = HAVE_vec_extractv8hihi;
  ena[377] = HAVE_vec_extractvnx8hihi;
  ena[378] = HAVE_vec_extractvnx4bisi;
  ena[379] = HAVE_vec_extractv2sisi;
  ena[380] = HAVE_vec_extractv4sisi;
  ena[381] = HAVE_vec_extractvnx4sisi;
  ena[382] = HAVE_vec_extractvnx2bidi;
  ena[383] = HAVE_vec_extractv2didi;
  ena[384] = HAVE_vec_extractvnx2didi;
  ena[385] = HAVE_vec_extractv4hfhf;
  ena[386] = HAVE_vec_extractv8hfhf;
  ena[387] = HAVE_vec_extractvnx8hfhf;
  ena[388] = HAVE_vec_extractv2sfsf;
  ena[389] = HAVE_vec_extractv4sfsf;
  ena[390] = HAVE_vec_extractvnx4sfsf;
  ena[391] = HAVE_vec_extractv2dfdf;
  ena[392] = HAVE_vec_extractvnx2dfdf;
  ena[393] = HAVE_vec_initv8qiqi;
  ena[394] = HAVE_vec_initv16qiqi;
  ena[395] = HAVE_vec_initv4hihi;
  ena[396] = HAVE_vec_initv8hihi;
  ena[397] = HAVE_vec_initv2sisi;
  ena[398] = HAVE_vec_initv4sisi;
  ena[399] = HAVE_vec_initv2didi;
  ena[400] = HAVE_vec_initv4hfhf;
  ena[401] = HAVE_vec_initv8hfhf;
  ena[402] = HAVE_vec_initv2sfsf;
  ena[403] = HAVE_vec_initv4sfsf;
  ena[404] = HAVE_vec_initv2dfdf;
  ena[405] = HAVE_while_ultsivnx16bi;
  ena[406] = HAVE_while_ultdivnx16bi;
  ena[407] = HAVE_while_ultsivnx8bi;
  ena[408] = HAVE_while_ultdivnx8bi;
  ena[409] = HAVE_while_ultsivnx4bi;
  ena[410] = HAVE_while_ultdivnx4bi;
  ena[411] = HAVE_while_ultsivnx2bi;
  ena[412] = HAVE_while_ultdivnx2bi;
  ena[413] = HAVE_addsi3;
  ena[414] = HAVE_adddi3;
  ena[415] = HAVE_addti3;
  ena[416] = HAVE_addhf3;
  ena[417] = HAVE_addsf3;
  ena[418] = HAVE_adddf3;
  ena[419] = HAVE_addv8qi3;
  ena[420] = HAVE_addv4hi3;
  ena[421] = HAVE_addv2si3;
  ena[422] = HAVE_addv16qi3;
  ena[423] = HAVE_addvnx16qi3;
  ena[424] = HAVE_addv8hi3;
  ena[425] = HAVE_addvnx8hi3;
  ena[426] = HAVE_addv4si3;
  ena[427] = HAVE_addvnx4si3;
  ena[428] = HAVE_addv2di3;
  ena[429] = HAVE_addvnx2di3;
  ena[430] = HAVE_addv4hf3;
  ena[431] = HAVE_addv2sf3;
  ena[432] = HAVE_addv8hf3;
  ena[433] = HAVE_addvnx8hf3;
  ena[434] = HAVE_addv4sf3;
  ena[435] = HAVE_addvnx4sf3;
  ena[436] = HAVE_addv2df3;
  ena[437] = HAVE_addvnx2df3;
  ena[438] = HAVE_subsi3;
  ena[439] = HAVE_subdi3;
  ena[440] = HAVE_subti3;
  ena[441] = HAVE_subhf3;
  ena[442] = HAVE_subsf3;
  ena[443] = HAVE_subdf3;
  ena[444] = HAVE_subv8qi3;
  ena[445] = HAVE_subv4hi3;
  ena[446] = HAVE_subv2si3;
  ena[447] = HAVE_subv16qi3;
  ena[448] = HAVE_subvnx16qi3;
  ena[449] = HAVE_subv8hi3;
  ena[450] = HAVE_subvnx8hi3;
  ena[451] = HAVE_subv4si3;
  ena[452] = HAVE_subvnx4si3;
  ena[453] = HAVE_subv2di3;
  ena[454] = HAVE_subvnx2di3;
  ena[455] = HAVE_subv4hf3;
  ena[456] = HAVE_subv2sf3;
  ena[457] = HAVE_subv8hf3;
  ena[458] = HAVE_subvnx8hf3;
  ena[459] = HAVE_subv4sf3;
  ena[460] = HAVE_subvnx4sf3;
  ena[461] = HAVE_subv2df3;
  ena[462] = HAVE_subvnx2df3;
  ena[463] = HAVE_mulsi3;
  ena[464] = HAVE_muldi3;
  ena[465] = HAVE_multi3;
  ena[466] = HAVE_mulhf3;
  ena[467] = HAVE_mulsf3;
  ena[468] = HAVE_muldf3;
  ena[469] = HAVE_mulv8qi3;
  ena[470] = HAVE_mulv4hi3;
  ena[471] = HAVE_mulv2si3;
  ena[472] = HAVE_mulv16qi3;
  ena[473] = HAVE_mulvnx16qi3;
  ena[474] = HAVE_mulv8hi3;
  ena[475] = HAVE_mulvnx8hi3;
  ena[476] = HAVE_mulv4si3;
  ena[477] = HAVE_mulvnx4si3;
  ena[478] = HAVE_mulvnx2di3;
  ena[479] = HAVE_mulv4hf3;
  ena[480] = HAVE_mulv2sf3;
  ena[481] = HAVE_mulv8hf3;
  ena[482] = HAVE_mulvnx8hf3;
  ena[483] = HAVE_mulv4sf3;
  ena[484] = HAVE_mulvnx4sf3;
  ena[485] = HAVE_mulv2df3;
  ena[486] = HAVE_mulvnx2df3;
  ena[487] = HAVE_divsi3;
  ena[488] = HAVE_divdi3;
  ena[489] = HAVE_divhf3;
  ena[490] = HAVE_divsf3;
  ena[491] = HAVE_divdf3;
  ena[492] = HAVE_divv4hf3;
  ena[493] = HAVE_divv2sf3;
  ena[494] = HAVE_divv8hf3;
  ena[495] = HAVE_divvnx8hf3;
  ena[496] = HAVE_divv4sf3;
  ena[497] = HAVE_divvnx4sf3;
  ena[498] = HAVE_divv2df3;
  ena[499] = HAVE_divvnx2df3;
  ena[500] = HAVE_udivsi3;
  ena[501] = HAVE_udivdi3;
  ena[502] = HAVE_modsi3;
  ena[503] = HAVE_moddi3;
  ena[504] = HAVE_ftruncv4hf2;
  ena[505] = HAVE_ftruncv2sf2;
  ena[506] = HAVE_ftruncv8hf2;
  ena[507] = HAVE_ftruncv4sf2;
  ena[508] = HAVE_ftruncv2df2;
  ena[509] = HAVE_andsi3;
  ena[510] = HAVE_anddi3;
  ena[511] = HAVE_andvnx16bi3;
  ena[512] = HAVE_andvnx8bi3;
  ena[513] = HAVE_andvnx4bi3;
  ena[514] = HAVE_andvnx2bi3;
  ena[515] = HAVE_andv8qi3;
  ena[516] = HAVE_andv4hi3;
  ena[517] = HAVE_andv2si3;
  ena[518] = HAVE_andv16qi3;
  ena[519] = HAVE_andvnx16qi3;
  ena[520] = HAVE_andv8hi3;
  ena[521] = HAVE_andvnx8hi3;
  ena[522] = HAVE_andv4si3;
  ena[523] = HAVE_andvnx4si3;
  ena[524] = HAVE_andv2di3;
  ena[525] = HAVE_andvnx2di3;
  ena[526] = HAVE_iorsi3;
  ena[527] = HAVE_iordi3;
  ena[528] = HAVE_iorvnx16bi3;
  ena[529] = HAVE_iorvnx8bi3;
  ena[530] = HAVE_iorvnx4bi3;
  ena[531] = HAVE_iorvnx2bi3;
  ena[532] = HAVE_iorv8qi3;
  ena[533] = HAVE_iorv4hi3;
  ena[534] = HAVE_iorv2si3;
  ena[535] = HAVE_iorv16qi3;
  ena[536] = HAVE_iorvnx16qi3;
  ena[537] = HAVE_iorv8hi3;
  ena[538] = HAVE_iorvnx8hi3;
  ena[539] = HAVE_iorv4si3;
  ena[540] = HAVE_iorvnx4si3;
  ena[541] = HAVE_iorv2di3;
  ena[542] = HAVE_iorvnx2di3;
  ena[543] = HAVE_xorsi3;
  ena[544] = HAVE_xordi3;
  ena[545] = HAVE_xorvnx16bi3;
  ena[546] = HAVE_xorvnx8bi3;
  ena[547] = HAVE_xorvnx4bi3;
  ena[548] = HAVE_xorvnx2bi3;
  ena[549] = HAVE_xorv8qi3;
  ena[550] = HAVE_xorv4hi3;
  ena[551] = HAVE_xorv2si3;
  ena[552] = HAVE_xorv16qi3;
  ena[553] = HAVE_xorvnx16qi3;
  ena[554] = HAVE_xorv8hi3;
  ena[555] = HAVE_xorvnx8hi3;
  ena[556] = HAVE_xorv4si3;
  ena[557] = HAVE_xorvnx4si3;
  ena[558] = HAVE_xorv2di3;
  ena[559] = HAVE_xorvnx2di3;
  ena[560] = HAVE_ashlqi3;
  ena[561] = HAVE_ashlhi3;
  ena[562] = HAVE_ashlsi3;
  ena[563] = HAVE_ashldi3;
  ena[564] = HAVE_ashlv8qi3;
  ena[565] = HAVE_ashlv4hi3;
  ena[566] = HAVE_ashlv2si3;
  ena[567] = HAVE_ashlv16qi3;
  ena[568] = HAVE_ashlvnx16qi3;
  ena[569] = HAVE_ashlv8hi3;
  ena[570] = HAVE_ashlvnx8hi3;
  ena[571] = HAVE_ashlv4si3;
  ena[572] = HAVE_ashlvnx4si3;
  ena[573] = HAVE_ashlv2di3;
  ena[574] = HAVE_ashlvnx2di3;
  ena[575] = HAVE_ashrsi3;
  ena[576] = HAVE_ashrdi3;
  ena[577] = HAVE_ashrv8qi3;
  ena[578] = HAVE_ashrv4hi3;
  ena[579] = HAVE_ashrv2si3;
  ena[580] = HAVE_ashrv16qi3;
  ena[581] = HAVE_ashrvnx16qi3;
  ena[582] = HAVE_ashrv8hi3;
  ena[583] = HAVE_ashrvnx8hi3;
  ena[584] = HAVE_ashrv4si3;
  ena[585] = HAVE_ashrvnx4si3;
  ena[586] = HAVE_ashrv2di3;
  ena[587] = HAVE_ashrvnx2di3;
  ena[588] = HAVE_lshrsi3;
  ena[589] = HAVE_lshrdi3;
  ena[590] = HAVE_lshrv8qi3;
  ena[591] = HAVE_lshrv4hi3;
  ena[592] = HAVE_lshrv2si3;
  ena[593] = HAVE_lshrv16qi3;
  ena[594] = HAVE_lshrvnx16qi3;
  ena[595] = HAVE_lshrv8hi3;
  ena[596] = HAVE_lshrvnx8hi3;
  ena[597] = HAVE_lshrv4si3;
  ena[598] = HAVE_lshrvnx4si3;
  ena[599] = HAVE_lshrv2di3;
  ena[600] = HAVE_lshrvnx2di3;
  ena[601] = HAVE_rotlsi3;
  ena[602] = HAVE_rotldi3;
  ena[603] = HAVE_rotrsi3;
  ena[604] = HAVE_rotrdi3;
  ena[605] = HAVE_vashlv8qi3;
  ena[606] = HAVE_vashlv4hi3;
  ena[607] = HAVE_vashlv2si3;
  ena[608] = HAVE_vashlv16qi3;
  ena[609] = HAVE_vashlvnx16qi3;
  ena[610] = HAVE_vashlv8hi3;
  ena[611] = HAVE_vashlvnx8hi3;
  ena[612] = HAVE_vashlv4si3;
  ena[613] = HAVE_vashlvnx4si3;
  ena[614] = HAVE_vashlv2di3;
  ena[615] = HAVE_vashlvnx2di3;
  ena[616] = HAVE_vashrv8qi3;
  ena[617] = HAVE_vashrv4hi3;
  ena[618] = HAVE_vashrv2si3;
  ena[619] = HAVE_vashrv16qi3;
  ena[620] = HAVE_vashrvnx16qi3;
  ena[621] = HAVE_vashrv8hi3;
  ena[622] = HAVE_vashrvnx8hi3;
  ena[623] = HAVE_vashrv4si3;
  ena[624] = HAVE_vashrvnx4si3;
  ena[625] = HAVE_vashrvnx2di3;
  ena[626] = HAVE_vlshrv8qi3;
  ena[627] = HAVE_vlshrv4hi3;
  ena[628] = HAVE_vlshrv2si3;
  ena[629] = HAVE_vlshrv16qi3;
  ena[630] = HAVE_vlshrvnx16qi3;
  ena[631] = HAVE_vlshrv8hi3;
  ena[632] = HAVE_vlshrvnx8hi3;
  ena[633] = HAVE_vlshrv4si3;
  ena[634] = HAVE_vlshrvnx4si3;
  ena[635] = HAVE_vlshrvnx2di3;
  ena[636] = HAVE_sminsf3;
  ena[637] = HAVE_smindf3;
  ena[638] = HAVE_sminv8qi3;
  ena[639] = HAVE_sminv4hi3;
  ena[640] = HAVE_sminv2si3;
  ena[641] = HAVE_sminv16qi3;
  ena[642] = HAVE_sminvnx16qi3;
  ena[643] = HAVE_sminv8hi3;
  ena[644] = HAVE_sminvnx8hi3;
  ena[645] = HAVE_sminv4si3;
  ena[646] = HAVE_sminvnx4si3;
  ena[647] = HAVE_sminv2di3;
  ena[648] = HAVE_sminvnx2di3;
  ena[649] = HAVE_sminv4hf3;
  ena[650] = HAVE_sminv2sf3;
  ena[651] = HAVE_sminv8hf3;
  ena[652] = HAVE_sminvnx8hf3;
  ena[653] = HAVE_sminv4sf3;
  ena[654] = HAVE_sminvnx4sf3;
  ena[655] = HAVE_sminv2df3;
  ena[656] = HAVE_sminvnx2df3;
  ena[657] = HAVE_smaxsf3;
  ena[658] = HAVE_smaxdf3;
  ena[659] = HAVE_smaxv8qi3;
  ena[660] = HAVE_smaxv4hi3;
  ena[661] = HAVE_smaxv2si3;
  ena[662] = HAVE_smaxv16qi3;
  ena[663] = HAVE_smaxvnx16qi3;
  ena[664] = HAVE_smaxv8hi3;
  ena[665] = HAVE_smaxvnx8hi3;
  ena[666] = HAVE_smaxv4si3;
  ena[667] = HAVE_smaxvnx4si3;
  ena[668] = HAVE_smaxv2di3;
  ena[669] = HAVE_smaxvnx2di3;
  ena[670] = HAVE_smaxv4hf3;
  ena[671] = HAVE_smaxv2sf3;
  ena[672] = HAVE_smaxv8hf3;
  ena[673] = HAVE_smaxvnx8hf3;
  ena[674] = HAVE_smaxv4sf3;
  ena[675] = HAVE_smaxvnx4sf3;
  ena[676] = HAVE_smaxv2df3;
  ena[677] = HAVE_smaxvnx2df3;
  ena[678] = HAVE_uminv8qi3;
  ena[679] = HAVE_uminv4hi3;
  ena[680] = HAVE_uminv2si3;
  ena[681] = HAVE_uminv16qi3;
  ena[682] = HAVE_uminvnx16qi3;
  ena[683] = HAVE_uminv8hi3;
  ena[684] = HAVE_uminvnx8hi3;
  ena[685] = HAVE_uminv4si3;
  ena[686] = HAVE_uminvnx4si3;
  ena[687] = HAVE_uminv2di3;
  ena[688] = HAVE_uminvnx2di3;
  ena[689] = HAVE_umaxsi3;
  ena[690] = HAVE_umaxdi3;
  ena[691] = HAVE_umaxv8qi3;
  ena[692] = HAVE_umaxv4hi3;
  ena[693] = HAVE_umaxv2si3;
  ena[694] = HAVE_umaxv16qi3;
  ena[695] = HAVE_umaxvnx16qi3;
  ena[696] = HAVE_umaxv8hi3;
  ena[697] = HAVE_umaxvnx8hi3;
  ena[698] = HAVE_umaxv4si3;
  ena[699] = HAVE_umaxvnx4si3;
  ena[700] = HAVE_umaxv2di3;
  ena[701] = HAVE_umaxvnx2di3;
  ena[702] = HAVE_negsi2;
  ena[703] = HAVE_negdi2;
  ena[704] = HAVE_neghf2;
  ena[705] = HAVE_negsf2;
  ena[706] = HAVE_negdf2;
  ena[707] = HAVE_negv8qi2;
  ena[708] = HAVE_negv4hi2;
  ena[709] = HAVE_negv2si2;
  ena[710] = HAVE_negv16qi2;
  ena[711] = HAVE_negvnx16qi2;
  ena[712] = HAVE_negv8hi2;
  ena[713] = HAVE_negvnx8hi2;
  ena[714] = HAVE_negv4si2;
  ena[715] = HAVE_negvnx4si2;
  ena[716] = HAVE_negv2di2;
  ena[717] = HAVE_negvnx2di2;
  ena[718] = HAVE_negv4hf2;
  ena[719] = HAVE_negv2sf2;
  ena[720] = HAVE_negv8hf2;
  ena[721] = HAVE_negvnx8hf2;
  ena[722] = HAVE_negv4sf2;
  ena[723] = HAVE_negvnx4sf2;
  ena[724] = HAVE_negv2df2;
  ena[725] = HAVE_negvnx2df2;
  ena[726] = HAVE_abssi2;
  ena[727] = HAVE_absdi2;
  ena[728] = HAVE_abshf2;
  ena[729] = HAVE_abssf2;
  ena[730] = HAVE_absdf2;
  ena[731] = HAVE_absv8qi2;
  ena[732] = HAVE_absv4hi2;
  ena[733] = HAVE_absv2si2;
  ena[734] = HAVE_absv16qi2;
  ena[735] = HAVE_absv8hi2;
  ena[736] = HAVE_absv4si2;
  ena[737] = HAVE_absv2di2;
  ena[738] = HAVE_absv4hf2;
  ena[739] = HAVE_absv2sf2;
  ena[740] = HAVE_absv8hf2;
  ena[741] = HAVE_absvnx8hf2;
  ena[742] = HAVE_absv4sf2;
  ena[743] = HAVE_absvnx4sf2;
  ena[744] = HAVE_absv2df2;
  ena[745] = HAVE_absvnx2df2;
  ena[746] = HAVE_one_cmplsi2;
  ena[747] = HAVE_one_cmpldi2;
  ena[748] = HAVE_one_cmplvnx16bi2;
  ena[749] = HAVE_one_cmplvnx8bi2;
  ena[750] = HAVE_one_cmplvnx4bi2;
  ena[751] = HAVE_one_cmplvnx2bi2;
  ena[752] = HAVE_one_cmplv8qi2;
  ena[753] = HAVE_one_cmplv4hi2;
  ena[754] = HAVE_one_cmplv2si2;
  ena[755] = HAVE_one_cmplv16qi2;
  ena[756] = HAVE_one_cmplvnx16qi2;
  ena[757] = HAVE_one_cmplv8hi2;
  ena[758] = HAVE_one_cmplvnx8hi2;
  ena[759] = HAVE_one_cmplv4si2;
  ena[760] = HAVE_one_cmplvnx4si2;
  ena[761] = HAVE_one_cmplv2di2;
  ena[762] = HAVE_one_cmplvnx2di2;
  ena[763] = HAVE_bswaphi2;
  ena[764] = HAVE_bswapsi2;
  ena[765] = HAVE_bswapdi2;
  ena[766] = HAVE_bswapv4hi2;
  ena[767] = HAVE_bswapv2si2;
  ena[768] = HAVE_bswapv8hi2;
  ena[769] = HAVE_bswapv4si2;
  ena[770] = HAVE_bswapv2di2;
  ena[771] = HAVE_ffssi2;
  ena[772] = HAVE_ffsdi2;
  ena[773] = HAVE_clzsi2;
  ena[774] = HAVE_clzdi2;
  ena[775] = HAVE_clzv8qi2;
  ena[776] = HAVE_clzv4hi2;
  ena[777] = HAVE_clzv2si2;
  ena[778] = HAVE_clzv16qi2;
  ena[779] = HAVE_clzv8hi2;
  ena[780] = HAVE_clzv4si2;
  ena[781] = HAVE_ctzsi2;
  ena[782] = HAVE_ctzdi2;
  ena[783] = HAVE_ctzv2si2;
  ena[784] = HAVE_ctzv4si2;
  ena[785] = HAVE_clrsbsi2;
  ena[786] = HAVE_clrsbdi2;
  ena[787] = HAVE_clrsbv8qi2;
  ena[788] = HAVE_clrsbv4hi2;
  ena[789] = HAVE_clrsbv2si2;
  ena[790] = HAVE_clrsbv16qi2;
  ena[791] = HAVE_clrsbv8hi2;
  ena[792] = HAVE_clrsbv4si2;
  ena[793] = HAVE_popcountsi2;
  ena[794] = HAVE_popcountdi2;
  ena[795] = HAVE_popcountv8qi2;
  ena[796] = HAVE_popcountv16qi2;
  ena[797] = HAVE_popcountvnx16qi2;
  ena[798] = HAVE_popcountvnx8hi2;
  ena[799] = HAVE_popcountvnx4si2;
  ena[800] = HAVE_popcountvnx2di2;
  ena[801] = HAVE_sqrthf2;
  ena[802] = HAVE_sqrtsf2;
  ena[803] = HAVE_sqrtdf2;
  ena[804] = HAVE_sqrtv4hf2;
  ena[805] = HAVE_sqrtv2sf2;
  ena[806] = HAVE_sqrtv8hf2;
  ena[807] = HAVE_sqrtvnx8hf2;
  ena[808] = HAVE_sqrtv4sf2;
  ena[809] = HAVE_sqrtvnx4sf2;
  ena[810] = HAVE_sqrtv2df2;
  ena[811] = HAVE_sqrtvnx2df2;
  ena[812] = HAVE_movqi;
  ena[813] = HAVE_movhi;
  ena[814] = HAVE_movsi;
  ena[815] = HAVE_movdi;
  ena[816] = HAVE_movti;
  ena[817] = HAVE_movoi;
  ena[818] = HAVE_movci;
  ena[819] = HAVE_movxi;
  ena[820] = HAVE_movhf;
  ena[821] = HAVE_movsf;
  ena[822] = HAVE_movdf;
  ena[823] = HAVE_movtf;
  ena[824] = HAVE_movvnx16bi;
  ena[825] = HAVE_movvnx8bi;
  ena[826] = HAVE_movvnx4bi;
  ena[827] = HAVE_movvnx2bi;
  ena[828] = HAVE_movv8qi;
  ena[829] = HAVE_movv4hi;
  ena[830] = HAVE_movv2si;
  ena[831] = HAVE_movv16qi;
  ena[832] = HAVE_movvnx16qi;
  ena[833] = HAVE_movv8hi;
  ena[834] = HAVE_movvnx8hi;
  ena[835] = HAVE_movv4si;
  ena[836] = HAVE_movvnx4si;
  ena[837] = HAVE_movv2di;
  ena[838] = HAVE_movvnx2di;
  ena[839] = HAVE_movvnx32qi;
  ena[840] = HAVE_movvnx16hi;
  ena[841] = HAVE_movvnx8si;
  ena[842] = HAVE_movvnx4di;
  ena[843] = HAVE_movvnx48qi;
  ena[844] = HAVE_movvnx24hi;
  ena[845] = HAVE_movvnx12si;
  ena[846] = HAVE_movvnx6di;
  ena[847] = HAVE_movvnx64qi;
  ena[848] = HAVE_movvnx32hi;
  ena[849] = HAVE_movvnx16si;
  ena[850] = HAVE_movvnx8di;
  ena[851] = HAVE_movv4hf;
  ena[852] = HAVE_movv2sf;
  ena[853] = HAVE_movv8hf;
  ena[854] = HAVE_movvnx8hf;
  ena[855] = HAVE_movv4sf;
  ena[856] = HAVE_movvnx4sf;
  ena[857] = HAVE_movv2df;
  ena[858] = HAVE_movvnx2df;
  ena[859] = HAVE_movvnx16hf;
  ena[860] = HAVE_movvnx8sf;
  ena[861] = HAVE_movvnx4df;
  ena[862] = HAVE_movvnx24hf;
  ena[863] = HAVE_movvnx12sf;
  ena[864] = HAVE_movvnx6df;
  ena[865] = HAVE_movvnx32hf;
  ena[866] = HAVE_movvnx16sf;
  ena[867] = HAVE_movvnx8df;
  ena[868] = HAVE_movmisalignv8qi;
  ena[869] = HAVE_movmisalignv4hi;
  ena[870] = HAVE_movmisalignv2si;
  ena[871] = HAVE_movmisalignv16qi;
  ena[872] = HAVE_movmisalignvnx16qi;
  ena[873] = HAVE_movmisalignv8hi;
  ena[874] = HAVE_movmisalignvnx8hi;
  ena[875] = HAVE_movmisalignv4si;
  ena[876] = HAVE_movmisalignvnx4si;
  ena[877] = HAVE_movmisalignv2di;
  ena[878] = HAVE_movmisalignvnx2di;
  ena[879] = HAVE_movmisalignv2sf;
  ena[880] = HAVE_movmisalignvnx8hf;
  ena[881] = HAVE_movmisalignv4sf;
  ena[882] = HAVE_movmisalignvnx4sf;
  ena[883] = HAVE_movmisalignv2df;
  ena[884] = HAVE_movmisalignvnx2df;
  ena[885] = HAVE_insvsi;
  ena[886] = HAVE_insvdi;
  ena[887] = HAVE_cbranchcc4;
  ena[888] = HAVE_cbranchsi4;
  ena[889] = HAVE_cbranchdi4;
  ena[890] = HAVE_cbranchsf4;
  ena[891] = HAVE_cbranchdf4;
  ena[892] = HAVE_cbranchvnx16bi4;
  ena[893] = HAVE_cbranchvnx8bi4;
  ena[894] = HAVE_cbranchvnx4bi4;
  ena[895] = HAVE_cbranchvnx2bi4;
  ena[896] = HAVE_negsicc;
  ena[897] = HAVE_negdicc;
  ena[898] = HAVE_notsicc;
  ena[899] = HAVE_notdicc;
  ena[900] = HAVE_movqicc;
  ena[901] = HAVE_movhicc;
  ena[902] = HAVE_movsicc;
  ena[903] = HAVE_movdicc;
  ena[904] = HAVE_movsfcc;
  ena[905] = HAVE_movdfcc;
  ena[906] = HAVE_cond_addvnx16qi;
  ena[907] = HAVE_cond_addvnx8hi;
  ena[908] = HAVE_cond_addvnx4si;
  ena[909] = HAVE_cond_addvnx2di;
  ena[910] = HAVE_cond_addvnx8hf;
  ena[911] = HAVE_cond_addvnx4sf;
  ena[912] = HAVE_cond_addvnx2df;
  ena[913] = HAVE_cond_subvnx16qi;
  ena[914] = HAVE_cond_subvnx8hi;
  ena[915] = HAVE_cond_subvnx4si;
  ena[916] = HAVE_cond_subvnx2di;
  ena[917] = HAVE_cond_subvnx8hf;
  ena[918] = HAVE_cond_subvnx4sf;
  ena[919] = HAVE_cond_subvnx2df;
  ena[920] = HAVE_cond_andvnx16qi;
  ena[921] = HAVE_cond_andvnx8hi;
  ena[922] = HAVE_cond_andvnx4si;
  ena[923] = HAVE_cond_andvnx2di;
  ena[924] = HAVE_cond_iorvnx16qi;
  ena[925] = HAVE_cond_iorvnx8hi;
  ena[926] = HAVE_cond_iorvnx4si;
  ena[927] = HAVE_cond_iorvnx2di;
  ena[928] = HAVE_cond_xorvnx16qi;
  ena[929] = HAVE_cond_xorvnx8hi;
  ena[930] = HAVE_cond_xorvnx4si;
  ena[931] = HAVE_cond_xorvnx2di;
  ena[932] = HAVE_cond_sminvnx16qi;
  ena[933] = HAVE_cond_sminvnx8hi;
  ena[934] = HAVE_cond_sminvnx4si;
  ena[935] = HAVE_cond_sminvnx2di;
  ena[936] = HAVE_cond_smaxvnx16qi;
  ena[937] = HAVE_cond_smaxvnx8hi;
  ena[938] = HAVE_cond_smaxvnx4si;
  ena[939] = HAVE_cond_smaxvnx2di;
  ena[940] = HAVE_cond_uminvnx16qi;
  ena[941] = HAVE_cond_uminvnx8hi;
  ena[942] = HAVE_cond_uminvnx4si;
  ena[943] = HAVE_cond_uminvnx2di;
  ena[944] = HAVE_cond_umaxvnx16qi;
  ena[945] = HAVE_cond_umaxvnx8hi;
  ena[946] = HAVE_cond_umaxvnx4si;
  ena[947] = HAVE_cond_umaxvnx2di;
  ena[948] = HAVE_cmovsi6;
  ena[949] = HAVE_cmovdi6;
  ena[950] = HAVE_cmovsf6;
  ena[951] = HAVE_cmovdf6;
  ena[952] = HAVE_cstorecc4;
  ena[953] = HAVE_cstoresi4;
  ena[954] = HAVE_cstoredi4;
  ena[955] = HAVE_cstoresf4;
  ena[956] = HAVE_cstoredf4;
  ena[957] = HAVE_smuldi3_highpart;
  ena[958] = HAVE_smulvnx16qi3_highpart;
  ena[959] = HAVE_smulvnx8hi3_highpart;
  ena[960] = HAVE_smulvnx4si3_highpart;
  ena[961] = HAVE_smulvnx2di3_highpart;
  ena[962] = HAVE_umuldi3_highpart;
  ena[963] = HAVE_umulvnx16qi3_highpart;
  ena[964] = HAVE_umulvnx8hi3_highpart;
  ena[965] = HAVE_umulvnx4si3_highpart;
  ena[966] = HAVE_umulvnx2di3_highpart;
  ena[967] = HAVE_movmemdi;
  ena[968] = HAVE_fmahf4;
  ena[969] = HAVE_fmasf4;
  ena[970] = HAVE_fmadf4;
  ena[971] = HAVE_fmav4hf4;
  ena[972] = HAVE_fmav2sf4;
  ena[973] = HAVE_fmav8hf4;
  ena[974] = HAVE_fmavnx8hf4;
  ena[975] = HAVE_fmav4sf4;
  ena[976] = HAVE_fmavnx4sf4;
  ena[977] = HAVE_fmav2df4;
  ena[978] = HAVE_fmavnx2df4;
  ena[979] = HAVE_fmssf4;
  ena[980] = HAVE_fmsdf4;
  ena[981] = HAVE_fmsvnx8hf4;
  ena[982] = HAVE_fmsvnx4sf4;
  ena[983] = HAVE_fmsvnx2df4;
  ena[984] = HAVE_fnmahf4;
  ena[985] = HAVE_fnmasf4;
  ena[986] = HAVE_fnmadf4;
  ena[987] = HAVE_fnmav4hf4;
  ena[988] = HAVE_fnmav2sf4;
  ena[989] = HAVE_fnmav8hf4;
  ena[990] = HAVE_fnmavnx8hf4;
  ena[991] = HAVE_fnmav4sf4;
  ena[992] = HAVE_fnmavnx4sf4;
  ena[993] = HAVE_fnmav2df4;
  ena[994] = HAVE_fnmavnx2df4;
  ena[995] = HAVE_fnmssf4;
  ena[996] = HAVE_fnmsdf4;
  ena[997] = HAVE_fnmsvnx8hf4;
  ena[998] = HAVE_fnmsvnx4sf4;
  ena[999] = HAVE_fnmsvnx2df4;
  ena[1000] = HAVE_rinthf2;
  ena[1001] = HAVE_rintsf2;
  ena[1002] = HAVE_rintdf2;
  ena[1003] = HAVE_rintv4hf2;
  ena[1004] = HAVE_rintv2sf2;
  ena[1005] = HAVE_rintv8hf2;
  ena[1006] = HAVE_rintvnx8hf2;
  ena[1007] = HAVE_rintv4sf2;
  ena[1008] = HAVE_rintvnx4sf2;
  ena[1009] = HAVE_rintv2df2;
  ena[1010] = HAVE_rintvnx2df2;
  ena[1011] = HAVE_roundhf2;
  ena[1012] = HAVE_roundsf2;
  ena[1013] = HAVE_rounddf2;
  ena[1014] = HAVE_roundv4hf2;
  ena[1015] = HAVE_roundv2sf2;
  ena[1016] = HAVE_roundv8hf2;
  ena[1017] = HAVE_roundvnx8hf2;
  ena[1018] = HAVE_roundv4sf2;
  ena[1019] = HAVE_roundvnx4sf2;
  ena[1020] = HAVE_roundv2df2;
  ena[1021] = HAVE_roundvnx2df2;
  ena[1022] = HAVE_floorhf2;
  ena[1023] = HAVE_floorsf2;
  ena[1024] = HAVE_floordf2;
  ena[1025] = HAVE_floorv4hf2;
  ena[1026] = HAVE_floorv2sf2;
  ena[1027] = HAVE_floorv8hf2;
  ena[1028] = HAVE_floorvnx8hf2;
  ena[1029] = HAVE_floorv4sf2;
  ena[1030] = HAVE_floorvnx4sf2;
  ena[1031] = HAVE_floorv2df2;
  ena[1032] = HAVE_floorvnx2df2;
  ena[1033] = HAVE_ceilhf2;
  ena[1034] = HAVE_ceilsf2;
  ena[1035] = HAVE_ceildf2;
  ena[1036] = HAVE_ceilv4hf2;
  ena[1037] = HAVE_ceilv2sf2;
  ena[1038] = HAVE_ceilv8hf2;
  ena[1039] = HAVE_ceilvnx8hf2;
  ena[1040] = HAVE_ceilv4sf2;
  ena[1041] = HAVE_ceilvnx4sf2;
  ena[1042] = HAVE_ceilv2df2;
  ena[1043] = HAVE_ceilvnx2df2;
  ena[1044] = HAVE_btrunchf2;
  ena[1045] = HAVE_btruncsf2;
  ena[1046] = HAVE_btruncdf2;
  ena[1047] = HAVE_btruncv4hf2;
  ena[1048] = HAVE_btruncv2sf2;
  ena[1049] = HAVE_btruncv8hf2;
  ena[1050] = HAVE_btruncvnx8hf2;
  ena[1051] = HAVE_btruncv4sf2;
  ena[1052] = HAVE_btruncvnx4sf2;
  ena[1053] = HAVE_btruncv2df2;
  ena[1054] = HAVE_btruncvnx2df2;
  ena[1055] = HAVE_nearbyinthf2;
  ena[1056] = HAVE_nearbyintsf2;
  ena[1057] = HAVE_nearbyintdf2;
  ena[1058] = HAVE_nearbyintv4hf2;
  ena[1059] = HAVE_nearbyintv2sf2;
  ena[1060] = HAVE_nearbyintv8hf2;
  ena[1061] = HAVE_nearbyintvnx8hf2;
  ena[1062] = HAVE_nearbyintv4sf2;
  ena[1063] = HAVE_nearbyintvnx4sf2;
  ena[1064] = HAVE_nearbyintv2df2;
  ena[1065] = HAVE_nearbyintvnx2df2;
  ena[1066] = HAVE_copysignsf3;
  ena[1067] = HAVE_copysigndf3;
  ena[1068] = HAVE_copysignv4hf3;
  ena[1069] = HAVE_copysignv2sf3;
  ena[1070] = HAVE_copysignv8hf3;
  ena[1071] = HAVE_copysignv4sf3;
  ena[1072] = HAVE_copysignv2df3;
  ena[1073] = HAVE_xorsignsf3;
  ena[1074] = HAVE_xorsigndf3;
  ena[1075] = HAVE_xorsignv4hf3;
  ena[1076] = HAVE_xorsignv2sf3;
  ena[1077] = HAVE_xorsignv8hf3;
  ena[1078] = HAVE_xorsignv4sf3;
  ena[1079] = HAVE_xorsignv2df3;
  ena[1080] = HAVE_rsqrtsf2;
  ena[1081] = HAVE_rsqrtdf2;
  ena[1082] = HAVE_rsqrtv2sf2;
  ena[1083] = HAVE_rsqrtv4sf2;
  ena[1084] = HAVE_rsqrtv2df2;
  ena[1085] = HAVE_fmaxhf3;
  ena[1086] = HAVE_fmaxsf3;
  ena[1087] = HAVE_fmaxdf3;
  ena[1088] = HAVE_fmaxv4hf3;
  ena[1089] = HAVE_fmaxv2sf3;
  ena[1090] = HAVE_fmaxv8hf3;
  ena[1091] = HAVE_fmaxvnx8hf3;
  ena[1092] = HAVE_fmaxv4sf3;
  ena[1093] = HAVE_fmaxvnx4sf3;
  ena[1094] = HAVE_fmaxv2df3;
  ena[1095] = HAVE_fmaxvnx2df3;
  ena[1096] = HAVE_fminhf3;
  ena[1097] = HAVE_fminsf3;
  ena[1098] = HAVE_fmindf3;
  ena[1099] = HAVE_fminv4hf3;
  ena[1100] = HAVE_fminv2sf3;
  ena[1101] = HAVE_fminv8hf3;
  ena[1102] = HAVE_fminvnx8hf3;
  ena[1103] = HAVE_fminv4sf3;
  ena[1104] = HAVE_fminvnx4sf3;
  ena[1105] = HAVE_fminv2df3;
  ena[1106] = HAVE_fminvnx2df3;
  ena[1107] = HAVE_reduc_smax_scal_v8qi;
  ena[1108] = HAVE_reduc_smax_scal_v4hi;
  ena[1109] = HAVE_reduc_smax_scal_v2si;
  ena[1110] = HAVE_reduc_smax_scal_v16qi;
  ena[1111] = HAVE_reduc_smax_scal_vnx16qi;
  ena[1112] = HAVE_reduc_smax_scal_v8hi;
  ena[1113] = HAVE_reduc_smax_scal_vnx8hi;
  ena[1114] = HAVE_reduc_smax_scal_v4si;
  ena[1115] = HAVE_reduc_smax_scal_vnx4si;
  ena[1116] = HAVE_reduc_smax_scal_vnx2di;
  ena[1117] = HAVE_reduc_smax_scal_v4hf;
  ena[1118] = HAVE_reduc_smax_scal_v2sf;
  ena[1119] = HAVE_reduc_smax_scal_v8hf;
  ena[1120] = HAVE_reduc_smax_scal_vnx8hf;
  ena[1121] = HAVE_reduc_smax_scal_v4sf;
  ena[1122] = HAVE_reduc_smax_scal_vnx4sf;
  ena[1123] = HAVE_reduc_smax_scal_v2df;
  ena[1124] = HAVE_reduc_smax_scal_vnx2df;
  ena[1125] = HAVE_reduc_smin_scal_v8qi;
  ena[1126] = HAVE_reduc_smin_scal_v4hi;
  ena[1127] = HAVE_reduc_smin_scal_v2si;
  ena[1128] = HAVE_reduc_smin_scal_v16qi;
  ena[1129] = HAVE_reduc_smin_scal_vnx16qi;
  ena[1130] = HAVE_reduc_smin_scal_v8hi;
  ena[1131] = HAVE_reduc_smin_scal_vnx8hi;
  ena[1132] = HAVE_reduc_smin_scal_v4si;
  ena[1133] = HAVE_reduc_smin_scal_vnx4si;
  ena[1134] = HAVE_reduc_smin_scal_vnx2di;
  ena[1135] = HAVE_reduc_smin_scal_v4hf;
  ena[1136] = HAVE_reduc_smin_scal_v2sf;
  ena[1137] = HAVE_reduc_smin_scal_v8hf;
  ena[1138] = HAVE_reduc_smin_scal_vnx8hf;
  ena[1139] = HAVE_reduc_smin_scal_v4sf;
  ena[1140] = HAVE_reduc_smin_scal_vnx4sf;
  ena[1141] = HAVE_reduc_smin_scal_v2df;
  ena[1142] = HAVE_reduc_smin_scal_vnx2df;
  ena[1143] = HAVE_reduc_plus_scal_v8qi;
  ena[1144] = HAVE_reduc_plus_scal_v4hi;
  ena[1145] = HAVE_reduc_plus_scal_v2si;
  ena[1146] = HAVE_reduc_plus_scal_v16qi;
  ena[1147] = HAVE_reduc_plus_scal_vnx16qi;
  ena[1148] = HAVE_reduc_plus_scal_v8hi;
  ena[1149] = HAVE_reduc_plus_scal_vnx8hi;
  ena[1150] = HAVE_reduc_plus_scal_v4si;
  ena[1151] = HAVE_reduc_plus_scal_vnx4si;
  ena[1152] = HAVE_reduc_plus_scal_v2di;
  ena[1153] = HAVE_reduc_plus_scal_vnx2di;
  ena[1154] = HAVE_reduc_plus_scal_v2sf;
  ena[1155] = HAVE_reduc_plus_scal_vnx8hf;
  ena[1156] = HAVE_reduc_plus_scal_v4sf;
  ena[1157] = HAVE_reduc_plus_scal_vnx4sf;
  ena[1158] = HAVE_reduc_plus_scal_v2df;
  ena[1159] = HAVE_reduc_plus_scal_vnx2df;
  ena[1160] = HAVE_reduc_umax_scal_v8qi;
  ena[1161] = HAVE_reduc_umax_scal_v4hi;
  ena[1162] = HAVE_reduc_umax_scal_v2si;
  ena[1163] = HAVE_reduc_umax_scal_v16qi;
  ena[1164] = HAVE_reduc_umax_scal_vnx16qi;
  ena[1165] = HAVE_reduc_umax_scal_v8hi;
  ena[1166] = HAVE_reduc_umax_scal_vnx8hi;
  ena[1167] = HAVE_reduc_umax_scal_v4si;
  ena[1168] = HAVE_reduc_umax_scal_vnx4si;
  ena[1169] = HAVE_reduc_umax_scal_vnx2di;
  ena[1170] = HAVE_reduc_umin_scal_v8qi;
  ena[1171] = HAVE_reduc_umin_scal_v4hi;
  ena[1172] = HAVE_reduc_umin_scal_v2si;
  ena[1173] = HAVE_reduc_umin_scal_v16qi;
  ena[1174] = HAVE_reduc_umin_scal_vnx16qi;
  ena[1175] = HAVE_reduc_umin_scal_v8hi;
  ena[1176] = HAVE_reduc_umin_scal_vnx8hi;
  ena[1177] = HAVE_reduc_umin_scal_v4si;
  ena[1178] = HAVE_reduc_umin_scal_vnx4si;
  ena[1179] = HAVE_reduc_umin_scal_vnx2di;
  ena[1180] = HAVE_reduc_and_scal_vnx16qi;
  ena[1181] = HAVE_reduc_and_scal_vnx8hi;
  ena[1182] = HAVE_reduc_and_scal_vnx4si;
  ena[1183] = HAVE_reduc_and_scal_vnx2di;
  ena[1184] = HAVE_reduc_ior_scal_vnx16qi;
  ena[1185] = HAVE_reduc_ior_scal_vnx8hi;
  ena[1186] = HAVE_reduc_ior_scal_vnx4si;
  ena[1187] = HAVE_reduc_ior_scal_vnx2di;
  ena[1188] = HAVE_reduc_xor_scal_vnx16qi;
  ena[1189] = HAVE_reduc_xor_scal_vnx8hi;
  ena[1190] = HAVE_reduc_xor_scal_vnx4si;
  ena[1191] = HAVE_reduc_xor_scal_vnx2di;
  ena[1192] = HAVE_fold_left_plus_vnx8hf;
  ena[1193] = HAVE_fold_left_plus_vnx4sf;
  ena[1194] = HAVE_fold_left_plus_vnx2df;
  ena[1195] = HAVE_extract_last_vnx16qi;
  ena[1196] = HAVE_extract_last_vnx8hi;
  ena[1197] = HAVE_extract_last_vnx4si;
  ena[1198] = HAVE_extract_last_vnx2di;
  ena[1199] = HAVE_extract_last_vnx8hf;
  ena[1200] = HAVE_extract_last_vnx4sf;
  ena[1201] = HAVE_extract_last_vnx2df;
  ena[1202] = HAVE_fold_extract_last_vnx16qi;
  ena[1203] = HAVE_fold_extract_last_vnx8hi;
  ena[1204] = HAVE_fold_extract_last_vnx4si;
  ena[1205] = HAVE_fold_extract_last_vnx2di;
  ena[1206] = HAVE_fold_extract_last_vnx8hf;
  ena[1207] = HAVE_fold_extract_last_vnx4sf;
  ena[1208] = HAVE_fold_extract_last_vnx2df;
  ena[1209] = HAVE_sdot_prodv8qi;
  ena[1210] = HAVE_sdot_prodv16qi;
  ena[1211] = HAVE_widen_ssumv8qi3;
  ena[1212] = HAVE_widen_ssumv4hi3;
  ena[1213] = HAVE_widen_ssumv2si3;
  ena[1214] = HAVE_widen_ssumv16qi3;
  ena[1215] = HAVE_widen_ssumv8hi3;
  ena[1216] = HAVE_widen_ssumv4si3;
  ena[1217] = HAVE_udot_prodv8qi;
  ena[1218] = HAVE_udot_prodv16qi;
  ena[1219] = HAVE_widen_usumv8qi3;
  ena[1220] = HAVE_widen_usumv4hi3;
  ena[1221] = HAVE_widen_usumv2si3;
  ena[1222] = HAVE_widen_usumv16qi3;
  ena[1223] = HAVE_widen_usumv8hi3;
  ena[1224] = HAVE_widen_usumv4si3;
  ena[1225] = HAVE_vec_pack_sfix_trunc_vnx2df;
  ena[1226] = HAVE_vec_pack_trunc_di;
  ena[1227] = HAVE_vec_pack_trunc_df;
  ena[1228] = HAVE_vec_pack_trunc_vnx8bi;
  ena[1229] = HAVE_vec_pack_trunc_vnx4bi;
  ena[1230] = HAVE_vec_pack_trunc_vnx2bi;
  ena[1231] = HAVE_vec_pack_trunc_v4hi;
  ena[1232] = HAVE_vec_pack_trunc_v2si;
  ena[1233] = HAVE_vec_pack_trunc_v8hi;
  ena[1234] = HAVE_vec_pack_trunc_vnx8hi;
  ena[1235] = HAVE_vec_pack_trunc_v4si;
  ena[1236] = HAVE_vec_pack_trunc_vnx4si;
  ena[1237] = HAVE_vec_pack_trunc_v2di;
  ena[1238] = HAVE_vec_pack_trunc_vnx2di;
  ena[1239] = HAVE_vec_pack_trunc_vnx4sf;
  ena[1240] = HAVE_vec_pack_trunc_v2df;
  ena[1241] = HAVE_vec_pack_trunc_vnx2df;
  ena[1242] = HAVE_vec_pack_ufix_trunc_vnx2df;
  ena[1243] = HAVE_vec_permv8qi;
  ena[1244] = HAVE_vec_permv16qi;
  ena[1245] = HAVE_vec_permvnx16qi;
  ena[1246] = HAVE_vec_permvnx8hi;
  ena[1247] = HAVE_vec_permvnx4si;
  ena[1248] = HAVE_vec_permvnx2di;
  ena[1249] = HAVE_vec_permvnx8hf;
  ena[1250] = HAVE_vec_permvnx4sf;
  ena[1251] = HAVE_vec_permvnx2df;
  ena[1252] = HAVE_vec_setv8qi;
  ena[1253] = HAVE_vec_setv4hi;
  ena[1254] = HAVE_vec_setv2si;
  ena[1255] = HAVE_vec_setv16qi;
  ena[1256] = HAVE_vec_setv8hi;
  ena[1257] = HAVE_vec_setv4si;
  ena[1258] = HAVE_vec_setv2di;
  ena[1259] = HAVE_vec_setv4hf;
  ena[1260] = HAVE_vec_setv2sf;
  ena[1261] = HAVE_vec_setv8hf;
  ena[1262] = HAVE_vec_setv4sf;
  ena[1263] = HAVE_vec_setv2df;
  ena[1264] = HAVE_vec_shr_v8qi;
  ena[1265] = HAVE_vec_shr_v4hi;
  ena[1266] = HAVE_vec_shr_v2si;
  ena[1267] = HAVE_vec_shr_v4hf;
  ena[1268] = HAVE_vec_shr_v2sf;
  ena[1269] = HAVE_vec_unpacks_float_hi_vnx4si;
  ena[1270] = HAVE_vec_unpacks_float_lo_vnx4si;
  ena[1271] = HAVE_vec_unpacks_hi_vnx16bi;
  ena[1272] = HAVE_vec_unpacks_hi_vnx8bi;
  ena[1273] = HAVE_vec_unpacks_hi_vnx4bi;
  ena[1274] = HAVE_vec_unpacks_hi_v16qi;
  ena[1275] = HAVE_vec_unpacks_hi_vnx16qi;
  ena[1276] = HAVE_vec_unpacks_hi_v8hi;
  ena[1277] = HAVE_vec_unpacks_hi_vnx8hi;
  ena[1278] = HAVE_vec_unpacks_hi_v4si;
  ena[1279] = HAVE_vec_unpacks_hi_vnx4si;
  ena[1280] = HAVE_vec_unpacks_hi_v8hf;
  ena[1281] = HAVE_vec_unpacks_hi_vnx8hf;
  ena[1282] = HAVE_vec_unpacks_hi_v4sf;
  ena[1283] = HAVE_vec_unpacks_hi_vnx4sf;
  ena[1284] = HAVE_vec_unpacks_lo_vnx16bi;
  ena[1285] = HAVE_vec_unpacks_lo_vnx8bi;
  ena[1286] = HAVE_vec_unpacks_lo_vnx4bi;
  ena[1287] = HAVE_vec_unpacks_lo_v16qi;
  ena[1288] = HAVE_vec_unpacks_lo_vnx16qi;
  ena[1289] = HAVE_vec_unpacks_lo_v8hi;
  ena[1290] = HAVE_vec_unpacks_lo_vnx8hi;
  ena[1291] = HAVE_vec_unpacks_lo_v4si;
  ena[1292] = HAVE_vec_unpacks_lo_vnx4si;
  ena[1293] = HAVE_vec_unpacks_lo_v8hf;
  ena[1294] = HAVE_vec_unpacks_lo_vnx8hf;
  ena[1295] = HAVE_vec_unpacks_lo_v4sf;
  ena[1296] = HAVE_vec_unpacks_lo_vnx4sf;
  ena[1297] = HAVE_vec_unpacku_float_hi_vnx4si;
  ena[1298] = HAVE_vec_unpacku_float_lo_vnx4si;
  ena[1299] = HAVE_vec_unpacku_hi_vnx16bi;
  ena[1300] = HAVE_vec_unpacku_hi_vnx8bi;
  ena[1301] = HAVE_vec_unpacku_hi_vnx4bi;
  ena[1302] = HAVE_vec_unpacku_hi_v16qi;
  ena[1303] = HAVE_vec_unpacku_hi_vnx16qi;
  ena[1304] = HAVE_vec_unpacku_hi_v8hi;
  ena[1305] = HAVE_vec_unpacku_hi_vnx8hi;
  ena[1306] = HAVE_vec_unpacku_hi_v4si;
  ena[1307] = HAVE_vec_unpacku_hi_vnx4si;
  ena[1308] = HAVE_vec_unpacku_lo_vnx16bi;
  ena[1309] = HAVE_vec_unpacku_lo_vnx8bi;
  ena[1310] = HAVE_vec_unpacku_lo_vnx4bi;
  ena[1311] = HAVE_vec_unpacku_lo_v16qi;
  ena[1312] = HAVE_vec_unpacku_lo_vnx16qi;
  ena[1313] = HAVE_vec_unpacku_lo_v8hi;
  ena[1314] = HAVE_vec_unpacku_lo_vnx8hi;
  ena[1315] = HAVE_vec_unpacku_lo_v4si;
  ena[1316] = HAVE_vec_unpacku_lo_vnx4si;
  ena[1317] = HAVE_vec_widen_smult_hi_v16qi;
  ena[1318] = HAVE_vec_widen_smult_hi_v8hi;
  ena[1319] = HAVE_vec_widen_smult_hi_v4si;
  ena[1320] = HAVE_vec_widen_smult_lo_v16qi;
  ena[1321] = HAVE_vec_widen_smult_lo_v8hi;
  ena[1322] = HAVE_vec_widen_smult_lo_v4si;
  ena[1323] = HAVE_vec_widen_umult_hi_v16qi;
  ena[1324] = HAVE_vec_widen_umult_hi_v8hi;
  ena[1325] = HAVE_vec_widen_umult_hi_v4si;
  ena[1326] = HAVE_vec_widen_umult_lo_v16qi;
  ena[1327] = HAVE_vec_widen_umult_lo_v8hi;
  ena[1328] = HAVE_vec_widen_umult_lo_v4si;
  ena[1329] = HAVE_atomic_add_fetchqi;
  ena[1330] = HAVE_atomic_add_fetchhi;
  ena[1331] = HAVE_atomic_add_fetchsi;
  ena[1332] = HAVE_atomic_add_fetchdi;
  ena[1333] = HAVE_atomic_addqi;
  ena[1334] = HAVE_atomic_addhi;
  ena[1335] = HAVE_atomic_addsi;
  ena[1336] = HAVE_atomic_adddi;
  ena[1337] = HAVE_atomic_and_fetchqi;
  ena[1338] = HAVE_atomic_and_fetchhi;
  ena[1339] = HAVE_atomic_and_fetchsi;
  ena[1340] = HAVE_atomic_and_fetchdi;
  ena[1341] = HAVE_atomic_andqi;
  ena[1342] = HAVE_atomic_andhi;
  ena[1343] = HAVE_atomic_andsi;
  ena[1344] = HAVE_atomic_anddi;
  ena[1345] = HAVE_atomic_compare_and_swapqi;
  ena[1346] = HAVE_atomic_compare_and_swaphi;
  ena[1347] = HAVE_atomic_compare_and_swapsi;
  ena[1348] = HAVE_atomic_compare_and_swapdi;
  ena[1349] = HAVE_atomic_exchangeqi;
  ena[1350] = HAVE_atomic_exchangehi;
  ena[1351] = HAVE_atomic_exchangesi;
  ena[1352] = HAVE_atomic_exchangedi;
  ena[1353] = HAVE_atomic_fetch_addqi;
  ena[1354] = HAVE_atomic_fetch_addhi;
  ena[1355] = HAVE_atomic_fetch_addsi;
  ena[1356] = HAVE_atomic_fetch_adddi;
  ena[1357] = HAVE_atomic_fetch_andqi;
  ena[1358] = HAVE_atomic_fetch_andhi;
  ena[1359] = HAVE_atomic_fetch_andsi;
  ena[1360] = HAVE_atomic_fetch_anddi;
  ena[1361] = HAVE_atomic_fetch_nandqi;
  ena[1362] = HAVE_atomic_fetch_nandhi;
  ena[1363] = HAVE_atomic_fetch_nandsi;
  ena[1364] = HAVE_atomic_fetch_nanddi;
  ena[1365] = HAVE_atomic_fetch_orqi;
  ena[1366] = HAVE_atomic_fetch_orhi;
  ena[1367] = HAVE_atomic_fetch_orsi;
  ena[1368] = HAVE_atomic_fetch_ordi;
  ena[1369] = HAVE_atomic_fetch_subqi;
  ena[1370] = HAVE_atomic_fetch_subhi;
  ena[1371] = HAVE_atomic_fetch_subsi;
  ena[1372] = HAVE_atomic_fetch_subdi;
  ena[1373] = HAVE_atomic_fetch_xorqi;
  ena[1374] = HAVE_atomic_fetch_xorhi;
  ena[1375] = HAVE_atomic_fetch_xorsi;
  ena[1376] = HAVE_atomic_fetch_xordi;
  ena[1377] = HAVE_atomic_loadqi;
  ena[1378] = HAVE_atomic_loadhi;
  ena[1379] = HAVE_atomic_loadsi;
  ena[1380] = HAVE_atomic_loaddi;
  ena[1381] = HAVE_atomic_nand_fetchqi;
  ena[1382] = HAVE_atomic_nand_fetchhi;
  ena[1383] = HAVE_atomic_nand_fetchsi;
  ena[1384] = HAVE_atomic_nand_fetchdi;
  ena[1385] = HAVE_atomic_nandqi;
  ena[1386] = HAVE_atomic_nandhi;
  ena[1387] = HAVE_atomic_nandsi;
  ena[1388] = HAVE_atomic_nanddi;
  ena[1389] = HAVE_atomic_or_fetchqi;
  ena[1390] = HAVE_atomic_or_fetchhi;
  ena[1391] = HAVE_atomic_or_fetchsi;
  ena[1392] = HAVE_atomic_or_fetchdi;
  ena[1393] = HAVE_atomic_orqi;
  ena[1394] = HAVE_atomic_orhi;
  ena[1395] = HAVE_atomic_orsi;
  ena[1396] = HAVE_atomic_ordi;
  ena[1397] = HAVE_atomic_storeqi;
  ena[1398] = HAVE_atomic_storehi;
  ena[1399] = HAVE_atomic_storesi;
  ena[1400] = HAVE_atomic_storedi;
  ena[1401] = HAVE_atomic_sub_fetchqi;
  ena[1402] = HAVE_atomic_sub_fetchhi;
  ena[1403] = HAVE_atomic_sub_fetchsi;
  ena[1404] = HAVE_atomic_sub_fetchdi;
  ena[1405] = HAVE_atomic_subqi;
  ena[1406] = HAVE_atomic_subhi;
  ena[1407] = HAVE_atomic_subsi;
  ena[1408] = HAVE_atomic_subdi;
  ena[1409] = HAVE_atomic_xor_fetchqi;
  ena[1410] = HAVE_atomic_xor_fetchhi;
  ena[1411] = HAVE_atomic_xor_fetchsi;
  ena[1412] = HAVE_atomic_xor_fetchdi;
  ena[1413] = HAVE_atomic_xorqi;
  ena[1414] = HAVE_atomic_xorhi;
  ena[1415] = HAVE_atomic_xorsi;
  ena[1416] = HAVE_atomic_xordi;
  ena[1417] = HAVE_get_thread_pointerdi;
  ena[1418] = HAVE_gather_loadvnx4si;
  ena[1419] = HAVE_gather_loadvnx2di;
  ena[1420] = HAVE_gather_loadvnx4sf;
  ena[1421] = HAVE_gather_loadvnx2df;
  ena[1422] = HAVE_mask_gather_loadvnx4si;
  ena[1423] = HAVE_mask_gather_loadvnx2di;
  ena[1424] = HAVE_mask_gather_loadvnx4sf;
  ena[1425] = HAVE_mask_gather_loadvnx2df;
  ena[1426] = HAVE_scatter_storevnx4si;
  ena[1427] = HAVE_scatter_storevnx2di;
  ena[1428] = HAVE_scatter_storevnx4sf;
  ena[1429] = HAVE_scatter_storevnx2df;
  ena[1430] = HAVE_mask_scatter_storevnx4si;
  ena[1431] = HAVE_mask_scatter_storevnx2di;
  ena[1432] = HAVE_mask_scatter_storevnx4sf;
  ena[1433] = HAVE_mask_scatter_storevnx2df;
  ena[1434] = HAVE_vec_duplicatevnx16bi;
  ena[1435] = HAVE_vec_duplicatevnx8bi;
  ena[1436] = HAVE_vec_duplicatevnx4bi;
  ena[1437] = HAVE_vec_duplicatevnx2bi;
  ena[1438] = HAVE_vec_duplicatevnx16qi;
  ena[1439] = HAVE_vec_duplicatevnx8hi;
  ena[1440] = HAVE_vec_duplicatevnx4si;
  ena[1441] = HAVE_vec_duplicatevnx2di;
  ena[1442] = HAVE_vec_duplicatevnx8hf;
  ena[1443] = HAVE_vec_duplicatevnx4sf;
  ena[1444] = HAVE_vec_duplicatevnx2df;
  ena[1445] = HAVE_vec_seriesvnx16qi;
  ena[1446] = HAVE_vec_seriesvnx8hi;
  ena[1447] = HAVE_vec_seriesvnx4si;
  ena[1448] = HAVE_vec_seriesvnx2di;
  ena[1449] = HAVE_vec_shl_insert_vnx16qi;
  ena[1450] = HAVE_vec_shl_insert_vnx8hi;
  ena[1451] = HAVE_vec_shl_insert_vnx4si;
  ena[1452] = HAVE_vec_shl_insert_vnx2di;
  ena[1453] = HAVE_vec_shl_insert_vnx8hf;
  ena[1454] = HAVE_vec_shl_insert_vnx4sf;
  ena[1455] = HAVE_vec_shl_insert_vnx2df;
}

static int
lookup_handler (unsigned scode)
{
  int l = 0, h = ARRAY_SIZE (pats), m;
  while (h > l)
    {
      m = (h + l) / 2;
      if (scode == pats[m].scode)
        return m;
      else if (scode < pats[m].scode)
        h = m;
      else
        l = m + 1;
    }
  return -1;
}

enum insn_code
raw_optab_handler (unsigned scode)
{
  int i = lookup_handler (scode);
  return (i >= 0 && this_fn_optabs->pat_enable[i]
          ? pats[i].icode : CODE_FOR_nothing);
}

bool
swap_optab_enable (optab op, machine_mode m, bool set)
{
  unsigned scode = (op << 16) | m;
  int i = lookup_handler (scode);
  if (i >= 0)
    {
      bool ret = this_fn_optabs->pat_enable[i];
      this_fn_optabs->pat_enable[i] = set;
      return ret;
    }
  else
    {
      gcc_assert (!set);
      return false;
    }
}

const struct convert_optab_libcall_d convlib_def[NUM_CONVLIB_OPTABS] = {
  { "extend", gen_extend_conv_libfunc },
  { "trunc", gen_trunc_conv_libfunc },
  { NULL, NULL },
  { "fix", gen_fp_to_int_conv_libfunc },
  { "fixuns", gen_fp_to_int_conv_libfunc },
  { "float", gen_int_to_fp_conv_libfunc },
  { NULL, gen_ufloat_conv_libfunc },
  { "lrint", gen_int_to_fp_nondecimal_conv_libfunc },
  { "lround", gen_int_to_fp_nondecimal_conv_libfunc },
  { "lfloor", gen_int_to_fp_nondecimal_conv_libfunc },
  { "lceil", gen_int_to_fp_nondecimal_conv_libfunc },
  { "fract", gen_fract_conv_libfunc },
  { "fractuns", gen_fractuns_conv_libfunc },
  { "satfract", gen_satfract_conv_libfunc },
  { "satfractuns", gen_satfractuns_conv_libfunc },
};

const struct optab_libcall_d normlib_def[NUM_NORMLIB_OPTABS] = {
  { '3', "add", gen_int_fp_fixed_libfunc },
  { '3', "add", gen_intv_fp_libfunc },
  { '3', "ssadd", gen_signed_fixed_libfunc },
  { '3', "usadd", gen_unsigned_fixed_libfunc },
  { '3', "sub", gen_int_fp_fixed_libfunc },
  { '3', "sub", gen_intv_fp_libfunc },
  { '3', "sssub", gen_signed_fixed_libfunc },
  { '3', "ussub", gen_unsigned_fixed_libfunc },
  { '3', "mul", gen_int_fp_fixed_libfunc },
  { '3', "mul", gen_intv_fp_libfunc },
  { '3', "ssmul", gen_signed_fixed_libfunc },
  { '3', "usmul", gen_unsigned_fixed_libfunc },
  { '3', "div", gen_int_fp_signed_fixed_libfunc },
  { '3', "divv", gen_int_libfunc },
  { '3', "ssdiv", gen_signed_fixed_libfunc },
  { '3', "udiv", gen_int_unsigned_fixed_libfunc },
  { '3', "usdiv", gen_unsigned_fixed_libfunc },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '3', "mod", gen_int_libfunc },
  { '3', "umod", gen_int_libfunc },
  { '2', "ftrunc", gen_fp_libfunc },
  { '3', "and", gen_int_libfunc },
  { '3', "ior", gen_int_libfunc },
  { '3', "xor", gen_int_libfunc },
  { '3', "ashl", gen_int_fixed_libfunc },
  { '3', "ssashl", gen_signed_fixed_libfunc },
  { '3', "usashl", gen_unsigned_fixed_libfunc },
  { '3', "ashr", gen_int_signed_fixed_libfunc },
  { '3', "lshr", gen_int_unsigned_fixed_libfunc },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '3', "min", gen_int_fp_libfunc },
  { '3', "max", gen_int_fp_libfunc },
  { '3', "umin", gen_int_libfunc },
  { '3', "umax", gen_int_libfunc },
  { '2', "neg", gen_int_fp_fixed_libfunc },
  { '2', "neg", gen_intv_fp_libfunc },
  { '2', "ssneg", gen_signed_fixed_libfunc },
  { '2', "usneg", gen_unsigned_fixed_libfunc },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '2', "one_cmpl", gen_int_libfunc },
  { '\0', NULL, NULL },
  { '2', "ffs", gen_int_libfunc },
  { '2', "clz", gen_int_libfunc },
  { '2', "ctz", gen_int_libfunc },
  { '2', "clrsb", gen_int_libfunc },
  { '2', "popcount", gen_int_libfunc },
  { '2', "parity", gen_int_libfunc },
  { '2', "cmp", gen_int_fp_fixed_libfunc },
  { '2', "ucmp", gen_int_libfunc },
  { '2', "eq", gen_fp_libfunc },
  { '2', "ne", gen_fp_libfunc },
  { '2', "gt", gen_fp_libfunc },
  { '2', "ge", gen_fp_libfunc },
  { '2', "lt", gen_fp_libfunc },
  { '2', "le", gen_fp_libfunc },
  { '2', "unord", gen_fp_libfunc },
  { '2', "powi", gen_fp_libfunc },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
  { '\0', NULL, NULL },
};

enum rtx_code const optab_to_code_[NUM_OPTABS] = {
  UNKNOWN,
  SIGN_EXTEND,
  TRUNCATE,
  ZERO_EXTEND,
  FIX,
  UNSIGNED_FIX,
  FLOAT,
  UNSIGNED_FLOAT,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  FRACT_CONVERT,
  UNSIGNED_FRACT_CONVERT,
  SAT_FRACT,
  UNSIGNED_SAT_FRACT,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  PLUS,
  PLUS,
  SS_PLUS,
  US_PLUS,
  MINUS,
  MINUS,
  SS_MINUS,
  US_MINUS,
  MULT,
  MULT,
  SS_MULT,
  US_MULT,
  DIV,
  DIV,
  SS_DIV,
  UDIV,
  US_DIV,
  UNKNOWN,
  UNKNOWN,
  MOD,
  UMOD,
  UNKNOWN,
  AND,
  IOR,
  XOR,
  ASHIFT,
  SS_ASHIFT,
  US_ASHIFT,
  ASHIFTRT,
  LSHIFTRT,
  ROTATE,
  ROTATERT,
  ASHIFT,
  ASHIFTRT,
  LSHIFTRT,
  ROTATE,
  ROTATERT,
  SMIN,
  SMAX,
  UMIN,
  UMAX,
  NEG,
  NEG,
  SS_NEG,
  US_NEG,
  ABS,
  ABS,
  NOT,
  BSWAP,
  FFS,
  CLZ,
  CTZ,
  CLRSB,
  POPCOUNT,
  PARITY,
  UNKNOWN,
  UNKNOWN,
  EQ,
  NE,
  GT,
  GE,
  LT,
  LE,
  UNORDERED,
  UNKNOWN,
  SQRT,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  SET,
  STRICT_LOW_PART,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  COMPARE,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  FMA,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  UNKNOWN,
  VEC_DUPLICATE,
  VEC_SERIES,
  UNKNOWN,
};

const optab code_to_optab_[NUM_RTX_CODE] = {
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  mov_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  movstrict_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  cbranch_optab,
  add_optab,
  sub_optab,
  neg_optab,
  smul_optab,
  ssmul_optab,
  usmul_optab,
  sdiv_optab,
  ssdiv_optab,
  usdiv_optab,
  smod_optab,
  udiv_optab,
  umod_optab,
  and_optab,
  ior_optab,
  xor_optab,
  one_cmpl_optab,
  ashl_optab,
  rotl_optab,
  ashr_optab,
  lshr_optab,
  rotr_optab,
  smin_optab,
  smax_optab,
  umin_optab,
  umax_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  ne_optab,
  eq_optab,
  ge_optab,
  gt_optab,
  le_optab,
  lt_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unord_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  abs_optab,
  sqrt_optab,
  bswap_optab,
  ffs_optab,
  clrsb_optab,
  clz_optab,
  ctz_optab,
  popcount_optab,
  parity_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  vec_duplicate_optab,
  vec_series_optab,
  ssadd_optab,
  usadd_optab,
  sssub_optab,
  ssneg_optab,
  usneg_optab,
  unknown_optab,
  ssashl_optab,
  usashl_optab,
  ussub_optab,
  unknown_optab,
  unknown_optab,
  fma_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
  unknown_optab,
};

