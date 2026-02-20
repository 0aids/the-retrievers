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
    currentState: number | null;
    currentStateName: string | null;
    prevState: number | null;
    prevStateName: string | null;
};

export type ApiState = {
    gps: GPSState;
    fsm: FSMState;
    radio: { lastPacketTime: number | null };
    stats: { packetsReceived: number | null };
};
