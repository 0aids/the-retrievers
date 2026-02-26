export enum PacketType {
    loraFsm_packetType_empty,
    loraFsm_packetType_test,
    loraFsm_packetType_ack,

    loraFsm_packetType_ping,
    loraFsm_packetType_pong,

    loraFsm_packetType_gpsData,
    loraFsm_packetType_stateData,
    loraFsm_packetType_sensorData,
    loraFsm_packetType_componentData,
    loraFsm_packetType_telemetryData,
    loraFsm_packetType_preflightData,

    loraFsm_packetType_preflightReq,
    loraFsm_packetType_preflightDataReq,
    loraFsm_packetType_prelaunchCompleteReq,

    loraFsm_packetType_buzzShortReq,
    loraFsm_packetType_buzzLongReq,

    loraFsm_packetType_fastForwardReq,
    loraFsm_packetType_stateOverrideReq,

    loraFsm_packetType_markComponentEnabledReq,
    loraFsm_packetType_markComponentDisabledReq,

    loraFsm_packetType_enableComponentReq,
    loraFsm_packetType_disableComponentReq,

    loraFsm_packetType_startComponentTaskReq,
    loraFsm_packetType_stopComponentTaskReq,

    loraFsm_packetType__COUNT,
}
