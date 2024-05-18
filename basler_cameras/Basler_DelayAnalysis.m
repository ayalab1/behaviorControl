log_camera = 'D:\videoCaptures\test2\time.csv';
log_gpio = 'C:\Users\Cornell\Documents\UARTLog\comport_data_20240518_184740.txt';

data1 = readtable(log_camera, 'Format', '%d%{yyyy-MM-dd HH:mm:ss.SSSSSS}D', 'Delimiter', ',');
% data1 = readtable(log_gpio, 'Format', '%d%{yyyy-MM-dd HH:mm:ss.SSSSSS}D', 'Delimiter', ',');
% % data1 = importdata(log_camera);
% data2=importdata(log_gpio);

fileID = fopen(log_gpio, 'r');

% Read data using textscan
data = textscan(fileID, 'Time: %s Data: %d', 'Delimiter', ',');
fclose(fileID);

% Extract and convert the timestamp strings to datetime objects
time1 = datetime(data1.Timestamp,'InputFormat', 'yyyy-MM-dd HH:mm:ss.SSSSSS');
time2 = datetime(data{1}, 'InputFormat', 'yyyy-MM-dd HH:mm:ss.SSSSSS');
Nsamples = min(length(time1),length(time2));
t_d = milliseconds(time2(1:Nsamples)-time1(1:Nsamples));

%%
histogram(t_d,50)
disp(['Std:' num2str(std(t_d)) ' Mean:' num2str(median(t_d))]);
%%

% Path to the video file
filename = 'D:\videoCaptures\test\C01__40482791__20240518_160219189.mp4';

% Create a VideoReader object
vidObj = VideoReader(filename);

%%

% Directory containing the video files
videoDir = 'D:\videoCaptures\test2'; % Change this to your directory

% Get a list of MP4 files in the directory
files = dir(fullfile(videoDir, '*.mp4'));

% Frame index to retrieve
frameIndex = 86202;

% Check if the folder is empty or no mp4 files are found
if isempty(files)
    disp('No MP4 files found in the specified directory.');
    return;
end

% Create a figure to display frames
figure;
col = ceil(sqrt(length(files))); % calculate number of columns in subplot
row = ceil(length(files) / col); % calculate number of rows in subplot

% Loop through each file
for i = 1:length(files)
    videoPath = fullfile(files(i).folder, files(i).name);
    vidObj = VideoReader(videoPath);

    % Check if the video has enough frames
    if vidObj.NumberOfFrames >= frameIndex || hasFrame(vidObj)
        % Read up to the desired frame if no direct access to NumberOfFrames
        currentFrame = read(vidObj, frameIndex);
        
        % Display the frame
        subplot(row, col, i);
        imshow(currentFrame);
        title(sprintf('Frame %d of %s', frameIndex, files(i).name));
    else
        disp(['The video ' files(i).name ' does not have ' num2str(frameIndex) ' frames.']);
    end
end
