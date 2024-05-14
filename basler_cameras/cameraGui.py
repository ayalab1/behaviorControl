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
import traceback
import csv


# Global flag to control the camera loop
running = False
base_dir = ""
cam_array = None
start_time = None
NUM_CAMERAS = 0
global framerate
framerate = 30.0
global exposure
exposure = 10
global action_command_running
action_command_running = False


def init_ptp(cam, idx):
    # PTP setup
    cam.PtpEnable.SetValue(False)
    if idx == 0:
        cam.BslPtpPriority1.SetValue(0)
    else:
        cam.BslPtpPriority1.SetValue(128)
    cam.BslPtpProfile.SetValue("DelayRequestResponseDefaultProfile")
    cam.BslPtpNetworkMode.SetValue("Multicast")
    cam.BslPtpManagementEnable.SetValue(False)
    cam.BslPtpTwoStep.SetValue(False)
    cam.PtpEnable.SetValue(True)

def check_ptp(cam_array):
    initialized = False
    locked = False

    # Wait until correctly initialized or timeout
    time1 = time.time()
    while not initialized:
        status_arr = np.zeros(cam_array.GetSize(), dtype=np.bool_)
        for i, cam in enumerate(cam_array):
            cam.PtpDataSetLatch.Execute()
            status_arr[i] = (cam.PtpStatus.GetValue() == 'Master' \
                            or cam.PtpStatus.GetValue() != 'Initializing')
        initialized = np.all(status_arr)
        if (time.time() - time1) > 3:
            if not initialized:
                error_log.insert(tk.END, "PTP not initialized -> Timeout.\n")
            break

    # If correctly initialized, wait until settled or timeout
    if initialized:
        time2 = time.time()
        while not locked:

            master_count = 0
            slave_count = 0
            for i, cam in enumerate(cam_array):
                cam.PtpDataSetLatch.Execute()
                if cam.PtpStatus.GetValue() == 'Master':
                    master_count += 1
                elif cam.PtpStatus.GetValue() == 'Slave':
                    slave_count += 1
            error_log.insert(tk.END, f"Master count: {master_count}, Slave count: {slave_count}.\n")

            locked = master_count == 1 and slave_count == NUM_CAMERAS - 1

            if (time.time() - time2) > 30:
                if not locked:
                    error_log.insert(tk.END, "PTP not initialized -> Timeout.\n")
                break

    return initialized and locked

def init_camera(cam, index):

    global framerate, exposure
    gain = 1.
    compression_ratio = 70.
    pixel_scaling = 2.79

    cam.GainAuto.SetValue("Off")
    cam.Gain.SetValue(gain)

    cam.ExposureAuto.SetValue("Off")
    cam.ExposureTime.SetValue(int(exposure*1000)) # into microsecond

    # Beyond Pixel setup
    cam.PixelFormat.SetValue("Mono8")
    # cam.BslScalingFactor.SetValue(pixel_scaling)

    # Beyond Compression setup
    # cam.ImageCompressionMode.SetValue("BaslerCompressionBeyond")
    # cam.ImageCompressionRateOption.SetValue("FixRatio")
    # cam.BslImageCompressionRatio.SetValue(compression_ratio)

    # Enable timestamp feature
    cam.StaticChunkNodeMapPoolSize.Value = cam.MaxNumBuffer.GetValue()
    cam.ChunkModeActive.SetValue(True)
    cam.ChunkSelector.Value = "Timestamp"
    cam.ChunkEnable.Value = True
    cam.BslChunkTimestampSelector.SetValue("FrameStart")

    # Select Line 2 (output line)
    cam.LineSelector.Value = "Line2"
    cam.LineMode.Value = "Output"
    cam.LineSource.Value = "ExposureActive"

    # Periodic Signal setup
    if cam.BslPeriodicSignalSource.GetValue() != 'PtpClock':
        error_log.insert(tk.END, "Clock source of periodic signal is not `PtpClock`\n")
    cam.BslPeriodicSignalPeriod.SetValue(1 / framerate * 1e6)
    cam.BslPeriodicSignalDelay.SetValue(0)
    # cam.TriggerSelector.SetValue("FrameStart")
    # cam.TriggerMode.SetValue("On")
    # cam.TriggerSource.SetValue("PeriodicSignal1")

    # Action Command Trigger Setup
    cam.ActionDeviceKey.Value = 0x01
    cam.ActionGroupKey.Value = 0x01
    cam.ActionGroupMask.Value = 4294967295
    cam.TriggerSource.Value = 'Action1'
    cam.TriggerMode.Value = 'On'
    cam.AcquisitionMode.Value = 'Continuous'

    # Synchronous free run setup

    # # Make sure that the Frame Start trigger is set to Off to enable free run
    # cam.AcquisitionMode.SetValue('Continuous')
    # cam.TriggerSelector.SetValue('FrameStart')
    # cam.TriggerMode.SetValue('Off')
    # # Let the free run start immediately without a specific start time
    # cam.SyncFreeRunTimerStartTimeLow.SetValue(0)
    # cam.SyncFreeRunTimerStartTimeHigh.SetValue(0)
    # # Specify a trigger rate of 30 frames per second
    # cam.SyncFreeRunTimerTriggerRateAbs.SetValue(framerate)
    # # Apply the changes
    # cam.SyncFreeRunTimerUpdate.Execute()
    # # Enable Synchronous Free Run
    # cam.SyncFreeRunTimerEnable.SetValue(True)

    # # Transport Layer Control
    # cam.GevSCPD.SetValue(222768)
    # cam.GevSCFTD.SetValue(8018*index)
    # cam.GevSCPSPacketSize.SetValue(8000)

def load_cameras():
    global cam_array, NUM_CAMERAS
    try:
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
            cam.Open()

        for idx, cam in enumerate(cam_array):
            init_ptp(cam, idx)  # Use the value from the entry field

        success = check_ptp(cam_array)
        if not success:
            error_log.insert(tk.END, "PTP initialization was not successful\n")
        else:
            error_log.insert(tk.END, "PTP initialization is successful\n")

        # Initialize general camera parameters
        for idx, cam in enumerate(cam_array):
            init_camera(cam, idx)
    except Exception as e:
        error_log.insert(tk.END, f"Error loading cameras: {e}\n")
        traceback.print_exc()

def start_camera():
    global running, start_time, start_time_str, cam_array, NUM_CAMERAS, framerate, action_command_running
    if not base_dir:
        error_log.insert(tk.END, "Please select a directory first.\n")
        error_log.see(tk.END)
        return

    start_time = datetime.now()
    start_time_str = start_time.strftime("%Y-%m-%d_%H-%M-%S")
    running = True
    action_command_running = True

    # Create output directories for each camera
    session_dirs = [os.path.join(base_dir, f'cam_{i}_{start_time_str}') for i in range(cam_array.GetSize())]
    for idx, dir in enumerate(session_dirs):
        os.makedirs(dir, exist_ok=True)

    # Register Image Handlers for each camera
    for idx, cam in enumerate(cam_array):
        handler = ImageHandler(idx, session_dirs[idx])
        cam.RegisterImageEventHandler(handler, py.RegistrationMode_ReplaceAll, py.Cleanup_Delete)

    # Start grabbing
    if cam_array.IsGrabbing():
        cam_array.StopGrabbing()
    cam_array.StartGrabbing(py.GrabStrategy_LatestImageOnly, py.GrabLoop_ProvidedByInstantCamera)

    # Start action commands in a separate thread
    threading.Thread(target=send_action_commands, args=(base_dir, start_time_str, framerate)).start()

    # Start updating the duration label
    update_duration()

def stop_camera():
    global running, action_command_running
    try:
        running = False
        action_command_running = False
        cam_array.StopGrabbing()
        cam_array.Close()
        duration_label.config(text="Recording duration: 00:00:00")  # Reset the duration label when stopping the camera
        error_log.insert(tk.END, "Camera acquisition stopped.\n")
    except Exception as e:
        error_log.insert(tk.END, f"Error stopping camera: {e}\n")
        traceback.print_exc()


# Define your image handler class
class ImageHandler(py.ImageEventHandler):
    def __init__(self, camera_index, output_directory):
        super().__init__()
        self.camera_index = camera_index
        self.output_directory = output_directory
        self.num_frames = 0

    def OnImageGrabbed(self, cam, grab_result):
        try:
            if grab_result.GrabSucceeded():
                current_time_seconds = time.time()
                image = grab_result.GetArray()
                self.num_frames += 1

                # use current computer time
                time_struct = time.localtime(current_time_seconds)
                current_time = time.strftime("%Y%m%d_%H%M%S",
                                             time_struct) + f"_{int((current_time_seconds - int(current_time_seconds)) * 1000):03d}"
                filename = f"camera{self.camera_index}_frame{self.num_frames}_{current_time}.jpg"

                # Save the image in the specified output directory
                cv2.imwrite(os.path.join(self.output_directory, filename), image)

            grab_result.Release()

        except Exception as e:
            traceback.print_exc()
            error_log.insert(tk.END, f"Error saving image: {e}\n")


def update_exposure_time(*args):
    global running, exposure
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
    exposure = new_exposure_time
    error_log.insert(tk.END, f"Exposure time changed to {new_exposure_time}\n")


def update_frame_rate(*args):
    global running, framerate
    if cam_array is None or running:
        error_log.insert(tk.END, "Cannot change frame rate while cameras are running\n")
        return
    try:
        framerate = float(frame_rate.get())
    except ValueError:
        return  # Ignore if the frame rate is not a valid integer
    for cam in cam_array:
        cam.AcquisitionFrameRate.SetValue(framerate)
    error_log.insert(tk.END, f"Frame rate changed to {framerate}\n")


def update_duration():
    global start_time
    if start_time and running:
        duration = datetime.now() - start_time
        hours, remainder = divmod(duration.seconds, 3600)
        minutes, seconds = divmod(remainder, 60)
        duration_label.config(text=f"Recording duration: {hours:02}:{minutes:02}:{seconds:02}")
    root.after(1000, update_duration)  # Schedule this function to be called again after 1 second


def send_action_commands(base_dir, start_time_str, framerate):
    global running, act_cmd, action_command_running

    # Initialize Pypylon and the action command
    tl_factory = py.TlFactory.GetInstance()
    gige_tl = tl_factory.CreateTl('BaslerGigE')
    act_cmd = gige_tl.ActionCommand(0x01, 0x01, 4294967295)
    action_command_running = True

    # Save the CSV file in the base directory
    csv_file_path = os.path.join(base_dir, f"action_commands_{start_time_str}.csv")

    with open(csv_file_path, 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Command Count', 'Timestamp'])  # Write header row

        command_count = 0
        while action_command_running:
            current_timestamp = datetime.now()
            ok = act_cmd.IssueNoWait()

            # Log the current timestamp to the CSV
            writer.writerow([command_count + 1, current_timestamp.strftime("%Y-%m-%d %H:%M:%S.%f")])
            command_count += 1

            time.sleep(1.0 / framerate)  # Adjust sleep time based on desired frame rate


def log_message(message):
    log.insert(tk.END, message + '\n')
    log.see(tk.END)

def select_directory():
    global base_dir
    base_dir = filedialog.askdirectory()
    dir_label.config(text=f"Saving images to: {base_dir}")


root = tk.Tk()
root.title("Basler Camera Control Suite")
root.configure(bg='lightgrey')

frame_rate = tk.StringVar()
frame_rate_label = tk.Label(root, text="Frame Rate")
frame_rate_label.grid(row=0, column=0, padx=10, pady=10)
frame_rate_dropdown = ttk.Combobox(root, textvariable=frame_rate)
frame_rate_dropdown['values'] = (20, 30, 40, 60, 50, 60)
frame_rate_dropdown.current(1)  # Set the default frame rate to the second one
frame_rate_dropdown.grid(row=1, column=0, padx=10, pady=10)

frame_rate.trace_add('write', update_frame_rate)

exposure_time = tk.StringVar(value='10000')  # Default exposure time
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
