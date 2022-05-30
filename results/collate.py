# this file should be run in the same directory as the results CSV files
# merges the results into a single CSV and does some data manipulation


import csv
import os

STAGE = "stage2"

RESULT_FILES = [
    "random.csv", "spam_corner.csv", "ordered.csv", "rotating.csv",
    "merge-rnd_t.csv", "merge-mnmx.csv", "merge-expmx.csv",
    "score-rnd_t.csv", "score-mnmx.csv", "score-expmx.csv",
    "corner-rnd_t.csv", "corner-mnmx.csv", "corner-expmx.csv",
    "monte_carlo.csv"
]

this_directory = os.path.dirname(__file__)

headings = None  # will be overwritten once a CSV file is read
collated_data = []

GAMES = 1
TILE_COUNTS = range(4, 17 + 1)
TOTAL_SCORE = 18
TOTAL_MOVES = 20

for csv_filename in RESULT_FILES:
    filename = os.fsdecode(csv_filename)

    with open(filename) as csv_file:
        reader = csv.reader(csv_file)

        rows: list[list] = [r.copy() for r in reader]
        headings, data = rows[0], rows[1:]
        for row in data:
            games = int(row[1])
            for i in TILE_COUNTS:
                row[i] = round(int(row[i]) * 100 / games, 2)  # convert to percentage

            # convert from total to average
            row[TOTAL_SCORE] = float(row[TOTAL_SCORE]) / games
            row[TOTAL_MOVES] = float(row[TOTAL_MOVES]) / games
            collated_data.append(row)

headings[TOTAL_SCORE] = "Average Score"
headings[TOTAL_MOVES] = "Average Move Count"

with open(f"../results-{STAGE}.csv", 'w') as out_csv_file:
    writer = csv.writer(out_csv_file)
    writer.writerow(headings)
    writer.writerows(collated_data)
