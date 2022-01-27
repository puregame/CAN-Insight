# create log files python script
from pathlib import Path
location = Path("D:/")
file_name = "CAN_{:03d}.LOG"
for i in range(1, 1000):
    this_file = file_name.format(i)
    print(this_file)
    f = open(location/this_file, "a")
    f.write("Now the file has more content!")
    f.close()
