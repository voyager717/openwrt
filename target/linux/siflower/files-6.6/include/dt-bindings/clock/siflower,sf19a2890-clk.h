/* SPDX-License-Identifier: (GPL-2.0 OR MIT) */
#ifndef __DT_BINDINGS_CLOCK_SIFLOWER_SF19A2890_CLK_H
#define __DT_BINDINGS_CLOCK_SIFLOWER_SF19A2890_CLK_H
#define CLK_PLL_CPU		0
#define CLK_PLL_DDR		1
#define CLK_PLL_CMN		2
#define CLK_MUXDIV_BUS1		3
#define CLK_MUXDIV_BUS2		4
#define CLK_MUXDIV_BUS3		5
#define CLK_MUXDIV_CPU		6
#define CLK_MUXDIV_PBUS		7
#define CLK_MUXDIV_MEM_PHY	8
#define CLK_MUXDIV_UART		9
#define CLK_MUXDIV_ETH_REF	10
#define CLK_MUXDIV_ETH_BYP_REF	11
#define CLK_MUXDIV_ETH_TSU	12
#define CLK_MUXDIV_GMAC_BYP_REF	13
#define CLK_MUXDIV_M6250_0	14
#define CLK_MUXDIV_M6250_1	15
#define CLK_MUXDIV_WLAN24_PLF	16
#define CLK_MUXDIV_WLAN5_PLF	17
#define CLK_MUXDIV_USBPHY_REF	18
#define CLK_MUXDIV_TCLK		19
#define CLK_MUXDIV_NPU_PE_CLK	20

#define CLK_SF19A2890_MAX	21
#endif /* __DT_BINDINGS_CLOCK_SIFLOWER_SF19A2890_CLK_H */
