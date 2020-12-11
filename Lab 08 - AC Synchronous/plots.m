position = csvread('y.csv');
voltage = csvread('u.csv');

tend = 1975*0.002
time = linspace(0,tend,1975);

figure
subplot(2,1,1)
plot(time, position)
xlabel('Time (seconds)')
ylabel('Y (radians)')

subplot(2,1,2)
plot(time, voltage)
xlabel('Time (seconds)')
ylabel('U (Volts)')



