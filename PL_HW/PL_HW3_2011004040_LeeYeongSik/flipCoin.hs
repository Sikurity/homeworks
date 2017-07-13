import System.Environment   
import Data.List
import Debug.Trace

rmplTX :: String -> String
rmplTX str = 
  if str == []
    then []
    else if (head str) == 'T'
      then ['X'] ++ rmplTX(tail str)
      else ['H'] ++ rmplTX(tail str)
      
rmplHT :: String -> String
rmplHT str = 
  if str == []
    then []
    else if (head str) == 'H'
      then ['T'] ++ rmplHT(tail str)
      else [(head str)] ++ rmplHT(tail str)
      
rmplXH :: String -> String
rmplXH str = 
  if str == []
    then []
    else if (head str) == 'X'
      then ['H'] ++ rmplXH(tail str)
      else ['T'] ++ rmplXH(tail str)
      
flip1N :: Int -> String -> String
flip1N index str = do
  rmplXH(rmplHT(rmplTX (reverse (take (index+1) str)))) ++ (drop (index+1) str)

checkStr :: Int -> String -> Bool
checkStr index str =
  if index < length str
    then 
      if (str!!index == 'H')
        then checkStr (index+1) str
        else if (str!!index == 'T')
          then False
          else error "Invalid Input"
    else True
    
findLastT :: Int -> Int -> String -> Int
findLastT last index str =
  if index < length str
  then 
    if (str!!index == 'T')
      then findLastT index (index+1) str
      else findLastT last (index+1) str
  else last

findLastHT :: Int -> Int -> String -> Int
findLastHT last index str =
  if index < length str
  then if and [str!!index == 'T', (str!!(index-1)) == 'H']
    then (findLastHT (index-1) (index+1) str)
    else (findLastHT last (index+1) str)
  else last
    
flipCoin :: String -> [Int]
flipCoin str =
  if (checkStr 0 str)
    then [0]
  else if (head str) == 'T'
    then do
      let lastT = (findLastT 0 0 str)
      (lastT+1) : (flipCoin (flip1N lastT str))
    else if (head str) == 'H'
      then do
        let lastHT = (findLastHT 0 0 str) 
        (lastHT+1) : (flipCoin (flip1N lastHT str))
      else error "Invalid Input"

main = do
  args <- getArgs
  if args == []
    then print [0]
    else print (flipCoin (args!!0))