// Lora configuration.
// clang-format off
#define loraCfg_frequency_d 915000000                 // Hz
#define loraCfg_txOutputPower_d 14                    // dBm
                                                   //
#define loraCfg_bandwidth_d 0                         // [0: 125 kHz,
                                                   //  1: 250 kHz,
                                                   //  2: 500 kHz,
                                                   //  3: Reserved]
                                                   //
#define loraCfg_spreadingFactor_d 7                   // [SF7..SF12]
                                                   //
#define loraCfg_codingRate_d 1                        // [1: 4/5,
                                                   //  2: 4/6,
                                                   //  3: 4/7,
                                                   //  4: 4/8]
                                                   //
#define loraCfg_preambleLength_d 8                    // Same for Tx and Rx
#define loraCfg_symbolTimeout_d 0                     // Symbols
#define loraCfg_fixLengthPayloadOn_d false
#define loraCfg_iqInversionOn_d false

#define loraCfg_headerPacketPreamble_d 0xff // 0b11111111
#define loraCfg_dataPacketPreamble_d 0xaa   // 0b10101010
#define loraCfg_footerPacketPreamble_d 0xcc // 0b11001100
#define loraCfg_txTimeout_d 3000
#define loraCfg_crcOn_d true
#define loraCfg_freqHop_d 0 // No frequency hopping (would get better range if yes?)
#define loraCfg_hopPeriod_d 0 // No frequency hopping (would get better range if yes?)

// This is to just give eachother enough time to ensure the packets are being received.
#define loraCfg_interPacketDelayMs_d 50 //ms
