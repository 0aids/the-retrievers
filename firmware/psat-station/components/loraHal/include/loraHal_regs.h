#define loraBit_set(curValue, masks, flags) \
    ((curValue = (curValue & ~(masks)) | (flags)))

#define loraReg_cmdWrite_d 1
#define loraReg_cmdRead_d  0

// For register addresses, put them here in the form
// For names that are quite long (longer than 10 chars), attempt to abbreiviate it.
// Use camel case with underscores for namespacing.
// #define loraReg_addr{registerName}_d

// For per-register bit-fields positions put them in the form
// #define loraBit_{registerName}_{bitFieldName}Pos_d

// For per-register masks, put them in the form
// #define loraBit_{registerName}_{bitfieldName}Msk_d

// For bitfield options, put them in the form
// #define loraBit_{registerName}_{bitfieldName}_{optionName}_d

#define loraReg_addrOpMode_d 0x01


#define loraBit_opMode_modePos_d (0)
#define loraBit_opMode_freqPos_d (3)
#define loraBit_opMode_accessSharedRegPos_d (6)
#define loraBit_opMode_longRangeModePos_d (7)

#define loraBit_opMode_modeMsk_d (0x7 << loraBit_opMode_modePos_d)
#define loraBit_opMode_freqMsk_d (0x1 << loraBit_opMode_freqPos_d)
#define loraBit_opMode_accessSharedRegMsk_d (0x1 << loraBit_opMode_accessSharedRegPos_d)
#define loraBit_opMode_longRangeModeMsk_d (0x1 << loraBit_opMode_longRangeModePos_d)

#define loraBit_opMode_mode_sleep_d (0x0 << loraBit_opMode_modePos_d)
#define loraBit_opMode_mode_stdby_d (0x1 << loraBit_opMode_modePos_d)
#define loraBit_opMode_mode_fstx_d (0x2 << loraBit_opMode_modePos_d)
#define loraBit_opMode_mode_tx_d (0x3 << loraBit_opMode_modePos_d)
#define loraBit_opMode_mode_fsrx_d (0x4 << loraBit_opMode_modePos_d)
#define loraBit_opMode_mode_rxc_d (0x5 << loraBit_opMode_modePos_d)
#define loraBit_opMode_mode_rxs_d (0x6 << loraBit_opMode_modePos_d)
#define loraBit_opMode_mode_cad_d (0x7 << loraBit_opMode_modePos_d)

#define loraBit_opMode_freq_high_d (0x0 << loraBit_opMode_freqPos_d)
#define loraBit_opMode_freq_low_d (0x1 << loraBit_opMode_freqPos_d)

#define loraBit_opMode_accessSharedReg_false_d (0x0 << loraBit_opMode_accessSharedRegPos_d)
#define loraBit_opMode_accessSharedReg_true_d (0x1 << loraBit_opMode_accessSharedRegPos_d)

#define loraBit_opMode_longRangeMode_fskOok_d (0x0 << loraBit_opMode_longRangeModePos_d)
#define loraBit_opMode_longRangeMode_lora_d (0x1 << loraBit_opMode_longRangeModePos_d)


/* Register Address */
#define loraReg_addrPaCfg_d                  0x09

/* Bit Positions */
#define loraBit_paCfg_paSelectPos_d          (7)
#define loraBit_paCfg_maxPowerPos_d          (4)
#define loraBit_paCfg_outputPowerPos_d       (0)

/* Bit Masks */
#define loraBit_paCfg_paSelectMsk_d          (0x1 << loraBit_paCfg_paSelectPos_d)
#define loraBit_paCfg_maxPowerMsk_d          (0x7 << loraBit_paCfg_maxPowerPos_d)
#define loraBit_paCfg_outputPowerMsk_d       (0xF << loraBit_paCfg_outputPowerPos_d)

/* Options */
#define loraBit_paCfg_paSelect_rfo_d         (0x0 << loraBit_paCfg_paSelectPos_d)    /* Output on RFO pin */
#define loraBit_paCfg_paSelect_boost_d       (0x1 << loraBit_paCfg_paSelectPos_d)    /* Output on PA_BOOST pin */

/* Note: MaxPower and OutputPower are numeric values, not enums. 
   MaxPower: Pmax = 10.8 + 0.6 * MaxPower 
   OutputPower: Pout = Pmax - (15 - OutputPower) if PaSelect = 0 (RFO) */


/* Register Address */
#define loraReg_addrLna_d                    0x0C

/* Bit Positions */
#define loraBit_lna_gainPos_d                (5)
#define loraBit_lna_boostHfPos_d             (0)

/* Bit Masks */
#define loraBit_lna_gainMsk_d                (0x7 << loraBit_lna_gainPos_d)
#define loraBit_lna_boostHfMsk_d             (0x3 << loraBit_lna_boostHfPos_d)

/* Options - Gain */
#define loraBit_lna_gain_g1_d                (0x1 << loraBit_lna_gainPos_d) /* Max Gain */
#define loraBit_lna_gain_g2_d                (0x2 << loraBit_lna_gainPos_d)
#define loraBit_lna_gain_g3_d                (0x3 << loraBit_lna_gainPos_d)
#define loraBit_lna_gain_g4_d                (0x4 << loraBit_lna_gainPos_d)
#define loraBit_lna_gain_g5_d                (0x5 << loraBit_lna_gainPos_d)
#define loraBit_lna_gain_g6_d                (0x6 << loraBit_lna_gainPos_d) /* Min Gain */

/* Options - Boost (Default 00, set to 11 for 150% LNA Current) */
#define loraBit_lna_boostHf_off_d            (0x0 << loraBit_lna_boostHfPos_d)
#define loraBit_lna_boostHf_on_d             (0x3 << loraBit_lna_boostHfPos_d)

/* Register Address */
#define loraReg_addrIrqFlags_d               0x12

/* Bit Positions */
#define loraBit_irqFlags_rxTimeoutPos_d      (7)
#define loraBit_irqFlags_rxDonePos_d         (6)
#define loraBit_irqFlags_crcErrPos_d         (5)
#define loraBit_irqFlags_validHeadPos_d      (4)
#define loraBit_irqFlags_txDonePos_d         (3)
#define loraBit_irqFlags_cadDonePos_d        (2)
#define loraBit_irqFlags_fhssChangePos_d     (1)
#define loraBit_irqFlags_cadDetectPos_d      (0)

/* Bit Masks (Used for checking or clearing) */
#define loraBit_irqFlags_rxTimeoutMsk_d      (0x1 << loraBit_irqFlags_rxTimeoutPos_d)
#define loraBit_irqFlags_rxDoneMsk_d         (0x1 << loraBit_irqFlags_rxDonePos_d)
#define loraBit_irqFlags_crcErrMsk_d         (0x1 << loraBit_irqFlags_crcErrPos_d)
#define loraBit_irqFlags_validHeadMsk_d      (0x1 << loraBit_irqFlags_validHeadPos_d)
#define loraBit_irqFlags_txDoneMsk_d         (0x1 << loraBit_irqFlags_txDonePos_d)
#define loraBit_irqFlags_cadDoneMsk_d        (0x1 << loraBit_irqFlags_cadDonePos_d)
#define loraBit_irqFlags_fhssChangeMsk_d     (0x1 << loraBit_irqFlags_fhssChangePos_d)
#define loraBit_irqFlags_cadDetectMsk_d      (0x1 << loraBit_irqFlags_cadDetectPos_d)

/* Helper: Clear All */
#define loraBit_irqFlags_clearAll_d          (0xFF)


/* Register Address */
#define loraReg_addrMdmCfg1_d                0x1D

/* Bit Positions */
#define loraBit_mdmCfg1_bwPos_d              (4)
#define loraBit_mdmCfg1_crPos_d              (1)
#define loraBit_mdmCfg1_implicitHeadPos_d    (0)

/* Bit Masks */
#define loraBit_mdmCfg1_bwMsk_d              (0xF << loraBit_mdmCfg1_bwPos_d)
#define loraBit_mdmCfg1_crMsk_d              (0x7 << loraBit_mdmCfg1_crPos_d)
#define loraBit_mdmCfg1_implicitHeadMsk_d    (0x1 << loraBit_mdmCfg1_implicitHeadPos_d)

/* Options - Bandwidth */
#define loraBit_mdmCfg1_bw_7k8_d             (0x0 << loraBit_mdmCfg1_bwPos_d)
#define loraBit_mdmCfg1_bw_10k4_d            (0x1 << loraBit_mdmCfg1_bwPos_d)
#define loraBit_mdmCfg1_bw_15k6_d            (0x2 << loraBit_mdmCfg1_bwPos_d)
#define loraBit_mdmCfg1_bw_20k8_d            (0x3 << loraBit_mdmCfg1_bwPos_d)
#define loraBit_mdmCfg1_bw_31k25_d           (0x4 << loraBit_mdmCfg1_bwPos_d)
#define loraBit_mdmCfg1_bw_41k7_d            (0x5 << loraBit_mdmCfg1_bwPos_d)
#define loraBit_mdmCfg1_bw_62k5_d            (0x6 << loraBit_mdmCfg1_bwPos_d)
#define loraBit_mdmCfg1_bw_125k_d            (0x7 << loraBit_mdmCfg1_bwPos_d)
#define loraBit_mdmCfg1_bw_250k_d            (0x8 << loraBit_mdmCfg1_bwPos_d)
#define loraBit_mdmCfg1_bw_500k_d            (0x9 << loraBit_mdmCfg1_bwPos_d)

/* Options - Coding Rate */
#define loraBit_mdmCfg1_cr_4_5_d             (0x1 << loraBit_mdmCfg1_crPos_d)
#define loraBit_mdmCfg1_cr_4_6_d             (0x2 << loraBit_mdmCfg1_crPos_d)
#define loraBit_mdmCfg1_cr_4_7_d             (0x3 << loraBit_mdmCfg1_crPos_d)
#define loraBit_mdmCfg1_cr_4_8_d             (0x4 << loraBit_mdmCfg1_crPos_d)

/* Options - Header Mode */
#define loraBit_mdmCfg1_implicitHead_off_d   (0x0 << loraBit_mdmCfg1_implicitHeadPos_d) /* Explicit */
#define loraBit_mdmCfg1_implicitHead_on_d    (0x1 << loraBit_mdmCfg1_implicitHeadPos_d) /* Implicit */

/* Register Address */
#define loraReg_addrMdmCfg2_d                0x1E

/* Bit Positions */
#define loraBit_mdmCfg2_sfPos_d              (4)
#define loraBit_mdmCfg2_txContPos_d          (3)
#define loraBit_mdmCfg2_rxCrcPos_d           (2)
#define loraBit_mdmCfg2_symbTimeMsbPos_d     (0)

/* Bit Masks */
#define loraBit_mdmCfg2_sfMsk_d              (0xF << loraBit_mdmCfg2_sfPos_d)
#define loraBit_mdmCfg2_txContMsk_d          (0x1 << loraBit_mdmCfg2_txContPos_d)
#define loraBit_mdmCfg2_rxCrcMsk_d           (0x1 << loraBit_mdmCfg2_rxCrcPos_d)
#define loraBit_mdmCfg2_symbTimeMsbMsk_d    (0x3 << loraBit_mdmCfg2_symbTimeMsbPos_d)

/* Options - Spreading Factor */
#define loraBit_mdmCfg2_sf_6_d               (0x6 << loraBit_mdmCfg2_sfPos_d)
#define loraBit_mdmCfg2_sf_7_d               (0x7 << loraBit_mdmCfg2_sfPos_d)
#define loraBit_mdmCfg2_sf_8_d               (0x8 << loraBit_mdmCfg2_sfPos_d)
#define loraBit_mdmCfg2_sf_9_d               (0x9 << loraBit_mdmCfg2_sfPos_d)
#define loraBit_mdmCfg2_sf_10_d              (0xA << loraBit_mdmCfg2_sfPos_d)
#define loraBit_mdmCfg2_sf_11_d              (0xB << loraBit_mdmCfg2_sfPos_d)
#define loraBit_mdmCfg2_sf_12_d              (0xC << loraBit_mdmCfg2_sfPos_d)

/* Options - CRC */
#define loraBit_mdmCfg2_rxCrc_off_d          (0x0 << loraBit_mdmCfg2_rxCrcPos_d)
#define loraBit_mdmCfg2_rxCrc_on_d           (0x1 << loraBit_mdmCfg2_rxCrcPos_d)

/* --- 0x1F: Symbol Timeout LSB (MSB is in MdmCfg2) --- */
#define loraReg_addrSymbTimeLsb_d         0x1F
#define loraBit_addrSymbTimeLsbMsk_d         0xff

/* Register Address */
#define loraReg_addrMdmCfg3_d                0x26

/* Bit Positions */
#define loraBit_mdmCfg3_lowDataOptPos_d      (3)
#define loraBit_mdmCfg3_agcAutoPos_d         (2)

/* Bit Masks */
#define loraBit_mdmCfg3_lowDataOptMsk_d      (0x1 << loraBit_mdmCfg3_lowDataOptPos_d)
#define loraBit_mdmCfg3_agcAutoMsk_d         (0x1 << loraBit_mdmCfg3_agcAutoPos_d)

/* Options */
#define loraBit_mdmCfg3_lowDataOpt_off_d     (0x0 << loraBit_mdmCfg3_lowDataOptPos_d)
#define loraBit_mdmCfg3_lowDataOpt_on_d      (0x1 << loraBit_mdmCfg3_lowDataOptPos_d)

#define loraBit_mdmCfg3_agcAuto_off_d        (0x0 << loraBit_mdmCfg3_agcAutoPos_d)
#define loraBit_mdmCfg3_agcAuto_on_d         (0x1 << loraBit_mdmCfg3_agcAutoPos_d)

/* --- 0x00: FIFO Read/Write Access --- */
#define loraReg_addrFifo_d                   0x00

/* --- 0x06 - 0x08: Carrier Frequency (Frf) --- */
#define loraReg_addrFrfMsb_d                 0x06
#define loraReg_addrFrfMid_d                 0x07
#define loraReg_addrFrfLsb_d                 0x08

/* --- 0x0A: PA Ramp Time --- */
#define loraReg_addrPaRamp_d                 0x0A
#define loraBit_paRamp_modulationShapingPos_d (5) /* FSK only, but mask exists */
#define loraBit_paRamp_paRampPos_d           (0)

#define loraBit_paRamp_paRampMsk_d           (0xF << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_3_4ms_d        (0x0 << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_2ms_d          (0x1 << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_1ms_d          (0x2 << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_500us_d        (0x3 << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_250us_d        (0x4 << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_125us_d        (0x5 << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_100us_d        (0x6 << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_62us_d         (0x7 << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_50us_d         (0x8 << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_40us_d         (0x9 << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_31us_d         (0xA << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_25us_d         (0xB << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_20us_d         (0xC << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_15us_d         (0xD << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_12us_d         (0xE << loraBit_paRamp_paRampPos_d)
#define loraBit_paRamp_paRamp_10us_d         (0xF << loraBit_paRamp_paRampPos_d)

/* --- 0x0B: Over Current Protection (OCP) --- */
#define loraReg_addrOcp_d                    0x0B
#define loraBit_ocp_onPos_d                  (5)
#define loraBit_ocp_trimPos_d                (0)

#define loraBit_ocp_onMsk_d                  (0x1 << loraBit_ocp_onPos_d)
#define loraBit_ocp_trimMsk_d                (0x1F << loraBit_ocp_trimPos_d)

#define loraBit_ocp_on_false_d               (0x0 << loraBit_ocp_onPos_d)
#define loraBit_ocp_on_true_d                (0x1 << loraBit_ocp_onPos_d)
/* Note: Imax = 45mA + 5mA * Trim (if Trim <= 15) */
/* Imax = -30mA + 10mA * Trim (if Trim > 15) */

/* --- 0x0D - 0x10: FIFO Pointers --- */
#define loraReg_addrFifoAddrPtr_d            0x0D /* SPI Interface Address Pointer */
#define loraReg_addrFifoTxBaseAddr_d         0x0E /* Write base address in FIFO */
#define loraReg_addrFifoRxBaseAddr_d         0x0F /* Read base address in FIFO */
#define loraReg_addrFifoRxCurAddr_d          0x10 /* Start address of last packet received */

/* --- 0x11: IRQ Flags Mask (Same layout as RegIrqFlags 0x12) --- */
#define loraReg_addrIrqFlagsMask_d           0x11
#define loraBit_irqFlagsMask_rxTimeoutPos_d  (7)
#define loraBit_irqFlagsMask_rxDonePos_d     (6)
#define loraBit_irqFlagsMask_crcErrPos_d     (5)
#define loraBit_irqFlagsMask_validHeadPos_d  (4)
#define loraBit_irqFlagsMask_txDonePos_d     (3)
#define loraBit_irqFlagsMask_cadDonePos_d    (2)
#define loraBit_irqFlagsMask_fhssChangePos_d (1)
#define loraBit_irqFlagsMask_cadDetectPos_d  (0)

/* --- 0x13: Number of Received Bytes --- */
#define loraReg_addrRxNbBytes_d              0x13

/* --- 0x14 - 0x15: Valid Header Count (Msb/Lsb) --- */
#define loraReg_addrRxHeadCntMsb_d           0x14
#define loraReg_addrRxHeadCntLsb_d           0x15

/* --- 0x16 - 0x17: Valid Packet Count (Msb/Lsb) --- */
#define loraReg_addrRxPktCntMsb_d            0x16
#define loraReg_addrRxPktCntLsb_d            0x17

/* --- 0x18: Modem Status --- */
#define loraReg_addrMdmStat_d                0x18
#define loraBit_mdmStat_rxCrPos_d            (5)
#define loraBit_mdmStat_modemClearPos_d      (4)
#define loraBit_mdmStat_headInfoValidPos_d   (3)
#define loraBit_mdmStat_rxOnGoingPos_d       (2)
#define loraBit_mdmStat_sigSyncPos_d         (1)
#define loraBit_mdmStat_sigDetectPos_d       (0)

#define loraBit_mdmStat_rxCrMsk_d            (0x7 << loraBit_mdmStat_rxCrPos_d)
#define loraBit_mdmStat_modemClearMsk_d      (0x1 << loraBit_mdmStat_modemClearPos_d)
#define loraBit_mdmStat_headInfoValidMsk_d   (0x1 << loraBit_mdmStat_headInfoValidPos_d)
#define loraBit_mdmStat_rxOnGoingMsk_d       (0x1 << loraBit_mdmStat_rxOnGoingPos_d)
#define loraBit_mdmStat_sigSyncMsk_d         (0x1 << loraBit_mdmStat_sigSyncPos_d)
#define loraBit_mdmStat_sigDetectMsk_d       (0x1 << loraBit_mdmStat_sigDetectPos_d)

/* --- 0x19: Packet SNR --- */
#define loraReg_addrPktSnr_d                 0x19 
/* Value is 2's complement. SNR = val / 4 */

/* --- 0x1A: Packet RSSI --- */
#define loraReg_addrPktRssi_d                0x1A
/* Value is numeric. RSSI = -157 + val (HF) or -164 + val (LF) */

/* --- 0x1B: Current RSSI --- */
#define loraReg_addrRssi_d                   0x1B
/* Value is numeric. RSSI = -157 + val (HF) or -164 + val (LF) */

/* --- 0x1C: Hop Channel (FHSS) --- */
#define loraReg_addrHopChan_d                0x1C
#define loraBit_hopChan_pllTimeoutPos_d      (7)
#define loraBit_hopChan_crcOnPayloadPos_d    (6)
#define loraBit_hopChan_fhssChanPos_d        (0)

#define loraBit_hopChan_pllTimeoutMsk_d      (0x1 << loraBit_hopChan_pllTimeoutPos_d)
#define loraBit_hopChan_crcOnPayloadMsk_d    (0x1 << loraBit_hopChan_crcOnPayloadPos_d)
#define loraBit_hopChan_fhssChanMsk_d        (0x3F << loraBit_hopChan_fhssChanPos_d)

/* --- 0x20 - 0x21: Preamble Length --- */
#define loraReg_addrPreambleMsb_d            0x20
#define loraReg_addrPreambleLsb_d            0x21

/* --- 0x22: Payload Length --- */
#define loraReg_addrPayloadLen_d             0x22

/* --- 0x23: Max Payload Length --- */
#define loraReg_addrMaxPayLen_d              0x23

/* --- 0x24: Hop Period --- */
#define loraReg_addrHopPeriod_d              0x24

/* --- 0x25: FIFO Rx Byte Address --- */
#define loraReg_addrFifoRxByteAddr_d         0x25

/* --- 0x28 - 0x2A: Frequency Error (Fei) --- */
#define loraReg_addrFeiMsb_d                 0x28 /* Bits 19-16? Check datasheet specific rev, usually 20-bit signed */
#define loraReg_addrFeiMid_d                 0x29
#define loraReg_addrFeiLsb_d                 0x2A

/* --- 0x2C: Wideband RSSI (Reserved/Optional in some sheets) --- */
#define loraReg_addrRssiWideband_d           0x2C
