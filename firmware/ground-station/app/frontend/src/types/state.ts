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

export type GeoPosition = {
    lat: number;
    lon: number;
    accuracy?: number;
};

export type ComponentData = {
    id: number;
    name: string;
    enabled: boolean;
    inited: boolean;
    task: boolean;
    error: boolean;
    preflightSuccess: boolean;
}[];

export type ApiState = {
    preflightSuccess: boolean | null;
    data: {
        gps: GPSState;
        ldrVoltage: number | null;
        servoAngle: number | null;
        batteryLevel: number | null;
        temperature: number | null;
        cameraOn: boolean | null;
        pressure: number | null;
        // TODO: accelerometer: {},
        // TODO: imu: {}
    };
    fsm: FSMState;
    components: ComponentData;
    radio: { lastPacketTime: number | null; packetsReceived: number | null };
};
