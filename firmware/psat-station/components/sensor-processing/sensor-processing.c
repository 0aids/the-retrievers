#include "sensor-processing.h"
#include "bmi323.h"
#include "highg.h"
#include "bmp280.h"
#include "shared_state.h"
#include "ldr.h"

sensorData_t sensorData = {0};


void updateAccData() {
    //relative to rocket pointing straight up and roll relative to power on position
    //gravity is always assumed to be measured as 9.81m/s by the sensor
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

//gyro time


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