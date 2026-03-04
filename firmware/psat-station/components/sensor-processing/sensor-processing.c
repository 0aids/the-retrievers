#include "sensor-processing.h"
#include "bmi323.h"
#include "driver/i2c_master.h"
#include "highg.h"
#include "bmp280.h"
#include "shared_state.h"
#include "ldr.h"
#include "esp_timer.h"
#include "math.h"




//////////////////////////////////////////////////////////////* */

//determine initial orientation on pad using acc
//once launch detected switch to gyro with low pass
//'xk = A*'x(k-1) + (1-A)xk
//kalman filter with barometer
//when total acc magnitude < 1.2g kalman filter attitude with accelerometer




bool onPad = true;

sensorData_t sensorData = {0};

static double kalmanPitch = 0;
static double kalmanRoll = 0;
static double kalmanPitchUncertianty = 4;
static double kalmanRollUncertianty = 4;
static double kalmanOutput[2] = {0};
//integrate sensor to estimate
static double kalmanAltitudeUncertianty = 10;

static struct accData_t {
    double x;
    double y;
    double z;
};

static struct accData_t accData[MOVING_AVG_NUM] = {0};

void kalman_1d(double KalmanState, double KalmanUncertainty, double KalmanInput, double KalmanMeasurement, double deviation, long double timeStep) {
  KalmanState=KalmanState+timeStep*KalmanInput;
  KalmanUncertainty=KalmanUncertainty + timeStep * timeStep * deviation * deviation;
  double KalmanGain=KalmanUncertainty * 1/(1*KalmanUncertainty + 3 * 3);
  KalmanState=KalmanState+KalmanGain * (KalmanMeasurement-KalmanState);
  KalmanUncertainty=(1-KalmanGain) * KalmanUncertainty;
  kalmanOutput[0]=KalmanState; 
  kalmanOutput[1]=KalmanUncertainty;
}

void updateAccData() {
    //relative to rocket pointing straight up and roll relative to power on position
    //gravity is always assumed to be measured as 9.81m/s/s by the sensor

    //get sensor data
    //change acc to be relative to starting orientation
    

    if(bmi323_data.accMag > 148 && highg_data.accMag > 148) {
    // use high g acc if too much acceleration

        


        return;
    }

    //update threashold based on actual sensor performance
    if(bmi323_data.accMag > 12) {
        //during low g flight phases use bmi
        

        return;
    }

    // for when only gravity is acting on the rocket

    //set orientation as well based on recieved data
    sensorData.orintationX = atan(bmi323_data.accY/(sqrt(bmi323_data.accX*bmi323_data.accX + bmi323_data.accZ*bmi323_data.accZ))); //Yaw
    sensorData.orintationY = atan(-bmi323_data.accX/(sqrt(bmi323_data.accY*bmi323_data.accY+bmi323_data.accZ*bmi323_data.accZ))); //Pitch
    sensorData.orintationZ = atan(bmi323_data.accZ/(sqrt(bmi323_data.accX*bmi323_data.accX + bmi323_data.accY*bmi323_data.accY))); //Roll

}


void updateVelocity() {
    //barometer and gps velocity too?
    //relevant acc time
    //velocity should be relative to starting orientation


    //update altitude
    //low pass acceleration and integrate for high g
    //add barameter after deployment

    /*
    if(psatFSM_getCurrentState() >= psatFSM_state_deployed){
        //determine uncertianty
        kalman_1d(sensorData.altitude, kalmanAltitudeUncertianty, sensorData.accZ, sensorData.barometric_altitude, 10, (sensorData.oriTime - sensorData.velTime)/1000000);
        sensorData.altitude = kalmanOutput[0];
        return;
    }
*/
    //sensorData.altitude = 
}

void updateOrientation() {
    //Acc roll and pitch
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

void lowPassOrientation() {
    //low pass filter factor
    double A = LOW_PASS_ALPHA; //add pin config 
    //'xk = A*'x(k-1) + (1-A)xk
    sensorData.orintationX = A*sensorData.orintationX+(1-A)*bmi323_data.gyroX*(bmi323_data.time-sensorData.oriTime)/1000000;
    sensorData.orintationY = A*sensorData.orintationY+(1-A)*bmi323_data.gyroY*(bmi323_data.time-sensorData.oriTime)/1000000;
    sensorData.orintationZ = A*sensorData.orintationZ+(1-A)*bmi323_data.gyroZ*(bmi323_data.time-sensorData.oriTime)/1000000;
}


void updateSensorData() {
    

    if(esp_timer_get_time()> 1*1000000){
        onPad = false;
    }

    sensorData.ldrVoltage = ldr_getVoltage();

    sensorData.batterVoltage = 1; //TODO once battery refac is done
    bmi323_getData();
    highg_getData();
    
    if(bmi323_data.accMag > 150) {
            onPad = false; 
    }

    if(onPad == true) {
        //moving avg = 'x(k-1) + (xk -x(k-n))/n

        struct accData_t current;

        current.x = bmi323_data.accX;
        current.y = bmi323_data.accY;
        current.z = bmi323_data.accZ;
        

        for(int i = MOVING_AVG_NUM-2; i >=0; i--) {
            accData[i] = accData[i+1];
        }

        accData[MOVING_AVG_NUM-1] = current;

        sensorData.accX = sensorData.accX + (current.x - accData[0].x)/MOVING_AVG_NUM;
        sensorData.accY = sensorData.accY + (current.y - accData[0].y)/MOVING_AVG_NUM;
        sensorData.accZ = sensorData.accZ + (current.z - accData[0].z)/MOVING_AVG_NUM;

        double accMag = sqrt(sensorData.accX*sensorData.accX + sensorData.accY*sensorData.accY + sensorData.accZ *sensorData.accZ);

        //calculate initial orientation
        //z vertical 
        //y left
        // x straight ahead

        

        sensorData.orintationX = (acos(sensorData.accX/accMag)/PI * 180) - 90;
        sensorData.orintationY = (acos(sensorData.accY/accMag)/PI * 180) - 90;
        sensorData.orintationZ = 0;

        return;
    }

    bmp280_getData();
    sensorData.temperature = bmp280_data.temperature;
    sensorData.pressure = bmp280_data.pressure;
    sensorData.barometric_altitude = bmp280_data.altitude;
    sensorData.barometerTime = bmp280_data.time;

    
    if(bmi323_data.accMag < 12) {
        updateAccData();
        updateOrientation();
        updateVelocity();
        return;
    }

    lowPassOrientation();
    updateAccData();
    updateVelocity();
}

//add bmi323 and highg preflight tests and register components
