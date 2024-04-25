function [Kp, Kd, Alpha, A_coeff,B_coeff, Disc_Controller] = getController(Gest, Tr, os, Hz_control)


%% Define System ID Values
a = Gest.Numerator(1);
b = Gest.Denominator(2);


%% Define Desired Pole Locations

% Continuous time design - compute poles
zeta = sqrt(log(os)^2 / (pi^2 + log(os)^2));
wn_tr = (1.53 + 2.31*zeta^2) / Tr;
wn = wn_tr;

sigma = zeta*wn;
Tp = (2*pi)/wn;


% By Step Response
N = 5;
order = 3;

lambda_step(1,1) = -zeta*wn + 1i*wn*sqrt(1-zeta^2);
lambda_step(2,1) = -zeta*wn - 1i*wn*sqrt(1-zeta^2);
lambda_step(3,1) = -N*zeta*wn;


% By Bessel Response
[~, lambda_bessel, ~] = besself(order, (2*pi)/Tp);

lambda = lambda_step;

%% Setup Equations And Calculate Gains/Alpha
syms kp kd al 'real'

Gfun = @(s) a/(s*(s+b));
Hfun = @(s) kp + (kd*s)*(al/(s+al));

soln = solve([1 + Gfun(lambda(1))*Hfun(lambda(1));
    1 + Gfun(lambda(2))*Hfun(lambda(2));
    1 + Gfun(lambda(3))*Hfun(lambda(3))]);

kp_s = eval(soln.kp);
kd_s = eval(soln.kd);
alpha = eval(soln.al);

control_freq = Hz_control;
nyquist_freq = control_freq / 2;
fprintf("Alpha percent of Nyquest at %d Hz control loop: %3.1f%%\n\n", Hz_control, (alpha/(2*pi*nyquist_freq)) *100);


%% Plot response to determine if acceptable
s = tf([1 0],1);
H1 = kp_s+(kd_s*s)*(alpha/(s+alpha));
H2 = kp_s;
G = a/(s*(s+b)); %GH
cl_tf = feedback(H2*G, H1/H2);

figure(1)
subplot(1,2,1)
step(cl_tf);
subplot(1,2,2)
bode(cl_tf);

response_data = stepinfo(cl_tf);

%% Check robustness to a and b
iters = 1000;
Tp_list = zeros(N, 1);
OS_list = zeros(N, 1);
Ts_list = zeros(N, 1);
Tr_list = zeros(N, 1);

a_unc = 0.05; % 10% uncertainty
b_unc = 0.05; % 10% uncertainty

for i=1:iters
    a_p = a*(1+a_unc*randn(1));
    b_p = b*(1+b_unc*randn(1));

    ol_tf = a_p/(s*(s+b_p)); %G
    cl_tf = feedback(H2*ol_tf, H1/H2); %GH/(1+GH)

    if any(real(eig(cl_tf))>0)
        disp('Unstable system found!')
    end

    info = stepinfo(cl_tf);
    Tp_list(i) = info.PeakTime;
    Tr_list(i) = info.RiseTime;
    OS_list(i) = info.Overshoot;
    Ts_list(i) = info.SettlingTime;
end

fprintf('\n---Monte Carlo Results---\n');
fprintf('Time to Peak:\t%2.2fs +/- %2.2fs, MIN: %2.2fs, MAX: %2.2fs\n', mean(Tp_list), std(Tp_list), min(Tp_list), max(Tp_list));
fprintf('Time to Rise:\t%2.2fs +/- %2.2fs, MIN: %2.2fs, MAX: %2.2fs\n', mean(Tr_list), std(Tr_list), min(Tr_list), max(Tr_list));
fprintf('%% Overshoot:\t%2.2f%% +/- %2.2f%%, MIN: %2.2fs, MAX: %2.2f%%\n', mean(OS_list), std(OS_list), min(OS_list), max(OS_list));
fprintf('Time to Settle:\t%2.2fs +/- %2.2fs, MIN: %2.2fs, MAX: %2.2fs\n', mean(Ts_list), std(Ts_list), min(Ts_list), max(Ts_list));

%% Transform to Z-Domain
dt = 1/control_freq;
disc_controller = c2d(H1/H2, dt);
[A, B] = tfdata(disc_controller);
fprintf('A coeff:\t%12.12f, %12.12f\n', A{1});
fprintf('B coeff:\t%12.12f, %12.12f\n', B{1});
fprintf('Kp: %d\n', kp_s)
fprintf('Kd: %d\n', kd_s)
fprintf('Control period: %d\n', 1/Hz_control)



Kd = kd_s;
Kp = kp_s;
Disc_Controller = disc_controller;
Alpha = alpha;
A_coeff = A;
B_coeff = B;

end

