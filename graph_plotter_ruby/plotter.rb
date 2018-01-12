# To change this license header, choose License Headers in Project Properties.
# To change this template file, choose Tools | Templates
# and open the template in the editor.
require 'rubygems'
require 'csv'
require 'gruff'


require 'rubygems'
require 'csv'
require 'gruff'
require_relative "bcrp_calc"

RESULT_DIR = "G:\\Documents and Settings\\Oleg\\My Documents\\RubyProjects\\Universal\\C\\results"
def readVecData(stockIndex)
    quoteTable = CSV.read(RESULT_DIR + "\\" + stockIndex + ".csv")
    
    sVec = Array.new
    quoteTable.each do |quoteRow|
      sn = quoteRow[1].to_f # get Adjusted end-of-day stock price 
      sVec << sn; # Build relative prices vector
    end
    puts "done parsing '#{stockIndex}....\n\n\n"
    return sVec
end

def addToPlot(stockName,sVec,g)
    g.data(stockName, sVec)   
end

def calcBestCrpPerformance(stocks,portfolio)
  numOfDays = stocks[0].size
  stockNum = stocks.size
  bestCRPVec = Array.new
  s = 1

  for i in (0..numOfDays-1)
      currDayProfit = 0
      for j in (0..stockNum-1)
        currDayProfit += portfolio[j]*stocks[j][i] 
      end
      s = s*currDayProfit
      bestCRPVec << s
  end
  return bestCRPVec
end

def calcStockPerformance(stockPriceRelVec)
  numOfDays =stockPriceRelVec.size
  sVec = Array.new
  s = 1

  for i in (0..numOfDays-1)
      currDayProfit = 0
      currDayProfit = 1*stockPriceRelVec[i] 
      s = s*currDayProfit
     sVec << s
  end
  return sVec
  puts "BIIB S: " + s.to_s
end

def addToPlotAverageIndex(bcrpEngine,g)
   stocks = bcrpEngine.getStockDb
   stockArray = bcrpEngine.getStockDb.values
   numOfDays = stockArray[0].size
   numOfStocks = stockArray.size
   averageVec = Array.new(numOfDays,0) 
   stocks.each do |stockNameAndVec|
     stockName = stockNameAndVec[0]
     stockPriceVec = stockNameAndVec[1]
     sVec = calcStockPerformance(stockPriceVec)
     i=0
     sVec.each do |s|
       averageVec[i] = averageVec[i] + s/numOfStocks 
       i += 1  
     end
  end
   g.data(" Average Index",averageVec)
end

def getBCRPHeadline(bestCRP,names)
  i = 0
  headline = "("
  bestCRP.each do |b|
    stockName = names[i]
    headline += "#{stockName} - #{b.to_s}," 
    i = i+1
  end
  headline += ")"   
  return headline
end

bcrpEngine = BCRPCalc.new
#stockNames = [['Agilent','a'],['Boston Scientific','bsx'],['Life Technologies Corp','life'],['Texas Instruments','txn'],['Motorola Solutions','msi']]
#stockNames = [['Adobe','ADBE'],['RHT','RHT'],['MedTronicks','MDT'],['NBIX','NBIX'],['ImmoniGen','IMMU'],["SQNM",'SQNM']]
stockNames = [['NBIX','NBIX'],['BSX','BSX'],['HPQ','HPQ'],['STJ','STJ']]
#stockNames = [['TXN','TXN'],['MDT','MDT'],['AAPL','AAPL'],['MSI','MSI']]
#stockNames = [['Agilent','a'],['Apple','AAPL']]
#stockNames = [['Agilent','a'],['Boston Scientific','bsx']]

stockNames.each do |currStock|
  bcrpEngine.parseCsvData(currStock[0],currStock[1])
end 

bestCRP = bcrpEngine.calcBestCRP
#universalProfitVec = bcrpEngine.calcUniversal

puts "Finshed Calculation."
headline = getBCRPHeadline(bestCRP,bcrpEngine.getStockDb.keys)
bestCRPProfitVec = calcBestCrpPerformance(bcrpEngine.getStockDb.values,bestCRP)
puts "Best CRP: #{headline}"
puts "BestCRP Profit: #{bcrpEngine.getMaxS.to_s}"
puts "Universal profit: #{bcrpEngine.getUniversalS.to_s}"  
puts "Plotting..."

numOfDays = 2519
g = Gruff::Line.new('800x500')
g.hide_line_markers
  g.hide_title
  g.top_margin = 10 # Empty space on the upper part of the chart
  g.bottom_margin = 30  # Empty space on the lower part of the chart
  g.legend_margin = 20
  g.x_axis_label = "Time in Days"
  g.y_axis_label = "Wealth"
  
  # graph.theme_odeo # available: theme_rails_keynote, theme_37signals, theme_odeo, or custom (fragile)
  g.theme_greyscale
  g.colors= %w(black blue red)
	#g.theme[:colors] =  %w(black blue red) 
	#    :colors => ['#3B5998'],
	#    :marker_color => 'silver',
	#    :font_color => '#333333',
	#    :background_colors => ['white', 'silver']
	#}
g.legend_font_size = 18
g.title_font_size = 12

stockNames = [['Universal','r_universal_C2_w_comission'],['BCRP','r_bcrp_C2_w_comission']]
stockNames.each do |currStock|
  sVec = readVecData(currStock[1])
  addToPlot(currStock[0],sVec,g)
end 

addToPlotAverageIndex(bcrpEngine,g);

fileName = 'G:\\Documents and Settings\\Oleg\\My Documents\\RubyProjects\\Universal\\test\\graph.png'
g.write(fileName)
puts "Finished plot."
