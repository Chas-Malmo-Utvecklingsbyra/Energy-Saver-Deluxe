# Energy-Saver-Deluxe

# Chas Malmö Utvecklingsbyrå

## Summary
* `Energy Saver Deluxe is a Local Energy Optimization Platform (LEOP), for forecasting and optimizing solar energy.`
* `This project builds on the skills we've learned in earlier courses and is designed to resemble and function as a real-world production energy system.`


## Project Goals:
* `To create a platform which collects weather data and spot price data, that use the collected data to calculate the optimal times for electricity consumption, charging batteries and selling surplus energy during the upcoming day with each time slot set at 15 minute intervals.`
* `A local runnable system without cloud dependencies to be used in a smart home, energy optimization or embedded contexts.`
* `By combining weather data and spot price data, the system can calculate optimal time slots for when the different actions are recommended.`
* `The project will be completed through an agile work set, in the form of the SCRUM-method`

## Dependencies
* Linux (Ubuntu)
* gcc
* curl
* premake
* make
* cJSON (included in include/core/json/)

## How to use
* 1) `Clone our repository - input the following into your terminal:` 
`git clone https://github.com/Chas-Malmo-Utvecklingsbyra/Energy-Saver-Deluxe.git`
* 2) `Install the prerequisites: 'premake5 install' in the terminal in the root folder`
* 3) `Starting the server: 'premake5 server'`
* 4) `Quit by typing 'q' or 'quit' in the terminal`
* 5) `Clean up with 'premake5 clean'`
*
* `Access the resulting data in one of three ways:`
* A) `Manually open and read the files created in the Energy_Advice_Reports and/or Energy_Advice_Report_Summary`
* B) `Starting the client: 'premake5 client' in a separate terminal after launching the server`
* C) `Alternatively, open http://localhost:8080 (default)`
*
* `Edit settings.json to select port and configure how fetching is done`

## Settings
* `Settings are done in settings.json, which is automatically created with default settings when the command 'premake5 install' is run`

* `http_server_port: select which port to host the localhost server on`
* `exec_fetcher_on_startup: runs the fetchers once before going into potential sleep modes`
* `fetcher_exec_path: defaults to /bin/http-request-service which is accurate for when installing with premake5, but can be altered if Http Request Service is installed elsewhere manually`
* `fetchers_command_count: number of fetchers to run, must match the number of commands entered`
* `fetcher_commands_args: list of arguments to be run`

* `Arguments`
* `--url        -u      Base url to fetch from`
* `--route      -r      API route for the base url`
* `--output     -o      Where to save the fetched file`
* `--name       -n      What to name the fetched file`
* `--intervals  -i      Time in seconds to sleep between fetching`
* `--quarter    -q      1 to activate. Fetch every quarter of an hour, at minutes 00, 15, 30, 45`
* `--timestamp  -ts     Time of day in 00:00 (hours:minutes) format for recurring daily fetches`
* `--read-fd    -fd     1 to activate. Enables pipe communication with parent process if run through a different program`
* `--write-fd   -fd     Same as above`

* `Examples`
* `"-q 1 -u 'https://api.open-meteo.com' -r '/v1/forecast?latitude=65.58&longitude=22.15&minutely_15=direct_radiation,diffuse_radiation,direct_normal_irradiance,temperature_2m,weather_code' -o /home/user/Chas-Malmo-Utvecklingsbyra/Energy-Saver-Deluxe/data/weather -n weather_SE1.json"`
* `Fetches data once every 15 minutes from Open-Meteo with various API arguments. Outputs the response in a file named weather_SE1.json in the specified folder`

* `"-ts 16:00 -u 'https://www.elprisetjustnu.se' -r '/api/v1/prices/' -o /home/user/Chas-Malmo-Utvecklingsbyra/Energy-Saver-Deluxe/data/price -n price"`
* `Fetches data from ElprisetJustNu once a day at 16:00 and outputs the result in a file named price`


### Data Collection: 
* `Spawns processes that fetch weather data from "OpenMeteo", this repeats every 15 minutes.`
* `Spawns processes that fetch spot price data from "Elprisetjustnu", done at program start, and repeated at the interval given by the user.`
* `Stores the collected data in JSON files by quarter of an hour segments`
* `When the collection of data is done, the program proceeds to the next part - Analyzing the data.`


### Analyzing Engine:
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
    * `All this information can also be viewed on http://localhost:8080 in the browser`
    * `Implementing caching in a local file system.`
* `This is only a suggestion by the program, based on the provided information, and it's up to the user whether or not they want to use this advice.`


## Deliverables:
* `A functional server application implemented in C.`
    * `Starts our program and processes`
* `A functional client application implemented in C++ with RAII.`
    * `A feature that gets the prognosis report in the terminal`
    * `A feature that gets the spot price-data for the coming day`
    * `A feature that gets the summarized information regarding the best time windows for the upcoming day` 
* `Documentation:`
    * `The systems architecture.`
    * `User instructions.`
    * `Instructions regarding further development.`

## Time Frame:
* `Project start: Week 3`
* `Iterative development according to the SCRUM model.`
* `End delivery including documentation: week 13 (2026-03-24 and 2026-03-25).`

## Primary Structure:
* `Core`                    # Our core-library
* `Energy Saver Deluxe`     # Includes everything energy related, our server and client.
* `HTTP Request Service`    # Includes functionality for fetching information

## Meet the 'CHAS Malmö Utvecklingsbyrå's members:
- Tech Lead: [Emilio Ganibegovic](https://github.com/AlCapone1234)
- SCRUM Master: [Pär Lundh](https://github.com/lundhpargmailcom)
- Developer: [Henrik Westerlund](https://github.com/Henrik-Westerlund)
- Developer: [Isa Shipshani](https://github.com/isashiphotmailcom)
- Developer: [Lukas Städe](https://github.com/HoffaQt)


## Donations
`Money for coffee is much appreciated. <3`
