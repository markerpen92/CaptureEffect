#!/bin/bash
cd ./ApplicationRecord
python3 CreateRecord.py
python3 Compiler.py
cd ..
cd ./CoCaptureEffect
python3 DeCompiler.py
./waf build
./waf --run "Capture.cc"
# 涉及資安問題，NS3路徑並未表明，所以這個shell無法正常執行