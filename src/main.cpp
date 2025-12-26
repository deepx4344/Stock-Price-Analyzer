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
    double simpleMovingAverage = 0;
    double exponentialMovingAverage = 0;
    double trueRange = 0;
    double change = 0.0;
    double averageGain = 0.0;
    double averageLoss = 0.0;
    double averageTrueRange = 0;
    double relativeStrengthIndex = 0;
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
         { return a.date < b.date; });
    LOG("Finished Sorting the Data");
}
void sortResult(std::vector<MAResult> &MAResults)
{
    sort(MAResults.begin(), MAResults.end(), [](const MAResult &a, const MAResult &b)
         { return a.date > b.date; });
}
int getSize(std::vector<stockDayData> &allStockData)
{
    return allStockData.size();
}

double calcInitialSum(const std::vector<stockDayData> &allStockData, const unsigned int &duration)
{
    double Sum = 0;
    int length = allStockData.size();
    if (length < duration || duration <= 0)
        throw std::invalid_argument("Duration Cannot be greater than the length of the dataSet");
    for (int i = 0; i < duration; i++)
    {
        std::cout << i << std::endl;
        Sum += allStockData[i].close;
    }
    return Sum;
}
void simpleMovingAverage(const std::vector<stockDayData> &allStockData, const unsigned int &duration, std::vector<MAResult> &MAResults)
{
    LOG("Analying Data");
    int length = allStockData.size();
    if (length < duration || duration <= 0)
        throw std::invalid_argument("Duration Cannot be greater than the length of the dataSet");
    MAResult current;
    for (int i = 0; i <= duration - 2; i++)
    {
        current.date = allStockData[i].date;
        current.open = allStockData[i].open;
        current.close = allStockData[i].close;
        current.high = allStockData[i].high;
        current.low = allStockData[i].low;
        current.name = allStockData[i].name;
        MAResults.push_back(current);
    }
    double Sum = calcInitialSum(allStockData, duration);
    current.date = allStockData[duration - 1].date;
    current.open = allStockData[duration - 1].open;
    current.close = allStockData[duration - 1].close;
    current.high = allStockData[duration - 1].high;
    current.low = allStockData[duration - 1].low;
    current.name = allStockData[duration - 1].name;
    current.simpleMovingAverage = Sum / duration;
    MAResults.push_back(current);
    for (int i = duration; i < length; i++)
    {
        current.date = allStockData[i].date;
        current.open = allStockData[i].open;
        current.close = allStockData[i].close;
        current.high = allStockData[i].high;
        current.low = allStockData[i].low;
        current.name = allStockData[i].name;
        Sum = Sum - allStockData[i - duration].close + allStockData[i].close;
        current.simpleMovingAverage = Sum / duration;
        MAResults.push_back(current);
    }
}

void exponentialMovingAverage(const std::vector<stockDayData> &allStockData, const unsigned int &duration, std::vector<MAResult> &MAResults)
{
    LOG("Now calculating exponentialMovingAverage");
    double multiplier = 2.0 / (duration + 1.0);
    double InitialSMA = (calcInitialSum(allStockData, duration)) / duration;
    int length = allStockData.size();
    MAResults[duration - 1].exponentialMovingAverage = InitialSMA;
    for (int i = duration; i < length; i++)
    {
        double exponentialMovingAverage = (MAResults[i].close * multiplier) + (MAResults[i - 1].exponentialMovingAverage * (1.0 - multiplier));
        MAResults[i].exponentialMovingAverage = exponentialMovingAverage;
    }
    LOG("Done With exponentialMovingAverage");
}
void calculateTrueRange(const unsigned int &duration, std::vector<MAResult> &MAResults)
{
    LOG("Now True Range");
    int length = MAResults.size();
    MAResults[0].trueRange = MAResults[0].high - MAResults[0].low;
    for (int i = 1; i < length; i++)
    {
        double highLow = MAResults[i].high - MAResults[i].low;
        double highClose = std::abs(MAResults[i].high - MAResults[i - 1].close);
        double lowPrevClose = std::abs(MAResults[i].low - MAResults[i - 1].close);

        double trueRange = std::max({highLow, highClose, lowPrevClose});
        MAResults[i].trueRange = trueRange;
    }
}
double calculateInitialAtr(const std::vector<MAResult> &MAResults, const unsigned int &duration)
{
    double Sum = 0.0;
    for (int i = 0; i < duration; i++)
    {
        Sum += MAResults[i].trueRange;
    }
    return Sum / duration;
}

void calculateAverageTrueRange(const unsigned int &duration, std::vector<MAResult> &MAResults)
{
    int atrSmoothning = duration;
    int length = MAResults.size();
    double InitialAtr = calculateInitialAtr(MAResults, atrSmoothning);
    MAResults[atrSmoothning - 1].averageTrueRange = InitialAtr;
    double averageTrueRange = 0.0;
    for (int i = atrSmoothning; i < length; i++)
    {
        averageTrueRange = ((MAResults[i - 1].averageTrueRange * 13) + MAResults[i].trueRange) / 14;
        MAResults[i].averageTrueRange = averageTrueRange;
    }
}
void calculateChange(std::vector<MAResult> &MAResults)
{
    int length = MAResults.size();
    for (int i = 1; i < length; i++)
    {
        MAResults[i].change = MAResults[i].close - MAResults[i - 1].close;
    }
}
void calculateAverages(std::vector<MAResult> &MAResults, const unsigned int &duration)
{
    int rsiSmoothning = duration;
    int length = MAResults.size();
    double Gain = 0.0;
    double Loss = 0.0;
    for (int i = 1; i <= rsiSmoothning; i++)
    {
        if (MAResults[i].change > 0)
        {
            Gain += MAResults[i].change;
        }
        else
        {
            Loss += std::abs(MAResults[i].change);
        }
    }
    MAResults[rsiSmoothning].averageGain = Gain / rsiSmoothning;
    MAResults[rsiSmoothning].averageLoss = Loss / rsiSmoothning;
}

void calculateRelativeStrengthIndex(const unsigned int &duration, std::vector<MAResult> &MAResults)
{
    calculateChange(MAResults);
    calculateAverages(MAResults, duration);
    int rsiSmoothning = duration;
    int length = MAResults.size();
    double relativeStrength = MAResults[rsiSmoothning].averageGain / MAResults[rsiSmoothning].averageLoss;
    double relativeStrengthIndex = 100 - (100 / (1 - relativeStrength));
    MAResults[rsiSmoothning].relativeStrengthIndex = relativeStrengthIndex;
    double averageGain = 0.0;
    double averageLoss = 0.0;
    for (int i = rsiSmoothning + 1; i < length; i++)
    {
        double gain;
        double loss;
        auto current = MAResults[i];
        if (current.change > 0)
        {
            gain = current.change;
        }
        else if (current.change < 0)
        {
            loss = current.change;
        }
        averageGain = ((MAResults[i - 1].averageGain * (rsiSmoothning - 1)) + gain) / rsiSmoothning;
        averageLoss = ((MAResults[i - 1].averageLoss * (rsiSmoothning - 1)) + std::abs(loss)) / rsiSmoothning;
        relativeStrength = averageGain / averageLoss;
        relativeStrengthIndex = 100 - (100 / (1 - relativeStrength));
        MAResults[i].relativeStrengthIndex = relativeStrengthIndex;
    }
}

void writeToFile(const std::vector<MAResult> &MAResults)
{
    LOG("Now writing to file");
    LOG(MAResults.size());
    std::string outputFilePath = "./output/xxx.csv";
    std::ofstream OutputFile(outputFilePath);
    OutputFile << "Name" << "," << "Date" << "," << "Close" << "," << "high" << "," << "low" << "," << "Simple Moving Average" << "," << "Exponential Moving Average" << "," << "True Range" << "," << "Average True Range" << "," << "Relative Strength Index" << std::endl;
    for (int i = 0; i < MAResults.size(); i++)
    {
        MAResult cur = MAResults[i];
        OutputFile << cur.name << "," << cur.date << "," << cur.close << "," << cur.high << "," << cur.low << "," << cur.simpleMovingAverage << "," << cur.exponentialMovingAverage << "," << cur.trueRange << "," << cur.averageTrueRange << "," << cur.relativeStrengthIndex << std::endl;
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
    calculateTrueRange(14, MAResults);
    // sortResult(MAResults);
    calculateAverageTrueRange(duration, MAResults);
    calculateRelativeStrengthIndex(duration, MAResults);
    writeToFile(MAResults);
    LOG("Processing Done");
    return 0;
}