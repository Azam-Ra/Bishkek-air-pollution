#pragma once
#include <iostream>
#include <string>

using namespace std;

class InstantR {
    string date;
    double abn[2];
    double pm[2]; // pm (particular matter) difference
    int streak, decrNum;
public:
    InstantR(string date_, double *pm_, int streak_, int decrNum_, double *abn_);
    double ratio() const;
};

InstantR::InstantR(string date_, double *pm_, int streak_, int decrNum_, double *abn_) {
    date = date_;
    streak = streak_;
    decrNum = decrNum_;
    pm[0] = pm_[0];
    pm[1] = pm_[1];
    abn[0] = abn_[0];
    abn[1] = abn_[1];
}

double InstantR::ratio() const {
    return (pm[0] - pm[1]) / pm[0];
}
