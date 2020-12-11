%% Copyright 2014 by David G. Taylor. All rights reserved.
clc, clear, close all

%% plant simulation model parameters
N = 2;
Lm = 34.47e-3;
Ls = 37.24e-3;
Lr = 37.24e-3;
Rs = 1.00;
Rr = 2.24;
J = 3.2e-4;
F = 7.2e-4;
sLs = (1-Lm^2/(Ls*Lr))*Ls;

%% controller parameters
V_dc = 24;
Flux = 50e-3;
lambda_r = 25;
lambda_e = 4*lambda_r;
Ts = 0.0002;

%% plant design model parameters
g = Lm/Lr;
R = Rs+Rr*g^2;
K = g*N*Flux;
Jhat = J*1;
Fhat = F*0;
alpha = (K^2+Fhat*R)/(Jhat*R);
beta = K/(Jhat*R);

%% design model coefficient matrices
Ad = [0,1;0,-alpha];
Bd = [0;beta];
Cd = [1,0];

%% controller feedback gain matrices
LL = [2*lambda_e-alpha;lambda_e^2-2*alpha*lambda_e+alpha^2];
KK1 = [3*lambda_r^2,3*lambda_r-alpha]/beta;
KK2 = lambda_r^3/beta;

%% time grid
h = Ts/100;
t = 0:h:4;
td = 0:Ts:4;

%% preallocation
Y = NaN(1,length(td));
U = NaN(1,length(td));
V = NaN(1,length(td));
vABC = NaN(3,length(td));
iABC = NaN(3,length(td));

%% initial conditions
tr = 0; wr = 0; fD = 0; fQ = 0; iD = 0; iQ = 0;
xhat = [0;0];
sigma = 0;
rho = 0;

%% simulation time loop
for n = 0:length(t)-1
    
    %% microcontroller code (discrete-time update)
    
    if mod(n*h,Ts) == 0
        
        % sensor output
        y = tr;
        
        % reference command generator
        if mod(n*h,10000*Ts) >= 5000*Ts
            r = 2*pi;
        else
            r = 0;
        end
        
        % integral controller output
        u = -KK1*xhat-KK2*sigma;
        
        % integral controller state update
        xhat = xhat+Ts*(Ad*xhat+Bd*u-LL*(Cd*xhat-y));
        sigma = sigma+Ts*(y-r);
        
        % voltage vector
        vd = (Rs/Lm)*Flux;
        vq = u;
        
        % excitation angle
        rho = mod(rho+Ts*(g^2*Rr/R)*(u/K),2*pi);
        theta_e = mod((Rs/R)*y+rho,2*pi);
        
        % rectangular to polar
        v_mag = sqrt(vd^2+vq^2);
        v_ang = atan2(vq,vd);
        
        % converter leg voltages
        theta_A = N*theta_e-pi/6;
        theta_B = N*theta_e-pi/6-2*pi/3;
        theta_C = N*theta_e-pi/6+2*pi/3;
        vA = (V_dc/2)+sqrt(2/9)*v_mag*cos(theta_A+v_ang);
        vB = (V_dc/2)+sqrt(2/9)*v_mag*cos(theta_B+v_ang);
        vC = (V_dc/2)+sqrt(2/9)*v_mag*cos(theta_C+v_ang);
        
        % stator-frame voltages from leg voltages
        va = vA-vB;
        vb = vB-vC;
        vc = vC-vA;
        vD = sqrt(2/3)*va-sqrt(1/6)*vb-sqrt(1/6)*vc;
        vQ = sqrt(1/2)*vb-sqrt(1/2)*vc;
        
        % leg currents from stator-frame currents
        ia = sqrt(2/3)*iD;
        ib = -sqrt(1/6)*iD+sqrt(1/2)*iQ;
        ic = -sqrt(1/6)*iD-sqrt(1/2)*iQ;
        iA = ia-ic;
        iB = ib-ia;
        iC = ic-ib;
        
        % data logger
        k = round(n*h/Ts);
        Y(k+1) = y;
        U(k+1) = u;
        V(k+1) = v_mag;
        vABC(:,k+1) = [vA;vB;vC];
        iABC(:,k+1) = [iA;iB;iC];
        
    end
    
    %% plant physics (continuous-time update)
    
    % stator-frame model
    tr = tr+h*wr;
    wr = wr+h*((N*Lm/Lr)*(fD*iQ-fQ*iD)-F*wr)/J;
    fD = fD+h*(-(Rr/Lr)*fD-N*wr*fQ+(Lm/Lr)*Rr*iD);
    fQ = fQ+h*(-(Rr/Lr)*fQ+N*wr*fD+(Lm/Lr)*Rr*iQ);
    iD = iD+h*((Lm/Lr)*(Rr/Lr)*fD+(Lm/Lr)*N*wr*fQ-R*iD+vD)/sLs;
    iQ = iQ+h*((Lm/Lr)*(Rr/Lr)*fQ-(Lm/Lr)*N*wr*fD-R*iQ+vQ)/sLs;
    
end

figure
subplot(311), plot(td,Y), ylabel('y [rad]')
subplot(312), plot(td,U), ylabel('u [V]')
subplot(313), plot(td,V), ylabel('|v| [V]'), xlabel('t [s]')
print -depsc ABC1.eps

figure
subplot(211), plot(td,vABC), ylabel('v_{ABC} [V]')
subplot(212), plot(td,iABC), ylabel('i_{ABC} [A]'), xlabel('t [s]')
print -depsc ABC2.eps