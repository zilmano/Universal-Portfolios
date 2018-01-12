# To change this license header, choose License Headers in Project Properties.
# To change this template file, choose Tools | Templates
# and open the template in the editor.

## s - Wealth increase factor
## b - Wealth allocation vector.b = (b_1, b_2,...b_m), where b_i is the propotion of wealth invested in stock i
class CRP
  def initialize
    @s = 1
    @b = Array.new
  end
  
  def getS
    @s
  end
  
  def getB
    @b
  end
  
  def setS(s)
    @s = s
  end
  
  def setB(b)
    @b = b
  end
end

class BCRPCalc
  TABLE_DIR =  "G:\\Documents and Settings\\Oleg\\My Documents\\RubyProjects\\Universal\\test\\tables"
  def initialize
    @stockDb = Hash.new
    @stockQuoteDb = Hash.new
    @stockNum = 0
    @stockArray = Array.new
    @stockDates = Array.new
    @stockKeys = Array.new
    @step = 0.1
    @delta = 0
    @portfolio = Array.new
    @nPortfolio = Array.new
    @bestPortfolio = Array.new
    @maxS = 0
    @calcProgress = 0
    @totalPortfolios = 0
    @numOfDays = 0
    @bCalcStartRanges = Array.new
    @bCalcEndRanges = Array.new
    @universalS = 0
    @universalSVec = Array.new
    @portfolios = Array.new
  end
  
  def getStockDb
    @stockDb  
  end
  
  def getStockQuoteDb
    @stockQuoteDb  
  end
  
  def getMaxS
    @maxS
  end
  def getUniversalS
    @universalS 
  end  
  
  def getUniversalSVec 
    @universalSVec  
  end  
  
  def parseCsvData(stockName,csvFile)
    quoteTable = CSV.read(TABLE_DIR + "\\" + csvFile + ".csv")
    quoteTable.shift; ## Skip header row
    
    priceRelativeVec = Array.new
    priceVec = Array.new
    dates =Array.new
    prevQuote = quoteTable[1][6].to_f; # First quote to calc relative prices
    puts "quote s: " + quoteTable[1][6] + " quote f:" + prevQuote.to_s
    if @stockNum != 0
      line=@stockDates[0].size-1
    end
    quoteTable.each do |quoteRow|
      date = quoteRow[0]
      quote = quoteRow[6].to_f # get Adjusted end-of-day stock price 
      currPriceRelative = prevQuote / quote; # get relative price
      if @stockNum != 0
        #clean stock vector
        if date != @stockDates[0][line]
          puts "gold date:#{@stockDates[0][line]} currDate:#{date}"
          puts "Notice: removing redundant date #{date} from curr stock's price vector"
          next
        end 
      end
      priceRelativeVec.unshift(currPriceRelative); # Build relative prices vector
      priceVec.unshift(quote)
      dates.unshift(date)  
      prevQuote = quote
      if @stockNum != 0
        line -= 1
      end
    end
    @stockDb[stockName] = priceRelativeVec
    @stockQuoteDb[stockName] = priceVec
    @stockDates << dates
    @stockNum = @stockDb.size
    @portfolio << 0
    @bCalcStartRanges << 0
    @bCalcEndRanges << 1
    puts "done parsing '#{stockName}....\n\n\n"
  end
  
  def printCRP
    print "  portfolio: ("
    @portfolio.each do |b|
      print "#{b},"
    end
    print ")\n"
  end
  
  def printBCRP
    print "  Best Portfolio: ("
    @bestPortfolio.each do |b|
      print "#{b},"
    end
    print ")\n"
  end
  
  def calcCRP
    bSum = 0
    @portfolio.each do |b|
      bSum += b
    end
    bi = 0
    @stockArray.each do |currPriceVec|
      if currPriceVec.size != @numOfDays
        di = 0
        @stockDates[0].each do |currDate|
          secondDate = @stockDates[bi][di]
          if secondDate != currDate
            puts "bad date - orig date: #{currDate}  diverging date: #{secondDate}"
            raise
          end
          di = di + 1
        end
        raise "Internal error. Err Id: 2"
      end
      bi = bi +1
    end
    if bSum != 1
      raise "Internal Error. Error Id:3 bSub: #{bSum.to_s}"
    end
    
    s = 1
    for i in 0..@numOfDays-1 do
      currDayProfit = 0
      for j in (0..@stockNum-1) do
        bi = @portfolio[j]
        xij = @stockArray[j][i]
        currDayProfit += bi* xij 
      end
      s = s*currDayProfit
    end
    return s
  end
  def calcBestCRPRecursive(stockIndex, prevPortfolioSum)
    
    rangeStart = @bCalcStartRanges[stockIndex]
    rangeEnd = @bCalcEndRanges[stockIndex]
    for i in (rangeStart..rangeEnd).step(@step) do
      @portfolio[stockIndex] = i
      portfolioSum = prevPortfolioSum + i
      if portfolioSum > 1
        return
      end
      if stockIndex == @stockNum-1
        if portfolioSum < 1
          next
        end
        currS = calcCRP
        
        if currS > @maxS
          @maxS = currS
          @bestPortfolio = @portfolio.dup 
        end
        return
      else
        if stockIndex == 1
          puts "#{@calcProgress} portfolios out of #{@totalPortfolios} checked...best Sn so far: " + @maxS.to_s
          @calcProgress += 10000
          printBCRP
        end
        calcBestCRPRecursive(stockIndex+1,portfolioSum)
      end
    end
  end
  
  def calcBestCRP
    @calcProgress = 0
    @stockArray = @stockDb.values
    @stockKeys = @stockDb.keys
    @totalPortfolios = (1/@step)**@stockNum
    @numOfDays = @stockArray[0].size
    calcBestCRPRecursive(0,0)
    
	i = 0
    @bestPortfolio.each do |b|
      @bCalcStartRanges[i] = [0,b-@step].max
      @bCalcEndRanges[i] = [1,b+@step].min
      i += 1
    end
    printBCRP
    @step = 0.01
    @calcProgress = 0
    puts "Starting Second Calculation Round..."
    calcBestCRPRecursive(0,0)
    printCRP
    puts "done. maxSn is: #{@maxS.to_s} BCRP is:"
    printBCRP
    return @bestPortfolio
  end
  
  def calcUniversalProfit
    @numOfDays = @stockArray[0].size
    @universalSVec = Array.new
    s = 1
    
	puts "start calculationg Universal total S..."
    for n in 0..@numOfDays-1 do
      puts "Day #{n.to_s}."
      currDayProfit = 0
      bSum = 0
      i=0
      @nPortfolio[n].each do |b|
        puts "b#{n.to_s}[#{i.to_s}]"
        bSum += b
        i += 1
      end
      if bSum > 1.001 || bSum < 0.999
        raise "Internal Error. Err Id: 4.  bSub: #{bSum.to_s}"
      end
      for j in (0..@stockNum-1) do
        bi = @nPortfolio[n][j]
        xij = @stockArray[j][n]
        currDayProfit += bi* xij 
      end
      s = s*currDayProfit
      @universalSVec << s
    end
    @universalS = s
  return @universalSVec
  end  
  
  def calcSnIncr(n)
    s = 1
    bSum = 0
    @portfolio.each do |b|
      bSum += b
    end
    if (bSum != 1)
      raise "calcSm: bSub is not one!!"
    end
    currDayProfit = 0
    for j in (0..@stockNum-1) do
      bi = @portfolio[j]
      xij = @stockArray[j][n]
      currDayProfit += bi* xij 
    end
    
    return currDayProfit
    
  end
  
  def calcSn(n)
    s = 1
    bSum = 0
    @portfolio.each do |b|
      bSum += b
    end
    if (bSum != 1)
      raise "Error: calcSm: bSub is not equal to 1, data missaligment."
    end
    for i in 0..n do
      currDayProfit = 0
      for j in (0..@stockNum-1) do
        bi = @portfolio[j]
        xij = @stockArray[j][i]
        currDayProfit += bi* xij 
      end
      s = s*currDayProfit
    end
    return s
    
  end
  
  def calcIntegral(n)
    puts "Calculating integaral for day #{(n+1).to_s}"
    @nPortfolio[n+1] = Array.new(@stockNum,0) 
    
    sIntegral = 0
    currS=0
    @portfolios.each do |crp|
       @portfolio = crp.getB
       sn = crp.getS
       sIncr = calcSnIncr(n)
       sn = sn*sIncr
       crp.setS(sn)
       sIntegral += sn*@delta
       for j in (0..@stockNum-1) do
         @nPortfolio[n+1][j] = @nPortfolio[n+1][j]+(@portfolio[j]*sn*@delta)
       end            
    end
    puts "Sintegral:" + sIntegral.to_s
    for j in (0..@stockNum-1) do
      @nPortfolio[n+1][j] = @nPortfolio[n+1][j]/sIntegral
      puts "b[#{(n+1).to_s}][#{j.to_s}]=#{@nPortfolio[n+1][j]}"   
    end
  end
    
  def calcIntegralDelta
    rangeStart = 0
    rangeEnd = 1-@step
    puts "Calculating integaral delta..."
    numOfSegments = 0
    for i0 in (rangeStart..rangeEnd).step(@step) do
      for i1 in (rangeStart..rangeEnd).step(@step) do
        if (i0+i1>1)
          break
        end
        for i2 in (rangeStart..rangeEnd).step(@step) do
          if (i0+i1+i2>1)
            break
          end
          for i3 in (rangeStart..rangeEnd).step(@step) do
            if (i0+i1+i2+i3>1)
              break
            end
            if (i0+i1+i2+i3==1)
              numOfSegments += 1
              currCRP = CRP.new
              b = Array.new
              b << i0
              b << i1
              b << i2
              b << i3
              currCRP.setB(b)
              @portfolios << currCRP 
            end
          end
        end
      end
    end
    puts "seg num is:" + numOfSegments.to_s
    @delta =  1/numOfSegments.to_f
    puts "Delta is:" + @delta.to_s
  end
  def calcUniversal
    @calcProgress = 0
    @stockArray = @stockDb.values
    @stockKeys = @stockDb.keys
    @totalPortfolios = (1/@step)**@stockNum
    @step = 0.01
    @numOfDays = @stockArray[0].size
    @nPortfolio = Array.new(@numOfDays,0)
    @nPortfolio[0] = Array.new(@stockNum,1/@stockNum.to_f)
    calcIntegralDelta
    for n in (0..@numOfDays-2) do
      calcIntegral(n)
      puts "finsihed calculation for day #{n.to_s}" 
    end
    return calcUniversalProfit  
  end
end
