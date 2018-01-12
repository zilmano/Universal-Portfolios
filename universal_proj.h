#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstdlib>
#include <cmath>

using namespace std;

// ** 
// Utility functions 
// **
inline void puts(const string& msg)
{
    cout << msg << endl; 
}
inline void myThrow(const string& msg) 
{
    cout << "ERROR: " + msg << endl;
    throw;
}

inline string toStr(float num)
{
  ostringstream stream;
  stream << num;
  return stream.str();
}

inline string toStr(long num)
{
  ostringstream stream;
  stream << num;
  return stream.str();
}

inline string toStr(int num)
{
  ostringstream stream;
  stream << num;
  return stream.str();
}

// **
// Constant Rebalanced Portfolio
// **
struct CRP
{
  // **
  // The variable naming follow the convetions of portfolio theory,
  // and are the same as used in the Universal Portfolio formula from Thomas Cover's paper.
  // S - factor of wealth increase.
  // B - wealth allocation vector. B = (b_1, b_2,...b_m), where b_i is the propotion of wealth invested in stock i
  //
  // **
  float s_;
  vector<float> b_;
  
  CRP(): s_(1) {};
  void setB(const vector<float>& b) {b_ = b;};
  void setS(float s) {s_ = s;};
  float getS() const { return s_;};
  vector<float> getB() const { return b_;};
};

typedef vector<CRP> CRPArray;
typedef vector<int>     IntArray;
typedef vector<float>   FloatArray;
typedef vector<string>  StringArray;
typedef map<string,int> StringIntMap;
typedef vector<IntArray>     IntArrayVec;
typedef vector<FloatArray>   FloatArrayVec;
typedef vector<StringArray>  StringArrayVec;  

// **
// BCRP - Best Constant Rebalanced Portfolio
// **
class BCRPEngine
{
  public:
    BCRPEngine(const string& tableDir,const string& resultDir,float step,bool withShorts,
               bool commission = false,float c = 0.005):
               tableDir_(tableDir),resultDir_(resultDir),step_(step),stockNum_(0),bestCrpS_(0),rangeStart_(0),rangeEnd_(1),
               withShorts_(withShorts),withCommission_(commission),c_(c) {
        if (withShorts)
          rangeStart_ = -1;        
    
    };
               
    void  parseCsvData(const string& stockName,const string& csvFile);
    void  findDeltaAndPortfolios();
    void  calcAvgIndex();
    const FloatArray& calcBestCRP();
    const FloatArray& calcUniversal();
    const FloatArray& calcBestCrpProfit();
    
  private:
    void findDeltaAndPortfoliosRecursive(int stockIndex,float prevPortfolioSum,float prevPortfolioNegSum);
    void printBCRP();
    void printCurrCRP();
    void  calcEodPortfolio(float currDayProfit);
    float calcDayProfitWithCommission(FloatArray nextDayPortfolio,float currDayProfit,int n);
    float calcCrpS(FloatArray& sVec);
    FloatArray calcBestCrp();
    void  calcDayIntegral(int n);
    float calcSnIncr(int n);
    void  calcUniversalProfit();
  
  private:
    int stockNum_;
    int numOfDays_;
    long int numOfPortfolios_;
    StringIntMap stockNameToIndx_;
    StringArray   indxToStockName_;
    FloatArrayVec stockArray_;
    FloatArrayVec stockQuoteArray_;
    StringArrayVec stockDates_;
      
    float bestCrpS_;
    float universalS_;
    float step_;
    float delta_;
    float rangeStart_;
    float rangeEnd_;
    FloatArray bestCrpPortfolio_;
    FloatArray currPortfolio_;
    FloatArray eodPortfolio_;
    FloatArray universalSVec_;
    CRPArray allPortfolios_;
    FloatArrayVec nDayPortfolio_;
    
    FloatArray bcrpCalcStartRanges_;
    FloatArray bcrpCalcEndRanges_;
    
    string tableDir_;
    string resultDir_;
    bool   withCommission_;
    bool   withShorts_;
    float c_;
};
