#include "sensor-processing.h"
#include "bmi323.h"
#include "highg.h"
#include "bmp280.h"
#include "shared_state.h"
#include "ldr.h"

sensorData_t sensorData = {0};

static double kalmanPitch = 0;
static double kalmanRoll = 0;
static double kalmanPitchUncertianty = 4;
static double kalmanRollUncertianty = 4;
static double kalmanOutput[2] = {0};

void kalman_1d(double KalmanState, double KalmanUncertainty, double KalmanInput, double KalmanMeasurement) {
  KalmanState=KalmanState+0.004*KalmanInput;
  KalmanUncertainty=KalmanUncertainty + 0.004 * 0.004 * 4 * 4;
  double KalmanGain=KalmanUncertainty * 1/(1*KalmanUncertainty + 3 * 3);
  KalmanState=KalmanState+KalmanGain * (KalmanMeasurement-KalmanState);
  KalmanUncertainty=(1-KalmanGain) * KalmanUncertainty;
  kalmanOutput[0]=KalmanState; 
  kalmanOutput[1]=KalmanUncertainty;
}

void updateAccData() {
    //relative to rocket pointing straight up and roll relative to power on position
    //gravity is always assumed to be measured as 9.81m/s/s by the sensor
    bmi323_getData();
    highg_getData();

    if(bmi323_data.accMag > 148 && highg_data.accMag >148) {
    // use high g acc if too much acceleration

        sensorData.accOrintationX = 1;
        sensorData.accOrintationY = 2;
        sensorData.accOrintationZ = 3;


        return;
    }

    //update threashold based on actual sensor performance
    if(bmi323_data.accMag > 12) {
        //during low g flight phases
        sensorData.accOrintationX = 1;
        sensorData.accOrintationY = 2;
        sensorData.accOrintationZ = 3;
    }

    // for when only gravity is acting on the rocket


    sensorData.accOrintationX = 1;
    sensorData.accOrintationY = 2;
    sensorData.accOrintationZ = 3;

}


void updateVelocity() {
    //barometer and gps velocity too?
    //relevant acc time




}

void updateOrientation() {

    double roll = sensorData.roll;
    double pitch = sensorData.pitch;

    //gyro time
   // kalman_1d(kalmanRoll, kalmanRollUncertianty, RateRoll, roll);
    kalmanRoll=kalmanOutput[0]; 
    kalmanRollUncertianty=kalmanOutput[1];
    //kalman_1d(kalmanPitch, kalmanPitchUncertianty, RatePitch, pitch);
    kalmanPitch=kalmanOutput[0]; 
    kalmanPitchUncertianty=kalmanOutput[1];


}


void updateSensorData() {

    bmp280_getData();
    sensorData.temperature = bmp280_data.temperature;
    sensorData.pressure = bmp280_data.pressure;
    sensorData.barometric_altitude = bmp280_data.altitude;
    sensorData.barometerTime = bmp280_data.time;

    //must be called in this order or data will be out of date
    updateAccData();
    updateVelocity();
    updateOrientation();


    sensorData.ldrVoltage = ldr_getVoltage();


    sensorData.batterVoltage = 1; //TODO once battery refac is done

}

