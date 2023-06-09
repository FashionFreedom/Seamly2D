import urllib.request, json 

#
# Download the release data from GitHub and create a markdown file and a csv file
#

counts = []

import requests

url = "https://api.github.com/repos/FashionFreedom/Seamly2D/releases"

# iterate over all pages of releases
res=requests.get(url)
data=res.json()
while 'next' in res.links.keys():
    res=requests.get(res.links['next']['url'])
    data.extend(res.json())

# iterate over all releases and extract the info we want
for thing in data:
    release_name = thing['name']
    for asset in thing['assets']:
        filename = asset['name']
        count = asset['download_count']
        counts.append((release_name, filename, count))

sorted_counts = sorted(counts)

# keep a dictionary of totals for each differenf file name
totals = {
    }

with open("DownloadCounts.md", "w") as md:
    md.write("# Release Download Counts\n")
    md.write("|Release | File | Downloads |\n")
    md.write("| :--- | :--- | :---: |\n")
    for release, filename, count in sorted_counts:
        if filename in totals.keys():
            totals[filename] += count
        else:
            totals[filename] = 1
        md.write(f"| {release} | {filename} | {count} |\n")

    md.write("## Totals\n")
    for key, value in totals.items():
        md.write(f"{key} : {value} \n")

with open("DownloadCounts.csv", "w") as md:
    md.write("Release, File, Downloads\n")
    for release, filename, count in sorted_counts:
        totals[filename] += count
        md.write(f"{release}, {filename}, {count}\n")

    md.write("Totals\n")
    for key, value in totals.items():
        md.write(f"{key}, {value}\n")
