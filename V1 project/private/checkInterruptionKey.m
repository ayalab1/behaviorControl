if KbCheck
    [~, ~, keyCode ] = KbCheck; % if any keyboard key is pressed
    if keyCode(123)>0 % if the key was F12
        abortScript = true; % if F12 was pressed, abort the script
        sca
        mode = 0;
        fprintf(aHandle,'%s',char(mode)); % write command to arduino to stop all reward ports
        data(currentLine,1) = GetSecs - startTime;
        data(currentLine,2) = -100; % abort script is coded as "-100"
        data(currentLine,3) = -1; % flash status "-1" is another code for blank screen
        data(currentLine,4) = trialNumber; % current trial number
        data(currentLine,5) = mode; % current mode (1=mouse port, 2=left port, 3=right port, 5=timeout)
        currentLine = currentLine+1;
        if write, dlmwrite(filename,data,'precision','%.8f'); end
        flushinput(aHandle); flushoutput(aHandle);
    end
end