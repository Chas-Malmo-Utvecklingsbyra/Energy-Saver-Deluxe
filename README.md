# Energy-Saver-Deluxe

# Chas Malmö Utvecklingsbyrå

## What this project is all about
* `This is a project based upon developing a Local Energy Optimization Platform (LEOP) for forecasting and optimizing solar energy. This project builds upon skills we've learned in earlier courses and will develop to feel and function as a real life production energy system.`


## Project Goals:
* `To create a platform which collects weather data and spotprice data, to then use that data to calculate the optimal times for electricity consumption, charging batteries and selling surplus energy during the upcoming day with each time slot set at 15 minute intervals.`
* `A local runable system without cloud dependencies to be used in a smart home, energy optimization or embedded contexts.`
* `By combining weather data and spotprice data, the system can calculate optimal time slots for when the different actions are recommended.`
* `The project will be completed through an agile work set, in the form of the SCRUM-method`

## How to use
* `Clone our repo to your computer: write the line under this in to your terminal:` 
`git clone https://github.com/Chas-Malmo-Utvecklingsbyra/Energy-Saver-Deluxe.git`
* `Run 'premake5 install`
* `Run 'premake5 server'`
* `Quit by typing 'q' or 'quit' in the terminal`
* `Clean up with 'premake5 clean'`
*
* `Access the resulting data in one of three ways:`
* `Manually open and read the files created in the Energy_Advice_Reports and/or Energy_Advice_Report_Summary`
* `Run 'premake5 client' in a separate terminal after launching the server`
* `Alternatively, open localhost:8080 (default)`
*
* `Edit settings.json to select port and configure how fetching is done`


### The collecting part: 
* `The program spawns processes that fetches weather data from "OpenMeteo" and spot price data from "Elprisetjustnu" and stores that info into json-files.`
* `The processes collects the weather data every 15 minutes, and spotprices get collected at program start, and then again at the time specified by the user (right now set as an argument in http-request-service and recommended to be put at 16:00 (local swedish time))`
* `After the program has collected the required data regarding weather and spot prices the program starts to proceed to building the analysis`


### The analysis part:
* `Will analyze the spot prices and weather data by each quarter of the given day, to calculate and grade each quarter.`
* `These grades will then be judged in the analysis and built into a "best window" in a summary which states the best times to perform the three given actions during the coming day (Charge, Consume and Sell)`
* `Functionality:`
    * `Uses the collected data on weather and spot price to start calculating and grading each quarters information.`
    * `Every point in the report will be graded between 0-1, where 0 is strongly discouraged and 1 is strongly recommended`
    * `It summarizes all the collected data into seven distinct categories:`
        * `Charging is set to two types: Charge from grid and Charge from source`
        * `Consuming has three types: Consume from grid, from source or from battery`
        * `Selling is set to two different types: Sell from battery and from source`
    * `These grades will then be checked and summarized into a (by the user) set window-span and by also checking if they meet the required threshold (also set by the user). Then the summary displays the days best window for the three actions`
    * `Should there not be a suitable window during the day, the program will signal this by displaying a message explaining this.`
    * `Storing this information into a text-file and a json-file and shared to our endpoints, enabling the client to be able to     present the data in the terminal with a clear structure.`
    * `All this information can also be viewed on our localhost:8080 in the browser`
    * `Implementing caching in a local file system.`
* `This is only a suggestion by the program, based on the provided information, and it's up to the user whether or not they want to use this advice.`


## Deliverables:
* `A functional server application implemented in C.`
    * `Starts our program and processes`
* `A functional client application implemented in C++ with RAII.`
    * `A feature that gets the prognosis report in the terminal`
    * `A feature that gets the spot price-data for the coming day`
    * `A feature that gets the summarized information regarding the best time windows for the upcoming day` 
* `Documentaion:`
    * `The systems architecture.`
    * `User instructions.`
    * `Instructions regarding further development.`

## Time Frame:
* `Project start: Week 3`
* `Iterative development according to the SCRUM model.`
* `End delivery including documentation: week 13 (2026-03-24 and 2026-03-25).`

## Primary Structure:
* `Core`                    # Our core-library
* `Energy Saver Deluxe`     # Includes everything energy related
* `HTTP Request Service`    # Includes functionality for fetching information

## Meet the 'CHAS Malmö Utvecklingsbyrå's members:
- Tech Lead: [Emilio Ganibegovic](https://github.com/AlCapone1234)
- SCRUM Master: [Pär Lundh](https://github.com/lundhpargmailcom)
- Developer: [Henrik Westerlund](https://github.com/Henrik-Westerlund)
- Developer: [Isa Shipshani](https://github.com/isashiphotmailcom)
- Developer: [Lukas Städe](https://github.com/HoffaQt)


## Donations
`Money for coffee is much appreciated. <3`
