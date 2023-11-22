from subprocess import run, PIPE
import os

class CallCommand:
	def __init__(self, filename):
		# PLEASE FIX PATHS
		self.COMMAND = '~/Desktop/OpenCV/OPENCVTEST/build/opencvtest ' + filename
		print(filename)

	def check_out(self) -> str:
		subp = run(self.COMMAND, stdout = PIPE, stderr = PIPE, shell=True, text=True)

		if subp.returncode == 0:
			coin_str = subp.stdout.split('/')[-1]
			coin_str = coin_str.split("__")[-1].replace(".jpg", "").replace("_", " ")
			return coin_str
		else:
			return "Not Found"