/* 
 * File:   Weather.h
 * Author: philippe
 *
 * Created on 11 août 2021, 15:40
 */

#ifndef WEATHER_H
#define WEATHER_H
#include <Position.h>"


class Weather : public Position {

    public:
    Weather(String _callsign,String _destination,String _path,const double _latitude,const double _longitude );
    

    virtual ~Weather();
    
    void setWind(int  _wind);
    void setSpeed(double _speed);
    void setGust(double _gust);
    void setRain(double  _rain);
    void setRain24(double  _rain24);
    void setRainMid(double  _rainMid);
    void setTemp(double  _temp);
    void setHumidity(double _humidity);
    void setPressure(double _pressure);
	//int getPduLength();
    char* getPduWx();

    private:
    int     wind;         // direction du vent
    int  speed;        // vitesse du vent
    int  gust;         // Pointe de vitesse
    int  temp;         // température en °F
    int  rain;         // Pluie en mm
    int  rain24;       // Pluie sur 24h
    int  rainMid;      // Pluie depuis minuit
    int  humidity;     // Humidité en %
    int  pressure;     // Pression barométrique    
        

};

#endif /* WEATHER_H */
