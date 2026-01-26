export type GPSState = {
    latitude: number | null;
    longitude: number | null;
    positionValid: boolean | null;
    speedKnots: number | null;
    speedKph: number | null;
    courseDeg: number | null;
    hdop: number | null;
    altitude: number | null;
    geoidalSep: number | null;
    day: number | null;
    month: number | null;
    year: number | null;
    hours: number | null;
    minutes: number | null;
    seconds: number | null;
    fixQuality: number | null;
    satellitesTracked: number | null;
    satsInView: number | null;
    navValid: boolean | null;
    fixInfoValid: boolean | null;
    altitudeValid: boolean | null;
};

export type FSMState = {
    state: number | null;
    name: string | null;
};

export type ApiState = {
    gps: GPSState;
    fsm: FSMState;
    radio: {
        last_packet_time: number | null;
    };
    stats: {
        packets_recieved: number | null;
    };
};
