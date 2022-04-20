# this file should be run in the same directory as the results CSV files
# merges the results into a single CSV and does some data manipulation


import csv
import os

RESULT_FILES = [
    "random.csv", "corner.csv", "ordered.csv",
    "merge.csv", "score.csv",
    "minimax.csv",
    "monte_carlo.csv"
]

this_directory = os.path.dirname(__file__)

headings = None  # will be overwritten once a CSV file is read
collated_data = []

for csv_filename in RESULT_FILES:
    filename = os.fsdecode(csv_filename)

    with open(filename) as csv_file:
        reader = csv.reader(csv_file)

        rows = [r.copy() for r in reader]
        headings, data = rows[0], rows[1:]
        for row in data:
            games = int(row[1])
            for i in range(4, len(row)):
                row[i] = round(int(row[i]) * 100 / games, 2)  # convert to percentage
            collated_data.append(row)

with open("../results.csv", 'w') as out_csv_file:
    writer = csv.writer(out_csv_file)
    writer.writerow(headings)
    writer.writerows(collated_data)
