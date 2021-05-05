#!/usr/bin/python3

import json
import subprocess

text = ["3RD-PARTY LICENSES", "=================="]

license_file = open("LICENSE-3RD-PARTY.txt", "w")
for line in text:
    license_file.write(line)
    license_file.write("\n")
license_file.write("\n")

license_file.write("The following licenses are used by dependencies of cloe-ui:\n")
result = subprocess.run(
    ["npx", "license-checker", "--summary", "--excludePrivatePackages"],
    stdout=subprocess.PIPE,
)
output = result.stdout.decode("utf-8")
license_file.write(output)

license_file.write("The following libraries are used by cloe-ui:\n")
license_output = subprocess.Popen(
    ["npx", "license-checker", "--relativeLicensePath", "--excludePrivatePackages"],
    stdout=subprocess.PIPE,
)
result = subprocess.run(
    ["egrep", "-v", "path:|email:"], stdin=license_output.stdout, stdout=subprocess.PIPE
)
output = result.stdout.decode("utf-8")

license_file.write(output)
