import System.Environment   
import Data.List

convStrInt :: String -> Int
convStrInt = read 

factors :: Int -> [Int]  
factors n = [x | x <- [1..n], n `mod` x == 0]

prime :: Int -> Bool  
prime n = factors n == [1, n]

primes :: Int -> [Int]
primes m = [x | x <- [m..], prime x]
  
main = do  
  args <- getArgs
  if( args == [] || (tail args) == [] )
    then error "Invalid Input"
    else mapM print (take ((map convStrInt args)!!1)  (primes ((map convStrInt args)!!0)) )