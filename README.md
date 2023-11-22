# Coin-Detection-System OpenCV4-Coins  
Coin recognition interface made with python Flask and OpenCV4 (C++)

## Building with CMake
```sh
mkdir build  
cd build
cmake ..
make
```
## Running Flask
```sh
# If you haven't already:
pip install -r requirements.txt
# Or just
pip install flask

cd SERVER  
python3 app.py  
```

### Note:
WorldCoins.zip is required but zip file is too big to put here.
Download WorldCoins.zip, extract and rename to "DATA_SETS" (without quotes)  
Then place DATA_SETS inside build directory.
(modify main.cpp code to match DATA_SETS absolute path's with yours)
