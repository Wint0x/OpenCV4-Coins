from subprocess import run, PIPE
import os

class CallCommand:
	def __init__(self, filename, mode=1):
		# COMMAND BUILDER
		# Mode: 1 = Houghcircles
		# Mode: 2 = Canny Edge Detection
		self.COMMAND = '~/Desktop/OpenCV/OPENCVTEST/build/opencvtest ' + mode + " " + filename
		print(filename)

	def check_out(self):
		subp = run(self.COMMAND, stdout = PIPE, stderr = PIPE, shell=True, text=True)

		if subp.returncode == 0:

			return_str = subp.stdout

			if len(return_str) < 3:
				coin_str = "Something went wrong / Could not detect any coins, consider switching modes!"
				return (None, coin_str)
				
			if not "Around" in return_str:
				coin_str = return_str.split('/')[-1]
				coin_str = coin_str.split("__")[-1].replace(".jpg", "").replace("_", " ")

				return (None, coin_str)
			
			else:
				# Check if we got a new file path
				new_file = ""
				try:
					new_file = return_str.split("|")[0].strip()
					new_file = new_file.split("/")[-1]

					return (new_file, return_str.split("|")[1].strip())

				except Exception as ex:
					return (None, "Not Found")

		else:
			print("TEST!")
			return (None, "Not Found")
