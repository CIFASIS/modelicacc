model TransmissionLineEquations
  "Transmission line circuit - Direct implementation by equations"
  parameter Integer N = 100 "number of segments";
  parameter Real L = 100 "length of the transmission line";
  final parameter Real l = L / N "length of the each segment";
  parameter Real res = 4.8e-05 "resistance per meter";
  parameter Real cap = 1.01e-10 "capacitance per meter";
  parameter Real ind = 2.53e-07 "inductance per meter";
  parameter Real RL = (ind / cap) ^ (1 / 2) "load resistance";
  parameter Real w = 5000000.0;
  final parameter Real TD = L / v "time delay of the transmission line";
  final parameter Real v = 1 / (ind * cap) ^ (1 / 2) "velocity of the signal";
  Real Vstep = if time > 0 then 1 else 0 "input step voltage";
  Real cur_N (start = 0) "current values at the nodes of the transmission line";
  Real cur[N-1](each start = 0) "current values at the nodes of the transmission line";
  Real vol[N](each start = 0) "voltage values at the nodes of the transmission line";
  Real vvol "derivative of input voltage";
equation
  vvol = der(vol[1]);
  Vstep = vol[1] + 2 * (1 / w) * der(vol[1]) + 1 / w ^ 2 * der(vvol);
  vol[N] = cur_N * RL;  
  for i in 1:N - 2 loop
    cap * der(vol[i + 1]) = (cur[i] - cur[i + 1]) / l;
    ind * der(cur[i]) = (-res * cur[i]) - (vol[i + 1] - vol[i]) / l;
  end for;
  ind * der(cur[N-1]) = (-res * cur[N-1]) - (vol[N] - vol[N-1]) / l;
  cap * der(vol[N]) = (cur[N-1] - cur_N) / l;
initial equation
  vol = zeros(N);
  cur[1:N-1] = zeros(N-1);
  vvol = 0;
  annotation(experiment(StopTime = 4e-6, Tolerance = 1e-6));
end TransmissionLineEquations;

