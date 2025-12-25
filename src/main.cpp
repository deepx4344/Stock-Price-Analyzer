#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstdlib>
#include <cmath>

#define LOG(X) std::cout << X << std::endl
const std::string DataWarning = "Not enough Data";

struct stockDayData
{
    std::string date;
    double open;
    double high;
    double low;
    double close;
    long long volume;
    std::string name;
};
struct MAResult
{
    std::string date;
    std::string name;
    double high;
    double low;
    double open;
    double close;
    double SMA = 0;
    double EMA = 0;
    double TrueRange = 0;
};

void readFileAndParseThenPushToVector(std::string &path, std::vector<stockDayData> &allStockData)
{
    LOG("Reading from file and parsing");
    std::string lineFromFile;
    int lineNumber = 0;
    std::ifstream myFile(path);
    if (!myFile.is_open())
    {
        std::cerr << "could not open file at " << path << std::endl;
        std::exit(EXIT_FAILURE);
    }
    while (getline(myFile, lineFromFile))
    {
        lineNumber++;
        if (lineNumber == 1)
        {
            continue;
        }
        stockDayData currentDay;
        std::stringstream ss(lineFromFile);
        std::string segment;
        int columnNumber = 0;
        try
        {
            while (getline(ss, segment, ','))
            {
                switch (columnNumber)
                {
                case 0:
                    currentDay.date = segment;
                    break;
                case 1:
                    currentDay.open = stod(segment);
                    break;
                case 2:
                    currentDay.high = stod(segment);
                    break;
                case 3:
                    currentDay.low = stod(segment);
                    break;
                case 4:
                    currentDay.close = stod(segment);
                    break;
                case 5:
                    currentDay.volume = stoll(segment);
                    break;
                case 6:
                    currentDay.name = segment;
                    break;
                }
                columnNumber++;
            }
            if (ss.peek() != std::stringstream::traits_type::eof())
            {
                std::cerr << "Warning: Extra data detected on " << lineNumber << std::endl;
            }
            allStockData.push_back(currentDay);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: Malformed data on line " << lineNumber << ": " << e.what() << std::endl;
        }
    }
    LOG("Finished Reading the File");
}

void sortVector(std::vector<stockDayData> &allStockData)
{
    LOG("Now Sorting the Data...");
    sort(allStockData.begin(), allStockData.end(), [](const stockDayData &a, const stockDayData &b)
         { return a.date > b.date; });
    LOG("Finished Sorting the Data");
}
int getSize(std::vector<stockDayData> &allStockData)
{
    return allStockData.size();
}

double calcInitialSum(const std::vector<stockDayData> &allStockData, const unsigned int &duration, bool fromBack)
{
    double Sum = 0;
    int length = allStockData.size();
    if (length < duration || duration <= 0)
        return -1;
    if (fromBack)
    {
        for (int i = length - 1; i >= length - duration; i--)
        {
            Sum += allStockData[i].close;
        }
    }
    else
    {
        for (int i = 0; i < duration; i++)
        {
            Sum += allStockData[i].close;
        }
    }
    return Sum;
}

void simpleMovingAverage(const std::vector<stockDayData> &allStockData, const unsigned int &duration, std::vector<MAResult> &MAResults)
{
    LOG("Analying Data");
    int length = allStockData.size();
    if (length < duration || duration <= 0)
        return;
    double Sum = calcInitialSum(allStockData, duration, false);
    MAResult current;
    current.date = allStockData[0].date;
    current.open = allStockData[0].open;
    current.close = allStockData[0].close;
    current.high = allStockData[0].high;
    current.low = allStockData[0].low;
    current.name = allStockData[0].name;
    current.SMA = Sum / duration;
    MAResults.push_back(current);
    for (int i = 1; i <= length - duration; i++)
    {
        current.date = allStockData[i].date;
        current.close = allStockData[i].close;
        current.name = allStockData[i].name;
        Sum = Sum - allStockData[i - 1].close + allStockData[i + duration - 1].close;
        current.SMA = Sum / duration;
        MAResults.push_back(current);
    }
    LOG("Finished Analying Data");
}

void exponentialMovingAverage(const std::vector<stockDayData> &allStockData, const unsigned int &duration, std::vector<MAResult> &MAResults)
{
    LOG("Now calculating EMA");
    double multiplier = 2.0 / (duration + 1.0);
    double InitialSMA = (calcInitialSum(allStockData, duration, true)) / duration;
    int length = allStockData.size();
    int total = length - duration + 1;
    MAResults[total - 1].EMA = InitialSMA;
    for (int i = total - 2; i >= 0; i--)
    {
        double EMA = (MAResults[i].close * multiplier) + (MAResults[i + 1].EMA * (1.0 - multiplier));
        MAResults[i].EMA = EMA;
    }
    LOG("Done With EMA");
}
void calculateTrueRange(const unsigned int &duration, std::vector<MAResult> &MAResults)
{
    LOG("Now True Range");
    int length = MAResults.size() - 2;
    for (int i = length; i >= 0; i--)
    {
        double highLow = MAResults[i].high - MAResults[i].low;
        double highClose = std::abs(MAResults[i].high - MAResults[i + 1].close);
        double lowPrevClose = std::abs(MAResults[i].low - MAResults[i + 1].close);

        double trueRange = std::max({highLow, highClose, lowPrevClose});
        MAResults[i].TrueRange = trueRange;
    }
}
// double calculateInitialAtr(const std::vector<stockDayData> &allStockData, const unsigned int &duration)
// {
//     double InitialAtr = 0;
//     int length = allStockData.size() - 1;
//     int last = 0;
//     for (int i = length; i > length - duration; i--)
//     {
//         // InitialAtr += MAResults[i].TrueRange;
//         last = i;
//     }
//     std::cout << "last >> " << last << std::endl;
//     InitialAtr = InitialAtr / duration;
// }

void calculateAverageTrueRange(const unsigned int &duration, std::vector<MAResult> &MAResults)
{
    double InitialAtr = 0;
}

void writeToFile(const std::vector<MAResult> &MAResults)
{
    LOG("Now writing to file");
    LOG(MAResults.size());
    std::string outputFilePath = "./output/xxx.csv";
    std::ofstream OutputFile(outputFilePath);
    OutputFile << "Name" << "," << "Date" << "," << "Close" << "," << "Simple Moving Average" << "," << "Exponential Moving Average" << "," << "True Range" << std::endl;
    for (int i = 0; i < MAResults.size(); i++)
    {
        MAResult cur = MAResults[i];
        OutputFile << cur.name << "," << cur.date << "," << cur.close << "," << cur.SMA << "," << cur.EMA << "," << cur.TrueRange << std::endl;
    }
}

int main()
{
    std::string path;
    int duration;
    std::cout << "Input Your File Path >> ";
    std::getline(std::cin, path);
    std::cout << "Input Your Preferred duration >> ";
    std::cin >> duration;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    LOG("Processing Data......");
    std::vector<stockDayData> allStockData;
    std::vector<MAResult> MAResults;
    readFileAndParseThenPushToVector(path, allStockData);
    sortVector(allStockData);
    simpleMovingAverage(allStockData, duration, MAResults);
    exponentialMovingAverage(allStockData, duration, MAResults);
    calculateTrueRange(duration, MAResults);
    // writeToFile(MAResults);
    calculateAverageTrueRange(duration, MAResults);
    LOG("Processing Done");
    return 0;
}