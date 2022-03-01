% helper_Ymaze_prepare_left

origLUT = Screen('ReadNormalizedGammaTable', screenid);
if size(origLUT, 1) ~= 256
    origLUT = origLUT(1:256, :);
end

newLUT = origLUT; newLUT(1,:)=0.5;
left_s = floor(min(width, height)/2)*10;
left_x = meshgrid(-left_s:left_s, -left_s:left_s);
left_fintex = ones(2*left_s+1,2*left_s+1);
left_fintex(:,:) = mod(left_x,255)+1;
left_texture = Screen('MakeTexture', win, left_fintex);

% Black background:
Screen('FillRect',win, 0);
% Single static gray-level ramp drawn as texture.
Screen('DrawTexture', win, left_texture(1));