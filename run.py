from os import access
import subprocess
import fileinput


assocs = [1,2,4,8]
block_sizes = [16,32,64]
cache_sizes = [12,32,64,128]
miss_pens = {16:30,32:32,64:36}
traces = ["art","crafty_mem","mcf","swim"]
pwd = r"/home/michael/Documents/School/Fall2021/CSEE4290/projects/cache-simulator-MichaelStarks"

for trace in traces:
    for block_size in block_sizes:
        for cache_size in cache_sizes:
            for assoc in assocs:
                cmd = subprocess.list2cmdline(["gunzip", "-c", r"traces/"+trace+".trace.gz",'|',"./cache.out", "-a", str(assoc), "-l", str(block_size), "-s", str(cache_size), "-mp", str(miss_pens[block_size]),"|","python3", "trials.py", trace])
                test = subprocess.Popen(cmd, cwd=pwd,shell=True)
