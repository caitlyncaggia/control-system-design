
position = csvread('measuredposition.csv');
voltage = csvread('avgvoltage.csv');
time = linspace(0,2,2000);

figure
subplot(2,1,1)
plot(time, position)
xlabel('Time (seconds)')
ylabel('Measured Position (radians)')

subplot(2,1,2)
plot(time, voltage)
xlabel('Time (seconds)')
ylabel('Desired Average Voltage (Volts)')

