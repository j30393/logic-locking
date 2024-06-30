# Overall introduction
There are three logic locking implementations: Random logic locking, Fault analysis-based logic locking, and Strong logic locking. You can refer to these implementations in their respective branches. Additionally, a draft of the flowchart is available in the Report.pdf.

## How to use this function
1. Run `make` to generate the `logic_lock` executable.
2. Run "./logic_lock  ./benchmark/c17.bench  test.bench" to generate the encrypted circuit 
3. Run "./sld test.bench ./benchmark/c17.bench" to pose the sat attack 
4. Run "./lcmp  ./benchmark/c17.bench  test.bench  key=???" to lunch the equivariance checking
![balabala](./source/image.png)

## template from 
This template is from https://github.com/Ohpaipai/Homework/tree/main