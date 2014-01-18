#include <iostream>

#include "Adafruit_ADS1015.h"

int main()
{
    Adafruit_ADS1115 ads;
    ads.beginContinuous(0);
    std::cout << ads.m_continuous << std::endl;
    return 0;
}
