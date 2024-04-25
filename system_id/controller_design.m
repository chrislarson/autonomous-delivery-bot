
clc; clear; clear vars;

% files = dir("data/*_OUTPUT.csv");
files = dir("sysid_73916/output.csv");


SECONDS_PER_MICROSECOND = 0.000001;
SECONDS_PER_MILLISECOND = 0.001;
WHEEL_DIAMETER = 38 + (2*5); % millimeters (inner circle + track thickness)

MM_PER_REVOLUTION = 2 * pi * WHEEL_DIAMETER; % millimeters

ENCODER_TICKS_PER_REVOLUTION = 400; % approx

MM_PER_ENCODER_TICK = MM_PER_REVOLUTION / ENCODER_TICKS_PER_REVOLUTION;



% 
% SECONDS_PER_MICROSECOND = 0.000001;
% RADIANS_PER_ENCODER_CT = 0.0069;


for f=1:length(files)
    file = files(f,:);
    file_path = fullfile(file.folder, file.name);

    exp_data = readmatrix(file_path);
    % Fill missing (NaN) linearly
    exp_data = fillmissing(exp_data, 'linear');

    % Select relevant data
    sample_ts = exp_data(:,2) * SECONDS_PER_MILLISECOND;
    dt_avg = mean(diff(sample_ts));

    % Left motor
    u_ls = exp_data(:,3);
    y_ls = exp_data(:,5) * MM_PER_ENCODER_TICK;

    % Right motor
    u_rs = exp_data(:,4);
    y_rs = exp_data(:,6) * MM_PER_ENCODER_TICK;

    num_samples = length(exp_data);


    % Collect left into iddata - POSITION MODE
    data_l = iddata(y_ls, u_ls, dt_avg);
    data_l.InputName = 'PWM_L';
    data_l.InputUnit = 'PWM_val';
    data_l.OutputName = 'Position_L (mm)';
    data_l.OutputUnit = 'mm';
    data_l.TimeUnit = 'seconds';


    % Collect right into iddata - POSITION MODE
    data_r = iddata(y_rs, u_rs, dt_avg);
    data_r.InputName = 'PWM_R';
    data_r.InputUnit = 'PWM_val';
    data_r.OutputName = 'Position_R (mm)';
    data_r.OutputUnit = 'mm';
    data_r.TimeUnit = 'seconds';


    % Estimate transfer functions
    np = 1;
    nz = 0;


    % TF Left (Position)
    Gest_l = tfest(data_l, np, nz);
    % Gest_l_filename = strcat("Gest_L_", file.name(1:end-4), ".mat");
    % Gest_l_filepath = fullfile(pwd, "estimated_tfs", Gest_l_filename);
    % save(Gest_l_filepath, 'Gest_l');


    % TF Right (Position)
    Gest_r = tfest(data_r, np, nz);
    % Gest_r_filename = strcat("Gest_R_vel_", file.name(1:end-4), ".mat");
    % Gest_r_filepath = fullfile(pwd, "estimated_tfs", Gest_r_filename);
    % save(Gest_r_filepath, 'Gest_r');


    % Validate the left estimated transfer function
    figure((10 * f) + 2);
    opt = compareOptions;
    opt.InitialCondition = 'z';
    compare(data_l, Gest_l, opt)
    set(findall(gca, 'Type', 'Line'),'LineWidth',4);
    grid on

end


Tr_des = 0.1;
OS_des = 0.03;

dt_des = 0.02;
control_hz = 1/dt_des;


disp("-------------");
disp("\n\nLEFT CONTROLLER - POSITION\n\n");
[Kp_L, Kd_L, Alpha_L, A_coeff_L,B_coeff_L, Disc_Controller_L] = getController(Gest_l, Tr_des, OS_des, control_hz);

A_coeff_L = A_coeff_L{1};
B_coeff_L = B_coeff_L{1};

disp("-------------");
disp("RIGHT CONTROLLER - POSITION/");
[Kp_R, Kd_R, Alpha_R, A_coeff_R,B_coeff_R, Disc_Controller_R] = getController(Gest_r, Tr_des, OS_des, control_hz);

A_coeff_R = A_coeff_R{1};
B_coeff_R = B_coeff_R{1};

