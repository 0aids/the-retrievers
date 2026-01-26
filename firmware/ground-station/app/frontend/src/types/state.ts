export type GPSState = {
    latitude: number | null;
    longitude: number | null;
    positionValid: boolean | null;
};

export type FSMState = {
    state: number | null;
    name: string | null;
};

export type ApiState = {
    gps: GPSState;
    fsm: FSMState;
};
