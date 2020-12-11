position = csvread('ydat.csv');
voltage = csvread('udat.csv');
positioncomp = csvread('ydatwithcomp.csv');
voltagecomp = csvread('udatwithcomp.csv');

time = linspace(0,2,1975);

%without anti-windup compensation
figure
subplot(2,1,1)
plot(time, position)
title('Without Anti-Windup Compensation')
xlabel('Time (seconds)')
ylabel('Y (radians)')

subplot(2,1,2)
plot(time, voltage)
xlabel('Time (seconds)')
ylabel('U (Volts)')

%with anti-windup compensation
figure
subplot(2,1,1)
plot(time, positioncomp)
title('With Anti-Windup Compensation')
xlabel('Time (seconds)')
ylabel('Y (radians)')

subplot(2,1,2)
plot(time, voltagecomp)
xlabel('Time (seconds)')
ylabel('U (Volts)')


