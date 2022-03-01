% helper_Ymaze_video_left

% Shift/Cycle the LUT entries:

speed = -3; % netative for going left
newLUT(2:end,:) = circshift(newLUT(2:end,:),speed);
% Update the hardware CLUT with our newLUT:
Screen('LoadNormalizedGammaTable', win, newLUT, 2);
% This 'Flip' waits for vertical retrace...
Screen('Flip', win, 0, 1);
