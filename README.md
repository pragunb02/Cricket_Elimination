# SixthSense

## Overview
SixthSense is a command-line tool that uses graph theory and network flow algorithms to determine which teams cannot secure first place in a sports tournament. By analyzing current standings and remaining match schedules, it identifies teams mathematically eliminated from the top position.

## Key Features
- Identifies teams that cannot achieve first place in the tournament
- Utilizes graph theory for tournament modeling
- Implements Edmonds-Karp algorithm within the Ford-Fulkerson framework
- Constructs and analyzes flow networks for each team

## Technical Details
- **Language**: C++
- **Algorithms**: Edmonds-Karp, Ford-Fulkerson (Maximum Flow)
- **Data Structure**: Flow Network
- **Interface**: Command Line Interface (CLI)

## How It Works
1. **Data Input**: Reads current points and remaining match schedules for each team.
2. **Graph Construction**: Creates a graph representation of the tournament.
3. **Flow Network Analysis**: Applies Edmonds-Karp algorithm to calculate maximum flow.
4. **First Place Elimination**: Determines which teams cannot mathematically achieve first place.

## Usage
1. Compile the program:
