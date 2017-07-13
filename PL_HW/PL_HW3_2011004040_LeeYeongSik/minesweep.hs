import System.Environment
import Data.List
import Debug.Trace

convStrInt :: String -> Int
convStrInt = read 

checkInput :: String -> Int -> Bool
checkInput str index = do
  if( index < length str )
    then if( str!!index == '.' || str!!index == '*' || str!!index == '\\' )
      then checkInput str (index+1)
      else False
    else
      True

replPZ :: String -> String
replPZ str = 
  if str == []
    then []
    else if (head str) == '.'
      then ['0'] ++ replPZ(tail str)
      else [(head str)] ++ replPZ(tail str)

splitOn :: Eq a => [a] -> [a] -> [[a]]
splitOn x y = func x y [[]]
    where
        func x [] z = reverse $ map (reverse) z
        func x (y:ys) (z:zs) = if (take (length x) (y:ys)) == x 
          then
              func x (drop (length x) (y:ys)) ([]:(z:zs))
          else
              func x ys ((y:z):zs)

change :: [String] -> Int -> Int -> [String]
change arr y x = do
  if( y < 0 || y >= (length arr) || x < 0 || x >= (length (arr!!0)) )
    then arr
    else if( (arr!!y)!!x == '*' )
      then arr
      else if( (arr!!y)!!x == '.' )
        then (take y arr) ++ [(take x (arr!!y)) ++ "1" ++ (drop (x+1) (arr!!y))] ++ (drop (y+1) arr)
        else if( (arr!!y)!!x == '1' )
          then (take y arr) ++ [(take x (arr!!y)) ++ "2" ++ (drop (x+1) (arr!!y))] ++ (drop (y+1) arr)
          else if( (arr!!y)!!x == '2' )
            then (take y arr) ++ [(take x (arr!!y)) ++ "3" ++ (drop (x+1) (arr!!y))] ++ (drop (y+1) arr)
            else if( (arr!!y)!!x == '3' )
              then (take y arr) ++ [(take x (arr!!y)) ++ "4" ++ (drop (x+1) (arr!!y))] ++ (drop (y+1) arr)
              else if( (arr!!y)!!x == '4' )
                then (take y arr) ++ [(take x (arr!!y)) ++ "5" ++ (drop (x+1) (arr!!y))] ++ (drop (y+1) arr)
                else if( (arr!!y)!!x == '5' )
                  then (take y arr) ++ [(take x (arr!!y)) ++ "6" ++ (drop (x+1) (arr!!y))] ++ (drop (y+1) arr)
                  else if( (arr!!y)!!x == '6' )
                    then (take y arr) ++ [(take x (arr!!y)) ++ "7" ++ (drop (x+1) (arr!!y))] ++ (drop (y+1) arr)
                    else if( (arr!!y)!!x == '7' )
                      then (take y arr) ++ [(take x (arr!!y)) ++ "8" ++ (drop (x+1) (arr!!y))] ++ (drop (y+1) arr)
                      else arr

change3x3 :: [String] -> Int -> Int -> [String]
change3x3 arr y x= do
  (change (change (change (change (change (change (change (change (change arr (y-1) (x-1)) (y-1) x) (y-1) (x+1) ) y (x-1) ) y x ) y (x+1) ) (y+1) (x-1) ) (y+1) x ) (y+1) (x+1))

minesweep :: [String] -> Int -> Int -> [String]
minesweep arr y x = do
  if (x + 1) >= length (arr!!0)
    then if (y + 1) >= length arr
      then if( (arr!!y)!!x == '*' )
        then (change3x3 arr y x) 
        else arr
      else if( (arr!!y)!!x == '*' )
        then minesweep (change3x3 arr y x) (y + 1) 0
        else minesweep arr (y + 1) 0
    else if( (arr!!y)!!x == '*' )
      then minesweep (change3x3 arr y x) y (x + 1)
      else minesweep arr y (x + 1)

checkRow :: [String] -> Int -> Int -> Bool
checkRow strL cur len = do
  if( strL == [] )
    then if( cur == 0 )
      then if( len == 0 )
        then True
        else False
      else True
    else if( len == (length (head strL)) )
      then (checkRow (tail strL) (cur + 1) len)
      else False

main = do
  args <- getArgs
  if (args == [] || (tail args) == [] || (tail (tail args)) == [] )
    then 
      error "Invaild Input"
    else if( checkInput (args!!2) 0 )
      then do
        let arr = (splitOn "\\" (args!!2))
        if( length arr == (map convStrInt args)!!0 && (checkRow arr 0 ((map convStrInt args)!!1)) )
          then putStrLn ("\""++(replPZ((intercalate "\\" (minesweep arr 0 0))))++"\"")
          else error "Invaild Input"
      else error "Invaild Input"