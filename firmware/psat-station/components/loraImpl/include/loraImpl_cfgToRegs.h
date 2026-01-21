#ifndef loraCfg_config_to_regs_h_INCLUDED
#define loraCfg_config_to_regs_h_INCLUDED
#include "loraRegs.h"
#include "loraCfg.h"

/* -------------------------------------------------------------------------- */
/* LORA CONFIGURATION MAPPING                                                 */
/* -------------------------------------------------------------------------- */

/* --- 1. Bandwidth (RegModemConfig1) --- */
#if (loraCfg_bandwidth_d == 0)
#define loraHalCfg_mdmCfg1_bw_d loraBit_mdmCfg1_bw_125k_d
#elif (loraCfg_bandwidth_d == 1)
#define loraHalCfg_mdmCfg1_bw_d loraBit_mdmCfg1_bw_250k_d
#elif (loraCfg_bandwidth_d == 2)
#define loraHalCfg_mdmCfg1_bw_d loraBit_mdmCfg1_bw_500k_d
#else
#error "Invalid LoRa Bandwidth: Check loraCfg_bandwidth_d"
#endif

/* --- 2. Coding Rate (RegModemConfig1) --- */
#if (loraCfg_codingRate_d == 1)
#define loraHalCfg_mdmCfg1_cr_d loraBit_mdmCfg1_cr_4_5_d
#elif (loraCfg_codingRate_d == 2)
#define loraHalCfg_mdmCfg1_cr_d loraBit_mdmCfg1_cr_4_6_d
#elif (loraCfg_codingRate_d == 3)
#define loraHalCfg_mdmCfg1_cr_d loraBit_mdmCfg1_cr_4_7_d
#elif (loraCfg_codingRate_d == 4)
#define loraHalCfg_mdmCfg1_cr_d loraBit_mdmCfg1_cr_4_8_d
#else
#error "Invalid LoRa Coding Rate: Check loraCfg_codingRate_d"
#endif

/* --- 3. Header Mode (RegModemConfig1) --- */
#if (loraCfg_fixLengthPayloadOn_d == true)
#define loraHalCfg_mdmCfg1_implicitHead_d                            \
    loraBit_mdmCfg1_implicitHead_on_d
#else
#define loraHalCfg_mdmCfg1_implicitHead_d                            \
    loraBit_mdmCfg1_implicitHead_off_d
#endif

/* --- 4. Spreading Factor (RegModemConfig2) --- */
#if (loraCfg_spreadingFactor_d == 6)
#define loraHalCfg_mdmCfg2_sf_d loraBit_mdmCfg2_sf_6_d
#elif (loraCfg_spreadingFactor_d == 7)
#define loraHalCfg_mdmCfg2_sf_d loraBit_mdmCfg2_sf_7_d
#elif (loraCfg_spreadingFactor_d == 8)
#define loraHalCfg_mdmCfg2_sf_d loraBit_mdmCfg2_sf_8_d
#elif (loraCfg_spreadingFactor_d == 9)
#define loraHalCfg_mdmCfg2_sf_d loraBit_mdmCfg2_sf_9_d
#elif (loraCfg_spreadingFactor_d == 10)
#define loraHalCfg_mdmCfg2_sf_d loraBit_mdmCfg2_sf_10_d
#elif (loraCfg_spreadingFactor_d == 11)
#define loraHalCfg_mdmCfg2_sf_d loraBit_mdmCfg2_sf_11_d
#elif (loraCfg_spreadingFactor_d == 12)
#define loraHalCfg_mdmCfg2_sf_d loraBit_mdmCfg2_sf_12_d
#else
#error "Invalid Spreading Factor"
#endif

/* --- 5. Rx CRC (RegModemConfig2) --- */
#if (loraCfg_crcOn_d == true)
#define loraHalCfg_mdmCfg2_rxCrc_d loraBit_mdmCfg2_rxCrc_on_d
#else
#define loraHalCfg_mdmCfg2_rxCrc_d loraBit_mdmCfg2_rxCrc_off_d
#endif

/* --- 6. Low Data Rate Optimization (RegModemConfig3) --- */
/* Required if Symbol Duration > 16ms */
#if ((loraCfg_bandwidth_d == 0) &&                                   \
     (loraCfg_spreadingFactor_d >= 11)) ||                           \
    ((loraCfg_bandwidth_d == 1) &&                                   \
     (loraCfg_spreadingFactor_d == 12))
#define loraHalCfg_mdmCfg3_lowDataOpt_d                              \
    loraBit_mdmCfg3_lowDataOpt_on_d
#else
#define loraHalCfg_mdmCfg3_lowDataOpt_d                              \
    loraBit_mdmCfg3_lowDataOpt_off_d
#endif

/* --- 7. AGC Auto (RegModemConfig3) --- */
#define loraHalCfg_mdmCfg3_agcAuto_d loraBit_mdmCfg3_agcAuto_off_d

/* --- 8. Frequency Calculation (RegFrf) --- */
#define LORA_XOSC_FREQ_HZ 32000000UL
/* Resulting value to be split into Msb, Mid, Lsb */
#define loraHalCfg_frf_val_d                                         \
    ((uint32_t)((uint64_t)loraCfg_frequency_d * 524288UL /           \
                LORA_XOSC_FREQ_HZ))

/* --- 9. Output Power (RegPaConfig) --- */
/* Pout = 2 + OutputPower (on PA_BOOST) */
#if (loraCfg_txOutputPower_d > 17 || loraCfg_txOutputPower_d < 2)
#error "Tx Power out of supported range for PA_BOOST"
#else
#define loraHalCfg_paCfg_paSelect_d loraBit_paCfg_paSelect_boost_d
#define loraHalCfg_paCfg_outputPower_d                               \
    ((loraCfg_txOutputPower_d - 2) << loraBit_paCfg_outputPowerPos_d)
#endif

/* --- 10. Helper Values (Direct Mappings) --- */
#define loraHalCfg_preamble_val_d ((uint16_t)loraCfg_preambleLength_d)
#define loraHalCfg_mdmCfg2_symbTimeoutMsb_d                          \
    ((uint8_t)((loraCfg_symbolTimeout_d >> 8) & 0x03))
#define loraHalCfg_symbTimeout_lsb_d                                 \
    ((uint8_t)(loraCfg_symbolTimeout_d & 0xFF))
#endif // loraCfg_config_to_regs_h_INCLUDED
