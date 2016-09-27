model TransmissionLineEquations
  "Transmission line circuit - Direct implementation by equations"
  import Modelica.SIunits;
  parameter Integer N = 10 "number of segments";
  parameter Real L = 100 "length of the transmission line";
  final parameter Real l = L / N "length of the each segment";
  parameter Real res = 48e-6 "resistance per meter";
  parameter Real cap = 101e-12 "capacitance per meter";
  parameter Real ind = 253e-9 "inductance per meter";
  parameter Real RL = (ind / cap) ^ (1 / 2) "load resistance";
  parameter Real w =  1 / 2e-7;
  final parameter Real TD = L / v "time delay of the transmission line";
  final parameter Real v = 1 / (ind * cap) ^ (1 / 2) "velocity of the signal";
  Real Vstep = if time > 0 then 1 else 0 "input step voltage";
  Real cur[N](each start = 0) "current values at the nodes of the transmission line";
  Real vol[N](each start = 0) "voltage values at the nodes of the transmission line";
  Real vvol "derivative of input voltage";
equation
  vvol = der(vol[1]);
  Vstep = vol[1] + 2 * (1 / w) * der(vol[1]) + 1 / w ^ 2 * der(vvol);
  // This is a derivative of   vol[N] = cur[N] * RL;  
  der(vol[N]) = der(cur[N]) * RL;
  for i in 1:N - 1 loop
    cap * der(vol[i + 1]) = (cur[i] - cur[i + 1]) / l;
    ind * der(cur[i]) = (-res * cur[i]) - (vol[i + 1] - vol[i]) / l;
  end for;
initial equation
  vol = zeros(N);
  cur[1:N-1] = zeros(N-1);
  vvol = 0;
  annotation(Documentation(info = "<html><p>In this model, a transmission line circuit is implemented by equations. Transmission line circuit is represented as in the figure below. The application is the same as the TransmissionLineModelica model.</p><p><img src=\"modelica://ScalableTestSuite/Resources/Images/TransmissionLine/TransmissionLineModelica.png\"/></p><p>Considering the nodes of the discrete transmission line(implemented in Electrical.Models.TransmissionLine), circuit equations are described. In the transmission line, there are N segments, therefore, there will be N+1 nodes and N+1 voltage and current variables.</p><p><img src=\"modelica://ScalableTestSuite/Resources/Images/TransmissionLine/tlmequation.png\"/></p><p>where j= 2,..,N and Rx is the resistance per meter, Cx is the capacitance per meter and Lx is the inductance per meter.</p><p>output voltage is described as:</p><p><img src=\"modelica://ScalableTestSuite/Resources/Images/TransmissionLine/tlmequation1.png\"/></p><p>Moreover, considering the form of the second order low pass filter, equation of the filter to a step input can be defined in the following way:</p><p><img src=\"modelica://ScalableTestSuite/Resources/Images/TransmissionLine/tlmequation2.png\"/></p><p>where Vstep  is the step voltage and v1is the output voltage of the filter. The parameters of the TransmissionLineEquations are:</p><table border=\"1\" cellspacing=\"0\" cellpadding=\"2\">
<tr>
  <th>Parameters</th>
  <th>Comment</th>
</tr>
            <tr>
  <td valign=\"top\">N</td>
  <td valign=\"top\">number of segments</td>
</tr>
<tr>
  <td valign=\"top\">L</td>
  <td valign=\"top\">length of transmission line</td>
</tr>
<tr>
  <td valign=\"top\">l</td>
  <td valign=\"top\">length of each segment</td>
</tr>
<tr>
  <td valign=\"top\">res</td>
  <td valign=\"top\">resistor per meter</td>
</tr>
            <tr>
  <td valign=\"top\">cap</td>
  <td valign=\"top\">capacitance per meter</td>
</tr>
            <tr>
  <td valign=\"top\">ind</td>
  <td valign=\"top\">inductance per meter</td>
</tr>
            <tr>
  <td valign=\"top\">RL</td>
  <td valign=\"top\">load resistance</td>
</tr>
            <tr>
  <td valign=\"top\">w</td>
  <td valign=\"top\">angular frequency</td>
</tr>
            <tr>
  <td valign=\"top\">TD</td>
  <td valign=\"top\">time delay of transmission line</td>
</tr>
            <tr>
  <td valign=\"top\">v</td>
  <td valign=\"top\">velocity of signal</td>
</tr>
</table>
</html>"));
end TransmissionLineEquations;

