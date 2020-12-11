% from "Least-Squares Parameter Identification" notes
% modify as necessary to conform to your data files

clc, clear, close all

T = 0.002;
t = (0:1999)*T;
u = xlsread('avgvoltage.csv')';
y = xlsread('measuredposition.csv')';

lambda = 50;
lambda0 = lambda^2;
lambda1 = 2*lambda;

AA = [0,1;-lambda0,-lambda1];
BB = [0;lambda0];
C0 = [1,0];
D0 = 0;
C1 = [0,1];
D1 = 0;
C2 = [-lambda0,-lambda1];
D2 = lambda0;

x0 = zeros(2,length(t));
for n = 1:length(t)-1
    x0(:,n+1) = x0(:,n)+T*(AA*x0(:,n)+BB*u(n));
end
u0 = C0*x0+D0*u;

x1 = zeros(2,length(t));
for n = 1:length(t)-1
    x1(:,n+1) = x1(:,n)+T*(AA*x1(:,n)+BB*y(n));
end
y1 = C1*x1+D1*y;

x2 = zeros(2,length(t));
for n = 1:length(t)-1
    x2(:,n+1) = x2(:,n)+T*(AA*x2(:,n)+BB*y(n));
end
y2 = C2*x2+D2*y;

data_matrix = [-y1',u0'];
data_vector = y2';
parameters = data_matrix\data_vector;
alpha = parameters(1);
beta = parameters(2);

A = [0,1;0,-alpha];
B = [0;beta];
C = [1,0];

x = zeros(2,length(t));
for n = 1:length(t)-1
    x(:,n+1) = x(:,n)+T*(A*x(:,n)+B*u(n));
end
y_sim = C*x;

figure
subplot(211), plot(t,u,'LineWidth',1.5)
xlabel('t [s]'), ylabel('u [V]'), ylim([-30,30])
subplot(212), plot(t,y,t,y_sim,'LineWidth',1.5)
xlabel('t [s]'), ylabel('y [rad]'), ylim([-20,80])
legend('measured','simulated','Location','NorthEast')