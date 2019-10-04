import time
import subprocess

with open("data.txt", "w") as file:
    for n in range(1, 17):
        for __ in range(5):
            startTime = time.time_ns()
            subprocess.run(["./a.out", str(n)])
            endTime = time.time_ns()
            file.write(f"{n} {endTime - startTime}\n")
            time.sleep(5)
