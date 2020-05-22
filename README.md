# EmonSimulator
This will simulate PFP PMon device for collecting trace data thru our cloud base platform. The source code provided is a template that produce fake data. You will need to expand and read from your device adc.

Your data can be viewed using P2Scan or P3Scan.


A. P2Scan(Standalone version)
   If you don't have P2scan please contact us at vtith@pfpcyber.com.

B. P3Scan(Cloud version)
Here are the steps needed to use the example program with PFP cloud platform.

1. Sign up for an account on our cloud platform if you don't have one. We will create an account and email the material needed to start running.

https://forms.gle/mKs6eTaYfRgFsNtY9


2. Compile the source code.( This assumed you are running linux )

gcc emonSimulator.c -o emonSim

3. Start the program from the terminal after compile. The program will open a socket on port 7001.

./emonSim

4. Now get the ip address of your computer. You will need this ip address for our cloud platform.

5. Run PFP Gatewway software (This information is provide when you signup). Note PFP Gateway software has to be on the same subnet as your emonSim.

6. Log into PFP cloud platform and add the device.


