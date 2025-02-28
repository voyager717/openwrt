/* Generated automatically from machmode.def and config/aarch64/aarch64-modes.def
   by genmodes.  */

#include "bconfig.h"
#include "system.h"
#include "coretypes.h"

const char *const mode_name[NUM_MACHINE_MODES] =
{
  "VOID",
  "BLK",
  "CC",
  "CCFP",
  "CCFPE",
  "CC_SWP",
  "CC_NZ",
  "CC_Z",
  "CC_C",
  "BI",
  "QI",
  "HI",
  "SI",
  "DI",
  "TI",
  "OI",
  "CI",
  "XI",
  "QQ",
  "HQ",
  "SQ",
  "DQ",
  "TQ",
  "UQQ",
  "UHQ",
  "USQ",
  "UDQ",
  "UTQ",
  "HA",
  "SA",
  "DA",
  "TA",
  "UHA",
  "USA",
  "UDA",
  "UTA",
  "HF",
  "SF",
  "DF",
  "TF",
  "SD",
  "DD",
  "TD",
  "CQI",
  "CHI",
  "CSI",
  "CDI",
  "CTI",
  "COI",
  "CCI",
  "CXI",
  "HC",
  "SC",
  "DC",
  "TC",
  "VNx16BI",
  "VNx8BI",
  "VNx4BI",
  "VNx2BI",
  "V8QI",
  "V4HI",
  "V2SI",
  "V16QI",
  "VNx16QI",
  "V8HI",
  "VNx8HI",
  "V4SI",
  "VNx4SI",
  "V2DI",
  "VNx2DI",
  "VNx32QI",
  "VNx16HI",
  "VNx8SI",
  "VNx4DI",
  "VNx2TI",
  "VNx48QI",
  "VNx24HI",
  "VNx12SI",
  "VNx6DI",
  "VNx3TI",
  "VNx64QI",
  "VNx32HI",
  "VNx16SI",
  "VNx8DI",
  "VNx4TI",
  "VNx2OI",
  "V2HF",
  "V4HF",
  "V2SF",
  "V1DF",
  "V8HF",
  "VNx8HF",
  "V4SF",
  "VNx4SF",
  "V2DF",
  "VNx2DF",
  "VNx16HF",
  "VNx8SF",
  "VNx4DF",
  "VNx24HF",
  "VNx12SF",
  "VNx6DF",
  "VNx32HF",
  "VNx16SF",
  "VNx8DF",
};

const unsigned char mode_class[NUM_MACHINE_MODES] =
{
  MODE_RANDOM,             /* VOID */
  MODE_RANDOM,             /* BLK */
  MODE_CC,                 /* CC */
  MODE_CC,                 /* CCFP */
  MODE_CC,                 /* CCFPE */
  MODE_CC,                 /* CC_SWP */
  MODE_CC,                 /* CC_NZ */
  MODE_CC,                 /* CC_Z */
  MODE_CC,                 /* CC_C */
  MODE_INT,                /* BI */
  MODE_INT,                /* QI */
  MODE_INT,                /* HI */
  MODE_INT,                /* SI */
  MODE_INT,                /* DI */
  MODE_INT,                /* TI */
  MODE_INT,                /* OI */
  MODE_INT,                /* CI */
  MODE_INT,                /* XI */
  MODE_FRACT,              /* QQ */
  MODE_FRACT,              /* HQ */
  MODE_FRACT,              /* SQ */
  MODE_FRACT,              /* DQ */
  MODE_FRACT,              /* TQ */
  MODE_UFRACT,             /* UQQ */
  MODE_UFRACT,             /* UHQ */
  MODE_UFRACT,             /* USQ */
  MODE_UFRACT,             /* UDQ */
  MODE_UFRACT,             /* UTQ */
  MODE_ACCUM,              /* HA */
  MODE_ACCUM,              /* SA */
  MODE_ACCUM,              /* DA */
  MODE_ACCUM,              /* TA */
  MODE_UACCUM,             /* UHA */
  MODE_UACCUM,             /* USA */
  MODE_UACCUM,             /* UDA */
  MODE_UACCUM,             /* UTA */
  MODE_FLOAT,              /* HF */
  MODE_FLOAT,              /* SF */
  MODE_FLOAT,              /* DF */
  MODE_FLOAT,              /* TF */
  MODE_DECIMAL_FLOAT,      /* SD */
  MODE_DECIMAL_FLOAT,      /* DD */
  MODE_DECIMAL_FLOAT,      /* TD */
  MODE_COMPLEX_INT,        /* CQI */
  MODE_COMPLEX_INT,        /* CHI */
  MODE_COMPLEX_INT,        /* CSI */
  MODE_COMPLEX_INT,        /* CDI */
  MODE_COMPLEX_INT,        /* CTI */
  MODE_COMPLEX_INT,        /* COI */
  MODE_COMPLEX_INT,        /* CCI */
  MODE_COMPLEX_INT,        /* CXI */
  MODE_COMPLEX_FLOAT,      /* HC */
  MODE_COMPLEX_FLOAT,      /* SC */
  MODE_COMPLEX_FLOAT,      /* DC */
  MODE_COMPLEX_FLOAT,      /* TC */
  MODE_VECTOR_BOOL,        /* VNx16BI */
  MODE_VECTOR_BOOL,        /* VNx8BI */
  MODE_VECTOR_BOOL,        /* VNx4BI */
  MODE_VECTOR_BOOL,        /* VNx2BI */
  MODE_VECTOR_INT,         /* V8QI */
  MODE_VECTOR_INT,         /* V4HI */
  MODE_VECTOR_INT,         /* V2SI */
  MODE_VECTOR_INT,         /* V16QI */
  MODE_VECTOR_INT,         /* VNx16QI */
  MODE_VECTOR_INT,         /* V8HI */
  MODE_VECTOR_INT,         /* VNx8HI */
  MODE_VECTOR_INT,         /* V4SI */
  MODE_VECTOR_INT,         /* VNx4SI */
  MODE_VECTOR_INT,         /* V2DI */
  MODE_VECTOR_INT,         /* VNx2DI */
  MODE_VECTOR_INT,         /* VNx32QI */
  MODE_VECTOR_INT,         /* VNx16HI */
  MODE_VECTOR_INT,         /* VNx8SI */
  MODE_VECTOR_INT,         /* VNx4DI */
  MODE_VECTOR_INT,         /* VNx2TI */
  MODE_VECTOR_INT,         /* VNx48QI */
  MODE_VECTOR_INT,         /* VNx24HI */
  MODE_VECTOR_INT,         /* VNx12SI */
  MODE_VECTOR_INT,         /* VNx6DI */
  MODE_VECTOR_INT,         /* VNx3TI */
  MODE_VECTOR_INT,         /* VNx64QI */
  MODE_VECTOR_INT,         /* VNx32HI */
  MODE_VECTOR_INT,         /* VNx16SI */
  MODE_VECTOR_INT,         /* VNx8DI */
  MODE_VECTOR_INT,         /* VNx4TI */
  MODE_VECTOR_INT,         /* VNx2OI */
  MODE_VECTOR_FLOAT,       /* V2HF */
  MODE_VECTOR_FLOAT,       /* V4HF */
  MODE_VECTOR_FLOAT,       /* V2SF */
  MODE_VECTOR_FLOAT,       /* V1DF */
  MODE_VECTOR_FLOAT,       /* V8HF */
  MODE_VECTOR_FLOAT,       /* VNx8HF */
  MODE_VECTOR_FLOAT,       /* V4SF */
  MODE_VECTOR_FLOAT,       /* VNx4SF */
  MODE_VECTOR_FLOAT,       /* V2DF */
  MODE_VECTOR_FLOAT,       /* VNx2DF */
  MODE_VECTOR_FLOAT,       /* VNx16HF */
  MODE_VECTOR_FLOAT,       /* VNx8SF */
  MODE_VECTOR_FLOAT,       /* VNx4DF */
  MODE_VECTOR_FLOAT,       /* VNx24HF */
  MODE_VECTOR_FLOAT,       /* VNx12SF */
  MODE_VECTOR_FLOAT,       /* VNx6DF */
  MODE_VECTOR_FLOAT,       /* VNx32HF */
  MODE_VECTOR_FLOAT,       /* VNx16SF */
  MODE_VECTOR_FLOAT,       /* VNx8DF */
};

poly_uint16_pod mode_nunits[NUM_MACHINE_MODES] = 
{
  { 0, 0 },                /* VOID */
  { 0, 0 },                /* BLK */
  { 1, 0 },                /* CC */
  { 1, 0 },                /* CCFP */
  { 1, 0 },                /* CCFPE */
  { 1, 0 },                /* CC_SWP */
  { 1, 0 },                /* CC_NZ */
  { 1, 0 },                /* CC_Z */
  { 1, 0 },                /* CC_C */
  { 1, 0 },                /* BI */
  { 1, 0 },                /* QI */
  { 1, 0 },                /* HI */
  { 1, 0 },                /* SI */
  { 1, 0 },                /* DI */
  { 1, 0 },                /* TI */
  { 1, 0 },                /* OI */
  { 1, 0 },                /* CI */
  { 1, 0 },                /* XI */
  { 1, 0 },                /* QQ */
  { 1, 0 },                /* HQ */
  { 1, 0 },                /* SQ */
  { 1, 0 },                /* DQ */
  { 1, 0 },                /* TQ */
  { 1, 0 },                /* UQQ */
  { 1, 0 },                /* UHQ */
  { 1, 0 },                /* USQ */
  { 1, 0 },                /* UDQ */
  { 1, 0 },                /* UTQ */
  { 1, 0 },                /* HA */
  { 1, 0 },                /* SA */
  { 1, 0 },                /* DA */
  { 1, 0 },                /* TA */
  { 1, 0 },                /* UHA */
  { 1, 0 },                /* USA */
  { 1, 0 },                /* UDA */
  { 1, 0 },                /* UTA */
  { 1, 0 },                /* HF */
  { 1, 0 },                /* SF */
  { 1, 0 },                /* DF */
  { 1, 0 },                /* TF */
  { 1, 0 },                /* SD */
  { 1, 0 },                /* DD */
  { 1, 0 },                /* TD */
  { 2, 0 },                /* CQI */
  { 2, 0 },                /* CHI */
  { 2, 0 },                /* CSI */
  { 2, 0 },                /* CDI */
  { 2, 0 },                /* CTI */
  { 2, 0 },                /* COI */
  { 2, 0 },                /* CCI */
  { 2, 0 },                /* CXI */
  { 2, 0 },                /* HC */
  { 2, 0 },                /* SC */
  { 2, 0 },                /* DC */
  { 2, 0 },                /* TC */
  { 16, 0 },               /* VNx16BI */
  { 8, 0 },                /* VNx8BI */
  { 4, 0 },                /* VNx4BI */
  { 2, 0 },                /* VNx2BI */
  { 8, 0 },                /* V8QI */
  { 4, 0 },                /* V4HI */
  { 2, 0 },                /* V2SI */
  { 16, 0 },               /* V16QI */
  { 16, 0 },               /* VNx16QI */
  { 8, 0 },                /* V8HI */
  { 8, 0 },                /* VNx8HI */
  { 4, 0 },                /* V4SI */
  { 4, 0 },                /* VNx4SI */
  { 2, 0 },                /* V2DI */
  { 2, 0 },                /* VNx2DI */
  { 32, 0 },               /* VNx32QI */
  { 16, 0 },               /* VNx16HI */
  { 8, 0 },                /* VNx8SI */
  { 4, 0 },                /* VNx4DI */
  { 2, 0 },                /* VNx2TI */
  { 48, 0 },               /* VNx48QI */
  { 24, 0 },               /* VNx24HI */
  { 12, 0 },               /* VNx12SI */
  { 6, 0 },                /* VNx6DI */
  { 3, 0 },                /* VNx3TI */
  { 64, 0 },               /* VNx64QI */
  { 32, 0 },               /* VNx32HI */
  { 16, 0 },               /* VNx16SI */
  { 8, 0 },                /* VNx8DI */
  { 4, 0 },                /* VNx4TI */
  { 2, 0 },                /* VNx2OI */
  { 2, 0 },                /* V2HF */
  { 4, 0 },                /* V4HF */
  { 2, 0 },                /* V2SF */
  { 1, 0 },                /* V1DF */
  { 8, 0 },                /* V8HF */
  { 8, 0 },                /* VNx8HF */
  { 4, 0 },                /* V4SF */
  { 4, 0 },                /* VNx4SF */
  { 2, 0 },                /* V2DF */
  { 2, 0 },                /* VNx2DF */
  { 16, 0 },               /* VNx16HF */
  { 8, 0 },                /* VNx8SF */
  { 4, 0 },                /* VNx4DF */
  { 24, 0 },               /* VNx24HF */
  { 12, 0 },               /* VNx12SF */
  { 6, 0 },                /* VNx6DF */
  { 32, 0 },               /* VNx32HF */
  { 16, 0 },               /* VNx16SF */
  { 8, 0 },                /* VNx8DF */
};

const unsigned char mode_wider[NUM_MACHINE_MODES] =
{
  E_VOIDmode,              /* VOID */
  E_VOIDmode,              /* BLK */
  E_VOIDmode,              /* CC */
  E_VOIDmode,              /* CCFP */
  E_VOIDmode,              /* CCFPE */
  E_VOIDmode,              /* CC_SWP */
  E_VOIDmode,              /* CC_NZ */
  E_VOIDmode,              /* CC_Z */
  E_VOIDmode,              /* CC_C */
  E_QImode,                /* BI */
  E_HImode,                /* QI */
  E_SImode,                /* HI */
  E_DImode,                /* SI */
  E_TImode,                /* DI */
  E_OImode,                /* TI */
  E_CImode,                /* OI */
  E_XImode,                /* CI */
  E_VOIDmode,              /* XI */
  E_HQmode,                /* QQ */
  E_SQmode,                /* HQ */
  E_DQmode,                /* SQ */
  E_TQmode,                /* DQ */
  E_VOIDmode,              /* TQ */
  E_UHQmode,               /* UQQ */
  E_USQmode,               /* UHQ */
  E_UDQmode,               /* USQ */
  E_UTQmode,               /* UDQ */
  E_VOIDmode,              /* UTQ */
  E_SAmode,                /* HA */
  E_DAmode,                /* SA */
  E_TAmode,                /* DA */
  E_VOIDmode,              /* TA */
  E_USAmode,               /* UHA */
  E_UDAmode,               /* USA */
  E_UTAmode,               /* UDA */
  E_VOIDmode,              /* UTA */
  E_SFmode,                /* HF */
  E_DFmode,                /* SF */
  E_TFmode,                /* DF */
  E_VOIDmode,              /* TF */
  E_DDmode,                /* SD */
  E_TDmode,                /* DD */
  E_VOIDmode,              /* TD */
  E_CHImode,               /* CQI */
  E_CSImode,               /* CHI */
  E_CDImode,               /* CSI */
  E_CTImode,               /* CDI */
  E_COImode,               /* CTI */
  E_CCImode,               /* COI */
  E_CXImode,               /* CCI */
  E_VOIDmode,              /* CXI */
  E_SCmode,                /* HC */
  E_DCmode,                /* SC */
  E_TCmode,                /* DC */
  E_VOIDmode,              /* TC */
  E_VNx8BImode,            /* VNx16BI */
  E_VNx4BImode,            /* VNx8BI */
  E_VNx2BImode,            /* VNx4BI */
  E_VOIDmode,              /* VNx2BI */
  E_V4HImode,              /* V8QI */
  E_V2SImode,              /* V4HI */
  E_V16QImode,             /* V2SI */
  E_VNx16QImode,           /* V16QI */
  E_V8HImode,              /* VNx16QI */
  E_VNx8HImode,            /* V8HI */
  E_V4SImode,              /* VNx8HI */
  E_VNx4SImode,            /* V4SI */
  E_V2DImode,              /* VNx4SI */
  E_VNx2DImode,            /* V2DI */
  E_VNx32QImode,           /* VNx2DI */
  E_VNx16HImode,           /* VNx32QI */
  E_VNx8SImode,            /* VNx16HI */
  E_VNx4DImode,            /* VNx8SI */
  E_VNx2TImode,            /* VNx4DI */
  E_VNx48QImode,           /* VNx2TI */
  E_VNx24HImode,           /* VNx48QI */
  E_VNx12SImode,           /* VNx24HI */
  E_VNx6DImode,            /* VNx12SI */
  E_VNx3TImode,            /* VNx6DI */
  E_VNx64QImode,           /* VNx3TI */
  E_VNx32HImode,           /* VNx64QI */
  E_VNx16SImode,           /* VNx32HI */
  E_VNx8DImode,            /* VNx16SI */
  E_VNx4TImode,            /* VNx8DI */
  E_VNx2OImode,            /* VNx4TI */
  E_VOIDmode,              /* VNx2OI */
  E_V4HFmode,              /* V2HF */
  E_V2SFmode,              /* V4HF */
  E_V1DFmode,              /* V2SF */
  E_V8HFmode,              /* V1DF */
  E_VNx8HFmode,            /* V8HF */
  E_V4SFmode,              /* VNx8HF */
  E_VNx4SFmode,            /* V4SF */
  E_V2DFmode,              /* VNx4SF */
  E_VNx2DFmode,            /* V2DF */
  E_VNx16HFmode,           /* VNx2DF */
  E_VNx8SFmode,            /* VNx16HF */
  E_VNx4DFmode,            /* VNx8SF */
  E_VNx24HFmode,           /* VNx4DF */
  E_VNx12SFmode,           /* VNx24HF */
  E_VNx6DFmode,            /* VNx12SF */
  E_VNx32HFmode,           /* VNx6DF */
  E_VNx16SFmode,           /* VNx32HF */
  E_VNx8DFmode,            /* VNx16SF */
  E_VOIDmode,              /* VNx8DF */
};

const unsigned char mode_2xwider[NUM_MACHINE_MODES] =
{
  E_VOIDmode,              /* VOID */
  E_BLKmode,               /* BLK */
  E_VOIDmode,              /* CC */
  E_VOIDmode,              /* CCFP */
  E_VOIDmode,              /* CCFPE */
  E_VOIDmode,              /* CC_SWP */
  E_VOIDmode,              /* CC_NZ */
  E_VOIDmode,              /* CC_Z */
  E_VOIDmode,              /* CC_C */
  E_VOIDmode,              /* BI */
  E_HImode,                /* QI */
  E_SImode,                /* HI */
  E_DImode,                /* SI */
  E_TImode,                /* DI */
  E_OImode,                /* TI */
  E_XImode,                /* OI */
  E_VOIDmode,              /* CI */
  E_VOIDmode,              /* XI */
  E_HQmode,                /* QQ */
  E_SQmode,                /* HQ */
  E_DQmode,                /* SQ */
  E_TQmode,                /* DQ */
  E_VOIDmode,              /* TQ */
  E_UHQmode,               /* UQQ */
  E_USQmode,               /* UHQ */
  E_UDQmode,               /* USQ */
  E_UTQmode,               /* UDQ */
  E_VOIDmode,              /* UTQ */
  E_SAmode,                /* HA */
  E_DAmode,                /* SA */
  E_TAmode,                /* DA */
  E_VOIDmode,              /* TA */
  E_USAmode,               /* UHA */
  E_UDAmode,               /* USA */
  E_UTAmode,               /* UDA */
  E_VOIDmode,              /* UTA */
  E_SFmode,                /* HF */
  E_DFmode,                /* SF */
  E_TFmode,                /* DF */
  E_VOIDmode,              /* TF */
  E_DDmode,                /* SD */
  E_TDmode,                /* DD */
  E_VOIDmode,              /* TD */
  E_CHImode,               /* CQI */
  E_CSImode,               /* CHI */
  E_CDImode,               /* CSI */
  E_CTImode,               /* CDI */
  E_COImode,               /* CTI */
  E_CXImode,               /* COI */
  E_VOIDmode,              /* CCI */
  E_VOIDmode,              /* CXI */
  E_SCmode,                /* HC */
  E_DCmode,                /* SC */
  E_TCmode,                /* DC */
  E_VOIDmode,              /* TC */
  E_VOIDmode,              /* VNx16BI */
  E_VOIDmode,              /* VNx8BI */
  E_VOIDmode,              /* VNx4BI */
  E_VOIDmode,              /* VNx2BI */
  E_V16QImode,             /* V8QI */
  E_V8HImode,              /* V4HI */
  E_V4SImode,              /* V2SI */
  E_VNx32QImode,           /* V16QI */
  E_VNx32QImode,           /* VNx16QI */
  E_VNx16HImode,           /* V8HI */
  E_VNx16HImode,           /* VNx8HI */
  E_VNx8SImode,            /* V4SI */
  E_VNx8SImode,            /* VNx4SI */
  E_VNx4DImode,            /* V2DI */
  E_VNx4DImode,            /* VNx2DI */
  E_VNx64QImode,           /* VNx32QI */
  E_VNx32HImode,           /* VNx16HI */
  E_VNx16SImode,           /* VNx8SI */
  E_VNx8DImode,            /* VNx4DI */
  E_VNx4TImode,            /* VNx2TI */
  E_VOIDmode,              /* VNx48QI */
  E_VOIDmode,              /* VNx24HI */
  E_VOIDmode,              /* VNx12SI */
  E_VOIDmode,              /* VNx6DI */
  E_VOIDmode,              /* VNx3TI */
  E_VOIDmode,              /* VNx64QI */
  E_VOIDmode,              /* VNx32HI */
  E_VOIDmode,              /* VNx16SI */
  E_VOIDmode,              /* VNx8DI */
  E_VOIDmode,              /* VNx4TI */
  E_VOIDmode,              /* VNx2OI */
  E_V4HFmode,              /* V2HF */
  E_V8HFmode,              /* V4HF */
  E_V4SFmode,              /* V2SF */
  E_V2DFmode,              /* V1DF */
  E_VNx16HFmode,           /* V8HF */
  E_VNx16HFmode,           /* VNx8HF */
  E_VNx8SFmode,            /* V4SF */
  E_VNx8SFmode,            /* VNx4SF */
  E_VNx4DFmode,            /* V2DF */
  E_VNx4DFmode,            /* VNx2DF */
  E_VNx32HFmode,           /* VNx16HF */
  E_VNx16SFmode,           /* VNx8SF */
  E_VNx8DFmode,            /* VNx4DF */
  E_VOIDmode,              /* VNx24HF */
  E_VOIDmode,              /* VNx12SF */
  E_VOIDmode,              /* VNx6DF */
  E_VOIDmode,              /* VNx32HF */
  E_VOIDmode,              /* VNx16SF */
  E_VOIDmode,              /* VNx8DF */
};

const unsigned char mode_inner[NUM_MACHINE_MODES] =
{
  E_VOIDmode,              /* VOID */
  E_BLKmode,               /* BLK */
  E_CCmode,                /* CC */
  E_CCFPmode,              /* CCFP */
  E_CCFPEmode,             /* CCFPE */
  E_CC_SWPmode,            /* CC_SWP */
  E_CC_NZmode,             /* CC_NZ */
  E_CC_Zmode,              /* CC_Z */
  E_CC_Cmode,              /* CC_C */
  E_BImode,                /* BI */
  E_QImode,                /* QI */
  E_HImode,                /* HI */
  E_SImode,                /* SI */
  E_DImode,                /* DI */
  E_TImode,                /* TI */
  E_OImode,                /* OI */
  E_CImode,                /* CI */
  E_XImode,                /* XI */
  E_QQmode,                /* QQ */
  E_HQmode,                /* HQ */
  E_SQmode,                /* SQ */
  E_DQmode,                /* DQ */
  E_TQmode,                /* TQ */
  E_UQQmode,               /* UQQ */
  E_UHQmode,               /* UHQ */
  E_USQmode,               /* USQ */
  E_UDQmode,               /* UDQ */
  E_UTQmode,               /* UTQ */
  E_HAmode,                /* HA */
  E_SAmode,                /* SA */
  E_DAmode,                /* DA */
  E_TAmode,                /* TA */
  E_UHAmode,               /* UHA */
  E_USAmode,               /* USA */
  E_UDAmode,               /* UDA */
  E_UTAmode,               /* UTA */
  E_HFmode,                /* HF */
  E_SFmode,                /* SF */
  E_DFmode,                /* DF */
  E_TFmode,                /* TF */
  E_SDmode,                /* SD */
  E_DDmode,                /* DD */
  E_TDmode,                /* TD */
  E_QImode,                /* CQI */
  E_HImode,                /* CHI */
  E_SImode,                /* CSI */
  E_DImode,                /* CDI */
  E_TImode,                /* CTI */
  E_OImode,                /* COI */
  E_CImode,                /* CCI */
  E_XImode,                /* CXI */
  E_HFmode,                /* HC */
  E_SFmode,                /* SC */
  E_DFmode,                /* DC */
  E_TFmode,                /* TC */
  E_BImode,                /* VNx16BI */
  E_BImode,                /* VNx8BI */
  E_BImode,                /* VNx4BI */
  E_BImode,                /* VNx2BI */
  E_QImode,                /* V8QI */
  E_HImode,                /* V4HI */
  E_SImode,                /* V2SI */
  E_QImode,                /* V16QI */
  E_QImode,                /* VNx16QI */
  E_HImode,                /* V8HI */
  E_HImode,                /* VNx8HI */
  E_SImode,                /* V4SI */
  E_SImode,                /* VNx4SI */
  E_DImode,                /* V2DI */
  E_DImode,                /* VNx2DI */
  E_QImode,                /* VNx32QI */
  E_HImode,                /* VNx16HI */
  E_SImode,                /* VNx8SI */
  E_DImode,                /* VNx4DI */
  E_TImode,                /* VNx2TI */
  E_QImode,                /* VNx48QI */
  E_HImode,                /* VNx24HI */
  E_SImode,                /* VNx12SI */
  E_DImode,                /* VNx6DI */
  E_TImode,                /* VNx3TI */
  E_QImode,                /* VNx64QI */
  E_HImode,                /* VNx32HI */
  E_SImode,                /* VNx16SI */
  E_DImode,                /* VNx8DI */
  E_TImode,                /* VNx4TI */
  E_OImode,                /* VNx2OI */
  E_HFmode,                /* V2HF */
  E_HFmode,                /* V4HF */
  E_SFmode,                /* V2SF */
  E_DFmode,                /* V1DF */
  E_HFmode,                /* V8HF */
  E_HFmode,                /* VNx8HF */
  E_SFmode,                /* V4SF */
  E_SFmode,                /* VNx4SF */
  E_DFmode,                /* V2DF */
  E_DFmode,                /* VNx2DF */
  E_HFmode,                /* VNx16HF */
  E_SFmode,                /* VNx8SF */
  E_DFmode,                /* VNx4DF */
  E_HFmode,                /* VNx24HF */
  E_SFmode,                /* VNx12SF */
  E_DFmode,                /* VNx6DF */
  E_HFmode,                /* VNx32HF */
  E_SFmode,                /* VNx16SF */
  E_DFmode,                /* VNx8DF */
};

const unsigned char class_narrowest_mode[MAX_MODE_CLASS] =
{
  MIN_MODE_RANDOM,         /* VOID */
  MIN_MODE_CC,             /* CC */
  MIN_MODE_INT,            /* QI */
  MIN_MODE_PARTIAL_INT,    /* VOID */
  MIN_MODE_POINTER_BOUNDS, /* VOID */
  MIN_MODE_FRACT,          /* QQ */
  MIN_MODE_UFRACT,         /* UQQ */
  MIN_MODE_ACCUM,          /* HA */
  MIN_MODE_UACCUM,         /* UHA */
  MIN_MODE_FLOAT,          /* HF */
  MIN_MODE_DECIMAL_FLOAT,  /* SD */
  MIN_MODE_COMPLEX_INT,    /* CQI */
  MIN_MODE_COMPLEX_FLOAT,  /* HC */
  MIN_MODE_VECTOR_BOOL,    /* VNx16BI */
  MIN_MODE_VECTOR_INT,     /* V8QI */
  MIN_MODE_VECTOR_FRACT,   /* VOID */
  MIN_MODE_VECTOR_UFRACT,  /* VOID */
  MIN_MODE_VECTOR_ACCUM,   /* VOID */
  MIN_MODE_VECTOR_UACCUM,  /* VOID */
  MIN_MODE_VECTOR_FLOAT,   /* V2HF */
};
