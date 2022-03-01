fwrite(aHandle,4,'uchar');% ask arduino send info
if(aHandle.BytesAvailable>6) % receive it if there is one
    signal0 =  fread(aHandle,aHandle.BytesAvailable);
    timestamp = GetSecs;
    signalEnd = signal0(end-6:end); % the last 7 digits contain 6 with the signal and 1 with the marker "250"
    signal = circshift(signalEnd,-find(signalEnd==250)); % shift them so that the marker "250" is last
    
    if ~mousePortCrossed && signal(1)>0
        data(currentLine,1) = timestamp - startTime;
        data(currentLine,2) = -10; % mouse port crossing is coded as stimulus "-10"
        data(currentLine,3) = -1; % flash status "-1" is another code for blank screen
        data(currentLine,4) = trialNumber; % current trial number
        data(currentLine,5) = mode; % current mode (1=mouse port, 2=left port, 3=right port, 5=timeout)
        currentLine = currentLine+1;
    end
    if ~leftPortCrossed && signal(2)>0
        data(currentLine,1) = timestamp - startTime;
        data(currentLine,2) = -20; % left port crossing is coded as stimulus "-20"
        data(currentLine,3) = -1; % flash status "-1" is another code for blank screen
        data(currentLine,4) = trialNumber; % current trial number
        data(currentLine,5) = mode; % current mode (1=mouse port, 2=left port, 3=right port, 5=timeout)
        currentLine = currentLine+1;
    end
    if ~rightPortCrossed && signal(3)>0
        data(currentLine,1) = timestamp - startTime;
        data(currentLine,2) = -30; % right port crossing is coded as stimulus "-30"
        data(currentLine,3) = -1; % flash status "-1" is another code for blank screen
        data(currentLine,4) = trialNumber; % current trial number
        data(currentLine,5) = mode; % current mode (1=mouse port, 2=left port, 3=right port, 5=timeout)
        currentLine = currentLine+1;
    end
    mousePortCrossed = signal(1); leftPortCrossed = signal(2); rightPortCrossed = signal(3); % update values
    arduinoOutputs(end+1,:) = [timestamp - startTime; signal];
    if show, for i=1:6, subplot(3,2,i); plot(arduinoOutputs(:,i+1)); end; drawnow; end
    flushinput(aHandle); flushoutput(aHandle);
end
