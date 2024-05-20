from pypylon import pylon
import numpy as np
import cv2
import time
import warnings
import os
import traceback
import tkinter as tk
from tkinter import filedialog

framerate = 30.
exposure = 10.
gain = 1.
compression_ratio = 70.
pixel_scaling = 2.79

def select_directory():
    root = tk.Tk()
    root.withdraw()  # Hide the main window
    output_directory = filedialog.askdirectory(title="Select Output Directory")
    return output_directory

# Call the function to select a directory
output_directory = select_directory()
print("Selected output directory:", output_directory)

# Create output directory if it doesn't exist
os.makedirs(output_directory, exist_ok=True)

def init_ptp(camera, index):
    # PTP setup
    camera.PtpEnable.SetValue(False)
    camera.BslPtpPriority1.SetValue(128)
    camera.BslPtpProfile.SetValue("DelayRequestResponseDefaultProfile")
    camera.BslPtpNetworkMode.SetValue("Multicast")
    camera.BslPtpManagementEnable.SetValue(False)
    camera.BslPtpTwoStep.SetValue(False)
    camera.PtpEnable.SetValue(True)

def check_ptp(cameras):
    initialized = False
    locked = False

    # Wait until correctly initialized or timeout
    time1 = time.time()
    while not initialized:
        status_arr = np.zeros(cameras.GetSize(), dtype=np.bool_)
        for i, camera in enumerate(cameras):
            camera.PtpDataSetLatch.Execute()
            status_arr[i] = (camera.PtpStatus.GetValue() == 'Master' \
                            or camera.PtpStatus.GetValue() != 'Initializing')
        initialized = np.all(status_arr)
        if (time.time() - time1) > 3:
            if not initialized:
                warnings.warn('PTP not initialized -> Timeout')
            break

    # If correctly initialized, wait until settled or timeout
    if initialized:
        time2 = time.time()
        while not locked:
            status_arr = np.zeros(cameras.GetSize(), dtype=np.bool_)
            status_string = ''
            for i, camera in enumerate(cameras):
                camera.PtpDataSetLatch.Execute()
                status_arr[i] = (camera.PtpStatus.GetValue() == 'Master' \
                                or camera.PtpServoStatus.GetValue() == 'Locked')
                status_string += 'Camera {:d} locked: {} | '.format(i, status_arr[i])
            print(status_string)
            locked = np.all(status_arr)
            if (time.time() - time2) > 30:
                if not locked:
                    warnings.warn('PTP not locked -> Timeout')
                break

    return initialized and locked

def init_camera(camera, index):
    camera.GainAuto.SetValue("Off")
    camera.Gain.SetValue(gain)

    camera.ExposureAuto.SetValue("Off")
    camera.ExposureTime.SetValue(int(exposure*1000))

    # Beyond Pixel setup
    camera.PixelFormat.SetValue("Mono8")
    # camera.BslScalingFactor.SetValue(pixel_scaling)

    # Beyond Compression setup
    # camera.ImageCompressionMode.SetValue("BaslerCompressionBeyond")
    # camera.ImageCompressionRateOption.SetValue("FixRatio")
    # camera.BslImageCompressionRatio.SetValue(compression_ratio)

    # Periodic Signal setup
    if camera.BslPeriodicSignalSource.GetValue() != 'PtpClock':
        warnings.warn('Clock source of periodic signal is not `PtpClock`')
    camera.BslPeriodicSignalPeriod.SetValue(1 / framerate * 1e6)
    camera.BslPeriodicSignalDelay.SetValue(0)
    camera.TriggerSelector.SetValue("FrameStart")
    camera.TriggerMode.SetValue("On")
    camera.TriggerSource.SetValue("PeriodicSignal1")

    # # Transport Layer Control
    # camera.GevSCPD.SetValue(222768)
    # camera.GevSCFTD.SetValue(8018*index)
    # camera.GevSCPSPacketSize.SetValue(8000)

# Define your image handler class
class ImageHandler(pylon.ImageEventHandler):
    def __init__(self, camera_index):
        super().__init__()
        self.camera_index = camera_index
        self.num_frames = 0

    def OnImageGrabbed(self, camera, grab_result):
        try:
            if grab_result.GrabSucceeded():
                image = grab_result.GetArray()
                self.num_frames += 1

                current_time_seconds = time.time()
                time_struct = time.localtime(current_time_seconds)
                current_time = time.strftime("%Y%m%d_%H%M%S",
                                             time_struct) + f"_{int((current_time_seconds - int(current_time_seconds)) * 1000):03d}"
                filename = f"camera{self.camera_index}_frame{self.num_frames}_{current_time}.jpg"
                cv2.imwrite(os.path.join(output_directory, filename), image)

            grab_result.Release()

        except Exception as e:
            traceback.print_exc()

# Get the transport layer factory
tlFactory = pylon.TlFactory.GetInstance()

# Get all attached devices and exit application if no device is found
devices = tlFactory.EnumerateDevices()
cam_count = len(devices)
if not cam_count:
    raise EnvironmentError('No camera device found')

# prepare count frames
num_frames = [0] * cam_count

# Create and attach all Pylon Devices
cameras = pylon.InstantCameraArray(cam_count)
for camera, device in zip(cameras, devices):
    print('Using {:s} @ {:s}'.format(device.GetModelName(), device.GetIpAddress()))
    camera.Attach(tlFactory.CreateDevice(device))
    camera.Open()

# Initialize PTP and check initialization
for i, camera in enumerate(cameras):
    init_ptp(camera, i)

success = check_ptp(cameras)
if not success:
    raise EnvironmentError('PTP initialization was not successful')

# Initialize general camera parameters
for i, camera in enumerate(cameras):
    init_camera(camera, i)

# Image decompression
# decompressor = pylon.ImageDecompressor()
# descriptor = cameras[0].BslImageCompressionBCBDescriptor.GetAll()
# decompressor.SetCompressionDescriptor(descriptor)

# Prepare image grabbing
imgs = [None] * cam_count
ids = [None] * cam_count

# cv2.namedWindow('image', cv2.WINDOW_NORMAL)
# Create smaller OpenCV windows for both camera streams
cv2.namedWindow('Camera 1 Stream', cv2.WINDOW_NORMAL)
cv2.resizeWindow('Camera 1 Stream', 960, 600)  # Adjust the size as needed

cv2.namedWindow('Camera 2 Stream', cv2.WINDOW_NORMAL)
cv2.resizeWindow('Camera 2 Stream', 960, 600)  # Adjust the size as needed

# Initialize image handlers
image_handlers = [ImageHandler(i) for i in range(cam_count)]
for i, camera in enumerate(cameras):
    camera.RegisterImageEventHandler(image_handlers[i], pylon.RegistrationMode_ReplaceAll, pylon.Cleanup_Delete)

# cameras.StartGrabbing(pylon.GrabStrategy_LatestImageOnly)
cameras.StartGrabbing(pylon.GrabStrategy_LatestImageOnly, pylon.GrabLoop_ProvidedByInstantCamera)

start_time = time.time()

while cameras.IsGrabbing():

    if cv2.waitKey(1) & 0xFF == 27:
        break

end_time = time.time()

cameras.StopGrabbing()
cameras.Close()
# out1.release()
# out2.release()

# Calculate the elapsed time
elapsed_time = end_time - start_time

print(f"Recording finished. Elapsed time: {elapsed_time:.2f} seconds")

# Print total acquisition stats
for i, handler in enumerate(image_handlers):
    frame_rate = handler.num_frames / elapsed_time
    print(f"Total frames acquired (Camera {i+1}): {handler.num_frames}, Frame rate: {frame_rate} fps")
