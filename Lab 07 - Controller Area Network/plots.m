y = csvread('y.csv');
u = csvread('u.csv');
r = csvread('r.csv');

time = linspace(0,1.975,1975);

figure
subplot(2,1,1)
plot(time, y, time, r)
legend('y','r')
xlabel('Time (seconds)')
ylabel('Measured Position (radians)')

subplot(2,1,2)
plot(time, u)
xlabel('Time (seconds)')
ylabel('Voltage (Volts)')

