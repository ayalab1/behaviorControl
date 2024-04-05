import tkinter as tk
from tkinter import filedialog, ttk
import threading
import pypylon.pylon as py
from pypylon import genicam
import os
from datetime import datetime
import cv2
from queue import Queue
import time
import numpy as np

"""
script to control multiple basler cameras. recordings can be continuous or event triggered. output = .jpg images
@Praveen Paudel, 2024

"""

# Global flag to control the camera loop
running = False
base_dir = ""
cam_array = None
start_time = None

def load_cameras():
    global cam_array
    if cam_array is not None and cam_array.GetSize() > 0 and cam_array[0].IsOpen():
        error_log.insert(tk.END, "Cameras are already loaded.\n")
        return
    tlf = py.TlFactory.GetInstance()
    devs = tlf.EnumerateDevices()

    if len(devs) == 0:  # Check if any cameras were detected
        error_log.insert(tk.END, "No cameras detected.\n")
        return

    NUM_CAMERAS = len(devs)  # Use the number of detected devices
    cam_array = py.InstantCameraArray(NUM_CAMERAS)

    for idx, cam in enumerate(cam_array):
        cam.Attach(tlf.CreateDevice(devs[idx]))
        camera_list.insert(tk.END, f"Camera {idx}: {cam.GetDeviceInfo().GetModelName()}")


    for idx, cam in enumerate(cam_array):
        cam.Open()
        cam.PixelFormat.SetValue('Mono8')
        cam.AcquisitionFrameRateEnable.SetValue(True)
        cam.AcquisitionFrameRate.SetValue(int(frame_rate.get()))  # Use the value from the dropdown menu
        cam.ExposureTime.SetValue(30000)
        cam.ExposureTime.SetValue(int(exposure_time.get()))  # Use the value from the entry field

        # PTP setup
        if ptp_enable.get():  # Use the value from the GUI option
            try:
                cam.PtpEnable.SetValue(False)
                cam.BslPtpPriority1.SetValue(128)
                cam.BslPtpProfile.SetValue("DelayRequestResponseDefaultProfile")
                cam.BslPtpNetworkMode.SetValue("Multicast")
                cam.BslPtpManagementEnable.SetValue(False)
                cam.BslPtpTwoStep.SetValue(False)
                cam.PtpEnable.SetValue(True)
                error_log.insert(tk.END, f"PTP setup successful for camera {idx}. Camera was loaded with PTP.\n")
            except genicam.LogicalErrorException:
                error_log.insert(tk.END, f"Failed to set up PTP for camera {idx}. This camera may not support PTP.\n")
                error_log.insert(tk.END, 'PTP initialization was not successful\n')
        else:
            error_log.insert(tk.END, f"Camera {idx} was loaded without PTP.\n")

def on_ptp_checkbox_click():
    if cam_array is not None and cam_array.GetSize() > 0 and cam_array[0].IsOpen():
        error_log.insert(tk.END, "Cannot change PTP settings while cameras are loaded. Please restart the GUI to change the settings.\n")
    else:
        if ptp_enable.get():
            error_log.insert(tk.END, "PTP enabled. It will be applied when cameras are loaded.\n")
        else:
            error_log.insert(tk.END, "PTP disabled. It will not be applied when cameras are loaded.\n")

def update_exposure_time(*args):
    global running
    if cam_array is None or running:
        error_log.insert(tk.END, "Cannot change exposure time while cameras are running\n")
        return
    try:
        new_exposure_time = int(exposure_time.get())
    except ValueError:
        return  # Ignore if the exposure time is not a valid integer
    if new_exposure_time < 19:
        return  # Ignore if the exposure time is less than the minimum allowed value
    for cam in cam_array:
        cam.ExposureTime.SetValue(new_exposure_time)
    error_log.insert(tk.END, f"Exposure time changed to {new_exposure_time}\n")

def update_frame_rate(*args):
    global running
    if cam_array is None or running:
        error_log.insert(tk.END, "Cannot change frame rate while cameras are running\n")
        return
    try:
        new_frame_rate = int(frame_rate.get())
    except ValueError:
        return  # Ignore if the frame rate is not a valid integer
    for cam in cam_array:
        cam.AcquisitionFrameRate.SetValue(new_frame_rate)
    error_log.insert(tk.END, f"Frame rate changed to {new_frame_rate}\n")

def update_duration():
    global start_time
    if start_time and running:
        duration = datetime.now() - start_time
        hours, remainder = divmod(duration.seconds, 3600)
        minutes, seconds = divmod(remainder, 60)
        duration_label.config(text=f"Recording duration: {hours:02}:{minutes:02}:{seconds:02}")
    root.after(1000, update_duration)  # Schedule this function to be called again after 1 second

def start_camera():
    global running, start_time, images_saved
    if not base_dir:
        error_log.insert(tk.END, "Please select a directory first.\n")
        error_log.see(tk.END)
        return
    start_time = datetime.now()
    images_saved = 0  # Reset the count when starting the camera
    running = True
    queue = Queue()  # Create a new queue
    threading.Thread(target=camera_loop, args=(queue,)).start()  # Pass the queue to the camera_loop function
    update_duration()  # Start updating the duration label

def stop_camera():
    global running
    running = False
    duration_label.config(text="Recording duration: 00:00:00")  # Reset the duration label when stopping the camera

def log_message(message):
    log.insert(tk.END, message + '\n')
    log.see(tk.END)

def select_directory():
    global base_dir
    base_dir = filedialog.askdirectory()
    dir_label.config(text=f"Saving images to: {base_dir}")


def camera_loop(queue):
    date_string = datetime.now().strftime("%Y%m%d_%H%M%S")
    session_dirs = [os.path.join(base_dir, f'cam_{i}_{date_string}') for i in range(cam_array.GetSize())]
    for idx, dir in enumerate(session_dirs):
        os.makedirs(dir, exist_ok=True)

    # Check if the camera is already grabbing before starting
    if cam_array.IsGrabbing():
        cam_array.StopGrabbing()
    cam_array.StartGrabbing()

    global running
    prev_frame = [None] * cam_array.GetSize()
    THRESHOLD_PERCENTAGE = 0.1  # Define a threshold for the sum of absolute differences as a percentage of the maximum possible sum
    COUNTER_THRESHOLD = 9000  # Define a threshold for the counter (5 minutes at 30 frames per second)
    counters = [0] * cam_array.GetSize()  # Create a separate counter for each camera
    save_frames = [True] * cam_array.GetSize()  # Create a separate save_frames flag for each camera

    while running:
        try:
            with cam_array.RetrieveResult(1000) as res:
                if res.GrabSucceeded():
                    img_nr = res.ImageNumber
                    cam_id = res.GetCameraContext()
                    log_message(f"cam #{cam_id}  image #{img_nr}")
                    
                    image = res.GetArray()

                    # If this is the first frame, save it and continue
                    if prev_frame[cam_id] is None:
                        prev_frame[cam_id] = image
                        continue

                    # If continuous recording is selected, save every frame
                    if recording_type.get() == "Continuous":
                        save_frames = [True] * cam_array.GetSize()  # Start saving frames for all cameras
                        save_frame(image, cam_id, session_dirs[cam_id])

                    # If event triggered recording is selected, only save the frame if it differs from the previous frame
                    elif recording_type.get() == "Event Triggered":
                        # Compute the absolute difference between the current frame and the previous frame
                        frame_delta = cv2.absdiff(prev_frame[cam_id], image)

                        # Apply a binary threshold to the frame delta
                        _, frame_delta = cv2.threshold(frame_delta, 25, 255, cv2.THRESH_BINARY)

                        # Compute the threshold based on the percentage of the maximum possible sum of absolute differences
                        max_sum = image.size * 255 if len(image.shape) == 2 else image.size * 255 * 3
                        threshold = THRESHOLD_PERCENTAGE * max_sum

                        # If the sum of the absolute differences is above a certain threshold, save the frame
                        if np.sum(frame_delta) > threshold:
                            counters[cam_id] = 0  # Reset the counter for this camera
                            if not save_frames[cam_id]:  # Only log the message when it starts saving again
                                error_log.insert(tk.END, f"cam #{cam_id} started saving frames at {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
                            save_frames[cam_id] = True  # Start saving frames again for this camera
                        else:
                            counters[cam_id] += 1  # Increment the counter for this camera
                            if counters[cam_id] >= COUNTER_THRESHOLD and save_frames[cam_id]:  # Only log the message when it stops saving
                                error_log.insert(tk.END, f"cam #{cam_id} stopped saving frames at {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
                                save_frames[cam_id] = False  # Stop saving frames for this camera

                    if save_frames[cam_id]:
                        save_frame(image, cam_id, session_dirs[cam_id])

                    # Update the previous frame
                    prev_frame[cam_id] = image
        except Exception as e:
            error_log.insert(tk.END, f"An error occurred: {e}")
            break


def save_frame(image, cam_id, dir):
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    milliseconds, microseconds = timestamp[-6:-3], timestamp[-3:]
    timestamp = timestamp[:-6] + milliseconds + '_' + microseconds
    filename = f'{timestamp}.jpg'
    filename = os.path.join(dir, filename)

    quality = 90  # Set the desired quality (0-100)
    cv2.imwrite(filename, image, [cv2.IMWRITE_JPEG_QUALITY, quality])


def update_recording_type(*args):
    global running
    if cam_array is None or running:
        error_log.insert(tk.END, "Cannot change recording type while cameras are running\n")
        return
    error_log.insert(tk.END, f"Recording type changed to {recording_type.get()}\n")

root = tk.Tk()
root.title("Basler Camera Control Suite")
root.configure(bg='lightgrey')


recording_type = tk.StringVar()
recording_type_label = tk.Label(root, text="Recording Type")
recording_type_label.grid(row=10, column=0, padx=10, pady=10)
recording_type_dropdown = ttk.Combobox(root, textvariable=recording_type)
recording_type_dropdown['values'] = ("Continuous", "Event Triggered")
recording_type_dropdown.current(0)  # Set the default recording type to the first one
recording_type_dropdown.grid(row=11, column=0, padx=10, pady=10)

recording_type.trace_add('write', update_recording_type)


ptp_enable = tk.BooleanVar()

ptp_checkbox = tk.Checkbutton(root, text="Enable PTP", variable=ptp_enable)
ptp_checkbox.grid(row=0, column=2, padx=10, pady=10) 

ptp_enable.trace_add("write", lambda *args: on_ptp_checkbox_click())

frame_rate = tk.StringVar()
frame_rate_label = tk.Label(root, text="Frame Rate")
frame_rate_label.grid(row=0, column=0, padx=10, pady=10)
frame_rate_dropdown = ttk.Combobox(root, textvariable=frame_rate)
frame_rate_dropdown['values'] = (20, 30, 40, 60, 50, 60) 
frame_rate_dropdown.current(1)  # Set the default frame rate to the second one
frame_rate_dropdown.grid(row=1, column=0, padx=10, pady=10)

frame_rate.trace_add('write', update_frame_rate)

exposure_time = tk.StringVar(value='30000')  # Default exposure time
exposure_time_label = tk.Label(root, text="Exposure Time")
exposure_time_label.grid(row=8, column=0, padx=10, pady=10)
exposure_time_entry = tk.Entry(root, textvariable=exposure_time)
exposure_time_entry.grid(row=9, column=0, padx=10, pady=10)
exposure_time.trace_add('write', update_exposure_time)

duration_label = tk.Label(root, text="Recording duration: 00:00:00")
duration_label.grid(row=2, column=0, padx=10, pady=10)

load_button = tk.Button(root, text="Load Cameras", command=load_cameras)
load_button.grid(row=3, column=0, padx=10, pady=10)

start_button = tk.Button(root, text="Start", command=start_camera, bg='green', fg='white')
start_button.grid(row=4, column=0, padx=10, pady=10)

stop_button = tk.Button(root, text="Stop", command=stop_camera, bg='red', fg='white')
stop_button.grid(row=5, column=0, padx=10, pady=10)

dir_button = tk.Button(root, text="Select Save Directory", command=select_directory)
dir_button.grid(row=6, column=0, padx=10, pady=10)

dir_label = tk.Label(root, text=f"Saving images to: {base_dir}")
dir_label.grid(row=7, column=0, padx=10, pady=10)

camera_list_label = tk.Label(root, text="Camera List")
camera_list_label.grid(row=0, column=1, padx=10, pady=10)
camera_list = tk.Listbox(root)
camera_list.grid(row=1, column=1, rowspan=4, padx=10, pady=10, sticky='nsew')

log_label = tk.Label(root, text="Recording Log")
log_label.grid(row=5, column=1, padx=10, pady=10)
log_frame = tk.Frame(root)
log_frame.grid(row=6, column=1, rowspan=2, padx=10, pady=10, sticky='nsew')
log = tk.Text(log_frame, width=50, height=10)
log.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
log_scroll = ttk.Scrollbar(log_frame, orient="vertical", command=log.yview)
log_scroll.pack(side=tk.RIGHT, fill=tk.Y)

log['yscrollcommand'] = log_scroll.set

error_log_label = tk.Label(root, text="Messages Log")
error_log_label.grid(row=8, column=1, padx=10, pady=10)
error_log_frame = tk.Frame(root)
error_log_frame.grid(row=9, column=1, padx=10, pady=10, sticky='nsew')
error_log = tk.Text(error_log_frame, width=50, height=10, fg='red')
error_log.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
error_log_scroll = ttk.Scrollbar(error_log_frame, orient="vertical", command=error_log.yview)
error_log_scroll.pack(side=tk.RIGHT, fill=tk.Y)
error_log['yscrollcommand'] = error_log_scroll.set

root.mainloop()