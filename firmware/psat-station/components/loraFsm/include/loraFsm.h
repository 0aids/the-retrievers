#ifndef loraFsm_h_INCLUDED
#define loraFsm_h_INCLUDED

// In seconds
#ifndef loraFsm_connTimeoutThreshold_s_d
#define loraFsm_connTimeoutThreshold_s_d 300
#endif

#ifndef loraFsm_txRoutineInterval_s_d
#define loraFsm_txRoutineInterval_s_d 20
#endif

#ifndef loraFsm_beaconRoutineInterval_s_d
#define loraFsm_beaconRoutineInterval_s_d 30
#endif

void loraFsm_init(void);
void loraFsm_start(void);

#endif // loraFsm_h_INCLUDED
