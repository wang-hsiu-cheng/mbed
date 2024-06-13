# embed final project
>by 111061237 wang hsiu cheng

## how to setup 
### setup mbed
1. Create a new program.
2. Open the File menu and select New Program....
3. Select "empty Mbed OS program" under MBED OS 6. Enter \<your project name> for Program name. Check "Make this the active program" (default). Under "Mbed OS Location", check "Link to an existing shared Mbed OS instance". Click "Add Program".
4. Copy .mbedignore.
5. Import PWMIn library.
    1. Click + in the Library tab under program source.
    2. Fill in "https://gitlab.larc-nthu.net/ee2405_2022/pwmin.git" And click "Next"
    3. Select "main" branch and click "Finish"
6. Replace main.cpp with main.cpp in the folder I provide.
7. Copy all the other files in the folder into the project
### setup python
3. install library
    1. Open terminal
    2. Type `pip install bleak`
    3. Type `pip install numpy`
    4. Type `pip install openpyxl`
    5. Type `pip install matplotlib`

## how to use
### use embed
1. Compile and run the program
2. Place the car at the left botton corner of the map
3. Unplug the **5V line** of qti and replug to reset it
    1. The car will run along the black strip
    2. If no, Unplug & replug again
4. the car will start doing the mission as the description in the PDF
### use python
1. Open terminal and move to the project's folder
2. Type `python.exe remoteControl.py` to run the python code.
    1. It might show error. If it happen, **retry until it connect to the mbed successfully**
    2. If it still don't work for many times. Type `python.exe remoteControl1.py` to **run another simpler version** 
3. If python program runs. the terminal will show path length and the qti pattern. And a plot will display the path graph of car