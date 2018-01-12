#include "universal_proj.h"
using namespace std;

void BCRPEngine::parseCsvData(const string& stockName,const string& csvFileName)
{
    ifstream    csvFile;
    FloatArray  priceRelVec,priceVec;
    StringArray dates;
    int   lineNum = 0;
    float quote = -1;
    float prevQuote = -1;
    
    csvFile.open(string(tableDir_ + csvFileName + ".csv").c_str());
    if (!csvFile.is_open())
      myThrow ("Couldn't open file '" + string(tableDir_ + csvFileName + ".csv")+ "'!");
   
    if (stockNum_ != 0)
       lineNum = stockDates_[0].size()-1;
    
    string line;
    puts ("Parsing csv data for " + stockName +"...");
    getline(csvFile,line); // Skip header line.
    while(!csvFile.eof()) {
      StringArray lineVec;
      getline(csvFile,line);
      if (line.empty() || line == ",,,,,,")
        continue;
      
	  size_t pos = line.find_first_of(",");
      size_t prevPos = 0;
      while(pos != string::npos) {
        if (line.substr(prevPos,pos-prevPos) == "")
           myThrow("Issue with line in the csv file, found line with insufficient num of cells");
        
		lineVec.push_back(line.substr(prevPos,pos-prevPos));
        prevPos = pos+1;
        pos = line.find_first_of(",",prevPos);
        if (pos == string::npos) 
          lineVec.push_back(line.substr(prevPos));
      }
      
      string date = lineVec[0];
      quote = atof(lineVec[6].c_str());
      if (prevQuote == -1)
        prevQuote = quote;
      
	  float relQuote = prevQuote/quote;
      if (stockNum_ != 0) {
        if (date != stockDates_[0][lineNum]) {
          puts ("Notice: Removing redundant date '" + date + "' from stock's price vector.");
          continue;
        }
	  }
      priceRelVec.insert(priceRelVec.begin(),relQuote);
      priceVec.insert(priceVec.begin(),quote);
      dates.insert(dates.begin(),date);
      prevQuote = quote;
      
      if (stockNum_ != 0)
        --lineNum;
        
    }
    
    puts ("finished parsing, updating database...");
    stockNameToIndx_.insert(make_pair(stockName,stockNum_));
    indxToStockName_.push_back(stockName);
    stockArray_.push_back(priceRelVec);
    stockQuoteArray_.push_back(priceVec);
    stockDates_.push_back(dates);
    stockNum_ = stockArray_.size();
    bcrpCalcStartRanges_.push_back(0);
    bcrpCalcEndRanges_.push_back(0);
    
    puts("closing I/O file handler... ");
    puts("Checking stock data integrity...");
    
	int prevDataSize = stockArray_[0].size(); 
    numOfDays_ = prevDataSize;
    for (int j=1; j<stockNum_; j++) {
        int currDataSize = stockArray_[j].size();
        if (currDataSize != prevDataSize)
          myThrow ("The data size for some size didn't fit the size of the data of the other stocks. stock0 size is " + toStr(prevDataSize)
                 + " and stock"+toStr(j)+" data size is" + toStr(currDataSize)); 
    }
    puts ("All data found OK");
    csvFile.close();
}

void BCRPEngine::calcAvgIndex()
{
    float avgSn = 0;
    for (int j=0; j<stockNum_; j++) {
        avgSn += stockQuoteArray_[j][numOfDays_-1]/stockQuoteArray_[j][0];  
    }
    
    avgSn = avgSn/stockNum_;
    puts("Average Index is:" + toStr(avgSn));
    
}

void BCRPEngine::findDeltaAndPortfoliosRecursive(int stockIndex,float prevPortfolioSum, float prevPortfolioNegSum)
{
    float portfolioSum=prevPortfolioSum;
    float portfolioNegSum=prevPortfolioNegSum;
       
    for (float i=rangeStart_; i <= rangeEnd_; i+=step_) {
      if (i>0)
        portfolioSum = prevPortfolioSum+i;
      else
        portfolioNegSum = prevPortfolioNegSum-i;
      currPortfolio_[stockIndex] = i;
         
      if (portfolioSum > 1.0001)
         continue;
      if (portfolioNegSum > 1.0001)
         continue;
        
      if (stockIndex == stockNum_-1) {
        
        if (portfolioSum > 0.9999 && portfolioSum < 1.00001 ) {
          ++numOfPortfolios_;
          CRP crp;
          crp.setB(currPortfolio_);
          allPortfolios_.push_back(crp);
        }
      } else {
        findDeltaAndPortfoliosRecursive(stockIndex+1,portfolioSum,portfolioNegSum);
      }
    }
}

void BCRPEngine::findDeltaAndPortfolios()
{
    puts("Calculation integral delta and setting all valid portfolios...");
    puts("stockNum:" + toStr(stockNum_));
    numOfPortfolios_=0;
    currPortfolio_.insert(currPortfolio_.begin(),stockNum_,0);
    eodPortfolio_.insert(eodPortfolio_.begin(),stockNum_,0);
    findDeltaAndPortfoliosRecursive(0,0,0);
    puts("finished recursive calculation...");
    delta_ = (1/(float)numOfPortfolios_);  
    puts("Number of portfoilois (segments) is:" + toStr(numOfPortfolios_));
    puts("Delta is:" + toStr(delta_));
}   

void BCRPEngine::printCurrCRP()
{
   cout << "current CRP is: (";
   for (int i=0; i < currPortfolio_.size(); i++) {
       cout << currPortfolio_[i] << ",";
   }
   cout << ")" << endl;
}

void BCRPEngine::printBCRP()
{
   cout << "Best CRP is: (";
   for (int i=0; i < bestCrpPortfolio_.size(); i++) {
       cout << bestCrpPortfolio_[i] << ",";
   }
   cout << ")" << endl;
}

void BCRPEngine::calcEodPortfolio(float currDayProfit)
{
    FloatArray eodProtfolio(stockNum_,0);
    for (int j=0; j<stockNum_; j++) 
        eodPortfolio_[j] = eodPortfolio_[j]/currDayProfit;
}

float BCRPEngine::calcDayProfitWithCommission(FloatArray nextDayPortfolio,float currDayProfit,int n)
{
    float commissionFactor = 0;
    for (int j=0; j<stockNum_; j++) {
        float stockPrice = stockQuoteArray_[j][n];
        float bjDiff = nextDayPortfolio[j]-eodPortfolio_[j];
        if (bjDiff < 0)
          bjDiff = bjDiff*(-1);
        commissionFactor += bjDiff*c_/stockPrice;
    }
    float currDayProfitWithCommission = currDayProfit*(1-commissionFactor);
    return currDayProfitWithCommission;
}

float BCRPEngine::calcCrpS(FloatArray& sVec)
{
   sVec.clear();
   float s = 1;
   for (int i = 0; i < numOfDays_; i++) {
       float currDayProfit = 0;
       for (int j=0; j<stockNum_; j++) {
           float bj = currPortfolio_[j];
           float xij = stockArray_[j][i];
           eodPortfolio_[j] = bj*xij;
           if (bj>0) {
             currDayProfit += bj*xij;
           } else {
             currDayProfit += (-1)*bj*(1-xij);
             
           }
       }
       if (withCommission_ && i != numOfDays_-1) {
         calcEodPortfolio(currDayProfit);
         currDayProfit = calcDayProfitWithCommission(currPortfolio_,currDayProfit,i);
       }
       s = s*currDayProfit;
       sVec.push_back(s);
   }
   return s;
}

const FloatArray& BCRPEngine::calcBestCRP()
{
    ofstream    csvFile;
    FloatArray  sVec,bestSVec;
    csvFile.open(string(resultDir_ + "r_bcrp.csv").c_str());

    puts("calculating best CRP...");
    long int currPortfolioNum = 0;
    if (withCommission_)
      puts("calculating BCRP with transaction costs..."); 
  
    for (CRPArray::const_iterator crpIt = allPortfolios_.begin();
        crpIt != allPortfolios_.end(); crpIt++) {
        currPortfolio_ = crpIt->getB();
        
		float currS = calcCrpS(sVec);
        if (currS > bestCrpS_) {
          bestCrpS_ = currS;
          bestCrpPortfolio_ = currPortfolio_;
		  bestSVec = sVec;
        }
        ++currPortfolioNum;
        
		if ((currPortfolioNum%10000) == 0) 
          puts("Finished checking " + toStr(currPortfolioNum/10000) + "0k out of " + toStr(numOfPortfolios_) + "...");
    }
    
	printBCRP();
    for (int i=0; i<bestSVec.size(); i++) {
       csvFile  << i << "," << bestSVec[i] << endl; 
    }
    csvFile.close();  

    puts("Done. Best CRP Sn is " + toStr(bestCrpS_));
    return bestCrpPortfolio_;
}

void BCRPEngine::calcUniversalProfit()
{
    ofstream    csvFile;
    csvFile.open(string(resultDir_ + "r_universal.csv").c_str());
    
    float s = 1;
    for (int n=0; n<numOfDays_; n++) {
        puts("Day " + toStr(n));
        float currDayProfit = 0;
        float bSum = 0;
        for (int j=0; j<stockNum_; j++) {
           float bj = nDayPortfolio_[n][j];
           float xij = stockArray_[j][n];
           if (bj>0) {
             bSum += bj;
             currDayProfit += bj*xij;
           } else {
             currDayProfit += (-1)*(bj)*(1-xij);
           }
           eodPortfolio_[j] = bj*xij;
        }
        
        if (withShorts_) 
           currDayProfit += 1-bSum;
        
        puts("currDProfit:" + toStr(currDayProfit));  
        
        if (withCommission_ && n != numOfDays_-1) { 
          calcEodPortfolio(currDayProfit);
          currDayProfit= calcDayProfitWithCommission(nDayPortfolio_[n+1],currDayProfit,n);
        }
        
        s = s*currDayProfit;
        puts(" s is:" + toStr(s) + "bSum is:" + toStr(bSum));
        universalSVec_.push_back(s);
    }
    universalS_ = s;
    for (int i=0; i<universalSVec_.size(); i++) {
       csvFile  << i << "," << universalSVec_[i] << endl; 
    }
    csvFile.close();
}

float BCRPEngine::calcSnIncr(int n)
{
    float s = 1;
    float currDayProfit = 0;
    for(int j=0; j<stockNum_; j++) {
        float bj  = currPortfolio_[j];
        float xij = stockArray_[j][n];
        if (bj>0)
          currDayProfit += bj*xij;
        else
          currDayProfit += (-bj)*(1-xij); 
    }
    return currDayProfit;
}

void BCRPEngine::calcDayIntegral(int n)
{
    FloatArray currDayPortfolio(stockNum_,0);
    float sIntegral = 0;
    
    for (CRPArray::iterator crpIt = allPortfolios_.begin();
           crpIt != allPortfolios_.end(); crpIt++) {
        currPortfolio_ = crpIt->getB();
        float sn = crpIt->getS();
        float sIncr = calcSnIncr(n);
        sn = sn*sIncr;
        crpIt->setS(sn);
        sIntegral += sn*delta_;
        for (int j=0; j<stockNum_; j++) {
            currDayPortfolio[j] += currPortfolio_[j]*sn*delta_;                    
        }
    }
    nDayPortfolio_.push_back(currDayPortfolio);
    cout << " sIntegral: " + toStr(sIntegral);
    cout << " nPortfolio:(";
    for (int j=0; j<stockNum_; j++) {
        nDayPortfolio_[n+1][j] = (nDayPortfolio_[n+1][j])/sIntegral; 
        cout << toStr(nDayPortfolio_[n+1][j]) << ",";
    }
    cout << ") ";    
}

const FloatArray&  BCRPEngine::calcUniversal()
{
   puts("Start calculating Universal-Portfolio...");
   nDayPortfolio_.push_back(FloatArray(stockNum_,1/(float)stockNum_));
   for (int n=0; n<numOfDays_-1; n++) {
       cout << "calculating integral for day" + toStr(n) + "...";
       calcDayIntegral(n);
       puts("  finished calculation for day " + toStr(n));
   }
   puts("Finished calculating Universal-Portfolio.");
   puts("\nStart Calculating Universal-Portfolio profit...");
   calcUniversalProfit();
   puts("Finished Calculating Universal-Portfolio profit.");
   puts ("****************************************");
   puts("Universal S:" + toStr(universalS_));
   return universalSVec_;
}
