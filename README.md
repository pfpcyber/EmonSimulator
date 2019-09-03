# EmonSimulator
This will simulate PFP PMon device for collection trace data thru our cload base platform. The source code provided is a template that you will need to expand from. You need to provide the business logic to collect the adc. The current progam just put incremental or random number.

Here's the steps need to use PFP cloud.

1. Sign up for the account on our cloud platform if you don't have one. We will create and account an email the material need to start running.

https://docs.google.com/forms/d/e/1FAIpQLSeGI2N-ELRc8O4-raBWqloPCQTnicRMXv9GBJgfbnkfncozEw/viewform


2. Compile the source code. This assumed you are running linux
gcc emonSimulator.c -o emonSim

3. Start the program from the terminal after compile. The program will open a socket on port 7001.
./emonSim

4. Run PFP Gatewway software (This information is provide when you signup)

5.


