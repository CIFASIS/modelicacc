model CocurrentHeatExchangerEquations
  "cocurrent heat exchanger implemented by equations"
  import Modelica.SIunits;
  parameter Real L=1 "length of the channels";
  parameter Integer N=20 "number of nodes";
  parameter Real wB=1 "mass flow rate of fluid B";
  parameter Real areaA=1 "cross sectional area of channel A";
  parameter Real areaB=1 "cross sectional area of channel B";
  parameter Real rhoA=1 "density of fluid A";
  parameter Real rhoB=1 "density of fluid B";
  parameter SIunits.SpecificHeatCapacity cpA=1
    "specific heat capacity of fluid A";
  parameter SIunits.SpecificHeatCapacity cpB=1
    "specific heat capacity of fluid B";
  parameter SIunits.SpecificHeatCapacity cpW=1
    "specific heat capacity of the wall";
  parameter SIunits.CoefficientOfHeatTransfer gammaA=1
    "heat transfer coefficient of fluid A";
  parameter SIunits.CoefficientOfHeatTransfer gammaB=1
    "heat transfer coefficient of fluid B";
  parameter Real omega=1 "perimeter";
  final parameter Real l = L / (N - 1)
    "length of the each wall segment";
  Real wA "mass flow rate of fluid A";
  Real QA[N - 1]
    "heat flow rate of fluid A in the segments";
  Real QB[N - 1]
    "heat flow rate of fluid B in the segments";
  Real TA_1 "temperature nodes on channel A";
  Real TB_1 "temperature nodes on channel B";
  Real TA[N - 1] "temperature nodes on channel A";
  Real TB[N - 1] "temperature nodes on channel B";
  Real TW[N - 1] "temperatures on the wall segments";
  Real QtotA "total heat flow rate of fluid A";
  Real QtotB "total heat flow rate of fluid B";
initial equation
  for i in 1:N-1 loop
    TA[i] = 300;
    TB[i] = 300;
    TW[i] = 300;
  end for;
equation

  der(TA[1]) = 0; // This is the derivative of equation TA[1] = if time < 8 then 300 else 301;
  der(TB[1]) = 0; // This is the derivative of TB[1] = 310;
  wA = if time < 15 then 1 else 1.1;
  for i in 1:N - 1 loop
    rhoA * l * cpA * areaA * der(TA[i]) = wA * cpA * TA[i-1] - wA * cpA * TA[i] + QA[i];
    rhoB * l * cpB * areaB * der(TB[i]) = wB * cpB * TB[i-1] - wB * cpB * TB[i] - QB[i];
    QA[i] = (TW[i] - (TA[i-1] + TA[i]) / 2) * gammaA * omega * l;
    QB[i] = ((TB[i-1] + TB[i]) / 2 - TW[i]) * gammaB * omega * l;
    cpW / (N - 1) * der(TW[i]) = (-QA[i]) + QB[i];
  end for;
  QtotA = sum(QA);
  QtotB = sum(QB);
  annotation(Documentation(info = "<html><p>Cocurrent heat exchanger model consist of two channels A and B, and a separating heat transfer wall in between.  Fluids A and B are flowing in the channels A and B respectively. Heat exchanger is assumed to be insulated around the outside, therefore, heat transfer occurs just between the fluids A and B. Fluid B is considered as the hot fluid and fluid A is considered to be the cold fluid.</p><img src=\"modelica://ScalableTestSuite/Resources/Images/HeatExchanger/cocurrent.png\"/><p>Heat exchanger mass balance equations for the fluids can be written as:</p><img src=\"modelica://ScalableTestSuite/Resources/Images/HeatExchanger/massbalance.png\"/><p>where ρ is the density of the fluid, A is the cross section and w is the mass flow rate. Density and the cross section are assumed to be constant for the fluids, hence mass flow rate is considered constant along the channels.</p>Heat exchanger energy balance equations are described considering a small portion l=L/(N-1) on the channels where L is the length of each channel, and N is the number of nodes on the channels. Therefore, N-1 corresponds to the number of channel and wall segments. And, there are N-1 heat flow rates and N-1 temperature variables for the wall which are considered for the segments. <p>Energy balance equations for cocurrent heat exchanger is very similar to the equations of countercurrent heat exchanger. The difference occurs because of the flow direction of the fluid B, therefore, in cocurrent heat exchanger boundary condition of T_B is described for the first node. And, boundary condition of T_A remained again for the first node.</p> <img src=\"modelica://ScalableTestSuite/Resources/Images/HeatExchanger/energybalancecocurrent.png\"/><p>for j=1…N-1.
Energy balance at the each wall segment is modified in terms of temperature variables of fluid B:
</p><img src=\"modelica://ScalableTestSuite/Resources/Images/HeatExchanger/energywallcocurrent.png\"/><p>for j=1…N-1.</p><p>Parameters in the CounterCurrentHeatExchangerEquations:</p><table border=\"1\" cellspacing=\"0\" cellpadding=\"2\">
<tr>
  <th>Parameters</th>
  <th>Comment</th>
</tr>
            <tr>
  <td valign=\"top\">L</td>
  <td valign=\"top\">length of the channels</td>
</tr>
            <tr>
  <td valign=\"top\">N</td>
  <td valign=\"top\">number of nodes</td>
</tr>
<tr>
  <td valign=\"top\">wB</td>
  <td valign=\"top\">mass flow rate of fluid B</td>
</tr>
<tr>
  <td valign=\"top\">areaA</td>
  <td valign=\"top\">cross sectional area of channel A</td>
</tr>
<tr>
  <td valign=\"top\">areaB</td>
  <td valign=\"top\">cross sectional area of channel B</td>
</tr>
            <tr>
  <td valign=\"top\">rhoA</td>
  <td valign=\"top\">density of fluid A</td>
</tr>
            <tr>
  <td valign=\"top\">rhoB</td>
  <td valign=\"top\">density of fluid B</td>
</tr>
            <tr>
  <td valign=\"top\">cpA</td>
  <td valign=\"top\">specific heat capacity of fluid A</td>
</tr>
            <tr>
  <td valign=\"top\">cpB</td>
  <td valign=\"top\">specific heat capacity of fluid B</td>
</tr>
            <tr>
  <td valign=\"top\">cpW</td>
  <td valign=\"top\">specific heat capacity of the wall</td>
</tr>
            <tr>
  <td valign=\"top\">gammaA</td>
  <td valign=\"top\">heat transfer coefficient of fluid A</td>
</tr>
            <tr>
  <td valign=\"top\">gammaB</td>
  <td valign=\"top\">heat transfer coefficient of fluid B</td>
</tr>
            <tr>
  <td valign=\"top\">omega</td>
  <td valign=\"top\">perimeter</td>
</tr>
            <tr>
  <td valign=\"top\">l</td>
  <td valign=\"top\">length of each wall segment</td>
</tr>
</table>
</html>"));
end CocurrentHeatExchangerEquations;

