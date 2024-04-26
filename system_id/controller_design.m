
clc; clear; clear vars;

% files = dir("data/*_OUTPUT.csv");
files = dir("sysid_73916/output.csv");


SECONDS_PER_MILLISECOND = 0.001;

ENCODER_TICKS_PER_MM = 1.55; % Experimentally captured
M_PER_ENCODER_TICK = ENCODER_TICKS_PER_MM / 1000;

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
    y_ls = exp_data(:,5); % * MM_PER_ENCODER_TICK;

    % Right motor
    u_rs = exp_data(:,4);
    y_rs = exp_data(:,6); % * MM_PER_ENCODER_TICK;

    num_samples = length(exp_data);

    % Collect left into iddata - POSITION MODE (millimeters)
    data_l = iddata(y_ls, u_ls, dt_avg);
    data_l.InputName = 'PWM_L';
    data_l.InputUnit = 'PWM_val (-255,255)';
    data_l.OutputName = 'Position_L';
    data_l.OutputUnit = 'encoder ticks';
    data_l.TimeUnit = 'seconds';

    % Collect right into iddata - POSITION MODE (millimeters)
    data_r = iddata(y_rs, u_rs, dt_avg);
    data_r.InputName = 'PWM_R';
    data_r.InputUnit = 'PWM_val (-255,255)';
    data_r.OutputName = 'Position_R';
    data_r.OutputUnit = 'encoder ticks';
    data_r.TimeUnit = 'seconds';

    % Plot input and output over time
    figure((10 * f) + 1);
    plot(data_l, data_r)
    grid on


    % Estimate transfer functions
    np = 2;
    nz = 0;

    % TF Left (Position)
    Gest_l = tfest(data_l, np, nz)

    % TF Right (Position)
    Gest_r = tfest(data_r, np, nz)


    % Validate the left estimated transfer function
    figure((10 * f) + 2);
    opt = compareOptions;
    opt.InitialCondition = 'z';
    compare(data_l, Gest_l, opt)
    set(findall(gca, 'Type', 'Line'),'LineWidth',4);
    grid on

    % Validate the right estimated transfer function
    figure((10 * f) + 3);
    opt = compareOptions;
    opt.InitialCondition = 'z';
    compare(data_r, Gest_r, opt)
    set(findall(gca, 'Type', 'Line'),'LineWidth',4);
    grid on

end


% 1M/s

Tr_des = 0.1;
OS_des = 0.03;

dt_des = 0.01;
control_hz = 1/dt_des;


PICK_POLES_WITH_STEP_INSTEAD_OF_BESSEL = false;

disp("-------------");
disp("\n\nLEFT CONTROLLER - POSITION\n\n");
[Kp_L, Kd_L, Alpha_L, A_coeff_L, B_coeff_L, Disc_Controller_L] = getController(Gest_l, Tr_des, OS_des, control_hz, PICK_POLES_WITH_STEP_INSTEAD_OF_BESSEL);

A_coeff_L = A_coeff_L{1};
B_coeff_L = B_coeff_L{1};

disp("-------------");
disp("RIGHT CONTROLLER - POSITION/");
[Kp_R, Kd_R, Alpha_R, A_coeff_R, B_coeff_R, Disc_Controller_R] = getController(Gest_r, Tr_des, OS_des, control_hz, PICK_POLES_WITH_STEP_INSTEAD_OF_BESSEL);

A_coeff_R = A_coeff_R{1};
B_coeff_R = B_coeff_R{1};

