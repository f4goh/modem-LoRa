/* 
 * File:   Weather.cpp
 * Author: philippe S (Touchard Washington)
 * 
 * Created on 11 août 2021, 15:40
 */


#include <Print.h>

#include "Weather.h"

Weather::Weather(String _callsign,String _destination,String _path,const double _latitude, const double _longitude) :
Position(_callsign, _destination, _path,_latitude, _longitude, "", '/', '_'),
wind(NAN),
speed(NAN),
gust(NAN),
temp(NAN),
rain(NAN),
rain24(NAN),
rainMid(NAN),
humidity(NAN),
pressure(NAN) {
}

Weather::~Weather() {
}

void Weather::setWind(int  _wind){
    wind = _wind;    
}

/**
 * 
 * @param _speed  vitesse du vent en m/s
 */
void Weather::setSpeed(double _speed){
    speed = (int) (2.2369 * _speed);     // 1 m/s = 2.2369 mph
}

void Weather::setGust(double _gust){
    gust = (int) (2.2369 * _gust);
}

void Weather::setTemp(double _temp) {
    temp = (int) (_temp * 9 / 5 + 32); // °F = °C x 9/5 + 32
}
/**
 * 
 * @param _rain     rain in last hour  (mm)
 */
void Weather::setRain(double  _rain){
    rain =(int) (_rain * 3.937);   // 1mm = 3.937 centièmes de pouce
}

void Weather::setRain24(double  _rain24){
    rain24 =(int) (_rain24 * 3.937);   // 1mm = 3.937 centièmes de pouce
}

void Weather::setRainMid(double  _rainMid){
    rainMid =(int) (_rainMid * 3.937);   // 1mm = 3.937 centièmes de pouce
}

void Weather::setHumidity(double _humidity){
    humidity = (int) _humidity;
}

void Weather::setPressure(double _pressure){
    pressure = (int) (_pressure * 10);
}

/*
int Weather::getPduLength(){
   return pduLength; 
}
*/

char* Weather::getPduWx() {
    char com[44];  //a reduire
    char data[100];
    char swind[4]     = "...";
    char sspeed[4]    = "...";
    char sgust[5]     = "g...";
    char stemp[5]     = "t...";
    char srain[5]     = "r...";
    char srain24[5]   = "p...";
    char srainMid[5]  = "P...";
    char shumidity[4] = "h..";
    char spressure[7] = "b.....";
    
    if (wind != -1 )      snprintf(swind,     sizeof (swind),     "%03d",     wind);
    if (!isnan(speed))    snprintf(sspeed,    sizeof (sspeed),    "%03d",  speed);
    if (!isnan(gust))     snprintf(sgust,     sizeof (sgust),     "g%03d", gust);
    if (!isnan(temp))     snprintf(stemp,     sizeof (stemp),     "t%03d", temp);
    if (!isnan(rain))     snprintf(srain,     sizeof (srain),     "r%03d", rain);
    if (!isnan(rain24))   snprintf(srain24,   sizeof (srain24),   "p%03d", rain24);
    if (!isnan(rainMid))  snprintf(srainMid,  sizeof (srainMid),  "P%03d", rainMid);
    if (!isnan(humidity)) snprintf(shumidity, sizeof (shumidity), "h%02d", humidity);
    if (!isnan(pressure)) snprintf(spressure, sizeof (spressure), "b%05d", pressure);
    
    snprintf(data, sizeof (data), "%s/%s%s%s%s%s%s%s%s", 
            swind, sspeed, sgust, stemp, srain, srain24, srainMid, shumidity, spressure);
    
    latitude_to_str();
    longitude_to_str();
    comment.toCharArray(com, 43);  //43 trop voir doc
    
	char headerArray[50];
    String header;
    header=callsign+">"+destination+","+path;
    header.toCharArray(headerArray,50);
	
	snprintf(pdu, sizeof (pdu), "%s:!%s%c%s%c%s%s", headerArray,slat, symboleTable, slong, symbole, data, com);
    pduLength=strlen(pdu);
    return pdu;
}
