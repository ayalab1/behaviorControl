from pypylon import pylon
import time
from datetime import datetime
import csv
import tkinter as tk
from tkinter import filedialog

# Function to prompt the user for a file location
def get_save_file_location():
    root = tk.Tk()
    root.withdraw()  # Hide the main window
    file_path = filedialog.asksaveasfilename(defaultextension='.csv', filetypes=[("CSV files", "*.csv")])
    root.destroy()
    return file_path

# Get the file location from the user
csv_file_path = get_save_file_location()
if not csv_file_path:
    raise ValueError("No file selected")

# Initialize Pypylon and the action command
tl_factory = pylon.TlFactory.GetInstance()
gige_tl = tl_factory.CreateTl('BaslerGigE')

act_cmd = gige_tl.ActionCommand(0x01, 0x01, 4294967295)

# Open the CSV file and start logging timestamps
with open(csv_file_path, 'w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(['Command Count', 'Timestamp'])  # Write header row

    command_count = 0
    while True:
        current_timestamp = datetime.now()
        ok = act_cmd.IssueNoWait()

        # Log the current timestamp to the CSV
        writer.writerow([command_count + 1, current_timestamp.strftime("%Y-%m-%d %H:%M:%S.%f")])
        command_count += 1

        time.sleep(1.0/40)  # Adjust sleep time as needed
