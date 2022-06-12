# this file should be run in the same directory as the results CSV files
# merges the results into a single CSV and does some data manipulation


import csv
import itertools
import os

STAGE = "stage3"
HEURISTICS = ["merge", "score", "corner", "full_wall", "wall_gap"]
STRATEGIES = ["rnd_t", "mnmx", "expmx"]
RESULT_FILES = ["random.csv", "spam_corner.csv", "ordered.csv", "rotating.csv"] + \
               [f"{x[0]}-{x[1]}.csv" for x in itertools.product(HEURISTICS, STRATEGIES)] + \
               ["monte_carlo.csv"]


this_directory = os.path.dirname(__file__)

headings = None  # will be overwritten once a CSV file is read
collated_data = []

GAMES = 1
TILE_COUNTS = range(4, 17 + 1)
TOTAL_SCORE = 18
TOTAL_MOVES = 20

DECIMALS = 4

for csv_filename in RESULT_FILES:
    filename = os.fsdecode(csv_filename)

    with open(filename) as csv_file:
        reader = csv.reader(csv_file)

        rows: list[list] = [r.copy() for r in reader]
        headings, data = rows[0], rows[1:]
        for row in data:
            games = int(row[1])
            for i in TILE_COUNTS:
                row[i] = round(int(row[i]) * 100 / games, DECIMALS)  # convert to percentage

            # convert from total to average
            row[TOTAL_SCORE] = round(float(row[TOTAL_SCORE]) / games, DECIMALS)
            row[TOTAL_MOVES] = round(float(row[TOTAL_MOVES]) / games, DECIMALS)
            collated_data.append(row)

headings[TOTAL_SCORE] = "Average Score"
headings[TOTAL_MOVES] = "Average Move Count"

with open(f"../results-{STAGE}.csv", 'w') as out_csv_file:
    writer = csv.writer(out_csv_file)
    writer.writerow(headings)
    writer.writerows(collated_data)
