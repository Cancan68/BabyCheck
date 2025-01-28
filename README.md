# BabyCheck v2.4 (47 ko)

![Screenshot](./screenshots/babycheckhr.jpg)

Babies information administration.

Weight and Height follow up during the 3 first years
Visual check with the average values
List of the first words
List of the first actions
Vaccinations checklist
Birth information: Weight, Height, Date and Time
Zodiacal sign and period
Firstname symbolic (lucky number, lucky period)
Export measurements, first words and first actions to the Memo in a CSV form (delimiter ';') allowing you to import the data in any spreadsheet or database on your desktop after Hotsync

This program works in Kg or Lb, Cm or In

The graphics are displayed in 320x320 mode for the SONY CLIé and Palm OS5 devices (if high density is available)
The extended modes 320x480 and 480x320 are fully supported for both the screens and graphics.

The program is Freeware

Web Site: <https://www.campredon.net/>

## Support

Send a email to [laurent@campredon.net](mailto:laurent@campredon.net?subject=[GitHub]%20BabyCheck) to support the application. This will motivate me to improve it.

## Requirements

You need at least Palm OS 3.5 to run this application

The application works on color and black&white devices.

## Installation

babycheck.prc		English
babycheckBr.prc		Brazilian
babycheckDe.prc		German
babycheckEs.prc		Spanish
babycheckFr.prc		French
babycheckNl.prc		Dutch
babycheckRu.prc		Russian

Install one of those files on the PDA.

Users of Microsoft Windows and the Macintosh should use the Install Tool that
comes with the Palm Desktop software to install one of the above PRC files. 
Afterwards, initiate a HotSync to complete the installation.

UNIX users should use a program like "pilot-xfer" which is a part of
the pilot-link package to install one of the above PRC files. (Other
packages like KPilot, JPilot, and gnome-pilot will work just as well.)

## Update

If you are updating from a previous version of BabyCheck, simply install the new .prc

## Usage

- To change the defaults settings (Kg and Cm), select menu Info submenu Preferences in the menu panel
- Before selecting the buttons (or menu items) update, delete, measurements, first words, first actions, vaccinations and symbol you must first select a baby (means you must at least create one baby entry)
- When entering a measurement, you don't need to enter the weight AND the height. If you don't know one of the value leave it to 0 (the values equal to 0 - 0.0 or 0,0 - are not taken into consideration during the graph drawings)
- The graph section is defaulted according to the last measurement entry date. You can change the graph view by selecting the number of months to be displayed at the bottom right part of the screen

## Contribution

I want to thank:
  - All the guys who are working on the Cygwin and Prc-Tools projects
  - Aaron Ardiri for Pilrc, the source codes on his web site and the nice games
  - Ronny Iversen for his perl script which converts those *@%@*% Sony Trap enums
  - Patrice Bernard for the Metro application and for the support
  - All the Palm freeware developpers
  - Faustino Montaner for the Spanish translation
  - Gerd Kempf for the German translation
  - Susan Krieger for the Dutch translation
  - Alexey Kashkarov for the Russian translation
  - Ciro Rodrigues for the Brazilian translation
  - All the beta testers from www.palmattitude.org
  - All of you who send me feedbacks, ideas, etc...

## Tests

The program was checked:
- on my SONY TH55
- with POSE (Palm OS emulator) with 100 000 gremlins
- with the Palm OS 5 Simulator (Full debug) with 100 000 gremlins with High and Low Density