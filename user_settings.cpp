#include "universal_proj.h"
int main()
{
  /************************************/
  /*********** User Settings **********/
  /************************************/
  string tableDir = "./tables/"; // dir for input stock data csv      
  string resultDir = "./results/"; // dir for output
  bool transactionCosts = false;
  bool withShorts = true;
  float commission = 0.005;
  /************************************/
  /************************************/
  BCRPEngine* bcrpEngine = new BCRPEngine(tableDir,resultDir,0.01,withShorts,transactionCosts,commission);
  puts("Start parsing csv data...");
  
  /************************************/
  /********* Set stocks Here ***********/
  /************************************/

  bcrpEngine->parseCsvData("NOK","NOK");
  bcrpEngine->parseCsvData("IMMU","IMMU");
  bcrpEngine->parseCsvData("BSX","BSX");
  
  /*bcrpEngine->parseCsvData("NBIX","NBIX");
  bcrpEngine->parseCsvData("BSX","BSX");
  bcrpEngine->parseCsvData("HP","HPQ");
  bcrpEngine->parseCsvData("STJ","STJ");*/
  
  /*bcrpEngine->parseCsvData("TXN","TXN");
  bcrpEngine->parseCsvData("MDT","MDT");
  bcrpEngine->parseCsvData("AAPL","AAPL");
  bcrpEngine->parseCsvData("MSI","MSI");*/
  
  /************************************/
  /************ Main flow *************/
  /************************************/
  puts("Finished parsing csv data.");
  
  puts("Start finding delta and all valid portfolios...");
  bcrpEngine->findDeltaAndPortfolios();
  puts("Finished finding delta and all valid portfolios.");
 
  puts("Start calculating best CRP");
  bcrpEngine->calcBestCRP();
  puts("Finished calculating best CRP");
  
  puts ("Start calculating universal protfolio...");
  bcrpEngine->calcUniversal();
  bcrpEngine->calcAvgIndex();
  puts ("****************************************");
   
  delete bcrpEngine;
  return 0;
}
