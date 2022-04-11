# this file should be run in the same directory as the results CSV files
# merges the results into a single CSV and does some data manipulation


import csv
import os

this_file = os.path.basename(__file__)
this_directory = os.path.dirname(__file__)

headings = None  # will be overwritten once a CSV file is read
collated_data = []

for csv_filename in sorted(os.listdir(this_directory)):  # just sort by strategy name for now
    filename = os.fsdecode(csv_filename)
    if filename == this_file:  # ignore this script
        continue

    with open(filename) as csv_file:
        reader = csv.reader(csv_file)
        headings, data = reader

        games = int(data[1])
        for i in range(4, len(data)):
            data[i] = round(int(data[i]) * 100 / games, 2)  # convert to percentage
        collated_data.append(data)

# put random, corner, ordered strategies first
collated_data.sort(reverse=True, key=lambda row: row[0] in ["random", "corner", "ordered"])

with open("../results.csv", 'w') as out_csv_file:
    writer = csv.writer(out_csv_file)
    writer.writerow(headings)
    writer.writerows(collated_data)
