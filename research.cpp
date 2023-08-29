#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "instantR.h"

using namespace std;

bool strcmpr(string s1, string s2)
{
    if (s1.length() != s2.length())
        return false;

    for (int i = 0; i < s1.length(); i++)
    {
        if (s1[i] != s2[i])
            return false;
    }
    return true;
}

// for cheching if the data in Air file is valid
bool validation(string s)
{
    if (strcmpr(s.substr(s.length() - 7, 7), "Invalid") == true)
        return false;
    return true;
}

void clearVector(vector<InstantR*> &v) {
    for (auto i = 0; i < v.size(); i++)  delete v[i];
    v.clear(); 
} 

double* research()
{
    vector<InstantR*> lev1; // hours with windspeed >= 20
    vector<InstantR*> lev2; // hours with windspeed >= 30
    vector<InstantR*> lev3; // hours with windspeed >= 40

    string windFilePath = "HighWindSpeeds2021.csv";
    string airFilePath = "AirBishkek2021.csv";
    ifstream winds, airs;

    try
    {
        winds.open(windFilePath);
        if (winds.fail())
            throw 1;
        airs.open(airFilePath);
        if (airs.fail())
            throw 2;
    }
    catch (int e)
    {
        if (e == 1)
            cerr << "Cannot open the file " << windFilePath << "!";
        if (e == 2)
            cerr << "Cannot open the file " << airFilePath << "!";
    }

    string date, hour;
    int streak = 1;
    double windspeed = 0;
    int decrNum = 0; // for how long the raw conc of pm was decreasing
    double pm[2];    // pm[0] & pm[1] hihgest & lowest pm concentrations, respectively
    double abn[2];   // 0) the max pm.conc. in the streak, which is abnormal if it is not pm[0]
    abn[1] = 0;      // 1) for counting cases when pm conc. increases in the streak

    string dump, aircheck; // to skip info in the file
    getline(winds, dump);
    streampos wpos = winds.tellg();
    streampos apos;

    while (winds.peek() != EOF)
    {
        winds.seekg(wpos, ios::beg);

        string wdate, whour;
        winds.ignore(1000, ','); // to skip location "Bishkek,"
        getline(winds, wdate, 'T');
        getline(winds, whour, ':');
        if (streak == 1)
        {
            date = wdate;
            hour = whour;
        }

        string windstr;
        for (int i = 0; i < 12; i++)
        {
            getline(winds, windstr, ',');
            if (windstr[0] == '"')
            {                            // in "rain,snow" for preciptype case
                winds.ignore(1000, '"'); // have to be careful with constant limitation
                winds.ignore(1000, ',');
            }
        }
        if (streak == 1)
            windspeed = stod(windstr);
        else
            windspeed = (windspeed + stod(windstr)) / 2;

        getline(winds, dump); // to go to the next line
        wpos = winds.tellg();

        // checking for a streak
        string ndate, nhour;
        winds.ignore(1000, ','); // to skip location "Bishkek,"
        getline(winds, ndate, 'T');
        getline(winds, nhour, ':');
        if (strcmpr(wdate, ndate) == true && stoi(whour) + 1 == stoi(nhour))
        {
            streak++;
        }
        else if (ndate.length() == 0)
        { // the end of the file
            break;
        }
        else if (stoi(wdate.substr(8, 2)) + 1 == stoi(ndate.substr(8, 2)) && strcmpr(whour, "23") == true && strcmpr(nhour, "00") == true)
        {
            streak++;
        }
        else
        {
            // finding raw pm conc. at the time
            int exis = 0;
            string adate, ahour;
            while (strcmpr(date, adate) == false || strcmpr(hour, ahour) == false)
            {
                getline(airs, dump); // first skip headers, then skip the rest of the line

                apos = airs.tellg();

                for (int i = 0; i < 2; i++)
                    airs.ignore(1000, ',');
                getline(airs, adate, ' ');
                for (int i = 0; i < 5; i++)
                    getline(airs, ahour, ',');

                exis++;
                if (exis > 8364)
                { // 8365 is the # of lines in the Air file
                    cerr << "The " << wdate << " date was not found in the Air file!";
                    exit(1); // there are no ways for the user to fix the problem
                }
            }

            // checking for the valid data
            airs.seekg(apos, ios::beg); // to come from checking the date
            getline(airs, aircheck);
            airs.seekg(apos, ios::beg);
            int acheck = 0;
            while (acheck < streak)
            {
                apos = airs.tellg();
                if (validation(aircheck) != true)
                    getline(airs, aircheck);
                else
                    break;
                acheck++;
            }
            airs.seekg(apos, ios::beg); // to start from the line with valid data

            if (acheck < streak)
            { // if we have no valid data in the streak (acheck>=streak), we can just move on
                string pmstr;
                for (int i = 0; i < 11; i++)
                    getline(airs, pmstr, ',');
                pm[0] = stod(pmstr);
                if (pm[0] < 0)
                    pm[0] = 0; // sensor sometimes puts negative values
                pm[1] = pm[0];
                abn[0] = pm[0];
                getline(airs, dump);

                // checking for number of hours that pm conc. was decreasing from the start of the streak
                streampos ntls = airs.tellg(); // not to lose data for streak check
                int vdcheck = 1;
                while (true)
                {
                    apos = airs.tellg();
                    getline(airs, aircheck);
                    airs.seekg(apos, ios::beg);

                    if (validation(aircheck) == true)
                    { // if the data is invalid, we can just move on
                        string npmstr;
                        for (int i = 0; i < 11; i++)
                            getline(airs, npmstr, ',');
                        if (stod(npmstr) < pm[1])
                        {
                            decrNum++;
                            pm[1] = stod(npmstr);
                        }
                        else if (stod(npmstr) >= pm[1] && vdcheck < streak)
                        {
                            abn[1]++;
                            if (stod(npmstr) > abn[0])
                                abn[0] = stod(npmstr);
                        }
                        else if (stod(npmstr) >= pm[1] && vdcheck >= streak)
                        {
                            break;
                        }
                    }
                    vdcheck++;
                    getline(airs, dump);
                }
                airs.seekg(ntls);
            }
            if (pm[1] < 0)  pm[1] = 0; // sometimes sensor puts negative values

            InstantR *r = new InstantR(date, pm, streak, decrNum, abn);
            if (windspeed >= 20 && windspeed <= 30)
                lev1.push_back(r);
            else if (windspeed >= 30 && windspeed <= 40)
                lev2.push_back(r);
            else
                lev3.push_back(r);

            streak = 1;
            decrNum = 0;
            abn[1] = 0;
            windspeed = 0;
        }
    }
    
    // finding the different ratios based on the windspeed
    double* res = new double[3];
    double sum = 0;
    for (auto it : lev1)  sum += it->ratio();
    res[0] = sum / lev1.size();

    sum = 0;
    for (auto it : lev2)  sum += it->ratio();
    res[1] = sum / lev2.size();

    sum = 0;
    for (auto it : lev3)  sum += it->ratio();
    res[2] = sum / lev3.size();

    clearVector(lev1);
    clearVector(lev2);
    clearVector(lev3);

    return res;
}

int main()
{
    double* result = research();
    for (int i = 0; i < 3; i++) 
        cout << "The ratio, when the windspeed was more than " << (i+2)*10 << "km/h : " << result[i] << endl;
    return 0;
}