import System.Environment   
import Data.List
import Debug.Trace

convStrInt :: String -> Int
convStrInt = read 

bitMake :: Int -> Int -> Int -> [Int]
bitMake cur len index = do
  if( cur > len )
    then []
    else [(mod index 2)] ++ (bitMake (cur+1) len (div index 2))
    
check :: Int -> Int -> Int -> Bool -> [Int] -> Bool
check len dep idx flag nList = do
  if( nList == [] )
    then if( (idx == 0) && flag )
      then True
      else False
    else if( (head nList) == 0 )
      then if( (idx + 1) > dep )
        then False
        else if( (idx + 1) == dep )
          then (check len dep (idx+1) True (tail nList))
          else (check len dep (idx+1) flag (tail nList))
      else if( (head nList) == 1 )
        then if( idx > 0 )
          then (check len dep (idx-1) flag (tail nList))
          else False
        else error "Something Wrong"

expression :: Int -> Int -> Int -> Int -> Int
expression len dep index ret = do
  if( 0 <= index && index < 2^len )
    then do
      let nList = (bitMake 1 len index)
      if( check len dep 0 False nList )
        then (expression len dep (index+1) (ret+1))
        else (expression len dep (index+1) ret)
    else ret
  
main = do  
  args <- getArgs
  if( args == [] || (tail args) == [] )
    then error "Invalid Input"
    else do
      let len = ((map convStrInt args)!!0)
      let dep = ((map convStrInt args)!!1)
      
      if( (mod len 2) == 1 )
        then print 0
        else print (expression len dep 0 0)
      