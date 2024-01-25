# Installation instructions for ngspice
ngspice is a derivative of UC Berkeley's SPICE simulation tool.  It allows for the simulation of electronic circuits which are described by a 'netlist' file.

## Step 1 - Download ngspice
**Note:** Windows instructions only for now - but ngspice is available under different operating systems!

ngspice can be downloaded from https://ngspice.sourceforge.io/download.html.  For a Windows installation, download `ngspice-42_64.zip` and extract the folder to a relevant location on your hard drive.  For example, I've used

    C:\tools\ngspice

## Step 2 - Add to path
By following `Control Panel > Environment Variables` in the Windows system menu (or simply type 'environment variable' into the search bar), add the `bin` directory of your ngspice installation to the `PATH` variable (either User or System, depending on whether you want to install just for yourself or multiple users).

For example, with the installation directory used above, I added the following directory to my `PATH` variable:

    C:\tools\ngspice\bin

## Step 3 - Install gnuplot [optional]
gnuplot allows for quick observation of the output from ngspice simulations in the console. It can be downloaded from http://www.gnuplot.info/download.html.

I have installed version 5.4.8 (the lastest version as of writing) which comes packaged as `gp548-win64-mingw.exe`.

Run the exectuable file to install gnuplot.

Under the menu **Select Additional Tasks**, make sure that the option 'Add application to your PATH enivornment variable' is selected.