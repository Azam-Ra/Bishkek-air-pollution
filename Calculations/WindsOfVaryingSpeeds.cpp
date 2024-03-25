#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <ios>  // used to get streamsize
#include <limits>  // used to get numeric limits
#include <ctime>

using namespace std;

class DateTime {
    private:
        // we follow 24-hour clock convention, where 23:59 translates into 00:00
        int year, month, day, hour;

        tm makeTM(int year, int month, int day, int hour) {
            tm tm = {0};
            tm.tm_year = year - 1900;
            tm.tm_mon = month - 1;
            tm.tm_mday = day;
            tm.tm_hour = hour;

            return tm;
        }

    public:
        DateTime(int year, int month, int day, int hour) {
            if (year < 2019 || year > 2023) 
                throw invalid_argument("The value of a year cannot be < 2019 or > 2023!");
            this->year = year;

            if (month < 1 || month > 12) 
                throw invalid_argument("The value of a month cannot be < 1 or > 12!");
            this->month = month;

            if (day < 1 || day > 31) 
                throw invalid_argument("The value of a day cannot be < 1 or > 31!");
            this->day = day;

            if (hour < 0 || hour > 23) 
                throw invalid_argument("The value of an hour cannot be < 0 or > 23!");
            this->hour = hour;
        }

        int* getParameters() {
            return new int[4]{year, month, day, hour};
        }

        // gives the difference between DateTime objects measured in hours
        double compare(DateTime& dateTime) {
            const int secondsInHour = 60*60;

            int* parameters = dateTime.getParameters();

            tm tm1 = makeTM(year, month, day, hour);
            tm tm2 = makeTM(parameters[0], parameters[1], parameters[2], parameters[3]);

            delete parameters;

            time_t time1 = mktime(&tm1);
            time_t time2 = mktime(&tm2);

            if (time1 == -1 || time2 == -1)
                throw new invalid_argument("The `tm` argument to covert into `time_t` was invalid!");

            return difftime(time1, time2) / secondsInHour;
        }
};

class CSVRow {
    private:
        string row;

    public:
        CSVRow(string row) {
            this->row = row;
        }

        string getRow() {
            return row;
        }

        // indexing starts from 0
        string getToken(int tokenPosition) {
            if (tokenPosition < 0) 
                throw invalid_argument("The token position value cannot be negative!");

            const int rowLength = row.length();

            int currentPosition = 0;
            int index = 0;
            while (index < rowLength) {
                if (currentPosition == tokenPosition) {
                    int startIndex = index;

                    // we assume that non-delimeter commas can occur only inside ""
                    if (row.at(startIndex) == '"') {
                        // we assume that quotation marks are always complete
                        // i.e. if opened, must be closed
                        while (row.at(++index) != '"');
                        return row.substr(startIndex, index - startIndex + 1);
                    }

                    while (index < rowLength && row.at(index) != ',') index++;
                    return row.substr(startIndex, index - startIndex);
                }

                if (row.at(index) == '"') {
                    while (row.at(index) != '"') index++;
                    index++;
                }
                if (row.at(index) == ',') {
                    currentPosition++;
                }

                index++;
            }

            throw out_of_range("Position exceeding the number of tokens was passed!");
        }

        bool isValid(int validationTokenPosition, string validationWord) {
            if (validationWord.compare(this->getToken(validationTokenPosition)) == 0)
                return true;
            return false;
        }

        bool isEmpty() {
            return (row.empty()) ? true : false;
        }

        DateTime getDateTime(int CSVRowType) {
            if (CSVRowType == 0) {
                string dateTimeStr = this->getToken(1);

                DateTime dateTime(stoi(dateTimeStr.substr(0, 4)), stoi(dateTimeStr.substr(5, 2)), 
                    stoi(dateTimeStr.substr(8, 2)), stoi(dateTimeStr.substr(11, 2)));

                return dateTime;
            } else if (CSVRowType == 1) {
                string dateStr = this->getToken(3);

                DateTime dateTime(stoi(dateStr.substr(0, 4)), stoi(dateStr.substr(5, 2)), 
                    stoi(dateStr.substr(8, 2)), stoi(this->getToken(6)));

                return dateTime;
            } else {
                throw invalid_argument("You have passed a non-supported value for CSVRow type!");
            }
        }
};

class CSVIterator {
    private:
        istream& stream;

    public:
        CSVIterator(istream& stream) : stream(stream) {}

        bool isMoreData() {
            return (stream.peek() != EOF) ? true : false;
        }

        string getLine() {
            string line; 
            getline(stream, line);
            return line;
        }
};

void analyse(CSVIterator& weatherIterator, CSVIterator& airIterator) {
    // to be implemented
}

int main() {
    ifstream weatherStream("../Data/WeatherBishkek2019.csv");
    ifstream airStream("../Data/AirBishkek2019.csv");

    // to ignore header rows
    weatherStream.ignore(numeric_limits<streamsize>::max(), '\n');  
    airStream.ignore(numeric_limits<streamsize>::max(), '\n');

    CSVIterator weatherIterator(weatherStream);
    CSVIterator airIterator(airStream);

    analyse(weatherIterator, airIterator);

    return 0;
}